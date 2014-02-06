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
// Name : InGameMenu.gs                                                      //
//                                                                           //
// Desc : Script containing the top level logic for the in game menu.        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "../Forms/InGameMenuForm.frm"
#include_once "../Forms/Options/VideoOptionsForm.frm"

//-----------------------------------------------------------------------------
// Name : InGameMenu (Class)
// Desc : Script containing the top level logic for the in game menu.
//-----------------------------------------------------------------------------
shared class InGameMenu : IScriptedAppState
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AppState@               mState;             // Application side state object
    private Form@                   mMainForm;
    private bool                    mActivated;
    private bool                    mReopenMenu;
    
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
        // Initialize variables
        mActivated  = false;
        mReopenMenu = false;

        // Open the menu form.
        openMenu();

        // Switch to cursor input.
        getAppInputDriver().setMouseMode( MouseHandlerMode::Cursor );

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
        // Do not re-open the main menu on form close, even if it was pending.
        mReopenMenu = false;

        // Release references.
        if ( @mMainForm != null )
            mMainForm.close();

        // Return to direct input.
        getAppInputDriver().setMouseMode( MouseHandlerMode::Direct );
    }

    //-------------------------------------------------------------------------
    // Name : raiseEvent ()
    // Desc : Called by the game state manager in order to notify us that an
    //        event is being raised on this state.
    //-------------------------------------------------------------------------
    bool raiseEvent( const String & eventId )
    {
        if ( eventId == "Open Options" )
            openVideoOptions();

        // Allow event to continue
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : update ()
    // Desc : Called by the game state manager in order to allow this state
    //        (and all other states) to perform any processing in its entirety
    //        prior to the rendering process.
    //-------------------------------------------------------------------------
    void update( )
    {
        // In the first update call, we are not yet activated.
        if ( !mState.isSuspended() && mActivated )
        {
            // If escape is pressed, just resume parent state.
            if ( getAppInputDriver().isKeyPressed( Keys::Escape, true ) )
                mState.end();

        } // End if activated

        // We are now activated
        mActivated = true;
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
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    void openMenu( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        // Load the UI form.
        UIManager @ interfaceManager = getAppUIManager();
        @mMainForm = interfaceManager.loadForm( "Scripts/ColonyVI/Forms/InGameMenuForm.frm", "frmMain" );
        @cast<InGameMenuForm>(mMainForm.getScriptObject()).parentState = mState;
        mMainForm.registerEventHandler( SystemMessages::UI_OnClose, "formClosed", this );

        // Don't re-open the menu on close.
        mReopenMenu = false;
    }

    void openVideoOptions( )
    {
        if ( @mMainForm != null )
            mMainForm.close();

        UIManager @ interfaceManager = getAppUIManager();
        @mMainForm = interfaceManager.loadForm( "Scripts/ColonyVI/Forms/Options/VideoOptionsForm.frm", "frmMain" );
        mMainForm.registerEventHandler( SystemMessages::UI_OnClose, "formClosed", this );

        // Re-open the main menu on close.
        mReopenMenu = true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    private void formClosed( UIControl @ sender )
    {
        bool requiresRestart = false;
        if ( mReopenMenu )
            requiresRestart = cast<VideoOptionsForm>(mMainForm.getScriptObject()).requiresRestart;

        // Clean up
        @mMainForm = null;

        // Exit if required.
        if ( requiresRestart )
        {
            mState.raiseEvent( "Exit" );
            return;
        
        } // End if restartRequired

        // Reopen the main menu when form is closed?
        if ( mReopenMenu )
            openMenu();
    }
    
};