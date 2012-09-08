//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : cgThreading.cpp                                                    //
//                                                                           //
// Desc : Provides platform independant base classes through which specific  //
//        implementations of system threading types (thread, critical        //
//        section, etc.) can be accessed.                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgThreading Module Includes
//-----------------------------------------------------------------------------
#include <System/cgThreading.h>
#include <System/Platform/cgWinThreading.h>
#include <cgBase.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgThreadPool::ThreadList    cgThreadPool::mThreads;
cgThread                  * cgThreadPool::mMonitorThread = CG_NULL;
cgCriticalSection         * cgThreadPool::mListSection = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgCriticalSection Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgCriticalSection * cgCriticalSection::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinCriticalSection();
    
    } // End Switch platform
    return CG_NULL;
}

///////////////////////////////////////////////////////////////////////////////
// cgThread Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgThread * cgThread ::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinThread();
    
    } // End Switch platform
    return CG_NULL;
}

///////////////////////////////////////////////////////////////////////////////
// cgEvent Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgEvent * cgEvent::createInstance( bool bAutoReset /* = true */ )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinEvent( bAutoReset );
    
    } // End Switch platform
    return CG_NULL;
}
    
///////////////////////////////////////////////////////////////////////////////
// cgThreadPool Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : createThread () (Static)
/// <summary>
/// Create a new thread object and add it to the pool. This thread will be
/// monitored and automatically destroyed as soon as it has completed.
/// </summary>
//-----------------------------------------------------------------------------
cgThread * cgThreadPool::createThread( cgThreadFunc pFunc, void * pContext )
{
    return createThread( pFunc, pContext, CG_NULL, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : createThread () (Static)
/// <summary>
/// Create a new thread object and add it to the pool. A 'completion' callback
/// can also be supplied that will be triggered when the thread completes and
/// just before it is automatically destroyed.
/// </summary>
//-----------------------------------------------------------------------------
cgThread * cgThreadPool::createThread( cgThreadFunc pFunc, void * pContext, cgThreadFunc pCompleteFunc, void * pCompleteContext )
{
    // Create the critical section that protects the
    // main thread list if we haven't already created it.
    if ( !mListSection )
        mListSection = cgCriticalSection::createInstance();

    // Create the requested thread.
    ThreadDesc Desc;
    Desc.workThread          = cgThread::createInstance();
    Desc.completionFunction    = pCompleteFunc;
    Desc.completionContext = pCompleteContext;
    Desc.workThread->setThreadData( pFunc, pContext );

    // Add it to the list of monitored threads.
    mListSection->enter();
	bool bFirstThread = mThreads.empty();
    mThreads.push_back( Desc );
    mListSection->exit();

    // Fire up the monitoring thread if this is the first item
    // in the list.
    if ( bFirstThread )
    {
        // First time creating the thread class?
        if ( !mMonitorThread )
        {
            mMonitorThread = cgThread::createInstance();
            mMonitorThread->setThreadData( cgThreadPool::monitorThreadFunction, CG_NULL );

        } // End if create

        // start the thread running.
        mMonitorThread->start();

    } // End if first thread

    // Success!
    return Desc.workThread;
}

//-----------------------------------------------------------------------------
//  Name : shutdown () (Static)
/// <summary>
/// Shuts down the entire pool. Thread completion functions can optionally
/// be triggered as required.
/// </summary>
//-----------------------------------------------------------------------------
void cgThreadPool::shutdown( bool bTriggerCompletion )
{
    // First, stop the monitoring thread if it is active.
    if ( mMonitorThread != CG_NULL )
    {
        mMonitorThread->terminate();
        delete mMonitorThread;
        mMonitorThread = CG_NULL;
    
    } // End if monitoring

    // Stop and destroy any remaining threads.
    if ( mListSection != CG_NULL )
    {
        mListSection->enter();
        ThreadList::iterator itThread;
        for ( itThread = mThreads.begin(); itThread != mThreads.end(); ++itThread )
        {
            ThreadDesc & Desc = (*itThread);

            // Request that the thread stop and then wait but do NOT clean 
            // up the thread data yet (that will be done in the destructor).
            // This ensures that the state of the thread remains 'Finished'
            // instead of 'Stopped'.
            Desc.workThread->signalTerminate();
            Desc.workThread->join();
            
            // Notify the caller that it completed if necessary.
            if ( bTriggerCompletion == true && Desc.completionFunction != CG_NULL )
                Desc.completionFunction( Desc.workThread, Desc.completionContext );

            // Finally destroy the thread.
            delete Desc.workThread;

        } // Next Thread
        mThreads.clear();

        // Clean up
        mListSection->exit();
        delete mListSection;
        mListSection = CG_NULL;

    } // End if list populated.
}

//-----------------------------------------------------------------------------
//  Name : monitorThreadFunction () (Private, Static)
/// <summary>
/// Monitors the status of all other currently running threads.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgThreadPool::monitorThreadFunction( cgThread * pThread, void * pContext )
{
    bool bExit = false;

    // Thread runs until there are no more threads to monitor
    // or the system requests that we exit..
    while ( bExit == false && pThread->terminateRequested() == false )
    {
        // Check the status of all running threads.
        mListSection->enter();
        ThreadList::iterator itThread, itErase;
        for ( itThread = mThreads.begin(); itThread != mThreads.end(); )
        {
            ThreadDesc & Desc = (*itThread);
            
            // Move to next element in case list is altered.
            itErase = itThread++;
            
            // Has the thread finished yet?
            if ( Desc.workThread->getThreadState() == cgThread::Finished )
            {
                // Notify the caller that the thread completed.
                if ( Desc.completionFunction != CG_NULL )
                    Desc.completionFunction( Desc.workThread, Desc.completionContext );

                // Clean up the thread.
                delete Desc.workThread;
                mThreads.erase( itErase );

            } // End if completed

        } // Next Thread

        // If there are no longer any items to monitor then we should exit the loop.
        if ( mThreads.empty() == true )
            bExit = true;
        mListSection->exit();

    } // Next Iteration
    return 0;
}