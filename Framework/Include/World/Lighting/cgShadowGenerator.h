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
// File : cgShadowGenerator.h                                                //
//                                                                           //
// Desc : Utility classes that provide the majority of the shadow mapping    //
//        implementation as used by the system provided light source types.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSHADOWGENERATOR_H_ )
#define _CGE_CGSHADOWGENERATOR_H_

//-----------------------------------------------------------------------------
// cgShadowGenerator Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/Lighting/cgLightingTypes.h>
#include <Resources/cgResourceTypes.h>
#include <Scripting/cgScriptInterop.h>
/*#include <Rendering/cgRenderingTypes.h>
#include <Resources/cgResourceHandles.h>
#include <Resources/cgTexturePool.h>
#include <Resources/cgTexture.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgFrustum.h>*/

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgTexturePool;
class cgLightNode;

/*class cgVisibilitySet;
class cgRenderDriver;
class cgCameraNode;
class cgLightNode;
class cgShadowGenerator;
class cgPoolResource;
class cgTexturePool;
class cgXMLNode;
struct cgMatrix;*/

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgShadowGenerator (Class)
/// <summary>
/// Utility class used to provide common shadow map rendering
/// functionality as implemented by our system provided lights.
/// New light types are not required to use this class in any way.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgShadowGenerator : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgShadowGenerator, "ShadowGenerator" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgShadowGenerator( cgLightNode * parentLight, cgUInt32 frustumIndex );
	virtual ~cgShadowGenerator( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // ToDo: 6767 - Which of these actually need to be virtual? Almost all are no longer.
    void                            setMethod                ( cgUInt32 method );
    cgUInt32                        getMethod                ( bool hardware = false ) const;
    cgShadowGeneratorType::Base     getType                  ( ) const;
    cgUInt32                        getWritePassCount        ( ) const;
    
    const cgString                & getName                  ( ) const;
    cgCameraNode                  * getCamera                ( ) const;
    cgVisibilitySet               * getVisibilitySet         ( ) const;
    bool                            canBlend                 ( cgShadowGenerator * generator ) const;
    bool                            shouldRegenerate         ( ) const;
    bool							containsRenderableObjects( ) const;

    cgUInt32                        assignResources          ( cgTexturePool * pool = CG_NULL );
    bool						    reassignResources        ( cgTexturePool * pool = CG_NULL );
    void   						    releaseResources         ( );
    bool                            requiresDefaultResource  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    initialize               ( );
    virtual bool                    update                   ( cgUInt32 resolution, const cgShadowSettingsSystem & systemSettings, const cgShadowSettingsLight & lightSettings, const cgTexturePoolResourceDesc::Array & descriptions, cgInt32 flags );
    virtual cgUInt32                getResolution            ( ) const;
    virtual bool                    computeVisibilitySet     ( cgCameraNode * sceneCamera, bool parallelSplit = false, cgFloat nearSplitDistance = 0.0f, cgFloat farSplitDistance = 0.0f );

    virtual cgInt32                 beginWrite               ( bool parallelSplit = false, bool fixedSplitDistances = false );
    virtual bool					beginWritePass           ( cgInt32 passIndex );
    virtual bool                    endWritePass             ( );
    virtual bool                    endWrite                 ( );
    virtual bool                    beginRead				 ( cgFloat attenuation = 1.0f, cgFloat minimumDistance = 10000.0f, cgFloat maximumDistance = 10001.0f, const cgMatrix * textureProjection = CG_NULL );
    virtual void                    endRead					 ( );
    virtual bool					setConstants             ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    // Constant buffer mapped structures
    struct _cbShadow
    {
        cgVector4 shadowAttenuation;  // x = min distance, y = max distance, z = pre-computed atten scale, w = pre-computed atten bias (i.e., 1 - z)
        cgVector4 shadowSplitBlend;
        cgVector4 shadowTextureSize;  // xy = dimensions, zw = 1/dimensions
		cgVector4 edgeTextureChannelMask0;      
		cgVector4 edgeTextureChannelMask1; 
		cgVector4 shadowBias;         // xy = Z scale/bias z = normal scale, w = light scale 
		cgVector4 shadowFilter;       // xy = min/max radius, zw = distance scale/bias

    }; // End Struct : _cbShadow

    enum ShadowOperation
	{
		DrawOpaqueShadowCasters = 0,
		DrawTransparentShadowCasters,
		DrawAllShadowCasters,
		DownsampleDepthMin,
		DownsampleDepthMax,
		DownsampleDepthAvg,
		DownsampleDepthMinMax,
		ComputeEdgeMask,
		ComputeStatistics,
		MergeColorAndEdge,
		ComputeShadows,
	};

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    // ToDo: 6767 - Which of these actually need to be virtual? Almost all are no longer.
    void                    buildOrthoMatrices      ( const cgBoundingBox & casterBounds, const cgBoundingBox & receiverBounds );
	void                    buildOrthoMatrices      ( const cgBoundingBox & casterBounds, const cgFrustum & splitFrustum );
	cgInt32                 getUserDataResourceIndex( cgInt32 data ) const;
    bool                    supportsEdgeMaps        ( ) const;
    cgUInt32                getEdgeMaskChannels     ( ) const;
    cgUInt32				getEdgeMaskChannelCount ( ) const;
    bool					usesEdgeMask            ( ) const;
    bool                    usesTranslucent         ( ) const;
    bool					usesStatistics          ( ) const;
    bool					updateEdgeMask          ( const cgShadowGeneratorOperation & operation );
    bool					copyEdgeToColor         ( const cgShadowGeneratorOperation & operation );
    bool					createStatisticsMap     ( const cgShadowGeneratorOperation & operation );
    bool					setInputs               ( const cgShadowGeneratorInput::Array & anputs );
    bool					updateSamplerStates     ( );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool			executePostOperation    ( const cgShadowGeneratorOperation & operation );
    virtual bool            buildOperations         ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------

	// System
	cgLightNode				          * mParent;                        // The parent light source to which this frustum is attached.
    cgResourceManager                 * mResourceManager;               // Resource manager through which internal resources will be allocated.
    cgRenderDriver                    * mDriver;                        // Render driver through which rendering functionality can be accessed.
  	cgTexturePool                     * mPool;                          // The shadow resource pool currently being used by this generator.

	// General
    cgString					        mName;                          // Name of this generator (a combination of the light source name etc.)
    cgShadowGeneratorType::Base         mType;                          // The generator type
	
	// Settings
	cgUInt32                            mResolution;                    // The current resolution
	cgInt32                             mMethodHW;                      // The current method/flags
	cgShadowSettings                    mSettings;                      // The current shadow settings

	// Resources
    cgTexturePoolResourceDesc::Array    mDescriptions;				    // The resource descriptions required for this frame
    cgTexturePoolResourceArray          mResources;				        // The resources currently assigned to this generator
	cgSamplerStateHandleArray           mSamplerStates;                 // The sampler states associated with reading the resources
	cgUInt32                            mDefaultStatus;			        // How many default resources were assigned?
	bool                                mReassignmentStatus;		    // Was reassignment successful?
	bool                                mDescriptionDirty;		        // Do the resource descriptions need updating?

	// Operations
    cgShadowGeneratorOperation::Array   mWriteOps;                      // The list of write operations to perform.
	cgShadowGeneratorOperation::Array   mPostOps;                       // The list of post-processing operations to perform.
	cgShadowGeneratorOperation::Array   mReadOps;                       // The list of read operations to perform.
	cgInt32                             mCurrentWritePass;              // The current write operation we are processing.
	cgInt32                             mCurrentReadPass;               // The current read operation we are processing.

	// Visibility
	cgCameraNode			          * mCamera;                        // Camera describing the viewer position, orientation and projection for this frustum.
    cgCameraNode			          * mSceneCamera;                   // Backup of the currently selected scene camera during shadow map fill (temporary). 
    bool						        mRegenerate;                    // Should the resources be regenerated because something changed?
    cgUInt32                            mLastVisibilityFrame;           // The last frame on which visibility was computed.

    // Parallel split support
    cgCameraNode			          * mSplitCamera;                   // Custom camera constructed for rendering an individual frustum in the parallel split case.
    cgBoundingBox				        mCasterBounds;                  // Axis aligned bounding box surrounding PSSM casters.
    cgBoundingBox				        mReceiverBounds;                // Axis aligned bounding box surrounding PSSM receivers.
	cgFloat                             mSphereRadius;                  // Radius of the bounding sphere surrounding the split frustum. Minimizes flickering artifacts.
	
	// Edge Masks
	cgUInt32                            mMaskChannels;		            // The currently assigned channel(s) in the mask texture
	bool                                mMaskDirty;			            // Does the mask need to be recomputed?
	bool                                mMaskCachedOnly;                // Should only cached resource pool masks be supported?

	// Attenuation
	cgFloat                             mAttenuation;                   // Precomputed attenuation 
	cgFloat                             mMinimumDistance;               // The falloff min distance 
	cgFloat                             mMaximumDistance;               // The falloff max distance

	// Shaders
	cgSurfaceShaderHandle		        mShader;                        // Internal surface shader file used to correctly handle shadow rendering features.
    cgConstantBufferHandle		        mShadowConstants;               // Constants that provide base shadowing terms.

	// Sampler Indices (ToDo: Make more generic?)
    cgInt32                             mDepthSamplerRegister;          // Register index for the main shadow depth map.    
    cgInt32                             mEdgeSamplerRegister;           // Register index for the edge map.    
    cgInt32                             mColorSamplerRegister;          // Register index for the color map.  
    cgInt32                             mCustomSamplerRegister;         // Register index for the custom map.  
    cgInt32                             mRandomSamplerRegister;         // Register index for the random numbers map.  

	//-------------------------------------------------------------------------
    // Surface Shaders / States / Samplers
    //-------------------------------------------------------------------------

	/// <summary>The image processing shader.</summary>
    cgSurfaceShaderHandle               mImageShader;
    /// <summary>The image processing constants.</summary>
    cgConstantBufferHandle              mImageProcessingConstants;
    /// <summary>The image processing constants.</summary>
    cgConstantBufferHandle              mBilateralConstants;
	/// <summary>The depth-stencil state block handle.</summary>
    cgDepthStencilStateHandle           mDisabledDepthState;
    /// <summary>Blend states for all channel mask permutations.</summary>
	cgBlendStateHandle                  mBlendStates[16];
	/// <summary>The input texture point sampler.</summary>
    cgSampler *                         mPointSampler;
    /// <summary>The input texture linear sampler.</summary>
    cgSampler *                         mLinearSampler;
	/// <summary>The samplers for vertical blurring with point filtering.</summary>
    cgSampler *                         mVerticalLinearSampler;
    cgSampler *                         mVerticalPointSampler;
	/// <summary>The samplers for vertical blurring with point filtering.</summary>
    cgSampler *                         mHorizontalLinearSampler;
    cgSampler *                         mHorizontalPointSampler;
};

//-----------------------------------------------------------------------------
//  Name : cgReflectanceGenerator (Class)
/// <summary>
/// Utility class used to provide reflective shadow map rendering
/// functionality as implemented by our system provided lights.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgReflectanceGenerator : public cgShadowGenerator
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgReflectanceGenerator, cgShadowGenerator, "ReflectanceGenerator" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
		     cgReflectanceGenerator( cgLightNode * pParentLight, cgUInt32 nFrustumIndex );
	virtual ~cgReflectanceGenerator( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    virtual bool                            initialize               ( );
    virtual bool							update                   ( cgUInt32 resolution, const cgShadowSettingsSystem & systemSettings, const cgShadowSettingsLight & lightSettings, const cgTexturePoolResourceDesc::Array & descriptions, cgInt32 flags );
    virtual bool                            computeVisibilitySet     ( cgCameraNode * sceneCamera, bool parallelSplit = false, cgFloat nearSplitDistance = 0.0f, cgFloat farSplitDistance = 0.0f );

    // Rendering
    virtual bool                            beginRead				 ( cgFloat attenuation = 1.0f, cgFloat minimumDistance = 10000.0f, cgFloat maximumDistance = 10001.0f, const cgMatrix * textureProjection = CG_NULL );
	virtual bool							setConstants             ( );
    virtual cgUInt32                        getResolution            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                            dispose                 ( bool bDisposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Enumerations
    //-------------------------------------------------------------------------
	enum ReflectanceOperation
	{
		WriteGBuffer = 100,
		ReadGBuffer,
		DownsampleRSMMin,
		DownsampleRSMMax,
		DownsampleRSMAvg,
		MergeDepthNormal
	};

    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct _cbRSM
    {
        cgVector4 textureSize;       
        cgVector4 textureScaleBias;  
		cgVector4 screenToViewScaleBias;
		cgVector4 depthUnpack;
		cgVector3 position;
		cgVector3 direction;
		cgVector3 color;
		cgFloat   sampleRadius; 
		cgFloat   geometryBias;
		cgMatrix  textureProjectionMatrix;
		cgMatrix  inverseViewMatrix;

    }; // End Struct : _cbRSM

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
	bool						buildDescriptions        ( );
    bool                        downsampleRSM            ( const cgShadowGeneratorOperation & operation );
	bool                        mergeDepthNormalBuffers  ( const cgShadowGeneratorOperation & operation );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgShadowGenerator)
    //-------------------------------------------------------------------------
    virtual bool		        buildOperations          ( );
    virtual bool				executePostOperation     ( const cgShadowGeneratorOperation & operation );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgConstantBufferHandle      mRSMConstants;              // Constants that provide base reflective shadow mapping terms.
	cgUInt32                    mRSMResolutionFinal;        // The resolution of the downsampled RSM buffers 
	cgInt32                     mResampleChainIndex;        // The index of the resample chain used for downsampling RSMs
	cgSampler                 * mRSMSampler0;
	cgSampler                 * mRSMSampler1;
	cgSampler                 * mRSMSampler2;
	cgTextureHandle             mRandomOffsetTexture;
};

#endif // !_CGE_CGSHADOWGENERATOR_H_