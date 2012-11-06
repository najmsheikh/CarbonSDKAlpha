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
// Name : cgSSAOProcessor.h                                                  //
//                                                                           //
// Desc : Image processing class designed to compute and apply ambient       //
//        occlusion for a scene in screen space as a post process.           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGSSAOPROCESSOR_H_)
#define _CGE_CGSSAOPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgSSAOProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSSAOProcessor (Class)
/// <summary>
/// Image processing class designed to compute and apply ambient occlusion for 
/// a scene in screen space as a post process.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSSAOProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSSAOProcessor, cgImageProcessor, "SSAOProcessor" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum SSAOMethod
    {
        HemisphereAO = 0,
        VolumetricAO = 1,
        ObscuranceAO = 2
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSSAOProcessor( );
    virtual ~cgSSAOProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            setSamplingMethod       ( SSAOMethod method );
    void            setSampleCount          ( cgInt32 samples );
    void            setSampleRadii          ( cgInt32 radiusCount, const cgVector4 & radii, const cgVector4 & powers, const cgVector4 & maximumDistances );
    void            setBiasFactor           ( cgFloat bias );
    void            setDepthFalloff         ( cgFloat falloff );
    bool            execute                 ( cgCameraNode * activeCamera, const cgTextureHandle & depthSource, const cgTextureHandle & normalSource, const cgRenderTargetHandle & destination, const cgDepthStencilTargetHandle & depthStencilTarget );

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
    struct _cbSSAO
    {
		cgVector4 textureSize;
		cgVector4 sampleTextureSize;
		cgVector4 planeBias; 
		cgVector2 sampleScreenToViewScale;
		cgVector2 sampleScreenToViewBias;
		cgVector2 sampleFocalLength;
		cgFloat   worldRadius;
		cgFloat   intensity;
		cgFloat   maxMipLevel;
		cgFloat   mipBias;
    };
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mSSAOShader;
    cgConstantBufferHandle  mSSAOConstants;
    cgTextureHandle         mRandomRotation180;
    cgTextureHandle         mRandomRotation360;
    cgSampler             * mDepthSampler;
    cgSampler             * mNormalSampler;
    cgSampler             * mRotationSampler;
    
    // Configuration
    SSAOMethod              mMethod;
    cgInt32                 mSampleCount;
    cgInt32                 mRadiusCount;
    _cbSSAO                 mOperationData;
    
    // Data retained during execution.
};

#endif // !_CGE_CGSSAOPROCESSOR_H_