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
// File: cgPolynomial.h                                                      //
//                                                                           //
// Desc: Classes designed to assist with the evaluation of polynomials.      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPOLYNOMIAL_H_ )
#define _CGE_CGPOLYNOMIAL_H_

//-----------------------------------------------------------------------------
// cgPolynomial Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgPolynomial (Class)
/// <summary>Polynomial value class.</summary>
//-----------------------------------------------------------------------------
class CGE_API cgPolynomial
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgPolynomial( cgUInt32 degree );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool            findRoots       ( cgFloatArray & roots, cgUInt16 digits );
    bool            findRoots       ( cgFloatArray & roots, cgFloat minimum, cgFloat maximum, cgUInt16 digits );
    bool            bisection       ( cgFloat minimum, cgFloat maximum, cgUInt16 digits, cgFloat & root );
    cgFloat         evaluate        ( cgFloat t );
    cgUInt32        getDegrees      ( ) const;
    cgPolynomial    getDerivative   ( ) const;
    cgFloat         getCoefficient  ( cgUInt32 index ) const;
    void            setCoefficient  ( cgUInt32 index, cgFloat value );
    
private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgFloatArray mCoeff;
};

#endif // !_CGE_CGPOLYNOMIAL_H_