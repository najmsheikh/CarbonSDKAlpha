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
// File: cgLeastSquares.h                                                    //
//                                                                           //
// Desc: Utility class for computing least squares planes.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLEASTSQUARES_H_ )
#define _CGE_CGLEASTSQUARES_H_

//-----------------------------------------------------------------------------
// cgLeastSquares Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Name : cgLeastSquaresSums (Class)
/// <summary>Utility class for computing least squares plane.</summary>
//-------------------------------------------------------------------------
class CGE_API cgLeastSquaresSums
{
public:
    //---------------------------------------------------------------------
    // Constructors & Destructors
    //---------------------------------------------------------------------
    cgLeastSquaresSums( ) : 
      x(0), y(0), z(0), 
      xx(0), xy(0), xz(0),
      zy(0), zz(0), samples(0) {}

    //---------------------------------------------------------------------
    // Public Methods
    //---------------------------------------------------------------------
    void clear()
    {
        x  = 0; y  = 0; z = 0;
        xx = 0; xy = 0; xz = 0;
        zy = 0; zz = 0;
        samples   = 0;
    }

    cgPlane computePlane( ) const
    {
        const float denom = -2*x*xz*z + xx*z*z + x*x*zz + samples*(xz*xz - xx*zz);
        
        // Compute the summed plane normal
        cgVector3 normal;
        normal.x = xy*(-z*z + samples*zz) - samples*xz*zy + x*z*zy + (xz*z - x*zz)*y;
        normal.y = 1.0f;
        normal.z = -x*x*zy + samples*(-xz*xy + xx*zy) - xx*z*y + x*(xy*z + xz*y);
        
        // (Optimization) Compute inverse values to allow us to multiply instead of divide
        const float denomRecip   = 1.0f / denom;
        const float samplesRecip = 1.0f / samples;

        // Compute the final plane normal
        normal.x *= denomRecip;
        normal.z *= denomRecip;
        cgVector3::normalize( normal, normal );

        // Compute the average center of the data set
        cgVector3 pos;
        pos.x = x * samplesRecip;
        pos.y = y * samplesRecip;
        pos.z = z * samplesRecip;

        // Construct final plane
        return cgPlane( normal.x, normal.y, normal.z, -cgVector3::dot( pos, normal ) );
    }

    //---------------------------------------------------------------------
    // Public Operators
    //---------------------------------------------------------------------
    cgLeastSquaresSums & operator+=( const cgLeastSquaresSums & sums )
    {
        x += sums.x;
        y += sums.y;
        z += sums.z;
        xx += sums.xx;
        xy += sums.xy;
        xz += sums.xz;
        zy += sums.zy;
        zz += sums.zz;
        samples += sums.samples;
        return *this;
    }

    //---------------------------------------------------------------------
    // Public Members
    //---------------------------------------------------------------------
    cgFloat x, y, z;
    cgFloat xx, xy, xz;
    cgFloat zy, zz;
    cgFloat samples;

}; // End Class cgLeastSquareSums

#endif // !_CGE_CGLEASTSQUARES_H_