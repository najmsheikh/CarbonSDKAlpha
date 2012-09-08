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
// Name : cgGroupBoxControl.h                                                //
//                                                                           //
// Desc : Built in user interface group box / control container.             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGGROUPBOXCONTROL_H_ )
#define _CGE_CGGROUPBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgGroupBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgLabelControl;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {C6D85EBA-2A73-462A-B83B-C33DE341B6E6}
const cgUID RTID_UIGroupBoxControl  = {0xC6D85EBA, 0x2A73, 0x462A, {0xB8, 0x3B, 0xC3, 0x3D, 0xE3, 0x41, 0xB6, 0xE6}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgGroupBoxControl (Class)
/// <summary>
/// An interface combo box that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgGroupBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgGroupBoxControl, cgUIControl, "GroupBox" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgGroupBoxControl( );
    virtual ~cgGroupBoxControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            setControlText      ( const cgString & text );
    virtual void            onInitControl       ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIGroupBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    updateGroupLabel    ( );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgLabelControl* mGroupLabel;        // The label control used to display the group label text.
    cgUIControl   * mOldBorder;         // The original BorderTopBegin element.

};

#endif // !_CGE_CGGROUPBOXCONTROL_H_