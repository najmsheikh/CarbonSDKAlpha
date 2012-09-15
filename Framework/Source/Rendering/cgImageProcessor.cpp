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
// Name : cgImageProcessor.cpp                                               //
//                                                                           //
// Desc : TODO                                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgImageProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgImageProcessor.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgConstantBuffer.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgImageProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgImageProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgImageProcessor::cgImageProcessor(  )
{
    // Initialize variables to sensible defaults
    mDriver                   = CG_NULL;
    mProcessShader            = CG_NULL;
    mDefaultUserColor         = cgColorValue( 0, 0, 0, 0 );
    mApplyRasterizerState     = true;
    mApplyDepthStencilState   = true;
    mApplyBlendState          = true;
    mTestStencilBuffer        = false;
    mAlphaChannelOnly         = false;
    mForceLinearSampling      = false;
    mForcePointSampling       = false;
    mStencilRef               = 0;
    
    // Clear structures
    memset( &mSamplers, 0, sizeof(Samplers) );    
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgImageProcessor::~cgImageProcessor( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgImageProcessor::dispose( bool bDisposeBase )
{
    // Release allocated memory.
    if ( mSamplers.point )
        mSamplers.point->scriptSafeDispose();
    if ( mSamplers.linear )
        mSamplers.linear->scriptSafeDispose();
    if ( mSamplers.imageLinear )
        mSamplers.imageLinear->scriptSafeDispose();
    if ( mSamplers.grain )
        mSamplers.grain->scriptSafeDispose();
    if ( mSamplers.vignette )
        mSamplers.vignette->scriptSafeDispose();
    if ( mSamplers.remap )
        mSamplers.remap->scriptSafeDispose();
    if ( mSamplers.verticalPoint )
        mSamplers.verticalPoint->scriptSafeDispose();
    if ( mSamplers.verticalLinear )
        mSamplers.verticalLinear->scriptSafeDispose();
    if ( mSamplers.horizontalPoint )
        mSamplers.horizontalPoint->scriptSafeDispose();
    if ( mSamplers.horizontalLinear )
        mSamplers.horizontalLinear->scriptSafeDispose();
    if ( mSamplers.depthSrc )
        mSamplers.depthSrc->scriptSafeDispose();
    if ( mSamplers.normalSrc )
        mSamplers.normalSrc->scriptSafeDispose();
    if ( mSamplers.depthDest )
        mSamplers.depthDest->scriptSafeDispose();
    if ( mSamplers.normalDest )
        mSamplers.normalDest->scriptSafeDispose();
    if ( mSamplers.curr )
        mSamplers.curr->scriptSafeDispose();
    if ( mSamplers.prev )
        mSamplers.prev->scriptSafeDispose();
    if ( mSamplers.edge )
        mSamplers.edge->scriptSafeDispose();

    // Close resource handles.
    mMultiOpShaderCache.clear();
    mDepthStencilTarget.close();
    mVignetteTexture.close();
    mGrainTexture.close();
    mRemapTexture.close();
    mProcessShaderHandle.close();
    mImageProcessingConstants.close();
    mColorControlsConstants.close();
    mBilateralConstants.close();
    mReprojectionConstants.close();
    mDisabledDepthState.close();
    mGreaterDepthState.close();
    mLessEqualDepthState.close();
    mEqualDepthState.close();
    mWriteDepthState.close();
    mWriteStencilMaskState.close();
    mReadStencilMaskState.close();
    mMaskPass0DepthState.close();
    mMaskPass1DepthState.close();
    mMaskNearDepthState.close();
    mMaskFarDepthState.close();
    mDefaultRGBABlendState.close();
    mDefaultRGBBlendState.close();
    mDefaultRGBlendState.close();
    mDefaultBABlendState.close();
    mDefaultAlphaOnlyBlendState.close();
    mAdditiveRGBABlendState.close();
    mAdditiveRGBBlendState.close();
    mAlphaBlendState.close();
    mModulativeRGBABlendState.close();
    mModulativeRGBBlendState.close();
    mModulativeAlphaBlendState.close();
    mColorWriteDisabledBlendState.close();
    mAddRGBSetAlphaBlendState.close();
    mStandardAlphaBlendState.close();

    // Clear variables
    mDriver                   = CG_NULL;
    mProcessShader            = CG_NULL;
    mDefaultUserColor          = cgColorValue( 0, 0, 0, 0 );
    mApplyRasterizerState     = true;
    mApplyDepthStencilState   = true;
    mApplyBlendState          = true;
    mTestStencilBuffer        = false;
    mAlphaChannelOnly         = false;
    mForceLinearSampling      = false;
    mForcePointSampling       = false;
    mStencilRef               = 0;

    // Clear structures and containers
    memset( &mSamplers, 0, sizeof(Samplers) );
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgImageProcessor::initialize( cgRenderDriver * pDriver )
{
    // Store the render driver for later access
    mDriver = pDriver;

    // Create an instance of the image processing surface shader
    cgResourceManager * pResources = pDriver->getResourceManager();
    if ( !pResources->createSurfaceShader( &mProcessShaderHandle, _T("sys://Shaders/ImageProcessing.sh"), 0, cgDebugSource() ) )
        return false;
    mProcessShader = mProcessShaderHandle.getResource(true);

    ///////////////////////////////////////////////////////////////
    // Depth Stencil States
    ///////////////////////////////////////////////////////////////
    cgDepthStencilStateDesc dsStates;

    // Depth buffer disabled
    dsStates.depthEnable      = false;
    dsStates.depthWriteEnable = false;
    pResources->createDepthStencilState( &mDisabledDepthState, dsStates, 0, cgDebugSource() );

    // Depth write state (for hardware depth buffer output)
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = true;
    dsStates.depthFunction    = cgComparisonFunction::Always;
    pResources->createDepthStencilState( &mWriteDepthState, dsStates, 0, cgDebugSource() );

    // Depth buffer less equal test (for scene)
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = false;
    dsStates.depthFunction    = cgComparisonFunction::LessEqual;
    pResources->createDepthStencilState( &mLessEqualDepthState, dsStates, 0, cgDebugSource() );

    // Depth buffer equal test (for scene)
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = false;
    dsStates.depthFunction    = cgComparisonFunction::Equal;
    pResources->createDepthStencilState( &mEqualDepthState, dsStates, 0, cgDebugSource() );

    // Depth buffer greater test (for scene)
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = false;
    dsStates.depthFunction    = cgComparisonFunction::Greater;
    pResources->createDepthStencilState( &mGreaterDepthState, dsStates, 0, cgDebugSource() );

    // Stencil buffer mask write
    dsStates.depthEnable                  = false;
    dsStates.depthWriteEnable             = false;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 1;
    dsStates.stencilWriteMask             = 1;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace; 
    pResources->createDepthStencilState( &mWriteStencilMaskState, dsStates, 0, cgDebugSource() );

    // Stencil buffer mask read
    dsStates.depthEnable                  = false;
    dsStates.depthWriteEnable             = false;
    dsStates.stencilEnable                = true;
    dsStates.stencilWriteMask             = 0;
    dsStates.stencilReadMask              = 1;
    dsStates.backFace.stencilFunction         = cgComparisonFunction::Equal;
    dsStates.backFace.stencilPassOperation       = cgStencilOperation::Keep;
    dsStates.backFace.stencilFailOperation       = cgStencilOperation::Keep;
    dsStates.backFace.stencilDepthFailOperation  = cgStencilOperation::Keep;
    dsStates.frontFace                    = dsStates.backFace; // Duplicate to improve batching.
    pResources->createDepthStencilState( &mReadStencilMaskState, dsStates, 0, cgDebugSource() );

    //////////////////////////////////////////////////////
    // Screen Mask Pass 0 (Stencil Fill Only)
    dsStates.depthEnable                  = true;
    dsStates.depthWriteEnable             = false;
    dsStates.depthFunction                    = cgComparisonFunction::Greater;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 1;
    dsStates.stencilWriteMask             = 1;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace; 
    pResources->createDepthStencilState( &mMaskPass0DepthState, dsStates, 0, cgDebugSource() );

    // Screen Mask Pass 1
    dsStates.depthEnable                  = true;
    dsStates.depthWriteEnable             = false;
    dsStates.depthFunction                    = cgComparisonFunction::LessEqual;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 1;
    dsStates.stencilWriteMask             = 0;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Equal;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Keep;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace; 
    pResources->createDepthStencilState( &mMaskPass1DepthState, dsStates, 0, cgDebugSource() );
    //////////////////////////////////////////////////////

    // Screen Mask Near Pass
    dsStates.depthEnable                  = true;
    dsStates.depthWriteEnable             = false;
    dsStates.depthFunction                    = cgComparisonFunction::Greater;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 1;
    dsStates.stencilWriteMask             = 3;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace; 
    pResources->createDepthStencilState( &mMaskNearDepthState, dsStates, 0, cgDebugSource() );

    // Screen Mask Far Pass
    dsStates.depthEnable                  = true;
    dsStates.depthWriteEnable             = false;
    dsStates.depthFunction                    = cgComparisonFunction::Less;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 1;
    dsStates.stencilWriteMask             = 3;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace; 
    pResources->createDepthStencilState( &mMaskFarDepthState, dsStates, 0, cgDebugSource() );

    ///////////////////////////////////////////////////////////////
    // Blend States
    ///////////////////////////////////////////////////////////////
    cgBlendStateDesc blStates;

    // Standard blending
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
    blStates.renderTarget[0].blendEnable           = false;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::Zero;
    pResources->createBlendState( &mDefaultRGBABlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Red | cgColorChannel::Green | cgColorChannel::Blue;
    pResources->createBlendState( &mDefaultRGBBlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Alpha;
    pResources->createBlendState( &mDefaultAlphaOnlyBlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Blue | cgColorChannel::Alpha;
    pResources->createBlendState( &mDefaultBABlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Red | cgColorChannel::Green;
    pResources->createBlendState( &mDefaultRGBlendState, blStates, 0, cgDebugSource() );

    // Additive blending
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
    blStates.renderTarget[0].blendEnable           = true;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::One;
    pResources->createBlendState( &mAdditiveRGBABlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Red | cgColorChannel::Green | cgColorChannel::Blue;
    pResources->createBlendState( &mAdditiveRGBBlendState, blStates, 0, cgDebugSource() );

    // Standard alpha blend state
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::SrcAlpha;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::InvSrcAlpha;
    pResources->createBlendState( &mAlphaBlendState, blStates, 0, cgDebugSource() );

    // Modulative blending states
    blStates.renderTarget[0].blendEnable           = true;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::Zero;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::SrcColor;
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
    pResources->createBlendState( &mModulativeRGBABlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Red | cgColorChannel::Green | cgColorChannel::Blue;
    pResources->createBlendState( &mModulativeRGBBlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Alpha;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::Zero;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::SrcAlpha;
    pResources->createBlendState( &mModulativeAlphaBlendState, blStates, 0, cgDebugSource() );

    // Color write disabled state
    blStates.renderTarget[0].blendEnable           = false;
    blStates.renderTarget[0].renderTargetWriteMask = 0;
    pResources->createBlendState( &mColorWriteDisabledBlendState, blStates, 0, cgDebugSource() );

    // Add RGB, replace A
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
    blStates.renderTarget[0].blendEnable           = true;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::One;
    blStates.renderTarget[0].separateAlphaBlendEnable = true;
    blStates.renderTarget[0].sourceBlendAlpha      = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlendAlpha = cgBlendMode::Zero;
    pResources->createBlendState( &mAddRGBSetAlphaBlendState, blStates, 0, cgDebugSource() );

    // Traditional alpha blending 
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
    blStates.renderTarget[0].blendEnable           = true;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::SrcAlpha;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::InvSrcAlpha;
    blStates.renderTarget[0].separateAlphaBlendEnable = true;
    blStates.renderTarget[0].sourceBlendAlpha      = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlendAlpha = cgBlendMode::Zero;
    pResources->createBlendState( &mStandardAlphaBlendState, blStates, 0, cgDebugSource() );

    ///////////////////////////////////////////////////////////////
    // Rasterizer States
    ///////////////////////////////////////////////////////////////
    cgRasterizerStateDesc rsStates;

    ///////////////////////////////////////////////////////////////
    // Sampler States
    ///////////////////////////////////////////////////////////////

    // Create samplers
    mSamplers.point            = pResources->createSampler( _T("Image"), mProcessShaderHandle );
    mSamplers.linear           = pResources->createSampler( _T("Image"), mProcessShaderHandle );
    mSamplers.imageLinear      = pResources->createSampler( _T("ImageLinear"), mProcessShaderHandle );
    mSamplers.vignette         = pResources->createSampler( _T("Vignette"), mProcessShaderHandle );
    mSamplers.grain            = pResources->createSampler( _T("Grain"), mProcessShaderHandle );
    mSamplers.remap            = pResources->createSampler( _T("Remap"), mProcessShaderHandle );
    mSamplers.verticalPoint    = pResources->createSampler( _T("Vertical"), mProcessShaderHandle );
    mSamplers.verticalLinear   = pResources->createSampler( _T("Vertical"), mProcessShaderHandle );
    mSamplers.horizontalPoint  = pResources->createSampler( _T("Horizontal"), mProcessShaderHandle );
    mSamplers.horizontalLinear = pResources->createSampler( _T("Horizontal"), mProcessShaderHandle );
    mSamplers.depthSrc   	   = pResources->createSampler( _T("DepthSrc"), mProcessShaderHandle );
    mSamplers.depthDest        = pResources->createSampler( _T("DepthDst"), mProcessShaderHandle );
    mSamplers.normalSrc        = pResources->createSampler( _T("NormalSrc"), mProcessShaderHandle );
    mSamplers.normalDest       = pResources->createSampler( _T("NormalDst"), mProcessShaderHandle );
    mSamplers.edge             = pResources->createSampler( _T("Edge"), mProcessShaderHandle );
    mSamplers.curr             = pResources->createSampler( _T("ImageCurr"), mProcessShaderHandle );
    mSamplers.prev             = pResources->createSampler( _T("ImagePrev"), mProcessShaderHandle );

    // Point sampling (clamped)
    cgSamplerStateDesc smpStates;
    smpStates.minificationFilter = cgFilterMethod::Point;
    smpStates.magnificationFilter = cgFilterMethod::Point;
    smpStates.mipmapFilter = cgFilterMethod::None;
    smpStates.addressU = cgAddressingMode::Clamp;
    smpStates.addressV = cgAddressingMode::Clamp;
    smpStates.addressW = cgAddressingMode::Clamp;
    mSamplers.point->setStates( smpStates );
    mSamplers.verticalPoint->setStates( smpStates );
    mSamplers.horizontalPoint->setStates( smpStates );
    mSamplers.depthSrc->setStates( smpStates );
    mSamplers.depthDest->setStates( smpStates );
    mSamplers.normalSrc->setStates( smpStates );
    mSamplers.normalDest->setStates( smpStates );
    mSamplers.edge->setStates( smpStates );
    mSamplers.curr->setStates( smpStates );
    mSamplers.prev->setStates( smpStates );

    // Linear sampling (clamped)
    smpStates.minificationFilter = cgFilterMethod::Linear;
    smpStates.magnificationFilter = cgFilterMethod::Linear;		
    mSamplers.linear->setStates( smpStates );
    mSamplers.vignette->setStates( smpStates );
    mSamplers.remap->setStates( smpStates );
    mSamplers.verticalLinear->setStates( smpStates );
    mSamplers.horizontalLinear->setStates( smpStates );
    mSamplers.imageLinear->setStates( smpStates );

    // Linear sampling (wrapped)
    smpStates.addressU = cgAddressingMode::Wrap;
    smpStates.addressV = cgAddressingMode::Wrap;
    smpStates.addressW = cgAddressingMode::Wrap;
    mSamplers.grain->setStates( smpStates );

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !pResources->createConstantBuffer( &mImageProcessingConstants, mProcessShaderHandle, _T("cbImageProcessing"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mColorControlsConstants, mProcessShaderHandle, _T("cbColorControls"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mReprojectionConstants, mProcessShaderHandle, _T("cbReprojection"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mBilateralConstants, mProcessShaderHandle, _T("cbBilateral"), cgDebugSource() ) )
        return false;

    // Return success
    return true;
}

void cgImageProcessor::processColorImage( const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation )
{
    processColorImage( cgTextureHandle::Null, hDest, Operation, mDefaultUserColor );
}
void cgImageProcessor::processColorImage( const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation, const cgColorValue & c )
{
    processColorImage( cgTextureHandle::Null, hDest, Operation, c );
}
void cgImageProcessor::processColorImage( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation )
{
    processColorImage( hSrc, hDest, Operation, mDefaultUserColor );
}
void cgImageProcessor::processColorImage( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation, const cgColorValue & c )
{
    // Setup rasterizer state (default).
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set depth-stencil state
    if ( mApplyDepthStencilState )
    {
        if ( mTestStencilBuffer )
            mDriver->setDepthStencilState( mReadStencilMaskState, mStencilRef );
        else	
            mDriver->setDepthStencilState( mDisabledDepthState );
    
    } // End if apply depth states

    // Setup the blend state based on the operation we will perform
    if ( mApplyBlendState )
        setBlendState( Operation );

    // Bind textures and set sampler state
    if ( hSrc.isValid() )
    {
        if ( useBilinearSampling( hSrc, hDest, Operation ) )
            mSamplers.linear->apply( hSrc );
        else
            mSamplers.point->apply( hSrc );
    
    } // End if has texture

    if ( Operation == cgImageOperation::Vignette )
        mSamplers.vignette->apply( mVignetteTexture );
    else if ( Operation == cgImageOperation::Grain )
        mSamplers.grain->apply( mGrainTexture );
    else if( Operation == cgImageOperation::ColorRemap )
        mSamplers.remap->apply( mRemapTexture );

    // Set required shader constants
    setConstants( hSrc, Operation, c );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
         !mProcessShader->selectPixelShader( _T("drawImage"), (cgInt32)Operation ) )
        return;

    // Execute
    drawClipQuad( hDest );
}


void cgImageProcessor::processColorImageMulti( const cgRenderTargetHandle & hDest, const cgImageOperation::Array & aOperations )
{
    processColorImageMulti( cgTextureHandle::Null, hDest, aOperations, mDefaultUserColor );
}
void cgImageProcessor::processColorImageMulti( const cgRenderTargetHandle & hDest, const cgImageOperation::Array & aOperations, const cgColorValue & c )
{
    processColorImageMulti( cgTextureHandle::Null, hDest, aOperations, c );
}
void cgImageProcessor::processColorImageMulti( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, const cgImageOperation::Array & aOperations )
{
    processColorImageMulti( hSrc, hDest, aOperations, mDefaultUserColor );
}
void cgImageProcessor::processColorImageMulti( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, const cgImageOperation::Array & aOperations, const cgColorValue & c )
{
    // Setup depth-stencil state (depth always disabled for image processing)
    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Bind main texture and set sampler state
    if ( hSrc.isValid() )
    {
        if ( useBilinearSampling( hSrc, hDest, cgImageOperation::None ) )
            mSamplers.linear->apply( hSrc );
        else
            mSamplers.point->apply( hSrc );

    } // End if has source

    // Apply required samplers based on the requested operations
    for ( size_t i = 0; i < aOperations.size(); ++i )
    {
        cgImageOperation::Base operation = aOperations[i];
        if ( operation == cgImageOperation::Vignette )
            mSamplers.vignette->apply( mVignetteTexture );
        else if ( operation == cgImageOperation::Grain )
            mSamplers.grain->apply( mGrainTexture );
        else if ( operation == cgImageOperation::ColorRemap )
            mSamplers.remap->apply( mRemapTexture );

    } // Next operation

    // Set the blend state for the last operation we will perform
    if ( mApplyBlendState )
        setBlendState( aOperations.back() );

    // Set required shader constants (for batched ops, we will force all constants to be set)
    setConstants( hSrc, cgImageOperation::All, c );

    // No vertex shader needed
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) )
        return;

    // Is there a matching multi-operation shader in the cache?
    MultiOpShaderCacheKey key( aOperations );
    MultiOpShaderCache::iterator itEntry = mMultiOpShaderCache.find( key );
    if ( itEntry != mMultiOpShaderCache.end() )
    {
        // Cached already, select.
        if ( !mProcessShader->selectPixelShader( itEntry->second ) )
            return;

    } // End if entry exists
    else
    {
        // Build a new shader
        cgScriptArgument::Array aArgs(1);
        aArgs[0] = cgScriptArgument( cgScriptArgumentType::Array, cgScriptArgumentType::DWord, _T("int[]"), &aOperations.front() );
        cgPixelShaderHandle hShader = mProcessShader->getPixelShader( _T("drawImageMultiOp"), aArgs );
        if ( !hShader.isValid() || !mProcessShader->selectPixelShader( hShader ) )
            return;

        // Store in cache
        mMultiOpShaderCache[key] = hShader;

    } // End if no entry exists

    // Execute
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name : processDepthImage()
/// <summary>
/// Entry point for depth image processing. Can read from a packed source 
/// texture and output to a packed destination texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::processDepthImage( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgDepthType::Base InputType, cgDepthType::Base OutputType )
{
    processDepthImage( hSrc, hDest, InputType, OutputType, cgDepthStencilTargetHandle() );
}
void cgImageProcessor::processDepthImage( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgDepthType::Base InputType, cgDepthType::Base OutputType, const cgDepthStencilTargetHandle & hDepthBuffer )
{
    // Are we outputting hardware depth as well?
    bool bOutputDepth = hDepthBuffer.isValid();

    // Set necessary states
    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( bOutputDepth ? mWriteDepthState : mDisabledDepthState );

    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Bind textures and set sampler state
    mSamplers.point->apply( hSrc );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), true ) ||
         !mProcessShader->selectPixelShader( _T("drawDepthImage"), (cgInt32)InputType, (cgInt32)OutputType, bOutputDepth ) )
        return;

    // Execute
    if ( mDriver->beginTargetRender( hDest, hDepthBuffer ) )
    {
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

    } // End if beginTargetRender
}

//-----------------------------------------------------------------------------
// Name : downSample()
/// <summary>
/// Perform a down sampling operation on the specified target.
/// Note: A valid src and dst must both be provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::downSample( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest )
{
    downSample( hSrc, hDest, cgImageOperation::DownSampleAverage, false, false, false ); 
}	
void cgImageProcessor::downSample( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation )
{
    downSample( hSrc, hDest, Operation, false, false, false ); 
}	
void cgImageProcessor::downSample( cgResampleChain * pChain )
{
    downSample( pChain, cgImageOperation::DownSampleAverage, false, false, false );
}
void cgImageProcessor::downSample( cgResampleChain * pChain, cgImageOperation::Base Operation )
{
    downSample( pChain, Operation, false, false, false );
}	
void cgImageProcessor::downSample( cgResampleChain * pChain, cgImageOperation::Base Operation, bool bAlphaWeighted, bool bAlphaWeightedBinary, bool bAlphaBinaryOutput )
{
    cgInt nNumLevels = (cgInt)pChain->getLevelCount();
    for ( cgInt i = 0; i < (nNumLevels - 1); ++i )
        downSample( pChain->getLevel( i ), pChain->getLevel( (i + 1) ), Operation, bAlphaWeighted, bAlphaWeightedBinary, bAlphaBinaryOutput );
}
void cgImageProcessor::downSample( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation, bool bAlphaWeighted, bool bAlphaWeightedBinary, bool bAlphaBinaryOutput )
{
    // Compute the required kernel size
    const cgTexture * pSrcTexture = hSrc.getResourceSilent();
    const cgRenderTarget * pDestTexture = hDest.getResourceSilent();
    cgInt32 nSrcWidth   = pSrcTexture->getSize().width;
	cgInt32 nSrcHeight  = pSrcTexture->getSize().height;
    cgInt32 nDstWidth   = pDestTexture->getSize().width;
    cgInt32 nKernelSize = 2; //srcWidth / dstWidth; // ToDo: revisit this support at some point

	// For an odd sized source image, we will force 3x3 downsampling
	bool oddU = (nSrcWidth  % 2) != 0;
	bool oddV = (nSrcHeight % 2) != 0;

    // Set necessary states
    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set core shader constants
    setConstants( hSrc, cgImageOperation::None, mDefaultUserColor );

    // Can we use bilinear sampling?
    bool bBilinear = useBilinearSampling( hSrc, hDest, Operation );
    if ( bBilinear && !mForceLinearSampling )
        bBilinear = !bAlphaWeighted && !bAlphaWeightedBinary;

	// Disallow linear filtering when an odd kernel is used (for now at least)
	if ( oddU || oddV )
		bBilinear = false;

    // Bind textures and set sampler state
    if ( bBilinear )
        mSamplers.linear->apply( hSrc );
    else
        mSamplers.point->apply( hSrc );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("downSample"), nKernelSize, (cgInt32)Operation, bBilinear, oddU, oddV, bAlphaWeighted, bAlphaWeightedBinary, bAlphaBinaryOutput ) )
        return;

    // Execute
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: downSampleDepth()
/// <summary>
/// Runs a down sampling operation on a depth texture. Can simultaneously 
/// downsample a normal texture as well based on the results of the depth
/// operation. 
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::downSampleDepth( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation )
{
    cgDepthType::Base SystemDepthType = (cgDepthType::Base)mDriver->getSystemState( cgSystemState::DepthType );
    downSampleDepth( hSrc, cgTextureHandle::Null, hDest, cgRenderTargetHandle::Null, SystemDepthType, SystemDepthType, Operation, false );
}    
void cgImageProcessor::downSampleDepth( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgImageOperation::Base Operation, bool bRecordOffset )
{
    cgDepthType::Base SystemDepthType = (cgDepthType::Base)mDriver->getSystemState( cgSystemState::DepthType );
    downSampleDepth( hSrc, cgTextureHandle::Null, hDest, cgRenderTargetHandle::Null, SystemDepthType, SystemDepthType, Operation, bRecordOffset );
}
void cgImageProcessor::downSampleDepth( const cgTextureHandle & hSrcDepth, const cgTextureHandle & hSrcData, 
                                        const cgRenderTargetHandle & hDestDepth, const cgRenderTargetHandle & hDestData,
                                        cgDepthType::Base InputType, cgDepthType::Base OutputType, cgImageOperation::Base Operation, 
                                        bool bRecordOffset )
{
    // Are we downsampling a second target (e.g., normals) as well?
    bool bDownsampleData = hSrcData.isValid() && hDestData.isValid();

    // Get the texture image information
    const cgTexture * pSrcTexture = hSrcDepth.getResourceSilent();
    const cgRenderTarget * pDestTexture = hDestDepth.getResourceSilent();
    const cgImageInfo & SrcInfo = pSrcTexture->getInfo();
    const cgImageInfo & DestInfo = pDestTexture->getInfo();

    // Compute the required kernel size
    cgInt32 nKernelSize = 2; // SrcInfo.width / DestInfo.width; // ToDo: revisit this support at some point

	// For an odd sized source image, we will force 3x3 downsampling
	if ( SrcInfo.width % 2 != 0 || SrcInfo.height % 2 != 0 )
		nKernelSize = 3;

    // Set necessary states
    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set core shader constants
    setConstants( hSrcDepth, cgImageOperation::None, mDefaultUserColor );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), true ) ||
         !mProcessShader->selectPixelShader( _T("downSampleDepth"), (cgInt32)InputType, (cgInt32)OutputType, nKernelSize, (cgInt32)Operation, false, bDownsampleData ) )
        return;

    // Setup inputs and outputs
    mSamplers.depthSrc->apply( hSrcDepth );

    // Setup output render targets.
    cgRenderTargetHandleArray aTargets(1);
    aTargets[ 0 ] = hDestDepth;
    if ( bDownsampleData )
    {
        if ( Operation == cgImageOperation::DownSampleAverage )
            mSamplers.linear->apply( hSrcData );
        else
            mSamplers.point->apply( hSrcData );
        aTargets.push_back( hDestData );
    
    } // End if downsample data

    // Run the operation
    if ( mDriver->beginTargetRender( aTargets, mDepthStencilTarget ) )
    {
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

    } // End if begun
}

//-----------------------------------------------------------------------------
// Name: blur()
/// <summary>
/// Runs M passes of NxN box blur filter. 
/// Note: if distanceFactor > 0 -> sigma value for gaussian weighting
///       if distanceFactor = 0 -> all samples equally weighted
///       if distanceFactor < 0 -> samples are weighted as 
///         pow( 1 / (pixel index + 1), abs(distanceFactor) )
/// Note: Recommended that bilinear sampling be disabled when alpha weighting 
///       is on.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgInt32 nNumPasses )
{
    blur( hSrc, hScratch, cgTextureHandle::Null, nPixelRadius, fDistanceFactor, 0.0, nNumPasses, cgAlphaWeightMethod::None, cgAlphaWeightMethod::None );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgInt32 nNumPasses, cgAlphaWeightMethod::Base InputAlpha, cgAlphaWeightMethod::Base OutputAlpha )
{
    blur( hSrc, hScratch, cgTextureHandle::Null, nPixelRadius, fDistanceFactor, 0.0, nNumPasses, InputAlpha, OutputAlpha );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgFloat fWorldRadius, cgInt32 nNumPasses )
{
    blur( hSrc, hScratch, hDepth, nPixelRadius, fDistanceFactor, fWorldRadius, nNumPasses, cgAlphaWeightMethod::None, cgAlphaWeightMethod::None );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgFloat fWorldRadius, cgInt32 nNumPasses, cgAlphaWeightMethod::Base InputAlpha, cgAlphaWeightMethod::Base OutputAlpha )
{
    cgBlurOpDesc::Array aData(1);
    aData[0].pixelRadiusV    = nPixelRadius;
	aData[0].pixelRadiusH    = nPixelRadius;
    aData[0].distanceFactorV = fDistanceFactor;
	aData[0].distanceFactorH = fDistanceFactor;
    aData[0].worldRadius     = fWorldRadius;
    aData[0].passCount       = nNumPasses;
    aData[0].inputAlpha      = InputAlpha;
    aData[0].outputAlpha     = OutputAlpha;
    blur( hSrc, hScratch, hDepth, aData );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgBlurOpDesc & Data )
{
    cgBlurOpDesc::Array aData(1,Data);
    blur( hSrc, hScratch, cgTextureHandle::Null, aData );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgBlurOpDesc::Array & aData )
{
    blur( hSrc, hScratch, cgTextureHandle::Null, aData );
}
void cgImageProcessor::blur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgBlurOpDesc::Array & Data )
{
    // Set rasterizer state
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set depth-stencil state
    if ( mApplyDepthStencilState )
    {
        if ( mTestStencilBuffer )
            mDriver->setDepthStencilState( mReadStencilMaskState, mStencilRef );
        else	
            mDriver->setDepthStencilState( mDisabledDepthState );
    }

    // Set blend state
    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    // Can we use bilinear sampling?
    bool bBilinear = useBilinearSampling( hScratch, hSrc, cgImageOperation::Blur );

    // Set core shader constants
    setConstants( hSrc, cgImageOperation::None, mDefaultUserColor );

    // Select vertex shader
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) )
        return;

    // For each level of blur 
    for ( size_t j = 0; j < Data.size(); ++j )
    {
        const cgBlurOpDesc & Info = Data[ j ];
        bool bPassBilinear = bBilinear;

        // Pre-bind both textures and set sampler states once
        if ( bPassBilinear && (mForceLinearSampling || Info.inputAlpha == cgAlphaWeightMethod::None) )
        {
            bPassBilinear = true;
            mSamplers.verticalLinear->apply( hSrc );
            mSamplers.horizontalLinear->apply( hScratch );
        
        } // End if linear
        else
        {
            bPassBilinear = false;
            mSamplers.verticalPoint->apply( hSrc );
            mSamplers.horizontalPoint->apply( hScratch );
        
        } // End if point

        // Retrieve the two pixel shaders we will use (avoids permutation search per pass)
        cgPixelShaderHandle hVerticalShader   = mProcessShader->getPixelShader( _T("blur"), Info.pixelRadiusV, Info.distanceFactorV, true,  bPassBilinear, Info.inputAlpha, Info.outputAlpha );
        cgPixelShaderHandle hHorizontalShader = mProcessShader->getPixelShader( _T("blur"), Info.pixelRadiusH, Info.distanceFactorH, false, bPassBilinear, Info.inputAlpha, Info.outputAlpha );
        if ( !hVerticalShader.isValid() || !hHorizontalShader.isValid() )
            return;

        // Run the specified number of blur passes using ping-ponging
        for ( cgInt32 i = 0; i < Info.passCount; ++i )
        {
            // Run the vertical blur
            if ( !mProcessShader->selectPixelShader( hVerticalShader ) )
                break;
            drawClipQuad( hScratch );	

            // Run the horizontal blur
            if ( !mProcessShader->selectPixelShader( hHorizontalShader ) )
                break;
            drawClipQuad( hSrc );
        
        } // Next pass
    
    } // Next operation
}

//-----------------------------------------------------------------------------
// Name: bilateralBlur()
/// <summary>
/// Runs M passes of NxN box or gaussian blur filter. For box filter, just
/// provide a distanceFactor value of 0.
/// Note: Valid src and scratch must both be provided and the sizes must match.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::bilateralBlur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgTextureHandle & hNormal,
                                      cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgInt32 nNumPasses, bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthType )
{
    bilateralBlur( hSrc, hScratch, hDepth, hNormal, nPixelRadius, fDistanceFactor, 0, nNumPasses, bCompareDepth, bCompareNormal, DepthType, false, false, 0.0, 0.0 );
}

void cgImageProcessor::bilateralBlur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgTextureHandle & hNormal,
                                      cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgInt32 nNumPasses, bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthType, bool bDepthAlphaAsColor )
{
    bilateralBlur( hSrc, hScratch, hDepth, hNormal, nPixelRadius, fDistanceFactor, 0, nNumPasses, bCompareDepth, bCompareNormal, DepthType, bDepthAlphaAsColor, false, 0.0, 0.0 );
}
void cgImageProcessor::bilateralBlur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgTextureHandle & hNormal,
                                      cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgFloat fKernelAdjust, cgInt32 nNumPasses, bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthType )
{
    bilateralBlur( hSrc, hScratch, hDepth, hNormal, nPixelRadius, fDistanceFactor, fKernelAdjust, nNumPasses, bCompareDepth, bCompareNormal, DepthType, false, false, 0.0, 0.0 );
}
void cgImageProcessor::bilateralBlur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgTextureHandle & hNormal,
                                      cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgFloat fKernelAdjust, cgInt32 nNumPasses, bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthType, bool bDepthAlphaAsColor )
{
    bilateralBlur( hSrc, hScratch, hDepth, hNormal, nPixelRadius, fDistanceFactor, fKernelAdjust, nNumPasses, bCompareDepth, bCompareNormal, DepthType, bDepthAlphaAsColor, false, 0.0, 0.0 );
}
void cgImageProcessor::bilateralBlur( const cgRenderTargetHandle & hSrc, const cgRenderTargetHandle & hScratch, const cgTextureHandle & hDepth, const cgTextureHandle & hNormal,
                                      cgInt32 nPixelRadius, cgFloat fDistanceFactor, cgFloat fKernelAdjust, cgInt32 nNumPasses, bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthType, bool bDepthAlphaAsColor, 
                                      bool bAlphaWeighted, cgFloat fWorldRadius, cgFloat fAnisotropy )
{		
    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set depth-stencil state
    if ( mApplyDepthStencilState )
    {
        if ( mTestStencilBuffer )
            mDriver->setDepthStencilState( mReadStencilMaskState, mStencilRef );
        else	
            mDriver->setDepthStencilState( mDisabledDepthState );
    }

    // Apply blend state
    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    // Pre-bind textures and set sampler states once
    mSamplers.verticalPoint->apply( hSrc );
    mSamplers.horizontalPoint->apply( hScratch );
    if ( bCompareDepth || bDepthAlphaAsColor || fKernelAdjust > 0 )
        mSamplers.depthSrc->apply( hDepth );
    if ( bCompareNormal )
        mSamplers.normalSrc->apply( hNormal );

    // Select vertex shader
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) )
        return;

    // Set shader constants
    setConstants( hSrc, cgImageOperation::BilateralBlur, mDefaultUserColor );

    // Retrieve the two pixel shaders we will use (avoids permutation search per pass)
    cgPixelShaderHandle hVerticalShader   = mProcessShader->getPixelShader( _T("bilateralBlur"), nPixelRadius, fDistanceFactor, fKernelAdjust, true,  bCompareDepth, bCompareNormal, (cgInt32)DepthType, bDepthAlphaAsColor, bAlphaWeighted, fWorldRadius, fAnisotropy );
    cgPixelShaderHandle hHorizontalShader = mProcessShader->getPixelShader( _T("bilateralBlur"), nPixelRadius, fDistanceFactor, fKernelAdjust, false, bCompareDepth, bCompareNormal, (cgInt32)DepthType, bDepthAlphaAsColor, bAlphaWeighted, fWorldRadius, fAnisotropy );
    if ( !hVerticalShader.isValid() || !hHorizontalShader.isValid() )
        return;

    // Run the specified number of blur passes using ping-ponging
    for ( cgInt32 i = 0; i < nNumPasses; i++ )
    {
        // Run the vertical blur
        if ( !mProcessShader->selectPixelShader( hVerticalShader ) )
            break;
        drawClipQuad( hScratch );	

        // Run the horizontal blur
        if ( !mProcessShader->selectPixelShader( hHorizontalShader ) )
            break;
        drawClipQuad( hSrc );
    
    } // Next pass
}

//-----------------------------------------------------------------------------
// Name: resample()
/// <summary>
/// Runs a resampling pass using optional depth/normals for weighting.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::resample( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest )
{
    resample( hSrc, cgTextureHandle::Null, cgTextureHandle::Null, hDest, cgTextureHandle::Null, cgTextureHandle::Null, 
              false, false, cgDepthType::None, cgDepthType::None, false, false, false );
}
void cgImageProcessor::resample( const cgTextureHandle & hSrc, const cgTextureHandle & hSrcDepth, const cgTextureHandle & hSrcNormal,
                                 const cgRenderTargetHandle & hDest, const cgTextureHandle & hDestDepth, const cgTextureHandle & hDestNormal,
                                 bool bCompareDepth, bool bCompareNormal, cgDepthType::Base DepthTypeSrc, cgDepthType::Base DepthTypeDest, 
                                 bool bDepthAlphaAsColor, bool bClipFailedPixels, bool bTestEdges )
{
    // Set core shader constants
    setConstants( hSrc, cgImageOperation::BilateralResample, mDefaultUserColor );

    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set depth-stencil state
    if ( mApplyDepthStencilState )
    {
        if ( mTestStencilBuffer )
            mDriver->setDepthStencilState( mReadStencilMaskState, mStencilRef );
        else	
            mDriver->setDepthStencilState( mDisabledDepthState );
    }

    // If the depth buffer alpha is our color buffer, only write to alpha channel
    if ( mApplyBlendState )
    {
        if ( bDepthAlphaAsColor )
            mDriver->setBlendState( mDefaultAlphaOnlyBlendState );
        else	
            mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );
    }

    // Setup samplers
    if( bTestEdges && hDestNormal.isValid() )
    {
        // Edge tests require a point and linear sampler for the texture we are resampling
        mSamplers.point->apply( hSrc );
        mSamplers.imageLinear->apply( hSrc );
    
    } // End if test edges
    else
    {
        bTestEdges = false;

        // If we are not doing a bilateral filter, do a hardware bilinear resample
        if ( !bCompareDepth && !bCompareNormal )
            mSamplers.linear->apply( hSrc );
        else				
            mSamplers.point->apply( hSrc );
    
    } // End if !test edges

    // Bind depth and normal buffers
    if ( bCompareDepth || bDepthAlphaAsColor )
    {
        mSamplers.depthSrc->apply( hSrcDepth );
        mSamplers.depthDest->apply( hDestDepth );
    }
    if ( bCompareNormal )
    {		
        mSamplers.normalSrc->apply( hSrcNormal );
        mSamplers.normalDest->apply( hDestNormal );
    }

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("resample"), bCompareDepth, bCompareNormal, (cgInt32)DepthTypeDest, (cgInt32)DepthTypeSrc, bDepthAlphaAsColor, bClipFailedPixels, bTestEdges ) )
        return;

    // Draw a full screen quad
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: discontinuityDetect()
/// <summary>
/// Runs a geometric discontinuity detection filter to find edges in the image.
/// ToDo: We should probably deal with geometry only (depth Greater test) here 
/// and do a separate pass on background (sky) pixels to identify where they 
/// meet up with the geometric edges identified during the initial pass.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::discontinuityDetect( const cgTextureHandle & hSrcDepth, const cgTextureHandle & hSrcNormal, const cgRenderTargetHandle & hDest, cgDiscontinuityTestMethod::Base Method, bool bPreserveDirection, cgDepthType::Base DepthType )
{
    // Choose pixel shader and select the appropriate testing parameters
    cgString strShaderName;
    bool bCompareDepth = false;
    bool bCompareNormal = false;
    switch( Method )
    {
        case cgDiscontinuityTestMethod::Depth:
            strShaderName = _T("discontinuityDetect");
            bCompareDepth = true;
            break;
        case cgDiscontinuityTestMethod::Normal:
            strShaderName = _T("discontinuityDetect");
            bCompareNormal = true;			
            break;
        case cgDiscontinuityTestMethod::Plane:
            strShaderName = _T("discontinuityDetectPlane");
            bCompareDepth = true;
            break;
        case cgDiscontinuityTestMethod::PlaneAndNormal:
            strShaderName = _T("discontinuityDetectPlane");
            bCompareDepth = true;
            bCompareNormal = true;			
            break;
        case cgDiscontinuityTestMethod::DepthFast:
            strShaderName = _T("discontinuityDetectFast");
            bCompareDepth = true;
            break;
        case cgDiscontinuityTestMethod::NormalFast:
            strShaderName = _T("discontinuityDetectFast");
            bCompareNormal = true;			
            break;
        case cgDiscontinuityTestMethod::DepthAndNormalFast:
            strShaderName = _T("discontinuityDetectFast");
            bCompareDepth = true;
            bCompareNormal = true;			
            break;
        default:
            strShaderName = _T("discontinuityDetect");
            bCompareDepth = true;
            bCompareNormal = true;			
            break;
    }

    // Set core shader constants
    setConstants( bCompareDepth ? hSrcDepth : hSrcNormal, cgImageOperation::BilateralResample, mDefaultUserColor );

    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set depth-stencil state
    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );
    //mDriver.setDepthStencilState( mGreaterDepthState ); //See ToDo above!

    // If the depth buffer alpha is our color buffer, only write to alpha channel
    if ( mApplyBlendState )
        mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );

    // Bind textures and set sampler state
    if ( bCompareDepth )
        mSamplers.depthSrc->apply( hSrcDepth );

    if ( bCompareNormal )
        mSamplers.normalSrc->apply( hSrcNormal );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( strShaderName, bCompareDepth, bCompareNormal, bPreserveDirection, (cgInt32)DepthType ) )
        return;

    // Draw a full screen quad
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: packDepthNormal()
/// <summary>
/// Packs depth and normal into the same texture
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::packDepthNormal( const cgTextureHandle & hSrcDepth, const cgRenderTargetHandle & hDestDepth, bool bDepthToNormalTarget )
{
    packDepthNormal( hSrcDepth, cgTextureHandle::Null, hDestDepth, bDepthToNormalTarget );
}
void cgImageProcessor::packDepthNormal( const cgTextureHandle & hSrcDepth, const cgTextureHandle & hSrcNormal, const cgRenderTargetHandle & hDest, bool bDepthToNormalTarget )
{
    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    if ( mApplyBlendState )
    {
        mDriver->setBlendState( mDefaultRGBABlendState );
        if ( bDepthToNormalTarget )
            mDriver->setBlendState( mDefaultBABlendState );
        else	
            mDriver->setBlendState( mDefaultRGBABlendState );
    }

    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    // Bind textures and set sampler state
    if ( hSrcDepth.isValid() )
        mSamplers.depthSrc->apply( hSrcDepth );

    if( hSrcNormal.isValid() )
        mSamplers.normalSrc->apply( hSrcNormal );

    // Set core shader constants
    setConstants( hSrcDepth, cgImageOperation::None, mDefaultUserColor );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("packDepthNormal"), bDepthToNormalTarget ) )
        return;

    // Draw a clip quad
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: computeScreenMask()
/// <summary>
/// Applies a screen mask to the stencil buffer
/// Note: Valid targets must be provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::computeScreenMask( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest )
{
    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    if ( mApplyBlendState )
        mDriver->setBlendState( mDefaultRGBABlendState );

    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    // Bind textures and set sampler state
    mSamplers.point->apply( hSrc );

    // Set core shader constants
    setConstants( hSrc, cgImageOperation::None, mDefaultUserColor );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("computeScreenMask") ) )
        return;

    // Draw a clip quad
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: downSampleScreenMask()
/// <summary>
/// Downsamples (dilates) the screen mask
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::downSampleScreenMask( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest )
{
    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    if ( mApplyBlendState )
        mDriver->setBlendState( mDefaultRGBABlendState );

    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    // Bind textures and set sampler state
    mSamplers.linear->apply( hSrc );

    // Set core shader constants
    setConstants( hSrc, cgImageOperation::None, mDefaultUserColor );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("downSampleScreenMask") ) )
        return;

    // Draw a clip quad
    drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: resolveScreenMask()
/// <summary>
/// Resolves the screen mask (1 if any channel > 0, else 0)
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::resolveScreenMask( const cgTextureHandle & hSrc )
{
    resolveScreenMask( hSrc, cgRenderTargetHandle::Null, -1.0f, false );
}
void cgImageProcessor::resolveScreenMask( const cgTextureHandle & hSrc, bool bStencilOnly )
{
    resolveScreenMask( hSrc, cgRenderTargetHandle::Null, -1.0f, bStencilOnly );
}
void cgImageProcessor::resolveScreenMask( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest )
{
    resolveScreenMask( hSrc, hDest, -1.0f, false );
}
void cgImageProcessor::resolveScreenMask( const cgTextureHandle & hSrc, cgFloat z )
{
    resolveScreenMask( hSrc, cgRenderTargetHandle::Null, z, false );
}
void cgImageProcessor::resolveScreenMask( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgFloat z, bool bStencilOnly )
{
    // Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Are we doing a stencil only fill?        
    if ( mApplyBlendState )
    {
        if ( bStencilOnly )
            mDriver->setBlendState( mColorWriteDisabledBlendState );
        else
            mDriver->setBlendState( mAlphaChannelOnly ? mDefaultAlphaOnlyBlendState : mDefaultRGBABlendState );
    }

    if ( mApplyDepthStencilState )
    {
        if ( bStencilOnly )
            mDriver->setDepthStencilState( mWriteStencilMaskState, 1 );
        else
            mDriver->setDepthStencilState( z < 0.0f ? mDisabledDepthState : mGreaterDepthState );
    }

    // Bind textures and set sampler state
    if ( mForcePointSampling )
        mSamplers.point->apply( hSrc );
    else
        mSamplers.linear->apply( hSrc );

    // Set core shader constants
    setConstants( hSrc, cgImageOperation::None, mDefaultUserColor );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( _T("resolveScreenMask"), bStencilOnly ) )
        return;

    // Draw a screen quad
    if ( z > 0.0f )
        drawClipQuad( hDest, z );	
    else
        drawClipQuad( hDest );
}

//-----------------------------------------------------------------------------
// Name: blendFrames()
/// <summary>
/// TOO: 6767
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::blendFrames( const cgTextureHandle & hCurrentColor, const cgTextureHandle & hPrevColor,
                                    const cgTextureHandle & hCurrentDepth, const cgTextureHandle & hPrevDepth, 
                                    const cgRenderTargetHandle & hDest, const cgMatrix & mtxReprojection, cgFloat fDepthTolerance, cgFloat fBlendAmount )
{
    /*// Set necessary states
    if ( mApplyRasterizerState )
        mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    if ( mApplyDepthStencilState )
        mDriver->setDepthStencilState( mDisabledDepthState );

    if ( mApplyBlendState )
        mDriver.setBlendState( null );

    // Bind textures and set sampler state
    mCurrSampler.apply( currColor );
    mPrevSampler.apply( prevColor );
    mDepthSrcSampler.apply( prevDepth );
    mDepthDstSampler.apply( currDepth );

    // Set core shader constants
    setConstants( hPrevColor, cgImageOperation::None, mDefaultUserColor );

    // Set reprojection constants
    ConstantBuffer@ buffer = mReprojectionConstants.getResource(true);
    buffer.setMatrix( "reprojectionMatrix", reprojectionMatrix );
    buffer.setFloat( "depthTolerance", depthTolerance );
    buffer.setFloat( "blendAmount", blendAmount );
    mDriver.setConstantBufferAuto( mReprojectionConstants );

    // Select shaders
    if ( !mProcessShader->selectVertexShader( _T("transform"), false ) ||
        !mProcessShader->selectPixelShader( "blendFrames" ) )
        return;

    // Draw a clip quad
    drawClipQuad( dest );	*/
}

//-----------------------------------------------------------------------------
// Name: setConstants()
/// <summary>
/// Sets shader constant data
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::setConstants( const cgTextureHandle & hSrcImage, cgInt32 nOperation, const cgColorValue & c )
{
    // Compute source image dimensions (including reciprocal)
    cgVector4 vSize( 0, 0, 0, 0 );
    if ( hSrcImage.isValid() )
    {
        const cgTexture * pTexture = hSrcImage.getResourceSilent();
        vSize.x = cgFloat(pTexture->getSize().width);
        vSize.y = cgFloat(pTexture->getSize().height);
        vSize.z = 1.0f / vSize.x;
        vSize.w = 1.0f / vSize.y;
    
    } // End if valid

    // Always set the core constants
    cgConstantBuffer * pBuffer = mImageProcessingConstants.getResource(true);
    pBuffer->setVector( _T("userColor"), c );
    pBuffer->setVector( _T("textureSize"), vSize );
    mDriver->setConstantBufferAuto( mImageProcessingConstants );

    // Should we set the color controls as well?
    if ( nOperation == cgImageOperation::All || (nOperation >= cgImageOperation::ColorControlsStart && nOperation <= cgImageOperation::ColorControlsEnd) )
    {
        /*cgFloat inRange   = 1.0f / (whiteLevel.x - blackLevel.x);
        cgFloat outRange  =  whiteLevel.y - blackLevel.y;
        cgFloat minAdjust = -blackLevel.x * inRange;

        pBuffer = mColorControlsConstants->getResource(true);
        buffer.setVector( "blackLevel", blackLevel );
        buffer.setVector( "whiteLevel", whiteLevel );
        buffer.setFloat( "inRange", inRange );
        buffer.setFloat( "outRange", outRange );
        buffer.setFloat( "minAdjust", minAdjust );
        buffer.setFloat( "gamma", gamma );
        buffer.setVector( "tint", tint );
        buffer.setFloat( "brightness", brightness );
        buffer.setFloat( "exposure", exposure );
        buffer.setFloat( "exposureAmount", exposureAmount );
        buffer.setFloat( "saturation", saturation );
        buffer.setFloat( "grainAmount", grainAmount );
        buffer.setVector( "vignette", vignette );*/
        mDriver->setConstantBufferAuto( mColorControlsConstants );
    
    } // End if color control op

    // Should we set the color controls as well?
    if ( nOperation == cgImageOperation::All || nOperation == cgImageOperation::BilateralResample || nOperation == cgImageOperation::BilateralBlur )
    {
        /*pBuffer = mBilateralConstants->getResource(true);

        cgVector2 DepthScaleBias;
        DepthScaleBias.x = 1.0f / ( depthExtents.y - depthExtents.x );
        DepthScaleBias.y = -depthExtents.x * depthScaleBias.x;

        cgVector4 NormalTestParams;
        normalTestParams.x = cosf( normalExtents.x * RADIANS_PER_DEGREE );
        normalTestParams.y = cosf( normalExtents.y * RADIANS_PER_DEGREE );
        normalTestParams.z = 1.0f / ( normalDistances.y - normalDistances.x );
        normalTestParams.w = -normalDistances.x * normalTestParams.z;

        Vector2 normalScaleBias;
        if( normalExtents.x == normalExtents.y )
            normalExtents.y += 0.1f;
        float minCos = cosf( normalExtents.x * RADIANS_PER_DEGREE );
        float maxCos = cosf( normalExtents.y * RADIANS_PER_DEGREE );
        minCos *= minCos;
        maxCos *= maxCos;
        normalScaleBias.x = 1.0f / ( minCos - maxCos );
        normalScaleBias.y = -maxCos * normalScaleBias.x;

        cbuffer.setVector( "depthScaleBias", depthScaleBias );
        cbuffer.setVector( "normalScaleBias", normalScaleBias );
        cbuffer.setVector( "normalTestParams", normalTestParams );
        cbuffer.setVector( "depthExtents", depthExtents );*/

        mDriver->setConstantBufferAuto( mBilateralConstants );
    
    } // End if Bilateral Op
}

//-----------------------------------------------------------------------------
// Name: setBlendState()
/// <summary>
/// Sets up the blend state for an operation
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::setBlendState( cgImageOperation::Base operation )
{
    if ( !mApplyBlendState )
        return;

    // Setup blend state  		
    switch( operation )
    {
        case cgImageOperation::SetColorA:
        case cgImageOperation::CopyAlpha:
            mDriver->setBlendState( mDefaultAlphaOnlyBlendState );
            break;
        case cgImageOperation::SetColorRGB:
        case cgImageOperation::CopyRGB:
        case cgImageOperation::ColorScaleRGB:
        case cgImageOperation::InverseRGB:
        case cgImageOperation::UnsignedToSignedRGB:
        case cgImageOperation::SignedToUnsignedRGB:
        case cgImageOperation::RGBEtoRGB:
            mDriver->setBlendState( mDefaultRGBBlendState );
            break;
        case cgImageOperation::AddRGBA:
        case cgImageOperation::ColorScaleAddRGBA:
            mDriver->setBlendState( mAdditiveRGBABlendState );
            break;
        case cgImageOperation::AddRGB:
        case cgImageOperation::ColorScaleAddRGB:
            mDriver->setBlendState( mAdditiveRGBBlendState );
            break;
        case cgImageOperation::AddRGBSetAlpha:
        case cgImageOperation::ColorScaleAddRGBSetAlpha:
            mDriver->setBlendState( mAddRGBSetAlphaBlendState );
            break;
		case cgImageOperation::ScaleUserColorRGBA:
		case cgImageOperation::TextureScaleRGBA:
            mDriver->setBlendState( mModulativeRGBABlendState );
            break;
		case cgImageOperation::ScaleUserColorRGB:
        case cgImageOperation::TextureScaleRGB:
            mDriver->setBlendState( mModulativeRGBBlendState );
            break;
		case cgImageOperation::ScaleUserColorA:
        case cgImageOperation::TextureScaleA:
            mDriver->setBlendState( mModulativeAlphaBlendState );
            break;
        default:
            mDriver->setBlendState( mDefaultRGBABlendState );
            break;
    };
}

void cgImageProcessor::drawClipQuad( cgFloat z )
{
    drawQuad( cgRenderTargetHandle::Null, true, z );
}

void cgImageProcessor::drawClipQuad( const cgRenderTargetHandle & destination, cgFloat z )
{
    drawQuad( destination, true, z );
}

void cgImageProcessor::drawClipQuad( )
{
    drawQuad( cgRenderTargetHandle::Null, true, -1.0f );
}

void cgImageProcessor::drawClipQuad( const cgRenderTargetHandle & destination )
{
    drawQuad( destination, true, -1.0f );
}

//-----------------------------------------------------------------------------
// Name: drawQuad()
/// <summary>
/// Draws a full screen quad for basic image processing. Assumes all states, 
/// shaders, etc. have been set. Can draw either clip or screen space geometry 
/// depending on whether a vertex shader is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgImageProcessor::drawQuad( const cgRenderTargetHandle & destination, bool clipSpace, cgFloat z )
{
    // ToDo: 6767 -- Does not support screen space even though a parameter controlling this is provided.

    // If a valid render target was provided, set it and draw. 
    if ( destination.isValid() )
    {
        if ( mDriver->beginTargetRender( destination, mDepthStencilTarget ) )
        {
            if ( z >= 0.0f )
                mDriver->drawClipQuad( z, true );
            else
                mDriver->drawClipQuad( );
            mDriver->endTargetRender( );

        } // End if beginTargetRender

    } // End if destination supplied
    else
    {
        // Assume caller has already set the target, so just draw.
        if ( z >= 0.0f )
            mDriver->drawClipQuad( z, true );
        else
            mDriver->drawClipQuad( );

    } // End if no destination
}

//-----------------------------------------------------------------------------
// Name: selectClipQuadVertexShader()
// Desc: Draws a full screen clip space quad for basic image processing. Returns
//       clip position and texture coordinates.
//-----------------------------------------------------------------------------
bool cgImageProcessor::selectClipQuadVertexShader( )
{
    static const cgString shaderName = _T("transform");
    return mProcessShader->selectVertexShader( shaderName, false );
}

//-----------------------------------------------------------------------------
// Name: selectClipQuadVertexShader()
// Desc: Draws a full screen clip space quad for basic image processing. Returns
//       clip position, texture coordinates, and an optional eye ray.
//-----------------------------------------------------------------------------
bool cgImageProcessor::selectClipQuadVertexShader( bool computeEyeRay )
{
    static const cgString shaderName = _T("transform");
    return mProcessShader->selectVertexShader( shaderName, computeEyeRay );
}

//-----------------------------------------------------------------------------
// Name: useBilinearSampling()
/// <summary>
/// Determine whether to use bilinear or point sampling for the input texture.
/// </summary>
//-----------------------------------------------------------------------------
bool cgImageProcessor::useBilinearSampling( const cgTextureHandle & source, const cgRenderTargetHandle & destination, cgImageOperation::Base operation )
{
    // Test forcing modes first
    if ( mForcePointSampling ) 
        return false;

    // If the operation requires point sampling...
    if ( operation == cgImageOperation::RGBEtoRGB )
        return false;

    // Get the source texture. If it's invalid, we can't sample.
    const cgTexture * sourceTexture = source.getResourceSilent();
    if ( !sourceTexture )
        return false;

    // Get the destination texture. If it's invalid, grab the current 
    // destination target from the active view. 
    const cgTexture * destinationTexture = destination.getResourceSilent();
    if ( !destinationTexture )
    {
        cgRenderView * activeView = mDriver->getActiveRenderView();
        destinationTexture = activeView->getViewBuffer().getResourceSilent();
    
    } // End if no destination

    // If the source and destination sizes match, do not use 
    // linear filtering (unless forced).
    if ( !mForceLinearSampling && sourceTexture->getSize().width == destinationTexture->getSize().width )
        return false;
    
    // Check for support.
    return sourceTexture->supportsLinearSampling();
}

void cgImageProcessor::setDepthStencilTarget( const cgDepthStencilTargetHandle & target )
{
    mDepthStencilTarget = target;
}

void cgImageProcessor::setDefaultUserColor( const cgColorValue & c )
{
    mDefaultUserColor = c;
}

void cgImageProcessor::setWhiteLevel( const cgVector2 & v )
{
    // ToDo: color control params.
}

void cgImageProcessor::setBlackLevel( const cgVector2 & v )
{
    // ToDo: color control params.
}

void cgImageProcessor::setGamma( cgFloat f )
{
    // ToDo: color control params.
}

void cgImageProcessor::setTint( const cgColorValue & c )
{
    // ToDo: color control params.
}

void cgImageProcessor::setBrightness( cgFloat f )
{
    // ToDo: color control params.
}

void cgImageProcessor::setExposure( cgFloat exposure, cgFloat strength )
{
    // ToDo: color control params.
}

void cgImageProcessor::setSaturation( cgFloat f )
{
    // ToDo: color control params.
}

void cgImageProcessor::setDepthExtents( cgFloat minimum, cgFloat maximum )
{
    // ToDo: set constants
}

void cgImageProcessor::setNormalExtents( cgFloat minimum, cgFloat maximum )
{
    // ToDo: set constants
}

void cgImageProcessor::setNormalDistances( cgFloat minimum, cgFloat maximum )
{
    // ToDo: set constants
}

void cgImageProcessor::setGrain( const cgTextureHandle & texture, cgFloat strength )
{
    mGrainTexture = texture;
    // ToDo: grain params.
}

void cgImageProcessor::setVignette( const cgTextureHandle & texture, cgFloat scale, cgFloat bias, cgFloat strength )
{
    mVignetteTexture = texture;
    // ToDo: Vignette params.    
}

void cgImageProcessor::setRemap( const cgTextureHandle & texture )
{
    mRemapTexture = texture;
}

void cgImageProcessor::forceLinearSampling( bool enable )
{
    mForceLinearSampling = enable;
}

void cgImageProcessor::forcePointSampling( bool enable )
{
    mForcePointSampling = enable;
}

void cgImageProcessor::testStencilBuffer( bool enable, cgUInt32 stencilRef )
{
    mTestStencilBuffer = enable;
    mStencilRef = stencilRef;
}

void cgImageProcessor::applyDepthStencilStates( bool enable )
{
    mApplyDepthStencilState = enable;
}

void cgImageProcessor::applyRasterizerStates( bool enable )
{
    mApplyRasterizerState = enable;
}

void cgImageProcessor::applyBlendStates( bool enable )
{
    mApplyBlendState = enable;
}