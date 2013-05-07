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
// Name : cgDepthOfFieldProcessor.cpp                                        //
//                                                                           //
// Desc : Image processing class designed to apply a depth of field / focus  //
//        blur effect to a rendered image.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgDepthOfFieldProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgDepthOfFieldProcessor.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgDepthOfFieldProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDepthOfFieldProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthOfFieldProcessor::cgDepthOfFieldProcessor(  )
{
    // Initialize variables to sensible defaults
    mLinearSampler      = CG_NULL;
    mPointSampler       = CG_NULL;
    mDepthSampler       = CG_NULL;
    mBlur0Sampler       = CG_NULL;
    mBlur1Sampler       = CG_NULL;
    mColorChain0        = CG_NULL;
    mColorChain1        = CG_NULL;
    mDepthChain         = CG_NULL;
    
    // Setup configuration defaults.
    mForegroundExtents   = cgRangeF(0,0);
    mBackgroundExtents   = cgRangeF(0,0);
    mApplyForegroundBlur = false;
    mApplyBackgroundBlur = false;
    mOptimizeBackground  = true;
    mPreClearedAlpha     = true;
    memset( &mDepthOfFieldConfig, 0, sizeof(_cbDepthOfField) );

    // We can eventually allow customization of the blur falloff distances
    // and even have separate ones for foreground and background. For the moment,
    // we are just going to assume standardized equal distances.
    const cgFloat d0 = 0.33333333f;  // Distance from no blur to small blur
    const cgFloat d1 = 0.33333333f;  // Distance from small blur to medium blur
    const cgFloat d2 = 0.33333333f;  // Distance from medium blur to high blur
    mDepthOfFieldConfig.blurScale = cgVector4( -1.0f / d0, -1.0f / d1, -1.0f / d2, 1.0f / d2 );
    mDepthOfFieldConfig.blurBias  = cgVector4( 1.0f, (1.0f - d2) / d1, 1.0f / d2, (d2 - 1.0f) / d2 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthOfFieldProcessor::~cgDepthOfFieldProcessor( )
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
void cgDepthOfFieldProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mDepthOfFieldShader.close();
    mDepthOfFieldConstants.close();
    mMaxBlurDepthState.close();
    mMaxIgnoreDepthState.close();
    mBackgroundUpSampleBlendState.close();
    mSource.close();
    mDestination.close();
    
    // Release memory
    if ( mLinearSampler )
        mLinearSampler->scriptSafeDispose();
    if ( mPointSampler )
        mPointSampler->scriptSafeDispose();
    if ( mDepthSampler )
        mDepthSampler->scriptSafeDispose();
    if ( mBlur0Sampler )
        mBlur0Sampler->scriptSafeDispose();
    if ( mBlur1Sampler )
        mBlur1Sampler->scriptSafeDispose();
    
    // Clear variables
    mLinearSampler      = CG_NULL;
    mPointSampler       = CG_NULL;
    mDepthSampler       = CG_NULL;
    mBlur0Sampler       = CG_NULL;
    mBlur1Sampler       = CG_NULL;
    mColorChain0        = CG_NULL;
    mColorChain1        = CG_NULL;
    mDepthChain         = CG_NULL;
    
    // Setup configuration defaults.
    mForegroundExtents      = cgRangeF(0,0);
    mBackgroundExtents      = cgRangeF(0,0);
    mApplyForegroundBlur    = false;
    mApplyBackgroundBlur    = false;
    mOptimizeBackground     = true;
    mPreClearedAlpha        = true;
    mBackgroundBlurOps[0]   = cgBlurOpDesc();
    mBackgroundBlurOps[1]   = cgBlurOpDesc();
    mForegroundBlurOps[0]   = cgBlurOpDesc();
    mForegroundBlurOps[1]   = cgBlurOpDesc();
    memset( &mDepthOfFieldConfig, 0, sizeof(_cbDepthOfField) );

    // We can eventually allow customization of the blur falloff distances
    // and even have separate ones for foreground and background. For the moment,
    // we are just going to assume standardized equal distances.
    const cgFloat d0 = 0.33333333f;  // Distance from no blur to small blur
    const cgFloat d1 = 0.33333333f;  // Distance from small blur to medium blur
    const cgFloat d2 = 0.33333333f;  // Distance from medium blur to high blur
    mDepthOfFieldConfig.blurScale = cgVector4( -1.0f / d0, -1.0f / d1, -1.0f / d2, 1.0f / d2 );
    mDepthOfFieldConfig.blurBias  = cgVector4( 1.0f, (1.0f - d2) / d1, 1.0f / d2, (d2 - 1.0f) / d2 );
    
    // Dispose base if requested.
    if ( disposeBase )
        cgImageProcessor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : setForegroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the foreground.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setForegroundExtents( cgFloat minimum, cgFloat maximum )
{
    setForegroundExtents( cgRangeF(minimum,maximum) );
}

//-----------------------------------------------------------------------------
//  Name : setForegroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the foreground.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setForegroundExtents( const cgRangeF & range )
{
    mForegroundExtents = range;
    mApplyForegroundBlur = ( range.min >= 0 && range.max > range.min );
    mDepthOfFieldConfig.regionScaleBias.x = mApplyForegroundBlur ? 1.0f / (range.max - range.min) : 0.0f;
    mDepthOfFieldConfig.regionScaleBias.z = mApplyForegroundBlur ? -mDepthOfFieldConfig.regionScaleBias.x * range.min : 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the background.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setBackgroundExtents( cgFloat minimum, cgFloat maximum )
{
    setBackgroundExtents( cgRangeF(minimum,maximum) );
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundExtents ()
/// <summary>
/// Set the distance range for the part of the scene that is to be considered
/// part of the background.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setBackgroundExtents( const cgRangeF & range )
{
    mBackgroundExtents = range;
    mApplyBackgroundBlur = ( range.min >= 0 && range.max > range.min );
    mDepthOfFieldConfig.regionScaleBias.y = mApplyBackgroundBlur ? 1.0f / (range.max - range.min) : 0.0f;
    mDepthOfFieldConfig.regionScaleBias.w = mApplyBackgroundBlur ? -mDepthOfFieldConfig.regionScaleBias.y * range.max : 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundBlur ()
/// <summary>
/// Configure the level of blur that is applied to the two background levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setBackgroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                                 cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
{
    setBackgroundBlur( cgBlurOpDesc( passCountHigh, pixelRadiusHigh, distanceFactorHigh ),
                       cgBlurOpDesc( passCountLow, pixelRadiusLow, distanceFactorLow ) );
}

//-----------------------------------------------------------------------------
//  Name : setBackgroundBlur ()
/// <summary>
/// Configure the level of blur that is applied to the two background levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setBackgroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
{
    mBackgroundBlurOps[0] = highBlur;
    mBackgroundBlurOps[1] = lowBlur;
    mBackgroundBlurOps[0].inputAlpha = cgAlphaWeightMethod::Sample;
    mBackgroundBlurOps[0].outputAlpha = cgAlphaWeightMethod::Sample;
    mBackgroundBlurOps[1].inputAlpha = cgAlphaWeightMethod::Sample;
    mBackgroundBlurOps[1].outputAlpha = cgAlphaWeightMethod::Sample;
}

//-----------------------------------------------------------------------------
//  Name : setForegroundBlur()
/// <summary>
/// Configure the level of blur that is applied to the two foreground levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setForegroundBlur( cgInt32 passCountHigh, cgInt32 pixelRadiusHigh, cgFloat distanceFactorHigh,
                                                 cgInt32 passCountLow, cgInt32 pixelRadiusLow, cgFloat distanceFactorLow )
{
    setForegroundBlur( cgBlurOpDesc( passCountHigh, pixelRadiusHigh, distanceFactorHigh ),
                       cgBlurOpDesc( passCountLow, pixelRadiusLow, distanceFactorLow ) );
}

//-----------------------------------------------------------------------------
//  Name : setForegroundBlur()
/// <summary>
/// Configure the level of blur that is applied to the two foreground levels.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setForegroundBlur( const cgBlurOpDesc & highBlur, const cgBlurOpDesc & lowBlur )
{
    mForegroundBlurOps[0] = highBlur;
    mForegroundBlurOps[1] = lowBlur;
    mForegroundBlurOps[0].inputAlpha = cgAlphaWeightMethod::None;
    mForegroundBlurOps[0].outputAlpha = cgAlphaWeightMethod::Center;
    mForegroundBlurOps[1].inputAlpha = cgAlphaWeightMethod::None;
    mForegroundBlurOps[1].outputAlpha = cgAlphaWeightMethod::None;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDepthOfFieldProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mDepthOfFieldShader, _T("sys://Shaders/DepthOfField.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mDepthOfFieldConstants, mDepthOfFieldShader, _T("cbDepthOfField"), cgDebugSource() ) )
        return false;
    cgAssert( mDepthOfFieldConstants->getDesc().length == sizeof(_cbDepthOfField) );

    ///////////////////////////////////////////////////////////////
    // Depth Stencil States
    ///////////////////////////////////////////////////////////////
    cgDepthStencilStateDesc dsStates;

    // Sky blur fill depth state 
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = false;
    dsStates.depthFunction    = cgComparisonFunction::LessEqual;
    dsStates.stencilEnable    = true;
    dsStates.stencilReadMask  = 1;
    dsStates.stencilWriteMask = 1;
    dsStates.frontFace.stencilFunction           = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                            = dsStates.frontFace; 
    resources->createDepthStencilState( &mMaxBlurDepthState, dsStates, 0, cgDebugSource() );

    // Sky ignore depth state
    dsStates.depthEnable      = true;
    dsStates.depthWriteEnable = false;
    dsStates.depthFunction    = cgComparisonFunction::LessEqual;
    dsStates.stencilEnable    = true;
    dsStates.stencilReadMask  = 1;
    dsStates.stencilWriteMask = 0;
    dsStates.backFace.stencilFunction           = cgComparisonFunction::Equal;
    dsStates.backFace.stencilPassOperation      = cgStencilOperation::Keep;
    dsStates.backFace.stencilFailOperation      = cgStencilOperation::Keep;
    dsStates.backFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.frontFace                          = dsStates.backFace; // Duplicate to improve batching.
    resources->createDepthStencilState( &mMaxIgnoreDepthState, dsStates, 0, cgDebugSource() );

    ///////////////////////////////////////////////////////////////
    // Blend States
    ///////////////////////////////////////////////////////////////
    cgBlendStateDesc blStates;

    // Alpha only blending
    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Alpha;
    blStates.renderTarget[0].blendEnable           = false;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::Zero;
    resources->createBlendState( &mDefaultAlphaOnlyBlendState, blStates, 0, cgDebugSource() );

    blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::Red | cgColorChannel::Green | cgColorChannel::Blue;
    blStates.renderTarget[0].blendEnable           = true;
    blStates.renderTarget[0].sourceBlend           = cgBlendMode::DestAlpha;
    blStates.renderTarget[0].destinationBlend      = cgBlendMode::InvDestAlpha;
    resources->createBlendState( &mBackgroundUpSampleBlendState, blStates, 0, cgDebugSource() );

    ///////////////////////////////////////////////////////////////
    // Sampler States
    ///////////////////////////////////////////////////////////////
    // Create samplers
    mLinearSampler = resources->createSampler( _T("Color"), mDepthOfFieldShader );
    mPointSampler  = resources->createSampler( _T("Color"), mDepthOfFieldShader );
    mDepthSampler  = resources->createSampler( _T("Depth"), mDepthOfFieldShader );
    mBlur0Sampler  = resources->createSampler( _T("Blur0"), mDepthOfFieldShader );
    mBlur1Sampler  = resources->createSampler( _T("Blur1"), mDepthOfFieldShader );
    mPointSampler->setStates( mSamplers.point->getStates() );
    mDepthSampler->setStates( mSamplers.point->getStates() );
    mLinearSampler->setStates( mSamplers.linear->getStates() );
    mBlur0Sampler->setStates( mSamplers.linear->getStates() );
    mBlur1Sampler->setStates( mSamplers.linear->getStates() );

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : execute()
/// <summary>
/// Post-processes the rendered scene data in order to make bright objects 
/// appear to glow or "glare".
/// </summary>
//-----------------------------------------------------------------------------
bool cgDepthOfFieldProcessor::execute( const cgRenderTargetHandle & source, const cgRenderTargetHandle & destination, 
                                       cgResampleChain * colorChain0, cgResampleChain * colorChain1, cgResampleChain * depthChain,
                                       cgDepthType::Base depthType )
{
    // Shader must be valid and loaded at this point.
    if ( !mDepthOfFieldShader.getResource(true) || !mDepthOfFieldShader.isLoaded() )
        return false;

    // If we have nothing to do, just return
    if ( !mApplyForegroundBlur && !mApplyBackgroundBlur )
        return false;

    // Store operation configuration.
    mSource         = source;
    mDestination    = destination;
    mColorChain0    = colorChain0;
    mColorChain1    = colorChain1;
    mDepthChain     = depthChain;
    mDepthType      = depthType;

    // Set constants    
    setConstants();

    // If we are processing background, an alpha aware down-sample and blur works well (removes halos)
    if ( mApplyBackgroundBlur )
    {
        // Prepare the source data as required
        if ( mOptimizeBackground && mPreClearedAlpha )
        {
            // Clear the alpha channel to 0 (i.e, everything initially is in-focus)
            processColorImage( cgRenderTargetHandle::Null, mSource, cgImageOperation::SetColorA, cgColorValue(0x00000000) );
        }
        else
        {
            // Copy the non-background texels into destination buffer while also clearing alpha to 0
            processColorImage( mSource, mDestination, cgImageOperation::CopyRGBSetAlpha, cgColorValue(0x00000000) );
        }

        // Compute background blurriness 
        computeBlurriness( false, true );

        // Downsample and blur
        downSampleBackground( 0 );
        downSampleBackground( 1 );

        forceLinearSampling( mOptimizeBackground );
        blur( mColorChain0->getLevel( 1 ), mColorChain1->getLevel( 1 ), mBackgroundBlurOps[0] );
        blur( mColorChain0->getLevel( 2 ), mColorChain1->getLevel( 2 ), mBackgroundBlurOps[1] );
        forceLinearSampling( false );

        // Composite results
        if ( mOptimizeBackground )
            fastBackgroundComposite( );
        else
            composite( false, true );
    
    } // End if blur background

    // If we are processing only foreground, a straight down-sample and blur works well
    if ( mApplyForegroundBlur )
    {
        // Swap the targets if we did an unoptimized background blur 
        if ( mApplyBackgroundBlur && mOptimizeBackground )
        {
            cgRenderTargetHandle tempHandle = mDestination;
            mDestination = mSource;
            mSource = tempHandle;
        
        } // End if swap

        // Clear the alpha channel to 0 (i.e, everything initially is in-focus)
        processColorImage( mSource, cgImageOperation::SetColorA, cgColorValue(0x00000000) );

        // Compute foreground blurriness
        computeBlurriness( true, false );

        // Downsample and blur
        downSample( source, mColorChain0->getLevel( 1 ), cgImageOperation::DownSampleAverage, false, false, false );
        downSample( mColorChain0->getLevel( 1 ), mColorChain0->getLevel( 2 ), cgImageOperation::DownSampleAverage, false, false, false );
        blur( mColorChain0->getLevel( 1 ), mColorChain1->getLevel( 1 ), mForegroundBlurOps[0] );
        blur( mColorChain0->getLevel( 2 ), mColorChain1->getLevel( 2 ), mForegroundBlurOps[1] );

        // Clean up the blurred coc
        finalizeForegroundCoC();

        // Composite results
        composite( true, false );
    
    } // End if blur foreground

    // Don't retain references to the targets.
    mSource.close();
    mDestination.close();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : computeBlurriness() (Protected)
/// <summary>
/// Computes blurriness factor and stores in alpha channel of source texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::computeBlurriness( bool foreground, bool background )
{
    // Set default rasterizer state and alpha only blend state
    mDriver->setBlendState( mDefaultAlphaOnlyBlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Do we want to run the pass over all pixels?
    if ( foreground && background )
    {		
        // Set states
        mDriver->setDepthStencilState( mDisabledDepthState );

        // Set texture and sampler states
        mDepthSampler->apply( mDepthChain->getLevel( 0 ) );

        // Select shaders
        if ( !selectClipQuadVertexShader( true ) ||
            !mDepthOfFieldShader->selectPixelShader( _T("computeBlurriness"), (cgInt32)mDepthType, foreground, background ) )
            return;

        // Draw quad 
        drawClipQuad( mSource );
    
    } // End if both
    else
    {
        // Process only a single region
        if ( foreground )
        {
            // Set states
            mDriver->setDepthStencilState( mGreaterDepthState );

            // Set texture and sampler states
            mDepthSampler->apply( mDepthChain->getLevel( 0 ) );

            // Select shaders
            if ( !selectClipQuadVertexShader(true) ||
                !mDepthOfFieldShader->selectPixelShader( _T("computeBlurriness"), (cgInt32)mDepthType, true, false ) )
                return;

            // Draw quad 
            drawClipQuad( mSource, mForegroundExtents.max );
        }
        else if ( background )
        {
            bool mSeparateMaxBlur = false;
            if ( mSeparateMaxBlur )
            {
                // Begin rendering to source alpha
                if ( mDriver->beginTargetRender( mSource ) )
                {
                    // Clear the stencil buffer to 0
                    mDriver->clear( cgClearFlags::Stencil, 0, 0, 0 );

                    // Select shaders
                    if ( !selectClipQuadVertexShader() ||
                        !mDepthOfFieldShader->selectPixelShader( _T("maxBlurriness") ) )
                    {
                        mDriver->endTargetRender();
                        return;

                    } // End if failed

                    // Draw with LE depth test at the background far plane, with a stencil test
                    mDriver->setDepthStencilState( mMaxBlurDepthState, 1 );
                    drawClipQuad( mBackgroundExtents.max );

                    // Set depth texture
                    mDepthSampler->apply( mDepthChain->getLevel( 0 ) );

                    // Select shaders for blurriness computation
                    if ( !mDepthOfFieldShader->selectPixelShader( _T("computeBlurriness"), (cgInt32)mDepthType, false, true ) )
                    {
                        mDriver->endTargetRender( );
                        return;

                    } // End if failed

                    // Draw with LE depth test at the background near plane, with a stencil test to avoid processing "max blur" pixels
                    mDriver->setDepthStencilState( mMaxIgnoreDepthState, 0 );
                    drawClipQuad( mBackgroundExtents.min );

                    // End rendering
                    mDriver->endTargetRender( );

                }  // End if beginTargetRender
            
            } // End if separate
            else
            {
                // Draw with LE depth test at the background near plane
                mDriver->setDepthStencilState( mLessEqualDepthState );

                // Set texture and sampler states
                mDepthSampler->apply( mDepthChain->getLevel( 0 ) );

                // Select shaders
                if ( !selectClipQuadVertexShader(true) ||
                    !mDepthOfFieldShader->selectPixelShader( _T("computeBlurriness"), (cgInt32)mDepthType, false, true ) )
                    return;

                // Draw quad 
                drawClipQuad( mSource, mBackgroundExtents.min );
            
            } // End if !separate
        
        } // End if background
    
    } // End if !both
}

//-----------------------------------------------------------------------------
// Name : composite() (Protected)
/// <summary>
/// Computes final depth of field effect 
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::composite( bool foreground, bool background )
{
    // Set states
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set textures and sampler states
    mLinearSampler->apply( mSource );
    mBlur0Sampler->apply( mColorChain0->getLevel( 1 ) );
    mBlur1Sampler->apply( mColorChain0->getLevel( 2 ) );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mDepthOfFieldShader->selectPixelShader( _T("composite"), foreground, background ) )
        return;

    // If we are isolating a single region only, we'll want to draw the 
    // quad at an appropriate location to process the necessary pixels
    if ( background )
    {
        // Draw with a less equal test at background near location
        mDriver->setDepthStencilState( mLessEqualDepthState );

        // Draw fullscreen quad		
        drawClipQuad( mDestination, mBackgroundExtents.min );
    
    } // End if background
    else
    {
        // Process every pixel
        mDriver->setDepthStencilState( mDisabledDepthState );

        // Draw fullscreen quad		
        drawClipQuad( mDestination );
    
    } // End if foreground
}

//-----------------------------------------------------------------------------
// Name : finalizeForegroundCoC()
/// <summary>
/// Merged blurred and unblurred foreground circle of confusion and performs a
/// small blur.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::finalizeForegroundCoC( )
{
    // Set states
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setDepthStencilState( mDisabledDepthState );

    // Set textures and sampler states
    mBlur0Sampler->apply( mColorChain0->getLevel( 1 ) );
    mBlur1Sampler->apply( mColorChain0->getLevel( 2 ) );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mDepthOfFieldShader->selectPixelShader( _T("finalizeForegroundCoC") ) )
        return;

    // Draw fullscreen quad		
    drawClipQuad( mColorChain1->getLevel( 1 ) );

    // Blur the results (simple 3x3 using linear filtering)
    mBlur0Sampler->apply( mColorChain1->getLevel( 1 ) );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mDepthOfFieldShader->selectPixelShader( _T("blurForegroundCoC") ) )
        return;

    // Draw fullscreen quad		
    mDriver->setBlendState( mDefaultAlphaOnlyBlendState );
    drawClipQuad( mColorChain0->getLevel( 1 ) );
}

//-----------------------------------------------------------------------------
// Name : downSampleBackground() (protected)
/// <summary>
/// Down-sample the background region.
/// </summary>
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::downSampleBackground( cgInt32 level )
{
    // Set states
    mDriver->setBlendState( mDefaultRGBABlendState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setDepthStencilState( mDisabledDepthState );

    // Set textures and sampler states
    if ( level == 0 )
        mPointSampler->apply( mSource );
    else
        mPointSampler->apply( mColorChain0->getLevel( 1 ) );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mDepthOfFieldShader->selectPixelShader( _T("downSampleBackground"), level ) )
        return;

    // Draw fullscreen quad		
    if ( level == 0 )
        drawClipQuad( mColorChain0->getLevel( 1 ) );
    else	
        drawClipQuad( mColorChain0->getLevel( 2 ) );
}

//-----------------------------------------------------------------------------
// Name : fastBackgroundComposite() (Protected)
/// <summary>
/// A faster, but less accurate/controllable means to render the background. It
/// interpolates the highest blur level and the medium blur level using the 
/// medium blur level's blurriness (dest alpha blending). Then it does the same 
/// for the medium blur level with the unblurred source image using the 
/// original (undownsampled) blurriness factor in the source's alpha.  
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::fastBackgroundComposite( )
{
    // Set states
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( mBackgroundUpSampleBlendState );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mDepthOfFieldShader->selectPixelShader( _T("upsampleBackgroundBlur") ) )
        return;

    // First upsample and blend the large blur to the small blur.
    mBlur0Sampler->apply( mColorChain0->getLevel( 2 ) );
    mDriver->setDepthStencilState( mDisabledDepthState );
    drawClipQuad( mColorChain0->getLevel( 1 ) );

    // Now upsample the combined results and blend over the source texture.
    mBlur0Sampler->apply( mColorChain0->getLevel( 1 ) );
    mDriver->setDepthStencilState( mLessEqualDepthState );
    drawClipQuad( mSource, mBackgroundExtents.min );

    // Swap the source and destination
    cgRenderTargetHandle tempHandle = mDestination;
    mDestination = mSource;
    mSource = tempHandle;
}

//-----------------------------------------------------------------------------
// Name : setConstants() (Protected)
// Desc : Applies constants for the depth of field effect.
//-----------------------------------------------------------------------------
void cgDepthOfFieldProcessor::setConstants( )
{
    // Original
    const cgRenderTarget * sourceTarget = mSource.getResourceSilent();
    cgSize sourceSize = sourceTarget->getSize();
    mDepthOfFieldConfig.textureSize = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 
                                                   1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );

    // Medium
    sourceTarget = mColorChain0->getLevel(1).getResourceSilent();
    sourceSize = sourceTarget->getSize();
    mDepthOfFieldConfig.textureSizeMedium = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 
                                                         1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );

    // Large
    sourceTarget = mColorChain0->getLevel(2).getResourceSilent();
    sourceSize = sourceTarget->getSize();
    mDepthOfFieldConfig.textureSizeLarge = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 
                                                        1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );
    // Update the constant buffer
    mDepthOfFieldConstants->updateBuffer( 0, 0, &mDepthOfFieldConfig );
    mDriver->setConstantBufferAuto( mDepthOfFieldConstants );
}