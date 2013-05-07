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
// Name : cgDepthOfFieldProcessor.h                                          //
//                                                                           //
// Desc : Image processing class designed to apply a depth of field / focus  //
//        blur effect to a rendered image.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGDEPTHOFFIELDPROCESSOR_H_)
#define _CGE_CGDEPTHOFFIELDPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgDepthOfFieldProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDepthOfFieldProcessor (Class)
/// <summary>
/// Image processing class designed to apply a depth of field / focus blur 
/// effect to a rendered image.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDepthOfFieldProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDepthOfFieldProcessor, cgImageProcessor, "DepthOfFieldProcessor" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDepthOfFieldProcessor( );
    virtual ~cgDepthOfFieldProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            setBackgroundExtents    ( float minimum, float maximum );
    void            setBackgroundExtents    ( const cgRangeF & range );
    void            setForegroundExtents    ( float minimum, float maximum );
    void            setForegroundExtents    ( const cgRangeF & range );
    void            setBackgroundBlur       ( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur );
    void            setBackgroundBlur       ( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                              cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow );
    void            setForegroundBlur       ( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur );
    void            setForegroundBlur       ( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                              cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow );
    bool            execute                 ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & destination, 
                                              cgResampleChain * colorChain0, cgResampleChain * colorChain1, cgResampleChain * depthChain,
                                              cgDepthType::Base depthType );

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
    struct _cbDepthOfField
    {
        cgVector4 regionScaleBias;       // x = 1/(FGMax-FGMin), y = 1/(BGMax-BGMin), z = -FGMin*x, w = -BGMin*y
        cgVector4 blurScale;
        cgVector4 blurBias;
        cgVector4 textureSize;
        cgVector4 textureSizeMedium;
        cgVector4 textureSizeLarge;
    };
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            computeBlurriness       ( bool foreground, bool background );
    void            composite               ( bool foreground, bool background );
    void            finalizeForegroundCoC   ( );
    void            downSampleBackground    ( cgInt32 level );
    void            fastBackgroundComposite ( );
    void            setConstants            ( );

    // Promote remaining base class method overloads
    using cgImageProcessor::setConstants;
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle       mDepthOfFieldShader;
    cgConstantBufferHandle      mDepthOfFieldConstants;
    cgDepthStencilStateHandle   mMaxBlurDepthState;
    cgDepthStencilStateHandle   mMaxIgnoreDepthState;
    cgBlendStateHandle          mBackgroundUpSampleBlendState;
    cgResampleChain           * mColorChain0;
    cgResampleChain           * mColorChain1;
    cgResampleChain           * mDepthChain;
    cgSampler                 * mLinearSampler;
    cgSampler                 * mPointSampler;
    cgSampler                 * mDepthSampler;
    cgSampler                 * mBlur0Sampler;
    cgSampler                 * mBlur1Sampler;
    
    // Configuration
    _cbDepthOfField             mDepthOfFieldConfig;
    cgRangeF                    mForegroundExtents;
    cgRangeF                    mBackgroundExtents;
    bool                        mApplyForegroundBlur;
    bool                        mApplyBackgroundBlur;
    bool                        mOptimizeBackground;
    bool                        mPreClearedAlpha;
    cgBlurOpDesc                mBackgroundBlurOps[2];
    cgBlurOpDesc                mForegroundBlurOps[2];
    
    // Data retained during execution.
    cgRenderTargetHandle        mSource;
    cgRenderTargetHandle        mDestination;
    cgDepthType::Base           mDepthType;
};

#endif // !_CGE_CGDEPTHOFFIELDPROCESSOR_H_