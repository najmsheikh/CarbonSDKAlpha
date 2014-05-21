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
// Name : cgDisplacementShape.cpp                                            //
//                                                                           //
// Desc : Class implementing collision *only* properties for an arbitrary    //
//        displacement / height map (such as that used by a terrain). This   //
//        shape cannot be used with a dynamic rigid body, and is used        //
//        primarily for collision detection only.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgDisplacementShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgDisplacementShape.h>
#include <Physics/cgPhysicsWorld.h>
/*#include <Physics/cgPhysicsBody.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgMesh.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgCollision.h>*/

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgDisplacementShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDisplacementShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDisplacementShape::cgDisplacementShape( cgPhysicsWorld * world, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize ) : 
    cgPhysicsShape( world ),
    mDisplacement( displacement ),
    mMapPitch( mapPitch ),
    mSection( section ),
    mCellSize( cellSize )
{
    // Initialize the local space bounding box.
    computeBounds();
    
    // Create the 'user' mesh object used to hook into newton.
    initMeshHooks();
}

//-----------------------------------------------------------------------------
//  Name : cgDisplacementShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDisplacementShape::cgDisplacementShape( cgPhysicsWorld * world, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize, const cgTransform & offset ) : 
    cgPhysicsShape( world ),
    mDisplacement( displacement ),
    mMapPitch( mapPitch ),
    mSection( section ),
    mCellSize( cellSize ),
    mOffset( offset )
{
    // Initialize the local space bounding box.
    computeBounds();

    // Create the 'user' mesh object used to hook into newton.
    initMeshHooks();
}

//-----------------------------------------------------------------------------
//  Name : ~cgDisplacementShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDisplacementShape::~cgDisplacementShape()
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
void cgDisplacementShape::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release internal references.
    mDisplacement = CG_NULL;
    
    // Dispose base class if requested.
    if ( bDisposeBase )
        cgPhysicsShape::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDisplacementShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DisplacementShape )
        return true;

    // Supported by base?
    return cgPhysicsShape::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : computeBounds () (Protected)
/// <summary>
/// Compute the local space bounding box of the displacement data.
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::computeBounds( )
{
    // Compute the minimum and maximum height range for the values contained
    // in the specified region of the displacement map.
    cgInt32 minHeight = 0x7FFF, maxHeight = -0x7FFF;
    const cgInt16 * buffer = &mDisplacement[mSection.left + mSection.top * mMapPitch.width];
    for ( cgInt32 y = mSection.top; y < mSection.bottom; ++y )
    {
        for ( cgInt32 x = mSection.left; x < mSection.right; ++x, ++buffer )
        {
            if ( *buffer < minHeight )
                minHeight = *buffer;
            if ( *buffer > maxHeight )
                maxHeight = *buffer;

        } // Next Column

        // Move to start of next row.
        buffer += mMapPitch.width - mSection.width();

    } // Next Row
    
    // Compute the final bounding box.
    mBounds.max.x = mSection.width() * mCellSize.x;
    mBounds.max.y = maxHeight * mCellSize.y;
    mBounds.max.z = 0;
    mBounds.min.x = 0;
    mBounds.min.y = minHeight * mCellSize.y;
    mBounds.min.z = mSection.height() * -mCellSize.z;
}

//-----------------------------------------------------------------------------
//  Name : setOffsetTransform ()
/// <summary>
/// Set a new offset transform that describes how the displaced shape is to be
/// scaled, sheared, rotated and translated with respect its parent body.
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::setOffsetTransform( const cgTransform & offset )
{
    cgToDo( "Physics", "Update cache???" );

    // Is this a no-op?
    if ( offset.compare( mOffset, CGE_EPSILON ) == 0 )
        return;

    // Update the mesh offset transform.
    mOffset = offset;

    // Release the old newton 'user mesh' collision shape.
    NewtonReleaseCollision( mWorld->getInternalWorld(), mShape );
    mShape = CG_NULL;

    // Transform the bounding box of the shape by the new offset transform
    // specified for this shape. This will become the final "object space"
    // box as understood by newton.
    cgBoundingBox bounds = mWorld->toPhysicsScale( mBounds );
    bounds.transform( mOffset );

    // Create the new newton shape with updated bounds.
    mShape = NewtonCreateUserMeshCollision( mWorld->getInternalWorld(), bounds.min, bounds.max, this,
                                            onCollide, onRayHit, onDestroy, onGetCollisionInfo, 
                                            onGetFacesInAABB, getReferenceId() );

    // Now we must iterate through all physics bodies that reference
    // this shape and update them with this new collision.
    ReferenceMap::iterator itRef;
    for ( itRef = mReferencedBy.begin(); itRef != mReferencedBy.end(); ++itRef )
    {
        if ( itRef->second.reference->queryReferenceType( RTID_PhysicsBody ) )
        {
            cgPhysicsBody * pBody = (cgPhysicsBody*)itRef->second.reference;
            NewtonBodySetCollision( pBody->getInternalBody(), mShape );

        } // End if RTID_PhysicsBody

    } // Next reference holder

}

//-----------------------------------------------------------------------------
//  Name : getOffsetTransform ()
/// <summary>
/// Get the offset transform that describes how the shape is to be scaled,
/// sheared, rotated and translated with respect its parent body.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgDisplacementShape::getOffsetTransform( ) const
{
    return mOffset;
}

//-----------------------------------------------------------------------------
//  Name : initMeshHooks () (Protected)
/// <summary>
/// Initialize the internal hooks that allow the physics engine to retrieve
/// information relating to this collision shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::initMeshHooks( )
{
    // Transform the bounding box of the shape by the offset transform
    // specified for this shape. This will become the final "object space"
    // box as understood by newton.
    cgBoundingBox bounds = mWorld->toPhysicsScale( mBounds );
    bounds.transform( mOffset );

    // Create the newton shape.
    mShape = NewtonCreateUserMeshCollision( mWorld->getInternalWorld(), bounds.min, bounds.max, this,
                                            onCollide, onRayHit, onDestroy, onGetCollisionInfo, 
                                            onGetFacesInAABB, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : onCollide() (Protected, Callback)
/// <summary>
/// Called whenever a mesh collision is checked by the physics engine when the 
/// shape potentially collides with another object (an object's AABB overlaps 
/// the mesh's AABB). 
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::onCollide( NewtonUserMeshCollisionCollideDesc * const collideDescData )
{
    // Retrieve the Carbon side shape to which this callback applies.
    cgDisplacementShape * thisPointer = (cgDisplacementShape*)collideDescData->m_userData;

    // Transform the bounding box into the space of the displacement map.
    cgTransform inverseOffset;
    cgBoundingBox bounds( collideDescData->m_boxP0, collideDescData->m_boxP1 );
    bounds.transform( thisPointer->mOffset.inverse( inverseOffset ) );
    bounds *= thisPointer->mWorld->fromPhysicsScale();

    // Anything to do? (Intersects our own local bounding box?)
    if ( !thisPointer->mBounds.intersect( bounds ) )
        return; // Nothing hit

    // Compute the minimum and maximum extents of the bounding box
    // in the space of the map so that we know which cells to include.
    cgInt32 minX = (cgInt32)floorf(bounds.min.x / thisPointer->mCellSize.x);
    cgInt32 minY = (cgInt32)floorf(bounds.max.z / -thisPointer->mCellSize.z);
    cgInt32 maxX = (cgInt32)ceilf(bounds.max.x / thisPointer->mCellSize.x);
    cgInt32 maxY = (cgInt32)ceilf(bounds.min.z / -thisPointer->mCellSize.z);

    // Clamp them as necessary.
    cgInt32 sectionWidth  = thisPointer->mSection.width();
    cgInt32 sectionHeight = thisPointer->mSection.height();
    minX = max(0,min(sectionWidth - 1, minX));
    maxX = max(0,min(sectionWidth - 1, maxX));
    minY = max(0,min(sectionHeight - 1, minY));
    maxY = max(0,min(sectionHeight - 1, maxY));

    // Return if the bounds are degenerate (no op)
    if ( maxX - minX <= 0 || maxY - minY <= 0 )
        return;

    // Size the output arrays appropriately if they do not contain enough elements.
    // First the vertex array which will need to store enough data for the entire
    // displacement grid.
    static cgArray<cgVector3> verticesOut;
    const size_t maxVertexCount = ((maxX-minX)+1) * ((maxY-minY)+1);
    size_t bufferSize = max( maxVertexCount, 1 );
    if ( bufferSize > verticesOut.size() )
        verticesOut.resize( bufferSize );

    // Next, the index array (6 indices per quad)
    static cgArray<cgInt> indicesOut;
    const size_t maxFaceCount = (maxX-minX) * (maxY-minY) * 2;
    const size_t maxIndexCount = maxFaceCount * 3;
    bufferSize = max( maxIndexCount, 1 );
    if ( bufferSize > indicesOut.size() )
        indicesOut.resize( bufferSize );
    
    // Next the user attributes buffer which will contain the triangle
    // identifiers useful in collision callbacks.
    static cgArray<cgInt> attributesOut;
    bufferSize = max( maxFaceCount, 1 );
    if ( bufferSize > attributesOut.size() )
        attributesOut.resize( bufferSize, 0 );
    
    // Finally, the face index count buffer. This is never explicitly populated
    // and will always contain a default value of '3' in every element.
    static cgArray<cgInt> faceIndexCountOut;
    bufferSize = max( maxFaceCount, 1 );
    if ( bufferSize > faceIndexCountOut.size() )
        faceIndexCountOut.resize( bufferSize, 3 );

    // ToDo: optimize displacement map array lookups and index calculations.

    // Now populate the data buffers one row at a time.
    cgInt * indices = &indicesOut[0];
    cgVector3 * vertices = &verticesOut[0];
    cgInt32 pitch = (maxX - minX) + 1;
    if ( thisPointer->mOffset.isIdentity() )
    {
        for ( cgInt32 y = minY, cy = 0; y <= maxY; ++y, ++cy )
        {
            // For each column
            for ( cgInt32 x = minX, cx = 0; x <= maxX; ++x, ++cx, ++vertices )
            {
                // Do not add triangle data for last column / row
                if ( y < maxY && x < maxX )
                {
                    // Build first triangle
                    *indices++ = cx + cy * pitch;
                    *indices++ = (cx+1) + cy * pitch;
                    *indices++ = cx + (cy+1) * pitch;
                    
                    // Build second triangle
                    *indices++ = (cx+1) + cy * pitch;
                    *indices++ = (cx+1) + (cy+1) * pitch;
                    *indices++ = cx + (cy+1) * pitch;

                } // End if last column / row

                // Calculate the vertex
                vertices->x = (cgFloat)x * thisPointer->mCellSize.x;
                vertices->z = -(cgFloat)y * thisPointer->mCellSize.z;
                vertices->y = thisPointer->mDisplacement[ (x + thisPointer->mSection.left) + (y + thisPointer->mSection.top) * thisPointer->mMapPitch.width ] * thisPointer->mCellSize.y;

                // Convert into physics scale.
                thisPointer->mWorld->toPhysicsScale(*vertices);
                
            } // Next column
    
        } // Next row
    
    } // End if identity offset
    else
    {
        for ( cgInt32 y = minY, cy = 0; y <= maxY; ++y, ++cy )
        {
            // For each column
            for ( cgInt32 x = minX, cx = 0; x <= maxX; ++x, ++cx, ++vertices )
            {
                // Do not add triangle data for last column / row
                if ( y < maxY && x < maxX )
                {
                    // Build first triangle
                    *indices++ = cx + cy * pitch;
                    *indices++ = (cx+1) + cy * pitch;
                    *indices++ = cx + (cy+1) * pitch;
                    
                    // Build second triangle
                    *indices++ = (cx+1) + cy * pitch;
                    *indices++ = (cx+1) + (cy+1) * pitch;
                    *indices++ = cx + (cy+1) * pitch;

                } // End if last column / row

                // Calculate the vertex
                vertices->x = (cgFloat)x * thisPointer->mCellSize.x;
                vertices->z = -(cgFloat)y * thisPointer->mCellSize.z;
                vertices->y = thisPointer->mDisplacement[ (x + thisPointer->mSection.left) + (y + thisPointer->mSection.top) * thisPointer->mMapPitch.width ] * thisPointer->mCellSize.y;

                // Transform into world space
                cgVector3::transformCoord( *vertices, *vertices, thisPointer->mOffset );

                // Convert into physics scale.
                thisPointer->mWorld->toPhysicsScale(*vertices);
                
            } // Next column
    
        } // Next row

    } // End if !identity

    // Output data.
    collideDescData->m_vertexStrideInBytes  = sizeof(cgVector3);
    collideDescData->m_faceCount            = maxFaceCount;
    collideDescData->m_vertex               = (cgFloat*)&verticesOut[0];
    collideDescData->m_userAttribute        = &attributesOut[0];
    collideDescData->m_faceIndexCount       = &faceIndexCountOut[0];
    collideDescData->m_faceVertexIndex      = &indicesOut[0];
}

//-----------------------------------------------------------------------------
//  Name : onRayHit() (Protected, Callback)
/// <summary>
/// Called whenever a raycast is performed on a mesh collision shape. 
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgDisplacementShape::onRayHit( NewtonUserMeshCollisionRayHitDesc * const lineDescData )
{
    cgToDoAssert( "Physics", "Unhandled callback detected (onRayHit())." );

    // Nothing was found
    return 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : onDestroy() (Protected, Callback)
/// <summary>
/// Called whenever the physics engine's representation of this mesh is 
/// destroyed.
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::onDestroy( void * const userData )
{
    // Nothing in this implementation.
}

//-----------------------------------------------------------------------------
//  Name : onGetCollisionInfo() (Protected, Callback)
/// <summary>
/// Called whenever information is requested about this mesh collision shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgDisplacementShape::onGetCollisionInfo( void * const userData, NewtonCollisionInfoRecord * const infoRecord )
{
    cgToDoAssert( "Physics", "Unhandled callback detected (onGetCollisionInfo())." );
}

//-----------------------------------------------------------------------------
//  Name : onGetFacesInAABB() (Protected, Callback)
/// <summary>
/// Called whenever a list of faces that fall within an AABB are required.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgDisplacementShape::onGetFacesInAABB( void * const userData, const cgFloat * const p0, const cgFloat * const p1, const cgFloat ** const vertexArray, cgInt * const vertexCount, cgInt * const vertexStrideInBytes, const cgInt * const indexList, cgInt maxIndexCount, const cgInt * const userDataList )
{
    cgToDoAssert( "Physics", "Unhandled callback detected (onGetFacesInAABB())." );
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgDisplacementShape::compare( cgPhysicsShape * shape ) const
{
    // Compare base properties first.
    cgInt result = cgPhysicsShape::compare( shape );
    if ( result != 0 )
        return result;

    // Now compare custom properties.
    cgDisplacementShape * displacementShape = (cgDisplacementShape*)shape;
    result = (mDisplacement == displacementShape->mDisplacement) ? 0 : (mDisplacement < displacementShape->mDisplacement) ? -1 : 1;
    if ( result != 0 )
        return result;
    result = (mMapPitch == displacementShape->mMapPitch) ? 0 : (mMapPitch < displacementShape->mMapPitch) ? -1 : 1;
    if ( result != 0 )
        return result;
    result = (mSection == displacementShape->mSection) ? 0 : (mSection < displacementShape->mSection) ? -1 : 1;
    if ( result != 0 )
        return result;
    result = (mCellSize == displacementShape->mCellSize) ? 0 : (mCellSize < displacementShape->mCellSize) ? -1 : 1;
    if ( result != 0 )
        return result;
    result = mOffset.compare( displacementShape->mOffset, CGE_EPSILON );
    if ( result != 0 )
        return result;

    // Equivalent
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cgDisplacementShapeCacheKey Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDisplacementShapeCacheKey () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDisplacementShapeCacheKey::cgDisplacementShapeCacheKey( const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize, const cgTransform & offset ) : 
    cgPhysicsShapeCacheKey( CG_NULL ), mDisplacement(displacement), mMapPitch(mapPitch), mSection(section), mCellSize(cellSize), mOffset(offset)
{
}

//-----------------------------------------------------------------------------
//  Name : operator< () (Virtual)
/// <summary>
/// Less-than comparison operator.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDisplacementShapeCacheKey::operator<( const cgPhysicsShapeCacheKey & key ) const
{
    cgAssert( key.mShape != CG_NULL );
    
    // Mesh type?
    if ( RTID_DisplacementShape == key.mShape->getReferenceType() )
    {
        cgDisplacementShape * displacementShape = (cgDisplacementShape*)key.mShape;
        cgInt result = (mDisplacement == displacementShape->mDisplacement) ? 0 : (mDisplacement < displacementShape->mDisplacement) ? -1 : 1;
        if ( result != 0 )
            return (result < 0);
        result = (mMapPitch == displacementShape->mMapPitch) ? 0 : (mMapPitch < displacementShape->mMapPitch) ? -1 : 1;
        if ( result != 0 )
            return (result < 0);
        result = (mSection == displacementShape->mSection) ? 0 : (mSection < displacementShape->mSection) ? -1 : 1;
        if ( result != 0 )
            return (result < 0);
        result = (mCellSize == displacementShape->mCellSize) ? 0 : (mCellSize < displacementShape->mCellSize) ? -1 : 1;
        if ( result != 0 )
            return (result < 0);
        result = mOffset.compare( displacementShape->mOffset, CGE_EPSILON );
        if ( result != 0 )
            return (result < 0);

        // Exact match (not less than).
        return false;

    } // End if matching type
    else if ( RTID_DisplacementShape < key.mShape->getReferenceType() )
        return true;
    else
        return false;
}