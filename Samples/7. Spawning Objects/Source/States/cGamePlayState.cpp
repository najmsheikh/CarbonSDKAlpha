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
// Name : cGamePlayState.cpp                                                 //
//                                                                           //
// Desc : Primary game state used to supply most functionality for the       //
//        demonstration project.                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// cGamePlayState Module Includes
//-----------------------------------------------------------------------------
#include "States/cGamePlayState.h"

//-----------------------------------------------------------------------------
// Name : cGamePlayState () (Constructor)
/// <summary> cGamePlayState Class Constructor </summary>
//-----------------------------------------------------------------------------
cGamePlayState::cGamePlayState( const cgString & stateId ) : cgAppState( stateId )
{
    // Initialize variables to sensible defaults
    mScene          = CG_NULL;
    mSceneView      = CG_NULL;
    mLastCamPos     = cgVector3(0,0,0);
    mBobCycle       = 0.0f;
    mLastBobOffset  = cgVector2(0,0);
    mForm           = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~cGamePlayState () (Destructor)
/// <summary> cGamePlayState Class Destructor </summary>
//-----------------------------------------------------------------------------
cGamePlayState::~cGamePlayState()
{
}

//-----------------------------------------------------------------------------
// Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy supports a
/// particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Sample_GamePlayState )
        return true;

    // Supported by base?
    return cgAppState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : initialize () (Virtual)
/// <summary>
/// The state has been registered and is being initialized. This function is 
/// somewhat like a constructor in that it is now part of the game state system
/// and any registration-time processing can take place.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::initialize( )
{
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : begin () (Virtual)
/// <summary>
/// This signifies that the state has actually been selected and activated by 
/// the state management system. This will generally be the point at which any 
/// specific resources relevant for the execution of this state will be 
/// built/loaded.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::begin( )
{
    // Allocate a new scene rendering view. This represents a collection of
    // surfaces / render targets, into which the scene will be rendered. It
    // is possible to create more than one scene view for multiple outputs
    // (perhaps split screen multi player) but in this case we only need one
    // view that spans the entire screen.
    cgRenderDriver * renderDriver = cgRenderDriver::getInstance();
    if ( !(mSceneView = renderDriver->createRenderView( _T("Sample View"), cgScaleMode::Relative, cgRectF(0,0,1,1) ) ) )
        return false;

    // Load the scene
    if ( !loadScene( ) )
        return false;

    // Load the main form that will display the options for
    // spawning objects (hidden initially).
    cgUIManager * interfaceManager = cgUIManager::getInstance();
    if ( !(mForm = interfaceManager->loadForm( _T("appdir://Scripts/7. Spawning Objects/SpawnObjects.frm"), _T("spawningForm") ) ) )
        return false;
    mForm->setVisible(false);

    // Pass the active scene to the form script so that it knows into 
    // which scene objects should be spawned.
    cgScriptArgument::Array arguments;
    cgScriptObject * formScript = mForm->getScriptObject();
    arguments.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("Scene@+"), (void*)mScene ) );
    formScript->executeMethodVoid( _T("setScene"), arguments, true );
    arguments[0] = cgScriptArgument( cgScriptArgumentType::Object, _T("CameraNode@+"), (void*)mCamera );
    formScript->executeMethodVoid( _T("setPlayerCamera"), arguments, true );
    
    // Switch to direct mouse input mode (no cursor)
    cgInputDriver * inputDriver = cgInputDriver::getInstance();
    inputDriver->setMouseMode( cgMouseHandlerMode::Direct );

    // Call base class implementation last.
    return cgAppState::begin();
}

//-----------------------------------------------------------------------------
// Name : End () (Virtual)
// Desc : This state is no longer required / running, and should clean up any
//        allocated resources.
//-----------------------------------------------------------------------------
void cGamePlayState::end( )
{
    // Dispose of scene rendering view
    if ( mSceneView )
        mSceneView->deleteReference();
    mSceneView = CG_NULL;

    // Unload the scene
    if ( mScene )
        mScene->unload();
    mScene = CG_NULL;

    // Call base class implementation
    cgAppState::end();
}

//-----------------------------------------------------------------------------
// Name : loadScene () (Private)
/// <summary>
/// Contains code responsible for loading the main scene.
/// </summary>
//-----------------------------------------------------------------------------
bool cGamePlayState::loadScene( )
{
    // Get access to required systems.
    cgWorld * world = cgWorld::getInstance();

    // Load the first scene from the file.
    if ( !(mScene = world->loadScene( 0x1 )) )
        return false;

    // Create a dummy object to represent the player
    mPlayer = static_cast<cgDummyNode*>(mScene->createObjectNode( true, RTID_DummyObject, false ));

    // We want the object's update process to execute every frame so that
    // its behavior script (assigned in a moment) will get a chance to execute.
    mPlayer->setUpdateRate( cgUpdateRate::Always );

    // Assign a character controller to the player.
    cgCharacterController * controller;
    controller = static_cast<cgCharacterController*>(cgPhysicsController::createInstance( _T("Core::PhysicsControllers::Character"), mScene->getPhysicsWorld() ));
    mPlayer->setPhysicsController( controller );

    // Allow the controller to initialize.
    controller->setCharacterRadius( 0.4f );
    controller->initialize( );
    
    // Position the player in the world.
    mPlayer->setPosition( cgVector3(0.0f, 0.1f, -20.0f) );
    //mPlayer->setPosition( cgVector3(0.0f, 2.8f, -2.0f) );
    
    // Now create a camera that can be attached to the player object.
    mCamera = static_cast<cgCameraNode*>(mScene->createObjectNode( true, RTID_CameraObject, false ));
    
    // Setup camera properties
    mCamera->setFOV( 85.0f );
    mCamera->setNearClip( 0.2f );
    mCamera->setFarClip( 10000.01f );
    mCamera->setUpdateRate( cgUpdateRate::Always );

    // Offset the camera to "eye" level based on the configured height
    // of the character controller prior to attaching to the player.
    mLastCamPos    = mPlayer->getPosition();
    mLastCamPos.y += controller->getCharacterHeight() * 0.95f;
    mCamera->setPosition( mLastCamPos );
    
    // Attach the camera as a child of the player object.
    mCamera->setParent( mPlayer );
    
    // Spawn the first person player actor (Reference Id: 0x16D) and attach it to the camera.
    cgActorNode * firstPersonActor = static_cast<cgActorNode*>(mScene->loadObjectNode( 0x16D, cgCloneMethod::ObjectInstance, true ));
    firstPersonActor->setWorldTransform( mCamera->getWorldTransform() );
    firstPersonActor->setParent( mCamera );

    // Assign necessary behavior to the player object. In this case we assign the 'Player.gs' behavior
    // to the player object which provides keyboard and mouse input to control the player and its child
    // camera. We assign the behavior *after* having attached the camera and first person actor so that
    // references to them can be found during the call to the script object's 'onAttach()' method.
    cgObjectBehavior * pBehavior = new cgObjectBehavior( );
    pBehavior->initialize( mScene->getResourceManager(), _T("Scripts/Behaviors/Player.gs"), _T("") );
    mPlayer->addBehavior( pBehavior );
    
    // Set up the scene ready for rendering
    mScene->setActiveCamera( mCamera );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : update () (Virtual)
/// <summary>
/// Called by the game state manager in order to allow this state (and all 
/// other states) to perform any processing in its entirety prior to the 
/// rendering process beginning.
/// </summary>
//-----------------------------------------------------------------------------
void cGamePlayState::update( )
{
    // Run the update process if the state is not currently suspended.
    if ( mScene && !isSuspended() )
    {
        // Allow the scene to perform any necessary update tasks.
        mScene->update();

        // Toggle form display when pressing tab. By specifying true for the second 
        // parameter, the 'isKeyPressed()' method will only return a positive result
        // if the key was not previously pressed in the prior frame.
        cgInputDriver * inputDriver = cgInputDriver::getInstance();
        if ( inputDriver->isKeyPressed( cgKeys::Tab, true ) )
        {
            // Toggle visibility of spawn form.
            mForm->setVisible( !mForm->isVisible() );

            // Enable cursor mode if form is visible.
            inputDriver->setMouseMode( (mForm->isVisible()) ? cgMouseHandlerMode::Cursor : cgMouseHandlerMode::Direct );
        
        } // End if pressed tab

        // If the cursor is not up, handle input to adjust character controller.
        cgCharacterController * controller = static_cast<cgCharacterController*>(mPlayer->getPhysicsController());
        if ( inputDriver->getMouseMode() != cgMouseHandlerMode::Cursor )
        {
            // Zoom when holding middle mouse button.
            if ( inputDriver->isMouseButtonPressed( cgMouseButtons::Middle ) )
                mCamera->setFOV( 20.0f );
            else
                mCamera->setFOV( 85.0f );

            // If the character is already airborne and the user presses space,
            // switch to 'fly mode'.
            if ( controller->getCharacterState() == cgCharacterController::Airborne && inputDriver->isKeyPressed( cgKeys::Space, true ) )
                controller->enableFlyMode( true, true );
            
            // Update player controller details in response to key presses.
            // Here we're handling player crouching by responding to control key presses.
            if ( !controller->isFlyModeEnabled() && inputDriver->isKeyPressed( cgKeys::LControl ) )
            {
                // Here we request that the character controller be switched
                // to 'Crouch' mode and reduce the maximum speed of the character
                // to a slower pace.
                controller->requestStandingMode( cgCharacterController::Crouching );
                controller->setMaximumWalkSpeed( 1.34112f * 2 );
            
            } // End if LControl
            else
            {
                // Request that the character controller switch to a standing
                // mode. This is a request only since there is no guarantee that 
                // it will immediately be able to switch if, for instance, there is
                // solid geometry above the player's head stopping this from happening.
                // The controller will however switch as soon as there is room for the
                // character to stand.
                controller->requestStandingMode( cgCharacterController::Standing );

                // Set running or walking speed if holding shift.
                if ( inputDriver->isKeyPressed( cgKeys::LShift ) )
                    controller->setMaximumWalkSpeed( 1.34112f * 2 );
                else
                    controller->setMaximumWalkSpeed( 1.34112f * 4 );
            
            } // End if !LControl

        } // End if no cursor

        // Here we'll add a little 'head bob' effect to the camera. This could
        // potentially be done in the player's behavior script, but we'll do it
        // right in line here to keep things simple for now. First compute the 
        // camera's required vertical offset from its parent player based on the
        // height of the character.
        cgFloat cameraOffset = controller->getCharacterHeight( true ) * 0.95f;

        // Compute head bob offsets (we only want it to bob if the character not 
        // currently sliding or airborne)
        cgTimer * timer = cgTimer::getInstance();
        cgVector2 bobOffset(0,0);
        if ( controller->getCharacterState() == cgCharacterController::OnFloor )
        {
            // Compute the current horizontal speed of the character. This
            // will be used to increase or decrease the rate of the head
            // bob effect as they walk faster / slower.
            const cgVector3 & velocity = controller->getVelocity();
            cgFloat characterSpeed = cgVector3::length( cgVector3( velocity.x, 0, velocity.z ) );

            // Advance the 'head bob cycle' based on the speed.
            cgFloat bobSpeed = 1.5f * characterSpeed;
            mBobCycle += timer->getTimeElapsed() * bobSpeed;

            // Based on the current cycle of the head bob, compute the amount by 
            // which we want to offset the camera along its local X and Y axes
            // when we compute its final position.
            const cgVector2 bobScale( 0.02f, 0.015f );
            bobOffset.x = (cgFloat)sin( mBobCycle ) * characterSpeed  * bobScale.x;
            bobOffset.y = (cgFloat)sin( 2 * mBobCycle ) * characterSpeed * bobScale.y;

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
        cgFloat bobSmoothFactor = 0.05f / timer->getTimeElapsed();
        bobOffset.x = cgMathUtility::smooth( bobOffset.x, mLastBobOffset.x, bobSmoothFactor );
        bobOffset.y = cgMathUtility::smooth( bobOffset.y, mLastBobOffset.y, bobSmoothFactor );
        mLastBobOffset = bobOffset;
        
        // Now we've computed the offsets we need for the headbob, it's time to
        // compute the final location of the camera. We apply a bit of smoothing here
        // too so that the camera acts like it's on a 'spring' of sorts. This produces
        // a more fluid camera motion that allows the player to -- for instance -- walk
        // up stairs without the camera bouncing up each step quite so dramatically.
        // We'll start by selecting the smoothing amounts we want on each axis
        // independently.
        cgFloat verticalSmoothFactor, horizontalSmoothFactor = 0.05f / timer->getTimeElapsed();
        if ( controller->getCharacterState() != cgCharacterController::OnFloor )
            verticalSmoothFactor = 0.03f / timer->getTimeElapsed();
        else
            verticalSmoothFactor = 0.08f / timer->getTimeElapsed();

        // Now smoothly transition camera position frame to frame.
        cgVector3 newPosition = mPlayer->getPosition();        
        newPosition.x = cgMathUtility::smooth( newPosition.x, mLastCamPos.x, horizontalSmoothFactor );
        newPosition.y = cgMathUtility::smooth( newPosition.y + cameraOffset, mLastCamPos.y, verticalSmoothFactor );
        newPosition.z = cgMathUtility::smooth( newPosition.z, mLastCamPos.z, horizontalSmoothFactor );
        mLastCamPos = newPosition;

        // Finally apply any head bob offset and set it to the camera.
        newPosition += mCamera->getXAxis() * bobOffset.x;
        newPosition += mCamera->getYAxis() * bobOffset.y;
        mCamera->setPosition( newPosition );
        
        // Apply a 'counter' bob to the child of the camera (first person weapon)
        cgObjectNode * childActor = mCamera->findChildOfType( RTID_ActorObject, false );
        if ( childActor )
        {
            cgFloat counterBobStrength = 0.1f;
            childActor->setPosition( mCamera->getPosition() );
            childActor->moveLocal( -bobOffset.x * counterBobStrength, bobOffset.y * counterBobStrength, 0 );
        
        } // End if has child
    
    } // End if !suspended

    // Exit event?
    cgInputDriver * inputDriver = cgInputDriver::getInstance();
    if ( inputDriver->isKeyPressed( cgKeys::Escape ) )
        raiseEvent( _T("Exit") );

    // Call base class implementation
    cgAppState::update();
}

//-----------------------------------------------------------------------------
// Name : render () (Virtual)
// Desc : Called by the game state manager in order to allow this state (and
//        all other states) to render whatever is necessary.
//-----------------------------------------------------------------------------
void cGamePlayState::render( )
{
    if ( mSceneView )
    {
        // Start rendering to our created scene view (full screen output in this case).
        if ( mSceneView->begin() )
        {
            // Allow the scene to render if one was loaded
            if ( mScene )
            {
                mScene->render();

                // Get the current state of the character (airborne, walking, etc.) in order 
                // to print this information to the screen
                cgString output = _T("Walking");
                cgCharacterController * controller = static_cast<cgCharacterController*>(mPlayer->getPhysicsController());
                cgCharacterController::CharacterState state = controller->getCharacterState();
                if ( state == cgCharacterController::Airborne )
                    output = _T("Airborne");
                else if ( state == cgCharacterController::OnRamp )
                    output = _T("Sliding");

                // Append the current 'standing' mode of the character too (i.e. is
                // the character currently crouching, etc.).
                cgCharacterController::StandingMode mode = controller->getActualStandingMode();
                if ( mode == cgCharacterController::Standing )
                    output += _T("\nStanding");
                else if ( mode == cgCharacterController::Crouching )
                    output += _T("\nCrouching");
                else if ( mode == cgCharacterController::Prone )
                    output += _T("\nProne");
                
                // Print it to the screen.
                cgUIManager * interfaceManager = cgUIManager::getInstance();
                cgSize screenSize = mSceneView->getSize();
                cgRect rcScreen( 10, 10, screenSize.width - 10, screenSize.height - 10 );
                interfaceManager->selectFont( _T("fixed_v01_white") );
                interfaceManager->printText( rcScreen, output, cgTextFlags::Multiline | cgTextFlags::AlignRight, 0xFFFF0000 );

                // Render a basic crosshair
                cgRenderDriver * renderDriver = mScene->getRenderDriver();
                cgFloat halfWidth = screenSize.width / 2.0f, halfHeight = screenSize.height / 2.0f;
                cgVector2 crosshair[8] = {
                    cgVector2( halfWidth - 8.0f, halfHeight ), cgVector2( halfWidth - 3.0f, halfHeight ),
                    cgVector2( halfWidth + 3.0f, halfHeight ), cgVector2( halfWidth + 8.0f, halfHeight ),
                    cgVector2( halfWidth, halfHeight - 8.0f ), cgVector2( halfWidth, halfHeight - 3.0f ),
                    cgVector2( halfWidth, halfHeight + 3.0f ), cgVector2( halfWidth, halfHeight + 8.0f )
                };  
                renderDriver->drawLines( crosshair, 4, 0xAAFFFFFF );

            } // End if scene loaded

            // Present the view to the frame buffer
            mSceneView->end( );

        } // End if begun

    } // End if valid view

    // Call base class implementation
    cgAppState::render();
}