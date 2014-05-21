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
// Name : cgDisplacementShape.h                                              //
//                                                                           //
// Desc : Class implementing collision *only* properties for an arbitrary    //
//        displacement / height map (such as that used by a terrain). This   //
//        shape cannot be used with a dynamic rigid body, and is used        //
//        primarily for collision detection only.                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDISPLACEMENTSHAPE_H_ )
#define _CGE_CGDISPLACEMENTSHAPE_H_

//-----------------------------------------------------------------------------
// cgDisplacementShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsShape.h>
#include <Scripting/cgScriptInterop.h>
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
// {46A24B47-9907-43F3-B1DF-7E73A542C91E}
const cgUID RTID_DisplacementShape = { 0x46a24b47, 0x9907, 0x43f3, { 0xb1, 0xdf, 0x7e, 0x73, 0xa5, 0x42, 0xc9, 0x1e } };;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDisplacementShape (Class)
/// <summary>
/// Class implementing collision *only* properties for an arbitrary 
/// displacement / height map (such as that used by a terrain). This shape 
/// cannot be used with a dynamic rigid body, and is used primarily for 
/// collision detection only.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDisplacementShape : public cgPhysicsShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDisplacementShape, cgPhysicsShape, "DisplacementShape" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgDisplacementShapeCacheKey;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDisplacementShape( cgPhysicsWorld * world, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize );
             cgDisplacementShape( cgPhysicsWorld * world, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize, const cgTransform & offset );
    virtual ~cgDisplacementShape( );

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
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_DisplacementShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            computeBounds           ( );
    void            initMeshHooks           ( );

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
    const cgInt16     * mDisplacement;  // Reference to the displacement / height map to be used. This is not a copy. Lifetime of the heightmap must be guaranteed.
    cgSize              mMapPitch;      // Pitch of the displacement map data.
    cgRect              mSection;       // The region of the displacement to use.
    cgVector3           mCellSize;      // The local space size of each individual cell in the height field.
    cgTransform         mOffset;        // Offset transform that should be applied to this shape.
    cgBoundingBox       mBounds;        // Local space bounding box of the entire collision shape.
};

//-----------------------------------------------------------------------------
//  Name : cgDisplacementShapeCacheKey (Class)
/// <summary>
/// Implements comparison operator for physics shape cache.
/// </summary>
//-----------------------------------------------------------------------------
class cgDisplacementShapeCacheKey : public cgPhysicsShapeCacheKey
{
public:
    cgDisplacementShapeCacheKey( const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & section, const cgVector3 & cellSize, const cgTransform & offset );

    // Comparison operator
    virtual bool operator < ( const cgPhysicsShapeCacheKey & v ) const;

    // Cache values
    const cgInt16 * mDisplacement;
    cgSize          mMapPitch;
    cgRect          mSection;
    cgVector3       mCellSize;
    cgTransform     mOffset;
};

#endif // !_CGE_CGDISPLACEMENTSHAPE_H_