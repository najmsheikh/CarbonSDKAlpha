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
// Name : cgDX9DepthStencilTarget.h                                          //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//        (DX9 implementation).                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9DEPTHSTENCILTARGET_H_ )
#define _CGE_CGDX9DEPTHSTENCILTARGET_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9DepthStencilTarget Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/Platform/cgDX9Texture.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DSurface9;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {AFE53457-3985-4674-A679-96D74A3E1554}
const cgUID RTID_DX9DepthStencilTargetResource = {0xAFE53457, 0x3985, 0x4674, {0xA6, 0x79, 0x96, 0xD7, 0x4A, 0x3E, 0x15, 0x54}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9DepthStencilTarget (Class)
/// <summary>
/// Wrapper for managing a depth stencil target.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9DepthStencilTarget : public cgDX9Texture<cgDepthStencilTarget>
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9DepthStencilTarget( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX9DepthStencilTarget( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DSurface9     * getD3DTargetSurface ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9DepthStencilTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgDX9Texture)
    //-------------------------------------------------------------------------
    virtual bool            createTexture       ( );
    virtual void            releaseTexture      ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    IDirect3DSurface9     * mSurface;   // Underlying Resource
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9DEPTHSTENCILTARGET_H_