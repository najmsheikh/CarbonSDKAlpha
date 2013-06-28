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
// Name : DeadState.gsh                                                      //
//                                                                           //
// Desc : Basic agent death state for NPC agents.                            //
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
// Name : DeadState (Class)
// Desc : Basic agent death state for NPC agents.
//-----------------------------------------------------------------------------
shared class DeadState : AgentState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@ mLastHitNode;
    private Vector3     mLastHitPoint;
    private Vector3     mLastHitImpulse;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : DeadState () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	DeadState( NPCAgent @ agent )
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
    void init( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        @mLastHitNode   = hitNode;
        mLastHitPoint   = hitPoint;
        mLastHitImpulse = hitImpulse;
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
        // Stop current animations.
        mAgent.actor.stopAnimationTrack( "Primary", true );
        mAgent.actor.stopAnimationTrack( "RecoverSourcePose", true );

        // Destroy the actor's current physics / character controller.
        mAgent.actor.setPhysicsController( null );

        // De-activate all collision volumes for any child skeleton.
        mAgent.actor.enableSkeletonCollision( false );

        // Agent is no longer alive.
        mAgent.setAlive( false );

        // Now apply ragdoll to have the agent collapse if it's supported.
        ObjectNode @ rootBone = null;
        if ( mAgent.supportsRagdoll )
        {
            // Find the root actor bone.
            @rootBone = mAgent.actor.findChild( mAgent.ragdollRootJoint );
            if ( @rootBone != null )
            {
                // First get the existing controller and restore hierarchy if one is attached.
                RagdollController @ ragdoll = cast<RagdollController>(rootBone.getPhysicsController());
                if ( @ragdoll != null )
                    ragdoll.restoreHierarchy( false );

                // Create a new ragdoll controller.
                Scene @ scene = mAgent.actor.getScene();
                @ragdoll = RagdollController( scene.getPhysicsWorld() );
                ragdoll.setDefaultConeLimit( mAgent.ragdollDefaultConeLimit );
                rootBone.setPhysicsController( ragdoll );
                ragdoll.initialize( );

                // Apply an impulse to the ragdoll to make it fly appropriately.
                if ( @mLastHitNode != null )
                    ragdoll.applyImpulseTo( mLastHitNode, mLastHitImpulse, mLastHitPoint );

            } // End if found root bone
        
        } // End if supports ragdoll
        else if ( mAgent.deathAnimationName != "" )
        {
            // Play death animation
            mAgent.selectAnimation( mAgent.deathAnimationName, AnimationPlaybackMode::PlayOnce, 0.0f );
        
        } // End if supports death animation

        // Switch success.
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
        // ToDo: Wait until ragdoll comes to rest (all bones sleeping)
        // and then replace with snapshot static mesh?
    }

    //-------------------------------------------------------------------------
	// Name : getStateName ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    String getStateName( )
    {
        return "Dead";
    }

    //-------------------------------------------------------------------------
	// Name : getStateIdentifier ()
	// Desc : Get the name of this state for debugging purposes.
	//-------------------------------------------------------------------------
    int getStateIdentifier( )
    {
        return AgentStateId::Dead;
    }
}