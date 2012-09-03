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
// Name : cgAntialiasProcessor.h                                             //
//                                                                           //
// Desc : Image processing class designed to remove aliasing artifacts from  //
//        a rendered image using post-process screen based filters.          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGANTIALIASPROCESSOR_H_)
#define _CGE_CGANTIALIASPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgAntialiasProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAntialiasProcessor (Class)
/// <summary>
/// Image processing class designed to remove aliasing artifacts from a 
/// rendered image using post-process screen based filters.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAntialiasProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAntialiasProcessor, cgImageProcessor, "AntialiasProcessor" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAntialiasProcessor( );
    virtual ~cgAntialiasProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
	bool			executeFXAA             ( const cgTextureHandle & source, const cgTextureHandle & velocity, const cgRenderTargetHandle & destination );
	bool			computePixelVelocity    ( cgCameraNode * activeCamera, const cgTextureHandle & depth, cgDepthType::Base depthType, const cgRenderTargetHandle & velocity );
	bool			temporalResolve		    ( const cgTextureHandle & curr, const cgTextureHandle & prev, const cgTextureHandle & velocity, const cgRenderTargetHandle & destination );

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
    // Protected Structures
    //-------------------------------------------------------------------------
    // Constant buffers
    struct _cbAntialiasing
    {
        cgVector4 textureSize;
		cgMatrix  previousViewProjMatrix; 
		cgFloat   resolveMaxSpeed;
		cgFloat   reprojectionWeight;
    };
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mAntialiasShader;
    cgConstantBufferHandle  mAntialiasConstants;
	cgSampler             * mImagePrevSampler;
	cgSampler             * mDepthSampler;
	cgSampler             * mVelocitySampler;
    cgSampler             * mAnisotropic4xSampler;
    
    // Configuration
    _cbAntialiasing         mAntialiasConfig;
};

#endif // !_CGE_CGANTIALIASPROCESSOR_H_