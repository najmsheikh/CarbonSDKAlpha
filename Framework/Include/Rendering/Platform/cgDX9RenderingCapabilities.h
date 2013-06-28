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
// Name : cgDX9RenderingCapabilities.h                                       //
//                                                                           //
// Desc : DX9 implementation of interface through which rendering            //
//        capabilities can be queried. Tested capabilities include supported //
//        buffer formats, shader model support, format filtering support     //
//        and so on.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGDX9RENDERINGCAPABILITIES_H_)
#define _CGE_CGDX9RENDERINGCAPABILITIES_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9RenderingCapabilities Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgRenderingCapabilities.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgDX9Initialize;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9RenderingCapabilities (Class)
/// <summary>
/// DX9 implementation of interface through which rendering capabilities can be
/// queried. Tested capabilities include supported buffer formats, shader model
/// support, format filtering support and so on.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9RenderingCapabilities : public cgRenderingCapabilities
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDX9RenderingCapabilities, cgRenderingCapabilities, "RenderingCapabilities" )

public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 MaxVBTSlotsVTF = 50;   // Vertex blending transformation matrices for vertex texture fetch.
    static const cgUInt32 MaxVBTSlotsCB  = 20;   // Vertex blending transformation matrices for constant buffer.

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9RenderingCapabilities( cgRenderDriver * driver );
    virtual ~cgDX9RenderingCapabilities( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                postInit                    ( cgDX9Initialize * data, cgUInt32 fullScreenAdapter );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgRenderingCapabilities)
    //-------------------------------------------------------------------------
    virtual bool        enumerate					( );
    virtual cgUInt32    getMaxBlendTransforms		( ) const;
    virtual cgUInt32    getMaxAnisotropySamples		( ) const;
    virtual bool        supportsFastStencilFill		( ) const;
    virtual bool        supportsNonPow2Textures		( ) const;
	virtual bool        supportsDepthStencilReading ( ) const;
    virtual bool        supportsShaderModel			( cgShaderModel::Base model ) const;
    virtual bool        requiresCursorEmulation     ( ) const;
    virtual bool        getDisplayModes             ( cgDisplayMode::Array & modes ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Member Variables
    //-------------------------------------------------------------------------
    D3DCAPS9                mHardwareCaps;      // Capabilities of the selected D3D device.
    cgDisplayMode::Array    mDisplayModes;      // Enumerated full screen display modes available for selection.
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9RENDERINGCAPABILITIES_H_