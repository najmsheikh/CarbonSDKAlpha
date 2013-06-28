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
// Name : ReloadingState.gsh                                                 //
//                                                                           //
// Desc : Basic weapon reloading state for NPC agents.                       //
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
// Name : ReloadingState (Class)
// Desc : Basic weapon reloading state for NPC agents.
//-----------------------------------------------------------------------------
shared class ReloadingState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ReloadingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ReloadingState( NPCAgent @ agent )
    {
        // Call base class constructor
        super( agent );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : init ()
	// Desc : Custom initialization method for this state.
	//-------------------------------------------------------------------------
    void init( AgentState @ returnState )
    {
        @mParentState = returnState;
    }

    //-------------------------------------------------------------------------
	// Name : begin ()
	// Desc : State has been selected and is about to begin. Return false
    //        to cancel state switch, and call 'Agent::switchState()' to select 
    //        the actual state you want to switch to (unless you want the 
    //        current state to remain).
	//-------------------------------------------------------------------------
	bool begin( AgentState @ from )
	{
        Weapon @ weapon = mAgent.getCurrentWeapon();

        // Request that current weapon magazine is restocked from total supply.
        if ( weapon.reload() == WeaponReloadResult::Success )
        {
            // Stop navigating where we are.
            if ( @mAgent.controller != null )
                mAgent.controller.navigateTo( mAgent.getSceneNode().getPosition() );

            // Play reload animation
            mAgent.selectAnimation( mAgent.reloadAnimationName, AnimationPlaybackMode::PlayOnce, 0.0f );

            // Switch success.
            return true;

        } // End if success
        else
        {
            // Don't switch
            return false;

        } // End if failed
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
        // Switch back to idle when reload is complete.
        if ( !mAgent.actor.isAnimationTrackPlaying( "Primary", false ) )
        {
            if ( @mParentState != null )
                mAgent.switchState( mParentState );
            else
                mAgent.switchState( AgentStateId::Idle );

        } // End if finished
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Reloading";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Reloading;
    }
}