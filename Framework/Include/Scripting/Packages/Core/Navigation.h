#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Navigation/Types.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Navigation
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Navigation" )
            DECLARE_PACKAGE_CHILD( Types )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Navigation