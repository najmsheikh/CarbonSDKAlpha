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
// Name : cgLandscapeTypes.h                                                 //
//                                                                           //
// Desc : Common system file that defines various landscape types and common //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLANDSCAPETYPES_H_ )
#define _CGE_CGLANDSCAPETYPES_H_

//-----------------------------------------------------------------------------
// cgLandscapeTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgHeightMap;

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
/// <summary>Various flags / properties that dictate how the landscape behaves.</summary>
namespace cgLandscapeFlags
{
    enum Base
    {
        /// <summary>No flags apply.</summary>
        None                = 0x0,
        /// <summary>Heightmap should be loaded and remain resident (i.e. for cases when we need access to the data).</summary>
        HeightMapResident   = 0x1,
        /// <summary>Terrain should be considered dynamic (combination flag, includes HeightMapResident bit).</summary>
        Dynamic             = 0x3,
        /// <summary>LOD system should not take into account the height of the camera.</summary>
        LODIgnoreY          = 0x4,
    
    };

}; // End Namespace : cgLandscapeFlags

namespace cgLandscapeRenderMethod
{
    enum Base
    {
        TerrainDepthFill   = 0,
        TerrainShadowFill,
        TerrainGBufferFill,
        TerrainGBufferPost
    };

} // End Namespace : cgLandscapeRenderMethod

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgLandscapeImportParams
{
    /// <summary>The heightmap from which to take the terrain displacement data.</summary>
    cgHeightMap           * heightMap;
    /// <summary>The overall size, in world space units, that describes the area in which the landscape should exist.</summary>
    cgVector3               dimensions;
    /// <summary>A vector describing how the terrain should be offset with <0,0,0> positioning the landscape's upper left corner at the origin.</summary>
    cgVector3               offset;
    /// <summary>Initialization flags.</summary>
    cgUInt32                initFlags;
    /// <summary>The size (expressed as a number of quads) of each created block, i.e. 64x64 describes a grid with 65x65 vertices.</summary>
    cgSize                  blockGrid;
    /// <summary>Allows the caller to describe which blocks should be created (1 = create, 0 = discard).</summary>
    cgByteArray             blockMask;
    /// <summary>Used to define the resolution of the blend maps stored at each block.</summary>
    cgSize                  blockBlendMapSize;
    /// <summary>Allows the caller to provide an initial map describing layer / texture assignments for each block.</summary>
    cgInt32Array            layerInit;
    /// <summary>Vertex color map (same layout as height map).</summary>
    cgUInt32Array           colorMap;  

}; // End Struct : cgLandscapeImportParams

struct cgLandscapePaintParams
{
    bool      erase;
    cgInt32   strength;
    cgFloat   innerRadius;
    cgFloat   outerRadius;
    bool      enableHeightEffect;
    cgFloat   minimumHeight;
    cgFloat   maximumHeight;
    cgFloat   heightAttenuation;
    bool      enableSlopeEffect;
    cgVector3 slopeAxis;
    cgFloat   slopeScale;
    cgFloat   slopeBias;
    bool      invertSlope;

}; // End Struct : cgLandscapePaintParams

#endif // !_CGE_CGLANDSCAPETYPES_H_