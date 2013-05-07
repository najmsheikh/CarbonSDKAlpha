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
// Name : cgResourceTypes.h                                                  //
//                                                                           //
// Desc : Common system file that defines various resource types and common  //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRESOURCETYPES_H_ )
#define _CGE_CGRESOURCETYPES_H_

//-----------------------------------------------------------------------------
// cgResourceTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Rendering/cgRenderingTypes.h> // ToDo: 6767 - Texturepool requires cgSamplerStateDesc
#include <System/cgPropertyContainer.h>

//-----------------------------------------------------------------------------
// Common Global Macros
//-----------------------------------------------------------------------------
#define CGEFOURCC(ch0, ch1, ch2, ch3) \
    ((cgUInt32)(cgByte)(ch0) | ((cgUInt32)(cgByte)(ch1) << 8) |   \
    ((cgUInt32)(cgByte)(ch2) << 16) | ((cgUInt32)(cgByte)(ch3) << 24 ))

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgBufferFormat
{
    enum Base
    {
        Unknown                      = 0,       // D3DFMT_UNKNOWN               -- D3D9, D3D10, D3D11
        R32G32B32A32_Typeless        = 1,       // 
        R32G32B32A32_Float           = 2,       // D3DFMT_A32B32G32R32F         -- D3D9, D3D10, D3D11
        R32G32B32A32_UInt            = 3,       //                              -- D3D10, D3D11
        R32G32B32A32_SInt            = 4,       //                              -- D3D10, D3D11
        R32G32B32_Typeless           = 5,       //                              -- D3D10, D3D11
        R32G32B32_Float              = 6,       //                              -- D3D10, D3D11
        R32G32B32_UInt               = 7,       //                              -- D3D10, D3D11
        R32G32B32_SInt               = 8,       //                              -- D3D10, D3D11
        R16G16B16A16_Typeless        = 9,       //                              -- D3D10, D3D11
        R16G16B16A16_Float           = 10,      // D3DFMT_A16B16G16R16F         -- D3D9, D3D10, D3D11
        R16G16B16A16                 = 11,      // D3DFMT_A16B16G16R16          -- D3D9, D3D10, D3D11
        R16G16B16A16_UInt            = 12,      //                              -- D3D10, D3D11
        R16G16B16A16_Signed          = 13,      // D3DFMT_Q16W16V16U16          -- D3D9, D3D10, D3D11
        R16G16B16A16_SInt            = 14,      //                              -- D3D10, D3D11
        R32G32_Typeless              = 15,      //                              -- D3D10, D3D11
        R32G32_Float                 = 16,      // D3DFMT_G32R32F               -- D3D9, D3D10, D3D11
        R32G32_UInt                  = 17,      //                              -- D3D10, D3D11
        R32G32_SInt                  = 18,      //                              -- D3D10, D3D11
        R32G8X24_Typeless            = 19,      //                              -- D3D10, D3D11
        D32_Float_S8X24_UInt         = 20,      //                              -- D3D10, D3D11
        R32_Float_X8X24_Typeless     = 21,      //                              -- D3D10, D3D11
        X32_Typeless_G8X24_UInt      = 22,      //                              -- D3D10, D3D11
        R10G10B10A2_Typeless         = 23,      //                              -- D3D10, D3D11
        R10G10B10A2                  = 24,      // D3DFMT_A2B10G10R10           -- D3D9, D3D10, D3D11
        R10G10B10A2_UInt             = 25,      //                              -- D3D10, D3D11
        R11G11B10_Float              = 26,      //                              -- D3D10, D3D11
        R8G8B8A8_Typeless            = 27,      //                              -- D3D10, D3D11
        R8G8B8A8                     = 28,      // D3DFMT_A8B8G8R8              -- D3D9, D3D10, D3D11
        R8G8B8A8_SRGB                = 29,      // D3DFMT_A8B8G8R8              -- D3D9, D3D10, D3D11
        R8G8B8A8_UInt                = 30,      //                              -- D3D10, D3D11
        R8G8B8A8_Signed              = 31,      // D3DFMT_Q8W8V8U8              -- D3D9, D3D10, D3D11
        R8G8B8A8_SInt                = 32,      //                              -- D3D10, D3D11
        R16G16_Typeless              = 33,      //                              -- D3D10, D3D11
        R16G16_Float                 = 34,      // D3DFMT_G16R16F               -- D3D9, D3D10, D3D11
        R16G16                       = 35,      // D3DFMT_G16R16                -- D3D9, D3D10, D3D11
        R16G16_UInt                  = 36,      //                              -- D3D10, D3D11
        R16G16_Signed                = 37,      // D3DFMT_V16U16                -- D3D9, D3D10, D3D11
        R16G16_SInt                  = 38,      //                              -- D3D10, D3D11
        R32_Typeless                 = 39,      //                              -- D3D10, D3D11
        D32_Float                    = 40,      // D3DFMT_D32_LOCKABLE          -- D3D9, D3D10, D3D11
        R32_Float                    = 41,      // D3DFMT_R32F                  -- D3D9, D3D10, D3D11
        R32_UInt                     = 42,      // D3DFMT_INDEX32               -- D3D9, D3D10, D3D11
        Index32                      = R32_UInt,
        R32_SInt                     = 43,      //                              -- D3D10, D3D11
        R24G8_Typeless               = 44,      //                              -- D3D10, D3D11
        D24_UNorm_S8_UInt            = 45,      // D3DFMT_S8D24                 -- D3D9, D3D10, D3D11
        D24S8                        = D24_UNorm_S8_UInt,
        R24_UNorm_X8_Typeless        = 46,      //                              -- D3D10, D3D11
        X24_Typeless_G8_UInt         = 47,      //                              -- D3D10, D3D11
        R8G8_Typeless                = 48,      //                              -- D3D10, D3D11
        R8G8                         = 49,      //                              -- D3D10, D3D11
        R8G8_UInt                    = 50,      //                              -- D3D10, D3D11
        R8G8_Signed                  = 51,      // D3DFMT_V8U8                  -- D3D9, D3D10, D3D11
        R8G8_SInt                    = 52,      //                              -- D3D10, D3D11
        R16_Typeless                 = 53,      //                              -- D3D10, D3D11
        R16_Float                    = 54,      // D3DFMT_R16F                  -- D3D9, D3D10, D3D11
        D16                          = 55,      // D3DFMT_D16                   -- D3D9, D3D10, D3D11
        R16                          = 56,      // D3DFMT_L16                   -- D3D9, D3D10, D3D11 -- Note: Use .r swizzle in shader to duplicate red to other components to get D3D9 behavior.
        R16_UInt                     = 57,      // D3DFMT_INDEX16               -- D3D9, D3D10, D3D11
        Index16                      = R16_UInt,
        R16_Signed                   = 58,      //                              -- D3D10, D3D11
        R16_SInt                     = 59,      //                              -- D3D10, D3D11
        R8_Typeless                  = 60,      //                              -- D3D10, D3D11
        R8                           = 61,      // D3DFMT_L8                    -- D3D9, D3D10, D3D11 -- Note: Use .r swizzle in shader to duplicate red to other components to get D3D9 behavior.
        R8_UInt                      = 62,      //                              -- D3D10, D3D11
        R8_Signed                    = 63,      //                              -- D3D10, D3D11
        R8_SInt                      = 64,      //                              -- D3D10, D3D11
        A8                           = 65,      // D3DFMT_A8                    -- D3D9, D3D10, D3D11
        R1                           = 66,      //                              -- D3D10, D3D11
        R9G9B9E5_SharedExp           = 67,      //                              -- D3D10, D3D11
        R8G8_B8G8                    = 68,      // D3DFMT_G8R8_G8B8             -- D3D9, D3D10, D3D11 -- Note: in DX9 the data was scaled up by 255.0f, but this can be handled in shader code.
        G8R8_G8B8                    = 69,      // D3DFMT_R8G8_B8G8             -- D3D9, D3D10, D3D11 -- Note: in DX9 the data was scaled up by 255.0f, but this can be handled in shader code.
        BC1_Typeless                 = 70,      //                              -- D3D10, D3D11
        BC1                          = 71,      // D3DFMT_DXT1                  -- D3D9, D3D10, D3D11
        BC1_SRGB                     = 72,      // D3DFMT_DXT1                  -- D3D9, D3D10, D3D11
        BC2_Typeless                 = 73,      //                              -- D3D10, D3D11
        BC2                          = 74,      // D3DFMT_DXT2 & D3DFMT_DXT3    -- D3D9, D3D10, D3D11
        BC2_SRGB                     = 75,      // D3DFMT_DXT2 & D3DFMT_DXT3    -- D3D9, D3D10, D3D11
        BC3_Typeless                 = 76,      //                              -- D3D10, D3D11
        BC3                          = 77,      // D3DFMT_DXT4 & D3DFMT_DXT5    -- D3D9, D3D10, D3D11
        BC3_SRGB                     = 78,      // D3DFMT_DXT4 & D3DFMT_DXT5    -- D3D9, D3D10, D3D11
        BC4_Typeless                 = 79,      //                              -- D3D10, D3D11
        BC4                          = 80,      //                              -- D3D10, D3D11
        BC4_Signed                   = 81,      //                              -- D3D10, D3D11
        BC5_Typeless                 = 82,      //                              -- D3D10, D3D11
        BC5                          = 83,      //                              -- D3D10, D3D11
        BC5_Signed                   = 84,      //                              -- D3D10, D3D11
        B5G6R5                       = 85,      // D3DFMT_R5G6B5                -- D3D9
        B5G5R5A1                     = 86,      // D3DFMT_A1R5G5B5              -- D3D9
        B8G8R8A8                     = 87,      // D3DFMT_A8R8G8B8              -- D3D9, D3D11
        B8G8R8X8                     = 88,      // D3DFMT_X8R8G8B8              -- D3D9
        //R10G10B10_XR_Bias_A2_UNorm   = 89,    //                              -- D3D11
        B8G8R8A8_Typeless            = 90,      //                              -- D3D11
        B8G8R8A8_SRGB                = 91,      //                              -- D3D11
        B8G8R8X8_Typeless            = 92,      //                              -- D3D11
        B8G8R8X8_SRGB                = 93,      //                              -- D3D11
        BC6H_Typeless                = 94,      //                              -- D3D11
        BC6H_UF16                    = 95,      //                              -- D3D11
        BC6H_SF16                    = 96,      //                              -- D3D11
        BC7_Typeless                 = 97,      //                              -- D3D11
        BC7                          = 98,      //                              -- D3D11
        BC7_SRGB                     = 99,      //                              -- D3D11

        // Custom four CC (ATI fetch 4, INTZ, RAWZ)
        DF16 = CGEFOURCC('D','F','1','6'),       //                              -- D3D9
        DF24 = CGEFOURCC('D','F','2','4'),       //                              -- D3D9
        RAWZ = CGEFOURCC('R','A','W','Z'),       //                              -- D3D9
        INTZ = CGEFOURCC('I','N','T','Z'),       //                              -- D3D9

        B8G8R8                  = 0xFF000001,   // D3DFMT_R8G8B8                -- D3D9 & Internal Carbon Format (cgImage)
        B5G5R5X1                = 0xFF000002,   // D3DFMT_X1R5G5B5              -- D3D9 & Internal Carbon Format (cgImage)
        D24_Float_S8_UInt       = 0xFF000100,   // D3DFMT_D24FS8                -- D3D9
        D24FS8                  = D24_Float_S8_UInt,
        D24_UNorm_X8_Typeless   = 0xFF000101,   // D3DFMT_D24X8                 -- D3D9
        D24X8                   = D24_UNorm_X8_Typeless
    };

    /*enum Base
    {
        Unknown              =  0,
        R8G8B8               = 20,
        A8R8G8B8             = 21,
        X8R8G8B8             = 22,
        R5G6B5               = 23,
        X1R5G5B5             = 24,
        A1R5G5B5             = 25,
        A4R4G4B4             = 26,
        R3G3B2               = 27,
        A8                   = 28,
        A8R3G3B2             = 29,
        X4R4G4B4             = 30,
        A2B10G10R10          = 31,
        A8B8G8R8             = 32,
        X8B8G8R8             = 33,
        G16R16               = 34,
        A2R10G10B10          = 35,
        A16B16G16R16         = 36,

        A8P8                 = 40,
        P8                   = 41,

        L8                   = 50,
        A8L8                 = 51,
        A4L4                 = 52,

        V8U8                 = 60,
        L6V5U5               = 61,
        X8L8V8U8             = 62,
        Q8W8V8U8             = 63,
        V16U16               = 64,
        A2W10V10U10          = 67,

        UYVY                 = CGEFOURCC('U', 'Y', 'V', 'Y'),
        R8G8_B8G8            = CGEFOURCC('R', 'G', 'B', 'G'),
        YUY2                 = CGEFOURCC('Y', 'U', 'Y', '2'),
        G8R8_G8B8            = CGEFOURCC('G', 'R', 'G', 'B'),
        DXT1                 = CGEFOURCC('D', 'X', 'T', '1'),
        DXT2                 = CGEFOURCC('D', 'X', 'T', '2'),
        DXT3                 = CGEFOURCC('D', 'X', 'T', '3'),
        DXT4                 = CGEFOURCC('D', 'X', 'T', '4'),
        DXT5                 = CGEFOURCC('D', 'X', 'T', '5'),

        D16_Lockable         = 70,
        D32                  = 71,
        D15S1                = 73,
        D24S8                = 75,
        D24X8                = 77,
        D24X4S4              = 79,
        D16                  = 80,

        D32F_Lockable        = 82,
        D24FS8               = 83,

        D32_Lockable         = 84,
        S8_Lockable          = 85,

        L16                  = 81,

        VertexData           = 100,
        Index16              = 101,
        Index32              = 102,

        Q16W16V16U16         = 110,

        Multi2_ARGB8         = CGEFOURCC('M','E','T','1'),

        // Floating point surface formats

        // s10e5 formats (16-bits per channel)
        R16F                 = 111,
        G16R16F              = 112,
        A16B16G16R16F        = 113,

        // IEEE s23e8 formats (32-bits per channel)
        R32F                 = 114,
        G32R32F              = 115,
        A32B32G32R32F        = 116,

        CxV8U8               = 117,

        // Monochrome 1 bit per pixel format
        A1                   = 118,


        // Binary format indicating that the data has no inherent type
        BinaryBuffer         = 199,

        // Custom four CC (nvidia hardware shadow mapping)
        DF16                 = CGEFOURCC('D','F','1','6'),
        DF24                 = CGEFOURCC('D','F','2','4')
    };*/

}; // End Namespace : cgBufferFormat

namespace cgBufferFormatCaps
{
    enum Base
    {
        CanWrite            = 0x1,
        CanSample           = 0x2,
        CanAutoGenMipMaps   = 0x4,
        CanCompare          = 0x8,
        CanGather           = 0x10,
        CanGatherCompare    = 0x20,
        CanLinearMagnify    = 0x40,
        CanLinearMinify     = 0x80,
        CanLinearFilter     = 0xC0   // Mask for Magnify or Minify
    };

} // End Namespace : cgBufferFormatCaps

namespace cgMemoryPool
{
    enum Base
    {
        Default     = 0,
        Managed     = 1,
        SystemMem   = 2,
        Scratch     = 3
    };

} // End Namespace : cgMemoryPool

namespace cgResourceType
{
    enum Base
    {
        None = 0,
        Texture,
        Material,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        RenderTarget,
        DepthStencilTarget,
        AnimationSet,
        Mesh,
        AudioBuffer,
        Script,
        SurfaceShader,
        VertexShader,
        PixelShader,
        SamplerState,
        DepthStencilState,
        RasterizerState,
        BlendState
    };

}; // End Namespace : cgResourceType

namespace cgResourceFlags
{
    enum Base
    {
        AlwaysResident  = 0x1,          // This resource handle is always resident in memory, even when nothing (but the resource manager) is referencing it.
        DeferredLoad    = 0x2,          // Resource handle should be created, but internal data should not be loaded until needed
        ForceNew        = 0x4           // Irrespective of whether or not another resource with matching name / data exists, always create a new one.
    };

}; // End Namespace : cgResourceFlags

namespace cgMaterialType
{
    enum Base
    {
        Standard        = 1,
        LandscapeLayer  = 2
    };

} // End Namespace : cgMaterialType

// Describes the various types of buffer resources
namespace cgBufferType
{
    enum Base
    {
        None = 0,                       // No buffer type specified.
        Texture1D,                      // One dimensional texture map.
        Texture2D,                      // Two dimensional texture map.
        Texture3D,                      // Three dimensional / volume texture map.
        TextureCube,                    // Cube texture map
        Surface,                        // Plain two dimensional surface.
        RenderTarget,                   // Render target surface / texture.
        RenderTargetCube,               // Cube map render target texture.
        DepthStencil,                   // depth stencil surface / texture.
        ShadowMap,                      // Custom depth stencil texture
        Video,                          // Video surface / texture.
        DeviceBackBuffer,               // Reserved for use by system
    };

}; // End Namespace : cgBufferType

// Values match D3D9 'D3DUSAGE_*' flags.
namespace cgBufferUsage
{
    enum Base
    {
        WriteOnly           = 0x00000008,
        SoftwareProcessing  = 0x00000010,
        Dynamic             = 0x00000200
    };

}; // End Namespace : cgBufferUsage

namespace cgMultiSampleType
{
    enum Base
    {
        None            =  0,
        NonMaskable     =  1,
        TwoSamples      =  2,
        ThreeSamples    =  3,
        FourSamples     =  4,
        FiveSamples     =  5,
        SixSamples      =  6,
        SevenSamples    =  7,
        EightSamples    =  8,
        NineSamples     =  9,
        TenSamples      = 10,
        ElevenSamples   = 11,
        TwelveSamples   = 12,
        ThirteenSamples = 13,
        FourteenSamples = 14,
        FifteenSamples  = 15,
        SixteenSamples  = 16
    };

}; // End Namespace : cgMultiSampleType

namespace cgMeshStatus
{
    enum Base
    {
        NotPrepared,
        Preparing,
        Prepared
    };

}; // End Namespace : cgMeshPrepareStatus

namespace cgMeshDrawMode
{
    enum Base
    {
        Automatic,
        Simple
    };

}; // End Namespace : cgMeshDrawMode

namespace cgMeshCreateOrigin
{
    enum Base
    {
        Bottom,
        Center,
        Top
    };

}; // End Namespace : cgMeshCreateOrigin

// Modes for automatic resampling of surfaces / targets.
namespace cgResampleMethod
{
    enum Base
	{
		Average = 0,
		Extents = 1
	
    };

}; // End Namespace : cgResampleMethod

namespace cgScaleMode
{
    enum Base
    {
        Absolute = 0,
        Relative = 1
    };

}; // End Namespace : cgScaleMode

namespace cgConstantType
{
    enum Base
    {
        Float       = 0xFFFFFFF0,   // -16
        Int         = 0xFFFFFFF1,   // -15
        UInt        = 0xFFFFFFF2,   // -14
        Bool        = 0xFFFFFFF3,   // -13
        Double      = 0xFFFFFFF4,   // -12
        Unresolved  = 0xFFFFFFFF    // -1
    };

}; // End Namespace : cgConstantType

// Values match D3D9 'D3DLOCK_*' flags.
namespace cgLockFlags
{
    enum Base
    {
        ReadOnly        = 0x00000010,
        NoSystemLock    = 0x00000800,
        NoOverwrite     = 0x00001000,
        Discard         = 0x00002000,
        DoNotWait       = 0x00004000,
        NoDirtyUpdate   = 0x00008000,
        WriteOnly       = 0xFF000000 // Not a D3D9 flag.

    };

} // End Namespace : cgLockFlags

// Flags for cgBufferFormatEnum::GetBestFormat()
namespace cgFormatSearchFlags
{
    enum Base
    {
        OneChannel              = 0x1,
        TwoChannels             = 0x2,
        FourChannels            = 0x8,
        RequireAlpha            = 0x10,
        RequireStencil          = 0x20,
        PreferCompressed        = 0x40,

        AllowPaddingChannels    = 0x100,

        HalfPrecisionFloat      = 0x1000,
        FullPrecisionFloat      = 0x2000,
        FloatingPoint           = 0xF000,
    };

}; // End Namespace : cgFormatSearchFlags

/// The basic pool resource types
namespace cgTexturePoolResourceType
{
    enum Base
    {
        Unknown = 0,
        Cached  = 1,
        Shared  = 2
    };

}; // End Namespace : cgPoolResourceType

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct CGE_API cgImageInfo
{
    cgUInt32                width;
    cgUInt32                height;
    cgUInt32                depth;
    cgUInt32                mipLevels;
    cgBufferFormat::Base    format;
    cgBufferType::Base      type;
    cgMemoryPool::Base      pool;
    cgMultiSampleType::Base multiSampleType;
    cgUInt32                multiSampleQuality;
    bool                    autoGenerateMipmaps;
    bool                    autoDetectFormat;
    bool                    dynamic;
    
    // Constructor
    cgImageInfo( ) :
        width(0), height(0), depth(1), mipLevels(1),
        format(cgBufferFormat::Unknown), type(cgBufferType::Texture2D),
        pool(cgMemoryPool::Default), multiSampleType(cgMultiSampleType::None),
        multiSampleQuality(0), autoGenerateMipmaps(false), autoDetectFormat(false),
        dynamic(false) {}

}; // End Struct : cgImageInfo

// Distinct from cgSamplerStateDesc; this describes an individual sampler as
// declared by a surface shader script.
struct cgSamplerDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgSamplerDesc, Array )

    // Public Variables
    cgString                parentNamespace;
    cgString                name;
    cgString                typeName;
    cgInt32                 registerIndex;
    cgPropertyContainer     parameters;

}; // End Struct : cgSamplerDesc

struct cgSamplerBlockDesc
{
    // Public Variables
    cgString                parentNamespace;
    cgString                name;
    cgSamplerDesc::Array    samplers;
    cgPropertyContainer     parameters;

}; // End Struct : cgSamplerBlockDesc

struct cgConstantDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgConstantDesc, Array )

    // Public Variables
    cgString                name;
    cgString                fullTypeName;
    cgString                primitiveTypeName;
    bool                    isUDT;
    bool                    isArray;
    cgInt32                 typeId;
    cgUInt32Array           arrayDimensions;
    cgUInt32                elements;
    cgUInt32                elementLength;
    cgUInt32                rows;
    cgUInt32                columns;
    cgUInt32                offset;
    cgUInt32                alignment;
    cgUInt32                totalLength;
    cgByteArray             defaultValue;
    cgPropertyContainer     parameters;

    // Register linkage
    cgUInt32                registerOffset;     // Offset with respect parent.
    cgUInt8                 registerComponent;  // Starting register component (x=0, y=1, z=2 or w=3)
    bool                    registerPacking;    // Was packing used for this constant?
    cgUInt32                registerCount;      // Number of registers consumed.
};

struct cgConstantTypeDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgConstantTypeDesc, Array )

    // Public Variables
    cgString                parentNamespace;
    cgString                name;
    cgUInt32                length;
    cgUInt32                alignment;
    cgPropertyContainer     parameters;
    cgConstantDesc::Array   constants;

    // Register Linkage
    cgUInt32                registerCount;
};

struct cgConstantBufferDesc : public cgConstantTypeDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgConstantBufferDesc, Array )

    // Public Variables
    cgUInt32                bufferRegister;
    cgUInt32                globalOffset;
    bool                    bindVS;
    bool                    bindPS;
};

struct cgShaderCallParameterDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgShaderCallParameterDesc, Array )

    // Public Variables
    cgString    name;
    cgString    typeName;
    cgString    modifier;
    bool        scriptParameter;
};

struct cgShaderCallFunctionDesc
{
    // Public Typedefs
    CGE_VECTOR_DECLARE( cgShaderCallFunctionDesc, Array )

        // Public Variables
    cgString    parentNamespace;
    cgString    name;
    cgString    declaration;
    cgShaderCallParameterDesc::Array parameters;
    cgString    callingParams;
    cgString    returnType;
    bool        requiresReturn;
};

//-----------------------------------------------------------------------------
//  Name : cgTexturePoolResourceDesc (Struct)
/// <summary>
/// Describes the properties of an individual texture pool resource.
/// </summary>
//-----------------------------------------------------------------------------
struct CGE_API cgTexturePoolResourceDesc
{
    // Typedefs
    CGE_VECTOR_DECLARE( cgTexturePoolResourceDesc, Array )

    // Members
    cgTexturePoolResourceType::Base type;           // The type of pool resource this is
    cgImageInfo                     bufferDesc;     // Basic image description
    cgUInt32                        channelCount;   // Number of channels this resource offers
    cgSamplerStateDesc              samplerStates;  // How should this resource be sampled?
    cgInt32                         userData;       // User custom data for this resource

    // Constructor to provide defaults
    cgTexturePoolResourceDesc() :
        type( cgTexturePoolResourceType::Unknown ), channelCount( 0 ), userData( -1 ) { }

    cgTexturePoolResourceDesc( cgBufferFormat::Base _format ) :
        type( cgTexturePoolResourceType::Unknown ), channelCount( 0 ), userData( -1 )
    {
        bufferDesc.format = _format;
    }

}; // End Struct : cgTexturePoolResourceDesc

//-----------------------------------------------------------------------------
//  Name : cgTexturePoolDepthFormatDesc (Struct)
/// <summary>
/// A description structure for managing depth-stencil formats within the pool.
/// </summary>
//-----------------------------------------------------------------------------
struct CGE_API cgTexturePoolDepthFormatDesc : public cgTexturePoolResourceDesc
{
    // Members
    cgUInt8     depthBits;
    cgUInt8     stencilBits;
    cgUInt32    capabilities;
    
    // Constructor to provide defaults
    cgTexturePoolDepthFormatDesc( cgBufferFormat::Base _format, cgUInt8 _depthBits, cgUInt8 _stencilBits ) :
        cgTexturePoolResourceDesc( _format ),
        depthBits( _depthBits ), stencilBits( _stencilBits ), 
        capabilities(0) {}

    cgTexturePoolDepthFormatDesc() :
        depthBits( 0 ), stencilBits( 0 ), capabilities(0) {}

}; // End Struct : cgTexturePoolDepthFormatDesc

//-----------------------------------------------------------------------------
//  Name : cgShaderIdentifier (Class)
/// <summary>
/// Unique identifier data for any compiled vertex / pixel shader that allows
/// us to rapidly select an individual shader permutation from either an
/// existing compiled shader cache file, or from an already managed in-memory 
/// resource.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgShaderIdentifier
{
public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    struct SourceFileInfo
    {
        /// <summary>Name and relative path of the source file that contributed to this shader.</summary>
        cgString    name;       
        /// <summary>Hash of the above source file.</summary>
        cgUInt32    hash[5];
    };
    CGE_VECTOR_DECLARE( SourceFileInfo, SourceArray )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgShaderIdentifier() {}
    cgShaderIdentifier( const cgShaderIdentifier * init )
    {
        if ( init )
        {
            sourceFiles = init->sourceFiles;
            shaderIdentifier = init->shaderIdentifier;
            parameterData = init->parameterData;
            memcpy( inputSignatureHash, init->inputSignatureHash, 5 * sizeof(cgUInt32) );
        
        } // End if clone
        else
        {
            memset( inputSignatureHash, 0, 5 * sizeof(cgUInt32) );

        } // End if !clone
    
    } // End Constructor

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgString generateHashName() const;

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline cgInt compare(const cgShaderIdentifier& key2) const
    {
        // Check simple count parameters first (fastest first).
        size_t sourceSize = sourceFiles.size();
        cgInt difference = ((cgInt)key2.sourceFiles.size() - (cgInt)sourceSize);
        if ( difference ) return difference;
        size_t paramSize  = parameterData.size();
        difference = ((cgInt)key2.parameterData.size() - (cgInt)paramSize);
        if ( difference ) return difference;

        // Compare shader identifier string (if any).
        difference = shaderIdentifier.compare( key2.shaderIdentifier );
        if ( difference ) return difference;

        // Check parameter contents
        if ( paramSize )
        {
            difference = memcmp( &key2.parameterData[0], &parameterData[0], paramSize * sizeof(cgUInt32) );
            if ( difference ) return difference;
        
        } // End if has parameters

        // Finally, check source hashes.
        for ( size_t i = 0; i < sourceSize; ++i )
        {
            difference = memcmp( key2.sourceFiles[i].hash, sourceFiles[i].hash, 20 );
            if ( difference ) return difference;
        
        } // Next Source

        // Identifiers match (for the purpose of comparison)
        return 0;
    }

    //-------------------------------------------------------------------------
    // Public Inline Operators
    //-------------------------------------------------------------------------
    inline bool operator < ( const cgShaderIdentifier & key2 ) const
    {
        return (compare( key2 ) < 0);
    }

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>List of source files that contributed to this shader. The first entry is always the top-level source file or physical source code.</summary>
    SourceArray     sourceFiles;
    /// <summary>String name / identifier for this shader. This is likely the entry point shader function name when using high level shading languages like HLSL. Otherwise, it will be user defined (but should still be unique).</summary>
    cgString        shaderIdentifier;
    /// <summary>List of 32 bit values representing the shader parameters specified (if compiled via a surface shader script).</summary>
    cgUInt32Array   parameterData;
    /// <summary>20 byte SHA1 hash that represents the unique signature of the shader inputs.</summary>
    cgUInt32        inputSignatureHash[5];

}; // End Class cgShaderIdentifier

//-----------------------------------------------------------------------------
// Common Global Containers
//-----------------------------------------------------------------------------
class cgTexturePoolResource;
CGE_VECTOR_DECLARE( cgTexturePoolResource *, cgTexturePoolResourceArray )

#endif // !_CGE_CGRESOURCETYPES_H_