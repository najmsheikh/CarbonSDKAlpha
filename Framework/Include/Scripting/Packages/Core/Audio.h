#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Audio/AudioDriver.h"
#include "Audio/Types.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Audio
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Audio" )
            DECLARE_PACKAGE_CHILD( AudioDriver )
            DECLARE_PACKAGE_CHILD( Types )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Audio