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
// Name : Enemy_Orc.gs                                                       //
//                                                                           //
// Desc : Behavior script associated with the Orc enemy NPC.                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/NPCAgent.gsh"
#include_once "../API/AudioManager.gsh"

// Custom states
#include_once "Custom States/OrcLeapDownState.gs"
#include_once "Custom States/OrcUsingTurretState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Enemy_Orc (Class)
// Desc : Behavior script associated with the Orc enemy NPC.
//-----------------------------------------------------------------------------
shared class Enemy_Orc : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    float                       mLastAimAngle;

    // Sound effects.
    private array<int>          mFootstepSounds;    // Array containing a random selection of footstep sounds for a metal surface.
    private array<int>          mIdleSounds;        // Array containing a random selection of grunting sounds for use during idling.
    private array<int>          mHuntingSounds;     // Array containing a random selection of grunting sounds for use during hunting.
    private array<int>          mDeathSounds;       // Array containing a random selection of grunting sounds for use during death.
    private array<int>          mHitSounds;         // Array containing a random selection of sounds for use when hit by a projectile.
    private array<SoundRef@>    mHitSoundChannels;
    
    private IntervalTimer       mIdleTimer;
    private IntervalTimer       mHuntingTimer;
    private IntervalTimer       mHitTimer;
   
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Enemy_Orc () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Enemy_Orc( )
    {
        // Setup NPC description
        mFactionId                  = 1;
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 40.0f;
        maxFiringConeH              = 15.0f;
        maxFiringConeV              = 45.0f;
        maxFiringRange              = 30.0f;
        minFiringRange              = 5.0f;
        idealFiringRange            = 6.0f;
        walkAnimationName           = "Weapon Move Slow";
        backpeddleAnimationName     = "Weapon Move Slow";
        strafeLeftAnimationName     = "Weapon Strafe Left";
        strafeRightAnimationName    = "Weapon Strafe Right";
        backpeddleAnimationSpeed    = -1.0f;

		mIdleTimer.setInterval( 7, 12 );
		mHuntingTimer.setInterval( 6, 12 );
		mHitTimer.setInterval( 3, 6 );

        // Setup agent description
        mMaximumHealth              = 100;
        mMaximumArmor               = 200;
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
        // Get the weapon associated with the orc.
        @mCurrentWeaponNode = object.findChild( "Weapon_Orc_Shotgun", true );

        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;
        mLastAimAngle       = 0;

        // Load sound effects.
        mIdleSounds.resize( 5 );
        mIdleSounds[0] = mAudioManager.loadSound( "Sounds/Growl 1.ogg", true );
        mIdleSounds[1] = mAudioManager.loadSound( "Sounds/Growl 2.ogg", true );
        mIdleSounds[2] = mAudioManager.loadSound( "Sounds/Growl 3.ogg", true );
        mIdleSounds[3] = mAudioManager.loadSound( "Sounds/Growl 4.ogg", true );
        mIdleSounds[4] = mAudioManager.loadSound( "Sounds/Growl 5.ogg", true );

        mHuntingSounds.resize( 3 );
        mHuntingSounds[0] = mAudioManager.loadSound( "Sounds/Growl Aggressive 0.ogg", true );
        mHuntingSounds[1] = mAudioManager.loadSound( "Sounds/Growl Aggressive 1.ogg", true );
        mHuntingSounds[2] = mAudioManager.loadSound( "Sounds/Growl Aggressive 2.ogg", true );

        mDeathSounds.resize( 1 );
        mDeathSounds[0] = mAudioManager.loadSound( "Sounds/Orc Death 0.ogg", true );

        mHitSounds.resize( 4 );
        mHitSoundChannels.resize( 4 );
        mHitSounds[0] = mAudioManager.loadSound( "Sounds/0042_impact_human_splat.ogg", true );
        mHitSounds[1] = mAudioManager.loadSound( "Sounds/Growl 7.ogg", true );
        mHitSounds[2] = mAudioManager.loadSound( "Sounds/Growl 8.ogg", true );
        mHitSounds[3] = mAudioManager.loadSound( "Sounds/Growl 9.ogg", true );

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
		mHitSounds.resize( 0 );
        mFootstepSounds.resize( 0 );

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
			// Play a random idle sound effect occasionally.
			if ( mIdleTimer.update( elapsedTime ) )
                mAudioManager.playSound( mIdleSounds[randomInt( 0, mIdleSounds.length()-1 )], true, false, 1.0f, Vector3(0,0,0), mNode );
		
        } // End if idle
		else
		{
			// Play a random hunting sound effect occasionally.
			if ( mHuntingTimer.update( elapsedTime ) )
                mAudioManager.playSound( mHuntingSounds[randomInt( 0, mHuntingSounds.length()-1 )], true, false, 1.0f, Vector3(0,0,0), mNode );
		
        } // End if !idle
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );

        // Should we be aiming?
        float angle = 0.0f;
        if ( targetAcquired )
        {
            // Compute the angle to the target.
            Vector3 dir;
            Vector3 from = actor.getPosition() + (actor.getYAxis() * getHeight() * 0.66f);
            Vector3 to   = targetLastKnownPos + actor.getYAxis() * (targetHeight * 0.66f);
            vec3Normalize( dir, to - from );
            angle = dir.y;
            if ( angle < -1.0f )
                angle = -1.0f;
            if ( angle > 1.0f )
                angle = 1.0f;
            angle = CGEToDegree(asin( angle ));
        
        } // End if has target
        
        // Smooth angle over time
        float aimSmoothFactor = 0.1f / elapsedTime;
        angle = smooth( angle, mLastAimAngle, aimSmoothFactor );
        mLastAimAngle = angle;

        // If there is any adjustment to be made, do it.
        if ( angle < -0.1f || angle > 0.1f )
        {
            // Adjust skeleton
            // TODO: Cache object
            ObjectNode @ joint = actor.findChild( "joint3", true );
            joint.rotateAxis( -angle, actor.getXAxis(), joint.getPosition() );
        
        } // End if acquired
        
        // Play sounds based on state
        processSound( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (NPCAgent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : createStateById ()
	// Desc : Create a new instance of the agent state associated with the 
    //        specified identifier. Can be overriden by derived types to supply
    //        custom states specific to their type for each identifier.
	//-------------------------------------------------------------------------
    AgentState @ createStateById( int id )
    {
        switch ( id )
        {
            case AgentStateId::LeapDown:
                return OrcLeapDownState( this );
            case AgentStateId::UsingTurret:
                return OrcUsingTurretState( this );
        
        } // End switch
        
        // Call base
        return NPCAgent::createStateById( id );
    }

    //-------------------------------------------------------------------------
	// Name : onProjectileHit () (Event)
	// Desc : Agent was hit by a projectile.
	//-------------------------------------------------------------------------
    void onProjectileHit( Weapon @ weapon, ObjectNode @ intersectedNode, const Vector3 & from, const Vector3 & to )
    {
        // Alive?
        if ( !isAlive() )
            return;
        
        // Play impact sound
        mAudioManager.playSound( mHitSounds[0], true, false, 0.6f, mNode.getPosition(), null );
        
        // Play hit/grunt sounds randomly
        if ( randomFloat( 0.0f, 1.0f ) > 0.2f )
        {
            // Are any grunt sounds currently playing?
            bool playing = false;
            for ( int i = 1; i <= 3; ++i )
            {   
                if ( mAudioManager.isSoundPlaying( mHitSoundChannels[i] ) )
                {
                    playing = true;
                    break;
                } // End if playing
            
            } // Next sound

            // If nothing was found to be playing, play a random hit sound
            if ( !playing )
            {
                int soundIndex = randomInt( 1, 3 );
                @mHitSoundChannels[soundIndex] = mAudioManager.playSound( mHitSounds[soundIndex], true, false, 1.0f, mNode.getPosition(), null );
            
            } // End if !playing
        
        } // End if 20% chance
        
        // Call base class    
        NPCAgent::onProjectileHit( weapon, intersectedNode, from, to );
	}	
	
    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger NPC death and apply an impulse to the specified node
    //        in the ragdoll that is spawned.
	//-------------------------------------------------------------------------
    void kill( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // No-op?
        if ( !isAlive() )
            return;

        // Call base class implementation first to finish up
        NPCAgent::kill( hitNode, hitPoint, hitImpulse );

        // Drop the weapon.
        if ( @mCurrentWeaponNode != null )
        {
            mCurrentWeaponNode.setParent( null );
            mCurrentWeaponNode.setPhysicsModel( PhysicsModel::RigidDynamic );
        
        } // End if has weapon

        // Agent no longer gets any updates.
        mNode.setUpdateRate( UpdateRate::Never );

		// Play a death sound	
        mAudioManager.playSound( mDeathSounds[randomInt( 0, mDeathSounds.length() - 1 )], true, false, 1.0f, mNode.getPosition(), null );
    }

} // End Class Enemy_Orc