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
// Name : SearchingState.gsh                                                 //
//                                                                           //
// Desc : Basic target search state for NPC agents.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"
#include_once "../NPCAgent.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : SearchingState (Class)
// Desc : Basic target search state for NPC agents.
//-----------------------------------------------------------------------------
shared class SearchingState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private NPCAgent@   mNPC;
    private int         mSearchPhase;
    private float       mTimeSpentSearching;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : SearchingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	SearchingState( Agent @ agent )
    {
        // Call base class constructor
        super( agent );

        // Cast to correct type to save time later
        @mNPC = cast<NPCAgent>(agent);
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
        // Navigate to the target's last known position.
        mSearchPhase = 0;
        mTimeSpentSearching = 0;
        if ( !mNPC.controller.navigateTo( mNPC.targetLastKnownPos ) )
        {
            // Couldn't find position, switch straght to the 
            // second phase (active searching).
            mSearchPhase = 1;
        
        } // End if failed

        // Switch to correct walking animation.
        mNPC.selectAnimation( mNPC.walkAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ) );

        // Switch success
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
        mTimeSpentSearching += elapsedTime;

        // Which phase are we on?
        if ( mSearchPhase == 0 )
        {
            // Moving to last known position.
            mNPC.requestedOrientation = mNPC.controller.getSuggestedHeading();

            // Is our target acquired again?
            if ( mNPC.targetAcquired )
            {
                float distanceToTarget = vec3Length( mNPC.getSceneNode().getPosition() - mNPC.targetLastKnownPos );

                // If target is outside of around half our allowed firing range, start 
                // advancing on them. Otherwise switch back to regular combat.
                if ( distanceToTarget > (mNPC.maxFiringRange * 0.5f) )
                {
                    mNPC.switchState( AdvancingState( mAgent ) );
                    return;
                
                } // End if out of range
                else
                {
                    mNPC.switchState( IdleState( mAgent ) );
                    return;

                } // End if in range
            
            } // End if acquired
            else
            {
                // Wait until we've arrived at our destination, or 20 seconds has elapsed
                // (whichever comes first).
                if ( mTimeSpentSearching >= 20.0f || mNPC.controller.getNavigationState() == NavigationTargetState::Arrived )
                {
                    // Switch to second phase (active searching).
                    mSearchPhase = 1;
                    return;

                } // End if arrived

            } // End if still not acquired

        } // End if first phase
        else
        {
            // ToDo: active searching. For now, just give up.
            mNPC.timeSinceTargetLost = 999999.0f;
            mNPC.timeSinceTargetHeard = 999999.0f;
            mNPC.switchState( IdleState( mAgent ) );

        } // End if second phase
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Searching";
    }
}