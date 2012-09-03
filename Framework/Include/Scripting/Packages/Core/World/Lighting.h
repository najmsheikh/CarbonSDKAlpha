#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Lighting/LightingManager.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Lighting
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Lighting" )
            DECLARE_PACKAGE_CHILD( LightingManager )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Lighting