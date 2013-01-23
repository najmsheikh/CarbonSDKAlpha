#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Physics/Types.h"
#include "Physics/Joints.h"
#include "Physics/Bodies.h"
#include "Physics/Controllers.h"
#include "Physics/PhysicsEntity.h"
#include "Physics/PhysicsWorld.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Physics
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( Joints )
            DECLARE_PACKAGE_CHILD( Bodies )
            DECLARE_PACKAGE_CHILD( Controllers )
            DECLARE_PACKAGE_CHILD( PhysicsEntity )
            DECLARE_PACKAGE_CHILD( PhysicsWorld )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Physics