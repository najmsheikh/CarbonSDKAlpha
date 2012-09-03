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
// Name : cgExtrudedBoundingBox.cpp                                          //
//                                                                           //
// Desc : Provides support for an "extruded" bounding box that, similar to   //
//        shadow volumes, extrudes an axis aligned bounding box by a         //
//        specified amount away from a single projection point.              //
//                                                                           //
// Note : Special thanks to NVIDIA Corporation for the original              //
//        implementation on which this class is based.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgExtrudedBoundingBox Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgExtrudedBoundingBox.h>
#include <Math/cgBoundingBox.h>

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    //  Friendly names for bit masked fields used throughout the various functions.
    //  This list is a 3-bit field which defines 1 of 8 points in an axis-aligned bounding box.
    enum
    {
	    MIN_X = 1,
	    MAX_X = 0,
	    MIN_Y = 2,
	    MAX_Y = 0,
	    MIN_Z = 4,
	    MAX_Z = 0,
	    INVALID = -1 
    };

    const int HalfSpaceRemap[64] = { 
	    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	    -1, -1, -1, -1, -1,  0,  1,  2, -1,  3,  4,  5, -1,  6,  7,  8,
	    -1, -1, -1, -1, -1,  9, 10, 11, -1, 12, 13, 14, -1, 15, 16, 17,
	    -1, -1, -1, -1, -1, 18, 19, 20, -1, 21, 22, 23, -1, 24, 25, -1
    };

    // Lookup table that stores which edges will be extruded for a given half-space configuration.
    const cgUInt32 SilhouetteLUT[26][6][2] = {
        { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z } },
	    { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z } },
	    { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z } },
	    { { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { INVALID, INVALID }, { INVALID, INVALID } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z } },
	    { { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z } },
	    { { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z } },
	    { { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z } },
	    { { MIN_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { INVALID, INVALID }, { INVALID, INVALID } },
	    { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z } },
	    { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z } },
	    { { MIN_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MAX_Y|MAX_Z }, { INVALID, INVALID }, { INVALID, INVALID } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z } },
	    { { MAX_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MIN_Y|MAX_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MIN_Y|MIN_Z }, { INVALID, INVALID }, { INVALID, INVALID } },
	    { { MAX_X|MIN_Y|MIN_Z, MAX_X|MIN_Y|MAX_Z }, { MAX_X|MAX_Y|MAX_Z, MAX_X|MAX_Y|MIN_Z }, { MAX_X|MAX_Y|MIN_Z, MAX_X|MIN_Y|MIN_Z }, { MAX_X|MIN_Y|MAX_Z, MAX_X|MAX_Y|MAX_Z }, { INVALID, INVALID }, { INVALID, INVALID } },
	    { { MIN_X|MIN_Y|MAX_Z, MIN_X|MIN_Y|MIN_Z }, { MIN_X|MAX_Y|MIN_Z, MIN_X|MAX_Y|MAX_Z }, { MIN_X|MIN_Y|MIN_Z, MIN_X|MAX_Y|MIN_Z }, { MIN_X|MAX_Y|MAX_Z, MIN_X|MIN_Y|MAX_Z }, { INVALID, INVALID }, { INVALID, INVALID } }
    };

} // End Unnamed Namespace

///////////////////////////////////////////////////////////////////////////////
// cgExtrudedBoundingBox Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgExtrudedBoundingBox () (Default Constructor)
/// <summary>
/// cgExtrudedBoundingBox Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgExtrudedBoundingBox::cgExtrudedBoundingBox( ) 
{
	// Initialise values
    reset();
}

//-----------------------------------------------------------------------------
//  Name : cgExtrudedBoundingBox () (Constructor)
/// <summary>
/// cgExtrudedBoundingBox Class Constructor, sets values from vector values passed
/// </summary>
//-----------------------------------------------------------------------------
cgExtrudedBoundingBox::cgExtrudedBoundingBox( const cgBoundingBox & AABB, const cgVector3 & vecOrigin, cgFloat fRange, const cgTransform * pTransform /* = CG_NULL */ )
{
    // Generate extrude box values automatically
    extrude( AABB, vecOrigin, fRange, pTransform );
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Resets the bounding box values.
/// </summary>
//-----------------------------------------------------------------------------
void cgExtrudedBoundingBox::reset()
{
    sourceMin       = cgVector3( 0, 0, 0 );
	sourceMax       = cgVector3( 0, 0, 0 );
    projectionPoint = cgVector3( 0, 0, 0 );
    projectionRange = 0.0f;
	edgeCount       = 0;
}

//-----------------------------------------------------------------------------
//  Name : extrude()
/// <summary>
/// Generate the extruded box planes / edges.
/// </summary>
//-----------------------------------------------------------------------------
void cgExtrudedBoundingBox::extrude( const cgBoundingBox & AABB, const cgVector3 & vecOrigin, cgFloat fRange, const cgTransform * pTransform /* = CG_NULL */ )
{
    cgBoundingBox Bounds = AABB;

    // Reset any previously computed data
	reset();

    // Transform bounds if matrix provided
    if ( pTransform != CG_NULL )
        Bounds.transform( *pTransform );

    // Make a copy of the values used to generate this box
    sourceMin       = Bounds.min;
    sourceMax       = Bounds.max;
    projectionPoint = vecOrigin;
    projectionRange = fRange;

    // Simple comparisons of the bounding box against the projection origin quickly determine which
    // halfspaces the box exists in. From this, look up values from the precomputed table that
    // define which edges of the bounding box are silhouettes.
	cgUInt32 nHalfSpace = 0;
    if ( sourceMin.x <= vecOrigin.x ) nHalfSpace |= 0x1;
	if ( sourceMax.x >= vecOrigin.x ) nHalfSpace |= 0x2;
	if ( sourceMin.y <= vecOrigin.y ) nHalfSpace |= 0x4;
	if ( sourceMax.y >= vecOrigin.y ) nHalfSpace |= 0x8;
	if ( sourceMin.z <= vecOrigin.z ) nHalfSpace |= 0x10;
	if ( sourceMax.z >= vecOrigin.z ) nHalfSpace |= 0x20;

    // Lookup the appropriate silhouette edges
    cgUInt32 nRemap = HalfSpaceRemap[nHalfSpace];
    for ( edgeCount = 0; (SilhouetteLUT[nRemap][edgeCount][0] != INVALID) && (edgeCount < 6); )
    {
        cgVector3 vPoint1, vPoint2;
		
        // Lookup correct points to use
        cgUInt32 nPoint1 = SilhouetteLUT[nRemap][edgeCount][0];
		cgUInt32 nPoint2 = SilhouetteLUT[nRemap][edgeCount][1];
		
        // Select the actual point coordinates
        vPoint1.x = (nPoint1 & MIN_X) ? sourceMin.x : sourceMax.x;
		vPoint1.y = (nPoint1 & MIN_Y) ? sourceMin.y : sourceMax.y;
		vPoint1.z = (nPoint1 & MIN_Z) ? sourceMin.z : sourceMax.z;
		vPoint2.x = (nPoint2 & MIN_X) ? sourceMin.x : sourceMax.x;
		vPoint2.y = (nPoint2 & MIN_Y) ? sourceMin.y : sourceMax.y;
		vPoint2.z = (nPoint2 & MIN_Z) ? sourceMin.z : sourceMax.z;
		
        // Record the edge data we used
		silhouetteEdges[edgeCount][0] = nPoint1;
		silhouetteEdges[edgeCount][1] = nPoint2;
		cgPlane::fromPoints( extrudedPlanes[edgeCount++], vecOrigin, vPoint1, vPoint2 );

    } // Next Potential

}

//-----------------------------------------------------------------------------
//  Name : getEdge()
/// <summary>
/// Retrieve the specified edge points.
/// </summary>
//-----------------------------------------------------------------------------
bool cgExtrudedBoundingBox::getEdge( cgUInt32 nEdge, cgVector3 & vPoint1, cgVector3 & vPoint2 ) const
{
    // Valid index?
    if ( nEdge >= edgeCount )
        return false;

    // Lookup correct points to use
    cgUInt32 nPoint1 = silhouetteEdges[nEdge][0];
	cgUInt32 nPoint2 = silhouetteEdges[nEdge][1];
	
    // Select the actual point coordinates
    vPoint1.x = (nPoint1 & MIN_X) ? sourceMin.x : sourceMax.x;
	vPoint1.y = (nPoint1 & MIN_Y) ? sourceMin.y : sourceMax.y;
	vPoint1.z = (nPoint1 & MIN_Z) ? sourceMin.z : sourceMax.z;
	vPoint2.x = (nPoint2 & MIN_X) ? sourceMin.x : sourceMax.x;
	vPoint2.y = (nPoint2 & MIN_Y) ? sourceMin.y : sourceMax.y;
	vPoint2.z = (nPoint2 & MIN_Z) ? sourceMin.z : sourceMax.z;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : testLine ()
/// <summary>
/// Determine whether or not the line passed is within the box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgExtrudedBoundingBox::testLine( const cgVector3 & v1, const cgVector3 & v2 ) const
{
    cgUInt8   nCode1 = 0, nCode2 = 0;
    cgFloat   fDist1, fDist2, t;
    int       nSide1, nSide2;
    cgVector3 vDir, vIntersect;
    cgUInt32  i;

    // Test each plane
    for ( i = 0; i < edgeCount; ++i )
    {
        // Classify each point of the line against the plane.
        fDist1 = cgPlane::dotCoord( extrudedPlanes[i], v1 );
        fDist2 = cgPlane::dotCoord( extrudedPlanes[i], v2 );
        nSide1 = (fDist1 >= 0) ? 1 : 0;
        nSide2 = (fDist2 >= 0) ? 1 : 0;

        // Accumulate the classification info to determine 
        // if the edge was spanning any of the planes.
        nCode1 |= (nSide1 << i);
        nCode2 |= (nSide2 << i);

        // If the line is completely in front of any plane
        // then it cannot possibly be intersecting.
        if ( nSide1 == 1 && nSide2 == 1 )
            return false;

        // The line is potentially spanning?
        if ( nSide1 ^ nSide2 )
        {
            // Compute the point at which the line intersects this plane.
            vDir = v2 - v1;
            t    = -cgPlane::dotCoord( extrudedPlanes[i], v1 ) / cgPlane::dotNormal( extrudedPlanes[i], vDir );
            
            // Truly spanning?
            if ( (t >= 0.0f) && (t <= 1.0f) )
            {
                vIntersect = v1 + (vDir * t);
                if ( testSphere( vIntersect, 0.01f ) )
                    return true;
            
            } // End if spanning
        
        } // End if different sides
    
    } // Next Plane
    
    // Intersecting?
    return (nCode1 == 0) || (nCode2 == 0);
}

//-----------------------------------------------------------------------------
//  Name : testSphere ()
/// <summary>
/// Determine whether or not the sphere passed is within the box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgExtrudedBoundingBox::testSphere( const cgVector3 & vecCenter, cgFloat fRadius ) const
{
    cgUInt32 i;

    // Test box planes
    for ( i = 0; i < edgeCount; ++i )
    {
        cgFloat fDot = cgPlane::dotCoord( extrudedPlanes[i], vecCenter );

        // Sphere entirely in front of plane
        if ( fDot >= fRadius )
            return false;

    } // Next Plane

    // Intersects
    return true;
}