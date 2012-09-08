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
// Name : cgRenderingTypes.h                                                 //
//                                                                           //
// Desc : Common system file that defines various rendering types and common //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRENDERINGTYPES_H_ )
#define _CGE_CGRENDERINGTYPES_H_

//-----------------------------------------------------------------------------
// cgRenderingTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

// ToDo: 6767 - Should any shadow stuff exist here? cgShadowTypes.h?

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
// ToDo: 6767 -- This can now be removed?
// Different capabilities tests that can be performed (see cgRenderDriver::CheckCapabilities())
namespace cgDriverCapabilities
{
    enum Base
    {
        FastStencilFill = 0,
        TexNonPow2,
        AutoGenMipMaps,
        SupportsShaderModel,
        // ToDo: 6767 - Maybe move into new capabilities system.
        HardwareDepthSample,
        HardwareDepthGather,
        HardwareDepthCompare,
        HardwareDepthGatherCompare,
    };

}; // End Namespace : cgDriverCapabilities

namespace cgSystemState
{
    enum Base
    {
        // Renderer
        ShadingQuality = 0,
        OrthographicCamera,
        OutputEncodingType,
        FogModel,
        HDRLighting,
        ViewSpaceLighting,
        DeferredRendering,
        DeferredLighting,
        SpecularColorOutput,
        GBufferSRGB,
        NonLinearZ,
        NormalizedDistance,
        SurfaceNormals,
        PackedDepth,
        ColorWrites,
        CullMode,

        // Geometry
        MaximumBlendIndex,
        DepthType,
        SurfaceNormalType,

        // Lights
        LightType,
        ShadowMethod,
        PrimaryTaps,
        SecondaryTaps,
        SampleAttenuation,
        SampleDistanceAttenuation,
        ColorTexture3D, // ToDo: 6767 - Rename this (and the corresponding script side variable)
        DiffuseIBL,
        SpecularIBL,
        ComputeAmbient,
        ComputeDiffuse,
        ComputeSpecular,
        UseSSAO,
        Trilighting, // ToDo: 6767 'UseTrilighting' ?

        // Materials
        NormalSource,
        ReflectionMode,
        LightTextureType,
        SampleDiffuseTexture,
        SampleSpecularColor,
        SampleSpecularMask,
        SampleGlossTexture,
        SampleEmissiveTexture,
        SampleOpacityTexture,
        DecodeSRGB,
        ComputeToksvig,
        CorrectNormals,
        OpacityInDiffuse,
        SurfaceFresnel,
        Metal,
        Transmissive,
        Translucent,
        AlphaTest,
        Emissive
    };

} // End Namespace : cgSystemState

// Various defined scene stencil Ids for masking deferred elements.
namespace cgStencilId
{
    enum Base
    {
        LightingOff         = 0x0,
        DeferredOn          = 0x1,
        Inside              = 0x2,
        LightBitsDynamic    = 0x7,  // Lowest 3 stencil bits are used by dynamic lights.
        Precomputed         = 0x8,
        LightBitsStatic     = 0xF,  // Lowest 4 stencil bits are used by static lights.
        Terrain             = 0x10,
        TransparentFDT      = 0x20,
        TransparentSDT      = 0x40,
        TransparentAny      = 0x80,
        All                 = 0xFF
    };

}; // End Namespace : cgStencilId

// The different types of supported primitives for rendering.
namespace cgPrimitiveType
{
    // Matched to D3DPRIMITIVETYPE & D3D11_PRIMITIVE_TOPOLOGY values.
    enum Base
    {    
        PointList       = 1,
        LineList        = 2,
        LineStrip       = 3,
        TriangleList    = 4,
        TriangleStrip   = 5,
        TriangleFan     = 6     // Unsupported topology type in D3D10+
    };

}; // End Namespace : cgPrimitiveType

// The different types of buffer clear operations.
namespace cgClearFlags 
{
    // Matched to D3DCLEAR_* values.
    enum Base
    {
        Target          = 1,
        Depth           = 2,
        Stencil         = 4
    };

}; // End Namespace : cgClearFlags

namespace cgHardwareType
{
    enum Base
    {
        Generic = 0,
        NVIDIA,
        AMD
    };

}; // End Namespace : cgHardwareType
    
namespace cgShaderModel
{
    enum Base
    {
        SM_2_0 = 0,
        SM_2_a,
        SM_2_b,
        SM_3_0,
        SM_4_0,
        SM_4_1,
        SM_5_0
    };

}; // End Namespace : cgShaderModel

namespace cgColorChannel
{
    // Matched to D3DCOLORWRITEENABLE_* & D3D11_COLOR_WRITE_ENABLE values.
    enum Base
    {
        None           = 0,
        Red            = (1L<<0),
        Green          = (1L<<1),
        Blue           = (1L<<2),
        Alpha          = (1L<<3),
        RedGreen       = (Red | Green),
		RedBlue        = (Red | Blue),
		RedAlpha       = (Red | Alpha),
		GreenBlue      = (Green | Blue),
		GreenAlpha     = (Green | Alpha),
		BlueAlpha      = (Blue | Alpha),
		RedGreenBlue   = (Red | Green | Blue),
		RedGreenAlpha  = (Red | Green | Alpha),
		GreenBlueAlpha = (Green | Blue | Alpha),
        All            = (Red | Green | Blue | Alpha)
    };

}; // End Namespace : cgColorChannel

namespace cgFilterMethod
{
    // Matched to D3DTEXTUREFILTERTYPE values.
    enum Base
    {
        None        = 0,
        Point       = 1,
        Linear      = 2,
        Anisotropic = 3
    };

}; // End Namespace : cgFilterMethod

namespace cgAddressingMode
{
    // Matched to D3DTEXTUREADDRESS & D3D11_TEXTURE_ADDRESS_MODE values.
    enum Base
    {
        Wrap        = 1,
        Mirror      = 2,
        Clamp       = 3,
        Border      = 4,
        MirrorOnce  = 5
    };

}; // End Namespace : cgAddressingMode

namespace cgBlendMode
{
    // Matched to D3DBLEND & D3D11_BLEND values (with notes).
    enum Base
    {
        Zero            = 1,
        One             = 2,
        SrcColor        = 3,
        InvSrcColor     = 4,
        SrcAlpha        = 5,
        InvSrcAlpha     = 6,
        DestAlpha       = 7,
        InvDestAlpha    = 8,
        DestColor       = 9,
        InvDestColor    = 10,
        SrcAlphaSat     = 11,
        BothSrcAlpha    = 12,   // D3D9 Only
        BothInvSrcAlpha = 13,   // D3D9 Only
        BlendFactor     = 14,
        InvBlendFactor  = 15,
        Src1Color       = 16,   // D3D9 Equivalent = SRCCOLOR2
        InvSrc1Color    = 17,   // D3D9 Equivalent = INVSRCCOLOR2
        Src1Alpha       = 18,   // D3D11 Only
        InvSrc1Alpha    = 19    // D3D11 Only
    };

}; // End Namespace : cgBlendMode

namespace cgBlendOperation
{
    // Matched to D3DBLENDOP & D3D11_BLEND_OP values.
    enum Base
    {
        Add             = 1,
        Subtract        = 2,
        RevSubtract     = 3,
        Min             = 4,
        Max             = 5
    };

}; // End Namespace : cgBlendOperation

namespace cgComparisonFunction
{
    // Matched to D3DCMPFUNC & D3D11_COMPARISON_FUNC values.
    enum Base
    {
        Never           = 1,
        Less            = 2,
        Equal           = 3,
        LessEqual       = 4,
        Greater         = 5,
        NotEqual        = 6,
        GreaterEqual    = 7,
        Always          = 8
    };

}; // End Namespace : cgComparisonFunction

namespace cgCullMode
{
    // Matched to D3D11_CULL_MODE values.
    enum Base
    {
        None            = 1,
        Front           = 2,
        Back            = 3
    };

} // End Namespace : cgCullMode

namespace cgFillMode
{
    // Matched to D3DFILLMODE & D3D11_FILL_MODE values (with notes).
    enum Base
    {
        Point           = 1,    // D3D9 Only
        Wireframe       = 2,
        Solid           = 3
    };

} // End Namespace : cgFillMode

namespace cgReflectionMode
{
    enum Base
    {
        None            = 0,
        Planar          = 1,
        EnvironmentMap  = 2
    };

}; // End Namespace : cgReflectionMode

namespace cgStencilOperation
{
    // Matched to D3DSTENCILOP & D3D11_STENCIL_OP values.
    enum Base
    {
        Keep            = 1,
        Zero            = 2,
        Replace         = 3,
        IncrSat         = 4,
        DecrSat         = 5,
        Invert          = 6,
        Incr            = 7,
        Decr            = 8
    };

} // End Namespace : cgStencilOperation

namespace cgTechniqueResult
{
    enum Base
    {
        Continue        = 0,
        Complete        = 1,
        Abort           = 2
    };

} // End Namespace : cgTechniqueResult

namespace cgDepthType
{
    enum Base
    {
	    None                     = 0,
	    LinearDistance           = 1,
	    LinearZ                  = 2,
	    NonLinearZ               = 3,
	    LinearDistance_Packed    = 101,
	    LinearZ_Packed           = 102,
	    NonLinearZ_Packed        = 103,
        LinearZ_Normal_Packed    = 200
    };

} // End Namespace : cgDepthType

namespace cgImageOperation
{
    enum Base
    {
        // Utility
        None                        = -2,
        All                         = -1,

	    // Basic operations
	    SetColorRGB                 = 0,
	    SetColorRGBA                = 1,
	    SetColorA                   = 2,
	    CopyAlpha                   = 3,
	    CopyRGBA                    = 4,
	    CopyRGB                     = 5,
	    CopyRGBSetAlpha             = 6,
	    ColorScaleRGBA              = 7,
	    ColorScaleRGB               = 8,
	    AddRGBA                     = 9,
	    AddRGB                      = 10,
	    AddRGBSetAlpha              = 11,
	    ColorScaleAddRGBA           = 12,
	    ColorScaleAddRGB            = 13,
	    ColorScaleAddRGBSetAlpha    = 14,
	    TextureScaleRGBA            = 15,
	    TextureScaleRGB             = 16,
	    TextureScaleA               = 17,
	    InverseRGBA                 = 18,
	    InverseRGB                  = 19,
	    UnsignedToSignedRGBA        = 20,
	    UnsignedToSignedRGB         = 21,
	    SignedToUnsignedRGBA        = 22,
	    SignedToUnsignedRGB         = 23,
	    Grayscale                   = 24,
	    Luminance                   = 25,
	    Sepia                       = 26,
	    RGBEtoRGB                   = 27,
        RGBAtoRGBL                  = 28,
        TestAlpha                   = 29,
	    TestAlphaDS                 = 30,
	    AlphaToRGBA                 = 31,
	    BilateralResample           = 32,
	    BilateralBlur               = 33,
	    Blur                        = 34,

		ScaleUserColorRGBA          = 40,
		ScaleUserColorRGB           = 41,
		ScaleUserColorA             = 42,

	    // Color controls
	    Exposure                    = 100,
	    Saturation                  = 101,
	    LevelsIn                    = 102,
	    LevelsOut                   = 103,
	    Gamma                       = 104,
	    ColorRemap                  = 105,
	    TintAndBrightness           = 106,        
	    Grain                       = 107,
	    Vignette                    = 108,
	    BlueShift                   = 109,

	    // Depth operations
	    LinearZToDistance           = 200,
	    DistanceToLinearZ           = 201,
	    LinearZToDepth              = 202,
	    DistanceToDepth             = 203,

	    // Down sampling operations
	    DownSampleAverage           = 300,
	    DownSampleMinimum           = 301,
	    DownSampleMaximum           = 302,
	    DownSampleMinMax            = 303,
    };
    static const cgInt ColorControlsStart = (cgInt)Exposure;
    static const cgInt ColorControlsEnd   = (cgInt)BlueShift;
    static const cgInt ColorOnlyOperationsStart = (cgInt)SetColorRGB;
    static const cgInt ColorOnlyOperationsEnd   = (cgInt)SetColorA;
    typedef std::vector<Base> Array;

} // End Namespace : cgImageOperation

namespace cgAlphaWeightMethod
{
    enum Base
    {
        None               = 0,
        SetZero            = 1,
        SetOne             = 2,
        Sample             = 3,
        Center             = 4,
        SampleCenter       = 5,
        BinarySample       = 6,
        BinaryCenter       = 7,
        BinarySampleCenter = 8,
        SamplePow2         = 9,
        SamplePow3         = 10,
        SamplePow4         = 11,
        AvgSample          = 12,
        AvgSampleCenter    = 13,
        CmpGreater         = 14,
        CmpGreaterEqual    = 15,
        CmpLess            = 16,
        CmpLessEqual       = 17,
        CmpMin             = 18,
        CmpMax             = 19,
        Bilateral          = 20,
        Bilateral_Pow2     = 21,
        Bilateral_Pow3     = 22
    };

} // End Namespace : cgAlphaWeightMethod

namespace cgDiscontinuityTestMethod
{
    enum Base
    {
        Depth              = 0,
        Normal             = 1,
        DepthAndNormal     = 2,
        Plane              = 3,
        PlaneAndNormal     = 4,
        DepthFast          = 5,
        NormalFast         = 6,
        DepthAndNormalFast = 7
    };

} // End namespace cgDiscontinuityTestMethod

namespace cgShadowMethod
{
	enum Base
	{
		// Depth Types
		Depth                 = (1L << 1),     // linear or non-linear Z
		Variance              = (1L << 2),     // depth and depth ^ depth
		Exponential           = (1L << 3),     // depth ^ n
		Reflective            = (1L << 4),     // reflective shadow map (depth/color/normal)

		// Precision (Desired)
		Bits16                = (1L << 9),
		Bits24                = (1L << 10),
		Bits32                = (1L << 11),
		ExtentsBits16         = (1L << 12),

		// Precomputation
		Precomputed           = (1L << 13),    // Shadows precomputed to separate buffer
		PrecomputedAlpha      = (1L << 14),    // Shadows precomputed to alpha channel of lighting buffer

		// Hardware features 
		Hardware              = (1L << 16),    // Hardware acceleration
		Compare               = (1L << 17),    // Automatic depth comparisons
		Gather                = (1L << 18),    // Automatic 2x2 depth gathering
		RAWZ                  = (1L << 19),    // Uses a RAWZ depth-stencil type

		// Modifiers
		DepthReads            = (1L << 20),    // Requires readable depths
		SoftShadows           = (1L << 21),    // Compute soft shadows 
		ContactHardening      = (1L << 22),    // Compute contact hardening 
		EdgeMask              = (1L << 23),    // Use an edge-only mask for optimization
		DepthExtentsMask      = (1L << 24),    // Use a depth extents mask for optimization
		Translucency          = (1L << 25),    // Support colored shadows for translucent objects
		Jitter                = (1L << 26),    // Jitter the center sample
		Rotate                = (1L << 27),    // Rotate secondary samples
		Manual2x2             = (1L << 28),    // Take manual 2x2 samples and filter
		NormalOffset          = (1L << 29),    // Use surface normal to offset coord computation
		BoxFilter             = (1L << 30),    // Should we use a box shaped filter or random taps?

		////////////////////////////
		// Presets
		////////////////////////////
		PCF                    = (Depth),
		PCSS                   = (Depth | SoftShadows | ContactHardening),
		VSM                    = (Variance),
		ESM                    = (Exponential),
		EVSM                   = (Exponential | Variance),
		RSM                    = (Reflective),
		AllMethods             = (PCF | PCSS | VSM | ESM | EVSM | RSM)
	};

} // End Namespace : cgShadowMethod

namespace cgShadowType
{
	enum Base
	{
		PCF  = cgShadowMethod::PCF,
		PCSS = cgShadowMethod::PCSS,
		VSM  = cgShadowMethod::VSM,
		ESM  = cgShadowMethod::ESM,
		EVSM = cgShadowMethod::EVSM,
		RSM  = cgShadowMethod::RSM,
	};

} // End Namespace : cgShadowType

// ToDo: 6767 - Rename type to method and method to flags (THIS WAS JOE'S COMMENT). 

namespace cgIndirectLightingMethod
{
	enum Base
	{
		// Types
		RadianceHints            = (1L << 1),  
		PropagationVolumes       = (1L << 2),  
		ScreenSpaceGI            = (1L << 3),  

		// Modifiers
		Occlusion2D              = (1L << 10),    // Depth buffer occlusion
		Occlusion3D              = (1L << 11),    // Volume occlusion
		VTF                      = (1L << 12),    // Use vertex texture fetch
		R2VB                     = (1L << 13),    // Use render to vertex buffer
	};

} // End Namespace : cgIndirectLightingMethod

namespace cgFogModel
{
    enum Base
    {
        None               = 0,
        Exponential        = 1,
        ExponentialSquared = 2,
        Linear             = 3
    };

} // End Namespace : cgFogModel

namespace cgAntialiasingMethod
{
    enum Base
    {    
        None            = 0,
        FXAA            = 1,
        FXAA_T2x        = 2,
    };

} // End Namespace : cgAntialiasingMethod

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgViewport
{
    // Binary compatibility with D3DVIEWPORT9
    union
    {
        // Portability Warning: Anonymous struct for non POD - C++ Extension (MSVC++)
        struct { cgPoint position; };
        struct { cgInt32 x, y; };
    };
    union
    {
        // Portability Warning: Anonymous struct for non POD - C++ Extension (MSVC++)
        struct { cgSize  size; };
        struct { cgInt32 width, height; };
    };
    cgFloat     minimumZ;
    cgFloat     maximumZ;

}; // End Struct : cgViewport

struct CGE_API cgSamplerStateDesc
{
    cgInt           magnificationFilter;    // cgFilterMethod
    cgInt           minificationFilter;     // cgFilterMethod
    cgInt           mipmapFilter;           // cgFilterMethod
    cgInt           addressU;               // cgAddressingMode
    cgInt           addressV;               // cgAddressingMode
    cgInt           addressW;               // cgAddressingMode
    cgFloat         mipmapLODBias;
    cgUInt          maximumAnisotropy;
    cgInt           comparisonFunction;     // cgComparisonFunction -- D3D11 Only
    cgColorValue    borderColor;
    cgFloat         minimumMipmapLOD;       // D3D9 Equivalent = D3DSAMP_MAXMIPLEVEL
    cgFloat         maximumMipmapLOD;       // D3D11 Only

    // Constructor (system defaults)
    cgSamplerStateDesc() :
        magnificationFilter( cgFilterMethod::Point ), minificationFilter( cgFilterMethod::Point ), mipmapFilter( cgFilterMethod::Point ),
        addressU( cgAddressingMode::Clamp ), addressV( cgAddressingMode::Clamp ), addressW( cgAddressingMode::Clamp ),
        mipmapLODBias( 0.0f ), maximumAnisotropy( 16 ), comparisonFunction( cgComparisonFunction::Never ), 
        borderColor( 0, 0, 0, 0 ), minimumMipmapLOD( 0.0f ), maximumMipmapLOD( 3.402823466e+38f ) {}

    // Inline operators
    inline bool operator==( const cgSamplerStateDesc & b ) const
    {
        return (memcmp( this, &b, sizeof(cgSamplerStateDesc) ) == 0);
    }
    inline bool operator!=( const cgSamplerStateDesc & b ) const
    {
        return (memcmp( this, &b, sizeof(cgSamplerStateDesc) ) != 0);
    }
};

struct cgTargetBlendStateDesc
{
    bool            blendEnable;
    cgInt           sourceBlend;                // cgBlendMode
    cgInt           destinationBlend;           // cgBlendMode
    cgInt           blendOperation;             // cgBlendOperation
    bool            separateAlphaBlendEnable;   // Handled manually by render driver in D3D11
    cgInt           sourceBlendAlpha;           // cgBlendMode
    cgInt           destinationBlendAlpha;      // cgBlendMode
    cgInt           blendOperationAlpha;        // cgBlendOperation
    cgInt           renderTargetWriteMask;      // cgColorChannel

    // Constructor (system defaults)
    cgTargetBlendStateDesc() :
        blendEnable( false ), sourceBlend( cgBlendMode::One ), destinationBlend( cgBlendMode::Zero ),
        blendOperation( cgBlendOperation::Add ), separateAlphaBlendEnable( false ), sourceBlendAlpha( cgBlendMode::One ), 
        destinationBlendAlpha( cgBlendMode::Zero ), blendOperationAlpha( cgBlendOperation::Add ), renderTargetWriteMask( cgColorChannel::All ) {}

};

struct cgBlendStateDesc
{
    bool                    alphaToCoverageEnable;  // D3D11 Only
    bool                    independentBlendEnable; // D3D11 Only
    cgTargetBlendStateDesc  renderTarget[8];

    // Constructor (system defaults)
    cgBlendStateDesc() :
        alphaToCoverageEnable( false ), independentBlendEnable( false ) {}
};

struct cgRasterizerStateDesc
{
    cgInt           fillMode;                   // cgFillMode
    cgInt           cullMode;                   // cgCullMode
    bool            frontCounterClockwise;      // Handled manually by render driver in D3D9
    cgInt           depthBias;
    cgFloat         depthBiasClamp;             // D3D11 Only
    cgFloat         slopeScaledDepthBias;
    bool            depthClipEnable;            // D3D11 Only
    bool            scissorTestEnable;
    bool            multisampleEnable;          // D3D9 Equivalent = D3DRS_MULTISAMPLEANTIALIAS
    bool            antialiasedLineEnable;      // D3D9 Equivalent = D3DRS_ANTIALIASEDLINEENABLE 

    // Constructor (system defaults)
    cgRasterizerStateDesc() :
        fillMode( cgFillMode::Solid ), cullMode( cgCullMode::Back ), frontCounterClockwise( false ),
        depthBias( 0 ), depthBiasClamp( 0.0f ), slopeScaledDepthBias( 0.0f ), depthClipEnable( true ),
        scissorTestEnable( false ), multisampleEnable( true ), antialiasedLineEnable( false ) {}
};

struct cgDepthStencilOpDesc
{
    cgInt                   stencilFailOperation;       // cgStencilOperation
    cgInt                   stencilDepthFailOperation;  // cgStencilOperation
    cgInt                   stencilPassOperation;       // cgStencilOperation
    cgInt                   stencilFunction;            // cgComparisonFunction

    // Constructor (system defaults)
    cgDepthStencilOpDesc() :
        stencilFailOperation( cgStencilOperation::Keep ), stencilDepthFailOperation( cgStencilOperation::Keep ),
        stencilPassOperation( cgStencilOperation::Keep ), stencilFunction( cgComparisonFunction::Always ) {}
};

struct cgDepthStencilStateDesc
{
    bool                    depthEnable;
    bool                    depthWriteEnable;   // D3D11 Equivalent = D3D11_DEPTH_WRITE_MASK
    cgInt                   depthFunction;      // cgComparisonFunction
    bool                    stencilEnable;
    cgUInt8                 stencilReadMask;
    cgUInt8                 stencilWriteMask;
    cgDepthStencilOpDesc    frontFace;          // cgDepthStencilOpDesc
    cgDepthStencilOpDesc    backFace;           // cgDepthStencilOpDesc

    // Constructor (system defaults)
    cgDepthStencilStateDesc() :
        depthEnable( true ), depthWriteEnable( true ), depthFunction( cgComparisonFunction::LessEqual ),
        stencilEnable( false ), stencilReadMask( 0xFF ), stencilWriteMask( 0xFF ) {}
};

// Used to supply material terms such as lighting reflectance values -- similar to D3DMATERIAL9
struct CGE_API cgMaterialTerms
{
    cgColorValue    diffuse;
    cgColorValue    ambient;
    cgColorValue    specular;
    cgColorValue    emissive;
    cgFloat         gloss;
    cgFloat         emissiveHDRScale;

    // Extended parameters
    cgFloat         diffuseOpacityMapStrength;
    cgFloat         specularOpacityMapStrength;
    cgFloat         metalnessAmount;
    cgFloat         metalnessDiffuse;
    cgFloat         metalnessSpecular;
    cgFloat         rimExponent;
    cgFloat         rimIntensity;
    
    // Reflections
    cgFloat         reflectionIntensity;
    cgFloat         reflectionBumpiness;
    cgFloat         reflectionMipLevel;

    // Fresnel
    cgFloat         fresnelDiffuse;
    cgFloat         fresnelSpecular;
    cgFloat         fresnelReflection;
    cgFloat         fresnelOpacity;
    cgFloat         fresnelExponent;

    // Custom constructor
    cgMaterialTerms() :
        diffuse                     ( cgColorValue( 1.0f, 1.0f, 1.0f, 1.0f ) ),
        ambient                     ( cgColorValue( 0.0f, 0.0f, 0.0f, 1.0f ) ),
        specular                    ( cgColorValue( 0.0f, 0.0f, 0.0f, 1.0f ) ),
        emissive                    ( cgColorValue( 0.0f, 0.0f, 0.0f, 1.0f ) ),
        emissiveHDRScale            ( 1.0f ),
        gloss                       ( 0.0f ),
        diffuseOpacityMapStrength   ( 1.0f ),
        specularOpacityMapStrength  ( 1.0f ),
        metalnessAmount             ( 0.0f ),
        metalnessDiffuse            ( 1.0f ),
        metalnessSpecular           ( 1.0f ),
        rimExponent                 ( 1.0f ),
        rimIntensity                ( 0.0f ),
        reflectionBumpiness         ( 1.0f ),
        reflectionIntensity         ( 1.0f ),
        reflectionMipLevel          ( 0.0f ),
        fresnelDiffuse              ( 0.0f ),
        fresnelSpecular             ( 0.0f ),
        fresnelReflection           ( 0.0f ),
        fresnelOpacity              ( 0.0f ),
        fresnelExponent             ( 5.0f )
        {}

}; // End Struct : cgMaterialTerms

struct cgBlurOpDesc
{
    // Typedefs
    typedef std::vector<cgBlurOpDesc> Array;

    // Members
    cgInt32                     passCount;
    cgInt32                     pixelRadiusV;
	cgInt32                     pixelRadiusH;
    cgFloat                     distanceFactorV;
	cgFloat                     distanceFactorH;
    cgFloat                     worldRadius;
    cgAlphaWeightMethod::Base   inputAlpha;
    cgAlphaWeightMethod::Base   outputAlpha;

    // Constructors
    cgBlurOpDesc() :
        passCount(0), pixelRadiusV(0), pixelRadiusH(0), distanceFactorV(0), distanceFactorH(0), worldRadius(0),
        inputAlpha(cgAlphaWeightMethod::None), outputAlpha(cgAlphaWeightMethod::None) {}
    cgBlurOpDesc( cgInt32 _passCount, cgInt32 _pixelRadius, cgFloat _distanceFactor ) :
        passCount(_passCount), pixelRadiusV(_pixelRadius), pixelRadiusH(_pixelRadius), distanceFactorV(_distanceFactor), distanceFactorH(_distanceFactor), worldRadius(0),
        inputAlpha(cgAlphaWeightMethod::None), outputAlpha(cgAlphaWeightMethod::None) {}
};

#endif // !_CGE_CGRENDERINGTYPES_H_