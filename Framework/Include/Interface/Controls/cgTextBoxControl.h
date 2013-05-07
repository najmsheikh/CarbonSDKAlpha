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
// Name : cgTextBoxControl.h                                                 //
//                                                                           //
// Desc : Built in user interface text box control.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTEXTBOXCONTROL_H_ )
#define _CGE_CGTEXTBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgTextBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>
#include <Interface/cgTextEngine.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScrollBarControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {BDCCD914-953A-4E8A-98B3-0FFD82E0F648}
const cgUID RTID_UITextBoxControl   = {0xBDCCD914, 0x953A, 0x4E8A, {0x98, 0xB3, 0xF, 0xFD, 0x82, 0xE0, 0xF6, 0x48}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTextBoxControl (Class)
/// <summary>
/// An interface text box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTextBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgTextBoxControl, cgUIControl, "TextBox" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTextBoxControl( );
             cgTextBoxControl( const cgString & elementName );
    virtual ~cgTextBoxControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            renderSecondary     ( );
    virtual void            setControlText      ( const cgString& text );
    virtual void            onSize              ( cgInt32 width, cgInt32 height );
    virtual bool            onMouseMove         ( const cgPoint & position, const cgPointF & offset );
    virtual bool            onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position );
    virtual bool            onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position );
    virtual bool            onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position );
    virtual bool            onKeyPressed        ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void            setFont             ( const cgString & fontName );
    virtual void            setTextColor        ( const cgColorValue & color );
    virtual void            setVisible          ( bool visible );
    virtual void            setParentVisible    ( bool visible );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UITextBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    virtual bool            processMessage      ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    setSelectionRange       ( cgInt32 start, cgInt32 end );
    cgString                getSelectedText         ( ) const;
    void                    setMultiline            ( bool multiline );
    bool                    getMultiline            ( ) const { return mMultiline; }
    void                    setReadOnly             ( bool readOnly );
    bool                    getReadOnly             ( ) const;
    void                    setAllowFormatCode      ( bool allow );
    bool                    getAllowFormatCode      ( ) const;
    void                    insertString            ( cgInt32 position, const cgString & text );
    void                    scrollToCaret           ( );
    void                    scrollToStart           ( );
    void                    scrollToEnd             ( );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgRect                  getTextArea         ( ) const;
    cgRect                  getTextArea         ( cgControlCoordinateSpace::Base origin ) const;
    cgInt32                 characterFromPoint  ( const cgPoint & position, cgTextMetrics::MetricRef * locationOut = CG_NULL );
    void                    computeTextMetrics  ( );
    void                    clearSelection      ( );
    bool                    hasSelection        ( ) const;
    void                    removeCharacter     ( cgInt32 character );
    void                    resetCaret          ( const cgPoint & position, bool adjustSelection = false );
    void                    resetCaret          ( cgInt32 position, bool adjustSelection = false );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgInt32                     mSelectionStart;        // Starting character for selection range
    cgInt32                     mSelectionEnd;          // The ending character of the selection range
    cgInt32                     mSelectionRef;          // The reference character selected originally on click
    cgInt32                     mCaretCharacter;        // The character prior to which the caret is currently selected
    cgTextMetrics::MetricRef    mMetricSelStart;        // Reference to an item in the metrics structure
    cgTextMetrics::MetricRef    mMetricSelEnd;          // Reference to an item in the metrics structure
    cgTextMetrics::MetricRef    mMetricSelRef;          // Reference to an item in the metrics structure
    cgTextMetrics::MetricRef    mMetricCaretChar;       // Reference to an item in the metrics structure
    bool                        mShowCaret;             // Should the caret be visible?
    cgTextMetrics               mTextMetrics;           // Cached metric information for the text in the textbox
    cgScrollBarControl        * mVerticalScrollBar;     // The vertical scroll bar for multi-line text boxes
    cgInt32                     mVerticalScrollAmount;  // The amount to scroll the text vertically.
    bool                        mMultiline;             // Is this a multi-line text box control (if true, will word wrap etc.)
    bool                        mReadOnly;              // Is this a read-only text box (if true, will prevent user input).
    bool                        mAllowFormatCode;       // Are formatting code tags allowed?
    bool                        mMetricsDirty;          // When true, indicates that the text was modified while the control was hidden.
};

#endif // !_CGE_CGTEXTBOXCONTROL_H_