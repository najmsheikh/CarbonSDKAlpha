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
// Name : cgFrustum.cpp                                                      //
//                                                                           //
// Desc : Frustum class. Simple but efficient frustum processing.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgFrustum Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgFrustum.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgExtrudedBoundingBox.h>

///////////////////////////////////////////////////////////////////////////////
// cgFrustum Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgFrustum () (Default Constructor)
/// <summary>
/// cgFrustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum::cgFrustum( ) 
{
	// Initialise values
    memset( planes, 0, 6 * sizeof(cgPlane) );
    memset( points, 0, 8 * sizeof(cgVector3) );
    position = cgVector3(0,0,0);
}

//-----------------------------------------------------------------------------
//  Name : cgFrustum () (Constructor)
/// <summary>
/// cgFrustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum::cgFrustum( const cgMatrix & View, const cgMatrix & Proj )
{
	update( View, Proj );
}

//-----------------------------------------------------------------------------
//  Name : cgFrustum () (Constructor)
/// <summary>
/// cgFrustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum::cgFrustum( const cgBoundingBox & AABB )
{
    using namespace cgVolumeGeometry;
    using namespace cgVolumePlane;

    // Compute planes
    planes[Left]   = AABB.getPlane( Left );
    planes[Right]  = AABB.getPlane( Right );
    planes[Top]    = AABB.getPlane( Top );
    planes[Bottom] = AABB.getPlane( Bottom );
    planes[Near]   = AABB.getPlane( Near );
    planes[Far]    = AABB.getPlane( Far );
    
    // Compute points
    cgVector3 e = AABB.getExtents();
    cgVector3 p = AABB.getCenter();
    points[LeftBottomNear]  = cgVector3( p.x - e.x, p.y - e.y, p.z + e.z );
    points[LeftBottomFar]   = cgVector3( p.x - e.x, p.y - e.y, p.z + e.z );
    points[RightBottomNear] = cgVector3( p.x + e.x, p.y - e.y, p.z - e.z );
    points[RightBottomFar]  = cgVector3( p.x + e.x, p.y - e.y, p.z + e.z );
    points[LeftTopNear]     = cgVector3( p.x - e.x, p.y + e.y, p.z + e.z );
    points[LeftTopFar]      = cgVector3( p.x - e.x, p.y + e.y, p.z + e.z );
    points[RightTopNear]    = cgVector3( p.x + e.x, p.y + e.y, p.z - e.z );
    points[RightTopFar]     = cgVector3( p.x + e.x, p.y + e.y, p.z + e.z );
    position = p;
}

//-----------------------------------------------------------------------------
//  Name : cgFrustum ()
/// <summary>
/// Alternate constructor that builds a frustum for the 90 degree faces
/// of a cube. 
/// Note : Clip plane normals point outwards.
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum::cgFrustum( cgUInt8 nFrustumIndex, const cgVector3 & vOrigin, cgFloat fFarDistance, cgFloat fNearDistance /* = 1.0f */ )
{
    using namespace cgVolumePlane;

	const cgFloat c = 0.7071067812f; // cosine of 45 degrees
	cgVector3     vNear, vFar;
	cgPlane       ClipPlanes[ 6 ];

	// Choose normals and near/far plane offsets based on frustum index
    switch ( nFrustumIndex )
	{
		case 0: // + X
			ClipPlanes[ Left ]   = cgPlane(  -c,   0,  c, 0);
			ClipPlanes[ Right ]  = cgPlane(  -c,   0, -c, 0);
			ClipPlanes[ Top ]    = cgPlane(  -c,   c,  0, 0);
			ClipPlanes[ Bottom ] = cgPlane(  -c,  -c,  0, 0);
			ClipPlanes[ Near ]   = cgPlane( -1.0f, 0,  0, 0);
			ClipPlanes[ Far ]    = cgPlane(  1.0f, 0,  0, 0);
			vNear = cgVector3( fNearDistance, 0, 0 );
			vFar  = cgVector3( fFarDistance, 0, 0 );
		    break;
		case 1: // - X 
			ClipPlanes[ Left ]   = cgPlane(   c,   0, -c, 0);
			ClipPlanes[ Right ]  = cgPlane(   c,   0,  c, 0);
			ClipPlanes[ Top ]    = cgPlane(   c,   c,  0, 0);
			ClipPlanes[ Bottom ] = cgPlane(   c,  -c,  0, 0);
			ClipPlanes[ Near ]   = cgPlane(  1.0f, 0,  0, 0);
			ClipPlanes[ Far ]    = cgPlane( -1.0f, 0,  0, 0);
			vNear = cgVector3( -fNearDistance, 0, 0 );
			vFar  = cgVector3( -fFarDistance, 0, 0 );
		    break;
		case 2: // +Y 
			ClipPlanes[ Left ]   = cgPlane(  -c,  -c,  0,  0);
			ClipPlanes[ Right ]  = cgPlane(   c,  -c,  0,  0);
			ClipPlanes[ Top ]    = cgPlane(   0,  -c, -c,  0);
			ClipPlanes[ Bottom ] = cgPlane(   0,  -c,  c,  0);
			ClipPlanes[ Near ]   = cgPlane(   0, -1.0f, 0, 0);
			ClipPlanes[ Far ]    = cgPlane(   0,  1.0f, 0, 0);
			vNear = cgVector3( 0, fNearDistance, 0 );
			vFar  = cgVector3( 0, fFarDistance, 0 );
		    break;
		case 3: // -Y
			ClipPlanes[ Left ]   = cgPlane(  -c,   c,     0, 0);
			ClipPlanes[ Right ]  = cgPlane(   c,   c,     0, 0);
			ClipPlanes[ Top ]    = cgPlane(   0,   c,     c, 0);
			ClipPlanes[ Bottom ] = cgPlane(   0,   c,    -c, 0);
			ClipPlanes[ Near ]   = cgPlane(   0,   1.0f,  0, 0);
			ClipPlanes[ Far ]    = cgPlane(   0,  -1.0f,  0, 0);
			vNear = cgVector3( 0, -fNearDistance, 0 );
			vFar  = cgVector3( 0, -fFarDistance, 0 );
		    break;
		case 4: // + Z
			ClipPlanes[ Left ]   = cgPlane(  -c,   0, -c,    0);
			ClipPlanes[ Right ]  = cgPlane(   c,   0, -c,    0);
			ClipPlanes[ Top ]    = cgPlane(   0,   c, -c,    0);
			ClipPlanes[ Bottom ] = cgPlane(   0,  -c, -c,    0);
			ClipPlanes[ Near ]   = cgPlane(   0,   0, -1.0f, 0);
			ClipPlanes[ Far ]    = cgPlane(   0,   0,  1.0f, 0);
			vNear = cgVector3( 0, 0, fNearDistance );
			vFar  = cgVector3( 0, 0, fFarDistance );
		    break;
		case 5: // - Z
			ClipPlanes[ Left ]   = cgPlane(   c,   0,  c,    0);
			ClipPlanes[ Right ]  = cgPlane(  -c,   0,  c,    0);
			ClipPlanes[ Top ]    = cgPlane(   0,   c,  c,    0);
			ClipPlanes[ Bottom ] = cgPlane(   0,  -c,  c,    0);
			ClipPlanes[ Near ]   = cgPlane(   0,   0,  1.0f, 0);
			ClipPlanes[ Far ]    = cgPlane(   0,   0, -1.0f, 0);
			vNear = cgVector3( 0, 0, -fNearDistance );
			vFar  = cgVector3( 0, 0, -fFarDistance );
		    break;

    } // End Switch

	// Compute distances
	ClipPlanes[ Left ].d   = -cgVector3::dot( (cgVector3&)ClipPlanes[ Left ],   vOrigin );
	ClipPlanes[ Right ].d  = -cgVector3::dot( (cgVector3&)ClipPlanes[ Right ],  vOrigin );
	ClipPlanes[ Top ].d    = -cgVector3::dot( (cgVector3&)ClipPlanes[ Top ],    vOrigin );
	ClipPlanes[ Bottom ].d = -cgVector3::dot( (cgVector3&)ClipPlanes[ Bottom ], vOrigin );
	ClipPlanes[ Near ].d   = -cgVector3::dot( (cgVector3&)ClipPlanes[ Near ],   vOrigin + vNear );
	ClipPlanes[ Far ].d    = -cgVector3::dot( (cgVector3&)ClipPlanes[ Far ],    vOrigin + vFar  );
	
	// Update with the new clip planes
	setPlanes( ClipPlanes );
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Compute the new frustum details based on the matrices specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgFrustum::update( const cgMatrix & View, const cgMatrix & Proj )
{
    using namespace cgVolumePlane;

    // Build a combined view & projection matrix
    cgMatrix m = View * Proj;

    // Left clipping plane
    planes[Left].a   = -(m._14 + m._11);
    planes[Left].b   = -(m._24 + m._21);
    planes[Left].c   = -(m._34 + m._31);
    planes[Left].d   = -(m._44 + m._41);

    // Right clipping plane
    planes[Right].a  = -(m._14 - m._11);
    planes[Right].b  = -(m._24 - m._21);
    planes[Right].c  = -(m._34 - m._31);
    planes[Right].d  = -(m._44 - m._41);

    // Top clipping plane
    planes[Top].a    = -(m._14 - m._12);
    planes[Top].b    = -(m._24 - m._22);
    planes[Top].c    = -(m._34 - m._32);
    planes[Top].d    = -(m._44 - m._42);

    // Bottom clipping plane
    planes[Bottom].a = -(m._14 + m._12);
    planes[Bottom].b = -(m._24 + m._22);
    planes[Bottom].c = -(m._34 + m._32);
    planes[Bottom].d = -(m._44 + m._42);

    // Near clipping plane
    planes[Near].a   = -(m._13);
    planes[Near].b   = -(m._23);
    planes[Near].c   = -(m._33);
    planes[Near].d   = -(m._43);

    // Far clipping plane
    planes[Far].a    = -(m._14 - m._13);
    planes[Far].b    = -(m._24 - m._23);
    planes[Far].c    = -(m._34 - m._33);
    planes[Far].d    = -(m._44 - m._43);

    // Normalize and compute additional information.
    setPlanes( planes );

    // Compute the originating position of the frustum.
    position  = cgVector3( View._11, View._21, View._31 ) * -View._41;
    position += cgVector3( View._12, View._22, View._32 ) * -View._42;
    position += cgVector3( View._13, View._23, View._33 ) * -View._43;
}

//-----------------------------------------------------------------------------
//  Name : setPlanes ()
/// <summary>
/// Compute the new frustum details based on the six planes specified. 
/// This method automatically recomputes the 8 corner points of the frustum
/// based on the supplied planes.
/// </summary>
//-----------------------------------------------------------------------------
void cgFrustum::setPlanes( const cgPlane _Planes[] )
{
    // Copy and normalize the planes
    for ( cgInt i = 0; i < 6; i++ )
        cgPlane::normalize( planes[i], _Planes[i] );

    // Recompute the frustum corner points.
    recomputePoints();
}

//-----------------------------------------------------------------------------
//  Name : recomputePoints ()
/// <summary>
/// Recompute the 8 corner points of the frustum based on the supplied planes.
/// </summary>
//-----------------------------------------------------------------------------
void cgFrustum::recomputePoints( )
{
    // Compute the 8 corner points
    for ( cgInt i = 0; i < 8; ++i )
    {
        const cgPlane& p0 = (i & 1) ? planes[cgVolumePlane::Near] : planes[cgVolumePlane::Far];
        const cgPlane& p1 = (i & 2) ? planes[cgVolumePlane::Top]  : planes[cgVolumePlane::Bottom];
        const cgPlane& p2 = (i & 4) ? planes[cgVolumePlane::Left] : planes[cgVolumePlane::Right];

        // Compute the point at which the three planes intersect
        cgFloat cosTheta, secTheta;
        cgVector3 n1_n2, n2_n0, n0_n1;  
        cgVector3 n0( p0.a, p0.b, p0.c );
        cgVector3 n1( p1.a, p1.b, p1.c );
        cgVector3 n2( p2.a, p2.b, p2.c );
        
        cgVector3::cross( n1_n2, n1, n2 );
        cgVector3::cross( n2_n0, n2, n0 );
        cgVector3::cross( n0_n1, n0, n1 );
        
        cosTheta = cgVector3::dot( n0, n1_n2 );
        secTheta = 1.f / cosTheta;
        
        n1_n2     = n1_n2 * p0.d;
        n2_n0     = n2_n0 * p1.d;
        n0_n1     = n0_n1 * p2.d;

        points[i] = -(n1_n2 + n2_n0 + n0_n1) * secTheta;
    
    } // Next Corner
}

//-----------------------------------------------------------------------------
//  Name : classifyAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifyAABB( const cgBoundingBox & AABB ) const
{
    cgVolumeQuery::Class  Result = cgVolumeQuery::Inside;
    cgVector3 NearPoint, FarPoint;
    for ( size_t i = 0; i < 6; i++ )
    {
        // Store the plane
        const cgPlane & Plane = planes[i];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) { FarPoint.x  = AABB.max.x; NearPoint.x = AABB.min.x; }
        else                  { FarPoint.x  = AABB.min.x; NearPoint.x = AABB.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = AABB.max.y; NearPoint.y = AABB.min.y; }
        else                  { FarPoint.y  = AABB.min.y; NearPoint.y = AABB.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = AABB.max.z; NearPoint.z = AABB.min.z; }
        else                  { FarPoint.z  = AABB.min.z; NearPoint.z = AABB.max.z; }

        // If near extreme point is outside, then the AABB is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return cgVolumeQuery::Outside;

        // If far extreme point is outside, then the AABB is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;

    } // Next Plane
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : classifyAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifyAABB( const cgBoundingBox & AABB, const cgTransform & transform ) const
{
    // Transform bounding box as requested.
    cgBoundingBox Bounds = cgBoundingBox::transform( AABB, transform );

    // Process planes.
    cgVolumeQuery::Class  Result = cgVolumeQuery::Inside;
    cgVector3 NearPoint, FarPoint;
    for ( size_t i = 0; i < 6; i++ )
    {
        // Store the plane
        const cgPlane & Plane = planes[i];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) { FarPoint.x  = Bounds.max.x; NearPoint.x = Bounds.min.x; }
        else                  { FarPoint.x  = Bounds.min.x; NearPoint.x = Bounds.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = Bounds.max.y; NearPoint.y = Bounds.min.y; }
        else                  { FarPoint.y  = Bounds.min.y; NearPoint.y = Bounds.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = Bounds.max.z; NearPoint.z = Bounds.min.z; }
        else                  { FarPoint.z  = Bounds.min.z; NearPoint.z = Bounds.max.z; }

        // If near extreme point is outside, then the Bounds is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return cgVolumeQuery::Outside;

        // If far extreme point is outside, then the Bounds is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;

    } // Next Plane
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : classifyAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifyAABB( const cgBoundingBox & AABB, cgUInt8 & FrustumBits, cgInt8 & LastOutside ) const
{
    // If the 'last outside plane' index was specified, test it first!
    cgVector3 NearPoint, FarPoint;
    cgVolumeQuery::Class Result = cgVolumeQuery::Inside;
    if ( LastOutside >= 0 && ( ((FrustumBits >> LastOutside) & 0x1) == 0x0 ) )
    {
        const cgPlane & Plane = planes[LastOutside];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) { FarPoint.x  = AABB.max.x; NearPoint.x = AABB.min.x; }
        else                  { FarPoint.x  = AABB.min.x; NearPoint.x = AABB.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = AABB.max.y; NearPoint.y = AABB.min.y; }
        else                  { FarPoint.y  = AABB.min.y; NearPoint.y = AABB.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = AABB.max.z; NearPoint.z = AABB.min.z; }
        else                  { FarPoint.z  = AABB.min.z; NearPoint.z = AABB.max.z; }

        // If near extreme point is outside, then the AABB is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return cgVolumeQuery::Outside;

        // If far extreme point is outside, then the AABB is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;
        else
            FrustumBits |= (0x1 << LastOutside); // We were totally inside this frustum plane, update our bit set

    } // End if last outside plane specified

    // Loop through all the planes
    for ( size_t i = 0; i < 6; i++ )
    {
        // Check the bit in the uchar passed to see if it should be tested (if it's 1, it's already passed)
        if ( ((FrustumBits >> i) & 0x1) == 0x1 )
            continue;

        // If 'last outside plane' index was specified, skip if it matches the plane index
        if ( LastOutside >= 0 && LastOutside == (cgInt8)i )
            continue;

        // Calculate near / far extreme points
        const cgPlane & Plane = planes[i];
        if ( Plane.a > 0.0f ) { FarPoint.x  = AABB.max.x; NearPoint.x = AABB.min.x; }
        else                  { FarPoint.x  = AABB.min.x; NearPoint.x = AABB.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = AABB.max.y; NearPoint.y = AABB.min.y; }
        else                  { FarPoint.y  = AABB.min.y; NearPoint.y = AABB.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = AABB.max.z; NearPoint.z = AABB.min.z; }
        else                  { FarPoint.z  = AABB.min.z; NearPoint.z = AABB.max.z; }

        // If near extreme point is outside, then the AABB is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
        {
            // Update the 'last outside' index and return.
            LastOutside = (cgInt8)i;
            return cgVolumeQuery::Outside;

        } // End if outside frustum plane

        // If far extreme point is outside, then the AABB is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;
        else
            FrustumBits |= (0x1 << i); // We were totally inside this frustum plane, update our bit set

    } // Next Plane

    // None outside
    LastOutside = -1;
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : classifyAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifyAABB( const cgBoundingBox & AABB, const cgTransform & transform, cgUInt8 & FrustumBits, cgInt8 & LastOutside ) const
{
    cgBoundingBox Bounds = cgBoundingBox::transform( AABB, transform );

    // If the 'last outside plane' index was specified, test it first!
    cgVector3 NearPoint, FarPoint;
    cgVolumeQuery::Class Result = cgVolumeQuery::Inside;
    if ( LastOutside >= 0 && ( ((FrustumBits >> LastOutside) & 0x1) == 0x0 ) )
    {
        const cgPlane & Plane = planes[LastOutside];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) { FarPoint.x  = Bounds.max.x; NearPoint.x = Bounds.min.x; }
        else                  { FarPoint.x  = Bounds.min.x; NearPoint.x = Bounds.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = Bounds.max.y; NearPoint.y = Bounds.min.y; }
        else                  { FarPoint.y  = Bounds.min.y; NearPoint.y = Bounds.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = Bounds.max.z; NearPoint.z = Bounds.min.z; }
        else                  { FarPoint.z  = Bounds.min.z; NearPoint.z = Bounds.max.z; }

        // If near extreme point is outside, then the Bounds is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return cgVolumeQuery::Outside;

        // If far extreme point is outside, then the Bounds is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;
        else
            FrustumBits |= (0x1 << LastOutside); // We were totally inside this frustum plane, update our bit set

    } // End if last outside plane specified

    // Loop through all the planes
    for ( size_t i = 0; i < 6; i++ )
    {
        // Check the bit in the uchar passed to see if it should be tested (if it's 1, it's already passed)
        if ( ((FrustumBits >> i) & 0x1) == 0x1 )
            continue;

        // If 'last outside plane' index was specified, skip if it matches the plane index
        if ( LastOutside >= 0 && LastOutside == (cgInt8)i )
            continue;

        // Calculate near / far extreme points
        const cgPlane & Plane = planes[i];
        if ( Plane.a > 0.0f ) { FarPoint.x  = Bounds.max.x; NearPoint.x = Bounds.min.x; }
        else                  { FarPoint.x  = Bounds.min.x; NearPoint.x = Bounds.max.x; }

        if ( Plane.b > 0.0f ) { FarPoint.y  = Bounds.max.y; NearPoint.y = Bounds.min.y; }
        else                  { FarPoint.y  = Bounds.min.y; NearPoint.y = Bounds.max.y; }

        if ( Plane.c > 0.0f ) { FarPoint.z  = Bounds.max.z; NearPoint.z = Bounds.min.z; }
        else                  { FarPoint.z  = Bounds.min.z; NearPoint.z = Bounds.max.z; }

        // If near extreme point is outside, then the Bounds is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
        {
            // Update the 'last outside' index and return.
            LastOutside = (cgInt8)i;
            return cgVolumeQuery::Outside;

        } // End if outside frustum plane

        // If far extreme point is outside, then the Bounds is intersecting the frustum
        if ( cgPlane::dotCoord( Plane, FarPoint ) > 0.0f )
            Result = cgVolumeQuery::Intersect;
        else
            FrustumBits |= (0x1 << i); // We were totally inside this frustum plane, update our bit set

    } // Next Plane

    // None outside
    LastOutside = -1;
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : testAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testAABB( const cgBoundingBox & AABB ) const
{
    // Loop through all the planes
    cgVector3 NearPoint;
    for ( size_t i = 0; i < 6; i++ )
    {
        const cgPlane & Plane = planes[i];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) NearPoint.x = AABB.min.x;
        else                  NearPoint.x = AABB.max.x;

        if ( Plane.b > 0.0f ) NearPoint.y = AABB.min.y;
        else                  NearPoint.y = AABB.max.y;

        if ( Plane.c > 0.0f ) NearPoint.z = AABB.min.z;
        else                  NearPoint.z = AABB.max.z;

        // If near extreme point is outside, then the AABB is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return false;

    } // Next Plane

    // Intersecting / inside
    return true;
}

//-----------------------------------------------------------------------------
//  Name : testAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testAABB( const cgBoundingBox & AABB, const cgTransform & transform ) const
{
    // Transform bounds.
    cgBoundingBox Bounds = cgBoundingBox::transform( AABB, transform );
    
    // Loop through all the planes
    cgVector3 NearPoint;
    for ( size_t i = 0; i < 6; i++ )
    {
        const cgPlane & Plane = planes[i];

        // Calculate near / far extreme points
        if ( Plane.a > 0.0f ) NearPoint.x = Bounds.min.x;
        else                  NearPoint.x = Bounds.max.x;

        if ( Plane.b > 0.0f ) NearPoint.y = Bounds.min.y;
        else                  NearPoint.y = Bounds.max.y;

        if ( Plane.c > 0.0f ) NearPoint.z = Bounds.min.z;
        else                  NearPoint.z = Bounds.max.z;

        // If near extreme point is outside, then the AABB is totally outside the frustum
        if ( cgPlane::dotCoord( Plane, NearPoint ) > 0.0f )
            return false;

    } // Next Plane

    // Intersecting / inside
    return true;
}

//-----------------------------------------------------------------------------
//  Name : testExtrudedAABB()
/// <summary>
/// Determine whether or not the box passed, extruded out away from the
/// specified origin by a required distance, falls within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testExtrudedAABB( const cgExtrudedBoundingBox & Box ) const
{
    bool     bIntersect1, bIntersect2;
    cgUInt32 i;

    //  Build an imaginary sphere around the origin, representing the volume of
	//  max attenuation -- if this doesn't intersect the view frustum, then
	//  this caster can be trivially rejected.
	if ( testSphere( Box.projectionPoint, Box.projectionRange ) == false )
		return false;

    // Test frustum edges against extruded box.
    using namespace cgVolumeGeometry;
    bIntersect1 = (Box.testLine( points[LeftBottomFar], points[LeftBottomNear] )) ||
                  (Box.testLine( points[LeftBottomNear], points[RightBottomNear] )) ||
                  (Box.testLine( points[RightBottomNear], points[RightBottomFar] )) ||
                  (Box.testLine( points[RightBottomFar], points[LeftBottomFar] )) ||
                  (Box.testLine( points[RightBottomFar], points[LeftTopFar] )) ||
                  (Box.testLine( points[RightBottomNear], points[RightTopNear] )) ||
                  (Box.testLine( points[LeftBottomFar], points[LeftTopFar] )) ||
                  (Box.testLine( points[LeftBottomNear], points[LeftTopNear] )) ||
                  (Box.testLine( points[LeftTopNear], points[LeftTopFar] )) ||
                  (Box.testLine( points[LeftTopFar], points[RightTopNear] )) ||
                  (Box.testLine( points[RightTopFar], points[RightTopNear] )) ||
                  (Box.testLine( points[RightTopNear], points[LeftTopNear] ));

	
    // Test extruded box edges against frustum
    bIntersect2 = false;
	for ( i = 0; (i < Box.edgeCount) && (bIntersect1 == false && bIntersect2 == false); ++i )
	{
        cgVector3 vRay, vPoint1, vPoint2;

        // Retrieve this silhouette edge from the extruded box
        Box.getEdge( i, vPoint1, vPoint2 );
        
        // Build an edge that extends for Box.ProjectionLength distance from 
        // the projection point and test for an intersection against the frustum.
        cgVector3::normalize( vRay, vPoint1 - Box.projectionPoint );
		vRay = Box.projectionPoint + (vRay * Box.projectionRange);    
		bIntersect2 |= testLine( vPoint1, vRay );                     
		cgVector3::normalize( vRay, vPoint2 - Box.projectionPoint );
		vRay = Box.projectionPoint + (vRay * Box.projectionRange);    
		bIntersect2 |= testLine( vPoint2, vRay );
	
    } // Next Extruded Edge

    // Intersects?
	return (bIntersect1 || bIntersect2);

}

//-----------------------------------------------------------------------------
//  Name : classifySphere ()
/// <summary>
/// Determine whether or not the sphere passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifySphere( const cgVector3 & vecCenter, cgFloat fRadius ) const
{
    cgVolumeQuery::Class Result = cgVolumeQuery::Inside;
    
    // Test frustum planes
    for ( cgUInt32 i = 0; i < 6; ++i )
    {
        cgFloat fDot = cgPlane::dotCoord( planes[i], vecCenter );

        // Sphere entirely in front of plane
        if ( fDot >= fRadius )
            return cgVolumeQuery::Outside;
        
        // Sphere spans plane
        if ( fDot >= -fRadius )
            Result = cgVolumeQuery::Intersect;

    } // Next Plane

    // Return the result
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : testSphere ()
/// <summary>
/// Determine whether or not the sphere passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testSphere( const cgVector3 & vecCenter, cgFloat fRadius ) const
{
    // Test frustum planes
    for ( cgInt i = 0; i < 6; ++i )
    {
        cgFloat fDot = cgPlane::dotCoord( planes[i], vecCenter );

        // Sphere entirely in front of plane
        if ( fDot >= fRadius )
            return false;

    } // Next Plane

    // Intersects
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sweptSphereIntersectPlane () (Private, Static)
/// <summary>
/// Determine whether or not the specified sphere, swept along the
/// provided direction vector, intersects a plane.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::sweptSphereIntersectPlane( cgFloat & t0, cgFloat & t1, const cgPlane & Plane, const cgVector3 & vecCenter, cgFloat fRadius, const cgVector3 & vecSweepDirection )
{
    cgFloat b_dot_n = cgPlane::dotCoord( Plane, vecCenter );
    cgFloat d_dot_n = cgPlane::dotNormal( Plane, vecSweepDirection );

    if ( d_dot_n == 0.0f )
    {
        if ( b_dot_n <= fRadius )
        {
            //  Effectively infinity
            t0 = 0.0f;
            t1 = FLT_MAX;
            return true;
        
        } // End if infinity
        else
            return false;
    
    } // End if runs parallel to plane
    else
    {
        // Compute the two possible intersections
        cgFloat tmp0 = ( fRadius - b_dot_n) / d_dot_n;
        cgFloat tmp1 = (-fRadius - b_dot_n) / d_dot_n;
        t0 = std::min<cgFloat>(tmp0, tmp1);
        t1 = std::max<cgFloat>(tmp0, tmp1);
        return true;
    
    } // End if intersection
}

//-----------------------------------------------------------------------------
//  Name : testSweptSphere ()
/// <summary>
/// Determine whether or not the specified sphere, swept along the
/// provided direction vector, intersects the frustum in some way.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testSweptSphere( const cgVector3 & vecCenter, cgFloat fRadius, const cgVector3 & vecSweepDirection ) const
{
    cgUInt32  i, nCount = 0;
    cgFloat   t0, t1, fDisplacedRadius;
    cgFloat   pDisplacements[12];
    cgVector3 vDisplacedCenter;

    // Determine all 12 intersection points of the swept sphere with the view frustum.
    for ( i = 0; i < 6; ++i )
    {
        // Intersects frustum plane?
        if ( sweptSphereIntersectPlane( t0, t1, planes[i], vecCenter, fRadius, vecSweepDirection) == true )
        {
            // ToDo: Possibly needs to be < 0?
            if ( t0 >= 0.0f )
                pDisplacements[nCount++] = t0;
            if ( t1 >= 0.0f )
                pDisplacements[nCount++] = t1;
        
        } // End if intersects
    
    } // Next Plane

    // For all points > 0, displace the sphere along the sweep direction. If the displaced
    // sphere falls inside the frustum then we have an intersection.
    for ( i = 0; i < nCount; ++i )
    {
        vDisplacedCenter = vecCenter + (vecSweepDirection * pDisplacements[i]);
        fDisplacedRadius = fRadius * 1.1f; // Tolerance.
        if ( testSphere( vDisplacedCenter, fDisplacedRadius ) == true )
            return true;
    
    } // Next Intersection
    
    // None of the displaced spheres intersected the frustum
    return false;
}

//-----------------------------------------------------------------------------
//  Name : testPoint ()
/// <summary>
/// Determine whether or not the specified point falls within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testPoint( const cgVector3 & vecPoint ) const
{
    return testSphere( vecPoint, 0.0f );
}

//-----------------------------------------------------------------------------
//  Name : testLine ()
/// <summary>
/// Determine whether or not the line passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testLine( const cgVector3 & v1, const cgVector3 & v2 ) const
{
    cgUInt8   nCode1 = 0, nCode2 = 0;
    cgFloat   fDist1, fDist2, t;
    int       nSide1, nSide2;
    cgVector3 vDir, vIntersect;
    cgUInt32  i;

    // Test each plane
    for ( i = 0; i < 6; ++i )
    {
        // Classify each point of the line against the plane.
        fDist1 = cgPlane::dotCoord( planes[i], v1 );
        fDist2 = cgPlane::dotCoord( planes[i], v2 );
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
            t    = -cgPlane::dotCoord( planes[i], v1 ) / cgPlane::dotNormal( planes[i], vDir );
            
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
//  Name : classifyPlane ()
/// <summary>
/// Classify the frustum with respect to the plane
/// </summary>
//-----------------------------------------------------------------------------
cgVolumeQuery::Class cgFrustum::classifyPlane( const cgPlane & plane ) const
{
	cgUInt32 nInFrontCount = 0;
	cgUInt32 nBehindCount  = 0;

    // Test frustum points
    for ( cgUInt32 i = 0; i < 8; ++i )
    {
		cgFloat fDot = cgPlane::dotCoord( plane, points[ i ] );
        if ( fDot > 0.0f )
			nInFrontCount++;
		else if ( fDot < 0.0f )
			nBehindCount++;

    } // Next Plane

    // Frustum entirely in front of plane
    if ( nInFrontCount == 8 )
        return cgVolumeQuery::Outside;
    
    // Frustum entire behind plane
    if ( nBehindCount == 8 )
        return cgVolumeQuery::Inside;

    // Return intersection (spanning the plane)
    return cgVolumeQuery::Intersect;
}


//-----------------------------------------------------------------------------
//  Name : testFrustum ()
/// <summary>
/// Determine whether or not the frustum passed is within this one.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::testFrustum( const cgFrustum & f ) const
{
    using namespace cgVolumeGeometry;
    
    // A -> B
    bool bIntersect1;
    bIntersect1 = testLine( f.points[LeftBottomFar], f.points[LeftBottomNear] ) ||
                  testLine( f.points[LeftBottomNear], f.points[RightBottomNear] ) ||
                  testLine( f.points[RightBottomNear], f.points[RightBottomFar] ) ||
                  testLine( f.points[RightBottomFar], f.points[LeftBottomFar] ) ||
                  testLine( f.points[RightBottomFar], f.points[RightTopFar] ) ||
                  testLine( f.points[RightBottomNear], f.points[RightTopNear] ) ||
                  testLine( f.points[LeftBottomFar], f.points[LeftTopFar] ) ||
                  testLine( f.points[LeftBottomNear], f.points[LeftTopNear] ) ||
                  testLine( f.points[LeftTopNear], f.points[LeftTopFar] ) ||
                  testLine( f.points[LeftTopFar], f.points[RightTopFar] ) ||
                  testLine( f.points[RightTopFar], f.points[RightTopNear] ) ||
                  testLine( f.points[RightTopNear], f.points[LeftTopNear] );
    
    // Early out
    if ( bIntersect1 )
        return true;
    
    // B -> A
    bool bIntersect2;
    bIntersect2 = f.testLine( points[LeftBottomFar], points[LeftBottomNear] ) ||
                  f.testLine( points[LeftBottomNear], points[RightBottomNear] ) ||
                  f.testLine( points[RightBottomNear], points[RightBottomFar] ) ||
                  f.testLine( points[RightBottomFar], points[LeftBottomFar] ) ||
                  f.testLine( points[RightBottomFar], points[LeftTopFar] ) ||
                  f.testLine( points[RightBottomNear], points[RightTopNear] ) ||
                  f.testLine( points[LeftBottomFar], points[LeftTopFar] ) ||
                  f.testLine( points[LeftBottomNear], points[LeftTopNear] ) ||
                  f.testLine( points[LeftTopNear], points[LeftTopFar] ) ||
                  f.testLine( points[LeftTopFar], points[RightTopNear] ) ||
                  f.testLine( points[RightTopFar], points[RightTopNear] ) ||
                  f.testLine( points[RightTopNear], points[LeftTopNear] );
    
    // Return intersection result
    return bIntersect2;

}

//-----------------------------------------------------------------------------
//  Name : transform ()
/// <summary>
/// Transforms this frustum by the specified matrix.
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum & cgFrustum::transform( const cgTransform & t )
{
    cgPlane NewPlane;
    cgMatrix mtxIT, mtx = t;
    cgMatrix::inverse( mtxIT, mtx );
    cgMatrix::transpose( mtxIT, mtxIT );

    // Transform planes
    for ( cgUInt32 i = 0; i < 6; ++i )
    {   
        cgPlane::transform( NewPlane, planes[i], mtxIT );
        cgPlane::normalize( planes[i], NewPlane );
    
    } // Next Plane

    // Transform points
    for ( cgUInt32 i = 0; i < 8; ++i )
        cgVector3::transformCoord( points[i], points[i], mtx );

    // Transform originating position.
    cgVector3::transformCoord( position, position, mtx );

    // Return reference to self.
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : transform () (Static)
/// <summary>
/// Transforms the specified frustum by the provide matrix and return the new
/// resulting frustum as a copy.
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum cgFrustum::transform( cgFrustum Frustum, const cgTransform & t )
{
    return Frustum.transform( t );
}

//-----------------------------------------------------------------------------
//  Name : operator== ( const cgFrustum& )
/// <summary>
/// Determine whether or not the two frustums match.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFrustum::operator == ( const cgFrustum & Frustum ) const
{
    // Compare planes.
    for ( int i = 0; i < 6; ++i )
    {
        const cgPlane & p1 = planes[i];
        const cgPlane & p2 = Frustum.planes[i];
        if ( (fabsf( p1.a - p2.a ) <= CGE_EPSILON && fabsf( p1.b - p2.b ) <= CGE_EPSILON && 
              fabsf( p1.c - p2.c ) <= CGE_EPSILON && fabsf( p1.d - p2.d ) <= CGE_EPSILON ) == false )
              return false;

    } // Next Plane

    // Match
    return true;
}