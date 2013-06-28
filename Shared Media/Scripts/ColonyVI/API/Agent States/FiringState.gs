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

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FiringState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FiringState( NPCAgent @ agent )
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
        Weapon @ weapon = mAgent.getCurrentWeapon();

        // Is the weapon in a ready state?
        if ( weapon.getState() == WeaponState::Ready )
        {
            // Set aim at target.
            ObjectNode @ agentNode = mAgent.getSceneNode();
            Vector3 from = agentNode.getPosition() + (agentNode.getYAxis() * mAgent.getHeight() * 0.66f) + (agentNode.getZAxis() * 0.8f);
            Vector3 to = mAgent.targetLastKnownPos + agentNode.getYAxis() * (mAgent.targetHeight * 0.66f);
            weapon.setAimPoints( from, to );

            // Start firing with the current weapon if possible. If not
            // return back to the previous state.
            if ( weapon.beginFiring() == WeaponFiringResult::Success )
            {
                // Switch to the firing animation.
                if ( weapon.getFiringMode() == WeaponFiringMode::FullyAutomatic || weapon.getFiringMode() == WeaponFiringMode::Burst )
                {
                    // Looped fire cycle.
                    if ( mAgent.firingAnimationName != "" )
                        mAgent.selectAnimation( mAgent.firingAnimationName, AnimationPlaybackMode::Loop, 0.0f );
                
                } // End if Automatic | Burst
                else if ( weapon.getFiringMode() == WeaponFiringMode::FullBurst )
                {
                    // Single animation cycle.
                    if ( mAgent.firingAnimationName != "" )
                        mAgent.selectAnimation( mAgent.firingAnimationName, AnimationPlaybackMode::PlayOnce, 0.0f );
                
                } // End if Full Burst
                else
                {
                    // Single shot case. Stop and restart the animation to ensure 
                    // rapid fire cases are handled.
                    //actor.stopAnimationTrack( "Primary", true );
                    //mAnimationTrackIndex = actor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Fire", AnimationPlaybackMode::PlayOnce );

                } // End if single shot
                
                // Switch success
                return true;
            
            } // End if success

        } // End if weapon ready
        else if ( weapon.getState() == WeaponState::Empty )
        {            
            // Weapon is out of ammo, switch to reloading state,
            // but make sure that it returns to any parent that
            // has been assigned to us when its done.
            ReloadingState @ state = cast<ReloadingState>(mAgent.createStateById( AgentStateId::Reloading ));
            state.init( mParentState );
            mAgent.switchState( state );
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
        bool agentCancel = !mAgent.targetAcquired || !mAgent.willingToFire;
        bool canCancel   = (weapon.getFiringMode() == WeaponFiringMode::Burst || weapon.getFiringMode() == WeaponFiringMode::FullyAutomatic);
        if ( weapon.getState() != WeaponState::Firing || (canCancel && agentCancel) )
        {
            // Switch back to specified state, or idle if not supplied.
            if ( @mParentState != null )
                mAgent.switchState( mParentState );
            else
                mAgent.switchState( AgentStateId::Idle );

        } // End if keep firing
        else
        {
            // Keep aiming at target while firing.
            // ToDo: Aim at correct point of destination
            ObjectNode @ agentNode = mAgent.getSceneNode();
            Vector3 from = agentNode.getPosition() + (agentNode.getYAxis() * mAgent.getHeight() * 0.66f) + (agentNode.getZAxis() * 0.8f);
            Vector3 to = mAgent.targetLastKnownPos + agentNode.getYAxis() * (mAgent.targetHeight * 0.66f);
            weapon.setAimPoints( from, to );

            // Turn the NPC too.
            mAgent.turnTowardPosition( mAgent.targetLastKnownPos );

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

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Firing;
    }
}