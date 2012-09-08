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
// File : cgShadowGenerator.cpp                                              //
//                                                                           //
// Desc : Utility classes that provide the majority of the shadow mapping    //
//        implementation as used by the system provided light source types.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgShadowGenerator Module Includes
//-----------------------------------------------------------------------------
#include <World/Lighting/cgShadowGenerator.h>
#include <World/Lighting/cgLightingManager.h>
#include <World/Objects/cgLightObject.h>
#include <World/Objects/cgCameraObject.h>
#include <Rendering/cgSampler.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgTexturePool.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Math/cgMathUtility.h>
/*#include <World/cgScene.h>
#include <World/cgLightingManager.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgResampleChain.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgTexturePool.h>
#include <System/cgStringUtility.h>*/

///////////////////////////////////////////////////////////////////////////////
// cgShadowGenerator Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgShadowGenerator () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgShadowGenerator::cgShadowGenerator( cgLightNode * pParentLight, cgUInt32 nFrustumIndex )
{
    // Initialize variables to sensible defaults
    mType                    = cgShadowGeneratorType::ShadowMap;
    mParent                  = pParentLight;

    mSceneCamera             = CG_NULL;
    mSplitCamera             = CG_NULL;
    mResourceManager         = CG_NULL;
	mDriver                  = CG_NULL;
	mPool                    = CG_NULL;
	mPointSampler            = CG_NULL;
	mLinearSampler           = CG_NULL;
    mVerticalLinearSampler   = CG_NULL;
    mVerticalPointSampler    = CG_NULL;
	mHorizontalLinearSampler = CG_NULL;
    mHorizontalPointSampler  = CG_NULL;
    mLastVisibilityFrame     = -1;

	mMethodHW                = 0;
	mResolution              = 0;
	mDefaultStatus           = 0;
	mReassignmentStatus      = false;
	mRegenerate              = false;
	mDescriptionDirty        = true;

	mCurrentWritePass        = -1;
	mCurrentReadPass         = -1;

    mDepthSamplerRegister    = -1;
    mEdgeSamplerRegister     = -1;
    mColorSamplerRegister    = -1;
	mCustomSamplerRegister   = -1;
	mRandomSamplerRegister   = -1;

	mMaskChannels            = 0;
	mMaskCachedOnly          = true;
	mMaskDirty               = false;

    mSphereRadius            = 0.0f;
	mAttenuation             = 1.0f;
	mMinimumDistance         = 0.0f;
	mMaximumDistance         = 0.0f;

    // Instantiate a local camera for us to use but make sure it does not exist 
    // in the scene's database (i.e. bypass the scene's CreateObjectNode() method).
    // A reference identifier of '0' is required to denote these temporary objects.
    mCamera = new cgCameraNode( 0, pParentLight->getScene() );
    mCamera->onNodeCreated( RTID_CameraObject, cgCloneMethod::None );

    // Build a good default name for the generator (for debug purposes)
    if ( pParentLight && !pParentLight->getName().empty() )
        mName = cgString::format( _T("%s.Generator[%i]"), pParentLight->getName().c_str(), nFrustumIndex );
    else
        mName = cgString::format( _T("{Global}.Generator[%i]"), nFrustumIndex );
}

//-----------------------------------------------------------------------------
//  Name : ~cgShadowGenerator () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgShadowGenerator::~cgShadowGenerator()
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
void cgShadowGenerator::dispose( bool bDisposeBase )
{
    // Unassign all generator resources to prevent 'dangling' pointers.
    releaseResources( );

    // Release allocated resources.
    mShader.close();
    mShadowConstants.close();
    mImageShader.close();
    mImageProcessingConstants.close();
    mBilateralConstants.close();
    mDisabledDepthState.close();
    for ( int i = 0; i <= 15; ++i )
        mBlendStates[i].close();
    
    // Release allocated memory
    if ( mCamera )
        mCamera->scriptSafeDispose();
    if ( mSplitCamera )
        mSplitCamera->scriptSafeDispose();
    if ( mPointSampler )
        mPointSampler->scriptSafeDispose();
    if ( mLinearSampler )
        mLinearSampler->scriptSafeDispose();
    if ( mVerticalLinearSampler )
        mVerticalLinearSampler->scriptSafeDispose();
    if ( mVerticalPointSampler )
        mVerticalPointSampler->scriptSafeDispose();
    if ( mHorizontalLinearSampler )
        mHorizontalLinearSampler->scriptSafeDispose();
    if ( mHorizontalPointSampler )
        mHorizontalPointSampler->scriptSafeDispose();

    // Clear values
    mParent                   = CG_NULL;
    mResourceManager          = CG_NULL;
    mDriver                   = CG_NULL;
    mPool                     = CG_NULL;
    mCamera                   = CG_NULL;
    mSceneCamera              = CG_NULL;
    mPointSampler             = CG_NULL;
    mLinearSampler            = CG_NULL;
    mVerticalLinearSampler    = CG_NULL;
    mVerticalPointSampler     = CG_NULL;
    mHorizontalLinearSampler  = CG_NULL;
    mHorizontalPointSampler   = CG_NULL;
    mLastVisibilityFrame      = -1;
    mResolution               = 0;
    mMethodHW                 = 0;
    mSettings                 = cgShadowSettings();
    mDefaultStatus            = 0;
    mCurrentWritePass         = -1;
    mCurrentReadPass          = -1;
    mDepthSamplerRegister     = -1;
    mEdgeSamplerRegister      = -1;
    mColorSamplerRegister     = -1;
    mCustomSamplerRegister    = -1;
    mRandomSamplerRegister    = -1;
    mMaskChannels             = 0;
    mMaskCachedOnly           = true;
    mMaskDirty                = false;
    mSphereRadius             = 0.0f;
    mAttenuation              = 1.0f;
    mMinimumDistance          = 0.0f;
    mMaximumDistance          = 0.0f;
    mReassignmentStatus       = false;
    mRegenerate               = false;
    mDescriptionDirty         = true;
    
    // Clear STL containers
    mName.clear();
    mDescriptions.clear();
    mResources.clear();
    mSamplerStates.clear();
    mWriteOps.clear();
    mPostOps.clear();
    mReadOps.clear();
}

//-----------------------------------------------------------------------------
//  Name : setMethod()
/// <summary>
/// Alter the method used to generate shadows.
/// </summary>
//-----------------------------------------------------------------------------
void cgShadowGenerator::setMethod( cgUInt32 nMethod )
{
    mSettings.method = nMethod;
}

//-----------------------------------------------------------------------------
//  Name : getMethod()
/// <summary>
/// Retrieve the method used to generate shadows.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::getMethod( bool bHW /* = false */ ) const
{
    return bHW ? mMethodHW : mSettings.method;
}

//-----------------------------------------------------------------------------
//  Name : getType()
/// <summary>
/// Determine the type of shadow generator this is.
/// </summary>
//-----------------------------------------------------------------------------
cgShadowGeneratorType::Base cgShadowGenerator::getType( ) const
{
    return mType;
}

//-----------------------------------------------------------------------------
//  Name : getWritePassCount()
/// <summary>
/// Determine the number of passes that this generate requires in order to
/// populate the shadow map.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::getWritePassCount( ) const
{
    return (cgUInt32)mWriteOps.size();
}

//-----------------------------------------------------------------------------
//  Name : getEdgeMaskChannels()
/// <summary>
/// Determine the channels which are in use for the edge mask assigned to this
/// generator.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::getEdgeMaskChannels( ) const
{
    return mMaskChannels;
}

//-----------------------------------------------------------------------------
//  Name : getName()
/// <summary>
/// Retrieve the "human readable" name of this frustum for debugging
/// purposes.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgShadowGenerator::getName( ) const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : usesEdgeMask () (Protected)
/// <summary>
/// Determine if this generator uses an edge mask.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::usesEdgeMask( ) const
{
	// We require a dedicated register
	if ( mEdgeSamplerRegister < 0 )
		return false;

	return (mSettings.method & (cgShadowMethod::DepthExtentsMask | cgShadowMethod::EdgeMask)) != 0;
}

//-----------------------------------------------------------------------------
//  Name : usesTranslucent () (Protected)
/// <summary>
/// Determine if this generator supports translucent rendering
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::usesTranslucent( ) const
{
	// We require a dedicated register and translucency support.
	if ( mColorSamplerRegister < 0 || mSettings.translucency )
		return false;

    // ToDo: 6767 - If we have no translucent casters, fail (TODO)
	return false;
}

//-----------------------------------------------------------------------------
//  Name : usesStatistics () (Protected)
/// <summary>
/// Determine if this generator uses a statistical method for shadow generation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::usesStatistics( ) const
{
	return (mSettings.method & (cgShadowMethod::Variance | cgShadowMethod::Exponential)) != 0;
}

//-----------------------------------------------------------------------------
//  Name : getEdgeMaskChannelCount () (Protected)
/// <summary>
/// Retrieve how many channels are needed to store edge data given the current
/// shadow method being used
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::getEdgeMaskChannelCount( ) const
{
	if ( mSettings.method & cgShadowMethod::DepthExtentsMask )
		return 2;
	else if ( mSettings.method & cgShadowMethod::EdgeMask )
		return 1;
	else
		return 0;
}

//-----------------------------------------------------------------------------
//  Name : supportsEdgeMaps () (Protected)
/// <summary>
/// Are edge maps theoretically supported for a generator?
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::supportsEdgeMaps( ) const
{
	return (mEdgeSamplerRegister != -1);
}

//-----------------------------------------------------------------------------
//  Name : getResolution () (Virtual)
/// <summary>
/// Retrieve the currently selected shadow map resolution based on the
/// earlier provided LOD scalar (SetLODScalar()).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::getResolution( ) const
{
    return mResolution;
}

//-----------------------------------------------------------------------------
//  Name : getCamera ()
/// <summary>
/// Retrieve the internal shadow rendering camera.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode * cgShadowGenerator::getCamera( ) const
{
    return mCamera;
}

//-----------------------------------------------------------------------------
//  Name : getVisibilitySet ()
/// <summary>
/// Retrieve the computed visibility set for this shadow frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet * cgShadowGenerator::getVisibilitySet( ) const
{
    return mCamera->getVisibilitySet();
}

//-----------------------------------------------------------------------------
//  Name : containsRenderableObjects ()
/// <summary>
/// Lets the caller know whether the frustum can see any objects that qualify for
/// the current rendering method (i.e., shadow depths vs. rsm-based indirect)
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::containsRenderableObjects( ) const
{
    // ToDo: 6767
	// ToDo 9999: This should be a broad-phase spatial tree query to determine
	//            whether or not we can see any legit objects for the rendering
	//            we are about to do. For the moment, we still rely on the idea
	//            of a pre-computed shadow set, but that will be going away.
    return !mCamera->getVisibilitySet()->isEmpty();
}

//-----------------------------------------------------------------------------
//  Name : shouldRegenerate()
/// <summary>
/// Determine if the internal shadow map data needs to be regenerated
/// as a matter of urgency because the map contains invalid data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::shouldRegenerate( ) const
{
    return mRegenerate;
}

//-----------------------------------------------------------------------------
//  Name : requiresDefaultResource () 
/// <summary>
/// Does this frustum require the use of default resources?
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::requiresDefaultResource( ) const
{
	return mDefaultStatus > 0;
}

//-----------------------------------------------------------------------------
//  Name : canBlend ()
/// <summary>
/// Determine if this shadow method supports blending/border repair.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::canBlend( cgShadowGenerator * pNeighbor ) const
{
	// Resolutions must match
	if ( pNeighbor->getResolution() != mResolution )
		return false;

	// Methods (high level) must match
	if ( pNeighbor->getMethod() != mSettings.method )
		return false;
	
	// Note: For now, we limit ourselves to statistical types (this might change)
	if ( (mSettings.method & (cgShadowMethod::Variance | cgShadowMethod::Exponential)) == 0 )
		return false;

	// Return ok
    return true; 
}

//-----------------------------------------------------------------------------
//  Name : getUserDataResourceIndex () (Protected)
/// <summary>
/// Attempts to locate the resource associated with a custom user data value
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgShadowGenerator::getUserDataResourceIndex( cgInt32 nUserData ) const
{
	for ( size_t i = 0; i < mDescriptions.size(); i++ )
	{
		if ( mDescriptions[ i ].userData == nUserData )
			return i;
	
    } // Next resource

	// Not found
	return -1;
}

//-----------------------------------------------------------------------------
//  Name : update() 
/// <summary>
/// Updates the generator with new settings and descriptions
/// Note: Some additional work needs to be done to ensure proper integration
///       of transparent casters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::update( cgUInt32 nResolution, const cgShadowSettingsSystem & SystemSettings, const cgShadowSettingsLight & LightSettings, const cgTexturePoolResourceDesc::Array & aDescriptions, cgInt32 nFlags )
{
	// Cache previous values
	cgUInt32 nPrevFlags       = mMethodHW;
	cgUInt32 nPrevResolution  = mResolution;
	cgFloat  fPrevRadius      = mSettings.filterRadius;

	// Record the new flags and resolution
	mMethodHW   = nFlags;
	mResolution = nResolution;

	// Update our settings
	mSettings = cgShadowSettings( SystemSettings, LightSettings );

	// If the method/flags changed, the description is considered dirty
	if ( nFlags != nPrevFlags )
		mDescriptionDirty = true;

	// If the resolution has changed, we will assume the need for all 
	// new resources, so clear everything
	if ( (nResolution != nPrevResolution) )
	{
		releaseResources();
		mDescriptionDirty = true;

	} // End if resolution changed

	// If the filter radius changes and we are using edge masks,
	// the description is considered dirty
	if ( mSettings.filterRadius != fPrevRadius && usesEdgeMask() )
		mDescriptionDirty = true;

	// If the description is dirty
	if ( mDescriptionDirty )
	{
		// Replace the resource description list
		mDescriptions.clear();
		mDescriptions = aDescriptions;

		// If we are using an edge mask
		if ( mSettings.maskType > 0 )
		{
			// Compute resolution based on filter size
            cgInt32  nMaskIndex      = getUserDataResourceIndex( cgShadowResourceType::EdgeMap );
			cgUInt32 nStartLevel     = cgMathUtility::log2( mResolution ); 
			cgFloat  fKernelSize     = ceilf( mSettings.filterRadius ) * 2.0f + 1.0f;
			cgUInt32 nNumMipLevels   = (cgUInt32)( ceil( log( fKernelSize / 3.0f ) / log( 2.0 ) ) );
			cgUInt32 nMaskResolution = (1L << (nStartLevel - nNumMipLevels)); 
			mDescriptions[ nMaskIndex ].bufferDesc.width  = nMaskResolution;
			mDescriptions[ nMaskIndex ].bufferDesc.height = nMaskResolution;

			// Normalize the mask threshold (currently in meters)
			mSettings.maskThreshold /= (mCamera->getFarClip() - mCamera->getNearClip());
		}

		// If we are computing translucent shadows, append an extra buffer to capture this data
		if ( usesTranslucent() )
		{
			cgSamplerStateDesc LinearStates;
			LinearStates.minificationFilter = cgFilterMethod::Linear;
			LinearStates.magnificationFilter = cgFilterMethod::Linear;
			LinearStates.mipmapFilter = cgFilterMethod::None;
			
			cgTexturePoolResourceDesc ColorDesc;
			ColorDesc.userData          = cgShadowResourceType::ColorMap;
            ColorDesc.type              = cgTexturePoolResourceType::Cached;
			ColorDesc.bufferDesc.format = mPool->getBestRenderTargetFormat( 8, 4 );
			ColorDesc.bufferDesc.type   = cgBufferType::RenderTarget;
			ColorDesc.samplerStates     = LinearStates;
			mDescriptions.push_back( ColorDesc );

            // Translucency is available.
			mMethodHW |= cgShadowMethod::Translucency;
		
        } // End if translucency

		// Update sampler states
		updateSamplerStates();

		// The description is no longer dirty
		mDescriptionDirty = false;
	
    } // End if dirty

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : updateSamplerStates () (Protected)
/// <summary>
/// Updates the sampler states to match the current descriptions 
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::updateSamplerStates()
{
	// Clear any current states
	for ( size_t i = 0; i < mSamplerStates.size(); ++i )
		mSamplerStates[ i ].close();

	// Resize to match required resource count
	mSamplerStates.resize( mDescriptions.size() );
	
	// Build states
	for ( size_t i = 0; i < mSamplerStates.size(); ++i )
		mResourceManager->createSamplerState( &mSamplerStates[ i ], mDescriptions[ i ].samplerStates, 0, cgDebugSource() );

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : buildOperations () (Protected, Virtual)
/// <summary>
/// Builds a list of operations required to read and write shadows based on 
/// the current method.
/// Note: All of our shadow write operations use a depth-stencil buffer 
/// (resource slot 0) and always have a depth texture available, even if 
/// shared (resource slot 1)
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::buildOperations( )
{
    cgAssert( mResources.size() >= 2 );

	// Clear the current operations lists
	mWriteOps.clear();
	mPostOps.clear();
	mReadOps.clear();
	
	// Build a read operation
	cgShadowGeneratorOperation ReadPass( ComputeShadows );

	// Add a depth write operation
	cgShadowGeneratorOperation DepthPass( DrawOpaqueShadowCasters );
	DepthPass.clearFlags      = cgClearFlags::Depth;
	DepthPass.clearDepthValue = 1.0f;
	DepthPass.colorWrites     = cgColorChannel::None;

    // Get depth buffer resource.
    cgTextureHandle hTarget = mResources[ 0 ]->getResource();
    if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::DepthStencilTarget )
	    DepthPass.depthStencil = cgDepthStencilTargetHandle( static_cast<cgDepthStencilTarget*>(hTarget.getResourceSilent()) );

    // ...and the color buffer / render target
    hTarget = mResources[ 1 ]->getResource();
    if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::RenderTarget )
	    DepthPass.outputs.push_back( cgRenderTargetHandle( static_cast<cgRenderTarget*>(hTarget.getResourceSilent()) ) );
    else
        DepthPass.outputs.push_back( cgRenderTargetHandle::Null );
	
	// If we are doing software depth reads, we'll need color writing turned on
	if ( (mMethodHW & cgShadowMethod::DepthReads) && !(mMethodHW & cgShadowMethod::Hardware) )
	{
		DepthPass.colorWrites     = cgColorChannel::All;
		DepthPass.clearFlags     |= cgClearFlags::Target;
		DepthPass.clearColorValue = 0xffffffff;
	
    } // End if software depth

	// Set face culling mode (initially use current LOD settings)
	DepthPass.cullMode = mSettings.cullMode; 

	// If the method uses face normal offsets, we need to draw front faces (i.e., standard shadow maps)
	if ( mSettings.method & cgShadowMethod::NormalOffset )
		DepthPass.cullMode = cgCullMode::Back;

	// Which resource are we sampling from?
	if ( mDescriptions[ 0 ].type != cgTexturePoolResourceType::Shared )
	{
        // Depth-stencil target is not a shared target so it is meant to be sampled
		ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 0 ]->getResource(), mSamplerStates[ 0 ] ) ); 
	
    } // End if depth !shared
	else if ( mDescriptions[ 1 ].type != cgTexturePoolResourceType::Shared )
	{
        // Depth /texture/ is not a shared target, so it is meant to be sampled
		ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
	
    } // End if color !shared

	// Add a post operation for computing statistics if needed
	if ( usesStatistics() )
	{
		// Statistical methods require front face drawing
		DepthPass.cullMode = cgCullMode::Back;

		// Get the resource index for the statistics map
		cgInt32 nStatsMapIndex = getUserDataResourceIndex( cgShadowResourceType::StatisticsMap );

		// Generate a new post operation
		cgShadowGeneratorOperation StatisticsPass( ComputeStatistics );
        
        // Assign the correct output
        hTarget = mResources[ nStatsMapIndex ]->getResource();
        if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::RenderTarget )
		    StatisticsPass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)hTarget.getResourceSilent() ) );
        else
            StatisticsPass.outputs.push_back( cgRenderTargetHandle::Null );
		
        // ...and input
        if ( DepthPass.colorWrites != cgColorChannel::None )
			StatisticsPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
		else
			StatisticsPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 0 ]->getResource(), mSamplerStates[ 0 ] ) ); 

        // Assign to post-op list.
		mPostOps.push_back( StatisticsPass );
		
		// Assign the statistics map to our depth slot for reading
		ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ nStatsMapIndex ]->getResource(), mSamplerStates[ nStatsMapIndex ] ) ); 

	} // End if statistical method

	// Add a post operation for computing depth masks if needed
	if ( usesEdgeMask() )
	{
		// We need a readable depth format, so force it if we have to
		cgInt32 nHWPCF = (cgShadowMethod::Hardware | cgShadowMethod::Compare);
		cgInt32 nHWF4  = (cgShadowMethod::Hardware | cgShadowMethod::Gather);
		if ( (DepthPass.colorWrites == cgColorChannel::None) && (((mMethodHW & nHWPCF) == nHWPCF) || ((mMethodHW & nHWF4) == nHWF4))	)
		{
			DepthPass.colorWrites     = cgColorChannel::All;
			DepthPass.clearFlags     |= cgClearFlags::Target;
			DepthPass.clearColorValue = 0xffffffff;
		
        } // End if readable depth

		// Get the resource index for the edge map
		cgInt32 nEdgeMapIndex = getUserDataResourceIndex( cgShadowResourceType::EdgeMap );

		// Generate a new post operation
		cgShadowGeneratorOperation EdgeMaskPass( ComputeEdgeMask );

        // Assign the correct output
        hTarget = mResources[ nEdgeMapIndex ]->getResource();
        if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::RenderTarget )
            EdgeMaskPass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)hTarget.getResourceSilent() ) );
        else
            EdgeMaskPass.outputs.push_back( cgRenderTargetHandle::Null );

        // ...and input
		if ( DepthPass.colorWrites != cgColorChannel::None )
			EdgeMaskPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
		else
			EdgeMaskPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 0 ]->getResource(), mSamplerStates[ 0 ] ) ); 

        // Assign to post-op list
		mPostOps.push_back( EdgeMaskPass );

		// Assign the edge map for reading
		ReadPass.inputs.push_back( cgShadowGeneratorInput( mEdgeSamplerRegister, mResources[ nEdgeMapIndex ]->getResource(), mSamplerStates[ nEdgeMapIndex ] ) ); 

	} // End if edges

	// Add the depth pass to our list
	mWriteOps.push_back( DepthPass );

	// If translucency is needed, we need another write pass
	if ( mMethodHW & cgShadowMethod::Translucency )
	{
		// Get the resource index for the color map
		cgInt32 nColorMapIndex = getUserDataResourceIndex( cgShadowResourceType::ColorMap );

		// Generate a new write operation
		cgShadowGeneratorOperation TranslucentPass( DrawTransparentShadowCasters );
        TranslucentPass.colorWrites     = cgColorChannel::All;
        TranslucentPass.clearFlags      = cgClearFlags::Target;
        TranslucentPass.clearColorValue = 0xffffffff;

        // Get depth buffer resource.
        hTarget = mResources[ 0 ]->getResource();
        if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::DepthStencilTarget )
            TranslucentPass.depthStencil = cgDepthStencilTargetHandle( (cgDepthStencilTarget*)hTarget.getResourceSilent() );

        // ...and the color buffer / render target
        hTarget = mResources[ nColorMapIndex ]->getResource();
        if ( hTarget.isValid() && hTarget.getResourceType() == cgResourceType::RenderTarget )
            TranslucentPass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)hTarget.getResourceSilent() ) );
        else
            TranslucentPass.outputs.push_back( cgRenderTargetHandle::Null );

		// Add to write-ops list.
		mWriteOps.push_back( TranslucentPass );

		// Assign the color map for reading
		ReadPass.inputs.push_back( cgShadowGeneratorInput( mColorSamplerRegister, mResources[ nColorMapIndex ]->getResource(), mSamplerStates[ nColorMapIndex ] ) ); 
	
    } // End if translucency

	// Add the read pass
	mReadOps.push_back( ReadPass );

	// Return success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : assignResources ()
/// <summary>
/// Attempts to assign the resources needed for the current shadow method based
/// on the desired description structures and the flags used to indicate what
/// resources, if any, were reassigned from a previous frame. The goal will be
/// to ensure that, one way or another, all necessary resources to fill a shadow
/// map are available at the end of this call. However, if any default resources
/// are needed to accomplish this, the method will indicate so in its return value.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgShadowGenerator::assignResources( cgTexturePool * pPool )
{
	// Keep track of the number of default resources we are assigned
	mDefaultStatus = 0;

	// If resource reassignment was already successful, filling is at the discretion of the caller.
	if ( mReassignmentStatus )
		return cgShadowGeneratorFillResult::CanFill;

	// Reassignment was not successful, so we'll need a new set of resources
	if ( !mPool->assignResources( mDescriptions, this, (cgUInt32)mType, mResources, mDefaultStatus ) )
	{
		cgAppLog::write( cgAppLog::Error, _T("Failed to find valid resources for shadow generator '%s' during assignment phase\n."), getName().c_str() );
		return cgShadowGeneratorFillResult::DoNothing;
	
    } // End if no resources

	// Edge masks require special processing
	if ( usesEdgeMask() )
	{
		// If we were unsuccessful finding a cached edge mask, we can turn off the feature
		cgInt32 nMaskIndex = getUserDataResourceIndex( cgShadowResourceType::EdgeMap );
		if ( mResources[ nMaskIndex ]->getType() == cgTexturePoolResourceType::Shared && mMaskCachedOnly )
		{
			mMaskChannels  = 0;
			mMaskDirty     = false;
			mMethodHW     &= ~(cgShadowMethod::DepthExtentsMask | cgShadowMethod::EdgeMask);
		
        } // End if no cached map
		else
		{
			// If we got a mask, grab the channels and turn on generation
			mMaskChannels = mResources[ nMaskIndex ]->getChannelMask( this );
			mMaskDirty    = true;

		} // End if found cached map

	} // End if edge masking

	// Anytime we get new resources, we need to rebuild the operations list
	if ( !buildOperations() )
		return false;

	// If we were assigned any defaults, we cannot fill now. Otherwise, we must fill.
	if ( mDefaultStatus > 0 )
		return cgShadowGeneratorFillResult::CannotFill;
	else
		return cgShadowGeneratorFillResult::MustFill;
}

//-----------------------------------------------------------------------------
//  Name : reassignResources ()
/// <summary>
/// Attempts to keep previously assigned resources from prior frames. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::reassignResources( cgTexturePool * pPool )
{	
	// Attempt reassignment
	mReassignmentStatus = mPool->reassignResources( mDescriptions, this, (cgUInt32)mType, mResources );

	// Initially assume we will find no matches
	if ( !mReassignmentStatus )
	{
		// If we did not successfully reassign, release all of our held resources
		releaseResources();
	
    } // End if failed
	else
	{
		// Are we using an edge mask?
		if ( usesEdgeMask() )
		{
			// If we got assigned different channels, we'll need to update the mask
			cgInt32 nMaskIndex = getUserDataResourceIndex( cgShadowResourceType::EdgeMap );
			cgUInt32 nChannels = mResources[ nMaskIndex ]->getChannelMask( this );
			if ( mMaskChannels != nChannels )
				mMaskDirty = true;
			
			// Set channels
			mMaskChannels = nChannels;
		
        } // End if edge mask
	
    } // End if success

	// Return status
	return mReassignmentStatus;
}

//-----------------------------------------------------------------------------
//  Name : releaseResources () 
/// <summary>
/// Releases all currently held resources back to the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgShadowGenerator::releaseResources( )
{
	for ( size_t i = 0; i < mResources.size(); i++ )
	{
		if ( mResources[ i ] )
			mResources[ i ]->unassign( this, true );
	
    } // Next resource

	mResources.clear();
	mMaskChannels = 0;
}

//-----------------------------------------------------------------------------
// Name : computeVisibilitySet( ) (Virtual)
// Desc : Creates the shadow mapping visibility set that contains objects also 
//        found within the parent light's shadow set. This  essentially narrows 
//        down the list of visible objects from the point of view of the light 
//        source to only those that can both be seen by the camera and cast
//        shadows with respect the parent light.
// Note : Returns true if this frustum contains visible casters that could
//        potentially affect the shadow map (whether the shadow map will
//        ultimately be regenerated or not).
//-----------------------------------------------------------------------------
bool cgShadowGenerator::computeVisibilitySet( cgCameraNode * pSceneCamera, bool bParallelSplit /* = false */, cgFloat fNearSplitDistance /* = 0.0f */, cgFloat fFarSplitDistance /* = 0.0f */ )
{
    // Get access to required systems.
    cgScene * pScene = mParent->getScene();

    // Do not regenerate shadow map until we know something has changed.
    mRegenerate = false;

    // We want to process the frustum camera's current visibility set.
    cgVisibilitySet * pFrustumVis = mCamera->getVisibilitySet();

    // If the light source moved or was updated, we can immediately
    // consider this frustum to be dirty.
    if ( !mLastVisibilityFrame || mParent->isNodeDirtySince( mLastVisibilityFrame ) )
    {
        mRegenerate = true;
    
    } // End if light source altered
    else
    {
        // The light source wasn't altered in any way, check shadow casting objects.
        cgObjectNodeSet::const_iterator itObject;
        const cgObjectNodeArray & Objects = pFrustumVis->getVisibleObjects();
        for ( size_t i = 0; i < Objects.size(); ++i )
        {
            // If the object was deleted, or has been altered (moved, animated etc.)
            // then we must also regenerate the shadow map.
            
            // ToDo: We can probably replace isValidReference() (as well as 
            // removing the 'm_ValidReferences' set in the reference manager) with
            // a call to 'GetReference( id )' if we stored the original reference 
            // id in the visibility set (i.e. m_ObjectNodes is a map instead of a set).
            cgObjectNode * pObject = Objects[i];
            if ( !cgReferenceManager::isValidReference( pObject ) || pObject->isNodeDirtySince( mLastVisibilityFrame ) )
            {
                mRegenerate = true;
                break;

            } // End if object altered

        } // Next object

    } // End if light source static

    // If the light source is employing an orthographic parallel split
    // rendering approach, we need to compute a new projection matrix.
    if ( bParallelSplit )
    {
        // Parallel split frustum is aligned to the main scene camera (not light source).
        // Build a custom camera using similar details to the scene camera, but adjust the
        // near / far clip planes to match those for the split so that we can query for objects
        // that fall within it.
        if ( mSplitCamera != CG_NULL )
            mSplitCamera->scriptSafeDispose();
        mSplitCamera = new cgCameraNode( 0, pSceneCamera->getScene(), pSceneCamera, cgCloneMethod::Copy, pSceneCamera->getWorldTransform() );
        mSplitCamera->setNearClip( fNearSplitDistance );
        mSplitCamera->setFarClip( fFarSplitDistance );
        
        // Get the sub-frustum for this split
		const cgFrustum & SplitFrustum = mSplitCamera->getFrustum();

		// If we haven't already, compute the radius of the bounding sphere around this frustum. It is 
		// important that we only do this the one time. Recomputing it per frame introduces very small floating
		// point errors that result in minor flickering. Caching this value now removes the problem.
		if ( mSphereRadius <= 0.0f )
		{
			cgVector3 SphereCenter(0,0,0);
			for( cgInt i = 0; i < 8; i++ )
				SphereCenter += SplitFrustum.points[ i ];
			SphereCenter /= 8.0f;

			for( cgInt i = 0; i < 8; i++ )
			{
				float r = cgVector3::length( SphereCenter - SplitFrustum.points[ i ] );
				if( r > mSphereRadius ) mSphereRadius = r;
			
            } // Next frustum point
		
        } // End if first time

        // ToDo: 6767 - Fix
		/*// Compute caster/receiver list and bounds
        pFrustumVis->Clear();
        cgVisibilitySet * pShadowSet       = mParent->GetShadowSet();
        cgVisibilitySet * pIlluminationSet = mParent->GetIlluminationSet();
		pShadowSet->Intersect( SplitFrustum, mParent->getZAxis(false), pFrustumVis, mCasterBounds );
    
		// ToDo: Figure out how best to handle the AllowLightmappedCasters option with PSSM...

		// ToDo: Note... Always adds receivers, even if this is not variance. I think we agree this was fine (we no longer
		// take this into consideration anywhere else I believe). Making a note however just in case.
        // Additional Note.... The dirty system may not work if we don't.
		pIlluminationSet->Intersect( SplitFrustum, mParent->getZAxis(false), pFrustumVis, mReceiverBounds );*/
    
    } // End if Parallel Split
    else
    {
        // Build our object filter for choosing casters to render into the shadowmap
        cgUInt32 nFlags = cgVisibilitySearchFlags::MustRender | cgVisibilitySearchFlags::MustCastShadows |
                          cgVisibilitySearchFlags::CollectMaterials;

		// Compute the new filtered visibility set for this frustum.
		mCamera->computeVisibility( nFlags );

        // ToDo: Figure out how to filter objects that can't cast a shadow
        // into the camera's frustum (i.e. perspective extrude AABB?). 

    } // End if Standard

    // Visibility has been recomputed on this frame.
    mLastVisibilityFrame = cgTimer::getInstance()->getFrameCounter();

    // If there is nothing for us to do, we can let the caller 
    // know to turn shadows off.
    if ( pFrustumVis->isEmpty() )
    {
        mRegenerate = false;
        return false;
    
    } // End if no casters

    // If we still don't think we need to regenerate, has anything been created
    // or moved recently such that it now exists in the new view?
    if ( !mRegenerate )
    {
        cgObjectNodeSet::const_iterator itObject;
        const cgObjectNodeArray & Objects = pFrustumVis->getVisibleObjects();
		for ( size_t i = 0; i < Objects.size(); ++i )
        {
            // If the object is dirty, we must regenerate!
            if ( Objects[i]->isNodeDirtySince( mLastVisibilityFrame ) )
            {
                mRegenerate = true;
                break;

            } // End if object altered

        } // Next object

    } // End if !bRegenerate

    // Casters are visible!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Allow the shadow frustum to initialize its internal resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::initialize( )
{
    // Store values required for future reference.
    mResourceManager = mParent->getScene()->getResourceManager();
    mDriver          = mResourceManager->getRenderDriver();
	mPool            = mParent->getScene()->getLightingManager()->getShadowMaps();

	// Load the core integration shader.
    if ( !mResourceManager->createSurfaceShader( &mShader, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load/compile light source shader while creating shadow generator '%s' of object node 0x%x in scene '%s'.\n"), mName.c_str(), mParent->getReferenceId(), mParent->getScene()->getName().c_str() );
        return false;
    
    } // End if failed

    // Create constant buffers
    if ( !mResourceManager->createConstantBuffer( &mShadowConstants, mShader, _T("_cbShadow"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required shadow frustum constant buffers for shadow generator '%s' of object node 0x%x in scene '%s'.\n"), mName.c_str(), mParent->getReferenceId(), mParent->getScene()->getName().c_str() );
        return false;
    
    } // End if failed
    cgAssert( mShadowConstants->getDesc().length == sizeof(_cbShadow) );

	// Have we already tested the non-critical shadow registers for validity?
    // ToDo: 6767 - Can't do this as statics -- reloading scripts will potentially invalidate the register indices!
	static bool bTestedEdgeRegister   = false;
	static bool bTestedColorRegister  = false;
	static bool bTestedCustomRegister = false;
	static bool bTestedRandomRegister = false;

	// Clear any prior shaders
	if ( mImageShader.isValid() )
        mImageShader.close();

    // Load the image processing surface shader
    if ( !mResourceManager->createSurfaceShader( &mImageShader, _T("sys://Shaders/ImageProcessing.sh"), 0, cgDebugSource() ) )
        return false;

    // Find the correct registers for sampling.
    mDepthSamplerRegister = mShader->findSamplerRegister( _T("Shadow") );
	if ( mDepthSamplerRegister < 0 )
	{
		cgAppLog::write( cgAppLog::Error, _T("Failed to bind the shadow sampler to generator. This is an application failure case.\n") );
		return false;
	
	} // End if failed

	mEdgeSamplerRegister = mShader->findSamplerRegister( _T("ShadowEdge") );
    if ( mEdgeSamplerRegister < 0 )
    {
		if ( !bTestedEdgeRegister )
			cgAppLog::write( cgAppLog::Warning, _T("Failed to bind a shadow edge sampler to generator. Edge optimizations are unavailable.\n") );
    
    } // End if failed

    mColorSamplerRegister = mShader->findSamplerRegister( _T("ShadowColor") );
    if ( mColorSamplerRegister < 0 )
    {
		if ( !bTestedColorRegister )
			cgAppLog::write( cgAppLog::Warning, _T("Failed to bind a color sampler to generator. Translucent shadows are unavailable.\n") );
    
    } // End if failed

    mCustomSamplerRegister = mShader->findSamplerRegister( _T("ShadowCustom") );
    if ( mCustomSamplerRegister < 0 )
    {
		if ( !bTestedCustomRegister )
			cgAppLog::write( cgAppLog::Warning, _T("Failed to bind a custom sampler to generator. Custom user data for shadows is unavailable.\n") );
    
    } // End if failed

    mRandomSamplerRegister = mShader->findSamplerRegister( _T("Random") );
    if ( mRandomSamplerRegister < 0 )
    {
		if ( !bTestedCustomRegister )
			cgAppLog::write( cgAppLog::Warning, _T("Failed to bind a random number sampler to generator. Randomized data for shadows is unavailable.\n") );
    
    } // End if failed

	// All global registers tested
	bTestedEdgeRegister   = true;
	bTestedColorRegister  = true;
	bTestedCustomRegister = true;
	bTestedRandomRegister = true;

    ///////////////////////////////////////////////////////////////
    // Constant Buffers
    ///////////////////////////////////////////////////////////////

    // Generate the constant buffer(s) used to get data to the image processor
    if ( !mResourceManager->createConstantBuffer( &mImageProcessingConstants, mImageShader, _T("cbImageProcessing"), cgDebugSource() ) )
        return false;
    if ( !mResourceManager->createConstantBuffer( &mBilateralConstants, mImageShader, _T("cbBilateral"), cgDebugSource() ) )
        return false;

    ///////////////////////////////////////////////////////////////
    // Depth Stencil States
    ///////////////////////////////////////////////////////////////
    cgDepthStencilStateDesc dsStates;
    dsStates.depthEnable      = false;
    dsStates.depthWriteEnable = false;
    if ( !mResourceManager->createDepthStencilState( &mDisabledDepthState, dsStates, 0, cgDebugSource() ) )
        return false;

    ///////////////////////////////////////////////////////////////
    // Blend States
    ///////////////////////////////////////////////////////////////
    cgBlendStateDesc blStates;
    blStates.renderTarget[0].blendEnable            = false;
    blStates.renderTarget[0].sourceBlend            = cgBlendMode::One;
    blStates.renderTarget[0].destinationBlend       = cgBlendMode::Zero;

	// Create all color channel mask permutations 
	for ( cgUInt32 i = 0; i <= 15; i++ )
	{
		blStates.renderTarget[0].renderTargetWriteMask = i;
		if ( !mResourceManager->createBlendState( &mBlendStates[ i ], blStates, 0, cgDebugSource() ) )
			return false;
	
    } // Next mask

    ///////////////////////////////////////////////////////////////
    // Samplers / Sampler States
    ///////////////////////////////////////////////////////////////
    mPointSampler            = mResourceManager->createSampler( _T("Image"), mImageShader );
    mLinearSampler           = mResourceManager->createSampler( _T("Image"), mImageShader );
    mVerticalPointSampler    = mResourceManager->createSampler( _T("Vertical"), mImageShader );
    mVerticalLinearSampler   = mResourceManager->createSampler( _T("Vertical"), mImageShader );
    mHorizontalPointSampler  = mResourceManager->createSampler( _T("Horizontal"), mImageShader );
    mHorizontalLinearSampler = mResourceManager->createSampler( _T("Horizontal"), mImageShader );

    // Point sampling (clamped)
    cgSamplerStateDesc smpStates;
    smpStates.addressU  = cgAddressingMode::Clamp;
    smpStates.addressV  = cgAddressingMode::Clamp;
    smpStates.addressW  = cgAddressingMode::Clamp;
    smpStates.minificationFilter = cgFilterMethod::Point;
    smpStates.magnificationFilter = cgFilterMethod::Point;
    smpStates.mipmapFilter = cgFilterMethod::None;
	mPointSampler->setStates( smpStates );
	mVerticalPointSampler->setStates( smpStates );
	mHorizontalPointSampler->setStates( smpStates );

    // Bilinear sampling (clamped)
    smpStates.minificationFilter = cgFilterMethod::Linear;
    smpStates.magnificationFilter = cgFilterMethod::Linear;		
	mLinearSampler->setStates( smpStates );
	mVerticalLinearSampler->setStates( smpStates );
	mHorizontalLinearSampler->setStates( smpStates );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setInputs()
/// <summary>
/// Sets up the required samplers based on the list of input maps
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::setInputs( const cgShadowGeneratorInput::Array & aInputs )
{
	for ( size_t i = 0; i < aInputs.size(); ++i )
	{	
		mDriver->setTexture( aInputs[ i ].samplerIndex, aInputs[ i ].texture );
		mDriver->setSamplerState( aInputs[ i ].samplerIndex, aInputs[ i ].states );

	} // Next map

	// Return success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : beginWrite()
/// <summary>
/// Prepare for the population of this frustum's shadow map with data.
/// ToDo : When filling shadow maps multiple times in a given frame, IsObjectDirty flag could cause regeneration. 
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgShadowGenerator::beginWrite( bool bParallelSplit, bool bFixedSplitDistances )
{
    // Retrieve the main scene camera so that we can restore it after
    // rendering to the shadow map, and also for additional computations
    // in the PSSM case to allow splits to be generated in relation to
    // the current main scene camera.
    mSceneCamera = mDriver->getCamera();

    // Final parallel split computations (should update only when
    // the shadow map is actually filled in order for shadow update 
    // rate limiting to function correctly).
    if ( bParallelSplit && mSplitCamera )
    {
        // Get the sub-frustum for this split
		const cgFrustum & SplitFrustum = mSplitCamera->getFrustum();

        // If we are using a fixed split distance...
		if ( bFixedSplitDistances )
		{
			// Compute transform matrices based on rotationally invariant frustum bounds
			buildOrthoMatrices( mCasterBounds, SplitFrustum );
		
        } // End if fixed splits
		else // for adaptive cases...
		{
            // Compute the bounds for this sub-frustum
            cgBoundingBox FrustumAABB;
		    for( cgInt i = 0; i < 8; i++ )
                FrustumAABB.addPoint( SplitFrustum.points[i] );

			// Build final receiver bounds
			mReceiverBounds.min.x = max( mReceiverBounds.min.x, FrustumAABB.min.x );
			mReceiverBounds.min.y = max( mReceiverBounds.min.y, FrustumAABB.min.y );
			mReceiverBounds.min.z = max( mReceiverBounds.min.z, FrustumAABB.min.z );
			mReceiverBounds.max.x = min( mReceiverBounds.max.x, FrustumAABB.max.x );
			mReceiverBounds.max.y = min( mReceiverBounds.max.y, FrustumAABB.max.y );
			mReceiverBounds.max.z = min( mReceiverBounds.max.z, FrustumAABB.max.z );

			// Compute transform matrices based on casters and receivers
			buildOrthoMatrices( mCasterBounds, mReceiverBounds );

        } // End if adaptive

    } // End if bParallelSplit
	
	// Set camera to shadow frustum's point of view
	mDriver->setCamera( mCamera );

	// Because we are potentially transitioning from deferred rendering to
	// forward rendering (shadow map fill is almost always a forward render)
	// backup the sampler / texture registers before we begin. This ensures
	// that any bound G-Buffer targets are not overwritten.
	mDriver->backupSamplerStates( 0, 15 );
	mDriver->backupTextures( 0, 15 );

    // Return number of write operations
	return mWriteOps.size();
}

//-----------------------------------------------------------------------------
//  Name : beginWritePass()
/// <summary>
/// Processes the write operation requested by the caller
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::beginWritePass( cgInt32 nPassIndex )
{
	// Store the current pass
    mCurrentWritePass = nPassIndex;

	// Get the current operation
	cgShadowGeneratorOperation & op = mWriteOps[ nPassIndex ];
		
	// Setup any input buffers we may need
	if ( !op.inputs.empty() )
		setInputs( op.inputs );

	// If we need to setup any output buffers
	if ( !op.outputs.empty() || op.depthStencil.isValid() )
	{
		if ( !mDriver->beginTargetRender( op.outputs, false, op.depthStencil ) )
			return false;

	} // End if output buffer(s)

	// If we want to clear any buffers
	if ( op.clearFlags > 0 )
	{
		// Build a clearing rectangle in case we are sharing any
		// of our resources (that may be larger) with other systems.
		cgRect rcClear( 0, 0, mResolution, mResolution );

		// Clear the render target(s).
		mDriver->clear( 1, &rcClear, op.clearFlags, op.clearColorValue, op.clearDepthValue, op.clearStencilValue );

	} // End clear required

	// Set system states
	mDriver->setSystemState( cgSystemState::ColorWrites, op.colorWrites );
	mDriver->setSystemState( cgSystemState::CullMode, op.cullMode );

    // Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : endWrite()
/// <summary>
/// Finish populating this frustum's shadow map with data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::endWritePass( )
{
	cgAssert( mCurrentWritePass >= 0 );

	// Get the current operation
	cgShadowGeneratorOperation & op = mWriteOps[ mCurrentWritePass ];

    // If we began rendering, end it
	if ( !op.outputs.empty() || op.depthStencil.isValid() )
	{
	    if ( !mDriver->endTargetRender( ) )
			return false;
	
    } // End if began

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : endWrite()
/// <summary>
/// Finish populating generator data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::endWrite( )
{
	// Run all post-processes
	for ( cgUInt32 i = 0; i < mPostOps.size(); i++ )
		executePostOperation( mPostOps[ i ] );

    // Because we are transitioning back from forward rendering to deferred 
    // rendering (shadow map filling is almost always a forward render) restore
    // the sampler / texture registers to the state they were in before we
    // began rendering. This ensures that any previously bound G-Buffer targets 
    // are restored to their previous registers.
	mDriver->restoreSamplerStates( 0, 15 );
	mDriver->restoreTextures( 0, 15 );

    // Restore the prior camera
    mDriver->setCamera( mSceneCamera );
    mSceneCamera = CG_NULL;
	
	// Reset pass
	mCurrentWritePass = -1;

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : executePostOperation()
/// <summary>
/// Runs a post-processing operation
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::executePostOperation( const cgShadowGeneratorOperation & Operation )
{
	switch( Operation.operationTypeId )
	{
		case ComputeEdgeMask:
			return updateEdgeMask( Operation );
		    break;
		case ComputeStatistics:
			return createStatisticsMap( Operation );
		    break;
		case MergeColorAndEdge:
			return copyEdgeToColor( Operation );
		    break;
	}
	return false;
}

//-----------------------------------------------------------------------------
//  Name : setConstants ()
/// <summary>
/// Sets the constant buffer(s) for this generator
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::setConstants()
{
	// Setup additional shadowing parameters
    cgConstantBuffer * pShadowParamsBuffer = mShadowConstants.getResource( true );
    cgAssert( pShadowParamsBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbShadow * pShadowData = CG_NULL;
    if ( !(pShadowData = (_cbShadow*)pShadowParamsBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock shadow parameter constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Update constants - shadow map size first.
    pShadowData->shadowTextureSize.x = (cgFloat)mResolution;
    pShadowData->shadowTextureSize.y = pShadowData->shadowTextureSize.x;
    pShadowData->shadowTextureSize.z = 1.0f / pShadowData->shadowTextureSize.x;
    pShadowData->shadowTextureSize.w = 1.0f / pShadowData->shadowTextureSize.y;

    // Shadow attenuation values.
    pShadowData->shadowAttenuation.x = mMinimumDistance;
    pShadowData->shadowAttenuation.y = mMaximumDistance;
    pShadowData->shadowAttenuation.z = mAttenuation;
    pShadowData->shadowAttenuation.w = 1.0f - mAttenuation;

    // Scale & bias. (ToDo: Get proper values from UI. These are just temporary.)
    pShadowData->shadowBias.x = mSettings.depthBiasSW;
    pShadowData->shadowBias.y = mSettings.slopeScaleBias;
    pShadowData->shadowBias.z = mSettings.normalBiasSurface;
    pShadowData->shadowBias.w = mSettings.normalBiasLight;

	// Filter data
	if ( usesStatistics() )
	{
		pShadowData->shadowFilter.x = mSettings.minimumVariance;
		pShadowData->shadowFilter.y = mSettings.exponent;
		pShadowData->shadowFilter.z = mSettings.minimumCutoff;
		pShadowData->shadowFilter.w = 0;
	}
	else
	{
		pShadowData->shadowFilter.x = mSettings.filterRadiusNear;
		pShadowData->shadowFilter.y = mSettings.filterRadiusFar;
		pShadowData->shadowFilter.z = 1.0f / (mSettings.filterDistanceFar - mSettings.filterDistanceNear);
		pShadowData->shadowFilter.w = mSettings.filterDistanceNear * pShadowData->shadowFilter.z;
	}

	// Edge texture channel masks
	if ( usesEdgeMask() )
	{
		cgUInt32 nNumChannels   = getEdgeMaskChannelCount();
		cgUInt32 nChannels      = getEdgeMaskChannels();
		cgUInt32 aChannels[ 2 ] = { 0, 0 };
		
		// Find the first channel
		if ( nChannels & cgColorChannel::Red )
			aChannels[ 0 ] = cgColorChannel::Red;
		else if ( nChannels & cgColorChannel::Green )
			aChannels[ 0 ] = cgColorChannel::Green;
		else if ( nChannels & cgColorChannel::Blue )
			aChannels[ 0 ] = cgColorChannel::Blue;
		else
			aChannels[ 0 ] = cgColorChannel::Alpha;

		// If there is more than one channel, grab it
		if ( nNumChannels > 1 )
			aChannels[ 1 ] = nChannels & ~aChannels[ 0 ];
		
		// Clear the current data
		cgVector4 channelMask( 0, 0, 0, 0 );
		pShadowData->edgeTextureChannelMask0 = channelMask;
		pShadowData->edgeTextureChannelMask1 = channelMask;

		// Fill the masks
		for ( cgUInt32 i = 0; i < nNumChannels; i++ )
		{
			switch ( aChannels[ i ] )
			{
				case cgColorChannel::Red:
					channelMask = cgVector4(1,0,0,0);
				    break;
				case cgColorChannel::Green:
					channelMask = cgVector4(0,1,0,0);
				    break;
				case cgColorChannel::Blue:
					channelMask = cgVector4(0,0,1,0);
				    break;
				case cgColorChannel::Alpha:
					channelMask = cgVector4(0,0,0,1);
				    break;

			} // End switch

			if ( i == 0 )
				pShadowData->edgeTextureChannelMask0 = channelMask;
			else
				pShadowData->edgeTextureChannelMask1 = channelMask;

		} // Next channel mask

	} // End if edge masked

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pShadowParamsBuffer->unlock();
    mDriver->setConstantBufferAuto( mShadowConstants );

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : beginRead ()
/// <summary>
/// Setup states relating to this shadow frustum that are required for
/// rendering the shadow / attenuation data
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::beginRead( cgFloat fAttenuation, cgFloat fMinDistance, cgFloat fMaxDistance, const cgMatrix * pTexProj )
{
	// Record inputs
	mAttenuation  = fAttenuation;
	mMinimumDistance  = fMinDistance;
	mMaximumDistance  = fMaxDistance;

	// Process the read operation (Note: We currently support only a single read operation. Can expand if desired.)
	if ( !mReadOps.empty() )
	{
		// Bind the textures for reading
		if ( !setInputs( mReadOps[ 0 ].inputs ) )
			return false;
	
    } // End if has read ops

	// Set constants
	if ( !setConstants() )
		return false;

    // Compute the texture projection matrix.
    const cgMatrix & mtxView = mCamera->getViewMatrix();
    const cgMatrix & mtxProj = mCamera->getProjectionMatrix();
    cgMatrix mtxShadowMap = mtxView * mtxProj;
    if ( !pTexProj )
    {
        static const cgMatrix mtxTexScale( 0.5f,  0.0f,  0.0f,  0.0f,
							               0.0f, -0.5f,  0.0f,  0.0f,
							               0.0f,  0.0f,  1.0f,  0.0f,
							               0.5f,  0.5f,  0.0f,  1.0f );
        mtxShadowMap *= mtxTexScale;
    
    } // End if standard projection
    else
    {
        mtxShadowMap *= (*pTexProj);
    
    } // End if custom projection
   
    // Give the lighting manager the current shadowmap matrix and set the buffer
    mParent->getScene()->getLightingManager()->setConstant( _T("_lightTexProjMatrix"), &mtxShadowMap );
    mParent->getScene()->getLightingManager()->applyLightingConstants();

	// Let the system know what kind of shadow mapping is active
    mDriver->setSystemState( cgSystemState::ShadowMethod, mMethodHW );

    // Let the system know how many taps to take
	mDriver->setSystemState( cgSystemState::PrimaryTaps, mSettings.primarySamples );
	mDriver->setSystemState( cgSystemState::SecondaryTaps, mSettings.secondarySamples ); 

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : endRead ()
/// <summary>
/// Restore any required states after a call to beginRead()
/// </summary>
//-----------------------------------------------------------------------------
void cgShadowGenerator::endRead( )
{
	// Always disable shadow features after a read operation
	mDriver->setSystemState( cgSystemState::PrimaryTaps, 0 );
	mDriver->setSystemState( cgSystemState::SecondaryTaps, 0 ); 

	// Clear out all samplers we might have used
	if ( mDepthSamplerRegister != -1 )
	{
		mDriver->setTexture( mDepthSamplerRegister, cgTextureHandle::Null );
		mDriver->setSamplerState( mDepthSamplerRegister, cgSamplerStateHandle::Null );
	}
	if ( mEdgeSamplerRegister != -1 )
	{
		mDriver->setTexture( mEdgeSamplerRegister, cgTextureHandle::Null );
	    mDriver->setSamplerState( mEdgeSamplerRegister, cgSamplerStateHandle::Null );
	}
	if ( mColorSamplerRegister != -1 )
	{
		mDriver->setTexture( mColorSamplerRegister, cgTextureHandle::Null );
	    mDriver->setSamplerState( mColorSamplerRegister, cgSamplerStateHandle::Null );
	}
	if ( mCustomSamplerRegister != -1 )
	{
		mDriver->setTexture( mCustomSamplerRegister, cgTextureHandle::Null );
	    mDriver->setSamplerState( mCustomSamplerRegister, cgSamplerStateHandle::Null );
	}
	if ( mRandomSamplerRegister != -1 )
	{
		mDriver->setTexture( mRandomSamplerRegister, cgTextureHandle::Null );
	    mDriver->setSamplerState( mRandomSamplerRegister, cgSamplerStateHandle::Null );
	}
}

//-----------------------------------------------------------------------------
//  Name : updateEdgeMask ()
/// <summary>
/// Updates the edge map data for this generator.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::updateEdgeMask( const cgShadowGeneratorOperation & Operation )
{
    // Only update the shadow edge map if needed
    if ( !mMaskDirty )
		return true;

	// We do 2x2 downsampling followed by a 3x3 final filter. Determine how many mip levels to process.
	cgFloat  fKernelSize   = ceil( mSettings.filterRadius ) * 2.0f + 1.0f;
	cgUInt32 nNumMipLevels = (cgUInt32)( ceil( log( fKernelSize / 3.0f ) / log( 2.0 ) ) );

    // Get the surface shader
    cgSurfaceShader * pShader = mImageShader.getResource(true);

    // Get a pointer to the constant buffers
    cgConstantBuffer * pcbImage = mImageProcessingConstants.getResource(true);
    cgConstantBuffer * pcbBilateral = mBilateralConstants.getResource(true);

    // Set rendering states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( cgBlendStateHandle::Null );

	// Both processes will use the same vertex shader
    static const bool bFalse = false;
    cgScriptArgument::Array vsArgs( 1 );
    vsArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bFalse );
    if ( !pShader->selectVertexShader( _T("transform"), vsArgs ) )
		return false;

	////////////////////////////////////////////
	// Detect edges
	////////////////////////////////////////////
	cgInt32 nDepthStencilIndex = 0;
	cgInt32 nDepthIndex        = 1;

	// Get the edge mask
	cgRenderTargetHandle hEdgeMask = Operation.outputs[ 0 ];
    cgRenderTarget * pEdgeTexture = hEdgeMask.getResource(false);
	
	// Get the edge mask scratch chain from the pool 
	cgResampleChain * pEdgeMaskChain = mPool->getResampleChain( pEdgeTexture->getInfo().format );

	// Find the level in the scratch chain that matches our shadow map size
	cgUInt32 nStartLevel = pEdgeMaskChain->getLevelIndex( mResolution, mResolution ); 
	cgUInt32 nEndLevel   = nStartLevel + nNumMipLevels;

	// Are we doing hardware reads?
	bool bHardwareDepthRead = false;
    cgTextureHandle hDepthTexture = Operation.inputs[ 0 ].texture;
	cgTexture * pDepthTexture = hDepthTexture.getResource(false);
	if ( pDepthTexture->getInfo().type == cgBufferType::ShadowMap || pDepthTexture->getInfo().type == cgBufferType::DepthStencil )
		bHardwareDepthRead = true;

	// What depth format is stored in the texture we are about to sample?
	cgUInt32 nDepthType = (bHardwareDepthRead) ? cgDepthType::NonLinearZ : cgDepthType::LinearZ;

    // Build the script argument array
    cgScriptArgument::Array psArgs( 2 );
    bool bEdgeOnly = (mSettings.method & cgShadowMethod::EdgeMask) > 0;
    psArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bEdgeOnly );
    psArgs[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  &nDepthType );

	// Set pixel shader
    if ( !pShader->selectPixelShader( _T("computeEdgeExtents"), psArgs ) )
         return false;

    // Set our render target for capturing edges
    if ( mDriver->beginTargetRender( pEdgeMaskChain->getLevel( nStartLevel ), cgDepthStencilTargetHandle::Null ) )
    {
	    // Bind the shadow depth texture as input
		mPointSampler->apply( Operation.inputs[ 0 ].texture );

        // Set source image dimensions (including reciprocal)
        const cgVector4 vSize(
            (float)pDepthTexture->getInfo().width,
            (float)pDepthTexture->getInfo().height,
            1.0f / (float)pDepthTexture->getInfo().width,
            1.0f / (float)pDepthTexture->getInfo().height
        );
        pcbImage->setVector( _T("textureSize"), vSize );
        mDriver->setConstantBufferAuto( mImageProcessingConstants );

		// Set depth threshold
        pcbBilateral->setVector( _T("depthExtents"), cgVector2( mSettings.maskThreshold, mSettings.maskThreshold ) );
        mDriver->setConstantBufferAuto( mBilateralConstants );

        // Draw a fullscreen quad
        mDriver->drawClipQuad();

        // End rendering to target
        mDriver->endTargetRender();
    
    } // End if began

	////////////////////////////////////////////
	// Dilate edges
	////////////////////////////////////////////

	// Set the pixel shader
    psArgs.resize( 2 );
    psArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bEdgeOnly );
	psArgs[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  &mMaskChannels );
	if ( !pShader->selectPixelShader( _T("downsampleExtents2x2"), psArgs ) )
		 return false;
	
	// Run the downsample
	cgRenderTargetHandle hOutput;
    for ( cgUInt32 i = nStartLevel; i < nEndLevel; ++i )
    {
        // Set the output target
        if ( mDriver->beginTargetRender( pEdgeMaskChain->getLevel( i + 1 ), cgDepthStencilTargetHandle::Null ) )
        {
			// Set the input texture
            cgTextureHandle inputTexture = pEdgeMaskChain->getLevel( i );

		    // Bind textures and set sampler state
			if ( bEdgeOnly )
				mLinearSampler->apply( inputTexture );
			else
				mPointSampler->apply( inputTexture );

            // Update the constant buffer with source image dimensions (including reciprocal)
            cgTexture * pTexture = inputTexture.getResource( false );
            const cgVector4 vSize(
                (float)pTexture->getInfo().width,
                (float)pTexture->getInfo().height,
                1.0f / (float)pTexture->getInfo().width,
                1.0f / (float)pTexture->getInfo().height
            );
            pcbImage->setVector( _T("textureSize"), vSize );
            mDriver->setConstantBufferAuto( mImageProcessingConstants );

            // Draw a fullscreen quad
            mDriver->drawClipQuad();

            // End rendering to target
            mDriver->endTargetRender();
        
        } // End if began
    
    } // Next Level
	
	// Apply a final 3x3 filter to account for neighbors missed during downsampling
    if ( !pShader->selectPixelShader( _T("filterExtents3x3"), psArgs ) )
         return false;

	// The color write channels need to be based on our currently assigned channels
	// to prevent possible overwriting of edge data owned by other frustums
	mDriver->setBlendState( mBlendStates[ mMaskChannels ] );

    // Set our render target for capturing edges
    if ( mDriver->beginTargetRender( hEdgeMask, cgDepthStencilTargetHandle::Null ) )
    {
		// Set the input texture
        cgTextureHandle inputTexture = pEdgeMaskChain->getLevel( nEndLevel );

	    // Bind the shadow depth texture as input
		if ( bEdgeOnly )
			mLinearSampler->apply( inputTexture );
		else
			mPointSampler->apply( inputTexture );

        // Update the constant buffer with source image dimensions (including reciprocal)
        cgTexture * pTexture = inputTexture.getResource( false );
        const cgVector4 vSize(
            (float)pTexture->getInfo().width,
            (float)pTexture->getInfo().height,
            1.0f / (float)pTexture->getInfo().width,
            1.0f / (float)pTexture->getInfo().height
        );
        pcbImage->setVector( _T("textureSize"), vSize );
        mDriver->setConstantBufferAuto( mImageProcessingConstants );

        // Draw a fullscreen quad
        mDriver->drawClipQuad();

        // End rendering to target
        mDriver->endTargetRender();
    
    } // End if began

	// Clear the sampler(s)
	mPointSampler->apply( cgTextureHandle::Null );
	mLinearSampler->apply( cgTextureHandle::Null );

	// The edge mask is no longer dirty
	mMaskDirty = false;

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : copyEdgeToColor ()
/// <summary>
/// Copies the edge mask into the alpha channel of the color texture. This is 
/// used when the user has decided to minimize sampler registers and force the
/// two features to share a sampler slot.
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::copyEdgeToColor( const cgShadowGeneratorOperation & Operation )
{
    cgInt32 nMaskIndex  = getUserDataResourceIndex( cgShadowResourceType::EdgeMap );
	cgInt32 nColorIndex = getUserDataResourceIndex( cgShadowResourceType::ColorMap );

    // Get the surface shader
    cgSurfaceShader * pShader = mImageShader.getResource(true);

    // Get a pointer to the constant buffers
    cgConstantBuffer * pcbImage = mImageProcessingConstants.getResource(true);

    // Set rendering states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mBlendStates[ cgColorChannel::Alpha ] );

	// Select the vertex shader
    static const bool bFalse = false; 
    cgScriptArgument::Array vsArgs( 1 );
    vsArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bFalse );
    if ( !pShader->selectVertexShader( _T("transform"), vsArgs ) )
		return false;

    // Build the script argument array
    cgScriptArgument::Array psArgs( 2 );
    const bool bEdgeOnly = (mSettings.method & cgShadowMethod::EdgeMask) > 0;
    psArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bEdgeOnly );
    psArgs[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  &mMaskChannels );

	// Set pixel shader
    if ( !pShader->selectPixelShader( _T("copyEdgeToAlpha"), psArgs ) )
         return false;

    // Set our render target (color buffer) 
    if ( mDriver->beginTargetRender( Operation.outputs[ 0 ], cgDepthStencilTargetHandle::Null ) )
    {
	    // Bind the edge mask as input
	    mPointSampler->apply( Operation.inputs[ 0 ].texture );

        // Set source image dimensions (including reciprocal)
        const cgTexture * pTexture = Operation.inputs[ 0 ].texture.getResourceSilent();
        const cgVector4 vSize(
            (float)pTexture->getInfo().width,
            (float)pTexture->getInfo().height,
            1.0f / (float)pTexture->getInfo().width,
            1.0f / (float)pTexture->getInfo().height
        );
        pcbImage->setVector( _T("textureSize"), vSize );
        mDriver->setConstantBufferAuto( mImageProcessingConstants );

        // Draw a fullscreen quad
        mDriver->drawClipQuad();

        // End rendering to target
        mDriver->endTargetRender();
    }
	
	// Clear the texture
	mPointSampler->apply( cgTextureHandle::Null );

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createStatisticsMap () 
/// <summary>
/// Finalizes the resources for statistics based shadow computations (e.g., vsm, esm)
/// </summary>
//-----------------------------------------------------------------------------
bool cgShadowGenerator::createStatisticsMap( const cgShadowGeneratorOperation & Operation )
{
    // Configuration
    static const bool bLinearSampling = false;  // Assume point sampling (for now)
    static const cgInt32 nRadius = (cgInt32)mSettings.filterRadius;
	
	// Get the (intended) final target (i.e., the statistical map)
	cgRenderTargetHandle hSource = Operation.outputs[ 0 ];
	cgTexture * pStatsTexture = (cgTexture *)hSource.getResource( false );

	// Get the scratch target (based on the current resolution and format)
	cgResampleChain * pScratchChain = mPool->getResampleChain( pStatsTexture->getInfo().format );
	cgUInt32 nMipIndex = pScratchChain->getLevelIndex( pStatsTexture->getInfo().width, pStatsTexture->getInfo().height );
	cgRenderTargetHandle hScratch = pScratchChain->getLevel( nMipIndex );

    // Get the surface shader
    cgSurfaceShader * pShader = mImageShader.getResource(true);

    // Select the vertex shader
    cgScriptArgument::Array vsArgs( 1 );
    static const bool bFalse = false; 
    vsArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bFalse );
    if ( !pShader->selectVertexShader( _T("transform"), vsArgs ) )
        return false;

    // Set rendering states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( mBlendStates[ cgColorChannel::All ] );

	// Set constants
    const cgTexture * pTexture = Operation.inputs[ 0 ].texture.getResourceSilent();
    const cgVector4 vSize(
        (float)pTexture->getInfo().width,
        (float)pTexture->getInfo().height,
        1.0f / (float)pTexture->getInfo().width,
        1.0f / (float)pTexture->getInfo().height
    );
    cgConstantBuffer * pcbImage = mImageProcessingConstants.getResource(true);
	pcbImage->setVector( _T("textureSize"), vSize );
	mDriver->setConstantBufferAuto( mImageProcessingConstants );

    // Are we doing hardware reads?
    bool bHardwareDepthRead = false;
    if ( pTexture->getInfo().type == cgBufferType::ShadowMap || pTexture->getInfo().type == cgBufferType::DepthStencil )
        bHardwareDepthRead = true;

    // On the first vertical pass, convert the depth from linear/non-linear Z to whatever the method requires
    cgInt32 nConvertDepth = bHardwareDepthRead ? 2 : 1; // 0 = no convert, 1 = convert linear Z, 2 = convert non-linear Z
    
    // Build the script argument array
    cgScriptArgument::Array psArgs( 6 );
	psArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),   &mMethodHW );
	psArgs[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),   &nRadius );
	psArgs[2] = cgScriptArgument( cgScriptArgumentType::Float, _T("float"), &mSettings.filterBlurFactor );
	psArgs[3] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"),  &bLinearSampling );
	psArgs[4] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),   &nConvertDepth );

    // Set the pixel shader for first vertical pass
    static const bool bTrue = true;
	psArgs[5] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bTrue );
	cgPixelShaderHandle hVerticalPass0 = pShader->getPixelShader( _T("shadowBlur"), psArgs );

	// Turn off depth conversion for passes beyond the first
	nConvertDepth = 0; 

    // Select pixel shader for other vertical passes
	psArgs[5] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bTrue );
	cgPixelShaderHandle hVerticalPassN = pShader->getPixelShader( _T("shadowBlur"), psArgs );

    // Select pixel shader for horizontal passes
	psArgs[5] = cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bFalse );
	cgPixelShaderHandle hHorizontalPass = pShader->getPixelShader( _T("shadowBlur"), psArgs );

	// Ensure we got valid pixel shaders
    if ( !hVerticalPass0.isValid() || !hVerticalPassN.isValid() || !hHorizontalPass.isValid() )
         return false;

	// Pre-bind the scratch surface as the horizontal pass texture
	if ( bLinearSampling )
		mHorizontalLinearSampler->apply( hScratch );
	else
		mHorizontalPointSampler->apply( hScratch );

	// Run the blur pass(es). 
	for ( cgUInt32 i = 0; i < mSettings.filterPasses; ++i )
	{
		// The first pass will convert linear or non-linear Z to statistical format.
		if ( i == 0 )
		{
			// Get the source texture (check for a readable depth-stencil)
			mVerticalPointSampler->apply( Operation.inputs[ 0 ].texture );
		}
		// Any remaining passes can just use the statistical map directly, so bind once
		else if ( i == 1 )
		{
			if ( bLinearSampling )
				mVerticalLinearSampler->apply( hSource );
			else
				mVerticalPointSampler->apply( hSource );
		
        } // End if second pass

		// Run the vertical blur
		if ( !pShader->selectPixelShader( (i == 0) ? hVerticalPass0 : hVerticalPassN ) )
            break;

		// Set our render target (color buffer) 
		if ( mDriver->beginTargetRender( hScratch, cgDepthStencilTargetHandle::Null ) )
		{
			// Draw a fullscreen quad
			mDriver->drawClipQuad();

			// End rendering to target
			mDriver->endTargetRender();
		}

		// Run the horizontal blur
		if ( !pShader->selectPixelShader( hHorizontalPass ) )
            break;

		// Set our render target (color buffer) 
		if ( mDriver->beginTargetRender( hSource, cgDepthStencilTargetHandle::Null ) )
		{
			// Draw a fullscreen quad
			mDriver->drawClipQuad();

			// End rendering to target
			mDriver->endTargetRender();
		}
	}

	// If we are using automatic mip map generation, trigger it now
	if ( pStatsTexture->getInfo().autoGenerateMipmaps )
		pStatsTexture->updateMipLevels();

	// Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name: buildOrthoMatrices
// Desc: Builds an orthographic shadow transformation matrix using a fixed frustum
//       to minimize aliasing during camera movement/rotation
//-----------------------------------------------------------------------------
void cgShadowGenerator::buildOrthoMatrices( const cgBoundingBox & CasterBounds, const cgFrustum & SplitFrustum )
{
	cgVector3 vTransformed[ 8 ];
	
	// Get current LOD settings 
	cgFloat ShadowMapSize   = (cgFloat)getResolution();
	cgFloat NumBorderTexels = ( mSettings.filterRadius - 1.0f ) / 2.0f;

    // ToDo:
	// For light position, use scene center point.
    // getScene( )->GetSpatialTree( )->GetSceneBounds( sceneMin, sceneMax );
	cgVector3 vLightPos(0,15000,0); // SceneBounds.GetCenter();

	// Orientate the camera appropriately for this light source.
    mCamera->lookAt( vLightPos, vLightPos + mParent->getZAxis(false) );
	const cgMatrix & mtxView = mCamera->getViewMatrix();

	// Transform caster box into view space for later use
	cgBoundingBox ViewCasterBounds = CasterBounds;
	ViewCasterBounds.transform( mtxView );

	// Build a light space sphere around the input frustum.
    cgBoundingBox ViewFrustumBounds;
    cgVector3 SphereCenter( 0, 0, 0 );
	for ( cgInt i = 0; i < 8; i++ )
	{
		cgVector3::transformCoord( vTransformed[ i ], SplitFrustum.points[ i ], mtxView );
		SphereCenter += vTransformed[ i ];
		ViewFrustumBounds.addPoint( vTransformed[ i ] ); 
	
    } // Next point
	SphereCenter /= 8.0f;

	// Our initial projection window center will be the sphere center
    cgVector2 ProjWindowCenter( SphereCenter.x, SphereCenter.y );
	
    // Our initial projection window size will be the diameter of our sphere
    // Radius is fixed for each split (pre-computed).
	cgFloat ProjWindowSize = mSphereRadius * 2.0f;

	// Adjust the projection window size to account for filtering (border texels)
	ProjWindowSize *= ( ShadowMapSize + NumBorderTexels ) / ShadowMapSize;

	// Snap projection window to texel boundaries (reduces aliasing).
	cgFloat texelSize   = ProjWindowSize / ShadowMapSize; 
	ProjWindowCenter.x -= fmod( ProjWindowCenter.x, texelSize );
	ProjWindowCenter.y -= fmod( ProjWindowCenter.y, texelSize ); 

	// Use light view space distances to build our orthogonal projection matrix.
	ProjWindowSize *= 0.5f;
    mCamera->setProjectionMode( cgProjectionMode::Orthographic );
    mCamera->setProjectionWindow( ProjWindowCenter.x - ProjWindowSize, ProjWindowCenter.x + ProjWindowSize,
									ProjWindowCenter.y - ProjWindowSize, ProjWindowCenter.y + ProjWindowSize );
    mCamera->setNearClip( ViewCasterBounds.min.z );
    mCamera->setFarClip( ViewFrustumBounds.max.z );
}

//-----------------------------------------------------------------------------
//  Name : buildOrthoMatrices () (Private)
/// <summary>
/// Builds PSSM view and projection matrices based on the the input
/// bounding boxes.
/// </summary>
//-----------------------------------------------------------------------------
void cgShadowGenerator::buildOrthoMatrices( const cgBoundingBox & CasterBounds, const cgBoundingBox & ReceiverBounds )
{
	// ToDo:
	// For light position, use scene center point.
	cgVector3 vLightPos(0,15000,0); // SceneBounds.GetCenter();

	// Orientate the camera appropriately for this light source.
    mCamera->lookAt( vLightPos, vLightPos + mParent->getZAxis(false) );

	// Transform frustum and caster bounds to light view space. 
	cgBoundingBox ViewReceiverBounds = ReceiverBounds;
    ViewReceiverBounds.transform( mCamera->getViewMatrix() );
    cgBoundingBox ViewCasterBounds = CasterBounds;
    ViewCasterBounds.transform( mCamera->getViewMatrix() );
	
	// Use light view space distances to build our orthogonal projection matrix.
    mCamera->setProjectionMode( cgProjectionMode::Orthographic );
    mCamera->setProjectionWindow( ViewReceiverBounds.min.x, ViewReceiverBounds.max.x,
									ViewReceiverBounds.min.y, ViewReceiverBounds.max.y );
    mCamera->setNearClip( ViewCasterBounds.min.z );
    mCamera->setFarClip( ViewReceiverBounds.max.z );
}

///////////////////////////////////////////////////////////////////////////////
// cgShadowGenerator Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgReflectanceGenerator () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgReflectanceGenerator::cgReflectanceGenerator( cgLightNode * pParentLight, cgUInt32 nFrustumIndex ) : cgShadowGenerator( pParentLight, nFrustumIndex )
{
    mType                = cgShadowGeneratorType::ReflectiveShadowMap;
	mRSMResolutionFinal = 0;
	mRSMSampler0        = CG_NULL;
	mRSMSampler1        = CG_NULL;
	mRSMSampler2        = CG_NULL;
	mResampleChainIndex = -1;
}

//-----------------------------------------------------------------------------
//  Name : ~cgReflectanceGenerator () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgReflectanceGenerator::~cgReflectanceGenerator()
{
    // Dispose of resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgReflectanceGenerator::dispose( bool bDisposeBase )
{
    // Release resources
    mRSMConstants.close();
    mRandomOffsetTexture.close();

    // Cleanup samplers
    if ( mRSMSampler0 )
        mRSMSampler0->scriptSafeDispose();
    if ( mRSMSampler1 )
        mRSMSampler1->scriptSafeDispose();
    if ( mRSMSampler2 )
        mRSMSampler2->scriptSafeDispose();

    // Clear variables
    mRSMSampler0 = CG_NULL;
    mRSMSampler1 = CG_NULL;
    mRSMSampler2 = CG_NULL;	

    // Dispose base if requested
    if ( bDisposeBase )
        cgShadowGenerator::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Allow the shadow frustum to initialize its internal resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::initialize( )
{
	// Call base class initialize function
	cgShadowGenerator::initialize( );

    // Create rsm constant buffer
	if ( !mResourceManager->createConstantBuffer( &mRSMConstants, mShader, _T("_cbRSM"), cgDebugSource() ) )
	{
		cgAppLog::write( cgAppLog::Error, _T("Failed to generate required shadow frustum RSM constant buffers for shadow generator '%s' of object node 0x%x in scene '%s'.\n"), mName.c_str(), mParent->getReferenceId(), mParent->getScene()->getName().c_str() );
		return false;
    
	} // End if failed
	cgAssert( mRSMConstants->getDesc().length == sizeof(_cbRSM) );

    // Setup samplers
    mRSMSampler0 = mResourceManager->createSampler( _T("Image0"), mImageShader );
    mRSMSampler1 = mResourceManager->createSampler( _T("Image1"), mImageShader );
    mRSMSampler2 = mResourceManager->createSampler( _T("Image2"), mImageShader );

    cgSamplerStateDesc smpStates;
    smpStates.minificationFilter = cgFilterMethod::Point;
    smpStates.magnificationFilter = cgFilterMethod::Point;
    smpStates.mipmapFilter = cgFilterMethod::None;
    smpStates.addressU  = cgAddressingMode::Clamp;
    smpStates.addressV  = cgAddressingMode::Clamp;
    smpStates.addressW  = cgAddressingMode::Clamp;
	mRSMSampler0->setStates( smpStates );
	mRSMSampler1->setStates( smpStates );
	mRSMSampler2->setStates( smpStates );

	// Load random offset texture
	if ( !mResourceManager->loadTexture( &mRandomOffsetTexture, "systex://RandomPoints.dds", 0, cgDebugSource() ) )
	{
		cgAppLog::write( cgAppLog::Warning, _T("Failed to load the system's random point offsets texture for use during RSM sampling.\n") );
	}

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getResolution ()
/// <summary>
/// Retrieve the final rsm resolution used
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgReflectanceGenerator::getResolution( ) const
{
    return mRSMResolutionFinal;
}

//-----------------------------------------------------------------------------
//  Name : setConstants ()
/// <summary>
/// Setup constant buffers for reading
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::setConstants( )
{
	// Call the base class
	if ( !cgShadowGenerator::setConstants() )
		return false;

	// Setup additional shadowing parameters
    cgConstantBuffer * pRSMParamsBuffer = mRSMConstants.getResource( true );
    cgAssert( pRSMParamsBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbRSM * pRSMData = CG_NULL;
    if ( !(pRSMData = (_cbRSM*)pRSMParamsBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock shadow parameter constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Update constants - rsm buffer size first.
    pRSMData->textureSize.x = (cgFloat)mResolution;
    pRSMData->textureSize.y = pRSMData->textureSize.x;
    pRSMData->textureSize.z = 1.0f / pRSMData->textureSize.x;
    pRSMData->textureSize.w = 1.0f / pRSMData->textureSize.y;

	// Get the size of the "merged" texture from the render driver to compute UV scale/bias terms
	cgSize mergedSize = mDriver->getVPLTextureDimensions();
	pRSMData->textureScaleBias.x = pRSMData->textureSize.x / (cgFloat)mergedSize.width;
	pRSMData->textureScaleBias.y = pRSMData->textureSize.y / (cgFloat)mergedSize.height;
	pRSMData->textureScaleBias.z = 0;
	pRSMData->textureScaleBias.w = 0;

    // Compute the shadow map screen projection matrix (projects
    // the shadow map depth texture onto the geometry from the point
    // of view of the main rendering camera).
    const cgMatrix & mtxView = mCamera->getViewMatrix();
    const cgMatrix & mtxProj = mCamera->getProjectionMatrix();
	pRSMData->textureProjectionMatrix = mtxView * mtxProj;

	// Compute scale/bias for converting tex coords & z to view space
    pRSMData->screenToViewScaleBias.x =   2.0f / mtxProj._11;
    pRSMData->screenToViewScaleBias.y =  -2.0f / mtxProj._22;
    pRSMData->screenToViewScaleBias.z =  (-1.0f / mtxProj._11) + (-mtxProj._41 / mtxProj._11);
    pRSMData->screenToViewScaleBias.w =  ( 1.0f / mtxProj._22) + (-mtxProj._42 / mtxProj._22);

	// Compute a term for unpacking depth and rescaling to camera space Z
	float fNear      = mCamera->getNearClip();
	float fFar       = mCamera->getFarClip();
	float fRange     = fFar - fNear;
	float fRangeBias = -fNear / fRange;
    float fScale     = fRange;          
    float fBias      = fRange * -fRangeBias; 
	pRSMData->depthUnpack.x = fScale;
	pRSMData->depthUnpack.y = fScale / 255.0f;
	pRSMData->depthUnpack.z = fScale / 65535.0f;
	pRSMData->depthUnpack.w = fBias;
	pRSMData->position      = mCamera->getPosition(true);
	pRSMData->direction     = mCamera->getZAxis(true);
	pRSMData->sampleRadius  = mSettings.filterRadius; 
	pRSMData->geometryBias  = mSettings.minimumCutoff;    
	
	// Get the HDR scale if applicable (rsm renders are ldr by default)
	pRSMData->color.x = mParent->getDiffuseColor().r * mSettings.intensity;
	pRSMData->color.y = mParent->getDiffuseColor().g * mSettings.intensity;
	pRSMData->color.z = mParent->getDiffuseColor().b * mSettings.intensity;
	if ( mDriver->getSystemState( cgSystemState::HDRLighting ) > 0 )
		pRSMData->color *= mParent->getDiffuseHDRScale();

	// Compute the inverse view matrix 	
	cgMatrix::inverse( pRSMData->inverseViewMatrix, mtxView );
	
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pRSMParamsBuffer->unlock();

    // Apply the constant buffer
	return mDriver->setConstantBufferAuto( mRSMConstants );
}

//-----------------------------------------------------------------------------
//  Name : beginRead ()
/// <summary>
/// Setup states relating to this shadow frustum that are required for
/// rendering the shadow / attenuation data to the attenuation mask.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::beginRead( cgFloat fAttenuation, cgFloat fMinDistance, cgFloat fMaxDistance, const cgMatrix * pLightTexProj /* = CG_NULL */ )
{
	// Call the base class begin read method
	cgShadowGenerator::beginRead( fAttenuation, fMinDistance, fMaxDistance, pLightTexProj );

	// If using propagation volumes
	if ( mParent->getIndirectLightingMethod() == cgIndirectLightingMethod::PropagationVolumes )
	{
		cgTextureHandle hDepth  = mResources[ getUserDataResourceIndex( cgShadowResourceType::DepthMap ) ]->getResource();
		cgTextureHandle hNormal = mResources[ getUserDataResourceIndex( cgShadowResourceType::NormalMap ) ]->getResource();
		mDriver->setVPLData( hDepth, hNormal );
	
    } // End if VPL
	else
	{
		// How will we be sampling the rsm? 
		cgUInt32 nSamplingMethod = 0;
		if ( mSettings.projectCell )
			nSamplingMethod = 1;
		else if ( mSettings.boxFilter )
			nSamplingMethod = 2;

		// Set system states
		mDriver->setSystemState( cgSystemState::ShadowMethod,  nSamplingMethod );
		mDriver->setSystemState( cgSystemState::PrimaryTaps,   mSettings.primarySamples );
		mDriver->setSystemState( cgSystemState::SecondaryTaps, mSettings.secondarySamples );

	} // End if RSM

	// Setup constant buffers
	return setConstants();
}

//-----------------------------------------------------------------------------
//  Name : update()
/// <summary>
/// Updates the generator in terms of resources and operations
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::update( cgUInt32 nResolution, const cgShadowSettingsSystem & SystemSettings, const cgShadowSettingsLight & LightSettings, const cgTexturePoolResourceDesc::Array & aDescriptions, cgInt32 nFlags )
{
	// Cache previous values
	cgUInt32 nPrevFlags         = mMethodHW;
	cgUInt32 nPrevResolution    = mResolution;
	cgUInt32 nPrevResolutionRSM = mRSMResolutionFinal;

	// Record the new flags and resolution
	mMethodHW           = nFlags;
	mResolution         = nResolution;
	mRSMResolutionFinal = nResolution;

	// If we are downsampling, capture new resolution
	if ( SystemSettings.boxFilter )
		 mRSMResolutionFinal = SystemSettings.primarySamples;

	// Update our settings
	mSettings = cgShadowSettings( SystemSettings, LightSettings );

	// If the method/flags changed, the description is considered dirty
	if ( nFlags != nPrevFlags )
		mDescriptionDirty = true;

	// If the resolution has changed, we will assume the need for all 
	// new resources, so clear everything
	if ( (nResolution != nPrevResolution) )
	{
		releaseResources();
		mDescriptionDirty = true;

	} // End if resolution changed
	
	// If we are downsampling differently than before, update
	if ( mRSMResolutionFinal != nPrevResolutionRSM )
		mDescriptionDirty = true;

	// If the description is dirty
	if ( mDescriptionDirty )
	{
		// Replace the resource description list
		mDescriptions.clear();
		mDescriptions = aDescriptions;

		// Update sampler states
		updateSamplerStates();

		// As we've changed the description, assign resources
		assignResources();

		// Build operations lists
		if ( !buildOperations() )
			return false;

		// The description is no longer dirty
		mDescriptionDirty = false;
	
    } // End if dirty

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : buildOperations () 
/// <summary>
/// Builds a list of operations required to read and write rsms
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::buildOperations( )
{
	// Clear the current operations lists
	mWriteOps.clear();
	mPostOps.clear();
	mReadOps.clear();
	
	// Add a g-buffer writing operation
    cgShadowGeneratorOperation WritePass( WriteGBuffer );
	WritePass.depthStencil = cgDepthStencilTargetHandle( (cgDepthStencilTarget*)mResources[ 0 ]->getResource().getResourceSilent() );
	WritePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 1 ]->getResource().getResourceSilent() ) );
	WritePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 2 ]->getResource().getResourceSilent() ) );
	WritePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 3 ]->getResource().getResourceSilent() ) );
	WritePass.clearFlags      = cgClearFlags::Target | cgClearFlags::Depth;
	WritePass.clearDepthValue = 1.0f;
	WritePass.clearColorValue = 0xFFFFFFFF;
	WritePass.colorWrites     = cgColorChannel::All;
	WritePass.cullMode         = cgCullMode::Back;

	// Add the depth pass
	mWriteOps.push_back( WritePass );

	// Get access to the (optional) merged map
	cgInt32 nMergedMapIndex = getUserDataResourceIndex( cgShadowResourceType::DepthNormalMap );

	// If we have been asked to downsample the RSM for reading...
	cgShadowGeneratorOperation ReadPass( ReadGBuffer );
	if ( mSettings.boxFilter )
	{
		// Generate a new post operation
		cgShadowGeneratorOperation DownsamplePass( DownsampleRSMMax );
		DownsamplePass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
		DownsamplePass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 2 ]->getResource(), mSamplerStates[ 2 ] ) ); 
		DownsamplePass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ 3 ]->getResource(), mSamplerStates[ 3 ] ) ); 
		DownsamplePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 4 ]->getResource().getResourceSilent() ) );
		DownsamplePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 5 ]->getResource().getResourceSilent() ) );
		DownsamplePass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 6 ]->getResource().getResourceSilent() ) );

		// Add the downsample pass
		mPostOps.push_back( DownsamplePass );

		// Build a read operation using the low res rsm
		if ( mSettings.method & cgIndirectLightingMethod::VTF )
		{
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister, mResources[ nMergedMapIndex ]->getResource(), mSamplerStates[ nMergedMapIndex ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mColorSamplerRegister, mResources[ 6 ]->getResource(), mSamplerStates[ 6 ] ) ); 
		
        } // End if VTF
		else
		{
			// Build a read operation using the low res rsm
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister,  mResources[ 4 ]->getResource(), mSamplerStates[ 4 ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mCustomSamplerRegister, mResources[ 5 ]->getResource(), mSamplerStates[ 5 ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mColorSamplerRegister,  mResources[ 6 ]->getResource(), mSamplerStates[ 6 ] ) ); 
		
        } // End if other
	
    } // End if boxFilter
	else
	{
		// Build a read operation using the high res rsm
		if ( mSettings.method & cgIndirectLightingMethod::VTF )
		{
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister,  mResources[ nMergedMapIndex ]->getResource(), mSamplerStates[ nMergedMapIndex ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mColorSamplerRegister,  mResources[ 3 ]->getResource(), mSamplerStates[ 3 ] ) ); 
		
        } // End if VTF
		else
		{
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister,  mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mCustomSamplerRegister, mResources[ 2 ]->getResource(), mSamplerStates[ 2 ] ) ); 
			ReadPass.inputs.push_back( cgShadowGeneratorInput( mColorSamplerRegister,  mResources[ 3 ]->getResource(), mSamplerStates[ 3 ] ) ); 
		
        } // End if other
	
    } // End if !boxFilter

	// To allow taking random samples from the RSM, bind a randomized number texture
	ReadPass.inputs.push_back( cgShadowGeneratorInput( mRandomSamplerRegister, mRandomOffsetTexture, mSamplerStates[ 1 ] ) ); 

	// Add the read pass
	mReadOps.push_back( ReadPass );

	// If we are doing LPV injections with VTF, we need a pass to generate a vertex shader 
	// readable depth/normal texture from our g-buffer
	if ( mSettings.method & cgIndirectLightingMethod::VTF )
	{
		// Generate a new post operation
		cgShadowGeneratorOperation MergeBuffersPass( MergeDepthNormal );
		MergeBuffersPass.inputs.push_back( cgShadowGeneratorInput( mDepthSamplerRegister,  mResources[ 1 ]->getResource(), mSamplerStates[ 1 ] ) ); 
		MergeBuffersPass.inputs.push_back( cgShadowGeneratorInput( mCustomSamplerRegister, mResources[ 2 ]->getResource(), mSamplerStates[ 2 ] ) ); 
		MergeBuffersPass.outputs.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ getUserDataResourceIndex( cgShadowResourceType::DepthNormalMap ) ]->getResource().getResourceSilent() ) );

		// Add the merge pass
		mPostOps.push_back( MergeBuffersPass );
	
    } // End if VTF

	// Return success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : executePostOperation()
/// <summary>
/// Runs a post-processing operation
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::executePostOperation( const cgShadowGeneratorOperation & Operation )
{
	switch( Operation.operationTypeId )
	{
		case DownsampleRSMMin:
		case DownsampleRSMMax:
		case DownsampleRSMAvg:
			return downsampleRSM( Operation );
		    break;
		case MergeDepthNormal:
			return mergeDepthNormalBuffers( Operation );
		    break;
	}
	return false;
}

//-----------------------------------------------------------------------------
//  Name : downsampleRSM ()
/// <summary>
/// Downsamples the RSM to a requested size
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::downsampleRSM( const cgShadowGeneratorOperation & Operation )
{
    // ToDo: 6767
	/*// If we don't have access to a resample chain, ask for one
	if ( mResampleChainIndex < 0 )
	{
		std::vector< cgBufferFormat::Base > Formats;
		Formats.push_back( mDescriptions[ 1 ].bufferDesc.format );
		Formats.push_back( mDescriptions[ 2 ].bufferDesc.format );
		Formats.push_back( mDescriptions[ 3 ].bufferDesc.format );
		mResampleChainIndex = mPool->CreateMRTResampleChain( Formats, mResolution, false );
	
    } // End if no chain

	// If we still don't have a valid chain, we cannot proceed
	if ( mResampleChainIndex < 0 )
		return false;
	
    // Get the surface shader
    cgSurfaceShader * pShader = mImageShader.getResource(true);

	// Select the vertex shader
    static const bool bFalse = false; 
    cgScriptArgument::Array vsArgs( 1 );
    vsArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bFalse );
    if ( !pShader->selectVertexShader( _T("transform"), vsArgs ) )
		return false;

	// Select the pixel shader
    cgScriptArgument::Array psArgs( 1 );
    psArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &Operation.operationTypeId );
	if ( !pShader->selectPixelShader( _T("downsampleRSM"), psArgs ) )
		 return false;

	// Setup our constant buffer
	setConstants();

    // Get a pointer to the constant buffers
    cgConstantBuffer * pcbImage = mImageProcessingConstants.getResource(true);

    // Set rendering states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( cgBlendStateHandle::Null );

	// Get the resampling chain
	cgResampleChainMRT * pChain = mPool->GetResampleChainMRT( mResampleChainIndex ); 

	// Temporarily place our high res rsm at the top of the chain
	cgRenderTargetHandleArray aHighResRSM;
	aHighResRSM.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 1 ]->getResource().getResourceSilent()) );
    aHighResRSM.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 2 ]->getResource().getResourceSilent()) );
    aHighResRSM.push_back( cgRenderTargetHandle( (cgRenderTarget*)mResources[ 3 ]->getResource().getResourceSilent()) );
	pChain->SetLevel( 0, aHighResRSM );

	// Temporarily place our low res rsm at the correct chain mip level
	cgUInt32 nLevel = pChain->getLevelIndex( mRSMResolutionFinal, mRSMResolutionFinal );
	pChain->SetLevel( nLevel, Operation.outputs );

	// Run the downsample
	cgUInt32 nNumLevels = pChain->GetLevelCount();
    for ( cgUInt32 i = 0; i < (nNumLevels - 1); ++i )
    {
		// Get our input and output targets
		cgRenderTargetHandleArray hInput  = pChain->getLevel( i );
		cgRenderTargetHandleArray hOutput = pChain->getLevel( i + 1 );

        // Set the output targets
        if ( mDriver->beginTargetRender( hOutput, cgDepthStencilTargetHandle::Null ) )
        {
			// Set the input textures
			mRSMSampler0->apply( hInput[ 0 ] );
			mRSMSampler1->apply( hInput[ 1 ] );
			mRSMSampler2->apply( hInput[ 2 ] );

            // Update the constant buffer with source image dimensions (including reciprocal)
            const cgTexture * pTexture = hInput[ 0 ].getResourceSilent();
            const cgVector4 vSize(
                (float)pTexture->getInfo().width,
                (float)pTexture->getInfo().height,
                1.0f / (float)pTexture->getInfo().width,
                1.0f / (float)pTexture->getInfo().height
            );
            pcbImage->setVector( _T("textureSize"), vSize );
            mDriver->setConstantBufferAuto( mImageProcessingConstants );

            // Draw a fullscreen quad
            mDriver->drawClipQuad();

            // End rendering to target
            mDriver->endTargetRender();

        } // End render
    
	} // Next level

	// Restore the targets we replaced above
	pChain->RestoreLevel( 0 );
	pChain->RestoreLevel( nLevel );*/

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : MergeDepthNormal ()
/// <summary>
/// Merges depth and normal buffers into a single texture for vertex texture fetch
/// </summary>
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::mergeDepthNormalBuffers( const cgShadowGeneratorOperation & Operation )
{
	bool bFalse = false; 

    // Get the surface shader
    cgSurfaceShader * pShader = mShader.getResource(true);

	// Select the pixel shader
	if ( !pShader->selectPixelShader( _T("mergeDepthAndNormalBuffers") ) )
		 return false;

    // Set rendering states
    mDriver->setDepthStencilState( mDisabledDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );
	mDriver->setBlendState( cgBlendStateHandle::Null );

	// Setup the constant buffer(s) we need
	setConstants();

    // Set the output targets
    if ( mDriver->beginTargetRender( Operation.outputs, cgDepthStencilTargetHandle::Null ) )
    {
		// Set the input textures
		mRSMSampler0->apply( Operation.inputs[ 0 ].texture );
		mRSMSampler1->apply( Operation.inputs[ 1 ].texture );

        // Draw a fullscreen quad
        mDriver->drawScreenQuad();

        // End rendering to target
        mDriver->endTargetRender();

    } // End render

	// Success
	return true;
}


//-----------------------------------------------------------------------------
// Name : computeVisibilitySet( ) (Virtual)
// Desc : Creates the shadow mapping visibility set that contains objects also 
//        found within the parent light's shadow set. This  essentially narrows 
//        down the list of visible objects from the point of view of the light 
//        source to only those that can both be seen by the camera and cast
//        shadows with respect the parent light.
// Note : Returns true if this frustum contains visible casters that could
//        potentially affect the shadow map (whether the shadow map will
//        ultimately be regenerated or not).
//-----------------------------------------------------------------------------
bool cgReflectanceGenerator::computeVisibilitySet( cgCameraNode * pSceneCamera, bool bParallelSplit /* = false */, cgFloat fNearSplitDistance /* = 0.0f */, cgFloat fFarSplitDistance /* = 0.0f */ )
{
    // Get access to required systems.
    cgScene * pScene = mParent->getScene();

    // Do not regenerate reflective shadow map until we know something has changed.
    mRegenerate = false;

    // We want to process the frustum camera's current visibility set.
    cgVisibilitySet * pFrustumVis = mCamera->getVisibilitySet();

    // If the light source moved or was updated, we can immediately
    // consider this frustum to be dirty.
    if ( !mLastVisibilityFrame || mParent->isNodeDirtySince( mLastVisibilityFrame ) )
    {
        mRegenerate = true;
    
    } // End if light source altered
    else
    {
        // The light source wasn't altered in any way, check shadow casting objects.
        cgObjectNodeSet::const_iterator itObject;
        const cgObjectNodeArray & Objects = pFrustumVis->getVisibleObjects();
        for ( size_t i = 0; i < Objects.size(); ++i )
        {
            // If the object was deleted, or has been altered (moved, animated etc.)
            // then we must also regenerate the shadow map.
            
            // ToDo: We can probably replace isValidReference() (as well as 
            // removing the 'm_ValidReferences' set in the reference manager) with
            // a call to 'GetReference( id )' if we stored the original reference 
            // id in the visibility set (i.e. m_ObjectNodes is a map instead of a set).
            cgObjectNode * pNode = Objects[i];
            if ( !cgReferenceManager::isValidReference( pNode ) || pNode->isNodeDirtySince( mLastVisibilityFrame ) )
            {
                mRegenerate = true;
                break;

            } // End if object altered

        } // Next object

    } // End if light source static

    // If the light source is employing an orthographic parallel split
    // rendering approach, we need to compute a new projection matrix.
    if ( bParallelSplit )
    {
        // ToDo: 6767
        /*// Parallel split frustum is aligned to the main scene camera (not light source).
        // Build a custom camera using similar details to the scene camera, but adjust the
        // near / far clip planes to match those for the split so that we can query for objects
        // that fall within it.
        if ( mSplitCamera != CG_NULL )
            mSplitCamera->scriptSafeDispose();
        mSplitCamera = new cgCameraNode( 0, pSceneCamera->getScene(), pSceneCamera, cgCloneMethod::Instance, pSceneCamera->getWorldTransform() );
        mSplitCamera->setNearClip( fNearSplitDistance );
        mSplitCamera->setFarClip( fFarSplitDistance );
        
        // Get the sub-frustum for this split
		const cgFrustum & SplitFrustum = mSplitCamera->getFrustum();

		// If we haven't already, compute the radius of the bounding sphere around this frustum. It is 
		// important that we only do this the one time. Recomputing it per frame introduces very small floating
		// point errors that result in minor flickering. Caching this value now removes the problem.
		if ( mSphereRadius <= 0.0f )
		{
			cgVector3 SphereCenter(0,0,0);
			for( cgInt32 i = 0; i < 8; i++ )
				SphereCenter += SplitFrustum.points[ i ];
			SphereCenter /= 8.0f;
			for ( cgInt32 i = 0; i < 8; i++ )
			{
				float r = cgVector3::length( &(SphereCenter - SplitFrustum.points[ i ]) );
				if( r > mSphereRadius ) mSphereRadius = r;
			
            } // Next point
		
        } // End if no radius computed

		// Compute caster/receiver list and bounds
        pFrustumVis->Clear();
        cgVisibilitySet * pShadowSet       = mParent->GetShadowSet();
        cgVisibilitySet * pIlluminationSet = mParent->GetIlluminationSet();
		pShadowSet->Intersect( SplitFrustum, mParent->getZAxis(false), pFrustumVis, mCasterBounds );
    
		// ToDo: Figure out how best to handle the AllowLightmappedCasters option with PSSM...

		// ToDo: Note... Always adds receivers, even if this is not variance. I think we agree this was fine (we no longer
		// take this into consideration anywhere else I believe). Making a note however just in case.
        // Additional Note.... The dirty system may not work if we don't.
		pIlluminationSet->Intersect( SplitFrustum, mParent->getZAxis(false), pFrustumVis, mReceiverBounds );*/
    
    } // End if Parallel Split
    else
    {
        // Build our object filter for choosing casters to render into the shadowmap
        cgUInt32 nFlags = cgVisibilitySearchFlags::MustRender | cgVisibilitySearchFlags::MustCastShadows |
                          cgVisibilitySearchFlags::CollectMaterials;

		// Compute the new filtered visibility set for this frustum.
		mCamera->computeVisibility( nFlags );

        // ToDo: Figure out how to filter objects that can't cast a shadow
        // into the camera's frustum (i.e. perspective extrude AABB?). 

    } // End if Standard

    // Visibility has been recomputed on this frame.
    mLastVisibilityFrame = cgTimer::getInstance()->getFrameCounter();

    // If there is nothing for us to do, we can let the caller 
    // know to turn shadows off.
    if ( pFrustumVis->isEmpty() )
    {
        mRegenerate = false;
        return false;
    
    } // End if no casters

    // If we still don't think we need to regenerate, has anything been created
    // or moved recently such that it now exists in the new view?
    if ( !mRegenerate )
    {
        const cgObjectNodeArray & Objects = pFrustumVis->getVisibleObjects();
        for ( size_t i = 0; i < Objects.size(); ++i )
        {
            // If the object is dirty, we must regenerate!
            if ( Objects[i]->isNodeDirtySince( mLastVisibilityFrame ) )
            {
                mRegenerate = true;
                break;

            } // End if object altered

        } // Next object

    } // End if !bRegenerate

    // Casters are visible!
    return true;
}