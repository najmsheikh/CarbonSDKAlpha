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
// Name : cgBezierSpline.cpp                                                 //
//                                                                           //
// Desc : Utility math classes designed to provide support for simple        //
//        splines primarily useful for mapping two dimensional time or       //
//        distance based value smoothing such as attenuation.                //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBezierSpline Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgBezierSpline.h>
#include <Math/cgPolynomial.h>
#include <Math/cgMathUtility.h>
#include <System/cgStringUtility.h>
#include <algorithm>
#include <math.h>
#include <limits>

///////////////////////////////////////////////////////////////////////////////
// cgBezierSpline2 Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBezierSpline2 () (Default Constructor)
/// <summary>
/// cgBezierSpline2 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::cgBezierSpline2( ) 
{
	// Default to linear attenuation
    setDescription( LinearDecay );
    mLength        = 0.0f;
    mComplexSpline = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBezierSpline2 () (Constructor)
/// <summary>
/// cgBezierSpline2 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::cgBezierSpline2( const SplinePointArray & SplinePoints )
{
    mPoints = SplinePoints;
    mSplineDirty   = true;
    mLength        = 0.0f;
    mComplexSpline = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBezierSpline2 () (Constructor)
/// <summary>
/// cgBezierSpline2 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::cgBezierSpline2( const cgBezierSpline2 & Spline ) 
{
	*this = Spline;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBezierSpline2 () (Destructor)
/// <summary>
/// cgBezierSpline2 Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::~cgBezierSpline2( ) 
{
    // Release any allocated memory
	clear();
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgBezierSpline2&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2 & cgBezierSpline2::operator=( const cgBezierSpline2 & Spline )
{
    // Is this a no-op?
    if ( &Spline == this )
        return *this;

    // Clear out any old data
    clear();

    // Copy data.
    mPoints           = Spline.mPoints;
    mPointDist        = Spline.mPointDist;
    mLength           = Spline.mLength;
    mSplineDirty      = Spline.mSplineDirty;
    mComplexSpline    = Spline.mComplexSpline;
    
    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
// Name : addPoint()
/// <summary>Add the specified control point.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::addPoint( const SplinePoint & pt )
{
    // Add point to end of list.
    mPoints.push_back( pt );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : insertPoint()
/// <summary>
/// Insert the specified control point at the referenced location.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::insertPoint( cgInt32 nInsertAt, const SplinePoint & pt )
{
    // Validate operation
    if ( nInsertAt < 0 || nInsertAt > (cgInt32)mPoints.size() )
        return;

    // Insert into array.
    mPoints.insert( mPoints.begin() + nInsertAt, pt );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : removePoint()
/// <summary>
/// Remove the control point at the referenced location.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::removePoint( cgInt32 nIndex )
{
    // Validate operation
    if ( nIndex < 0 || nIndex >= (cgInt32)mPoints.size() )
        return;

    // Shift all control points down
    mPoints.erase( mPoints.begin() + nIndex );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : clear()
/// <summary>Clear all control points.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::clear( )
{
    // Resize array
    mPoints.clear();
    mPointDist.clear();
    mSplineDirty   = true;
    mComplexSpline = false;
}

//-----------------------------------------------------------------------------
// Name : sortByX()
/// <summary>
/// Sort the control points based on their X axis value. Optionally populates 
/// a remap array whose size is equal to the number of points, the key 
/// represents the new point index, and the value represents the original index
/// for that point. In this case, this method will return false if no
/// modification was made to the spline, or true if the order was altered.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBezierSpline2::sortByX( cgUInt32Array * pRemap /* = CG_NULL */ )
{
    // Anything to do?
    if ( mPoints.empty() )
        return false;

    // Was remap information requested? If not, just sort the points
    // directly, otherwise build a remap and re-arrange based on that.
    if ( !pRemap )
    {
        // Standard sort.
        struct SortComparison
        {
            bool operator() ( const SplinePoint & p1, const SplinePoint & p2 )
            {
                return (p1.point.x < p2.point.x);
            }
        } Comparer;
        std::sort( mPoints.begin(), mPoints.end(), Comparer );
        mSplineDirty = true;
        return true;

    } // End if no remap
    else
    {
        struct SortComparison
        {
            // Constructor
            SortComparison( SplinePointArray & _points ) :
                aPoints( _points ) {}

            // Members
            const SplinePointArray & aPoints;

            // Comparison
            bool operator() ( cgUInt32 i1, cgUInt32 i2 )
            {
                return (aPoints[i1].point.x < aPoints[i2].point.x);
            }
        } Comparer( mPoints );

        // Populate remap with initial indices.
        pRemap->resize( mPoints.size() );
        for ( size_t i = 0; i < mPoints.size(); ++i )
            (*pRemap)[i] = i;

        // Sort the remap array
        std::sort( pRemap->begin(), pRemap->end(), Comparer );
        
        // Build new spline points.
        bool bModified = false;
        SplinePointArray aNewPoints( mPoints.size() );
        for ( size_t i = 0; i < mPoints.size(); ++i )
        {
            aNewPoints[i] = mPoints[(*pRemap)[i]];
            bModified |= ( (*pRemap)[i] != i );

        } // Next point

        // Mark additional spline data as dirty if required
        mSplineDirty |= bModified;

        // Swap out old data.
        mPoints.swap( aNewPoints );
        return bModified;

    } // End if remap

}

//-----------------------------------------------------------------------------
// Name : evaluateForX()
/// <summary>
/// Resolve the Y axis value for the given X axis distance. Note: This method
/// will return an invalid result (NaN) if this is a complex spline.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgBezierSpline2::evaluateForX( cgFloat x, bool bApproximate /* = false */, cgUInt16 nDigits /* = 4 */ )
{
    static SplinePoint pt1, pt2;
    static cgUInt32 i, nSegment;

    // Recompute spline data if required.
    if ( mSplineDirty )
        updateSplineData();
    if ( mLength < CGE_EPSILON )
        return mPoints[0].point.y;

    /*// Find the correct segment for this X location
    for ( cgUInt32 i = 0; i < mPoints.size() - 1; ++i )
    {
        const SplinePoint & ptt1 = mPoints[i];
        const SplinePoint & ptt2 = mPoints[i+1];

        if ( x >= ptt1.point.x && x <= ptt2.point.x )
        {
            if ( bApproximate )
            {
                cgFloat fSegmentDist = (ptt2.point.x - ptt1.point.x);
                if ( fSegmentDist < CGE_EPSILON )
                    return evaluateSegment( i, 0.0f ).y;
                else
                    return evaluateSegment( i, (x - ptt1.point.x) / fSegmentDist ).y;

            } // End if approximate
            else
            {
                pt1 = ptt1;
                pt2 = ptt2;
                nSegment = i;
                break;

            } // End if !approximate
        
        } // End if within segment range
    
    } // Next Segment*/

    // Find the correct segment for this X location
    cgInt32 nFirst  = 0;
    cgInt32 nLast   = (cgInt32)mPoints.size() - 1;

    // Compute boundaries for the two candidate keys
    cgFloat fFirstKeyX = mPoints[ nFirst ].point.x;
    cgFloat fLastKeyX  = mPoints[ nLast ].point.x;

    // Test to see if the position is out of range first of all
    if ( x <= fFirstKeyX )
    {
        return mPoints[ nFirst ].point.y;
    
    } // End if prior to first key
    else if ( x >= fLastKeyX )
    {
        return mPoints[ nLast ].point.y;
    
    } // End if after last key
    else
    {
        // Perform a binary search for the correct key starting in the middle
        cgInt32 nCenter = (nFirst + nLast) / 2;
        for ( ; ; )
        {
            // Have we reached the final two keys?
            if ( (nLast - nFirst) < 2 )
            {
                const SplinePoint & ptt1 = mPoints[nFirst];
                const SplinePoint & ptt2 = mPoints[nLast];
                if ( bApproximate )
                {
                    cgFloat fSegmentDist = (ptt2.point.x - ptt1.point.x);
                    if ( fSegmentDist < CGE_EPSILON )
                        return ptt1.point.y;
                    else
                    {
                        // Note: Experiments with improving accuracy of the approximate sampling.
                        /*const double t = (x - ptt1.point.x) / fSegmentDist;                        
                        const double maxpow=4;
                        const double s=t*2-1;
                        const double lerpf=abs(s);
                        const double finalpow = maxpow+((1-maxpow)*lerpf);
                        const double f=1-pow(1-lerpf,finalpow);
                        return evaluateSegment( nFirst, (s>0) ? (f/2)+0.5 : (-f/2)+0.5 ).y;*/

                        return evaluateSegment( nFirst, (x - ptt1.point.x) / fSegmentDist ).y;
                    }

                } // End if approximate
                else
                {
                    pt1 = ptt1;
                    pt2 = ptt2;
                    nSegment = nFirst;
                    break;

                } // End if !approximate

            } // End if final key
            else
            {
                // Comput x for the center key
                fFirstKeyX = mPoints[ nCenter ].point.x;

                if ( x < fFirstKeyX )
                {
                    nLast   = nCenter;
                    nCenter = (nFirst + nLast) / 2;
                
                } // End if prior to center key
                else if ( x > fFirstKeyX )
                {
                    nFirst  = nCenter;
                    nCenter = (nFirst + nLast) / 2;
                
                } // End if after center key
                else
                {
                    return mPoints[ nCenter ].point.y;

                } // End if exact match

            } // End if more testing may be necessary

        } // Next Key

    } // End if not out of range

    // Compute equation coefficients (could be stored, but saves a significant amount of memory this way).
    const cgDouble d = pt1.point.x - x;
    const cgDouble c = 3.0 * (pt1.controlPointOut.x - pt1.point.x);
    const cgDouble b = 3.0 * (pt2.controlPointIn.x - pt1.controlPointOut.x) - c;
    const cgDouble a = pt2.point.x - pt1.point.x - c - b;

    // Solve the cubic equation (or potentially fall back to quadratic if a==0)
    cgInt nRootsFound;
    cgDouble pRoots[3];
    cgMathUtility::solveCubic( a, b, c, d, &nRootsFound, pRoots );
    
    // Return the first root in the correct range.
    for ( cgInt i = 0; i < nRootsFound; ++i )
    {
        // Note: Account for numeric instability
        cgDouble fRoot = pRoots[i];
        if ( fRoot >= -0.01 && fRoot <= 1.01 )
        {
            if ( fRoot > 1.0 ) fRoot = 1.0;
            if ( fRoot < 0.0 ) fRoot = 0.0;
            return evaluateSegment( nSegment, (cgFloat)fRoot ).y;

        } // End if within range

    } // Next root

    // No roots found!
    return std::numeric_limits<cgFloat>::quiet_NaN();
}

//-----------------------------------------------------------------------------
// Name : evaluate()
/// <summary>Resolve the point for the given distance along the spline.</summary>
//-----------------------------------------------------------------------------
cgVector2 cgBezierSpline2::evaluate( cgFloat t )
{
    static cgFloat      BezierBasis[4][4] = { {-1,  3, -3, 1}, { 3, -6,  3, 0}, {-3,  3,  0, 0}, { 1,  0,  0, 0}};
    static SplinePoint  pt1, pt2;
    static cgFloat      t2, t3;
    static cgUInt32     i;

    // Note: While not a perfect method, this gives us the closest approximation of 
    // the original evaluateSegment (and MAX).    

    // Recompute spline data if required.
    if ( mSplineDirty == true )
        updateSplineData();   
    if ( mLength < CGE_EPSILON )
        return mPoints[0].point;

    // Clamp t
    if ( t < 0.0f )
        t = 0.0f;
    if ( t > 1.0f )
        t = 1.0f;

    // Find the two bounding points.
    t *= mLength;
    for ( i = 0; i < mPoints.size() - 1; ++i )
    {
        cgFloat fSegmentLength = (mPointDist[i+1] - mPointDist[i]);
        if ( t >= mPointDist[i] && t <= mPointDist[i+1] )
        {
            pt1 = mPoints[i];
            pt2 = mPoints[i+1];
            t -= mPointDist[i];
            if ( fSegmentLength > CGE_EPSILON )
                t /= fSegmentLength;
            break;
        
        } // End if within segment range
    
    } // Next Segment

    // Bezier Evaluate
    t2 = t*t; t3 = t2*t;
    return pt1.point           * (BezierBasis[0][0]*t3 + BezierBasis[1][0]*t2 + BezierBasis[2][0]*t + BezierBasis[3][0]) +
           pt1.controlPointOut * (BezierBasis[0][1]*t3 + BezierBasis[1][1]*t2 + BezierBasis[2][1]*t + BezierBasis[3][1]) + 
           pt2.controlPointIn  * (BezierBasis[0][2]*t3 + BezierBasis[1][2]*t2 + BezierBasis[2][2]*t + BezierBasis[3][2]) + 
           pt2.point           * (BezierBasis[0][3]*t3 + BezierBasis[1][3]*t2 + BezierBasis[2][3]*t + BezierBasis[3][3]);

}

//-----------------------------------------------------------------------------
// Name : evaluateSegment()
/// <summary>
/// Resolve the point for the given distance along the specified spline
/// segment.
/// </summary>
//-----------------------------------------------------------------------------
cgVector2 cgBezierSpline2::evaluateSegment( cgInt32 nSegment, cgFloat t )
{
    static cgFloat BezierBasis[4][4] = { {-1,  3, -3, 1}, { 3, -6,  3, 0}, {-3,  3,  0, 0}, { 1,  0,  0, 0}};
    static cgFloat t2, t3;

    // Extract segment points
    const SplinePoint & pt1 = mPoints[nSegment];
    const SplinePoint & pt2 = mPoints[nSegment+1];
    
    // Bezier Evaluate
    t2 = t*t; t3 = t2*t;
    return pt1.point           * (BezierBasis[0][0]*t3 + BezierBasis[1][0]*t2 + BezierBasis[2][0]*t + BezierBasis[3][0]) +
           pt1.controlPointOut * (BezierBasis[0][1]*t3 + BezierBasis[1][1]*t2 + BezierBasis[2][1]*t + BezierBasis[3][1]) + 
           pt2.controlPointIn  * (BezierBasis[0][2]*t3 + BezierBasis[1][2]*t2 + BezierBasis[2][2]*t + BezierBasis[3][2]) + 
           pt2.point           * (BezierBasis[0][3]*t3 + BezierBasis[1][3]*t2 + BezierBasis[2][3]*t + BezierBasis[3][3]);
}

//-----------------------------------------------------------------------------
// Name : updateSplineData () (Protected)
/// <summary>
/// Pre-compute certain data necessary for evaluating the spline.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::updateSplineData()
{
    // Bail if spline points are not dirty.
    if ( mSplineDirty == false )
        return;

    // Reset values to be recomputed as necessary.
    mComplexSpline = false;

    // Spline is now empty?
    if ( mPoints.empty() == true )
    {
        mPointDist.clear();
        mLength = 0.0f;
    
    } // End if empty
    else
    {
        // Re-allocate array.
        if ( mPointDist.size() != mPoints.size() )
            mPointDist.resize( mPoints.size() );
        
        // Evaluate each segment for complexity and approximate
        // total spline length.
        mLength = 0.0f;
        bool bFirstSample = true;
        cgVector2 vSamplePos, vPrevSamplePos;
        for ( cgUInt32 i = 0; i < mPoints.size() - 1; ++i )
        {
            // Record distance so far.
            mPointDist[i] = mLength;

            // Sample along the segment.
            for ( cgFloat t = 0; t <= 1.0f; t += 0.01f )
            {
                vSamplePos = evaluateSegment( i, t );
                if ( !bFirstSample )
                {
                    // Complex if turns back on itself?
                    if ( !mComplexSpline && (vSamplePos.x < vPrevSamplePos.x) )
                        mComplexSpline = true;
                    
                    // Increment total length.
                    mLength += cgVector2::length( vSamplePos - vPrevSamplePos );

                } // End if !first
                else
                {
                    bFirstSample = false;
                
                } // End if first

                // Record previous sample position.
                vPrevSamplePos = vSamplePos;
            
            } // Next t

        } // Next Segment
        
        // Record distance to final point
        mPointDist.back() = mLength;

    } // End if valid

    // Spline data is no longer dirty
    mSplineDirty = false;
}

//-----------------------------------------------------------------------------
// Name : getPointCount ()
/// <summary>Determine the number of control points stored.</summary>
//-----------------------------------------------------------------------------
cgInt32 cgBezierSpline2::getPointCount( ) const
{
    return (cgInt32)mPoints.size();
}

//-----------------------------------------------------------------------------
// Name : getSegmentCount ()
/// <summary>Determine the number of segments in spline.</summary>
//-----------------------------------------------------------------------------
cgInt32 cgBezierSpline2::getSegmentCount( ) const
{
    return (mPoints.empty() == true) ? 0 : (cgUInt32)mPoints.size() - 1;
}

//-----------------------------------------------------------------------------
// Name : getSplinePoints ( ) (const overload)
/// <summary>Get the list of control points.</summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2::SplinePointArray & cgBezierSpline2::getSplinePoints( ) const
{
    return mPoints;
}

//-----------------------------------------------------------------------------
// Name : getSplinePoint ( ) (const overload)
/// <summary>Get the specified control point.</summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2::SplinePoint & cgBezierSpline2::getSplinePoint( cgInt32 nIndex ) const
{
    return mPoints[nIndex];
}

//-----------------------------------------------------------------------------
// Name : getSplinePoint ( )
/// <summary>Get the specified control point.</summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::SplinePoint & cgBezierSpline2::getSplinePoint( cgInt32 nIndex )
{
    return mPoints[nIndex];
}

//-----------------------------------------------------------------------------
// Name : setSplinePoint ( )
/// <summary>Update the specified control point.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::setSplinePoint( cgInt32 nIndex, const SplinePoint & pt )
{
    mPoints[nIndex] = pt;
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : isComplex ( )
/// <summary>
/// If spline is invalid for evaluation on X (i.e. the curve folds back on 
/// itself such that it can produce more than one result for a given X) then 
/// this spline is considered complex.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBezierSpline2::isComplex( )
{
    if ( mSplineDirty == true )
        updateSplineData();
    return mComplexSpline;
}

//-----------------------------------------------------------------------------
// Name : getDescription ( )
/// <summary>
/// Get the analyzed description of this spline based on its data.
/// See the SplineDescription enum for valid types.
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline2::SplineDescription cgBezierSpline2::getDescription( ) const
{
    if ( mPoints.size() == 2 )
    {
        
        if ( cgMathUtility::compareVectors( mPoints[0].point, cgVector2( 0.0f, 1.0f ), CGE_EPSILON ) == 0 && 
             cgMathUtility::compareVectors( mPoints[1].point, cgVector2( 1.0f, 0.0f ), CGE_EPSILON ) == 0 )
        {
            // This is potentially linear decay. Extract normalized control points.
            cgVector2 vDirOut = mPoints[0].controlPointOut - mPoints[0].point;
            cgVector2 vDirIn  = mPoints[1].controlPointIn - mPoints[1].point;
            cgFloat fLenOut = cgVector2::length( vDirOut );
            cgFloat fLenIn  = cgVector2::length( vDirIn );
            if ( fLenOut >= CGE_EPSILON )
                vDirOut /= fLenOut;
            if ( fLenIn >= CGE_EPSILON )
                vDirIn /= fLenIn;

            if ( (fLenOut < CGE_EPSILON || cgMathUtility::compareVectors( vDirOut, cgVector2( 0.7071068f, -0.7071068f), CGE_EPSILON ) == 0) &&
                 (fLenIn < CGE_EPSILON || cgMathUtility::compareVectors( vDirIn, cgVector2( -0.7071068f, 0.7071068f), CGE_EPSILON ) == 0) )
                 return LinearDecay;

        } // End if linear decay
        else if ( cgMathUtility::compareVectors( mPoints[0].point, cgVector2( 0.0f, 0.0f ), CGE_EPSILON ) == 0 && 
                  cgMathUtility::compareVectors( mPoints[1].point, cgVector2( 1.0f, 1.0f ), CGE_EPSILON ) == 0 )
        {
            // This is potentially linear growth. Extract normalized control points.
            cgVector2 vDirOut = mPoints[0].controlPointOut - mPoints[0].point;
            cgVector2 vDirIn  = mPoints[1].controlPointIn - mPoints[1].point;
            cgFloat fLenOut = cgVector2::length( vDirOut );
            cgFloat fLenIn  = cgVector2::length( vDirIn );
            if ( fLenOut >= CGE_EPSILON )
                vDirOut /= fLenOut;
            if ( fLenIn >= CGE_EPSILON )
                vDirIn /= fLenIn;

            if ( (fLenOut < CGE_EPSILON || cgMathUtility::compareVectors( vDirOut, cgVector2( 0.7071068f, 0.7071068f), CGE_EPSILON ) == 0) &&
                 (fLenIn < CGE_EPSILON || cgMathUtility::compareVectors( vDirIn, cgVector2( -0.7071068f, -0.7071068f), CGE_EPSILON ) == 0) )
                 return LinearGrowth;

        } // End if linear growth
        else if ( cgMathUtility::compareVectors( mPoints[0].point, cgVector2( 0.0f, 1.0f ), CGE_EPSILON ) == 0 && 
                  cgMathUtility::compareVectors( mPoints[1].point, cgVector2( 1.0f, 1.0f ), CGE_EPSILON ) == 0 )
        {
            // This is potentially maximum. Extract normalized control points.
            cgVector2 vDirOut = mPoints[0].controlPointOut - mPoints[0].point;
            cgVector2 vDirIn  = mPoints[1].controlPointIn - mPoints[1].point;
            cgFloat fLenOut = cgVector2::length( vDirOut );
            cgFloat fLenIn  = cgVector2::length( vDirIn );
            if ( fLenOut >= CGE_EPSILON )
                vDirOut /= fLenOut;
            if ( fLenIn >= CGE_EPSILON )
                vDirIn /= fLenIn;

            if ( (fLenOut < CGE_EPSILON || cgMathUtility::compareVectors( vDirOut, cgVector2( 1.0f, 0.0f ), CGE_EPSILON ) == 0) &&
                 (fLenIn < CGE_EPSILON || cgMathUtility::compareVectors( vDirIn, cgVector2( -1.0f, 0.0f ), CGE_EPSILON ) == 0) )
                 return Maximum;

        } // End if maximum
        else if ( cgMathUtility::compareVectors( mPoints[0].point, cgVector2( 0.0f, 0.0f ), CGE_EPSILON ) == 0 && 
                  cgMathUtility::compareVectors( mPoints[1].point, cgVector2( 1.0f, 0.0f ), CGE_EPSILON ) == 0 )
        {
            // This is potentially minimum. Extract normalized control points.
            cgVector2 vDirOut = mPoints[0].controlPointOut - mPoints[0].point;
            cgVector2 vDirIn  = mPoints[1].controlPointIn - mPoints[1].point;
            cgFloat fLenOut = cgVector2::length( vDirOut );
            cgFloat fLenIn  = cgVector2::length( vDirIn );
            if ( fLenOut >= CGE_EPSILON )
                vDirOut /= fLenOut;
            if ( fLenIn >= CGE_EPSILON )
                vDirIn /= fLenIn;

            if ( (fLenOut < CGE_EPSILON || cgMathUtility::compareVectors( vDirOut, cgVector2( 1.0f, 0.0f ), CGE_EPSILON ) == 0) &&
                 (fLenIn < CGE_EPSILON || cgMathUtility::compareVectors( vDirIn, cgVector2( -1.0f, 0.0f ), CGE_EPSILON ) == 0) )
                 return Minimum;

        } // End if minimum

    } // End if has 2 points
    
    // Unknown spline
    return Custom;

}

//-----------------------------------------------------------------------------
// Name : setDescription ( )
/// <summary>
/// Set the shape of the spline based on the specified description.
/// See the SplineDescription enum for valid types.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline2::setDescription( SplineDescription Desc )
{
    SplinePoint pt;

    // Select new preset.
    switch ( Desc )
    {
        case LinearDecay:
            clear();
            pt.point           = cgVector2( 0.0f, 1.0f );
            pt.controlPointOut = pt.point + cgVector2( 0.7071068f * 0.25f, -0.7071068f * 0.25f );
            pt.controlPointIn  = pt.point + (pt.point - pt.controlPointOut);
            addPoint( pt );
            pt.point           = cgVector2( 1.0f, 0.0f );
            pt.controlPointIn  = pt.point + cgVector2( -0.7071068f * 0.25f, 0.7071068f * 0.25f );
            pt.controlPointOut = pt.point + (pt.point - pt.controlPointIn);
            addPoint( pt );
            break;
        
        case LinearGrowth:
            clear();
            pt.point           = cgVector2( 0.0f, 0.0f );
            pt.controlPointOut = pt.point + cgVector2( 0.7071068f * 0.25f, 0.7071068f * 0.25f );
            pt.controlPointIn  = pt.point + (pt.point - pt.controlPointOut);
            addPoint( pt );
            pt.point           = cgVector2( 1.0f, 1.0f );
            pt.controlPointIn  = pt.point + cgVector2( -0.7071068f * 0.25f, -0.7071068f * 0.25f );
            pt.controlPointOut = pt.point + (pt.point - pt.controlPointIn);
            addPoint( pt );
            break;
        
        case Maximum:
            clear();
            pt.point           = cgVector2( 0.0f, 1.0f );
            pt.controlPointOut = pt.point + cgVector2( 1.0f * 0.25f, 0.0f );
            pt.controlPointIn  = pt.point + (pt.point - pt.controlPointOut);
            addPoint( pt );
            pt.point           = cgVector2( 1.0f, 1.0f );
            pt.controlPointIn  = pt.point + cgVector2( -1.0f * 0.25f, 0.0f );
            pt.controlPointOut = pt.point + (pt.point - pt.controlPointIn);
            addPoint( pt );
            break;
        
        case Minimum:
            clear();
            pt.point           = cgVector2( 0.0f, 0.0f );
            pt.controlPointOut = pt.point + cgVector2( 1.0f * 0.25f, 0.0f );
            pt.controlPointIn  = pt.point + (pt.point - pt.controlPointOut);
            addPoint( pt );
            pt.point           = cgVector2( 1.0f, 0.0f );
            pt.controlPointIn  = pt.point + cgVector2( -1.0f * 0.25f, 0.0f );
            pt.controlPointOut = pt.point + (pt.point - pt.controlPointIn);
            addPoint( pt );
            break;

    } // End Switch
}

//-----------------------------------------------------------------------------
//  Name : computeHash()
/// <summary>
/// Compute a hash string for the curve. Control point values will be 
/// encoded directly into the string based on the number of significant
/// digits required (allows hashes to be purposefully "collapsed" in some
/// cases).
/// </summary>
//-----------------------------------------------------------------------------
cgString cgBezierSpline2::computeHash( cgInt16 nNumSignificantDigits /* = -1 */ )
{
    cgString strHash, strSpecifier;

    // No-op?
    if ( mPoints.empty() == true )
        return _T("NULL");

    // Data needs re-generating?
    if ( mSplineDirty == true )
        updateSplineData();
    
    // Compute format specifier (i.e. %.2f)
    if ( nNumSignificantDigits >= 0 )
        strSpecifier = cgString::format( _T("%%.%if,%%.%if"), nNumSignificantDigits, nNumSignificantDigits );
    else
        strSpecifier = _T("%g,%g");

    // Process each control point
    for ( size_t i = 0; i < mPoints.size(); ++i )
    {
        if ( i > 0 )
        {
            strHash += cgString::format( strSpecifier.c_str(), mPoints[i].controlPointIn.x, mPoints[i].controlPointIn.y );
            strHash += _T("|");
        
        } // End if not first point
        strHash += cgString::format( strSpecifier.c_str(), mPoints[i].point.x, mPoints[i].point.y );
        if ( i < mPoints.size() - 1 )
        {
            strHash += _T("|");
            strHash += cgString::format( strSpecifier.c_str(), mPoints[i].controlPointOut.x, mPoints[i].controlPointOut.y );
            strHash += _T("|");
        
        } // End if not last point
        
    } // Next point

    // Success!
    return strHash;
}

///////////////////////////////////////////////////////////////////////////////
// cgBezierSpline3 Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBezierSpline3 () (Default Constructor)
/// <summary>
/// cgBezierSpline3 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline3::cgBezierSpline3( ) 
{
	// Default to linear attenuation
    mSplineDirty   = false;
    mLength        = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgBezierSpline3 () (Constructor)
/// <summary>
/// cgBezierSpline3 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline3::cgBezierSpline3( const SplinePointArray & SplinePoints )
{
    mPoints        = SplinePoints;
    mSplineDirty   = true;
    mLength        = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgBezierSpline3 () (Constructor)
/// <summary>
/// cgBezierSpline3 Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline3::cgBezierSpline3( const cgBezierSpline3 & Spline ) 
{
	*this = Spline;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBezierSpline3 () (Destructor)
/// <summary>
/// cgBezierSpline3 Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline3::~cgBezierSpline3( ) 
{
    // Release any allocated memory
	clear();
}

//-----------------------------------------------------------------------------
//  Name : operator= () (cgBezierSpline3&)
/// <summary>
/// Overloaded assignment operator.
/// </summary>
//-----------------------------------------------------------------------------
cgBezierSpline3 & cgBezierSpline3::operator=( const cgBezierSpline3 & Spline )
{
    // Is this a no-op?
    if ( &Spline == this )
        return *this;

    // Clear out any old data
    clear();

    // Copy data.
    mPoints           = Spline.mPoints;
    mPointDist        = Spline.mPointDist;
    mLength           = Spline.mLength;
    mSplineDirty      = Spline.mSplineDirty;
    
    // Return reference to self in order to allow multiple assignments (i.e. a=b=c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator== () (const cgBezierSpline3&)
/// <summary>
/// Test for equality with the specified spline.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBezierSpline3::operator==( const cgBezierSpline3 & spline ) const
{
    if (mPoints.size() != spline.mPoints.size())
        return false;
    if ( mPoints.empty() )
        return true;
    return (memcmp( &mPoints.front(), &spline.mPoints.front(), mPoints.size() * sizeof(SplinePoint)) == 0);
}

//-----------------------------------------------------------------------------
// Name : addPoint()
/// <summary>Add the specified control point.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::addPoint( const SplinePoint & pt )
{
    // Add point to end of list.
    mPoints.push_back( pt );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : insertPoint()
/// <summary>
/// Insert the specified control point at the referenced location.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::insertPoint( cgInt32 nInsertAt, const SplinePoint & pt )
{
    // Validate operation
    if ( nInsertAt < 0 || nInsertAt > (cgInt32)mPoints.size() )
        return;

    // Insert into array.
    mPoints.insert( mPoints.begin() + nInsertAt, pt );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : removePoint()
/// <summary>
/// Remove the control point at the referenced location.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::removePoint( cgInt32 nIndex )
{
    // Validate operation
    if ( nIndex < 0 || nIndex >= (cgInt32)mPoints.size() )
        return;

    // Shift all control points down
    mPoints.erase( mPoints.begin() + nIndex );
    mSplineDirty = true;
}

//-----------------------------------------------------------------------------
// Name : clear()
/// <summary>Clear all control points.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::clear( )
{
    // Resize array
    mPoints.clear();
    mPointDist.clear();
    mSplineDirty   = true;
}

//-----------------------------------------------------------------------------
// Name : evaluate()
/// <summary>Resolve the point for the given distance along the spline.</summary>
//-----------------------------------------------------------------------------
cgVector3 cgBezierSpline3::evaluate( cgFloat t )
{
    static cgFloat      BezierBasis[4][4] = { {-1,  3, -3, 1}, { 3, -6,  3, 0}, {-3,  3,  0, 0}, { 1,  0,  0, 0}};
    static SplinePoint  pt1, pt2;
    static cgFloat      t2, t3;
    static cgUInt32     i;

    // Note: While not a perfect method, this gives us the closest approximation of 
    // the original evaluateSegment (and MAX).    

    // Recompute spline data if required.
    if ( mSplineDirty == true )
        updateSplineData();   
    if ( mLength < CGE_EPSILON )
        return mPoints[0].point;

    // Clamp t
    if ( t < 0.0f )
        t = 0.0f;
    if ( t > 1.0f )
        t = 1.0f;

    // Find the two bounding points.
    t *= mLength;
    for ( i = 0; i < mPoints.size() - 1; ++i )
    {
        cgFloat fSegmentLength = (mPointDist[i+1] - mPointDist[i]);
        if ( t >= mPointDist[i] && t <= mPointDist[i+1] )
        {
            pt1 = mPoints[i];
            pt2 = mPoints[i+1];
            t -= mPointDist[i];
            if ( fSegmentLength > CGE_EPSILON )
                t /= fSegmentLength;
            break;
        
        } // End if within segment range
    
    } // Next Segment

    // Bezier Evaluate
    t2 = t*t; t3 = t2*t;
    return pt1.point           * (BezierBasis[0][0]*t3 + BezierBasis[1][0]*t2 + BezierBasis[2][0]*t + BezierBasis[3][0]) +
           pt1.controlPointOut * (BezierBasis[0][1]*t3 + BezierBasis[1][1]*t2 + BezierBasis[2][1]*t + BezierBasis[3][1]) + 
           pt2.controlPointIn  * (BezierBasis[0][2]*t3 + BezierBasis[1][2]*t2 + BezierBasis[2][2]*t + BezierBasis[3][2]) + 
           pt2.point           * (BezierBasis[0][3]*t3 + BezierBasis[1][3]*t2 + BezierBasis[2][3]*t + BezierBasis[3][3]);

}

//-----------------------------------------------------------------------------
// Name : evaluateSegment()
/// <summary>
/// Resolve the point for the given distance along the specified spline
/// segment.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBezierSpline3::evaluateSegment( cgInt32 nSegment, cgFloat t )
{
    static cgFloat BezierBasis[4][4] = { {-1,  3, -3, 1}, { 3, -6,  3, 0}, {-3,  3,  0, 0}, { 1,  0,  0, 0}};
    static cgFloat t2, t3;

    // Extract segment points
    const SplinePoint & pt1 = mPoints[nSegment];
    const SplinePoint & pt2 = mPoints[nSegment+1];
    
    // Bezier Evaluate
    t2 = t*t; t3 = t2*t;
    return pt1.point           * (BezierBasis[0][0]*t3 + BezierBasis[1][0]*t2 + BezierBasis[2][0]*t + BezierBasis[3][0]) +
           pt1.controlPointOut * (BezierBasis[0][1]*t3 + BezierBasis[1][1]*t2 + BezierBasis[2][1]*t + BezierBasis[3][1]) + 
           pt2.controlPointIn  * (BezierBasis[0][2]*t3 + BezierBasis[1][2]*t2 + BezierBasis[2][2]*t + BezierBasis[3][2]) + 
           pt2.point           * (BezierBasis[0][3]*t3 + BezierBasis[1][3]*t2 + BezierBasis[2][3]*t + BezierBasis[3][3]);
}

//-----------------------------------------------------------------------------
// Name : updateSplineData () (Protected)
/// <summary>
/// Pre-compute certain data necessary for evaluating the spline.
/// </summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::updateSplineData()
{
    // Bail if spline points are not dirty.
    if ( mSplineDirty == false )
        return;

    // Spline is now empty?
    if ( mPoints.empty() == true )
    {
        mPointDist.clear();
        mLength = 0.0f;
    
    } // End if empty
    else
    {
        // Re-allocate array.
        if ( mPointDist.size() != mPoints.size() )
            mPointDist.resize( mPoints.size() );
        
        // Evaluate each segment to approximate total spline length.
        mLength = 0.0f;
        bool bFirstSample = true;
        cgVector3 vSamplePos, vPrevSamplePos;
        for ( cgUInt32 i = 0; i < mPoints.size() - 1; ++i )
        {
            // Record distance so far.
            mPointDist[i] = mLength;

            // Sample along the segment.
            for ( cgFloat t = 0; t <= 1.0f; t += 0.01f )
            {
                vSamplePos = evaluateSegment( i, t );
                if ( !bFirstSample )
                    mLength += cgVector3::length( vSamplePos - vPrevSamplePos );
                else
                    bFirstSample = false;

                // Record previous sample position.
                vPrevSamplePos = vSamplePos;
            
            } // Next t

        } // Next Segment
        
        // Record distance to final point
        mPointDist.back() = mLength;

    } // End if valid

    // Spline data is no longer dirty
    mSplineDirty = false;
}

//-----------------------------------------------------------------------------
// Name : getPointCount ()
/// <summary>Determine the number of control points stored.</summary>
//-----------------------------------------------------------------------------
cgInt32 cgBezierSpline3::getPointCount( ) const
{
    return (cgInt32)mPoints.size();
}

//-----------------------------------------------------------------------------
// Name : getSegmentCount ()
/// <summary>Determine the number of segments in spline.</summary>
//-----------------------------------------------------------------------------
cgInt32 cgBezierSpline3::getSegmentCount( ) const
{
    return (mPoints.empty() == true) ? 0 : (cgUInt32)mPoints.size() - 1;
}

//-----------------------------------------------------------------------------
// Name : getSplinePoints ( ) (const overload)
/// <summary>Get the list of control points.</summary>
//-----------------------------------------------------------------------------
const cgBezierSpline3::SplinePointArray & cgBezierSpline3::getSplinePoints( ) const
{
    return mPoints;
}

//-----------------------------------------------------------------------------
// Name : getSplinePoint ( ) (const overload)
/// <summary>Get the specified control point.</summary>
//-----------------------------------------------------------------------------
const cgBezierSpline3::SplinePoint & cgBezierSpline3::getSplinePoint( cgInt32 nIndex ) const
{
    return mPoints[nIndex];
}

//-----------------------------------------------------------------------------
// Name : getSplinePoint ( )
/// <summary>Get the specified control point.</summary>
//-----------------------------------------------------------------------------
cgBezierSpline3::SplinePoint & cgBezierSpline3::getSplinePoint( cgInt32 nIndex )
{
    return mPoints[nIndex];
}

//-----------------------------------------------------------------------------
// Name : setSplinePoint ( )
/// <summary>Update the specified control point.</summary>
//-----------------------------------------------------------------------------
void cgBezierSpline3::setSplinePoint( cgInt32 nIndex, const SplinePoint & pt )
{
    mPoints[nIndex] = pt;
    mSplineDirty = true;
}