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
// Name : cgImageBoxControl.h                                                //
//                                                                           //
// Desc : Built in user interface image box control.                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGIMAGEBOXCONTROL_H_ )
#define _CGE_CGIMAGEBOXCONTROL_H_

//-----------------------------------------------------------------------------
// cgImageBoxControl Header Includes
//-----------------------------------------------------------------------------
#include <Interface/cgUIControl.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7C2D4240-49FA-4F2C-9C02-085C17B0FC43}
const cgUID RTID_UIImageBoxControl  = {0x7C2D4240, 0x49FA, 0x4F2C, {0x9C, 0x2, 0x8, 0x5C, 0x17, 0xB0, 0xFC, 0x43}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgImageBoxControl(Class)
/// <summary>
/// An image container that can be attached to a form or as a child of
/// any other control.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgImageBoxControl : public cgUIControl
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgImageBoxControl, cgUIControl, "ImageBox" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgImageBoxControl( );
    virtual ~cgImageBoxControl( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgUIControl)
    //-------------------------------------------------------------------------
    virtual void            renderSecondary     ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_UIImageBoxControl; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    setImage            ( const cgString & referenceName );
    const cgString        & getImage            ( ) const;
    cgSize                  getImageSize        ( ) const;
    void                    setScaleMode        ( cgImageScaleMode::Base mode );
    cgImageScaleMode::Base  getScaleMode        ( ) const;

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    bool                    autoSize;   // Should the control be automatically sized when an image is specified

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString                mImageReference;    // The current image reference.
    cgString                mLibrary;           // The library referenced by the above string.
    cgString                mLibraryItem;       // The item specified in the above library.
    cgImageScaleMode::Base  mScaleMode;         // What scale mode are we using for rendering the image.
};

#endif // !_CGE_CGIMAGEBOXCONTROL_H_