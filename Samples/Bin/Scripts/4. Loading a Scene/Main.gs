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
// Name : Main.gs (4. Loading a Scene)                                       //
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
    private AppState@   mState;     // Application side state object
    private World@      mWorld;     // Main game world object.
    private Scene@      mScene;     // The scene we've loaded.
    private RenderView@ mSceneView; // Render view into which the scene will be rendered.
    private CameraNode@ mCamera;    // Custom camera object describing the rendered PoV.

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

        // Create a camera that can view the world.
        @mCamera = cast<CameraNode>(mScene.createObjectNode( true, RTID_CameraObject, false ));
        
        // Setup camera properties
        mCamera.setFOV( 75.0f );
        mCamera.setNearClip( 0.2f );
        mCamera.setFarClip( 10000.01f );
        mCamera.setPosition( Vector3(0.0f, 2.0f, -4.0f) );
        mCamera.setUpdateRate( UpdateRate::Always );

        // Set up the scene ready for rendering
        mScene.setActiveCamera( mCamera );

        // Success!
        return true;
    }
};