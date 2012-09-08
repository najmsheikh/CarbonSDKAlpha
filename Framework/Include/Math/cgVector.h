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
// Name : cgVector.h                                                         //
//                                                                           //
// Desc : Header file providing core vector math type type definitions and   //
//        inline functionality.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGVECTOR_H_ )
#define _CGE_CGVECTOR_H_

//-----------------------------------------------------------------------------
// cgVector Header Includes
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
class cgMatrix;
class cgVector3;
class cgVector2;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVector4 (Class)
/// <summary>
/// Storage for three dimensional coordinates.
/// </summary>
//-----------------------------------------------------------------------------
class cgVector4
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgVector4 operator * ( cgFloat s, const cgVector4 & v );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgVector4() {};
    cgVector4( const cgFloat * v ) : 
        x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}
    cgVector4( const cgHalf * v ) :
        x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}
    cgVector4( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _w ) :
        x(_x), y(_y), z(_z), w(_w) {}

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    inline static cgFloat cgVector4::dot( const cgVector4 & v1, const cgVector4 & v2 )
    {
        return D3DXVec4Dot( (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2 );
    }
    inline static cgVector4 * cgVector4::cross( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2, const cgVector4 & v3 )
    {
        return (cgVector4*)D3DXVec4Cross( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2, (D3DXVECTOR4*)&v3 );
    }
    inline static cgFloat cgVector4::length( const cgVector4 & v )
    {
        return D3DXVec4Length( (D3DXVECTOR4*)&v );
    }
    inline static cgFloat cgVector4::lengthSq( const cgVector4 & v )
    {
        return D3DXVec4LengthSq( (D3DXVECTOR4*)&v );
    }
    inline static cgVector4 * cgVector4::normalize( cgVector4 & out, const cgVector4 & v )
    {
        return (cgVector4*)D3DXVec4Normalize( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v );
    }
    inline static cgVector4 * cgVector4::transform( cgVector4 & out, const cgVector4 & v, const cgMatrix & m )
    {
        return (cgVector4*)D3DXVec4Transform( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector4 * cgVector4::add( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2 )
    {
        return (cgVector4*)D3DXVec4Add( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2 );
    }
    inline static cgVector4 * cgVector4::subtract( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2 )
    {
        return (cgVector4*)D3DXVec4Subtract( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2 );
    }
    inline static cgVector4 * cgVector4::minimize( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2 )
    {
        return (cgVector4*)D3DXVec4Minimize( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2 );
    }
    inline static cgVector4 * cgVector4::maximize( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2 )
    {
        return (cgVector4*)D3DXVec4Maximize( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2 );
    }
    inline static cgVector4 * cgVector4::scale( cgVector4 & out, const cgVector4 & v, cgFloat s )
    {
        return (cgVector4*)D3DXVec4Scale( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v, s );
    }
    inline static cgVector4 * cgVector4::lerp( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2, cgFloat s )
    {
        return (cgVector4*)D3DXVec4Lerp( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2, s );
    }
    inline static cgVector4 * cgVector4::hermite( cgVector4 & out, const cgVector4 & v1, const cgVector4 & t1, const cgVector4 & v2, const cgVector4 & t2, cgFloat s )
    {
        return (cgVector4*)D3DXVec4Hermite( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&t1, (D3DXVECTOR4*)&v2, (D3DXVECTOR4*)&t2, s );
    }
    inline static cgVector4 * cgVector4::catmullRom( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2, const cgVector4 & v3, const cgVector4 & v4, cgFloat s )
    {
        return (cgVector4*)D3DXVec4CatmullRom( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2, (D3DXVECTOR4*)&v3, (D3DXVECTOR4*)&v4, s );
    }
    inline static cgVector4 * cgVector4::baryCentric( cgVector4 & out, const cgVector4 & v1, const cgVector4 & v2, const cgVector4 & v3, cgFloat f, cgFloat g )
    {
        return (cgVector4*)D3DXVec4BaryCentric( (D3DXVECTOR4*)&out, (D3DXVECTOR4*)&v1, (D3DXVECTOR4*)&v2, (D3DXVECTOR4*)&v3, f, g );
    }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline cgVector4 operator + ( const cgVector4 & v ) const
    {
        return cgVector4( x + v.x, y + v.y, z + v.z, w + v.w );
    }

    inline cgVector4 operator - ( const cgVector4 & v ) const
    {
        return cgVector4( x - v.x, y - v.y, z - v.z, w - v.w );
    }

    inline cgVector4 operator * ( cgFloat s ) const
    {
        return cgVector4( x * s, y * s, z * s, w * s );
    }

    inline cgVector4 operator / ( cgFloat s ) const
    {
        return cgVector4( x / s, y / s, z / s, w / s );
    }

    inline cgVector4 & operator += ( const cgVector4 & v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    inline cgVector4 & operator -= ( const cgVector4 & v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    inline cgVector4 & operator *= ( cgFloat s )
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    inline cgVector4 & operator /= ( cgFloat s )
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }
    inline cgVector4 operator + () const
    {
        return *this;
    }

    inline cgVector4 operator - () const
    {
        return cgVector4( -x, -y, -z, -w );
    }

    inline bool operator == ( const cgVector4 & v ) const
    {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }

    inline bool operator != ( const cgVector4 & v ) const
    {
        return x != v.x || y != v.y || z != v.z || w != v.w;
    }

    inline operator cgFloat* ()
    {
        return &x;
    }

    inline operator const cgFloat* () const
    {
        return &x;
    }

    // Component Access
    inline const cgVector3    & xyz () const { return (const cgVector3&)x; }
    inline const cgVector2    & xy  () const { return (const cgVector2&)x; }
    
    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    cgFloat x, y, z, w;
};

//-----------------------------------------------------------------------------
//  Name : cgVector3 (Class)
/// <summary>
/// Storage for three dimensional coordinates.
/// </summary>
//-----------------------------------------------------------------------------
class cgVector3
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgVector3 operator * ( cgFloat s, const cgVector3 & v );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgVector3() {};
    cgVector3( const cgFloat * v ) : 
        x(v[0]), y(v[1]), z(v[2]) {}
    cgVector3( const cgHalf * v ) :
        x(v[0]), y(v[1]), z(v[2]) {}
    cgVector3( cgFloat _x, cgFloat _y, cgFloat _z ) :
        x(_x), y(_y), z(_z) {}

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    inline static cgFloat cgVector3::dot( const cgVector3 & v1, const cgVector3 & v2 )
    {
        return D3DXVec3Dot( (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgVector3 * cgVector3::cross( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXVec3Cross( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgFloat cgVector3::length( const cgVector3 & v )
    {
        return D3DXVec3Length( (D3DXVECTOR3*)&v );
    }
    inline static cgFloat cgVector3::lengthSq( const cgVector3 & v )
    {
        return D3DXVec3LengthSq( (D3DXVECTOR3*)&v );
    }
    inline static cgVector3 * cgVector3::normalize( cgVector3 & out, const cgVector3 & v )
    {
        return (cgVector3*)D3DXVec3Normalize( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v );
    }
    inline static cgVector3 * cgVector3::transformCoord( cgVector3 & out, const cgVector3 & v, const cgMatrix & m )
    {
        return (cgVector3*)D3DXVec3TransformCoord( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector3 * cgVector3::transformNormal( cgVector3 & out, const cgVector3 & v, const cgMatrix & m )
    {
        return (cgVector3*)D3DXVec3TransformNormal( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector4 * cgVector3::transform( cgVector4 & out, const cgVector3 & v, const cgMatrix & m )
    {
        return (cgVector4*)D3DXVec3Transform( (D3DXVECTOR4*)&out, (D3DXVECTOR3*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector3 * cgVector3::add( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXVec3Add( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgVector3 * cgVector3::subtract( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXVec3Subtract( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgVector3 * cgVector3::minimize( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXVec3Minimize( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgVector3 * cgVector3::maximize( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2 )
    {
        return (cgVector3*)D3DXVec3Maximize( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2 );
    }
    inline static cgVector3 * cgVector3::scale( cgVector3 & out, const cgVector3 & v, cgFloat s )
    {
        return (cgVector3*)D3DXVec3Scale( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v, s );
    }
    inline static cgVector3 * cgVector3::lerp( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2, cgFloat s )
    {
        return (cgVector3*)D3DXVec3Lerp( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2, s );
    }
    inline static cgVector3 * cgVector3::hermite( cgVector3 & out, const cgVector3 & v1, const cgVector3 & t1, const cgVector3 & v2, const cgVector3 & t2, cgFloat s )
    {
        return (cgVector3*)D3DXVec3Hermite( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&t1, (D3DXVECTOR3*)&v2, (D3DXVECTOR3*)&t2, s );
    }
    inline static cgVector3 * cgVector3::catmullRom( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2, const cgVector3 & v3, const cgVector3 & v4, cgFloat s )
    {
        return (cgVector3*)D3DXVec3CatmullRom( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2, (D3DXVECTOR3*)&v3, (D3DXVECTOR3*)&v4, s );
    }
    inline static cgVector3 * cgVector3::baryCentric( cgVector3 & out, const cgVector3 & v1, const cgVector3 & v2, const cgVector3 & v3, cgFloat f, cgFloat g )
    {
        return (cgVector3*)D3DXVec3BaryCentric( (D3DXVECTOR3*)&out, (D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2, (D3DXVECTOR3*)&v3, f, g );
    }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline cgVector3 operator + ( const cgVector3 & v ) const
    {
        return cgVector3( x + v.x, y + v.y, z + v.z );
    }

    inline cgVector3 operator - ( const cgVector3 & v ) const
    {
        return cgVector3( x - v.x, y - v.y, z - v.z );
    }

    inline cgVector3 operator * ( cgFloat s ) const
    {
        return cgVector3( x * s, y * s, z * s );
    }

    inline cgVector3 operator / ( cgFloat s ) const
    {
        return cgVector3( x / s, y / s, z / s );
    }

    inline cgVector3 & operator += ( const cgVector3 & v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    inline cgVector3 & operator -= ( const cgVector3 & v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    inline cgVector3 & operator *= ( cgFloat s )
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    inline cgVector3 & operator /= ( cgFloat s )
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    inline cgVector3 operator + () const
    {
        return *this;
    }

    inline cgVector3 operator - () const
    {
        return cgVector3( -x, -y, -z );
    }

    inline bool operator == ( const cgVector3 & v ) const
    {
        return x == v.x && y == v.y && z == v.z;
    }

    inline bool operator != ( const cgVector3 & v ) const
    {
        return x != v.x || y != v.y || z != v.z;
    }

    inline operator cgFloat* ()
    {
        return &x;
    }

    inline operator const cgFloat* () const
    {
        return &x;
    }

    // Component Access
    inline const cgVector2    & xy  () const { return (const cgVector2&)x; }

    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    cgFloat x, y, z;
};

//-----------------------------------------------------------------------------
//  Name : cgVector2 (Class)
/// <summary>
/// Storage for two dimensional coordinates.
/// </summary>
//-----------------------------------------------------------------------------
class cgVector2
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgVector2 operator * ( cgFloat s, const cgVector2 & v );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgVector2() {};
    cgVector2( const cgFloat * v ) : 
        x(v[0]), y(v[1]) {}
    cgVector2( const cgHalf * v ) :
        x(v[0]), y(v[1]) {}
    cgVector2( cgFloat _x, cgFloat _y ) :
        x(_x), y(_y) {}

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    inline static cgFloat cgVector2::dot( const cgVector2 & v1, const cgVector2 & v2 )
    {
        return D3DXVec2Dot( (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgFloat cgVector2::ccw( const cgVector2 & v1, const cgVector2 & v2 )
    {
        return D3DXVec2CCW( (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgFloat cgVector2::length( const cgVector2 & v )
    {
        return D3DXVec2Length( (D3DXVECTOR2*)&v );
    }
    inline static cgFloat cgVector2::lengthSq( const cgVector2 & v )
    {
        return D3DXVec2LengthSq( (D3DXVECTOR2*)&v );
    }
    inline static cgVector2 * cgVector2::normalize( cgVector2 & out, const cgVector2 & v )
    {
        return (cgVector2*)D3DXVec2Normalize( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v );
    }
    inline static cgVector2 * cgVector2::transformCoord( cgVector2 & out, const cgVector2 & v, const cgMatrix & m )
    {
        return (cgVector2*)D3DXVec2TransformCoord( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector2 * cgVector2::transformNormal( cgVector2 & out, const cgVector2 & v, const cgMatrix & m )
    {
        return (cgVector2*)D3DXVec2TransformNormal( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector4 * cgVector2::transform( cgVector4 & out, const cgVector2 & v, const cgMatrix & m )
    {
        return (cgVector4*)D3DXVec2Transform( (D3DXVECTOR4*)&out, (D3DXVECTOR2*)&v, (D3DXMATRIX*)&m );
    }
    inline static cgVector2 * cgVector2::add( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2 )
    {
        return (cgVector2*)D3DXVec2Add( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgVector2 * cgVector2::subtract( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2 )
    {
        return (cgVector2*)D3DXVec2Subtract( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgVector2 * cgVector2::minimize( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2 )
    {
        return (cgVector2*)D3DXVec2Minimize( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgVector2 * cgVector2::maximize( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2 )
    {
        return (cgVector2*)D3DXVec2Maximize( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2 );
    }
    inline static cgVector2 * cgVector2::scale( cgVector2 & out, const cgVector2 & v, cgFloat s )
    {
        return (cgVector2*)D3DXVec2Scale( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v, s );
    }
    inline static cgVector2 * cgVector2::lerp( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2, cgFloat s )
    {
        return (cgVector2*)D3DXVec2Lerp( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2, s );
    }
    inline static cgVector2 * cgVector2::hermite( cgVector2 & out, const cgVector2 & v1, const cgVector2 & t1, const cgVector2 & v2, const cgVector2 & t2, cgFloat s )
    {
        return (cgVector2*)D3DXVec2Hermite( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&t1, (D3DXVECTOR2*)&v2, (D3DXVECTOR2*)&t2, s );
    }
    inline static cgVector2 * cgVector2::catmullRom( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2, const cgVector2 & v3, const cgVector2 & v4, cgFloat s )
    {
        return (cgVector2*)D3DXVec2CatmullRom( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2, (D3DXVECTOR2*)&v3, (D3DXVECTOR2*)&v4, s );
    }
    inline static cgVector2 * cgVector2::baryCentric( cgVector2 & out, const cgVector2 & v1, const cgVector2 & v2, const cgVector2 & v3, cgFloat f, cgFloat g )
    {
        return (cgVector2*)D3DXVec2BaryCentric( (D3DXVECTOR2*)&out, (D3DXVECTOR2*)&v1, (D3DXVECTOR2*)&v2, (D3DXVECTOR2*)&v3, f, g );
    }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline cgVector2 operator + ( const cgVector2 & v ) const
    {
        return cgVector2( x + v.x, y + v.y );
    }

    inline cgVector2 operator - ( const cgVector2 & v ) const
    {
        return cgVector2( x - v.x, y - v.y );
    }

    inline cgVector2 operator * ( cgFloat s ) const
    {
        return cgVector2( x * s, y * s );
    }

    inline cgVector2 operator / ( cgFloat s ) const
    {
        return cgVector2( x / s, y / s );
    }

    inline cgVector2 & operator += ( const cgVector2 & v )
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    inline cgVector2 & operator -= ( const cgVector2 & v )
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    inline cgVector2 & operator *= ( cgFloat s )
    {
        x *= s;
        y *= s;
        return *this;
    }

    inline cgVector2 & operator /= ( cgFloat s )
    {
        x /= s;
        y /= s;
        return *this;
    }
    inline cgVector2 operator + () const
    {
        return *this;
    }

    inline cgVector2 operator - () const
    {
        return cgVector2( -x, -y );
    }

    inline bool operator == ( const cgVector2 & v ) const
    {
        return x == v.x && y == v.y;
    }

    inline bool operator != ( const cgVector2 & v ) const
    {
        return x != v.x || y != v.y;
    }

    inline operator cgFloat* ()
    {
        return &x;
    }

    inline operator const cgFloat* () const
    {
        return &x;
    }

    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    cgFloat x, y;
};

//-----------------------------------------------------------------------------
// Global Inline Operators (cgVector4)
//-----------------------------------------------------------------------------
inline cgVector4 operator * ( cgFloat s, const cgVector4 & v )
{
    return cgVector4( v.x * s, v.y * s, v.z * s, v.w * s );
}

//-----------------------------------------------------------------------------
// Global Inline Operators (cgVector3)
//-----------------------------------------------------------------------------
inline cgVector3 operator * ( cgFloat s, const cgVector3 & v )
{
    return cgVector3( v.x * s, v.y * s, v.z * s );
}

//-----------------------------------------------------------------------------
// Global Inline Operators (cgVector2)
//-----------------------------------------------------------------------------
inline cgVector2 operator * ( cgFloat s, const cgVector2 & v )
{
    return cgVector2( v.x * s, v.y * s );
}

#endif // !_CGE_CGVECTOR_H_