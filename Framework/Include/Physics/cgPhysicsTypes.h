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
// Name : cgPhysicsTypes.h                                                   //
//                                                                           //
// Desc : Common system file that defines various physics types and common   //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSTYPES_H_ )
#define _CGE_CGPHYSICSTYPES_H_

//-----------------------------------------------------------------------------
// cgPhysicsTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgTransform.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsBody;

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgPhysicsModel
{
    enum Base
    {
        None = 0,
        CollisionOnly,
        RigidStatic,
        RigidDynamic,
        Kinematic
    };

}; // End Namespace : cgPhysicsModel

namespace cgDefaultPhysicsShape
{
    enum Base
    {
        Auto = 0,
        Box,
        Sphere,
        Cylinder,
        Cone,
        Capsule,
        ConvexHull
    };

}; // End Namespace : cgDefaultPhysicsShape

namespace cgSimulationQuality
{
    enum Base
    {
        Default = 0,
        CCD = 1
    };

}; // End Namespace : cgSimulationQuality

namespace cgDefaultPhysicsMaterialGroup
{
    enum Base
    {
        Standard = 0,
        Character,
        PlayerCharacter,
        Ragdoll,
        CastOnly,
        Count
    };

}; // End Namespace : cgDefaultPhysicsMaterialGroup

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct CGE_API cgRigidBodyCreateParams
{
    cgPhysicsModel::Base        model;              // Type of motion (if any) to simulate.
    cgTransform                 initialTransform;   // The starting transformation of this body.
    cgSimulationQuality::Base   quality;            // The quality of the simulation (describes its relative expense).
    cgFloat                     mass;               // Initial mass of the rigid body.
    cgVector3                   centerOfMass;       // Offset to the body's center of mass relative to its position and orientation. 
    cgVector3                   shapeOffset;        // Amount to offset the body's shape relative to the position.

    // Provide defaults
    cgRigidBodyCreateParams() :
        model( cgPhysicsModel::RigidDynamic ), quality( cgSimulationQuality::Default ),
        mass( 1.0f ), centerOfMass(0,0,0), shapeOffset(0,0,0) {}

}; // End Struct : cgRigidBodyCreateParams

struct CGE_API cgRayCastContact
{
    CGE_ARRAY_DECLARE( cgRayCastContact, Array );

    cgPhysicsBody * body;               // The intersected physics body.
    cgFloat         intersectParam;     // Intersection 't' value along the specified path.
    cgVector3       contactNormal;      // Surface normal at the point of contact.
    cgInt32         collisionId;        // Custom identifier associated returned by the shape in contact.

    // Provide defaults
    cgRayCastContact() :
        body(CG_NULL), intersectParam(FLT_MAX), contactNormal(0,0,0), collisionId(-1) {}

}; // End Struct : cgRayCastContact

struct CGE_API cgCollisionContact
{
    CGE_ARRAY_DECLARE( cgCollisionContact, Array );

    cgVector3   point;
    cgVector3   normal;
    cgFloat     speed;

}; // End Struct : cgCollisionContact

struct CGE_API cgBodyCollision
{
    cgPhysicsBody             * thisBody;
    cgPhysicsBody             * otherBody;
    cgCollisionContact::Array   contacts;

    // Provide defaults
    cgBodyCollision() :
        thisBody(CG_NULL), otherBody(CG_NULL) {}

}; // End Struct : cgBodyCollision

#endif // !_CGE_CGPHYSICSTYPES_H_