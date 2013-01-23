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
// Name : cgButtonControl.h                                                  //
//                                                                           //
// Desc : Built in user interface button control.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBUTTONCONTROL_H_ )
#define _CGE_CGBUTTONCONTROL_H_

//-----------------------------------------------------------------------------
// cgButtonControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLabelControl;
class cgImageBoxControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {BAD41274-389D-418E-92C0-0E325B0AECED}
const cgUID RTID_UIButtonControl    = {0xBAD41274, 0x389D, 0x418E, {0x92, 0xC0, 0xE, 0x32, 0x5B, 0xA, 0xEC, 0xED}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgButtonControl (Class)
/// <summary>
/// An interface button that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgButtonControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgButtonControl, cgUIControl, "Button" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgButtonControl( );
             cgButtonControl( const cgString & strElementName );
    virtual ~cgButtonControl( );

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIButtonControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            onClick             ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    setImage            ( const cgString & referenceName );
    const cgString        & getImage            ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgLabelControl    * mLabel;     // The label control that will render the button text.
    cgImageBoxControl * mImage;     // The image control that will render the button image (if any specified).
    
};

#endif // !_CGE_CGBUTTONCONTROL_H_