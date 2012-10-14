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
// Name : cgUIForm.cpp                                                       //
//                                                                           //
// Desc : This module houses a special type of 'Interface Control' known as  //
//        a form. This is like a window which provides the specialized       //
//        functionality required for this type of interface item.            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUIForm Module Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIForm.h>
#include <Interface/cgUILayers.h>
#include <Interface/cgUISkin.h>
#include <Interface/cgUIControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/Controls/cgLabelControl.h>
#include <Interface/Controls/cgButtonControl.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgScript.h>
#include <Input/cgInputDriver.h>
#include <System/cgMessageTypes.h>

// ToDo: Form control buttons (minimize/maximize etc.) must do something.
// Done: When resizing, if the form didn't get any smaller / larger, don't update that axis of the old position (to prevent the 'anchor point' from sliding).

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgUIForm::FormAllocTypeMap  cgUIForm::mRegisteredForms;

///////////////////////////////////////////////////////////////////////////////
// cgUIForm Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgUIForm () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIForm::cgUIForm( cgUIControlLayer * pLayer ) : cgUIControl( Complex, _T("Form") )
{
    // Initialize variables to sensible defaults
    mUIManager        = CG_NULL;
    mUILayer          = CG_NULL;
    mCaption          = CG_NULL;
    mCloseButton      = CG_NULL;
    mMinimizeButton   = CG_NULL;
    mMaximizeButton   = CG_NULL;
    mFormScriptRes    = CG_NULL;
    mFormScriptObject = CG_NULL;
    mAcceptButton     = CG_NULL;
    mCancelButton     = CG_NULL;

    // Store the interface manager from the layer
    mUILayer          = pLayer;
    mUIManager        = pLayer->getUIManager();
    mRootForm         = this;
}

//-----------------------------------------------------------------------------
//  Name : ~cgUIForm () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgUIForm::~cgUIForm()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgUIForm::dispose( bool bDisposeBase )
{
    // We are in the process of disposing.
    mDisposing = true;

    // Release any script objects we retain.
    if ( mFormScriptObject )
        mFormScriptObject->release();

    // Clear variables
    mUIManager        = CG_NULL;
    mUILayer          = CG_NULL;
    mCaption          = CG_NULL;
    mCloseButton      = CG_NULL;
    mMinimizeButton   = CG_NULL;
    mMaximizeButton   = CG_NULL;
    mFormScriptRes       = CG_NULL;
    mFormScriptObject = CG_NULL;
    mAcceptButton     = CG_NULL;
    mCancelButton     = CG_NULL;

    // Release resources
    mFormScript.close();

    // Pass through to base class if required.
    if ( bDisposeBase )
        cgUIControl::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIForm )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : registerType() (Static)
/// <summary>
/// Allows the application to register all of the various form
/// types that are supported by the application. These types can then be 
/// referenced directly by name / initialization data etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIForm::registerType( const cgString & strTypeName, FormAllocFunc pFunction )
{
    // Store the function pointer
    mRegisteredForms[ strTypeName ] = pFunction;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Call this function to create a form of any given type, by name. The 
/// form will then ultimately be initialized based on the derived type's
/// specifications (if applicable).
/// </summary>
//-----------------------------------------------------------------------------
cgUIForm * cgUIForm::createInstance( const cgString & strTypeName, cgUIControlLayer * pLayer )
{
    FormAllocFunc Allocate = CG_NULL;
    FormAllocTypeMap::iterator itAlloc;

    // Find the allocation function based on the name specified
    itAlloc = mRegisteredForms.find( strTypeName );
    if ( itAlloc == mRegisteredForms.end() )
        return CG_NULL;

    // Extract the function pointer
    Allocate = itAlloc->second;
    if ( Allocate == CG_NULL )
        return CG_NULL;

    // Call the registered allocation function
    return Allocate( pLayer );
}

//-----------------------------------------------------------------------------
//  Name : createForm ()
/// <summary>
/// Initialize the form based on the derived classes specification and 
/// prepare for rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::loadForm( cgInputStream FormScriptStream, const cgString & strName )
{
    // Get access to the resource manager
    cgResourceManager * pResources = mUIManager->getResourceManager();
    if ( !pResources )
        return false;

    // Load the form script
    if ( !pResources->loadScript( &mFormScript, FormScriptStream, 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to instantiate interface form '%s' from the script '%s'. See any previous errors for more information.\n"), strName.c_str(), FormScriptStream.getName().c_str() );
        return false;

    } // End if failed to load script

    // Cache the pointer to the underlying script object
    mFormScriptRes = (cgScript*)mFormScript.getResource(true);

    // Attempt to instantiate a form script object
    try
    {
        cgScriptArgument::Array ScriptArgs;
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("Form@+"), (void*)this ) );
        mFormScriptObject = mFormScriptRes->executeFunctionObject( _T("IScriptedForm"), _T("createForm"), ScriptArgs );

    } // End try to execute

    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute createForm() function in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        return false;

    } // End catch exception

    // Pass through to standard form creation.
    return createForm( strName );
}

//-----------------------------------------------------------------------------
//  Name : createForm ()
/// <summary>
/// Initialize the form based on the derived classes specification and 
/// prepare for rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::createForm( const cgString & strName )
{
    cgRect rcControl;

    // Validate requirements
    if ( !mUIManager )
        return false;

    // Set the form's name
    setName( strName );

    // Get the currently selected skin
    cgUISkin * pCurrentSkin = mUIManager->getCurrentSkin();
    if ( !pCurrentSkin )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create interface form '%s' because no valid skin has been selected.\n"), getName().c_str() );
        return false;

    } // End if failed to get skin

    // Begin preparing the layer buffer.
    if ( !mUILayer->prepareLayer( pCurrentSkin->getTextureName() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to begin preparing render buffer for form '%s'.\n"), getName().c_str() );
        return false;

    } // End if failed to prepare

    // Add all the required elements from the skin to the control layer in order
    // for it to access frame / frame group data. Uses named frames / groups to allow the
    // information for the elements to be easily retrieved later.
    if ( !pCurrentSkin->prepareControlFrames( mUILayer, cgUIFormStyle::Overlapped ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to populate control layer frameset information for form '%s'.\n"), getName().c_str() );
        return false;

    } // End if failed to prepare

    // Create the form caption control
    mCaption = new cgLabelControl( );
    mCaption->setName( _T("Form.CaptionLabel") );
    addChildControl( mCaption );

    // Default form properties in case onPreCreateForm was not included in derived class
    mProperties.style       = cgUIFormStyle::Overlapped;
    mProperties.sizable     = true;
    mProperties.movable     = true;
    mProperties.canClose    = true;
    mProperties.canMaximize = false;
    mProperties.canMinimize = false;

    // Allow derived class / script to populate properties
    if ( !onPreCreateForm( mProperties ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create form '%s' while executing 'onPreCreateForm()' for property selection.\n"), getName().c_str() );
        return false;

    } // End if failed to init

    // Execute the form initialization routine
    if ( !onCreateForm( ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create form '%s' while executing 'onCreateForm()' for control creation.\n"), getName().c_str() );
        return false;

    } // End if failed to init

    // Create the correct close button (if can close or either of the other two control buttons are visible)
    if ( mProperties.canClose || (mProperties.canMinimize || mProperties.canMaximize) )
    {
        cgString strButtonElement = _T("Form.ButtonClose");

        // If minimize and maximize are disabled, use the standalone close button if available
        if ( !mProperties.canMinimize && !mProperties.canMaximize )
        {
            if ( mUILayer->getSkinElement( _T("Form.ButtonStandaloneClose") ) )
                strButtonElement = _T("Form.ButtonStandaloneClose");

        } // End if maximize AND minimize disabled

        // Create the button, and register us as the target for handling its click event
        mCloseButton = new cgButtonControl( strButtonElement );
        mCloseButton->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
        addChildControl( mCloseButton );

    } // End if can close.

    // If either minimize or maximize is enabled, we'll display them both
    if ( mProperties.canMinimize || mProperties.canMaximize )
    {
        mMinimizeButton = new cgButtonControl( _T("Form.ButtonMinimize") );
        mMaximizeButton = new cgButtonControl( _T("Form.ButtonMaximize") );
        mMinimizeButton->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
        mMaximizeButton->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
        addChildControl( mMinimizeButton );
        addChildControl( mMaximizeButton );
    
    } // End if either are enabled

    // Enable / Disable relevant buttons
    if ( !mProperties.canClose && mCloseButton )
        mCloseButton->setEnabled( false );
    if ( !mProperties.canMinimize && mMinimizeButton )
        mMinimizeButton->setEnabled( false );
    if ( !mProperties.canMaximize && mMaximizeButton )
        mMaximizeButton->setEnabled( false );

    // Build all control data
    if ( !build( ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to build final render data for form '%s'.\n"), getName().c_str() );
        return false;

    } // End if failed to build

    // Finalize the billboard buffer preparation
    if ( !mUILayer->endPrepareLayer() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to complete preperation of render data for form '%s'.\n"), getName().c_str() );
        return false;

    } // End if failed to prepare

    // Retrieve the form description from the current skin
    const cgUIFormStyleDesc & FormDesc = pCurrentSkin->getFormStyleDefinition( cgUIFormStyle::Overlapped );
    
    // Position and size the caption
    rcControl = elementAreaToClientRect( FormDesc.caption );
    mCaption->setPosition( rcControl.left, rcControl.top );
    mCaption->setSize( rcControl.right - rcControl.left, rcControl.bottom - rcControl.top );

    // Position the control buttons
    rcControl = elementAreaToClientRect( FormDesc.buttonClose );
    if ( mCloseButton )
        mCloseButton->setPosition( rcControl.left, rcControl.top );

    rcControl = elementAreaToClientRect( FormDesc.buttonMinimize );
    if ( mMinimizeButton )
        mMinimizeButton->setPosition( rcControl.left, rcControl.top );
    
    rcControl = elementAreaToClientRect( FormDesc.buttonMaximize );
    if ( mMaximizeButton )
        mMaximizeButton->setPosition( rcControl.left, rcControl.top );

    // Set up the caption label rendering style
    mCaption->setMultiline( false );
    mCaption->setTruncationMode( cgTextTruncationMode::Ellipsis );
    mCaption->setHorizontalAlignment( cgHorizontalAlignment::Left );
    mCaption->setVerticalAlignment( cgVerticalAlignment::Middle );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setAcceptButton ()
/// <summary>
/// Set the default button to be triggered when the user presses 'enter'
/// to accept the form.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIForm::setAcceptButton( cgButtonControl * pButton )
{
    mAcceptButton = pButton;
}

//-----------------------------------------------------------------------------
//  Name : getAcceptButton ()
/// <summary>
/// Get the default button that will be triggered when the user presses 'enter'
/// to accept the form.
/// </summary>
//-----------------------------------------------------------------------------
cgButtonControl * cgUIForm::getAcceptButton( ) const
{
    return mAcceptButton;
}

//-----------------------------------------------------------------------------
//  Name : setCancelButton ()
/// <summary>
/// Set the default button to be triggered when the user presses 'escape'
/// to cancel the form.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIForm::setCancelButton( cgButtonControl * pButton )
{
    mCancelButton = pButton;
}

//-----------------------------------------------------------------------------
//  Name : getCancelButton ()
/// <summary>
/// Get the default button that will be triggered when the user presses 'escape'
/// to cancel the form.
/// </summary>
//-----------------------------------------------------------------------------
cgButtonControl * cgUIForm::getCancelButton( ) const
{
    return mCancelButton;
}

//-----------------------------------------------------------------------------
//  Name : onPreCreateForm () (Virtual)
/// <summary>
/// This method is called during form creation in order to allow the
/// derived class to adjust the form properties as necessary prior to 
/// creation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onPreCreateForm( cgUIFormProperties & Properties )
{
    // Notify the script (if any).
    if ( mFormScriptObject )
    {
        try
        {
            bool bSuccess, bResult;
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("FormProperties &"), (void*)&Properties ) );
            bResult = mFormScriptObject->executeMethodBool( _T("onPreCreateForm"), ScriptArgs, true, &bSuccess );
            if ( bSuccess )
                return bResult;
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onPreCreateForm() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;
        
        } // End catch exception

    } // End if valid

    // Success by default.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onCreateForm () (Virtual)
/// <summary>
/// This method is called during form creation in order to allow the
/// derived class to create child controls and define the form.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onCreateForm( )
{
    // Notify the script (if any).
    if ( mFormScriptObject )
    {
        try
        {
            bool bSuccess, bResult;
            cgScriptArgument::Array ScriptArgs;
            bResult = mFormScriptObject->executeMethodBool( _T("onCreateForm"), ScriptArgs, true, &bSuccess );
            if ( bSuccess )
                return bResult;
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute onCreateForm() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;
        
        } // End catch exception

    } // End if valid

    // Success by default.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Do nothing if form is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseButtonDown( nButtons, Position ) == true )
    {
        // Move this form to the top, we used this message
        mUILayer->bringToFront();
        return true;
    
    } // End if some child processed

    // Only process if no other control is captured
    if ( mUIManager->getCapture() != CG_NULL )
        return false;
    
    // Is this within our control's rectangle?
    if ( pointInControl( Position ) == true )
    {
        // Already captured?
        if ( mUIManager->getCapture() == CG_NULL )
        {
            mUIManager->setCapture( this );

            // Store the current position for movement tracking
            mOldCursorPos = Position;
            
        } // End if not captured yet

        // Bring the layer to the front
        mUILayer->bringToFront();

        // We processed this
        return true;
    
    } // End if point within the control

    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonUp () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Call base class implementation
    if ( cgUIControl::onMouseButtonUp( nButtons, Position ) == true )
        return true;

    // We only pay attention to the mouse up if we were previously captured
    if ( mUIManager->getCapture() == this )
    {
        // Uncapture the form
        mUIManager->setCapture( CG_NULL );

        // We processed this
        return true;

    } // End if we're the captured control

    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// Triggered whenever the mouse moves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    cgUIControl * pCapturedControl = mUIManager->getCapture();

    // Are we the captured item?
    if ( pCapturedControl == this )
    {
        cgSize OldSize = getSize();

        // Compute the initial offset amount. The amount that we actually
        // offset may be adjusted depending on the viability of the operation
        // being performed (i.e. resize was only partially possible due to
        // minimum or maximum size constraints).
        cgPoint OffsetAmount( Position.x - mOldCursorPos.x, Position.y - mOldCursorPos.y );

        // Is the previous cursor position over any of our resize handles (if any defined)
        if ( mProperties.sizable )
        {
            switch ( mCapturedHandle )
            {
                case cgUIHandleType::N:
                    mUIManager->selectCursor( _T("SizeNS") );

                    // Resize the form and reposition it
                    setSize( mSize.width, mSize.height - OffsetAmount.y );
                    OffsetAmount.y = OldSize.height - mSize.height;
                    move( 0, OffsetAmount.y );
                    break;

                case cgUIHandleType::NE:
                    mUIManager->selectCursor( _T("SizeNESW") );

                    // Resize the form and reposition it
                    setSize( mSize.width + OffsetAmount.x, mSize.height - OffsetAmount.y );
                    OffsetAmount.x = mSize.width - OldSize.width;
                    OffsetAmount.y = OldSize.height - mSize.height;
                    move( 0, OffsetAmount.y );
                    break;

                case cgUIHandleType::E:
                    mUIManager->selectCursor( _T("SizeWE") );

                    // Resize the form
                    setSize( mSize.width + OffsetAmount.x, mSize.height );
                    OffsetAmount.x = mSize.width - OldSize.width;
                    break;

                case cgUIHandleType::SE:
                    mUIManager->selectCursor( _T("SizeNWSE") );

                    // Resize the form
                    setSize( mSize.width + OffsetAmount.x, mSize.height + OffsetAmount.y );
                    OffsetAmount.x = mSize.width - OldSize.width;
                    OffsetAmount.y = mSize.height - OldSize.height;
                    break;

                case cgUIHandleType::S:
                    mUIManager->selectCursor( _T("SizeNS") );

                    // Resize the form
                    setSize( mSize.width, mSize.height + OffsetAmount.y );
                    OffsetAmount.y = mSize.height - OldSize.height;
                    break;

                case cgUIHandleType::SW:
                    mUIManager->selectCursor( _T("SizeNESW") );
                    
                    // Resize the form and reposition it
                    setSize( mSize.width - OffsetAmount.x, mSize.height + OffsetAmount.y );
                    OffsetAmount.x = OldSize.width - mSize.width;
                    OffsetAmount.y = mSize.height - OldSize.height;
                    move( OffsetAmount.x, 0 );
                    break;

                case cgUIHandleType::W:
                    mUIManager->selectCursor( _T("SizeWE") );
                    
                    // Resize the form and reposition it
                    setSize( mSize.width - OffsetAmount.x, mSize.height );
                    OffsetAmount.x = OldSize.width - mSize.width;
                    move( OffsetAmount.x, 0 );
                    break;

                case cgUIHandleType::NW:
                    mUIManager->selectCursor( _T("SizeNWSE") );

                    // Resize the form and reposition it
                    setSize( mSize.width - OffsetAmount.x, mSize.height - OffsetAmount.y );
                    OffsetAmount.x = OldSize.width - mSize.width;
                    OffsetAmount.y = OldSize.height - mSize.height;
                    move( OffsetAmount.x, OffsetAmount.y );
                    break;

                default:
                    // Move the form if we're entitled to
                    if ( mProperties.movable == true )
                        move( OffsetAmount.x, OffsetAmount.y );
                    break;

            } // End Switch Handle

        } // End if can resize
        else
        {
            // Just move the form if we're entitled to
            if ( mProperties.movable == true )
                move( OffsetAmount.x, OffsetAmount.y );

        } // End if cannot reisize

        // Update the old cursor position
        mOldCursorPos.x += OffsetAmount.x;
        mOldCursorPos.y += OffsetAmount.y;
    
        // Nobody else should process this message
        return true;
    
    } // End if the form is captured and is sizable
    else if ( mProperties.sizable )
    {
        // Is the cursor over any of our resize handles (if any defined)
        cgUIHandleType::Base Handle = pointOverHandle( Position );
        switch ( Handle )
        {
            case cgUIHandleType::N:
            case cgUIHandleType::S:
                mUIManager->selectCursor( _T("SizeNS") );
                break;

            case cgUIHandleType::NE:
            case cgUIHandleType::SW:
                mUIManager->selectCursor( _T("SizeNESW") );
                break;

            case cgUIHandleType::W:
            case cgUIHandleType::E:
                mUIManager->selectCursor( _T("SizeWE") );
                break;

            case cgUIHandleType::NW:
            case cgUIHandleType::SE:
                mUIManager->selectCursor( _T("SizeNWSE") );
                break;

        } // End Switch Handle

        // Store the handle that currently contains the cursor (if any)
        mCapturedHandle = Handle;

    } // End if form is not captured but is sizable
    else
    {
        // No handle captured
        mCapturedHandle = cgUIHandleType::Invalid;
    
    } // End if not captured and not sizable

    // Call base class implementation
    if ( cgUIControl::onMouseMove( Position, Offset ) == true )
        return true;
    
    // We handled this message if the cursor was inside the control area
    return pointInControl( Position );
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Virtual)
/// <summary>
/// This method is called whenever a key is pressed, and subsequent times
/// if the key is held taking into account repeat delay and rate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgUIForm::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;
    
    // Call base class implementation
    if ( cgUIControl::onKeyPressed( nKeyCode, nModifiers ) == true )
        return true;

    // Default accept / cancel handlers?
    if ( nKeyCode == cgKeys::Return )
    {
        if ( mAcceptButton )
        {
            // Raise the 'onClick' event.
            mAcceptButton->onClick();
            return true;
        
        } // End if has accept

    } // End if Return
    else if ( nKeyCode == cgKeys::Escape )
    {
        if ( mCancelButton )
        {
            // Raise the 'onClick' event.
            mCancelButton->onClick();
            return true;
        
        } // End if has cancel

    } // End if Return

    // We did not handle this message.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the form is resized.
/// </summary>
//-----------------------------------------------------------------------------
void cgUIForm::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    cgRect rcControl;

    // Call the base class implementation
    cgUIControl::onSize( nWidth, nHeight );

    // Recompute the layout of the secondary form elements
    // First, retrieve the form description from the current skin
    cgUISkin                * pCurrentSkin = mUIManager->getCurrentSkin();
    const cgUIFormStyleDesc & FormDesc     = pCurrentSkin->getFormStyleDefinition( cgUIFormStyle::Overlapped );
    
    // Position and size the caption
    if ( mCaption != CG_NULL )
    {
        rcControl = elementAreaToClientRect( FormDesc.caption );
        mCaption->setPosition( rcControl.left, rcControl.top );
        mCaption->setSize( rcControl.right - rcControl.left, rcControl.bottom - rcControl.top );
    
    } // End if caption available

    // Position the control buttons
    if ( mCloseButton != CG_NULL )
    {
        rcControl = elementAreaToClientRect( FormDesc.buttonClose );
        mCloseButton->setPosition( rcControl.left, rcControl.top );
    } // End if close button available
    if ( mMinimizeButton != CG_NULL )
    {
        rcControl = elementAreaToClientRect( FormDesc.buttonMinimize );
        mMinimizeButton->setPosition( rcControl.left, rcControl.top );
    } // End if minimize button available
    if ( mMaximizeButton != CG_NULL )
    {
        rcControl = elementAreaToClientRect( FormDesc.buttonMaximize );
        mMaximizeButton->setPosition( rcControl.left, rcControl.top );
    } // End if maximize button available
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual)
/// <summary>
/// Update the common text string for the control (i.e. caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgUIForm::setControlText( const cgString & strText )
{
    // Call base class implementation
    cgUIControl::setControlText( strText );

    // Update the child label control with the caption text
    if ( mCaption != CG_NULL )
        mCaption->setControlText( strText );

}

//-----------------------------------------------------------------------------
//  Name : elementAreaToClientRect ()
/// <summary>
/// Given an element area definition defined within the skin, compute the
/// actual rectangle in client space.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgUIForm::elementAreaToClientRect( const cgUIElementArea & Area )
{
    cgRect  rcClient       = Area.bounds;
    cgPoint ptClientOffset = getControlOrigin( cgControlCoordinateSpace::ClientRelative );

    // Take the alignment specified into account
    if ( Area.align[0] == 1 )
        rcClient.left = mSize.width - rcClient.left;
    if ( Area.align[1] == 1 )
        rcClient.top = mSize.height - rcClient.top;
    if ( Area.align[2] == 1 )
        rcClient.right = mSize.width - rcClient.right;
    if ( Area.align[3] == 1 )
        rcClient.bottom = mSize.height - rcClient.bottom;

    // Convert to client space
    rcClient.left   += ptClientOffset.x;
    rcClient.right  += ptClientOffset.x;
    rcClient.top    += ptClientOffset.y;
    rcClient.bottom += ptClientOffset.y;
    
    // Return the rectangle
    return rcClient;
}