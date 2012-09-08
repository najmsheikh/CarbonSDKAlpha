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
// Name : cgMeshShape.cpp                                                    //
//                                                                           //
// Desc : Class implementing collision *only* properties for an arbitrary    //
//        mesh. This shape cannot be used with a dynamic rigid body, and is  //
//        used primarily for collision detection only.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgMeshShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgMeshShape.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgMesh.h>
#include <Math/cgBoundingBox.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgMeshShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMeshShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshShape::cgMeshShape( cgPhysicsWorld * pWorld, const cgMeshHandle & hMesh ) : cgPhysicsShape( pWorld )
{
    // Initialize variables to sensible defaults
    mMesh     = hMesh;
    mRootNode = CG_NULL;

    // Build any required broadphase data, including a suitable BVH tree.
    buildBroadphaseData();

    // Create the 'user' mesh object used to hook into newton.
    initMeshHooks();
}

//-----------------------------------------------------------------------------
//  Name : cgMeshShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshShape::cgMeshShape( cgPhysicsWorld * pWorld, const cgMeshHandle & hMesh, const cgTransform & Offset ) : cgPhysicsShape( pWorld )
{
    // Initialize variables to sensible defaults
    mMesh     = hMesh;
    mOffset    = Offset;
    mRootNode = CG_NULL;

    // Build any required broadphase data, including a suitable BVH tree.
    buildBroadphaseData();

    // Create the 'user' mesh object used to hook into newton.
    initMeshHooks();
}

//-----------------------------------------------------------------------------
//  Name : ~cgMeshShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshShape::~cgMeshShape()
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
void cgMeshShape::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Destroy BVH tree
    delete mRootNode;
    mRootNode = CG_NULL;

    // Release internal references.
    mMesh.close();
    
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
bool cgMeshShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_MeshShape )
        return true;

    // Supported by base?
    return cgPhysicsShape::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : setOffsetTransform ()
/// <summary>
/// Set a new offset transform that describes how the mesh is to be scaled,
/// sheared, rotated and translated with respect its parent body.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::setOffsetTransform( const cgTransform & Offset )
{
    cgToDo( "Physics", "Update cache???" );

    // Is this a no-op?
    if ( Offset.compare( mOffset, CGE_EPSILON ) == 0 )
        return;

    // Update the mesh offset transform.
    mOffset = Offset;

    // Release the old newton 'user mesh' collision shape.
    NewtonReleaseCollision( mWorld->getInternalWorld(), mShape );
    mShape = CG_NULL;

    // Transform the bounding box of the mesh by the new offset transform
    // specified for this shape. This will become the final "object space"
    // box as understood by newton.
    cgBoundingBox Bounds = mWorld->toPhysicsScale( mMeshBounds );
    Bounds.transform( mOffset );

    // Create the new newton shape with updated bounds.
    mShape = NewtonCreateUserMeshCollision( mWorld->getInternalWorld(), Bounds.min, Bounds.max, this,
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
/// Get the offset transform that describes how the mesh is to be scaled,
/// sheared, rotated and translated with respect its parent body.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgMeshShape::getOffsetTransform( ) const
{
    return mOffset;
}

//-----------------------------------------------------------------------------
//  Name : initMeshHooks () (Protected)
/// <summary>
/// Initialize the internal hooks that allow the physics engine to retrieve
/// information relating to this mesh collision shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::initMeshHooks( )
{
    // Transform the bounding box of the mesh by the offset transform
    // specified for this shape. This will become the final "object space"
    // box as understood by newton.
    cgBoundingBox Bounds = mWorld->toPhysicsScale( mMeshBounds );
    Bounds.transform( mOffset );

    // Create the newton shape.
    mShape = NewtonCreateUserMeshCollision( mWorld->getInternalWorld(), Bounds.min, Bounds.max, this,
                                              onCollide, onRayHit, onDestroy, onGetCollisionInfo, 
                                              onGetFacesInAABB, getReferenceId() );
}

//-----------------------------------------------------------------------------
//  Name : buildBroadphaseData () (Protected)
/// <summary>
/// Construct the internal face and cell data that is used to optimize face
/// selection during collision queries.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::buildBroadphaseData( )
{
    // Validate requirements.
    cgMesh * pMesh = mMesh.getResource( true );
    if ( !pMesh || !pMesh->isLoaded() )
        return;

    // Cache the local space bounding box of the mesh for later use.
    mMeshBounds = pMesh->getBoundingBox();

    // Get necessary internal mesh data.
    cgVertexFormat * pFormat = pMesh->getVertexFormat();
    cgUInt32   nFaces     = pMesh->getFaceCount();
    cgUInt32   nStride    = (cgUInt32)pFormat->getStride();
    cgUInt32 * pIndices   = pMesh->getSystemIB();
    cgByte   * pVertices  = pMesh->getSystemVB() + pFormat->getElementOffset( D3DDECLUSAGE_POSITION );

    // Process each triangle in the mesh and build additional face data.
    mFaces.resize( nFaces );
    cgUInt32Array aTreeFaces( nFaces );
    for ( cgUInt32 i = 0; i < nFaces; ++i )
    {
        // Build the bounding box for this face
        cgBoundingBox & Bounds = mFaces[i].bounds;
        Bounds.addPoint( *(cgVector3*)(pVertices + pIndices[i*3] * nStride) );
        Bounds.addPoint( *(cgVector3*)(pVertices + pIndices[(i*3)+1] * nStride) );
        Bounds.addPoint( *(cgVector3*)(pVertices + pIndices[(i*3)+2] * nStride) );

        // We pass in a list of /all/ faces to the top level of the 
        // BVH tree generation process.
        aTreeFaces[ i ] = i;

    } // Next triangle

    // Create the root node of the BVH tree and initialize it to the 
    // full size of the mesh as it exists in its local space.
    mRootNode = new BVHNode();
    mRootNode->bounds = mMeshBounds;

    // Expand bounding box by 1% to allow for overlap.
    cgBoundingBox ExpandedBounds = mRootNode->bounds;
    cgFloat fDiagonal = cgVector3::length( ExpandedBounds.getDimensions() );
    ExpandedBounds.inflate( fDiagonal * 0.005f );
    mRootNode->bounds = ExpandedBounds;

    // Recursively build the tree.
    buildBVHTree( 0, mRootNode, aTreeFaces, mMeshBounds, pVertices, pIndices, nStride );
}

//-----------------------------------------------------------------------------
//  Name : buildBVHTree () (Protected, Recursive)
/// <summary>
/// Recursively build the bounding volume hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::buildBVHTree( cgInt nDepth, BVHNode * pNode, const cgUInt32Array & aFaces, const cgBoundingBox & ParentBounds, cgByte * pVertices, cgUInt32 * pIndices, cgUInt32 nVertexStride )
{
    const cgUInt32 nMaxFaces = 50;
    const cgUInt32 nMaxDepth = 20;

    // ToDo: Additional note: Most likely, all of the faces are in a corner and we just get one node subdividing over and over again.
    // Find a better way to exit in this condition?
    cgToDo( "Physics", "Without the max depth check, this often gets stuck in an infinite recursion. Investigate this." );

    // If there are only a limited number of faces remaining, build a leaf here.
    if ( aFaces.size() <= nMaxFaces || nDepth >= nMaxDepth )
    {
        pNode->leaf = new BVHLeaf();
        pNode->leaf->faces = aFaces;
        return;

    } // End if limit

    // Check the bounding box of each of the faces against
    // the 8 possible child nodes.
    cgBoundingBox ChildBounds;
    cgVector3 vCenter = ParentBounds.getCenter();
    for ( cgInt i = 0; i < 8; ++i )
    {
        // Build an appropriate child bounding volume.
        switch ( i )
        {
            case cgVolumeGeometry::LeftTopFar:
                ChildBounds.min = cgVector3( ParentBounds.min.x, vCenter.y, vCenter.z );
                ChildBounds.max = cgVector3( vCenter.x, ParentBounds.max.y, ParentBounds.max.z );
                break;

            case cgVolumeGeometry::RightTopFar:
                ChildBounds.min = cgVector3( vCenter.x, vCenter.y, vCenter.z );
                ChildBounds.max = cgVector3( ParentBounds.max.x, ParentBounds.max.y, ParentBounds.max.z );
                break;

            case cgVolumeGeometry::RightTopNear:
                ChildBounds.min = cgVector3( vCenter.x, vCenter.y, ParentBounds.min.z );
                ChildBounds.max = cgVector3( ParentBounds.max.x, ParentBounds.max.y, vCenter.z );
                break;

            case cgVolumeGeometry::LeftTopNear:
                ChildBounds.min = cgVector3( ParentBounds.min.x, vCenter.y, ParentBounds.min.z );
                ChildBounds.max = cgVector3( vCenter.x, ParentBounds.max.y, vCenter.z );
                break;

            case cgVolumeGeometry::LeftBottomFar:
                ChildBounds.min = cgVector3( ParentBounds.min.x, ParentBounds.min.y, vCenter.z );
                ChildBounds.max = cgVector3( vCenter.x, vCenter.y, ParentBounds.max.z );
                break;

            case cgVolumeGeometry::RightBottomFar:
                ChildBounds.min = cgVector3( vCenter.x, ParentBounds.min.y, vCenter.z );
                ChildBounds.max = cgVector3( ParentBounds.max.x, vCenter.y, ParentBounds.max.z );
                break;

            case cgVolumeGeometry::RightBottomNear:
                ChildBounds.min = cgVector3( vCenter.x, ParentBounds.min.y, ParentBounds.min.z );
                ChildBounds.max = cgVector3( ParentBounds.max.x, vCenter.y, vCenter.z );
                break;

            case cgVolumeGeometry::LeftBottomNear:
                ChildBounds.min = cgVector3( ParentBounds.min.x, ParentBounds.min.y, ParentBounds.min.z );
                ChildBounds.max = cgVector3( vCenter.x, vCenter.y, vCenter.z );
                break;

        } // End switch

        // Test against a bounding box that is 1% larger to allow for overlap.
        cgBoundingBox ExpandedBounds = ChildBounds;
        cgFloat fDiagonal = cgVector3::length( ExpandedBounds.getDimensions() );
        ExpandedBounds.inflate( fDiagonal * 0.005f );

        // Categorize all faces that have made it thus far.
        cgUInt32Array aChildFaces;
        aChildFaces.reserve( aFaces.size() );
        for ( size_t j = 0; j < aFaces.size(); ++j )
        {
            // Face intersects the bounding volume?
            const cgVector3 & v0 = *(cgVector3*)(pVertices + pIndices[aFaces[j]*3] * nVertexStride);
            const cgVector3 & v1 = *(cgVector3*)(pVertices + pIndices[(aFaces[j]*3)+1] * nVertexStride);
            const cgVector3 & v2 = *(cgVector3*)(pVertices + pIndices[(aFaces[j]*3)+2] * nVertexStride);
            if ( ExpandedBounds.intersect( v0, v1, v2, mFaces[aFaces[j]].bounds ) )
                aChildFaces.push_back( aFaces[j] );

        } // Next Face

        // Build a new child node here if any faces fall into this volume.
        if ( !aChildFaces.empty() )
        {
            // Node is expanded, but the recursive construction
            // process uses the precisely subdivided bounds for the
            // next iteration!
            pNode->children[i] = new BVHNode();
            pNode->children[i]->bounds = ExpandedBounds;
            buildBVHTree( nDepth + 1, pNode->children[i], aChildFaces, ChildBounds, pVertices, pIndices, nVertexStride );

        } // End if faces exist

    } // Next Child

}

//-----------------------------------------------------------------------------
//  Name : collectIntersectedFaces () (Protected, Recursive)
/// <summary>
/// Recursively search the bounding volume hierarchy to find triangles that
/// intersect the specified bounding box.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::collectIntersectedFaces( BVHNode * pNode, const cgBoundingBox & Bounds, FaceIntersectionSet & aFaces, cgByte * pVertices, cgUInt32 * pIndices, cgUInt32 nVertexStride )
{
    // Is leaf?
    if ( pNode->leaf )
    {
        // Check faces.
        cgUInt32Array & aLeafFaces = pNode->leaf->faces;
        for ( size_t i = 0; i < aLeafFaces.size(); ++i )
        {
            // Face intersects the bounding volume?
            cgUInt32 nFaceIndex = aLeafFaces[i];
            const cgVector3 & v0 = *(cgVector3*)(pVertices + pIndices[nFaceIndex*3] * nVertexStride);
            const cgVector3 & v1 = *(cgVector3*)(pVertices + pIndices[(nFaceIndex*3)+1] * nVertexStride);
            const cgVector3 & v2 = *(cgVector3*)(pVertices + pIndices[(nFaceIndex*3)+2] * nVertexStride);
            //if ( Bounds.intersect( mFaces[nFaceIndex].Bounds ) )
            if ( Bounds.intersect( v0, v1, v2, mFaces[nFaceIndex].bounds ) )
                aFaces.insert( nFaceIndex );
                //aFaces.push_back( nFaceIndex );

        } // Next face from leaf
        return;

    } // End if is leaf

    // Check child nodes
    for ( cgInt i = 0; i < 8; ++i )
    {
        if ( pNode->children[i] && Bounds.intersect( pNode->children[i]->bounds ) )
            collectIntersectedFaces( pNode->children[i], Bounds, aFaces, pVertices, pIndices, nVertexStride );

    } // Next child
}

//-----------------------------------------------------------------------------
//  Name : onCollide() (Protected, Callback)
/// <summary>
/// Called whenever a mesh collision is checked by the physics engine when the 
/// shape potentially collides with another object (an object's AABB overlaps 
/// the mesh's AABB). 
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::onCollide( NewtonUserMeshCollisionCollideDesc * const collideDescData )
{
    // Retrieve the Carbon side shape to which this callback applies.
    cgMeshShape * pThis = (cgMeshShape*)collideDescData->m_userData;

    // Retrieve the mesh referenced by this shape.
    cgMesh * pMesh = pThis->mMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
        return;

    // Get necessary internal mesh data.
    cgVertexFormat * pFormat = pMesh->getVertexFormat();
    cgUInt32   nStride   = (cgUInt32)pFormat->getStride();
    cgUInt32 * pIndices  = pMesh->getSystemIB();
    cgByte   * pVertices = pMesh->getSystemVB() + pFormat->getElementOffset( D3DDECLUSAGE_POSITION );

    // Transform the bounding box into the space of the mesh.
    cgTransform InverseOffset;
    cgBoundingBox Bounds( collideDescData->m_boxP0, collideDescData->m_boxP1 );
    Bounds.transform( pThis->mOffset.inverse( InverseOffset ) );
    Bounds *= pThis->mWorld->fromPhysicsScale();

    // Collect the list of intersected faces.
    //cgUInt32Array aFaces;
    FaceIntersectionSet aFaces;
    pThis->collectIntersectedFaces( pThis->mRootNode, Bounds, aFaces, pVertices, pIndices, nStride );

    // Anything intersected?
    if ( aFaces.empty() )
        return;

    // Size the output arrays appropriately if they do not contain enough elements.
    // First, the vertex array.
    static std::vector<cgVector3> aVertices;
    size_t nVertexCount = aFaces.size() * 3;
    size_t nBufferSize = max( nVertexCount, 1 );
    if ( nBufferSize > aVertices.size() )
        aVertices.resize( nBufferSize );

    // Then the index array. This is never explicitly populated and will
    // always contain a simple incrementing sequence of numbers designed
    // to directly and linearly map to the above vertex array.
    static std::vector<cgInt> aIndices;
    size_t nIndexCount = aFaces.size() * 3;
    nBufferSize = max( nIndexCount, 1 );
    if ( nBufferSize > aIndices.size() )
    {
        size_t nOldSize = aIndices.size();
        aIndices.resize( nBufferSize );
        for ( size_t i = nOldSize; i < nBufferSize; ++i )
            aIndices[i] = i;
    
    } // End if inflate

    // Next the user attributes buffer which will contain the triangle
    // identifiers useful in collision callbacks.
    static std::vector<cgInt> aUserAttributes;
    nBufferSize = max( aFaces.size(), 1 );
    if ( nBufferSize > aUserAttributes.size() )
        aUserAttributes.resize( nBufferSize, 0 );

    // Finally, the face index count buffer. This is never explicitly populated
    // and will always contain a default value of '3' in every element.
    static std::vector<cgInt> aFaceIndexCount;
    nBufferSize = max( aFaces.size(), 1 );
    if ( nBufferSize > aFaceIndexCount.size() )
        aFaceIndexCount.resize( nBufferSize, 3 );

    // Now populate the two buffers we need to generate starting with the vertex data.
    // These need to be transformed by the mesh shape offset and subsequently
    // scaled into meters.
    cgUInt32 i = 0;
    cgFloat fScale = pThis->mWorld->toPhysicsScale();
    for ( FaceIntersectionSet::iterator itFace = aFaces.begin(); itFace != aFaces.end(); ++itFace, ++i )
    //for ( cgUInt32 i = 0; i < aFaces.size(); ++i )
    {
        // Generate final vertices
        //cgUInt32 nFaceIndex = aFaces[i];
        cgUInt32 nFaceIndex = *itFace;
        cgVector3 & v0 = aVertices[i*3];
        cgVector3 & v1 = aVertices[i*3+1];
        cgVector3 & v2 = aVertices[i*3+2];
        v0 = *(cgVector3*)(pVertices + pIndices[nFaceIndex*3] * nStride) * fScale;
        v1 = *(cgVector3*)(pVertices + pIndices[(nFaceIndex*3)+1] * nStride) * fScale;
        v2 = *(cgVector3*)(pVertices + pIndices[(nFaceIndex*3)+2] * nStride) * fScale;
        pThis->mOffset.transformCoord( v0, v0 );
        pThis->mOffset.transformCoord( v1, v1 );
        pThis->mOffset.transformCoord( v2, v2 );

        // Populate user attribute with source face index
        aUserAttributes[i] = nFaceIndex;
    
    } // Next vertex
    
    // Populate the collision description
    collideDescData->m_vertexStrideInBytes  = sizeof(cgVector3);
    collideDescData->m_faceCount            = aFaces.size();
    collideDescData->m_vertex               = (cgFloat*)&aVertices[0];
    collideDescData->m_userAttribute        = &aUserAttributes[0];
    collideDescData->m_faceIndexCount       = &aFaceIndexCount[0];
    collideDescData->m_faceVertexIndex      = &aIndices[0];
}

//-----------------------------------------------------------------------------
//  Name : onRayHit() (Protected, Callback)
/// <summary>
/// Called whenever a raycast is performed on a mesh collision shape. 
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgMeshShape::onRayHit( NewtonUserMeshCollisionRayHitDesc * const lineDescData )
{
    //cgToDoAssert( "Physics", "Unhandled callback detected (onRayHit())." );
    cgToDo( "Physics", "Unhandled callback detected (onRayHit())." );

    // Nothing was hit
    return 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : onDestroy() (Protected, Callback)
/// <summary>
/// Called whenever the physics engine's representation of this mesh is 
/// destroyed.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::onDestroy( void * const userData )
{
    // Nothing in this implementation.
}

//-----------------------------------------------------------------------------
//  Name : onGetCollisionInfo() (Protected, Callback)
/// <summary>
/// Called whenever information is requested about this mesh collision shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgMeshShape::onGetCollisionInfo( void * const userData, NewtonCollisionInfoRecord * const infoRecord )
{
    cgToDoAssert( "Physics", "Unhandled callback detected (onGetCollisionInfo())." );
}

//-----------------------------------------------------------------------------
//  Name : onGetFacesInAABB() (Protected, Callback)
/// <summary>
/// Called whenever a list of faces that fall within an AABB are required.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgMeshShape::onGetFacesInAABB( void * const userData, const cgFloat * const p0, const cgFloat * const p1, const cgFloat ** const vertexArray, cgInt * const vertexCount, cgInt * const vertexStrideInBytes, const cgInt * const indexList, cgInt maxIndexCount, const cgInt * const userDataList )
{
    cgToDoAssert( "Physics", "Unhandled callback detected (onGetFacesInAABB())." );
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
cgInt cgMeshShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgMeshShape * pMeshShape = (cgMeshShape*)pShape;
    nResult = (mMesh == pMeshShape->mMesh) ? 0 : (mMesh < pMeshShape->mMesh) ? -1 : 1;
    if ( nResult != 0 ) return nResult;
    nResult = mOffset.compare( pMeshShape->mOffset, CGE_EPSILON );
    if ( nResult != 0 ) return nResult;

    // Equivalent
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// cgMeshShapeCacheKey Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMeshShapeCacheKey () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshShapeCacheKey::cgMeshShapeCacheKey( const cgMeshHandle & hMesh, const cgTransform & Offset ) : 
    cgPhysicsShapeCacheKey( CG_NULL ), mMesh(hMesh), mOffset(Offset)
{
}

//-----------------------------------------------------------------------------
//  Name : operator< () (Virtual)
/// <summary>
/// Less-than comparison operator.
/// </summary>
//-----------------------------------------------------------------------------
bool cgMeshShapeCacheKey::operator<( const cgPhysicsShapeCacheKey & Key ) const
{
    cgAssert( Key.mShape != CG_NULL );
    
    // Mesh type?
    if ( RTID_MeshShape == Key.mShape->getReferenceType() )
    {
        cgMeshShape * pMeshShape = (cgMeshShape*)Key.mShape;
        cgInt nResult = (mMesh == pMeshShape->mMesh) ? 0 : (mMesh < pMeshShape->mMesh) ? -1 : 1;
        if ( nResult != 0 ) return (nResult < 0) ? true : false;
        nResult = mOffset.compare( pMeshShape->mOffset );
        if ( nResult != 0 ) return (nResult < 0) ? true : false;

        // Exact match (not less than).
        return false;

    } // End if matching type
    else if ( RTID_MeshShape < Key.mShape->getReferenceType() )
        return true;
    else
        return false;
}