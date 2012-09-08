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
// Name : cgWinThreading.h                                                   //
//                                                                           //
// Desc : Provides threading classes specific to the Windows(tm) platform.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINTHREADING_H_ )
#define _CGE_CGWINTHREADING_H_

//-----------------------------------------------------------------------------
// cgWinThreading Header Includes
//-----------------------------------------------------------------------------
#include <System/cgThreading.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWinCriticalSection (Class)
/// <summary>
/// Windows(tm) platform specific critical section implementation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinCriticalSection : public cgCriticalSection
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWinCriticalSection( );
    virtual ~cgWinCriticalSection( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgCriticalSection)
    //-------------------------------------------------------------------------
    virtual bool        enter           ( );
    virtual bool        tryEnter        ( );
    virtual bool        exit            ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    CRITICAL_SECTION    mSection;

}; // End Class cgWinCriticalSection

//-----------------------------------------------------------------------------
//  Name : cgWinEvent (Class)
/// <summary>
/// Windows(tm) platform specific event implementation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinEvent : public cgEvent
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWinEvent( );
             cgWinEvent( bool autoReset );
    virtual ~cgWinEvent( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    HANDLE          getHandle       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgEvent)
    //-------------------------------------------------------------------------
    virtual bool    hasSignaled    ( );
    virtual void    signal         ( );
    virtual void    reset          ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    HANDLE mEvent;  // Handle for event

}; // End Class cgWinEvent

//-----------------------------------------------------------------------------
//  Name : cgWinThread (Class)
/// <summary>
/// Windows(tm) platform specific thread implementation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinThread : public cgThread
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWinThread( );
    virtual ~cgWinThread( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgThread)
    //-------------------------------------------------------------------------
    virtual void        setPriority         ( cgThreadPriority::Base priority );
    virtual void        setThreadData       ( cgThreadFunc workFunction, void * context );
    virtual ThreadState getThreadState      ( ) const;
    virtual bool        start               ( cgThreadFunc workFunction, void * context );
    virtual bool        start               ( );
    virtual void        suspend             ( );
    virtual void        resume              ( );
    virtual bool        suspendedWait       ( );
    virtual void        sleep               ( cgUInt32 milliseconds );
    virtual void        terminate           ( );
    virtual void        join                ( );
    virtual void        signalTerminate     ( );
    virtual bool        terminateRequested  ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    HANDLE                  mThread;                // Native handle for thread
    cgUInt32                mThreadId;              // Native identifier for the thread
    cgThreadFunc            mThreadFunction;        // User supplied callback function pointer to execute.
    void                  * mThreadContext;         // User supplied context data to pass to the thread function.
    cgWinEvent            * mExitEvent;             // Event signaled when the thread should exit.
    cgWinEvent            * mSuspendChangeEvent;    // Suspended state has changed.
    cgThreadPriority::Base  mPriority;              // Selected thread priority.
    volatile cgInt32        mSuspended;             // We are in a suspended state if > 0

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static DWORD WINAPI threadStub( void * param );

}; // End Class cgWinThread

    
#endif // !_CGE_CGWINTHREADING_H_