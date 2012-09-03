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
// File: cgEulerAngles.cpp                                                   //
//                                                                           //
// Desc: Utility class for computing and working with Euler angles.          //
//                                                                           //
// Note: Special thanks to Fletcher Dunn & Ian Parberry whose examples on    //
//       which this is heavily based.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgEulerAngles Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgEulerAngles.h>

///////////////////////////////////////////////////////////////////////////////
// cgEulerAngles Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : fromQuaternion ()
/// <summary>
/// Compute Euler angles from the specified quaternion assuming the given
/// order of reconstruction.
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAngles & cgEulerAngles::fromQuaternion( const cgQuaternion & q, cgInt _Order )
{
    cgFloat sp = -2.0f * (q.y * q.z - q.w * q.x );
    if ( fabs( sp ) > 0.9999f )
    {
        x = 1.570796f * sp;
        y = atan2f( -q.x*q.z + q.w*q.y, 0.5f - q.y*q.y - q.z*q.z );
        z = 0.0f;
    }
    else
    {
        x = asinf(sp);
        y = atan2f( q.x*q.z + q.w*q.y, 0.5f - q.x*q.x - q.y*q.y );
        z = atan2f( q.x*q.y + q.w*q.z, 0.5f - q.x*q.x - q.z*q.z );
    }
    order = _Order;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : fromMatrix ()
/// <summary>
/// Compute Euler angles from the specified matrix assuming the given
/// order of reconstruction.
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAngles & cgEulerAngles::fromMatrix( const cgMatrix & m, cgInt _Order )
{
    cgQuaternion q;
    cgQuaternion::rotationMatrix( q, m );
    return fromQuaternion( q, _Order );
}

//-----------------------------------------------------------------------------
//  Name : fromTransform ()
/// <summary>
/// Compute Euler angles from the specified transform assuming the given
/// order of reconstruction.
/// </summary>
//-----------------------------------------------------------------------------
cgEulerAngles & cgEulerAngles::fromTransform( const cgTransform & t, cgInt _Order )
{
    return fromQuaternion( t.orientation(), _Order );
}

//-----------------------------------------------------------------------------
//  Name : toQuaternion ()
/// <summary>
/// Compute a quaternion from the current Euler angles.
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternion & cgEulerAngles::toQuaternion( cgQuaternion & q ) const
{
    cgQuaternion::rotationYawPitchRoll( q, y, x, z );
    return q;
}

//-----------------------------------------------------------------------------
//  Name : toMatrix ()
/// <summary>
/// Compute a matrix from the current Euler angles.
/// </summary>
//-----------------------------------------------------------------------------
cgMatrix & cgEulerAngles::toMatrix( cgMatrix & m ) const
{
    cgMatrix::rotationYawPitchRoll( m, y, x, z );
    return m;
}

//-----------------------------------------------------------------------------
//  Name : toTransform ()
/// <summary>
/// Compute a transform object from the current Euler angles.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform & cgEulerAngles::toTransform( cgTransform & t ) const
{
    // NB: 'cgTransform::rotation()' combines in YXZ order.
    t.rotation( x, y, z );
    return t;
}