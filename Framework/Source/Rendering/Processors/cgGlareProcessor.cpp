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
// Name : cgGlareProcessor.cpp                                               //
//                                                                           //
// Desc : Image processing class designed to apply a glare / bloom to a      //
//        rendered image.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgGlareProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgVertexFormats.h>
#include <Rendering/Processors/cgGlareProcessor.h>
#include <Rendering/Processors/cgToneMapProcessor.h>
#include <Rendering/Processors/cgAntialiasProcessor.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>


// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgGlareProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgGlareProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGlareProcessor::cgGlareProcessor(  )
{
	// Initialize variables to sensible defaults
	mResampleChain0         = CG_NULL;
	mResampleChain1         = CG_NULL;
	mPointSampler           = CG_NULL;
	mLinearSampler          = CG_NULL;
	mLuminanceSampler       = CG_NULL;
	mToneMapper             = CG_NULL;
	mDepthSampler           = CG_NULL;
	mFlareSampler           = CG_NULL;
	mPointBorderSampler     = CG_NULL;
	mLinearBorderSampler    = CG_NULL;
	mILRVertexFormat        = CG_NULL;

	// Setup configuration defaults.
	mBrightThreshold = cgRangeF(0,0);
	mBrightPassConfig.brightRangeScale  = 1.0f;
	mBrightPassConfig.brightRangeBias   = 0.0f;
	mBrightPassConfig.alphaMaskAmount   = 0.0f;
	mBrightPassConfig.textureSizeRecip  = cgVector2(0,0);
	mUpdateCacheConfig.blendAmount	    = 0.0f;
	mUpdateCacheConfig.blendRate        = 0.0f;
	mGlareAmount                        = 0;

	mAnamorphicIntensity                = 0.4f;
	mAnamorphicPasses                   = 0;
	mAnamorphicRadius                   = 4;
	mAnamorphicConfig.flareColor        = cgVector3( 0.55f, 0.55f, 1.0f );
	mAnamorphicConfig.flarePassScale    = 0;
	mAnamorphicConfig.flareCoordScale   = 0.75f;
	mAnamorphicConfig.flareCoordBias    = 0.65f;

	mFullSizeBrightPass                 = false;
	mDownsampleBlurPrePass              = false; 
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGlareProcessor::~cgGlareProcessor( )
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
void cgGlareProcessor::dispose( bool disposeBase )
{
	// Close resource handles.
	mGlareShader.close();
	mBrightPassConstants.close();
	mUpdateCacheConstants.close();
	mOperationTarget.close();
	mAlphaBlendState.close();

	// Release memory
	if ( mPointSampler )
		mPointSampler->scriptSafeDispose();
	if ( mLinearSampler )
		mLinearSampler->scriptSafeDispose();
	if ( mLuminanceSampler ) 
		mLuminanceSampler->scriptSafeDispose();
	if ( mDepthSampler ) 
		mDepthSampler->scriptSafeDispose();
	if ( mFlareSampler ) 
		mFlareSampler->scriptSafeDispose();
	if ( mPointBorderSampler ) 
		mPointBorderSampler->scriptSafeDispose();
	if ( mLinearBorderSampler ) 
		mLinearBorderSampler->scriptSafeDispose();

	mSteps.clear();

	// Clear variables
	mResampleChain0         = CG_NULL;
	mResampleChain1         = CG_NULL;
	mPointSampler           = CG_NULL;
	mLinearSampler          = CG_NULL;
	mLuminanceSampler       = CG_NULL;
	mILRVertexFormat		= CG_NULL;

	// Setup configuration defaults.
	mBrightThreshold = cgRangeF(0,0);
	mBrightPassConfig.brightRangeScale  = 1.0f;
	mBrightPassConfig.brightRangeBias   = 0.0f;
	mBrightPassConfig.alphaMaskAmount   = 0.0f;
	mBrightPassConfig.textureSizeRecip  = cgVector2(0,0);
	mUpdateCacheConfig.blendAmount	    = 0.0f;
	mUpdateCacheConfig.blendRate        = 0.0f;
	mGlareAmount                        = 0;

	mAnamorphicIntensity                = 0.4f;
	mAnamorphicPasses                   = 0;
	mAnamorphicRadius                   = 4;
	mAnamorphicConfig.flareColor        = cgVector3( 0.55f, 0.55f, 1 );
	mAnamorphicConfig.flarePassScale    = 0;
	mAnamorphicConfig.flareCoordScale   = 0.75f;
	mAnamorphicConfig.flareCoordBias    = 0.65f;

	// Dispose base if requested.
	if ( disposeBase )
		cgImageProcessor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : setBrightThreshold()
/// <summary>
/// Configure the intensity ranges to be considered for the glare process.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setBrightThreshold( cgFloat minimum, cgFloat maximum )
{
	setBrightThreshold( cgRangeF(minimum,maximum) );
}

//-----------------------------------------------------------------------------
//  Name : setBrightThreshold()
/// <summary>
/// Configure the intensity ranges to be considered for the glare process.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setBrightThreshold( const cgRangeF & range )
{
	mBrightThreshold = range;
	mBrightPassConfig.brightRangeScale = 1.0f / (range.max - range.min);
	mBrightPassConfig.brightRangeBias  = -range.min / (range.max - range.min);
}

//-----------------------------------------------------------------------------
//  Name : setGlareSteps()
/// <summary>
/// Configure the number and type of operations that the glare processor
/// should perform at each down-sample / up-sample step.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setGlareSteps( const GlareStepArray & steps )
{
	mSteps = steps;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGlareProcessor::initialize( cgRenderDriver * driver )
{
	// Call base class implementation first.
	if ( !cgImageProcessor::initialize( driver ) )
		return false;

	// Create an instance of the glare processing surface shader
	cgResourceManager * resources = driver->getResourceManager();
	if ( !resources->createSurfaceShader( &mGlareShader, _T("sys://Shaders/Glare.sh"), 0, cgDebugSource() ) )
		return false;

	// Create constant buffers that map operation data
	// to the physical processing shader.
	if ( !resources->createConstantBuffer( &mBrightPassConstants, mGlareShader, _T("cbBrightPass"), cgDebugSource() ) )
		return false;
	if ( !resources->createConstantBuffer( &mUpdateCacheConstants, mGlareShader, _T("cbUpdateCache"), cgDebugSource() ) )
		return false;
	if ( !resources->createConstantBuffer( &mAddLayerConstants, mGlareShader, _T("cbAddLayer"), cgDebugSource() ) )
		return false;
	if ( !resources->createConstantBuffer( &mILRBrightPassConstants, mGlareShader, _T("cbILRBrightPass"), cgDebugSource() ) )
		return false;
	if ( !resources->createConstantBuffer( &mILRCompositePassConstants, mGlareShader, _T("cbILRCompositePass"), cgDebugSource() ) )
		return false;
	if ( !resources->createConstantBuffer( &mAnamorphicConstants, mGlareShader, _T("cbAnamorphicFlare"), cgDebugSource() ) )
		return false;

	cgAssert( mBrightPassConstants->getDesc().length == sizeof(_cbBrightPass) );
	cgAssert( mUpdateCacheConstants->getDesc().length == sizeof(_cbUpdateCache) );
	cgAssert( mAddLayerConstants->getDesc().length == sizeof(_cbAddLayer) );
	cgAssert( mILRBrightPassConstants->getDesc().length == sizeof(_cbILRBrightPass) );
	cgAssert( mILRCompositePassConstants->getDesc().length == sizeof(_cbILRCompositePass) );
	cgAssert( mAnamorphicConstants->getDesc().length == sizeof(_cbAnamorphicFlare) );

	// Load the ILR flare color texture
	if ( !resources->loadTexture( &mFlareColor, _T("sys://Textures/FlareColorMap.dds"), 0, cgDebugSource() ) )
		cgAppLog::write( cgAppLog::Error, _T("Failed to load ILR flare color texture.\n") );

	// Create samplers
	mPointSampler = resources->createSampler( _T("Source"), mGlareShader );
	mLinearSampler = resources->createSampler( _T("Source"), mGlareShader );
	mPointBorderSampler = resources->createSampler( _T("Source"), mGlareShader );
	mLinearBorderSampler = resources->createSampler( _T("Source"), mGlareShader );
	mLuminanceSampler = resources->createSampler( _T("Luminance"), mGlareShader );
	mDepthSampler = resources->createSampler( _T("Depth"), mGlareShader );
	mFlareSampler = resources->createSampler( _T("Flare"), mGlareShader );
	mPointSampler->setStates( mSamplers.point->getStates() );
	mLinearSampler->setStates( mSamplers.linear->getStates() );
	mPointBorderSampler->setStates( mSamplers.pointBorder->getStates() );
	mLinearBorderSampler->setStates( mSamplers.linearBorder->getStates() );
	mLuminanceSampler->setStates( mSamplers.point->getStates() );
	mDepthSampler->setStates( mSamplers.point->getStates() );
	mFlareSampler->setStates( mSamplers.linear->getStates() );

	// Create our layer blending state
	cgBlendStateDesc blStates;
	blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
	blStates.renderTarget[0].blendEnable           = true;
	blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
	blStates.renderTarget[0].destinationBlend      = cgBlendMode::SrcAlpha;
	resources->createBlendState( &mLayerBlendState, blStates, 0, cgDebugSource() );

	// Create the ILR vertex format
	mILRVertexFormat = cgVertexFormat::formatFromDeclarator( cgScreenVertex::Declarator );

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
bool cgGlareProcessor::execute( const cgRenderTargetHandle & target, cgResampleChain * chain0, cgResampleChain * chain1, bool hdrGlare, float alphaMaskAmt, bool fullSizeBrightPass, bool blurPrePass )
{
	// Glare shader must be valid and loaded at this point.
	if ( !mGlareShader.getResource(true) || !mGlareShader.isLoaded() )
		return false;

	// Get the target data.
	const cgRenderTarget * targetResource = target.getResourceSilent();
	if ( !targetResource )
		return false;
	cgSize targetSize = targetResource->getSize();
	mBrightPassConfig.textureSizeRecip = cgVector2( 1.0f / (cgFloat)targetSize.width, 1.0f / (cgFloat)targetSize.height );

	// Set the amount of alpha masking
	mBrightPassConfig.alphaMaskAmount = alphaMaskAmt;

	// Store operation configuration.
	mOperationTarget       = target;
	mResampleChain0        = chain0;
	mResampleChain1        = chain1;
	mFullSizeBrightPass    = fullSizeBrightPass;
	mDownsampleBlurPrePass = blurPrePass;
	mHDRGlare              = hdrGlare;

	// Run the bright pass on the specified target.
	brightPass();

	// Down-sample and blur
	downsampleAndBlur();

	// Don't retain a reference to the targets.
	mOperationTarget.close();

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : brightPass() (Protected)
/// <summary>
/// Execute the 'bright pass' as part of the first down sample. This process
/// retains only the parts of the image that have a luminance greater than
/// the specified threshold so that objects appear to glow or "glare" at
/// bright spots.
//-----------------------------------------------------------------------------
void cgGlareProcessor::brightPass( )
{
	bool supportsLinearSampling;

	// Downsample to 1/4 size
	if ( !mFullSizeBrightPass )
	{
		downSample( mOperationTarget, mResampleChain1->getLevel( 1 ) );

		// Set the texture and sampler states
		const cgRenderTarget * pTarget = mOperationTarget.getResourceSilent();
		supportsLinearSampling = (pTarget) ? pTarget->supportsLinearSampling() : false;
		if ( supportsLinearSampling )
			mLinearSampler->apply( mResampleChain1->getLevel( 1 ) );
		else
			mPointSampler->apply( mResampleChain1->getLevel( 1 ) );
	}
	else
	{
		// Set the texture and sampler states
		const cgRenderTarget * pTarget = mOperationTarget.getResourceSilent();
		supportsLinearSampling = (pTarget) ? pTarget->supportsLinearSampling() : false;
		if ( supportsLinearSampling )
			mLinearSampler->apply( mOperationTarget );
		else
			mPointSampler->apply( mOperationTarget );
	}

	// Setup depth-stencil state (depth always disabled for image processing)
	// and rasterizer state (default).
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

	// Set blend states
	mDriver->setBlendState( mDefaultRGBABlendState );

	// Set shader constants
	mBrightPassConstants->updateBuffer( 0, 0, &mBrightPassConfig );
	mDriver->setConstantBufferAuto( mBrightPassConstants );

	// If we have a tone mapper, set the luminance data
	cgInt32 toneMappingMethod = -1;
	if ( mHDRGlare )
	{
		toneMappingMethod = cgToneMapProcessor::Filmic;
		if ( mToneMapper )
		{
			// Get the current luminance for the scene. 
			// ToDo: 6767 - Once tonemapper is fixed up properly, remove its call to getLuminanceData since we'll be fetching directly here
			//cgRenderView * activeView = mDriver->getActiveRenderView();
			//cgString bufferName = cgString::format( _T("Tonemapper::LuminanceCurr1x1_%i"), activeView->getReferenceId() );
			//mLuminanceCurrTarget = activeView->getRenderSurface( bufferName ); 
			mLuminanceSampler->apply( mToneMapper->getLuminanceData() );
		}
	}

	// Are we using an alpha/glow mask?
	bool glowMask = mBrightPassConfig.alphaMaskAmount > 0.0f;

	// Select shaders
	if ( !selectClipQuadVertexShader() ||
		!mGlareShader->selectPixelShader( _T("brightPass"), supportsLinearSampling, toneMappingMethod, !mFullSizeBrightPass, glowMask ) )
		return;

	// Do we want a full size bright pass or a quarter size pass?
	if ( mFullSizeBrightPass )
	{
		// Get a scratch buffer (full size)
		cgRenderView * activeView = mDriver->getActiveRenderView();
		cgImageInfo info = mOperationTarget->getInfo();
		cgString scratchName = cgString::format( _T("System::Core::ScratchTarget_%ix%i"), info.width, info.height );
		const cgRenderTargetHandle & tmpTarget = activeView->getRenderSurface( info.format, 1.0f, 1.0f, info.multiSampleType, info.multiSampleQuality, scratchName );

		// Draw to full size target
		drawClipQuad( tmpTarget );

		// Downsample to 1/4 size
		downSample( tmpTarget, mResampleChain0->getLevel( 1 ) );
	}
	else
	{
		// Draw to 1/4 size target
		drawClipQuad( mResampleChain0->getLevel( 1 ) );
	}
}

//-----------------------------------------------------------------------------
// Name : downsampleAndBlur() (Protected)
/// <summary>
/// Down samples and blurs the thresholded texture
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::downsampleAndBlur()
{
	// We'll be building a local array of the glare step array indices 
	// that match the associated mip levels in the resample chain.
	const size_t levelCount = mResampleChain0->getLevelCount();
	cgUInt32Array stepIndices( levelCount );

	// We don't do any work at mip level 0 as the initial bright-pass 
	// is also a down sample.
	stepIndices[ 0 ] = 0xFFFFFFFF; 

	// Iterate the mip levels and process the glare steps to determine
	// the range of levels on which we need to perform operations in
	// addition to populating the above array.
	cgInt32 minimumLevel = 0xFFFFFF, maximumLevel = 0;
	for ( size_t i = 1; i < levelCount; i++ )
	{
		stepIndices[ i ] = getGlareStepIndex( i );
		if ( stepIndices[ i ] != 0xFFFFFFFF )
		{
			if ( mSteps[ stepIndices[ i ] ].levelIndex < minimumLevel )
				minimumLevel = mSteps[ stepIndices[ i ] ].levelIndex;

			if ( mSteps[ stepIndices[ i ] ].levelIndex > maximumLevel )
				maximumLevel = mSteps[ stepIndices[ i ] ].levelIndex;

		} // End if step supplied

	} // Next level

	// Apply a blur to the first level before downsampling (optional)
	if ( mDownsampleBlurPrePass )
	{
		cgBlurOpDesc blurDesc( 1, 2, 2.0 );
		blur( mResampleChain0->getLevel( 1 ), mResampleChain1->getLevel( 1 ), blurDesc );
	}

	// Downsample first
	for ( cgInt32 i = 1; i <= maximumLevel; ++i )
	{
		// Down sample to next level
		if ( i != maximumLevel )
			downSample( mResampleChain0->getLevel( i ), mResampleChain0->getLevel( i + 1 ) );

	} // Next level

	// Weighted additive upsample
	int nLastLayer = -1;
	_cbAddLayer layerConfig;
	for ( cgInt32 i = maximumLevel; i >= 1; --i )
	{
		// Blur current level if user data is available
		if ( stepIndices[ i ] != 0xFFFFFFFF )
		{
			// Get the current layer's weight (intensity)
			float I = mSteps[ stepIndices[ i ] ].intensity;

			// In HDR mode, adjust by overall glare amount
			if ( mHDRGlare ) I *= mGlareAmount;

			// If we are adding a prior image to the current image
			if ( nLastLayer != -1 )
			{
				// Use modulative blending to scale the current layer by its weight
				layerConfig.layerWeight = I;
				mAddLayerConstants->updateBuffer( 0, 0, &layerConfig );
				mDriver->setConstantBufferAuto( mAddLayerConstants );

				// Set the previous level to add
				mLinearSampler->apply( mResampleChain0->getLevel( nLastLayer ) );

				// Setup states (adds prev layer and modulates current by its weight)
				mDriver->setDepthStencilState( mDisabledDepthState );
				mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
				mDriver->setBlendState( mLayerBlendState );

				// Select shaders
				if ( !selectClipQuadVertexShader() ||
					!mGlareShader->selectPixelShader( _T("addLayer") ) )
					return;

				// Draw quad
				drawClipQuad( mResampleChain0->getLevel( i ) );
			}
			else
			{
				// Note: In LDR mode, this can only be used to reduce layer intensity, not increase it (due to clamp of 1)
				processColorImage( mResampleChain0->getLevel( i ), cgImageOperation::ScaleUserColorRGB, cgColorValue( I, I, I, 1.0f ) );
			}

			// Blur the current image
			if ( mSteps[ stepIndices[ i ] ].blurOp.passCount > 0 )
				blur( mResampleChain0->getLevel( i ), mResampleChain1->getLevel( i ), mSteps[ stepIndices[ i ] ].blurOp );

			// If this layer is using caching, blend the prior data in. (Could also try it pre-blur as well??)
			// Note: Because of the upsample/blur approach, any cache blend layer will automatically affect layers below it, even if they
			//       were not directly flagged as cached. That said, caching is usually preferred for lower levels anyway.
			if ( mSteps[ stepIndices[ i ] ].cacheBlendAmount > 0.0f )
				updateCache( i );

			// Track the last index
			nLastLayer = i;

		} // End if user info available

	} // Next level

	// Add final 
	if ( mHDRGlare )
		processColorImage( mResampleChain0->getLevel( nLastLayer ), mOperationTarget, cgImageOperation::AddRGB );
	else
		processColorImage( mResampleChain0->getLevel( nLastLayer ), mOperationTarget, cgImageOperation::ColorScaleAddRGB, cgColorValue( mGlareAmount, mGlareAmount, mGlareAmount, 1.0f ) );

	// Optionally compute anamorphic flares
	if ( mAnamorphicPasses > 0 )
		computeAnamorphicFlares( nLastLayer );
}

//-----------------------------------------------------------------------------
// Name : updateCache() (Protected)
/// <summary>
/// Updates the cache with the current down sampled glare.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::updateCache( size_t level )
{
	// Get the active render view
	cgRenderView * activeView = mDriver->getActiveRenderView();

	// Get the current target we are going to update
	const cgRenderTargetHandle & currTarget = mResampleChain0->getLevel( level );

	const cgImageInfo & info = currTarget->getInfo();

	// TEMPORARY...
	// ToDo: 6767 - We need a function in the render view to get a surface by actual size.
	cgSize viewSize     = activeView->getSize();
	float  scalarWidth  = ( ((float)info.width )  / (float)viewSize.width );
	float  scalarHeight = ( ((float)info.height - 0.1f ) / (float)viewSize.height );

	// Get the prior cached values
	cgString cacheName = cgString::format( _T("Glare::Cache_L%i_%i"), level, activeView->getReferenceId() );
	const cgRenderTargetHandle & cacheTarget = activeView->getRenderSurface( info.format, scalarWidth, scalarHeight, info.multiSampleType, info.multiSampleQuality, cacheName );

	// If the cache target size doesn't match, bail for now (REMOVE THIS AFTER ABOVE GETS FIXED!)
	const cgImageInfo & cacheinfo = cacheTarget->getInfo();
	if ( cacheinfo.width != info.width || cacheinfo.height != info.height )
	{
		printf("FAILED - %ix%i vs. %ix%i (%.4f, %.4f)\n", cacheinfo.width, cacheinfo.height, info.width, info.height, scalarWidth, scalarHeight );
		return;
	}

	// Setup states
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mAlphaBlendState );

	// Set shader constants
	int glareStepIndex = getGlareStepIndex( level );
	mUpdateCacheConfig.blendAmount = mSteps[ glareStepIndex ].cacheBlendAmount;
	mUpdateCacheConfig.blendRate   = mSteps[ glareStepIndex ].cacheBlendRate > 0.0f ? 1.0f / mSteps[ glareStepIndex ].cacheBlendRate : 0.0f;
	mUpdateCacheConstants->updateBuffer( 0, 0, &mUpdateCacheConfig );
	mDriver->setConstantBufferAuto( mUpdateCacheConstants );

	// Set the previous levels
	mPointSampler->apply( cacheTarget );

	// Select shaders
	if ( !selectClipQuadVertexShader() ||
		!mGlareShader->selectPixelShader( _T("updateCache") ) )
		return;

	// Draw to new current
	drawClipQuad( currTarget );

	// Copy current to cache
	processColorImage( currTarget, cacheTarget, cgImageOperation::CopyRGBA );
}

//-----------------------------------------------------------------------------
// Name : getGlareStepIndex() (Protected)
/// <summary>
/// Maps the specified mip / resample chain level index to the associated
/// element in the glare step array. Otherwise, 0xFFFFFFFF is returned.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgGlareProcessor::getGlareStepIndex( size_t levelIndex )
{
	for ( size_t i = 0; i < mSteps.size(); ++i )
	{
		if ( mSteps[ i ].levelIndex == levelIndex )
			return i;

	} // Next step

	// No user data found
	return -1;
}

//-----------------------------------------------------------------------------
// Name : setCacheValues() (Protected)
/// <summary>
/// Updates the values for cache blending
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setCacheValues( cgFloat amount, cgFloat rate )
{
	mUpdateCacheConfig.blendAmount = amount;
	mUpdateCacheConfig.blendRate   = 1.0f / rate;
}

//-----------------------------------------------------------------------------
// Name : setGlareAmount() (Protected)
/// <summary>
/// Sets the overall intensity of the glare
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setGlareAmount( cgFloat amt )
{
	mGlareAmount = amt;
}

//-----------------------------------------------------------------------------
// Name : setToneMapper() (Protected)
/// <summary>
/// Gives the glare access to the tonemapper.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setToneMapper( cgToneMapProcessor * pToneMapper )
{
	mToneMapper = pToneMapper;
}


////////////////////////////////////////////////////////////////////////////////
// ILR
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Name : setILRElements() (Protected)
/// <summary>
/// Sets the ILR elements array, which controls the behavior of individual flares.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRElements( const ILRElementArray & elements )
{
	mILRElements = elements;
}

//-----------------------------------------------------------------------------
// Name : setILRBrightThreshold() (Protected)
/// <summary>
/// Sets the minimum luminance (LDR) that determines which scene pixels are used for flares.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRBrightThreshold( cgFloat minimum )
{
	mILRBrightThreshold = minimum;
}

//-----------------------------------------------------------------------------
// Name : setILRContrast() (Protected)
/// <summary>
/// Optional contrast for pixel color used during the bright pass. Only works in LDR mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRContrast( cgFloat contrast )
{
	mILRFlareContrast = contrast;
}

//-----------------------------------------------------------------------------
// Name : setILRDepthRange() (Protected)
/// <summary>
/// The depth range of pixels (in meters) that can contribute to the ILR effect.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRDepthRange( cgFloat minimum, cgFloat maximum )
{
	mILRDistanceRange.x = minimum;
	mILRDistanceRange.y = maximum;
}

//-----------------------------------------------------------------------------
// Name : setILRLowResData() (Protected)
/// <summary>
/// Filtering data for the low resolution flares.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRLowResData( cgInt32 targetPadding, cgInt32 filterPasses, cgInt32 filterSize, cgFloat filterFactor )
{
	mILRLowTargetPadding = targetPadding;
	mILRLowFilter = cgBlurOpDesc( filterPasses, filterSize, filterFactor );
}

//-----------------------------------------------------------------------------
// Name : setILRHighResData() (Protected)
/// <summary>
/// Filtering data for the high resolution flares.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setILRHighResData( cgInt32 targetPadding, cgInt32 filterPasses, cgInt32 filterSize, cgFloat filterFactor )
{
	mILRHighTargetPadding = targetPadding;
	mILRHighFilter = cgBlurOpDesc( filterPasses, filterSize, filterFactor );
}

//-----------------------------------------------------------------------------
// Name : executeILR() (Protected)
/// <summary>
/// Runs the ILR post process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGlareProcessor::executeILR( const cgRenderTargetHandle & target, const cgTextureHandle & depth, cgResampleChain * chain0, cgResampleChain * chain1, bool hdr, bool materialAlphaMask )
{
	// Glare shader must be valid and loaded at this point.
	if ( !mGlareShader.getResource(true) || !mGlareShader.isLoaded() )
		return false;

	// Store operation configuration.
	mOperationTarget       = target;
	mDepthBuffer           = depth;
	mResampleChain0        = chain0;
	mResampleChain1        = chain1;
	mHDRILR                = hdr;
	mILRMaterialAlphaMask  = materialAlphaMask;

	// Run the ILR process
	internalLensReflection();

	// Don't retain a reference to the targets.
	mOperationTarget.close();
	mDepthBuffer.close();

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : internalLensReflection()
// Desc : Post-processes the rendered scene data in order to apply an internal
//        lens reflection effect.
//-----------------------------------------------------------------------------
void cgGlareProcessor::internalLensReflection( )
{
	cgVector2 highScale( 1, 1 ), lowScale( 1, 1 );

	// Create a viewport for updating ILR
	cgViewport viewport;
	viewport.minimumZ = 0.0f;
	viewport.maximumZ = 1.0f;

	//////////////////////////////////////////
	// Main Bright Pass + 1st Downsample/Blur
	//////////////////////////////////////////

	// First we'll need to perform the bright pass into the high detail target.
	// We render into the central area of the high detail target with the required
	// padding in order to ensure any applied blur is not clipped out. First we
	// need to set the high detail target into which we will be rendering and clear it.

	// Set shader constants
	mILRBrightPassConfig.brightThreshold = mILRBrightThreshold;
	mILRBrightPassConfig.flareContrast   = mILRFlareContrast;
	mILRBrightPassConfig.distanceAttenScaleBias.x = 1.0f / (mILRDistanceRange.y - mILRDistanceRange.x);
	mILRBrightPassConfig.distanceAttenScaleBias.y = -mILRDistanceRange.x / (mILRDistanceRange.y - mILRDistanceRange.x);
	mILRBrightPassConstants->updateBuffer( 0, 0, &mILRBrightPassConfig );
	mDriver->setConstantBufferAuto( mILRBrightPassConstants );

	// Begin rendering to target
	if ( mDriver->beginTargetRender( mResampleChain0->getLevel( 2 ) ) )
	{
		// Bind relevant render targets for reading in this pass.
		mLinearSampler->apply( mResampleChain0->getLevel( 1 ) );
		mDepthSampler->apply( mDepthBuffer );

		// Setup states
		mDriver->setDepthStencilState( mDisabledDepthState );
		mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
		mDriver->setBlendState( mDefaultRGBABlendState );

		// Clear the target
		mDriver->clear( cgClearFlags::Target, 0, 1, 0 );

		// With the target set and cleared, now we must adjust the driver's current
		// viewport (originally set during BeginTargetRender()) based on the specified
		// padding so that we can blur as required.
		if ( mILRHighTargetPadding > 0 )
		{
			viewport = mDriver->getViewport( );

			// First save the adjustment scale that will be used to blow
			// the image back up to its original size during rendering.
			cgFloat vpWidth   = (cgFloat)viewport.width;
			cgFloat vpHeight  = (cgFloat)viewport.height;
			cgFloat adjWidth  = (cgFloat)viewport.width  - (mILRHighTargetPadding * 2);
			cgFloat adjHeight = (cgFloat)viewport.height - (mILRHighTargetPadding * 2);
			highScale.x       = vpWidth  / adjWidth;
			highScale.y       = vpHeight / adjHeight;

			// Adjust the viewport and set back to the driver.
			viewport.x      += mILRHighTargetPadding;
			viewport.y      += mILRHighTargetPadding;
			viewport.width  -= mILRHighTargetPadding * 2;
			viewport.height -= mILRHighTargetPadding * 2;

			mDriver->setViewport( &viewport );

		} // End if padding required

		// If we have a tone mapper, set the luminance data
		cgInt32 toneMappingMethod = -1;
		if ( mHDRILR )
		{
			toneMappingMethod = cgToneMapProcessor::Filmic;
			if ( mToneMapper )
			{
				// Get the current luminance for the scene. 
				// ToDo: 6767 - Once tonemapper is fixed up properly, remove its call to getLuminanceData since we'll be fetching directly here
				//cgRenderView * activeView = mDriver->getActiveRenderView();
				//cgString bufferName = cgString::format( _T("Tonemapper::LuminanceCurr1x1_%i"), activeView->getReferenceId() );
				//mLuminanceCurrTarget = activeView->getRenderSurface( bufferName ); 
				mLuminanceSampler->apply( mToneMapper->getLuminanceData() );
			}
		}

		// Select shaders
		if ( !selectClipQuadVertexShader() ||
			 !mGlareShader->selectPixelShader( _T("flareBrightPass"), mILRFlareContrast != 0.0f && !mHDRILR, mILRMaterialAlphaMask, toneMappingMethod ) )
			return;

		// Draw quad
		drawClipQuad();

		// End rendering
		mDriver->endTargetRender();

	}  // End if success

	// We should now have fully populated the high detail target, so we can apply a filter.
	if ( mILRHighFilter.passCount > 0 )
		blur( mResampleChain0->getLevel( 2 ), mResampleChain1->getLevel( 2 ), mILRHighFilter );

	//////////////////////////////////////////
	// 2nd Downsample/Blur
	//////////////////////////////////////////

	// Setup states
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mDefaultRGBABlendState );

	// Bind relevant render targets for reading in this pass.
	mLinearSampler->apply( mResampleChain0->getLevel( 2 ) );

	// Begin rendering to target
	if ( mDriver->beginTargetRender( mResampleChain0->getLevel( 3 ) ) )
	{
		// Clear the target
		mDriver->clear( 0, NULL, cgClearFlags::Target, 0, 1, 0 );

		// With the target set and cleared, now we must adjust the driver's current
		// viewport (originally set during BeginTargetRender()) based on the specified
		// padding so that we can blur as required.
		if ( mILRLowTargetPadding > 0 )
		{
			viewport = mDriver->getViewport( );

			// First save the adjustment scale that will be used to blow
			// the image back up to its original size during rendering.
			cgFloat vpWidth   = (cgFloat)viewport.width;
			cgFloat vpHeight  = (cgFloat)viewport.height;
			cgFloat adjWidth  = (cgFloat)viewport.width  - (mILRLowTargetPadding * 2);
			cgFloat adjHeight = (cgFloat)viewport.height - (mILRLowTargetPadding * 2);
			highScale.x       = vpWidth  / adjWidth;
			highScale.y       = vpHeight / adjHeight;

			// Adjust the viewport and set back to the driver.
			viewport.x      += mILRLowTargetPadding;
			viewport.y      += mILRLowTargetPadding;
			viewport.width  -= mILRLowTargetPadding * 2;
			viewport.height -= mILRLowTargetPadding * 2;

			mDriver->setViewport( &viewport );

		} // End if padding required

		// Select shaders
		if ( !selectClipQuadVertexShader() ||
			 !mGlareShader->selectPixelShader( _T("flareDownsample") ) )
			return;

		// Draw quad
		drawClipQuad();

		// End rendering
		mDriver->endTargetRender( );

	}  // End if success

	// We should now have fully populated the low detail target, so we can apply a filter.
	if ( mILRLowFilter.passCount > 0 )
		blur( mResampleChain0->getLevel( 3 ), mResampleChain1->getLevel( 3 ), mILRLowFilter );

	//////////////////////////////////////////
	// Internal Lens Reflection (Additive)
	//////////////////////////////////////////

	// In HDR, we'll do a half resolution compositing to reduce bandwidth.
	bool halfResComposite = mHDRILR;

	// Setup states
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mAdditiveRGBABlendState );

	// Set the 1D texture that allows for adjusting flare color.
	mFlareSampler->apply( mFlareColor );

	// Begin rendering to target
	if ( mDriver->beginTargetRender( halfResComposite ? mResampleChain0->getLevel( 1 ) : mOperationTarget ) )
	{
		// If compositing at half resolution, clear the target to 0
		if ( halfResComposite )
			mDriver->clear( 0, NULL, cgClearFlags::Target, 0, 1, 0 );

		// Record half viewport sizes to aid in quad construction.
		viewport = mDriver->getViewport( );
		float halfWidth  = viewport.width  * 0.5f;
		float halfHeight = viewport.height * 0.5f;

		// Select shaders
		if ( !mGlareShader->selectPixelShader( _T("flareDraw") ) )
			return;

		// Set the vertex format
		mDriver->setVertexFormat( mILRVertexFormat );

		// Process the two different types of flares.
		for ( cgUInt32 i = 0; i < 2; ++i )
		{
			// Is this a high detail pass?
			bool highDetail = (i == 1) ? true : false;

			// We'll render the low detail ones first followed by any high 
			// detail ones to save texture switches.
			if ( highDetail )
				mLinearSampler->apply( mResampleChain0->getLevel( 2 ) );
			else
				mLinearSampler->apply( mResampleChain0->getLevel( 3 ) );

			// For each simulated lens element.
			for ( cgUInt32 j = 0; j < mILRElements.size(); ++j )
			{
				// Skip if this is not an element with the required detail.
				if ( mILRElements[j].flareHighDetail != highDetail )
					continue;

				// Supply required shader parameters for this particular flare
				mILRCompositePassConfig.flareDelta = mILRElements[j].flareDelta;
				mILRCompositePassConfig.flareAlpha = mILRElements[j].flareAlpha;
				mILRCompositePassConfig.flareSlice = ( (-mILRElements[j].flareDelta + 1.0f) * 0.5f );
				mILRCompositePassConstants->updateBuffer( 0, 0, &mILRCompositePassConfig );
				mDriver->setConstantBufferAuto( mILRCompositePassConstants );

				// Construct a quad of the appropriate size
				float scaleU = mILRElements[j].flareDelta * ((highDetail) ? highScale.x : highScale.x * lowScale.x);
				float scaleV = mILRElements[j].flareDelta * ((highDetail) ? highScale.y : highScale.y * lowScale.y);

				// Fill vertices
				cgScreenVertex Vertices[4];
				Vertices[0].position	  = cgVector4( halfWidth - (halfWidth * scaleU), halfHeight - (halfHeight * scaleV), 0, 1 );
				Vertices[1].position	  = cgVector4( halfWidth + (halfWidth * scaleU), halfHeight - (halfHeight * scaleV), 0, 1 );
				Vertices[2].position	  = cgVector4( halfWidth + (halfWidth * scaleU), halfHeight + (halfHeight * scaleV), 0, 1 );
				Vertices[3].position	  = cgVector4( halfWidth - (halfWidth * scaleU), halfHeight + (halfHeight * scaleV), 0, 1 );
				Vertices[0].textureCoords = cgVector2( 0, 0 );
				Vertices[1].textureCoords = cgVector2( 1, 0 );
				Vertices[2].textureCoords = cgVector2( 1, 1 );
				Vertices[3].textureCoords = cgVector2( 0, 1 );

				// Draw the quad
				mDriver->drawPrimitiveUP( cgPrimitiveType::TriangleFan, 2, Vertices );

			} // Next lens element

		} // Next detail level

		// Finish up
		mDriver->endTargetRender();

	} // Done rendering to target
	
	// If compositing at half-resolution, add to final buffer
	if( halfResComposite )
		processColorImage( mResampleChain0->getLevel( 1 ), mOperationTarget, cgImageOperation::AddRGB );
}

//-----------------------------------------------------------------------------
// Name : setAnamorphicData() (Protected)
/// <summary>
/// Set the inputs for the anamorphic flare post-process effect.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::setAnamorphicData( cgInt32 numPasses, cgInt32 radius, cgFloat intensity, const cgVector3 & flareColor, cgFloat edgeScale, cgFloat edgeBias )
{
	mAnamorphicPasses                 = numPasses;
	mAnamorphicRadius                 = radius;
	mAnamorphicIntensity              = intensity;
	mAnamorphicConfig.flareColor      = flareColor;
	mAnamorphicConfig.flareCoordScale = edgeScale;
	mAnamorphicConfig.flareCoordBias  = edgeBias;
}

//-----------------------------------------------------------------------------
// Name : computeAnamorphicFlares() (Protected)
/// <summary>
/// Procedurally computed anamorphic lens flare effect.
/// Note: This is a work in progress and is just an initial test of some ideas. 
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::computeAnamorphicFlares( size_t nLayer )
{
	// Get the source layer
	cgRenderTargetHandle target0 = mResampleChain0->getLevel( nLayer );
	cgRenderTargetHandle target1 = mResampleChain1->getLevel( nLayer );
	cgRenderTargetHandle currSrc, currDst;

	// Can we use linear sampling?
	const cgRenderTarget * pTarget = target0.getResourceSilent();
	bool supportsLinearSampling = (pTarget) ? pTarget->supportsLinearSampling() : false;

	// Select the vertex shader
	if ( !selectClipQuadVertexShader() )
		return;

	// Set shader constants
	mAnamorphicConfig.flarePassScale = 1.0f / float(mAnamorphicPasses);
	mAnamorphicConstants->updateBuffer( 0, 0, &mAnamorphicConfig );
	mDriver->setConstantBufferAuto( mAnamorphicConstants );

	// Setup states
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mDefaultRGBABlendState );

	// Ping-pong over N passes
	for ( cgInt32 i = 0; i < mAnamorphicPasses; i++ )
	{
		currSrc = (i % 2) ? target0 : target1;
		currDst = (i % 2) ? target1 : target0;

		// Set the texture and sampler states
		if ( supportsLinearSampling )
			mLinearSampler->apply( currSrc );
		else
			mPointSampler->apply( currSrc );

		// Compute pass data and select shaders
		int passDistance = (int)powf( mAnamorphicPasses > 3 ? 2.0f : 4.0f, cgFloat( i + 1 ) );
		int passFalloff  = (int)powf( mAnamorphicPasses > 3 ? 2.0f : 4.0f, cgFloat( i + 1 ) ) * 4;
		if ( !mGlareShader->selectPixelShader( _T("anamorphicFlare"), mAnamorphicRadius, passDistance, passFalloff, i == (mAnamorphicPasses - 1) ) )
			return;

		if ( mDriver->beginTargetRender( currDst, cgDepthStencilTargetHandle::Null ) )
		{
			mDriver->drawClipQuad( );
			mDriver->endTargetRender( );
		}

	} // Next pass

	// Add final 
	processColorImage( currDst, mOperationTarget, cgImageOperation::ColorScaleAddRGBA, cgColorValue( mAnamorphicIntensity, mAnamorphicIntensity, mAnamorphicIntensity, 1 ) );
}
