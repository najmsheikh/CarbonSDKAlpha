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
// Name : cgWinThreading.cpp                                                 //
//                                                                           //
// Desc : Provides threading classes specific to the Windows(tm) platform.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinThreading Module Includes
//-----------------------------------------------------------------------------
#include <System/Platform/cgWinThreading.h>

///////////////////////////////////////////////////////////////////////////////
// cgWinCriticalSection Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWinCriticalSection () (Constructor)
/// <summary>
/// cgWinCriticalSection Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinCriticalSection::cgWinCriticalSection()
{
    // Create the wrapped section.
    InitializeCriticalSection( &mSection );
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinCriticalSection () (Destructor)
/// <summary>
/// cgWinCriticalSection Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinCriticalSection::~cgWinCriticalSection()
{
    // Destroy the wrapped section
    DeleteCriticalSection( &mSection );
}

//-----------------------------------------------------------------------------
//  Name : enter () (Virtual)
/// <summary>
/// Enter / lock the critical section in order to protect a certain
/// block of code / data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinCriticalSection::enter( )
{
    EnterCriticalSection( &mSection );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : tryEnter () (Virtual)
/// <summary>
/// Attempt to enter / lock the critical section in order to protect a certain
/// block of code / data without blocking. If the section is already locked,
/// this method returns immediately.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinCriticalSection::tryEnter( )
{
    return ( TryEnterCriticalSection( &mSection ) != 0 );
}

//-----------------------------------------------------------------------------
//  Name : exit () (Virtual)
/// <summary>
/// Exit / unlock the critical section in order to release a certain
/// block of code / data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinCriticalSection::exit( )
{
    LeaveCriticalSection( &mSection );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgWinThread Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWinThread () (Constructor)
/// <summary>
/// cgWinThread Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinThread::cgWinThread()
{
    // Initialize variables to sensible defaults.
    mThread             = CG_NULL;
    mThreadId           = 0;
    mExitEvent          = CG_NULL;
    mSuspendChangeEvent = CG_NULL;
    mSuspended          = 0;
    mPriority           = cgThreadPriority::Normal;
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinThread () (Destructor)
/// <summary>
/// cgWinThread Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinThread::~cgWinThread()
{
    // Terminate any running thread and wait for it to stop.
    terminate();
}

//-----------------------------------------------------------------------------
//  Name : setPriority () (Virtual)
/// <summary>
/// Alter the priority of the running thread, or a thread due to be started.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::setPriority( cgThreadPriority::Base Priority )
{
    mPriority = Priority;
    if ( mThread )
    {
        cgInt nPriority = THREAD_PRIORITY_NORMAL;
        switch ( Priority )
        {
            case cgThreadPriority::Lowest:
                nPriority = THREAD_PRIORITY_LOWEST;
                break;
            case cgThreadPriority::BelowNormal:
                nPriority = THREAD_PRIORITY_BELOW_NORMAL;
                break;
            case cgThreadPriority::AboveNormal:
                nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            case cgThreadPriority::Highest:
                nPriority = THREAD_PRIORITY_HIGHEST;
                break;
        
        } // End switch priority
        SetThreadPriority( mThread, nPriority );
    
    } // End if thread running
}

//-----------------------------------------------------------------------------
//  Name : setThreadData () (Virtual)
/// <summary>
/// Set the thread function and context to use when the thread is 
/// next started.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::setThreadData( cgThreadFunc pFunc, void * pContext )
{
    mThreadFunction       = pFunc;
    mThreadContext    = pContext;
}

//-----------------------------------------------------------------------------
//  Name : getThreadState () (Virtual)
/// <summary>
/// Retrieve the current state of this thread.
/// </summary>
//-----------------------------------------------------------------------------
cgThread::ThreadState cgWinThread::getThreadState( ) const
{
    // Not started yet?
    if ( mThread == CG_NULL )
        return Stopped;

    // Thread is active?
    cgUInt32 nResult = ::WaitForSingleObject( mThread, 0 );
    if ( nResult != WAIT_OBJECT_0 )
    {
        // Suspended?
        if ( mSuspended > 0 )
            return Suspended;
        else
            return Running;

    } // End if thread running
    else
        return Finished;
}

//-----------------------------------------------------------------------------
//  Name : start () (Virtual)
/// <summary>
/// Start a new thread and execute the specified function asynchronously.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinThread::start( cgThreadFunc pFunc, void * pContext )
{
    // Stop any currently executing thread.
    terminate();

    // Store callback and context for execution.
    mThreadFunction    = pFunc;
    mThreadContext = pContext;

    // Start new thread
    return start( );
}

//-----------------------------------------------------------------------------
//  Name : start () (Virtual)
/// <summary>
/// Start a new thread and execute the supplied function asynchronously.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinThread::start( )
{
    // Stop any currently executing thread.
    terminate();

    // Create the events that allow us to signal for suspension and ending the thread.
    mExitEvent            = new cgWinEvent( false );
    mSuspendChangeEvent   = new cgWinEvent( false );

    // Create a new thread.
    if ( (mThread = CreateThread( 0, 0, threadStub, this, 0, &mThreadId )) != CG_NULL )
    {
        // Set initial thread priority
        setPriority( mPriority );
        return true;
    
    } // End if started

    // Failed
    delete mExitEvent;
    delete mSuspendChangeEvent;
    mExitEvent          = CG_NULL;
    mSuspendChangeEvent = CG_NULL;
    return false;
}

//-----------------------------------------------------------------------------
//  Name : suspend () (Virtual)
/// <summary>
/// Signal the thread that it should suspend its operations.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::suspend( )
{
    // Wait for the suspended variable to be udpated
    while( 1 != ::InterlockedExchange( &mSuspended, 1 ) )
    {
    }

    // Fire suspend change event
    mSuspendChangeEvent->signal();
}

//-----------------------------------------------------------------------------
// Name : resume() (Virtual)
/// <summary>Resume execution of the thread.</summary>
//-----------------------------------------------------------------------------
void cgWinThread::resume( )
{
    // Wait for the suspended variable to be udpated
    while( 0 != ::InterlockedExchange( &mSuspended, 0 ) )
    {
    }

    // Fire suspend change event
    mSuspendChangeEvent->signal();
}

//-----------------------------------------------------------------------------
// Name : join()
/// <summary>Wait for execution of the thread to complete.</summary>
//-----------------------------------------------------------------------------
void cgWinThread::join( )
{
    if ( mThread != CG_NULL )
        WaitForSingleObject( mThread, INFINITE );
}

//-----------------------------------------------------------------------------
// Name : suspendedWait() (Virtual)
/// <summary>
/// Call this method within the code being executed by the thread in order to 
/// wait if the thread is suspended.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinThread::suspendedWait( )
{
    if ( mSuspendChangeEvent == CG_NULL )
        return false;

    // Has the state of the suspended flag changed since we were last called?
    bool bSuspendChanged = mSuspendChangeEvent->hasSignaled();
    if ( bSuspendChanged == true )
    {
        bool bShouldWait = (mSuspended != 0);
        mSuspendChangeEvent->reset();
                
        // If we need to suspend, burn time until the state changes again
        if ( bShouldWait == true )
        {
            // Wait for either the suspend or terminate events to signal.
            HANDLE pHandles[] = { mSuspendChangeEvent->getHandle(), mExitEvent->getHandle() };
            WaitForMultipleObjects( 2, pHandles, FALSE, INFINITE );
            return true;

        } // End if should suspend

    } // End if suspend changed

    // No need to wait
    return false;
}

//-----------------------------------------------------------------------------
//  Name : threadStub () (Private Static)
/// <summary>
/// Win32 compatible thread function providing a wrapper around the user
/// supplied callback data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWinThread::threadStub( void * pParam )
{
    // Call the user suppliead thread callback function.
    cgWinThread * pThread = (cgWinThread*)pParam;
    cgThreadFunc UserFunc = pThread->mThreadFunction;
    if ( UserFunc != NULL )
        return UserFunc( pThread, pThread->mThreadContext );
    return 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : terminate () (Virtual)
/// <summary>
/// Stop any currently executing thread and block until it exits.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::terminate( )
{
    if ( mThread != CG_NULL )
    {
        // Issue quit message and wait for thread to exit.
        signalTerminate();
        join( );
        CloseHandle( mThread );
    
    } // End if started

    // Release memory
    cgWinEvent * pEvent = mExitEvent;
    mExitEvent = CG_NULL;
    delete pEvent;
    pEvent = mSuspendChangeEvent;
    mSuspendChangeEvent = CG_NULL;
    delete pEvent;

    // Clean up
    mThread    = CG_NULL;
    mThreadId  = 0;
}

//-----------------------------------------------------------------------------
//  Name : sleep () (Virtual)
/// <summary>
/// Sleep the calling thread for the specified number of milliseconds.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::sleep( cgUInt32 nMilliseconds )
{
    // Cheap "hack" :D Use platform (Sleep) for now.
    ::Sleep( nMilliseconds );
}

//-----------------------------------------------------------------------------
//  Name : signalTerminate () (Virtual)
/// <summary>
/// Simply sends the signal for the thread to exit but does not wait for it
/// to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinThread::signalTerminate( )
{
    // Issue quit message if thread is running.
    if ( mThread != CG_NULL && mExitEvent != CG_NULL )
        mExitEvent->signal();
}

//-----------------------------------------------------------------------------
//  Name : terminateRequested () (Virtual)
/// <summary>
/// Determine if someone has requested that the thread be shut down.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinThread::terminateRequested( ) const
{
    // Always assume we are waiting for an exit if the event is dead.
    if ( mExitEvent == CG_NULL )
        return true;

    // exit signalled?
    return mExitEvent->hasSignaled();
}

///////////////////////////////////////////////////////////////////////////////
// cgWinEvent Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWinEvent () (Constructor)
/// <summary>
/// cgWinEvent Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinEvent::cgWinEvent( )
{
    // Initialize variables to sensible defaults.
    mEvent = CreateEvent( CG_NULL, FALSE, FALSE, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : cgWinEvent () (Constructor)
/// <summary>
/// cgWinEvent Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinEvent::cgWinEvent( bool bAutoReset )
{
    // Initialize variables to sensible defaults.
    mEvent = CreateEvent( CG_NULL, (bAutoReset == true) ? FALSE : TRUE, FALSE, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinEvent () (Destructor)
/// <summary>
/// cgWinEvent Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinEvent::~cgWinEvent()
{
    // Release the event handle.
    if ( mEvent != NULL )
        CloseHandle( mEvent );

    // Clear variables
    mEvent = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : hasSignaled () (Virtual)
/// <summary>
/// Determine if the event is currently in a signaled state.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinEvent::hasSignaled( )
{
    cgUInt32 nResult = ::WaitForSingleObject( mEvent, 0 );
    return ( nResult == WAIT_OBJECT_0 );
}

//-----------------------------------------------------------------------------
//  Name : signal () (Virtual)
/// <summary>
/// Set / signal the event.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinEvent::signal( )
{
    ::SetEvent( mEvent );
}

//-----------------------------------------------------------------------------
//  Name : reset () (Virtual)
/// <summary>
/// Reset the event / signal.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinEvent::reset( )
{
    ::ResetEvent( mEvent );
}

//-----------------------------------------------------------------------------
//  Name : getHandle () (Virtual)
/// <summary>
/// Retrieve the OS event handle.
/// </summary>
//-----------------------------------------------------------------------------
HANDLE cgWinEvent::getHandle( ) const
{
    return mEvent;
}