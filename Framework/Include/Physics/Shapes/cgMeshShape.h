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
// Name : cgMeshShape.h                                                      //
//                                                                           //
// Desc : Class implementing collision *only* properties for an arbitrary    //
//        mesh. This shape cannot be used with a dynamic rigid body, and is  //
//        used primarily for collision detection only.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMESHSHAPE_H_ )
#define _CGE_CGMESHSHAPE_H_

//-----------------------------------------------------------------------------
// cgMeshShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexShape.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgTransform.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct NewtonUserMeshCollisionCollideDesc;
struct NewtonUserMeshCollisionRayHitDesc;
struct NewtonCollisionInfoRecord;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FF70943C-AA5F-4D1B-B634-AFC89EE97916}
const cgUID RTID_MeshShape = {0xFF70943C, 0xAA5F, 0x4D1B, {0xB6, 0x34, 0xAF, 0xC8, 0x9E, 0xE9, 0x79, 0x16}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgMeshShape (Class)
/// <summary>
/// Class implementing collision *only* properties for an arbitrary mesh. This 
/// shape cannot be used with a dynamic rigid body, and is used primarily for 
/// collision detection only.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMeshShape : public cgPhysicsShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMeshShape, cgPhysicsShape, "MeshShape" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgMeshShapeCacheKey;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgMeshShape( cgPhysicsWorld * world, const cgMeshHandle & mesh );
             cgMeshShape( cgPhysicsWorld * world, const cgMeshHandle & mesh, const cgTransform & offset );
    virtual ~cgMeshShape( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgTransform         & getOffsetTransform      ( ) const;
    void                        setOffsetTransform      ( const cgTransform & offset );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_MeshShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Structures & Typedefs
    //-------------------------------------------------------------------------
    struct FaceData
    {
        CGE_VECTOR_DECLARE( FaceData, Array )
        cgBoundingBox   bounds;
    };

    struct BVHLeaf
    {
        cgUInt32Array   faces;
    };
    
    struct BVHNode
    {
        cgBoundingBox   bounds;
        BVHNode       * children[8];
        BVHLeaf       * leaf;

        // Constructor
        BVHNode() : leaf( CG_NULL )
        {
            memset( children, 0, sizeof(children) );
        }
        
        // Destructor
        ~BVHNode()
        {
            // Destroy any chid leaf
            delete leaf;
            leaf = CG_NULL;
            
            // Destroy any child nodes.
            for ( cgInt i = 0; i < 8; ++i )
                delete children[i];
            memset( children, 0, sizeof(children) );
        }
    };
    CGE_UNORDEREDSET_DECLARE( cgUInt32, FaceIntersectionSet );
    CGE_VECTOR_DECLARE( cgInt, AttributeArray )
    CGE_VECTOR_DECLARE( cgInt, IndexArray )
    CGE_VECTOR_DECLARE( cgVector3, VertexArray )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            initMeshHooks           ( );
    void            buildBroadphaseData     ( );
    void            buildBVHTree            ( cgInt depth, BVHNode * node, const cgUInt32Array & faces, const cgBoundingBox & parentBounds, cgByte * vertices, cgUInt32 * indices, cgUInt32 vertexStride );
    void            collectIntersectedFaces ( BVHNode * node, const cgBoundingBox & bounds, FaceIntersectionSet & faces, const cgByte * vertices, const cgUInt32 * indices, cgUInt32 vertexStride );
    bool            rayTest                 ( BVHNode * node, const cgVector3 & from, const cgVector3 & to, const cgByte * vertices, const cgUInt32 * indices, cgUInt32 vertexStride, cgFloat & closestDistance, cgUInt32 & closestFace, cgVector3 & closestNormal );
    bool            collectCollisionData    ( BVHNode * node, const cgBoundingBox & bounds, const cgByte * vertices, const cgUInt32 * indices, cgUInt32 vertexStride, cgUInt32 * faceMap, VertexArray & verticesOut, AttributeArray & attributesOut, cgUInt32 & vertexCountOut, cgUInt32 & faceCountOut );

    //-------------------------------------------------------------------------
    // Protected Static Methods
    //-------------------------------------------------------------------------
    static void     onCollide               ( NewtonUserMeshCollisionCollideDesc * const collideDescData );
    static cgFloat  onRayHit                ( NewtonUserMeshCollisionRayHitDesc * const lineDescData );
    static void     onDestroy               ( void * const userData );
    static void     onGetCollisionInfo      ( void * const userData, NewtonCollisionInfoRecord * const infoRecord );
    static cgInt    onGetFacesInAABB        ( void * const userData, const cgFloat * const p0, const cgFloat * const p1, const cgFloat ** const vertexArray, cgInt * const vertexCount, cgInt * const vertexStrideInBytes, const cgInt * const indexList, cgInt maxIndexCount, const cgInt * const userDataList );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgMeshHandle    mMesh;          // Mesh used as the source for collision detection and response
    cgTransform     mOffset;        // Offset transform that should be applied to this mesh.
    cgBoundingBox   mMeshBounds;    // Local space bounding box of the mesh itself.
    BVHNode       * mRootNode;      // Root node of the broadphase BVH (oct) tree.
    FaceData::Array mFaces;         // Additional data for each face in the mesh.

};

//-----------------------------------------------------------------------------
//  Name : cgMeshShapeCacheKey (Class)
/// <summary>
/// Implements comparison operator for physics shape cache.
/// </summary>
//-----------------------------------------------------------------------------
class cgMeshShapeCacheKey : public cgPhysicsShapeCacheKey
{
public:
    cgMeshShapeCacheKey( const cgMeshHandle & mesh, const cgTransform & offset );

    // Comparison operator
    virtual bool operator < ( const cgPhysicsShapeCacheKey & v ) const;

    // Cache values
    cgMeshHandle    mMesh;
    cgTransform     mOffset;
};

#endif // !_CGE_CGMESHSHAPE_H_