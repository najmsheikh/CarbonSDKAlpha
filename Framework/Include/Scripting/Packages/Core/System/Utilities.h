#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Utilities/Profiler.h"
#include "Utilities/Timer.h"
#include "Utilities/FilterExpression.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Utilities
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Utilities" )
            DECLARE_PACKAGE_CHILD( Timer )
            DECLARE_PACKAGE_CHILD( Profiler )
            DECLARE_PACKAGE_CHILD( FilterExpression )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Utilities