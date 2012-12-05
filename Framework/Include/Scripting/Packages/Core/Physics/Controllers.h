#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Controllers/PhysicsController.h"
#include "Controllers/CharacterController.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace Controllers
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Controllers" )
            DECLARE_PACKAGE_CHILD( PhysicsController )
            DECLARE_PACKAGE_CHILD( CharacterController )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::Controllers
