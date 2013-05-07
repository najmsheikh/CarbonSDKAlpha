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
// Name : cgDX11DepthStencilTarget.h                                         //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//        (DX11 implementation).                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11DEPTHSTENCILTARGET_H_ )
#define _CGE_CGDX11DEPTHSTENCILTARGET_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11DepthStencilTarget Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/Platform/cgDX11Texture.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {3C1A0681-D429-40F6-AEA7-24095E79F16A}
const cgUID RTID_DX11DepthStencilTargetResource = {0x3C1A0681, 0xD429, 0x40F6, {0xAE, 0xA7, 0x24, 0x9, 0x5E, 0x79, 0xF1, 0x6A}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11DepthStencilTarget (Class)
/// <summary>
/// Wrapper for managing a depth stencil target.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11DepthStencilTarget : public cgDX11Texture<cgDepthStencilTarget>
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11DepthStencilTarget( cgUInt32 referenceId, const cgImageInfo & description );
    virtual ~cgDX11DepthStencilTarget( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgTexture)
    //-------------------------------------------------------------------------
    ID3D11DepthStencilView* getD3DTargetView    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11DepthStencilTargetResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgDX11Texture)
    //-------------------------------------------------------------------------
    virtual bool            createTexture       ( );
    virtual void            releaseTexture      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ID3D11DepthStencilView* mView;      // View that allows us to map the texture as a depth stencil target.
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11DEPTHSTENCILTARGET_H_