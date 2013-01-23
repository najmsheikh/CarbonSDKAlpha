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
// Name : cgListBoxControl.cpp                                               //
//                                                                           //
// Desc : Built in user interface list box control.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgListBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgListBoxControl.h>
#include <Interface/Controls/cgScrollBarControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUISkin.h>
#include <Input/cgInputDriver.h>
#include <System/cgMessageTypes.h>

///////////////////////////////////////////////////////////////////////////////
// cgListBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgListBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgListBoxControl::cgListBoxControl( ) : cgUIControl( Complex, _T("ControlFrame") )
{
    // Initialize variables to sensible defaults
    mVerticalScrollAmount   = 0;
    mSelectedIndex          = -1;
    mMetricsDirty           = false;

    // Create the vertical scroll bar that will be used to scroll the list.
    mVerticalScrollBar = new cgScrollBarControl( );
    addChildControl( mVerticalScrollBar );

    // Make sure that the scroll bar can never steal focus.
    mVerticalScrollBar->setCanGainFocus( false );

    // We need to capture value change messages from the scroll bar
    mVerticalScrollBar->registerEventHandler( cgSystemMessages::UI_ScrollBar_OnValueChange, getReferenceId() );

    // Set default padding
    setPadding( 4, 4, 4, 4 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgListBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgListBoxControl::~cgListBoxControl()
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
void cgListBoxControl::dispose( bool disposeBase )
{
    // Release allocated memory
    mItems.clear();

    // Clear variables (controls are not owned by us).
    mVerticalScrollBar = CG_NULL;
    mSelectedIndex     = -1;

    // Dispose of base if required.
    if ( disposeBase )
        cgUIControl::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UIListBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : addItem()
/// <summary>
/// Add a new string to the list box item array. Returns the index of the
/// newly added item.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgListBoxControl::addItem( const cgString & value )
{
    // Add the item to the list.
    mItems.push_back( value );

    // Recalculate list item text metrics if visible, otherwise mark 
    // as dirty for update when we next become visible.
    if ( isVisible( true ) )
        computeTextMetrics();
    else
        mMetricsDirty = true;

    // Return the index of the most recently added item.
    return (cgInt32)mItems.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the layout of the control has been recomputed.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::onSize( cgInt32 width, cgInt32 height )
{
    // Call base class implementation
    cgUIControl::onSize( width, height );

    // Reposition scroll bar (ensure it fills entire control area irrespective of the padding)
    cgRect controlArea = getControlArea( cgControlCoordinateSpace::ClientRelative );
    mVerticalScrollBar->setPosition( controlArea.right - 17, controlArea.top );
    mVerticalScrollBar->setSize( 17, controlArea.height() );
    mVerticalScrollBar->setVisible( false ); // Invisible until overflow detected

    // Recompute the metrics now our scroll bar is correctly sized
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : onMouseButtonDown () (Virtual)
/// <summary>
/// This method is called whenever a mouse button is pressed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
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
    cgRect rcItems = getItemArea( cgControlCoordinateSpace::ScreenRelative );
    if ( rcItems.containsPoint( Position ) )
    {
        // Already captured?
        if ( !mUIManager->getCapture() )
            mUIManager->setCapture( this );

        // We have focus now
        mUIManager->setFocus( this );

        // Control is enabled ?
        if ( isEnabled() )
        {
            // Compute the item that was clicked (Y is offset by padding amount).
            cgTextMetrics::MetricRef clickRef;
            mTextMetrics.characterFromPoint( cgPoint( Position.x - rcItems.left, (Position.y - rcItems.top) + 2), &clickRef );
            if ( clickRef.line == -1 ) // Before first line
                setSelectedIndex( (mItems.empty()) ? -1 : 0 );
            else if ( clickRef.line == -2 ) // After last line
                setSelectedIndex( (cgInt32)mItems.size() - 1 );
            else
                setSelectedIndex( clickRef.line );
        
        } // End if control is enabled
        
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
bool cgListBoxControl::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible UNLESS we are captured.
    if ( !isVisible() && mUIManager->getCapture() != this )
        return false;
    
    // Call base class implementation
    if ( cgUIControl::onMouseButtonUp( nButtons, Position ) )
        return true;

    // We only pay attention to the mouse up if we were previously captured
    if ( mUIManager->getCapture() == this )
    {
        // Uncapture
        mUIManager->setCapture( CG_NULL );

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseButtonUp, &UI_OnMouseButtonUpArgs( nButtons, Position ) );

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
bool cgListBoxControl::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseMove( Position, Offset ) )
        return true;

    // Are we the captured item?
    cgUIControl * pCapturedControl = mUIManager->getCapture();
    if ( pCapturedControl == this )
    {
        /*pManager->selectCursor( _T("IBeam") );

        // Control is enabled ?
        if ( isEnabled() )
        {
            // Compute the caret data and selection rectangle from the cursor position
            resetCaret( Position, true );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if control is enabled*/

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseMove, &UI_OnMouseMoveArgs( Position, Offset ) );
        return true;

    } // End if we're the captured item
    else
    {
        /*// Set the cursor if applicable
        if ( getTextArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( Position ) )
            pManager->selectCursor( _T("IBeam") );*/
    
    } // End if not the captured item
    
    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// This method is called whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseWheelScroll( nDelta, Position ) )
        return true;

    // If we're over our item area, scroll it.
    if ( getItemArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( Position ) )
    {
        mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount - (nDelta * (signed)mTextMetrics.getLineHeight())) );

        // Raise the event
        raiseEvent( cgSystemMessages::UI_OnMouseWheelScroll, &UI_OnMouseWheelScrollArgs( nDelta, Position ) );

        // We processed this
        return true;

    } // End if within text area

    // Don't halt the processing of this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : onKeyPressed () (Virtual)
/// <summary>
/// This method is called whenever a key is pressed, and subsequent times
/// if the key is held taking into account repeat delay and rate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;
    
    // Call base class implementation
    if ( cgUIControl::onKeyPressed( nKeyCode, nModifiers ) )
        return true;

    // Just bail if not enabled
    if ( !isEnabled() )
        return false;

    // Do we currently have focus?
    if ( mUIManager->getFocus() == this )
    {
        // Read only commands
        if ( nKeyCode == cgKeys::Up )
        {
            // Move to previous selection.
            setSelectedIndex( mSelectedIndex - 1 );
            
        } // End if up
        else if ( nKeyCode == cgKeys::Down )
        {
            // Move to next selection
            setSelectedIndex( mSelectedIndex + 1 );
            
        } // End if down
        
        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnKeyPressed, &UI_OnKeyPressedArgs( nKeyCode, nModifiers ) );
        
        // We processed this message
        return true;

    } // End if we currently have focus

    // We did not process this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getItemArea () (Protected)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the scroll bars.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgListBoxControl::getItemArea( cgControlCoordinateSpace::Base Origin ) const
{
    cgRect rcItems = getClientArea( Origin );

    // Adjust the client area, removing the size of the scrollbar if visible
    if ( mVerticalScrollBar->isVisible( false ) )
        rcItems.right -= mVerticalScrollBar->getSize().width;

    // Return the final rectangle
    return rcItems;
}

//-----------------------------------------------------------------------------
//  Name : getItemArea () (Protected)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the scroll bars.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgListBoxControl::getItemArea( ) const
{
    // Return the rectangle
    return getItemArea( cgControlCoordinateSpace::ControlRelative );
}

//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::renderSecondary( )
{
    // Bail if control is not visible
    if ( !isVisible() )
        return;

    // Get access to interface manager
    cgUISkin        * pSkin    = mUIManager->getCurrentSkin();
    cgRenderDriver  * pDriver  = mUIManager->getRenderDriver();
    cgTextEngine    * pEngine  = mUIManager->getTextEngine();

    // Get skin configuration for control rendering
    const cgUISkin::ControlConfig & Config = pSkin->getControlConfig();

    // Selection rect to client area.
    cgRect rcItems = getItemArea( cgControlCoordinateSpace::ScreenRelative );
    pDriver->pushScissorRect( &rcItems );

    // Draw selection rectangle?
    bool selectionVisible = (mSelectedIndex >= 0) && 
                            (mSelectedIndex >= mTextMetrics.getFirstVisibleLine()) &&
                            (mSelectedIndex <= mTextMetrics.getLastVisibleLine());
    if ( isEnabled() && selectionVisible )
    {
        cgColorValue Color = Config.listBox.selectionColor;

        // If we don't currently have focus, use a light gray selection color
        if ( mUIManager->getFocus() != this )
            Color = cgColorValue( 0.7f, 0.7f, 0.7f, 0.3f );

        // Get the rectangle for the selected item.
        const cgTextMetrics::TextLine & Line = mTextMetrics.getLineMetrics( mSelectedIndex );
        cgRect rcDraw = Line.bounds;

        // Use full width and account for item padding.
        rcDraw.left    = 0;
        rcDraw.right   = rcItems.width();
        rcDraw.top    -= 2;
        rcDraw.bottom -= 2;
        
        // Offset rectangle to client area
        rcDraw.left   += rcItems.left;
        rcDraw.right  += rcItems.left;
        rcDraw.top    += rcItems.top;
        rcDraw.bottom += rcItems.top;

        // Ensure rectangle is at least 3 pixels wide (for empty lines)
        if ( (rcDraw.right - rcDraw.left) < 3 )
            rcDraw.right = rcDraw.left + 3;

        // Render the selection rectangle (clipped to the item area).
        rcDraw = cgRect::intersect( rcItems, rcDraw );
        if ( !rcDraw.isEmpty() )
            pDriver->drawRectangle( rcDraw, Color, true );
    
    } // End if draw selection rectangle

    // Reset clipping rect
    pDriver->popScissorRect( );

    // Build text rendering flags.
    cgUInt32 nFlags = cgTextFlags::ClipRectangle | cgTextFlags::Multiline;

    // Draw the text from our pre-cached metrics
    pEngine->setKerning( 0 );
    pEngine->setLineSpacing( 4 ); // Default item padding (2 top, 2 bottom)
    mUIManager->selectFont( getFont() );
    pEngine->setColor( mControlTextColor );
    pEngine->printText( mTextMetrics, cgPoint( rcItems.left, rcItems.top ) );

    // Call base class implementation
    cgUIControl::renderSecondary();
}

//-----------------------------------------------------------------------------
//  Name : computeTextMetrics () (Protected)
/// <summary>
/// Recompute the text metrics (line/character sizes etc.) for use during
/// text processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::computeTextMetrics()
{
    // Retrieve the text engine from the manager, we need low level access
    cgTextEngine * pEngine  = mUIManager->getTextEngine();

    // First build metric computation flags.
    cgUInt32 nFlags = cgTextFlags::ClipRectangle | cgTextFlags::Multiline;
    /*if ( mAllowFormatCode )
        nFlags |= cgTextFlags::AllowFormatCode;*/

    // Generate the display text.
    // ToDo: Cache, or build string containing only items that could
    // possibly be visible.
    cgString strDisplayText;
    for ( size_t i = 0; i < mItems.size(); ++i )
    {
        if ( i != 0 )
            strDisplayText.append( _T("\n") );
        strDisplayText.append( mItems[i] );

    } // Next item

    // Compute metrics for this text relative to the client area first of all
    cgRect rcText = getClientArea( cgControlCoordinateSpace::ClientRelative );
    cgPoint ptOffset( 0, -mVerticalScrollAmount );
    mUIManager->selectFont( getFont() );
    pEngine->setKerning( 0 );
    pEngine->setLineSpacing( 4 ); // Default item padding (2 top, 2 bottom)
    pEngine->setColor( mControlTextColor );
    pEngine->computeTextMetrics( rcText, nFlags, strDisplayText, ptOffset, mTextMetrics );

    // Did the text overflow the original rectangle?
    if ( mTextMetrics.isOverflowing() )
    {
        // Show the scroll bar
        if ( !mVerticalScrollBar->isVisible() )
            mVerticalScrollBar->setVisible( true );
            
        // We must recompute the text metrics again for area minus scroll bar size
        rcText = getItemArea( cgControlCoordinateSpace::ClientRelative );
        pEngine->computeTextMetrics( rcText, nFlags, strDisplayText, ptOffset, mTextMetrics );

        // Set the scroll bar range
        cgRect rcFull = mTextMetrics.getFullBounds();
        mVerticalScrollBar->setValueRange( 0, (cgFloat)((rcFull.bottom-rcFull.top) - (rcText.bottom-rcText.top)) );
        mVerticalScrollBar->setVisiblePercentage( (cgFloat)(rcText.bottom-rcText.top) / (cgFloat)(rcFull.bottom-rcFull.top) );
        mVerticalScrollBar->setStepSizes( (cgFloat)mTextMetrics.getLineHeight(), (cgFloat)(rcText.bottom-rcText.top) );
    
    } // End if overflowing
    else
    {
        // Hide the scroll bar (no need to recompute metrics)
        if ( mVerticalScrollBar->isVisible() )
            mVerticalScrollBar->setVisible( false );
        
        // Scroll bar now has no range
        mVerticalScrollBar->setValueRange( 0, 0 );
        mVerticalScrollBar->setVisiblePercentage( 1.0f );
        mVerticalScrollBar->setStepSizes( 0, 0 );

    } // End if not overflowing

    // We're going to need to recompute our references too
    /*mTextMetrics.characterToLocation( mCaretCharacter, mMetricCaretChar );
    mTextMetrics.characterToLocation( mSelectionStart, mMetricSelStart );
    mTextMetrics.characterToLocation( mSelectionEnd, mMetricSelEnd );
    mTextMetrics.characterToLocation( mSelectionRef, mMetricSelRef );*/

    // Metrics are no longer dirty
    mMetricsDirty = false;
}

//-----------------------------------------------------------------------------
//  Name : onSelectedIndexChange() (Virtual)
/// <summary>
/// Triggered whenever a new list box item / row is selected in some way.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::onSelectedIndexChange( cgInt32 oldIndex, cgInt32 newIndex )
{
    // Raise the event with registered targets / script
    raiseEvent( cgSystemMessages::UI_ListBox_OnSelectedIndexChange, &UI_ListBox_OnSelectedIndexChangeArgs(oldIndex, newIndex) );
}

//-----------------------------------------------------------------------------
//  Name : scrollToSelection()
/// <summary>
/// Call this method in order to ensure that the currently selected item is
/// visible.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::scrollToSelection( )
{
    // Scroll the control to ensure that the selected item is visible.
    if ( mSelectedIndex >= 0 )
    {
        // If metrics are dirty, they first need to be recomputed.
        if ( mMetricsDirty )
            computeTextMetrics();

        // Compute line rectangle for this line relative to the text
        cgRect rcText = mTextMetrics.getFullBounds();
        cgInt32 nLineTop = (mSelectedIndex * mTextMetrics.getLineHeight()) + rcText.top;
        cgInt32 nLineBottom = ((mSelectedIndex + 1) * mTextMetrics.getLineHeight()) + rcText.top;
            
        // Item off the top of the item area.
        if ( nLineTop < 0 )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount + nLineTop) );
            
        // Item off the bottom of the item area.
        if ( nLineBottom > getItemArea( cgControlCoordinateSpace::ClientRelative ).bottom )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount + (nLineBottom - getItemArea( cgControlCoordinateSpace::ClientRelative ).bottom) ) );

    } // End if item selected
}

//-----------------------------------------------------------------------------
//  Name : setSelectedIndex()
/// <summary>
/// Select a new item from the list by supplying a valid index in the range
/// 0 through <list size> - 1. Specifying a value of -1 will deselect the
/// currently selected item.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::setSelectedIndex( cgInt32 index )
{
    // Clamp the index to the last item in the list.
    index = min( (cgInt32)mItems.size() - 1, index );

    // Update the currently selected index.
    cgInt32 oldIndex = mSelectedIndex;
    mSelectedIndex = index;
    if ( mSelectedIndex < -1 )
        mSelectedIndex = -1;

    // Scroll the control to ensure that the selected item is visible.
    scrollToSelection();

    // Raise event.
    onSelectedIndexChange( oldIndex, mSelectedIndex );
}

//-----------------------------------------------------------------------------
//  Name : getSelectedIndex()
/// <summary>
/// Retrieve the index of the currently selected item, or -1 if nothing is
/// currently selected.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgListBoxControl::getSelectedIndex() const
{
    return mSelectedIndex;
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgListBoxControl::processMessage( cgMessage * message )
{
    // Retrieve the message source
    cgReference * source = cgReferenceManager::getReference( message->fromId );

    // Was this our vertical scroll bar message?
    if ( source == mVerticalScrollBar && message->messageId == cgSystemMessages::UI_ScrollBar_OnValueChange )
    {
        cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs * arguments = (cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs*)message->messageData;

        // Store vertical scroll value
        cgInt32 oldScrollAmount = mVerticalScrollAmount;
        mVerticalScrollAmount = (cgInt32)arguments->value;

        // Recompute the text metrics to take into account this new offset if it is different
        if ( oldScrollAmount != mVerticalScrollAmount )
            computeTextMetrics();

        // Processed message
        return true;

    } // End if message from negative button

    // Message was not processed
    return cgUIControl::processMessage( message );
}

//-----------------------------------------------------------------------------
//  Name : setVisible () (Virtual)
/// <summary>
/// Hide / show this control and all its children.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::setVisible( bool bVisible )
{
    // Call base class implementation.
    cgUIControl::setVisible( bVisible );

    // If we are now visible and text metrics are dirty, recompute them.
    if ( isVisible() && mMetricsDirty )
        computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setParentVisible () (Virtual)
/// <summary>
/// Parent visibility and child visibility are treated differently.
/// Because the visibility of the parent should not affect the original
/// visibility of the child, this is tracked separately.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::setParentVisible( bool bVisible )
{
    // Call base class implementation.
    cgUIControl::setParentVisible( bVisible );

    // If we are now visible and text metrics are dirty, recompute them.
    if ( isVisible() && mMetricsDirty )
        computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setFont () (Virtual)
/// <summary>
/// Set the font to use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::setFont( const cgString & strFont )
{
    // Call base class implementation.
    cgUIControl::setFont( strFont );

    // Recompute the metrics with the new font selection
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setTextColor () (Virtual)
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgListBoxControl::setTextColor( const cgColorValue & Color )
{
    // Call base class implementation
    cgUIControl::setTextColor( Color );
    
    // Recompute text metrics (contains per character colors)
    computeTextMetrics();
}