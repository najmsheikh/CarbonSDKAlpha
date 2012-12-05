#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Elements/SceneElement.h"
#include "Elements/SkyElement.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Elements
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Elements" )
            DECLARE_PACKAGE_CHILD( SceneElement )
            DECLARE_PACKAGE_CHILD( SkyElement )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Elements