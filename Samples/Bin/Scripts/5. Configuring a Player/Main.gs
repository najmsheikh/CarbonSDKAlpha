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
// Name : Main.gs (5. Configuring a Player)                                  //
//                                                                           //
// Desc : Script containing the majority of the logic for demonstrating how  //
//        to load a scene from the currently open master world / project     //
//        and render it.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Main (Class)
// Desc : Scripted application state object used as the main entry point for
//        a given framework demonstration.
//-----------------------------------------------------------------------------
class Main : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@   mState;         // Application side state object
    private World@      mWorld;         // Main game world object.
    private Scene@      mScene;         // The scene we've loaded.
    private RenderView@ mSceneView;     // Render view into which the scene will be rendered.
    private CameraNode@ mCamera;        // Custom camera object describing the rendered PoV.
    private ObjectNode@ mPlayer;        // Object node used to represent dummy player object.
    private Vector3     mLastCamPos;    // Last computed position of the camera for smoothing.

    // Head bob! :)
    private float       mBobCycle;
    private Vector2     mLastBobOffset;

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedAppState)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : initialize ()
    // Desc : The state has been registered and is being initialized (usually
    //        at application startup), but is not yet necessarily being 
    //        activated. The script can perform any registration time 
    //        processing at this point.
    //-------------------------------------------------------------------------
    bool initialize( AppState @ state )
    {
        // Store required values.
        @mState = state;

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : begin ()
    // Desc : This event signifies that the state has actually been selected
    //        and activated by the state management system. This will generally
    //        be the point at which any specific resources relevant for the
    //        execution of this state will be built/loaded.
    //-------------------------------------------------------------------------
    bool begin( )
    {
        // Retrieve the application's main world object instance.
        @mWorld = getAppWorld();

        // Establish a connection to the master world databse we need.
        if (!mWorld.open( "Colony VI.cwm" ))
            return false;

        // Allocate a new scene rendering view. This represents a collection of
        // surfaces / render targets, into which the scene will be rendered. It
        // is possible to create more than one scene view for multiple outputs
        // (perhaps split screen multi player) but in this case we only need one
        // view that spans the entire screen.
        RenderDriver @ renderDriver = getAppRenderDriver();
        @mSceneView = renderDriver.createRenderView( "Sample View", ScaleMode::Relative, RectF(0,0,1,1) );

        // Load the scene
        if ( !loadScene( ) )
            return false;

        // Switch to direct mouse input mode (no cursor)
        InputDriver @ inputDriver = getAppInputDriver();
        inputDriver.setMouseMode( MouseHandlerMode::Direct );

        // Success!
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : end ()
    // Desc : This state is no longer required / running, and should clean up
    //        any allocated resources.
    //-------------------------------------------------------------------------
    void end( )
    {
        // Dispose of scene rendering view
        if ( @mSceneView != null )
            mSceneView.deleteReference();
        @mSceneView = null;

        // Unload the scene
        if ( @mScene != null )
            mScene.unload();
        @mScene = null;

        // Close the world file we opened.
        mWorld.close();
        @mWorld = null;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        // Run the update process if the state is not currently suspended.
        if ( @mScene != null && !mState.isSuspended() )
        {
            // Allow the scene to perform any necessary update tasks.
            mScene.update();

            // If the cursor is not up, handle input to adjust character controller.
            InputDriver @ inputDriver = getAppInputDriver();
            CharacterController @ controller = cast<CharacterController>(mPlayer.getPhysicsController());
            if ( inputDriver.getMouseMode() != MouseHandlerMode::Cursor )
            {
                // Zoom when holding middle mouse button.
                if ( inputDriver.isMouseButtonPressed( MouseButtons::Middle ) )
                    mCamera.setFOV( 20.0f );
                else
                    mCamera.setFOV( 85.0f );

                // If the character is already airborne and the user presses space,
                // switch to 'fly mode'.
                if ( controller.getCharacterState() == CharacterState::Airborne && inputDriver.isKeyPressed( Keys::Space, true ) )
                    controller.enableFlyMode( true, true );

                // Update player controller details in response to key presses.
                // Here we're handling player crouching by responding to control key presses.
                if ( !controller.isFlyModeEnabled() &&  inputDriver.isKeyPressed( Keys::LControl ) )
                {
                    // Here we request that the character controller be switched
                    // to 'Crouch' mode and reduce the maximum speed of the character
                    // to a slower pace.
                    controller.requestStandingMode( CharacterStandingMode::Crouching );
                    controller.setMaximumWalkSpeed( 1.34112f * 2 );
                
                } // End if LControl
                else
                {
                    // Request that the character controller switch to a standing
                    // mode. This is a request only since there is no guarantee that 
                    // it will immediately be able to switch if, for instance, there is
                    // solid geometry above the player's head stopping this from happening.
                    // The controller will however switch as soon as there is room for the
                    // character to stand.
                    controller.requestStandingMode( CharacterStandingMode::Standing );

                    // Set running or walking speed if holding shift.
                    if ( inputDriver.isKeyPressed( Keys::LShift ) )
                        controller.setMaximumWalkSpeed( 1.34112f * 2 );
                    else
                        controller.setMaximumWalkSpeed( 1.34112f * 4 );
                
                } // End if !LControl

            } // End if no cursor

            // Here we'll add a little 'head bob' effect to the camera. This could
            // potentially be done in the player's behavior script, but we'll do it
            // right in line here to keep things simple for now. First compute the 
            // camera's required vertical offset from its parent player based on the
            // height of the character.
            float cameraOffset = controller.getCharacterHeight( true ) * 0.95f;

            // Compute head bob offsets (we only want it to bob if the character not 
            // currently sliding or airborne)
            Timer @ timer = getAppTimer();
            Vector2 bobOffset(0,0);
            if ( controller.getCharacterState() == CharacterState::OnFloor )
            {
                // Compute the current horizontal speed of the character. This
                // will be used to increase or decrease the rate of the head
                // bob effect as they walk faster / slower.
                Vector3 velocity = controller.getVelocity();
                float characterSpeed = vec3Length( Vector3( velocity.x, 0, velocity.z ) );

                // Advance the 'head bob cycle' based on the speed.
                float bobSpeed = 1.5f * characterSpeed;
                mBobCycle += timer.getTimeElapsed() * bobSpeed;

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

            // Smooth any computed offsets so that we get a more elastic
            // and gradual adjustment as its magnitude changes over time.
            float bobSmoothFactor = 0.05f / timer.getTimeElapsed();
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
            float verticalSmoothFactor, horizontalSmoothFactor = 0.05f / timer.getTimeElapsed();
            if ( controller.getCharacterState() != CharacterState::OnFloor )
                verticalSmoothFactor = 0.03f / timer.getTimeElapsed();
            else
                verticalSmoothFactor = 0.08f / timer.getTimeElapsed();

            // Now smoothly transition camera position frame to frame.
            Vector3 newPosition = mPlayer.getPosition();        
            newPosition.x = smooth( newPosition.x, mLastCamPos.x, horizontalSmoothFactor );
            newPosition.y = smooth( newPosition.y + cameraOffset, mLastCamPos.y, verticalSmoothFactor );
            newPosition.z = smooth( newPosition.z, mLastCamPos.z, horizontalSmoothFactor );
            mLastCamPos = newPosition;

            // Finally apply any head bob offset and set it to the camera.
            newPosition += mCamera.getXAxis() * bobOffset.x;
            newPosition += mCamera.getYAxis() * bobOffset.y;
            mCamera.setPosition( newPosition );
        
        } // End if !suspended

        // Exit event?
        InputDriver @ inputDriver = getAppInputDriver();
        if ( inputDriver.isKeyPressed( Keys::Escape ) )
            mState.raiseEvent( "Exit" );

    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        if ( @mSceneView == null || @mScene == null )
            return;

        // Start rendering to our created scene view (full screen output in this case).
        if ( mSceneView.begin() )
        {
            // Allow the scene to render
            mScene.render();

            // Get the current state of the character (airborne, walking, etc.) in order 
            // to print this information to the screen
            String output = "Walking";
            CharacterController @ controller = cast<CharacterController>(mPlayer.getPhysicsController());
            CharacterState state = controller.getCharacterState();
            if ( state == CharacterState::Airborne )
                output = "Airborne";
            else if ( state == CharacterState::OnRamp )
                output = "Sliding";

            // Append the current 'standing' mode of the character too (i.e. is
            // the character currently crouching, etc.).
            CharacterStandingMode mode = controller.getActualStandingMode();
            if ( mode == CharacterStandingMode::Standing )
                output += "\nStanding";
            else if ( mode == CharacterStandingMode::Crouching )
                output += "\nCrouching";
            else if ( mode == CharacterStandingMode::Prone )
                output += "\nProne";
            
            // Print it to the screen.
            UIManager @ interfaceManager = getAppUIManager();
            Size screenSize = mSceneView.getSize();
            Rect rcScreen( 10, 10, screenSize.width - 10, screenSize.height - 10 );
            interfaceManager.selectFont( "fixed_v01_white" );
            interfaceManager.printText( rcScreen, output, TextFlags::Multiline | TextFlags::AlignRight, 0xFFFF0000 );

            // Display instructions.
            interfaceManager.printText( rcScreen, "\n\n[c=#ffcccccc]<RMB>[/c] to pickup an object. [c=#ffcccccc]<LMB>[/c] to throw.", TextFlags::Multiline | TextFlags::AllowFormatCode, 0xFFFFFFFF, 0, 10 );

            // Present the view to the frame buffer
            mSceneView.end( );

        } // End if begun
    }

    //-------------------------------------------------------------------------
    // Name : loadScene () (Private)
    // Desc : Contains code responsible for loading the main scene.
    //-------------------------------------------------------------------------
    private bool loadScene( )
    {
        // Load the first scene from the file.
        @mScene = mWorld.loadScene( 0x1 );
        if ( @mScene == null )
            return false;

        // Create a dummy object to represent the player
        @mPlayer = mScene.createObjectNode( true, RTID_DummyObject, false );

        // We want the objects update process to execute every frame so that
        // its behavior script (assigned in a moment) will get a chance to execute.
        mPlayer.setUpdateRate( UpdateRate::Always );

        // Assign a character controller to the player.
        CharacterController @ controller = CharacterController( mScene.getPhysicsWorld() );
        mPlayer.setPhysicsController( controller );

        // Allow the controller to initialize.
        controller.setCharacterRadius( 0.4f );
        controller.initialize( );
        
        // Position the player in the world.
        mPlayer.setPosition( Vector3(0.0f, 0.1f, -4.0f) );
        
        // Now create a camera that can be attached to the player object.
        @mCamera = cast<CameraNode>(mScene.createObjectNode( true, RTID_CameraObject, false ));
        
        // Setup camera properties
        mCamera.setFOV( 85.0f );
        mCamera.setNearClip( 0.2f );
        mCamera.setFarClip( 10000.01f );
        mCamera.setUpdateRate( UpdateRate::Always );

        // Offset the camera to "eye" level based on the configured height
        // of the character controller prior to attaching to the player.
        mLastCamPos    = mPlayer.getPosition();
        mLastCamPos.y += controller.getCharacterHeight() * 0.95f;
        mCamera.setPosition( mLastCamPos );
        
        // Attach the camera as a child of the player object.
        mCamera.setParent( mPlayer );
        
        // Assign necessary "input" behavior to the player object. In this case 
        // we assign the 'Player.gs' behavior to the player which provides
        // keyboard and mouse input to control the player and its child camera.
        ObjectBehavior @ behavior = ObjectBehavior( );
        behavior.initialize( mScene.getResourceManager(), "Scripts/Behaviors/Player.gs", "" );
        mPlayer.addBehavior( behavior );
        
        // Set up the scene ready for rendering
        mScene.setActiveCamera( mCamera );

        // Success!
        return true;
    }
};