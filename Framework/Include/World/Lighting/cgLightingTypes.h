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
// Name : cgLightingTypes.h                                                  //
//                                                                           //
// Desc : Common system file that defines various lighting related types and //
//        common enumerations.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLIGHTINGTYPES_H_ )
#define _CGE_CGLIGHTINGTYPES_H_

//-----------------------------------------------------------------------------
// cgLightingTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Rendering/cgRenderingTypes.h> // ToDo: 6767 - Required by cgShadowSystemStates
#include <Resources/cgResourceHandles.h> // ToDo: 6767 - Required by cgShadowGeneratorInput

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSampler;    // ToDo: 6767 - cgLightingSystemStates needs this

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
/// <summary>Describes the specific indirect lighting task to perform.</summary>
namespace cgIndirectLightingTaskType
{
    enum Base
    {
        Unassigned = 0,
        ReassignRSMs,
        FillRSMs,
        InjectRSMs,
        GatherRSMs,
        ReprojectGrid,
        Bounce,
        Propagate,
    };

} // End Namespace : cgIndirectLightingTaskType

namespace cgShadowGeneratorType
{
    enum Base
    {
        ShadowMap           = 1,
        ReflectiveShadowMap = 2
    };

} // End Namespace : cgShadowGeneratorType

namespace cgShadowResourceType
{
    enum Base
    {
        DepthStencilBuffer,
        DepthMap,
        ColorMap,
        StatisticsMap,
        NormalMap,
        EdgeMap,
        DepthNormalMap
    };

} // End Namespace : cgShadowResourceType

// ToDo: 6767 - Don't like this name, it's not a result. Request?
namespace cgShadowGeneratorFillResult
{
    enum Base
    {
        DoNothing       = 0,
        CannotFill      = (1L << 0),
        CanFill         = (1L << 1),
        MustFill        = (1L << 2),
    };

}; // End Namespace : cgShadowGeneratorFillResult

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
/// <summary>Describes a specific region of an indirect lighting grid.</summary>
struct cgIndirectLightingGridSlice
{
    CGE_ARRAY_DECLARE( cgIndirectLightingGridSlice, Array )

    // Public members
    cgInt32 gridIndex;
    cgInt32 sliceStart;
    cgInt32 sliceEnd;
    
}; // End Struct : cgIndirectLightingGridSlice

/// <summary>Describes an indirect lighting task to be performed.</summary>
struct CGE_API cgIndirectLightingTask
{        
    cgInt32                             frame;                  // The frame to do the work on
    cgIndirectLightingTaskType::Base    type;                   // The type of task to do
    cgInt32                             lightIndexStart;        // The first light to process
    cgInt32                             lightIndexEnd;          // The last light to process
    bool                                staticUpdate;           // Static vs. dynamic update
    cgIndirectLightingGridSlice::Array  slices;                 // The list of slices to process

    // Constructor
    cgIndirectLightingTask() : 
        frame(-1), type(cgIndirectLightingTaskType::Unassigned), lightIndexStart(-1), lightIndexEnd(-1), staticUpdate(true) {}

}; // End Struct : cgIndirectLightingTask

/// <summary>A description structure for shadow detail levels.</summary>
struct CGE_API cgShadowSettingsSystem
{
    // Basic Settings
    cgUInt32            method;                // The current shadow method
    cgUInt32            resolutionAdjust;      // The resolution shift
    cgUInt32            precision;             // The precision of the primary depth target

    // Sampling
    cgUInt32            primarySamples;        // Number of primary samples to take
    cgUInt32            secondarySamples;      // Number of secondary samples to take
    cgUInt32            anisotropy;            // Number anisotropic samples
    // ToDo: 6767 - Can this go away?
    cgUInt32		    MSAASamples;           // Number MSAA samples
    bool                autoGenerateMipmaps;   // Are we using auto mip map generation?
    bool                jitter;                // Are we using sample jittering?
    bool                rotate;                // Are we using random sample rotation? 
    bool                boxFilter;             // Are we using a box shape filter (vs. random)?
    bool                bilinearFiltering;     // Should bilinear filtering be used?
    bool                trilinearFiltering;    // Should trilinear filtering be used?
    bool                normalOffset;          // Should normal offset biasing be used?

    // Filtering
    cgUInt32            filterPasses;          // Number of filtering passes to run
    cgFloat             filterRadius;          // Filter radius 
    cgFloat             filterRadiusNear;      // A minimum filter radius
    cgFloat             filterRadiusFar;       // A maximum filter radius

    // Edge Masking
    cgUInt32            maskType;              // What type of edge masking should be used?
    cgUInt32            maskPrecision;         // How many bits should we use per edge channel (8, 16, or 32)?

    // Misc
    bool                translucency;          // Do we need to support translucent shadow casters?

    // Constructor
    cgShadowSettingsSystem() :
        method(0), resolutionAdjust(0), precision(0), primarySamples(0), secondarySamples(0),
        anisotropy(0), MSAASamples(0), autoGenerateMipmaps(false), jitter(false),
        rotate(false), boxFilter(false), bilinearFiltering(false), trilinearFiltering(false),
        normalOffset(false), filterPasses(0), filterRadius(0), filterRadiusNear(0),
        filterRadiusFar(0), maskType(0), maskPrecision(0), translucency(false) {}

}; // End Struct : cgShadowSettingsSystem

struct CGE_API cgShadowSettingsLight
{
    // Basic Settings
    cgUInt32            method;                 // The method to use
    cgUInt32            resolutionAdjust;       // The resolution shift

    // General rendering
    cgCullMode::Base    cullMode;               // How the geometry should be culled during rendering.

    // Filtering
    cgFloat             filterBlurFactor;       // A value that controls filter blurriness
    cgFloat             filterDistanceNear;     // Distance from 0 for which near radius is used (meters)
    cgFloat             filterDistanceFar;      // Distance at which maximum radius is used (meters)

    // Biasing
    cgFloat             depthBiasSW;            // Software bias used to help reduce erroneous self-shadowing artifacts.
    cgFloat             depthBiasHW;            // Hardware depth bias used to help reduce erroneous self-shadowing artifacts.
    cgFloat             slopeScaleBias;         // Hardware slope based biasing used to help reduce erroneous self-shadowing artifacts.
    cgFloat             normalBiasSurface;      // Scale value to use when biasing is based on surface normals
    cgFloat             normalBiasLight;        // Scale value to use when biasing is based on surface normals

    // Statistical Properties
    cgFloat             minimumVariance;        // Specifies a boundary for the variance estimate (to help avoid filtering errors and reduce artifacts).
    cgFloat             exponent;               // The exponent to use with exponential shadow mapping
    cgFloat             minimumCutoff;          // The minimum intensity under which the occlusion result will be remapped to zero (helps reduce light bleeding).

    // Edge Masking
    cgFloat             maskThreshold;          // Minimum depth difference to declare an edge (meters)

    // Global Illumination
    bool                projectCell;            // What sampling mode is active?
    cgFloat             intensity;              // Custom intensity control

    // Misc
    bool                translucency;           // Do we need to support translucent shadow casters?

    cgShadowSettingsLight() :
        method(0), resolutionAdjust(0), cullMode(cgCullMode::None), filterBlurFactor(0),
        filterDistanceNear(0), filterDistanceFar(0), depthBiasSW(0), depthBiasHW(0),
        slopeScaleBias(0), normalBiasSurface(0), normalBiasLight(0), minimumVariance(0),
        exponent(0), minimumCutoff(0), maskThreshold(0), projectCell(false),
        intensity(0), translucency(false) {}

}; // End Struct : cgShadowSettingsLight

// A description structure for shadow detail levels
struct CGE_API cgShadowSettings
{
    // Basic Settings
    cgUInt32            method;                // The method to use
    cgUInt32            resolutionAdjust;      // The adjustment to maximum resolution (shift)
    cgUInt32            precision;             // The precision of the primary depth target

    // General rendering
    cgCullMode::Base    cullMode;              // How the geometry should be culled during rendering.

    // Sampling
    cgUInt32            primarySamples;        // Number of primary samples to take
    cgUInt32            secondarySamples;      // Number of secondary samples to take
    cgUInt32            anisotropy;            // Number anisotropic samples
    // ToDo: 6767 - Can this go away?
    cgUInt32		    msaaSamples;           // Number MSAA samples
    bool                jitter;                // Are we using sample jittering?
    bool                rotate;                // Are we using random sample rotation? 
    bool                boxFilter;             // Are we using a box shape filter (vs. random)?
    bool                bilinearFiltering;     // Should bilinear filtering be used?
    bool                trilinearFiltering;    // Should trilinear filtering be used?

    // Filtering
    cgFloat             filterRadius;          // Filter radius 
    cgFloat             filterBlurFactor;      // A value that controls filter blurriness
    cgUInt32            filterPasses;          // Number of filtering passes to run
    cgFloat             filterRadiusNear;      // A minimum filter radius
    cgFloat             filterRadiusFar;       // A maximum filter radius
    cgFloat             filterDistanceNear;    // Distance from 0 for which near radius is used (meters)
    cgFloat             filterDistanceFar;     // Distance at which maximum radius is used (meters)

    // Biasing
    cgFloat             depthBiasSW;           // Software bias used to help reduce erroneous self-shadowing artifacts.
    cgFloat             depthBiasHW;           // Hardware depth bias used to help reduce erroneous self-shadowing artifacts.
    cgFloat             slopeScaleBias;        // Hardware slope based biasing used to help reduce erroneous self-shadowing artifacts.
    cgFloat             normalBiasSurface;     // Scale value to use when biasing is based on surface normals
    cgFloat             normalBiasLight;       // Scale value to use when biasing is based on surface normals

    // Statistical Properties
    cgFloat             minimumVariance;       // Specifies a boundary for the variance estimate (to help avoid filtering errors and reduce artifacts).
    cgFloat             exponent;              // The exponent to use with exponential shadow mapping

    // Edge Masking
    cgUInt32            maskType;              // What type of edge masking should be used?
    cgUInt32            maskPrecision;         // How many bits should we use per edge channel (8, 16, or 32)?
    cgFloat             maskThreshold;         // Minimum depth difference to declare an edge (meters)

    // Global Illumination
    bool                projectCell;           // What sampling mode is active?
    cgFloat             intensity;             // Custom intensity control

    // Misc
    bool                translucency;          // Do we need to support translucent shadow casters?
    cgFloat             minimumCutoff;          // A minimum intensity value for use in user-defined ways (e.g., vsm light bleed reduction, vpl biasing) 

    // Constructors
    cgShadowSettings() :
        method(0), resolutionAdjust(0), precision(0), cullMode(cgCullMode::None), 
        primarySamples(0), secondarySamples(0), anisotropy(0), msaaSamples(0),
        jitter(false), rotate(false), boxFilter(false), bilinearFiltering(false),
        trilinearFiltering(false), filterRadius(0), filterBlurFactor(0), filterPasses(0),
        filterRadiusNear(0), filterRadiusFar(0), filterDistanceNear(0), filterDistanceFar(0),
        depthBiasSW(0), depthBiasHW(0), slopeScaleBias(0), normalBiasSurface(0),
        normalBiasLight(0), minimumVariance(0), exponent(0), maskType(0),
        maskPrecision(0), maskThreshold(0), projectCell(false), intensity(0),
        translucency(false), minimumCutoff(0) {}

    cgShadowSettings( const cgShadowSettingsSystem & s, const cgShadowSettingsLight & l ) :
        method              (s.method),
        resolutionAdjust    (s.resolutionAdjust + l.resolutionAdjust), 
        precision           (s.precision),

        // General rendering
        cullMode            (l.cullMode),

        // Sampling
        primarySamples      (s.primarySamples),
        secondarySamples    (s.secondarySamples),
        anisotropy          (s.anisotropy),
        msaaSamples         (s.MSAASamples),
        jitter				(s.jitter),
        rotate				(s.rotate),
        boxFilter			(s.boxFilter),
        bilinearFiltering   (s.bilinearFiltering),
        trilinearFiltering  (s.trilinearFiltering),

        // Filtering
        filterRadius        (s.filterRadius),
        filterBlurFactor    (l.filterBlurFactor),
        filterPasses        (s.filterPasses),
        filterRadiusNear    (s.filterRadiusNear),
        filterRadiusFar     (s.filterRadiusFar),
        filterDistanceNear  (l.filterDistanceNear),
        filterDistanceFar   (l.filterDistanceFar),

        // Biasing
        depthBiasSW			(l.depthBiasSW),
        depthBiasHW			(l.depthBiasHW),
        slopeScaleBias      (l.slopeScaleBias),
        normalBiasSurface   (l.normalBiasSurface),
        normalBiasLight     (l.normalBiasLight),

        // Statistical Properties
        minimumVariance     (l.minimumVariance),
        exponent			(l.exponent),

        // Edge Masking
        maskType			(s.maskType),
        maskPrecision       (s.maskPrecision),
        maskThreshold       (l.maskThreshold),

        // Global Illumination
        projectCell         (l.projectCell),
        intensity           (l.intensity),

        // Misc
        translucency        (s.translucency && l.translucency),
        minimumCutoff		(l.minimumCutoff) {}

}; // End Struct : cgShadowSettings

struct cgShadowSettingsLOD
{
    CGE_ARRAY_DECLARE( cgShadowSettingsLOD, Array )

    // Public Members
    cgInt32  level;
    cgString name;  // ToDo: 6767 - Can change to integer based (map as well)

}; // End Struct: cgShadowSettingsLOD

// ToDo: 6767 -- Do we even need this? Seems like this is just a 'sampler'.
struct CGE_API cgShadowGeneratorInput
{
    CGE_ARRAY_DECLARE( cgShadowGeneratorInput, Array )

    // Public Members
    cgInt32              samplerIndex;
    cgTextureHandle      texture;
    cgSamplerStateHandle states;

    // Constructors
    cgShadowGeneratorInput() :
        samplerIndex( -1 ) {}
    cgShadowGeneratorInput( cgInt32 _samplerIndex, cgTextureHandle _texture ) : 
        samplerIndex( _samplerIndex ), texture( _texture ){}
    cgShadowGeneratorInput( cgInt32 _samplerIndex, cgTextureHandle _texture, cgSamplerStateHandle _states ) : 
        samplerIndex( _samplerIndex ), texture( _texture ), states( _states ){}

}; // End Struct : cgShadowGeneratorInput

struct CGE_API cgShadowGeneratorOperation
{
    CGE_ARRAY_DECLARE( cgShadowGeneratorOperation, Array )

    // Public Members
    cgUInt32						operationTypeId;
    cgShadowGeneratorInput::Array   inputs;
    cgRenderTargetHandleArray       outputs;
    cgDepthStencilTargetHandle      depthStencil;
    cgUInt32					    clearFlags;
    cgUInt32					    clearColorValue;
    cgFloat						    clearDepthValue;
    cgUInt8						    clearStencilValue;
    cgCullMode::Base                cullMode;
    cgUInt32                        colorWrites;

    // Constructors
    cgShadowGeneratorOperation( ) :
        operationTypeId(0xFFFFFFFF), clearFlags(0), clearColorValue(0),
        clearDepthValue(1.0f), clearStencilValue(0), colorWrites(cgColorChannel::All),
        cullMode( cgCullMode::Back ) {}
    
    cgShadowGeneratorOperation( cgUInt32 _operationTypeId ) :
        operationTypeId(_operationTypeId), clearFlags(0), clearColorValue(0),
        clearDepthValue(1.0f), clearStencilValue(0), colorWrites(cgColorChannel::All),
        cullMode( cgCullMode::Back ) {}

}; // End Struct : cgShadowGeneratorOperation

struct CGE_API cgLightingSystemStates
{
    // Resources
    cgSurfaceShaderHandle       lightingShader;			        //< Internal surface shader used to correctly handle lighting.                      
    cgConstantBufferHandle      lightingSystemConstants;		//< Lighting system level constants (view space position, texture projection, etc.) 
    cgConstantBufferHandle      bilateralConstants;				//< Constant buffer for bilateral filtering
    cgRenderTargetHandle        currentLightingTarget;
    cgRenderTargetHandle        lightingTargetLow;
    cgRenderTargetHandle        depthTargetLow;
    cgDepthStencilTargetHandle  depthStencilBufferLow;          //< Low res depth stencil buffer used during compositing
    cgDepthStencilTargetHandle  depthStencilBufferHigh;         //< High res depth stencil buffer used during compositing

    // States
    cgDepthStencilStateHandle   disabledDepthState;             //< The disabled depth-stencil state block handle.
    cgDepthStencilStateHandle   greaterDepthState;              //< The greater than depth-stencil state block handle. 
    cgDepthStencilStateHandle   lessEqualTestDepthState;        //< The greater than depth-stencil state block handle. 
    cgDepthStencilStateHandle   testStencilDepthState;          //< The compositing depth-stencil state block handle. 
    cgDepthStencilStateHandle   stencilClearState;              //< The stencil clearing depth-stencil state block handle. 

    cgBlendStateHandle          additiveBlendState;             //< The additive blend state block handle.
    cgBlendStateHandle          disabledBlendState;             //< Blend state for stencil clearing
    cgBlendStateHandle          multiplyDestBlendState;         //< Blend state for destination multiplication
    cgBlendStateHandle          alphaBlendState;                //< Standard src/invsrc alpha blending.

    // Samplers
    cgSampler                 * harmonicsRedSampler;            //< Spherical harmonics sampler (red)
    cgSampler                 * harmonicsGreenSampler;          //< Spherical harmonics sampler (green)
    cgSampler                 * harmonicsBlueSampler;           //< Spherical harmonics sampler (blue)
    cgSampler                 * harmonicsAuxiliarySampler;      //< Spherical harmonics sampler (auxiliary)

    cgSampler                 * harmonicsRedPointSampler;       //< Spherical harmonics sampler (red)
    cgSampler                 * harmonicsGreenPointSampler;     //< Spherical harmonics sampler (green)
    cgSampler                 * harmonicsBluePointSampler;      //< Spherical harmonics sampler (blue)
    cgSampler                 * harmonicsAuxiliaryPointSampler; //< Spherical harmonics sampler (auxiliary)

    cgSampler                 * harmonicsRedVerticalSampler;    //< Spherical harmonics sampler (red)
    cgSampler                 * harmonicsGreenVerticalSampler;  //< Spherical harmonics sampler (green)
    cgSampler                 * harmonicsBlueVerticalSampler;   //< Spherical harmonics sampler (blue)

    cgSampler                 * harmonicsRedHorizontalSampler;  //< Spherical harmonics sampler (red)
    cgSampler                 * harmonicsGreenHorizontalSampler;//< Spherical harmonics sampler (green)
    cgSampler                 * harmonicsBlueHorizontalSampler; //< Spherical harmonics sampler (blue)

    cgSampler                 * depthLowResSampler;             //< Low res depth buffer
    cgSampler                 * lightingSampler;                //< Low res lighting sampler

    // Constructor
    cgLightingSystemStates() :
        harmonicsRedSampler(CG_NULL), harmonicsGreenSampler(CG_NULL), harmonicsBlueSampler(CG_NULL),
        harmonicsAuxiliarySampler(CG_NULL), harmonicsRedPointSampler(CG_NULL), harmonicsGreenPointSampler(CG_NULL),
        harmonicsBluePointSampler(CG_NULL), harmonicsAuxiliaryPointSampler(CG_NULL), harmonicsRedVerticalSampler(CG_NULL),
        harmonicsGreenVerticalSampler(CG_NULL), harmonicsBlueVerticalSampler(CG_NULL), harmonicsRedHorizontalSampler(CG_NULL),
        harmonicsGreenHorizontalSampler(CG_NULL), harmonicsBlueHorizontalSampler(CG_NULL), depthLowResSampler(CG_NULL),
        lightingSampler(CG_NULL) {}


}; // End Struct : cgLightingSystemStates

#endif // !_CGE_CGLIGHTINGTYPES_H_