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
// Name : FiringState.gsh                                                    //
//                                                                           //
// Desc : Basic weapon firing state for NPC agents.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"
#include_once "../NPCAgent.gsh"
#include_once "ReloadingState.gs"
#include_once "IdleState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : FiringState (Class)
// Desc : Basic weapon firing state for NPC agents.
//-----------------------------------------------------------------------------
shared class FiringState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private NPCAgent@   mNPC;
    private AgentState@ mReturnState;   // The (optional) state we wish to return to.

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FiringState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FiringState( Agent @ agent, AgentState @ returnState )
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

        // Is the weapon in a ready state?
        if ( weapon.getState() == WeaponState::Ready )
        {
            // Start firing with the current weapon if possible. If not
            // return back to the previous state.
            if ( weapon.beginFiring() == WeaponFiringResult::Success )
            {
                // Switch to the firing animation.
                if ( weapon.getFiringMode() != WeaponFiringMode::SingleShot )
                {
                    // Looped fire cycle.
                    if ( mNPC.firingAnimationName != "" )
                        mNPC.selectAnimation( mNPC.firingAnimationName, AnimationPlaybackMode::Loop, 0.0f );
                
                } // End if !single shot
                else
                {
                    // Single shot case. Stop and restart the animation to ensure 
                    // rapid fire cases are handled.
                    //actor.stopAnimationTrack( "Primary", true );
                    //mAnimationTrackIndex = actor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Fire", AnimationPlaybackMode::PlayOnce );

                } // End if single shot

                // Set aim at target.
                ObjectNode @ agentNode = mAgent.getSceneNode();
                Vector3 from = agentNode.getPosition() + agentNode.getYAxis() + (agentNode.getZAxis() * 0.8f);
                Vector3 to = mNPC.targetLastKnownPos + mNPC.playerNode.getYAxis() * (mNPC.playerAgent.getHeight() * 0.5f);
                weapon.setAimPoints( from, to );
                
                // Switch success
                return true;
            
            } // End if success

        } // End if weapon ready
        else if ( weapon.getState() == WeaponState::Empty )
        {            
            // Weapon is out of ammo, switch to reloading state.
            mAgent.switchState( ReloadingState( mAgent, mReturnState ) );
            return false;

        } // End if weapon empty

        // Not ready to switch
        return false;
	}

    //-------------------------------------------------------------------------
	// Name : end ()
	// Desc : State has been deselected and is about to end.
	//-------------------------------------------------------------------------
    void end( AgentState @ to )
    {
        Weapon @ weapon = mAgent.getCurrentWeapon();

        // Forcibly stop firing when we switch away for whatever reason.
        weapon.endFiring();
    }

    //-------------------------------------------------------------------------
	// Name : update ()
	// Desc : Allow the state to perform its update processing.
	//-------------------------------------------------------------------------
    void update( float elapsedTime )
    {
        Weapon @ weapon = mAgent.getCurrentWeapon();

        // Wait until the weapon stops firing of its own accord, our target 
        // is no longer acquired, or we are no longer willing to fire, and 
        // then switch back to required state.
        if ( weapon.getState() != WeaponState::Firing || !mNPC.targetAcquired || !mNPC.willingToFire )
        {
            // Switch back to specified state, or idle if not supplied.
            if ( @mReturnState != null )
                mAgent.switchState( mReturnState );
            else
                mAgent.switchState( IdleState( mAgent ) );

        } // End if keep firing
        else
        {
            // Keep aiming at target while firing.
            // ToDo: Aim at correct point of destination
            ObjectNode @ agentNode = mAgent.getSceneNode();
            Vector3 from = agentNode.getPosition() + agentNode.getYAxis() + (agentNode.getZAxis() * 0.8f);
            weapon.setAimPoints( from, mNPC.targetLastKnownPos + agentNode.getYAxis() );

        } // End if keep firing
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Firing";
    }
}