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
// Name : Enemy_Explosive_Hound.gs                                           //
//                                                                           //
// Desc : Behavior script associated with the explosive hound enemy NPC.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "NPCAgent.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Enemy_Explosive_Hound (Class)
// Desc : Behavior script associated with the explosive hound enemy NPC.
//-----------------------------------------------------------------------------
shared class Enemy_Explosive_Hound : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AudioBufferHandle       mExplosionSound;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Enemy_Explosive_Hound () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Enemy_Explosive_Hound( )
    {
        // Setup NPC description
        maxDetectionConeH       = 200.0f;
        maxDetectionConeV       = 90.0f;
        maxDetectionRange       = 40.0f;
        maxFiringRange          = 2.0f; // Move within 2 radius when advancing on the player.
        mMaximumHealth          = 75;
        mMaximumArmor           = 0;
        supportsRagdoll         = false;
        idleAnimationName       = "Idle";
        reloadAnimationName     = "";
        walkAnimationName       = "Run";
        maxSpeed                = 10.0f;

    }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedObjectBehavior)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : onAttach () (Event)
	// Desc : Called by the application when the behavior is first attached to
	//        an object and can initialize.
	//-------------------------------------------------------------------------
	void onAttach( ObjectNode @ object )
	{
        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;

        // Trigger base class implementation
        NPCAgent::onAttach( object );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Call base class implementation
        NPCAgent::onDetach( object );
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Exploded yet?
        if ( !isAlive() )
        {
            // Wait until sound has finished and then unload the agent and all its children.
            AudioBuffer @ effect = mExplosionSound.getResource(true);
            if ( @effect == null || !effect.isLoaded() || !effect.isPlaying() )
                mNode.unload( false ); // ToDo: Unload all children!
        
        } // End if exploded
        else
        {
            // If the agent is within 2 meters of the player, explode!
            InputDriver @ input = getAppInputDriver();
            if ( vec3Length(mNode.getPosition() - playerNode.getPosition()) < 2.0f )
                kill();
        
        } // End if !exploded

        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );
	}

    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger NPC death.
	//-------------------------------------------------------------------------
    void kill( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // No-op?
        if ( !isAlive() )
            return;

        // Call the base class implementation first to finish up.
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );

        // Spawn the explosion particle emitter.
        Scene @ scene = mNode.getScene();
        ObjectNode @ explosion = scene.loadObjectNode( 0x603, CloneMethod::ObjectInstance, false );
        explosion.setPosition( mNode.getPosition() );

        // Throw nearby dynamic objects and damage agents.
        float forceRange = 7.0f;
        array<ObjectNode@> nodes;
        scene.getObjectNodesInBounds( mNode.getPosition(), forceRange, nodes );
        for ( int i = 0; i < nodes.length(); ++i )
        {
            // Skip self
            ObjectNode @ testNode = nodes[i];
            if ( @mNode == @testNode )
                continue;

            // Compute actual distance to object
            Vector3 delta = testNode.getPosition() - mNode.getPosition();
            float distance = vec3Length( delta );

            // If this object has a behavior, is it an agent?
            Agent @ agent = null;
            if ( testNode.getBehaviorCount() != 0 )
                @agent = cast<Agent>(testNode.getScriptedBehavior(0));

            // Notify if agent, otherwise attempt to throw.
            if ( @agent != null )
            {
                // Notify!
                agent.onExplosionHit( mNode.getPosition(), 60, (1.0f-(distance / forceRange)), 100.0f );

            } // End if agent
            else
            {
                // Has physics body?
                PhysicsBody @ body = testNode.getPhysicsBody();
                if ( @body != null && body.getMass() > CGE_EPSILON )
                {
                    delta /= distance; // normalize
                    body.applyImpulse( delta * (400.0f / body.getMass()) * (1.0f-(distance / forceRange)) );
                
                } // End if has body

            } // End if !agent
                                
        } // Next object

        // Hide the agent
        mNode.showNode( false, true );

        // Play the explosion sound.
        ResourceManager @ resources = scene.getResourceManager();
        resources.loadAudioBuffer( mExplosionSound, "Sounds/Grenade Explosion.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
        AudioBuffer @ effect = mExplosionSound.getResource(true);
        if ( @effect != null && effect.isLoaded() )
        {
            effect.set3DSoundPosition( mNode.getPosition() );
            //effect.setVolume( 0.9f );
            effect.setBufferPosition( 0 );
            effect.play( false ); // Once
        
        } // End if loaded

    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (NPCAgent)
	///////////////////////////////////////////////////////////////////////////

} // End Class Enemy_Explosive_Hound