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
#include_once "DestructibleLight.gs"

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
    private ObjectNode@             mNode;                  // The node to which we are attached.
    private Agent@                  mOwnerAgent;            // The agent that owns this weapon.
    private ObjectNode@             mEjectionPortSpawn;     // The point at which casings will be ejected (if any).
    private ParticleEmitterNode@    mMuzzleFlashEmitter;    // Particle emitter representing muzzle flash.
    private ObjectNode@             mMuzzleFlashLight;      // Light source used when firing.
    private array<Tracer@>          mTracers;               // List of available tracers.

    // Sound effects.
    private AudioBufferHandle       mReloadSound;           // Sound played during reloading.
    private AudioBufferHandle       mFireLoopSound;         // Looping fire sound.
    private AudioBufferHandle       mFireOnceSound;         // Single shot fire sound.
    private AudioBufferHandle       mFireEndSound;          // Sound to play when firing ceases (slight echo).
    private AudioBufferHandle       mWeaponDrySound;        // Sound to play when unable to reload.
    private AudioBufferHandle       mMagazineLowSound;      // Sound that plays over the top of the fire loop when magazine almost empty.
    private AudioBufferHandle       mToggleModeSound;       // Sound that plays when the firing mode is toggled.

    // Weapon descriptor
    private float                   mRepeatCycle;           // Amount of time (in seconds) between each firing.
    private int                     mRoundsPerMagazine;     // Number of rounds in a magazine (fixed).
    private int                     mRoundsPerBurst;        // In burst fire mode, how many rounds should be fired?
    private float                   mBaseDamage;            // Base damage amount for this weapon.
    private float                   mMaxDamageRange;        // At this range, no damage will be applied.
    private float                   mMinDamageRange;        // At this range, full damage will be applied.
    private float                   mFireCooldown;          // After firing, this describes the amount of time until the gun is ready to fire again.
    private int                     mProjectilesReleased;   // How many projectiles are released for each round fired (equal distribution of base damage)?
    private float                   mProjectileSpread;      // Angle in degrees that specifies the cone of the overall spread of released projectiles.
    private float                   mProjectileVelocity;    // Velocity of each projectile as it leaves the barrel in meters per second.

    // Current weapon properties
    private WeaponState             mState;                 // The current state of the weapon (firing, etc.)
    private WeaponFiringMode        mFiringMode;            // The current firing mode of the weapon (burst, full auto, etc.)
    private int                     mCurrentMagazineRounds; // Number of rounds in the current magazine.
    private int                     mTotalRounds;           // Total available rounds (including those in current magazine).
    private float                   mFireCycleTime;         // Cycling timer used to determine when to fire.
    private int                     mRoundsFired;           // Number of rounds fired since firing began.
    private Vector3                 mAimFrom;               // The source point for weapon projectiles.
    private Vector3                 mAimTo;                 // The destination point for weapon projectiles.
    
    // Time delay handling
    private float                   mTimeSinceFired;        // Amount of time since last firing.
    private bool                    mHasFired;              // Should we still be processing things after firing? (muzzle flash, sound)

    // Miscellaneous
    private ScriptHandle            mCasingScript;          // Used simply to keep the casing script 'warm'.
    
	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : Weapon () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	Weapon( )
    {
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
        resources.loadScript( mCasingScript, "Scripts/ColonyVI/Behaviors/Casing.gs", 0, DebugSource() );

        // Create an array with enough room for some tracer billboards.
        mTracers.resize( 10 );
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

	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        // Release our references.
        @mNode                  = null;
        @mMuzzleFlashEmitter    = null;
        @mMuzzleFlashLight      = null;
        @mOwnerAgent            = null;
        mTracers.resize(0);

        // Unload sounds.
        mReloadSound.close();
        mFireLoopSound.close();
        mFireEndSound.close();
        mWeaponDrySound.close();
        mMagazineLowSound.close();
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
            // Increment the firing cycle.
            mFireCycleTime += elapsedTime;

            // If enough time has elapsed, send the shot.
            if ( mFireCycleTime >= mRepeatCycle )
                fireRound( );

            // Update the 3D position of the firing loop sound if playing.
            AudioBuffer @ effect = mFireLoopSound.getResource(true);
            if ( @effect != null && effect.isLoaded() )
                effect.set3DSoundPosition( mNode.getPosition() );
        
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
                    if ( @mMuzzleFlashEmitter != null )
                        mMuzzleFlashEmitter.enableLayerEmission( 0, false );

                    // Stop the the looped firing sound if it is playing, and play 
                    // the optional loop end sound effect.
                    AudioBuffer @ loopEffect = mFireLoopSound.getResource(true);
                    if ( @loopEffect != null && loopEffect.isPlaying() )
                    {
                        // Play the fire end sound effect.
                        AudioBuffer @ effect = mFireEndSound.getResource(true);
                        if ( @effect != null && effect.isLoaded() )
                        {
                            //effect.setVolume( 0.9f );
                            effect.set3DSoundPosition( mNode.getPosition() );
                            effect.setBufferPosition( 0 );
                            effect.play( false ); // Once
                        
                        } // End if loaded

                        // Stop the firing effect loop
                        loopEffect.stop();

                    } // End if loop playing

                    // Disable further processing.
                    mHasFired = false;

                } // End if !firing

            } // End if >= 50ms

        } // End if has fired recently

        // Increment time since last fired.
        mTimeSinceFired += elapsedTime;
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
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
            mFireCycleTime = mRepeatCycle;
            mTimeSinceFired = mRepeatCycle;
            mHasFired = false;
            mRoundsFired = 0;
            
            // Begin emitting flash billboards.
            if ( @mMuzzleFlashEmitter != null )
                mMuzzleFlashEmitter.enableLayerEmission( 0, true );

            // If we're in single shot mode, play the fire once sound
            bool loop = false;
            AudioBuffer @ effect;
            if ( mFiringMode == WeaponFiringMode::SingleShot )
                @effect = mFireOnceSound.getResource(true);

            // Otherwise play the firing loop sound effect.
            if ( @effect == null )
            {
                loop = true;
                @effect = mFireLoopSound.getResource(true);
            } // End if no single shot
            if ( @effect != null && effect.isLoaded() )
            {
                //effect.setVolume( 0.9f );
                effect.set3DSoundPosition( mNode.getPosition() );
                effect.setBufferPosition( 0 );
                effect.play( loop );
            
            } // End if loaded

            // Switch to firing state.
            mState = WeaponState::Firing;
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
            AudioBuffer @ effect = mWeaponDrySound.getResource(true);
            if ( @effect != null && effect.isLoaded() )
            {
                //effect.setVolume( 0.8f );
                effect.set3DSoundPosition( mNode.getPosition() );
                effect.setBufferPosition( 0 );
                effect.play( false ); // Once
            
            } // End if loaded
            return WeaponReloadResult::NoAmmo;
        
        } // End if no ammo

        // Fill magazine.
        mCurrentMagazineRounds = min( mRoundsPerMagazine, mTotalRounds );

        // Weapon is now ready if it was previously empty.
        mState = WeaponState::Ready;

        // Play the reload sound effect.
        AudioBuffer @ effect = mReloadSound.getResource(true);
        if ( @effect != null && effect.isLoaded() )
        {
            //effect.setVolume( 0.85f );
            effect.set3DSoundPosition( mNode.getPosition() );
            effect.setBufferPosition( 0 );
            effect.play( false ); // Once
        
        } // End if loaded

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
        AudioBuffer @ effect = mToggleModeSound.getResource(true);
        if ( @effect != null && effect.isLoaded() )
        {
            //effect.setVolume( 0.8f );
            effect.set3DSoundPosition( mNode.getPosition() );
            effect.setBufferPosition( 0 );
            effect.play( false ); // Once
        
        } // End if loaded
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

    //-------------------------------------------------------------------------
	// Name : fireRound () (Private)
	// Desc : Event that is triggered when a round should be fired. Call this
    //        last after additional processing in derived class. 'endfiring()'
    //        will automatically be called when there are no further rounds
    //        available.
	//-------------------------------------------------------------------------
    private void fireRound( )
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
            AudioBuffer @ effect = mMagazineLowSound.getResource(true);
            if ( @effect != null && effect.isLoaded() )
            {
                float level = float((lowIndicator - mCurrentMagazineRounds) / float(lowIndicator));
                effect.setVolume( level * 0.5f );
                effect.set3DSoundPosition( mNode.getPosition() );
                effect.setBufferPosition( 0 );
                effect.play( false ); // Once
            
            } // End if loaded

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
            else if ( mFiringMode == WeaponFiringMode::Burst && mRoundsFired >= mRoundsPerBurst )
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
            behavior.initialize( scene.getResourceManager(), "Scripts/ColonyVI/Behaviors/Casing.gs", "" );
            casing.addBehavior( behavior );
        
        } // End if has ejection

        // Release required number of projectiles.
        for ( int i = 0; i < mProjectilesReleased; ++i )
        {
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
                hitEffectRefId = uint(pickedNode.getCustomProperties().getProperty( "projectile_hit_fx", uint(0) ));

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
                            hitEffectRefId = uint(group.getCustomProperties().getProperty( "projectile_hit_fx", uint(0) ));
                    
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
                    
                    } // End if has behaviors
                
                } // End if not agent

            } // End if intersect

            // Emit a tracer.
            bool hit = (@pickedNode!=null);
            emitTracer( hit, mMuzzleFlashEmitter.getPosition(), (hit) ? intersection : rayOrigin + rayDir * 200, hitEffectRefId );

        } // Next projectile
    }

} // End Class Weapon