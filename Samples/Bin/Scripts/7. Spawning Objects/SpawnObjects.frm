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
// Name : SpawnObjects.frm                                                   //
//                                                                           //
// Desc : User interface form for sample 'Spawning Objects' that displays    //
//        several buttons that, when clicked, will cause various objects     //
//        from the associated sample world to be spawned.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
IScriptedForm @ createForm( Form @ owner )
{
    return SpawnObjects( owner );
}

///////////////////////////////////////////////////////////////////////////////
// Class Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : SpawnObjects (Class)
// Desc : Form used to present buttons for spawning various objects.
//-----------------------------------------------------------------------------
shared class SpawnObjects : IScriptedForm
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private Form@           mForm;
    private Scene@          mScene;

    // Controls
    private GroupBox@       mGroupSimplePrimitives;
    private GroupBox@       mGroupObjects;
    private array<Button@>  mButtons;

    
    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : SpawnObjects () (Constructor)
	// Desc : Custom constructor for this class.
	//-------------------------------------------------------------------------
	SpawnObjects( Form @ owner )
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
        properties.sizable     = true;
        properties.movable     = true;
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
        mForm.size              = Size( 410, 300 );
        mForm.position          = Point( (screenSize.width - mForm.size.width) / 2, (screenSize.height - mForm.size.height) / 3 );
        mForm.controlText       = "Spawn Objects";
        mForm.padding           = Rect( 0, 10, 0, 0 );
        mForm.backgroundOpacity = 0.85f;
        mForm.registerEventHandler( SystemMessages::UI_OnSize, "form_OnSize", this );

        // Create a group box for simple primitives
        @mGroupSimplePrimitives            = createGroupBox( "groupBoxPrimitives", mForm );
        mGroupSimplePrimitives.position    = Point( 0, 0 );
        mGroupSimplePrimitives.size        = Size( (mForm.clientSize.width / 2) - 5, mForm.clientSize.height );
        mGroupSimplePrimitives.controlText = "Simple Primitives";

        // Create a group box for objects
        @mGroupObjects            = createGroupBox( "groupBoxObjects", mForm );
        mGroupObjects.position    = Point( (mForm.clientSize.width / 2) + 5, 0 );
        mGroupObjects.size        = Size( (mForm.clientSize.width / 2) - 5, mForm.clientSize.height );
        mGroupObjects.controlText = "Objects";

        // Create buttons.
        int i = 0;
        mButtons.resize( 8 );
        Size buttonSize( mGroupSimplePrimitives.clientSize.width - 20, 20 );

        // *****************
        // Simple Primitives
        // *****************
        int y = 0;

        // Spawn sphere button
        Button @button = createButton( "spawnSphere", mGroupSimplePrimitives );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Sphere";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnSphere_OnClick", this );
        @mButtons[i++] = button;

        // Spawn box button
        @button = createButton( "spawnBox", mGroupSimplePrimitives );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Box";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnBox_OnClick", this );
        @mButtons[i++] = button;

        // Spawn cylinder button
        @button = createButton( "spawnCylinder", mGroupSimplePrimitives );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Cylinder";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnCylinder_OnClick", this );
        @mButtons[i++] = button;

        // Spawn cone button
        @button = createButton( "spawnCone", mGroupSimplePrimitives );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Cone";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnCone_OnClick", this );
        @mButtons[i++] = button;


        // *****************
        // Objects
        // *****************
        y = 0;

        // Spawn crate button
        @button = createButton( "spawnCrate", mGroupObjects );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Crate";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnCrate_OnClick", this );
        @mButtons[i++] = button;

        // Spawn white lamp button
        @button = createButton( "spawnWhiteLamp", mGroupObjects );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "White Lamp";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnWhiteLamp_OnClick", this );
        @mButtons[i++] = button;

        // Spawn red lamp button
        @button = createButton( "spawnRedLamp", mGroupObjects );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Red Lamp";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnRedLamp_OnClick", this );
        @mButtons[i++] = button;

        // Spawn flaming barrel button
        @button = createButton( "spawnBarrel", mGroupObjects );
        button.position = Point( 10, 10 + (y++ * (buttonSize.height + 3)) );
        button.size     = buttonSize;
        button.controlText = "Barrel";
        button.registerEventHandler( SystemMessages::UI_Button_OnClick, "spawnBarrel_OnClick", this );
        @mButtons[i++] = button;

        // Success!
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : setScene()
    // Desc : Called by the application to set the scene into which objects
    //        will be spawned.
    //-------------------------------------------------------------------------
    void setScene( Scene @ scene )
    {
        // Store the scene handle for later use.
        @mScene = scene;
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : spawnObject()
    // Desc : Called internally to spawn an object in response to a button
    //        click.
    //-------------------------------------------------------------------------
    private void spawnObject( uint referenceId )
    {
        // Get the currently active camera. We'll spawn just in front of it.
        CameraNode @ camera = mScene.getActiveCamera();
        if ( @camera == null )
            return;

        // Spawn the referenced object (and all of its children) into the scene.
        ObjectNode @ node = mScene.loadObjectNode( referenceId, CloneMethod::ObjectInstance, true );
        if ( @node == null )
            return;

        // Position three meters in front of the camera.
        node.setPosition( camera.getPosition() + camera.getZAxis() * 3.0f );
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

        // Adjust controls to fill the available form client area
        mGroupSimplePrimitives.size = Size( (clientArea.width / 2) - 5, clientArea.height );
        mGroupObjects.size = mGroupSimplePrimitives.size;
        mGroupObjects.position = Point( (clientArea.width / 2) + 5, 0 );

        // Scale buttons
        Size buttonSize( mGroupSimplePrimitives.clientSize.width - 20, 20 );
        for ( uint i = 0; i < mButtons.length(); ++i )
            mButtons[i].size = buttonSize;
    }

    //-------------------------------------------------------------------------
    // Name : spawn*_OnClick() (Event)
    // Desc : Each of the following methods are triggered when the 
    //        corresponding button is clicked.
    //-------------------------------------------------------------------------
    void spawnSphere_OnClick( UIControl @ control )
    {
        // Spawn the sphere object from the 'Simple Primitives' scene.
        spawnObject( 0x38 );
    }

    void spawnBox_OnClick( UIControl @ control )
    {
        // Spawn the box object from the 'Simple Primitives' scene.
        spawnObject( 0x45 );
    }

    void spawnCylinder_OnClick( UIControl @ control )
    {
        // Spawn the cylinder object from the 'Simple Primitives' scene.
        spawnObject( 0x4E );
    }

    void spawnCone_OnClick( UIControl @ control )
    {
        // Spawn the cone object from the 'Simple Primitives' scene.
        spawnObject( 0x6D );
    }

    void spawnCrate_OnClick( UIControl @ control )
    {
        // Spawn the crate object from the 'Objects' scene.
        spawnObject( 0x88 );
    }

    void spawnWhiteLamp_OnClick( UIControl @ control )
    {
        // Spawn the white lamp object from the 'Objects' scene.
        spawnObject( 0x8B );
    }

    void spawnRedLamp_OnClick( UIControl @ control )
    {
        // Spawn the red lamp object from the 'Objects' scene.
        spawnObject( 0xBA );
    }

    void spawnBarrel_OnClick( UIControl @ control )
    {
        // Spawn the barrel object from the 'Objects' scene.
        spawnObject( 0x18F );
    }

} // End Class SpawnObjects