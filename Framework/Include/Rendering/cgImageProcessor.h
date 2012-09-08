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
// Name : cgImageProcessor.h                                                 //
//                                                                           //
// Desc : TODO 6767
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGIMAGEPROCESSOR_H_)
#define _CGE_CGIMAGEPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgImageProcessor Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Rendering/cgRenderingTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgResampleChain;
class cgSampler;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgImageProcessor (Class)
/// <summary>
/// TODO 6767
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgImageProcessor : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgImageProcessor, "ImageProcessor" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgImageProcessor( );
    virtual ~cgImageProcessor( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool    initialize              ( cgRenderDriver * driver );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Parameter setup
    void            setDepthStencilTarget   ( const cgDepthStencilTargetHandle & target );
    void            setDefaultUserColor     ( const cgColorValue & c );
    void            setDepthExtents         ( cgFloat minimum, cgFloat maximum );
    void            setNormalExtents        ( cgFloat minimum, cgFloat maximum );
    void            setNormalDistances      ( cgFloat minimum, cgFloat maximum );
    void            setWhiteLevel           ( const cgVector2 & v );
    void            setBlackLevel           ( const cgVector2 & v );
    void            setGamma                ( cgFloat f );
    void            setTint                 ( const cgColorValue & c );
    void            setBrightness           ( cgFloat f );
    void            setExposure             ( cgFloat exposure, cgFloat strength );
    void            setSaturation           ( cgFloat f );
    void            setGrain                ( const cgTextureHandle & texture, cgFloat strength );
    void            setVignette             ( const cgTextureHandle & texture, cgFloat scale, cgFloat bias, cgFloat strength );
    void            setRemap                ( const cgTextureHandle & texture );
    void            forceLinearSampling     ( bool enable );
    void            forcePointSampling      ( bool enable );
    void            testStencilBuffer       ( bool enable, cgUInt32 stencilRef );
    void            applyDepthStencilStates ( bool enable );
    void            applyRasterizerStates   ( bool enable );
    void            applyBlendStates        ( bool enable );

    // Operations
    void            processColorImage       ( const cgRenderTargetHandle & destination, cgImageOperation::Base operation );
    void            processColorImage       ( const cgRenderTargetHandle & destination, cgImageOperation::Base operation, const cgColorValue & c );
    void            processColorImage       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation);
    void            processColorImage       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation, const cgColorValue & c );

    void            processColorImageMulti  ( const cgRenderTargetHandle & destination, const cgImageOperation::Array & operations );
    void            processColorImageMulti  ( const cgRenderTargetHandle & destination, const cgImageOperation::Array & operations, const cgColorValue & c );
    void            processColorImageMulti  ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, const cgImageOperation::Array & operations );
    void            processColorImageMulti  ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, const cgImageOperation::Array & operations, const cgColorValue & c );

    void            processDepthImage       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgDepthType::Base inputType, cgDepthType::Base outputType );
    void            processDepthImage       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgDepthType::Base inputType, cgDepthType::Base outputType, const cgDepthStencilTargetHandle & depthBuffer );

    void            downSample              ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    void            downSample              ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation );
    void            downSample              ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation, bool alphaWeighted, bool alphaWeightedBinary, bool alphaBinaryOutput );
    void            downSample              ( cgResampleChain * chain );
    void            downSample              ( cgResampleChain * chain, cgImageOperation::Base operation );
    void            downSample              ( cgResampleChain * chain, cgImageOperation::Base operation, bool alphaWeighted, bool alphaWeightedBinary, bool alphaBinaryOutput );

    void            downSampleDepth         ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation );
    void            downSampleDepth         ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation, bool recordOffset );
    void            downSampleDepth         ( const cgTextureHandle & sourceDepth, const cgTextureHandle & sourceData, const cgRenderTargetHandle & destinationDepth, const cgRenderTargetHandle & destinationData,
                                              cgDepthType::Base inputType, cgDepthType::Base outputType, cgImageOperation::Base operation, bool recordOffset );

    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, cgInt32 pixelRadius, cgFloat distanceFactor, cgInt32 passCount );
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, cgInt32 pixelRadius, cgFloat distanceFactor, cgInt32 passCount, cgAlphaWeightMethod::Base inputAlpha, cgAlphaWeightMethod::Base outputAlpha);
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, cgInt32 pixelRadius, cgFloat distanceFactor, cgFloat worldRadius, cgInt32 passCount );
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, cgInt32 pixelRadius, cgFloat distanceFactor, cgFloat worldRadius, cgInt32 passCount, cgAlphaWeightMethod::Base inputAlpha, cgAlphaWeightMethod::Base outputAlpha );
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgBlurOpDesc & description );
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgBlurOpDesc::Array & steps );
    void            blur                    ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgBlurOpDesc::Array & steps );

    void            bilateralBlur           ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgTextureHandle & normal,
                                              cgInt32 pixelRadius, cgFloat distanceFactor, cgInt32 passCount, bool compareDepth, bool compareNormal, cgDepthType::Base depthType );
    void            bilateralBlur           ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgTextureHandle & normal,
                                              cgInt32 pixelRadius, cgFloat distanceFactor, cgInt32 passCount, bool compareDepth, bool compareNormal, cgDepthType::Base depthType, bool depthAlphaAsColor );
    void            bilateralBlur           ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgTextureHandle & normal,
                                              cgInt32 pixelRadius, cgFloat distanceFactor, cgFloat kernelAdjust, cgInt32 passCount, bool compareDepth, bool compareNormal, cgDepthType::Base depthType );
    void            bilateralBlur           ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgTextureHandle & normal,
                                              cgInt32 pixelRadius, cgFloat distanceFactor, cgFloat kernelAdjust, cgInt32 passCount, bool compareDepth, bool compareNormal, cgDepthType::Base depthType, bool depthAlphaAsColor );
    void            bilateralBlur           ( const cgRenderTargetHandle & source, const cgRenderTargetHandle & scratch, const cgTextureHandle & depth, const cgTextureHandle & normal,
                                              cgInt32 pixelRadius, cgFloat distanceFactor, cgFloat kernelAdjust, cgInt32 passCount, bool compareDepth, bool compareNormal, cgDepthType::Base depthType, bool depthAlphaAsColor, bool alphaWeighted, cgFloat worldRadius, cgFloat anisotropy );

    void            resample                ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    void            resample                ( const cgTextureHandle & source, const cgTextureHandle & sourceDepth, const cgTextureHandle & sourceNormal, 
                                              const cgRenderTargetHandle & destination, const cgTextureHandle & destDepth, const cgTextureHandle & destNormal, 
                                              bool compareDepth, bool compareNormal, cgDepthType::Base sourceDepthType, cgDepthType::Base destinationDepthType, 
                                              bool depthAlphaAsColor, bool clipFailedPixels, bool testEdges );

    void            discontinuityDetect     ( const cgTextureHandle & sourceDepth, const cgTextureHandle & sourceNormal, const cgRenderTargetHandle & destination, cgDiscontinuityTestMethod::Base method, bool preserveDirection, cgDepthType::Base depthType );

    void            packDepthNormal         ( const cgTextureHandle & sourceDepth, const cgRenderTargetHandle & destination, bool depthToNormalTarget );
    void            packDepthNormal         ( const cgTextureHandle & sourceDepth, const cgTextureHandle & sourceNormal, const cgRenderTargetHandle & destination, bool depthToNormalTarget );

    void            computeScreenMask       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    void            downSampleScreenMask    ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    void            resolveScreenMask       ( const cgTextureHandle & source );
    void            resolveScreenMask       ( const cgTextureHandle & source, bool stencilOnly );
    void            resolveScreenMask       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    void            resolveScreenMask       ( const cgTextureHandle & source, cgFloat z );
    void            resolveScreenMask       ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgFloat z, bool stencilOnly );
    
    void            blendFrames             ( const cgTextureHandle & currentColor, const cgTextureHandle & prevColor,
                                              const cgTextureHandle & currentDepth, const cgTextureHandle & prevDepth, 
                                              const cgRenderTargetHandle & destination, const cgMatrix & reprojection, cgFloat depthTolerance, cgFloat blendAmount );

    void            drawClipQuad            ( );
    void            drawClipQuad            ( cgFloat z );
    void            drawClipQuad            ( const cgRenderTargetHandle & destination );
    void            drawClipQuad            ( const cgRenderTargetHandle & destination, cgFloat z );
    void            drawQuad                ( const cgRenderTargetHandle & destination, bool clipSpace, cgFloat z );

    bool            selectClipQuadVertexShader   ( );
    bool            selectClipQuadVertexShader   ( bool computeEyeRay );
    void            setConstants            ( const cgTextureHandle & sourceImage, cgInt32 operation, const cgColorValue & c );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct Samplers
    {
        cgSampler * point;
        cgSampler * linear;
        cgSampler * imageLinear;
        cgSampler * grain;
        cgSampler * vignette;
        cgSampler * remap;
        cgSampler * verticalPoint;
        cgSampler * verticalLinear;
        cgSampler * horizontalPoint;
        cgSampler * horizontalLinear;
        cgSampler * depthSrc;
        cgSampler * normalSrc;
        cgSampler * depthDest;
        cgSampler * normalDest;
        cgSampler * curr;
        cgSampler * prev;
        cgSampler * edge;
    };

    struct MultiOpShaderCacheKey
    {
        cgImageOperation::Array operations;
        MultiOpShaderCacheKey( const cgImageOperation::Array & _operations ) :
            operations(_operations) {}
        bool operator < ( const MultiOpShaderCacheKey & key2 ) const
        {
            if ( operations.size() < key2.operations.size() )
                return true;
            if ( operations.size() > key2.operations.size() )
                return false;
            if ( operations.empty() )
                return false;
            return (memcmp( &operations.front(), &key2.operations.front(), operations.size() * sizeof(cgImageOperation::Base) ) < 0);
        }
    };
    CGE_MAP_DECLARE( MultiOpShaderCacheKey, cgPixelShaderHandle, MultiOpShaderCache )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void setBlendState          ( cgImageOperation::Base operation );
    bool useBilinearSampling    ( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgRenderDriver            * mDriver;                        // Render driver to which the image processor is linked

    cgSurfaceShaderHandle       mProcessShaderHandle;           // Handle to the image processing shader resource.
    cgSurfaceShader *           mProcessShader;                 // Actual application side image processing shader object.
    cgConstantBufferHandle      mImageProcessingConstants;
    cgConstantBufferHandle      mColorControlsConstants;
    cgConstantBufferHandle      mBilateralConstants;
    cgConstantBufferHandle      mReprojectionConstants;

    // Configuration
    bool                        mApplyDepthStencilState;
    bool                        mApplyRasterizerState;
    bool                        mApplyBlendState;
    bool                        mAlphaChannelOnly;
    bool                        mTestStencilBuffer;
    bool                        mForceLinearSampling;
    bool                        mForcePointSampling;
    cgUInt32                    mStencilRef;
    cgColorValue                mDefaultUserColor;

    // Inputs
    cgTextureHandle             mVignetteTexture;
    cgTextureHandle             mGrainTexture;
    cgTextureHandle             mRemapTexture;
    cgDepthStencilTargetHandle  mDepthStencilTarget;

    // Depth Stencil States
    cgDepthStencilStateHandle   mDisabledDepthState;
    cgDepthStencilStateHandle   mGreaterDepthState;
    cgDepthStencilStateHandle   mLessEqualDepthState;
    cgDepthStencilStateHandle   mEqualDepthState;
    cgDepthStencilStateHandle   mWriteDepthState;
    cgDepthStencilStateHandle   mWriteStencilMaskState;
    cgDepthStencilStateHandle   mReadStencilMaskState;

    cgDepthStencilStateHandle   mMaskPass0DepthState;
    cgDepthStencilStateHandle   mMaskPass1DepthState;
    cgDepthStencilStateHandle   mMaskNearDepthState;
    cgDepthStencilStateHandle   mMaskFarDepthState;

    // Blend States
    cgBlendStateHandle          mDefaultRGBABlendState;
    cgBlendStateHandle          mDefaultRGBBlendState;
    cgBlendStateHandle          mDefaultRGBlendState;
    cgBlendStateHandle          mDefaultBABlendState;
    cgBlendStateHandle          mDefaultAlphaOnlyBlendState;
    cgBlendStateHandle          mAdditiveRGBABlendState;
    cgBlendStateHandle          mAdditiveRGBBlendState;
    cgBlendStateHandle          mAlphaBlendState;
    cgBlendStateHandle          mModulativeRGBABlendState;
    cgBlendStateHandle          mModulativeRGBBlendState;
    cgBlendStateHandle          mModulativeAlphaBlendState;
    cgBlendStateHandle          mColorWriteDisabledBlendState;
    cgBlendStateHandle          mAddRGBSetAlphaBlendState;
    cgBlendStateHandle          mStandardAlphaBlendState;

    // Samplers
    Samplers                    mSamplers;

    // Shader cache for multi-operation list.
    MultiOpShaderCache          mMultiOpShaderCache;
};

#endif // !_CGE_CGIMAGEPROCESSOR_H_