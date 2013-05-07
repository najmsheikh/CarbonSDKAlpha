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
// Name : cgAtmosphericsProcessor.h                                          //
//                                                                           //
// Desc : Image processing class designed to apply atmospherics effects such //
//        as fog to a rendered scene image.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGATMOSPHERICSPROCESSOR_H_)
#define _CGE_CGATMOSPHERICSPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgAtmosphericsProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSkyElement;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAtmosphericsProcessor (Class)
/// <summary>
/// Image processing class designed to apply atmospherics effects such as fog 
/// to a rendered scene image.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAtmosphericsProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAtmosphericsProcessor, cgImageProcessor, "AtmosphericsProcessor" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAtmosphericsProcessor( );
    virtual ~cgAtmosphericsProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            drawSky                 ( cgSkyElement * properties, bool decodeSRGB );
    void            drawSky                 ( cgSkyElement * properties, bool decodeSRGB, const cgRenderTargetHandle & destination );
    void            drawSkyColor            ( cgColorValue color, bool decodeSRGB );
    void            drawSkyColor            ( cgColorValue color, bool decodeSRGB, const cgRenderTargetHandle & destination );
    void            drawSkyBox              ( const cgTextureHandle & skyCubeTexture, bool decodeSRGB );
    void            drawSkyBox              ( const cgTextureHandle & skyCubeTexture, bool decodeSRGB, const cgRenderTargetHandle & destination );
    void            drawFog                 ( cgFogModel::Base model, const cgTextureHandle & depthTexture, cgDepthType::Base depthType );
    void            drawFog                 ( cgFogModel::Base model, const cgTextureHandle & depthTexture, cgDepthType::Base depthType, const cgRenderTargetHandle & destination );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgImageProcessor)
    //-------------------------------------------------------------------------
    virtual bool    initialize              ( cgRenderDriver * driver );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structurs
    //-------------------------------------------------------------------------
    struct _cbSkyData
    {
        float           hdrScale;
        cgColorValue    baseColor;
    };

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mAtmosphericsShader;
    cgInt32                 mSkySamplerRegister;
    cgConstantBufferHandle  mSkyConstants;
    _cbSkyData              mSkyConfig;
    cgSampler             * mSkyBoxSampler;
    cgSampler             * mDepthSampler;
    
};

#endif // !_CGE_CGATMOSPHERICSPROCESSOR_H_