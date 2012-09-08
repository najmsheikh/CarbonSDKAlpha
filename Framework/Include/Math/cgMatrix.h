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
// Name : cgMatrix.h                                                         //
//                                                                           //
// Desc : Header file providing core matrix math type type definitions and   //
//        inline functionality.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMATRIX_H_ )
#define _CGE_CGMATRIX_H_ 

//-----------------------------------------------------------------------------
// cgMatrix Header Includes
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
class cgQuaternion;
class cgPlane;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgMatrix (Class)
/// <summary>
/// Storage for 4x4 matrix.
/// </summary>
//-----------------------------------------------------------------------------
class cgMatrix
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgMatrix operator * ( cgFloat, const cgMatrix& );

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgMatrix() {};
    cgMatrix( const cgFloat * m )
    {
        memcpy(&_11,m,sizeof(cgMatrix));
    }
    cgMatrix( const cgMatrix & m )
    {
        memcpy(&_11,&m,sizeof(cgMatrix));
    }
    cgMatrix( const cgHalf * m )
    {
        // ToDo: 6767
    }
    cgMatrix( cgFloat m11, cgFloat m12, cgFloat m13, cgFloat m14,
              cgFloat m21, cgFloat m22, cgFloat m23, cgFloat m24,
              cgFloat m31, cgFloat m32, cgFloat m33, cgFloat m34,
              cgFloat m41, cgFloat m42, cgFloat m43, cgFloat m44 ) :
        _11(m11), _12(m12), _13(m13), _14(m14),
        _21(m21), _22(m22), _23(m23), _24(m24),
        _31(m31), _32(m32), _33(m33), _34(m34),
        _41(m41), _42(m42), _43(m43), _44(m44) {}

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    inline static bool cgMatrix::isIdentity( const cgMatrix & m )
    {
        return (D3DXMatrixIsIdentity( (D3DXMATRIX*)&m ) == TRUE );
    }
    inline static cgMatrix * cgMatrix::multiply( cgMatrix & out, const cgMatrix & m1, const cgMatrix & m2 )
    {
        return (cgMatrix*)D3DXMatrixMultiply( (D3DXMATRIX*)&out, (D3DXMATRIX*)&m1, (D3DXMATRIX*)&m2 );
    }
    inline static cgMatrix * cgMatrix::inverse( cgMatrix & out, const cgMatrix & m )
    {
        return (cgMatrix*)D3DXMatrixInverse( (D3DXMATRIX*)&out, CG_NULL, (D3DXMATRIX*)&m );
    }
    inline static cgMatrix * cgMatrix::inverse( cgMatrix & out, cgFloat & determinantOut, const cgMatrix & m )
    {
        return (cgMatrix*)D3DXMatrixInverse( (D3DXMATRIX*)&out, &determinantOut, (D3DXMATRIX*)&m );
    }
    inline static cgMatrix * cgMatrix::transpose( cgMatrix & out, const cgMatrix & m )
    {
        return (cgMatrix*)D3DXMatrixTranspose( (D3DXMATRIX*)&out, (D3DXMATRIX*)&m );
    }
    inline static cgMatrix * cgMatrix::identity( cgMatrix & out )
    {
        return (cgMatrix*)D3DXMatrixIdentity( (D3DXMATRIX*)&out );
    }
    inline static cgMatrix * cgMatrix::translation( cgMatrix & out, cgFloat x, cgFloat y, cgFloat z )
    {
        return (cgMatrix*)D3DXMatrixTranslation( (D3DXMATRIX*)&out, x, y, z );
    }
    inline static cgMatrix * cgMatrix::scaling( cgMatrix & out, cgFloat x, cgFloat y, cgFloat z )
    {
        return (cgMatrix*)D3DXMatrixScaling( (D3DXMATRIX*)&out, x, y, z );
    }
    inline static cgMatrix * cgMatrix::rotationAxis( cgMatrix & out, const cgVector3 & axis, cgFloat radians )
    {
        return (cgMatrix*)D3DXMatrixRotationAxis( (D3DXMATRIX*)&out, (D3DXVECTOR3*)&axis, radians );
    }
    inline static cgMatrix * cgMatrix::rotationYawPitchRoll( cgMatrix & out, cgFloat yaw, cgFloat pitch, cgFloat roll )
    {
        return (cgMatrix*)D3DXMatrixRotationYawPitchRoll( (D3DXMATRIX*)&out, yaw, pitch, roll );
    }
    inline static cgMatrix * cgMatrix::rotationX( cgMatrix & out, cgFloat radians )
    {
        return (cgMatrix*)D3DXMatrixRotationX( (D3DXMATRIX*)&out, radians );
    }
    inline static cgMatrix * cgMatrix::rotationY( cgMatrix & out, cgFloat radians )
    {
        return (cgMatrix*)D3DXMatrixRotationY( (D3DXMATRIX*)&out, radians );
    }
    inline static cgMatrix * cgMatrix::rotationZ( cgMatrix & out, cgFloat radians )
    {
        return (cgMatrix*)D3DXMatrixRotationZ( (D3DXMATRIX*)&out, radians );
    }
    inline static cgMatrix * cgMatrix::rotationQuaternion( cgMatrix & out, const cgQuaternion & q )
    {
        return (cgMatrix*)D3DXMatrixRotationQuaternion( (D3DXMATRIX*)&out, (D3DXQUATERNION*)&q );
    }
    inline static cgFloat cgMatrix::determinant( cgMatrix & m )
    {
        return D3DXMatrixDeterminant( (D3DXMATRIX*)&m );
    }
    inline static bool cgMatrix::decompose( cgVector3 & outScale, cgQuaternion & outRotation, cgVector3 & outTranslation, const cgMatrix & m )
    {
        return SUCCEEDED(D3DXMatrixDecompose( (D3DXVECTOR3*)&outScale, (D3DXQUATERNION*)&outRotation, (D3DXVECTOR3*)&outTranslation, (D3DXMATRIX*)&m ));
    }
    inline static cgMatrix * cgMatrix::transformation( cgMatrix & out, const cgVector3 & scalingCenter, const cgQuaternion & scalingRotation, const cgVector3 & scaling, const cgVector3 & rotationCenter, const cgQuaternion & rotation, const cgVector3 & translation )
    {
        return (cgMatrix*)D3DXMatrixTransformation( (D3DXMATRIX*)&out, (D3DXVECTOR3*)&scalingCenter, (D3DXQUATERNION*)&scalingRotation, (D3DXVECTOR3*)&scaling, (D3DXVECTOR3*)&rotationCenter, (D3DXQUATERNION*)&rotation, (D3DXVECTOR3*)&translation );
    }
    inline static cgMatrix * cgMatrix::transformation2D( cgMatrix & out, const cgVector2 & scalingCenter, cgFloat scalingRotation, const cgVector2 & scaling, const cgVector2 & rotationCenter, cgFloat rotation, const cgVector2 & translation )
    {
        return (cgMatrix*)D3DXMatrixTransformation2D( (D3DXMATRIX*)&out, (D3DXVECTOR2*)&scalingCenter, scalingRotation, (D3DXVECTOR2*)&scaling, (D3DXVECTOR2*)&rotationCenter, rotation, (D3DXVECTOR2*)&translation );
    }
    inline static cgMatrix * cgMatrix::affineTransformation( cgMatrix & out, float scaling, const cgVector3 & rotationCenter, const cgQuaternion & rotation, const cgVector3 & translation )
    {
        return (cgMatrix*)D3DXMatrixAffineTransformation( (D3DXMATRIX*)&out, scaling, (D3DXVECTOR3*)&rotationCenter, (D3DXQUATERNION*)&rotation, (D3DXVECTOR3*)&translation );
    }
    inline static cgMatrix * cgMatrix::affineTransformation2D( cgMatrix & out, float scaling, const cgVector2 & rotationCenter, cgFloat rotation, const cgVector2 & translation )
    {
        return (cgMatrix*)D3DXMatrixAffineTransformation2D( (D3DXMATRIX*)&out, scaling, (D3DXVECTOR2*)&rotationCenter, rotation, (D3DXVECTOR2*)&translation );
    }
    inline static cgMatrix * cgMatrix::lookAtRH( cgMatrix & out, const cgVector3 & eye, const cgVector3 & at, const cgVector3 & up )
    {
        return (cgMatrix*)D3DXMatrixLookAtRH( (D3DXMATRIX*)&out, (D3DXVECTOR3*)&eye, (D3DXVECTOR3*)&at, (D3DXVECTOR3*)&up );
    }
    inline static cgMatrix * cgMatrix::lookAtLH( cgMatrix & out, const cgVector3 & eye, const cgVector3 & at, const cgVector3 & up )
    {
        return (cgMatrix*)D3DXMatrixLookAtLH( (D3DXMATRIX*)&out, (D3DXVECTOR3*)&eye, (D3DXVECTOR3*)&at, (D3DXVECTOR3*)&up );
    }
    inline static cgMatrix * cgMatrix::perspectiveRH( cgMatrix & out, cgFloat width, cgFloat height, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveRH( (D3DXMATRIX*)&out, width, height, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::perspectiveLH( cgMatrix & out, cgFloat width, cgFloat height, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveLH( (D3DXMATRIX*)&out, width, height, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::perspectiveOffCenterRH( cgMatrix & out, cgFloat left, cgFloat right, cgFloat bottom, cgFloat top, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveOffCenterRH( (D3DXMATRIX*)&out, left, right, bottom, top, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::perspectiveOffCenterLH( cgMatrix & out, cgFloat left, cgFloat right, cgFloat bottom, cgFloat top, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveOffCenterLH( (D3DXMATRIX*)&out, left, right, bottom, top, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::perspectiveFovRH( cgMatrix & out, cgFloat fovY, cgFloat aspect, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveFovRH( (D3DXMATRIX*)&out, fovY, aspect, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::perspectiveFovLH( cgMatrix & out, cgFloat fovY, cgFloat aspect, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixPerspectiveFovLH( (D3DXMATRIX*)&out, fovY, aspect, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::orthoRH( cgMatrix & out, cgFloat width, cgFloat height, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixOrthoRH( (D3DXMATRIX*)&out, width, height, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::orthoLH( cgMatrix & out, cgFloat width, cgFloat height, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixOrthoLH( (D3DXMATRIX*)&out, width, height, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::orthoOffCenterRH( cgMatrix & out, cgFloat left, cgFloat right, cgFloat bottom, cgFloat top, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixOrthoOffCenterRH( (D3DXMATRIX*)&out, left, right, bottom, top, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::orthoOffCenterLH( cgMatrix & out, cgFloat left, cgFloat right, cgFloat bottom, cgFloat top, cgFloat nearClip, cgFloat farClip )
    {
        return (cgMatrix*)D3DXMatrixOrthoOffCenterLH( (D3DXMATRIX*)&out, left, right, bottom, top, nearClip, farClip );
    }
    inline static cgMatrix * cgMatrix::shadow( cgMatrix & out, const cgVector4 & light, const cgPlane & plane )
    {
        return (cgMatrix*)D3DXMatrixShadow( (D3DXMATRIX*)&out, (D3DXVECTOR4*)&light, (D3DXPLANE*)&plane );
    }
    inline static cgMatrix * cgMatrix::reflect( cgMatrix & out, const cgPlane & plane )
    {
        return (cgMatrix*)D3DXMatrixReflect( (D3DXMATRIX*)&out, (D3DXPLANE*)&plane );
    }
    
    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline cgMatrix operator * ( const cgMatrix & m ) const
    {
        cgMatrix out;
        multiply( out, *this, m );
        return out;
    }
    
    inline cgMatrix operator + ( const cgMatrix & m ) const
    {
        return cgMatrix( _11 + m._11, _12 + m._12, _13 + m._13, _14 + m._14,
                         _21 + m._21, _22 + m._22, _23 + m._23, _24 + m._24,
                         _31 + m._31, _32 + m._32, _33 + m._33, _34 + m._34,
                         _41 + m._41, _42 + m._42, _43 + m._43, _44 + m._44 );
    }
    
    inline cgMatrix operator - ( const cgMatrix & m ) const
    {
        return cgMatrix( _11 - m._11, _12 - m._12, _13 - m._13, _14 - m._14,
                         _21 - m._21, _22 - m._22, _23 - m._23, _24 - m._24,
                         _31 - m._31, _32 - m._32, _33 - m._33, _34 - m._34,
                         _41 - m._41, _42 - m._42, _43 - m._43, _44 - m._44 );
    }
    
    inline cgMatrix operator * ( cgFloat s ) const
    {
        return cgMatrix( _11 * s, _12 * s, _13 * s, _14 * s,
                         _21 * s, _22 * s, _23 * s, _24 * s,
                         _31 * s, _32 * s, _33 * s, _34 * s,
                         _41 * s, _42 * s, _43 * s, _44 * s );
    }
    
    inline cgMatrix operator / ( cgFloat s ) const
    {
        return cgMatrix( _11 / s, _12 / s, _13 / s, _14 / s,
                         _21 / s, _22 / s, _23 / s, _24 / s,
                         _31 / s, _32 / s, _33 / s, _34 / s,
                         _41 / s, _42 / s, _43 / s, _44 / s );
    }

    inline cgMatrix & operator *= ( const cgMatrix & m )
    {
        multiply( *this, *this, m );
        return *this;
    }
    
    inline cgMatrix & operator += ( const cgMatrix & m )
    {
        _11 += m._11; _12 += m._12; _13 += m._13; _14 += m._14;
        _21 += m._21; _22 += m._22; _23 += m._23; _24 += m._24;
        _31 += m._31; _32 += m._32; _33 += m._33; _34 += m._34;
        _41 += m._41; _42 += m._42; _43 += m._43; _44 += m._44;
        return *this;
    }
    
    inline cgMatrix & operator -= ( const cgMatrix & m )
    {
        _11 -= m._11; _12 -= m._12; _13 -= m._13; _14 -= m._14;
        _21 -= m._21; _22 -= m._22; _23 -= m._23; _24 -= m._24;
        _31 -= m._31; _32 -= m._32; _33 -= m._33; _34 -= m._34;
        _41 -= m._41; _42 -= m._42; _43 -= m._43; _44 -= m._44;
        return *this;
    }
    
    inline cgMatrix & operator *= ( cgFloat s )
    {
        _11 *= s; _12 *= s; _13 *= s; _14 *= s;
        _21 *= s; _22 *= s; _23 *= s; _24 *= s;
        _31 *= s; _32 *= s; _33 *= s; _34 *= s;
        _41 *= s; _42 *= s; _43 *= s; _44 *= s;
        return *this;
    }
    
    inline cgMatrix & operator /= ( cgFloat s )
    {
        _11 /= s; _12 /= s; _13 /= s; _14 /= s;
        _21 /= s; _22 /= s; _23 /= s; _24 /= s;
        _31 /= s; _32 /= s; _33 /= s; _34 /= s;
        _41 /= s; _42 /= s; _43 /= s; _44 /= s;
        return *this;
    }

    inline cgMatrix operator + () const
    {
        return *this;
    }
    
    inline cgMatrix operator - () const
    {
        return cgMatrix( -_11, -_12, -_13, -_14,
                         -_21, -_22, -_23, -_24,
                         -_31, -_32, -_33, -_34,
                         -_41, -_42, -_43, -_44 );
    }

    inline bool operator == ( const cgMatrix & m ) const
    {
        return (memcmp( this, &m, sizeof(cgMatrix) ) == 0);
    }
    
    inline bool operator != ( const cgMatrix & m ) const
    {
        return (memcmp( this, &m, sizeof(cgMatrix) ) != 0);
    }
        
    inline operator cgFloat* ()
    {
        return &_11;
    }
    
    inline operator const cgFloat* () const
    {
        return &_11;
    }
    
    inline cgFloat & operator () ( cgUInt32 rowIndex, cgUInt32 columnIndex )
    {
        return m[rowIndex][columnIndex];
    }

    inline cgFloat operator () ( cgUInt32 rowIndex, cgUInt32 columnIndex ) const
    {
        return m[rowIndex][columnIndex];
    }
    
    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    union
    {
        struct
        {
            cgFloat _11, _12, _13, _14;
            cgFloat _21, _22, _23, _24;
            cgFloat _31, _32, _33, _34;
            cgFloat _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

//-----------------------------------------------------------------------------
// Global Inline Operators (cgMatrix)
//-----------------------------------------------------------------------------
inline cgMatrix operator * ( cgFloat s, const cgMatrix & m )
{
    return cgMatrix( m._11 * s, m._12 * s, m._13 * s, m._14 * s,
                     m._21 * s, m._22 * s, m._23 * s, m._24 * s,
                     m._31 * s, m._32 * s, m._33 * s, m._34 * s,
                     m._41 * s, m._42 * s, m._43 * s, m._44 * s );
}

#endif // !_CGE_CGMATRIX_H_ 