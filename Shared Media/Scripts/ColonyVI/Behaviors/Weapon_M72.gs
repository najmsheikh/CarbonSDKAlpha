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
// Name : Weapon_M72.gs                                                      //
//                                                                           //
// Desc : Behavior script associated with the player's M72 RPG launcher.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/Weapon.gsh"
#include_once "Rocket.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Weapon_M72 (Class)
// Desc : Behavior script associated with the player's M72 RPG launcher.
//-----------------------------------------------------------------------------
shared class Weapon_M72 : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_M72 () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_M72( )
    {
        // Describe the weapon
        mRoundsPerMagazine      = 1;
        mFireCooldown           = 2.0f;     // 2000ms cooldown on firing.
        mBaseDamage             = 100;
        mMinDamageRange         = 0;
        mMaxDamageRange         = 0;
        mProjectilesReleased    = 0;        // We never release projectiles
        mProjectileSpread       = 0.0f;
        mProjectileVelocity     = 30.0f;
		mWeaponFireVolume       = 1.0f;
        mCanSelect              = false;    // Player cannot select this weapon by default (until they pick it up)

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::SingleShot;
        mCurrentMagazineRounds  = mRoundsPerMagazine; // Start with one loaded when we get it
        mTotalRounds            = 0;                  // No initial ammo.
        mMaximumRounds          = 8;                  // Can carry a total of 8 rounds
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
        // Setup base class with references to required objects.
        mMuzzleFlashEmitters.resize(1);
        @mMuzzleFlashEmitters[0] = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_M72" ));
        //@mMuzzleFlashLight   = object.findChild( "Muzzle_Light_Beretta" );
        //@mEjectionPortSpawn  = object.findChild( "Weapon_Beretta_Ejection" );

        // Load sound effects.
        mReloadSound        = mAudioManager.loadSound( "Sounds/Rocket Launcher Reload.ogg", true );
        mFireOnceSound      = mAudioManager.loadSound( "Sounds/Rocket Fire.ogg", true );
        mWeaponDrySound     = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
        mWeaponCollectSound = mAudioManager.loadSound( "Sounds/Weapon Collect.ogg", true );

        // Trigger base class implementation
        Weapon::onAttach( object );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Call base class implementation
        Weapon::onDetach( object );
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Call base class implementation
        Weapon::onUpdate( elapsedTime );
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Weapon)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : attemptWeaponCollect ()
	// Desc : Attempt to collect ammunition from the specified weapon.
	//-------------------------------------------------------------------------
    bool attemptWeaponCollect( Weapon @ weapon )
    {
        // If we have no space left for ammo, don't collect.
        if ( mTotalRounds >= mMaximumRounds )
            return false;

        // Should we try and collect?
        if ( weapon.canCollect() && weapon.getWeaponCollectIdentifier() == "Weapon_M72" )
        {
            int numMagazines = randomInt( weapon.getAmmoCollectMagsMin(), weapon.getAmmoCollectMagsMax() );
            mTotalRounds += weapon.getRoundsPerMagazine() * numMagazines;
            if ( mTotalRounds > mMaximumRounds )
                mTotalRounds = mMaximumRounds;

            // Play the weapon collect sound.
            mAudioManager.playSound( mWeaponCollectSound, true, false, 1.0f, Vector3(0,0,0), mNode );

            // Weapon can now be selected by the player
            mCanSelect = true;

            // Collected
            return true;

        } // End if can collect
        
        // Pass down to base
        return Weapon::attemptWeaponCollect( weapon );
    }

    //-------------------------------------------------------------------------
	// Name : getClass ()
	// Desc : Retrieve the class of this weapon (i.e. Pistol, Rifle, etc.)
	//-------------------------------------------------------------------------
    String getClass( )
    {
        // Although it's a shotgun, it has a magazine and works
        // like a rifle.
        return "Rocket Launcher";
    }

    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_M72";
    }

    //-------------------------------------------------------------------------
	// Name : cycleFiringMode ()
	// Desc : Switch between the available firing modes for this weapon.
	//-------------------------------------------------------------------------
    WeaponFiringMode cycleFiringMode( )
    {
        // Weapon ONLY supports single shot.
        return WeaponFiringMode::SingleShot;
    }

    //-------------------------------------------------------------------------
	// Name : fireRound ()
	// Desc : Event that is triggered when a round should be fired. Call this
    //        last after additional processing in derived class. 'endfiring()'
    //        will automatically be called when there are no further rounds
    //        available.
	//-------------------------------------------------------------------------
    void fireRound( )
    {
        // Call base class implementation.
        Weapon::fireRound();

        // Spawn a rocket and release it into the world.
        Scene @ scene = mNode.getScene();
        ObjectNode @ rocketNode = scene.loadObjectNode( 0xF99, CloneMethod::ObjectInstance, true );
        ObjectNode @ attach = mMuzzleFlashEmitters[0];
        rocketNode.setPosition( attach.getPosition() );

        Vector3 x, y = Vector3(0,1,0), z;
        vec3Normalize( z, mAimTo - mAimFrom );
        vec3Cross( x, y, z );
        vec3Cross( y, z, x );
        rocketNode.setOrientation( x, y, z );

        // Initialize the rocket.
        Rocket @ rocket = cast<Rocket>(rocketNode.getScriptedBehavior(0));
        rocket.velocity = rocketNode.getZAxis() * mProjectileVelocity;
    }

} // End Class Weapon_M72