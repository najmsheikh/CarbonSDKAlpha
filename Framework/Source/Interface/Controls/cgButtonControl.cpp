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
// Name : cgButtonControl.cpp                                                //
//                                                                           //
// Desc : Built in user interface button control.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgButtonControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgButtonControl.h>
#include <Interface/Controls/cgLabelControl.h>
#include <Interface/Controls/cgImageBoxControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUIForm.h>
#include <System/cgMessageTypes.h>

// ToDo: Remove these comments once completed.
// ToDo: Button needs to more robustly handle images (and image+text) scenarios.

///////////////////////////////////////////////////////////////////////////////
// cgButtonControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgButtonControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgButtonControl::cgButtonControl( ) : cgUIControl( Complex, _T("Button") )
{
    // Initialize variables to sensible defaults

    // Create and attach the button image control
    mImage = new cgImageBoxControl();
    addChildControl( mImage );
    
    // Create and attach the button label control
    mLabel = new cgLabelControl();
    addChildControl( mLabel );

    // Set up the label
    mLabel->setMultiline( true );
    mLabel->setHorizontalAlignment( cgHorizontalAlignment::Center );
    mLabel->setVerticalAlignment( cgVerticalAlignment::Middle );
}

//-----------------------------------------------------------------------------
//  Name : cgButtonControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgButtonControl::cgButtonControl( const cgString & strElementName ) : cgUIControl( Simple, strElementName )
{
    // Initialize variables to sensible defaults

    // Create and attach the button image control
    mImage = new cgImageBoxControl();
    addChildControl( mImage );
    
    // Create and attach the button label control
    mLabel = new cgLabelControl();
    addChildControl( mLabel );

    // Set up the label
    mLabel->setMultiline( true );
    mLabel->setHorizontalAlignment( cgHorizontalAlignment::Center );
    mLabel->setVerticalAlignment( cgVerticalAlignment::Middle );
}

//-----------------------------------------------------------------------------
//  Name : ~cgButtonControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgButtonControl::~cgButtonControl( )
{
    // Unset as the default form accept / cancel button
    // if we're set as one.
    if ( mRootForm )
    {
        if ( mRootForm->getAcceptButton() == this )
            mRootForm->setAcceptButton( CG_NULL );
        else if ( mRootForm->getCancelButton() == this )
            mRootForm->setCancelButton( CG_NULL );

    } // End if assigned.

    // Clear variables
    // Note : We don't release the label control as we don't directly own it.
    //        This will be cleaned up automatically elsewhere.
    mLabel = CG_NULL;
    mImage = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgButtonControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIButtonControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : onInitControl () (Virtual)
/// <summary>
/// Triggered whenever the control has been initialized.
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::onInitControl( )
{
    // Call base class implementation
    cgUIControl::onInitControl();

    // Get size of client and image ready for resizing
    cgRect ClientSize = getClientArea( cgControlCoordinateSpace::ClientRelative );
    cgSize ImageSize  = mImage->getImageSize();

    // Resize the child label and image controls
    mLabel->setPosition( 0, 0 );
    mLabel->setSize( ClientSize.right, ClientSize.bottom );
    mImage->setPosition( (ClientSize.right/2) - (ImageSize.width/2), (ClientSize.bottom/2) - (ImageSize.height/2) );
    mImage->setSize( ImageSize );
}

//-----------------------------------------------------------------------------
//  Name : setTextColor () (Virtual)
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::setTextColor( const cgColorValue & Color )
{
    // Call base class implementation.
    cgUIControl::setTextColor( Color );

    // Pass through to label.
    mLabel->setTextColor( Color );
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual)
/// <summary>
/// Update the common text string for the control (i.e. Caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::setControlText( const cgString & strText )
{
    // Call base class implementation
    cgUIControl::setControlText( strText );

    // Update the child label control with the button text
    mLabel->setControlText( strText );

    
    // ToDo: Need to reposition image if there wasn't text before?
}

//-----------------------------------------------------------------------------
//  Name : setImage ()
/// <summary>
/// Allows external code / scripts to select the image that should be
/// rendered inside this control.
/// Note : Reference is in the format "LIBRARY::ITEM".
/// </summary>
//-----------------------------------------------------------------------------
bool cgButtonControl::setImage( const cgString & strImageRef )
{
    // Allow image to auto-size itself
    mImage->autoSize = true;

    // Set the image to our internal image control
    if ( !mImage->setImage( strImageRef ) )
        return false;

    // Any text to render at the moment?
    if ( mControlText.empty() )
    {
        // Position the image so that it sits in the center of the button
        // if there is currently no text.
        cgSize ImageSize  = mImage->getImageSize();
        cgRect ClientSize = getClientArea( cgControlCoordinateSpace::ClientRelative );
        mImage->setPosition( (ClientSize.right/2) - (ImageSize.width/2), (ClientSize.bottom/2) - (ImageSize.height/2) );
    
    } // End if just image
    else
    {
        // ToDo: Text and image)

    } // End if text and image
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getImage ()
/// <summary>
/// Return the currently set image reference.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgButtonControl::getImage( ) const
{
    return mImage->getImage();
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Called in response to a resize event.
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Call the base class implementation
    cgUIControl::onSize( nWidth, nHeight );

    // Get current client size
    cgRect ClientSize = getClientArea( cgControlCoordinateSpace::ClientRelative );

    // Any text to render at the moment?
    if ( mControlText.empty() )
    {
        // Position the image so that it sits in the center of the button
        // if there is currently no text.
        cgSize ImageSize  = mImage->getImageSize();
        mImage->setPosition( (ClientSize.right/2) - (ImageSize.width/2), (ClientSize.bottom/2) - (ImageSize.height/2) );
    
    } // End if just image
    else
    {
        // ToDo: Reposition image too.

        // Resize our child label control to fill the client area
        mLabel->setSize( ClientSize.right, ClientSize.bottom );

    } // End if text and image
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgButtonControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseButtonDown( nButtons, Position ) )
        return true;

    // Only process if no other control is captured
    if ( mUIManager->getCapture() )
        return false;

    // Is this within our control's rectangle?
    if ( pointInControl( Position ) )
    {
        // Already captured?
        if ( !mUIManager->getCapture() )
        {
            // Capture the control
            mUIManager->setCapture( this );
            
            // Control is enabled?
            if ( isEnabled() )
            {
                // Use "pressed" image and offset the label 
                setRenderMode( RenderMode_Pressed );
                mLabel->setPosition( 1, 1 );

                // ToDo: Shift image too so it looks like it's clicking down

            } // End if enabled

            // We're now the focus control
            mUIManager->setFocus( this );
        
        } // End if not captured yet

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseButtonDown, &UI_OnMouseButtonDownArgs( nButtons, Position ) );

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
bool cgButtonControl::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() && mUIManager->getCapture() != this )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseButtonUp( nButtons, Position ) )
        return true;

    // We only pay attention to the mouse up if we were previously captured
    if ( mUIManager->getCapture() == this )
    {
        // Control is enabled?
        if ( isEnabled() )
        {
            // Set back to normal image and reset the label 
            setRenderMode( RenderMode_Normal );
            mLabel->setPosition( 0, 0 );

        } // End if enabled

        // Uncapture
        mUIManager->setCapture( CG_NULL );

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseButtonUp, &UI_OnMouseButtonUpArgs( nButtons, Position ) );

        // Raise the 'onClick' event if we released inside the control and control is enabled
        if ( isEnabled() && pointInControl( Position ) )
            onClick();

        // We processed this
        return true;

    } // End if we're the captured control

    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onClick () (Virtual)
/// <summary>
/// This method is called whenever the button registers an actual click.
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::onClick(  )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return;

    // Raise the event with registered targets / script
    raiseEvent( cgSystemMessages::UI_Button_OnClick, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// Triggered whenever the mouse moves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgButtonControl::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseMove( Position, Offset ) )
        return true;

    cgUIControl * pCapturedControl = mUIManager->getCapture();
    bool bPointInControl = pointInControl( Position );
    if ( bPointInControl )
        mUIManager->selectCursor( ( isEnabled() ) ? _T("Busy") : _T("Arrow") );
    
    // Are we the captured item?
    if ( pCapturedControl == this )
    {
        // Is this within our control's rectangle?
        if ( bPointInControl )
        {
            // Control is enabled?
            if ( isEnabled() )
            {
                // Use "pressed" image and offset label
                setRenderMode( RenderMode_Pressed );
                mLabel->setPosition( 1, 1 );
            
            } // End if control enabled
        
        } // End if show as pressed again
        else
        {
            // Control is enabled?
            if ( isEnabled() )
            {
                // Use "hover" image and reset label position
                setRenderMode( RenderMode_Hover );
                mLabel->setPosition( 0, 0 );

            } // End if control is enabled
        
        } // End if hover only again

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseMove, &UI_OnMouseMoveArgs( Position, Offset ) );

        // We processed this!
        return true;
    
    } // End if we're the captured item
    else if ( isEnabled() )
    {
        // Set hover state only if nothing is captured and the cursor is inside this control
        if ( !pCapturedControl && bPointInControl )
            setRenderMode( RenderMode_Hover );
        else
            setRenderMode( RenderMode_Normal );
    
    } // End if not the captured item
    
    // Don't halt the processing of this message
    return false;
}


//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgButtonControl::renderSecondary( )
{
    cgUIControl::renderSecondary();
}