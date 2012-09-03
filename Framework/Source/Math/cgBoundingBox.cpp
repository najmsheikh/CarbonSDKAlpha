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
// Name : cgBoundingBox.cpp                                                  //
//                                                                           //
// Desc : Bounding Box class. Simple but efficient bounding box processing.  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBoundingBox Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgBoundingBox.h>
#include <memory.h>

///////////////////////////////////////////////////////////////////////////////
// Static Member Definitions
///////////////////////////////////////////////////////////////////////////////
cgBoundingBox cgBoundingBox::Empty( 0, 0, 0, 0, 0, 0 );

///////////////////////////////////////////////////////////////////////////////
// cgBoundingBox Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBoundingBox () (Default Constructor)
/// <summary>
/// cgBoundingBox Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox::cgBoundingBox( ) 
{
	// Initialise values
    reset();
}

//-----------------------------------------------------------------------------
//  Name : cgBoundingBox () (Constructor)
/// <summary>
/// cgBoundingBox Class Constructor, sets values from vector values passed
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox::cgBoundingBox( const cgVector3& vecMin, const cgVector3& vecMax ) 
{
	// Copy vector values
	min = vecMin;
	max = vecMax;
}

//-----------------------------------------------------------------------------
//  Name : cgBoundingBox () (Constructor)
/// <summary>
/// cgBoundingBox Class Constructor, sets values from float values passed
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox::cgBoundingBox( cgFloat xMin, cgFloat yMin, cgFloat zMin, cgFloat xMax, cgFloat yMax, cgFloat zMax )
{
    // Copy coordinate values
    min = cgVector3( xMin, yMin, zMin );
    max = cgVector3( xMax, yMax, zMax );
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Resets the bounding box values.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoundingBox::reset()
{
    min = cgVector3(  FLT_MAX,  FLT_MAX,  FLT_MAX );
	max = cgVector3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
}

//-----------------------------------------------------------------------------
//  Name : isPopulated ()
/// <summary>
/// Remains in reset state or has been populated?
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::isPopulated( ) const
{
    if ( memcmp( &min, &cgVector3(  FLT_MAX,  FLT_MAX,  FLT_MAX ), sizeof(cgVector3) ) != 0 || 
         memcmp( &max, &cgVector3( -FLT_MAX, -FLT_MAX, -FLT_MAX ), sizeof(cgVector3) ) != 0 )
        return true;

    // Still at reset state.
    return false;
}

//-----------------------------------------------------------------------------
// Name : isDegenerate ( )
/// <summary>
/// Determine if the bounding box is empty / degenerate.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::isDegenerate( ) const
{
    return ( fabsf( max.x - min.x ) < CGE_EPSILON && 
             fabsf( max.y - min.y ) < CGE_EPSILON && 
             fabsf( max.z - min.z ) < CGE_EPSILON );
}

//-----------------------------------------------------------------------------
//  Name : getDimensions ()
/// <summary>
/// Returns a vector containing the dimensions of the bounding box
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoundingBox::getDimensions() const
{
    return max - min;
}

//-----------------------------------------------------------------------------
//  Name : getCenter ()
/// <summary>
/// Returns a vector containing the exact centre point of the box
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoundingBox::getCenter() const
{
    return (max + min) * 0.5f;
}

//-----------------------------------------------------------------------------
//  Name : getExtents ()
/// <summary>
/// Returns a vector containing the extents of the bounding box (the
/// half-dimensions).
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoundingBox::getExtents( ) const
{
    return max - getCenter();
}

//-----------------------------------------------------------------------------
//  Name : getPlane ()
/// <summary>
/// Retrieves the plane for the specified side of the bounding box
/// </summary>
//-----------------------------------------------------------------------------
cgPlane cgBoundingBox::getPlane( cgVolumePlane::Side Side ) const
{
    cgPlane BoundsPlane;
	memset( &BoundsPlane, 0, sizeof(cgPlane) );
    
    // Select the requested side
    switch ( Side )
    {
        case cgVolumePlane::Top:
            BoundsPlane.b = 1;
            BoundsPlane.d = -cgVector3::dot( max, (cgVector3&)BoundsPlane );
            break;
        case cgVolumePlane::Right:
            BoundsPlane.a = 1;
            BoundsPlane.d = -cgVector3::dot( max, (cgVector3&)BoundsPlane );
            break;
        case cgVolumePlane::Far:
            BoundsPlane.c = 1;
            BoundsPlane.d = -cgVector3::dot( max, (cgVector3&)BoundsPlane );
            break;
        case cgVolumePlane::Bottom:
            BoundsPlane.b = -1;
            BoundsPlane.d = -cgVector3::dot( min, (cgVector3&)BoundsPlane );
            break;
        case cgVolumePlane::Left:
            BoundsPlane.a = -1;
            BoundsPlane.d = -cgVector3::dot( min, (cgVector3&)BoundsPlane );
            break;
        case cgVolumePlane::Near:
            BoundsPlane.c = -1;
            BoundsPlane.d = -cgVector3::dot( min, (cgVector3&)BoundsPlane );
            break;
    } // End Side Switch

    // Return the plane
    return BoundsPlane;
}

//-----------------------------------------------------------------------------
//  Name : calculateFromPolygon ()
/// <summary>
/// Calculates the bounding box based on the Face passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::calculateFromPolygon( const cgVector3 pVertices[], cgUInt32 VertexCount, cgUInt32 VertexStride, bool bReset /* = true */ )
{
    cgByte * pVerts  = (cgByte*)pVertices;
    cgUInt32 v;

    // Check for invalid params
    if ( !pVertices ) return *this;

    // Reset the box if requested
    if ( bReset ) reset();

    // Loop round all the Vertices in the face
    for ( v = 0; v < VertexCount; v++, pVerts += VertexStride ) 
        addPoint( *(cgVector3*)pVerts );

    return *this;
}

//-----------------------------------------------------------------------------
//  Name : addPoint ()
/// <summary>
/// Grows the bounding box based on the point passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::addPoint( const cgVector3 & Point )
{
    // Grow by this point
    if ( Point.x < min.x ) min.x = Point.x;
    if ( Point.y < min.y ) min.y = Point.y;
    if ( Point.z < min.z ) min.z = Point.z;
    if ( Point.x > max.x ) max.x = Point.x;
    if ( Point.y > max.y ) max.y = Point.y;
    if ( Point.z > max.z ) max.z = Point.z;

    return *this;
}

//-----------------------------------------------------------------------------
//  Name : validate ()
/// <summary>
/// Ensures that the values placed in the min / max values never make the
/// bounding box itself inverted.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoundingBox::validate()
{
    cgFloat rTemp;
    if ( max.x < min.x ) { rTemp = max.x; max.x = min.x; min.x = rTemp; }
    if ( max.y < min.y ) { rTemp = max.y; max.y = min.y; min.y = rTemp; }
    if ( max.z < min.z ) { rTemp = max.z; max.z = min.z; min.z = rTemp; }
}

//-----------------------------------------------------------------------------
//  Name : intersect()
/// <summary>
/// Tests to see if this AABB is intersected by another AABB
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgBoundingBox& Bounds ) const
{
    return (min.x <= Bounds.max.x) && (min.y <= Bounds.max.y) &&
           (min.z <= Bounds.max.z) && (max.x >= Bounds.min.x) &&
           (max.y >= Bounds.min.y) && (max.z >= Bounds.min.z);
}

//-----------------------------------------------------------------------------
//  Name : intersect()
/// <summary>
/// Tests to see if this AABB is intersected by another AABB with full
/// containment test.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgBoundingBox& Bounds, bool & bContained ) const
{
    // Set to true by default
    bContained = true;

    // Does the point fall outside any of the AABB planes?
         if ( Bounds.min.x < min.x || Bounds.min.x > max.x ) bContained = false;
    else if ( Bounds.min.y < min.y || Bounds.min.y > max.y ) bContained = false;
    else if ( Bounds.min.z < min.z || Bounds.min.z > max.z ) bContained = false;
    else if ( Bounds.max.x < min.x || Bounds.max.x > max.x ) bContained = false;
    else if ( Bounds.max.y < min.y || Bounds.max.y > max.y ) bContained = false;
    else if ( Bounds.max.z < min.z || Bounds.max.z > max.z ) bContained = false;

    // Return immediately if it's fully contained
    if ( bContained == true ) return true;

    // Perform full intersection test
    return (min.x <= Bounds.max.x) && (min.y <= Bounds.max.y) &&
           (min.z <= Bounds.max.z) && (max.x >= Bounds.min.x) &&
           (max.y >= Bounds.min.y) && (max.z >= Bounds.min.z);
}

//-----------------------------------------------------------------------------
//  Name : intersect()
/// <summary>
/// Tests to see if this AABB is intersected by another AABB and return
/// the resulting intersection.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgBoundingBox& Bounds, cgBoundingBox& Intersection ) const
{
    Intersection.min.x = __max( min.x, Bounds.min.x );
    Intersection.min.y = __max( min.y, Bounds.min.y );
    Intersection.min.z = __max( min.z, Bounds.min.z );
    Intersection.max.x = __min( max.x, Bounds.max.x );
    Intersection.max.y = __min( max.y, Bounds.max.y );
    Intersection.max.z = __min( max.z, Bounds.max.z );

    // Test for intersection
    if ( Intersection.min.x > Intersection.max.x ||
         Intersection.min.y > Intersection.max.y ||
         Intersection.min.z > Intersection.max.z ) return false;

    // Intersecting!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : intersect()
/// <summary>
/// Tests to see if this AABB is intersected by another AABB, includes
/// a tolerance for checking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgBoundingBox& Bounds, const cgVector3& Tolerance ) const
{
	return ((min.x - Tolerance.x) <= (Bounds.max.x + Tolerance.x)) &&
           ((min.y - Tolerance.y) <= (Bounds.max.y + Tolerance.y)) &&
           ((min.z - Tolerance.z) <= (Bounds.max.z + Tolerance.z)) &&
           ((max.x + Tolerance.x) >= (Bounds.min.x - Tolerance.x)) &&
           ((max.y + Tolerance.y) >= (Bounds.min.y - Tolerance.y)) &&
           ((max.z + Tolerance.z) >= (Bounds.min.z - Tolerance.z));
}

//-----------------------------------------------------------------------------
//  Name : intersect ()
/// <summary>
/// This function tests to see if a ray intersects the AABB.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgVector3 & Origin, const cgVector3 & Velocity, cgFloat & t, bool RestrictRange /* = true */ ) const
{
    cgFloat tMin = -FLT_MAX;
    cgFloat tMax = FLT_MAX;
    cgFloat t1, t2, fTemp;

    // If ray origin is inside bounding box, just return true (treat AABB as a solid box)
	if ( containsPoint( Origin ) == true ) 
	{
		t = 0.0f;
		return true;
	
    } // End if point in box

    // X Slabs
    // Is it pointing toward?
    if ( fabsf(Velocity.x) > CGE_EPSILON )
    {
        fTemp = 1.0f / Velocity.x;
        t1    = (max.x - Origin.x) * fTemp;
        t2    = (min.x - Origin.x) * fTemp;
        
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
        //if ( Origin.x < (min.x - Origin.x) || Origin.x > (max.x - Origin.x) )
        if ( Origin.x < min.x || Origin.x > max.x )
            return false;
    
    } // End else
    
    // Y Slabs
    // Is it pointing toward?
    if ( fabsf(Velocity.y) > CGE_EPSILON )
    {
        fTemp = 1.0f / Velocity.y;
        t1    = (max.y - Origin.y) * fTemp;
        t2    = (min.y - Origin.y) * fTemp;
        
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
        //if ( Origin.y < (min.y - Origin.y) || Origin.y > (max.y - Origin.y) )
        if ( Origin.y < min.y || Origin.y > max.y )
            return false;
    
    } // End else

    // Z Slabs
    // Is it pointing toward?
    if ( fabsf(Velocity.z) > CGE_EPSILON )
    {
        fTemp = 1.0f / Velocity.z;
        t1    = (max.z - Origin.z) * fTemp;
        t2    = (min.z - Origin.z) * fTemp;
        
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
        //if ( Origin.z < (min.z - Origin.z) || Origin.z > (max.z - Origin.z) )
        if ( Origin.z < min.z || Origin.z > max.z )
            return false;
    
    } // End else
    
    // Pick the correct t value
    if ( tMin > 0 ) t = tMin; else t = tMax;

	// Outside our valid range? if yes, return no collide
	if ( t < 0.0f || (RestrictRange == true && t > 1.0f) )
        return false;

    // We intersected!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : intersect ()
/// <summary>
/// This function tests to see if a triangle intersects the AABB.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::intersect( const cgVector3 & vTri0, const cgVector3 & vTri1, const cgVector3 & vTri2, const cgBoundingBox & TriBounds ) const
{
    // Perform rough "broadphase" rejection by testing for intersection
    // between the supplied triangle bounding box and the source box.
    if ( TriBounds.min.x > max.x || TriBounds.max.x < min.x ||
         TriBounds.min.y > max.y || TriBounds.max.y < min.y ||
         TriBounds.min.z > max.z || TriBounds.max.z < min.z )
        return false;

    // Move everything such that the box center is located at <0,0,0>
    // and the entire test becomes relative to it.
    const cgVector3     vCenter   = getCenter();
    const cgVector3     vExtents  = getExtents();
    const cgVector3     v0        = vTri0 - vCenter;
    const cgVector3     v1        = vTri1 - vCenter;
    const cgVector3     v2        = vTri2 - vCenter;

    // Next we need to test to see if the triangle's plane intersects the 
    // source box. Begin by generating the plane itself. Note: We need
    // the edge vectors for later tests, so keep them around.
    cgVector3 vNormal;
    cgVector3 vEdge0 = v1 - v0;
    cgVector3 vEdge1 = v2 - v1;
    cgVector3::cross( vNormal, vEdge0, vEdge1 );
    cgFloat fPlaneDistance = -cgVector3::dot( vNormal, v0 );
    
    // Calculate near / far extreme points
    cgVector3 vNearPoint, vFarPoint;
    if ( vNormal.x > 0.0f ) { vFarPoint.x  = max.x; vNearPoint.x = min.x; }
    else                    { vFarPoint.x  = min.x; vNearPoint.x = max.x; }
    if ( vNormal.y > 0.0f ) { vFarPoint.y  = max.y; vNearPoint.y = min.y; }
    else                    { vFarPoint.y  = min.y; vNearPoint.y = max.y; }
    if ( vNormal.z > 0.0f ) { vFarPoint.z  = max.z; vNearPoint.z = min.z; }
    else                    { vFarPoint.z  = min.z; vNearPoint.z = max.z; }

    // If near extreme point is outside, then the AABB is totally outside the plane
    if ( cgVector3::dot( vNormal, vNearPoint - vCenter ) + fPlaneDistance > 0.0f )
        return false;
        
    // If far extreme point is inside, then the AABB is not intersecting the plane
    if ( cgVector3::dot( vNormal, vFarPoint - vCenter ) + fPlaneDistance < 0.0f )
        return false;

    // AXISTEST macro required variables
    cgVector3 vAbsEdge;
    cgFloat fTemp0, fTemp1, fMin, fMax;
    #define AXISTEST( vEdge, vP0, vP1, nComponent0, nComponent1 ) \
        fTemp0 = vEdge[nComponent1] * vP0[nComponent0] - vEdge[nComponent0] * vP0[nComponent1]; \
        fTemp1 = vEdge[nComponent1] * vP1[nComponent0] - vEdge[nComponent0] * vP1[nComponent1]; \
        if ( fTemp0 < fTemp1 ) { fMin = fTemp0; fMax = fTemp1; } else { fMin = fTemp1; fMax = fTemp0; } \
        fTemp0 = vAbsEdge[nComponent1] * vExtents[nComponent0] + vAbsEdge[nComponent0] * vExtents[nComponent1]; \
        if ( fMin > fTemp0 || fMax < -fTemp0 ) return false;
    
    #define AXISTEST2( vEdge, vP0, vP1, nComponent0, nComponent1 ) \
        fTemp0 = -vEdge[nComponent1] * vP0[nComponent0] + vEdge[nComponent0] * vP0[nComponent1]; \
        fTemp1 = -vEdge[nComponent1] * vP1[nComponent0] + vEdge[nComponent0] * vP1[nComponent1]; \
        if ( fTemp0 < fTemp1 ) { fMin = fTemp0; fMax = fTemp1; } else { fMin = fTemp1; fMax = fTemp0; } \
        fTemp0 = vAbsEdge[nComponent1] * vExtents[nComponent0] + vAbsEdge[nComponent0] * vExtents[nComponent1]; \
        if ( fMin > fTemp0 || fMax < -fTemp0 ) return false;

    // Test to see if the triangle edges cross the box.
    vAbsEdge.x = fabs(vEdge0.x);
    vAbsEdge.y = fabs(vEdge0.y);
    vAbsEdge.z = fabs(vEdge0.z);
    AXISTEST( vEdge0, v0, v2, 1, 2 ); // X
    AXISTEST2( vEdge0, v0, v2, 0, 2 ); // Y
    AXISTEST( vEdge0, v2, v1, 0, 1 ); // Z

    vAbsEdge.x = fabs(vEdge1.x);
    vAbsEdge.y = fabs(vEdge1.y);
    vAbsEdge.z = fabs(vEdge1.z);
    AXISTEST( vEdge1, v0, v2, 1, 2 ); // X
    AXISTEST2( vEdge1, v0, v2, 0, 2 ); // Y
    AXISTEST( vEdge1, v0, v1, 0, 1 ); // Z

    const cgVector3 vEdge2 = v0 - v2;
    vAbsEdge.x = fabs(vEdge2.x);
    vAbsEdge.y = fabs(vEdge2.y);
    vAbsEdge.z = fabs(vEdge2.z);
    AXISTEST( vEdge2, v0, v1, 1, 2 ); // X
    AXISTEST2( vEdge2, v0, v1, 0, 2 ); // Y
    AXISTEST( vEdge2, v2, v1, 0, 1 ); // Z

    // Overlapping
    return true;
}

//-----------------------------------------------------------------------------
//  Name : containsPoint()
/// <summary>
/// Tests to see if a point falls within this bounding box or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::containsPoint( const cgVector3& Point ) const
{
    if (Point.x < min.x || Point.x > max.x) return false;
    if (Point.y < min.y || Point.y > max.y) return false;
    if (Point.z < min.z || Point.z > max.z) return false;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : containsPoint()
/// <summary>
/// Tests to see if a point falls within this bounding box or not
/// including a specific tolerance around the box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::containsPoint( const cgVector3& Point, const cgVector3& Tolerance ) const
{
    if (Point.x < min.x - Tolerance.x || Point.x > max.x + Tolerance.x) return false;
    if (Point.y < min.y - Tolerance.y || Point.y > max.y + Tolerance.y) return false;
    if (Point.z < min.z - Tolerance.z || Point.z > max.z + Tolerance.z) return false;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : closestPoint ()
/// <summary>
/// Compute a point, on the surface of the AABB, that falls closest to 
/// the input point.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoundingBox::closestPoint( const cgVector3 & TestPoint ) const
{
	cgVector3 Closest;
	
	// Test X extent
	if ( TestPoint.x < min.x )
		Closest.x = min.x;
	else if ( TestPoint.x > max.x )
		Closest.x = max.x;
	else
		Closest.x = TestPoint.x;
	
	// Test Y extent
	if ( TestPoint.y < min.y )
		Closest.y = min.y;
	else if ( TestPoint.y > max.y )
		Closest.y = max.y;
	else
		Closest.y = TestPoint.y;
	
	// Test Z extent
	if ( TestPoint.z < min.z )
		Closest.z = min.z;
	else if ( TestPoint.z > max.z )
		Closest.z = max.z;
	else
		Closest.z = TestPoint.z;
	
	// Return the closest TestPoint
	return Closest;
}

//-----------------------------------------------------------------------------
//  Name : transform ()
/// <summary>
/// Transforms an axis aligned bounding box, by the specified matrix,
/// outputting new AAB values which are a best fit about that 'virtual
/// transformation'.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox & cgBoundingBox::transform( const cgTransform & t )
{
    cgVector3 BoundsCenter = getCenter();
    
    // Compute new extents values
    const cgVector3 Ex = t.xAxis() * (max.x - BoundsCenter.x);
    const cgVector3 Ey = t.yAxis() * (max.y - BoundsCenter.y);
    const cgVector3 Ez = t.zAxis() * (max.z - BoundsCenter.z);

    // Calculate new extents actual
    const cgFloat fEx = fabsf(Ex.x) + fabsf(Ey.x) + fabsf(Ez.x);
    const cgFloat fEy = fabsf(Ex.y) + fabsf(Ey.y) + fabsf(Ez.y);
    const cgFloat fEz = fabsf(Ex.z) + fabsf(Ey.z) + fabsf(Ez.z);
    
    // Compute new centre (we use 'transformNormal' because we only
    // want to apply rotation and scale).
    t.transformNormal( BoundsCenter, BoundsCenter );
    
    // Calculate final bounding box (add on translation)
    const cgVector3 & vTranslation = t.position();
    min.x = (BoundsCenter.x - fEx) + vTranslation.x;
    min.y = (BoundsCenter.y - fEy) + vTranslation.y;
    min.z = (BoundsCenter.z - fEz) + vTranslation.z;
    max.x = (BoundsCenter.x + fEx) + vTranslation.x;
    max.y = (BoundsCenter.y + fEy) + vTranslation.y;
    max.z = (BoundsCenter.z + fEz) + vTranslation.z;

    // Return reference to self
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : transform () (Static)
/// <summary>
/// Transforms the specified bounding box by the provided matrix and return the
/// new resulting box as a copy.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgBoundingBox::transform( cgBoundingBox Bounds, const cgTransform & t )
{
    return Bounds.transform( t );
}

//-----------------------------------------------------------------------------
//  Name : inflate()
/// <summary>
/// Grow (or shrink if you specify a negative value) the bounding box
/// by the specified number of world space units on all three axes.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoundingBox::inflate( cgFloat fGrowSize )
{
    min.x -= fGrowSize;
    min.y -= fGrowSize;
    min.z -= fGrowSize;
    max.x += fGrowSize;
    max.y += fGrowSize;
    max.z += fGrowSize;
}

//-----------------------------------------------------------------------------
//  Name : inflate()
/// <summary>
/// Grow (or shrink if you specify a negative value) the bounding box
/// by the specified numbers number of world space units on each of the 
/// three axes independantly.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoundingBox::inflate( const cgVector3& vecGrowSize )
{
    min.x -= vecGrowSize.x;
    min.y -= vecGrowSize.y;
    min.z -= vecGrowSize.z;
    max.x += vecGrowSize.x;
    max.y += vecGrowSize.y;
    max.z += vecGrowSize.z;
}

//-----------------------------------------------------------------------------
//  Name : operator+=()
/// <summary>
/// Moves the bounding box by the vector passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::operator+= ( const cgVector3& vecShift )
{
    min += vecShift;
    max += vecShift;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator-=()
/// <summary>
/// Moves the bounding box by the vector passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::operator-= ( const cgVector3& vecShift )
{
    min -= vecShift;
    max -= vecShift;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator*()
/// <summary>
/// Scales the bounding box values by the scalar passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgBoundingBox::operator* ( float fScale ) const
{
    return cgBoundingBox( min * fScale, max * fScale );
}

//-----------------------------------------------------------------------------
//  Name : operator*=()
/// <summary>
/// Scales the bounding box values by the scalar passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::operator*= ( float fScale )
{
    min *= fScale;
    max *= fScale;
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator*=()
/// <summary>
/// Transforms the bounding box by the matrix passed.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox& cgBoundingBox::operator*= ( const cgTransform & t )
{
    transform( t );
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator==()
/// <summary>
/// Test for quality between this bounding box and the other.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::operator== ( const cgBoundingBox& Bounds ) const
{
    return (min == Bounds.min) && (max == Bounds.max);
}

//-----------------------------------------------------------------------------
//  Name : operator!=()
/// <summary>
/// Test for quality between this bounding box and the other.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoundingBox::operator!= ( const cgBoundingBox& Bounds ) const
{
    return (min != Bounds.min) || (max != Bounds.max);
}