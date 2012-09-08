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
// Name : cgPlane.h                                                          //
//                                                                           //
// Desc : Header file providing core plane math type type definitions and    //
//        inline functionality.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPLANE_H_ )
#define _CGE_CGPLANE_H_

//-----------------------------------------------------------------------------
// cgPlane Header Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>
#include <cgBaseTypes.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <d3dx9.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector4;
class cgVector3;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPlane (Class)
/// <summary>
/// Storage for infinite plane.
/// </summary>
//-----------------------------------------------------------------------------
class cgPlane
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgPlane operator * ( cgFloat, const cgPlane& );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgPlane() {}
    cgPlane( const cgFloat * p ) :
        a(p[0]), b(p[1]), c(p[2]), d(p[3]) {}
    cgPlane( const cgHalf * p ) :
        a(p[0]), b(p[1]), c(p[2]), d(p[3]) {}
    cgPlane( cgFloat _a, cgFloat _b, cgFloat _c, cgFloat _d ) :
        a(_a), b(_b), c(_c), d(_d) {}
    
    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    inline static cgFloat dot( const cgPlane & p, const cgVector4 & v )
    {
        return D3DXPlaneDot( (D3DXPLANE*)&p, (D3DXVECTOR4*)&v );
    }
    inline static cgFloat dotCoord( const cgPlane & p, const cgVector3 & v )
    {
        return D3DXPlaneDotCoord( (D3DXPLANE*)&p, (D3DXVECTOR3*)&v );
    }
    inline static cgFloat dotNormal( const cgPlane & p, const cgVector3 & v )
    {
        return D3DXPlaneDotNormal( (D3DXPLANE*)&p, (D3DXVECTOR3*)&v );
    }
    inline static cgPlane * fromPointNormal( cgPlane & out, const cgVector3 & point, const cgVector3 & normal )
    {
        return (cgPlane*)D3DXPlaneFromPointNormal( (D3DXPLANE*)&out, (D3DXVECTOR3*)&point, (D3DXVECTOR3*)&normal );
    }
    inline static cgPlane * fromPoints( cgPlane & out, const cgVector3 & v1, const cgVector3 & v2, const cgVector3 & v3 )
    {
        return (cgPlane*)D3DXPlaneFromPoints( (D3DXPLANE*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2, (D3DXVECTOR3*)&v3 );
    }
    inline static cgPlane * normalize( cgPlane & out, const cgPlane & p )
    {
        return (cgPlane*)D3DXPlaneNormalize( (D3DXPLANE*)&out, (D3DXPLANE*)&p );
    }
    inline static cgPlane * transform( cgPlane & out, const cgPlane & p, const cgMatrix & m )
    {
        return (cgPlane*)D3DXPlaneTransform( (D3DXPLANE*)&out, (D3DXPLANE*)&p, (D3DXMATRIX*)&m );
    }
    inline static cgPlane * transformArray( cgPlane planesOut[], cgUInt32 outStride, const cgPlane planesIn[], cgUInt32 inStride, const cgMatrix & m, cgUInt32 planeCount  )
    {
        return (cgPlane*)D3DXPlaneTransformArray( (D3DXPLANE*)planesOut, (UINT)outStride, (const D3DXPLANE*)planesIn, (UINT)inStride, (D3DXMATRIX*)&m, (UINT)planeCount );
    }
    inline static cgPlane * scale( cgPlane & out, const cgPlane & p, cgFloat s )
    {
        return (cgPlane*)D3DXPlaneScale( (D3DXPLANE*)&out, (D3DXPLANE*)&p, s );
    }
    inline static cgVector3 * intersectLine( cgVector3 & out, const cgPlane & p, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXPlaneIntersectLine( (D3DXVECTOR3*)&out, (D3DXPLANE*)&p, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline cgPlane operator * ( cgFloat s ) const
    {
        return cgPlane( a * s, b * s, c * s, d * s );
    }

    inline cgPlane operator / ( cgFloat s ) const
    {
        return cgPlane( a / s, b / s, c / s, d / s );
    }

    inline cgPlane & operator *= ( cgFloat s )
    {
        a *= s;
        b *= s;
        c *= s;
        d *= s;
        return *this;
    }

    inline cgPlane & operator /= ( cgFloat s )
    {
        a /= s;
        b /= s;
        c /= s;
        d /= s;
        return *this;
    }

    inline cgPlane operator + () const
    {
        return *this;
    }

    inline cgPlane operator - () const
    {
        return cgPlane( -a, -b, -c, -d );
    }

    inline bool operator == ( const cgPlane & p ) const
    {
        return a == p.a && b == p.b && c == p.c && d == p.d;
    }

    inline bool operator != ( const cgPlane & p ) const
    {
        return a != p.a || b != p.b || c != p.c || d != p.d;
    }

    inline operator cgFloat* ()
    {
        return &a;
    }

    inline operator const cgFloat* () const
    {
        return &a;
    }

    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    cgFloat a, b, c, d;

};

//-----------------------------------------------------------------------------
// Global Inline Operators (cgPlane)
//-----------------------------------------------------------------------------
inline cgPlane operator * ( cgFloat s, const cgPlane & p )
{
    return cgPlane( p.a * s, p.b * s, p.c * s, p.d * s );
}

#endif // !_CGE_CGPLANE_H_