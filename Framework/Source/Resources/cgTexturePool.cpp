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
// File : cgTexturePool.cpp                                                  //
//                                                                           //
// Desc : Contains classes responsible for managing the various different    //
//        types of supported textures.                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgTexturePool Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgTexturePool.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgVertexBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgResampleChain.h>
#include <Rendering/cgVertexFormats.h>
#include <Math/cgMathUtility.h>

//-----------------------------------------------------------------------------
// Global values for ATI fetch 4 in DX9
//-----------------------------------------------------------------------------
DWORD g_4CC_Fetch4Enable  = ((DWORD)MAKEFOURCC( 'G', 'E', 'T', '4' ));
DWORD g_4CC_Fetch4Disable = ((DWORD)MAKEFOURCC( 'G', 'E', 'T', '1' ));

///////////////////////////////////////////////////////////////////////////////
// cgTexturePoolResource Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTexturePoolResource ()
/// <summary>
/// Class constructor
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePoolResource::cgTexturePoolResource( cgTexturePoolResourceType::Base ResourceType )
{
	mDescription.type  = ResourceType;
	mAvailableChannels = 0;
	mLastAssigned      = 0.0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTexturePoolResource ()
/// <summary>
/// Class destructor
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePoolResource::~cgTexturePoolResource( )
{
    // Release resources
    mResource.close();
    
    // Clear variables
    mChannelAssignments.clear();
}

//-----------------------------------------------------------------------------
//  Name : assign ()
/// <summary>
/// Attempts to assign the input generator (with optional channel reuse) to this
/// resource and updates the number of available channels accordingly. If all
/// available channels are occupied, this resource is removed from the available
/// pool (assuming it was a pool map to begin with).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTexturePoolResource::assign( void * pOwner, cgUInt32 nDataTypeId, cgUInt32 nChannelCount, cgUInt32 nPreviousChannelMask /* = 0 */ )
{
	// Nobody can own a shared resource
	if ( mDescription.type == cgTexturePoolResourceType::Shared )
		return cgColorChannel::All;

	// If caller specified 0, this means ALL channels are to be used
	if ( nChannelCount == 0 )
		nChannelCount = mDescription.channelCount;

	// Do we have enough channels to accommodate this map?
	if ( nChannelCount > mAvailableChannels )
		return cgColorChannel::None;

	// Attempt to reuse previous channels if supplied?
	if ( nPreviousChannelMask )
	{
		// If the channels are still available, reuse them
		if ( channelsAvailable( nPreviousChannelMask ) )
		{
            // Re-assign the owner to the specified channels.
			for ( cgUInt32 i = 0; i < mDescription.channelCount; i++ )
			{
				if ( (1L << i) & nPreviousChannelMask )
				{
					mChannelAssignments[i].currOwner  = pOwner;
                    mChannelAssignments[i].dataTypeId = nDataTypeId;
				
                } // End if assigned

			} // Next channel

			// Decrement our available channel count
			mAvailableChannels -= nChannelCount;

			// Update the assignment time stamp
			mLastAssigned = cgTimer::getInstance()->getTime();

			// Set the owner as a filler of this resource
			setFiller( pOwner );

			// Return the assigned channel mask
			return nPreviousChannelMask; 

		} // End if channels available

	} // End if reuse channels

	// Determine which channels are available for assignment
	cgUInt32 nNumAssigned = 0, nChannelMask = 0;
	for ( cgUInt32 i = 0; i < mDescription.channelCount && nNumAssigned < nChannelCount; ++i )
	{	
		// If we found an available channel
		if ( !mChannelAssignments[i].currOwner )
		{
			// Assign the data
			mChannelAssignments[i].currOwner  = pOwner;
            mChannelAssignments[i].dataTypeId = nDataTypeId;

			// Update the channel mask we will return to the caller
			nChannelMask |= (1L << i);
		
			// Update number assigned
			++nNumAssigned;

		} // End if open slot

	} // Next channel

	// Decrement our available channel count
	mAvailableChannels -= nChannelCount;

	// Update the assignment time stamp
	mLastAssigned = cgTimer::getInstance()->getTime();

	// Set the owner as a filler of this resource
	setFiller( pOwner );

	// Return the final mask
	return nChannelMask;
}

//-----------------------------------------------------------------------------
//  Name : unassign ()
/// <summary>
/// Removes any references to the specified owner held by this resource and 
/// updates the number of available channels accordingly. 
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePoolResource::unassign( void * pOwner, bool bForceTimeReset /* = false */ )
{
	// Nobody 'owns' a shared resource
	if ( mDescription.type == cgTexturePoolResourceType::Shared )
		return;

	// Determine which channels are available for assignment
	for ( cgUInt32 i = 0; i < mDescription.channelCount; ++i )
	{
		// If we stored the data in this channel
		if ( mChannelAssignments[i].currOwner == pOwner )
		{
			// Clear the channel
			mChannelAssignments[i].currOwner  = CG_NULL;
			mChannelAssignments[i].dataTypeId = 0xFFFFFFFF;
			
			// This channel is available for use once more.
			++mAvailableChannels;

		} // End if found data

	} // Next channel

	/*
	// Do we want the last-assigned time stamp reset to 0?
	if ( bForceTimeReset )
	{
		// Can only do so if all channels are available
		if ( mAvailableChannels == mDescription.channelCount )
			mLastAssigned = 0;

	} // End forced reset
	*/
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initializes the pool resource.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePoolResource::initialize( cgResourceManager * pResources, const cgImageInfo & Desc, const cgString & strMapRefName )
{
    switch ( Desc.type )
    {
        case cgBufferType::RenderTarget:
        {
            // Create the render target (Note: we always want a unique target, do not match by name).
            cgRenderTargetHandle hTarget;
		    if ( !pResources->createRenderTarget( &hTarget, Desc, cgResourceFlags::ForceNew, strMapRefName, cgDebugSource() ) )
		    {
			    cgAppLog::write( cgAppLog::Error, _T("Failed to create pool resource render target '%s'.\n"), strMapRefName.c_str() );
			    return false;

		    } // End if failed
            mResource = hTarget;
            break;
        
        } // End case RenderTarget
        
        case cgBufferType::ShadowMap:
        case cgBufferType::DepthStencil:
        {
		    // Create the depth-stencil target (Note: we always want a unique target, do not match by name).
            cgDepthStencilTargetHandle hTarget;
		    if ( pResources->createDepthStencilTarget( &hTarget, Desc, cgResourceFlags::ForceNew, strMapRefName, cgDebugSource() ) == false )
		    {
			    cgAppLog::write( cgAppLog::Error, _T("Failed to create pool resource depth-stencil target '%s'.\n"), strMapRefName.c_str() );
			    return false;

		    } // End if failed
            mResource = hTarget;
            break;
        
        } // End case ShadowMap | DepthStencil
        default:
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempted to create texture pool resource '%s' using an unsupported type.\n"), strMapRefName.c_str() );
            return false;
    
    } // End switch type

	// Copy the data
	mDescription.bufferDesc   = Desc;
	mDescription.channelCount = cgBufferFormatEnum::formatChannelCount( Desc.format );
	
	// All channels are available
	mAvailableChannels = mDescription.channelCount;
	
	// Allocate slots for storing channel assignment data
    mChannelAssignments.resize( mAvailableChannels );
	
	// Success!
	return true;
}

//-----------------------------------------------------------------------------
//  Name : clearAssignments ()
/// <summary>
/// Clears out any prior assignments and resets the number of available channels
/// to the original value
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePoolResource::clearAssignments( )
{
	// Keep track of prior owner and free the slots
	for ( UINT i = 0; i < mChannelAssignments.size(); i++ )
	{
		mChannelAssignments[i].currOwner  = CG_NULL;
		mChannelAssignments[i].dataTypeId = 0xFFFFFFFF;
	}

	// Reset the available channel count
	mAvailableChannels = mDescription.channelCount;

    // Reset last assigned timestamp?
    //mLastAssigned = 0.0;
}

//-----------------------------------------------------------------------------
//  Name : isAssignedType ()
/// <summary>
/// Does this resource have assigned data of the indicated type?
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePoolResource::isAssignedType( cgUInt32 nDataTypeId ) const
{
	for ( size_t i = 0; i < mChannelAssignments.size(); ++i )
	{
		if ( mChannelAssignments[i].dataTypeId == nDataTypeId )
			return true;
	
    } // Next channel
	return false;
}

//-----------------------------------------------------------------------------
//  Name : getChannelMask ()
/// <summary>
/// Builds a mask based on which channels the specified owner occupies.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTexturePoolResource::getChannelMask( void * pOwner ) const
{
	// Determine which channels use the input pointer
	cgUInt32 nChannelMask = 0;
	for ( size_t i = 0; i < mChannelAssignments.size(); ++i )
	{
		// If we own the data in this channel, update the channel mask
		if ( mChannelAssignments[i].currOwner == pOwner || ( mChannelAssignments[i].currOwner == CG_NULL && mChannelAssignments[i].lastFiller == pOwner ) )
			nChannelMask |= (1L << i);

	} // Next channel

	// Return the final mask
	return nChannelMask;
}

//-----------------------------------------------------------------------------
//  Name : wasPreviousOwner ()
/// <summary>
/// Returns whether the input owner pointer was the prior owner/filler of this resource
/// (i.e., was it the last one to fill/use it?).
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePoolResource::wasPreviousOwner( void * pOwner, cgUInt32 nPreviousChannelMask /* = 0 */ ) const
{
	// Determine which channels use the input pointer
	cgUInt32 nChannelMask = 0;
	for ( size_t i = 0; i < mChannelAssignments.size(); ++i )
	{
		// If we found a match, update the channel mask
		if ( mChannelAssignments[i].lastFiller == pOwner )
			nChannelMask |= (1L << i);

	} // Next channel

	// If we want a like for like channel comparison...
	if ( nPreviousChannelMask != 0 )
	{
		return nPreviousChannelMask == nChannelMask;
	}
	else // We just want to know if the previous owner owned any channels...
	{
		return nChannelMask != 0;
	}
}

//-----------------------------------------------------------------------------
//  Name : setFiller ()
/// <summary>
/// Flags the owner as the filler of this resource
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePoolResource::setFiller( void * pOwner )
{
	// Do not track fillers of shared resources
	if ( mDescription.type == cgTexturePoolResourceType::Shared )
		return;

	// Determine which channels are available for assignment
	for ( cgUInt32 i = 0; i < mDescription.channelCount; ++i )
	{
		// If we stored the data in this channel
		if ( mChannelAssignments[i].currOwner == pOwner )
		{
			mChannelAssignments[i].lastFiller    = pOwner;
			mChannelAssignments[i].lastFillFrame = cgTimer::getInstance()->getFrameCounter();

		} // End if found data

	} // Next channel

}

//-----------------------------------------------------------------------------
//  Name : channelsAvailable ()
/// <summary>
/// Queries to see whether or not the requested channels are available
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePoolResource::channelsAvailable( cgUInt32 nDesiredChannelMask ) const
{
	// Now test the individual cases to see what is available
	for ( cgUInt32 i = 0; i < mDescription.channelCount; ++i )
	{
		// Exit immediately if a requested channel is already occupied
		if ( nDesiredChannelMask & (1L << i) )
		{
			if ( mChannelAssignments[i].currOwner )
				return false;

		} // End if match
		
	} // Next channel

	// Yes, the channels are available
	return true;
}

//-----------------------------------------------------------------------------
//  Name : canAccommodate ()
/// <summary>
/// Can the resource accommodate the requested format, resolution, and channel
/// count?
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePoolResource::canAccommodate( cgBufferType::Base BufferType, cgBufferFormat::Base Format, cgUInt32 nResolution, cgUInt32 nChannelCount /* = 0 */, cgUInt32 nDataTypeId /* = 0xFFFFFFFF */ ) const
{
	// Buffer types must match
	if ( BufferType != mDescription.bufferDesc.type )
		return false;

	// Resolutions must match
	if ( nResolution != mDescription.bufferDesc.width )
		return false;

	// Formats must match
	if ( Format != mDescription.bufferDesc.format )
		return false;

	// If # channels not specified, assume caller wants all channels 
	if ( nChannelCount == 0 )
		nChannelCount = mDescription.channelCount;
	
	// Must have at least required number of channels available 
	if ( nChannelCount > mAvailableChannels )
		return false;

	// If a data type identifier was provided, make sure we don't have data already
	// assigned that has a different identifier
	if ( nDataTypeId != 0xFFFFFFFF )
	{
        // ToDo: 6767 - Is this the behavior we actually want? Will bail
        // immediately if ANY channel contains other data? I'd guess yes,
        // otherwise this loop would seem to be completely unnecessary.

		// Exit immediately if a channel is already occupied by another "type"
		for ( cgUInt32 i = 0; i < mDescription.channelCount; i++ )
		{
			if ( mChannelAssignments[i].dataTypeId != nDataTypeId && 
                 mChannelAssignments[i].dataTypeId != 0xFFFFFFFF )
				return false;
			
		} // Next channel

	} // End if test identifier

	// Success
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgTexturePool Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgTexturePool () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePool::cgTexturePool( )
{
	mResources                = CG_NULL;               
    mPoolMemoryConsumption    = 0;
    mConfiguring              = false;
    mSharedDepthFormat        = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgTexturePool () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePool::~cgTexturePool()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgTexturePool::dispose( bool bDisposeBase )
{
    // Release allocated memory
    clear();

    // Reset configuration
    mSettings         = Config();
    mConfiguring     = false;
}

//-----------------------------------------------------------------------------
//  Name : beginConfigure ()
/// <summary>
/// Begin the pool configuration process. Subsequent calls can be made
/// to addDefaultMaps(), addCachedMaps() and endConfigure() to finish up
/// the configuration process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePool::beginConfigure( cgResourceManager * pResources, cgUInt32 nPoolMemLimit, cgUInt32 nMinResolution, cgUInt32 nMaxResolution, cgUInt32 nMaxSharedPerType )
{
    // If we are already configuring (or parameters are invalid), fail.
    if ( mConfiguring || !pResources )
        return false;

    // Clear out any old data to be safe.
    clear();
    mSettings = Config();

    // Store required values for later use
    mResources                     = pResources;
    mSettings.poolMemoryLimit      = nPoolMemLimit;
    mSettings.minimumResolution    = nMinResolution;
    mSettings.maximumResolution    = nMaxResolution;
    mSettings.maximumSharedPerType = nMaxSharedPerType;
	
    // We're beginning the configuration process.
    mConfiguring = true;
	
	// Allocate storage based on the number of mip resolutions possible given the
	// maximum resource resolution supported
	mCachedResources.resize( 1 + cgMathUtility::log2( nMaxResolution ) );
	mSharedResources.resize( 1 + cgMathUtility::log2( nMaxResolution ) );

	// Build a list of supported depth formats before adding new descriptions
	buildDepthFormatArray();
	
	// Add system level shared resources. 
	addSystemMaps();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addSystemMaps ()
/// <summary>
/// Adds "shared" resources to the pool. These are used to provide buffers to
/// accompany pooled resources where needed (e.g., a depth-stencil buffer to 
/// go along with a software depth texture during filling).
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::addSystemMaps( )
{
	// Get access to the buffer format enum
	const cgBufferFormatEnum & Enum = mResources->getBufferFormats();

	// For our shared color buffers, we choose a format that supports the 
	// highest precision single depth value
    cgBufferFormat::Base SharedRTFormat = getBestRenderTargetFormat( 0, 1 );

	// For our shared depth buffers, we prefer a 24-bit format that supports reading
	// If we cannot get a readable buffer, we'll fall back to a standard depth-stencil
	mSharedDepthFormat = getBestReadableDepthFormat( 24 );
	if ( !mSharedDepthFormat )
		mSharedDepthFormat = getBestDepthFormat( 0, false, false, false );
	
	// Create resources between min and max resolutions
    cgTexturePoolResourceDesc ResourceDesc;
	ResourceDesc.type = cgTexturePoolResourceType::Shared;
	cgUInt32 nMinMipLevel = cgMathUtility::log2( mSettings.minimumResolution );
	cgUInt32 nMaxMipLevel = cgMathUtility::log2( mSettings.maximumResolution );
	for ( cgUInt32 i = nMinMipLevel; i <= nMaxMipLevel; i++ )
	{
		ResourceDesc.bufferDesc.width  = (1L << i);
		ResourceDesc.bufferDesc.height = (1L << i);
		
        bool bGather = ((mSharedDepthFormat->capabilities & cgBufferFormatCaps::CanGather) != 0);
        bool bSample = ((mSharedDepthFormat->capabilities & cgBufferFormatCaps::CanSample) != 0);
		ResourceDesc.bufferDesc.type = (bGather || bSample) ? cgBufferType::ShadowMap : cgBufferType::DepthStencil;
		ResourceDesc.bufferDesc.format = mSharedDepthFormat->bufferDesc.format;
		mSettings.resources.push_back( ResourceDesc );

		ResourceDesc.bufferDesc.type = cgBufferType::RenderTarget;
        ResourceDesc.bufferDesc.format = SharedRTFormat;
		mSettings.resources.push_back( ResourceDesc );

	} // Next mip level
}

//-----------------------------------------------------------------------------
//  Name : getSharedDepthStencilFormat ()
/// <summary>
/// Retrieve the description of the format selected for the shared depth
/// stencil buffer.
/// </summary>
//-----------------------------------------------------------------------------
const cgTexturePoolDepthFormatDesc * cgTexturePool::getSharedDepthStencilFormat( ) const
{
    return mSharedDepthFormat;
}

//-----------------------------------------------------------------------------
//  Name : addDefaultMaps ()
/// <summary>
/// Adds resources to the shared pool for any cached resources in the description
/// list. These can act as fallbacks for cases when cached resources are not
/// available. 
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::addDefaultMaps( const cgTexturePoolResourceDesc::Array & aDescriptions )
{
    cgTexturePoolResourceDesc ResourceDesc;
    ResourceDesc.type = cgTexturePoolResourceType::Shared;

	// Create one shared resource for each supported mip level
    cgUInt32 nMinLevel = cgMathUtility::log2( getConfig().minimumResolution );
    cgUInt32 nMaxLevel = cgMathUtility::log2( getConfig().maximumResolution );
	for ( cgUInt32 i = nMinLevel; i <= nMaxLevel; i++ )
	{
		cgUInt32 nResolution = (1L << i);
		cgUInt32 nNumMips    = cgMathUtility::log2( nResolution ) + 1;
		for ( size_t j = 0; j < aDescriptions.size(); j++ )
		{
			if ( aDescriptions[ j ].type == cgTexturePoolResourceType::Cached )
			{
				ResourceDesc.bufferDesc        = aDescriptions[ j ].bufferDesc;
				ResourceDesc.bufferDesc.width  = nResolution;
				ResourceDesc.bufferDesc.height = nResolution;

                // ToDo: 6767 - Why the conditional?
                // Force allocation of required number of mip levels.
				if ( ResourceDesc.bufferDesc.mipLevels != 1 )
					ResourceDesc.bufferDesc.mipLevels = nNumMips;

                // Add to the resource list.
				mSettings.resources.push_back( ResourceDesc );

			} // End if pool type

		} // Next description 

	} // Next mip level
}

//-----------------------------------------------------------------------------
//  Name : addCachedMaps ()
/// <summary>
/// Add the specified number of resources for this method to the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::addCachedMaps( const cgTexturePoolResourceDesc::Array & aDescriptions, cgUInt32 nResolution, cgUInt32 nCount )
{
	// Is this a no-op?
    if ( !nCount )
        return;
	
	// Exit if cached maps are disabled
	if ( !mSettings.poolMemoryLimit )
	{
		cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Cannot add cached maps because pool is disabled (i.e., reserved memory = 0 bytes.)\n") );
		return;
	
    } // End if no memory pool

	// Exit if attempting to add maps greater than the supported maximum
	if ( nResolution > mSettings.maximumResolution )
	{	
		cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Ignoring attempt to add %i %ix%i cached maps that exceed the maximum size of %ix%i allowed for the pool.\n"), nCount, nResolution, nResolution, mSettings.maximumResolution, mSettings.maximumResolution );
		return;
	
    } // End if too big

    // Add the requested number of resources
    cgTexturePoolResourceDesc ResourceDesc;
    ResourceDesc.type = cgTexturePoolResourceType::Cached;
    cgUInt32 nNumMips = cgMathUtility::log2( nResolution ) + 1;
	for ( cgUInt32 i = 0; i < nCount; ++i )
	{
		for ( size_t j = 0; j < aDescriptions.size(); ++j )
		{
            // Cached map?
			if ( aDescriptions[ j ].type == cgTexturePoolResourceType::Cached )
			{
				ResourceDesc.bufferDesc        = aDescriptions[ j ].bufferDesc;
				ResourceDesc.bufferDesc.width  = nResolution;
				ResourceDesc.bufferDesc.height = nResolution;

                // ToDo: 6767 - Why the conditional?
                // Force allocation of required number of mip levels.
				if ( ResourceDesc.bufferDesc.mipLevels != 1 )
					ResourceDesc.bufferDesc.mipLevels = nNumMips;

                // Add to the resource list
				mSettings.resources.push_back( ResourceDesc );

			} // End if pool type

		} // Next description 

	} // Next instance

}

//-----------------------------------------------------------------------------
//  Name : AddMap()
/// <summary>
/// Add the specified number of resources of a given size and format to the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::addMaps( cgTexturePoolResourceType::Base ResourceType, cgBufferFormat::Base Format, cgUInt32 nResolution, cgUInt32 nCount )
{
    // Is this a no-op?
    if ( !nCount )
        return;

    // Exit if cached maps are disabled
    if ( ResourceType == cgTexturePoolResourceType::Cached && !mSettings.poolMemoryLimit )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Cannot add cached maps because pool is disabled (i.e., reserved memory = 0 bytes.)\n") );
        return;

    } // End if no memory pool

    // Exit if attempting to add maps greater than the supported maximum
    if ( nResolution > mSettings.maximumResolution )
    {	
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Ignoring attempt to add %i %ix%i cached maps that exceed the maximum size of %ix%i allowed for the pool.\n"), nCount, nResolution, nResolution, mSettings.maximumResolution, mSettings.maximumResolution );
        return;

    } // End if too big

    // Add the specified number of resources
    cgTexturePoolResourceDesc ResourceDesc;
	ResourceDesc.type              = ResourceType;
	ResourceDesc.bufferDesc.type   = cgBufferType::RenderTarget;
	ResourceDesc.bufferDesc.format = Format;
	ResourceDesc.bufferDesc.width  = nResolution;
	ResourceDesc.bufferDesc.height = nResolution;
	for ( cgUInt32 i = 0; i < nCount; ++i )
		mSettings.resources.push_back( ResourceDesc );

	// Ensure we track the format so that we can create scratch resources
	mSettings.resampleFormats.insert( Format );
}

//-----------------------------------------------------------------------------
//  Name : createResampleChain () (Private)
/// <summary>
/// Creates a resampling chain for use with a given format.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChain * cgTexturePool::createResampleChain( cgBufferFormat::Base Format )
{
	cgTexturePoolResourceDesc Desc;
	Desc.type                   = cgTexturePoolResourceType::Shared;
	Desc.bufferDesc.type        = cgBufferType::RenderTarget;
	Desc.bufferDesc.format      = Format;
	Desc.bufferDesc.width       = mSettings.maximumResolution;
	Desc.bufferDesc.height      = mSettings.maximumResolution;
    Desc.bufferDesc.mipLevels   = 1;

    // Attempt to create the top-level resource
	cgTexturePoolResource * pResource = createResource( Desc ); 
	if ( !pResource )
        return CG_NULL;
	
	// Build a mip chain.
    cgResampleChain * pChain = new cgResampleChain();
    cgRenderTargetHandle hTarget( (cgRenderTarget*)pResource->getResource().getResourceSilent() );
	pChain->setSource( hTarget );

    // ToDo: 6767 - Just overwrites existing chain without deleting one that might exists there first?
    // This may be fine since this is a protected method and we know this can never happen.

	// Add the chain to our pool.
	mResampleChains[ Format ] = pChain;

	// Return success
	return pChain;
}

//-----------------------------------------------------------------------------
//  Name : endConfigure ()
/// <summary>
/// Initialize the pool and pre-populate based on the information provided during configuration.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePool::endConfigure( )
{
    // Valid?
    if ( !mConfiguring )
        return false;

    // ToDo: 6767 - HANDLE RESOURCE CREATION FAILURE!

	// Attempt to build all requested resources. 
	for ( cgUInt32 i = 0; i < mSettings.resources.size(); i++ )
		createResource( mSettings.resources[ i ] );

	// Setup resampling chains for scratch work.
	FormatTable::iterator itFormat;
	for ( itFormat = mSettings.resampleFormats.begin(); 
		  itFormat != mSettings.resampleFormats.end();
		  ++itFormat )
		createResampleChain( *itFormat );

    // Success!
    mConfiguring = false;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMemoryConsumption ()
/// <summary>
/// Retrieve the total estimated memory consumption of all of the
/// resources that currently exist in the pool.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgTexturePool::getMemoryConsumption( ) const
{
    return mPoolMemoryConsumption;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieves the configuration settings for this pool.
/// </summary>
//-----------------------------------------------------------------------------
const cgTexturePool::Config & cgTexturePool::getConfig( ) const
{
    return mSettings;
}

//-----------------------------------------------------------------------------
//  Name : getResampleChain ()
/// <summary>
/// Retrieve the resampling chain used for scratch processing.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChain * cgTexturePool::getResampleChain( cgBufferFormat::Base Format )
{
	ResampleChainTable::iterator itChain = mResampleChains.find( Format );
	if ( itChain != mResampleChains.end() )
		return itChain->second;	

	// If we could not find a chain, build one, but print a warning to the log
	// so that next time around it can be added to the pool properly.
	cgResampleChain * pChain = createResampleChain( Format );
	if ( pChain )
		cgAppLog::write( cgAppLog::Warning, _T("Dynamic memory allocation triggered for a resampling chain of format '%s'. Chains should be added to the pool during initialization.\n"), cgBufferFormatEnum::formatToString( Format ).c_str() );

    // Return chain.
	return pChain;
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Clear out all resources currently contained in the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::clear()
{
	// Clear the cached resource array
	for ( size_t i = 0; i < mCachedResources.size(); ++i )
	{
		// For each format in the table
        PoolResourceTable::iterator itTable;
		for( itTable = mCachedResources[ i ].begin(); itTable != mCachedResources[ i ].end(); ++itTable )
		{
			// For each resource
			const cgTexturePoolResourceArray & aResources = itTable->second;
			for ( size_t j = 0; j < aResources.size(); ++j )
                delete aResources[ j ];

		} // Next format

	} // Next mip level	
    mCachedResources.clear();

	// Clear the shared resource array
	for ( size_t i = 0; i < mSharedResources.size(); ++i )
	{
		// For each format in the table
        PoolResourceTable::iterator itTable;
		for( itTable = mSharedResources[ i ].begin(); itTable != mSharedResources[ i ].end(); ++itTable )
		{
			// For each resource
			const cgTexturePoolResourceArray & aResources = itTable->second;
			for ( size_t j = 0; j < aResources.size(); ++j )
			    delete aResources[ j ];

		} // Next format

	} // Next mip level	
    mSharedResources.clear();

    // Clear the scratch chain table
	ResampleChainTable::iterator itChain;
	for ( itChain  = mResampleChains.begin(); itChain != mResampleChains.end(); ++itChain )
	{
		cgResampleChain * pChain = itChain->second;
		pChain->setSource( cgRenderTargetHandle::Null );
		delete pChain;

	} // Next chain
	mResampleChains.clear();

	// Clear our depth formats
	for ( size_t i = 0; i < 3; i++ )
		mDepthFormats[ i ].clear();

	// Clear Variables
    mPoolMemoryConsumption = 0;
    mSharedDepthFormat = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : resetAvailability ()
/// <summary>
/// Resets resources in the pool back to an available status for possible
/// assignment or reassignment.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::resetAvailability( )
{
    resetAvailability( 0xFFFFFFFF );
}

//-----------------------------------------------------------------------------
//  Name : resetAvailability ()
/// <summary>
/// Resets resources in the pool back to an available status for possible
/// assignment or reassignment. This overload allows the caller to filter 
/// based on the data type identifier specified during assignment.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::resetAvailability( cgUInt32 nDataTypeId )
{
    size_t i, j;
    PoolResourceTable::iterator itTable;

	// For each mip level
	for ( i = 0; i < mCachedResources.size(); ++i )
	{
		// For each format in the table
		for( itTable = mCachedResources[ i ].begin(); itTable != mCachedResources[ i ].end(); ++itTable )
		{
			// For each pool resource
			const cgTexturePoolResourceArray & aResources = itTable->second;
			for ( j = 0; j < aResources.size(); ++j )
			{
                // ToDo: 6767 - anyway to optimize isAssignedType?
				if ( (nDataTypeId == 0xFFFFFFFF) || (aResources[ j ]->isAssignedType( nDataTypeId )) )
                {
                    // ToDo: 6767 - Clears ALL channels even if some of them were assigned other types?
                    // This jives with the restrictions in the resource canAccommodate method, but clarify why?
					aResources[ j ]->clearAssignments();
                }

			} // Next resource

		} // Next format

	} // Next mip level	

}

//-----------------------------------------------------------------------------
//  Name : getResource ()
/// <summary>
/// Attempts to find a resource matching the input criteria. Can optionally create
/// a new resource or return a default resource.
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePoolResource * cgTexturePool::getResource( const cgTexturePoolResourceDesc & Desc, const ResourceSet & ExclusionList, cgUInt32 nChannelCount /* = 0 */, bool bAutoCreate /* = true */, bool bAutoReturnDefault /* = true */ )
{
	// Get the buffer format enumerator
    const cgBufferFormatEnum & Enum = mResources->getBufferFormats();

	// Compute the mip level for this resolution to serve as an index
    cgUInt32 nIndex = cgMathUtility::log2( Desc.bufferDesc.width );
	cgAssertEx( nIndex < mCachedResources.size(), "Resolution not available in cached texture pool." );

	// If the caller did not indicate a specific number of channels, use the whole resource.
	if ( !nChannelCount )
		nChannelCount = Enum.formatChannelCount( Desc.bufferDesc.format );

	// Was a cached resource requested?
	if ( Desc.type == cgTexturePoolResourceType::Cached )
	{
		// Find a match on format and mip level
		PoolResourceTable::iterator itTable = mCachedResources[ nIndex ].find( Desc.bufferDesc.format );
		if ( itTable != mCachedResources[ nIndex ].end() )
		{
			// Check for an available resource that has preferably not been used recently.
            cgDouble fBestTime = FLT_MAX;
			cgTexturePoolResourceArray & aResources = itTable->second;
			cgTexturePoolResource * pBestResource = CG_NULL;
			for ( size_t i = 0; i < aResources.size(); i++ )
			{
                // Are there enough available channels in this resource?
				cgTexturePoolResource * pResource = aResources[ i ];
				if ( pResource->getChannelCount( true ) >= nChannelCount )
				{
					// Get the last time this resource was updated (i.e., assigned to something)
					cgDouble fLastAssigned = pResource->getTimeLastAssigned();
					
					// If this is an unused resource, return it immediately
					if ( fLastAssigned == 0.0f )
						return pResource;
					
					// Otherwise, find the oldest available (i.e., the one with the smallest value)
					if ( fLastAssigned < fBestTime )
					{
						pBestResource = pResource;
						fBestTime     = fLastAssigned;
					
                    } // End if older
				
                } // End if has enough room

			} // Next resource

			// Return the resource if we found one
			if ( pBestResource != CG_NULL )
				return pBestResource;

		} // End if found

		// We could not find a match, so try adding a new resource to the pool 
		if ( bAutoCreate )
		{
			// Create a new resource if we can
			cgTexturePoolResource * pResource = createResource( Desc, true );
			if ( pResource )
			{
				// Throw a warning that a resource was dynamically allocated.
				cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Initiated a dynamic texture pool resource allocation after scene had been initialized: %ix%i (%s). Consider growing your available pool.\n" ), Desc.bufferDesc.width, Desc.bufferDesc.height, cgBufferFormatEnum::formatToString( Desc.bufferDesc.format ).c_str() );

				// Return the new resource
				return pResource;
			
            } // End if succeeded

		} // End if not add

		// If we still have no resource, and we are not allowed to use a shared resource,
		// there is nothing more we can do...
		if ( !bAutoReturnDefault )
			return CG_NULL;

	} // End if pool type requested 

	// If we get here, the caller either wants a shared resource, or could not get a cached
	// resource and wants to fall back to a shared resource. Either way, attempt to get one.
	cgAssertEx( nIndex < mSharedResources.size(), "Resolution not available in shared texture pool." );
	PoolResourceTable::iterator itTable = mSharedResources[ nIndex ].find( Desc.bufferDesc.format );
	if ( itTable != mSharedResources[ nIndex ].end() )
	{
		// Return the first resource found that is not in the exclusion list
		cgTexturePoolResourceArray & aResources = itTable->second;
		for ( size_t i = 0; i < aResources.size(); ++i )
		{
			if ( ExclusionList.find( aResources[ i ] ) == ExclusionList.end() )
				return aResources[ i ];
		
        } // Next resource
	
    } // End if found shared

	// A shared resource could not be found, so fail.
	cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A default %ix%i (%s) resource was not available in the pool and resource selection has failed.\n"), Desc.bufferDesc.width, Desc.bufferDesc.height, cgBufferFormatEnum::formatToString( Desc.bufferDesc.format ).c_str() );
	return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createResource( )
/// <summary>
/// Creates a resource of a given size and format
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePoolResource * cgTexturePool::createResource( const cgTexturePoolResourceDesc & Desc, bool bSilent /* = false */ )
{
	// Get the buffer format enumerator
    const cgBufferFormatEnum & Enum = mResources->getBufferFormats();

    // Estimate memory consumption
    size_t nMemoryEstimate = Enum.estimateBufferSize( Desc.bufferDesc );
    if ( !nMemoryEstimate )
    {
        if ( !bSilent )
            cgAppLog::write( cgAppLog::Error, _T("Pool resource %ix%i (%s) is unsupported on the current hardware. This is a critical failure.\n"), Desc.bufferDesc.width, Desc.bufferDesc.height, Enum.formatToString( Desc.bufferDesc.format ).c_str() );
        return CG_NULL;
    
    } // End if unsupported

	// If this is intended to be a cached resource, see if there is enough memory remaining to allocate it
	if ( (Desc.type == cgTexturePoolResourceType::Cached) && (mPoolMemoryConsumption + nMemoryEstimate) > (mSettings.poolMemoryLimit * 1048576) )
    {
        if ( !bSilent )
	        cgAppLog::write( cgAppLog::Warning, _T("Skipped allocation of %ix%i (%s) resource because it would cause the pool to exceed the specified limit of %i megabytes.\n"), Desc.bufferDesc.width, Desc.bufferDesc.height, Enum.formatToString( Desc.bufferDesc.format ).c_str(), mSettings.poolMemoryLimit );
        return CG_NULL;
    
    } // End if out of memory

    // Compute the mip level for this resolution to serve as an index
    cgUInt32 nIndex = cgMathUtility::log2( Desc.bufferDesc.width );

    // Get the array of entries for this type/size (will auto-create if one doesn't exist).
    PoolResources & Pool = (Desc.type == cgTexturePoolResourceType::Cached) ? mCachedResources : mSharedResources;
    cgTexturePoolResourceArray & aResources = Pool[ nIndex ][ Desc.bufferDesc.format ];
    
    // If this is a shared buffer, we should stop as soon as the shared buffer
    // for a given type is full.
    if ( Desc.type == cgTexturePoolResourceType::Shared )
    {
        // Shared buffer already full?
        if ( aResources.size() >= mSettings.maximumSharedPerType )
            return CG_NULL;
    
    } // End if shared

	// Generate a name for this resource.
    cgString strResourceName;
    if ( Desc.type == cgTexturePoolResourceType::Shared )
		strResourceName = cgString::format( _T("Pool(Shared)_%ix%i_%s"), Desc.bufferDesc.width, Desc.bufferDesc.height, Enum.formatToString( Desc.bufferDesc.format ).c_str() );
	else
		strResourceName = cgString::format( _T("Pool(Cached)_%ix%i_%s"), Desc.bufferDesc.width, Desc.bufferDesc.height, Enum.formatToString( Desc.bufferDesc.format ).c_str() );

	// Create a resource of this type
    cgTexturePoolResource * pResource = new cgTexturePoolResource( Desc.type );
    if ( !pResource->initialize( mResources, Desc.bufferDesc, strResourceName ) )
	{
        if ( !bSilent )
            cgAppLog::write( cgAppLog::Error, _T("Unable to initialize pool resource %s.\n"), strResourceName.c_str() );
		delete pResource; 
		return CG_NULL;
	
    } // End if failed

    // Add the resource to the selected pool
    aResources.push_back( pResource );

    // Update the memory consumed
    if ( Desc.type == cgTexturePoolResourceType::Cached )
		mPoolMemoryConsumption += nMemoryEstimate;

	// Return the resource
	return pResource;
}

//-----------------------------------------------------------------------------
//  Name : getBestRenderTargetFormat ()
/// <summary>
/// Finds the best supported render target format for a given precision and 
/// number of channels. Prefers signed integer formats over floats given the 
/// extra bit of precision available.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgTexturePool::getBestRenderTargetFormat( cgUInt32 nPrecision, cgUInt32 nNumChannels ) const
{
	// Get the buffer format enumerator
    const cgBufferFormatEnum & Enum = mResources->getBufferFormats();

	// One channel
	if ( nNumChannels == 1 )
	{
		switch( nPrecision )
		{
			case 0:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32_UInt ) )
					return cgBufferFormat::R32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32_Float ) )
					return cgBufferFormat::R32_Float;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16_UInt ) )
					return cgBufferFormat::R16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16 ) )
					return cgBufferFormat::R16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16_Float ) )
					return cgBufferFormat::R16_Float;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::B8G8R8A8 ) )
					return cgBufferFormat::B8G8R8A8;
			    break;
			case 8:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::B8G8R8A8 ) )
					return cgBufferFormat::B8G8R8A8;
			    break;
			case 16:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16_UInt ) )
					return cgBufferFormat::R16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16 ) )
					return cgBufferFormat::R16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16_Float ) )
					return cgBufferFormat::R16_Float;
			    break;
			case 24:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R24_UNorm_X8_Typeless ) )
					return cgBufferFormat::R24_UNorm_X8_Typeless;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R24G8_Typeless ) )
					return cgBufferFormat::R24G8_Typeless;
			    break;
			case 32:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32_UInt ) )
					return cgBufferFormat::R32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32_Float ) )
					return cgBufferFormat::R32_Float;
			    break;
		
        } // End switch precision

	} // End if 1 channel
	else if ( nNumChannels == 2 )
	{
		switch( nPrecision )
		{
			case 0:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32_UInt ) )
					return cgBufferFormat::R32G32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32_Float ) )
					return cgBufferFormat::R32G32_Float;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16_UInt ) )
					return cgBufferFormat::R16G16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16 ) )
					return cgBufferFormat::R16G16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16_Float ) )
					return cgBufferFormat::R16G16_Float;
			    break;
			case 16:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16_UInt ) )
					return cgBufferFormat::R16G16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16 ) )
					return cgBufferFormat::R16G16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16_Float ) )
					return cgBufferFormat::R16G16_Float;
			    break;
			case 32:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32_UInt ) )
					return cgBufferFormat::R32G32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32_Float ) )
					return cgBufferFormat::R32G32_Float;
			    break;
		
        } // End switch precision
		
	} // End if two channels
	else if ( nNumChannels == 4 )
	{
		switch( nPrecision )
		{
			case 0:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32B32A32_UInt ) )
					return cgBufferFormat::R32G32B32A32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32B32A32_Float ) )
					return cgBufferFormat::R32G32B32A32_Float;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16_UInt ) )
					return cgBufferFormat::R16G16B16A16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16 ) )
					return cgBufferFormat::R16G16B16A16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16_Float ) )
					return cgBufferFormat::R16G16B16A16_Float;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::B8G8R8A8 ) )
					return cgBufferFormat::B8G8R8A8;
			    break;
			case 8:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::B8G8R8A8 ) )
					return cgBufferFormat::B8G8R8A8;
			    break;
			case 16:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16_UInt ) )
					return cgBufferFormat::R16G16B16A16_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16 ) )
					return cgBufferFormat::R16G16B16A16;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R16G16B16A16_Float ) )
					return cgBufferFormat::R16G16B16A16_Float;
			    break;
			case 32:
				if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32B32A32_UInt ) )
					return cgBufferFormat::R32G32B32A32_UInt;
				else if ( Enum.isFormatSupported( cgBufferType::RenderTarget, cgBufferFormat::R32G32B32A32_Float ) )
					return cgBufferFormat::R32G32B32A32_Float;
			    break;
		
        } // End switch precision
	
    } // End if four channels

	// Return no match found
	return cgBufferFormat::Unknown;
}

//-----------------------------------------------------------------------------
//  Name : getBestDepthFormat ()
/// <summary>
/// Finds the best supported depth format for a given precision and options.
/// </summary>
//-----------------------------------------------------------------------------
const cgTexturePoolDepthFormatDesc * cgTexturePool::getBestDepthFormat( cgUInt32 nPrecision, bool bSample, bool bGather, bool bCompare, cgUInt8 nStencilBits /* = 0 */ ) const
{
	// If precision is not given, prefer 24 bits
	if ( nPrecision == 0 )
        nPrecision = 24;

	// Which precision slot will we be searching?
	cgUInt32 nBitDepthIndex = 0; // 16 bits
	if ( nPrecision == 24 )
		nBitDepthIndex = 1;      // 24 bits
	else if ( nPrecision == 32 )
		nBitDepthIndex = 2;      // 32 bits

	// Look for a format that matches the criteria indicated
	for ( size_t i = 0; i < mDepthFormats[ nBitDepthIndex ].size(); ++i )
	{
		const cgTexturePoolDepthFormatDesc & Format = mDepthFormats[ nBitDepthIndex ][ i ];

        // Matches requirements?
        if ( bSample && !(Format.capabilities & cgBufferFormatCaps::CanSample) )
			continue;
		if ( bGather && !(Format.capabilities & cgBufferFormatCaps::CanGather) )
			continue;
		if ( bCompare && !(Format.capabilities & cgBufferFormatCaps::CanCompare) )
			continue;
		if ( nStencilBits > Format.stencilBits )
			continue;

        // This is a suitable format.
		return &mDepthFormats[ nBitDepthIndex ][ i ];
	
    } // Next format

	// No match.
	return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getBestReadableDepthFormat()
/// <summary>
/// Get the best supported sampleable depth-stencil format for a given precision.
/// </summary>
//-----------------------------------------------------------------------------
const cgTexturePoolDepthFormatDesc * cgTexturePool::getBestReadableDepthFormat( cgUInt32 nPrecision, cgUInt8 nStencilBits ) const
{
    const cgTexturePoolDepthFormatDesc * pFormat;

	// First attempt a gather test
	if ( pFormat = getBestDepthFormat( nPrecision, false, true, false, nStencilBits ) )
	    return pFormat;
	
	// Finally try a read test
	if ( pFormat = getBestDepthFormat( nPrecision, true, false, false, nStencilBits ) )
	    return pFormat;

	// Nothing suitable found.
	return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : buildDepthFormatArray ()
/// <summary>
/// Compiles a list of supported depth formats and their potential sampling states.
/// </summary>
//-----------------------------------------------------------------------------
void cgTexturePool::buildDepthFormatArray( )
{
    // DirectX9 has some caveats we need to handle
	bool bIsDirectX9 = cgGetEngineConfig().renderAPI == cgRenderAPI::DirectX9;

	// Get the buffer format enumerator
	const cgBufferFormatEnum & Enum = mResources->getBufferFormats();
	
	// Create a generic clamped sampler state description
    cgSamplerStateDesc smpStates;
    smpStates.addressU  = cgAddressingMode::Clamp;
    smpStates.addressV  = cgAddressingMode::Clamp;
    smpStates.addressW  = cgAddressingMode::Clamp;

	// Create a list of depth formats for testing
    cgTexturePoolDepthFormatDesc DepthFormats[] =
    {
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::DF16, 16, 0),
        cgTexturePoolDepthFormatDesc(cgBufferFormat::D16, 16, 0),
        cgTexturePoolDepthFormatDesc(cgBufferFormat::DF24, 24, 0),
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::D24_Float_S8_UInt, 24, 8),
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::D24_UNorm_S8_UInt, 24, 8),
        cgTexturePoolDepthFormatDesc(cgBufferFormat::D24_UNorm_X8_Typeless, 24, 0),
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::INTZ, 24, 0),
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::RAWZ, 24, 0),
	    cgTexturePoolDepthFormatDesc(cgBufferFormat::D32_Float, 32, 0)
    };
    cgUInt32 nNumFormats = sizeof(DepthFormats) / sizeof(DepthFormats[0]);

	// Build a table of supported depth formats and possible read states
	for ( cgUInt32 i = 0; i < nNumFormats; ++i )
	{
		// Get the format
		cgTexturePoolDepthFormatDesc & Desc = DepthFormats[ i ];

		// Get the exposed capabilities for this format.
        if (!(Desc.capabilities = Enum.getFormatCaps( cgBufferType::DepthStencil, Desc.bufferDesc.format )))
            continue;
		
		// Build sampling states if supported
        if ( Desc.capabilities & cgBufferFormatCaps::CanCompare )
		{
			// For comparisons, we'll always use linear filtering
			smpStates.minificationFilter = cgFilterMethod::Linear;
			smpStates.magnificationFilter = cgFilterMethod::Linear;
			smpStates.mipmapFilter = cgFilterMethod::None;
			smpStates.comparisonFunction = cgComparisonFunction::Less;
			Desc.samplerStates = smpStates;
		
        } // End if compare
        else if ( (Desc.capabilities & cgBufferFormatCaps::CanGather) || (Desc.capabilities & cgBufferFormatCaps::CanSample) )
		{
			// For sampling, we'll always use point filtering
			smpStates.minificationFilter = cgFilterMethod::Point;
			smpStates.magnificationFilter = cgFilterMethod::Point;
			smpStates.mipmapFilter = cgFilterMethod::None;

			// Gathering in DirectX9 requires some unfortunate caveats (ATI hw only)
			if ( bIsDirectX9 && (Desc.capabilities & cgBufferFormatCaps::CanGather) )
				smpStates.mipmapLODBias = (float&)g_4CC_Fetch4Enable;

			Desc.samplerStates = smpStates;
		
        } // End if gather | sample

		// Add to array
		if ( Desc.depthBits == 16 )
			mDepthFormats[ 0 ].push_back( Desc );
		else if ( Desc.depthBits == 24 )
			mDepthFormats[ 1 ].push_back( Desc );
		else if ( Desc.depthBits == 32 )
			mDepthFormats[ 2 ].push_back( Desc );

	} // Next format

}

//-----------------------------------------------------------------------------
//  Name : assignResources ()
/// <summary>
/// Attempts to find and assign resources in the pool given the input descriptions.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePool::assignResources( const cgTexturePoolResourceDesc::Array & aDescriptions, void * pOwner, cgUInt32 nDataTypeId, cgTexturePoolResourceArray & aResources, cgUInt32 & nDefaultCount )
{
	ResourceSet ExclusionList;

	// Track the number of cached resource requests that must fall back to shared
	nDefaultCount = 0;
	
	// Release current resources, if any, back to the pool
	for ( size_t i = 0; i < aResources.size(); i++ )
	{
		if ( aResources[ i ] )
			aResources[ i ]->unassign( pOwner, true );
	
    } // Next resource
	aResources.clear();
	
	// Allocate space for our new resources
	aResources.resize( aDescriptions.size() );
	
	// Attempt to gather required resources
	for ( size_t i = 0; i < aDescriptions.size(); i++ )
	{
		// Attempt to get a resource
		aResources[ i ] = getResource( aDescriptions[ i ], ExclusionList, aDescriptions[ i ].channelCount );  
		
		// If we could not get any resource at all, something is wrong and we can't proceed
		if ( !aResources[ i ] )
		{
			cgAppLog::write( cgAppLog::Error, _T("Failed to find a valid resource during resource slot '%i' assignment.\n"), i );
			return false;
		
        } // End if failed

		// If we got back a shared resource
		if ( aResources[ i ]->getType() == cgTexturePoolResourceType::Shared )
		{
			// Add it to the exclusion list so that we don't get it again 
			ExclusionList.insert( aResources[ i ] );

			// If we didn't want a shared resource, but have to use one, update count
			if ( aDescriptions[ i ].type == cgTexturePoolResourceType::Cached )
				++nDefaultCount;
		
        } // End if shared
		else
		{
			// We requested and received a cached resource, so do an assignment
			aResources[ i ]->assign( pOwner, nDataTypeId, aDescriptions[ i ].channelCount );
		
        } // End if !shared
		
	} // Next description

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : reassignResources ()
/// <summary>
/// Attempts to reassign resources the same textures as used previously
/// </summary>
//-----------------------------------------------------------------------------
bool cgTexturePool::reassignResources( const cgTexturePoolResourceDesc::Array & aDescriptions, void * pOwner, cgUInt32 nDataTypeId, cgTexturePoolResourceArray & aResources )
{	
	ResourceSet ExclusionList;
	cgUInt32 numMatches = 0;

	// If either buffer is empty, fail
	if ( aDescriptions.empty() || aResources.empty() )
		return false;

	// If the number of resources currently assigned differs from the
	// number of resources needed, no reassignment is possible and the
	// resource array needs to be cleared
	if ( aDescriptions.size() != aResources.size() )
	{
		for ( size_t i = 0; i < aResources.size(); ++i )
			aResources[ i ]->unassign( pOwner, true );

		aResources.clear();
		return false;
	}

	// Keep track of previous channel assignments
	cgArray< cgUInt32 > previousChannels;

	// Do an initial pass to ensure everything will work out. Also, unassign
	// the resource during this process. We'll get it back if 
	for ( size_t i = 0; i < aResources.size(); ++i )
	{
		// Get a reference to the resource description
		const cgTexturePoolResourceDesc & Desc = aDescriptions[ i ];

		// Ideally we'll want the same channels as before
		cgUInt32 channelCount = Desc.channelCount;

		// Collect the channels that were previously assigned to this owner (if any)
		cgUInt32 prevChannels = channelCount ? aResources[ i ]->getChannelMask( pOwner ) : 0;

		// Add to prev channel list
		previousChannels.push_back( prevChannels );

		// If a cached resource is needed... (Note: we assume shared resources can always be found)
		if ( Desc.type == cgTexturePoolResourceType::Cached )
		{
			// If the pool types don't match, fail
			if ( aResources[ i ]->getType() != Desc.type )
				continue;
			
			// If the resource was lost, fail
			if ( aResources[i]->isResourceLost() )
				continue;

			// If this owner was not the prior filler of this resource, fail
			if ( !aResources[i]->wasPreviousOwner( pOwner, prevChannels ) )
				continue;

			// If our prior resource cannot accommodate us, fail
			if( !aResources[ i ]->canAccommodate( Desc.bufferDesc.type, Desc.bufferDesc.format, Desc.bufferDesc.width, channelCount, nDataTypeId ) )
				continue;

			// If we have specific channels we want to reuse and they are not available, fail
			if ( prevChannels != 0 && !aResources[ i ]->channelsAvailable( prevChannels ) )
				continue;

		} // End !shared

		// Unassign
		aResources[ i ]->unassign( pOwner );

		// Increment match count
		numMatches++;

	} // Next resource

	// If we didn't get a match on everything, clear and exit
	if ( numMatches != aResources.size() )
	{
		aResources.clear();
		return false;
	}

	// Everything matched, so let's do the reassignment
	for ( size_t i = 0; i < aResources.size(); ++i )
	{
		// Get a reference to the resource description
		const cgTexturePoolResourceDesc & Desc = aDescriptions[ i ];
	
		// If a shared resource is needed...
		if ( Desc.type == cgTexturePoolResourceType::Shared )
		{
			// Get the resource
			aResources[ i ] = getResource( Desc, ExclusionList );

            // If we could not get a resource at all, something is very wrong and we can't proceed
            cgAssert( aResources[ i ] != CG_NULL );

			// Add this resource to the exclusion list to prevent getting it again
			ExclusionList.insert( aResources[ i ] );

		} // End if shared
		else
		{
			// Reassign 
			aResources[ i ]->assign( pOwner, nDataTypeId, Desc.channelCount, previousChannels[ i ] );

		} // End !shared

	} // Next resource

	// Return status
	return true;
}