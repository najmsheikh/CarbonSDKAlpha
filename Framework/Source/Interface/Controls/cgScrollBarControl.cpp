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
// Name : cgScrollBarControl.cpp                                             //
//                                                                           //
// Desc : Built in user interface scroll bar control.                        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScrollBarControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgScrollBarControl.h>
#include <Interface/Controls/cgButtonControl.h>
#include <Interface/cgUIManager.h>
#include <System/cgMessageTypes.h>

// ToDo: Remove these comments once completed.
// ToDo: Add scrollbar cursor key support and possibly pgdown/pgup

///////////////////////////////////////////////////////////////////////////////
// cgScrollBarControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScrollBarControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScrollBarControl::cgScrollBarControl( bool bHorizontal /* = false */ ) : 
    cgUIControl( Complex, (bHorizontal) ? _T("ScrollBarFrame.Horizontal") : _T("ScrollBarFrame.Vertical") )
{
    // Create the child negative, positive and thumb track buttons
    mButtonNegative = new cgButtonControl();
    mButtonPositive = new cgButtonControl();
    mThumbButton    = new cgButtonControl();
    addChildControl( mButtonNegative );
    addChildControl( mButtonPositive );
    addChildControl( mThumbButton );

    // Store specified values
    mHorizontal        = bHorizontal;

    // Reset variables to sensible defaults
    mMinimumValue          = 0;
    mMaximumValue          = 100;
    mValue             = 0;
    mVisiblePercentage = 0.5f;
    mSmallStep         = 1.0f;
    mLargeStep         = 1.0f;

}

//-----------------------------------------------------------------------------
//  Name : ~cgScrollBarControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScrollBarControl::~cgScrollBarControl()
{
    // Release allocated memory
    
    // Clear variables
    // Note : We don't release the child controls because we don't directly own 
    //        them. These will be cleaned up automatically elsewhere.
    mButtonNegative = CG_NULL;
    mButtonPositive = CG_NULL;
    mThumbButton    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScrollBarControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIScrollBarControl )
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
void cgScrollBarControl::onInitControl( )
{
    // Call base class implementation
    cgUIControl::onInitControl();

    // Set the images for the scrollbar child buttons
    if ( !mHorizontal )
    {
        mButtonNegative->setImage( _T("::UpArrow") );
        mButtonPositive->setImage( _T("::DownArrow") );
        mThumbButton->setImage( _T("::VerticalGripper") );

    } // End if horizontal scrollbar
    else
    {
        mButtonNegative->setImage( _T("::LeftArrow") );
        mButtonPositive->setImage( _T("::RightArrow") );
        mThumbButton->setImage( _T("::HorizontalGripper") );

    } // End if vertical scrollbar

    // Register this as the target for any applicable events raised by child controls
    mButtonNegative->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
    mButtonPositive->registerEventHandler( cgSystemMessages::UI_Button_OnClick, getReferenceId() );
    mThumbButton->registerEventHandler( cgSystemMessages::UI_OnMouseButtonDown, getReferenceId() );
    mThumbButton->registerEventHandler( cgSystemMessages::UI_OnMouseMove, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Called in response to a resize event.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    cgRect rcClient;

    // Pass through to base class first
    cgUIControl::onSize( nWidth, nHeight );

    // Retrieve client area rectangle so we can corretly position the child controls
    rcClient = getClientArea( cgControlCoordinateSpace::ClientRelative );

    // Position and size the scroll bar buttons
    if ( !mHorizontal )
    {
        cgInt32 nClientWidth = (rcClient.right - rcClient.left);

        // Set position of controls to two extremes of the scrollbar client area
        mButtonNegative->setPosition( rcClient.left, rcClient.top );
        mButtonPositive->setPosition( rcClient.left, rcClient.bottom - nClientWidth );

        // Set size such that it they are square
        mButtonNegative->setSize( nClientWidth, nClientWidth );
        mButtonPositive->setSize( nClientWidth, nClientWidth );

        // Update thumb button
        updateThumbButton();

    } // End if vertical
    else
    {
        cgInt32 nClientHeight = rcClient.bottom - rcClient.top;

        // Set position of controls to two extremes of the scrollbar client area
        mButtonNegative->setPosition( rcClient.left, rcClient.top );
        mButtonPositive->setPosition( rcClient.right - nClientHeight, rcClient.top );

        // Set size such that it they are square
        mButtonNegative->setSize( nClientHeight, nClientHeight );
        mButtonPositive->setSize( nClientHeight, nClientHeight );

        // Update thumb button
        updateThumbButton();

    } // End if horizontal
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScrollBarControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseButtonDown( nButtons, Position ) )
        return true;
    
    // If we're over the scroll channel (everything else will have been
    // handled by the base class implementation), scroll it.
    if ( pointInControl( Position ) )
    {
        // Control is enabled ?
        if ( isEnabled() )
        {
            // Above or below the thumb button?
            if ( Position.y < mThumbButton->getControlArea( cgControlCoordinateSpace::ScreenRelative ).top )
            {
                // Step back by the large step amount.
                stepLarge( -1 );
            
            } // End if above thumb button
            else
            {
                // Step forward by the large step amount.
                stepLarge( 1 );

            } // End if below thumb button
        
        } // End if control is enabled

        // Raise the event
        raiseEvent( cgSystemMessages::UI_OnMouseButtonDown, &UI_OnMouseButtonDownArgs( nButtons, Position ) );

        // We processed this
        return true;

    } // End if within text area

    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// This method is called whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScrollBarControl::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Note: We're going to intercept this message before any of our children
    //       receive it. As a result, we call the base class last if we didn't
    //       process the message.

    // If we're over the scroll bar control as a whole, scroll it.
    if ( pointInControl( Position ) )
    {
        // Control is enabled ?
        if ( isEnabled() )
        {
            // Step by the small step amount.
            stepSmall( -nDelta );
        
        } // End if control is enabled

        // Raise the event
        raiseEvent( cgSystemMessages::UI_OnMouseWheelScroll, &UI_OnMouseWheelScrollArgs( nDelta, Position ) );

        // We processed this
        return true;

    } // End if within text area

    // Call base class implementation
    return cgUIControl::onMouseWheelScroll( nDelta, Position );
}

//-----------------------------------------------------------------------------
//  Name : stepSmall () (Virtual)
/// <summary>
/// Step the scroll bar value by the amount specified multiplied by the
/// small step value.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::stepSmall( cgInt32 nAmount )
{
    // Shift value
    mValue += (cgFloat)nAmount * mSmallStep;

    // Clamp value
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    
    // Update the thumb button
    updateThumbButton();

    // Trigger the value change event
    raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );

}

//-----------------------------------------------------------------------------
//  Name : stepLarge () (Virtual)
/// <summary>
/// Step the scroll bar value by the amount specified multiplied by the
/// large step value.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::stepLarge( cgInt32 nAmount )
{
    // Shift value
    mValue += (cgFloat)nAmount * mLargeStep;

    // Clamp value
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    
    // Update the thumb button
    updateThumbButton();

    // Trigger the value change event
    raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );
}

//-----------------------------------------------------------------------------
//  Name : updateThumbButton () (Protected)
/// <summary>
/// Called whenever a property has changed that requires the thumb
/// button's position or size to be recalculated.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::updateThumbButton()
{
    cgRect  ScrollArea;
    cgSize  ScrollSize;
    cgFloat fRangeDelta, fPositionScalar;

    // Retrieve dimensions of the scroll area so we can correctly position the child controls
    ScrollArea        = getScrollArea( );
    ScrollSize.width  = ScrollArea.right - ScrollArea.left;
    ScrollSize.height = ScrollArea.bottom - ScrollArea.top;

    // Compute the position scalar based on the value / range
    fRangeDelta = max( 0, mMaximumValue - mMinimumValue );
    if ( fRangeDelta > 0 )
        fPositionScalar = mValue / (mMaximumValue - mMinimumValue);
    else
        fPositionScalar = 0.0;

    // ToDo: Thumb button minimum size should really depend on the size of the gripper image?

    // Update size / position
    if ( !mHorizontal )
    {
        // Compute the thumb button's size and set it
        cgUInt32 nHeight = max( 13, (cgUInt32)((cgFloat)ScrollSize.height * mVisiblePercentage) );
        mThumbButton->setSize( ScrollSize.width, nHeight );

        // Compute its position and set it
        cgUInt32 nPosition = (cgUInt32)(fPositionScalar * ((cgFloat)ScrollSize.height - (cgFloat)nHeight));
        mThumbButton->setPosition( 0, ScrollArea.top + nPosition );
        
    } // End if vertical
    else
    {
        // Compute the thumb button's size and set it
        cgUInt32 nWidth = max( 13, (cgUInt32)((cgFloat)ScrollSize.width * mVisiblePercentage) );
        mThumbButton->setSize( nWidth, ScrollSize.height );

        // Compute its position and set it
        cgUInt32 nPosition = (cgUInt32)(fPositionScalar * ((cgFloat)ScrollSize.width - (cgFloat)nWidth));
        mThumbButton->setPosition( ScrollArea.left + nPosition, 0 );

    } // End if horizontal
}

//-----------------------------------------------------------------------------
//  Name : getScrollArea ()
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the netgative/positive buttons (i.e. just the channel in which
/// the thumb track button will travel).
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgScrollBarControl::getScrollArea( cgControlCoordinateSpace::Base Origin ) const
{
    cgRect rcScroll = getClientArea( Origin );

    // Adjust the client area, removing the size of the buttons.
    if ( !mHorizontal )
    {
        rcScroll.top    += rcScroll.right - rcScroll.left;
        rcScroll.bottom -= rcScroll.right - rcScroll.left;
    
    } // End if vertical
    else
    {
        rcScroll.left   += rcScroll.bottom - rcScroll.top;
        rcScroll.right  -= rcScroll.bottom - rcScroll.top;
    
    } // End if horizontal

    // Return the final rectangle
    return rcScroll;
}

//-----------------------------------------------------------------------------
//  Name : getValue ()
/// <summary>
/// Retrieve the current value of the scroll bar
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getValue( ) const
{
    return mValue;
}

//-----------------------------------------------------------------------------
//  Name : getMinimumValue ()
/// <summary>
/// Retrieve the minimum scroll range value.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getMinimumValue( ) const
{
    return mMinimumValue;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumValue ()
/// <summary>
/// Retrieve the maximum scroll range value.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getMaximumValue( ) const
{
    return mMaximumValue;
}

//-----------------------------------------------------------------------------
//  Name : getSmallStep ()
/// <summary>
/// Retrieve the small scroll bar stepping value.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getSmallStep( ) const
{
    return mSmallStep;
}

//-----------------------------------------------------------------------------
//  Name : getLargeStep ()
/// <summary>
/// Retrieve the large scroll bar stepping value.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getLargeStep( ) const
{
    return mLargeStep;
}

//-----------------------------------------------------------------------------
//  Name : getVisiblePercentage ()
/// <summary>
/// Get the size of the thumb button relative to the size of the scroll
/// area. Value range between 0 and 1.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgScrollBarControl::getVisiblePercentage( ) const
{
    return mVisiblePercentage;
}

//-----------------------------------------------------------------------------
//  Name : setValue ()
/// <summary>
/// Set the current value of the scroll bar control.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setValue( cgFloat fValue )
{
    // Update internal value
    mValue = fValue;

    // Clamp
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;

    // Update the thumb button position based on the new value.
    updateThumbButton();

    // Raise event.
    raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );
}

//-----------------------------------------------------------------------------
//  Name : setSmallStep ()
/// <summary>
/// Sets the amount to step when clicking the positive / negative buttons or
/// when calling 'stepSmall()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setSmallStep( cgFloat fValue )
{
    mSmallStep = fValue;
}

//-----------------------------------------------------------------------------
//  Name : setLargeStep ()
/// <summary>
/// Sets the amount to step when clicking somewhere in the scroll channel
/// when calling 'stepLarge()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setLargeStep( cgFloat fValue )
{
    mLargeStep = fValue;
}

//-----------------------------------------------------------------------------
//  Name : setStepSizes ()
/// <summary>
/// Sets the amount to step when clicking either the positive / negative
/// buttons, or somewhere in the scroll channel.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setStepSizes( cgFloat fSmallStep, cgFloat fLargeStep )
{
    mSmallStep = fSmallStep;
    mLargeStep = fLargeStep;
}

//-----------------------------------------------------------------------------
//  Name : setMinimumValue ()
/// <summary>
/// Set the smallest value that can be represented by the scroll bar control.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setMinimumValue( cgFloat fMin )
{
    cgFloat fOldValue = mValue;

    // Update internal values
    mMinimumValue = fMin;

    // Ensure the max value is never less than minimum.
    if ( mMaximumValue < mMinimumValue )
        mMaximumValue = mMinimumValue;

    // Clamp the already set value
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;

    // Update the thumb button position based on the new value.
    updateThumbButton();

    // If value changed, raise event.
    if ( fOldValue != mValue )
        raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );
}

//-----------------------------------------------------------------------------
//  Name : setMaximumValue ()
/// <summary>
/// Set the largest value that can be represented by the scroll bar control.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setMaximumValue( cgFloat fMax )
{
    cgFloat fOldValue = mValue;

    // Update internal values
    mMaximumValue = fMax;

    // Ensure the max value is never less than minimum.
    if ( mMaximumValue < mMinimumValue )
        mMinimumValue = mMaximumValue;

    // Clamp the already set value
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;

    // Update the thumb button position based on the new value.
    updateThumbButton();

    // If value changed, raise event.
    if ( fOldValue != mValue )
        raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );
}

//-----------------------------------------------------------------------------
//  Name : setValueRange ()
/// <summary>
/// Set the current range of values that can be represented by the scroll 
/// bar control.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setValueRange( cgFloat fMin, cgFloat fMax )
{
    cgFloat fOldValue = mValue;

    // Update internal values
    mMinimumValue = fMin;
    mMaximumValue = fMax;

    // Ensure the max value is never less than minimum.
    if ( mMaximumValue < mMinimumValue ) mMinimumValue = mMaximumValue;

    // Clamp the already set value
    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
    if ( mValue > mMaximumValue ) mValue = mMaximumValue;

    // Update the thumb button position based on the new value.
    updateThumbButton();

    // If value changed, raise event.
    if ( fOldValue != mValue )
        raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );
}

//-----------------------------------------------------------------------------
//  Name : setVisiblePercentage ()
/// <summary>
/// Set the size of the thumb button relative to the size of the scroll
/// area. Value range between 0 and 1.
/// </summary>
//-----------------------------------------------------------------------------
void cgScrollBarControl::setVisiblePercentage( cgFloat fPercent )
{
    // Update internal value
    mVisiblePercentage = fPercent;

    // Clamp visible percentage
    if ( mVisiblePercentage < 0.0f ) mVisiblePercentage = 0.0f;
    if ( mVisiblePercentage > 1.0f ) mVisiblePercentage = 1.0f;

    // Update the thumb button scale based on the new value.
    updateThumbButton();
}

//-----------------------------------------------------------------------------
//  Name : getScrollArea () (Overload)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the netgative/positive buttons (i.e. just the channel in which
/// the thumb track button will travel) relative to the control.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgScrollBarControl::getScrollArea( ) const
{
    // Return the rectangle
    return getScrollArea( cgControlCoordinateSpace::ControlRelative );
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgScrollBarControl::processMessage( cgMessage * pMessage )
{
    // Retrieve the reference target
    cgReference * pTarget = cgReferenceManager::getReference( pMessage->fromId );

    // Was this our thumb button message?
    if ( pTarget == mThumbButton )
    {
        switch ( pMessage->messageId )
        {
            case cgSystemMessages::UI_OnMouseButtonDown:
            {
                UI_OnMouseButtonDownArgs * pArgs = (UI_OnMouseButtonDownArgs*)pMessage->messageData;
                mThumbTrackPos  = pArgs->position;
                mThumbTrackValue = mValue;

                break;
            
            } // UI_OnMouseButtonDown
            
            case cgSystemMessages::UI_OnMouseMove:
            {
                UI_OnMouseMoveArgs * pArgs = (UI_OnMouseMoveArgs*)pMessage->messageData;
                
                // If the thumb button is currently captured, we should update
                // the scroll bar value accordingly.
                if ( mUIManager->getCapture() == mThumbButton )
                {
                    cgSize  ThumbSize = mThumbButton->getSize();
                    cgPoint ptShift   = cgPoint( pArgs->position.x - mThumbTrackPos.x, pArgs->position.y - mThumbTrackPos.y );
                    cgRect  rcScroll  = getScrollArea();
                    
                    // Compute value shift based on the change in position
                    if ( !mHorizontal )
                        mValue = mThumbTrackValue + (((mMaximumValue - mMinimumValue) / ((cgFloat)(rcScroll.bottom - rcScroll.top) - (cgFloat)ThumbSize.height)) * (cgFloat)ptShift.y);
                    else
                        mValue = mThumbTrackValue + (((mMaximumValue - mMinimumValue) / ((cgFloat)(rcScroll.right - rcScroll.left) - (cgFloat)ThumbSize.width)) * (cgFloat)ptShift.x);

                    // Clamp value
                    if ( mValue > mMaximumValue ) mValue = mMaximumValue;
                    if ( mValue < mMinimumValue ) mValue = mMinimumValue;
                    
                    // Update the thumb button
                    updateThumbButton();

                    // Trigger the value change event
                    raiseEvent( cgSystemMessages::UI_ScrollBar_OnValueChange, &UI_ScrollBar_OnValueChangeArgs( mValue ) );

                } // End if thumb button is currently captured
                
                // Processed message
                return true;
            
            } // UI_OnMouseMove

        } // End switch message
    
    } // End if message from thumb button

    // ToDo: Scroll bar buttons should ideally repeat just like the keyboard..... Should probably treat mouse clicks like key events (similar to Win32) instead of capturing messages

    // Was this our negative button message?
    if ( pTarget == mButtonNegative && pMessage->messageId == cgSystemMessages::UI_Button_OnClick )
    {
        // Step backwards by 1
        stepSmall( -1 );

        // Processed message
        return true;
    
    } // End if message from negative button

    // Was this our positive button message?
    if ( pTarget == mButtonPositive && pMessage->messageId == cgSystemMessages::UI_Button_OnClick )
    {
        // Step forwards by 1
        stepSmall( 1 );

        // Processed message
        return true;
    
    } // End if message from negative button
    
    // Message was not processed
    return cgUIControl::processMessage( pMessage );
}