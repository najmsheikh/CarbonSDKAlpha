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
// Name : FirstPersonActor.gs                                                //
//                                                                           //
// Desc : Behavior script associated with the actor designed to represent    //
//        the first person version of the player character.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
shared enum WeaponState
{
    Lowered,    // Weapon is lowered
    Lowering,   // Weapon is in the process of being lowered
    Raising,    // Weapon is in the process of being raised
    Idle,       // Player is idling
    Firing,     // Weapon is being fired
    Reloading   // Weapon is being reloaded
};

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : FirstPersonActor (Class)
// Desc : Behavior script associated with the actor designed to represent the 
//        first person version of the player character.
//-----------------------------------------------------------------------------
shared class FirstPersonActor : IScriptedObjectBehavior
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private ActorNode@              mActor;                 // The actor to which we are attached.
    private ParticleEmitterNode@    mMuzzleFlashEmitter;    // Particle emitter representing muzzle flash.
    private ObjectNode@             mMuzzleFlashLight;      // Light source used when firing.
    private ObjectNode@             mDummyWeaponMag;        // Dummy weapon magazine in the player's hand (hidden unless reloading).
    private ObjectNode@             mActualWeaponMag;       // Actual weapon magazine in the gun (hidden during reloading).
    private bool                    mMagHidden;             // Magazine is in hidden state?
    private double                  mFireCycle;             // Timings for firing.
    private float                   mTimeToFire;            // Time until we send another bullet.
    private WeaponState             mState;                 // Current state of the weapon (i.e. idling, firing, etc.)
    private AudioBufferHandle       mReloadSound;
    private AudioBufferHandle       mFireLoopSound;
    private AudioBufferHandle       mFireEndSound;

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FirstPersonActor () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FirstPersonActor( )
    {
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
        mState      = WeaponState::Lowered;
        mMagHidden  = false;

        // Cache references to required objects.
        @mActor              = cast<ActorNode>(object);
        @mMuzzleFlashEmitter = cast<ParticleEmitterNode>(mActor.findChild( "Muzzle Flash" ));
        @mMuzzleFlashLight   = mActor.findChild( "Muzzle Flash Light" );
        @mDummyWeaponMag     = mActor.findChild( "Dummy Magazine" );
        @mActualWeaponMag    = mActor.findChild( "Weapon Magazine" );

        // Setup the actor's animation controller.
        mActor.setTrackFadeTimes( 0.15f, 0.15f );

        // Auto raise weapon from lowered state.
        mState = WeaponState::Raising;
        mActor.playAnimationSet( "Action", "Raising", AnimationPlaybackMode::PlayOnce );

        // Load sound effects.
        ResourceManager @ resources = mActor.getScene().getResourceManager();
        resources.loadAudioBuffer( mReloadSound,   "Sounds/GunReload.wav", AudioBufferFlags::Complex, 0, DebugSource() );
        resources.loadAudioBuffer( mFireLoopSound, "Sounds/Carbine Fire.ogg", AudioBufferFlags::Complex, 0, DebugSource() );
        resources.loadAudioBuffer( mFireEndSound,  "Sounds/Carbine Fire End.ogg", AudioBufferFlags::Complex, 0, DebugSource() );

        // Initially hide the dummy weapon magazine.
        if ( @mDummyWeaponMag != null )
            mDummyWeaponMag.showNode( false, true );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mActor                 = null;
        @mMuzzleFlashEmitter    = null;
        @mMuzzleFlashLight      = null;
        @mDummyWeaponMag        = null;
        @mActualWeaponMag       = null;
    }

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Do nothing if we are not attached.
        if ( @mActor == null )
            return;

        AnimationController @ controller = mActor.getAnimationController();
        InputDriver @ input = getAppInputDriver();
        bool directMode = (input.getMouseMode() == MouseHandlerMode::Direct);

        // What is the current weapon state?
        switch ( mState )
        {
            case WeaponState::Lowered:

                // Start raising the weapon when LMB is pressed
                if ( input.isMouseButtonPressed( MouseButtons::Left ) && directMode )
                {
                    mState = WeaponState::Raising;
                    mActor.playAnimationSet( "Action", "Raising", AnimationPlaybackMode::PlayOnce );

                } // End if LMB
                break;

            case WeaponState::Raising:

                // Switch to idle when raising is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    // Switch to idle state when we've finished raising
                    mState = WeaponState::Idle;
                    mActor.playAnimationSet( "Primary", "Idle", AnimationPlaybackMode::Loop );
                
                } // End if complete
                break;

            case WeaponState::Firing:
                if ( !input.isMouseButtonPressed( MouseButtons::Left ) || !directMode)
                {
                    // Switch back to idle state
                    mState = WeaponState::Idle;
                    mActor.playAnimationSet( "Primary", "Idle", AnimationPlaybackMode::Loop );

                    // Play the fire end sound effect.
                    AudioBuffer @ effect = mFireEndSound.getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.setVolume( 0.9f );
                        effect.setBufferPosition( 0 );
                        effect.play( false ); // Once
                    
                    } // End if loaded

                    // Stop the firing effect loop
                    @effect = mFireLoopSound.getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                        effect.stop();
                    
                    // Hide muzzle flash billboard
                    if ( @mMuzzleFlashEmitter != null )
                        mMuzzleFlashEmitter.enableLayerEmission( 0, false );

                    // Hide muzzle flash light
                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( false, false );

                } // End if !LMB
                else
                {
                    mFireCycle += elapsedTime;
                    mTimeToFire -= elapsedTime;
                    if ( mTimeToFire <= 0.0f )
                    {
                        // Send a bullet
                        fireShot();

                        // Reset
                        mTimeToFire += 0.1f;
                    }

                    // Cycle muzzle flash light
                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( (mFireCycle % 0.1f) <= 0.05f, false );
                
                } // End if LMB
                break;

            case WeaponState::Idle:

                // If user presses 'R', switch to reload or fire with LMB.
                if ( input.isKeyPressed( Keys::R, true ) && directMode )
                {
                    mState = WeaponState::Reloading;
                    mActor.stopAnimationTrack( "Primary" );
                    mActor.playAnimationSet( "Action", "Reload", AnimationPlaybackMode::PlayOnce, 1, 0.1f );

                    // Play the reload sound effect.
                    AudioBuffer @ effect = mReloadSound.getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.setVolume( 0.85f );
                        effect.setBufferPosition( 0 );
                        effect.play( false ); // Once
                    
                    } // End if loaded

                } // End if 'R'
                else if ( input.isMouseButtonPressed( MouseButtons::Left ) && directMode )
                {
                    // Switch to firing state.
                    mState = WeaponState::Firing;
                    mFireCycle = 0;
                    mTimeToFire = 0;
                    mActor.playAnimationSet( "Primary", "Fire Cycle", AnimationPlaybackMode::Loop );

                    // Play the firing sound effect.
                    AudioBuffer @ effect = mFireLoopSound.getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.setVolume( 0.9f );
                        effect.setBufferPosition( 0 );
                        effect.play( true ); // Loop
                    
                    } // End if loaded
                    
                    // Show muzzle flash emitter.
                    if ( @mMuzzleFlashEmitter != null )
                        mMuzzleFlashEmitter.enableLayerEmission( 0, true );

                    // Show muzzle flash light source.
                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( true, false );

                } // End if LMB
                break;

            case WeaponState::Reloading:

                // Switch back to idle when reload is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    // Switch back to idle.
                    mState = WeaponState::Idle;
                    mActor.playAnimationSet( "Primary", "Idle", AnimationPlaybackMode::Loop );
                
                } // End if complete
                else
                {
                    int track = mActor.playAnimationSet( "Action", "Reload", AnimationPlaybackMode::PlayOnce );
                    double pos = controller.getTrackPosition( track );

                    if ( !mMagHidden )
                    {
                        // Hide actual magazine (in weapon) and show dummy magazine (in hand) on frame 21.
                        if ( pos >= (27.0f / 30.0f) && pos < (61.0f / 30.0f) )
                        {
                            if ( @mActualWeaponMag != null )
                                mActualWeaponMag.showNode( false, true );
                            if ( @mDummyWeaponMag != null )
                                mDummyWeaponMag.showNode( true, true );
                            mMagHidden = true;
                        
                        } // End if >= 21
                    
                    } // End if !hidden
                    
                    if ( mMagHidden )
                    {
                        // Restore actual magazine (in weapon) on frame 65.
                        if ( pos >= (61.0f / 30.0f) )
                        {
                            if ( @mActualWeaponMag != null )
                                mActualWeaponMag.showNode( true, true );
                            if ( @mDummyWeaponMag != null )
                                mDummyWeaponMag.showNode( false, true );
                            mMagHidden = false;
                        
                        } // End if >= 65

                    } // End if hidden

                } // End if reloading
                break;

        } // End switch state
	}

    //-------------------------------------------------------------------------
	// Name : fireShot () (Private)
	// Desc : Fire a shot into the scene and react (temporary).
	//-------------------------------------------------------------------------
    private void fireShot( )
    {
        RenderDriver @ driver = getAppRenderDriver();
        Viewport viewport = driver.getViewport();
        Size viewportSize = Size(viewport.width, viewport.height);

        // Convert the location at the center of the screen into a ray for picking
        Vector3 rayOrigin, rayDir;
        CameraNode @ camera = cast<CameraNode>(mActor.getParent());
        if ( @camera == null || !camera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
            return;

        // Ask the scene to retrieve the closest intersected object node.
        Vector3 intersection;
        Scene @ scene = mActor.getScene();
        ObjectNode @ pickedNode = scene.pickClosestNode( viewportSize, rayOrigin, rayDir, intersection );

        // Take action based on the selected object node
        if ( @pickedNode != null )
        {
            // Push the object if it has a physics body
            PhysicsBody @ body = pickedNode.getPhysicsBody();
            if ( @body != null )
                body.applyImpulse( rayDir * 6.0f, intersection );

        } // End if intersect
    }

} // End Class FirstPersonActor