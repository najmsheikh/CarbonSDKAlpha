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
// Name : cgLightingManager.h                                                //
//                                                                           //
// Desc : Provides classes responsible for the management of the various     //
//        lighting systems used by the engine.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLIGHTINGMANAGER_H_ )
#define _CGE_CGLIGHTINGMANAGER_H_

//-----------------------------------------------------------------------------
// cgLightingManager Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/Lighting/cgLightingTypes.h>
#include <World/cgWorldTypes.h>
#include <World/cgWorldQuery.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>
#include <Resources/cgResourceTypes.h>
#include <System/cgEventDispatcher.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgTexturePool;
class cgRadianceGrid;
class cgRadianceBuffer;
/*class cgScene;
class cgRenderDriver;
class cgTexturePool;
class cgCameraNode;
class cgLightNode;
class cgScriptObject;*/


//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgLightingManager (Class)
/// <summary>
/// The manager class responsible for handling various direct and indirect lighting
/// tasks.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLightingManager : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgLightingManager, "LightingManager" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgRadianceGrid;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLightingManager( cgScene * pParent );
    virtual ~cgLightingManager( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        initialize              ( cgScriptObject * renderControlScriptObject );
    bool                        update                  ( cgCameraNode * camera );
    void                        setRenderControl        ( cgScriptObject * scriptObject );
    cgScene                   * getParentScene          ( ) const;
    cgRenderDriver            * getRenderDriver         ( ) const;
    cgResourceManager         * getResourceManager      ( ) const;
    cgTexturePool             * getShadowMaps           ( ) const;
    
    // Automated Light Processing
    void                        processShadowMaps       ( cgCameraNode * camera, bool reassignMaps, const cgString & renderCallback );
    void                        processLights           ( cgCameraNode * camera, bool deferred, bool applyShadows, bool viewSpace, const cgString & lightCallback, const cgString & processCallback );
    void                        processLights           ( const cgRenderTargetHandle& lightingBuffer, cgCameraNode * camera, bool deferred, bool applyShadows, bool viewSpace, const cgString & lightCallback, const cgString & processCallback );
    void                        processIndirectLights   ( const cgRenderTargetHandle& lightingBuffer, const cgRenderTargetHandle& lightingBufferLow, const cgRenderTargetHandle& depthBufferLow, const cgDepthStencilTargetHandle& lowResDS, const cgDepthStencilTargetHandle& highResDS, cgCameraNode * camera, const cgString & lightCallback, const cgString & processCallback );

    // Constant buffer updates
    bool                        setConstant             ( const cgString & name, const void * value );
    bool                        applyLightingConstants  ( );

	// Indirect lighting
	bool						addRadianceGrid         ( cgFloat cellSize, const cgVector3 & dimensions, cgUInt32 propogationCount, cgUInt32 padding, cgFloat lightStrength, cgFloat occlusionStrength );
    cgRadianceGrid            * getRadianceGrid         ( cgUInt32 index ) const;

	// Shadow map management
	bool                        loadShadowSettings      ( );
	bool                        addShadowSettings       ( const cgString & referenceName, const cgShadowSettingsSystem & settings );
    cgInt32						getShadowSettings       ( const cgShadowSettingsLOD::Array & detailLevels, bool RSM, cgShadowSettingsSystem & settingsOut, cgTexturePoolResourceDesc::Array & descriptionsOut, cgInt32 & flagsOut ) const;
	bool					    validateShadowSettings  ( const cgShadowSettingsSystem & settings ) const;
    bool						getResourceDescriptions ( const cgShadowSettingsSystem & settings, cgTexturePoolResourceDesc::Array & descriptionsOut, cgInt32 & flagsOut ) const;
	void						generateShadowSettings  ( cgUInt32 flags, cgShadowSettingsSystem & settingsOut ) const;
	bool                        beginShadowConfigure    ( cgUInt32 poolMemoryLimit, cgUInt32 minimumResolution, cgUInt32 maximumResolution );
	void						addDefaultMaps			( cgUInt32 method );
	void						addCachedMaps           ( cgUInt32 method, cgUInt32 resolution, cgUInt32 count );
	void						addCachedMaps           ( cgBufferFormat::Base format, cgUInt32 resolution, cgUInt32 count );
	bool                        endShadowConfigure      ( );
	cgUInt32                    getMaxShadowResolution  ( bool inPixels = true ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Functions
    //-------------------------------------------------------------------------
    // ToDo: 6767 - Clean up.
	inline cgInt32              getSystemShadowDetailLevel  ( ) const { return mShadowSystemLOD; }
	inline void                 getSystemShadowDetailLevel  ( cgInt32 level ) { mShadowSystemLOD = level; }

	inline cgInt32              getSystemGIDetailLevel      ( ) const { return mIndirectSystemLOD; }
	inline void                 getSystemGIDetailLevel      ( cgInt32 level ) { mIndirectSystemLOD = level; }

	inline cgInt32              getIndirectMethod           ( ){ return mIndirectMethod; }

protected:
    //-------------------------------------------------------------------------
    // Protected Structures & Typedefs
    //-------------------------------------------------------------------------
    // Constant buffer mapped structures
    struct _cbLightingSystem
    {
        cgVector3       position;               // The position in the space the lighting system is currently operating in
        cgVector3       direction;              // The position in the space the lighting system is currently operating in
        cgVector4       attenuationBufferMask;  // The attenuation buffer texture channel for the current light being processed
        cgMatrix        textureProjMatrix;      // The projection matrix modified for the space the lighting system is currently operating in

    }; // End Struct : _cbLightingSystem

    struct _cbBilateral
    {
        cgVector4       textureSize;            // The texture dimensions and reciprocals
        cgVector2       depthScaleBias;         // Scale and bias for depth compares

    }; // End Struct : _cbBilateral

    // Utility structures
    struct ShadowSettingsData
    {
        cgInt32                             flags;
        cgShadowSettingsSystem              settings;
        cgTexturePoolResourceDesc::Array    resourceDescriptions;
    
    }; // End Struct : ShadowSettingsData
    
    // Container declarations
    CGE_MAP_DECLARE     (cgString, ShadowSettingsData, ShadowSettingsTable)
    CGE_VECTOR_DECLARE  (cgRadianceGrid *, GridArray)
    CGE_VECTOR_DECLARE  (cgRadianceBuffer *, RadianceBufferArray)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        processLight                ( cgLightNode * light, cgCameraNode * camera, bool deferred, bool applyShadows, const cgString & lightCallback );
    
    void						updateIndirectMaps          ( const cgObjectNodeArray & lights, cgCameraNode * camera, bool reassignMaps, cgUInt32 resetAvailabilityFilter, const cgString & renderCallback );
    void						collectIndirectLights       ( cgCameraNode * camera );
	void						updateGrids                 ( cgCameraNode * camera );
	void						computeIndirectLighting     ( cgCameraNode * camera, const cgString & processCallback );
	void						smoothIndirectLighting      ( cgCameraNode * camera, const cgString & processCallback );
	void						computeIndirectLightingLow  ( cgCameraNode * camera, const cgString & processCallback );
	void						processTasks                ( );
	void                        debugGrids                  ( cgCameraNode * camera );

    cgRadianceBuffer          * getScratchBuffer            ( const cgVector3 & dimensions );
    bool						getScratchBuffers           ( const cgVector3 & dimensions, cgRadianceBuffer *& buffer0, cgRadianceBuffer *& buffer1 );
    void                        replaceScratchBuffer        ( cgRadianceBuffer * oldBuffer, cgRadianceBuffer * newBuffer );

	void                        blurSphericalHarmonics      ( cgRenderTargetHandle & redTarget, cgRenderTargetHandle & greenTarget, cgRenderTargetHandle & blueTarget );
	void                        upsampleLighting            ( cgRenderTargetHandle & redTarget, cgRenderTargetHandle & greenTarget, cgRenderTargetHandle & blueTarget );
	void						reprojectSphericalHarmonics ( cgCameraNode * camera, cgRenderTargetHandle & redTarget, cgRenderTargetHandle & greenTarget, cgRenderTargetHandle & blueTarget );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgScene                   * mParentScene;                   //< The scene that owns this lighting manager.
    cgScriptObject            * mScriptObject;                  //< Reference to the scripted render control object (owned exclusively by the script).
    _cbLightingSystem           mConstants;		                //< Local copy of the constants for constant buffer uploading (lighting)
    cgScriptArgument::Array     mCompositeVSArgs;			    //< Preset argument script list for use during compositing (vertex shader)
    cgScriptArgument::Array     mCompositePSArgs;			    //< Preset argument script list for use during compositing (pixel shader)
    cgLightingSystemStates      mStates;                        //< Collection of lighting system states and other rendering resources.

    // Shadows
    cgTexturePool             * mShadowMapPool;			        //< Pool of shadow maps for caching light source shadows (includes default maps for each shadow type).
	ShadowSettingsTable         mShadowSettings;                //< Table of shadow settings for various types and levels of detail.
	cgString                    mDefaultShadowSettings;         //< The name of the default/fallback shadow settings in the table
	cgInt32                     mShadowSystemLOD;               //< The current level of shadow detail for the system

    // Global Illumination
    cgInt32                     mIndirectMethod;                //< The current indirect lighting algorithm
	cgObjectNodeArray           mStaticIndirectLights;          //< List of current static indirect lights
	cgObjectNodeArray           mDynamicIndirectLights;         //< List of current dynamic indirect lights
	GridArray                   mRadianceGrids;			        //< An array of radiance grids for indirect lighting.
	cgString				    mIndirectLightCallback;         //< Script callback function for light source updates (i.e., rsm fills)
	cgString                    mProcessCallback;               //< Script callback function for running pre and post lighting processes
	cgInt32                     mIndirectSystemLOD;             //< The current level of global illumination detail for the system
	cgFloat                     mDynamicTimeThreshold;          //< Amount of elapsed time without changes before a light is considered static
	RadianceBufferArray         mScratchRadianceBuffers0;       //< An array of radiance buffers for scratch purposes (batch 0)
	RadianceBufferArray         mScratchRadianceBuffers1;       //< An array of radiance buffers for scratch purposes (batch 1)
};

#endif // !_CGE_CGLIGHTINGMANAGER_H_