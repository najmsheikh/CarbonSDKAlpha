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
// Name : cgCheckBoxControl.cpp                                              //
//                                                                           //
// Desc : Built in user interface check box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCheckBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgCheckBoxControl.h>
#include <Interface/Controls/cgLabelControl.h>
#include <Interface/cgUIManager.h>
#include <System/cgMessageTypes.h>

///////////////////////////////////////////////////////////////////////////////
// cgCheckBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCheckBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCheckBoxControl::cgCheckBoxControl( ) : cgUIControl( Simple, _T("") )
{
    // Initialize variables to sensible defaults

    // Create and attach the check display control
    mCheckedControl = new cgUIControl( Simple, _T("CheckBoxChecked") );
    addChildControl( mCheckedControl );
    mUncheckedControl = new cgUIControl( Simple, _T("CheckBoxUnchecked") );
    addChildControl( mUncheckedControl );

    // Start in unchecked state.
    mCheckedControl->setVisible( false );
    mCurrentControl = mUncheckedControl;
    
    // Create and attach the label control
    mLabel = new cgLabelControl();
    addChildControl( mLabel );

    // Set up the label
    mLabel->setMultiline( false );
    mLabel->setHorizontalAlignment( cgHorizontalAlignment::Left );
    mLabel->setVerticalAlignment( cgVerticalAlignment::Middle );

    // Set default padding
    setPadding( 5, 2, 2, 2 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgCheckBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCheckBoxControl::~cgCheckBoxControl( )
{
    // Clear variables
    // Note : We don't release the label control as we don't directly own it.
    //        This will be cleaned up automatically elsewhere.
    mLabel              = CG_NULL;
    mCheckedControl     = CG_NULL;
    mUncheckedControl   = CG_NULL;
    mCurrentControl     = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCheckBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UICheckBoxControl )
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
void cgCheckBoxControl::onInitControl( )
{
    // Call base class implementation
    cgUIControl::onInitControl();

    // Position the checked/unchecked display boxes.
    cgSize ClientSize = getClientSize();
    cgSize BoxSize = mCheckedControl->getSize();
    mCheckedControl->setPosition( -mPadding.left, (cgInt32)(ClientSize.height/2.0f - BoxSize.height/2.0f) );
    mUncheckedControl->setPosition( mCheckedControl->getPosition() );

    // Resize and reposition the child label control
    mLabel->setPadding( mPadding );
    mLabel->setPosition( BoxSize.width - mPadding.left, -mPadding.top );
    mLabel->setSize( mSize.width - BoxSize.width, mSize.height );
}

//-----------------------------------------------------------------------------
//  Name : setTextColor () (Virtual)
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgCheckBoxControl::setTextColor( const cgColorValue & Color )
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
void cgCheckBoxControl::setControlText( const cgString & strText )
{
    // Call base class implementation
    cgUIControl::setControlText( strText );

    // Update the child label control with the button text
    mLabel->setControlText( strText );
}

//-----------------------------------------------------------------------------
//  Name : setChecked ()
/// <summary>
/// Set the checked state of this check box.
/// </summary>
//-----------------------------------------------------------------------------
void cgCheckBoxControl::setChecked( bool checked )
{
    // No-op?
    if ( checked && mCurrentControl == mCheckedControl )
        return;
    if ( !checked && mCurrentControl == mUncheckedControl )
        return;

    // Hide the current control.
    mCurrentControl->setVisible( false );

    // Select the new control
    mCurrentControl = (checked) ? mCheckedControl : mUncheckedControl;

    // Display it
    mCurrentControl->setVisible( true );

    // Raise the event with registered targets / script
    raiseEvent( cgSystemMessages::UI_CheckBox_OnCheckedStateChange, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : isChecked ()
/// <summary>
/// Determine if this check box is currently in a checked state.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCheckBoxControl::isChecked( ) const
{
    return (mCurrentControl == mCheckedControl);
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Called in response to a resize event.
/// </summary>
//-----------------------------------------------------------------------------
void cgCheckBoxControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Call the base class implementation
    cgUIControl::onSize( nWidth, nHeight );

    // Position the checked/unchecked display boxes.
    cgSize ClientSize = getClientSize();
    cgSize BoxSize = mCheckedControl->getSize();
    mCheckedControl->setPosition( -mPadding.left, (cgInt32)(ClientSize.height/2.0f - BoxSize.height/2.0f) );
    mUncheckedControl->setPosition( mCheckedControl->getPosition() );

    // Resize and reposition the child label control
    mLabel->setPadding( mPadding );
    mLabel->setPosition( BoxSize.width - mPadding.left, -mPadding.top );
    mLabel->setSize( mSize.width - BoxSize.width, mSize.height );
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCheckBoxControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
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
                // Use "pressed" image
                mCurrentControl->setRenderMode( RenderMode_Pressed );

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
bool cgCheckBoxControl::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
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
            // Set back to normal
            mCurrentControl->setRenderMode( RenderMode_Normal );

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
void cgCheckBoxControl::onClick(  )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return;

    // Restore rendering mode before we alter.
    mCurrentControl->setRenderMode( RenderMode_Normal );

    // Toggle checked state.
    if ( mCurrentControl == mCheckedControl )
        setChecked( false );
    else if ( mCurrentControl == mUncheckedControl )
        setChecked( true );

    // Set the new control to pressed state.
    mCurrentControl->setRenderMode( RenderMode_Pressed );

    // Raise the onClick event with registered targets / script
    raiseEvent( cgSystemMessages::UI_CheckBox_OnClick, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : onMouseMove () (Virtual)
/// <summary>
/// Triggered whenever the mouse moves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCheckBoxControl::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseMove( Position, Offset ) )
        return true;

    // Are we the captured item?
    cgUIControl * pCapturedControl = mUIManager->getCapture();
    bool bPointInControl = pointInControl( Position );
    if ( pCapturedControl == this )
    {
        // Is this within our control's rectangle?
        if ( bPointInControl )
        {
            // Control is enabled?
            if ( isEnabled() )
            {
                // Use "pressed" image
                mCurrentControl->setRenderMode( RenderMode_Pressed );
            
            } // End if control enabled
        
        } // End if show as pressed again
        else
        {
            // Control is enabled?
            if ( isEnabled() )
            {
                // Use "hover" image.
                mCurrentControl->setRenderMode( RenderMode_Hover );

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
            mCurrentControl->setRenderMode( RenderMode_Hover );
        else
            mCurrentControl->setRenderMode( RenderMode_Normal );
    
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
void cgCheckBoxControl::renderSecondary( )
{
    cgUIControl::renderSecondary();
}