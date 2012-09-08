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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
    bool            execute                 ( const cgRenderTargetHandle & target, cgResampleChain * chain0, cgResampleChain * chain1, float alphaMaskAmt );
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
        float blendAmount;
        float blendRate;
    };

	struct _cbAddLayer
	{
		float layerWeight;
	};

    struct _cbBrightPass
    {
        float     brightRangeScale;
        float     brightRangeBias;
		float     alphaMaskAmount;
        cgVector2 textureSizeRecip;
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            brightPass              ( );
    void            downsampleAndBlur       ( );
	void			updateCache				( size_t levelIndex );
    size_t          getGlareStepIndex       ( size_t levelIndex );
	void			computeAnamorphicFlares ( size_t levelIndex );

	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mGlareShader;
    cgConstantBufferHandle  mBrightPassConstants;
    cgConstantBufferHandle  mUpdateCacheConstants;
	cgConstantBufferHandle  mAddLayerConstants;
	cgBlendStateHandle      mLayerBlendState;

    cgSampler             * mPointSampler;
    cgSampler             * mLinearSampler;
	cgSampler             * mLuminanceSampler;
	cgToneMapProcessor    * mToneMapper;

    // Configuration
	cgFloat                 mGlareAmount;
	cgRangeF                mBrightThreshold;
    _cbBrightPass           mBrightPassConfig;
	_cbUpdateCache          mUpdateCacheConfig;
    GlareStepArray          mSteps;
	bool					mAnamorphicFlares;

    // Data retained during execution.
    cgRenderTargetHandle    mOperationTarget;
    cgResampleChain       * mResampleChain0;
    cgResampleChain       * mResampleChain1;
};

#endif // !_CGE_CGGLAREPROCESSOR_H_