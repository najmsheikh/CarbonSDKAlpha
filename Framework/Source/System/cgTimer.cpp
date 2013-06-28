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
// Name : cgTimer.cpp                                                        //
//                                                                           //
// Desc : This class handles all timing functionality. This includes         //
//        counting the number of frames per second, to scaling vectors and   //
//        values relative to the time that has passed since the previous     //
//        frame.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTimer Module Includes
//-----------------------------------------------------------------------------
#include <System/cgTimer.h>
#include <Math/cgMathUtility.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>  // Warning: Portability
#include <Mmsystem.h> // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgTimer * cgTimer::mSingleton = CG_NULL;

//-----------------------------------------------------------------------------
// Local Function Definitions
//-----------------------------------------------------------------------------
namespace
{
    int FrameTimeSort( const void * pKey1, const void * pKey2 )
    {
        if ( *((cgDouble*)pKey1) == *((cgDouble*)pKey2) )
            return 0;
        return (*((cgDouble*)pKey1) < *((cgDouble*)pKey2)) ? -1 : 1;
    }
};

//-----------------------------------------------------------------------------
//  Name : cgTimer () (Constructor)
/// <summary>
/// cgTimer Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTimer::cgTimer()
{
	// Query performance hardware and setup time scaling values
	if (QueryPerformanceFrequency((LARGE_INTEGER *)&mPerfFreq)) 
    { 
		mUsePerfHardware		= TRUE;
		QueryPerformanceCounter((LARGE_INTEGER *) &mLastTime); 
		mTimeScale			= 1.0 / mPerfFreq;
	} 
    else 
    { 
		// no performance counter, read in using timeGetTime 
		mUsePerfHardware		= FALSE;
		mLastTime			= timeGetTime(); 
		mTimeScale			= 0.001;
	
    } // End If No Hardware

	// Clear any needed values
    mUseSmoothing          = false;
    mStartTime             = mLastTime;
    mCurrentTime           = mLastTime;
    mCurrentTimeSeconds    = (cgDouble)(mCurrentTime - mStartTime) * mTimeScale;
    mTimeElapsed           = 0.0;
    mPrevTimeElapsed       = 0.0;
    mSimulationSpeed       = 1.0;
    mSimulationTimeElapsed = 0.0;
    mTimeDebt              = 0.0;
    mRateLimit             = 0.0;
    mFrameRate             = 0;
    mFPSFrameCount         = 0;
    mFPSTimeElapsed        = 0.0;
    mFrameCounter         = 0;
    memset( mFrameTime, 0, sizeof(mFrameTime) );
}

//-----------------------------------------------------------------------------
//  Name : cgTimer () (Destructor)
/// <summary>
/// cgTimer Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgTimer::~cgTimer()
{
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Retrieve pointer for singleton timer object.
/// </summary>
//-----------------------------------------------------------------------------
cgTimer * cgTimer::getInstance()
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
void cgTimer::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgTimer();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgTimer::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton != CG_NULL )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : tick () 
/// <summary>
/// Function which signals that frame has advanced
/// Note : You can specify a number of frames per second to lock the frame rate
/// to. This will simply soak up the remaining time to hit that target.
/// </summary>
//-----------------------------------------------------------------------------
void cgTimer::tick( )
{
    tick( 0.0f );
}
void cgTimer::tick( cgDouble fLockFPS )
{
    cgDouble fTimeElapsed;

    // Is performance hardware available?
	if ( mUsePerfHardware ) 
    {
        // Query high-resolution performance hardware
		QueryPerformanceCounter((LARGE_INTEGER *)&mCurrentTime);
	} 
    else 
    {
        // Fall back to less accurate timer
		mCurrentTime = timeGetTime();

	} // End If no hardware available

	// Calculate elapsed time in seconds
	fTimeElapsed = (cgDouble)(mCurrentTime - mLastTime) * mTimeScale;

    // Smoothly ramp up frame rate to prevent jittering
    //if ( fLockFPS == 0.0 && mTimeElapsed > 0 ) fLockFPS = (1.0 / mTimeElapsed) + 20.0;

    // Any permanent rate limit applied?
    if ( fLockFPS <= 0.0 )
        fLockFPS = mRateLimit;
    
    // Should we lock the frame rate ?
    if ( fLockFPS > 0.0 )
    {
        while ( fTimeElapsed < (1.0 / fLockFPS))
        {
            // Is performance hardware available?
	        if ( mUsePerfHardware ) 
            {
                // Query high-resolution performance hardware
		        QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentTime);
	        } 
            else 
            {
                // Fall back to less accurate timer
		        mCurrentTime = timeGetTime();

	        } // End If no hardware available

	        // Calculate elapsed time in seconds
	        fTimeElapsed = (cgDouble)(mCurrentTime - mLastTime) * mTimeScale;

        } // End While

    } // End If

	// Save current frame time
	mLastTime = mCurrentTime;

    // Use the exact frame time in all samples for at least the first 11 frames.
    // Otherwise, start wrapping the elapsed time sample buffer.
    if ( mFrameCounter <= 11 )
    {
        for ( cgInt i = 0; i < 11; ++i )
            mFrameTime[i] = fTimeElapsed;
        mPrevTimeElapsed = fTimeElapsed;
    
    } // End if <= 11
    else
    {
        //for ( cgInt i = 10; i > 0; --i )
            //mFrameTime[i] = mFrameTime[i-1];
        memmove( &mFrameTime[1], mFrameTime, 10 * sizeof(cgDouble) );
        mFrameTime[ 0 ] = fTimeElapsed;
    
    } // End if > 11

	// Calculate Frame Rate
	mFPSFrameCount++;
	mFPSTimeElapsed += fTimeElapsed;
	if ( mFPSTimeElapsed > 1.0) 
    {
		mFrameRate		= mFPSFrameCount;
		mFPSFrameCount  = 0;
		mFPSTimeElapsed	= 0.0;
	
    } // End If Second Elapsed

    // If smoothing is enabled, calculate the mean of all 11 samples in the 
    // frame time buffer (minus the two highest and two lowest outliers). 
    // Otherwise, just use the measured time as-is.
    if ( mUseSmoothing )
    {
        // Compute the mean of the frame time, discarding the two highest
        // and two lowest outliers (we use a quicksort to achieve this).
        mTimeElapsed = 0.0f;
        cgDouble Samples[11];
        memcpy( Samples, mFrameTime, sizeof(mFrameTime) );
        qsort( Samples, 11, sizeof(cgDouble), FrameTimeSort );
        for ( cgInt i = 2; i < 9; ++i )
            mTimeElapsed += Samples[i];
        mTimeElapsed /= 7.0;

        // Smooth time step from previous frame.
        mTimeElapsed = (0.2 * mTimeElapsed) + (0.8 * mPrevTimeElapsed);

        // Keep track of the time 'debt'; the difference between the sum of all
        // smoothed time deltas (in total, over time) and the real world clock.
        mTimeDebt += mTimeElapsed - fTimeElapsed;
    
    } // End if smoothing
    else
    {
        mTimeElapsed = fTimeElapsed;
    
    } // End if !smoothing

    // Record the final (reported) elapsed time for the previous frame.
    mPrevTimeElapsed = mTimeElapsed;
    
    // Store current time and final simulation elapsed time in seconds
    // to save the multiply in each call to the various accessors.
    mCurrentTimeSeconds    = (cgDouble)(mCurrentTime - mStartTime) * mTimeScale;
    mSimulationTimeElapsed = mTimeElapsed * mSimulationSpeed;
    mFrameCounter++;
}

//---------------------------------------------------------------------------------
// Name : getTime () 
// Desc : Returns the current floating point time in seconds. This can either be 
//        the time at the last call to tick, or can optionally be re-sampled 
//        precisely.
//---------------------------------------------------------------------------------
cgDouble cgTimer::getTime( bool bQueryPerfCounter ) const
{
    if ( !bQueryPerfCounter )
    {
        return mCurrentTimeSeconds;
    
    } // End if last tick time
    else
    {
        cgInt64 CurrentTime;

        // Is performance hardware available?
        if ( mUsePerfHardware ) 
        {
            // Query high-resolution performance hardware
	        QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);
        } 
        else 
        {
            // Fall back to less accurate timer
	        CurrentTime = timeGetTime();

        } // End If no hardware available

        // Return result in seconds.
        return (cgDouble)(CurrentTime - mStartTime) * mTimeScale;

    } // End if re-query.
}

//---------------------------------------------------------------------------------
// Name : getPerfomanceCounter () 
// Desc : Returns the value read from the performance counter hardware.
//        This can either be the value at the last call to tick, or can optionally 
//        be re-sampled precisely.
//---------------------------------------------------------------------------------
cgInt64 cgTimer::getPerfomanceCounter( bool bQuery ) const
{
    if ( bQuery == false )
    {
        return mCurrentTime;
    
    } // End if last tick time
    else
    {
        cgInt64 CurrentTime;

        // Is performance hardware available?
        if ( mUsePerfHardware ) 
        {
            // Query high-resolution performance hardware
	        QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);
        } 
        else 
        {
            // Fall back to less accurate timer
	        CurrentTime = timeGetTime();

        } // End If no hardware available

        // Return result directly.
        return CurrentTime;

    } // End if re-query.
}

//---------------------------------------------------------------------------------
// Name : measurePeriod () 
// Desc : Convert the difference between the two performance counter values into
//        a measured time in seconds.
//---------------------------------------------------------------------------------
cgDouble cgTimer::measurePeriod( cgInt64 nCounter1, cgInt64 nCounter2 ) const
{
    return (cgDouble)(nCounter2 - nCounter1) * mTimeScale;
}

//-----------------------------------------------------------------------------
//  Name : getFrameRate () 
/// <summary>
/// Returns the frame rate, sampled over the last second or so.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTimer::getFrameRate( ) const
{
    return mFrameRate;
}

//-----------------------------------------------------------------------------
//  Name : SetFrameRate () 
/// <summary>
/// Set the permanent frame rate limit to use (if any) when none is supplied
/// directly to the 'tick()' method. Set to a value <= 0 to disable.
/// </summary>
//-----------------------------------------------------------------------------
void cgTimer::setRateLimit( cgDouble fRateLimit )
{
    mRateLimit = fRateLimit;
}

//-----------------------------------------------------------------------------
//  Name : enableSmoothing () 
/// <summary>
/// Enable elapsed time smoothing.
/// </summary>
//-----------------------------------------------------------------------------
void cgTimer::enableSmoothing( bool bEnabled )
{
    mUseSmoothing = bEnabled;
}