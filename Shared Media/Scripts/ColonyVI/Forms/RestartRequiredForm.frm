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
// Name : RestartRequiredForm.frm                                            //
//                                                                           //
// Desc : Simple modal confirmation dialog.                                  // 
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : RestartRequiredForm (Class)
// Desc : Form used to display restart confirmation.
//-----------------------------------------------------------------------------
shared class RestartRequiredForm : IScriptedForm
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Form@               mForm;

    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////    
    // Controls
    Label@                      labelMessage;
    Button@                     buttonOK;
    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : RestartRequiredForm() (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	RestartRequiredForm( Form @ owner )
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
        properties.movable     = true;
        properties.canClose    = true;
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
        mForm.minimumSize       = Size( 350, 160 );
        mForm.size              = Size( 350, 160 );
        mForm.position          = Point( (screenSize.width / 2) - (mForm.size.width / 2), (screenSize.height / 2) - (mForm.size.height / 2) );
        mForm.controlText       = "Restart Required";
        mForm.padding           = Rect( 10, 10, 10, 10 );
        mForm.backgroundOpacity = 0.85f;
        
        // Message Label
        Rect clientArea = mForm.getClientArea();
        @labelMessage = createLabel( "labelMessage", mForm );
        labelMessage.position    = Point( 0, 0 );
        labelMessage.size        = Size( clientArea.width, 30 );
        labelMessage.multiline   = true;
        labelMessage.controlText = "In order to apply your selected configuration changes, a restart is required. Click OK to exit the application.";
        
        // OK button.
        @buttonOK = createButton( "buttonOK", mForm );
        buttonOK.size = Size( 50, 30 );
        buttonOK.position = Point( (clientArea.width / 2) - (buttonOK.size.width / 2), (clientArea.height - 30) );
        buttonOK.controlText = "OK";
        buttonOK.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonOK_OnClick", this );

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
    // Name : buttonOK_OnClick() (Event)
    // Desc : Triggered when the user hits the 'OK' button.
    //-------------------------------------------------------------------------
    void buttonOK_OnClick( UIControl @ control )
    {
        mForm.close();
    }

} // End Class RestartRequiredForm