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
// Name : Weapon_M16.gs                                                      //
//                                                                           //
// Desc : Behavior script associated with the M16 rifle held either by the   //
//        first person player, or another NPC.                               //
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
// Name : Weapon_M16 (Class)
// Desc : Behavior script associated with the M16 rifle held either by the
//        first person player, or another NPC.
//-----------------------------------------------------------------------------
shared class Weapon_M16 : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_M16 () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_M16( )
    {
        // Describe the weapon
        mRepeatCycle            = 0.1f; // 100ms
        mRoundsPerMagazine      = 30;
        mRoundsPerBurst         = 3;    // When in burst fire mode.
        mBaseDamage             = 25;
        mMinDamageRange         = 80;
        mMaxDamageRange         = 100;

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::FullyAutomatic;
        mCurrentMagazineRounds  = mRoundsPerMagazine;
        mTotalRounds            = mRoundsPerMagazine * 7;
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
        mMuzzleFlashEmitters.resize( 1 );
        @mMuzzleFlashEmitters[0] = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_M16" ));
        @mMuzzleFlashLight       = object.findChild( "Muzzle_Light_M16" );
        @mEjectionPortSpawn      = object.findChild( "Weapon_M16_Ejection" );

        // Load sound effects.
        mReloadSound        = mAudioManager.loadSound( "Sounds/M16 Reload.ogg", true );
        mFireLoopSound      = mAudioManager.loadSound( "Sounds/Carbine Fire.ogg", true );
        mFireEndSound       = mAudioManager.loadSound( "Sounds/Carbine Fire End.ogg", true );
        mFireOnceSound      = mAudioManager.loadSound( "Sounds/Carbine Shot.ogg", true );
        mWeaponDrySound     = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
        mMagazineLowSound   = mAudioManager.loadSound( "Sounds/Magazine Low.ogg", true );
        mToggleModeSound    = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
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
        if ( weapon.canCollect() && weapon.getWeaponCollectIdentifier() == "Weapon_M16" )
        {
            int numMagazines = randomInt( weapon.getAmmoCollectMagsMin(), weapon.getAmmoCollectMagsMax() );
            mTotalRounds += weapon.getRoundsPerMagazine() * numMagazines;
            if ( mTotalRounds > mMaximumRounds )
                mTotalRounds = mMaximumRounds;

            // Play the weapon collect sound.
            mAudioManager.playSound( mWeaponCollectSound, true, false, 1.0f, Vector3(0,0,0), mNode );

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
        return "Rifle";
    }
    
    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_M16";
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

} // End Class Weapon_M16