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

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : IdleState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	IdleState( NPCAgent @ agent )
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
        // Stop navigating where we are.
        if ( @mAgent.controller != null )
            mAgent.controller.navigateTo( mAgent.getSceneNode().getPosition() );

        // Switch to idle animation
        mAgent.selectAnimation( mAgent.idleAnimationName, AnimationPlaybackMode::Loop, 0.0f );

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
        // Turning?
        Transform t;
        t.setOrientation( mAgent.requestedOrientation );
        Vector3 requestedLook = t.normalizedZAxis();
        Vector3 actualLook = mAgent.actor.getZAxis();
        float angleDelta = acos( vec3Dot( actualLook, requestedLook ) );
        if ( abs(angleDelta) > 0.1f )
        {
            // Right or left turn?
            Vector3 direction;
            vec3Cross( direction, requestedLook, actualLook );
            if ( direction.y > 0 )
            {
                if ( mAgent.strafeLeftAnimationName != "" )
                    mAgent.selectAnimation( mAgent.strafeLeftAnimationName, AnimationPlaybackMode::Loop, 0.0f, 0.75f );
            
            } // End if left
            else
            {
                if ( mAgent.strafeRightAnimationName != "" )
                    mAgent.selectAnimation( mAgent.strafeRightAnimationName, AnimationPlaybackMode::Loop, 0.0f, 0.75f );
            
            } // End if right
        
        } // End if turning
        else
            mAgent.selectAnimation( mAgent.idleAnimationName, AnimationPlaybackMode::Loop, 0.0f );

        // Navigation behaviors?
        if ( @mAgent.controller != null )
        {
            // We have a target now?
            if ( mAgent.targetAcquired )
            {
                float distanceToTarget = vec3Length( mAgent.getSceneNode().getPosition() - mAgent.targetLastKnownPos );

                // If target is outside of our ideal firing range, or we have 
                // less than 75% visibility on them, start repositioning.
                if ( distanceToTarget < mAgent.minFiringRange || 
                     distanceToTarget > mAgent.idealFiringRange || 
                     mAgent.targetVisibilityScore < 0.75f )
                {
                    mAgent.switchState( AgentStateId::Repositioning );
                    return;
                
                } // End if out of range
                else
                {
                    // Turn toward them.
                    mAgent.turnTowardPosition( mAgent.targetLastKnownPos );
                
                } // End if turn
            
            } // End if acquired
            else
            {
                // If we saw or heard them just a little while ago, search!
                if ( mAgent.timeSinceTargetLost <= 10.0f || mAgent.timeSinceTargetHeard <= 10.0f )
                {
                    mAgent.switchState( AgentStateId::Searching );
                    return;
                
                } // End if start searching
            
            } // End if !acquired
        
        } // End if can navigate
        else
        {
            // Turn on target position if acquired.
            if ( mAgent.targetAcquired )
                mAgent.turnTowardPosition( mAgent.targetLastKnownPos );

        } // End if cannot navigate

        // NPC has a weapon?
        Weapon @ weapon = mAgent.getCurrentWeapon();
        if ( @weapon != null )
        {
            // If our target is acquired, the NPC is willing to fire, and the weapon
            // has ammo remaining, attempt to switch to the firing state.
            if ( mAgent.targetAcquired && mAgent.willingToFire && weapon.getAvailableRounds() > 0 && weapon.getState() != WeaponState::Cooldown )
                mAgent.switchState( AgentStateId::Firing );

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

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Idle;
    }
}