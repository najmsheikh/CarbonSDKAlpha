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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSTYPES_H_ )
#define _CGE_CGPHYSICSTYPES_H_

//-----------------------------------------------------------------------------
// cgPhysicsTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

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

namespace cgSimulationQuality
{
    enum Base
    {
        Default = 0,
        CCD = 1
    };

}; // End Namespace : cgSimulationQuality

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgRigidBodyCreateParams
{
    cgPhysicsModel::Base        model;              // Type of motion (if any) to simulate.
    cgTransform                 initialTransform;   // The starting transformation of this body.
    cgSimulationQuality::Base   quality;            // The quality of the simulation (describes its relative expense).
    cgFloat                     mass;               // Initial mass of the rigid body.
    cgVector3                   centerOfMass;       // Offset to the body's center of mass relative to its position and orientation. 
    cgVector3                   shapeOffset;        // Amount to offset the body's shape relative to the position.

    // Provide defaults
    cgRigidBodyCreateParams()
    {
        model           = cgPhysicsModel::RigidDynamic;
        quality         = cgSimulationQuality::Default;
        mass            = 1.0f;
        centerOfMass    = cgVector3(0,0,0);
        shapeOffset     = cgVector3(0,0,0);
    }

}; // End cgRigidBodyCreateParams

#endif // !_CGE_CGPHYSICSTYPES_H_