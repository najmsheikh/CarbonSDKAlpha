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
// Name : cgLabelControl.cpp                                                 //
//                                                                           //
// Desc : Built in user interface label control.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLabelControl Module Includes
//-----------------------------------------------------------------------------
#include <Interface/Controls/cgLabelControl.h>
#include <Interface/cgUImanager.h>
#include <Interface/cgUIForm.h>

///////////////////////////////////////////////////////////////////////////////
// cgLabelControl Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLabelControl () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLabelControl::cgLabelControl( ) : cgUIControl( Simple, _T("") )
{
    // Initialize variables to sensible defaults
    mMultiline          = false;
    mTruncationMode     = cgTextTruncationMode::None;
    mAutoSize           = false;
    mHorizontalAlign    = cgHorizontalAlignment::Left;
    mVerticalAlign      = cgVerticalAlignment::Top;
    mTextColor          = cgColorValue( 1, 1, 1, 1 );
    mAllowFormatCode    = false;

    // Set default padding
    setPadding( 2, 2, 2, 2 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgLabelControl () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLabelControl::~cgLabelControl()
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
bool cgLabelControl::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_UILabelControl )
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
void cgLabelControl::setAllowFormatCode( bool bAllow )
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
bool cgLabelControl::getAllowFormatCode( ) const
{
    return mAllowFormatCode;
}

//-----------------------------------------------------------------------------
//  Name : setTextColor ()
/// <summary>
/// Set the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setTextColor( const cgColorValue & Color )
{
    mTextColor = Color;

    // Recompute metrics (contains colors per character)
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : getTextColor ()
/// <summary>
/// Retreive the default color of any text rendered for this control.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLabelControl::getTextColor( ) const
{
    return mTextColor;
}

//-----------------------------------------------------------------------------
//  Name : setMultiline () (Virtual)
/// <summary>
/// Enable / disable multi-line support for this label control.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setMultiline( bool bMultiline )
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
//  Name : setAutoSize () (Virtual)
/// <summary>
/// Enable / disable automatic sizing of the label control when new text
/// is supplied (single line only).
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setAutoSize( bool bAutoSize )
{
    // Skip if no-op.
    if ( mAutoSize == bAutoSize )
        return;

    // Update flag
    mAutoSize = bAutoSize;

    // Recompute the metrics based on new selection.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setTruncationMode () (Virtual)
/// <summary>
/// Select the type of truncation that should be applied if this is a 
/// single line control and the client area is too small.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setTruncationMode( cgTextTruncationMode::Base Mode )
{
    // Skip if no-op.
    if ( mTruncationMode == Mode )
        return;

    // Update flag
    mTruncationMode = Mode;

    // Recompute the metrics based on new selection.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setHorizontalAlignment () (Virtual)
/// <summary>
/// Select the method used to align the text horizontally.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setHorizontalAlignment( cgHorizontalAlignment::Base Align )
{
    // Skip if no-op.
    if ( mHorizontalAlign == Align )
        return;

    // Update flag
    mHorizontalAlign = Align;

    // Recompute the metrics based on new selection.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setVerticalAlignment () (Virtual)
/// <summary>
/// Select the method used to align the text vertically.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setVerticalAlignment( cgVerticalAlignment::Base Align )
{
    // Skip if no-op.
    if ( mVerticalAlign == Align )
        return;

    // Update flag
    mVerticalAlign = Align;

    // Recompute the metrics based on new selection.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : computeTextMetrics () (Protected)
/// <summary>
/// Recompute the text metrics (line/character sizes etc.) for use during
/// text processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::computeTextMetrics()
{
    cgUInt32 nFlags = cgTextFlags::ClipRectangle;

    // Cannot compute if not yet attached
    if ( !mRootForm )
        return;

    // Build text rendering flags.
    if ( mMultiline )
        nFlags |= cgTextFlags::Multiline;
    if ( mAllowFormatCode )
        nFlags |= cgTextFlags::AllowFormatCode;

    // ToDo: Support ellipsis
    /*if ( !autoSize && !Multiline && Ellipsis )
        nFlags |= cgTextFlags::Ellipsis;*/

        // Select requested alignment if we're not autosizing
    if ( !mAutoSize || mMultiline )
    {
        // Which horizontal alignment
        switch ( mHorizontalAlign )
        {
            case cgHorizontalAlignment::Center:
                nFlags |= cgTextFlags::AlignCenter;
                break;

            case cgHorizontalAlignment::Right:
                nFlags |= cgTextFlags::AlignRight;
                break;
        
        } // End Switch HAlign

        // Which vertical alignment
        switch ( mVerticalAlign )
        {
            case cgVerticalAlignment::Middle:
                nFlags |= cgTextFlags::VAlignCenter;
                break;

            case cgVerticalAlignment::Bottom:
                nFlags |= cgTextFlags::VAlignBottom;
                break;
        
        } // End Switch VAlign

    } // End if not autosizing

    // Select the correct font.
    cgUIManager  * pManager    = mRootForm->getUIManager();
    cgTextEngine * pTextEngine = pManager->getTextEngine();
    pTextEngine->setColor( mTextColor );
    pManager->selectFont( getFont() );

    // Compute metrics for this text.
    pTextEngine->computeTextMetrics( getClientArea( cgControlCoordinateSpace::ClientRelative ), 
                                     nFlags, mControlText, mTextMetrics );

    // If auto-size is enabled, re-size the control.
    if ( mAutoSize && !mMultiline )
    {
        const cgRect & rcText = mTextMetrics.getFullBounds();
        // ToDo: do not recompute metrics in this case (triggers 'onSize()').
        setSize( (rcText.right - rcText.left) + mPadding.left + mPadding.right, 
                 (rcText.bottom - rcText.top) + mPadding.top + mPadding.bottom );

    } // End if auto size
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the control is resized.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::onSize( cgInt32 nWidth, cgInt32 nHeight )
{
    // Call base class implementation
    cgUIControl::onSize( nWidth, nHeight );

    // Recompute text metrics based on new client area.
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : renderSecondary () (Virtual)
/// <summary>
/// Render any secondary elements for this control. This would include
/// items such as the text for any controls.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::renderSecondary( )
{
    // Bail if control is not visible
    if ( !isVisible() )
        return;

    // Get access to required systems
    cgUIManager  * pManager = mRootForm->getUIManager();
    cgTextEngine * pEngine  = pManager->getTextEngine();
    
    // Draw the label text based on computed metrics
    cgRect  rcText   = getClientArea( cgControlCoordinateSpace::ScreenRelative );
    cgPoint ptOffset = cgPoint( rcText.left, rcText.top );
    pEngine->printText( mTextMetrics, ptOffset );
    
    // Call base class implementation
    cgUIControl::renderSecondary();
}


//-----------------------------------------------------------------------------
//  Name : setFont () (Virtual)
/// <summary>
/// Set the font to use when rendering this control.
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setFont( const cgString & strFont )
{
    // Call base class implementation.
    cgUIControl::setFont( strFont );

    // Recompute the metrics with the new font selection
    computeTextMetrics();
}

//-----------------------------------------------------------------------------
//  Name : setControlText () (Virtual, Override)
/// <summary>
/// Update the common text string for the control (i.e. Caption for a
/// form, button text for a button etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgLabelControl::setControlText( const cgString &strText )
{
    // Bail if this is a no-op
    if ( strText == mControlText )
        return;

    // Pass through to base class first
    cgUIControl::setControlText( strText );

    // Recompute metrics based on new text.
    computeTextMetrics();
}