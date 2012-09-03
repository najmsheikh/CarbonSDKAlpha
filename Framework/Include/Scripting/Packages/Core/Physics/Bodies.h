#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Bodies/PhysicsBody.h"
#include "Bodies/RigidBody.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace Bodies
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Bodies" )
            DECLARE_PACKAGE_CHILD( PhysicsBody )
            DECLARE_PACKAGE_CHILD( RigidBody )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::Bodies
