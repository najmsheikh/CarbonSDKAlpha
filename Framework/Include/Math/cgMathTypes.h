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
// Name : cgMathTypes.h                                                      //
//                                                                           //
// Desc : Common header file that defines various math types and common enum //
//        results.                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMATHTYPES_H_ )
#define _CGE_CGMATHTYPES_H_

//-----------------------------------------------------------------------------
// cgMathTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgTransform.h>
#include <Math/cgVector.h>
#include <Math/cgMatrix.h>
#include <Math/cgPlane.h>
#include <Math/cgQuaternion.h>

//-----------------------------------------------------------------------------
// Math Defines
//-----------------------------------------------------------------------------
#define CGE_PI              ((cgReal)3.141592654)
#define CGE_TWO_PI          ((cgReal)6.283185307)
#define CGE_RECIP_PI        ((cgReal)0.318309886)
#define CGE_EPSILON_1M      ((cgReal)1)             // 1 meter (m) tolerance (1)
#define CGE_EPSILON_1CM     ((cgReal)0.01)          // 1 centimeter (cm) tolerance (1e-2)
#define CGE_EPSILON_1MM     ((cgReal)0.001)         // 1 millimeter (mm) tolerance (1e-3)
#define CGE_EPSILON_1UM     ((cgReal)0.000001)      // 1 micrometer (um) tolerance (1e-6)
#define CGE_EPSILON_1NM     ((cgReal)0.000000001)   // 1 nanometer (nm) tolerance (1e-9)
#define CGE_EPSILON_SINGLE  FLT_EPSILON             // Single precision epsilon
#define CGE_EPSILON_DOUBLE  DBL_EPSILON             // Double precision epsilon
#define CGE_EPSILON         1e-5f                   // Generic 0-1 range epsilon (useful for normal tests, divide by 0 tests, etc.)

//-----------------------------------------------------------------------------
// Math Macros
//-----------------------------------------------------------------------------
#define CGEToRadian( x ) ((x) * (CGE_PI / ((cgReal)180.0)))
#define CGEToDegree( x ) ((x) * (((cgReal)180.0) / CGE_PI))

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
// Resulting classifications for volume queries such as AABB vs. Frustum, etc.
namespace cgVolumeQuery
{
    enum Class
    {
        Inside = 0,
        Outside,
        Intersect
    };

} // End Namespace : cgVolumeQuery

namespace cgPlaneQuery
{
    enum Class
    {
        Front = 0,
        Back,
        On,
        Spanning
    };

} // End Namespace : cgPlaneQuery

// Used to identify specific planes of volumes such as the sides of an AABB or Frustum
namespace cgVolumePlane
{
    enum Side
    {
        Left    = 0,
        Right,
        Top,
        Bottom,
        Near,
        Far
    };

}; // End Namespace : cgVolumePlane

// Used to identify specific volume boundary points such as the 8 points of an AABB or Frustum
namespace cgVolumeGeometry
{
    enum Point
    {
        RightBottomFar  = 0,
        RightBottomNear,
        RightTopFar,
        RightTopNear,
        LeftBottomFar,
        LeftBottomNear,
        LeftTopFar,
        LeftTopNear
    };

}; // End Namespace : cgVolumeGeometry

#endif // !_CGE_CGMATHTYPES_H_