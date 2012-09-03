#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Materials/Material.h"
#include "Materials/StandardMaterial.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Materials
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Materials" )
            DECLARE_PACKAGE_CHILD( Material )
            DECLARE_PACKAGE_CHILD( StandardMaterial )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Materials