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
// Name : InGameMenuForm.frm                                                 //
//                                                                           //
// Desc : UI form for displaying the in game menu selections.                // 
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
//#include_once "../States/MainMenu.gs"

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : InGameMenuForm (Class)
// Desc : Form used to present in game menu selections.
//-----------------------------------------------------------------------------
shared class InGameMenuForm : IScriptedForm
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Form@               mForm;

    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    AppState@                   parentState;
    
    // Controls
    Button@                     buttonResume;
    Button@                     buttonOptions;
    Button@                     buttonRestart;
    Button@                     buttonReload;
    Button@                     buttonExit;
    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : InGameMenuForm() (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	InGameMenuForm( Form @ owner )
    {
        @mForm = owner;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Interface Method Overrides (IScriptedForm)
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : onPreCreateForm()
    // Desc : Allows the script to override the default properties used in
    //        order to create the form.
    //-------------------------------------------------------------------------
    bool onPreCreateForm( FormProperties & properties )
    {
        // Set form properties
        properties.style       = FormStyle::Overlapped;
        properties.sizable     = false;
        properties.movable     = false;
        properties.canClose    = false;
        properties.canMaximize = false;
        properties.canMinimize = false;

        // Continue initializing
        return true;
    }

    //-------------------------------------------------------------------------
    // Name : onCreateForm()
    // Desc : Triggered by the application in order to have the script
    //        initialize the form. This includes creating and attaching any
    //        child controls and defining form properties etc.
    //-------------------------------------------------------------------------
    bool onCreateForm( )
    {
        RenderDriver @ driver = getAppRenderDriver();
        Size screenSize = driver.getScreenSize();

        // Configure controls - Main form first.
        mForm.minimumSize       = Size( 290, 290 );
        mForm.size              = Size( 350, 290 );
        mForm.position          = Point( (screenSize.width / 2) - (mForm.size.width / 2), (screenSize.height / 2) - (mForm.size.height / 2) );
        mForm.controlText       = "Paused";
        mForm.padding           = Rect( 10, 10, 10, 10 );
        mForm.backgroundOpacity = 0.85f;
        mForm.registerEventHandler( SystemMessages::UI_OnSize, "form_OnSize", this );
        mForm.registerEventHandler( SystemMessages::UI_OnScreenLayoutChange, "form_OnScreenLayoutChange", this );

        // Dialog buttons.
        Rect clientArea = mForm.getClientArea();
        @buttonResume = createButton( "buttonResume", mForm );
        buttonResume.position = Point( 0, 0 );
        buttonResume.size = Size( clientArea.width, 30 );
        buttonResume.controlText = "Resume Game";
        buttonResume.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonResume_OnClick", this );

        @buttonRestart = createButton( "buttonRestart", mForm );
        buttonRestart.position = Point( 0, 45 );
        buttonRestart.size = Size( clientArea.width, 30 );
        buttonRestart.controlText = "Restart Level";
        buttonRestart.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonRestart_OnClick", this );

        /*@buttonReload = createButton( "buttonReload", mForm );
        buttonReload.position = Point( 0, 90 );
        buttonReload.size = Size( clientArea.width, 30 );
        buttonReload.controlText = "Restart from Last Checkpoint";
        buttonReload.enabled = false;
        buttonReload.backgroundOpacity = 0.5f;
        buttonReload.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonReload_OnClick", this );*/

        @buttonOptions= createButton( "buttonOptions", mForm );
        buttonOptions.position = Point( 0, 90 /*135*/ );
        buttonOptions.size = Size( clientArea.width, 30 );
        buttonOptions.controlText = "Options";
        buttonOptions.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonOptions_OnClick", this );

        @buttonExit= createButton( "buttonExit", mForm );
        buttonExit.position = Point( 0, 135 /*180*/ );
        buttonExit.size = Size( clientArea.width, 30 );
        buttonExit.controlText = "Exit Game";
        buttonExit.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonExit_OnClick", this );
        
        // Success!
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
	// Event Handlers
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : form_OnSize() (Event)
    // Desc : Called whenever the main form is resized.
    //-------------------------------------------------------------------------
    void form_OnSize( UIControl @ control, int width, int height )
    {
        Rect clientArea = control.getClientArea();
        buttonResume.position = Point( 0, 0 );
        buttonResume.size = Size( clientArea.width, 30 );
        buttonRestart.position = Point( 0, 45 );
        buttonRestart.size = Size( clientArea.width, 30 );
        /*buttonReload.position = Point( 0, 90 );
        buttonReload.size = Size( clientArea.width, 30 );*/
        buttonOptions.position = Point( 0, 90 /*135*/ );
        buttonOptions.size = Size( clientArea.width, 30 );
        buttonExit.position = Point( 0, 135 /*180*/ );
        buttonExit.size = Size( clientArea.width, 30 );
    }

    //-------------------------------------------------------------------------
    // Name : form_OnScreenLayoutChange() (Event)
    // Desc : Called whenever the size of the output display may have been
    //        altered in some way.
    //-------------------------------------------------------------------------
    void form_OnScreenLayoutChange( UIControl @ control )
    {
        // Device has been restored (possibly due to a window size change)
        // and we should resize our form appropriately.
        RenderDriver @ driver = getAppRenderDriver();
        Size screenSize = driver.getScreenSize();
        mForm.position  = Point( (screenSize.width / 2) - (mForm.size.width / 2), (screenSize.height / 2) - (mForm.size.height / 2) );
    }

    //-------------------------------------------------------------------------
    // Name : buttonResume_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Resume Game' button.
    //-------------------------------------------------------------------------
    void buttonResume_OnClick( UIControl @ control )
    {
        // Resume playing
        parentState.raiseEvent( "Resume" );
    }

    //-------------------------------------------------------------------------
    // Name : buttonRestart_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Restart Level' button.
    //-------------------------------------------------------------------------
    void buttonRestart_OnClick( UIControl @ control )
    {
        // Force a restart of the level.
        parentState.raiseEvent( "Restart" );
    }

    //-------------------------------------------------------------------------
    // Name : buttonReload_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Restart Level' button.
    //-------------------------------------------------------------------------
    void buttonReload_OnClick( UIControl @ control )
    {
        // Restart from last checkpoint
        parentState.raiseEvent( "Reload" );
    }

    //-------------------------------------------------------------------------
    // Name : buttonOptions_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Options' button.
    //-------------------------------------------------------------------------
    void buttonOptions_OnClick( UIControl @ control )
    {
        parentState.raiseEvent( "Open Options" );
    }

    //-------------------------------------------------------------------------
    // Name : buttonExit_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Exit' button.
    //-------------------------------------------------------------------------
    void buttonExit_OnClick( UIControl @ control )
    {
        // Exit the entire game.
        parentState.raiseEvent( "Exit" );
    }

} // End Class InGameMenuForm