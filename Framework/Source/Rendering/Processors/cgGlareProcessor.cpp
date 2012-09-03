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
    mPreviousCacheSampler   = CG_NULL;
    mUseCache               = true;

    // Setup configuation defaults.
    mBrightThreshold = cgRangeF(0,0);
    mHDREnabled      = false;
    mBrightPassConfig.brightRangeScale  = 1.0f;
    mBrightPassConfig.brightRangeBias   = 0.0f;
    mBrightPassConfig.textureSizeRecip  = cgVector2(0,0);
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
    mUpSampleConstants.close();
    mOperationTarget.close();
    mGlarePrevious.close();
    mGlareCurrent.close();

    // Release memory
    if ( mPointSampler )
        mPointSampler->scriptSafeDispose();
    if ( mLinearSampler )
        mLinearSampler->scriptSafeDispose();
    if ( mPreviousCacheSampler ) 
        mPreviousCacheSampler->scriptSafeDispose();
    mSteps.clear();

    // Clear variables
    mResampleChain0         = CG_NULL;
    mResampleChain1         = CG_NULL;
    mPointSampler           = CG_NULL;
    mLinearSampler          = CG_NULL;
    mPreviousCacheSampler   = CG_NULL;
    mUseCache               = true;
    
    // Setup configuation defaults.
    mBrightThreshold = cgRangeF(0,0);
    mHDREnabled      = false;
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
    if ( !resources->createConstantBuffer( &mUpSampleConstants, mGlareShader, _T("cbUpSample"), cgDebugSource() ) )
        return false;
    cgAssert( mBrightPassConstants->getDesc().length == sizeof(_cbBrightPass) );
    cgAssert( mUpSampleConstants->getDesc().length == sizeof(_cbUpSample) );

    // Build buffers to cache average glare values.
    cgImageInfo description;
    description.width       = 1;
    description.height      = 1;
    description.mipLevels   = 1;

    // Select an appropriate floating point format.
    const cgBufferFormatEnum & formats = resources->getBufferFormats();
    description.format = formats.getBestFormat( cgBufferType::RenderTarget, cgFormatSearchFlags::FloatingPoint | cgFormatSearchFlags::FullPrecisionFloat | 
                                                                            cgFormatSearchFlags::HalfPrecisionFloat | cgFormatSearchFlags::FourChannels );

    // Create the targets.
    if ( !resources->createRenderTarget( &mGlarePrevious, description, 0, _T("Core::Glare::PreviousAverage"), cgDebugSource() ) )
        return false;
    if ( !resources->createRenderTarget( &mGlareCurrent, description, 0, _T("Core::Glare::CurrentAverage"), cgDebugSource() ) )
        return false;

    // Create samplers
    mPointSampler = resources->createSampler( _T("Source"), mGlareShader );
    mLinearSampler = resources->createSampler( _T("Source"), mGlareShader );
    mPreviousCacheSampler = resources->createSampler( _T("Previous"), mGlareShader );
    mPointSampler->setStates( mSamplers.point->getStates() );
    mLinearSampler->setStates( mSamplers.linear->getStates() );
    mPreviousCacheSampler->setStates( mSamplers.linear->getStates() ); // ToDo: 6767 - uses linear states, but no test was done to see if linear is supported on floating point format.
    
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
bool cgGlareProcessor::execute( const cgRenderTargetHandle & target, cgResampleChain * chain0, cgResampleChain * chain1, bool useCache )
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

    // Store operation configuration.
    mOperationTarget = target;
    mResampleChain0  = chain0;
    mResampleChain1  = chain1;
    mUseCache = useCache;

    // Run the bright pass on the specified target.
    brightPass();

    // Down-sample and blur
    downsampleAndBlur();

    // Up-sample and accumulate
    composite();

    // Don't retain a reference to the target.
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
    // Setup depth-stencil state (depth always disabled for image processing)
    // and rasterizer state (default).
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set blend states
    mDriver->setBlendState( mDefaultRGBABlendState );

    // Set the texture and sampler states
    mPointSampler->apply( mOperationTarget );

    // Set shader constants
    mBrightPassConstants->updateBuffer( 0, 0, &mBrightPassConfig );
    mDriver->setConstantBufferAuto( mBrightPassConstants );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
         !mGlareShader->selectPixelShader( _T("brightPass"), mHDREnabled, true ) )
        return;

    // Draw to 1/4 size target
    drawClipQuad( mResampleChain0->getLevel( 1 ) );
}

//-----------------------------------------------------------------------------
// Name : downsampleAndBlur() (Protected)
/// <summary>
/// Down samples and blurs the thresholded texture
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::downsampleAndBlur( )
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
        else
        {
            // If caching was requested, the user must indicate an intensity value 
            // for the lowest mip level otherwise we need to disable the cache request
            if ( i == (levelCount - 1) && mUseCache )
                mUseCache = false;
        
        } // End if no step data

    } // Next level

    // Down sample and blur the targets
    for ( cgInt32 i = 1; i <= maximumLevel; ++i )
    {
        // Blur current level if user data is available
        if ( stepIndices[ i ] != 0xFFFFFFFF )
        {
            if ( mSteps[ stepIndices[ i ] ].blurOp.passCount > 0 )
                blur( mResampleChain0->getLevel( i ), mResampleChain1->getLevel( i ), mSteps[ stepIndices[ i ] ].blurOp );

        } // End if user info available

        // Down sample to next level
        if ( i != maximumLevel )
            downSample( mResampleChain0->getLevel( i ), mResampleChain0->getLevel( i + 1 ) );

    } // Next level

    // Update the cache
    if ( mUseCache )
        updateCache( );
}

//-----------------------------------------------------------------------------
// Name : updateCache() (Protected)
/// <summary>
/// Updates the cache with the current down sampled glare.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::updateCache( )
{
    // Setup states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( mDefaultRGBABlendState );

    // Set the current and previous levels
    mLinearSampler->apply( mResampleChain0->getLevel( mResampleChain0->getLevelCount() - 1 ) );
    mPreviousCacheSampler->apply( mGlarePrevious );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mGlareShader->selectPixelShader( _T("updateCache") ) )
        return;

    // Draw to new current
    drawClipQuad( mGlareCurrent );

    // Replace previous with current for next frame
    processColorImage( mGlareCurrent, mGlarePrevious, cgImageOperation::CopyRGBA );
}

//-----------------------------------------------------------------------------
// Name : composite()
/// <summary>
/// Merges the blurred results and additively blends them on top of the 
/// original image.
/// </summary>
//-----------------------------------------------------------------------------
void cgGlareProcessor::composite( )
{
    _cbUpSample upSampleConfig;

    // Setup states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    mDriver->setBlendState( mAdditiveRGBABlendState );

    // Select shaders
    if ( !selectClipQuadVertexShader() ||
        !mGlareShader->selectPixelShader( "upSample" ) )
        return;

    // Up sample and combine the levels from bottom to top.
    cgInt maximumLevel = mResampleChain0->getLevelCount() - 1;
    cgInt stepCount = (cgInt)mSteps.size();
    for ( cgInt i = (stepCount-1); i >= 0; --i )
    {
        // Get information for this level 
        cgInt sourceIndex = mSteps[ i ].levelIndex;
        upSampleConfig.sourceIntensity = mSteps[ i ].intensity;

        // If this up sample is the final one, add results to the main 
        // target (set alpha to 0 for future processes)
        if ( i == 0 )
        { 
            forceLinearSampling( true );

            // If we have only 1 blur level just scale and add
            if ( stepCount == 1 )
                processColorImage( mResampleChain0->getLevel( sourceIndex ), mOperationTarget, cgImageOperation::ColorScaleAddRGBSetAlpha, 
                                   cgColorValue( upSampleConfig.sourceIntensity, upSampleConfig.sourceIntensity, upSampleConfig.sourceIntensity, 0 ) );
            else
                processColorImage( mResampleChain0->getLevel( sourceIndex ), mOperationTarget, cgImageOperation::AddRGBSetAlpha, 
                                   cgColorValue( 0, 0, 0, 0 ) );
            
            forceLinearSampling( false );
        
        } // End if final resample
        else
        {
            cgInt destinationIndex = mSteps[ i - 1 ].levelIndex;
            upSampleConfig.destinationIntensity = mSteps[ i - 1 ].intensity;

            // If we've already processed the bottom level, its intensity has already
            // been factored into the running sum. Thus the source intensity will now be 1.
            if ( i != (stepCount-1) )
                upSampleConfig.sourceIntensity = 1.0f;

            // Set the texture and sampler states
            if ( mUseCache && sourceIndex == maximumLevel ) 
                mLinearSampler->apply( mGlareCurrent );
            else			
                mLinearSampler->apply( mResampleChain0->getLevel( sourceIndex ) );

            // Set the up sample constants
            mUpSampleConstants->updateBuffer( 0, 0, &upSampleConfig );
            mDriver->setConstantBufferAuto( mUpSampleConstants );

            // Draw to the destination buffer
            drawClipQuad( mResampleChain0->getLevel( destinationIndex ) );
        
        } // End if other resample
    
    } // Next step
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