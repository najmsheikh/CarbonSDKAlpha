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
// Name : MechLeapDownState.gsh                                              //
//                                                                           //
// Desc : State that allows the enemy mech to jump down from the balcony.    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../../API/AgentState.gsh"

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
const float InitialLeapPause = 1.0f;
const float LeapAccelerationY = 35.0f;
const float LeapAccelerationZ = 7.5f;
const float LeapAccelerationTime = 0.5f;
const float LeapAnimationTime    = 1.2f;
const float AccelerationY = -20;
const float AnimationSpeed = 0.42f;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : MechLeapDownState (Class)
// Desc : Agent state that allows the enemy mech to jump down from the balcony
//-----------------------------------------------------------------------------
shared class MechLeapDownState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@ mNodeCOG;
    private float       mStateActiveTime;
    private int         mLeapState;
    private Vector3     mInitialOffset;
    private Vector3     mPosition;
    private Vector3     mVelocity;

    private int         mGroundImpactSound;
	
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : MechLeapDownState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	MechLeapDownState( NPCAgent @ agent )
    {
        // Call base class constructor
        super( agent );

        // Initialize variables
        mStateActiveTime    = 0.0f;
        mLeapState          = 0;
        mGroundImpactSound  = -1;

        // Cache references.
        @mNodeCOG = mAgent.actor.findChild( "COG", false );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : begin ()
	// Desc : State has been selected and is about to begin. Return false
    //        to cancel state switch, and call 'Agent::switchState()' to select 
    //        the actual state you want to switch to (unless you want the 
    //        current state to remain).
	//-------------------------------------------------------------------------
	bool begin( AgentState @ from )
	{
		// Array containing names of a random selection of sounds for the mech landing on a metal surface.	
		array<String> groundImpactSoundFiles( 5 ); 
        groundImpactSoundFiles[0] = "Sounds/0027_dungeon_foley_hit_metal_01.ogg";
        groundImpactSoundFiles[1] = "Sounds/0042_impact_metal_4.ogg";
        groundImpactSoundFiles[2] = "Sounds/0027_dungeon_foley_drop_metal_01.ogg";
        groundImpactSoundFiles[3] = "Sounds/0027_dungeon_foley_drop_metal_02.ogg";
        groundImpactSoundFiles[4] = "Sounds/0027_dungeon_foley_drop_metal_03.ogg";

		// Load the sound effect we will play when hitting the ground
        int soundIndex = randomInt( 0, groundImpactSoundFiles.length()-1 );
        AudioManager @ audioManager = getAudioManager();
        mGroundImpactSound = audioManager.loadSound( groundImpactSoundFiles[ soundIndex ], true );

        // Switch to idle animation initially.
        mAgent.selectAnimation( mAgent.idleAnimationName, AnimationPlaybackMode::Loop, 0.0f );
		
        // Switch success.
        return true;
	}

    //-------------------------------------------------------------------------
	// Name : end ()
	// Desc : State has been deselected and is about to end.
	//-------------------------------------------------------------------------
    void end( AgentState @ to )
    {
    }

    //-------------------------------------------------------------------------
	// Name : update ()
	// Desc : Allow the state to perform its update processing.
	//-------------------------------------------------------------------------
    void update( float elapsedTime )
    {
        if ( mLeapState == 0 )
        {
            // Play leap animation after desired time.
            if ( mStateActiveTime >= InitialLeapPause )
            {
                // Record the initial position of the mech's center of gravity.
                mPosition = mAgent.actor.getPosition();
                mInitialOffset = mPosition - mNodeCOG.getPosition();
                mVelocity = Vector3(0,0,0);

                // Play leap forward anim and switch to leaping.
                mAgent.selectAnimation( "Dash Forward", AnimationPlaybackMode::PlayOnce, 0.0f, AnimationSpeed );
                mLeapState = 1;

            } // End if > pause time
        
        } // End if waiting to start
        else
        {
            // What state are we in?
            if ( mLeapState == 1 )
            {
                // Launch for the appropriate time.
                if ( (mStateActiveTime - InitialLeapPause) < LeapAccelerationTime )
                {
                    mVelocity += mAgent.actor.getYAxis() * LeapAccelerationY * elapsedTime;
                    mVelocity += mAgent.actor.getZAxis() * LeapAccelerationZ * elapsedTime;

                } // End if in accel time
                else if ( (mStateActiveTime - InitialLeapPause) < LeapAnimationTime )
                {
                    // Wait until animation time elapses
                
                } // End if in anim time
                else
                {
                    // We're free-falling
                    mLeapState = 2;
                
                } // End if out of time

            } // End if launching
            else if ( mLeapState == 2 )
            {
                // Wait for the animation to finish after freefall and then switch to idle.
                if ( !mAgent.actor.isAnimationTrackPlaying( "Primary", false ) && mPosition.y <= -1.0f )
                {
                    // Stop the animation track immediately so we get no blending, 
                    // and then switch to the idle pose.
                    mAgent.actor.stopAnimationTrack( "Primary", true  );
                    mAgent.selectAnimation( mAgent.idleAnimationName, AnimationPlaybackMode::Loop, 0.0f );

                    // Reposition the actor to the correct location.
                    mAgent.actor.setPosition( mPosition );

                    // Enable NPC navigation now.
                    mAgent.enableNavigation( true );

                    // Switch to idle state.
					mAgent.switchState( AgentStateId::Idle );
                    return;
					
                } // End if finished leap
            
            } // End if wait for anim
            
            // Adjust velocity due to affect of gravity.
            mVelocity.y += AccelerationY * elapsedTime;

            // Reposition the actor.
            mPosition += mVelocity * elapsedTime;
            if ( mPosition.y <= -1.0f )
            {
                mVelocity = Vector3(0,0,0);
                mPosition.y = -1.0f;

                // Play the landing sound (once)
                if ( mGroundImpactSound >= 0 )
                {
                    AudioManager @ audioManager = getAudioManager();
                    audioManager.playSound( mGroundImpactSound, true, false, 1.0f, mPosition, null );
                    mGroundImpactSound = -1;
                
                } // End if play sound
            
            } // End if hit the ground

            // Counteract movement in the animation and update actor position.
            Vector3 currentPos = mAgent.actor.getPosition();
            Vector3 newOffset = currentPos - mNodeCOG.getPosition();
            mAgent.actor.setPosition( mPosition.x - (mInitialOffset.x - newOffset.x), mPosition.y, mPosition.z - (mInitialOffset.z - newOffset.z) );

        } // End if leaping

        // Progress state.
        mStateActiveTime += elapsedTime;
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Leap Down Sequence";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::LeapDown;
    }
}