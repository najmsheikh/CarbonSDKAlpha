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
// cgPhysicsTyoes Header Includes
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

#endif // !_CGE_CGPHYSICSTYPES_H_