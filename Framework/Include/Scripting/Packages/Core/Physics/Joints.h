#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Joints/PhysicsJoint.h"
#include "Joints/KinematicControllerJoint.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace Joints
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Joints" )
            DECLARE_PACKAGE_CHILD( PhysicsJoint )
            DECLARE_PACKAGE_CHILD( KinematicControllerJoint )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::Joints
