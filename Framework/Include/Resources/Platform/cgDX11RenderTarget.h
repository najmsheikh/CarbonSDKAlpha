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
// Name : cgDX11RenderTarget.h                                               //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data (DX11 implementation).                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11RENDERTARGET_H_ )
#define _CGE_CGDX11RENDERTARGET_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11RenderTarget Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgRenderTarget.h>
#include <Resources/Platform/cgDX11Texture.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A363D84E-26E8-45FF-85E3-65B61D1ADA01}
const cgUID RTID_DX11RenderTargetResource = {0xA363D84E, 0x26E8, 0x45FF, {0x85, 0xE3, 0x65, 0xB6, 0x1D, 0x1A, 0xDA, 0x1}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderTarget (Class)
/// <summary>
/// Wrapper for managing render target (color buffer) resources.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11RenderTarget : public cgDX11Texture<cgRenderTarget>
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11RenderTarget( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX11RenderTarget( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    ID3D11RenderTargetView* getD3DTargetView    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    virtual void            update              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11RenderTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgDX11Texture)
    //-------------------------------------------------------------------------
    virtual bool            createTexture       ( );
    virtual void            releaseTexture      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ID3D11RenderTargetView* mView;  // View that allows us to map the texture as a render target.

};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11RENDERTARGET_H_