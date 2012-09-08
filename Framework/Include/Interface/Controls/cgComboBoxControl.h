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
// Name : cgComboBoxControl.h                                                //
//                                                                           //
// Desc : Built in user interface combo box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCOMBOBOXCONTROL_H_ )
#define _CGE_CGCOMBOBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgComboBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgButtonControl;
class cgTextBoxControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {640ADE73-5881-41A3-9B74-87B1D99498CC}
const cgUID RTID_UIComboBoxControl  = {0x640ADE73, 0x5881, 0x41A3, {0x9B, 0x74, 0x87, 0xB1, 0xD9, 0x94, 0x98, 0xCC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgComboBoxControl (Class)
/// <summary>
/// An interface combo box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgComboBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgComboBoxControl, cgUIControl, "ComboBox" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgComboBoxControl( );
    virtual ~cgComboBoxControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            setControlText      ( const cgString & text );
    virtual void            onInitControl       ( );
    virtual void            onSize              ( cgInt32 width, cgInt32 height );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIComboBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------

    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgButtonControl   * mDropButton;    // The button which opens the combo box dropdown list
    cgTextBoxControl  * mTextBox;       // The text box for the (potentially editable) "text" portion of the combo
};

#endif // !_CGE_CGCOMBOBOXCONTROL_H_