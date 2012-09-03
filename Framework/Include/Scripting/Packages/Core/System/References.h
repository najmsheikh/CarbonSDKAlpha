#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "References/Messaging.h"
#include "References/Reference.h"
#include "References/Management.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace References
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.References" )
            DECLARE_PACKAGE_CHILD( Messaging )
            DECLARE_PACKAGE_CHILD( Reference )
            DECLARE_PACKAGE_CHILD( Management )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::References