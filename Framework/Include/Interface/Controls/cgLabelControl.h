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
// Name : cgLabelControl.h                                                   //
//                                                                           //
// Desc : Built in user interface label control.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLABELCONTROL_H_ )
#define _CGE_CGLABELCONTROL_H_

//-----------------------------------------------------------------------------
// cgLabelControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>
#include <Interface/cgTextEngine.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {D269CD0F-E484-4E41-B55A-B2065A6305FA}
const cgUID RTID_UILabelControl     = {0xD269CD0F, 0xE484, 0x4E41, {0xB5, 0x5A, 0xB2, 0x6, 0x5A, 0x63, 0x5, 0xFA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgLabelControl(Class)
/// <summary>
/// A label / text container that can be attached to a form or as a child
/// of any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLabelControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgLabelControl, cgUIControl, "Label" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLabelControl( );
    virtual ~cgLabelControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void                    renderSecondary     ( );
    virtual void                    setControlText      ( const cgString & text );
    virtual void                    setFont             ( const cgString & fontName );
    virtual void                    setTextColor        ( const cgColorValue & color );
    virtual void                    onSize              ( cgInt32 width, cgInt32 height );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType    ( ) const { return RTID_UILabelControl; }
    virtual bool                    queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                            setMultiline            ( bool multiline );
    bool                            getMultiline            ( ) const { return mMultiline; }
    void                            setTruncationMode       ( cgTextTruncationMode::Base method );
    cgTextTruncationMode::Base      getTruncationMode       ( ) const { return mTruncationMode; }
    void                            setAutoSize             ( bool autoSize );
    bool                            getAutoSize             ( ) const { return mAutoSize; }
    void                            setHorizontalAlignment  ( cgHorizontalAlignment::Base align );
    cgHorizontalAlignment::Base     getHorizontalAlignment  ( ) const { return mHorizontalAlign; }
    void                            setVerticalAlignment    ( cgVerticalAlignment::Base align );
    cgVerticalAlignment::Base       getVerticalAlignment    ( ) const { return mVerticalAlign; }
    void                            setAllowFormatCode      ( bool allow );
    bool                            getAllowFormatCode      ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            computeTextMetrics      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgTextMetrics               mTextMetrics;       // Precomputed metrics for the currently assigned text.
    bool                        mMultiline;         // Is this a multi-line label control (if true, will word wrap etc.)
    cgTextTruncationMode::Base  mTruncationMode;    // The type of truncation that should be applied if this is a single line control and the client area is too small.
    bool                        mAutoSize;          // When enabled, the control will automatically re-size itself to fit any specified text (single line only).
    cgHorizontalAlignment::Base mHorizontalAlign;   // Describes how the text should be aligned horizontally
    cgVerticalAlignment::Base   mVerticalAlign;     // Describes how the text should be aligned vertically
    bool                        mAllowFormatCode;   // Are formatting code tags allowed?
};

#endif // !_CGE_CGLABELCONTROL_H_