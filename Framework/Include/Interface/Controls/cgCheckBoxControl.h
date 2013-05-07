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
// Name : cgCheckBoxControl.h                                                //
//                                                                           //
// Desc : Built in user interface check box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCHECKBOXCONTROL_H_ )
#define _CGE_CGCHECKBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgCheckBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLabelControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {D5898072-9B49-466D-85D0-76E72DDAF57F}
const cgUID RTID_UICheckBoxControl = { 0xd5898072, 0x9b49, 0x466d, { 0x85, 0xd0, 0x76, 0xe7, 0x2d, 0xda, 0xf5, 0x7f } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCheckBoxControl (Class)
/// <summary>
/// An interface check box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCheckBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCheckBoxControl, cgUIControl, "CheckBox" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCheckBoxControl( );
    virtual ~cgCheckBoxControl( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    setChecked          ( bool checked );
    bool                    isChecked           ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            renderSecondary     ( );
    virtual void            setControlText      ( const cgString & text );
    virtual void            setTextColor        ( const cgColorValue & color );
    virtual void            onInitControl       ( );
    virtual void            onSize              ( cgInt32 width, cgInt32 height );
    virtual bool            onMouseMove         ( const cgPoint & position, const cgPointF & offset );
    virtual bool            onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position );
    virtual bool            onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UICheckBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            onClick             ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgLabelControl    * mLabel;             // The label control that will render the button text.
    cgUIControl       * mCheckedControl;    // The actual box display item providing a visual representation of the state of the check box when checked.
    cgUIControl       * mUncheckedControl;  // The actual box display item providing a visual representation of the state of the check box when unchecked.
    cgUIControl       * mCurrentControl;    // Current check display control.
};

#endif // !_CGE_CGCHECKBOXCONTROL_H_