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
// Name : MainMenuForm.frm                                                   //
//                                                                           //
// Desc : UI form for displaying the top level main menu selections.         // 
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
#ifndef _PARENT_SCRIPT
IScriptedForm @ createForm( Form @ owner )
{
    return MainMenuForm( owner );
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : MainMenuForm (Class)
// Desc : Form used to present main menu selections.
//-----------------------------------------------------------------------------
shared class MainMenuForm : IScriptedForm
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Form@               mForm;
    
    // Controls
    AppState@                   parentState;
    Button@                     buttonNewGame;
    Button@                     buttonOptions;
    Button@                     buttonCredits;
    Button@                     buttonExit;
    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : MainMenuForm() (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	MainMenuForm( Form @ owner )
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
        mForm.size              = Size( screenSize.width / 2, screenSize.height - (((screenSize.height/9) * 2) + 50) );
        mForm.position          = Point( 16, (screenSize.height / 9) + 50 );
        mForm.controlText       = "Main Menu";
        mForm.padding           = Rect( 10, 10, 10, 10 );
        mForm.backgroundOpacity = 0.85f;
        mForm.registerEventHandler( SystemMessages::UI_OnSize, "form_OnSize", this );
        mForm.registerEventHandler( SystemMessages::UI_OnScreenLayoutChange, "form_OnScreenLayoutChange", this );

        // Dialog buttons.
        Rect clientArea = mForm.getClientArea();
        @buttonNewGame = createButton( "buttonNewGame", mForm );
        buttonNewGame.position = Point( 0, 0 );
        buttonNewGame.size = Size( clientArea.width, 50 );
        buttonNewGame.controlText = "New Game";
        buttonNewGame.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonNewGame_OnClick", this );

        @buttonOptions= createButton( "buttonOptions", mForm );
        buttonOptions.position = Point( 0, 55 );
        buttonOptions.size = Size( clientArea.width, 50 );
        buttonOptions.controlText = "Options";
        buttonOptions.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonOptions_OnClick", this );

        @buttonCredits= createButton( "buttonCredits", mForm );
        buttonCredits.position = Point( 0, 110 );
        buttonCredits.size = Size( clientArea.width, 50 );
        buttonCredits.controlText = "View Credits";
        buttonCredits.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonCredits_OnClick", this );

        @buttonExit= createButton( "buttonExit", mForm );
        buttonExit.position = Point( 0, 165 );
        buttonExit.size = Size( clientArea.width, 50 );
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
        buttonNewGame.position = Point( 0, 0 );
        buttonNewGame.size = Size( clientArea.width, 50 );
        buttonOptions.position = Point( 0, 55 );
        buttonOptions.size = Size( clientArea.width, 50 );
        buttonCredits.position = Point( 0, 110 );
        buttonCredits.size = Size( clientArea.width, 50 );
        buttonExit.position = Point( 0, 165 );
        buttonExit.size = Size( clientArea.width, 50 );
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
        mForm.size     = Size( screenSize.width / 2, screenSize.height - (((screenSize.height/9) * 2) + 50) );
        mForm.position = Point( 16, (screenSize.height / 9) + 50 );
    }

    //-------------------------------------------------------------------------
    // Name : buttonNewGame_OnClick() (Event)
    // Desc : Triggered when the user hits the 'New Game' button.
    //-------------------------------------------------------------------------
    void buttonNewGame_OnClick( UIControl @ control )
    {
        mForm.close();
        parentState.raiseEvent( "New Game" );
    }

    //-------------------------------------------------------------------------
    // Name : buttonOptions_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Options' button.
    //-------------------------------------------------------------------------
    void buttonOptions_OnClick( UIControl @ control )
    {
    }

    //-------------------------------------------------------------------------
    // Name : buttonCredits_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Credits' button.
    //-------------------------------------------------------------------------
    void buttonCredits_OnClick( UIControl @ control )
    {
    }

    //-------------------------------------------------------------------------
    // Name : buttonExit_OnClick() (Event)
    // Desc : Triggered when the user hits the 'Exit' button.
    //-------------------------------------------------------------------------
    void buttonExit_OnClick( UIControl @ control )
    {
        parentState.raiseEvent( "Exit" );
    }

} // End Class MainMenuForm