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
// File : cgTexturePool.h                                                    //
//                                                                           //
// Desc : Contains classes responsible for managing the various different    //
//        types of pooled textures.                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//
#pragma once
#if !defined( _CGE_CGTEXTUREPOOL_H_ )
#define _CGE_CGTEXTUREPOOL_H_

//-----------------------------------------------------------------------------
// cgTexturePool Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceHandles.h>
#include <Rendering/cgRenderingTypes.h>
#include <Resources/cgResourceTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgSampler;
class  cgRenderDriver;
class  cgResampleChain;
class  cgResampleChainMRT;
class  cgTexturePoolResource;
class  cgTexturePool;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTexturePoolResource (Class)
/// <summary>
/// A simple utility class responsible for managing texture pool resources.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTexturePoolResource
{
public:

	//-------------------------------------------------------------------------
	// Private Structures & Typedefs
	//-------------------------------------------------------------------------
	struct ChannelAssignment
	{
		void      * currOwner;
		cgUInt32    dataTypeId;
		void      * lastFiller;
		int         lastFillFrame;

		// Constructor
		ChannelAssignment() : 
		currOwner(CG_NULL), dataTypeId(0xFFFFFFFF),lastFillFrame(-1),lastFiller(CG_NULL){}
	};
	CGE_VECTOR_DECLARE( ChannelAssignment, ChannelAssignmentArray )

	//-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	 cgTexturePoolResource( cgTexturePoolResourceType::Base resourceType );
	~cgTexturePoolResource( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
	bool						initialize				( cgResourceManager * resourceManager, const cgImageInfo & description, const cgString & referenceName );
	cgUInt32					assign			        ( void * owner, cgUInt32 dataTypeId, cgUInt32 channelCount = 0, cgUInt32 previousChannelMask = 0 );
	void						unassign		        ( void * owner, bool forceTimeReset = false );
	void						clearAssignments        ( );
    bool						canAccommodate			( cgBufferType::Base type, cgBufferFormat::Base format, cgUInt32 resolution, cgUInt32 channelCount = 0, cgUInt32 dataTypeId = 0xFFFFFFFF ) const;
    bool						channelsAvailable		( cgUInt32 desiredChannelMask ) const;
    cgUInt32					getChannelMask		    ( void * owner ) const;
	bool						wasPreviousOwner        ( void * owner, cgUInt32 previousChannelMask = 0 ) const;
	bool						isAssignedType          ( cgUInt32 dataTypeId ) const;
	void                        setFiller               ( void * pFiller );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
	inline const cgTexturePoolResourceDesc   & getDescription   ( ) const { return mDescription; }
	inline cgTexturePoolResourceType::Base     getType          ( ) const { return mDescription.type; }
	inline cgBufferType::Base   getBufferType           ( ) const { return mDescription.bufferDesc.type; }
	inline cgBufferFormat::Base getFormat               ( ) const { return mDescription.bufferDesc.format; }
	inline cgUInt32             getResolution           ( ) const { return mDescription.bufferDesc.width; }
    inline cgUInt32             getChannelCount         ( bool bAvailable = false ) const { return (bAvailable) ? mAvailableChannels : mDescription.channelCount; }
    inline cgTextureHandle      getResource             ( ) const { return mResource; }
    inline bool					isResourceLost			( ) const { return mResource.isResourceLost(); }
	inline cgDouble             getTimeLastAssigned     ( ) const { return mLastAssigned; }
	ChannelAssignmentArray  &   getChannelAssignments   ( ){ return mChannelAssignments; }

private:

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
	cgTexturePoolResourceDesc   mDescription;            // A description of this resource
    cgTextureHandle             mResource;               // The handle to the underlying resource.
	cgUInt32                    mAvailableChannels;      // Number of channels available for assignment
	cgDouble                    mLastAssigned;           // The last time at which this resource was assigned
    ChannelAssignmentArray      mChannelAssignments;     // Describes the owner and type of the data assigned to a given resource channel.
};

//-----------------------------------------------------------------------------
//  Name : cgTexturePool (Class)
/// <summary>
/// A container class responsible for allocating, storing and rapidly
/// retrieving texture resources from an internal collection.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTexturePool : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgTexturePool, "TexturePool" )

public:
    //-------------------------------------------------------------------------
    // Public Structures & Typedefs
    //-------------------------------------------------------------------------
	CGE_SET_DECLARE(cgBufferFormat::Base, FormatTable)
	CGE_SET_DECLARE(cgTexturePoolResource *, ResourceSet)

    // Describes settings to use for the pool
    struct CGE_API Config
    {
        cgUInt32                          minimumResolution;    // The minimum texture resolution that is allowed (resolution will be clamped below this value)
        cgUInt32                          maximumResolution;    // The maximum texture resolution that forms the upper boundary of the 0-1 light source scalar.
        cgUInt32                          maximumSharedPerType; // Maximum number of shared textures allowed per type
        cgUInt32                          poolMemoryLimit;      // Maximum amount of memory that can be consumed by the pool (in megabytes).
		cgTexturePoolResourceDesc::Array  resources;            // Descriptions for the various resources needed by the system
		FormatTable                       resampleFormats;      // Unique formats for resampling/scratch target creation

        // Constructor to provide sensible defaults
        Config() :
            minimumResolution( 1 ), maximumResolution(2048), poolMemoryLimit(0), maximumSharedPerType(1) {}

    }; // End Struct Config

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgTexturePool( );
	virtual ~cgTexturePool( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                                clear                       ( );
    size_t                              getMemoryConsumption        ( ) const;

    // Configuration
    bool                                beginConfigure              ( cgResourceManager * resources, cgUInt32 poolMemoryLimit, cgUInt32 minimumResolution, cgUInt32 maximumResolution, cgUInt32 maximumSharedPerType = 1 );
	void						        addCachedMaps               ( const cgTexturePoolResourceDesc::Array & descriptions, cgUInt32 resolution, cgUInt32 count );
	void						        addDefaultMaps			    ( const cgTexturePoolResourceDesc::Array & descriptions );
	void						        addMaps                     ( cgTexturePoolResourceType::Base type, cgBufferFormat::Base format, cgUInt32 resolution, cgUInt32 count );
	bool                                endConfigure                ( );
	const Config                      & getConfig                   ( ) const;

	// Resource Management
    void                                resetAvailability           (  );
	void                                resetAvailability           ( cgUInt32 dataTypeId );
	bool						        assignResources             ( const cgTexturePoolResourceDesc::Array & descriptions, void * owner, cgUInt32 dataTypeId, cgTexturePoolResourceArray & resources, cgUInt32 & defaultCount );
	bool						        reassignResources           ( const cgTexturePoolResourceDesc::Array & descriptions, void * owner, cgUInt32 dataTypeId, cgTexturePoolResourceArray & resources );
	cgTexturePoolResource             * createResource              ( const cgTexturePoolResourceDesc & description, bool silent = false );
	cgTexturePoolResource             * getResource                 ( const cgTexturePoolResourceDesc & description, const ResourceSet & exclusionList, cgUInt32 channelCount = 0, bool autoCreate = true, bool autoReturnDefault = true );
	cgResampleChain 		          * getResampleChain            ( cgBufferFormat::Base format ); 
	cgBufferFormat::Base		        getBestRenderTargetFormat   ( cgUInt32 precision, cgUInt32 numChannels ) const;
    // ToDo: 6767 - should be references?
	const cgTexturePoolDepthFormatDesc* getBestDepthFormat          ( cgUInt32 precision, bool sample, bool gather, bool compare, cgUInt8 stencilBits = 0 ) const;
	const cgTexturePoolDepthFormatDesc* getBestReadableDepthFormat  ( cgUInt32 precision, cgUInt8 stencilBits = 0 ) const;
    const cgTexturePoolDepthFormatDesc* getSharedDepthStencilFormat ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                        dispose                     ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE     (cgBufferFormat::Base, cgTexturePoolResourceArray, PoolResourceTable)
	CGE_VECTOR_DECLARE  (PoolResourceTable, PoolResources)
    CGE_MAP_DECLARE     (cgBufferFormat::Base, cgResampleChain *, ResampleChainTable)
	CGE_VECTOR_DECLARE  (cgResampleChainMRT *, MRTResampleChainArray)
	CGE_VECTOR_DECLARE  (cgTexturePoolDepthFormatDesc, DepthFormatArray)

    //-------------------------------------------------------------------------
    // Private Functions
    //-------------------------------------------------------------------------
	void						buildDepthFormatArray       ( );
	void						addSystemMaps			    ( );
	cgResampleChain    	      * createResampleChain         ( cgBufferFormat::Base format );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgResourceManager                 * mResources;                 // Resource manager used to allocate data.
	Config                              mSettings;                  // The current configuration settings for our pool.
    size_t                              mPoolMemoryConsumption;     // Current memory consumption of the pool in bytes.
	PoolResources                       mCachedResources;           // The cached resources.
	PoolResources                       mSharedResources;           // The shared resources.
	ResampleChainTable                  mResampleChains;            // Scratch resampling chains (one per requested format) 
	DepthFormatArray                    mDepthFormats[3];           // Supported depth formats (array slots = 16, 24, 32 bits)
	bool                                mConfiguring;               // We're in the process of configuring.
    const cgTexturePoolDepthFormatDesc* mSharedDepthFormat;         // Format information selected for the shared depth buffer.
};

#endif // !_CGE_CGTEXTUREPOOL_H_