#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Navigation/NavigationWaypoint.h"
#include "Navigation/NavigationPatrolPoint.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Navigation
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Navigation" )
            DECLARE_PACKAGE_CHILD( NavigationWaypoint )
            DECLARE_PACKAGE_CHILD( NavigationPatrolPoint )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Navigation