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
// Name : cgTextBoxControl.cpp                                               //
//                                                                           //
// Desc : Built in user interface text box control.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTextBoxControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgTextBoxControl.h>
#include <Interface/Controls/cgScrollBarControl.h>
#include <Interface/cgUIManager.h>
#include <Interface/cgUISkin.h>
#include <Input/cgInputDriver.h>
#include <System/cgMessageTypes.h>
#include <System/cgStringUtility.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // Warning: Portability
#include <Mmsystem.h>   // timeGetTime() -- Warning: Portability
#undef WIN32_LEAN_AND_MEAN

// ToDo: Remove these comments once completed.
// ToDo: Single line text box needs to scroll horizontally.
// ToDo: Textbox needs undo support. Probably should use a timed undo buffer where we only update it if we haven't typed for a large amount of time? Or possibly just when any other key other than a valid character insert (i.e. move etc.)
// ToDo: Potentially add CTRL+Cursor/Delete/Backspace to operate on words instead of characters

///////////////////////////////////////////////////////////////////////////////
// cgTextBoxControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTextBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextBoxControl::cgTextBoxControl( ) : cgUIControl( Complex, _T("ControlFrame") )
{
    // Initialize variables to sensible defaults
    mMultiline              = false;
    mShowCaret              = true;
    mReadOnly               = false;
    mAllowFormatCode        = false;
    mSelectionStart         = 0;
    mSelectionEnd           = 0;
    mSelectionRef           = 0;
    mCaretCharacter         = 0;
    mVerticalScrollAmount   = 0;
    mMetricsDirty           = false;
    
    // Clear structures
    memset( &mMetricSelStart, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricSelEnd, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricSelRef, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricCaretChar, 0, sizeof(cgTextMetrics::MetricRef));

    // Create the vertical scroll bar that will be used to scroll multi-line text boxes
    mVerticalScrollBar = new cgScrollBarControl( );
    addChildControl( mVerticalScrollBar );

    // We need to capture value change messages from the scroll bar
    mVerticalScrollBar->registerEventHandler( cgSystemMessages::UI_ScrollBar_OnValueChange, getReferenceId() );

    // Set default padding
    setPadding( 2, 2, 2, 2 );
}

//-----------------------------------------------------------------------------
//  Name : cgTextBoxControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextBoxControl::cgTextBoxControl( const cgString & strElementName ) : cgUIControl( Simple, strElementName )
{
    // Initialize variables to sensible defaults
    mMultiline              = false;
    mShowCaret              = true;
    mReadOnly               = false;
    mAllowFormatCode        = false;
    mSelectionStart         = 0;
    mSelectionEnd           = 0;
    mSelectionRef           = 0;
    mCaretCharacter         = 0;
    mVerticalScrollAmount   = 0;
    mMetricsDirty           = false;
    
    // Clear structures
    memset( &mMetricSelStart, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricSelEnd, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricSelRef, 0, sizeof(cgTextMetrics::MetricRef));
    memset( &mMetricCaretChar, 0, sizeof(cgTextMetrics::MetricRef));

    // Create the vertical scroll bar that will be used to scroll multi-line text boxes
    mVerticalScrollBar = new cgScrollBarControl( );
    addChildControl( mVerticalScrollBar );

    // We need to capture value change messages from the scroll bar
    mVerticalScrollBar->registerEventHandler( cgSystemMessages::UI_ScrollBar_OnValueChange, getReferenceId() );

    // Set default padding
    setPadding( 2, 2, 2, 2 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgTextBoxControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTextBoxControl::~cgTextBoxControl()
{
    // Release allocated memory
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UITextBoxControl )
        return true;

    // Supported by base?
    return cgUIControl::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : setAllowFormatCode ()
/// <summary>
/// Enable / disable text format code support for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setAllowFormatCode( bool bAllow )
{
    // Is this a no-op?
    if ( mAllowFormatCode == bAllow )
        return;

    // Update local member
    mAllowFormatCode = bAllow;

    // Recompute text metrics in case it contained formatting tags.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : getAllowFormatCode ()
/// <summary>
/// Determine if text format code is supported for this control.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::getAllowFormatCode( ) const
{
    return mAllowFormatCode;
}

//-----------------------------------------------------------------------------
//  Name : setTextColor () (Virtual)
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setTextColor( const cgColorValue & Color )
{
    // Call base class implementation
    cgUIControl::setTextColor( Color );
    
    // Recompute text metrics (contains per character colors)
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::renderSecondary( )
{
    cgUInt32            nFlags = cgTextFlags::ClipRectangle;
    cgRect              rcDraw, rcText;
    std::vector<cgRect> Rectangles;
    cgUInt32            i;

    // Bail if control is not visible
    if ( !isVisible() )
        return;

    // Get access to interface manager
    cgUISkin        * pSkin    = mUIManager->getCurrentSkin();
    cgRenderDriver  * pDriver  = mUIManager->getRenderDriver();
    cgTextEngine    * pEngine  = mUIManager->getTextEngine();

    // Build text rendering flags.
    if ( mMultiline )
        nFlags |= cgTextFlags::Multiline;
    else
        nFlags |= cgTextFlags::VAlignCenter;
    
    // Draw the text from our pre-cached metrics
    rcText = getTextArea( cgControlCoordinateSpace::ScreenRelative );
    pEngine->setKerning( 0 );
    pEngine->setLineSpacing( 0 );
    mUIManager->selectFont( getFont() );
    pEngine->printText( mTextMetrics, cgPoint( rcText.left, rcText.top ) );

    // Get skin configuration for control rendering
    const cgUISkin::ControlConfig & Config = pSkin->getControlConfig();

    // Clip caret and selection rect to client area too
    pDriver->pushScissorRect( &rcText );

    // Render caret?
    if ( mShowCaret && isEnabled() && mCaretCharacter >= 0 && mUIManager->getFocus() == this )
    {
        // Blink on and off as required
        if ( Config.textBox.caretBlinkSpeed == 0 || 
            timeGetTime() % (Config.textBox.caretBlinkSpeed * 2) < Config.textBox.caretBlinkSpeed )
        {
            // Compute the rectangle for the caret position only
            Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
            
            // Render the caret as a rectangle
            if ( Rectangles.size() > 0 )
            {
                // Compute a 1 pixel wide rectangle
                rcDraw         = Rectangles[0];
                rcDraw.left   += rcText.left;
                rcDraw.right  += rcText.left;
                rcDraw.top    += rcText.top;
                rcDraw.bottom += rcText.top;
                rcDraw.right   = rcDraw.left + 1;

                // Draw the rectangle in the chosen color
                pDriver->drawRectangle( rcDraw, Config.textBox.caretColor, true );
            
            } // End if data returned
        
        } // End if within visible time

    } // End if caret should be visible

    // Draw selection rectangle?
    if ( hasSelection() && isEnabled() )
    {
        cgColorValue Color = Config.textBox.selectionColor;

        // If we don't currently have focus, use a light gray selection color
        if ( mUIManager->getFocus() != this )
            Color = cgColorValue( 0.7f, 0.7f, 0.7f, 0.3f );

        // Compute the rectangles for the selected range
        Rectangles = mTextMetrics.computeTextRectangles( mMetricSelStart, mMetricSelEnd );

        // Render for each rectangle returned
        for ( i = 0; i < Rectangles.size(); ++i )
        {
            // Offset rectangle to client area
            rcDraw         = Rectangles[i];
            rcDraw.left   += rcText.left;
            rcDraw.right  += rcText.left;
            rcDraw.top    += rcText.top;
            rcDraw.bottom += rcText.top;

            // Ensure rectangle is at least 3 pixels wide (for empty lines)
            if ( (rcDraw.right - rcDraw.left) < 3 )
                rcDraw.right = rcDraw.left + 3;

            // Render the selection rectangle
            pDriver->drawRectangle( rcDraw, Color, true );
        
        } // Next Rectangle
    
    } // End if draw selection rectangle

    // Reset clipping rect
    pDriver->popScissorRect( );

    // Call base class implementation
    cgUIControl::renderSecondary();
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the layout of the control has been recomputed.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Call base class implementation
    cgUIControl::onSize( nWidth, nHeight );

    // Reposition scroll bar (ensure it fills entire client area irrespective of the padding)
    /*cgRect controlArea = getClientArea();
    mVerticalScrollBar->setPosition( ((controlArea.right - controlArea.left) - 15) + mPadding.right, -mPadding.top );
    mVerticalScrollBar->setSize( 15, (controlArea.bottom - controlArea.top) + mPadding.top + mPadding.bottom );
    mVerticalScrollBar->setVisible( false );*/

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
bool cgTextBoxControl::onMouseButtonDown( cgInt32 nButtons, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseButtonDown( nButtons, Position ) )
        return true;

    // Only process if no other control is captured
    cgInputDriver * pInput   = cgInputDriver::getInstance();
    if ( mUIManager->getCapture() )
        return false;
    
    // Is this within our control's rectangle?
    if ( getTextArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( Position ) )
    {
        // Already captured?
        if ( !mUIManager->getCapture() )
            mUIManager->setCapture( this );

        // We have focus now
        mUIManager->setFocus( this );

        // Control is enabled ?
        if ( isEnabled() )
        {
            // Compute the caret data and selection start from the click position
            resetCaret( Position, (pInput->isKeyPressed( cgKeys::LShift ) || pInput->isKeyPressed( cgKeys::RShift ) ) );
        
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
bool cgTextBoxControl::onMouseButtonUp( cgInt32 nButtons, const cgPoint & Position )
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
//  Name : onMouseWheelScroll () (Virtual)
/// <summary>
/// This method is called whenever the mouse wheel is scrolled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::onMouseWheelScroll( cgInt32 nDelta, const cgPoint & Position )
{
    // Ignore if control is not visible
    if ( !isVisible() )
        return false;

    // Call base class implementation
    if ( cgUIControl::onMouseWheelScroll( nDelta, Position ) )
        return true;

    // If we're over our text area, scroll it.
    if ( getTextArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( Position ) )
    {
        if ( mMultiline )
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
//  Name : onMouseMove () (Virtual)
/// <summary>
/// Triggered whenever the mouse moves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::onMouseMove( const cgPoint & Position, const cgPointF & Offset )
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
        mUIManager->selectCursor( _T("IBeam") );

        // Control is enabled ?
        if ( isEnabled() )
        {
            // Compute the caret data and selection rectangle from the cursor position
            resetCaret( Position, true );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if control is enabled

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnMouseMove, &UI_OnMouseMoveArgs( Position, Offset ) );
        return true;

    } // End if we're the captured item
    else
    {
        // Set the cursor if applicable
        if ( getTextArea( cgControlCoordinateSpace::ScreenRelative ).containsPoint( Position ) )
            mUIManager->selectCursor( _T("IBeam") );
    
    } // End if not the captured item
    
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
bool cgTextBoxControl::onKeyPressed( cgInt32 nKeyCode, cgUInt32 nModifiers )
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

    // Get useful modifier values
    bool bShift, bControl;
    bShift   = (nModifiers & cgModifierKeys::Shift) != 0;
    bControl = (nModifiers & cgModifierKeys::Control) != 0;

    // Do we currently have focus?
    if ( mUIManager->getFocus() == this )
    {
        // Convert the key code to a character
        cgString strChar = cgInputDriver::getInstance()->keyCodeToCharacter( nKeyCode );

        // Writable?
        if ( !mReadOnly )
        {
            // Special cases?
            if ( nKeyCode == cgKeys::Backspace )
            {
                // Clear selection if naything selected, otherwise remove the character on the left
                if ( hasSelection() )
                    clearSelection();
                else
                    removeCharacter( mCaretCharacter - 1 );

                // Ensure caret is visible
                scrollToCaret();
                
            } // End if backspace
            else if ( nKeyCode == cgKeys::Delete )
            {
                // Clear selection if naything selected, otherwise remove the character on the right
                if ( hasSelection() )
                    clearSelection();
                else
                    removeCharacter( mCaretCharacter );

                // Ensure caret is visible
                scrollToCaret();
                
            } // End if delete
            else if ( nKeyCode == cgKeys::X && bControl )
            {
                // Copy selected text
                cgStringUtility::setClipboardText( getSelectedText() );

                // Clear selection
                clearSelection();
                
                // Ensure caret is visible
                scrollToCaret();

            } // End if CTRL+X
            else if ( nKeyCode == cgKeys::V && bControl )
            {
                // Just clear the selection first, and then insert the text
                clearSelection();

                // Ensure caret is visible before we begin inserting
                scrollToCaret();

                // Insert the character into the text
                insertString( mCaretCharacter, cgStringUtility::getClipboardText() );

                // Ensure caret is visible after we've inserted
                scrollToCaret();

            } // End if CTRL+V
            else if ( !mMultiline && nKeyCode == cgKeys::Return )
            {
                // Enter is accepted in single-line controls.
                return false;

            } // End if Return
            else if ( !strChar.empty() )
            {
                // Just clear the selection first, and then insert the text
                clearSelection();

                // Ensure caret is visible before we begin inserting
                scrollToCaret();

                // Insert the character into the text
                insertString( mCaretCharacter, strChar );

                // Ensure caret is visible after we've inserted
                scrollToCaret();

            } // End if any other character

        } // End if !read only
        else
        {
            if ( nKeyCode == cgKeys::X && bControl )
            {
                // Cut acts like Copy in read only mode.
                cgStringUtility::setClipboardText( getSelectedText() );

            } // End if CTRL+X

        } // End if read only

        // Read only commands
        if ( nKeyCode == cgKeys::Left )
        {
            // Move back one char
            resetCaret( mCaretCharacter - 1, bShift );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if left
        else if ( nKeyCode == cgKeys::Right )
        {
            // Move forward one char
            resetCaret( mCaretCharacter + 1, bShift );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if right
        else if ( nKeyCode == cgKeys::Up && mMultiline )
        {
            // Ensure caret is visible before we move
            scrollToCaret();

            // Move up one line by resolving the caret's
            // current position, and moving up by a single
            // line's height.
            std::vector<cgRect> Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
            if ( Rectangles.size() > 0 )
            {
                cgRect  rcTextArea = getTextArea( cgControlCoordinateSpace::ScreenRelative );
                cgPoint Position   = cgPoint( Rectangles[0].left + rcTextArea.left, Rectangles[0].top + rcTextArea.top );
                Position.y        -= (mTextMetrics.getLineHeight() - 1);
                resetCaret( Position, bShift );
                
                // Ensure final caret position is visible
                scrollToCaret();

            } // End if caret rect available
        
        } // End if up
        else if ( nKeyCode == cgKeys::Down && mMultiline )
        {
            // Ensure caret is visible before we move
            scrollToCaret();

            // Move down one line by resolving the caret's
            // current position, and moving down by a single
            // line's height.
            std::vector<cgRect> Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
            if ( Rectangles.size() > 0 )
            {
                cgRect  rcTextArea = getTextArea( cgControlCoordinateSpace::ScreenRelative );
                cgPoint Position   = cgPoint( Rectangles[0].right + rcTextArea.left, Rectangles[0].bottom + rcTextArea.top );
                Position.y        += (mTextMetrics.getLineHeight() - 1);
                resetCaret( Position, bShift );
                
                // Ensure final caret position is visible
                scrollToCaret();

            } // End if caret rect available
        
        } // End if down
        else if ( nKeyCode == cgKeys::PageDown && mMultiline )
        {
            // Ensure caret is visible before we move
            scrollToCaret();

            // Cache the caret's current location
            std::vector<cgRect> Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
            cgRect  rcTextArea = getTextArea( cgControlCoordinateSpace::ScreenRelative );
            cgPoint Position   = cgPoint( Rectangles[0].right + rcTextArea.left, Rectangles[0].bottom + rcTextArea.top );
            
            // Scroll down by a page
            mVerticalScrollBar->stepLarge( 1 );

            // Reset caret to same position
            resetCaret( Position, bShift ); 

            // Ensure caret is fully on the screen
            scrollToCaret();
            
        } // End if page down
        else if ( nKeyCode == cgKeys::PageUp && mMultiline )
        {
            // Ensure caret is visible before we move
            scrollToCaret();

            // Cache the caret's current location
            std::vector<cgRect> Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
            cgRect  rcTextArea = getTextArea( cgControlCoordinateSpace::ScreenRelative );
            cgPoint Position   = cgPoint( Rectangles[0].right + rcTextArea.left, Rectangles[0].bottom + rcTextArea.top );
            
            // Scroll up by a page
            mVerticalScrollBar->stepLarge( -1 );

            // Reset caret to same position
            resetCaret( Position, bShift ); 

            // Ensure caret is fully on the screen
            scrollToCaret();
            
        } // End if page up
        else if ( nKeyCode == cgKeys::Home && bControl )
        {
            // Before we do anything, scroll to the beginning for rapid seeking to start
            scrollToStart();

            // Move caret to beginning of text
            resetCaret( 0, bShift );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if CTRL+Home
        else if ( nKeyCode == cgKeys::Home )
        {
            // Generate caret metric for start of line
            cgTextMetrics::MetricRef RefLine = mMetricCaretChar;
            RefLine.column = -1; // Prior code

            // Reset caret to this position
            resetCaret( mTextMetrics.characterFromLocation( RefLine ), bShift );
            
            // Ensure caret is visible
            scrollToCaret();
        
        } // End if Home
        else if ( nKeyCode == cgKeys::End && bControl )
        {
            // Before we do anything, scroll to the end for rapid seeking
            scrollToEnd();

            // Move caret to end of text
            resetCaret( (cgInt32)mControlText.length(), bShift );

            // Ensure caret is visible
            scrollToCaret();
        
        } // End if CTRL+Home
        else if ( nKeyCode == cgKeys::End )
        {
            // Generate caret metric for end of line
            cgTextMetrics::MetricRef RefLine = mMetricCaretChar;
            RefLine.column = -2; // After code

            // Reset caret to this position
            resetCaret( mTextMetrics.characterFromLocation( RefLine ), bShift );
            
            // Ensure caret is visible
            scrollToCaret();
        
        } // End if Home
        else if ( nKeyCode == cgKeys::A && bControl )
        {
            // Before we do anything else, scroll right to the end of the text
            scrollToEnd();

            // Move caret to end of text
            resetCaret( (cgInt32)mControlText.length() );

            // Select all text
            setSelectionRange( 0, (cgInt32)mControlText.length() );
            
            // Ensure caret is visible
            scrollToCaret();

        } // End if CTRL+A
        else if ( nKeyCode == cgKeys::C && bControl )
        {
            // Copy selected text
            cgStringUtility::setClipboardText( getSelectedText() );

        } // End if CTRL+C

        // Raise the event, we processed this
        raiseEvent( cgSystemMessages::UI_OnKeyPressed, &UI_OnKeyPressedArgs( nKeyCode, nModifiers ) );
        
        // We processed this message
        return true;

    } // End if we currently have focus

    // We did not process this message
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setSelectionRange ()
/// <summary>
/// Set the range of selected characters.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setSelectionRange( cgInt32 nSelStart, cgInt32 nSelEnd )
{
    // Ensure SelEnd is never less than SelStart
    if ( nSelEnd < nSelStart ) nSelEnd = nSelStart;

    // Compute selection metrics based on specified character
    mSelectionStart = max( 0, min( nSelStart, (cgInt32)mControlText.length() ) );
    mTextMetrics.characterToLocation( mSelectionStart, mMetricSelStart );
    mSelectionEnd   = max( 0, min( nSelEnd, (cgInt32)mControlText.length() ) );
    mTextMetrics.characterToLocation( mSelectionEnd, mMetricSelEnd );

    // Selection reference position should equal the first character
    // in the selection
    mSelectionRef      = mSelectionStart;
    mMetricSelRef = mMetricSelStart;
}

//-----------------------------------------------------------------------------
//  Name : resetCaret () (Protected)
/// <summary>
/// Position the caret based on the specified screen space point.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::resetCaret( const cgPoint & Position, bool bAdjustSelection /* = false */ )
{
    // Update caret
    mCaretCharacter = characterFromPoint( Position, &mMetricCaretChar );

    // Reset selection start and end
    mSelectionStart      = mCaretCharacter;
    mSelectionEnd        = mCaretCharacter;
    mMetricSelStart = mMetricCaretChar;
    mMetricSelEnd   = mMetricCaretChar;

    // Selection handling
    if ( !bAdjustSelection )
    {
        // Not adjusting selection, just reset reference to caret
        mSelectionRef        = mCaretCharacter;
        mMetricSelRef   = mMetricCaretChar;
    
    } // End if NOT adjusting selection
    else
    {
        // Adjust the selection start / end depend on where caret
        // is relative to the selection reference
        if ( mCaretCharacter < mSelectionRef )
        {
            mSelectionEnd      = mSelectionRef;
            mMetricSelEnd = mMetricSelRef;
        
        } // End if end at reference
        else
        {
            mSelectionStart      = mSelectionRef;
            mMetricSelStart = mMetricSelRef;

        } // End if start reference
        
    } // End if adjusting selection
}

//-----------------------------------------------------------------------------
//  Name : resetCaret () (Protected)
/// <summary>
/// Position the caret based on the specified character index.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::resetCaret( cgInt32 nCharacter, bool bAdjustSelection /* = false */ )
{
    // Compute caret metrics based on specified character
    mCaretCharacter = max( 0, min( nCharacter, (cgInt32)mControlText.length() ) );
    mTextMetrics.characterToLocation( mCaretCharacter, mMetricCaretChar );

    // Reset selection start and end
    mSelectionStart      = mCaretCharacter;
    mSelectionEnd        = mCaretCharacter;
    mMetricSelStart = mMetricCaretChar;
    mMetricSelEnd   = mMetricCaretChar;

    // Selection handling
    if ( !bAdjustSelection )
    {
        // Not adjusting selection, just reset reference to caret
        mSelectionRef        = mCaretCharacter;
        mMetricSelRef   = mMetricCaretChar;
    
    } // End if NOT adjusting selection
    else
    {
        // Adjust the selection start / end depend on where caret
        // is relative to the selection reference
        if ( mCaretCharacter < mSelectionRef )
        {
            mSelectionEnd      = mSelectionRef;
            mMetricSelEnd = mMetricSelRef;
        
        } // End if end at reference
        else
        {
            mSelectionStart      = mSelectionRef;
            mMetricSelStart = mMetricSelRef;

        } // End if start reference
        
    } // End if adjusting selection
}

//-----------------------------------------------------------------------------
//  Name : scrollToCaret ()
/// <summary>
/// Scroll if necessary in order to ensure the caret is visible.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::scrollToCaret()
{
    cgRect              rcText;
    std::vector<cgRect> Rectangles;
    cgInt32             nCaretLine;

    // Caret not visible.
    if ( mCaretCharacter < 0 )
        return;

    // Vertical scroll first if multi-line
    if ( mMultiline )
    {
        // Get the line index for the caret position (works even if the line was not visible)
        nCaretLine = mTextMetrics.characterToLine( mCaretCharacter );
        
        // Compute caret rectangle for this line relative to the text
        rcText               = mTextMetrics.getFullBounds();
        cgInt32 nCaretTop    = (nCaretLine * mTextMetrics.getLineHeight()) + rcText.top;
        cgInt32 nCaretBottom = ((nCaretLine + 1) * mTextMetrics.getLineHeight()) + rcText.top;
        
        // Caret off the top of the text area.
        if ( nCaretTop < 0 )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount + nCaretTop ) );
        
        // Caret off the bottom of the text area.
        if ( nCaretBottom > getTextArea( cgControlCoordinateSpace::ClientRelative ).bottom )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount + (nCaretBottom - getTextArea( cgControlCoordinateSpace::ClientRelative ).bottom) ) );
        
        /*// Scroll up until the line of text it's on is visible
        for ( ; mMetricCaretChar.nLine == -1; )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount - mTextMetrics.getLineHeight()) );
            

        // Scroll down until the line of text it's on is visible
        for ( ; mMetricCaretChar.nLine == -2; )
            mVerticalScrollBar->setValue( (cgFloat)(mVerticalScrollAmount + mTextMetrics.getLineHeight()) );

        // Now ensure the caret itself is visible
        std::vector<cgRect> Rectangles = mTextMetrics.computeTextRectangles( mMetricCaretChar, mMetricCaretChar );
        if ( Rectangles.size() == 0 ) return;
        rcCaret = Rectangles[0];*/
    
    } // End if multi-line textbox
}

//-----------------------------------------------------------------------------
//  Name : scrollToStart ()
/// <summary>
/// Scroll the text to the very beginning.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::scrollToStart()
{
    // Scroll vertically first if multi-line
    if ( mMultiline )
        mVerticalScrollBar->setValue( mVerticalScrollBar->getMinimumValue() );
}

//-----------------------------------------------------------------------------
//  Name : scrollToEnd ()
/// <summary>
/// Scroll the text to the very end.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::scrollToEnd()
{
    // Scroll vertically first if multi-line
    if ( mMultiline )
        mVerticalScrollBar->setValue( mVerticalScrollBar->getMaximumValue() );
}

//-----------------------------------------------------------------------------
//  Name : clearSelection () (Protected)
/// <summary>
/// If any text is selected, clear it out.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::clearSelection( )
{
    // Ensure we have a selection
    if ( !hasSelection() )
        return;

    // Clear the selection.
    setControlText( mControlText.substr( 0, mSelectionStart ) + mControlText.substr( mSelectionEnd ) );

    // Update caret position if it's affected by the clear
    if ( mCaretCharacter > mSelectionStart && mCaretCharacter <= mSelectionEnd )
        mCaretCharacter = mSelectionStart;
    else if ( mCaretCharacter > mSelectionEnd )
        mCaretCharacter -= (mSelectionEnd - mSelectionStart);

    // Reset caret to selected position
    resetCaret( mCaretCharacter );
}

//-----------------------------------------------------------------------------
//  Name : hasSelection () (Protected)
/// <summary>
/// Is any text selected?
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::hasSelection( ) const
{
    return ( mSelectionStart >= 0 && mSelectionEnd > mSelectionStart );
}

//-----------------------------------------------------------------------------
//  Name : getSelectedText ()
/// <summary>
/// Simply retrieve any text that is selected.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgTextBoxControl::getSelectedText( ) const
{
    if ( !hasSelection() )
        return _T("");

    // Return the correct subset of the text.
    return mControlText.substr( mSelectionStart, mSelectionEnd - mSelectionStart );
}

//-----------------------------------------------------------------------------
//  Name : removeCharacter () (Protected)
/// <summary>
/// Simply remove a single character at the specified position.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::removeCharacter( cgInt32 nCharacter )
{
    // Clear the selection.
    setControlText( mControlText.substr( 0, nCharacter ) + mControlText.substr( nCharacter + 1 ) );

    // Update caret position if it's affected by the clear
    if ( mCaretCharacter > nCharacter )
        mCaretCharacter--;

    // Reset caret to selected position
    resetCaret( mCaretCharacter );
}

//-----------------------------------------------------------------------------
//  Name : insertString ()
/// <summary>
/// Simply insert a string at the specified position.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::insertString( cgInt32 nPosition, const cgString & strInsert )
{
    // Position < 0 means end.
    if ( nPosition < 0 )
        nPosition = mControlText.length();

    // Ensure position is within range
    nPosition = min( nPosition, (cgInt32)mControlText.length() );
    
    // Insert into string
    mControlText.insert( nPosition, strInsert );

    // Update caret position if it's affected by the insert
    if ( mCaretCharacter >= nPosition )
        mCaretCharacter += (cgInt32)strInsert.length();

    // Reset caret here
    resetCaret( mCaretCharacter );

    // Trigger the events associated with changing text
    setControlText( mControlText );
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual, Override)
/// <summary>
/// Update the common text string for the control (i.e. Caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setControlText( const cgString &strText )
{
    // Pass through to base class first
    cgUIControl::setControlText( strText );
    
    // Recalculate text metrics if visible, otherwise mark as dirty
    // for update when we next become visible.
    if ( isVisible( true ) )
        computeTextMetrics();
    else
        mMetricsDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setFont () (Virtual)
/// <summary>
/// Set the font to use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setFont( const cgString & strFont )
{
    // Call base class implementation.
    cgUIControl::setFont( strFont );

    // Recompute the metrics with the new font selection
    computeTextMetrics();
}


//-----------------------------------------------------------------------------
//  Name : setMultiline () (Virtual)
/// <summary>
/// Enable / disable multi-line support for this text box control.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setMultiline( bool bMultiline )
{
    // Skip if no-op.
    if ( mMultiline == bMultiline )
        return;

    // Update flag
    mMultiline = bMultiline;

    // Recompute the metrics based on new selection.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setReadOnly () (Virtual)
/// <summary>
/// Enable / disable read-only state of this text box control.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setReadOnly( bool bReadOnly )
{
    mReadOnly = bReadOnly;
}

//-----------------------------------------------------------------------------
//  Name : getReadOnly () (Virtual)
/// <summary>
/// Retrieve the read-only state of this text box control.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::getReadOnly( ) const
{
    return mReadOnly;
}

//-----------------------------------------------------------------------------
//  Name : computeTextMetrics () (Protected)
/// <summary>
/// Recompute the text metrics (line/character sizes etc.) for use during
/// text processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::computeTextMetrics()
{
    cgUInt32 nFlags = cgTextFlags::ClipRectangle;
    cgPoint  ptOffset;
    cgRect   rcText;

    // Retrieve the text engine from the manager, we need low level access
    cgTextEngine * pEngine = mUIManager->getTextEngine();

    // First build metric computation flags.
    if ( mMultiline )
        nFlags |= cgTextFlags::Multiline;
    else
        nFlags |= cgTextFlags::VAlignCenter;
    if ( mAllowFormatCode )
        nFlags |= cgTextFlags::AllowFormatCode;

    // Compute metrics for this text relative to the client area first of all
    rcText     = getClientArea( cgControlCoordinateSpace::ClientRelative );
    ptOffset.x = 0;
    ptOffset.y = -mVerticalScrollAmount;
    mUIManager->selectFont( getFont() );
    pEngine->setKerning( 0 );
    pEngine->setLineSpacing( 0 );
    pEngine->setColor( mControlTextColor );
    pEngine->computeTextMetrics( rcText, nFlags, mControlText, ptOffset, mTextMetrics );

    // Did the text overflow the specified rectangle when multiline rendering?
    if ( mMultiline )
    {
        // Did the text overflow the original rectangle?
        if ( mTextMetrics.isOverflowing() )
        {
            // Show the scroll bar
            if ( !mVerticalScrollBar->isVisible() )
                mVerticalScrollBar->setVisible( true );
                
            // We must recompute the text metrics again for area minus scroll bar size
            rcText = getTextArea( cgControlCoordinateSpace::ClientRelative );
            pEngine->computeTextMetrics( rcText, nFlags, mControlText, ptOffset, mTextMetrics );

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

    } // End if Multiline

    // We're going to need to recompute our references too
    mTextMetrics.characterToLocation( mCaretCharacter, mMetricCaretChar );
    mTextMetrics.characterToLocation( mSelectionStart, mMetricSelStart );
    mTextMetrics.characterToLocation( mSelectionEnd, mMetricSelEnd );
    mTextMetrics.characterToLocation( mSelectionRef, mMetricSelRef );

    // Metrics are no longer dirty
    mMetricsDirty = false;
}

//-----------------------------------------------------------------------------
//  Name : characterFromPoint () (Protected)
/// <summary>
/// Given a position that falls within the control, compute the character
/// over which this point is located.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTextBoxControl::characterFromPoint( const cgPoint & Position, cgTextMetrics::MetricRef * pMetricRef /* = CG_NULL */ )
{
    cgRect  rcText;
    cgPoint ptTest;

    // Metrics are computed relative to client area
    rcText   = getTextArea( cgControlCoordinateSpace::ScreenRelative );
    ptTest.x = Position.x - rcText.left;
    ptTest.y = Position.y - rcText.top;

    // Allow metric class to process
    return mTextMetrics.characterFromPoint( ptTest, pMetricRef );
}

//-----------------------------------------------------------------------------
//  Name : getTextArea () (Protected)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the scroll bars.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextBoxControl::getTextArea( cgControlCoordinateSpace::Base Origin ) const
{
    cgRect rcText = getClientArea( Origin );

    // Adjust the client area, removing the size of the scrollbar if visible
    if ( mVerticalScrollBar->isVisible( false ) )
        rcText.right -= mVerticalScrollBar->getSize().width;

    // Return the final rectangle
    return rcText;
}

//-----------------------------------------------------------------------------
//  Name : getTextArea () (Protected)
/// <summary>
/// Calculate the rectangle that describes the interior of this control 
/// minus the scroll bars.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgTextBoxControl::getTextArea( ) const
{
    // Return the rectangle
    return getTextArea( cgControlCoordinateSpace::ControlRelative );
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgTextBoxControl::processMessage( cgMessage * pMessage )
{
    // Retrieve the reference target
    cgReference * pTarget = cgReferenceManager::getReference( pMessage->fromId );

    // Was this our vertical scroll bar message?
    if ( pTarget == mVerticalScrollBar && pMessage->messageId == cgSystemMessages::UI_ScrollBar_OnValueChange )
    {
        cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs * pArgs = (cgScrollBarControl::UI_ScrollBar_OnValueChangeArgs*)pMessage->messageData;

        // Store vertical scroll value
        cgInt32 nOldVScrollAmount = mVerticalScrollAmount;
        mVerticalScrollAmount = (cgInt32)pArgs->value;

        // Recompute the text metrics to take into account this new offset if it is different
        if ( nOldVScrollAmount != mVerticalScrollAmount )
            computeTextMetrics();
        
        // Processed message
        return true;
    
    } // End if message from negative button

    // Message was not processed
    return cgUIControl::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : setVisible () (Virtual)
/// <summary>
/// Hide / show this control and all its children.
/// </summary>
//-----------------------------------------------------------------------------
void cgTextBoxControl::setVisible( bool bVisible )
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
void cgTextBoxControl::setParentVisible( bool bVisible )
{
    // Call base class implementation.
    cgUIControl::setParentVisible( bVisible );

    // If we are now visible and text metrics are dirty, recompute them.
    if ( isVisible() && mMetricsDirty )
        computeTextMetrics();
}