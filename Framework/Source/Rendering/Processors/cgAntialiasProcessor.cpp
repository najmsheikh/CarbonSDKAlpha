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
// Name : cgAntialiasProcessor.cpp                                           //
//                                                                           //
// Desc : Image processing class designed to remove aliasing artifacts from  //
//        a rendered image using post-process screen based filters.          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAntialiasProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgAntialiasProcessor.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgTexture.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <World/Objects/cgCameraObject.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgAntialiasProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAntialiasProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAntialiasProcessor::cgAntialiasProcessor(  )
{
    // Initialize variables to sensible defaults
	mAnisotropic4xSampler = CG_NULL;
	mVelocitySampler      = CG_NULL;
	mImagePrevSampler	  = CG_NULL;
	mDepthSampler		  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAntialiasProcessor::~cgAntialiasProcessor( )
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
void cgAntialiasProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mAntialiasConstants.close();
    mAntialiasShader.close();

    // Release memory
    if ( mAnisotropic4xSampler )
        mAnisotropic4xSampler->scriptSafeDispose();
	if ( mVelocitySampler )
		mVelocitySampler->scriptSafeDispose();
	if ( mImagePrevSampler )
		mImagePrevSampler->scriptSafeDispose();
	if ( mDepthSampler )
		mDepthSampler->scriptSafeDispose();

    // Clear variables
    mAnisotropic4xSampler = CG_NULL;
	mVelocitySampler      = CG_NULL;
	mImagePrevSampler	  = CG_NULL;
	mDepthSampler		  = CG_NULL;
    
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
bool cgAntialiasProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mAntialiasShader, _T("sys://Shaders/Antialiasing.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mAntialiasConstants, mAntialiasShader, _T("cbAntialiasing"), cgDebugSource() ) )
        return false;
    cgAssert( mAntialiasConstants->getDesc().length == sizeof(_cbAntialiasing) );

    // Create samplers
	mVelocitySampler = resources->createSampler( _T("Velocity"), mAntialiasShader );
	mVelocitySampler->setStates( mSamplers.point->getStates() );

	mDepthSampler = resources->createSampler( _T("Depth"), mAntialiasShader );
	mDepthSampler->setStates( mSamplers.point->getStates() );

	mImagePrevSampler = resources->createSampler( _T("ImagePrev"), mAntialiasShader );
	mImagePrevSampler->setStates( mSamplers.point->getStates() );

	mAnisotropic4xSampler = resources->createSampler( _T("Image"), mAntialiasShader );
    mAnisotropic4xSampler->setMinificationFilter( cgFilterMethod::Anisotropic );
    mAnisotropic4xSampler->setMagnificationFilter( cgFilterMethod::Anisotropic );
    mAnisotropic4xSampler->setMipmapFilter( cgFilterMethod::Anisotropic );
    mAnisotropic4xSampler->setAddressU( cgAddressingMode::Clamp );
    mAnisotropic4xSampler->setAddressV( cgAddressingMode::Clamp );
    mAnisotropic4xSampler->setAddressW( cgAddressingMode::Clamp );
    mAnisotropic4xSampler->setMaximumAnisotropy( 4 );
    mAnisotropic4xSampler->setMaximumMipmapLOD( 0.0f );
    mAnisotropic4xSampler->setMinimumMipmapLOD( 0.0f );

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : executeFXAA()
/// <summary>
/// Post-processes the rendered scene data via FXAA in order to attempt to remove any 
/// aliasing that may be present in the image.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAntialiasProcessor::executeFXAA( const cgTextureHandle & source, const cgTextureHandle & velocity, const cgRenderTargetHandle & destination )
{
	// Shader must be valid and loaded at this point.
	if ( !mAntialiasShader.getResource(true) || !mAntialiasShader.isLoaded() )
		return false;

	// Get the source texture details.
	const cgTexture * sourceResource = source.getResourceSilent();
	if ( !sourceResource )
		return false;
	cgSize sourceSize = sourceResource->getSize();
	mAntialiasConfig.textureSize = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 
		1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );

	// Set necessary states
	if ( mApplyRasterizerState )
		mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	if ( mApplyBlendState )
		mDriver->setBlendState( mDefaultRGBABlendState );
	if ( mApplyDepthStencilState )
	{
		if ( mTestStencilBuffer )
			mDriver->setDepthStencilState( mReadStencilMaskState, mStencilRef );
		else
			mDriver->setDepthStencilState( mDisabledDepthState );

	} // End if apply states

	// Bind textures and set sampler state
	mSamplers.linear->apply( source );
	mVelocitySampler->apply( velocity );

	// Set constants
	mAntialiasConstants->updateBuffer( 0, 0, &mAntialiasConfig );
	mDriver->setConstantBufferAuto( mAntialiasConstants );

	// Select shaders
	if ( !mAntialiasShader->selectVertexShader( _T("transform"), false ) ||
		 !mAntialiasShader->selectPixelShader( _T("fxaa") ) )
		return false;

	// Composite the low res blurred results with the original source texture
	if ( mDriver->beginTargetRender( destination ) )
	{
		mDriver->drawScreenQuad( );
		mDriver->endTargetRender( );
	}

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : computePixelVelocity()
/// <summary>
/// Computes pixel velocities for temporal resolve.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAntialiasProcessor::computePixelVelocity( cgCameraNode * activeCamera, const cgTextureHandle & depth, cgDepthType::Base depthType, const cgRenderTargetHandle & velocity )
{
	// Shader must be valid and loaded at this point.
	if ( !mAntialiasShader.getResource(true) || !mAntialiasShader.isLoaded() )
		return false;

	// Set shader constants
	const cgTexture * sourceResource = depth.getResourceSilent();
	if ( !sourceResource )
		return false;
	cgSize sourceSize = sourceResource->getSize();
	mAntialiasConfig.textureSize = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );
	mAntialiasConfig.previousViewProjMatrix = activeCamera->getPreviousViewMatrix() * activeCamera->getPreviousProjectionMatrix();
	mAntialiasConfig.resolveMaxSpeed    = 0.05f;
	mAntialiasConfig.reprojectionWeight = 10.0f;
	mAntialiasConstants->updateBuffer( 0, 0, &mAntialiasConfig );
	mDriver->setConstantBufferAuto( mAntialiasConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	// Use a null vertex shader for all screen quad draws
	mAntialiasShader->selectVertexShader( cgVertexShaderHandle::Null );

	// Compute velocity
	if ( mAntialiasShader->selectPixelShader( _T("pixelVelocity"), depthType ) )
	{
		mDepthSampler->apply( depth );
		if ( mDriver->beginTargetRender( velocity ) )
		{
			mDriver->drawScreenQuad( );
			mDriver->endTargetRender( );
		}
	}

	// Success!
	return true;
}

//-----------------------------------------------------------------------------
// Name : temporalResolve()
/// <summary>
/// Blends previous and current frames of anti-aliased results.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAntialiasProcessor::temporalResolve( const cgTextureHandle & curr, const cgTextureHandle & prev, const cgTextureHandle & velocity, const cgRenderTargetHandle & destination )
{
	// Shader must be valid and loaded at this point.
	if ( !mAntialiasShader.getResource(true) || !mAntialiasShader.isLoaded() )
		return false;

	// Set shader constants
	const cgTexture * sourceResource = curr.getResourceSilent();
	if ( !sourceResource )
		return false;
	cgSize sourceSize = sourceResource->getSize();
	mAntialiasConfig.textureSize = cgVector4( (cgFloat)sourceSize.width, (cgFloat)sourceSize.height, 1.0f / (cgFloat)sourceSize.width, 1.0f / (cgFloat)sourceSize.height );
	mAntialiasConfig.resolveMaxSpeed    = 0.05f;
	mAntialiasConfig.reprojectionWeight = 10.0f;
	mAntialiasConstants->updateBuffer( 0, 0, &mAntialiasConfig );
	mDriver->setConstantBufferAuto( mAntialiasConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	// Use a null vertex shader for all screen quad draws
	mAntialiasShader->selectVertexShader( cgVertexShaderHandle::Null );

	// Bind textures and set sampler state
	mSamplers.linear->apply( curr );
	mImagePrevSampler->apply( prev );
	mVelocitySampler->apply( velocity );

	// Compute velocity
	if ( mAntialiasShader->selectPixelShader( _T("temporalResolve") ) )
	{
		if ( mDriver->beginTargetRender( destination ) )
		{
			mDriver->drawScreenQuad( );
			mDriver->endTargetRender( );
		}
	}

	// Success!
	return true;
}
