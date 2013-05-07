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
// Name : Main.gs                                                            //
//                                                                           //
// Desc : Script containing the top level application management state whose //
//        primary purpose is to manage overall application flow.             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : Main (Class)
// Desc : Scripted application state object used as the main entry point for
//        this framework demonstration.
//-----------------------------------------------------------------------------
class Main : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@       mState;             // Application side state object
    
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
        // Construct the application state hierarchy for this app.
        if ( !constructAppStates() )
            return false;

        // Configure audio driver.
        getAppAudioDriver().setTrackFadeTimes( 4.0f, 4.0f );

        // Spawn the main menu state initially.
        mState.spawnChildState( "MainMenu", false );
        //mState.spawnChildState( "GamePlay", false );

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
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
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
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : constructAppStates () (Private)
    // Desc : Construct the application state hierarchy for this app.
    //-------------------------------------------------------------------------
    private bool constructAppStates( )
    {
        AppStateManager @ manager = mState.getManager();

        // Construct a new application state for the main menu.
        AppState @ mainMenuState = AppState( "MainMenu", "Scripts/ColonyVI/States/MainMenu.gs", getAppResourceManager() );
        manager.registerState( mainMenuState );

        // Register events for main menu
        AppStateEventActionDesc desc;
        desc.actionType = AppStateEventActionType::Transition;
        desc.flags      = 0;
        desc.toStateId  = "GamePlay";
        mainMenuState.registerEventAction( "New Game", desc );

        // Register events for main menu
        desc.actionType = AppStateEventActionType::EndRoot;
        desc.flags      = 0;
        desc.toStateId  = "";
        mainMenuState.registerEventAction( "Exit", desc );

        // Construct a new application state for the main game play.
        AppState @ gamePlayState = AppState( "GamePlay", "Scripts/ColonyVI/States/GamePlay.gs", getAppResourceManager() );
        manager.registerState( gamePlayState );

        // Register events for game play
        desc.actionType = AppStateEventActionType::EndRoot;
        desc.flags      = 0;
        desc.toStateId  = "";
        gamePlayState.registerEventAction( "Exit", desc );

        // Register events for game play
        desc.actionType = AppStateEventActionType::Transition;
        desc.flags      = 0;
        desc.toStateId  = "GamePlay";
        gamePlayState.registerEventAction( "Restart", desc );
        
        // Success!
        return true;
    }
};