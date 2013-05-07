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
// Name : cgMotionBlurProcessor.cpp                                          //
//                                                                           //
// Desc : Image processing class designed to apply a blur to a rendered      //
//        based on the amount and direction of motion of the camera and / or //
//        objects in the scene.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMotionBlurProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgMotionBlurProcessor.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <World/Objects/cgCameraObject.h>
#include <Math/cgMathUtility.h>
#include <Math/cgEulerAngles.h>


// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgMotionBlurProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMotionBlurProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMotionBlurProcessor::cgMotionBlurProcessor(  )
{
    // Initialize variables to sensible defaults
    mColorSampler           = CG_NULL;
    mDepthSampler           = CG_NULL;
	mVelocitySampler        = CG_NULL;
	mColorLowSampler        = CG_NULL;

    // Clear matrices
    cgMatrix::identity( mOperationData.interpolatedCameraMatrix );

	mOperationData.blurAmount          = 0.5f;
	mOperationData.maxSpeed            = 0.05f;

    // Setup configuration defaults.
	mBlurAmount             = 0.5f;
	mRotationalBlurAmt      = 1.0f;   
	mTranslationBlurAmt     = 0.0f;  

	mMinimumAngularVelocity = 10.0f;
	mMaximumAngularVelocity = 50.0f;
	mBlendPower             = 1.0f;

    mTargetRate             = 50;
    mAttenuationRates       = cgRangeF( 0, 0 );
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMotionBlurProcessor::~cgMotionBlurProcessor( )
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
void cgMotionBlurProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mMotionBlurShader.close();
    mMotionBlurConstants.close();
    
    // Release memory
    if ( mColorSampler )
        mColorSampler->scriptSafeDispose();
    if ( mDepthSampler )
        mDepthSampler->scriptSafeDispose();
	if ( mColorLowSampler )
		mColorLowSampler->scriptSafeDispose();
	if ( mVelocitySampler )
		mVelocitySampler->scriptSafeDispose();

    // Clear variables
    mColorSampler           = CG_NULL;
    mDepthSampler           = CG_NULL;
	mVelocitySampler        = CG_NULL;
	mColorLowSampler        = CG_NULL;
    mCameraData.clear();

    // Clear matrices
	cgMatrix::identity( mOperationData.interpolatedCameraMatrix );

	mOperationData.blurAmount     = 0.1f;
	mOperationData.maxSpeed       = 0.05f;
	mOperationData.compositeBlend = 100.0f;
    
    // Setup configuration defaults.
    mBlurAmount             = 0.5f;
    mRotationalBlurAmt      = 1.0f;   
    mTranslationBlurAmt     = 0.0f;  
    mTargetRate             = 50;
    mAttenuationRates       = cgRangeF( 0, 0 );
    
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
bool cgMotionBlurProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the glare processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mMotionBlurShader, _T("sys://Shaders/MotionBlur.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mMotionBlurConstants, mMotionBlurShader, _T("cbMotionBlur"), cgDebugSource() ) )
        return false;
    cgAssert( mMotionBlurConstants->getDesc().length == sizeof(_cbMotionBlur) );
    
    // Create samplers
    mColorSampler    = resources->createSampler( _T("Color"), mMotionBlurShader );
    mDepthSampler    = resources->createSampler( _T("Depth"), mMotionBlurShader );
	mVelocitySampler = resources->createSampler( _T("Velocity"), mMotionBlurShader );
	mColorLowSampler = resources->createSampler( _T("ColorLow"), mMotionBlurShader );

    mDepthSampler->setStates( mSamplers.point->getStates() );
    mColorSampler->setStates( mSamplers.linear->getStates() );
	mColorLowSampler->setStates( mSamplers.linear->getStates() );
	mVelocitySampler->setStates( mSamplers.point->getStates() );

	mColorSampler->setAddressU( cgAddressingMode::Border );
    mColorSampler->setAddressV( cgAddressingMode::Border );
    mColorSampler->setBorderColor( cgColorValue(0,0,0,0) );

	// Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : computePixelVelocity()
/// <summary>
/// Computes a pixel velocity buffer based on the current and previous cameras.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::computePixelVelocity( cgCameraNode * activeCamera, cgFloat timeDelta, const cgTextureHandle & sourceDepth, cgDepthType::Base depthType, const cgRenderTargetHandle & velocity )
{
	// Motion blur shader must be valid and loaded at this point.
	if ( !mMotionBlurShader.getResource(true) || !mMotionBlurShader.isLoaded() )
		return false;

	// Update motion blur data based on camera and elapsed time.
	if ( !update( activeCamera, timeDelta ) )
		return false;

	// Set shader constants
	mMotionBlurConstants->updateBuffer( 0, 0, &mOperationData );
	mDriver->setConstantBufferAuto( mMotionBlurConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	// Compute velocity
	if ( mMotionBlurShader->selectVertexShader( _T("transform") ) &&
         mMotionBlurShader->selectPixelShader( _T("cameraPixelVelocity"), depthType ) )
	{
		mDepthSampler->apply( sourceDepth );
        if ( mDriver->beginTargetRender( velocity, cgDepthStencilTargetHandle::Null ) )
		{
			mDriver->drawClipQuad( );
			mDriver->endTargetRender( );
		}
	}

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : execute()
/// <summary>
/// Post-processes the rendered scene data in order to apply an amount of blur
/// to the image based on player camera motion. This method returns false if
/// there was nothing to do, or true if a blur was applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::execute( cgInt32 nPasses,
									 const cgTextureHandle & sourceColor, 
									 const cgTextureHandle & sourceVelocity, 
									 const cgRenderTargetHandle & sourceColorLow, 
									 const cgRenderTargetHandle & sourceColorLowScratch, 
									 const cgRenderTargetHandle & destination )
{
	// Motion blur shader must be valid and loaded at this point.
	if ( !mMotionBlurShader.getResource(true) || !mMotionBlurShader.isLoaded() )
		return false;

	// Setup targets
	cgRenderTargetHandle target0 = sourceColorLowScratch;
	cgRenderTargetHandle target1 = sourceColorLow;

	// Set shader constants
	mMotionBlurConstants->updateBuffer( 0, 0, &mOperationData );
	mDriver->setConstantBufferAuto( mMotionBlurConstants );

	// Set necessary states
	mDriver->setBlendState( mDefaultRGBABlendState );
	mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setDepthStencilState( mDisabledDepthState );

	mVelocitySampler->apply( sourceVelocity );
	if ( mMotionBlurShader->selectVertexShader( _T("transform") ) &&
         mMotionBlurShader->selectPixelShader( _T("cameraMotionBlur") ) )
	{
		cgRenderTargetHandle currSrc = target1;
		cgRenderTargetHandle currDst = target0;

		// Ping-pong over N passes
		for ( cgInt32 i = 0; i < nPasses; i++ )
		{
			currSrc = (i % 2) ? target0 : target1;
			currDst = (i % 2) ? target1 : target0;
			mColorSampler->apply( currSrc );
            if ( mDriver->beginTargetRender( currDst, cgDepthStencilTargetHandle::Null ) )
			{
				mDriver->drawClipQuad( );
				mDriver->endTargetRender( );
			}

		} // Next pass

		// If a separate destination was provided, do not use blending
		bool useBlending = (sourceColor == destination);

		// Composite the low res blurred results with the original source texture
		if ( mMotionBlurShader->selectPixelShader( _T("cameraMotionBlurComposite"), useBlending ) )
		{
			if ( !useBlending )
			{
				mDriver->setBlendState( mDefaultRGBABlendState );
			}
			else
			{
				mDriver->setBlendState( mAlphaBlendState );
			}

			mColorSampler->apply( sourceColor );
			mColorLowSampler->apply( currDst );
			if ( mDriver->beginTargetRender( destination, cgDepthStencilTargetHandle::Null ) )
			{
				mDriver->drawClipQuad( );
				mDriver->endTargetRender( );
			}
		}
		
	} // End if shader ok

	// Success
	return true;
}

//-----------------------------------------------------------------------------
// Name : update()
/// <summary>
/// Updates the matrices used for motion blur. Ultimately computes the strength
/// of the motion blur effect [0, 1] along with the 'previous' view-projection
/// matrices which the shader will use to reconstruct a per-pixel motion vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMotionBlurProcessor::update( cgCameraNode * activeCamera, cgFloat timeDelta )
{
	// Get the camera's world, view, and projection matrices (compute inverse view as well)
	cgMatrix cameraWorld = activeCamera->getWorldTransform(false);
	cgMatrix cameraView  = activeCamera->getViewMatrix();
	cgMatrix cameraProj  = activeCamera->getProjectionMatrix();
	cgMatrix invCameraView;
	cgMatrix::inverse( invCameraView, cameraView );

    // Get the camera data for this camera.
    CameraData & data = mCameraData[activeCamera];

    // Increment the time since we last captured blur matrices
    data.currentRenderRate = (timeDelta > 0.0f) ? (1.0f / timeDelta) : 10000.0f;
    data.accumulatedTime  += timeDelta;

    // ToDo: Remove camera data if it has not been used in a while.

    // If enough time has elapsed such that we can sample new
    // matrices, then cycle the two key frames and capture new data.
	if ( data.accumulatedTime >= (1.0f / mTargetRate) )
	{
        // Cycle key frames.
        data.viewKeyMatrix1 = data.viewKeyMatrix2;

        // Capture current camera data.
        data.viewKeyMatrix2 = cameraWorld;

        // Setup for next capture
        data.accumulatedTime -= (1.0f/mTargetRate);

    } // End if time expired

	// Compute angular velocity
	cgEulerAngles euler0, euler1;
	euler0.fromMatrix( data.prevCameraMatrix, cgEulerAnglesOrder::YXZ );
	euler1.fromMatrix( cameraWorld, cgEulerAnglesOrder::YXZ );
	cgEulerAngles deltaEuler;
	deltaEuler.x = euler1.x - euler0.x;
	deltaEuler.y = euler1.y - euler0.y;
	deltaEuler.z = euler1.z - euler0.z;
	if ( deltaEuler.x > CGE_PI )
		deltaEuler.x -= CGE_PI * 2.0f;
	if ( deltaEuler.y > CGE_PI )
		deltaEuler.y -= CGE_PI * 2.0f;
	if ( deltaEuler.z > CGE_PI )
		deltaEuler.z -= CGE_PI * 2.0f;

	// Abs the values
	deltaEuler.x = fabs( deltaEuler.x );
	deltaEuler.y = fabs( deltaEuler.y );
	deltaEuler.z = fabs( deltaEuler.z );

	// Get the maximum angle difference
	cgFloat maxAngleDelta = max( deltaEuler.x, max( deltaEuler.y, deltaEuler.z ) );
	cgFloat angularVelocity = fabs( CGEToDegree(maxAngleDelta) / timeDelta );
	
	// Compute a weight for blending blurred and unblurred based on angular velocity (faster = blurrier)
	cgFloat angularVelocityWeight = cgMathUtility::clamp( (angularVelocity - mMinimumAngularVelocity) / (mMaximumAngularVelocity - mMinimumAngularVelocity), 0.0f, 1.0f );
	        angularVelocityWeight = powf( angularVelocityWeight, mBlendPower );
	mOperationData.compositeBlend = angularVelocityWeight;

    // Compute the required blur attenuation based on current rate.
    cgFloat blurAttenuation = (data.currentRenderRate - mAttenuationRates.min) / (mAttenuationRates.max - mAttenuationRates.min);
	blurAttenuation = max( 0.0f, min( 1.0f, blurAttenuation ) );

	// Attenuate blurriness factor(s)
	mOperationData.blurAmount = mBlurAmount * blurAttenuation * (mTargetRate / data.currentRenderRate);

    // If there is blurring to do
    if ( blurAttenuation > 0.0f )
	{
		// Compute current rotation for resulting blur matrix.
		cgQuaternion rotation, tmpQuat;
		cgFloat frameDelta = min( 1.0f, data.accumulatedTime * mTargetRate );
		cgQuaternion qkey1, qkey2;
		cgQuaternion::rotationMatrix( qkey1, data.viewKeyMatrix1 );
		cgQuaternion::rotationMatrix( qkey2, data.viewKeyMatrix2 );
		cgQuaternion::slerp( rotation, qkey1, qkey2, frameDelta );
		cgQuaternion::rotationMatrix( tmpQuat, cameraWorld );
		cgQuaternion::slerp( rotation, tmpQuat, rotation, mRotationalBlurAmt * 1.0f );

		// Compute current translation for resulting blur matrix
		cgVector3 translation;
		const cgVector3 & key1 = (cgVector3&)data.viewKeyMatrix1._41;
		const cgVector3 & key2 = (cgVector3&)data.viewKeyMatrix2._41;
		cgVector3::lerp( translation, key1, key2, frameDelta );
		cgVector3::lerp( translation, (cgVector3&)cameraWorld._41, translation, mTranslationBlurAmt * 1.0f );

		// Generate scene blur matrix. (Rotation + Translation)
		cgMatrix m;
		cgMatrix::rotationQuaternion( m, rotation );
		(cgVector3&)m._41 = translation;
		cgMatrix::inverse( cameraView, m );
	}

	// Combine matrices
	mOperationData.interpolatedCameraMatrix = invCameraView * (cameraView * cameraProj);

	// Cache current camera matrix for use in the next frame
	data.prevCameraMatrix = cameraWorld;

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : setTargetRate()
/// <summary>
/// Set the rate at which the camera transformation data will be captured 
/// (expressed in terms of samples per second), and with which the motion blur
/// processor will compute a consistent difference / velocity matrix 
/// irrespective of the actual rendering rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setTargetRate( cgFloat rate )
{
    mTargetRate = rate;
}

//-----------------------------------------------------------------------------
// Name : setAttenuationRates()
/// <summary>
/// Set the rendering rates (expressed in terms of samples per second) between
/// which any applied motion blur will begin to fade out until no blur is 
/// applied at all. No motion blur will be applied below the minimum rate, and
/// full motion blur will be applied above the maximum rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setAttenuationRates( cgFloat minimum, cgFloat maximum )
{
    setAttenuationRates( cgRangeF(minimum, maximum) );
}

//-----------------------------------------------------------------------------
// Name : setAttenuationRates()
/// <summary>
/// Set the rendering rates (expressed in terms of samples per second) between
/// which any applied motion blur will begin to fade out until no blur is 
/// applied at all. No motion blur will be applied below the minimum rate, and
/// full motion blur will be applied above the maximum rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setAttenuationRates( const cgRangeF & range )
{
    mAttenuationRates = range;
}

//-----------------------------------------------------------------------------
// Name : setRotationBlurAmount()
/// <summary>
/// Sets the amount of blur due to camera rotations.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setRotationBlurAmount( cgFloat amt )
{
	mRotationalBlurAmt = cgMathUtility::clamp( amt, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Name : setTranslationBlurAmount()
/// <summary>
/// Sets the amount of blur due to camera translations.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setTranslationBlurAmount( cgFloat amt )
{
	mTranslationBlurAmt = cgMathUtility::clamp( amt, 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Name : setBlurAmount()
/// <summary>
/// Sets the amount of blur for the system.
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setBlurAmount( cgFloat amt )
{
	mBlurAmount = amt;
}

//-----------------------------------------------------------------------------
// Name : setMaxSpeed()
/// <summary>
/// Sets the maximum length of the pixel velocity vector
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setMaxSpeed( cgFloat speed )
{
	mOperationData.maxSpeed = speed;
}

//-----------------------------------------------------------------------------
// Name : setCompositeSpeedScale()
/// <summary>
/// Sets the amount of blending between blurred and original colors during compositing (speed based).
/// </summary>
//-----------------------------------------------------------------------------
void cgMotionBlurProcessor::setCompositeSpeedScale( cgFloat minimumAngularVelocity, cgFloat maximumAngularVelocity, cgFloat power )
{
	mMinimumAngularVelocity = minimumAngularVelocity;
	mMaximumAngularVelocity = maximumAngularVelocity;
	mBlendPower             = power;
}

