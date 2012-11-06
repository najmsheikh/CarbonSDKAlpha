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
// Name : cgQuaternion.h                                                     //
//                                                                           //
// Desc : Header file providing core quaternion math type type definitions   //
//        and inline functionality.                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGQUATERNION_H_ )
#define _CGE_CGQUATERNION_H_

//-----------------------------------------------------------------------------
// cgQuaternion Header Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>
#include <cgBaseTypes.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>	// Win8 SDK required
#include <d3dx9.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgMatrix;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgQuaternion (Class)
/// <summary>
/// Storage for quaternion.
/// </summary>
//-----------------------------------------------------------------------------
class cgQuaternion
{
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    friend cgQuaternion operator * ( cgFloat, const cgQuaternion& );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgQuaternion() {}
    cgQuaternion( const cgFloat * q ) :
        x(q[0]), y(q[1]), z(q[2]), w(q[3]) {}
    cgQuaternion( const cgHalf * q ) :
        x(q[0]), y(q[1]), z(q[2]), w(q[3]) {}
    cgQuaternion( cgFloat _x, cgFloat _y, cgFloat _z, cgFloat _w ) :
        x(_x), y(_y), z(_z), w(_w) {}

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    inline static cgFloat dot( const cgQuaternion & q1, const cgQuaternion & q2 )
    {
        return D3DXQuaternionDot( (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&q2 );
    }
    inline static cgFloat cgQuaternion::length( const cgQuaternion & q )
    {
        return D3DXQuaternionLength( (D3DXQUATERNION*)&q );
    }
    inline static cgFloat cgQuaternion::lengthSq( const cgQuaternion & q )
    {
        return D3DXQuaternionLengthSq( (D3DXQUATERNION*)&q );
    }
    inline static bool cgQuaternion::isIdentity( const cgQuaternion & q )
    {
        return (D3DXQuaternionIsIdentity( (D3DXQUATERNION*)&q ) == TRUE);
    }
    inline static cgQuaternion * cgQuaternion::identity( cgQuaternion & out )
    {
        return (cgQuaternion*)D3DXQuaternionIdentity( (D3DXQUATERNION*)&out );
    }
    inline static cgQuaternion * cgQuaternion::conjugate( cgQuaternion & out, const cgQuaternion & q )
    {
        return (cgQuaternion*)D3DXQuaternionConjugate( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgQuaternion * cgQuaternion::inverse( cgQuaternion & out, const cgQuaternion & q )
    {
        return (cgQuaternion*)D3DXQuaternionInverse( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgQuaternion * cgQuaternion::multiply( cgQuaternion & out, const cgQuaternion & q1, const cgQuaternion & q2 )
    {
        return (cgQuaternion*)D3DXQuaternionMultiply( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&q2 );
    }
    inline static cgQuaternion * cgQuaternion::slerp( cgQuaternion & out, const cgQuaternion & q1, const cgQuaternion & q2, const cgFloat t )
    {
        return (cgQuaternion*)D3DXQuaternionSlerp( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&q2, t );
    }
    inline static cgQuaternion * cgQuaternion::squad( cgQuaternion & out, const cgQuaternion & q1, const cgQuaternion & a, const cgQuaternion & b, const cgQuaternion & c, const cgFloat t )
    {
        return (cgQuaternion*)D3DXQuaternionSquad( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&a, (D3DXQUATERNION*)&b, (D3DXQUATERNION*)&c, t );
    }
    inline static void cgQuaternion::squadSetup( cgQuaternion & outA, cgQuaternion & outB, cgQuaternion & outC, const cgQuaternion & q0, const cgQuaternion & q1, const cgQuaternion & q2, const cgQuaternion & q3 )
    {
        D3DXQuaternionSquadSetup( (D3DXQUATERNION*)&outA, (D3DXQUATERNION*)&outB, (D3DXQUATERNION*)&outC, (D3DXQUATERNION*)&q0, (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&q2, (D3DXQUATERNION*)&q3 );
    }
    inline static cgQuaternion * normalize( cgQuaternion & out, const cgQuaternion & q )
    {
        return (cgQuaternion*)D3DXQuaternionNormalize( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgQuaternion * cgQuaternion::rotationAxis( cgQuaternion & out, const cgVector3 & axis, cgFloat radians )
    {
        return (cgQuaternion*)D3DXQuaternionRotationAxis( (D3DXQUATERNION*)&out, (D3DXVECTOR3*)&axis, radians );
    }
    inline static cgQuaternion * cgQuaternion::rotationYawPitchRoll( cgQuaternion & out, cgFloat yaw, cgFloat pitch, cgFloat roll )
    {
        return (cgQuaternion*)D3DXQuaternionRotationYawPitchRoll( (D3DXQUATERNION*)&out, yaw, pitch, roll );
    }
    inline static cgQuaternion * rotationMatrix( cgQuaternion & out, const cgMatrix & m )
    {
        return (cgQuaternion*)D3DXQuaternionRotationMatrix( (D3DXQUATERNION*)&out, (D3DXMATRIX*)&m );
    }
    inline static void toAxisAngle( const cgQuaternion & q, cgVector3 & outAxis, cgFloat & outAngle )
    {
        return D3DXQuaternionToAxisAngle( (D3DXQUATERNION*)&q, (D3DXVECTOR3*)&outAxis, &outAngle );
    }
    inline static cgQuaternion * ln( cgQuaternion & out, const cgQuaternion & q )
    {
        return (cgQuaternion*)D3DXQuaternionLn( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgQuaternion * exp( cgQuaternion & out, const cgQuaternion & q )
    {
        return (cgQuaternion*)D3DXQuaternionExp( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgQuaternion * cgQuaternion::baryCentric( cgQuaternion & out, const cgQuaternion & q1, const cgQuaternion & q2, const cgQuaternion & q3, cgFloat f, cgFloat g )
    {
        return (cgQuaternion*)D3DXQuaternionBaryCentric( (D3DXQUATERNION*)&out, (D3DXQUATERNION*)&q1, (D3DXQUATERNION*)&q2, (D3DXQUATERNION*)&q3, f, g );
    }
    

    //-------------------------------------------------------------------------
    // Public Operations
    //-------------------------------------------------------------------------
    inline cgQuaternion operator + ( const cgQuaternion & q ) const
    {
        return cgQuaternion( x + q.x, y + q.y, z + q.z, w + q.w );
    }

    inline cgQuaternion operator - ( const cgQuaternion & q ) const
    {
        return cgQuaternion( x - q.x, y - q.y, z - q.z, w - q.w );
    }

    inline cgQuaternion operator * ( const cgQuaternion & q ) const
    {
        cgQuaternion out;
        multiply( out, *this, q );
        return out;
    }

    inline cgQuaternion operator * ( cgFloat s ) const
    {
        return cgQuaternion( x * s, y * s, z * s, w * s );
    }

    inline cgQuaternion operator / ( cgFloat s ) const
    {
        return cgQuaternion( x / s, y / s, z / s, w / s );
    }

    inline cgQuaternion & operator += ( const cgQuaternion & q )
    {
        x += q.x;
        y += q.y;
        z += q.z;
        w += q.w;
        return * this;
    }

    inline cgQuaternion & operator -= ( const cgQuaternion & q )
    {
        x -= q.x;
        y -= q.y;
        z -= q.z;
        w -= q.w;
        return * this;
    }

    inline cgQuaternion & operator *= ( const cgQuaternion & q )
    {
        multiply( *this, *this, q );
        return *this;
    }

    inline cgQuaternion & operator *= ( cgFloat s )
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    inline cgQuaternion & operator /= ( cgFloat s )
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    inline cgQuaternion operator + () const
    {
        return *this;
    }

    inline cgQuaternion operator - () const
    {
        return cgQuaternion( -x, -y, -z, -w );
    }

    inline bool operator == ( const cgQuaternion & q ) const
    {
        return x == q.x && y == q.y && z == q.z && w == q.w;
    }

    inline bool operator != ( const cgQuaternion & q ) const
    {
        return x != q.x || y != q.y || z != q.z || w != q.w;
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
    cgFloat x, y, z, w;
};

//-----------------------------------------------------------------------------
// Global Operators (cgQuaternion)
//-----------------------------------------------------------------------------
inline cgQuaternion operator * ( cgFloat s, const cgQuaternion & q )
{
    return cgQuaternion( q.x * s, q.y * s, q.z * s, q.w * s );
}

#endif // !_CGE_CGQUATERNION_H_