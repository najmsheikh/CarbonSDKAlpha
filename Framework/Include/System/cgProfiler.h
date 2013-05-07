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
// Name : cgProfiler.h                                                       //
//                                                                           //
// Desc : General profiling (timing) and statistics gathering class.         //
//        Singleton allocation grants application wide access to this        //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPROFILER_H_ )
#define _CGE_CGPROFILER_H_

//-----------------------------------------------------------------------------
// cgProfiler Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgTimer.h>
#include <Scripting/cgScriptInterop.h>
#include <sstream>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBroadcast;

//-----------------------------------------------------------------------------
// Application Wide Enumerations
//-----------------------------------------------------------------------------
/// <summary>Broadcast system command codes</summary>
namespace cgBroadcastCommand
{
    namespace Profiler
    {
        enum
        {
            /// <summary>New data is available after frame completed.</summary>
            FrameData = 0x0100
        };

    }; // End Namespace : Profiler

}; // End Namespace : cgBroadcastCommand

//-----------------------------------------------------------------------------
// Application Wide Structures
//-----------------------------------------------------------------------------
/// <summary>
/// Allows us to record the amount of time spent in each of several per-frame 
/// processes. All periods are measured in units equivalent to those returned 
/// by the hardware performance counter.
/// </summary>
struct CGE_API cgProfilerStatistics
{
    CGE_MAP_DECLARE(cgString, cgProfilerStatistics, Map )
    CGE_STACK_DECLARE(cgProfilerStatistics*, Stack )
    
    /// <summary>Structure that represent the various ranges of values seen for this process.</summary>
    struct CGE_API Values
    {
        /// <summary>Total number of samples considered for this set of values.</summary>
        cgInt64     sampleCount;
        /// <summary>Total time (or other unit) recorded within this process since the application began.</summary>
        cgInt64     total;
        /// <summary>Smallest time period (or other unit) recorded within this process.</summary>
        cgInt64     minimum;
        /// <summary>Largest time period (or other unit) recorded within this process.</summary>
        cgInt64     maximum;
        /// <summary>Exact time period (or other unit) recorded within the most recent measurement.</summary>
        cgInt64     current;

        // Constructor
        Values( ) :
            sampleCount( 0 ),
            total      ( 0 ),
            minimum    ( 0 ),
            maximum    ( 0 ),
            current    ( 0 ) {}
    };

    /// <summary>Unique identifier for this statistic.</summary>
    cgInt32     statisticId;
    /// <summary>Unique identifier of the parent process of this statistic.</summary>
    cgInt32     parentId;
    /// <summary>The most recent frame on which data for this process was sampled.</summary>
    cgInt64     lastFrameSample;    
    /// <summary>Internal value for used during the measurement process for any required purpose.</summary>
    cgInt64     sampleData;

    /// <summary>Overall range of values seen during the entire application lifetime.</summary>
    Values      overall;
    /// <summary>Internal set of values used to accumulate data for the period described by 'cgProfiler::mSnapshotInterval' (transferred to 'cgProfiler::snapshot').</summary>
    Values      recent;
    /// <summary>Range of values seen for the period described by 'cgProfiler::mSnapshotInterval'.</summary>
    Values      snapshot;
    /// <summary>Time at which the 'snapshot' values were most recently taken (see cgProfilerStatistics::snapshot).</summary>
    cgInt64     snapshotTime;
    
    /// <summary>Child processes (if any).</summary>
    Map         children;

    // Constructor
    cgProfilerStatistics( ) :
        statisticId     ( -1 ),
        parentId        ( -1 ),
        lastFrameSample ( -1 ),
        sampleData      ( 0 ),
        snapshotTime    ( 0 ) {}


}; // End Struct cgProfilerStatistics

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgProfiler (Class)
/// <summary>
/// Subsystem profiling and timing utility class.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProfiler : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgProfiler, "Profiler" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProfiler( );
    virtual ~cgProfiler( );

    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct CGE_API InitConfig                   // The selected profiler configuration options
    {
        bool                enabled;            // Is profiling enabled in general?
        bool                broadcastData;      // Start a broadcast server capable of distributing data to any connected clients.
        cgUInt16            broadcastPort;      // The port on which to listen if broadcasting profiler data.
        cgFloat             broadcastInterval;  // Interval, in seconds, at which the profiler data will be broadcast.
        bool                outputMarkers;      // Output performance markers to any performance monitoring API (i.e. D3DPERF)
        
        // Constructor
        InitConfig()
        {
            enabled             = false;
            broadcastData       = false;
            broadcastPort       = 46352;
            broadcastInterval   = 0.25f;
            outputMarkers       = false;
            
        } // End Constructor
    };

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgProfiler     * getInstance         ( );
    static void             createSingleton     ( );
    static void             destroySingleton    ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Configuration and Capabilities
    bool                    loadConfig          ( const cgString & fileName );
    bool                    saveConfig          ( const cgString & fileName );
    const InitConfig      & getConfig           ( ) const;
    bool                    initialize          ( );

    // Profiling
    void                    beginFrame          ( );
    void                    beginProcess        ( const cgString & processName );
    void                    beginProcess        ( const cgString & processName, bool asRoot );
    void                    endProcess          ( );
    void                    endFrame            ( );
    void                    primitivesDrawn     ( cgUInt32 primitiveCount );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    broadcastProfile    ( );
    void                    broadcastStatTree   ( const cgString & name, const cgProfilerStatistics & stats, std::stringstream & outStream );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Profiler configuration settings.</summary>
    InitConfig                      mConfig;
    /// <summary>Measured period (time) statistics data for individual named processes (i.e. update, visibility, render, etc.)</summary>
    cgProfilerStatistics::Map       mRootProcessStats;
    /// <summary>Measured period (time) statistics data for each 'frame' as a whole.</summary>
    cgProfilerStatistics            mFrameStats;
    /// <summary>Measured primitive rendering statistics data for each 'frame' as a whole.</summary>
    cgProfilerStatistics            mPrimitiveStats;
    /// <summary>The current stack of processes being updated (currently within beginProcess, endProcess calls).
    cgProfilerStatistics::Stack     mCurrentProcesses;
    /// <summary>Our own local timer class for running the profile.</summary>
    cgTimer                         mTimer;
    /// <summary>Has a call to 'beginFrame()' been made?</summary>
    bool                            mFrameBegun;
    /// <summary>Amount of time, in seconds, to update the 'current' member of each recorded stat (except frame stats which always update at 1 second intervals).</summary>
    cgDouble                        mSnapshotInterval;
    /// <summary>If enabled, this is the broadcast server that will be used to export profiler data to any connected clients.</summary>
    cgBroadcast                   * mBroadcastServer;
    /// <summary>The next unique identifier to assign to a particular measured statistic.</summary>
    cgUInt32                        mNextStatId;

private:
    //-------------------------------------------------------------------------
    // Private Static Variables.
    //-------------------------------------------------------------------------
    static cgProfiler * mSingleton;       // Static singleton object instance.
};

#endif // !_CGE_CGPROFILER_H_

