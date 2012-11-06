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
    private WeaponState             mState;                 // Current state of the weapon (i.e. idling, firing, etc.)

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

        // Get the current track position.
        double pos = controller.getTrackPosition( 0 );

        // What is the current weapon state?
        switch ( mState )
        {
            case WeaponState::Lowered:

                // Start raising the weapon when LMB is pressed
                if ( input.isMouseButtonPressed( MouseButtons::Left ) )
                {
                    mState = WeaponState::Raising;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Raising" ) );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );

                } // End if LMB
                break;

            case WeaponState::Raising:

                // Switch to idle when raising is complete.
                if ( pos >= (24.0f / 30.0f) )
                {
                    mState = WeaponState::Idle;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Idle" ) );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );
                
                } // End if complete
                else
                {
                    // Only break if we don't want to drop through to idle state.
                    break;
                
                } // End if playing

            case WeaponState::Firing:
                if ( !input.isMouseButtonPressed( MouseButtons::Left ) )
                {
                    mState = WeaponState::Idle;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Idle" ) );
                    controller.setTrackFrameLimits( 0, 0x7FFFFFFF, 0x7FFFFFFF );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );

                    if ( @mMuzzleFlashEmitter != null )
                        mMuzzleFlashEmitter.enableLayerEmission( 0, false );

                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( false, false );

                } // End if LMB
                else
                {
                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( (pos % (5.0f / 30.0f)) <= (2.5f / 30.0f), false );
                }
                break;

            case WeaponState::Idle:

                // If user presses 'R', switch to reload.
                if ( input.isKeyPressed( Keys::R, true ) )
                {
                    mState = WeaponState::Reloading;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Reload" ) );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );

                } // End if 'R'
                // Start raising the weapon when LMB is pressed
                else if ( input.isMouseButtonPressed( MouseButtons::Left ) )
                {
                    mState = WeaponState::Firing;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Fire" ) );
                    controller.setTrackFrameLimits( 0, 0, 5 );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );

                    if ( @mMuzzleFlashEmitter != null )
                        mMuzzleFlashEmitter.enableLayerEmission( 0, true );

                    if ( @mMuzzleFlashLight != null )
                        mMuzzleFlashLight.showNode( true, false );

                } // End if LMB
                break;

            case WeaponState::Reloading:

                // Switch back to idle when reload is complete.
                if ( pos >= (139.0f / 30.0f) )
                {
                    mState = WeaponState::Idle;
                    controller.setTrackAnimationSet( 0, mActor.getAnimationSetByName( "Idle" ) );
                    controller.setTrackPosition( 0, 0 );
                    controller.setTrackSpeed( 0, 1.0f );
                
                } // End if complete
                else
                {
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

} // End Class FirstPersonActor