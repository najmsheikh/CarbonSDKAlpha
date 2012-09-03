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
// Name : cgMathUtility.h                                                    //
//                                                                           //
// Desc : Simple namespace containing various different mathematics utility  //
//        functions.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMATHUTILITY_H_ )
#define _CGE_CGMATHUTILITY_H_

// ToDo: Clean this up and find homes where appropriate.

//-----------------------------------------------------------------------------
// cgMathUtility Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgMatrix;
class cgVector2;
class cgVector3;
class cgVector4;
class cgPlane;

//-----------------------------------------------------------------------------
// cgMathUtility Namespace
//-----------------------------------------------------------------------------
namespace cgMathUtility
{
    cgFloat            CGE_API  distanceToLineSegment       ( const cgVector3& point, const cgVector3& v1, const cgVector3& v2 );
    cgVector3          CGE_API  closestPointOnLineSegment   ( const cgVector3& vecPoint, const cgVector3& vecStart, const cgVector3& vecEnd );
	void               CGE_API  buildLookAtMatrix           ( cgMatrix *out, cgVector3 *eye, cgVector3 *at );
	cgMatrix           CGE_API  clipProjectionMatrix        ( cgMatrix& view, cgMatrix& proj, cgPlane& clipPlane );
    cgInt              CGE_API  compareVectors              ( const cgVector2 & v1, const cgVector2 & v2, cgFloat tolerance = 0.0f );
    cgInt              CGE_API  compareVectors              ( const cgVector3 & v1, const cgVector3 & v2, cgFloat tolerance = 0.0f );
    cgInt              CGE_API  compareVectors              ( const cgVector4 & v1, const cgVector4 & v2, cgFloat tolerance = 0.0f );
    const cgColorValue CGE_API& randomColor                 ( );
    cgFloat            CGE_API  distanceToPlane             ( const cgVector3 & point, const cgPlane & plane, const cgVector3 & direction );
    cgMatrix           CGE_API* matrixSwapYZ                ( cgMatrix * out, const cgMatrix * in );
    cgMatrix           CGE_API* matrixSwapHandedness        ( cgMatrix * out, const cgMatrix * in );
    void               CGE_API  solveQuadratic              ( cgDouble a, cgDouble b, cgDouble c, cgInt * solutionCount, cgDouble * solutions );
    void               CGE_API  solveCubic                  ( cgDouble a, cgDouble b, cgDouble c, cgDouble d, cgInt * solutionCount, cgDouble * solutions );

	// Inline Functions
	inline cgFloat randomFloat( cgFloat minimum, cgFloat maximum )
	{
		return minimum + ( ((cgFloat)rand()/(cgFloat)RAND_MAX) * (maximum-minimum) );
	}

	inline int randomInt( int minimum, int maximum )
	{
		return minimum + (int)( ((cgFloat)rand()/(cgFloat)RAND_MAX) * (maximum-minimum) );
	}

	inline cgFloat clamp( cgFloat val, cgFloat lo, cgFloat hi )
	{
		if( val < lo ) val = lo;
		else if( val > hi )	val = hi;
        return val;
	}

	inline cgFloat saturate( cgFloat val )
	{
		cgFloat t = val;
		clamp( t, 0.0f, 1.0f );
		return t;
	}

    inline cgUInt32 nextPowerOfTwo( cgUInt32 val )
    {
        if (val == 0)
            return 1;
        val--;
        for ( cgInt i = 1; i < 32; i <<= 1)
            val = val | val >> i;
        return val+1; 
    }
    
    inline cgUInt32 closestPowerOfTwo( cgUInt32 val )
    {
        return 1 << (cgUInt32)floor( (log( (cgDouble)val ) / log( 2.0f ) ) + 0.5f );
    }

    inline cgUInt32 log2( cgUInt32 val )
	{
		return (cgUInt32)( ( log( (cgDouble)val ) / log( 2.0 ) ) + 0.5f );
	}

	inline cgFloat log2f( cgUInt32 val )
	{
		return (cgFloat)( log( (cgDouble)val ) / log( 2.0 ) );
	}

    inline bool isPowerOfTwo( cgUInt32 val )
    {
        return (val & (val - 1)) == 0;
    }
    
    inline bool isEven( cgUInt32 val )
    {
        return ((val & 1) == 0);
    }

    inline cgFloat smooth( cgFloat newValue, cgFloat oldValue, cgFloat increments )
    {
        cgInt oldSign = ((oldValue - newValue) > 0) ? 1 : -1;
        cgFloat slip = (oldValue - newValue) / increments;
        oldValue -= slip;
        cgInt newSign = ((oldValue - newValue) > 0) ? 1 : -1;
        return ( oldSign != newSign ) ? newValue : oldValue;
    }

    // Fast Math Routines
    inline cgInt integerFloor(const cgFloat & v)
    {
	    const cgInt & u = *(const cgInt*)&v;
	    cgInt sign = u >> 31;
	    cgInt exponent = ((u & 0x7fffffff)>>23) - 127;
	    cgInt expsign = ~(exponent >> 31);
	    cgInt mantissa = (u & ((1<<23) - 1));
	    return (((((cgUInt)(mantissa | (1<<23)) << 8) >> (31-exponent)) & expsign) ^ (sign)) + ((!((mantissa<<8) & (((1<<(31-(exponent & expsign)))) - 1))) & sign);
    }
    
    inline cgInt integerCeil(const cgFloat & v)
    {
	    cgInt u = (*(const cgInt*)&v) ^ 0x80000000;
	    cgInt sign = u >> 31;
	    cgInt exponent = ((u & 0x7fffffff)>>23) - 127;
	    cgInt expsign = ~(exponent>>31);
	    cgInt mantissa = (u & ((1<<23) - 1));
	    return -(cgInt)((((((cgUInt)(mantissa | (1<<23)) << 8) >> (31-exponent)) & expsign) ^ (sign)) + ((!((mantissa<<8) & (((1<<(31-(exponent & expsign)))) - 1))) & sign & expsign));
    }

}; // End Namespace : cgMathUtility

#endif // !_CGE_CGMATHUTILITY_H_