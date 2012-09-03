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
// Name : cgCollision.h                                                      //
//                                                                           //
// Desc: Our collision detection / scene library. Provides us with collision //
//       detection and response routines for use throughout our application. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCOLLISION_H_ )
#define _CGE_CGCOLLISION_H_

//-----------------------------------------------------------------------------
// cgCollision Header Includes
//-----------------------------------------------------------------------------
#include <Math/cgMathTypes.h>
#include <Math/cgBoundingBox.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVertexFormat;
class cgSpatialTree;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCollision (Class)
/// <summary>
/// The purpose of this class is to provide all support necessary for
/// geometric intersection testing, and their utility functions.
/// Several static functions are defined for public use for various
/// operations such as ray plane testing etc.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCollision : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgCollision, "Collision" )

public:
    //-------------------------------------------------------------------------
    // Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    struct CollIntersect
    {
        cgVector3       newCenter;      // The new sphere/ellipsoid centre point
        cgVector3       intersectPoint; // The point on the exterior of the sphere/ellipsoid where contact occurred
        cgVector3       intersectNormal;// The intersection normal (sliding plane)
        cgFloat         interval;       // The time of intersection (Centre + Velocity * interval)
        cgUInt32        triangleIndex;  // The index of the triangle we are intersecting
        cgObjectNode  * object;         // Pointer to the object we hit (if any).
    };

    // Used to inform caller how to process collision geometry.
    enum QueryResult
    {
        Query_None,     // No collision geometry available
        Query_All,      // Process all geometry
        Query_Selective // Process only selected faces.
    };

    struct QueryData
    {
        cgVertexFormat* vertexFormat;   // Format of the vertex data supplied.
        void          * vertices;       // Mesh vertex data.
        cgUInt32      * indices;        // Mesh index data (triCount * 3).
        cgVector3     * normals;        // Triangle normal buffer (triCount).
        cgUInt32        triCount;       // Number of triangles described by the above index buffer.
        cgUInt32Array   testFaces;      // Describes a list of individual faces to test where necessary.

        // Constructor
        QueryData() : vertexFormat( CG_NULL ), vertices( CG_NULL ), indices( CG_NULL ), normals( CG_NULL ), triCount( 0 ) {}
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCollision();
    virtual ~cgCollision();

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static bool                 pointInTriangle             ( const cgVector3& point, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, float tolerance = 0.0f );
    static bool                 pointInTriangle             ( const cgVector3& point, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& normal, float tolerance = 0.0f );
    static bool                 pointInAABB                 ( const cgVector3& point, const cgBoundingBox & AABB, bool ignoreX = false, bool ignoreY = false, bool ignoreZ = false );
    
    static bool                 rayIntersectPlane           ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& planeNormal, const cgVector3& planePoint, cgFloat& t, bool biDirectional = false, bool restrictRange = true );
    static bool                 rayIntersectPlane           ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& planeNormal, cgFloat planeDistance, cgFloat& t, bool biDirectional = false, bool restrictRange = true );
    static bool                 rayIntersectPlane           ( const cgVector3& origin, const cgVector3& velocity, const cgPlane& plane, cgFloat& t, bool biDirectional = false, bool restrictRange = true );
    static bool                 rayIntersectTriangle        ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, cgFloat& t, cgFloat tolerance = 0, bool biDirectional = false, bool restrictRange = true );
    static bool                 rayIntersectTriangle        ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& normal, cgFloat& t, cgFloat tolerance = 0, bool biDirectional = false, bool restrictRange = true );
    static bool                 rayIntersectAABB            ( const cgVector3& origin, const cgVector3& velocity, const cgBoundingBox & AABB, cgFloat& t, bool ignoreX = false, bool ignoreY = false, bool ignoreZ = false, bool restrictRange = true );
    static bool                 rayIntersectSphere          ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& center, cgFloat radius, cgFloat& t );

    static bool                 sphereIntersectPlane        ( const cgVector3& center, cgFloat radius, const cgVector3& velocity, const cgVector3& planeNormal, const cgVector3& planePoint, cgFloat& tMax );
    static bool                 sphereIntersectLineSegment  ( const cgVector3& center, cgFloat radius, const cgVector3& velocity, const cgVector3& v1, const cgVector3& v2, cgFloat& tMax, cgVector3& collisionNormal );
    static bool                 sphereIntersectPoint        ( const cgVector3& center, cgFloat radius, const cgVector3& velocity, const cgVector3& point, cgFloat& tMax, cgVector3& collisionNormal );
    static bool                 sphereIntersectTriangle     ( const cgVector3& center, cgFloat radius, const cgVector3& velocity, const cgVector3& v1, const cgVector3& v2, const cgVector3& v3, const cgVector3& normal, cgFloat& tMax, cgVector3& collisionNormal );

    static bool                 AABBIntersectAABB           ( const cgBoundingBox & AABB1, const cgBoundingBox & AABB2, bool ignoreX = false, bool ignoreY = false, bool ignoreZ = false );
    static bool                 AABBIntersectAABB           ( bool & contained, const cgBoundingBox & AABB1, const cgBoundingBox & AABB2, bool ignoreX = false, bool ignoreY = false, bool ignoreZ = false );
    
    static cgPlaneQuery::Class  pointClassifyPlane          ( const cgVector3& point, const cgVector3& planeNormal, cgFloat planeDistance );
    static cgPlaneQuery::Class  rayClassifyPlane            ( const cgVector3& origin, const cgVector3& velocity, const cgVector3& planeNormal, cgFloat planeDistance );
    static cgPlaneQuery::Class  polyClassifyPlane           ( void * vertices, cgUInt32 vertexCount, cgUInt32 stride, const cgVector3& planeNormal, cgFloat planeDistance );
    static cgPlaneQuery::Class  AABBClassifyPlane           ( const cgBoundingBox & AABB, const cgVector3& planeNormal, cgFloat planeDistance );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                setQueryTree                ( cgSpatialTree * tree );
    void                setMaxIntersections         ( cgUInt16 maxIntersections );
    void                setMaxIterations            ( cgUInt16 maxIterations );
    bool                addObject                   ( cgObjectNode * object );
    bool                removeObject                ( cgObjectNode * object );
    bool                ellipsoidIntersectScene     ( cgObjectNode * sourceObject, const cgVector3& center, const cgVector3& radius, const cgVector3& velocity, CollIntersect intersections[], cgUInt32 & intersectionCount, bool inputEllipsoidSpace = false, bool returnEllipsoidSpace = false );
    bool                simulateEllipsoidSlide      ( cgObjectNode * sourceObject, const cgVector3& center, const cgVector3& radius, const cgVector3& velocity, cgVector3& newCenter, cgVector3& newIntegrationVelocity, cgBoundingBox & collisionExtents );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                     ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static bool                 solveCollision      ( cgFloat a, cgFloat b, cgFloat c, cgFloat& t );
    inline static cgVector3     vectorScale         ( const cgVector3& v1, const cgVector3& v2 ) { return cgVector3( v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ); }

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void                calculateEllipsoidBounds    ( const cgVector3& center, const cgVector3& radius, const cgVector3& velocity );
    cgUInt32            ellipsoidIntersectBuffers   ( QueryData & data, QueryResult processing, const cgVector3& eCenter, const cgVector3& radius, const cgVector3& inverseRadius, const cgVector3& eVelocity, cgFloat& eInterval, CollIntersect intersections[], cgUInt32 & intersectionCount, const cgMatrix * transform = CG_NULL );

    //-------------------------------------------------------------------------
    // Private Member Variables
    //-------------------------------------------------------------------------
    cgSpatialTree     * mQueryTree;             // Tree used to query for nearby objects (optional).
    cgObjectNodeList    mObjects;               // Objects against which we will test for collision.
    cgUInt16            mMaxIntersections;      // The total number of intersections which we should record
    cgUInt16            mMaxIterations;         // The maximum number of collision test iterations we should try before failing
    CollIntersect     * mIntersections;         // Internal buffer for storing intersection information.
    cgBoundingBox       mEllipsoidBounds;       // World space AABB for the ellipsoid being testing.

};

#endif // !_CGE_CGCOLLISION_H_