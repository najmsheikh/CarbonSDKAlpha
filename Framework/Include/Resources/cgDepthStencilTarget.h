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
// Name : cgDepthStencilTarget.h                                             //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDEPTHSTENCILTARGET_H_ )
#define _CGE_CGDEPTHSTENCILTARGET_H_

//-----------------------------------------------------------------------------
// cgDepthStencilTarget Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgTexture.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {7CFD9E6F-7439-4B5C-9873-90B870D80272}
const cgUID RTID_DepthStencilTargetResource = {0x7CFD9E6F, 0x7439, 0x4B5C, {0x98, 0x73, 0x90, 0xB8, 0x70, 0xD8, 0x2, 0x72}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDepthStencilTarget (Class)
/// <summary>
/// Wrapper for managing a depth stencil target.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDepthStencilTarget : public cgTexture
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDepthStencilTarget, cgTexture, "DepthStencilTarget" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDepthStencilTarget( cgUInt32 referenceId, const cgImageInfo & info );
    virtual ~cgDepthStencilTarget( );

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgDepthStencilTarget * CreateInstance( cgUInt32 referenceId, const cgImageInfo & info );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DepthStencilTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGDEPTHSTENCILTARGET_H_