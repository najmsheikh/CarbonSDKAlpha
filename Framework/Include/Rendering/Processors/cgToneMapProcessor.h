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
// Name : cgToneMapProcessor.h                                               //
//                                                                           //
// Desc : Image processing class designed to apply tone mapping and other    //
//        luminance based adjustments to a rendered HDR image in order to    //
//        convert it into LDR.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGTONEMAPPROCESSOR_H_)
#define _CGE_CGTONEMAPPROCESSOR_H_

//-----------------------------------------------------------------------------
// cgToneMapProcessor Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgToneMapProcessor (Class)
/// <summary>
/// Image processing class designed to apply tone mapping and other luminance 
/// based adjustments to a rendered HDR image in order to convert it into LDR.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgToneMapProcessor : public cgImageProcessor
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgToneMapProcessor, cgImageProcessor, "ToneMapProcessor" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum ToneMapMethod
    {
        Photographic           = 0,
		PhotographicWhitePoint = 1,
        Filmic                 = 2,
		FilmicHable            = 3,
        Exponential            = 4,
		ExponentialWhitePoint  = 5
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgToneMapProcessor( );
    virtual ~cgToneMapProcessor( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void					enableLuminanceCache    ( bool enable );
    void					setToneMapMethod        ( ToneMapMethod method );
    void					setLuminanceSampleRate  ( cgFloat rate );
    void					setLuminanceRange       ( cgFloat minimum, cgFloat maximum );
    void					setLuminanceRange       ( const cgRangeF & range );
    void					setWhitePointAdjust     ( cgFloat scale, cgFloat bias );
    void					setKeyAdjust            ( cgFloat scale, cgFloat bias );
    void					setLuminanceAdaptation  ( cgFloat coneTime, cgFloat rodTime, cgFloat rodSensitivity );
    bool					execute                 ( cgRenderView * activeView, cgFloat timeDelta, const cgTextureHandle & source, const cgRenderTargetHandle & destination );
	cgInt32					getMethod               ( ){ return mMethod; }
	cgRenderTargetHandle &  getLuminanceData		( ){ return mLuminanceCurrTarget; } // ToDo: 6767 - Remove this AFTER we fix the render view target retrieval

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
    struct _cbToneMapping
    {
        cgFloat keyScale;
        cgFloat keyBias;
        cgFloat whiteScale;
        cgFloat whiteBias;
    };
    struct _cbLuminance
    {
        cgFloat coneTime;
        cgFloat rodTime;
        cgFloat rodSensitivity;
        cgFloat minimumLuminance;
        cgFloat maximumLuminance;
        cgVector4 luminanceTextureSize;
        cgVector4 cacheTextureSize;
    };

	struct _cbDownsample
	{
		cgVector4 luminanceTextureSize;
	};
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool            computeLuminance        ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
    bool            downSampleLuminance     ( );
    bool            updateLuminanceCache    ( );
    bool            adaptLuminance          ( );
    bool            toneMap                 ( const cgTextureHandle & source, const cgRenderTargetHandle & destination );
	bool			openLuminanceBuffers    ( cgRenderView * activeView );
	void			closeLuminanceBuffers   ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mToneMapShader;
    cgConstantBufferHandle  mToneMapConstants;
    cgConstantBufferHandle  mLuminanceConstants;
	cgConstantBufferHandle  mDownsampleConstants;

    cgRenderTargetHandle    mLuminanceCacheTarget;
    cgRenderTargetHandle    mLuminanceAvgTarget;
    cgRenderTargetHandle    mLuminanceCurrTarget;
    cgRenderTargetHandle    mLuminancePrevTarget;
	cgRenderTargetHandle    mLuminanceBuffer;

    cgSampler             * mLightingSampler;
    cgSampler             * mLightingPointSampler;
    cgSampler             * mLuminanceSampler;
    cgSampler             * mLuminanceCacheSampler;
    cgSampler             * mLuminancePrevSampler;
    cgSampler             * mLuminanceCurrSampler;
    cgSampler             * mLuminanceAvgSampler;
    cgResampleChain       * mLuminanceChain;
    cgInt                   mCurrentCacheFrame;
    cgFloat                 mAccumulatedTime;
    bool                    mAdaptationActive;
    
    // Configuration
    _cbToneMapping          mToneMapConfig;
    _cbLuminance            mLuminanceConfig;
    bool                    mUseLuminanceCache;
    ToneMapMethod           mMethod;
    cgFloat                 mLuminanceSamplingRate;
    
    // Data retained during execution.
};

#endif // !_CGE_CGTONEMAPPROCESSOR_H_