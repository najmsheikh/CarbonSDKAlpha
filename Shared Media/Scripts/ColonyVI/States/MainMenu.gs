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
// Name : MainMenu.gs                                                        //
//                                                                           //
// Desc : Script containing the top level logic for the main game menu.      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "../Forms/Options/VideoOptionsForm.frm"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared class MenuOptionData
{
    Vector2 position;
    Rect    bounds;
    float   angleOffset;
    String  libraryItem;
    bool    hovering;
    float   lastHoverTime;
};

//-----------------------------------------------------------------------------
// Name : MainMenu (Class)
// Desc : Script containing the top level logic for the main game menu.
//-----------------------------------------------------------------------------
shared class MainMenu : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private array<MenuOptionData@>  mOptions;
    private AppState@               mState;             // Application side state object
    private RenderView@             mView;              // Render view into which the menu will be rendered.
    private Scene@                  mBackgroundScene;   // Scene that we're rendering in the background.
    private CameraNode@             mBackgroundCamera;  // The camera used to render the background scene.
    private ObjectNode@             mBackgroundPlanet;
    private Form@                   mMainForm;
    
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
        // Setup menu options.
        mOptions.resize(3);

        // Play demo
        MenuOptionData @data = MenuOptionData();
        data.libraryItem    = "play_demo";
        data.angleOffset    = 280;
        data.hovering       = false;
        data.lastHoverTime  = -100;
        @mOptions[0]        = data;

        // Options
        @data               = MenuOptionData();
        data.libraryItem    = "options";
        data.angleOffset    = 270;
        data.hovering       = false;
        data.lastHoverTime  = -100;
        @mOptions[1]        = data;

        // Exit
        @data               = MenuOptionData();
        data.libraryItem    = "exit_game";
        data.angleOffset    = 260;
        data.hovering       = false;
        data.lastHoverTime  = -100;
        @mOptions[2]        = data;

        // Load the scene that we'll be displaying in the background.
        @mBackgroundScene = getAppWorld().loadScene( 0x8 );
        if ( @mBackgroundScene == null )
            return false;

        // Create a camera through which we will view the background scene.
        @mBackgroundCamera = cast<CameraNode>(mBackgroundScene.createObjectNode( true, RTID_CameraObject, false ));
        mBackgroundCamera.setFOV( 60.0f );
        mBackgroundCamera.setNearClip( 0.2f );
        mBackgroundCamera.setFarClip( 10000.01f );
        mBackgroundCamera.setPosition( -180, 50, -150 );
        mBackgroundCamera.rotate( 0, 20, 15 );
        mBackgroundScene.setActiveCamera( mBackgroundCamera );

        // Get the planet object so that we can animate it.
        @mBackgroundPlanet = mBackgroundScene.getObjectNodeById( 0xC7E );
        mBackgroundPlanet.rotateLocal( 0, 165, 0 );

        // Load the image elements we need to display the main menu.
        UIManager @ interfaceManager = getAppUIManager();
        interfaceManager.addImageLibrary( "Textures/UI/MainMenuElements.xml", "MainMenuElements" );

        // Add fonts for use by forms.
        interfaceManager.addFont( "Textures/UI/Fonts/ScreenFont_020.fnt" );

        // Allocate a new scene rendering view. This represents a collection of
        // surfaces / render targets, into which the scene will be rendered. It
        // is possible to create more than one scene view for multiple outputs
        // (perhaps split screen multi player) but in this case we only need one
        // view that spans the entire screen.
        RenderDriver @ renderDriver = getAppRenderDriver();
        @mView = renderDriver.createRenderView( "Menu View", ScaleMode::Relative, RectF(0,0,1,1) );

        // Open main game menu.
        //openMainMenu();

        // Play menu music.
        getAppAudioDriver().loadAmbientTrack( "Music", "Music/The Descent.ogg", 0.3f, 0.3f );
        // Mechanolith
        // The Descent
        // Mistake the Getaway

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
        interfaceManager.removeImageLibrary( "MainMenuElements" );

        // Stop music.
        getAppAudioDriver().stopAmbientTrack( "Music" );

        // Release references.
        @mView     = null;
        @mMainForm = null;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        if ( !mState.isSuspended() )
        {
            InputDriver @ input = getAppInputDriver();
            Timer @ timer = getAppTimer();
            float currentTime = timer.getTime( false );
            
            // Animate background scene
            mBackgroundPlanet.rotateLocal( 0, 1 * timer.getTimeElapsed(), 0 );

            // Process options.
            if ( @mMainForm == null )
            {
                Point cursorPos = getAppInputDriver().getCursorPosition();
                for ( uint i = 0; i < mOptions.length(); ++i )
                {
                    MenuOptionData @ option = mOptions[i];

                    // Test to see if the cursor is contained
                    option.hovering = false;
                    if ( option.bounds.containsPoint( cursorPos ) )
                    {
                        option.hovering = true;
                        option.lastHoverTime = currentTime;

                        // Releasing a click (this should only ideally be done if the click
                        // was first registered in this button, but this works and feels nice for now)?
                        if ( input.wasMouseButtonPressed( MouseButtons::Left ) && !input.isMouseButtonPressed( MouseButtons::Left ) )
                        {
                            switch ( i )
                            {
                                case 0:
                                    beginNewGame();
                                    break;

                                case 1:
                                    openVideoOptions();
                                    break;

                                case 2:
                                    exit();
                                    break;
                            
                            } // End switch

                        } // End if clicking

                    } // End if contains

                } // Next option

            } // End if form not open

        } // End if !suspended
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
        Timer @ timer = getAppTimer();
        float currentTime = timer.getTime( false );
        Size screenSize = mView.getSize();
        
        // Start rendering to our created view (full screen output in this case).
        if ( mView.begin() )
        {
            // Draw the main background scene.
            mBackgroundScene.render();

            // Draw black bar at the top of the screen.
            int barHeight = screenSize.height / 9;
            Rect topBarRect( 0, 0, screenSize.width, barHeight );
            renderDriver.drawRectangle( topBarRect, ColorValue( 0xFF000000 ), true );

            // Draw black bar at the bottom of the screen.
            Rect bottomBarRect( 0, screenSize.height - barHeight, screenSize.width, screenSize.height );
            renderDriver.drawRectangle( bottomBarRect, ColorValue( 0xFF000000 ), true );

            // Draw the game title text
            //Size titleSize = interfaceManager.getImageSize( "MainMenuElements", "title_text" );
            Size titleSize = interfaceManager.getImageSize( "MainMenuElements", "carbon_title_text" );
            RectF titleRect( 0.6f, 0, 0.34f, 0.89f ); // Left, Automatic, Width, Bottom
            //RectF titleRect( 0.76f, 0, 0.24f, 0.89f ); // Left, Automatic, Width, Bottom
            titleRect.left *= screenSize.width;
            titleRect.right = titleRect.left + titleRect.right * screenSize.width;
            titleRect.bottom *= screenSize.height;
            titleRect.top = titleRect.bottom - ((titleRect.right - titleRect.left) * (float(titleSize.height) / float(titleSize.width)));
            //titleRect.left -= 520;
            //titleRect.right -= 520;
            //interfaceManager.drawImage( titleRect, "MainMenuElements", "title_text", ColorValue(0xFAFFFFFF), true );
            interfaceManager.drawImage( titleRect, "MainMenuElements", "carbon_title_text", ColorValue(0xFAFFFFFF), true );

            // Draw the menu page text
            Rect titleBackRect( 16, topBarRect.bottom + 6, 3000, topBarRect.bottom + 45 );
            renderDriver.drawRectangle( titleBackRect, ColorValue( 0x55000000 ), true );
            Rect barRect( 20, topBarRect.bottom + 10, 40, topBarRect.bottom + 40 );
            renderDriver.drawRectangle( barRect, ColorValue( 0xFAFFFFFF ), false );
            barRect = Rect( 22, topBarRect.bottom + 12, 39, topBarRect.bottom + 39 );
            renderDriver.drawRectangle( barRect, ColorValue( 0xEAFFFFFF ), true );
            interfaceManager.selectFont( "ScreenFont_020" );
            interfaceManager.printText( Point( barRect.right + 10, barRect.top ), "Main Menu", 0, 0xFAFFFFFF );
            interfaceManager.selectDefaultFont();

            // Draw the color tint overlay
            Rect screenRect( 0, 0, screenSize.width, screenSize.height );
            //renderDriver.drawRectangle( screenRect, ColorValue(0x25c3ebff), true );

            // Render vector menu overlay into anti-aliased buffer to avoid jaggys.
            // Allocate a target (if one is not already available), and then copy the
            // currently rendered buffer into it as a background.
            // TODO: Pick highest available MSAA option rather than hard-coding!
            RenderTargetHandle target = mView.getRenderSurface( BufferFormat::B8G8R8A8, MultiSampleType::EightSamples, 0 );
            renderDriver.stretchRect( mView.getViewBuffer(), target );

            // Begin rendering into the multi-sampled buffer.
            renderDriver.beginTargetRender( target, null );

            // Draw large concentric rings.
            float scaleFactor = screenSize.width / 1280.0f;
            float outerRadius = 593 * scaleFactor;
            float innerRadius = 479 * scaleFactor;
            Vector2 centerPos(screenSize.width * 0.56796875f, screenSize.height * 0.465277778f);
            renderDriver.drawCircle( centerPos, outerRadius, ColorValue( 0x4Ab5b5b5 ), 32 * scaleFactor, 100 );
            renderDriver.drawCircle( centerPos, innerRadius, ColorValue( 0x20b5b5b5 ), 103 * scaleFactor, 100 );

            // Process and draw the smaller 'option' circle around the circumference.
            for ( uint i = 0; i < mOptions.length(); ++i )
            {
                MenuOptionData @ option = mOptions[i];
                option.position = Vector2( centerPos.x + sin(CGEToRadian(option.angleOffset)) * outerRadius, centerPos.y - cos(CGEToRadian(option.angleOffset)) * outerRadius );
                renderDriver.drawCircle( option.position, 42 * scaleFactor, ColorValue( (option.hovering) ? 0xE0f1dc7b : 0x80f1dc7b ), 2 * scaleFactor, 100 );
                renderDriver.drawCircle( option.position, 34 * scaleFactor, ColorValue( (option.hovering) ? 0x80f1dc7b : 0x40f1dc7b ), 7 * scaleFactor, 100 );

                // Set the initial option bounds.
                option.bounds.left   = int(option.position.x - 43 * scaleFactor);
                option.bounds.right  = int(option.position.x + 43 * scaleFactor);
                option.bounds.top    = int(option.position.y - 43 * scaleFactor);
                option.bounds.bottom = int(option.position.y + 43 * scaleFactor);

            } // Next option

            // We're done rendering into the multi-sampled buffer, copy it
            // back into the view's main buffer.
            renderDriver.endTargetRender( );
            renderDriver.stretchRect( target, mView.getViewBuffer() );

            // Draw the option area background pattern.
            Rect patternRect;
            Size patternSize = interfaceManager.getImageSize( "MainMenuElements", "option_bg" );
            patternSize.width  = patternSize.width * scaleFactor;
            patternSize.height = patternSize.height * scaleFactor;
            patternRect.left   = mOptions[0].position.x + (44 * scaleFactor);
            patternRect.top    = mOptions[0].position.y - (53 * scaleFactor);
            patternRect.right  = patternRect.left + patternSize.width;
            patternRect.bottom = patternRect.top + patternSize.height;
            interfaceManager.drawImage( patternRect, "MainMenuElements", "option_bg", ColorValue(0xFFFFFFFF), true );

            // Draw the option texts. Start by selecting a scale for the text
            // that will scale it to the closest power of two.
            /*float textScale = 0.3333f;
            textScale = (1.0f / (textScale * scaleFactor));
            textScale = 1.0f / float(closestPowerOfTwo(uint(textScale)));*/
            float textScale = 0.5f;
            if ( screenSize.width < 1024 )
                textScale = 0.25f;

            // Now draw them.
            for ( uint i = 0; i < mOptions.length(); ++i )
            {
                MenuOptionData @ option = mOptions[i];
                
                // Scale the text billboard to the correct size for this resolution.
                Size textSize   = interfaceManager.getImageSize( "MainMenuElements", option.libraryItem );
                textSize.width  = textSize.width * textScale;
                textSize.height = textSize.height * textScale;

                // Generate output rectangle and draw
                Rect textRect;
                textRect.left = option.position.x + (75 * scaleFactor);
                textRect.top  = option.position.y - (textSize.height * 0.33333f);
                textRect.right  = textRect.left + textSize.width;
                textRect.bottom = textRect.top  + textSize.height;
                interfaceManager.drawImage( textRect, "MainMenuElements", option.libraryItem, ColorValue((option.hovering) ? 0xFFf1dc7b : 0xBFFFFFFF), true );

                // Expand option bounds to include the text rectangle.
                option.bounds.right = textRect.right;

                // Display the item selection icon if we have recently hovered.
                float fadeTime = 0.5f;
                float hoverDelta = currentTime - option.lastHoverTime;
                if ( option.hovering || hoverDelta < fadeTime )
                {
                    // Compute location
                    Rect iconRect;
                    Size iconSize = interfaceManager.getImageSize( "MainMenuElements", "select_icon" );
                    iconSize.width  *= scaleFactor * 0.5f;
                    iconSize.height *= scaleFactor * 0.5f;
                    iconRect.left = option.position.x - (iconSize.width / 2.0f);
                    iconRect.top  = option.position.y - (iconSize.height / 2.0f) + 1;
                    iconRect.right  = iconRect.left + iconSize.width;
                    iconRect.bottom = iconRect.top  + iconSize.height;

                    // Draw (fade out if no longer hovering)
                    hoverDelta = (option.hovering) ? 1.0f : 1.0f - (hoverDelta / fadeTime);
                    interfaceManager.drawImage( iconRect, "MainMenuElements", "select_icon", ColorValue( 1, 1, 1, hoverDelta ), true );
                
                } // End if recently hovering

            } // Next option

            // Output copyright info.
            interfaceManager.printText( Point( 20, screenSize.height - 25 ), "Copyright (c) Game Institute - All Rights Reserved", 0, 0xAAFFFFFF );

            // Present the view to the frame buffer
            mView.end( );

        } // End if begun

    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    void beginNewGame( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        mState.raiseEvent( "New Game" );
    }

    void openVideoOptions( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        UIManager @ interfaceManager = getAppUIManager();
        @mMainForm = interfaceManager.loadForm( "Scripts/ColonyVI/Forms/Options/VideoOptionsForm.frm", "frmMain" );
        mMainForm.registerEventHandler( SystemMessages::UI_OnClose, "formClosed", this );
    }

    void exit( )
    {
        mState.raiseEvent( "Exit" );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    private void formClosed( UIControl @ sender )
    {
        // Was a restart requested?
        bool requiresRestart = cast<VideoOptionsForm>(mMainForm.getScriptObject()).requiresRestart;

        // Clean up
        @mMainForm = null;

        // Exit if requested
        if ( requiresRestart )
            exit();
    }
    
};