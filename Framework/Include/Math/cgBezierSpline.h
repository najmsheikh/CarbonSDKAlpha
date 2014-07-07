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
// Name : cgBezierSpline.h                                                   //
//                                                                           //
// Desc : Utility math classes designed to provide support for simple        //
//        splines primarily useful for mapping two dimensional time or       //
//        distance based value smoothing such as attenuation.                //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBEZIERSPLINE_H_ )
#define _CGE_CGBEZIERSPLINE_H_

//-----------------------------------------------------------------------------
// cgBezierSpline Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBezierSpline2 (Class)
/// <summary>
/// Contains support for evaluating values along a specified set of 
/// 2D control points that allow the artist to define a specific curve for
/// certain values such as distance attenuation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBezierSpline2
{
public:
    //------------------------------------------------------------------------
    // Public Enumerators
    //------------------------------------------------------------------------
    enum SplineDescription
    {
        Custom          = 0,
        LinearDecay     = 1,
        LinearGrowth    = 2,
        Maximum         = 3,
        Minimum         = 4
    };
    enum EvaluateMethod
    {
        NormalizePlusVariance  = 0,
        NormalizeScaleVariance = 1,
        NormalizeOnly          = 2,
        SplinePlusVariance     = 3,
        SplineScaleVariance    = 4,
        SplineOnly             = 5
    };

    //------------------------------------------------------------------------
    // Public Structures
    //------------------------------------------------------------------------
    struct CGE_API SplinePoint
    {
        cgVector2 controlPointIn;
        cgVector2 point;
        cgVector2 controlPointOut;

        // Constructors
        SplinePoint( ) {}
        SplinePoint( const cgVector2 & _controlPointIn, const cgVector2 & _point, const cgVector2 & _controlPointOut ) :
            controlPointIn( _controlPointIn ), point( _point ), controlPointOut( _controlPointOut ) {}
    };
    CGE_ARRAY_DECLARE(SplinePoint, SplinePointArray)

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgBezierSpline2( );
     cgBezierSpline2( const SplinePointArray & splinePoints );
     cgBezierSpline2( const cgBezierSpline2 & spline );
    ~cgBezierSpline2( );
    
	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                    addPoint            ( const SplinePoint & pt );
    void                    insertPoint         ( cgInt32 insertAt, const SplinePoint & pt );
    void                    removePoint         ( cgInt32 index );
    void                    clear               ( );
    cgString                computeHash         ( cgInt16 numSignificantDigits = -1 );
    bool                    sortByX             ( cgUInt32Array * remap = CG_NULL );
    
    // Sampling methods
    cgVector2               evaluate            ( cgFloat t );
    cgFloat                 evaluateForX        ( cgFloat x, bool approximate = false, cgUInt16 digits = 4 );
    cgFloat                 evaluateForX        ( EvaluateMethod method, cgFloat x, cgFloat rand = 0, bool approximate = false, cgUInt16 digits = 4 );
    cgVector2               evaluateSegment     ( cgInt32 segment, cgFloat t );

    // Accessors
    const SplinePointArray& getPoints           ( ) const;
    const SplinePoint     & getPoint            ( cgInt32 index ) const;
    SplinePoint           & getPoint            ( cgInt32 index );
    void                    setPoint            ( cgInt32 index, const SplinePoint & pt );
    cgInt32                 getPointCount       ( ) const;
    cgInt32                 getSegmentCount     ( ) const;
    bool                    isComplex           ( );
    SplineDescription       getDescription      ( ) const;
    void                    setDescription      ( SplineDescription desc );
    void                    setRange            ( const cgRangeF & range );
    void                    setRange            ( cgFloat minimum, cgFloat maximum );
    void                    setRangeMinimum     ( cgFloat value );
    void                    setRangeMaximum     ( cgFloat value );
    const cgRangeF        & getRange            ( ) const;
    void                    setVariance         ( cgFloat variance );
    cgFloat                 getVariance         ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Operator Overloads
    //-------------------------------------------------------------------------
    cgBezierSpline2       & operator=           ( const cgBezierSpline2 & spline );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    updateSplineData    ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    SplinePointArray    mPoints;
    cgFloatArray        mPointDist;
    cgFloat             mLength;
    cgRangeF            mRange;
    cgFloat             mVariance;
    bool                mSplineDirty;
    bool                mComplexSpline;
};

//-----------------------------------------------------------------------------
//  Name : cgBezierSpline3 (Class)
/// <summary>
/// Contains support for evaluating values along a specified set of 
/// 3D control points that allow the artist to define a specific curve for
/// certain values such as distance attenuation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBezierSpline3
{
public:
    //------------------------------------------------------------------------
    // Public Structures
    //------------------------------------------------------------------------
    struct CGE_API SplinePoint
    {
        cgVector3 controlPointIn;
        cgVector3 point;
        cgVector3 controlPointOut;

        // Constructors
        SplinePoint( ) {}
        SplinePoint( const cgVector3 & _controlPointIn, const cgVector3 & _point, const cgVector3 & _controlPointOut ) :
            controlPointIn( _controlPointIn ), point( _point ), controlPointOut( _controlPointOut ) {}
    };
    CGE_ARRAY_DECLARE(SplinePoint, SplinePointArray)

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
     cgBezierSpline3( );
     cgBezierSpline3( const SplinePointArray & splinePoints );
     cgBezierSpline3( const cgBezierSpline3 & spline );
    ~cgBezierSpline3( );
    
	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                    addPoint            ( const SplinePoint & pt );
    void                    insertPoint         ( cgInt32 insertAt, const SplinePoint & pt );
    void                    removePoint         ( cgInt32 index );
    void                    clear               ( );
    
    // Sampling methods
    cgVector3               evaluate            ( cgFloat t );
    cgVector3               evaluateSegment     ( cgInt32 segment, cgFloat t );

    // Accessors
    const SplinePointArray& getPoints           ( ) const;
    const SplinePoint     & getPoint            ( cgInt32 index ) const;
    SplinePoint           & getPoint            ( cgInt32 index );
    void                    setPoint            ( cgInt32 index, const SplinePoint & pt );
    cgInt32                 getPointCount       ( ) const;
    cgInt32                 getSegmentCount     ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Operator Overloads
    //-------------------------------------------------------------------------
    cgBezierSpline3       & operator=           ( const cgBezierSpline3 & spline );
    bool                    operator==          ( const cgBezierSpline3 & spline ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    updateSplineData    ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    SplinePointArray    mPoints;
    cgFloatArray        mPointDist;
    cgFloat             mLength;
    bool                mSplineDirty;
};

#endif // !_CGE_CGBEZIERSPLINE_H_