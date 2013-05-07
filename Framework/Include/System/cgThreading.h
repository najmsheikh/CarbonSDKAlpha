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
// Name : cgThreading.h                                                      //
//                                                                           //
// Desc : Provides platform independant base classes through which specific  //
//        implementations of system threading types (thread, critical        //
//        section, etc.) can be accessed.                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTHREADING_H_ )
#define _CGE_CGTHREADING_H_

//-----------------------------------------------------------------------------
// cgThreading Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgThread;

//-----------------------------------------------------------------------------
// Common Global Typedefs
//-----------------------------------------------------------------------------
typedef cgUInt32 (*cgThreadFunc)( cgThread * executingThread, void * context );

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgThreadPriority
{
    enum Base
    {
        Lowest = 0,
        BelowNormal,
        Normal,
        AboveNormal,
        Highest
    };
} // End Namespace : cgThreadPriority

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCriticalSection (Class)
/// <summary>
/// Abstract interface class through which critical sections can be
/// created, locked, unlocked and destroyed. Platform specific versions
/// of this class can be created through the 'createInstance()' method.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCriticalSection
{
public:
    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgCriticalSection * createInstance( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        enter           ( ) = 0;
    virtual bool        tryEnter        ( ) = 0;
    virtual bool        exit            ( ) = 0;

}; // End Class cgCriticalSection

//-----------------------------------------------------------------------------
//  Name : cgThread (Class)
/// <summary>
/// Abstract interface class through which platform threads can be
/// created, executed, suspended, etc. Platform specific versions
/// of this class can be created through the 'createInstance()' method.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgThread
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum ThreadState
    {
        Stopped,
        Running,
        Finished,
        Suspended
    };

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgThread * createInstance( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void        setPriority         ( cgThreadPriority::Base priority ) = 0;
    virtual void        setThreadData       ( cgThreadFunc workFunction, void * context ) = 0;
    virtual ThreadState getThreadState      ( ) const = 0;
    virtual bool        start               ( cgThreadFunc workFunction, void * context ) = 0;
    virtual bool        start               ( ) = 0;
    virtual void        suspend             ( ) = 0;
    virtual void        resume              ( ) = 0;
    virtual bool        suspendedWait       ( ) = 0;
    virtual void        sleep               ( cgUInt32 milliseconds ) = 0;
    virtual void        terminate           ( ) = 0;
    virtual void        signalTerminate     ( ) = 0;
    virtual void        join                ( ) = 0;
    virtual bool        terminateRequested  ( ) const = 0;

}; // End Class cgThread

//-----------------------------------------------------------------------------
//  Name : cgEvent (Class)
/// <summary>
/// Abstract interface class through which platform events can be
/// created, signalled, queried, etc. Platform specific versions of this
/// class can be created through the 'createInstance()' method.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgEvent
{
public:
    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgEvent * createInstance( bool autoReset = true );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool hasSignaled    ( ) = 0;
    virtual void signal         ( ) = 0;
    virtual void reset          ( ) = 0;

}; // End Class cgEvent

//-----------------------------------------------------------------------------
//  Name : cgThreadPool (Class)
/// <summary>
/// Static class through which threads can be managed. While it is possible for
/// the application to create standalone threads, this thread pool class
/// provides additional features such as monitoring and completion events.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgThreadPool
{
public:
    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgThread   * createThread    ( cgThreadFunc workFunction, void * context );
    static cgThread   * createThread    ( cgThreadFunc workFunction, void * context, cgThreadFunc completionFunction, void * completionContext );
    static void         shutdown        ( bool triggerCompletion );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    struct ThreadDesc
    {
        cgThread      * workThread;
        cgThreadFunc    completionFunction;
        void          * completionContext;
    
    }; // End Struct ThreadDesc
    CGE_LIST_DECLARE(ThreadDesc, ThreadList)

    //-------------------------------------------------------------------------
    // Private Statuc Functions
    //-------------------------------------------------------------------------
    static cgUInt32     monitorThreadFunction   ( cgThread * parentThread, void * context );
    
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static ThreadList           mThreads;
    static cgThread           * mMonitorThread;
    static cgCriticalSection  * mListSection;
};

#endif // !_CGE_CGTHREADING_H_