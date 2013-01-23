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
// Name : cgListBoxControl.h                                                 //
//                                                                           //
// Desc : Built in user interface list box control.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLISTBOXCONTROL_H_ )
#define _CGE_CGLISTBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgListBoxControl Header Includes
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
// {950F9000-365C-425C-BAB5-832D91A6F726}
const cgUID RTID_UIListBoxControl   = {0x950F9000, 0x365C, 0x425C, {0xBA, 0xB5, 0x83, 0x2D, 0x91, 0xA6, 0xF7, 0x26}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgListBoxControl (Class)
/// <summary>
/// An interface list box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgListBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgListBoxControl, cgUIControl, "ListBox" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    // UI_ListBox_OnSelectedIndexChangeArgs Message Data
    struct UI_ListBox_OnSelectedIndexChangeArgs : public UIEventArgs
    {
        cgInt32 oldIndex;
        cgInt32 newIndex;
        
        // Constructor
        UI_ListBox_OnSelectedIndexChangeArgs( cgInt32 _oldIndex, cgInt32 _newIndex ) : 
            oldIndex(_oldIndex), newIndex(_newIndex) {}

        // Conversion to script argument list
        virtual void toArgumentList( cgScriptArgument::Array & arguments ) const
        {
            arguments.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&oldIndex ) );
            arguments.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), (void*)&newIndex ) );
        }
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgListBoxControl( );
    virtual ~cgListBoxControl( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt32                 addItem                 ( const cgString & value );
    void                    setSelectedIndex        ( cgInt32 index );
    cgInt32                 getSelectedIndex        ( ) const;
    void                    scrollToSelection       ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            onSelectedIndexChange   ( cgInt32 oldIndex, cgInt32 newIndex );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            renderSecondary         ( );
    virtual void            onSize                  ( cgInt32 width, cgInt32 height );
    virtual bool            onMouseMove             ( const cgPoint & position, const cgPointF & offset );
    virtual bool            onMouseButtonDown       ( cgInt32 buttons, const cgPoint & position );
    virtual bool            onMouseButtonUp         ( cgInt32 buttons, const cgPoint & position );
    virtual bool            onMouseWheelScroll      ( cgInt32 delta, const cgPoint & position );
    virtual bool            onKeyPressed            ( cgInt32 keyCode, cgUInt32 modifiers );
    virtual void            setFont                 ( const cgString & fontName );
    virtual void            setTextColor            ( const cgColorValue & color );
    virtual void            setVisible              ( bool visible );
    virtual void            setParentVisible        ( bool visible );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual bool            processMessage          ( cgMessage * message );
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_UIListBoxControl; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgRect                  getItemArea             ( ) const;
    cgRect                  getItemArea             ( cgControlCoordinateSpace::Base origin ) const;
    void                    computeTextMetrics      ( );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScrollBarControl* mVerticalScrollBar;     // The vertical scroll bar for the list box.
    cgInt32             mVerticalScrollAmount;  // The amount to scroll the list vertically.
    cgInt32             mSelectedIndex;         // The index of the currently selected item.
    cgStringArray       mItems;                 // List of items represented by this list box.
    cgTextMetrics       mTextMetrics;           // Cached metric information for the text in the list box
    bool                mMetricsDirty;          // When true, indicates that the list data was modified while the control was hidden.
};

#endif // !_CGE_CGLISTBOXCONTROL_H_