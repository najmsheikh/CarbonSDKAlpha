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
// File: cgEulerAngles.h                                                     //
//                                                                           //
// Desc: Utility class for computing and working with Euler angles.          //
//                                                                           //
// Note: Special thanks to Fletcher Dunn & Ian Parberry whose examples on    //
//       which this is heavily based.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGEULERANGLES_H_ )
#define _CGE_CGEULERANGLES_H_

//-----------------------------------------------------------------------------
// cgEulerAngles Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Global Namespaces
//-----------------------------------------------------------------------------
namespace cgEulerAnglesOrder
{
    enum Base
    {
        YXZ
    };

}; // End Namespace cgEulerAnglesOrder

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Name : cgEulerAngles (Class)
/// <summary>
/// Utility class for computing and working with Euler angles.
/// </summary>
//-------------------------------------------------------------------------
class CGE_API cgEulerAngles
{
public:
    //---------------------------------------------------------------------
    // Constructors & Destructors
    //---------------------------------------------------------------------
    cgEulerAngles() :
      x(0), y(0), z(0), order( cgEulerAnglesOrder::YXZ ) {}
    cgEulerAngles( cgFloat _x, cgFloat _y, cgFloat _z, cgInt _order = cgEulerAnglesOrder::YXZ ) :
      x(_x), y(_y), z(_z), order( _order ) {}
    cgEulerAngles( const cgQuaternion & q, cgInt _order = cgEulerAnglesOrder::YXZ )
    {
        fromQuaternion( q, _order );
    }
    cgEulerAngles( const cgMatrix & m, cgInt _order = cgEulerAnglesOrder::YXZ )
    {
        fromMatrix( m, _order );
    }
    cgEulerAngles( const cgTransform & t, cgInt _order = cgEulerAnglesOrder::YXZ )
    {
        fromTransform( t, _order );
    }

    //---------------------------------------------------------------------
    // Public Methods
    //---------------------------------------------------------------------
    cgEulerAngles     & fromQuaternion  ( const cgQuaternion & q, cgInt _order );
    cgEulerAngles     & fromMatrix      ( const cgMatrix & m, cgInt _order );
    cgEulerAngles     & fromTransform   ( const cgTransform & t, cgInt _order );
    cgQuaternion      & toQuaternion    ( cgQuaternion & q ) const;
    cgMatrix          & toMatrix        ( cgMatrix & m ) const;
    cgTransform       & toTransform     ( cgTransform & t ) const;

    //---------------------------------------------------------------------
    // Public Inline Operators
    //---------------------------------------------------------------------
    inline operator cgFloat* ()
    {
        return (cgFloat*)&x;
    
    } // End Cast

    inline operator const cgFloat* () const
    {
        return (const cgFloat*)&x;
    
    } // End Cast
    
    //---------------------------------------------------------------------
    // Public Members
    //---------------------------------------------------------------------
    cgFloat x;
    cgFloat y;
    cgFloat z;
    cgInt   order;
    
}; // End Class cgEulerAngles

#endif // !_CGE_CGEULERANGLES_H_