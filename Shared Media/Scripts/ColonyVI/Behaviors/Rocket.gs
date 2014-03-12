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
// Name : Rocket.gs                                                          //
//                                                                           //
// Desc : Behavior associated with fired rockets.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/Agent.gsh"
#include_once "../API/AudioManager.gsh"
#include_once "../States/GamePlay.gs"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
const float DamageRadius        = 10.0f;
const float MaximumDamage       = 1000.0f;
const float MaximumObjectForce  = 400.0f;
const float MaximumAgentForce   = 200.0f;

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Grenade (Class)
// Desc : Behavior associated with fired rockets.
//-----------------------------------------------------------------------------
shared class Rocket : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    Vector3             velocity;

    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ObjectNode@             mNode;                  // The node to which we are attached.
    private ParticleEmitterNode@    mEmitter;
    private float                   mTimeAlive;             // Amount of time this has been alive.
    private bool                    mExploded;
    private SoundRef@               mRocketSound;
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Rocket () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Rocket( )
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
        @mNode      = object;
        mTimeAlive  = 0;
        mExploded   = false;

        // Get the rocket's child emitter.
        @mEmitter = cast<ParticleEmitterNode>(mNode.findChild( "rocket_emitter", false ));

        // Play rocket sound effects!
        AudioManager @ audioManager = getAudioManager();
        @mRocketSound = audioManager.playSound( "Sounds/0022_spaceship user - lightspeed space wind.ogg", true, false, 1.0f, Vector3(0,0,0), mNode );
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

        if ( !mExploded )
        {
            // Compute the new node position.
            Vector3 oldPos = mNode.getPosition();
            Vector3 newPos = oldPos + velocity * elapsedTime;

            // Cast a ray between old and new position and determine if there is anything to hit.
            SceneRayCastContact contact;
            Scene @ scene = mNode.getScene();
            bool hit = scene.rayCastClosest( oldPos, newPos, contact ) ||
                       scene.rayCastClosest( oldPos + mNode.getYAxis() * 0.1f + mNode.getXAxis() * 0.1f, newPos + mNode.getYAxis() * 0.1f + mNode.getXAxis() * 0.1f, contact ) ||
                       scene.rayCastClosest( oldPos - (mNode.getYAxis() * 0.1f + mNode.getXAxis() * 0.1f), newPos - (mNode.getYAxis() * 0.1f + mNode.getXAxis() * 0.1f), contact );
            if ( hit )
            {
                // Compute the intersection point.
                newPos = oldPos + ((newPos - oldPos) * contact.intersectParam);

                // Spawn the explosion particle emitter (Large Explosion).
                Vector3 dir;
                vec3Normalize( dir, (newPos - oldPos) );
                ObjectNode @ explosion = scene.loadObjectNode( 0xF9F, CloneMethod::ObjectInstance, false );
                explosion.setPosition( newPos - (dir * 1.0f) ); // Position explosion a little closer than the actual hit 

                // Position the original emitter closer than the explosion so that it draws on top.
                mEmitter.setPosition( newPos - (dir * 1.1f) );

                // Throw nearby dynamic objects and damage agents.
                array<ObjectNode@> nodes;
                scene.getObjectNodesInBounds( newPos, DamageRadius, nodes );
                for ( int i = 0; i < nodes.length(); ++i )
                {
                    // Skip self
                    ObjectNode @ testNode = nodes[i];
                    if ( @mNode == @testNode )
                        continue;

                    // Compute actual distance to object
                    Vector3 delta = testNode.getPosition() - newPos;
                    float distance = vec3Length( delta );

                    // If this object has a behavior, is it an agent?
                    Agent @ agent = null;
                    if ( testNode.getBehaviorCount() != 0 )
                        @agent = cast<Agent>(testNode.getScriptedBehavior(0));

                    // Notify if agent, otherwise attempt to throw.
                    if ( @agent != null )
                    {
                        // Notify!
                        agent.onExplosionHit( newPos, MaximumDamage, (1.0f-(distance / DamageRadius)), MaximumAgentForce );

                    } // End if agent
                    else
                    {
                        // Has physics body?
                        PhysicsBody @ body = testNode.getPhysicsBody();
                        if ( @body != null && body.getMass() > CGE_EPSILON )
                        {
                            delta /= distance; // normalize
                            body.applyImpulse( delta * (MaximumObjectForce / body.getMass()) * (1.0f-(distance / DamageRadius)) );
                        
                        } // End if has body

                    } // End if !agent
                                        
                } // Next object

                // Shake the camera.
                AppStateManager @ stateManager = getAppStateManager();
                AppState @ state = stateManager.getState( "GamePlay" );
                if ( @state != null )
                {
                    GamePlay @ gamePlay = cast<GamePlay>( state.getScriptObject() );
                    PlayerAgent @ player = gamePlay.getPlayer();
                    player.shakeCamera( 1.5f, 0.65f );
                
                } // End if found

                // Stop the rocket sand and play the explosion sound.
                AudioManager @ audioManager = getAudioManager();
                audioManager.stopSound( mRocketSound );
                audioManager.playSound( "Sounds/Grenade Explosion.ogg", true, false, 1.0f, newPos, null );

                // Hide the rocket and disable particle emission.
                mNode.showNode( false, false );
                for ( uint i = 0; i < mEmitter.getLayerCount(); ++i )
                    mEmitter.enableLayerEmission( i, false );
                mExploded = true;
            
            } // End if hit
            else
            {
                // Just reposition the node
                mNode.setPosition( newPos );
            
            } // End if !hit

        } // End if !exploded
        else
        {
            // Wait until particles are all spent and then unload us!.
            if ( mEmitter.particlesSpent( true ) )
                mNode.unload( true );

        } // End if exploded
	}

} // End Class Rocket