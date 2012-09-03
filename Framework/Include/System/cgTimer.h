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
// Name : cgTimer.h                                                          //
//                                                                           //
// Desc : This class handles all timing functionality. This includes         //
//        counting the number of frames per second, to scaling vectors and   //
//        values relative to the time that has passed since the previous     //
//        frame.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTIMER_H_ )
#define _CGE_CGTIMER_H_

//-----------------------------------------------------------------------------
// cgTimer Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTimer (Class)
/// <summary>
/// Timer class, queries performance hardware if available, and 
/// calculates all the various values required for frame rate based
/// vector / value scaling.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTimer : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgTimer, "Timer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
	         cgTimer();
	virtual ~cgTimer();

    //------------------------------------------------------------
    // Public Static Functions
    //------------------------------------------------------------
    static cgTimer* getInstance                     ( );
    static void     createSingleton                 ( );
    static void     destroySingleton                ( );

	//------------------------------------------------------------
	// Public Methods
	//------------------------------------------------------------
	void	        tick                            ( );
    void	        tick                            ( cgFloat lockFPS );
    void            enableSmoothing                 ( bool enabled );
    cgDouble        getTime                         ( bool queryTimer ) const;
    cgInt64         getPerfomanceCounter            ( bool queryTimer ) const;
    cgUInt32        getFrameRate                    ( ) const;
    cgDouble        measurePeriod                   ( cgInt64 counterBegin, cgInt64 counterEnd ) const;
    void            setSimulationSpeed              ( cgFloat speed ) { mSimulationSpeed = speed; }
    void            setRateLimit                    ( cgFloat rateLimit );
    void            incrementFrameCounter           ( ) { ++mFrameCounter; }

    // Inlines for rapid access to internal data
    inline cgUInt32 getFrameCounter                 ( ) const { return mFrameCounter; }
    inline cgReal   getTimeElapsed                  ( ) const { return mSimulationTimeElapsed; }
    inline cgInt64  getPerformanceCounterOrigin     ( ) const { return mStartTime; }
    inline cgInt64  getPerfomanceCounterFrequency   ( ) const { return mPerfFreq; }
    inline cgDouble getTime                         ( ) const { return mCurrentTimeSeconds; }

private:
    //------------------------------------------------------------
	// Private Variables
	//------------------------------------------------------------
    bool        mUsePerfHardware;           // Has Performance Counter
    bool        mUseSmoothing;              // Is time-step smoothing enabled?
	cgDouble    mTimeScale;                 // Amount to scale counter
	cgFloat     mTimeElapsed;               // Final REPORTED elapsed time since previous frame (may be smoothed)
    cgFloat     mPrevTimeElapsed;           // Previous REPORTED elapsed time (may be smoothed).
    cgFloat     mSimulationTimeElapsed;     // Pre-multiplied mTimeElapsed * mSimulationSpeed.
    cgDouble    mTimeDebt;                  // How much have the summed elapsed time's drifted from the real-world clock?
    cgInt64     mCurrentTime;               // Current Performance Counter
    cgDouble    mCurrentTimeSeconds;        // Pre-multiplied mCurrentTime * mTimeScale;
    cgInt64     mStartTime;                 // Time at which timer was initialized.
    cgInt64     mLastTime;                  // Performance Counter last frame
	cgInt64     mPerfFreq;                  // Performance Frequency

    cgFloat     mFrameTime[11];
    cgFloat     mSimulationSpeed;
    cgFloat     mRateLimit;

    cgUInt32    mFrameRate;                 // Stores current framerate
	cgUInt32    mFPSFrameCount;             // Elapsed frames in any given second
	cgFloat     mFPSTimeElapsed;            // How much time has passed during FPS sample

    cgUInt32    mFrameCounter;              // Simple Application counter
	
	//------------------------------------------------------------
	// Private Static Variables
	//------------------------------------------------------------
    static cgTimer * mSingleton;
};

#endif // !_CGE_CGTIMER_H_