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
// Name : cgGlareProcessor.h                                                 //
//                                                                           //
// Desc : Image processing class designed to apply a glare / bloom to a      //
//        rendered image.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGGLAREPROCESSOR_H_)
#define _CGE_CGGLAREPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgGlareProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgToneMapProcessor;
class cgAntialiasProcessor;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgGlareProcessor (Class)
/// <summary>
/// Image processing class designed to apply a glare / bloom to a rendered 
/// image.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgGlareProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgGlareProcessor, cgImageProcessor, "GlareProcessor" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs & Structures
    //-------------------------------------------------------------------------
    struct GlareStepDesc
    {
        cgInt32         levelIndex;
        cgFloat         intensity;
        cgBlurOpDesc    blurOp;
		cgFloat         cacheBlendAmount;
		cgFloat         cacheBlendRate;

        // Constructors
        GlareStepDesc() {}
        GlareStepDesc( cgInt32 _levelIndex, cgFloat _levelIntensity, cgInt32 _blurPassCount, cgInt32 _blurPixelRadius, cgFloat _blurDistanceFactor, cgFloat _cacheBlendAmount, cgFloat _cacheBlendRate ) :
            levelIndex( _levelIndex ), intensity( _levelIntensity ), blurOp( _blurPassCount, _blurPixelRadius, _blurDistanceFactor ), cacheBlendAmount(_cacheBlendAmount), cacheBlendRate(_cacheBlendRate) {}
    };
    CGE_VECTOR_DECLARE( GlareStepDesc, GlareStepArray )

	struct ILRElement
	{
		cgFloat flareDelta;
		cgFloat flareAlpha;
		bool    flareHighDetail;
		
		ILRElement(){}
		ILRElement( cgFloat delta, cgFloat alpha, bool highDetail ) : flareDelta(delta), flareAlpha(alpha), flareHighDetail(highDetail){}
	};
	CGE_VECTOR_DECLARE( ILRElement, ILRElementArray )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgGlareProcessor( );
    virtual ~cgGlareProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void            setBrightThreshold      ( cgFloat minimum, cgFloat maximum );
    void            setBrightThreshold      ( const cgRangeF & range );
	void            setGlareAmount          ( cgFloat amount );
	void            setCacheValues          ( cgFloat amount, cgFloat rate );
    void            setGlareSteps           ( const GlareStepArray & steps );
    bool            execute                 ( const cgRenderTargetHandle & target, cgResampleChain * chain0, cgResampleChain * chain1, bool hdrGlare, float alphaMaskAmt, bool fullSizeBrightPass, bool blurPrePass );

	void            setILRBrightThreshold   ( cgFloat minimum );
	void            setILRContrast          ( cgFloat contrast );
	void			setILRDepthRange        ( cgFloat minimum, cgFloat maximum );
	void            setILRElements          ( const ILRElementArray & elements );
	void            setILRLowResData        ( cgInt32 targetPadding, cgInt32 filterPasses, cgInt32 filterSize, cgFloat filterFactor );
	void            setILRHighResData       ( cgInt32 targetPadding, cgInt32 filterPasses, cgInt32 filterSize, cgFloat filterFactor );
	bool            executeILR              ( const cgRenderTargetHandle & target, const cgTextureHandle & depth, cgResampleChain * chain0, cgResampleChain * chain1, bool hdr, bool materialAlphaMask );

	void            setAnamorphicData       ( cgInt32 numPasses, cgInt32 radius, cgFloat intensity, const cgVector3 & flareColor, cgFloat edgeScale, cgFloat edgeBias );
	void            setToneMapper           ( cgToneMapProcessor * toneMapper );

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
    struct _cbUpdateCache
    {
        cgFloat blendAmount;
        cgFloat blendRate;
    };

	struct _cbAddLayer
	{
		cgFloat layerWeight;
	};

    struct _cbBrightPass
    {
        cgFloat   brightRangeScale;
        cgFloat   brightRangeBias;
		cgFloat   alphaMaskAmount;
        cgVector2 textureSizeRecip;
    };

	struct _cbILRBrightPass
	{
		cgFloat   brightThreshold;
		cgFloat   flareContrast;
		cgVector2 distanceAttenScaleBias;
	};

	struct _cbILRCompositePass
	{
		cgFloat flareDelta;
		cgFloat flareAlpha;
		cgFloat flareSlice;
	};

	struct _cbAnamorphicFlare
	{
		cgVector3 flareColor;
		cgFloat   flarePassScale;
		cgFloat   flareCoordScale;
		cgFloat   flareCoordBias;
	};

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            brightPass              ( );
    void            downsampleAndBlur       ( );
	void			updateCache				( size_t levelIndex );
    size_t          getGlareStepIndex       ( size_t levelIndex );
	void			computeAnamorphicFlares ( size_t levelIndex );
	void			internalLensReflection  ( );

	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mGlareShader;
    cgConstantBufferHandle  mBrightPassConstants;
    cgConstantBufferHandle  mUpdateCacheConstants;
	cgConstantBufferHandle  mAddLayerConstants;
	cgConstantBufferHandle  mAnamorphicConstants;
	cgBlendStateHandle      mLayerBlendState;

    cgSampler             * mPointSampler;
    cgSampler             * mLinearSampler;
	cgSampler             * mPointBorderSampler;
	cgSampler             * mLinearBorderSampler;
	cgSampler             * mLuminanceSampler;

	cgToneMapProcessor    * mToneMapper;

    // Configuration
	cgFloat                 mGlareAmount;
	cgRangeF                mBrightThreshold;
    _cbBrightPass           mBrightPassConfig;
	_cbUpdateCache          mUpdateCacheConfig;
    GlareStepArray          mSteps;
	bool                    mHDRGlare;
	bool                    mFullSizeBrightPass;
	bool                    mDownsampleBlurPrePass;

    // Data retained during execution.
    cgRenderTargetHandle    mOperationTarget;
    cgResampleChain       * mResampleChain0;
    cgResampleChain       * mResampleChain1;

	// ILR 
	cgVertexFormat        * mILRVertexFormat;
	cgSampler             * mDepthSampler;
	cgSampler             * mFlareSampler;
	cgTextureHandle         mDepthBuffer;
	cgTextureHandle         mFlareColor;
	ILRElementArray 		mILRElements;
	cgFloat					mILRBrightThreshold;
	cgFloat					mILRFlareContrast;
	cgInt32					mILRHighTargetPadding, mILRLowTargetPadding;
	cgBlurOpDesc            mILRLowFilter, mILRHighFilter;
	cgVector2				mILRDistanceRange;
	bool					mILRMaterialAlphaMask;
	bool					mHDRILR;

	_cbILRBrightPass		mILRBrightPassConfig;
	_cbILRCompositePass		mILRCompositePassConfig;
	cgConstantBufferHandle  mILRBrightPassConstants;
	cgConstantBufferHandle  mILRCompositePassConstants;

	// Anamorphic flares 
	cgFloat				    mAnamorphicIntensity;
	cgInt32				    mAnamorphicPasses;
	cgInt32                 mAnamorphicRadius;
	_cbAnamorphicFlare      mAnamorphicConfig;
};

#endif // !_CGE_CGGLAREPROCESSOR_H_