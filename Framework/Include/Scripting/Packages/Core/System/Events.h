#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Events/EventDispatcher.h"
#include "Events/EventListener.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Events
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Events" )
            DECLARE_PACKAGE_CHILD( EventDispatcher )
            DECLARE_PACKAGE_CHILD( EventListener )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Events