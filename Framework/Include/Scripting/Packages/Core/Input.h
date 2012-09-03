#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Input/Types.h"
#include "Input/InputDriver.h"
#include "Input/InputListener.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Input
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Input" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( InputDriver )
            DECLARE_PACKAGE_CHILD( InputListener )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Input