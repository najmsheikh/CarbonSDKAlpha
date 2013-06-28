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
#include_once "../API/NPCAgent.gsh"

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
    private int                 mExplosionSound;
    private int                 mFootstepSound;
    private array<int>          mIdleSounds;            // Array containing a random selection of grunting sounds for use during idling.
    private array<int>          mHuntingSounds;         // Array containing a random selection of grunting sounds for use during hunting.
    private array<int>          mDeathSounds;           // Array containing a random selection of grunting sounds for use during death.
    
    private array<SoundRef@>    mHuntingSoundChannels;
    private SoundRef@           mFootstepSoundChannel;
    
    private IntervalTimer       mIdleTimer;
    private IntervalTimer       mHuntingTimer;

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
        mFactionId              = 1;
        maxDetectionConeH       = 200.0f;
        maxDetectionConeV       = 90.0f;
        maxDetectionRange       = 40.0f;
        maxFiringRange          = 0.0f;
        idealFiringRange        = 0.0f;
        minFiringRange          = 0.0f;     // Don't try and stay separated.
        throttlePathFinding     = false;    // Constantly update path to target when charging.
        mMaximumHealth          = 75;
        mMaximumArmor           = 0;
        supportsRagdoll         = false;
        idleAnimationName       = "Idle";
        reloadAnimationName     = "";
        walkAnimationName       = "Run";
        maxSpeed                = 10.0f;

		mIdleTimer.setInterval( 7, 12 );
		mHuntingTimer.setInterval( 6, 12 );
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

        // Load sound effects.
        mIdleSounds.resize( 3 );
        mIdleSounds[0] = mAudioManager.loadSound( "Sounds/Wildlife 1.ogg", true );
        mIdleSounds[1] = mAudioManager.loadSound( "Sounds/Wildlife 2.ogg", true );
        mIdleSounds[2] = mAudioManager.loadSound( "Sounds/Wildlife 3.ogg", true );

        mHuntingSounds.resize( 1 );
        mHuntingSounds[0] = mAudioManager.loadSound( "Sounds/Monster Demon Screech.ogg", true );
		mFootstepSound    = mAudioManager.loadSound( "Sounds/Bug Movement.ogg", true );
        mExplosionSound   = mAudioManager.loadSound( "Sounds/Grenade Explosion.ogg", true );

        mHuntingSoundChannels.resize( 1 );
        @mHuntingSoundChannels[0] = null;
        @mFootstepSoundChannel    = null;
        
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
		// Release memory
		mIdleSounds.resize( 0 );
		mHuntingSounds.resize( 0 );
		mDeathSounds.resize( 0 );
		
        // Call base class implementation
        NPCAgent::onDetach( object );
    }

    //-------------------------------------------------------------------------
	// Name : processSound () (Event)
	// Desc : Called during onUpdate to select any sounds we wish to play based
	//        on current state.
	// Note : There are other ways to do this. This was the simplest although
	//        perhaps a bit hacky.
	//-------------------------------------------------------------------------
    void processSound( float elapsedTime )
    {
		if ( !isAlive() )
			return;

		int state = mState.getStateIdentifier();
    	if ( state == AgentStateId::Idle )
		{
            // Play idle sounds occassionally.
			if ( mIdleTimer.update( elapsedTime ) )
                mAudioManager.playSound( mIdleSounds[randomInt( 0, mIdleSounds.length()-1 )], true, false, 1.0f, Vector3(0,0,0), mNode );
		
		} // End if idle
    	else if ( state == AgentStateId::Repositioning )
		{
			// Play looping footsteps sound if not already.
            if ( !mAudioManager.isSoundPlaying( mFootstepSoundChannel ))
                @mFootstepSoundChannel = mAudioManager.playSound( mFootstepSound, true, true, 1.0f, Vector3(0,0,0), mNode );
            
            // If the creature is within X meters of its target, scream!
            float distanceToEnemy = vec3Length(mNode.getPosition() - targetLastKnownPos);
            if ( distanceToEnemy < 12.0f && !mAudioManager.isSoundPlaying( mHuntingSoundChannels[0] ) )
                @mHuntingSoundChannels[0] = mAudioManager.playSound( mHuntingSounds[0], true, false, 1.0f, Vector3(0,0,0), mNode );
		}
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing if we are no longer alive.
        if ( !isAlive() )
            return;

        // If the agent is within 2 meters of its target, explode!
        if ( targetAcquired )
        {
            float distanceToEnemy = vec3Length(mNode.getPosition() - targetLastKnownPos);
            if ( distanceToEnemy < 2.0f )
            {
                kill();
                return;
            
            } // End if explode!

        } // End if has target

        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );
        
        // Play sounds based on state
        processSound( elapsedTime );
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

        // Call the base class implementation first to finish up
        // (this will cause a switch to the 'dead' state).
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
                agent.onExplosionHit( mNode.getPosition(), 100, (1.0f-(distance / forceRange)), 100.0f );

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

        // Play the explosion sound.
        mAudioManager.playSound( mExplosionSound, true, false, 1.0f, mNode.getPosition(), null );

        // Stop any sounds that might still be playing.
        mAudioManager.stopSound( mHuntingSoundChannels[0] );
        mAudioManager.stopSound( mFootstepSoundChannel );

        // Unload the node entirely.
        mNode.unload( true );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (NPCAgent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
        return 1.1f;
    }

} // End Class Enemy_Explosive_Hound