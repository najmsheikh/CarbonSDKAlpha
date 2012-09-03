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
// File: cgPolynomial.cpp                                                    //
//                                                                           //
// Desc: Classes designed to assist with the evaluation of polynomials.      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPolynomial Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgPolynomial.h>
#include <Math/cgMathTypes.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// cgPolynomial Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgPolynomial() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgPolynomial::cgPolynomial( cgUInt32 nDegree )
{
    mCoeff.resize( nDegree + 1 );
}

//-----------------------------------------------------------------------------
// Name : findRoots ()
/// <summary>Determine the roots of this polynomial.</summary>
//-----------------------------------------------------------------------------
bool cgPolynomial::findRoots( cgFloatArray & aRoots, cgUInt16 nDigits )
{
    // Be polite and clear output variables
    aRoots.clear();

    // Valid?
    cgUInt32 nDegrees = getDegrees();
    if ( nDegrees < 1 )
        return false;

    // Determine bounds
    cgFloat fMax      = 0.0f;
    cgFloat fRecipDeg = 1.0f / mCoeff[ nDegrees ];
    for ( cgUInt32 i = 0; i < nDegrees; ++i )
    {
        cgFloat fValue = fabsf( mCoeff[i] ) * fRecipDeg;
        if ( fValue > fMax )
            fMax = fValue;
        
    } // Next

    // Pass through to main method
    return findRoots( aRoots, -(1.0f + fMax), 1.0f + fMax, nDigits );
}
bool cgPolynomial::findRoots( cgFloatArray & aRoots, cgFloat fMin, cgFloat fMax, cgUInt16 nDigits )
{
    cgFloat fRoot;
    cgUInt32 nDegrees = getDegrees();

    // Be polite and clear output variables
    aRoots.clear();

    // Simple 1 degree case?
    if ( nDegrees == 1 )
    {
        if ( bisection( fMin, fMax, nDigits, fRoot ) == true )
        {
            aRoots.resize( nDegrees );
            aRoots[0] = fRoot;
            return true;

        } // End if bisected
        return false;

    } // End if 1 degree

    // Get roots of derivative polynomial
    cgFloatArray aDerivRoots;
    getDerivative().findRoots( aDerivRoots, fMin, fMax, nDigits );
    
    // Found derivative roots?
    if ( aDerivRoots.empty() == false )
    {
        cgUInt32 nRootCount = 0;

        // Allocate space for potential roots.
        aRoots.resize( nDegrees );

        // Find roots.
        if ( bisection( fMin, aDerivRoots[0], nDigits, fRoot) == true )
            aRoots[nRootCount++] = fRoot;
        
        for ( cgInt i = 0; i <= (cgInt)aDerivRoots.size() - 2; ++i )
        {
            if ( bisection( aDerivRoots[i], aDerivRoots[i+1], nDigits, fRoot) == true )
                aRoots[nRootCount++] = fRoot;
        
        } // Next

        if ( bisection( aDerivRoots[ aDerivRoots.size() - 1 ], fMax, nDigits, fRoot ) == true )
            aRoots[nRootCount++] = fRoot;

        // Shrink roots array to actual size
        if ( nRootCount == 0 )
        {
            aRoots.clear();
        
        } // End if no roots
        else
        {
            if ( nRootCount != aRoots.size() )
                aRoots.resize( nRootCount );
        
        } // End if roots

    } // End if roots
    else
    {
        // cgPolynomial has a maximum of one root.
        if ( bisection( fMin, fMax, nDigits, fRoot) == true )
        {
            aRoots.resize( 1 );
            aRoots[0] = fRoot;
            
        } // End if

    } // End if no roots


    // Found any?
    return (aRoots.empty() == false);
}

//-----------------------------------------------------------------------------
// Name : bisection ()
/// <summary>Perform polynomial bisection.</summary>
//-----------------------------------------------------------------------------
bool cgPolynomial::bisection( cgFloat fMin, cgFloat fMax, cgUInt16 digits, cgFloat & fRoot )
{
    cgFloat fP0 = evaluate( fMin );
    if ( fabsf( fP0 ) <= CGE_EPSILON )
    {
        fRoot = fMin;
        return true;
    } // End if <= 0

    float fP1 = evaluate( fMax );
    if ( fabsf( fP1 ) <= CGE_EPSILON )
    {
        fRoot = fMax;
        return true;
    } // End if <= 0
    
    if ( fP0 * fP1 > 0.0f )
        return false;

    // Compute number of iterations for specified accuracy (in digits).
    cgFloat fTmp0 = logf( fMax - fMin );
    cgFloat fTmp1 = ((cgFloat )digits) * logf( 10.0f );
    cgFloat fArg  = (fTmp0 + fTmp1) / logf( 2.0f );
    cgInt32 nIterations = (cgInt32)(fArg + 0.5f);

    for ( cgInt32 i = 0; i < nIterations; ++i )
    {
        fRoot = (fMin + fMax) * 0.5f;
        cgFloat fP = evaluate( fRoot );
        cgFloat fProduct = fP*fP0;
        if ( fProduct < 0.0f )
        {
            fMax = fRoot;
            fP1 = fP;
        
        } // End if < 0
        else if ( fProduct > 0.0f )
        {
            fMin = fRoot;
            fP0 = fP;
        } // End if > 0
        else
        {
            break;
        
        } // End if == 0
    
    } // Next Iteration

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : evaluate ()
/// <summary>Evaluate the polynomial for the specified T.</summary>
//-----------------------------------------------------------------------------
cgFloat cgPolynomial::evaluate( cgFloat t )
{
    cgUInt32 nDegrees = getDegrees();
    cgFloat fValue = mCoeff[ nDegrees ];
    for ( cgInt32 i = nDegrees - 1; i >= 0; --i )
    {
        fValue *= t;
        fValue += mCoeff[i];
    }
    return fValue;
}

//-----------------------------------------------------------------------------
// Name : getDegrees ( )
/// <summary>Determine the number of degrees for this polynomial.</summary>
//-----------------------------------------------------------------------------
cgUInt32 cgPolynomial::getDegrees( ) const
{
    return ( mCoeff.empty() == true ) ? 0 : (cgUInt32)mCoeff.size() - 1;
}

//-----------------------------------------------------------------------------
// Name : getDerivative ( )
/// <summary>Get the derivative for this polynomial.</summary>
//-----------------------------------------------------------------------------
cgPolynomial cgPolynomial::getDerivative( ) const
{
    cgUInt32 nDegrees = getDegrees();
    if ( nDegrees > 0 )
    {
        cgPolynomial Out( nDegrees - 1 );
        for ( cgUInt32 i0 = 0, i1 = 1; i0 < nDegrees; i0++, i1++)
            Out.setCoefficient(i0, mCoeff[i1] * i1 );
        return Out;
    
    } // End if > 0 degrees
    
    // Other.
    cgPolynomial Out( 0 );
    Out.setCoefficient(0, 0.0f);
    return Out;
}

//-----------------------------------------------------------------------------
// Name : getCoefficient ( )
/// <summary>Get specified coefficient.</summary>
//-----------------------------------------------------------------------------
cgFloat cgPolynomial::getCoefficient( cgUInt32 nIndex ) const
{
    return mCoeff[nIndex];
}

//-----------------------------------------------------------------------------
// Name : setCoefficient ( )
/// <summary>Set specified coefficient.</summary>
//-----------------------------------------------------------------------------
void cgPolynomial::setCoefficient( cgUInt32 nIndex, cgFloat value )
{
    mCoeff[nIndex] = value;
}