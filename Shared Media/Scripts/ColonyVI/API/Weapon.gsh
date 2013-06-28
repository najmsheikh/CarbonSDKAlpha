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
// Name : Weapon.gsh                                                         //
//                                                                           //
// Desc : Base weapon class from which all weapon types should derive.       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Agent.gsh"
#include_once "AudioManager.gsh"
#include_once "../Behaviors/DestructibleLight.gs"

//-----------------------------------------------------------------------------
// Public Enumerations
//-----------------------------------------------------------------------------
shared enum WeaponFiringResult
{
    Success,
    MagazineEmpty
}

shared enum WeaponFiringMode
{
    SingleShot,
    Burst,
    FullBurst,      // Burst cannot be interrupted.
    FullyAutomatic
}

shared enum WeaponState
{
    Ready,
    Empty,
    Firing,
    Cooldown
}

shared enum WeaponReloadResult
{
    Success,
    InvalidState,
    NoAmmo
}

shared class Tracer
{
    BillboardNode@  node;
    float           distanceToTarget;
    Vector3         velocity;
    Vector3         direction;
    Vector3         from;
    Vector3         to;
    bool            hit;
    bool            active;
    uint            hitEffect;
};

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Weapon (Base Class)
// Desc : Base weapon class from which all weapon types should derive.
//-----------------------------------------------------------------------------
shared class Weapon : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AudioManager@               mAudioManager;              // Manager responsible for the weapon sounds.
    private ObjectNode@                 mNode;                      // The node to which we are attached.
    private Agent@                      mOwnerAgent;                // The agent that owns this weapon.
    private ObjectNode@                 mEjectionPortSpawn;         // The point at which casings will be ejected (if any).
    private array<ParticleEmitterNode@> mMuzzleFlashEmitters;       // Particle emitter representing muzzle flash.
    private ObjectNode@                 mMuzzleFlashLight;          // Light source used when firing.
    private array<Tracer@>              mTracers;                   // List of available tracers.

    // Sound effects.
    private int                         mReloadSound;               // Sound played during reloading.
    private int                         mFireLoopSound;             // Looping fire sound.
    private SoundRef@                   mFireLoopSoundChannel;
    private int                         mFireOnceSound;             // Single shot fire sound.
    private int                         mFireEndSound;              // Sound to play when firing ceases (slight echo).
    private int                         mWeaponDrySound;            // Sound to play when unable to reload.
    private int                         mMagazineLowSound;          // Sound that plays over the top of the fire loop when magazine almost empty.
    private int                         mToggleModeSound;           // Sound that plays when the firing mode is toggled.
    private int                         mSpinUpSound;               // Sound that plays when the weapon is spinning up
    private SoundRef@                   mSpinUpSoundChannel;
    private int                         mSpinDownSound;             // Sound that plays when the weapon is spinning down
    private int                         mWeaponCollectSound;        // Sound to play when weapon / ammo is collected.
    private array<int>                  mHitEnvironmentSounds;      // Sound played when a projectile hits the environment
    private array<int>                  mRicochetSounds;            // Sound played when a projectile ricochets off the environment
	
    // Weapon descriptor
    private int                         mMaximumRounds;             // Maximum number of rounds that can be carried for this weapon type.
    private float                       mRepeatCycle;               // Amount of time (in seconds) between each firing.
    private int                         mRoundsPerMagazine;         // Number of rounds in a magazine (fixed).
    private int                         mRoundsPerBurst;            // In burst fire mode, how many rounds should be fired?
    private float                       mBaseDamage;                // Base damage amount for this weapon.
    private float                       mMaxDamageRange;            // At this range, no damage will be applied.
    private float                       mMinDamageRange;            // At this range, full damage will be applied.
    private float                       mFireCooldown;              // After firing, this describes the amount of time until the gun is ready to fire again.
    private int                         mProjectilesReleased;       // How many projectiles are released for each round fired (equal distribution of base damage)?
    private float                       mProjectileSpread;          // Angle in degrees that specifies the cone of the overall spread of released projectiles.
    private float                       mProjectileVelocity;        // Velocity of each projectile as it leaves the barrel in meters per second.
    private float                       mFiringSpinUpTime;          // Amount of time it takes the weapon to spin up before firing. Weapon will be classed as firing during this time, but will not release projectiles.
    private float                       mFiringSpinDownTime;        // Amount of time it takes the weapon to spin down after firing (i.e. FullBurst).

    private bool                        mCanSelect;                 // Can this weapon be selected for use by the player?
    private bool                        mCanCollect;                // Can this weapon be collected when it is dropped on the floor?
    private String                      mWeaponCollectIdentifier;   // What player weapon does this apply to when collected.
    private int                         mAmmoCollectMagsMin;        // Minimum number of mags to be added to player's inventory on collect.
    private int                         mAmmoCollectMagsMax;        // Maximum number of mags to be added to player's inventory on collect.

    // Current weapon properties
    private WeaponState                 mState;                     // The current state of the weapon (firing, etc.)
    private WeaponFiringMode            mFiringMode;                // The current firing mode of the weapon (burst, full auto, etc.)
    private int                         mCurrentMagazineRounds;     // Number of rounds in the current magazine.
    private int                         mTotalRounds;               // Total available rounds (including those in current magazine).
    private float                       mFireCycleTime;             // Cycling timer used to determine when to fire.
    private int                         mRoundsFired;               // Number of rounds fired since firing began.
    private Vector3                     mAimFrom;                   // The source point for weapon projectiles.
    private Vector3                     mAimTo;                     // The destination point for weapon projectiles.
    private int                         mCurrentEmitter;            // Round robin counter for muzzle flash emitter
    private float                       mWeaponFireVolume;          // The volume level for the firing sound
	
    // Time delay handling
    private float                       mFireTimeTotal;             // Total amount of time we have been firing.
    private float                       mTimeSinceFired;            // Amount of time since last firing.
    private bool                        mHasFired;                  // Should we still be processing things after firing? (muzzle flash, sound)

    // Miscellaneous
	private String                  	mCasingScriptName;          // Name of the script for ejected casings. 
    private ScriptHandle                mCasingScript;              // Used simply to keep the casing script 'warm'.
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon( )
    {
        @mAudioManager          = getAudioManager();
        mRepeatCycle            = 0;
        mRoundsPerMagazine      = 0;
        mRoundsPerBurst         = 3;
        mFireCooldown           = 0;
        mBaseDamage             = 0;
        mMaxDamageRange         = 0;
        mMinDamageRange         = 0;
        mProjectileSpread       = 2;
        mProjectilesReleased    = 1;
        mProjectileVelocity     = 300.0f;
        mFiringSpinUpTime       = 0.0f;
        mFiringSpinDownTime     = 0.0f;
        mCurrentEmitter         = 0;
		mWeaponFireVolume       = 1.0f;
        mCanCollect             = false;
		mCasingScriptName       = "Scripts/ColonyVI/Behaviors/Casing.gs";
        mCanSelect              = true;

        // Default (no sounds)
        mReloadSound        = -1;
        mFireLoopSound      = -1;
        mFireOnceSound      = -1;
        mFireEndSound       = -1;
        mWeaponDrySound     = -1;
        mMagazineLowSound   = -1;
        mToggleModeSound    = -1;
        mSpinUpSound        = -1;
        mSpinDownSound      = -1;
        mWeaponCollectSound = -1;
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
        @mNode          = object;
        mHasFired       = false;
        mTimeSinceFired = 99999999.0f;
        mFireCycleTime  = 0;
        mRoundsFired    = 0;

        // Hide the weapon initially.
        mNode.showNode( false, true );

        // Set weapon to ready if we have any ammo.
        if ( mCurrentMagazineRounds > 0 )
            mState = WeaponState::Ready;
        else
            mState = WeaponState::Empty;

        // Load the casing (ejection) script to keep it resident in memory.
        ResourceManager @ resources = object.getScene().getResourceManager();
        resources.loadScript( mCasingScript, mCasingScriptName, 0, DebugSource() );

        // Create an array with enough room for some tracer billboards.
        mTracers.resize( 20 );
        Scene @ scene = object.getScene();
        for ( uint i = 0; i < mTracers.length(); ++i )
        {
            Tracer @ tracer = Tracer();
            @tracer.node = cast<BillboardNode>(scene.createObjectNode( true, RTID_BillboardObject, false ));
            tracer.node.enableBillboardAlignment( true );
            tracer.node.showNode( false, false );
            tracer.active = false;
            @mTracers[i] = tracer;
        
        } // Next billboard
		
		// Load sound effects for projectile-environment collisions
        mHitEnvironmentSounds.resize( 3 );
        mHitEnvironmentSounds[0] = mAudioManager.loadSound( "Sounds/0009_combat_dagger_hit_metal_c.ogg", true );
        mHitEnvironmentSounds[1] = mAudioManager.loadSound( "Sounds/0009_combat_dagger_hit_metal_e.ogg", true );
        mHitEnvironmentSounds[2] = mAudioManager.loadSound( "Sounds/0012_combat_dagger_hit_metal_b.ogg", true );

        mRicochetSounds.resize( 9 );
        mRicochetSounds[0] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_01.ogg", true );
        mRicochetSounds[1] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_02.ogg", true );
        mRicochetSounds[2] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_03.ogg", true );
        mRicochetSounds[3] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_04.ogg", true );
        mRicochetSounds[4] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_05.ogg", true );
        mRicochetSounds[5] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_07.ogg", true );
        mRicochetSounds[6] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_08.ogg", true );
        mRicochetSounds[7] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_09.ogg", true );
        mRicochetSounds[8] = mAudioManager.loadSound( "Sounds/0017_bullet_ric_pass_10.ogg", true );

        // Default to not playing state
        @mFireLoopSoundChannel = null;
        @mSpinUpSoundChannel   = null;
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release memory
        mHitEnvironmentSounds.resize( 0 );
        mRicochetSounds.resize( 0 );

        // Release our references.
        @mNode                  = null;
        @mMuzzleFlashLight      = null;
        @mOwnerAgent            = null;
        @mAudioManager          = null;
        mMuzzleFlashEmitters.resize(0);
        mTracers.resize(0);
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Process any active tracers.
        updateTracers( elapsedTime );

        // Firing?
        if ( mState == WeaponState::Firing )
        {
            // Has enough time elapsed to account for weapon 'spin up' time?
            if ( mFireTimeTotal > mFiringSpinUpTime )
            {
                // We've spun the weapon up (don't stop it immediately, but allow it to play again later).
                @mSpinUpSoundChannel = null;

                // Increment the firing cycle.
                mFireCycleTime += elapsedTime;

                // If enough time has elapsed, send the shot.
                if ( mFireCycleTime >= mRepeatCycle )
                    fireRound( );

                // Enable muzzle flash emitters if they are not already.
                for ( int i = 0; i < mMuzzleFlashEmitters.length(); ++i )
                {
                    ParticleEmitterNode @ emitter = mMuzzleFlashEmitters[i];
                    if ( @emitter != null )
                    {
                        uint layerCount = emitter.getLayerCount();
                        for ( uint j = 0; j < layerCount; ++j )
                        {
                            if ( !emitter.isLayerEmissionEnabled( j ) )
                                emitter.enableLayerEmission( j, true );
                        
                        } // Next layer
                    
                    } // End if valid
                
                } // Next emitter

                // Start the fire loop sound effect playing if it is not already (due to weapon spin up delay?)
                if ( getFiringMode() != WeaponFiringMode::SingleShot && @mFireLoopSoundChannel == null )
                    @mFireLoopSoundChannel = mAudioManager.playSound( mFireLoopSound, true, true, mWeaponFireVolume, Vector3(0,0,0), mNode );

            } // End if weapon has spun up.
			else
			{
				// Weapon is spinning up (for this implementation, we only play the sound once -- i.e., no loop -- but it could/should be configurable)
                if ( @mSpinUpSoundChannel == null )
                    @mSpinUpSoundChannel = mAudioManager.playSound( mSpinUpSound, true, false, 1.0f, Vector3(0,0,0), mNode );
                
			} // End if spinning up
        
        } // End if firing

        // Handle later update of necessary elements after a shot has been fired.
        if ( mHasFired )
        {
            // 50ms has elapsed?
            if ( mTimeSinceFired >= 0.05f )
            {
                // Hide the muzzle flash
                if ( @mMuzzleFlashLight != null )
                    mMuzzleFlashLight.showNode( false, false );

                // Handle delayed fire completion
                if ( mState != WeaponState::Firing )
                {
                    // Stop emitting muzzle flash billboards
                    for ( int i = 0; i < mMuzzleFlashEmitters.length(); ++i )
                    {
                        ParticleEmitterNode @ emitter = mMuzzleFlashEmitters[i];
                        if ( @emitter != null )
                        {
                            uint layerCount = emitter.getLayerCount();
                            for ( uint j = 0; j < layerCount; ++j )
                            {
                                if ( emitter.isLayerEmissionEnabled( j ) )
                                    emitter.enableLayerEmission( j, false );
                            
                            } // Next layer
                        
                        } // End if valid
                    
                    } // Next emitter

                    // Stop the the looped firing sound if it is playing, and play 
                    // the optional loop end sound effect.
                    if ( @mFireLoopSoundChannel != null )
                    {
                        mAudioManager.stopSound( mFireLoopSoundChannel );
                        @mFireLoopSoundChannel = null;
                        
                        // Play the fire end sound effect.
                        mAudioManager.playSound( mFireEndSound, true, false, mWeaponFireVolume, Vector3(0,0,0), mNode );
                    
                    } // End if fire loop playing
                    
                    // Disable further processing.
                    mHasFired = false;

                } // End if !firing

            } // End if >= 50ms

        } // End if has fired recently

        // Increment time since last fired.
        mTimeSinceFired += elapsedTime;
        mFireTimeTotal += elapsedTime;
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getSceneNode ()
	// Desc : Get the scene node associated with this weapon.
	//-------------------------------------------------------------------------
    ObjectNode @ getSceneNode( )
    {
        return mNode;
    }

    //-------------------------------------------------------------------------
	// Name : canSelect ()
	// Desc : Can this weapon be selected for use by the player?
	//-------------------------------------------------------------------------
    bool canSelect()
    {
        return mCanSelect;
    }

    //-------------------------------------------------------------------------
	// Name : canCollect ()
	// Desc : Weapon can be collected when it is dropped.
	//-------------------------------------------------------------------------
    bool canCollect()
    {
        return mCanCollect;
    }

    //-------------------------------------------------------------------------
	// Name : getWeaponCollectIdentifier ()
	// Desc : What player weapon does this apply to when collected.
	//-------------------------------------------------------------------------
    const String & getWeaponCollectIdentifier()
    {
        return mWeaponCollectIdentifier;
    }

    //-------------------------------------------------------------------------
	// Name : getAmmoCollectMagsMin ()
	// Desc : Minimum number of mags to be added to player's inventory on 
    //        collect.
	//-------------------------------------------------------------------------
    int getAmmoCollectMagsMin( )
    {
        return mAmmoCollectMagsMin;
    }

    //-------------------------------------------------------------------------
	// Name : getAmmoCollectMagsMax ()
	// Desc : Maximum number of mags to be added to player's inventory on 
    //        collect.
	//-------------------------------------------------------------------------
    int getAmmoCollectMagsMax( )
    {
        return mAmmoCollectMagsMax;
    }

    //-------------------------------------------------------------------------
	// Name : attemptWeaponCollect ()
	// Desc : Attempt to collect ammunition from the specified weapon.
	//-------------------------------------------------------------------------
    bool attemptWeaponCollect( Weapon @ weapon )
    {
        // Cannot collect by default.
        return false;
    }

    //-------------------------------------------------------------------------
	// Name : getRoundsPerMagazine ()
	// Desc : Retrieve the total number of rounds that are available in each
    //        magazine appropriate for this weapon.
	//-------------------------------------------------------------------------
    int getRoundsPerMagazine( )
    {
        return mRoundsPerMagazine;
    }

    //-------------------------------------------------------------------------
	// Name : getAvailableRounds ()
	// Desc : Retrieve the total rounds available for this weapon, including
    //        those in the current magazine.
	//-------------------------------------------------------------------------
    int getAvailableRounds( )
    {
        return mTotalRounds;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentMagazineRounds ()
	// Desc : Retrieve the number of rounds available in the current magazine
    //        of this weapon.
	//-------------------------------------------------------------------------
    int getCurrentMagazineRounds( )
    {
        return mCurrentMagazineRounds;
    }

    //-------------------------------------------------------------------------
	// Name : setOwnerAgent ()
	// Desc : Set the agent that owns this weapon.
	//-------------------------------------------------------------------------
    void setOwnerAgent( Agent @ agent )
    {
        @mOwnerAgent = agent;
    }
    
    //-------------------------------------------------------------------------
	// Name : getOwnerAgent ()
	// Desc : Retrieve the agent that owns this weapon.
	//-------------------------------------------------------------------------
    Agent @ getOwnerAgent( )
    {
        return @mOwnerAgent;
    }

    //-------------------------------------------------------------------------
	// Name : getState ()
	// Desc : Retrieve the current state of the weapon.
	//-------------------------------------------------------------------------
    WeaponState getState( )
    {
        // In full burst mode, how much time is there remaining for the weapon to spin down?
        if ( getFiringMode() == WeaponFiringMode::FullBurst )
        {
            if ( mState != WeaponState::Firing )
            {
                if ( mTimeSinceFired < mFiringSpinDownTime )
                    return WeaponState::Firing;
            
            } // End if !Firing

        } // End if full burst

        // Weapon still in firing cooldown?
        if ( mState == WeaponState::Ready )
        {
            if ( mTimeSinceFired < mFireCooldown )
                return WeaponState::Cooldown;
        
        } // End if 'Ready'

        return mState;
    }

    //-------------------------------------------------------------------------
	// Name : getFiringMode ()
	// Desc : Retrieve the current firing mode of the weapon.
	//-------------------------------------------------------------------------
    WeaponFiringMode getFiringMode( )
    {
        return mFiringMode;
    }

    //-------------------------------------------------------------------------
	// Name : getClass ()
	// Desc : Retrieve the class of this weapon (i.e. Pistol, Rifle, etc.)
	//-------------------------------------------------------------------------
    String getClass( )
    {
        return "";
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
	// Name : getTimeSinceFired ()
	// Desc : Retrieve the amount of time that has elapsed since this weapon
    //        was last fired.
	//-------------------------------------------------------------------------
    float getTimeSinceFired( )
    {
        return mTimeSinceFired;
    }

    //-------------------------------------------------------------------------
	// Name : select ()
	// Desc : Select and activate this weapon.
	//-------------------------------------------------------------------------
    void select( )
    {
        // Show the weapon.
        mNode.showNode( true, true );

        // Make sure that the muzzle flash light source remains hidden
        // even if the weapon itself is shown.
        if ( @mMuzzleFlashLight != null )
            mMuzzleFlashLight.showNode( false, false );

        // Enable updates for this weapon.
        mNode.setUpdateRate( UpdateRate::Always );
    }

    //-------------------------------------------------------------------------
	// Name : deselect ()
	// Desc : Shut down and deselect this weapon.
	//-------------------------------------------------------------------------
    void deselect( )
    {
        // Hide the weapon (and all of its children -- including the muzzle flash).
        mNode.showNode( false, true );

        // Disable updates for this weapon.
        mNode.setUpdateRate( UpdateRate::Never );
    }

    //-------------------------------------------------------------------------
	// Name : setAimPoints ()
	// Desc : Set the source and destination points for projectiles.
	//-------------------------------------------------------------------------
    void setAimPoints( const Vector3 & from, const Vector3 & to )
    {
        mAimFrom = from;
        mAimTo   = to;
    }

    //-------------------------------------------------------------------------
	// Name : getAimPoints ()
	// Desc : Retrieve the source and destination points for projectiles.
	//-------------------------------------------------------------------------
    void getAimPoints( Vector3 & from, Vector3 & to )
    {
        from = mAimFrom;
        to = mAimTo;
    }

    //-------------------------------------------------------------------------
	// Name : beginFiring ()
	// Desc : Call in order to start firing with this weapon.
	//-------------------------------------------------------------------------
    WeaponFiringResult beginFiring( )
    {
        // Do we have any rounds in the current magazine?
        if ( mCurrentMagazineRounds > 0 )
        {
            // We should start firing. Make sure that the first shot
            // is fired immediately by setting the cycle time to
            // the minimum repeat cycle.
            mFireTimeTotal = 0;
            mFireCycleTime = mRepeatCycle;
            mTimeSinceFired = mRepeatCycle;
            mHasFired = false;
            mRoundsFired = 0;
            
            // Begin emitting flash billboards if there is no spin up time.
            if ( mFiringSpinUpTime <= 0 )
            {
                for ( int i = 0; i < mMuzzleFlashEmitters.length(); ++i )
                {
                    ParticleEmitterNode @ emitter = mMuzzleFlashEmitters[i];
                    if ( @emitter != null )
                    {
                        uint layerCount = emitter.getLayerCount();
                        for ( uint j = 0; j < layerCount; ++j )
                        {
                            if ( !emitter.isLayerEmissionEnabled( j ) )
                                emitter.enableLayerEmission( j, true );
                        
                        } // Next layer
                    
                    } // End if valid
                
                } // Next emitter
            
            } // End if no spin up time

            // If we're in single shot mode, play the fire once sound.
            // Otherwise play the firing loop sound effect if there is no spin up time.
            if ( mFiringMode == WeaponFiringMode::SingleShot )
                mAudioManager.playSound( mFireOnceSound, true, false, mWeaponFireVolume, Vector3(0,0,0), mNode );
            else if ( mFiringSpinUpTime <= 0 && @mFireLoopSoundChannel == null )
                @mFireLoopSoundChannel = mAudioManager.playSound( mFireLoopSound, true, true, mWeaponFireVolume, Vector3(0,0,0), mNode );

            // Switch to firing state.
            mState = WeaponState::Firing;

            // Fire a round immediately if there is no spin up time.
            if ( mFiringSpinUpTime <= 0 )
                fireRound( );

            return WeaponFiringResult::Success;
        
        } // End if ammo available

        // No ammo in the current magazine.
        return WeaponFiringResult::MagazineEmpty;
    }

    //-------------------------------------------------------------------------
	// Name : endFiring ()
	// Desc : Call in order to finish firing with this weapon.
	//-------------------------------------------------------------------------
    void endFiring( )
    {
        // Do nothing if we weren't firing.
        if ( mState != WeaponState::Firing )
            return;

        // Stop the spin up sound and allow it to play again in the future.
        mAudioManager.stopSound( mSpinUpSoundChannel );
        @mSpinUpSoundChannel = null;

		// If a spin down sound is available, play it (if not already)
        mAudioManager.playSound( mSpinDownSound, true, false, 1.0f, Vector3(0,0,0), mNode );
			
        // Reset weapon state
        if ( mCurrentMagazineRounds > 0 )
            mState = WeaponState::Ready;
        else
            mState = WeaponState::Empty;
    }

    //-------------------------------------------------------------------------
	// Name : reload ()
	// Desc : Begin the reload process for this weapon.
	//-------------------------------------------------------------------------
    WeaponReloadResult reload( )
    {
        // Must be in a valid state
        if ( mState != WeaponState::Ready && mState != WeaponState::Empty )
            return WeaponReloadResult::InvalidState;

        // Any ammo left at all?
        if ( mTotalRounds <= 0 )
        {
            // Play the dry sound effect.
            mAudioManager.playSound( mWeaponDrySound, true, false, 1.0f, mNode.getPosition(), null );
            
            // Out of ammo
            return WeaponReloadResult::NoAmmo;
        
        } // End if no ammo

        // Fill magazine.
        mCurrentMagazineRounds = min( mRoundsPerMagazine, mTotalRounds );

        // Weapon is now ready if it was previously empty.
        mState = WeaponState::Ready;

        // Play the reload sound effect.
        mAudioManager.playSound( mReloadSound, true, false, 1.0f, Vector3(0,0,0), mNode );

        // Weapon successfully reloaded.
        return WeaponReloadResult::Success;
    }

    //-------------------------------------------------------------------------
	// Name : cycleFiringMode ()
	// Desc : Switch between the available firing modes for this weapon.
	//-------------------------------------------------------------------------
    WeaponFiringMode cycleFiringMode( )
    {
        // Play the toggle sound effect.
        mAudioManager.playSound( mToggleModeSound, true, false, 1.0f, mNode.getPosition(), null );
        return mFiringMode;
    }

    //-------------------------------------------------------------------------
	// Name : computeDamage ()
	// Desc : Compute the damage that would be inflicted by this weapon at the
    //        given range.
	//-------------------------------------------------------------------------
    float computeDamage( float distance )
    {
        float baseProjectileDamage = mBaseDamage / float(mProjectilesReleased);
        
        // ToDo: Add random variation?
        if ( mMaxDamageRange <= 0 )
            return baseProjectileDamage;

        // No falloff?
        if ( (mMaxDamageRange - mMinDamageRange) <= 0 )
            return (distance < mMaxDamageRange) ? baseProjectileDamage : 0;

        // Compute damage over distance
        float damage = (distance - mMinDamageRange) / (mMaxDamageRange - mMinDamageRange);
        damage = max( 0.0f, damage );
        damage = min( 1.0f, damage );
        return (1.0f - damage) * baseProjectileDamage;
    }

    //-------------------------------------------------------------------------
	// Name : emitTracer ()
	// Desc : Event that is triggered when a tracer should be released.
	//-------------------------------------------------------------------------
    void emitTracer( bool hit, const Vector3 & from, const Vector3 & to, uint hitEffectRefId )
    {
        // Find an available tracer.
        int availableTracer = -1;
        for ( uint i = 0; i < mTracers.length(); ++i )
        {
            // Inactive?
            if ( !mTracers[i].active )
            {
                availableTracer = int(i);
                break;
            
            } // End if inactive

        } // Next tracer

        // If a tracer is available, set it up and let it fly.
        if ( availableTracer >= 0 )
        {
            Scene @ scene = mNode.getScene();
            Tracer @ tracer = mTracers[availableTracer];
            tracer.active   = true;
            tracer.hit      = hit;
            tracer.from     = from;
            tracer.to       = to;
            tracer.hitEffect = (hitEffectRefId != 0) ? hitEffectRefId : 0x5ED;

            // Deviate the tracer direction by some random amount
            Transform tracerTransform;
            tracerTransform.lookAt( tracer.from, tracer.to );

            // Compute final velocity.
            tracer.direction = tracerTransform.normalizedZAxis();
            tracer.velocity = tracer.direction * mProjectileVelocity; // m/s

            // Set the renderable tracer node's details.
            float length = vec3Length(tracer.velocity)*0.04f;
            tracer.node.setHDRScale( 500.0f );
            tracer.node.setSize( SizeF(0.05f, length) );
            tracer.node.setPosition( tracer.from + (tracer.direction * (length*0.4f)) );
            tracer.node.setOrientation( tracerTransform.orientation() );
            tracer.node.showNode( true, false );

            // Compute lifetime
            tracer.distanceToTarget = vec3Length( tracer.to - tracer.from ) - (length * 0.4f);

        } // End if found tracer
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
        // Display the muzzle flash light
        if ( @mMuzzleFlashLight != null )
            mMuzzleFlashLight.showNode( true, false );

        // Reset time to next cycle.
        mFireCycleTime -= mRepeatCycle;

        // Reset the time since fired so that we know when to hide the light
        // and / or stop sound effect processing.
        mTimeSinceFired = 0;
        mHasFired = true;

        // Decrement round count.
        mCurrentMagazineRounds--;
        mTotalRounds--;

        // Increment rounds fired in this run (burst tracking).
        mRoundsFired++;

        // Play the "almost out" sound for the last 20% of the magazine.
        int lowIndicator = (mRoundsPerMagazine / 5);
        if ( lowIndicator != 0 && mCurrentMagazineRounds <= lowIndicator )
        {
            float level = float((lowIndicator - mCurrentMagazineRounds) / float(lowIndicator));
            mAudioManager.playSound( mMagazineLowSound, true, false, level * 0.5f, mNode.getPosition(), null );
        
        } // End if last few rounds

        // If we're out of rounds in the current magazine, end firing.
        // Alternatively, in single / burst shot mode we may need to
        // end after firing all of our rounds.
        if ( mCurrentMagazineRounds <= 0 )
        {
            endFiring( );
        
        } // End if magazine empty
        else
        {
            // Stop firing if we've completed our run.
            if ( mFiringMode == WeaponFiringMode::SingleShot )
                endFiring();
            else if ( (mFiringMode == WeaponFiringMode::Burst || mFiringMode == WeaponFiringMode::FullBurst) && mRoundsFired >= mRoundsPerBurst )
                endFiring();

        } // End if !empty

        // If the weapon has an ejection port, spawn a casing and fire along +X
        Scene @ scene = mNode.getScene();
        if ( @mEjectionPortSpawn != null )
        {
            ObjectNode @ casing = scene.loadObjectNode( 0x5F6, CloneMethod::ObjectInstance, false );
            casing.setWorldTransform( mEjectionPortSpawn.getWorldTransform() );
            casing.setUpdateRate( UpdateRate::FPS30 );

            // Attach the simple casing behavior script.
            ObjectBehavior @ behavior = ObjectBehavior( );
            behavior.initialize( scene.getResourceManager(), mCasingScriptName, "" );
            casing.addBehavior( behavior );
        
        } // End if has ejection

        // Release required number of projectiles.
        for ( int i = 0; i < mProjectilesReleased; ++i )
        {
            mAimFrom = mMuzzleFlashEmitters[mCurrentEmitter].getPosition();

            // Deviate the projectile direction by some random amount
            Transform projectileTransform;
            projectileTransform.lookAt( mAimFrom, mAimTo );
            projectileTransform.rotateLocal( 0, 0, randomFloat( 0, CGE_PI) );
            projectileTransform.rotateLocal( randomFloat(-CGEToRadian(mProjectileSpread*0.5f), CGEToRadian(mProjectileSpread*0.5f)), 0, 0 );

            // Ask the scene to retrieve the closest intersected object node.
            SceneCollisionContact contact;
            ObjectNode @ pickedNode = null;
            Vector3 intersection, rayOrigin = mAimFrom, rayDir = projectileTransform.normalizedZAxis();
            bool result = scene.rayCastClosest( rayOrigin, rayDir * 10000.0f, contact );
            if ( result )
            {
                @pickedNode = contact.node;
                intersection = rayOrigin + (rayDir * 10000.0f * contact.intersectParam);
                
            } // End if scene cast
            
            // Take action based on the selected object node
            uint hitEffectRefId = 0;
            if ( @pickedNode != null )
            {
                hitEffectRefId = uint(pickedNode.getCustomProperty( "projectile_hit_fx", 0 ));

                // Push the object if it has a physics body
                PhysicsBody @ body = pickedNode.getPhysicsBody();
                if ( @body != null && body.getMass() > CGE_EPSILON )
                    body.applyImpulse( rayDir * ((100.0f / float(mProjectilesReleased)) / body.getMass()), intersection );

                // Is the object we hit an agent?
                Agent @ agent = null;
                if ( pickedNode.getBehaviorCount() > 0 )
                    @agent = cast<Agent>(pickedNode.getScriptedBehavior(0));

                // If it wasn't, how about its owner group?
                if ( @agent == null )
                {
                    GroupNode @ group = pickedNode.getOwnerGroup();
                    if ( @group != null && group.getBehaviorCount() > 0 )
                    {
                        @agent = cast<Agent>(group.getScriptedBehavior(0));
                        if ( @agent != null )
                            hitEffectRefId = uint(group.getCustomProperty( "projectile_hit_fx", 0 ));
                    
                    } // End if has group with script
                 
                } // End if try group

                // Trigger projectile hit event.
                if ( @agent != null )
                    agent.onProjectileHit( this, pickedNode, rayOrigin, intersection );
                else
                {
                    // Destructable light?
                    if ( pickedNode.getBehaviorCount() > 0 )
                    {
                        DestructibleLight @ light = cast<DestructibleLight>(pickedNode.getScriptedBehavior(0));
                        if ( @light != null )
                            light.deactivate();
                    
						// Play glass break/pop sound (TODO)
					
                    } // End if has behaviors
                
					// Play hit sound (metal for this demo)
                    float level = randomFloat( 0.3, 0.5 );
                    mAudioManager.playSound( mHitEnvironmentSounds[ randomInt( 0, mHitEnvironmentSounds.length() - 1 ) ], true, false, level, intersection, null );
				
					// Play an optional ricochet sound every now and again
					if ( randomFloat( 0.0f, 1.0f ) < 0.25f )
					{
                        level = randomFloat( 0.7, 0.9 );
                        mAudioManager.playSound( mRicochetSounds[ randomInt( 0, mRicochetSounds.length() - 1 ) ], true, false, level, intersection, null );
						
					} // End ricochet sound
                
                } // End if not agent

            } // End if intersect

            // Emit a tracer (round robin from each emitter).
            bool hit = (@pickedNode!=null);
            emitTracer( hit, mMuzzleFlashEmitters[mCurrentEmitter].getPosition(), (hit) ? intersection : rayOrigin + rayDir * 200, hitEffectRefId );
            mCurrentEmitter++;
            if ( mCurrentEmitter >= mMuzzleFlashEmitters.length() )
                mCurrentEmitter = 0;

        } // Next projectile
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : updateTracers () (Private)
	// Desc : Performs the update process for projectile tracers.
	//-------------------------------------------------------------------------
    private void updateTracers( float timeDelta )
    {
        Scene @ scene = mNode.getScene();
        for ( uint i = 0; i < mTracers.length(); ++i )
        {
            Tracer @ tracer = mTracers[i];

            // Active?
            if ( !tracer.active )
                continue;

            // Move the tracer
            tracer.node.move( tracer.velocity * timeDelta );

            // Kill the tracer if it's moved far enough (destination is behind us).
            float distanceToTarget = vec3Dot( tracer.to - tracer.node.getPosition(), tracer.direction );
            if ( distanceToTarget <= 0 )
            {
                // Hide the tracer
                tracer.active = false;
                tracer.node.showNode( false, false );

                // Spawn a particle emitter if we hit.
                if ( tracer.hit )
                {
                    Vector3 right, up = Vector3(0,1,0), look = -tracer.direction;
                    vec3Cross( right, look, up );
                    vec3Cross( up, right, look );
                    ObjectNode @ sparks = scene.loadObjectNode( tracer.hitEffect, CloneMethod::ObjectInstance, false );
                    sparks.setPosition( tracer.to );
                    sparks.setOrientation( right, up, look );
                
                } // End if hit
            
            } // End if done.
            else
            {
                // Update alpha.
                float alpha = (distanceToTarget / tracer.distanceToTarget);
                tracer.node.setColor( ColorValue( 1.0f, 1.0f, 1.0f, alpha ) );

            } // End if !done

        } // Next tracer
    }

} // End Class Weapon