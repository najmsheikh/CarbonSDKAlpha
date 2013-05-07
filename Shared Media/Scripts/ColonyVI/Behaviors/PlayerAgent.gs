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
// Name : PlayerAgent.gs                                                     //
//                                                                           //
// Desc : Script responsible for managing the main player controlled         //
//        character and its components, including the handling of input,     //
//        camera control and other necessary behaviors.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "Agent.gsh"
#include_once "Objective.gsh"
#include_once "FirstPersonActor.gs"

//-----------------------------------------------------------------------------
// Configuration
//-----------------------------------------------------------------------------
const int   FirstPersonActorId  = 0x5AC;
const float CharacterHeight     = 2.0f;
const float CrouchingSpeed      = 1.34112f * 2;
const float WalkingSpeed        = 1.34112f * 2;
const float RunningSpeed        = 1.34112f * 4;

//-----------------------------------------------------------------------------
// Notes & Todo List
//-----------------------------------------------------------------------------
// * applyCameraOffsets() should ideally be performed after the full scene update, rather than during.

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Player (Class)
// Desc : Behavior used to map input and other events to the main player
//        object.
//-----------------------------------------------------------------------------
shared class PlayerAgent : Agent
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    // Object references
    private CameraNode@                 mCamera;            // The camera associated with the player.
    private ActorNode@                  mFPActorNode;       // Spawned actor designed to represent the first person player model.
    private FirstPersonActor@           mFPActor;           // Script object currently attached to the player's first person actor.
    private CharacterController@        mController;        // Character controller associated with the player.

    // States
    private CharacterState              mPreviousState;     // Previous state of the character controller (for transitions -- air to floor, etc.)
    private CharacterState              mCurrentState;      // Current state of the character controller.
    private bool                        mLeftFootHit;       // Have we handled the left foot hit (mostly for sound effects).
    private bool                        mRightFootHit;      // Have we handled the right foot hit (mostly for sound effects).

    // Feedback
    private float                       mTimeSinceDamage;

    // Camera control
    private Vector3                     mLastCamPos;        // Last computed position of the camera for smoothing.
    private float                       mBobCycle;
    private Vector2                     mLastBobOffset;
    private bool                        mShakeCamera;
    private float                       mCameraShakeLength;
    private float                       mCameraShakeTime;
    private float                       mCameraShakeMagnitude;
    private float                       mCameraShakeLastUpdate;
    private Vector2                     mCameraShakeOffset;

    // Input
    private Vector2                     mTotalOffset;       // Total amount of mouse motion recorded subsequent to previous update.
	private Vector2                     mPrevRotation;      // Previous rotation angles used for a single frame average.

    // Sounds
    private array<AudioBufferHandle>    mFootstepSounds;    // Array containing a random selection of footstep sounds for a metal surface.

    // Right click physics grab
    private ObjectNode@                 mSelectedNode;      // Node selected when right clicking to grab objects.
    private KinematicControllerJoint@   mJoint;             // Joint used for the physics constraint.
    private float                       mGrabDistance;      // Distance from the camera to the grabbed object on first click.

    // Objectives
    private Objective@                  mCurrentObjective;  // The player's current objective.

	///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : PlayerAgent () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	PlayerAgent( )
    {
        // Setup agent description
        mMaximumHealth  = 100;
        mMaximumArmor   = 100;

        // Initialize variables to sensible defaults.
        mTotalOffset    = Vector2( 0.0f, 0.0f );
	    mPrevRotation   = Vector2( 0.0f, 0.0f );
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
        // Assign a character controller to the player.
        Scene @ scene = object.getScene();
        @mController = CharacterController( scene.getPhysicsWorld(), true );
        object.setPhysicsController( mController );

        // Allow the controller to initialize.
        mController.setCharacterHeight( CharacterHeight );
        mController.setCharacterRadius( 0.4f );
        mController.initialize( );
        
        // Now create a camera that can be attached to the player object.
        @mCamera = cast<CameraNode>(scene.createObjectNode( true, RTID_CameraObject, false ));
        
        // Setup camera properties
        mCamera.setFOV( 75.0f );
        mCamera.setNearClip( 0.2f );
        mCamera.setFarClip( 10000.01f );
        //mCamera.setUpdateRate( UpdateRate::Always );

        // Offset the camera to "eye" level based on the configured height
        // of the character controller prior to attaching to the player.
        Transform playerTransform = object.getWorldTransform();
        mLastBobOffset = Vector2(0,0);
        mLastCamPos = playerTransform.position();
        mLastCamPos.y += CharacterHeight * 0.95f;
        mCamera.setPosition( mLastCamPos );
        mCamera.setOrientation( playerTransform.orientation() );
        
        // Attach the camera as a child of the player object.
        mCamera.setParent( object );

        // Set the camera as the scene's active.
        scene.setActiveCamera( mCamera );

        // Spawn the first person player actor and attach it to the camera.
        @mFPActorNode = cast<ActorNode@>(scene.loadObjectNode( FirstPersonActorId, CloneMethod::ObjectInstance, true ));
        if ( @mFPActorNode != null )
        {
            // Position pivot frame at the camera location and attach.
            mFPActorNode.setWorldTransform( mCamera.getWorldTransform() );
            mFPActorNode.setParent( mCamera );
            
            // Get the behavior associated with the first person actor so that we 
            // can interact with it and call its methods where necessary.
            @mFPActor = cast<FirstPersonActor>(mFPActorNode.getScriptedBehavior(0));
            if ( @mFPActor != null )
                mFPActor.setOwnerAgent( this );
        
        } // End if loaded

        // Load all footstep sounds.
        ResourceManager @ resources = scene.getResourceManager();
        mFootstepSounds.resize( 10 );
        for ( int i = 0; i < 5; ++i )
        {
            String fileName = "Sounds/Footstep_Metal_L" + (i+1) + ".wav";
            resources.loadAudioBuffer( mFootstepSounds[i], fileName, AudioBufferFlags::Complex, 0, DebugSource() );
            fileName = "Sounds/Footstep_Metal_R" + (i+1) + ".wav";
            resources.loadAudioBuffer( mFootstepSounds[i+5], fileName, AudioBufferFlags::Complex, 0, DebugSource() );
            
        } // Next footstep

        // Setup initial states.
        mCurrentHealth      = mMaximumHealth;
        mCurrentArmor       = mMaximumArmor;
        mTimeSinceDamage    = 9999999.0f;

        // Call base class implementation
        Agent::onAttach( object );
	}

	//-------------------------------------------------------------------------
	// Name : onDetach () (Event)
	// Desc : Called by the application when the behavior is detached from an
	//        object and can shut down.
	//-------------------------------------------------------------------------
	void onDetach( ObjectNode @ object )
	{
        // Relinquish our references.
        @mCamera            = null;
        @mFPActorNode       = null;
        @mFPActor           = null;
        @mController        = null;
        @mCurrentObjective  = null;

        // Close all resources
        mFootstepSounds.resize( 0 );
	}

	//-------------------------------------------------------------------------
	// Name : onUpdate () (Event)
	// Desc : Object is performing its update step.
	//-------------------------------------------------------------------------
	void onUpdate( float elapsedTime )
	{
        // Update tracking.
        mTimeSinceDamage += elapsedTime;

        // Do nothing if we don't have references to the necessary objects.
        if ( @mNode == null || @mCamera == null || !isAlive() )
            return;

        // Recharge armor after 6 seconds of no damage at a rate of 10 per second
        if ( mTimeSinceDamage >= 6.0f && mCurrentArmor < mMaximumArmor )
            mCurrentArmor = min( mMaximumArmor, mCurrentArmor + 10.0f * elapsedTime );

        // Record new character state. We use these to check for
        // transitions from floor to air, and other events.
        mPreviousState = mCurrentState;
        mCurrentState = mController.getCharacterState();

        // Process input first of all in order to update the player and camera 
        // orientation, trigger other behaviors, etc.
        processInput( elapsedTime );

        // Apply smoothing and head bob to the camera and first person models.
        applyCameraOffsets( elapsedTime );

        // Handle sound effects for player (footsteps, etc.)
        playCharacterSounds();

        // Recompute position of the joint that may be holding an object based
        // on the current (potentially new) orientation of the player camera
        // and any configured "grab distance" (mouse wheel scroll).
        if ( @mJoint != null )
        {
            RenderDriver @ driver = getAppRenderDriver();
            Viewport viewport = driver.getViewport();
            Size viewportSize = Size(viewport.width, viewport.height);

            // Convert the location at the center of the screen into a ray for picking
            // and update the location of the joint.
            Vector3 rayOrigin, rayDir;
            if ( mCamera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
                mJoint.setPosition( rayOrigin + rayDir * mGrabDistance );
        
        } // End if holding an object
	}

	//-------------------------------------------------------------------------
	// Name : onKeyPressed ( ) (Event)
	// Desc : Called by the input driver whenever a key is first pressed OR if
	//        the key repeat is fired.
	//-------------------------------------------------------------------------
	void onKeyPressed( int keyCode, uint controlCodes )
	{
		InputDriver @ input = getAppInputDriver();

        // Are we still alive?
        if ( isAlive() )
        { 
            // Weapon switch?
            if ( @mFPActor != null )
            {
                if ( keyCode == Keys::D1 )
                    mFPActor.requestWeaponSwitch( WeaponType::Beretta );
                else if ( keyCode == Keys::D2 )
                    mFPActor.requestWeaponSwitch( WeaponType::M16 );

            } // End if valid actor

            // Debug: Replenish health
            if ( keyCode == Keys::H )
            {
                mCurrentHealth = mMaximumHealth;
                mCurrentArmor = mMaximumArmor;

            } // End if H

            // Objective trigger?
            if ( @mCurrentObjective != null && mCurrentObjective.isObjectiveInRange( mNode.getPosition() ) )
            {
                if ( keyCode == Keys::E )
                    @mCurrentObjective = mCurrentObjective.onActivate( mNode );
            
            } // End if objective in range

        } // End if still alive
	}

	//-------------------------------------------------------------------------
	// Name : onMouseMove () (Event)
	// Desc : Called by the input driver whenever the mouse moves at all.
	//-------------------------------------------------------------------------
	void onMouseMove( const Point & position, const Vector2 & offset )
	{
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

		// Only record object rotation if we're not in cursor mode
        mTotalOffset += offset;
	}

    //-------------------------------------------------------------------------
	// Name : onMouseWheelScroll () (Event)
	// Desc : Called by the input driver whenever the user scrolls the mouse
    //        wheel.
	//-------------------------------------------------------------------------
    void onMouseWheelScroll( int delta, const Point & position )
    {
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

        // Adjust object grab distance to bring it closer to / father away from the player.
        mGrabDistance += float(delta) * 0.5f;
        if ( mGrabDistance < 0.7 )
            mGrabDistance = 0.7;
    }

    //-------------------------------------------------------------------------
	// Name : onMouseButtonDown () (Event)
	// Desc : Called by the input driver whenever the user presses a mouse
    //        button.
	//-------------------------------------------------------------------------
    void onMouseButtonDown( int buttons, const Point & position )
    {
        InputDriver @ input = getAppInputDriver();
		if ( input.getMouseMode() == MouseHandlerMode::Cursor )
            return;

        /*// Grab and/or throw an object.
        if ( @mSelectedNode != null && (buttons & MouseButtons::Left) != 0 )
        {
            // Throw the object by applying an impulse at the point we're grabbing.
            mSelectedNode.applyImpulse( mCamera.getZAxis() * 20.0f, mJoint.getPosition() );

            // Destroy any joint holding up this body.
            @mJoint = null;

        } // End if object held
        else if ( (buttons & MouseButtons::Right) != 0 )
        {
            // Attempt to grab an object in the world.
            grabObject();
        
        } // End if RMB*/
    }

    //-------------------------------------------------------------------------
	// Name : onMouseButtonDown () (Event)
	// Desc : Called by the input driver whenever the user releases a mouse
    //        button.
	//-------------------------------------------------------------------------
    void onMouseButtonUp( int buttons, const Point & position )
    {
        // Always allow this to run even if UI is up so event doesn't get lost.
        if ( @mSelectedNode != null && (buttons & MouseButtons::Right) != 0 )
        {
            // Destroy any joint holding up this body.
            @mJoint = null;
            @mSelectedNode = null;

        } // End if RMB
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Method Overrides (Agent)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : getCurrentWeapon ()
	// Desc : Get the weapon currently selected for this agent.
	//-------------------------------------------------------------------------
    Weapon @ getCurrentWeapon( )
    {
        // Pass through to first person actor.
        return mFPActor.getCurrentWeapon();
    }

    //-------------------------------------------------------------------------
	// Name : getHeight ()
	// Desc : Calculate current height of the agent.
	//-------------------------------------------------------------------------
    float getHeight( )
    {
        return mController.getCharacterHeight( true );
    }

    //-------------------------------------------------------------------------
	// Name : onProjectileHit () (Event)
	// Desc : Agent was hit by a projectile.
	//-------------------------------------------------------------------------
    void onProjectileHit( Weapon @ weapon, ObjectNode @ intersectedNode, const Vector3 & from, const Vector3 & to )
    {
        // Alive?
        if ( !isAlive() )
            return;

        // Get a reference to the agent that is holding the weapon that shot us.
        Agent @ sourceAgent = weapon.getOwnerAgent();
        if ( @sourceAgent == null )
            return;

        // Compute total damage
        float damage = weapon.computeDamage( vec3Length( to - from ) );

        // Apply damage to armor (if there is any)
        float armorDamage = min( mCurrentArmor, damage );
        mCurrentArmor -= armorDamage;
        damage -= armorDamage;

        // Apply damage to health (if there is any)
        float healthDamage = min( mCurrentHealth, damage );
        mCurrentHealth -= healthDamage;
        damage -= healthDamage;

        // Are we dead?
        if ( mCurrentHealth <= CGE_EPSILON )
        {
            // We died.
            kill( );

        } // End if dead
        else
        {

        } // End if !dead

        // Shake the camera if it isn't already
        if ( !mShakeCamera )
        {
            mShakeCamera = true;
            mCameraShakeLength = 0.2f;
            mCameraShakeTime = 0.0f;
            mCameraShakeLastUpdate = 0.0f;
            mCameraShakeMagnitude = 0.3f;
        
        } // End if not shaking

        // We were damaged
        mTimeSinceDamage = 0;
    }

    //-------------------------------------------------------------------------
	// Name : onExplosionHit () (Event)
	// Desc : Agent was near to an explosion.
	//-------------------------------------------------------------------------
    void onExplosionHit( const Vector3 & source, float maxDamage, float damageScale, float force )
    {
        // Alive?
        if ( !isAlive() )
            return;

        // Compute total damage
        float damage = maxDamage * damageScale;

        // Apply damage to armor (if there is any)
        float armorDamage = min( mCurrentArmor, damage );
        mCurrentArmor -= armorDamage;
        damage -= armorDamage;

        // Apply damage to health (if there is any)
        float healthDamage = min( mCurrentHealth, damage );
        mCurrentHealth -= healthDamage;
        damage -= healthDamage;

        // Are we dead?
        if ( mCurrentHealth <= CGE_EPSILON )
        {
            // We died.
            kill( );

        } // End if dead
        else
        {
            // Throw the player.
            Vector3 center  = mNode.getPosition() + (mNode.getYAxis() * mController.getCharacterHeight( true ) * 0.5f);
            float distance = vec3Length( center - source );
            if ( distance > CGE_EPSILON )
            {
                Vector3 impulse = ((center - source) / distance) * (force/30.0f);
                mController.applyImpulse( impulse );
            
            } // End if not dead on

        } // End if !dead

        // Shake the camera
        mShakeCamera = true;
        mCameraShakeLength = 0.5f;
        mCameraShakeTime = 0.0f;
        mCameraShakeLastUpdate = 0.0f;
        mCameraShakeMagnitude = 0.7f * min(1.0f,damageScale + 0.5f);

        // We were damaged.
        mTimeSinceDamage = 0;
    }

    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger player death and perform any death effects.
	//-------------------------------------------------------------------------
    void kill( )
    {
        kill( null, Vector3(0,0,0), Vector3(0,0,0) );
    }

    //-------------------------------------------------------------------------
	// Name : kill ()
	// Desc : Trigger player death and perform any death effects.
	//-------------------------------------------------------------------------
    void kill( ObjectNode @ hitNode, const Vector3 & hitPoint, const Vector3 & hitImpulse )
    {
        // No-op?
        if ( !isAlive() )
            return;

        // We are no longer alive.
        setAlive( false );

        // Switch away weapon and hide FP actor entirely.
        if ( @mFPActor != null )
            mFPActor.requestWeaponSwitch( WeaponType::None );
        if ( @mFPActorNode != null )
            mFPActorNode.showNode( false, true );

        // Destroy controller.
        mNode.setPhysicsController( null );
        @mController = null;

        // Reset camera states
        mCamera.enableDepthOfField( false );
        mCamera.setFOV( 75.0f );

        // Create a new dummy object for the death sequence.
        Scene @ scene = mCamera.getScene();
        DummyNode @ dummy = cast<DummyNode@>(scene.createObjectNode( true, RTID_CameraObject, false ));

        // Position it at the camera location
        dummy.setWorldTransform( mCamera.getWorldTransform() );

        // Detach the camera from the player and attach it to the dummy.
        mCamera.setParent( dummy );

        // Set its size to something reasonable and then enable rigid dynamics.
        // Dummy will fall to the floor and maybe even roll!
        dummy.setPhysicsModel( PhysicsModel::RigidDynamic );

        // Apply a random torque.
        dummy.applyTorqueImpulse( Vector3( randomFloat( -10.0f, 10.0f ), randomFloat( -10.0f, 10.0f ), randomFloat( -10.0f, 10.0f ) ) );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : teleportTo ()
	// Desc : Reposition the player at the specified location.
	//-------------------------------------------------------------------------
    void teleportTo( const Vector3 & position )
    {
        Vector3 delta = position - mNode.getPosition();
        mNode.setPosition( position );
        mLastCamPos += delta;
    }

    //-------------------------------------------------------------------------
	// Name : getTimeSinceDamage ()
	// Desc : Get the total amount of time in seconds since we last took damage
	//-------------------------------------------------------------------------
    float getTimeSinceDamage( )
    {
        return mTimeSinceDamage;
    }

    //-------------------------------------------------------------------------
	// Name : getTimeSinceFired ()
	// Desc : Get the total amount of time in seconds since we last fired our
    //        current weapon.
	//-------------------------------------------------------------------------
    float getTimeSinceFired( )
    {
        Weapon @ weapon = getCurrentWeapon();
        if ( @weapon != null )
            return weapon.getTimeSinceFired();
        return 99999999.0f;
    }

    //-------------------------------------------------------------------------
	// Name : getCurrentObjective ()
	// Desc : Get the player's current objective point.
	//-------------------------------------------------------------------------
    Objective @ getCurrentObjective( )
    {
        return mCurrentObjective;
    }

    //-------------------------------------------------------------------------
	// Name : setCurrentObjective ()
	// Desc : Set the player's current objective point.
	//-------------------------------------------------------------------------
    void setCurrentObjective( Objective @ objective )
    {
        @mCurrentObjective = objective;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : processInput () (Private)
	// Desc : Process any input recorded in response to input events, or
    //        perform any additional input processing that we want to occur
    //        specifically during the object 'update()' pass.
	//-------------------------------------------------------------------------
    private void processInput( float elapsedTime )
    {
        // Bail if the cursor is currently visible.
        InputDriver @ input = getAppInputDriver();
        if ( input.getMouseMode() == MouseHandlerMode::Cursor )
        {
            // Set all input to zero.
            mNode.setInputChannelState( "ForwardBackward", 0 );
            mNode.setInputChannelState( "LeftRight", 0 );
            mNode.setInputChannelState( "ClimbSink", 0 );
            mNode.setInputChannelState( "Jump", 0 );
            return;
        
        } // End if cursor visible

        // Reduce rotation if aiming down sights.
        Vector2 finalRotation = mTotalOffset;
        if ( input.isMouseButtonPressed( MouseButtons::Right ) )
            finalRotation *= 0.6f;

		// Rotate parent left/right and its child camera up/down.
		if ( finalRotation.x != 0.0f )
			mNode.rotateAxis( finalRotation.x, Vector3( 0, 1, 0 ) );
        if ( @mCamera != null && finalRotation.y != 0.0f )
        {
            // Clamp Y axis rotation.
            float currentRotation = CGEToDegree(acos(min(1.0f,max(-1.0f,mCamera.getZAxis().y))));
            currentRotation = currentRotation + finalRotation.y;
            if ( currentRotation > 179 )
                finalRotation.y -= (currentRotation - 179);
            else if ( currentRotation < 1 )
                finalRotation.y += (1 - currentRotation);

            // Apply final rotation
            mCamera.rotateLocal( finalRotation.y, 0, 0 );

        } // End if rotate camera

		// Remove consumed cursor motion (so we don't keep on applying it)
		mTotalOffset = Vector2(0,0);

        // Check the relevant keys for movement input
        float strafeState = 0.0f, dollyState = 0.0f, jumpState = 0.0f, climbState = 0.0f;
	    if ( input.isKeyPressed( Keys::W ) || input.isKeyPressed( Keys::Up ) )
            dollyState += 1.0f;
	    if ( input.isKeyPressed( Keys::S ) || input.isKeyPressed( Keys::Down ) )
		    dollyState -= 1.0f;
	    if ( input.isKeyPressed( Keys::A ) || input.isKeyPressed( Keys::Left ))
		    strafeState -= 1.0f;
	    if ( input.isKeyPressed( Keys::D ) || input.isKeyPressed( Keys::Right ) )
		    strafeState += 1.0f;

        // Space will jump, or fly up when held. Flying mode will be enabled
        // if the user double taps space or otherwise jumps whilst airborne.
        if ( input.isKeyPressed( Keys::Space ) )
        {
            // Jump and fly up / climb
            jumpState = 1.0f;
            climbState = 1.0f;

            // If the character is already airborne and the user presses space
            // a second time, switch to 'fly mode'.
            if ( mController.getCharacterState() == CharacterState::Airborne && !input.wasKeyPressed( Keys::Space ) )
                mController.enableFlyMode( true, true );
        
        } // End if Space
        
        // If left control is held, sink down (in fly mode) or switch the
        // character's standing mode to crouch. Otherwise, restore standing
        // mode at the next available opportunity.
        if ( input.isKeyPressed( Keys::LControl ) )
        {
            // Sink
            climbState = -1.0f;

            // Switch standing mode to crouch (if not flying)
            if ( !mController.isFlyModeEnabled() )
                mController.requestStandingMode( CharacterStandingMode::Crouching );
        
        } // End if held
        else
        {
            // Request that the character controller switch to a standing
            // mode. This is a request only since there is no guarantee that 
            // it will immediately be able to switch if, for instance, there is
            // solid geometry above the player's head stopping this from happening.
            // The controller will however switch as soon as there is room for the
            // character to stand.
            mController.requestStandingMode( CharacterStandingMode::Standing );

        } // End if released

        // Select current maximum speed
        switch ( mController.getActualStandingMode() )
        {
            case CharacterStandingMode::Crouching:
                mController.setMaximumWalkSpeed( CrouchingSpeed );
                break;

            case CharacterStandingMode::Standing:
                if ( input.isKeyPressed( Keys::LShift ) || input.isMouseButtonPressed( MouseButtons::Right ) )
                    mController.setMaximumWalkSpeed( WalkingSpeed );
                else
                    mController.setMaximumWalkSpeed( RunningSpeed );  
                break;        
        
        } // End switch standing mode

        // Set the input channel states (the character controller listens in on these).
        mNode.setInputChannelState( "ForwardBackward", dollyState );
        mNode.setInputChannelState( "LeftRight", strafeState );
        mNode.setInputChannelState( "ClimbSink", climbState );
        mNode.setInputChannelState( "Jump", jumpState );
    }

    //-------------------------------------------------------------------------
	// Name : applyCameraOffsets () (Private)
	// Desc : Apply camera position smoothing and any other camera based
    //        effects that may be necessary (such as head bob / sway).
	//-------------------------------------------------------------------------
    private void applyCameraOffsets( float elapsedTime )
    {
        // Here we'll add a little 'head bob' effect to the camera. First compute 
        // the camera's required vertical offset from its parent player based on the
        // height of the character as it currently stands.
        float cameraOffset = mController.getCharacterHeight( true ) * 0.95f;

        // Compute head bob offsets (we only want it to bob if the character not 
        // currently sliding or airborne)
        Vector2 bobOffset(0,0);
        if ( mCurrentState == CharacterState::OnFloor )
        {
            // Compute the current horizontal speed of the character. This
            // will be used to increase or decrease the rate of the head
            // bob effect as they walk faster / slower.
            Vector3 velocity = mController.getVelocity();
            float characterSpeed = vec3Length( Vector3( velocity.x, 0, velocity.z ) );

            // Advance the 'head bob cycle' based on the speed.
            float bobSpeed = 1.5f * characterSpeed;
            mBobCycle += elapsedTime * bobSpeed;

            // Based on the current cycle of the head bob, compute the amount by 
            // which we want to offset the camera along its local X and Y axes
            // when we compute its final position.
            Vector2 bobScale( 0.02f, 0.015f );
            bobOffset.x = sin( mBobCycle ) * characterSpeed  * bobScale.x;
            bobOffset.y = sin( 2 * mBobCycle ) * characterSpeed * bobScale.y;

            // Reset the head bob cycle to the beginning if they come to
            // a complete standstill for any amount of time.
            if ( characterSpeed < CGE_EPSILON )
                mBobCycle = 0.0f;
        
        } // End if on floor
        else
        {
            // Reset the head bob cycle if they become airborne or start sliding.
            mBobCycle = 0.0f;

        } // End if !on floor

        // Apply camera shake along with bob
        if ( mShakeCamera )
        {
            mCameraShakeTime += elapsedTime;
            if ( mCameraShakeTime >= mCameraShakeLength )
            {
                mShakeCamera = false;
            
            } // End if complete
            else
            {
                // Compute a new offset every 1/40th of a second
                if ( mCameraShakeLastUpdate <= CGE_EPSILON || (mCameraShakeTime - mCameraShakeLastUpdate) > (1/40.0f))
                {
                    float mag = mCameraShakeMagnitude * (1.0f-(mCameraShakeTime / mCameraShakeLength));
                    mCameraShakeOffset.x = randomFloat( -mag, mag );
                    mCameraShakeOffset.y = randomFloat( -mag, mag );
                    mCameraShakeLastUpdate = mCameraShakeTime;
                
                } // End if new offset

                // Apply shake offset
                bobOffset.x += mCameraShakeOffset.x;
                bobOffset.y += mCameraShakeOffset.y;
            
            } // End if continue
        
        } // End if shake

        // Smooth any computed offsets so that we get a more elastic
        // and gradual adjustment as its magnitude changes over time.
        float bobSmoothFactor = 0.05f / elapsedTime;
        bobOffset.x = smooth( bobOffset.x, mLastBobOffset.x, bobSmoothFactor );
        bobOffset.y = smooth( bobOffset.y, mLastBobOffset.y, bobSmoothFactor );
        mLastBobOffset = bobOffset;
        
        // Now we've computed the offsets we need for the headbob, it's time to
        // compute the final location of the camera. We apply a bit of smoothing here
        // too so that the camera acts like it's on a 'spring' of sorts. This produces
        // a more fluid camera motion that allows the player to -- for instance -- walk
        // up stairs without the camera bouncing up each step quite so dramatically.
        // We'll start by selecting the smoothing amounts we want on each axis
        // independently.
        float verticalSmoothFactor, horizontalSmoothFactor = 0.05f / elapsedTime;
        if ( mController.getCharacterState() != CharacterState::OnFloor )
            verticalSmoothFactor = 0.03f / elapsedTime;
        else
            verticalSmoothFactor = 0.08f / elapsedTime;

        // Now smoothly transition camera position frame to frame.
        Vector3 newPosition = mNode.getPosition();        
        newPosition.x = smooth( newPosition.x, mLastCamPos.x, horizontalSmoothFactor );
        newPosition.y = smooth( newPosition.y + cameraOffset, mLastCamPos.y, verticalSmoothFactor );
        newPosition.z = smooth( newPosition.z, mLastCamPos.z, horizontalSmoothFactor );
        mLastCamPos = newPosition;

        // Finally apply any head bob offset and set it to the camera.
        newPosition += mCamera.getXAxis() * bobOffset.x;
        newPosition += mCamera.getYAxis() * bobOffset.y;
        mCamera.setPosition( newPosition );

        // Handle ADS and first person actor effects.
        if ( @mFPActorNode != null )
        {
            // Reset actor back precisely on the camera.
            mFPActorNode.setWorldTransform( mCamera.getWorldTransform() );

            // Allow actor to perform its offsets.
            float counterBobStrength = 0.1f;
            mFPActor.applyCameraOffsets( Vector3( -bobOffset.x * counterBobStrength, bobOffset.y * counterBobStrength, 0 ) );

            // RMB = ADS
            InputDriver @ input = getAppInputDriver();
            if ( input.isMouseButtonPressed( MouseButtons::Right ) )
            {
                // Update the camera.
                mCamera.enableDepthOfField( true );
                mCamera.setForegroundExtents( 0.19, 0.24 );
                mCamera.setForegroundBlur( 1, 2, -1, 1, 1, -1.0f );
                mCamera.setBackgroundExtents( 0, 0 ); // Disabled
                mCamera.setFOV( 30.0f );
            
            } // End if ADS
            else
            {
                mCamera.enableDepthOfField( false );
                mCamera.setFOV( 75.0f );
            
            } // End if !ADS
        
        } // End if valid actor
    }

    //-------------------------------------------------------------------------
	// Name : playCharacterSounds () (Private)
	// Desc : Play appropriate sound effects based on the current state of the
    //        player (footsteps, breath sounds, etc.)
	//-------------------------------------------------------------------------
    private void playCharacterSounds( )
    {
        // Play footsteps if the character is currently on the floor.
        if ( mCurrentState == CharacterState::OnFloor )
        {
            // Compute the current horizontal speed of the character. This
            // will be used to increase or decrease the rate of the head
            // bob effect as they walk faster / slower.
            Vector3 velocity = mController.getVelocity();
            float characterSpeed = vec3Length( Vector3( velocity.x, 0, velocity.z ) );

            // Are we landing on the floor this frame?
            bool landing = (mPreviousState == CharacterState::Airborne);

            // Compute our vertical and lateral movement due to sway. We'll play sounds 
            // at relevant intervals synchronized to the camera offsets.
            float downCycle = sin( 2 * mBobCycle );
            float sideCycle = sin( mBobCycle );

            // Process further if we're at the bottom of the vertical cycle, or we 
            // are landing on the floor from some other state.
            if ( downCycle <= -0.9f || landing )
            {
                // If we are on the left side of the cycle and we haven't played the
                // left footstep yet, do so now. Also play if we are landing.
                if ( (!mLeftFootHit && sideCycle < 0) || landing )
                {
                    // Play random left footstep
                    AudioBuffer @ effect = mFootstepSounds[randomInt(0,4)].getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.setPan( -0.025f );                       // A little in the left speaker.
                        effect.setVolume( (landing) ? 0.75f : 0.35f );  // Louder if we're landing.
                        effect.setBufferPosition(0);                    // Play once from the start.
                        effect.play( false );                           
                    
                    } // End if loaded

                    // Don't play again until we're back on this cycle.
                    mLeftFootHit  = !landing;
                    mRightFootHit = false;
                
                } // End if left cycle

                // Check the same again for the right side of the cycle (or landing)
                if ( (!mRightFootHit && sideCycle > 0) || landing )
                {
                    // Play random right footstep
                    AudioBuffer @ effect = mFootstepSounds[randomInt(5,9)].getResource(true);
                    if ( @effect != null && effect.isLoaded() )
                    {
                        effect.setPan( 0.025f );                        // A little in the right speaker.
                        effect.setVolume( (landing) ? 0.75f : 0.35f );  // Louder if we're landing.
                        effect.setBufferPosition(0);                    // Play once from the start.
                        effect.play( false );                           
                    
                    } // End if loaded

                    // Don't play again until we're back on this cycle.
                    mRightFootHit = !landing;
                    mLeftFootHit  = false;
                
                } // End if right cycle
            
            } // End if foot down

            // Reset playback if the player comes to a complete standstill for 
            // any amount of time.
            if ( characterSpeed < CGE_EPSILON )
            {
                mRightFootHit = false;
                mLeftFootHit  = false;
            
            } // End if reset

        } // End if on the floor
    }

    //-------------------------------------------------------------------------
	// Name : grabObject () (Private)
	// Desc : Attempt to grab an object in the scene and pick it up (by 
    //        creating and attaching kinematic physics controller joint).
	//-------------------------------------------------------------------------
    private void grabObject( )
    {
        RenderDriver @ driver = getAppRenderDriver();
        Viewport viewport = driver.getViewport();
        Size viewportSize = Size(viewport.width, viewport.height);

        // Convert the location at the center of the screen into a ray for picking
        Vector3 rayOrigin, rayDir;
        if ( !mCamera.viewportToRay( viewportSize, Vector2(viewportSize.width/2.0f, viewportSize.height/2.0f), rayOrigin, rayDir ) )
            return;

        // Ask the scene to retrieve the closest intersected object node.
        Vector3 intersection;
        Scene @ scene = mNode.getScene();
        @mSelectedNode = scene.pickClosestNode( viewportSize, rayOrigin, rayDir, intersection );

        // Take action based on the selected object node
        if ( @mSelectedNode != null && @mSelectedNode.getPhysicsBody() != null )
        {
            // Compute the distance at which we 'grabbed' the node
            // so that this can be maintained.
            mGrabDistance = vec3Length( (intersection - rayOrigin) );

            // Create a new kinematic controller joint in the physics world at the intersection location.
            @mJoint = KinematicControllerJoint( scene.getPhysicsWorld(), mSelectedNode.getPhysicsBody(), intersection );

            // Set limits.
            mJoint.setConstraintMode( KinematicControllerMode::SinglePoint );
            mJoint.setMaxAngularFriction( 50.0f );
            mJoint.setMaxLinearFriction( 500.0f );

        } // End if is physics object
        else
        {
            @mSelectedNode = null;
        
        } // End if no physics object
    }

} // End Class Player