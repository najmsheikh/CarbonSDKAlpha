#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Visibility/VisibilitySet.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Visibility
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Visibility" )
            DECLARE_PACKAGE_CHILD( VisibilitySet )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Visibility