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
// Name : Enemy_Mech.gs                                                      //
//                                                                           //
// Desc : Behavior script associated with the mech enemy NPC.                //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/NPCAgent.gsh"

// Custom states
#include_once "Custom States/MechLeapDownState.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Enemy_Mech (Class)
// Desc : Behavior script associated with the mech enemy NPC.
//-----------------------------------------------------------------------------
shared class Enemy_Mech : NPCAgent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    // Sound effects.
    private int                 mExplosionSound;
    private int                 mDeathSound;         
    private array<int>          mMechanicalSounds;
    private int                 mFootstepSound;
    private int                 mMinigunSpinUpSound;
    private int                 mMinigunFireSound;
    private int                 mMinigunSpinDownSound;
    private array<int>          mHitSounds;

    private array<SoundRef@>    mMechanicalSoundChannels;   // Channel in which mechanical sounds are playing

    private IntervalTimer       mFootstepTimer;
	
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Enemy_Mech () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Enemy_Mech( )
    {
        // Setup NPC description
        mFactionId                  = 1;
        mLOSRadius                  = 2.0f;
        maxDetectionConeH           = 200.0f;
        maxDetectionConeV           = 90.0f;
        maxDetectionRange           = 50.0f;
        maxFiringRange              = 30.0f;
        minFiringRange              = 7.0f;
        idealFiringRange            = 8.0f;
        maxFiringConeH              = 25.0f;
        mMaximumHealth              = 400;
        mMaximumArmor               = 400;
        maxSpeed                    = 2.0f;
        turnRate                    = 2.0f;
        supportsRagdoll             = false;
        canFireWhileMoving          = false;
        idleAnimationName           = "Idle";
        reloadAnimationName         = "";
        walkAnimationName           = "Walking";
        backpeddleAnimationName     = "Walking";
        deathAnimationName          = "Destroyed";
        strafeLeftAnimationName     = "Strafe Left";
        strafeRightAnimationName    = "Strafe Right";
        firingAnimationName         = "Short Gun Burst";
        backpeddleAnimationSpeed    = -1.0f;
		
		mFootstepTimer.setInterval( 0.5f, 0.8f );
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
        // Get the weapon associated with the mech.
        @mCurrentWeaponNode = object.findChild( "Weapon_Mech_Miniguns", true );

        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;

        // Load sound effects.
        mDeathSound    = mAudioManager.loadSound( "Sounds/Mech Death B.ogg", true );
        mFootstepSound = mAudioManager.loadSound( "Sounds/Mech Walk 2.ogg", true );

        mMechanicalSounds.resize( 2 );
        mMechanicalSounds[0] = mAudioManager.loadSound( "Sounds/0007_diesel_truck_ex_3.ogg", true );
        mMechanicalSounds[1] = mAudioManager.loadSound( "Sounds/0022_spaceship user - lightspeed space wind.ogg", true );
        
        mHitSounds.resize( 3 );
        mHitSounds[0] = mAudioManager.loadSound( "Sounds/0009_combat_dagger_hit_metal_c.ogg", true );
        mHitSounds[1] = mAudioManager.loadSound( "Sounds/0009_combat_dagger_hit_metal_e.ogg", true );
        mHitSounds[2] = mAudioManager.loadSound( "Sounds/0012_combat_dagger_hit_metal_b.ogg", true );

        
        // The mech will always play basic looping 'mechanical' sounds
        mMechanicalSoundChannels.resize( 2 );
        for ( int i = 0; i < mMechanicalSounds.length(); i++ )
		{
            float volume = (i == 0 ? 0.95f : 0.3f);
            @mMechanicalSoundChannels[i] = mAudioManager.playSound( mMechanicalSounds[i], true, true, volume, Vector3(0,0,0), mNode );
		
        } // Next sound
	
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
		mMechanicalSounds.resize( 0 );
        mHitSounds.resize( 0 );
		
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

		// Play a walking sound if advancing
		int state = mState.getStateIdentifier();
    	if ( state == AgentStateId::Searching || state == AgentStateId::Repositioning || state == AgentStateId::RepositionFiring )
		{
			if ( mFootstepTimer.update( elapsedTime ) )
                mAudioManager.playSound( mFootstepSound, true, false, 1.0f, Vector3(0,0,0), mNode );
		
        } // End if moving
    }
	
	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // If we're not alive, wait for animation to finish playing and then disable future updates.
        if ( !isAlive() )
        {
            if ( !actor.isAnimationTrackPlaying( "Primary", false ) )
                mNode.setUpdateRate( UpdateRate::Never );
        
        } // End if dead
        else
        {
            // Play sounds based on state
            processSound( elapsedTime );
        
        } // End if alive

        // Call base class implementation
        NPCAgent::onUpdate( elapsedTime );
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
                return MechLeapDownState( this );
        
        } // End switch
        
        // Call base
        return NPCAgent::createStateById( id );
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

        // Stop playing the mechanical sounds
		for ( int i = 0; i < mMechanicalSoundChannels.length(); i++ )
            mAudioManager.stopSound( mMechanicalSoundChannels[i] );

		// Play death sound(s)
        mAudioManager.playSound( mDeathSound, true, false, 1.0f, mNode.getPosition(), null );
    }

    //-------------------------------------------------------------------------
	// Name : onProjectileHit () (Event)
	// Desc : Agent was hit by a projectile.
	//-------------------------------------------------------------------------
    void onProjectileHit( Weapon @ weapon, ObjectNode @ intersectedNode, const Vector3 & from, const Vector3 & to )
    {
		// Always play a metal pinging sound when hit
        float volume = randomFloat( 0.3, 0.5 );
        mAudioManager.playSound( mHitSounds[ randomInt( 0, mHitSounds.length() - 1 ) ], true, false, volume, mNode.getPosition(), null );
				
        // Alive?
        if ( !isAlive() )
            return;
		
        // Call base class    
        NPCAgent::onProjectileHit( weapon, intersectedNode, from, to );
	}	
	
	
} // End Class Enemy_Mech