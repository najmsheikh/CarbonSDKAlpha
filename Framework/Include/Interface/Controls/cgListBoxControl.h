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
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgListBoxControl( );
    virtual ~cgListBoxControl( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt32                 addItem             ( const cgString & value );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            onSize              ( cgInt32 width, cgInt32 height );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual bool            processMessage      ( cgMessage * message );
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIListBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScrollBarControl* mVerticalScrollBar;     // The vertical scroll bar for the list box.
    cgInt32             mVerticalScrollAmount;  // The amount to scroll the list vertically.
    cgStringArray       mItems;
};

#endif // !_CGE_CGLISTBOXCONTROL_H_