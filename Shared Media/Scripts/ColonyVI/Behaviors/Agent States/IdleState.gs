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
// Name : IdleState.gsh                                                      //
//                                                                           //
// Desc : Basic idling state for NPC agents.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"
#include_once "../NPCAgent.gsh"
#include_once "FiringState.gs"
#include_once "AdvancingState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : IdleState (Class)
// Desc : Basic idling state for NPC agents.
//-----------------------------------------------------------------------------
shared class IdleState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private NPCAgent @ mNPC;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : IdleState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	IdleState( Agent @ agent )
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
        // Stop navigating where we are.
        if ( @mNPC.controller != null )
            mNPC.controller.navigateTo( mNPC.getSceneNode().getPosition() );

        // Switch to idle animation
        mNPC.selectAnimation( mNPC.idleAnimationName, AnimationPlaybackMode::Loop, 0.0f );

        // Success!
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
        // Navigation behaviors?
        if ( @mNPC.controller != null )
        {
            // We have a target now?
            if ( mNPC.targetAcquired )
            {
                float distanceToTarget = vec3Length( mNPC.getSceneNode().getPosition() - mNPC.targetLastKnownPos );

                // If target is outside of around half our allowed firing range, or we have less than
                // 75% visibility on them, start advancing.
                if ( distanceToTarget > (mNPC.maxFiringRange * 0.5f) || mNPC.targetVisibilityScore < 0.75f )
                {
                    mNPC.switchState( AdvancingState( mAgent ) );
                    return;
                
                } // End if out of range
                else
                {
                    // Turn toward them.
                    mNPC.turnTowardPosition( mNPC.targetLastKnownPos );
                
                } // End if turn
            
            } // End if acquired
            else
            {
                // If we saw or heard them just a little while ago, search!
                if ( mNPC.timeSinceTargetLost <= 10.0f || mNPC.timeSinceTargetHeard <= 10.0f )
                {
                    mNPC.switchState( SearchingState( mAgent ) );
                    return;
                
                } // End if start searching
            
            } // End if !acquired
        
        } // End if can navigate
        else
        {
            // Turn on target position if acquired.
            if ( mNPC.targetAcquired )
                mNPC.turnTowardPosition( mNPC.targetLastKnownPos );

        } // End if cannot navigate

        // NPC has a weapon?
        Weapon @ weapon = mNPC.getCurrentWeapon();
        if ( @weapon != null )
        {
            // If our target is acquired, the NPC is willing to fire, and the weapon
            // has ammo remaining, attempt to switch to the firing state.
            if ( mNPC.targetAcquired && mNPC.willingToFire && weapon.getAvailableRounds() > 0 && weapon.getState() != WeaponState::Cooldown )
                mAgent.switchState( FiringState( mAgent, null ) );

        } // End if has weapon
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Idle";
    }
}