#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Lights/Light.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Lights
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights" )
            DECLARE_PACKAGE_CHILD( Light )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Lights