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
// Name : VideoOptionsForm.frm                                               //
//                                                                           //
// Desc : UI form for displaying available video options to the user.        // 
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////
#include_once "../../States/MainMenu.gs"

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : VideoOptionsForm (Class)
// Desc : Form used to present display options to the user.
//-----------------------------------------------------------------------------
shared class VideoOptionsForm : IScriptedForm
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Form@               mForm;
    private array<DisplayMode>  mDisplayModes;
    
    ///////////////////////////////////////////////////////////////////////////
	// Public Member Variables
	///////////////////////////////////////////////////////////////////////////
    MainMenu@                   parentState;

    // Controls
    GroupBox@                   groupDisplayModes;
    GroupBox@                   groupQuality;
    Button@                     buttonApply;
    Button@                     buttonAccept;
    Button@                     buttonCancel;
    ListBox@                    listDisplayModes;
    CheckBox@                   checkFullScreen;
    Label@                      labelShadingQuality;
    ComboBox@                   comboShadingQuality;
    Label@                      labelPostProcessQuality;
    ComboBox@                   comboPostProcessQuality;
    Label@                      labelAntiAliasQuality;
    ComboBox@                   comboAntiAliasQuality;
    Label@                      labelVerticalSync;
    CheckBox@                   checkVerticalSync;
    Label@                      labelTripleBuffer;
    CheckBox@                   checkTripleBuffer;
    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : VideoOptionsForm () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	VideoOptionsForm( Form @ owner )
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
        //mForm.size              = Size( screenSize.width - (screenSize.width / 4), screenSize.height - (screenSize.height / 4) );
        //mForm.position          = Point( (screenSize.width - mForm.size.width) / 2, (screenSize.height - mForm.size.height) / 2 );
        mForm.size              = Size( 550, screenSize.height - (screenSize.height / 3) );
        mForm.position          = Point( (screenSize.width - mForm.size.width) / 2, (screenSize.height - mForm.size.height) / 2 );
        mForm.controlText       = "Video Options";
        mForm.padding           = Rect( 0, 10, 0, 0 );
        mForm.backgroundOpacity = 0.85f;
        mForm.registerEventHandler( SystemMessages::UI_OnSize, "form_OnSize", this );
        mForm.registerEventHandler( SystemMessages::UI_OnScreenLayoutChange, "form_OnScreenLayoutChange", this );

        // Create a group box for display modes.
        @groupDisplayModes = createGroupBox( "groupBoxModes", mForm );
        groupDisplayModes.position    = Point( 0, 0 );
        groupDisplayModes.size        = Size( (mForm.clientSize.width / 2) - 5, mForm.clientSize.height - 30 );
        groupDisplayModes.controlText = "Display Mode";

        // Create the display mode list.
        @listDisplayModes = createListBox( "listBoxModes", groupDisplayModes );
        listDisplayModes.position = Point( 0, 0 );
        listDisplayModes.size = Size( groupDisplayModes.clientSize.width, groupDisplayModes.clientSize.height - 20 );
        listDisplayModes.font = "fixed_v01_white_plain";
        listDisplayModes.textColor = ColorValue( 0, 0, 0, 1 ); // Black

        // Full screen check box
        @checkFullScreen = createCheckBox( "checkFullScreen", groupDisplayModes );
        checkFullScreen.position = Point( 0, listDisplayModes.size.height + 5 );
        checkFullScreen.size = Size( groupDisplayModes.clientSize.width / 2, 15 );
        checkFullScreen.controlText = "Full Screen";
        checkFullScreen.checked = !driver.getConfig().windowed;
        //checkFullScreen.registerEventHandler( SystemMessages::UI_CheckBox_OnCheckedStateChange, "checkFullScreen_OnCheckedStateChange", this );

        // Create a group box for rendering quality.
        @groupQuality = createGroupBox( "groupQuality", mForm );
        groupQuality.position    = Point( (mForm.clientSize.width / 2) + 5, 0 );
        groupQuality.size        = Size( (mForm.clientSize.width / 2) - 5, mForm.clientSize.height - 30 );
        groupQuality.padding     = Rect(10,10,10,10);
        groupQuality.controlText = "Rendering Options";

        // Shading quality
        @labelShadingQuality = createLabel( "labelShadingQuality", groupQuality );
        labelShadingQuality.position = Point( 0, 0 );
        labelShadingQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelShadingQuality.controlText = "Shading";
        @comboShadingQuality = createComboBox( "comboShadingQuality", groupQuality );
        comboShadingQuality.position = Point( groupQuality.clientSize.width / 2, 0 );
        comboShadingQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboShadingQuality.font = "fixed_v01_white_plain";
        comboShadingQuality.textColor = ColorValue( 0, 0, 0, 1 ); // Black
        comboShadingQuality.addItem( "Lowest" );
        comboShadingQuality.addItem( "Low" );
        comboShadingQuality.addItem( "Medium" );
        comboShadingQuality.addItem( "High" );
        comboShadingQuality.addItem( "Ultra" );
        comboShadingQuality.selectedIndex = driver.getSystemState( SystemState::ShadingQuality );

        // Post processing quality.
        @labelPostProcessQuality = createLabel( "labelPostProcessQuality", groupQuality );
        labelPostProcessQuality.position = Point( 0, 22 );
        labelPostProcessQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelPostProcessQuality.controlText = "Post Processing";
        @comboPostProcessQuality = createComboBox( "comboPostProcessQuality", groupQuality );
        comboPostProcessQuality.position = Point( groupQuality.clientSize.width / 2, 22 );
        comboPostProcessQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboPostProcessQuality.font = "fixed_v01_white_plain";
        comboPostProcessQuality.textColor = ColorValue( 0, 0, 0, 1 ); // Black
        comboPostProcessQuality.addItem( "Lowest" );
        comboPostProcessQuality.addItem( "Low" );
        comboPostProcessQuality.addItem( "Medium" );
        comboPostProcessQuality.addItem( "High" );
        comboPostProcessQuality.addItem( "Ultra" );
        comboPostProcessQuality.selectedIndex = driver.getSystemState( SystemState::PostProcessQuality );
       

        // Post processing quality.
        @labelAntiAliasQuality = createLabel( "labelAntiAliasQuality", groupQuality );
        labelAntiAliasQuality.position = Point( 0, 44 );
        labelAntiAliasQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelAntiAliasQuality.controlText = "Anti-Aliasing";
        @comboAntiAliasQuality = createComboBox( "comboAntiAliasQuality", groupQuality );
        comboAntiAliasQuality.position = Point( groupQuality.clientSize.width / 2, 44 );
        comboAntiAliasQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboAntiAliasQuality.font = "fixed_v01_white_plain";
        comboAntiAliasQuality.textColor = ColorValue( 0, 0, 0, 1 ); // Black
        comboAntiAliasQuality.addItem( "Lowest" );
        comboAntiAliasQuality.addItem( "Low" );
        comboAntiAliasQuality.addItem( "Medium" );
        comboAntiAliasQuality.addItem( "High" );
        comboAntiAliasQuality.addItem( "Ultra" );
        comboAntiAliasQuality.selectedIndex = driver.getSystemState( SystemState::AntiAliasingQuality );

        // V-Sync
        @labelVerticalSync = createLabel( "labelVerticalSync", groupQuality );
        labelVerticalSync.position = Point( 0, 66 );
        labelVerticalSync.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelVerticalSync.controlText = "V-sync";
        @checkVerticalSync = createCheckBox( "checkVerticalSync", groupQuality );
        checkVerticalSync.position = Point( groupQuality.clientSize.width / 2, 66 );
        checkVerticalSync.size = Size( groupQuality.clientSize.width / 2, 17 );
        checkVerticalSync.controlText = "";
        checkVerticalSync.checked = driver.getConfig().useVSync;

        // Triple Buffering
        @labelTripleBuffer = createLabel( "labelTripleBuffer", groupQuality );
        labelTripleBuffer.position = Point( 0, 88 );
        labelTripleBuffer.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelTripleBuffer.controlText = "Triple Buffering";
        @checkTripleBuffer = createCheckBox( "checkTripleBuffer", groupQuality );
        checkTripleBuffer.position = Point( groupQuality.clientSize.width / 2, 88 );
        checkTripleBuffer.size = Size( groupQuality.clientSize.width / 2, 17 );
        checkTripleBuffer.controlText = "";
        checkTripleBuffer.checked = driver.getConfig().useTripleBuffering;

        // Dialog buttons.
        @buttonApply = createButton( "buttonApply", mForm );
        buttonApply.position = Point( mForm.clientSize.width - (70 * 3), mForm.clientSize.height - 25 );
        buttonApply.size = Size( 60, 25 );
        buttonApply.controlText = "Apply";
        buttonApply.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonApply_OnClick", this );

        @buttonAccept = createButton( "buttonAccept", mForm );
        buttonAccept.position = Point( mForm.clientSize.width - (70 * 2), mForm.clientSize.height - 25 );
        buttonAccept.size = Size( 60, 25 );
        buttonAccept.controlText = "Accept";
        buttonAccept.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonAccept_OnClick", this );

        @buttonCancel = createButton( "buttonCancel", mForm );
        buttonCancel.position = Point( mForm.clientSize.width - (70 * 1), mForm.clientSize.height - 25 );
        buttonCancel.size = Size( 60, 25 );
        buttonCancel.controlText = "Cancel";
        buttonCancel.registerEventHandler( SystemMessages::UI_Button_OnClick, "buttonCancel_OnClick", this );

        // Fill the display mode list box with the available display modes.
        populateDisplayModes();
        
        // Success!
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : populateDisplayModes()
    // Desc : Called in order to re-populate the display mode list box
    //        with the currently available display modes.
    //-------------------------------------------------------------------------
    void populateDisplayModes( )
    {
        // Get the list of available display modes for the current device.
        RenderDriver @ driver = getAppRenderDriver();
        RenderingCapabilities @ caps = driver.getCapabilities();
        caps.getDisplayModes( mDisplayModes );

        // Get the current render driver configuration so we can select the 
        // current mode from the list on completion.
        RenderDriverConfig config = driver.getConfig();
        
        // Add each mode to the display mode list.
        int currentMode = -1;
        for ( uint i = 0; i < mDisplayModes.length(); ++i )
        {
            DisplayMode mode = mDisplayModes[i];
            String item = mode.width + " x " + mode.height + " @ " + int(mode.refreshRate) + "hz";
            listDisplayModes.addItem( item );

            // Configuration matches this mode?
            if ( config.width == mode.width && config.height == mode.height &&
                 config.refreshRate == mode.refreshRate )
                currentMode = i;

        } // Next item

        // Select the correct item for the current display mode.
        listDisplayModes.selectedIndex = currentMode;
    }

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

        // Resize the display mode group
        groupDisplayModes.size = Size( (clientArea.width / 2) - 5, clientArea.height - 30 );
        listDisplayModes.size = Size( groupDisplayModes.clientSize.width, groupDisplayModes.clientSize.height - 20 );
        checkFullScreen.position = Point( 0, listDisplayModes.size.height + 5 );
        checkFullScreen.size = Size( groupDisplayModes.clientSize.width / 2, 15 );

        // Resize the rendering quality group.
        groupQuality.position = Point( (mForm.clientSize.width / 2) + 5, 0 );
        groupQuality.size = Size( (mForm.clientSize.width / 2) - 5, mForm.clientSize.height - 30 );
        labelShadingQuality.position = Point( 0, 0 );
        labelShadingQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboShadingQuality.position = Point( groupQuality.clientSize.width / 2, 0 );
        comboShadingQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelPostProcessQuality.position = Point( 0, 22 );
        labelPostProcessQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboPostProcessQuality.position = Point( groupQuality.clientSize.width / 2, 22 );
        comboPostProcessQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelAntiAliasQuality.position = Point( 0, 44 );
        labelAntiAliasQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        comboAntiAliasQuality.position = Point( groupQuality.clientSize.width / 2, 44 );
        comboAntiAliasQuality.size = Size( groupQuality.clientSize.width / 2, 17 );
        labelVerticalSync.position = Point( 0, 66 );
        labelVerticalSync.size = Size( groupQuality.clientSize.width / 2, 17 );
        //checkVerticalSync.position = Point( groupQuality.clientSize.width / 2, 66 );
        //checkVerticalSync.size = Size( groupQuality.clientSize.width / 2, 17 );
        checkVerticalSync.position = Point( groupQuality.clientSize.width - 14, 66 );
        checkVerticalSync.size = Size( 17, 17 );
        labelTripleBuffer.position = Point( 0, 88 );
        labelTripleBuffer.size = Size( groupQuality.clientSize.width / 2, 17 );
        //checkTripleBuffer.position = Point( groupQuality.clientSize.width / 2, 88 );
        //checkTripleBuffer.size = Size( groupQuality.clientSize.width / 2, 17 );
        checkTripleBuffer.position = Point( groupQuality.clientSize.width - 14, 88 );
        checkTripleBuffer.size = Size( 17, 17 );
        
        // Reposition the dialog buttons
        buttonApply.position  = Point( clientArea.width - (70 * 3), clientArea.height - 25 );
        buttonAccept.position = Point( clientArea.width - (70 * 2), clientArea.height - 25 );
        buttonCancel.position = Point( clientArea.width - (70 * 1), clientArea.height - 25 );
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
        //mForm.size     = Size( screenSize.width - (screenSize.width / 4), screenSize.height - (screenSize.height / 4) );
        //mForm.position = Point( (screenSize.width - mForm.size.width) / 2, (screenSize.height - mForm.size.height) / 2 );
        mForm.size              = Size( 550, screenSize.height - (screenSize.height / 3) );
        mForm.position          = Point( (screenSize.width - mForm.size.width) / 2, (screenSize.height - mForm.size.height) / 2 );
    }

    //-------------------------------------------------------------------------
    // Name : buttonApply_OnClick() (Event)
    // Desc : Triggered when the user hits the 'apply' button.
    //-------------------------------------------------------------------------
    void buttonApply_OnClick( UIControl @ control )
    {
        // Get the currently selected display mode.
        int selectedModeIndex = listDisplayModes.selectedIndex;
        if ( selectedModeIndex >= 0 && selectedModeIndex < mDisplayModes.length() )
        {
            // Alter the display mode.
            DisplayMode mode = mDisplayModes[selectedModeIndex];
            RenderDriver @ driver = getAppRenderDriver();
            driver.updateDisplayMode( mode, !checkFullScreen.checked, checkVerticalSync.checked );

            // Set quality
            driver.setSystemState( SystemState::ShadingQuality, comboShadingQuality.selectedIndex );
            driver.setSystemState( SystemState::PostProcessQuality, comboPostProcessQuality.selectedIndex );
            driver.setSystemState( SystemState::AntiAliasingQuality, comboAntiAliasQuality.selectedIndex );

        } // End if valid mode
    }

    //-------------------------------------------------------------------------
    // Name : buttonAccept_OnClick() (Event)
    // Desc : Triggered when the user hits the 'accept' button.
    //-------------------------------------------------------------------------
    void buttonAccept_OnClick( UIControl @ control )
    {
        // Trigger the 'apply' behavior then close the form.
        buttonApply_OnClick( buttonApply );
        //mForm.close();
        parentState.openMainMenu();
    }

    //-------------------------------------------------------------------------
    // Name : buttonCancel_OnClick() (Event)
    // Desc : Triggered when the user hits the 'cancel' button.
    //-------------------------------------------------------------------------
    void buttonCancel_OnClick( UIControl @ control )
    {
        parentState.openMainMenu(); //.close();
    }

} // End Class VideoOptionsForm