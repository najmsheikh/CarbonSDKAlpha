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

	// Setup configuration defaults.
	mBrightThreshold = cgRangeF(0,0);
	mBrightPassConfig.brightRangeScale  = 1.0f;
	mBrightPassConfig.brightRangeBias   = 0.0f;
	mBrightPassConfig.alphaMaskAmount   = 0.0f;
	mBrightPassConfig.textureSizeRecip  = cgVector2(0,0);
	mUpdateCacheConfig.blendAmount	    = 0.0f;
	mUpdateCacheConfig.blendRate        = 0.0f;
	mGlareAmount                        = 0;
	mAnamorphicFlares                   = false;

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

	mSteps.clear();

	// Clear variables
	mResampleChain0         = CG_NULL;
	mResampleChain1         = CG_NULL;
	mPointSampler           = CG_NULL;
	mLinearSampler          = CG_NULL;
	mLuminanceSampler       = CG_NULL;

	// Setup configuration defaults.
	mBrightThreshold = cgRangeF(0,0);
	mBrightPassConfig.brightRangeScale  = 1.0f;
	mBrightPassConfig.brightRangeBias   = 0.0f;
	mBrightPassConfig.textureSizeRecip  = cgVector2(0,0);

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

	cgAssert( mBrightPassConstants->getDesc().length == sizeof(_cbBrightPass) );
	cgAssert( mUpdateCacheConstants->getDesc().length == sizeof(_cbUpdateCache) );
	cgAssert( mAddLayerConstants->getDesc().length == sizeof(_cbAddLayer) );

	// Create samplers
	mPointSampler = resources->createSampler( _T("Source"), mGlareShader );
	mLinearSampler = resources->createSampler( _T("Source"), mGlareShader );
	mLuminanceSampler = resources->createSampler( _T("Luminance"), mGlareShader );

	mPointSampler->setStates( mSamplers.point->getStates() );
	mLinearSampler->setStates( mSamplers.linear->getStates() );
	mLuminanceSampler->setStates( mSamplers.point->getStates() );

	// Create our layer blending state
	cgBlendStateDesc blStates;
	blStates.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
	blStates.renderTarget[0].blendEnable           = true;
	blStates.renderTarget[0].sourceBlend           = cgBlendMode::One;
	blStates.renderTarget[0].destinationBlend      = cgBlendMode::SrcAlpha;
	resources->createBlendState( &mLayerBlendState, blStates, 0, cgDebugSource() );

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
			float I = mSteps[ stepIndices[ i ] ].intensity * mGlareAmount;

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
	processColorImage( mResampleChain0->getLevel( nLastLayer ), mOperationTarget, cgImageOperation::AddRGB );

	// Optionally compute anamorphic flares
	bool mAnamorphicFlares = false;
	if ( mAnamorphicFlares )
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
// Name : computeAnamorphicFlares() (Protected)
/// <summary>
/// Procedurally computed anamorphic lens flare effect.
/// Note: This is a work in progress and is just an initial test of some ideas. 
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::computeAnamorphicFlares( size_t nLayer )
{
	///////////////////////////////////////////
	// Start by running a custom bright pass for the merged bloom layer 
	///////////////////////////////////////////

	// Setup states
	mDriver->setDepthStencilState( mDisabledDepthState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mDefaultRGBABlendState );

	// Get the source layer
	const cgRenderTargetHandle & hSource = mResampleChain0->getLevel( nLayer );
	const cgRenderTargetHandle & hDest   = mResampleChain1->getLevel( nLayer );

	// Set the texture and sampler states
	const cgRenderTarget * pTarget = hSource.getResourceSilent();
	bool supportsLinearSampling = (pTarget) ? pTarget->supportsLinearSampling() : false;
	if ( supportsLinearSampling )
		mLinearSampler->apply( hSource );
	else
		mPointSampler->apply( hSource );

	// Start by taking a vertical average and running a bright pass on the end result
	// Set shader constants
	setBrightThreshold( 1, 2 ); // <- See note above for now. This will come from a proper input function once the effect is complete. This is for testing only.
	mBrightPassConstants->updateBuffer( 0, 0, &mBrightPassConfig );
	mDriver->setConstantBufferAuto( mBrightPassConstants );

	// Select shaders
	if ( !selectClipQuadVertexShader() ||
		!mGlareShader->selectPixelShader( _T("verticalBrightPass"), 50 ) )
		return;

	// Draw to 1/4 size target
	drawClipQuad( hDest );

	// Now run a set of blurs that favor the horizontal
	cgBlurOpDesc blurDesc( 2, 60, 50.0 );
	blurDesc.pixelRadiusV    = 1;
	blurDesc.distanceFactorV = 5.0f;
	blur( hDest, hSource, blurDesc );

	// Add final 
	processColorImage( hDest, mOperationTarget, cgImageOperation::ColorScaleAddRGBA, cgColorValue( 0.6f, 0.6f, 0.6f, 1 ) );
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
