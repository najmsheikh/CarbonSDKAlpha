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
// Name : cgCollision.cpp                                                    //
//                                                                           //
// Desc: Our collision detection / scene library. Provides us with collision //
//       detection and response routines for use throughout our application. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCollision Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgCollision.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgVertexFormats.h>
#include <algorithm>

//-----------------------------------------------------------------------------
// Static Utility functions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : pointInTriangle () (Static)
/// <summary>
/// Test to see if a point falls within the bounds of a triangle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::pointInTriangle( const cgVector3& Point, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& TriNormal, float fDistTolerance /* = 0.0f */ )
{
    cgVector3 Edge, EdgeNormal, Direction;

    // First edge
    Edge      = v2 - v1;
    Direction = v1 - Point;
    cgVector3::cross( EdgeNormal, Edge, TriNormal );

    // In front of edge?
    if ( fDistTolerance != 0.0f ) cgVector3::normalize( EdgeNormal, EdgeNormal );
    if ( cgVector3::dot( Direction, EdgeNormal ) < -fDistTolerance ) return false;

    // Second edge
    Edge      = v3 - v2;
    Direction = v2 - Point;
    cgVector3::cross( EdgeNormal, Edge, TriNormal );

    // In front of edge?
    if ( fDistTolerance != 0.0f ) cgVector3::normalize( EdgeNormal, EdgeNormal );
    if ( cgVector3::dot( Direction, EdgeNormal ) < -fDistTolerance ) return false;

    // Third edge
    Edge      = v1 - v3;
    Direction = v3 - Point;
    cgVector3::cross( EdgeNormal, Edge, TriNormal );

    // In front of edge?
    if ( fDistTolerance != 0.0f ) cgVector3::normalize( EdgeNormal, EdgeNormal );
    if ( cgVector3::dot( Direction, EdgeNormal ) < -fDistTolerance ) return false;

    // We are behind all planes
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pointInTriangle () (Static, Overload)
/// <summary>
/// Test to see if a point falls within the bounds of a triangle.
/// Note : This is an overload function for cases where no normal is provided.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::pointInTriangle( const cgVector3& Point, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, float fDistTolerance /* = 0.0f */ )
{
    cgVector3 Normal;

    // Generate the triangle normal
    cgVector3::cross( Normal, v2 - v1, v3 - v1 );
    cgVector3::normalize( Normal, Normal );

    // Pass through to standard function
    return cgCollision::pointInTriangle( Point, v1, v2, v3, Normal, fDistTolerance );
}

//-----------------------------------------------------------------------------
//  Name : pointInAABB () (Static)
/// <summary>
/// Determine if the specified point falls within the axis aligned
/// bounding box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::pointInAABB( const cgVector3& Point, const cgBoundingBox & AABB, bool bIgnoreX /* = false */, bool bIgnoreY /* = false */, bool bIgnoreZ /* = false */ )
{
    // Does the point fall outside any of the AABB planes?
    if ( !bIgnoreX && (Point.x < AABB.min.x || Point.x > AABB.max.x) ) return false;
    if ( !bIgnoreY && (Point.y < AABB.min.y || Point.y > AABB.max.y) ) return false;
    if ( !bIgnoreZ && (Point.z < AABB.min.z || Point.z > AABB.max.z) ) return false;
    
    // We are behind all planes
    return true;
}

//-----------------------------------------------------------------------------
//  Name : rayIntersectPlane () (Static)
/// <summary>
/// Determine if the specified ray intersects the plane, and if so
/// at what 't' value.
/// Form : 't = ((P - O) . N) / (V . N)' where:
/// P = Ray Origin, O = Point on Plane, V = Ray Velocity, N = Plane Normal
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectPlane( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& PlaneNormal, const cgVector3& PlanePoint, cgFloat& t, bool BiDirectional /* = false */, bool RestrictRange /* = true */ )
{
    // Get the length of the 'adjacent' side of the virtual triangle formed
    // by the velocity and normal.
    cgFloat ProjRayLength = cgVector3::dot( Velocity, PlaneNormal );

    // If they are pointing in the same direction, we're can't possibly be colliding (i.e.
    // the dot product returned a positive value, with epsilon testing ). Remember that
    // our Velocity and Plane normal vectors are pointing towards one another in the case
    // where the two can be colliding (i.e. cos(a) >= -1 and <= 0).
    if ( BiDirectional == false && ProjRayLength > -CGE_EPSILON )
        return false;

    // Ray is parallel to plane?
	if ( fabsf( ProjRayLength ) < CGE_EPSILON )
        return false;

    // Since the interval (0 - 1) for intersection will be the same regardless of the distance
    // to the plane along either the hypotenuse OR the adjacent side of the triangle (bearing
    // in mind that we do 'distance / length_of_edge', and the length of the adjacent side is
    // relative to the length of the hypotenuse), we can just calculate the distance to the plane
    // the 'quick and easy way'(tm) ('shortest distance to' or 'distance along the negative plane normal')
    cgFloat Distance = cgVector3::dot( Origin - PlanePoint, PlaneNormal );

    // Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
    t = Distance / -ProjRayLength;

    // Outside our valid range? If yes, return no collide.
    if ( t < 0.0f || (RestrictRange == true && t > 1.0f) )
        return false;

    // We're intersecting
    return true;
}

//-----------------------------------------------------------------------------
//  Name : rayIntersectPlane () (Static)
/// <summary>
/// Overload of previous function, accepting a plane distance instead.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectPlane( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& PlaneNormal, cgFloat PlaneDistance, cgFloat& t, bool BiDirectional /* = false */, bool RestrictRange /* = true */ )
{
    // Get the length of the 'adjacent' side of the virtual triangle formed
    // by the velocity and normal.
    cgFloat ProjRayLength = cgVector3::dot( Velocity, PlaneNormal );
    if ( BiDirectional == false && ProjRayLength > -CGE_EPSILON )
        return false;
    if ( fabsf( ProjRayLength ) < CGE_EPSILON )
        return false;

    // Calculate distance to plane along it's normal
    cgFloat Distance = cgVector3::dot( Origin, PlaneNormal ) + PlaneDistance;

    // Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
    t = Distance / -ProjRayLength;

    // Outside our valid range? If yes, return no collide.
    if ( t < 0.0f || (RestrictRange == true && t > 1.0f) )
        return false;

    // We're intersecting
    return true;
}

//-----------------------------------------------------------------------------
//  Name : rayIntersectPlane () (Static)
/// <summary>
/// Overload of previous function, accepting a full plane.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectPlane( const cgVector3& Origin, const cgVector3& Velocity, const cgPlane& Plane, cgFloat& t, bool BiDirectional /* = false */, bool RestrictRange /* = true */ )
{
    // Get the length of the 'adjacent' side of the virtual triangle formed
    // by the velocity and normal.
    cgFloat ProjRayLength = cgVector3::dot( Velocity, (cgVector3&)Plane );
    if ( BiDirectional == false && ProjRayLength > -CGE_EPSILON )
        return false;
    if ( fabsf( ProjRayLength ) < CGE_EPSILON )
        return false;

    // Calculate distance to plane along it's normal
    cgFloat Distance = cgVector3::dot( Origin, (cgVector3&)Plane ) + Plane.d;

    // Calculate the actual interval (Distance along the adjacent side / length of adjacent side).
    t = Distance / -ProjRayLength;

    // Outside our valid range? If yes, return no collide.
    if ( t < 0.0f || (RestrictRange == true && t > 1.0f) )
        return false;

    // We're intersecting
    return true;
}

//-----------------------------------------------------------------------------
//  Name : rayIntersectPlane () (Static)
/// <summary>
/// Determine if the specified ray intersects the triangle, and if so
/// at what 't' value.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectTriangle( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& TriNormal, cgFloat& t, cgFloat fTolerance /* = 0*/, bool BiDirectional /* = false */, bool RestrictRange /* = true */ )
{
    cgVector3 Point;

    // First calculate the intersection point with the triangle plane
    if ( !cgCollision::rayIntersectPlane( Origin, Velocity, TriNormal, v1, t, BiDirectional, RestrictRange ) )
        return false;

    // Calculate the intersection point on the plane
    Point = Origin + (Velocity * t);

    // If this point does not fall within the bounds of the triangle, we are not intersecting
    if ( !cgCollision::pointInTriangle( Point, v1, v2, v3, TriNormal, fTolerance ) )
        return false;

    // We're intersecting the triangle
    return true;

}

//-----------------------------------------------------------------------------
//  Name : rayIntersectTriangle () (Static, Overload)
/// <summary>
/// Determine if the specified ray intersects the triangle, and if so
/// at what 't' value.
/// Note : This overload is called if no normal is available.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectTriangle( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, cgFloat& t, cgFloat fTolerance /* = 0*/, bool BiDirectional /* = false */, bool RestrictRange /* = true */ )
{
    cgVector3 Normal;

    // Generate the triangle normal
    cgVector3::cross( Normal, v2 - v1, v3 - v1 );
    cgVector3::normalize( Normal, Normal );

    // Pass through to standard function
    return cgCollision::rayIntersectTriangle( Origin, Velocity, v1, v2, v3, Normal, t, fTolerance, BiDirectional, RestrictRange );
}

//-----------------------------------------------------------------------------
//  Name : rayIntersectAABB () (Static)
/// <summary>
/// This function test to see if a ray intersects the specified axis
/// aligned bounding box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectAABB( const cgVector3& Origin, const cgVector3& Velocity, const cgBoundingBox & AABB, cgFloat& t, bool bIgnoreX /* = false */, bool bIgnoreY /* = false */, bool bIgnoreZ /* = false */, bool bRestrictRange /* = true */ )
{
    cgFloat tMin = -FLT_MAX;
    cgFloat tMax = FLT_MAX;
    cgFloat t1, t2, fTemp;
    
	// If ray origin is inside bounding box, just return true (treat AABB as a solid box)
	if ( pointInAABB( Origin, AABB, bIgnoreX, bIgnoreY, bIgnoreZ ) ) 
	{
		t = 0.0f;
		return true;
	
    } // End if point in box

    // X Slabs
    if ( bIgnoreX == false )
    {
        // Is it pointing toward?
        if ( fabsf(Velocity.x) > CGE_EPSILON )
        {
            fTemp = 1.0f / Velocity.x;
            t1    = (AABB.max.x - Origin.x) * fTemp;
            t2    = (AABB.min.x - Origin.x) * fTemp;
            
            // Reorder if necessary
            if (t1 > t2) { fTemp = t1; t1 = t2; t2 = fTemp; }
            
            // Compare and validate
            if (t1 > tMin) tMin = t1;
            if (t2 < tMax) tMax = t2;
            if (tMin > tMax) return false;
            if (tMax < 0) return false;
        
        } // End if
        else
        {
            // We cannot be intersecting in this case if the origin is outside of the slab
            //if ( Origin.x < (AABB.min.x - Origin.x) || Origin.x > (AABB.max.x - Origin.x) )
            if ( Origin.x < AABB.min.x || Origin.x > AABB.max.x )
                return false;
        
        } // End else

    } // End if process X
    
    // Y Slabs
    if ( bIgnoreY == false )
    {
        // Is it pointing toward?
        if ( fabsf(Velocity.y) > CGE_EPSILON )
        {
            fTemp = 1.0f / Velocity.y;
            t1    = (AABB.max.y - Origin.y) * fTemp;
            t2    = (AABB.min.y - Origin.y) * fTemp;
            
            // Reorder if necessary
            if (t1 > t2) { fTemp = t1; t1 = t2; t2 = fTemp; }
            
            // Compare and validate
            if (t1 > tMin) tMin = t1;
            if (t2 < tMax) tMax = t2;
            if (tMin > tMax) return false;
            if (tMax < 0) return false;
        
        } // End if
        else
        {
            // We cannot be intersecting in this case if the origin is outside of the slab
            //if ( Origin.y < (AABB.min.y - Origin.y) || Origin.y > (AABB.max.y - Origin.y) )
            if ( Origin.y < AABB.min.y || Origin.y > AABB.max.y )
                return false;
        
        } // End else

    } // End if process Y

    // Z Slabs
    if ( bIgnoreZ == false )
    {
        // Is it pointing toward?
        if ( fabsf(Velocity.z) > CGE_EPSILON )
        {
            fTemp = 1.0f / Velocity.z;
            t1    = (AABB.max.z - Origin.z) * fTemp;
            t2    = (AABB.min.z - Origin.z) * fTemp;
            
            // Reorder if necessary
            if (t1 > t2) { fTemp = t1; t1 = t2; t2 = fTemp; }
            
            // Compare and validate
            if (t1 > tMin) tMin = t1;
            if (t2 < tMax) tMax = t2;
            if (tMin > tMax) return false;
            if (tMax < 0) return false;
        
        } // End if
        else
        {
            // We cannot be intersecting in this case if the origin is outside of the slab
            //if ( Origin.z < (AABB.min.z - Origin.z) || Origin.z > (AABB.max.z - Origin.z) )
            if ( Origin.z < AABB.min.z || Origin.z > AABB.max.z )
                return false;
        
        } // End else

    } // End if process Z
    
    // Pick the correct t value
    if ( tMin > 0 ) t = tMin; else t = tMax;

	// Outside our valid range? if yes, return no collide
	if ( bRestrictRange == true && (t < 0.0f || t > 1.0f) )
        return false;

    // We intersected!
    return true;
}

//-----------------------------------------------------------------------------
// Name : rayIntersectSphere () (Static)
// Desc : Determine if the specified ray intersects the specified sphere.
//-----------------------------------------------------------------------------
bool cgCollision::rayIntersectSphere( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& Center, float Radius, float& t )
{
    cgVector3 vecDir = Origin - Center;
    cgFloat b = cgVector3::dot( vecDir, Velocity );
    cgFloat c = cgVector3::lengthSq( vecDir ) - (Radius * Radius);
    cgFloat d = (b*b) - c;
    if ( d > 0 )
    {
        t = -b - sqrtf(d);
        return true;
    
    } // End if intersects
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sphereIntersectTriangle () (Static)
/// <summary>
/// Determine wether or not the sphere intersects a triangle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::sphereIntersectTriangle( const cgVector3& Center, cgFloat Radius, const cgVector3& Velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& TriNormal, cgFloat& tMax, cgVector3& CollisionNormal )
{

    cgFloat   t = tMax;
    cgVector3 CollisionCenter;
    bool      bCollided = false;

    // Find the time of collision with the triangle's plane.
    if ( !sphereIntersectPlane( Center, Radius, Velocity, TriNormal, v1, t )) return false;
    
    // Calculate the sphere's center at the point of collision with the plane
    if ( t < 0 )
        CollisionCenter = Center + (TriNormal * -t);
    else
        CollisionCenter = Center + (Velocity * t);

    // If this point is within the bounds of the triangle, we have found the collision
    if ( pointInTriangle( CollisionCenter, v1, v2, v3, TriNormal ) )
    {
        // Collision normal is just the triangle normal
        CollisionNormal = TriNormal;
        tMax            = t;

        // Intersecting!
        return true;

    } // End if point within triangle interior

    // Otherwise we need to test each edge
    bCollided |= sphereIntersectLineSegment( Center, Radius, Velocity, v1, v2, tMax, CollisionNormal );
    bCollided |= sphereIntersectLineSegment( Center, Radius, Velocity, v2, v3, tMax, CollisionNormal );
    bCollided |= sphereIntersectLineSegment( Center, Radius, Velocity, v3, v1, tMax, CollisionNormal );
    return bCollided;
}

//-----------------------------------------------------------------------------
//  Name : sphereIntersectPlane () (Static)
/// <summary>
/// Determine whether the specified sphere (+velocity) intersects
/// a plane.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::sphereIntersectPlane( const cgVector3& Center, cgFloat Radius, const cgVector3& Velocity, const cgVector3& PlaneNormal, const cgVector3& PlanePoint, cgFloat& tMax )
{
    cgFloat numer, denom, t;
    
    // Setup equation
    numer = cgVector3::dot( Center - PlanePoint, PlaneNormal ) - Radius;
    denom = cgVector3::dot( Velocity, PlaneNormal );

    // Are we already overlapping?
    if ( numer < 0.0f || denom > -0.0000001f )
    {
        // The sphere is moving away from the plane
        if ( denom > -CGE_EPSILON ) return false;

        // Sphere is too far away from the plane
        if ( numer < -Radius ) return false;

        // Calculate the penetration depth
        tMax = numer;

        // Intersecting!
        return true;
    
    } // End if overlapping

    // We are not overlapping, perform ray-plane intersection
    t = -(numer / denom);

    // Ensure we are within range
    if ( t < 0.0f || t > tMax ) return false;

    // Store interval
    tMax = t;

    // Intersecting!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sphereIntersectLineSegment () (Static)
/// <summary>
/// Determine wether or not the sphere intersects a line segment.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::sphereIntersectLineSegment( const cgVector3& Center, cgFloat Radius, const cgVector3& Velocity, const cgVector3& v1, const cgVector3& v2, cgFloat& tMax, cgVector3& CollisionNormal )
{
    cgVector3 E, L, X, Y, PointOnEdge, CollisionCenter;
    cgFloat   a, b, c, d, e, t, n;

    // Setup the equation values
    E = v2 - v1;
    L = Center - v1;

    // Re-normalise the cylinder radius with segment length (((P - C) x E)² = r²)
    e = cgVector3::length( E );

    // If e == 0 we can't possibly succeed, the edge is degenerate and we'll
    // get a divide by 0 error in the normalization and 2nd order solving.
    if ( e < CGE_EPSILON ) return false;

    // Normalize the line vector
    E /= e;

    // Generate cross values
    cgVector3::cross( X, L, E );
    cgVector3::cross( Y, Velocity, E );

    // Setup the input values for the quadratic equation
    a = cgVector3::lengthSq( Y );
    b = 2.0f * cgVector3::dot( X, Y );
    c = cgVector3::lengthSq( X ) - (Radius*Radius);

    // If the sphere centre is already inside the cylinder, we need an overlap test
    if ( c < 0.0f )
    {
        // Find the distance along the line where our sphere center is positioned.
        // (i.e. sphere center projected down onto line)
        d = cgVector3::dot( L, E );

        // Is this before or after line start?
        if (d < 0.0f)
        {
            // The point is before the beginning of the line, test against the first vertex
            return sphereIntersectPoint( Center, Radius, Velocity, v1, tMax, CollisionNormal );
        
        } // End if before line start
        else if ( d > e )
        {
            // The point is after the end of the line, test against the second vertex
            return sphereIntersectPoint( Center, Radius, Velocity, v2, tMax, CollisionNormal );
        
        } // End if after line end
        else
        {
            // Point within the line segment
            PointOnEdge = v1 + E * d;

            // Generate collision normal
            CollisionNormal = Center - PointOnEdge;
            n = cgVector3::length( CollisionNormal );
            CollisionNormal /= n;

            // Calculate t value (remember we only enter here if we're already overlapping)
            // Remember, when we're overlapping we have no choice but to return a physical distance (the penetration depth)
            t = n - Radius;
            if (tMax < t) return false;
            
            // Store t and return
            tMax = t;

            // Edge Overlap
            return true;
        
        } // End if inside line segment
    
    } // End if sphere inside cylinder

    // If we are already checking for overlaps, return
    if ( tMax < 0.0f ) return false;
    
    // Solve the quadratic for t
    if ( !solveCollision(a, b, c, t) ) return false;

    // Is the segment too far away?
    if ( t > tMax ) return false;

    // Calculate the new sphere center at the time of collision
    CollisionCenter = Center + Velocity * t;

    // Project this down onto the edge
    d = cgVector3::dot( CollisionCenter - v1, E );

    // Simply check whether we need to test the end points as before
    if ( d < 0.0f )
        return sphereIntersectPoint( Center, Radius, Velocity, v1, tMax, CollisionNormal);
    else if ( d > e )
        return sphereIntersectPoint( Center, Radius, Velocity, v2, tMax, CollisionNormal);
    
    // Caclulate the Point of contact on the line segment
    PointOnEdge = v1 + E * d;

    // We can now generate our normal, store the interval and return
    cgVector3::normalize( CollisionNormal, CollisionCenter - PointOnEdge );
    tMax = t;

    // Intersecting!
    return true;

}

//-----------------------------------------------------------------------------
//  Name : sphereIntersectPoint () (Static)
/// <summary>
/// Determine wether or not the sphere intersects a point.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::sphereIntersectPoint( const cgVector3& Center, cgFloat Radius, const cgVector3& Velocity, const cgVector3& Point, cgFloat& tMax, cgVector3& CollisionNormal )
{
    cgVector3 L, CollisionCenter;
    cgFloat   a, b, c, l, l2, t;

    // Setup the equation values
    L  = Center - Point;
    l2 = cgVector3::lengthSq( L );

    // Setup the input values for the quadratic equation
    a = cgVector3::lengthSq( Velocity );
    b = 2.0f * cgVector3::dot( Velocity, L );
    c = l2 - (Radius * Radius);

    // If c < 0 then we are overlapping, return the overlap
    if ( c < 0.0f )
    {
        // Remember, when we're overlapping we have no choice 
        // but to return a physical distance (the penetration depth)
        l = sqrtf( l2 );
        t = l - Radius;
        
        // Outside our range?
        if (tMax < t) return false;

        // Generate the collision normal
        CollisionNormal = L / l;
        
        // Store t and return
        tMax = t;

        // Vertex Overlap
        return true;
    
    } // End if overlapping 

    // If we are already checking for overlaps, return
    if ( tMax < 0.0f ) return false;

    // Solve the quadratic for t
    if (!solveCollision(a, b, c, t)) return false;

    // Is the vertex too far away?
    if ( t > tMax ) return false;

    // Calculate the new sphere position at the time of contact
    CollisionCenter = Center + Velocity * t;

    // We can now generate our normal, store the interval and return
    cgVector3::normalize( CollisionNormal, CollisionCenter - Point );
    tMax = t;

    // Intersecting!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : AABBIntersectAABB () (Static)
/// <summary>
/// Determine if the two Axis Aligned bounding boxes specified are
/// intersecting with one another.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::AABBIntersectAABB( const cgBoundingBox & AABB1, const cgBoundingBox & AABB2, bool bIgnoreX /* = false */, bool bIgnoreY /* = false */, bool bIgnoreZ /* = false */ )
{
    return (bIgnoreX || AABB1.min.x <= AABB2.max.x) && (bIgnoreY || AABB1.min.y <= AABB2.max.y) &&
           (bIgnoreZ || AABB1.min.z <= AABB2.max.z) && (bIgnoreX || AABB1.max.x >= AABB2.min.x) &&
           (bIgnoreY || AABB1.max.y >= AABB2.min.y) && (bIgnoreZ || AABB1.max.z >= AABB2.min.z);
}

//-----------------------------------------------------------------------------
//  Name : AABBIntersectAABB () (Static, Overload)
/// <summary>
/// Determine if the two Axis Aligned bounding boxes specified are
/// intersecting with one another.
/// Note : This overload also tests wether Bounds2 is fully contained within
/// Bounds1.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::AABBIntersectAABB( bool & bContained, const cgBoundingBox & AABB1, const cgBoundingBox & AABB2, bool bIgnoreX /* = false */, bool bIgnoreY /* = false */, bool bIgnoreZ /* = false */ )
{
    // Set to true by default
    bContained = true;

    // Does the point fall outside any of the AABB planes?
         if ( !bIgnoreX && (AABB2.min.x < AABB1.min.x || AABB2.min.x > AABB1.max.x) ) bContained = false;
    else if ( !bIgnoreY && (AABB2.min.y < AABB1.min.y || AABB2.min.y > AABB1.max.y) ) bContained = false;
    else if ( !bIgnoreZ && (AABB2.min.z < AABB1.min.z || AABB2.min.z > AABB1.max.z) ) bContained = false;
    else if ( !bIgnoreX && (AABB2.max.x < AABB1.min.x || AABB2.max.x > AABB1.max.x) ) bContained = false;
    else if ( !bIgnoreY && (AABB2.max.y < AABB1.min.y || AABB2.max.y > AABB1.max.y) ) bContained = false;
    else if ( !bIgnoreZ && (AABB2.max.z < AABB1.min.z || AABB2.max.z > AABB1.max.z) ) bContained = false;

    // Return immediately if it's fully contained
    if ( bContained == true ) return true;

    // Perform full intersection test
    return (bIgnoreX || AABB1.min.x <= AABB2.max.x) && (bIgnoreY || AABB1.min.y <= AABB2.max.y) &&
           (bIgnoreZ || AABB1.min.z <= AABB2.max.z) && (bIgnoreX || AABB1.max.x >= AABB2.min.x) &&
           (bIgnoreY || AABB1.max.y >= AABB2.min.y) && (bIgnoreZ || AABB1.max.z >= AABB2.min.z);
}

//-----------------------------------------------------------------------------
//  Name : pointClassifyPlane () (Static)
/// <summary>
/// Classify the point against a plane.
/// </summary>
//-----------------------------------------------------------------------------
cgPlaneQuery::Class cgCollision::pointClassifyPlane( const cgVector3& Point, const cgVector3& PlaneNormal, cgFloat PlaneDistance )
{
    // Calculate distance from plane
    cgFloat fDistance = cgVector3::dot( Point, PlaneNormal ) + PlaneDistance;

    // Retrieve classification
    cgPlaneQuery::Class Location = cgPlaneQuery::On;
    if ( fDistance < -CGE_EPSILON ) Location = cgPlaneQuery::Back;
    if ( fDistance >  CGE_EPSILON ) Location = cgPlaneQuery::Front;

    // Return the classification
    return Location;
}

//-----------------------------------------------------------------------------
//  Name : rayClassifyPlane () (Static)
/// <summary>
/// Classify the ray against a plane.
/// </summary>
//-----------------------------------------------------------------------------
cgPlaneQuery::Class cgCollision::rayClassifyPlane( const cgVector3& Origin, const cgVector3& Velocity, const cgVector3& PlaneNormal, cgFloat PlaneDistance )
{
    cgPlaneQuery::Class Location[2];
    cgUInt32            Infront  = 0, Behind = 0, OnPlane=0, i;
    
    // Classify the two end points of our ray
    Location[0] = pointClassifyPlane( Origin, PlaneNormal, PlaneDistance );
    Location[1] = pointClassifyPlane( Origin + Velocity, PlaneNormal, PlaneDistance );

    // Count up the locations
    for ( i = 0; i < 2; ++i )
    {
        // Check the position
        if (Location[i] == cgPlaneQuery::Front )
            Infront++;
        else if (Location[i] == cgPlaneQuery::Back )
            Behind++;
        else
        {
            OnPlane++;
            Infront++;
            Behind++;

        } // End if on plane
    
    } // Next ray point

    // Return Result
    if ( OnPlane == 2 ) return cgPlaneQuery::On;     // On Plane
    if ( Behind  == 2 ) return cgPlaneQuery::Back;      // Behind
    if ( Infront == 2 ) return cgPlaneQuery::Front;     // In Front
    return cgPlaneQuery::Spanning; // Spanning

}

//-----------------------------------------------------------------------------
//  Name : polyClassifyPlane () (Static)
/// <summary>
/// Classify the poly against a plane.
/// </summary>
//-----------------------------------------------------------------------------
cgPlaneQuery::Class cgCollision::polyClassifyPlane( void * pVertices, cgUInt32 VertexCount, cgUInt32 Stride, const cgVector3& PlaneNormal, cgFloat PlaneDistance )
{
    cgUInt32 Infront  = 0, Behind = 0, OnPlane=0, i;
    cgUInt8  Location = 0;
    cgFloat	 Result   = 0;
    cgByte * pBuffer  = (cgByte*)pVertices;

    // Loop round each vector
    for ( i = 0; i < VertexCount; ++i )
    {
        // Calculate distance from plane
        cgFloat fDistance = cgVector3::dot( (cgVector3&)*pBuffer, PlaneNormal ) + PlaneDistance;
        pBuffer += Stride;

        // Retrieve classification
        Location = cgPlaneQuery::On;
        if ( fDistance < -CGE_EPSILON ) Location = cgPlaneQuery::Back;
        if ( fDistance >  CGE_EPSILON ) Location = cgPlaneQuery::Front;

        // Check the position
        if (Location == cgPlaneQuery::Front )
            Infront++;
        else if (Location == cgPlaneQuery::Back )
            Behind++;
        else
        {
            OnPlane++;
            Infront++;
            Behind++;

        } // End if on plane

    } // Next Vertex

    // Return Result
    if ( OnPlane == VertexCount ) return cgPlaneQuery::On;     // On Plane
    if ( Behind  == VertexCount ) return cgPlaneQuery::Back;      // Behind
    if ( Infront == VertexCount ) return cgPlaneQuery::Front;     // In Front
    return cgPlaneQuery::Spanning; // Spanning
}

//-----------------------------------------------------------------------------
//  Name : AABBClassifyPlane () (Static)
/// <summary>
/// Classify the AABB against a plane.
/// </summary>
//-----------------------------------------------------------------------------
cgPlaneQuery::Class cgCollision::AABBClassifyPlane( const cgBoundingBox & AABB, const cgVector3& PlaneNormal, cgFloat PlaneDistance )
{
    cgVector3 NearPoint, FarPoint;

    // Calculate near / far extreme points
    if ( PlaneNormal.x > 0.0f ) { FarPoint.x  = AABB.max.x; NearPoint.x = AABB.min.x; }
    else                        { FarPoint.x  = AABB.min.x; NearPoint.x = AABB.max.x; }

    if ( PlaneNormal.y > 0.0f ) { FarPoint.y  = AABB.max.y; NearPoint.y = AABB.min.y; }
    else                        { FarPoint.y  = AABB.min.y; NearPoint.y = AABB.max.y; }

    if ( PlaneNormal.z > 0.0f ) { FarPoint.z  = AABB.max.z; NearPoint.z = AABB.min.z; }
    else                        { FarPoint.z  = AABB.min.z; NearPoint.z = AABB.max.z; }

    // If near extreme point is outside, then the AABB is totally outside the plane
    if ( cgVector3::dot( PlaneNormal, NearPoint ) + PlaneDistance > 0.0f )
        return cgPlaneQuery::Front;
        
    // If far extreme point is outside, then the AABB is intersecting the plane
    if ( cgVector3::dot( PlaneNormal, FarPoint ) + PlaneDistance >= 0.0f )
        return cgPlaneQuery::Spanning;
    
    // We're behind
    return cgPlaneQuery::Back;
}

//-----------------------------------------------------------------------------
//  Name : solveCollision () (Private, Static)
/// <summary>
/// Determine wether or not the sphere intersects a triangle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::solveCollision( cgFloat a, cgFloat b, cgFloat c, cgFloat& t )
{
    cgFloat d, one_over_two_a, t0, t1, temp;

    // Basic equation solving
    d = b*b - 4*a*c;

    // No root if d < 0
    if (d < 0.0f) return false;

    // Setup for calculation
    d = sqrtf( d );
    one_over_two_a = 1.0f / (2.0f * a);

    // Calculate the two possible roots
    t0 = (-b - d) * one_over_two_a;
    t1 = (-b + d) * one_over_two_a;

    // Order the results
    if (t1 < t0) { temp = t0; t0 = t1; t1 = temp; }

    // Fail if both results are negative
    if (t1 < 0.0f) return false;

    // Return the first positive root
    if (t0 < 0.0f) t = t1; else t = t0;

    // Solution found
    return true;

}

//-----------------------------------------------------------------------------
//  Name : cgCollision () (Constructor)
/// <summary>
/// cgCollision Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgCollision::cgCollision()
{
   // Initialize variables to sensible defaults
    mQueryTree        = CG_NULL;
    mMaxIntersections = 100;
    mMaxIterations    = 10;
    mIntersections    = new CollIntersect[ mMaxIntersections ];
}

//-----------------------------------------------------------------------------
//  Name : cgCollision () (Destructor)
/// <summary>
/// cgCollision Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgCollision::~cgCollision()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgCollision::dispose( bool bDisposeBase )
{
    // Release memory
    if ( mIntersections != CG_NULL )
        delete []mIntersections;
    
    // Clear vars
    mQueryTree        = CG_NULL;
    mIntersections    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : setMaxIntersections ()
/// <summary>
/// Specifies the maximum number of intersections the collision system
/// should test for in each iteration. Instructs 'ellipsoidIntersectScene'
/// how many it can add to the specified array before it's full.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollision::setMaxIntersections( cgUInt16 nMaxIntersections )
{
    // Store max intersections
    mMaxIntersections = max( 1, nMaxIntersections );

    // Delete our old buffer
    if ( mIntersections != CG_NULL )
        delete []mIntersections;
    
    // Allocate a new buffer
    mIntersections = new CollIntersect[ mMaxIntersections ];
}

//-----------------------------------------------------------------------------
//  Name : setMaxIterations ()
/// <summary>
/// Specifies the number of iterations the collision response will
/// attempt before bailing.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollision::setMaxIterations( cgUInt16 nMaxIterations )
{
    // Store max iterations
    mMaxIterations = max( 1, nMaxIterations );
}

//-----------------------------------------------------------------------------
//  Name : setQueryTree ()
/// <summary>
/// Allows the application to pass in a spatial tree object which
/// the can be used during broad phase processing to detect nearby
/// objects. In addition, this tree's polygon data will be tested.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::setQueryTree( cgSpatialTree * pTree )
{
    // Store the tree
    mQueryTree = pTree;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addObject ()
/// <summary>
/// Add object for testing during collision in cases where a query tree
/// may not be supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::addObject( cgObjectNode * pObject )
{
    // Validate requirements
    if ( pObject == CG_NULL )
        return false;

    // Store the object
    mObjects.push_back( pObject );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : removeObject ()
/// <summary>
/// Remove object that was supplied for testing during collision in cases 
/// where a query tree may not be supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::removeObject( cgObjectNode * pObject )
{
    // Validate requirements
    if ( pObject == CG_NULL )
        return false;

    // Find the object for removal
    cgObjectNodeList::iterator itObject;
    for ( itObject = mObjects.begin(); itObject != mObjects.end(); ++itObject )
    {
        if ( *itObject == pObject )
        {
            // Remove it
            mObjects.erase( itObject );

            // Success!
            return true;
        
        } // End if match
    
    } // Next Object
    
    // No object found
    return false;    
}

//-----------------------------------------------------------------------------
//  Name : simulateEllipsoidSlide()
/// <summary>
/// Useful utility function used to simulate an ellipsoid that is moving
/// through the scene and should collide with other objects and slide off
/// their surface.
/// Note : For ease of understanding, all variables used in this function which
/// begin with a lower case 'e' (i.e. eNormal) denote that the values
/// contained within it are described in ellipsoid space.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::simulateEllipsoidSlide( cgObjectNode * pSrcObject, const cgVector3& Center, const cgVector3& Radius, const cgVector3& Velocity, cgVector3& NewCenter, cgVector3& NewIntegrationVelocity, cgBoundingBox & CollisionExtents )
{   
    // ToDo: The physics engine should probably do this directly calling ellipsoidIntersectScene().
    cgVector3 vecOutputPos, vecOutputVelocity, InvRadius, vecNormal, vecExtents;
    cgVector3 eVelocity, eInputVelocity, eFrom, eTo, vecNewCenter, vecIntersectPoint;
    cgVector3 vecFrom, vecEndPoint, vecAdjust;
    cgUInt32  i, IntersectionCount;
    //cgFloat   fDistance, fDot;
    bool      bHit = false;
    cgMatrix  mtxVelocity;

    // Default the output to move in clear space, regardless of whether we 
    // return true or not (we're going to alter this if we register a hit)
    // Note : We store this in a separate variable in case the user passed
    //        a reference to the same vector to both vecPos and vecNewPos
    vecOutputPos        = Center + Velocity;
    vecOutputVelocity   = Velocity;

    // Default our intersection extents
    CollisionExtents.min.x = Radius.x;
    CollisionExtents.min.y = Radius.y;
    CollisionExtents.min.z = Radius.z;
    CollisionExtents.max.x = -Radius.x;
    CollisionExtents.max.y = -Radius.y;
    CollisionExtents.max.z = -Radius.z;

    // Store ellipsoid transformation values
    InvRadius = cgVector3( 1.0f / Radius.x, 1.0f / Radius.y, 1.0f / Radius.z );

    // Calculate values in ellipsoid space
    eVelocity = vectorScale( Velocity, InvRadius );
    eFrom     = vectorScale( Center, InvRadius );
    eTo       = eFrom + eVelocity;

    // Store the input velocity for jump testing
    eInputVelocity = eVelocity;

    // Keep testing until we hit our max iteration limit
    for ( i = 0; i < mMaxIterations; ++i )
    {
        // Break out if our velocity is too small (optional but sometimes beneficial)
        //if ( cgVector3::length( &eVelocity ) < CGE_EPSILON ) break;

        /* ToDo: Restore?
        // Attempt scene intersection
        // Note : we are working totally in ellipsoid space at this point, so we specify that both
        // our input values are in ellipsoid space, and we would like our output values in ellipsoid space also.
        if ( ellipsoidIntersectScene( pSrcObject, eFrom, Radius, eVelocity, mIntersections, IntersectionCount, true, true ) )
        {
            // Retrieve the first collision intersections
            CollIntersect & FirstIntersect = mIntersections[0];

            // Calculate the WORLD space sliding normal and intersection point.
            cgVector3 vecHitPoint = vectorScale( FirstIntersect.intersectPoint, Radius );
            cgVector3::normalize( &vecNormal, &vectorScale( FirstIntersect.intersectNormal, InvRadius ) );
            
            // Did we collide with an object object?
            if ( FirstIntersect.object )
            {
                cgObjectNode * object = FirstIntersect.object;

                // Calculate our adjustment vector in world space. This is for our
                // velocity adjustment for objects so we have to work in the original world space.
                vecFrom     = vectorScale( eFrom, Radius );
                vecEndPoint = vecFrom + vecOutputVelocity;
                
                // Compute the velocity matrix for this object. Based on its prior world
                // matrix from the previous frame, and its current.
                cgMatrix::inverse( &mtxVelocity, CG_NULL, &object->GetPreviousObjectMatrix() );
                mtxVelocity *= object->GetObjectMatrix();

                // Transform the end point
                cgVector3::transformCoord( &vecAdjust, &vecEndPoint, &mtxVelocity );

                // Translate back so we only get the distance.
                vecAdjust -= vecEndPoint;

                // Flatten out to slide velocity
                fDistance = cgVector3::dot( &vecOutputVelocity, &vecNormal );
                vecOutputVelocity -= vecNormal * fDistance;

                // Apply the object's momentum to our velocity along the intersection normal
                fDot = cgVector3::dot( &vecAdjust, &vecNormal );
                vecOutputVelocity += vecNormal * fDot;

                // Clear the tree leaf cache because we have added energy to our object
                // ToDo: Note - Commented but needs re-adding?
                // m_TreeLeafList.clear();

                // Notify the scene object(s) that a hit occured
                if ( pSrcObject != CG_NULL )
                {
                    object->HitByObject( pSrcObject, vecHitPoint, vecNormal );
                    pSrcObject->ObjectHit( object, vecHitPoint, vecNormal );

                } // End if source object provided

            } // End if colliding with dynamic object
            else
            {
                // Generate slide velocity
                fDistance = cgVector3::dot( &vecOutputVelocity, &vecNormal );
                vecOutputVelocity -= vecNormal * fDistance;
            
            } // End if colliding with static scene

            // Set the sphere position to the collision position for the next iteration of testing
            eFrom = FirstIntersect.newCenter;

            // Project the end of the velocity vector onto the collision plane (at the sphere centre)
            fDistance = cgVector3::dot( &( eTo - FirstIntersect.newCenter ), &FirstIntersect.intersectNormal );
            eTo -= FirstIntersect.intersectNormal * fDistance;
            
            // Transform the sphere position back into world space, and recalculate the intersect point
            // (we recalculate because we want our collision extents to be based on the slope of intersections
            //  given in world space, rather than ellipsoid space. This gives us a better quality slope test later).
            vecNewCenter      = vectorScale( FirstIntersect.newCenter, Radius );
            vecIntersectPoint = vecNewCenter - vectorScale( vecNormal, Radius );

            // Calculate the min / max collision extents around the ellipsoid center
            vecExtents = vecIntersectPoint - vecNewCenter;
            if ( vecExtents.x > CollisionExtents.max.x ) CollisionExtents.max.x = vecExtents.x;
            if ( vecExtents.y > CollisionExtents.max.y ) CollisionExtents.max.y = vecExtents.y;
            if ( vecExtents.z > CollisionExtents.max.z ) CollisionExtents.max.z = vecExtents.z;
            if ( vecExtents.x < CollisionExtents.min.x ) CollisionExtents.min.x = vecExtents.x;
            if ( vecExtents.y < CollisionExtents.min.y ) CollisionExtents.min.y = vecExtents.y;
            if ( vecExtents.z < CollisionExtents.min.z ) CollisionExtents.min.z = vecExtents.z;

            // Update the velocity value
            eVelocity = eTo - eFrom;

            // We hit something
            bHit = true;

            // Filter 'Impulse' jumps
            if ( cgVector3::dot( &eVelocity, &eInputVelocity ) < 0 )
            {
                eTo = eFrom;
                break;
            
            } // End if points back on itself

        } // End if we got some intersections
        else
        {
            // We found no collisions, so break out of the loop
            break;

        } // End if no collision*/

        // ToDo: Remove?
        // Increment the app counter so that our polygon testing is reset
        //IncrementAppCounter();

    } // Next Iteration

    // Did we register any intersection at all?
    if ( bHit == true )
    {
        // ToDo: Remove?
        // Increment the app counter so that our polygon testing is reset
        //IncrementAppCounter();

        // Did we finish neatly or not?
        if ( i < mMaxIterations ) 
        {
            // Return our final position in world space
            vecOutputPos = vectorScale( eTo, Radius );

        } // End if in clear space
        else
        {
            // Just find the closest intersection
            eFrom = vectorScale( Center, InvRadius );

            // Attempt to intersect the scene
            IntersectionCount = 0;
            if ( ellipsoidIntersectScene( pSrcObject, eFrom, Radius, eInputVelocity, mIntersections, IntersectionCount, true, true ) )
            {
                // ToDo: No epsilon used any more? Still necessary? I imagine it's still a good idea but just to be sure.
                // Retrieve the intersection point in clear space, but ensure that we undo the epsilon
                // shift we apply during the above call. This ensures that when we are stuck between two
                // planes, we don't slowly push our way through.
                vecOutputPos = mIntersections[0].newCenter - (mIntersections[0].intersectNormal * CGE_EPSILON);
                
                // Scale back into world space
                vecOutputPos = vectorScale( vecOutputPos, Radius );
            
            } // End if collision
            else
            {
                // Don't move at all, stay where we were
                vecOutputPos = Center;
            
            } // End if no collision

        } // End if bad situation

    } // End if intersection found

    // Store the resulting output values
    NewCenter              = vecOutputPos;
    NewIntegrationVelocity = vecOutputVelocity;

    // Return hit code
    return bHit;

}

//-----------------------------------------------------------------------------
//  Name : ellipsoidIntersectScene ()
/// <summary>
/// Test for collision against the database using the ellipsoid specified
/// Note : For ease of understanding, all variables used in this function which
/// begin with a lower case 'e' (i.e. eNormal) denote that the values
/// contained within it are described in ellipsoid space.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollision::ellipsoidIntersectScene( cgObjectNode * pSrcObject, const cgVector3 &Center, const cgVector3& Radius, const cgVector3& Velocity, CollIntersect Intersections[], cgUInt32 & IntersectionCount, bool bInputEllipsoidSpace /* = false */, bool bReturnEllipsoidSpace /* = false */ )
{
    cgMatrix  mtxVelocity;
    cgVector3 eCenter, eVelocity, eAdjust, vecEndPoint, InvRadius;
    cgFloat   eInterval;
    
    // Calculate the reciprocal radius to prevent the many divides we would otherwise need.
    InvRadius = cgVector3( 1.0f / Radius.x, 1.0f / Radius.y, 1.0f / Radius.z );

    // Convert the values specified into ellipsoid space if required
    if ( bInputEllipsoidSpace == false )
    {
        // The input values were not in ellipsoid space
        eCenter   = vectorScale( Center, InvRadius );
        eVelocity = vectorScale( Velocity, InvRadius );
    
    } // End if !bInputEllipsoidSpace
    else
    {
        // The input values are already in ellipsoid space
        eCenter   = Center;
        eVelocity = Velocity;

    } // End if bInputEllipsoidSpace

    // Reset ellipsoid space interval to maximum
    eInterval = 1.0f;

    // Reset initial intersection count to 0 to save the caller having to do this.
    IntersectionCount = 0;

    // Calculate the bounding box of the ellipsoid
    cgVector3 vecCenter   = vectorScale( eCenter, Radius );
    cgVector3 vecVelocity = vectorScale( eVelocity, Radius );        
    calculateEllipsoidBounds( vecCenter, Radius, vecVelocity );

    // Using the spatial tree for broadphase query?
    if ( mQueryTree != CG_NULL )
    {
        // Perform the ellipsoid intersect test against our static scene
        //ellipsoidIntersectBuffers( m_CollVertices, m_CollTriangles, eCenter, Radius, InvRadius, eVelocity, eInterval, Intersections, IntersectionCount );*/

    } // End if tree broadphase
    else
    {
        // Iterate through object database directly.
        cgObjectNodeList::iterator itObject = mObjects.begin();
        for ( ; itObject != mObjects.end(); ++itObject )
        {
            // ToDo: Restore?
            /*cgObjectNode * object = *itObject;

            // Skip self
            if ( object == pSrcObject )
                continue;

            // Broad Phase AABB test against object
            cgBoundingBox Bounds = object->GetBoundingBox();
            if ( AABBIntersectAABB( Bounds, mEllipsoidBounds ) == false )
                continue;

            // ToDo: Clean up query mechanism.
            // Ask object for access to collision geometry.
            QueryData Data;
            QueryResult Result = Query_None;
            //QueryResult Result = object->QueryCollisionGeometry( mEllipsoidBounds, Data );

            // Anything to do?
            if ( Result == Query_None )
                continue;

            // Calculate our adjustment vector in world space. This is for our
            // velocity adjustment for objects so we have to work in the original world space.
            vecEndPoint = (vectorScale( eCenter, Radius ) + vectorScale( eVelocity, Radius ));

            // Compute the velocity matrix for this object. Based on its prior world
            // matrix from the previous frame, and its current.
            cgMatrix::inverse( &mtxVelocity, CG_NULL, &object->GetPreviousObjectMatrix() );
            mtxVelocity *= object->GetObjectMatrix();

            // Transform the end point based on the above velocity matrix.
            cgVector3::transformCoord( &eAdjust, &vecEndPoint, &mtxVelocity );

            // Translate back so we have the difference
            eAdjust -= vecEndPoint;

            // Scale back into ellipsoid space
            eAdjust  = vectorScale( eAdjust, InvRadius );

            // Perform the ellipsoid intersect test against this object
            cgUInt32 StartIntersection = ellipsoidIntersectBuffers( Data, Result, eCenter, Radius, InvRadius, eVelocity - eAdjust, eInterval, Intersections, IntersectionCount, &object->GetPreviousObjectMatrix() );

            // Loop through the intersections returned
            for ( cgUInt32 i = StartIntersection; i < IntersectionCount; ++i )
            {
                // Move us to the correct point (including the objects velocity)
                // if we were not embedded.
                if ( Intersections[i].interval > 0 )
                {
                    // Translate back
                    Intersections[i].newCenter      += eAdjust;
                    Intersections[i].intersectPoint += eAdjust;
                
                } // End if not embedded
                
                // Store object
                Intersections[i].object = object;

            } // Next Intersection*/

        } // Next Object

    } // End if no tree broadphase

    // If we were requested to return the values in normal space
    // then we must take the values back out of ellipsoid space here
    if ( bReturnEllipsoidSpace == false )
    {
        // For each intersection found
        for ( cgUInt32 i = 0; i < IntersectionCount; ++i )
        {
            // Transform the new center position and intersection point
            Intersections[ i ].newCenter      = vectorScale( Intersections[ i ].newCenter, Radius );
            Intersections[ i ].intersectPoint = vectorScale( Intersections[ i ].intersectPoint, Radius );
            
            // Transform the normal (again we do this in the opposite way to a coordinate)
            cgVector3 Normal = vectorScale( Intersections[ i ].intersectNormal, InvRadius );
            cgVector3::normalize( Normal, Normal );

            // Store the transformed normal
            Intersections[ i ].intersectNormal = Normal;
        
        } // Next Intersection
    
    } // End if !bReturnEllipsoidSpace

    // Return hit.
    return (IntersectionCount > 0);
}

//-----------------------------------------------------------------------------
//  Name : ellipsoidIntersectBuffers () (Private)
/// <summary>
/// The internal function which actually tests for intersection against
/// the specified data buffers.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgCollision::ellipsoidIntersectBuffers( QueryData & Data, QueryResult Processing, const cgVector3& eCenter, const cgVector3& Radius, const cgVector3& InvRadius, const cgVector3& eVelocity, cgFloat& eInterval, CollIntersect Intersections[], cgUInt32 & IntersectionCount, const cgMatrix * pTransform /*= CG_NULL*/ )
{
    cgVector3       ePoints[3], vPoints[3], eNormal, vecEdge1, vecEdge2;
    cgVector3       eIntersectNormal, eNewCenter;
    cgUInt32        nTriCount, TriIndex, NewIndex, FirstIndex, Indices[3];
    cgBoundingBox   EllipsoidBounds, TriBounds;
    bool            AddToList;

    // First we need to determine the offset of the vertex position in the data
    // set, and also the stride so that we can step to each required vertex.
    cgUInt32 nPosOffset = Data.vertexFormat->getElementOffset( D3DDECLUSAGE_POSITION );
    cgUInt32 nStride    = Data.vertexFormat->getStride();

    // FirstIndex tracks the first item to be added the intersection list.
    FirstIndex = IntersectionCount;

    // How many triangles are we processing?
    nTriCount = ( Processing == Query_All ) ? Data.triCount : (cgUInt32)Data.testFaces.size();

    // Compute the bounding box of the ellipsoid transformed into the space of the object 
    // (if necessary). This will be used to attempt to further reject triangles with a simple 
    // bounding box test.
    EllipsoidBounds = mEllipsoidBounds;
    if ( pTransform != CG_NULL )
    {
        cgMatrix  mtxInvTransform;
        cgMatrix::inverse( mtxInvTransform, *pTransform );
        EllipsoidBounds.transform( mtxInvTransform );

    } // End if object transform
    
    // Iterate through our triangle database
    for ( cgUInt32 i = 0; i < Data.triCount; ++i )
    {
        // Get the triangle information
        TriIndex   = ( Processing == Query_All ) ? i : Data.testFaces[i];
        Indices[0] = Data.indices[TriIndex*3];
        Indices[1] = Data.indices[(TriIndex*3)+1];
        Indices[2] = Data.indices[(TriIndex*3)+2];
        vPoints[0] = *(cgVector3*)(((cgByte*)Data.vertices) + (Indices[0]*nStride) + nPosOffset);
        vPoints[1] = *(cgVector3*)(((cgByte*)Data.vertices) + (Indices[1]*nStride) + nPosOffset);
        vPoints[2] = *(cgVector3*)(((cgByte*)Data.vertices) + (Indices[2]*nStride) + nPosOffset);

        // Compute the bounding box of this triangle and reject
        // if it does not intersect the ellipsoid AABB.
        TriBounds.reset();
        TriBounds.addPoint( vPoints[0] );
        TriBounds.addPoint( vPoints[1] );
        TriBounds.addPoint( vPoints[2] );
        if ( cgCollision::AABBIntersectAABB( EllipsoidBounds, TriBounds ) == false )
            continue;

        // Requires transformation?
        if ( pTransform )
        {
            // Get transformed points
            cgVector3::transformCoord( ePoints[0], vPoints[0], *pTransform );
            cgVector3::transformCoord( ePoints[1], vPoints[1], *pTransform );
            cgVector3::transformCoord( ePoints[2], vPoints[2], *pTransform );

            // Transform points into ellipsoid space
            ePoints[0] = vectorScale( ePoints[0], InvRadius );
            ePoints[1] = vectorScale( ePoints[1], InvRadius );
            ePoints[2] = vectorScale( ePoints[2], InvRadius );

            // Transform any available normal and normalize (Note how we do not use InvRadius for the normal)
            if ( Data.normals != CG_NULL )
            {
                cgVector3::transformNormal( eNormal, Data.normals[TriIndex], *pTransform );
                eNormal = vectorScale( eNormal, Radius );
                cgVector3::normalize( eNormal, eNormal );

            } // End if normals available

        } // End if requires transformation
        else
        {
            // Get points and transform into ellipsoid space
            ePoints[0] = vectorScale( vPoints[0], InvRadius );
            ePoints[1] = vectorScale( vPoints[1], InvRadius );
            ePoints[2] = vectorScale( vPoints[2], InvRadius );

            // Transform any available normal and normalize (Note how we do not use InvRadius for the normal)
            if ( Data.normals != CG_NULL )
            {
                eNormal = vectorScale( Data.normals[TriIndex], Radius );
                cgVector3::normalize( eNormal, eNormal );

            } // End if normals available
        
        } // End if no transformation

        // Generate normal if one was not available.
        if ( Data.normals == CG_NULL )
        {
            // Compute the two edge vectors required for generating our normal
            //vecEdge1 = ePoints[1] - ePoints[0];
            //vecEdge2 = ePoints[2] - ePoints[0];
            // ToDo: Normalize necessary?
            cgVector3::normalize( vecEdge1, (ePoints[1] - ePoints[0]) );
            cgVector3::normalize( vecEdge2, (ePoints[2] - ePoints[0]) );

            // Generate the normal
            cgVector3::normalize( eNormal, *cgVector3::cross( eNormal, vecEdge1, vecEdge2 ) );

        } // End if no normals

        // Test for intersection with a unit sphere and the ellipsoid space triangle
        if ( sphereIntersectTriangle( eCenter, 1.0f, eVelocity, ePoints[0], ePoints[1], ePoints[2], eNormal, eInterval, eIntersectNormal ) )
        {
            // Calculate our new sphere center at the point of intersection
            if ( eInterval > 0 )
                eNewCenter = eCenter + (eVelocity * eInterval);
            else
                eNewCenter = eCenter - (eIntersectNormal * eInterval);

            // Where in the array should it go?
            AddToList = false;
            if ( IntersectionCount == 0 || eInterval < Intersections[0].interval )
            {
                // We either have nothing in the array yet, or the new intersection is closer to us
                AddToList         = true;
                NewIndex          = 0;
                IntersectionCount = 1;
                
                // Reset, we've cleared the list
                FirstIndex        = 0;

            } // End if overwrite existing intersections
            else if ( fabsf( eInterval - Intersections[0].interval ) < CGE_EPSILON )
            {
                // It has the same interval as those in our list already, append to 
                // the end unless we've already reached our limit
                if ( IntersectionCount < mMaxIntersections )
                {
                    AddToList         = true;
                    NewIndex          = IntersectionCount;
                    IntersectionCount++;

                } // End if we have room to store more

            } // End if the same interval

            // Add to the list?
            if ( AddToList )
            {
                Intersections[ NewIndex ].interval        = eInterval;
                Intersections[ NewIndex ].newCenter       = eNewCenter + (eIntersectNormal * CGE_EPSILON); // Push back from the plane slightly to improve accuracy
                Intersections[ NewIndex ].intersectPoint  = eNewCenter - eIntersectNormal;           // The intersection point on the surface of the sphere (and triangle)
                Intersections[ NewIndex ].intersectNormal = eIntersectNormal;
                Intersections[ NewIndex ].triangleIndex   = TriIndex;
                Intersections[ NewIndex ].object         = CG_NULL;

            } // End if we are inserting in our list

        } // End if collided

    } // Next Triangle

    // Return hit.
    return FirstIndex;
}

//-----------------------------------------------------------------------------
//  Name : calculateEllipsoidBounds () (Private)
/// <summary>
/// Calculate the bounding box of the ellipsoid prior to processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollision::calculateEllipsoidBounds( const cgVector3& Center, const cgVector3& Radius, const cgVector3& Velocity )
{
    cgFloat     fLargestExtent;

    cgVector3 & vecMin = mEllipsoidBounds.min;
    cgVector3 & vecMax = mEllipsoidBounds.max;

    // Find the largest extent of our ellipsoid including the velocity length all around.
    fLargestExtent = Radius.x;
    if ( Radius.y > fLargestExtent ) fLargestExtent = Radius.y;
    if ( Radius.z > fLargestExtent ) fLargestExtent = Radius.z;

    // Reset the bounding box values
    vecMin = cgVector3( FLT_MAX, FLT_MAX, FLT_MAX );
    vecMax = cgVector3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    // Calculate the bounding box extents of where the ellipsoid currently 
    // is, and the position it will be moving to.
    if ( Center.x + fLargestExtent > vecMax.x ) vecMax.x = Center.x + fLargestExtent;
    if ( Center.y + fLargestExtent > vecMax.y ) vecMax.y = Center.y + fLargestExtent;
    if ( Center.z + fLargestExtent > vecMax.z ) vecMax.z = Center.z + fLargestExtent;

    if ( Center.x - fLargestExtent < vecMin.x ) vecMin.x = Center.x - fLargestExtent;
    if ( Center.y - fLargestExtent < vecMin.y ) vecMin.y = Center.y - fLargestExtent;
    if ( Center.z - fLargestExtent < vecMin.z ) vecMin.z = Center.z - fLargestExtent;

    if ( Center.x + Velocity.x + fLargestExtent > vecMax.x ) vecMax.x = Center.x + Velocity.x + fLargestExtent;
    if ( Center.y + Velocity.y + fLargestExtent > vecMax.y ) vecMax.y = Center.y + Velocity.y + fLargestExtent;
    if ( Center.z + Velocity.z + fLargestExtent > vecMax.z ) vecMax.z = Center.z + Velocity.z + fLargestExtent;

    if ( Center.x + Velocity.x - fLargestExtent < vecMin.x ) vecMin.x = Center.x + Velocity.x - fLargestExtent;
    if ( Center.y + Velocity.y - fLargestExtent < vecMin.y ) vecMin.y = Center.y + Velocity.y - fLargestExtent;
    if ( Center.z + Velocity.z - fLargestExtent < vecMin.z ) vecMin.z = Center.z + Velocity.z - fLargestExtent;

    // Add Tolerance values
    vecMin -= cgVector3( 1.0f, 1.0f, 1.0f );
    vecMax += cgVector3( 1.0f, 1.0f, 1.0f );
}