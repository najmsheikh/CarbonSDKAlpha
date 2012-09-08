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
// Name : cgDX9RenderTarget.h                                                //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data (DX9 implementation).                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9RENDERTARGET_H_ )
#define _CGE_CGDX9RENDERTARGET_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9RenderTarget Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgRenderTarget.h>
#include <Resources/Platform/cgDX9Texture.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DSurface9;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {F1AF23DA-C2DF-4919-9867-79C6EBBAFBF9}
const cgUID RTID_DX9RenderTargetResource = {0xF1AF23DA, 0xC2DF, 0x4919, {0x98, 0x67, 0x79, 0xC6, 0xEB, 0xBA, 0xFB, 0xF9}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9RenderTarget (Class)
/// <summary>
/// Wrapper for managing render target (color buffer) resources.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9RenderTarget : public cgDX9Texture<cgRenderTarget>
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgDX9RenderDriver;

public:
    
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9RenderTarget( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX9RenderTarget( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DSurface9     * getD3DTargetSurface ( bool autoUseMultiSample = true );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    virtual void            update              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9RenderTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgDX9Texture)
    //-------------------------------------------------------------------------
    virtual bool            createTexture       ( );
    virtual void            releaseTexture      ( );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
	bool                mMSAADirty;     // Is the internal multisampling surface dirty such that the matching texture needs updating?
    IDirect3DSurface9 * mMSAASurface;   // Standard render target /surface/ into which the device will physically render if MSAA is requested (since a render target /texture/ cannot use MSAA in DX9).
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9RENDERTARGET_H_