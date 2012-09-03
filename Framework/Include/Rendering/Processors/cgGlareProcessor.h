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

        // Constructors
        GlareStepDesc() {}
        GlareStepDesc( cgInt32 _levelIndex, cgFloat _levelIntensity, cgInt32 _blurPassCount, cgInt32 _blurPixelRadius, cgFloat _blurDistanceFactor ) :
            levelIndex( _levelIndex ), intensity( _levelIntensity ), blurOp( _blurPassCount, _blurPixelRadius, _blurDistanceFactor ) {}
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
    void            setGlareSteps           ( const GlareStepArray & steps );
    bool            execute                 ( const cgRenderTargetHandle & target, cgResampleChain * chain0, cgResampleChain * chain1, bool useCache );

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
    struct _cbUpSample
    {
        float  sourceIntensity;
        float  destinationIntensity;
    };

    struct _cbBrightPass
    {
        float     brightRangeScale;
        float     brightRangeBias;
        cgVector2 textureSizeRecip;
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            brightPass              ( );
    void            downsampleAndBlur       ( );
    void            updateCache             ( );
    void            composite               ( );
    size_t          getGlareStepIndex       ( size_t levelIndex );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle   mGlareShader;
    cgConstantBufferHandle  mBrightPassConstants;
    cgConstantBufferHandle  mUpSampleConstants;
    cgRenderTargetHandle    mGlarePrevious;         // Glare (previous frame)
    cgRenderTargetHandle    mGlareCurrent;          // Glare (current frame)
    cgSampler             * mPointSampler;
    cgSampler             * mLinearSampler;
    cgSampler             * mPreviousCacheSampler;
    
    // Configuration
    cgRangeF                mBrightThreshold;
    bool                    mHDREnabled;
    _cbBrightPass           mBrightPassConfig;
    GlareStepArray          mSteps;

    // Data retained during execution.
    cgRenderTargetHandle    mOperationTarget;
    cgResampleChain       * mResampleChain0;
    cgResampleChain       * mResampleChain1;
    bool                    mUseCache;
};

#endif // !_CGE_CGGLAREPROCESSOR_H_