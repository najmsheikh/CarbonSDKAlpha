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
// Name : Weapon_Beretta.gs                                                  //
//                                                                           //
// Desc : Behavior script associated with the Beretta pistol held either by  //
//        the first person player, or another NPC.                           //
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
// Name : Weapon_Beretta (Class)
// Desc : Behavior script associated with the Beretta pistol held either by the
//        first person player, or another NPC.
//-----------------------------------------------------------------------------
shared class Weapon_Beretta : Weapon
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon_Beretta () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon_Beretta( )
    {
        // Describe the weapon
        mRoundsPerMagazine      = 15;
        mBaseDamage             = 30;
        mMinDamageRange         = 50;
        mMaxDamageRange         = 100;
        mProjectileSpread       = 1.0f;

        // Setup the initial weapon state.
        mFiringMode             = WeaponFiringMode::SingleShot;
        mCurrentMagazineRounds  = mRoundsPerMagazine;
        mTotalRounds            = mRoundsPerMagazine * 11;
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
        @mMuzzleFlashEmitter = cast<ParticleEmitterNode>(object.findChild( "Muzzle_Flash_Beretta" ));
        @mMuzzleFlashLight   = object.findChild( "Muzzle_Light_Beretta" );
        @mEjectionPortSpawn  = object.findChild( "Weapon_Beretta_Ejection" );

        // Load sound effects.
        ResourceManager @ resources = object.getScene().getResourceManager();
        resources.loadAudioBuffer( mReloadSound,   "Sounds/Beretta Reload.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
        resources.loadAudioBuffer( mFireOnceSound, "Sounds/Beretta Shot.wav", AudioBufferFlags::Complex3D, 0, DebugSource() );
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
        return "Pistol";
    }

    //-------------------------------------------------------------------------
	// Name : getIdentifier ()
	// Desc : Retrieve the instance identifier of this weapon.
	//-------------------------------------------------------------------------
    String getIdentifier( )
    {
        return "Weapon_Beretta";
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

} // End Class Weapon_Beretta