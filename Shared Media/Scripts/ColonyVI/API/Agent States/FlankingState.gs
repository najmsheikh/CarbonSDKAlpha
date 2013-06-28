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
// Name : FlankingState.gsh                                                  //
//                                                                           //
// Desc : Basic target flank attempt state for NPC agents.                   //
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
// Name : FlankingState (Class)
// Desc : Basic target flank attempt state for NPC agents.
//-----------------------------------------------------------------------------
shared class FlankingState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FlankingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FlankingState( NPCAgent @ agent )
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
        mAgent.log( "Entered flanking state." );

        // Navigate behind the target's last known position.
        if ( !mAgent.controller.navigateToAtRange( mAgent.targetLastKnownPos, mAgent.minFiringRange, CGE_PI ) ) // BEHIND target from current position.
        {
            mAgent.log( "Unable to find an appropriate navigation point." );

            // Couldn't find position, switch back to idle state.
            mAgent.switchState( AgentStateId::Idle );
            return false;
        
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
        // Moving to last known position.
        mAgent.requestedOrientation = mAgent.controller.getSuggestedHeading();

        // Done?
        if ( mAgent.controller.getNavigationState() == NavigationTargetState::Arrived )
        {
            mAgent.log( "Flank attempt complete." );
            mAgent.switchState( AgentStateId::Idle );
            return;

        } // End if arrived
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Flanking";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Flanking;
    }
}