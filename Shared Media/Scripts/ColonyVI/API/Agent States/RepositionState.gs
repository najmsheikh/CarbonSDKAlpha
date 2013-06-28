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
// Name : RepositionState.gsh                                                //
//                                                                           //
// Desc : NPC agent is repositioning toward current target.                  //
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
// Name : RepositionState (Class)
// Desc : NPC agent is repositioning toward current target.
//-----------------------------------------------------------------------------
shared class RepositionState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Vector3    mLastNavigateTarget;
    private bool       mFiring;
    private float      mTimeSpentInState;
    private float      mCircleStrafeOffset;
    private float      mCircleStrafeSign;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : RepositionState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	RepositionState( NPCAgent @ agent )
    {
        // Call base class constructor
        super( agent );

        // Initialize variables
        mFiring = false;
        mCircleStrafeOffset = 0.0f;
        mCircleStrafeSign   = (randomFloat(0,1) > 0.5f) ? 1 : -1;
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
        mTimeSpentInState = 0.0f;

        // If we're returning from a switch to the "reposition and firing" state 
        // (but we were just temporarily suspended), we don't need to initialize
        // again.
        if ( mFiring )
        {
            // Continue navigation to our last target just in case we were stopped.
            mAgent.controller.navigateToAtRange( mLastNavigateTarget, mAgent.minFiringRange, 0 );
            mFiring = false;
            return true;
        
        } // End if suspended

        // Start advancing toward the last known position of the target
        mLastNavigateTarget = mAgent.targetLastKnownPos;
        if ( !mAgent.controller.navigateToAtRange( mAgent.targetLastKnownPos, mAgent.minFiringRange, randomFloat( -CGE_PI/2, CGE_PI/2 ) ) )
        {
            // Just try navigating directly to target.
            if ( !mAgent.controller.navigateTo( mAgent.targetLastKnownPos ) )
            {
                // If we were unable to set a target, switch to searching.
                mAgent.switchState( AgentStateId::Searching );
                return false;
            
            } // End if failed
        
        } // End if failed

        // Switch to correct walking animation.
        processAnimation();

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
        Weapon @ weapon = mAgent.getCurrentWeapon();
        mTimeSpentInState += elapsedTime;

        // We still have a target?
        if ( mAgent.targetAcquired )
        {
            // Target is still currently acquired. If it's inside our ideal
            // firing range, or we have less than 75% visibility, stop advancing 
            // on them. Otherwise keep advancing.
            float distanceToTarget = vec3Length( mAgent.getSceneNode().getPosition() - mAgent.targetLastKnownPos );
            if ( distanceToTarget >= mAgent.minFiringRange && 
                 distanceToTarget <= mAgent.idealFiringRange && 
                 mAgent.targetVisibilityScore >= 0.75f )
            {
                mAgent.switchState( AgentStateId::Idle );
                return;
            
            } // End if close enough

            // If the target is inside the total maximum firing range (and we're not already firing), fire!
            // The exception to this is if the NPC is unable to move while firing, in which case it
            // should be allowed to move for at least a couple of seconds before firing.
            if ( !mFiring && distanceToTarget <= mAgent.maxFiringRange && 
                 (mAgent.canFireWhileMoving || mTimeSpentInState > 2.0f || mAgent.controller.getNavigationState() == NavigationTargetState::Arrived ) )
            {
                // NPC has a weapon?
                if ( @weapon != null )
                {
                    // If our target is acquired, the NPC is willing to fire, and the weapon
                    // has ammo remaining, attempt to switch to the firing state.
                    if ( mAgent.targetAcquired && mAgent.willingToFire && weapon.getAvailableRounds() > 0 && weapon.getState() != WeaponState::Cooldown )
                    {
                        // Has to stop to fire?
                        if ( mAgent.canFireWhileMoving )
                        {
                            mAgent.switchState( AgentStateId::RepositionFiring );

                            // We're suspended if the state switch was successful.
                            // Other state(s) may switch back to us when they are done.
                            if ( @mAgent.getState() != @this )
                            {
                                mFiring = true;
                                return;
                            
                            } // End if switched
                        
                        } // End if can fire while moving
                        else
                        {
                            // Stop navigating where we are and fire (will switch back
                            // to idle on completion).
                            mAgent.controller.navigateTo( mAgent.getSceneNode().getPosition() );
                            mAgent.switchState( AgentStateId::Firing );
                            return;
                        
                        } // End if cannot fire while moving

                    } // End if fire! 

                } // End if has weapon

            } // End if within firing range

            // If we ARE firing, and the target is now out of range, switch back to this state
            // from our parent 'RepositionFiring' state!
            if ( mFiring && distanceToTarget > mAgent.maxFiringRange )
            {
                mAgent.switchState( this );
                return;
            
            } // End if out of range

            // Keep advancing (switch to new position when the target moves
            // more than a four meters or so -- or the NPC needs to backpeddle).
            mAgent.turnTowardPosition( mAgent.targetLastKnownPos );
            float targetTravelled = vec3Length( mAgent.targetLastKnownPos - mLastNavigateTarget );
            if ( !mAgent.throttlePathFinding || targetTravelled >= 4.0f || distanceToTarget < mAgent.minFiringRange )
            {
                // Attempt to strafe around the target.
                if ( distanceToTarget < mAgent.minFiringRange )
                    mCircleStrafeOffset = CGE_PI / 10.0f; //+= (CGE_PI/20.0f) * elapsedTime;
                else
                {
                    mCircleStrafeSign = (randomFloat(0,1) > 0.5f) ? 1 : -1;
                    mCircleStrafeOffset = 0.0f;
                
                } // End if reset

                // Navigate
                mLastNavigateTarget = mAgent.targetLastKnownPos;
                if ( !mAgent.controller.navigateToAtRange( mAgent.targetLastKnownPos, mAgent.minFiringRange, mCircleStrafeOffset * mCircleStrafeSign ) )
                {
                    if ( targetTravelled >= 4.0f )
                        mAgent.controller.navigateTo( mAgent.targetLastKnownPos );
                
                } // End if failed
            
            } // End if should update path
            
            // Continue selecting correct walking animation.
            processAnimation();
        
        } // End if still acquired
        else
        {
            // Target is no longer acquired. Switch to searching state.
            mAgent.switchState( AgentStateId::Searching );
            return;

        } // End if not acquired
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Repositioning";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Repositioning;
    }

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
	// Name : processAnimation () (Private)
	// Desc : Select the correct animation for the NPC during reposition.
	//-------------------------------------------------------------------------
    private void processAnimation( )
    {
        Vector3 velocity = mAgent.controller.getVelocity();
        velocity.y = 0.0f;
        vec3Normalize( velocity, velocity );
        float walkSpeed = vec3Dot( velocity, mAgent.getSceneNode().getZAxis() );
        if ( walkSpeed > 0.7f )
        {
            if ( mAgent.walkAnimationName != "" )
                mAgent.selectAnimation( mAgent.walkAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ), mAgent.walkAnimationSpeed );

        } // End if forward
        else if ( walkSpeed < -0.7f )
        {
            if ( mAgent.backpeddleAnimationName != "" )
                mAgent.selectAnimation( mAgent.backpeddleAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ), mAgent.backpeddleAnimationSpeed );

        } // End if backward
        else
        {
            float strafeSpeed = vec3Dot( velocity, mAgent.getSceneNode().getXAxis() );
            if ( strafeSpeed >= 0.7f && mAgent.strafeLeftAnimationName != "" )
                mAgent.selectAnimation( mAgent.strafeRightAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ), mAgent.strafeAnimationSpeed );
            else if ( strafeSpeed <= -0.7f && mAgent.strafeRightAnimationName != "" )
                mAgent.selectAnimation( mAgent.strafeLeftAnimationName, AnimationPlaybackMode::Loop, randomFloat( 0.0f, 10.0f ), mAgent.strafeAnimationSpeed );

        } // End if strafe
    }

}