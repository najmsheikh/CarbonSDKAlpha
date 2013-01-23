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
// Name : Weapon_Orc_Shotgun.gs                                              //
//                                                                           //
// Desc : Behavior script associated with the enemy orc shotgun held either  //
//        by the first person player, or another NPC.                        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Weapon.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Weapon_Orc_Shotgun (Class)
// Desc : Behavior script associated with the enemy orc shotgun held either by 
//        the first person player, or another NPC.
//-----------------------------------------------------------------------------
shared class Weapon_Orc_Shotgun : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_Orc_Shotgun () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_Orc_Shotgun( )
    {
        // Describe the weapon
        mRoundsPerMagazine      = 8;
        mFireCooldown           = 0.8f; // 800ms cooldown on firing.
        mBaseDamage             = 100;
        mMinDamageRange         = 0;
        mMaxDamageRange         = 30;
        mProjectilesReleased    = 7;
        mProjectileSpread       = 10.0f;
        mProjectileVelocity     = 100.0f;

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::SingleShot;
        mCurrentMagazineRounds  = mRoundsPerMagazine;
        mTotalRounds            = mRoundsPerMagazine * 4000;
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
        @mMuzzleFlashEmitter = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_Orc_Shotgun" ));
        //@mMuzzleFlashLight   = object.findChild( "Muzzle_Light_Beretta" );
        //@mEjectionPortSpawn  = object.findChild( "Weapon_Beretta_Ejection" );

        // Load sound effects.
        ResourceManager @ resources = object.getScene().getResourceManager();
        resources.loadAudioBuffer( mReloadSound,   "Sounds/Beretta Reload.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mFireOnceSound, "Sounds/Shotgun Fire.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mFireEndSound,  "Sounds/Carbine Fire End.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mWeaponDrySound,  "Sounds/Carbine Dry.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mMagazineLowSound,  "Sounds/Magazine Low.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mToggleModeSound,  "Sounds/Carbine Dry.ogg", AudioBufferFlags::Complex3D, 0, DebugSource() );

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
	// Name : getClass ()
	// Desc : Retrieve the class of this weapon (i.e. Pistol, Rifle, etc.)
	//-------------------------------------------------------------------------
    String getClass( )
    {
        // Although it's a shotgun, it has a magazine and works
        // like a rifle.
        return "Rifle";
    }

    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_Orc_Shotgun";
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

} // End Class Weapon_Orc_Shotgun