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
// Script Includes
//-----------------------------------------------------------------------------
#include_once "Agent.gsh"
#include_once "Weapon.gsh"

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------
shared enum WeaponType
{
    None    = 0,
    M16     = 1,    // 0x570,
    Beretta = 2,    // 0x584
    Count   = 3
};

shared enum FirstPersonActorState
{
    Lowered,        // Weapon is lowered
    Lowering,       // Weapon is in the process of being lowered
    Raising,        // Weapon is in the process of being raised
    Idle,           // Player is idling
    Firing,         // Weapon is being fired
    Reloading,      // Weapon is being reloaded
    ThrowGrenade    // Throwing grenade
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
    private Agent@                  mOwnerAgent;            // The agent that owns this actor.
    private FirstPersonActorState   mState;                 // Current state of the weapon (i.e. idling, firing, etc.)
    
    private array<String>           mWeaponIds;             // Available weapon identifiers.
    private WeaponType              mRequestedWeaponType;   // The weapon that it is requested we switch to whenever we're able.
    private WeaponType              mCurrentWeaponType;     // The reference identifier of the source weapon type currently equipped by the player.
    private ObjectNode@             mCurrentWeaponNode;     // The physical scene node of the weapon currently equipped by the player.
    private ObjectNode@             mCurrentADSNode;        // Child node that represents the offset to apply when aiming down sights.
    private Weapon@                 mCurrentWeapon;         // The script associated with the weapon currently equipped by the player.
    private bool                    mAllowTriggerHold;      // Can the player currently hold the trigger to continue to the next action? (i.e. Reload to fire, fire to reload, etc.)
    private bool                    mThrowGrenade;          // We're about to throw a grenade.
    private bool                    mMagHidden;             // Temporary state recording the state of the magazine during reload.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : FirstPersonActor () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	FirstPersonActor( )
    {
        // Configure available weapons for this player.
        mWeaponIds.resize( WeaponType::Count );
        mWeaponIds[ WeaponType::M16 ]       = "Weapon_M16";
        mWeaponIds[ WeaponType::Beretta ]   = "Weapon_Beretta";
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
        Scene @ scene = object.getScene();

        // Initialize variables
        mState              = FirstPersonActorState::Lowered;
        mAllowTriggerHold   = true;
        mMagHidden          = false;
        mThrowGrenade       = false;

        // Cache references to required objects.
        @mActor             = cast<ActorNode>(object);

        // Setup the actor's animation controller.
        mActor.setTrackFadeTimes( 0.05f, 0.05f );

        // Select the default initial weapon.
        mRequestedWeaponType    = WeaponType::M16;
        mCurrentWeaponType      = mRequestedWeaponType;
        @mCurrentWeaponNode     = mActor.findChild( mWeaponIds[mCurrentWeaponType] );
        @mCurrentWeapon         = cast<Weapon>(mCurrentWeaponNode.getScriptedBehavior(0));
        @mCurrentADSNode        = mActor.findChild( mWeaponIds[mCurrentWeaponType] + "_ADS" );

        // Show the current weapon.
        mCurrentWeapon.setOwnerAgent( mOwnerAgent );
        mCurrentWeapon.select( );

        // Auto raise weapon from lowered state.
        mState = FirstPersonActorState::Raising;
        mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Raise", AnimationPlaybackMode::PlayOnce );

        // Hide dummy magazines.
        ObjectNode @ dummyMag = mActor.findChild( "Magazine_M16_Dummy" );
        if ( @dummyMag != null )
            dummyMag.showNode( false, true );
        @dummyMag = mActor.findChild( "Magazine_Beretta_Dummy" );
        if ( @dummyMag != null )
            dummyMag.showNode( false, true );
	}

    //-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from its
	//        parent object.
	//-------------------------------------------------------------------------
    void onDetach( ObjectNode @ object )
    {
        @mActor             = null;
        @mCurrentWeaponNode = null;
        @mCurrentWeapon     = null;
        @mOwnerAgent        = null;
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

        // What is the current weapon state?
        switch ( mState )
        {
            case FirstPersonActorState::Lowered:
            {
                bool raiseWeapon = false;

                // Switching weapon type?
                if ( mRequestedWeaponType != mCurrentWeaponType )
                {
                    // Deactivate the previous weapon.
                    mCurrentWeapon.deselect();

                    // Select the new weapon.
                    mCurrentWeaponType  = mRequestedWeaponType;
                    @mCurrentWeaponNode = mActor.findChild( mWeaponIds[mCurrentWeaponType] );
                    @mCurrentWeapon     = cast<Weapon>(mCurrentWeaponNode.getScriptedBehavior(0));
                    @mCurrentADSNode    = mActor.findChild( mWeaponIds[mCurrentWeaponType] + "_ADS" );

                    // Activate the new weapon.
                    mCurrentWeapon.setOwnerAgent( mOwnerAgent );
                    mCurrentWeapon.select( );

                    // Ready to raise
                    raiseWeapon = true;

                } // End if weapon switch

                // Start raising the weapon?
                if ( raiseWeapon )
                {
                    mState = FirstPersonActorState::Raising;
                    mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Raise", AnimationPlaybackMode::PlayOnce );

                } // End if LMB
                break;
            
            } // End if lowered
            case FirstPersonActorState::Lowering:

                // Switch to lowered or throw grenade when lowering is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    // Throwing grenade?
                    if ( mThrowGrenade )
                    {
                        // Deactivate current weapon.
                        mCurrentWeapon.deselect();

                        // Load the grenade model.
                        Scene @ scene = mActor.getScene();
                        ObjectNode @ grenade = scene.loadObjectNode( 0x607, CloneMethod::ObjectInstance, false );
                        ObjectNode @ attach = mActor.findChild( "Grenade_Hardpoint" );
                        grenade.setPosition( attach.getPosition() );
                        grenade.setOrientation( attach.getXAxis(), attach.getYAxis(), attach.getZAxis() );
                        grenade.setParent( attach );

                        // Play grenade animation and switch to appropriate state
                        mActor.playAnimationSet( "Action", "Throw Grenade", AnimationPlaybackMode::PlayOnce, 1.3f, 0.0f );
                        mState = FirstPersonActorState::ThrowGrenade;
                        mThrowGrenade = false;

                    } // End if throw grenade
                    else
                    {
                        // Switch to standard lowered state
                        mState = FirstPersonActorState::Lowered;
                        mActor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Lowered", AnimationPlaybackMode::PlayOnce );

                    } // End if lowered
                
                } // End if complete
                break;

            case FirstPersonActorState::Raising:

                // Reactivate current weapon.
                mCurrentWeapon.select();

                // Switch to idle when raising is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    mState = FirstPersonActorState::Idle;
                    mActor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Idle", AnimationPlaybackMode::Loop );
                
                } // End if complete
                break;

            case FirstPersonActorState::ThrowGrenade:

                // Switch to raising weapon when throwing is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    mState = FirstPersonActorState::Raising;
                    mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Raise", AnimationPlaybackMode::PlayOnce );
                
                } // End if complete
                break;

            case FirstPersonActorState::Idle:

                // Throw grenade with 'G'
                if ( input.isKeyPressed( Keys::G, true ) )
                    mThrowGrenade = true;

                // Lower current weapon if a change was requested.
                if ( mThrowGrenade || mRequestedWeaponType != mCurrentWeaponType )
                {
                    mState = FirstPersonActorState::Lowering;
                    mActor.stopAnimationTrack( "Primary" );
                    mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Lower", AnimationPlaybackMode::PlayOnce );
                    break;

                } // End if weapon swap

                // Player has a weapon?
                if ( @mCurrentWeapon != null )
                {
                    bool triggerReload = false;

                    // Cycle through available firing modes with 'X'.
                    if ( input.isKeyPressed( Keys::X, true ) )
                        mCurrentWeapon.cycleFiringMode();

                    // If the user is pressing the left mouse button and the weapon
                    // is in a state where it is ready to fire, attempt to start firing.
                    // Otherwise we should trigger the reload.
                    if ( input.isMouseButtonPressed( MouseButtons::Left, !mAllowTriggerHold ) )
                    {
                        if ( mCurrentWeapon.getState() == WeaponState::Ready )
                        {
                            // Start firing with the current weapon if possible.
                            if ( mCurrentWeapon.beginFiring() == WeaponFiringResult::Success )
                            {
                                // Switch to the firing animation.
                                if ( mCurrentWeapon.getFiringMode() != WeaponFiringMode::SingleShot )
                                {
                                    // Looped fire cycle.
                                    mActor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Fire Cycle", AnimationPlaybackMode::Loop );
                                
                                } // End if !single shot
                                else
                                {
                                    // Single shot case. Stop and restart the animation to ensure 
                                    // rapid fire cases are handled.
                                    mActor.stopAnimationTrack( "Action", true );
                                    mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Fire", AnimationPlaybackMode::PlayOnce );

                                } // End if single shot

                                // Switch to firing state.
                                mState = FirstPersonActorState::Firing;

                                // Next action *cannot* be automatically triggered by holding the 
                                // button (i.e. firing into immediate reload, or firing repeatedly with single shot fire).
                                mAllowTriggerHold = false;

                            } // End if success

                        } // End if weapon ready
                        else if ( mCurrentWeapon.getState() == WeaponState::Empty )
                        {
                            triggerReload = true;

                        } // End if weapon empty

                    } // End if LMB with weapon

                    // If the user presses 'R' with a weapon, also trigger reload.
                    if ( input.isKeyPressed( Keys::R, true ) )
                        triggerReload = true;

                    // Trigger reload instead of firing?
                    if ( triggerReload )
                    {
                        // Request that weapon reload if possible.
                        if ( mCurrentWeapon.reload() == WeaponReloadResult::Success )
                        {
                            // Play reload animation
                            mActor.stopAnimationTrack( "Primary" );
                            mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Reload", AnimationPlaybackMode::PlayOnce, 1, 0.1f );

                            // Switch to reload state.
                            mState = FirstPersonActorState::Reloading;

                        } // End if success

                    } // End if reloading

                } // End if has weapon
                break;

            case FirstPersonActorState::Firing:

                // Wait until the weapon stops firing of its own accord,
                // or the user releases the left mouse button, and then
                // switch back to idle mode.
                if ( mCurrentWeapon.getState() != WeaponState::Firing || !input.isMouseButtonPressed( MouseButtons::Left ) )
                {
                    // Stop firing (in case button was manually released).
                    mCurrentWeapon.endFiring();

                    // Switch back to idle state
                    mState = FirstPersonActorState::Idle;
                    mActor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Idle", AnimationPlaybackMode::Loop );

                } // End if !LMB
                break;

            case FirstPersonActorState::Reloading:

                // Switch back to idle when reload is complete.
                if ( !mActor.isAnimationTrackPlaying( "Action", true ) )
                {
                    // Switch back to idle.
                    mState = FirstPersonActorState::Idle;
                    mActor.playAnimationSet( "Primary", mCurrentWeapon.getClass() + " Idle", AnimationPlaybackMode::Loop );

                    // Next action *can* be automatically triggered by holding the 
                    // button (i.e. reload back into firing immediately).
                    mAllowTriggerHold = true;
                
                } // End if complete
                else
                {
                    // Get the current position of the reload animation.
                    int track = mActor.playAnimationSet( "Action", mCurrentWeapon.getClass() + " Reload", AnimationPlaybackMode::PlayOnce );
                    double pos = controller.getTrackPosition( track );

                    if ( mCurrentWeaponType == WeaponType::M16 )
                    {
                        if ( !mMagHidden )
                        {
                            // Hide actual magazine (in weapon) and show dummy magazine (in hand) on frame 21.
                            if ( pos >= (27.0f / 30.0f) && pos < (61.0f / 30.0f) )
                            {
                                ObjectNode @ actualMag = mActor.findChild( "Magazine_M16" );
                                ObjectNode @ dummyMag = mActor.findChild( "Magazine_M16_Dummy" );
                                if ( @actualMag != null )
                                    actualMag.showNode( false, true );
                                if ( @dummyMag != null )
                                    dummyMag.showNode( true, true );
                                mMagHidden = true;
                            
                            } // End if >= 21
                        
                        } // End if !hidden
                        
                        if ( mMagHidden )
                        {
                            // Restore actual magazine (in weapon) on frame 65.
                            if ( pos >= (61.0f / 30.0f) )
                            {
                                ObjectNode @ actualMag = mActor.findChild( "Magazine_M16" );
                                ObjectNode @ dummyMag = mActor.findChild( "Magazine_M16_Dummy" );
                                if ( @actualMag != null )
                                    actualMag.showNode( true, true );
                                if ( @dummyMag != null )
                                    dummyMag.showNode( false, true );
                                mMagHidden = false;
                            
                            } // End if >= 65

                        } // End if hidden

                    } // End if M16
                    else if ( mCurrentWeaponType == WeaponType::Beretta )
                    {
                        if ( !mMagHidden )
                        {
                            // Hide actual magazine (in weapon) and show dummy magazine (in hand) on frame 18.
                            if ( pos >= (18.0f / 30.0f) && pos < (28.0f / 30.0f) )
                            {
                                ObjectNode @ actualMag = mActor.findChild( "Magazine_Beretta" );
                                ObjectNode @ dummyMag = mActor.findChild( "Magazine_Beretta_Dummy" );
                                if ( @actualMag != null )
                                    actualMag.showNode( false, true );
                                if ( @dummyMag != null )
                                    dummyMag.showNode( true, true );
                                mMagHidden = true;
                            
                            } // End if >= 18
                        
                        } // End if !hidden
                        
                        if ( mMagHidden )
                        {
                            // Restore actual magazine (in weapon) on frame 28.
                            if ( pos >= (28.0f / 30.0f) )
                            {
                                ObjectNode @ actualMag = mActor.findChild( "Magazine_Beretta" );
                                ObjectNode @ dummyMag = mActor.findChild( "Magazine_Beretta_Dummy" );
                                if ( @actualMag != null )
                                    actualMag.showNode( true, true );
                                if ( @dummyMag != null )
                                    dummyMag.showNode( false, true );
                                mMagHidden = false;
                            
                            } // End if >= 28

                        } // End if hidden

                    } // End if Beretta

                } // End if reloading
                break;

        } // End switch state
	}

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : setOwnerAgent ()
	// Desc : Set the agent that owns this actor.
	//-------------------------------------------------------------------------
    void setOwnerAgent( Agent @ agent )
    {
        @mOwnerAgent = agent;

        // Update the owner agent of the current weapon.
        if ( @mCurrentWeapon != null )
            mCurrentWeapon.setOwnerAgent( agent );
    }
    
    //-------------------------------------------------------------------------
	// Name : getOwnerAgent ()
	// Desc : Retrieve the agent that owns this actor.
	//-------------------------------------------------------------------------
    Agent @ getOwnerAgent( )
    {
        return @mOwnerAgent;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentWeapon ()
	// Desc : Retrieve the current weapon assigned to this actor.
	//-------------------------------------------------------------------------
    Weapon @ getCurrentWeapon( )
    {
        return @mCurrentWeapon;
    }

    //-------------------------------------------------------------------------
	// Name : requestWeaponSwitch ()
	// Desc : Switch to the specified weapon at the next available
    //        opportunity.
	//-------------------------------------------------------------------------
    void requestWeaponSwitch( WeaponType weapon )
    {
        mRequestedWeaponType = weapon;
    }

    //-------------------------------------------------------------------------
	// Name : applyCameraOffsets () (Private)
	// Desc : Apply offset position smoothing and any other camera based
    //        effects that may be necessary (for the first person model).
	//-------------------------------------------------------------------------
    void applyCameraOffsets( Vector3 bobOffset )
    {
        // RMB = Aim down sight (ADS)
        InputDriver @ input = getAppInputDriver();
        if ( input.isMouseButtonPressed( MouseButtons::Right ) )
        {
            // Offset us to the correct location for aim down sights.
            mActor.setWorldTransform( mCurrentADSNode.getWorldTransform() );

            // Reduce counter bob effect
            bobOffset *= 0.5f;
        
        } // End if ADS
        
        // Apply the 'counter' bob.
        mActor.moveLocal( bobOffset );

        // Move if throwing grenade.
        if ( mState == FirstPersonActorState::ThrowGrenade )
            mActor.moveLocal( 0, 0, 0.15f );

        // Update weapon aim points.
        if ( @mCurrentWeapon != null )
        {
            Scene @ scene = mActor.getScene();
            RenderDriver @ driver = scene.getRenderDriver();
            Viewport viewport = driver.getViewport();
            Size viewportSize = Size(viewport.width, viewport.height);

            // Convert the location at the center of the screen into a ray for picking
            Vector3 rayOrigin, rayDir;
            CameraNode @ camera = cast<CameraNode>(scene.getActiveCamera());
            if ( @camera != null && camera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
                mCurrentWeapon.setAimPoints( rayOrigin + (rayDir * 0.8f), rayOrigin + (rayDir * 1000.0f) );
        
        } // End if has weapon
    }
}