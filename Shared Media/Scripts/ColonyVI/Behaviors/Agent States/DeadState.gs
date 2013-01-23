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
#include_once "../NPCAgent.gsh"

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
    private NPCAgent@   mNPC;
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
	DeadState( Agent @ agent, ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // Call base class constructor
        super( agent );

        // Initialize variables
        @mLastHitNode   = hitNode;
        mLastHitPoint   = hitPoint;
        mLastHitImpulse = hitImpulse;

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
        // Stop current animations.
        mNPC.actor.stopAnimationTrack( "Primary", true );
        mNPC.actor.stopAnimationTrack( "RecoverSourcePose", true );

        // Destroy the actor's current physics / character controller.
        mNPC.actor.setPhysicsController( null );

        // Agent is no longer alive.
        mNPC.setAlive( false );

        // Now apply ragdoll to have the agent collapse if it's supported.
        ObjectNode @ rootBone = null;
        if ( mNPC.supportsRagdoll )
        {
            // Find the root actor bone.
            @rootBone = mNPC.actor.findChild( "joint0" );
            if ( @rootBone != null )
            {
                // First get the existing controller and restore hierarchy if one is attached.
                RagdollController @ ragdoll = cast<RagdollController>(rootBone.getPhysicsController());
                if ( @ragdoll != null )
                    ragdoll.restoreHierarchy( false );

                // Create a new ragdoll controller.
                Scene @ scene = mNPC.actor.getScene();
                @ragdoll = RagdollController( scene.getPhysicsWorld() );
                rootBone.setPhysicsController( ragdoll );
                ragdoll.initialize( );

                // Apply an impulse to the ragdoll to make it fly appropriately.
                if ( @mLastHitNode != null )
                    ragdoll.applyImpulseTo( mLastHitNode, mLastHitImpulse, mLastHitPoint );

            } // End if found root bone
        
        } // End if supports ragdoll

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
}