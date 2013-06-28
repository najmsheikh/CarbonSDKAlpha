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
// Name : Weapon_Trooper_Rifle.gs                                            //
//                                                                           //
// Desc : Behavior script associated with the trooper rifle held either by   //
//        the first person player, or another NPC.                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "../API/Weapon.gsh"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Weapon_Trooper_Rifle (Class)
// Desc : Behavior script associated with the trooper rifle held either by the
//        first person player, or another NPC.
//-----------------------------------------------------------------------------
shared class Weapon_Trooper_Rifle : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_Trooper_Rifle () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_Trooper_Rifle( )
    {
        // Describe the weapon
        mRepeatCycle            = 0.1f; // 100ms
        mRoundsPerMagazine      = 30;
        mRoundsPerBurst         = 3;    // When in burst fire mode.
        mBaseDamage             = 2;
        mMinDamageRange         = 80;
        mMaxDamageRange         = 100;
        mProjectileVelocity     = 100.0f;

        // Can be collected by the player
        mCanCollect              = true;
        mWeaponCollectIdentifier = "Weapon_M16";
        mAmmoCollectMagsMin      = 3;
        mAmmoCollectMagsMax      = 5;

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::FullyAutomatic;
        mCurrentMagazineRounds  = mRoundsPerMagazine;
        mTotalRounds            = mRoundsPerMagazine * 5000;
        mMaximumRounds          = mTotalRounds;
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
        @mMuzzleFlashEmitters[0] = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_Trooper_Rifle" ));
        //@mMuzzleFlashLight   = object.findChild( "Muzzle_Light_M16" );
        //@mEjectionPortSpawn  = object.findChild( "Weapon_Trooper_Rifle_Ejection" );

        // Load sound effects.
        mReloadSound      = mAudioManager.loadSound( "Sounds/M16 Reload.ogg", true );
        mFireLoopSound    = mAudioManager.loadSound( "Sounds/Carbine Fire.ogg", true );
        mFireEndSound     = mAudioManager.loadSound( "Sounds/Carbine Fire End.ogg", true );
        mFireOnceSound    = mAudioManager.loadSound( "Sounds/Carbine Shot.ogg", true );
        mWeaponDrySound   = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
        mMagazineLowSound = mAudioManager.loadSound( "Sounds/Magazine Low.ogg", true );
        mToggleModeSound  = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );

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
        return "Rifle";
    }
    
    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_Trooper_Rifle";
    }

    //-------------------------------------------------------------------------
	// Name : cycleFiringMode ()
	// Desc : Switch between the available firing modes for this weapon.
	//-------------------------------------------------------------------------
    WeaponFiringMode cycleFiringMode( )
    {
        // Switch between modes.
        if ( mFiringMode == WeaponFiringMode::SingleShot )
            mFiringMode = WeaponFiringMode::Burst;
        else if ( mFiringMode == WeaponFiringMode::Burst )
            mFiringMode = WeaponFiringMode::FullyAutomatic;
        else if ( mFiringMode == WeaponFiringMode::FullyAutomatic )
            mFiringMode = WeaponFiringMode::SingleShot;

        // Call base class last to have it play the sound.
        return Weapon::cycleFiringMode();
    }

} // End Class Weapon_Trooper_Rifle