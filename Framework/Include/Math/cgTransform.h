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
// Name : cgTransform.h                                                      //
//                                                                           //
// Desc : General purpose transformation class designed to maintain each     //
//        component of the transformation separate (translation, rotation,   //
//        scale and shear) whilst providing much of the same functionality   //
//        provided by standard matrices.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGTRANSFORM_H_ )
#define _CGE_CGTRANSFORM_H_

//-----------------------------------------------------------------------------
// cgTransform Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgVector.h>
#include <Math/cgMatrix.h>
#include <Math/cgPlane.h>
#include <Math/cgQuaternion.h>

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTransform (Class)
/// <summary>
/// General purpose transformation class designed to maintain each component of
/// the transformation separate (translation, rotation, scale and shear) whilst
/// providing much of the same functionality provided by standard matrices.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTransform
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
    cgTransform( );
    cgTransform( const cgTransform & t );
    cgTransform( const cgVector3 & translation );
    cgTransform( const cgQuaternion & orientation, const cgVector3 & translation );
    cgTransform( const cgMatrix & m );
    
	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgVector3         & transformCoord              ( cgVector3 & out, const cgVector3 & v ) const;
    cgVector3         & inverseTransformCoord       ( cgVector3 & out, const cgVector3 & v ) const;
    cgVector3         & transformNormal             ( cgVector3 & out, const cgVector3 & v ) const;
    cgVector3         & inverseTransformNormal      ( cgVector3 & out, const cgVector3 & v ) const;
    bool                decompose                   ( cgVector3 & scale, cgVector3 & shear, cgQuaternion & rotation, cgVector3 & translation ) const;
    bool                decompose                   ( cgQuaternion & rotation, cgVector3 & translation ) const;
    cgTransform       & multiply                    ( cgTransform & out, const cgTransform & t ) const;
    cgTransform       & add                         ( cgTransform & out, const cgTransform & t ) const;
    cgTransform       & inverse                     ( cgTransform & out ) const;
    cgTransform       & invert                      ( );
    cgVector3         & position                    ( );
    const cgVector3   & position                    ( ) const;
    cgVector3           localScale                  ( ) const;
    cgQuaternion        orientation                 ( ) const;
    const cgVector3   & xAxis                       ( ) const;
    const cgVector3   & yAxis                       ( ) const;
    const cgVector3   & zAxis                       ( ) const;
    cgVector3           xUnitAxis                   ( ) const;
    cgVector3           yUnitAxis                   ( ) const;
    cgVector3           zUnitAxis                   ( ) const;

    // Transformation Operations
    cgTransform       & rotate                      ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & rotate                      ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center );
    cgTransform       & rotateAxis                  ( cgFloat a, const cgVector3 & v );
    cgTransform       & rotateAxis                  ( cgFloat a, const cgVector3 & v, const cgVector3 & center );
    cgTransform       & rotateLocal                 ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & scale                       ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & scale                       ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center );
    cgTransform       & scaleLocal                  ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & translate                   ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & translate                   ( const cgVector3 & v );
    cgTransform       & translateLocal              ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & translateLocal              ( const cgVector3 & v );
    cgTransform       & setPosition                 ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & setPosition                 ( const cgVector3 & v );
    cgTransform       & setLocalScale               ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & setLocalScale               ( const cgVector3 & v );
    cgTransform       & setLocalShear               ( cgFloat xy, cgFloat xz, cgFloat yz );
    cgTransform       & setOrientation              ( const cgVector3 & x, const cgVector3 & y, const cgVector3 & z );
    cgTransform       & setOrientation              ( const cgQuaternion & q );

    // Full Re-Populate
    cgTransform       & zero                        ( );
    cgTransform       & identity                    ( );
    cgTransform       & compose                     ( const cgVector3 & scale, const cgVector3 & shear, const cgQuaternion & rotation, const cgVector3 & translation );
    cgTransform       & compose                     ( const cgQuaternion & rotation, const cgVector3 & translation );
    cgTransform       & scaling                     ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & rotation                    ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & rotationAxis                ( cgFloat angle, const cgVector3 & axis );
    cgTransform       & translation                 ( cgFloat x, cgFloat y, cgFloat z );
    cgTransform       & translation                 ( const cgVector3 & v );
    cgTransform       & lookAt                      ( const cgVector3 & eye, const cgVector3 & at );
    cgTransform       & lookAt                      ( const cgVector3 & eye, const cgVector3 & at, const cgVector3 & upAlign );

    // Comparisons
    cgInt               compare                     ( const cgTransform & t ) const;
    cgInt               compare                     ( const cgTransform & t, cgFloat tolerance ) const;
    bool                isIdentity                  ( ) const;
    
    //-------------------------------------------------------------------------
	// Public Operator Overloads
	//-------------------------------------------------------------------------
                        operator const cgMatrix &   ( ) const;
                        operator cgMatrix *         ( );
                        operator const cgMatrix *   ( ) const;
    cgTransform       & operator=                   ( const cgMatrix & m );
    bool                operator==                  ( const cgTransform & t ) const;
    bool                operator!=                  ( const cgTransform & t ) const;

    // Assignment operators
    cgTransform       & operator *=                 ( const cgTransform & t );
    cgTransform       & operator *=                 ( cgFloat f );
    cgTransform       & operator +=                 ( const cgTransform & t );

    // Binary operators
    cgTransform         operator *                  ( const cgTransform & t ) const;
    cgTransform         operator *                  ( cgFloat f ) const;
    cgTransform         operator +                  ( const cgTransform & t ) const;

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgVector3  & transformCoord              ( cgVector3 & out, const cgVector3 & v, const cgTransform & t );
    static cgVector3  & inverseTransformCoord       ( cgVector3 & out, const cgVector3 & v, const cgTransform & t );
    static cgVector3  & transformNormal             ( cgVector3 & out, const cgVector3 & v, const cgTransform & t );
    static cgVector3  & inverseTransformNormal      ( cgVector3 & out, const cgVector3 & v, const cgTransform & t );
    static cgTransform& multiply                    ( cgTransform & out, const cgTransform & t1, const cgTransform & t2 );
    static cgTransform& add                         ( cgTransform & out, const cgTransform & t1, const cgTransform & t2 );
    static cgTransform& inverse                     ( cgTransform & out, const cgTransform & t );
    static bool         decompose                   ( cgVector3 & scale, cgVector3 & shear, cgQuaternion & rotation, cgVector3 & translation, const cgTransform & t );
    static bool         decompose                   ( cgQuaternion & rotation, cgVector3 & translation, const cgTransform & t );
    static cgTransform& compose                     ( cgTransform & out, const cgVector3 & scale, const cgVector3 & shear, const cgQuaternion & rotation, const cgVector3 & translation );
    static cgTransform& compose                     ( cgTransform & out, const cgQuaternion & rotation, const cgVector3 & translation );
    static cgTransform& identity                    ( cgTransform & out );
    static cgTransform& lookAt                      ( cgTransform & out, const cgVector3 & eye, const cgVector3 & at );
    static cgTransform& lookAt                      ( cgTransform & out, const cgVector3 & eye, const cgVector3 & at, const cgVector3 & upAlign );
    static cgTransform& scaling                     ( cgTransform & out, cgFloat x, cgFloat y, cgFloat z );
    static cgTransform& rotation                    ( cgTransform & out, cgFloat x, cgFloat y, cgFloat z );
    static cgTransform& rotationAxis                ( cgTransform & out, cgFloat angle, const cgVector3 & axis );
    static cgTransform& translation                 ( cgTransform & out, cgFloat x, cgFloat y, cgFloat z );
    static cgTransform& translation                 ( cgTransform & out, const cgVector3 & v );
    
protected:
    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    cgMatrix _m;
};

#endif // !_CGE_CGTRANSFORM_H_