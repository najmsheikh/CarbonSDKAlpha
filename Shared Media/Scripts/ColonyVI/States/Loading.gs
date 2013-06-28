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
// Name : Loading.gs                                                          //
//                                                                           //
// Desc : Script containing the logic for rendering a load screen.           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name : Loading (Class)
// Desc : Script containing the logic for rendering a load screen.
//-----------------------------------------------------------------------------
shared class Loading : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@               mState;             // Application side state object
    private RenderView@             mView;              // Render view into which the menu will be rendered.
    private float                   mBeginTime;
    
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
        // Allocate a new scene rendering view. This represents a collection of
        // surfaces / render targets, into which the scene will be rendered. It
        // is possible to create more than one scene view for multiple outputs
        // (perhaps split screen multi player) but in this case we only need one
        // view that spans the entire screen.
        RenderDriver @ renderDriver = getAppRenderDriver();
        @mView = renderDriver.createRenderView( "Loading View", ScaleMode::Relative, RectF(0,0,1,1) );

        // Listen for scene loading messages.
        subscribeToGroup( mState.getReferenceId(), MGID_Scene );

        // Record time at which loading began.
        mBeginTime = getAppTimer().getTime( true );

        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.addImage( InputStream("Textures/UI/Loading Backdrop.dds"), "LoadingStateBackground" );

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
        if ( @mView != null )
            mView.deleteReference();

        // Unload resources
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.removeImageLibrary( "LoadingStateBackground" );

        // We should no longer listen for scene loading messages.
        unsubscribeFromGroup( mState.getReferenceId(), MGID_Scene );

        // Release references
        @mView = null;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
    }

    //-------------------------------------------------------------------------
    // Name : render ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to render whatever is necessary.
    //-------------------------------------------------------------------------
    void render( )
    {
        if ( @mView == null )
            return;

        // Get access to required properties
        UIManager @ interfaceManager = getAppUIManager();
        RenderDriver @ renderDriver = getAppRenderDriver();
        Size screenSize = mView.getSize();

        // Start drawing.
        renderDriver.beginFrame( true );
        
        // Start rendering to our created view (full screen output in this case).
        if ( mView.begin() )
        {
            int segments = 80;

            // Render vector menu overlay into anti-aliased buffer to avoid jaggys.
            // Allocate a target (if one is not already available).
            // TODO: Pick highest available MSAA option rather than hard-coding!
            RenderTargetHandle target = mView.getRenderSurface( BufferFormat::B8G8R8A8, MultiSampleType::EightSamples, 0 );

            // Begin rendering into the multi-sampled buffer.
            renderDriver.beginTargetRender( target, null );

            // Fill background.
            renderDriver.drawRectangle( Rect( 0, 0, screenSize.width, screenSize.height ), ColorValue(0xFF111111), true );
            interfaceManager.drawImage( ImageScaleMode::Stretch, "LoadingStateBackground" );

            // Compute common values.
            float currentTime = getAppTimer().getTime( true );
            float scaleFactor = screenSize.width / 1280.0f;
            Vector2 centerPos( screenSize.width * 0.5f, screenSize.height * 0.5f );
            
            float outerRadius = 300 * scaleFactor;
            float thickness = 32 * scaleFactor;
            float arcBegin = ((currentTime - mBeginTime) * 80.0f);
            float arcEnd   = arcBegin + 320.0f;
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x5F000000 ), thickness * 1.3f, segments, arcBegin - 1, arcEnd + 1 );
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x8Ab5b5b5 ), thickness, segments, arcBegin, arcEnd );

            outerRadius = 260 * scaleFactor;
            thickness = 16 * scaleFactor;
            arcBegin = -((currentTime - mBeginTime) * 50.0f) + 190.0f;
            arcEnd   = arcBegin + 320.0f;
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x5F000000 ), thickness * 1.6f, segments, arcBegin - 1, arcEnd + 1 );
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x7Ab5b5b5 ), thickness, segments, arcBegin, arcEnd );

            outerRadius = 230 * scaleFactor;
            thickness = 8 * scaleFactor;
            arcBegin = ((currentTime - mBeginTime) * 60.0f) + 45.0f;
            arcEnd   = arcBegin + 320.0f;
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x5F000000 ), thickness * 1.9f, segments, arcBegin - 1, arcEnd + 1 );
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x6Ab5b5b5 ), thickness, segments, arcBegin, arcEnd );

            // Render large semi-off-screen circle
            outerRadius = 700 * scaleFactor;
            thickness = 200 * scaleFactor;
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x3Ab5b5b5 ), thickness, 100 );

            // We're done rendering into the multi-sampled buffer, copy it
            // back into the view's main buffer.
            renderDriver.endTargetRender( );
            renderDriver.stretchRect( target, mView.getViewBuffer() );

            // Display loading text.
            interfaceManager.selectFont( "Nasalization" );
            interfaceManager.printText( Rect( 0, 0, screenSize.width, screenSize.height ), "Loading...", TextFlags::AlignCenter | TextFlags::VAlignCenter, 0xAAFFFFFF );
            interfaceManager.selectDefaultFont();

            // Present the view to the frame buffer
            mView.end( );

        } // End if begun

        // Complete drawing.
        renderDriver.endFrame( );
    }

    //-------------------------------------------------------------------------
    // Name : processMessage ()
    // Desc : Handles application / system message processing for the class
    //-------------------------------------------------------------------------
   	bool processMessage( Message & msg )
	{
		// Handle messages sent from self
		if ( msg.messageId == SystemMessages::Scene_LoadProgressUpdate )
		{
            render();

            // We processed the message
			return true;
			
		} // End if load progress

		// We did NOT handle this message
		return false;
	}
    
};