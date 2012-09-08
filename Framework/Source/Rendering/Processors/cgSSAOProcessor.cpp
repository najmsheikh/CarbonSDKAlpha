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
// Name : cgSSAOProcessor.cpp                                                //
//                                                                           //
// Desc : Image processing class designed to compute and apply ambient       //
//        occlusion for a scene in screen space as a post process.           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSSAOProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgSSAOProcessor.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgTexture.h>
#include <World/Objects/cgCameraObject.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgSSAOProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSSAOProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSSAOProcessor::cgSSAOProcessor(  )
{
    // Initialize variables to sensible defaults
    mDepthSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mRotationSampler    = CG_NULL;
    
    // Setup configuration defaults.
    mMethod                         = HemisphereAO;
    mSampleCount                    = 16;
    mRadiusCount                    = 1;
    mOperationData.radii            = cgVector4( 0.5f, 0, 0, 0 );
    mOperationData.powers           = cgVector4( 1.0f, 0, 0, 0 );
    mOperationData.maximumDistances = cgVector4( 0, 0, 0, 0 );
    mOperationData.radiusBias       = 0.004f;
    mOperationData.depthFalloff     = 1.2f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSSAOProcessor::~cgSSAOProcessor( )
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
void cgSSAOProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mSSAOShader.close();
    mSSAOConstants.close();
    mRandomRotation180.close();
    mRandomRotation360.close();
    
    // Release memory
    if ( mDepthSampler )
        mDepthSampler->scriptSafeDispose();
    if ( mNormalSampler )
        mNormalSampler->scriptSafeDispose();
    if ( mRotationSampler )
        mRotationSampler->scriptSafeDispose();
    
    // Clear variables
    mDepthSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mRotationSampler    = CG_NULL;
    
    // Setup configuration defaults.
    mMethod                         = HemisphereAO;
    mSampleCount                    = 16;
    mRadiusCount                    = 1;
    mOperationData.radii            = cgVector4( 0.5f, 0, 0, 0 );
    mOperationData.powers           = cgVector4( 1.0f, 0, 0, 0 );
    mOperationData.maximumDistances = cgVector4( 0, 0, 0, 0 );
    mOperationData.radiusBias       = 0.004f;
    mOperationData.depthFalloff     = 1.2f;
    
    // Dispose base if requested.
    if ( disposeBase )
        cgImageProcessor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSSAOProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mSSAOShader, _T("sys://Shaders/SSAO.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mSSAOConstants, mSSAOShader, _T("cbSSAO"), cgDebugSource() ) )
        return false;
    cgAssert( mSSAOConstants->getDesc().length == sizeof(_cbSSAO) );

    // Create samplers
    mDepthSampler = resources->createSampler( _T("Depth"),    mSSAOShader );
    mDepthSampler->setStates( mSamplers.point->getStates() );
    mNormalSampler = resources->createSampler( _T("Normal"),   mSSAOShader );
    mNormalSampler->setStates( mSamplers.point->getStates() );
    mRotationSampler = resources->createSampler( _T("Rotation"), mSSAOShader );
    mRotationSampler->setStates( mSamplers.point->getStates() );
    mRotationSampler->setAddressU( cgAddressingMode::Wrap );
    mRotationSampler->setAddressV( cgAddressingMode::Wrap );

    // Load the rotation textures (180 and 360 rotations)
    if ( !resources->loadTexture( &mRandomRotation180, "sys://Textures/Rotation4x4_180.dds", 0, cgDebugSource() ) || 
         !resources->loadTexture( &mRandomRotation360, "sys://Textures/Rotation4x4_360.dds", 0, cgDebugSource() ) )
        return false; 

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : execute()
/// <summary>
/// Post-processes the rendered scene data in order to generate a screen space
/// ambient occlusion texture that can be applied to a rendered image.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSSAOProcessor::execute( cgCameraNode * activeCamera, const cgTextureHandle & depthSource, const cgTextureHandle & normalSource, const cgRenderTargetHandle & destination, const cgDepthStencilTargetHandle & depthStencilTarget )
{
    // SSAO shader must be valid and loaded at this point.
    if ( !mSSAOShader.getResource(true) || !mSSAOShader.isLoaded() )
        return false;

    // Depth source must be valid
    if ( !depthSource.isValid() )
        return false;

    // Is this an orthographic camera?
	bool orthographicCamera = (activeCamera->getProjectionMode() == cgProjectionMode::Orthographic);

	// Set depth-stencil state
	if ( depthStencilTarget.isValid() )
		mDriver->setDepthStencilState( mGreaterDepthState );
	else	
		mDriver->setDepthStencilState( mDisabledDepthState );
    
    // Set necessary states
	//mDriver->setBlendState( mDefaultAlphaOnlyBlendState );
    mDriver->setBlendState( cgBlendStateHandle::Null );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

	// Set the textures and sampler states
	mDepthSampler->apply( depthSource );
	mNormalSampler->apply( normalSource );
	mRotationSampler->apply( mRandomRotation360 );
	
    // Set shader constants
    const cgTexture * sourceTexture = depthSource.getResourceSilent();
    mOperationData.textureSize.x = (cgFloat)sourceTexture->getInfo().width;
    mOperationData.textureSize.y = (cgFloat)sourceTexture->getInfo().height;
    mOperationData.textureSize.z = 1.0f / mOperationData.textureSize.x;
    mOperationData.textureSize.w = 1.0f / mOperationData.textureSize.w;
    mSSAOConstants->updateBuffer( 0, 0, &mOperationData );
    mDriver->setConstantBufferAuto( mSSAOConstants );

    // Select shaders
    if ( !selectClipQuadVertexShader( true ) )
        return false;
	switch( mMethod )
	{
		case VolumetricAO:
	        if ( !mSSAOShader->selectPixelShader( _T("drawVolumetricAO"), mSampleCount, min( 2, mRadiusCount ), false, orthographicCamera ) )
                return false;
		    break;
		case ObscuranceAO:
	        if ( !mSSAOShader->selectPixelShader( _T("drawObscuranceAO"), mSampleCount, min( 2, mRadiusCount ), false, orthographicCamera ) )
                return false;
		    break;
		default:
	        if ( !mSSAOShader->selectPixelShader( _T("drawHemisphereAO"), mSampleCount, mRadiusCount, false, orthographicCamera ) )
                return false;
		    break;
	
    } // End switch method

	// Compute ambient occlusion in screen space
    cgDepthStencilTargetHandle oldDepthStencil = mDepthStencilTarget;
	setDepthStencilTarget( depthStencilTarget );
	drawClipQuad( destination );
	setDepthStencilTarget( oldDepthStencil );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setSamplingMethod()
/// <summary>
/// Set the approach used to sample and compute ambient occlusion results.
/// </summary>
//-----------------------------------------------------------------------------
void cgSSAOProcessor::setSamplingMethod( SSAOMethod method )
{
    mMethod = method;
}

//-----------------------------------------------------------------------------
// Name : setSampleCount()
/// <summary>
/// Set the total number of occlusion samples to take for each pixel
/// encountered in the source depth buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgSSAOProcessor::setSampleCount( cgInt32 samples )
{
    mSampleCount = samples;
}

//-----------------------------------------------------------------------------
// Name : setSampleRadii()
/// <summary>
/// Set the details for a maximum of 4 different scales at which to sample
/// occlusion.
/// </summary>
//-----------------------------------------------------------------------------
void cgSSAOProcessor::setSampleRadii( cgInt32 radiusCount, const cgVector4 & radii, const cgVector4 & powers, const cgVector4 & maximumDistances )
{
    mRadiusCount = radiusCount;
    mOperationData.radii = radii;
    mOperationData.powers = powers;
    mOperationData.maximumDistances = maximumDistances;
}

//-----------------------------------------------------------------------------
// Name : setBiasFactor()
/// <summary>
/// Set the adaptive biasing factor (based on reference depth) to help reduce 
/// shadowing from co-planar occluders.
/// </summary>
//-----------------------------------------------------------------------------
void cgSSAOProcessor::setBiasFactor( cgFloat bias )
{
    mOperationData.radiusBias = bias;
}

//-----------------------------------------------------------------------------
// Name : setDepthFalloff()
/// <summary>
/// Factor by which sampled occlusion contribution falls off over distance
/// from the source pixel.
/// </summary>
//-----------------------------------------------------------------------------
void cgSSAOProcessor::setDepthFalloff( cgFloat falloff )
{
    mOperationData.depthFalloff = falloff;
}