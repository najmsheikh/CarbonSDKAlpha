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
// Name : Weapon_Mech_Miniguns.gs                                            //
//                                                                           //
// Desc : Behavior script associated with the enemy mech's built in miniguns.//
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
// Name : Weapon_Mech_Miniguns (Class)
// Desc : Behavior script associated with the enemy mech's built in miniguns.
//-----------------------------------------------------------------------------
shared class Weapon_Mech_Miniguns : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_Mech_Miniguns () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_Mech_Miniguns( )
    {
        // Describe the weapon
        mRepeatCycle            = 0.05f; // 50ms
        mRoundsPerMagazine      = 99999;
        mRoundsPerBurst         = 20;
        mFiringSpinUpTime       = 1.8f;
        mFiringSpinDownTime     = 2.0f;
        mBaseDamage             = 6.25;
        mMinDamageRange         = 80;
        mMaxDamageRange         = 100;
        mProjectilesReleased    = 1;
        mProjectileVelocity     = 100.0f;

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::FullBurst;
        mCurrentMagazineRounds  = mRoundsPerMagazine;
        mTotalRounds            = mRoundsPerMagazine;
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
        mMuzzleFlashEmitters.resize( 2 );
        @mMuzzleFlashEmitters[0] = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_Mech_Minigun_01" ));
        @mMuzzleFlashEmitters[1] = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_Mech_Minigun_02" ));
        //@mMuzzleFlashLight   = object.findChild( "Muzzle_Light_M16" );
        //@mEjectionPortSpawn  = object.findChild( "Weapon_Trooper_Rifle_Ejection" );

        // Load sound effects.
        mReloadSound      = mAudioManager.loadSound( "Sounds/M16 Reload.ogg", true );
        mFireLoopSound    = mAudioManager.loadSound( "Sounds/Minigun Fire.ogg", true );
        mFireEndSound     = mAudioManager.loadSound( "Sounds/Carbine Fire End.ogg", true );
        mFireOnceSound    = mAudioManager.loadSound( "Sounds/Carbine Shot.ogg", true );
        mWeaponDrySound   = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
        mMagazineLowSound = mAudioManager.loadSound( "Sounds/Magazine Low.ogg", true );
        mToggleModeSound  = mAudioManager.loadSound( "Sounds/Carbine Dry.ogg", true );
        mSpinUpSound      = mAudioManager.loadSound( "Sounds/Minigun Spin Up.ogg", true );
        mSpinDownSound    = mAudioManager.loadSound( "Sounds/Minigun Spin Down.ogg", true );
		
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
        return "Fixed Miniguns";
    }
    
    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_Mech_Miniguns";
    }

    //-------------------------------------------------------------------------
	// Name : cycleFiringMode ()
	// Desc : Switch between the available firing modes for this weapon.
	//-------------------------------------------------------------------------
    WeaponFiringMode cycleFiringMode( )
    {
        // Weapon ONLY supports full burst mode.
        return WeaponFiringMode::FullBurst;
    }

} // End Class Weapon_Mech_Miniguns