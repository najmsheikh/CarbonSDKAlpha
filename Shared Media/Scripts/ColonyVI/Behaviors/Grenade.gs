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
// Name : Grenade.gs                                                         //
//                                                                           //
// Desc : Behavior associated with thrown grenades.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Agent.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Grenade (Class)
// Desc : Behavior associated with thrown grenades.
//-----------------------------------------------------------------------------
shared class Grenade : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@             mNode;                  // The node to which we are attached.
    private float                   mTimeAlive;             // Amount of time this has been alive.
    private bool                    mArmed;
    private bool                    mExploded;
    private AudioBufferHandle       mExplosionSound;
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Grenade () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Grenade( )
    {
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
        // Initialize variables
        @mNode     = object;
        mArmed     = (mNode.getCustomProperties().getProperty( "armed", 0 ) != 0);
        mTimeAlive = 0;
        mExploded  = false;
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mNode = null;
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        mTimeAlive += elapsedTime;

        // Armed vs. unarmed grenade
        if ( !mArmed )
        {
            if ( mTimeAlive >= 1.75f )
            {
                // Spawn in the armed (world space) version of the grenade.
                Scene @ scene = mNode.getScene();
                ObjectNode @ armedGrenade = scene.loadObjectNode( 0x618, CloneMethod::ObjectInstance, true );
                
                // Position appropriately in relation to the owner actor.
                ObjectNode @ owner = mNode.getParentOfType( RTID_ActorObject );
                armedGrenade.setOrientation( owner.getZAxis(), -owner.getXAxis(), -owner.getYAxis() );
                armedGrenade.setPosition( owner.getPosition() + (owner.getZAxis() * 0.6f) + (owner.getYAxis() * 0.2f) + (owner.getXAxis() * 0.3f) );

                // Throw
                armedGrenade.applyImpulse( (owner.getZAxis() * 20.0f) + (owner.getYAxis() * 4.0f) );

                // Unload the unarmed grenade
                mNode.unload();
            
            } // End if throw
        
        } // End if !armed
        else
        {
            if ( !mExploded )
            {
                // Explode after a further 3 seconds from being thrown.
                if ( mTimeAlive >= 3.0f )
                {
                    // Spawn the explosion particle emitter.
                    Scene @ scene = mNode.getScene();
                    ObjectNode @ explosion = scene.loadObjectNode( 0x603, CloneMethod::ObjectInstance, false );
                    explosion.setPosition( mNode.getPosition() );

                    // Throw nearby dynamic objects and damage agents.
                    float forceRange = 10.0f;
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
                            agent.onExplosionHit( mNode.getPosition(), 400, (1.0f-(distance / forceRange)), 100.0f );

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

                    // Hide the grenade
                    mNode.showNode( false, true );
                    mExploded = true;

                    // Play the explosion sound.
                    ResourceManager @ resources = mNode.getScene().getResourceManager();
                    resources.loadAudioBuffer( mExplosionSound, "Sounds/Grenade Explosion.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
                    AudioBuffer @ effect = mExplosionSound.getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.set3DSoundPosition( mNode.getPosition() );
                        //effect.setVolume( 0.9f );
                        effect.setBufferPosition( 0 );
                        effect.play( false ); // Once
                    
                    } // End if loaded
                
                } // End if after 3 seconds

            } // End if !exploded
            else
            {
                // Wait until sound has finished
                AudioBuffer @ effect = mExplosionSound.getResource(true);
                if ( @effect == null || !effect.isLoaded() || !effect.isPlaying() )
                    mNode.unload();

            } // End if exploded
        
        } // End if armed
	}

} // End Class Grenade