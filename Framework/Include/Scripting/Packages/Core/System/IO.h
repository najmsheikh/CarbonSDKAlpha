#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "IO/Types.h"
#include "IO/InputStream.h"
#include "IO/Logging.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace IO
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.IO" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( InputStream )
            DECLARE_PACKAGE_CHILD( Logging )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::IO