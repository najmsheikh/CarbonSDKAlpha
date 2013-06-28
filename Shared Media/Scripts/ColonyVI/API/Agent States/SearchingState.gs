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
    private int         mSearchPhase;
    private float       mTimeSpentSearching;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : SearchingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	SearchingState( NPCAgent @ agent )
    {
        // Call base class constructor
        super( agent );
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
        // If navigation is not yet enabled for the agent, it must now be enabled.
        if ( @mAgent.controller == null )
            mAgent.enableNavigation( true );

        // If there is still no controller, we cannot search.
        if ( @mAgent.controller == null )
            return false;
        
        // Navigate to the target's last known position.
        mSearchPhase = 0;
        mTimeSpentSearching = 0;
        if ( !mAgent.controller.navigateTo( mAgent.targetLastKnownPos ) )
        {
            // Couldn't find position, switch straght to the 
            // second phase (active searching).
            mSearchPhase = 1;
        
        } // End if failed

        // Switch to correct walking animation.
        mAgent.selectAnimation( mAgent.walkAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ), mAgent.walkAnimationSpeed );

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
            mAgent.requestedOrientation = mAgent.controller.getSuggestedHeading();

            // Is our target acquired again?
            if ( mAgent.targetAcquired )
            {
                float distanceToTarget = vec3Length( mAgent.getSceneNode().getPosition() - mAgent.targetLastKnownPos );

                // If target is outside of our ideal firing range, start 
                // advancing on them. Otherwise switch back to regular combat.
                if ( distanceToTarget > mAgent.idealFiringRange )
                {
                    mAgent.switchState( AgentStateId::Repositioning );
                    return;
                
                } // End if out of range
                else
                {
                    mAgent.switchState( AgentStateId::Idle );
                    return;

                } // End if in range
            
            } // End if acquired
            else
            {
                // Wait until we've arrived at our destination, or 20 seconds has elapsed
                // (whichever comes first).
                if ( mTimeSpentSearching >= 20.0f || mAgent.controller.getNavigationState() == NavigationTargetState::Arrived )
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
            mAgent.resetTarget();
            mAgent.switchState( AgentStateId::Idle );

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

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Searching;
    }
}