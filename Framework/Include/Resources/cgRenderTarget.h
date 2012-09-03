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
// Name : cgRenderTarget.h                                                   //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data.                                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRENDERTARGET_H_ )
#define _CGE_CGRENDERTARGET_H_

//-----------------------------------------------------------------------------
// cgRenderTarget Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgTexture.h>
#include <Rendering/cgRenderingTypes.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {6A114854-C42F-4AF1-92DB-C8A140CD58E4}
const cgUID RTID_RenderTargetResource = {0x6A114854, 0xC42F, 0x4AF1, {0x92, 0xDB, 0xC8, 0xA1, 0x40, 0xCD, 0x58, 0xE4}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRenderTarget (Class)
/// <summary>
/// Wrapper for managing render target (color buffer) resources.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRenderTarget : public cgTexture
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRenderTarget, cgTexture, "RenderTarget" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRenderTarget( cgUInt32 referenceId, const cgImageInfo & info );
    virtual ~cgRenderTarget( );

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgRenderTarget * createInstance      ( cgUInt32 referenceId, const cgImageInfo & info );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_RenderTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool bDisposeBase );
};



#endif // !_CGE_CGRENDERTARGET_H_