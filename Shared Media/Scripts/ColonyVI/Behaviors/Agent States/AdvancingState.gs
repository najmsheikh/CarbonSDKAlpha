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
// Name : AdvancingState.gsh                                                 //
//                                                                           //
// Desc : NPC agent is advancing toward current target.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../AgentState.gsh"
#include_once "../NPCAgent.gsh"
#include_once "SearchingState.gs"
#include_once "AdvanceFiringState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : AdvancingState (Class)
// Desc : NPC agent is advancing toward current target.
//-----------------------------------------------------------------------------
shared class AdvancingState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private NPCAgent @ mNPC;
    private Vector3    mLastNavigateTarget;
    private bool       mFiring;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AdvancingState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	AdvancingState( Agent @ agent )
    {
        // Call base class constructor
        super( agent );

        // Initialize variables
        mFiring = false;

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
        // If we're returning from a switch to the "advance and firing" state 
        // (but we were just temporarily suspended), we don't need to initialize
        // again.
        if ( mFiring )
        {
            // Continue navigation to our last target just in case we were stopped.
            mNPC.controller.navigateTo( mLastNavigateTarget );
            mFiring = false;
            return true;
        
        } // End if suspended

        // Start advancing toward the last known position of the target
        mLastNavigateTarget = mNPC.targetLastKnownPos;
        if ( !mNPC.controller.navigateTo( mNPC.targetLastKnownPos ) )
        {
            // If we were unable to set a target, switch to searching.
            mNPC.switchState( SearchingState( mAgent ) );
            return false;
        
        } // End if failed

        // Switch to correct walking animation.
        Vector3 velocity = mNPC.controller.getVelocity();
        velocity.y = 0.0f;
        vec3Normalize( velocity, velocity );
        if ( mNPC.strafeLeftAnimationName != "" && abs( vec3Dot( velocity, mNPC.getSceneNode().getZAxis() ) ) < 0.7f )
        {
            if ( vec3Dot( velocity, mNPC.getSceneNode().getXAxis() ) >= 0 )
                mNPC.selectAnimation( mNPC.strafeRightAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ) );
            else
                mNPC.selectAnimation( mNPC.strafeLeftAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ) );

        } // End if strafe
        else
            mNPC.selectAnimation( mNPC.walkAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ) );

        // Success!
        return true;
	}

    //-------------------------------------------------------------------------
	// Name : end ()
	// Desc : State has been deselected and is about to end.
	//-------------------------------------------------------------------------
    void end( AgentState @ to )
    {
        // Do nothing if we are just being suspended.
        if ( mFiring )
            return;
    }

    //-------------------------------------------------------------------------
	// Name : update ()
	// Desc : Allow the state to perform its update processing.
	//-------------------------------------------------------------------------
    void update( float elapsedTime )
    {
        Weapon @ weapon = mNPC.getCurrentWeapon();

        // We still have a target and can fire while moving?
        if ( mNPC.targetAcquired && mNPC.canFireWhileMoving )
        {
            // Target is still currently acquired. If it's inside around half our allowed 
            // firing range, or we have less than 75% visibility, stop advancing on them. 
            // Otherwise keep advancing.
            float distanceToTarget = vec3Length( mNPC.getSceneNode().getPosition() - mNPC.targetLastKnownPos );
            if ( distanceToTarget <= (mNPC.maxFiringRange * 0.5f) && mNPC.targetVisibilityScore >= 0.75f )
            {
                mNPC.switchState( IdleState( mAgent ) );
                return;
            
            } // End if close enough

            // If it's inside the total maximum firing range (and we're not already firing), fire!
            if ( !mFiring && distanceToTarget <= mNPC.maxFiringRange )
            {
                // NPC has a weapon?
                if ( @weapon != null )
                {
                    // If our target is acquired, the NPC is willing to fire, and the weapon
                    // has ammo remaining, attempt to switch to the firing state.
                    if ( mNPC.targetAcquired && mNPC.willingToFire && weapon.getAvailableRounds() > 0 && weapon.getState() != WeaponState::Cooldown )
                    {
                        mNPC.switchState( AdvanceFiringState( mAgent, this ) );

                        // We're suspended if the state switch was successful.
                        // Other state(s) may switch back to us when they are done.
                        if ( @mNPC.getState() != @this )
                        {
                            mFiring = true;
                            return;
                        
                        } // End if switched

                    } // End if fire! 

                } // End if has weapon

            } // End if within firing range

            // If we ARE firing, and the target is now out of range, switch back to this state
            // from our parent 'AdvanceFiring' state!
            if ( mFiring && distanceToTarget > mNPC.maxFiringRange )
            {
                mNPC.switchState( this );
                return;
            
            } // End if out of range

            // Keep advancing (switch to new position when the target moves
            // more than a four meters or so).
            mNPC.turnTowardPosition( mNPC.targetLastKnownPos );
            if ( vec3Length( mNPC.targetLastKnownPos - mLastNavigateTarget ) < 4.0f )
            {
                mLastNavigateTarget = mNPC.targetLastKnownPos;
                mNPC.controller.navigateTo( mNPC.targetLastKnownPos );
            
            } // End if moved > 2 meters

            // Continue selecting correct walking animation.
            Vector3 velocity = mNPC.controller.getVelocity();
            velocity.y = 0.0f;
            vec3Normalize( velocity, velocity );
            if ( mNPC.strafeLeftAnimationName != "" && abs( vec3Dot( velocity, mNPC.getSceneNode().getZAxis() ) ) < 0.7f )
            {
                if ( vec3Dot( velocity, mNPC.getSceneNode().getXAxis() ) >= 0 )
                    mNPC.selectAnimation( mNPC.strafeRightAnimationName, AnimationPlaybackMode::Loop, 0.0f );
                else
                    mNPC.selectAnimation( mNPC.strafeLeftAnimationName, AnimationPlaybackMode::Loop, 0.0f );
            } // End if strafe
            else
                mNPC.selectAnimation( mNPC.walkAnimationName, AnimationPlaybackMode::Loop, 0.0f );
        
        } // End if still acquired
        else
        {
            // Target is no longer acquired. Switch to searching state.
            mNPC.switchState( SearchingState( mAgent ) );
            return;

        } // End if not acquired
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Advancing";
    }
}