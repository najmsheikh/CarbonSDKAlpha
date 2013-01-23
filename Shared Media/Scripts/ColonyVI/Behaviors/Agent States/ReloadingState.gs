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
#include_once "../NPCAgent.gsh"
#include_once "IdleState.gs"

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
    private NPCAgent @ mNPC;
    private AgentState@ mReturnState;   // The (optional) state we wish to return to.

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : ReloadingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	ReloadingState( Agent @ agent, AgentState @ returnState )
    {
        // Call base class constructor
        super( agent );

        // Initialize variables
        @mReturnState = returnState;

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
        Weapon @ weapon = mAgent.getCurrentWeapon();

        // Request that current weapon magazine is restocked from total supply.
        if ( weapon.reload() == WeaponReloadResult::Success )
        {
            // Stop navigating where we are.
            if ( @mNPC.controller != null )
                mNPC.controller.navigateTo( mNPC.getSceneNode().getPosition() );

            // Play reload animation
            mNPC.selectAnimation( mNPC.reloadAnimationName, AnimationPlaybackMode::PlayOnce, 0.0f );

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
        if ( !mNPC.actor.isAnimationTrackPlaying( "Primary", false ) )
        {
            if ( @mReturnState != null )
                mAgent.switchState( mReturnState );
            else
                mAgent.switchState( IdleState( mAgent ) );

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
}