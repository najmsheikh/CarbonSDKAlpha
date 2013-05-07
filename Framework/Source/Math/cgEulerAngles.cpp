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
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
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
    if ( _Order == cgEulerAnglesOrder::YXZ )
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


    } // End if YXZ
    else if ( _Order == cgEulerAnglesOrder::XYZ )
    {
        cgMatrix m;
        cgMatrix::rotationQuaternion( m, q );
        if (m(0,2) < 1.f)
	    {
		    if (m(0,2) > -1.f)
		    {
			    y = asinf(m(0,2));
			    x = atan2f(-m(1,2), m(2,2));
			    z = atan2f(-m(0,1), m(0,0));
		    }
		    else
		    {
			    // Not a unique solution: z - x = atan2(m[1][0], m[1][1]);
			    y = -CGE_PI/2.f;
			    x = -atan2f(m(1,0), m(1,1));
			    z = 0.f;
		    }
	    }
	    else
	    {
		    // Not a unique solution: z + x = atan2(m[1][0], m[1][1]);
		    y = CGE_PI/2.f;
		    x = atan2f(m(1,0), m(1,1));
		    z = 0.f;
	    }

    } // End if XYZ

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
    if ( order == cgEulerAnglesOrder::YXZ )
        cgQuaternion::rotationYawPitchRoll( q, y, x, z );
    else
    {
        cgQuaternion qx, qy, qz;
        cgQuaternion::rotationAxis( qx, cgVector3(1,0,0), x );
        cgQuaternion::rotationAxis( qy, cgVector3(0,1,0), y );
        cgQuaternion::rotationAxis( qz, cgVector3(0,0,1), z );
        if ( order == cgEulerAnglesOrder::XYZ )
            q = qx * qy * qz;
    
    } // End if !YXZ
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
    if ( order == cgEulerAnglesOrder::YXZ )
        cgMatrix::rotationYawPitchRoll( m, y, x, z );
    else
    {
        cgMatrix mx, my, mz;
        cgMatrix::rotationAxis( mx, cgVector3(1,0,0), x );
        cgMatrix::rotationAxis( my, cgVector3(0,1,0), y );
        cgMatrix::rotationAxis( mz, cgVector3(0,0,1), z );
        if ( order == cgEulerAnglesOrder::XYZ )
            m = mx * my * mz;

    } // End if !YXZ
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
    if ( order == cgEulerAnglesOrder::YXZ )
        t.rotation( x, y, z );
    else
    {
        cgMatrix m;
        t = toMatrix( m );
    } // End if !YXZ
    return t;
}