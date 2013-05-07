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
// Name : cgProfiler.cpp                                                     //
//                                                                           //
// Desc : General profiling (timing) and statistics gathering class.         //
//        Singleton allocation grants application wide access to this        //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgProfiler Module Includes
//-----------------------------------------------------------------------------
#include <System/cgProfiler.h>
#include <System/cgStringUtility.h>
#include <Network/cgBroadcast.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>            // ToDo: GetPrivateProfile*(). Remove when we move to a scripted config (or XML).
#include <d3d9.h>
#undef WIN32_LEAN_AND_MEAN

cgToDo( "Carbon General", "Pay attention to a broadcast frequency." )

//-----------------------------------------------------------------------------
// Static Member Definitions.
//-----------------------------------------------------------------------------
cgProfiler * cgProfiler::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgProfiler Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProfiler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProfiler::cgProfiler()
{
    // initialize variables to sensible defaults
    mFrameBegun       = false;
    mNextStatId       = 0x100;
    mSnapshotInterval = 1.0;
    mBroadcastServer  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgProfiler () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProfiler::~cgProfiler()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgProfiler::dispose( bool bDisposeBase )
{
    // Reset variables
    mNextStatId = 0x100;

    // Shut down broadcast server and disconnect all clients if allocated.
    if ( mBroadcastServer )
        mBroadcastServer->scriptSafeDispose();
    mBroadcastServer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgProfiler * cgProfiler::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgProfiler();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton != CG_NULL )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : loadConfig ()
/// <summary>
/// Load the resource configuration from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProfiler::loadConfig( const cgString & strFileName )
{
    // Validate requirements
    if ( strFileName.empty() )
        return false;

    // Retrieve configuration options
    mConfig.enabled           = GetPrivateProfileInt( _T("Profiler"), _T("Enabled"), 0, strFileName.c_str() ) > 0;
    mConfig.broadcastData     = GetPrivateProfileInt( _T("Profiler"), _T("BroadcastData"), 0, strFileName.c_str() ) > 0;
    mConfig.broadcastPort     = GetPrivateProfileInt( _T("Profiler"), _T("BroadcastPort"), 46352, strFileName.c_str() );
    mConfig.broadcastInterval = cgStringUtility::getPrivateProfileFloat( _T("Profiler"), _T("BroadcastInterval"), 0.25f, strFileName.c_str() );
    mConfig.outputMarkers     = GetPrivateProfileInt( _T("Profiler"), _T("OutputMarkers"), 0, strFileName.c_str() ) > 0;
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : saveConfig ()
/// <summary>
/// Save the configuration to the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProfiler::saveConfig( const cgString & strFileName )
{
    LPCTSTR strSection = _T("Profiler");

    // Validate requirements
    if ( strFileName.empty() )
        return false;

    // Save configuration options
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("Enabled"), mConfig.enabled, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("BroadcastData"), mConfig.broadcastData, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("BroadcastPort"), mConfig.broadcastPort, strFileName.c_str() );
    cgStringUtility::writePrivateProfileFloat( strSection, _T("BroadcastInterval"), mConfig.broadcastInterval, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("OutputMarkers"), mConfig.outputMarkers, strFileName.c_str() );
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the profiler.
/// </summary>
//-----------------------------------------------------------------------------
const cgProfiler::InitConfig & cgProfiler::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// initialize the profiler. Starts listening for broadcast server connections
/// if this feature was enabled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProfiler::initialize( )
{
    // Network profiler broadcasting enabled?
    if ( mConfig.enabled && mConfig.broadcastData )
    {
        mBroadcastServer = cgBroadcast::createInstance();

        // Start listening for connections.
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Starting profiler broadcast server. Listening for incoming connections on port %i.\n"), mConfig.broadcastPort );
        if ( !mBroadcastServer->listenForConnections( mConfig.broadcastPort, _T("Core::Broadcasters::cgProfiler"), 1000, 1000 ) )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to start application profiler broadcast server. Profiling will continue, but connections will not be accepted.\n") );
            mBroadcastServer->scriptSafeDispose();
            mBroadcastServer = CG_NULL;
            return false;

        } // End if failed

    } // End if broadcast

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginFrame()
/// <summary>
/// Begin profiling a new frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::beginFrame( )
{
    // Already within a frame?
    if ( mFrameBegun )
        return;

    // New frame processing has begun.
    mFrameBegun = true;

    // Reset geometry statistics for this new frame.
    mPrimitiveStats.sampleData = 0;

    // Record the initial time for the start of the frame (in performance 
    // counter cycles). We do this at the very end in order to ensure 
    // minimal superfluous overhead is included in the timing.
    mFrameStats.sampleData = mTimer.getPerfomanceCounter(true);
}

//-----------------------------------------------------------------------------
//  Name : beginProcess()
/// <summary>
/// Begin the profiling of a process (can be called in a hierarchical fashion).
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::beginProcess( const cgString & Process )
{
    // Forcibly disabled?
    if ( !mConfig.enabled )
        return;

    // Root process, or child?
    cgInt32 nParentId = -1;
    cgProfilerStatistics * pProcess = CG_NULL;
    if ( !mCurrentProcesses.empty() )
    {
        // This is a child process.
        cgProfilerStatistics & TopProcess = *mCurrentProcesses.top();
        pProcess = &TopProcess.children[ Process ];
        nParentId = TopProcess.statisticId;

    } // End if child process
    else
    {
        // This is a root process.
        pProcess = &mRootProcessStats[ Process ];
        
    } // End if root process

    // Assign an identifier if this is a new process.
    if ( pProcess->statisticId < 0 )
    {
        pProcess->statisticId = mNextStatId++;
        pProcess->parentId    = nParentId;
    
    } // End if new stat

    // Record it as in-progress.
    mCurrentProcesses.push( pProcess );

    // Record the initial time for the start of the frame (in performance 
    // counter cycles). We do this at the very end in order to ensure 
    // minimal superfluous overhead is included in the timing.
    pProcess->sampleData = mTimer.getPerfomanceCounter(true);

    // Output performance marker if enabled.
    if ( mConfig.outputMarkers )
    {
        cgRenderAPI::Base RenderAPI = cgGetEngineConfig().renderAPI;
        switch ( RenderAPI )
        {
            case cgRenderAPI::Null:
                break;

#if defined( CGE_DX9_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX9:
            {
                STRING_CONVERT;
                D3DPERF_BeginEvent( 0xFF00FF00, stringConvertT2CW(Process.c_str()) );
                break;
        
            } // End case DirectX9

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX11:
            {
                STRING_CONVERT;
                D3DPERF_BeginEvent( 0xFF00FF00, stringConvertT2CW(Process.c_str()) );
                break;
        
            } // End case DirectX11

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch RenderAPI
    
    } // End if outputMarkers
}

//-----------------------------------------------------------------------------
//  Name : beginProcess()
/// <summary>
/// Begin the profiling of a process (can be called in a hierarchical fashion).
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::beginProcess( const cgString & Process, bool bAsRoot )
{
    // Forcably disabled?
    if ( !mConfig.enabled )
        return;

    // Root process, or child?
    cgInt32 nParentId = -1;
    cgProfilerStatistics * pProcess = CG_NULL;
    if ( bAsRoot == false && !mCurrentProcesses.empty() )
    {
        // This is a child process.
        cgProfilerStatistics & TopProcess = *mCurrentProcesses.top();
        pProcess = &TopProcess.children[ Process ];
        nParentId = TopProcess.statisticId;

    } // End if child process
    else
    {
        // This is a root process.
        pProcess = &mRootProcessStats[ Process ];
        
    } // End if root process

    // Assign an identifier if this is a new process.
    if ( pProcess->statisticId < 0 )
    {
        pProcess->statisticId = mNextStatId++;
        pProcess->parentId    = nParentId;
    
    } // End if new stat

    // Record it as in-progress.
    mCurrentProcesses.push( pProcess );

    // Record the initial time for the start of the frame (in performance 
    // counter cycles). We do this at the very end in order to ensure 
    // minimal superfluous overhead is included in the timing.
    pProcess->sampleData = mTimer.getPerfomanceCounter(true);

    // Output performance marker if enabled.
    if ( mConfig.outputMarkers )
    {
        cgRenderAPI::Base RenderAPI = cgGetEngineConfig().renderAPI;
        switch ( RenderAPI )
        {
            case cgRenderAPI::Null:
                break;

#if defined( CGE_DX9_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX9:
            {
                STRING_CONVERT;
                D3DPERF_BeginEvent( 0xFF00FF00, stringConvertT2CW(Process.c_str()) );
                break;
        
            } // End case DirectX9

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX11:
            {
                STRING_CONVERT;
                D3DPERF_BeginEvent( 0xFF00FF00, stringConvertT2CW(Process.c_str()) );
                break;
        
            } // End case DirectX11

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch RenderAPI
    
    } // End if outputMarkers
}

//-----------------------------------------------------------------------------
//  Name : endProcess()
/// <summary>
/// Complete the profiling of the process most recently begun (can be called 
/// in a hierarchical fashion).
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::endProcess( )
{
    // Forcably disabled?
    if ( !mConfig.enabled )
        return;

    // Capture the current performance counter time before we
    // do any more work to avoid unnecessary overhead skewing
    // the results.
    cgInt64 nCurrentTime = mTimer.getPerfomanceCounter(true);

    // Not a valid 'end'?
    if ( mCurrentProcesses.empty() )
        return;    

    // Output performance marker if enabled.
    if ( mConfig.outputMarkers )
    {
        cgRenderAPI::Base RenderAPI = cgGetEngineConfig().renderAPI;
        switch ( RenderAPI )
        {
            case cgRenderAPI::Null:
                break;

#if defined( CGE_DX9_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX9:
                D3DPERF_EndEvent( );
                break;
        
#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT ) 

            case cgRenderAPI::DirectX11:
                D3DPERF_EndEvent( );
                break;

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI
    
    } // End if outputMarkers

    // Retrieve the process currently being sampled and then
    // mark it as sampled (includes removal from the process stack).
    cgProfilerStatistics * pProcess = mCurrentProcesses.top();
    pProcess->lastFrameSample = mFrameStats.overall.sampleCount;
    mCurrentProcesses.pop();

    // Measure the amount of time for this run of the process (in performance counter cycles).
    cgInt64 nPeriod = nCurrentTime - pProcess->sampleData;

    // Update the 'overall' (application lifetime) statistics.
    cgProfilerStatistics::Values & Overall = pProcess->overall;
    ++Overall.sampleCount;
    Overall.current = nPeriod;
    Overall.total  += nPeriod;
    if ( Overall.sampleCount == 1 || nPeriod < Overall.minimum )
        Overall.minimum = nPeriod;
    if ( Overall.sampleCount == 1 || nPeriod > Overall.maximum )
        Overall.maximum = nPeriod;

    // ...and again for the 'recent' sample values.
    cgProfilerStatistics::Values & Recent  = pProcess->recent;
    ++Recent.sampleCount;
    Recent.current = nPeriod;
    Recent.total  += nPeriod;
    if ( Recent.sampleCount == 1 || nPeriod < Recent.minimum )
        Recent.minimum = nPeriod;
    if ( Recent.sampleCount == 1 || nPeriod > Recent.maximum )
        Recent.maximum = nPeriod;
    
    // Enough time has elapsed to take a new 'snapshot' sample?
    if ( mTimer.measurePeriod( pProcess->snapshotTime, nCurrentTime ) >= mSnapshotInterval )
    {
        // Snapshot recent values and then reset.
        pProcess->snapshot     = pProcess->recent;
        pProcess->recent       = cgProfilerStatistics::Values();
        pProcess->snapshotTime = nCurrentTime;
    
    } // End if update snapshot
}

//-----------------------------------------------------------------------------
//  Name : beginFrame()
/// <summary>
/// Complete the profiling of the current frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::endFrame( )
{
    // Forcably disabled?
    if ( !mConfig.enabled )
        return;

    // Capture the current performance counter time before we
    // do any more work to avoid unnecessary overhead skewing
    // the results.
    cgInt64 nCurrentTime = mTimer.getPerfomanceCounter(true);

    // Must have started a new frame.
    if ( !mFrameBegun )
        return;

    // A new frame has been (entirely) sampled.
    ++mFrameStats.lastFrameSample;

    // Measure the amount of time for the entire frame (in performance counter cycles).
    cgInt64 nPeriod = nCurrentTime - mFrameStats.sampleData;

    // Update the 'overall' (application lifetime) statistics.
    ++mFrameStats.overall.sampleCount;
    mFrameStats.overall.current = nPeriod;
    mFrameStats.overall.total  += nPeriod;
    if ( mFrameStats.overall.sampleCount == 1 || nPeriod < mFrameStats.overall.minimum )
        mFrameStats.overall.minimum = nPeriod;
    if ( mFrameStats.overall.sampleCount == 1 || nPeriod > mFrameStats.overall.maximum )
        mFrameStats.overall.maximum = nPeriod;

    // ...and again for the 'recent' sample values.
    ++mFrameStats.recent.sampleCount;
    mFrameStats.recent.current = nPeriod;
    mFrameStats.recent.total  += nPeriod;
    if ( mFrameStats.recent.sampleCount == 1 || nPeriod < mFrameStats.recent.minimum )
        mFrameStats.recent.minimum = nPeriod;
    if ( mFrameStats.recent.sampleCount == 1 || nPeriod > mFrameStats.recent.maximum )
        mFrameStats.recent.maximum = nPeriod;
    
    // Enough time has elapsed to take a new 'snapshot' sample (always 1.0
    // for frame statistics -- frames per second)?
    if ( mTimer.measurePeriod( mFrameStats.snapshotTime, nCurrentTime ) >= 1.0 )
    {
        // Snapshot recent values and then reset.
        mFrameStats.snapshot     = mFrameStats.recent;
        mFrameStats.recent       = cgProfilerStatistics::Values();
        mFrameStats.snapshotTime = nCurrentTime;
    
    } // End if update snapshot

    // Now the same again for the primitive 'overall' statistics.
    cgInt64 nPrimitives = mPrimitiveStats.sampleData;
    ++mPrimitiveStats.overall.sampleCount;
    mPrimitiveStats.overall.current = nPrimitives;
    mPrimitiveStats.overall.total  += nPrimitives;
    if ( mPrimitiveStats.overall.sampleCount == 1 || nPrimitives < mPrimitiveStats.overall.minimum )
        mPrimitiveStats.overall.minimum = nPrimitives;
    if ( mPrimitiveStats.overall.sampleCount == 1 || nPrimitives > mPrimitiveStats.overall.maximum )
        mPrimitiveStats.overall.maximum = nPrimitives;

    // ...and again for the 'recent' sample values.
    ++mPrimitiveStats.recent.sampleCount;
    mPrimitiveStats.recent.current = nPrimitives;
    mPrimitiveStats.recent.total  += nPrimitives;
    if ( mPrimitiveStats.recent.sampleCount == 1 || nPrimitives < mPrimitiveStats.recent.minimum )
        mPrimitiveStats.recent.minimum = nPrimitives;
    if ( mPrimitiveStats.recent.sampleCount == 1 || nPrimitives > mPrimitiveStats.recent.maximum )
        mPrimitiveStats.recent.maximum = nPrimitives;
    
    // Enough time has elapsed to take a new 'snapshot' sample?
    if ( mTimer.measurePeriod( mPrimitiveStats.snapshotTime, nCurrentTime ) >= mSnapshotInterval )
    {
        // Snapshot recent values and then reset.
        mPrimitiveStats.snapshot     = mPrimitiveStats.recent;
        mPrimitiveStats.recent       = cgProfilerStatistics::Values();
        mPrimitiveStats.snapshotTime = nCurrentTime;
    
    } // End if update snapshot

    // Frame processing complete
    mFrameBegun = false;

    // Send data to broadcast clients if appropriate.
    broadcastProfile();
}

//-----------------------------------------------------------------------------
//  Name : broadcastProfile() (Protected)
/// <summary>
/// Send profile statistics to all connected broadcast server clients.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::broadcastProfile( )
{
    // Valid to broadcast?
    if ( !mConfig.enabled || !mConfig.broadcastData || !mBroadcastServer )
        return;

    // Output configuration details first.
    std::stringstream Out;
    cgInt64 nPerfFrequency = mTimer.getPerfomanceCounterFrequency();
    Out.write( (char*)&nPerfFrequency, sizeof(cgInt64) );
    Out.write( (char*)&mSnapshotInterval, sizeof(cgDouble) );

    // Output frame statistics
    Out.write( (char*)&mFrameStats.lastFrameSample, sizeof(cgInt64) );
    Out.write( (char*)&mFrameStats.snapshotTime, sizeof(cgInt64) );
    Out.write( (char*)&mFrameStats.overall, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&mFrameStats.recent, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&mFrameStats.snapshot, sizeof(cgProfilerStatistics::Values) );

    // Now primitive statistics
    Out.write( (char*)&mPrimitiveStats.lastFrameSample, sizeof(cgInt64) );
    Out.write( (char*)&mPrimitiveStats.snapshotTime, sizeof(cgInt64) );
    Out.write( (char*)&mPrimitiveStats.overall, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&mPrimitiveStats.recent, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&mPrimitiveStats.snapshot, sizeof(cgProfilerStatistics::Values) );

    // Write hierarchical recorded stats.
    cgUInt16 nCount = (cgUInt16)mRootProcessStats.size();
    Out.write( (char*)&nCount, sizeof(cgUInt16) );
    cgProfilerStatistics::Map::iterator itStat;
    for ( itStat = mRootProcessStats.begin(); itStat != mRootProcessStats.end(); ++itStat )
        broadcastStatTree( itStat->first, itStat->second, Out );

    // Send the data.
    mBroadcastServer->sendToAll( cgBroadcastCommand::Profiler::FrameData, 
                                   (void*)Out.str().c_str(), Out.str().length() );
}

//-----------------------------------------------------------------------------
//  Name : broadcastStatTree() (Protected, Recursive)
/// <summary>
/// Output the specified set of measure statistics (including any children)
/// to the provided buffer ready for broadcast.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::broadcastStatTree( const cgString & strName, const cgProfilerStatistics & Stats, std::stringstream & Out )
{
    STRING_CONVERT;

    // First output the name (ANSI).
    cgUInt16 nLength = (cgUInt16)strName.size();
    Out.write( (char*)&nLength, sizeof(cgUInt16) );
    if ( nLength > 0 )
        Out.write( stringConvertT2CA(strName.c_str()), nLength );

    // Then output the statistics.
    Out.write( (char*)&Stats.statisticId, sizeof(cgInt32) );
    Out.write( (char*)&Stats.parentId, sizeof(cgInt32) );
    Out.write( (char*)&Stats.lastFrameSample, sizeof(cgInt64) );
    Out.write( (char*)&Stats.snapshotTime, sizeof(cgInt64) );
    Out.write( (char*)&Stats.overall, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&Stats.recent, sizeof(cgProfilerStatistics::Values) );
    Out.write( (char*)&Stats.snapshot, sizeof(cgProfilerStatistics::Values) );

    // Process children.
    cgUInt16 nCount = (cgUInt16)Stats.children.size();
    Out.write( (char*)&nCount, sizeof(cgUInt16) );
    cgProfilerStatistics::Map::const_iterator itStat;
    for ( itStat = Stats.children.begin(); itStat != Stats.children.end(); ++itStat )
        broadcastStatTree( itStat->first, itStat->second, Out );
}

//-----------------------------------------------------------------------------
//  Name : primitivesDrawn()
/// <summary>
/// Call to inform the profile that the specified number of primimtives have
/// recently been drawn.
/// </summary>
//-----------------------------------------------------------------------------
void cgProfiler::primitivesDrawn( cgUInt32 nPrimitiveCount )
{
    if ( mFrameBegun )
        mPrimitiveStats.sampleData += nPrimitiveCount;
}