#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Animation/AnimationTarget.h"
#include "Animation/AnimationController.h"
#include "Animation/Types.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Animation
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Animation" )
            DECLARE_PACKAGE_CHILD( AnimationTarget )
            DECLARE_PACKAGE_CHILD( AnimationController )
            DECLARE_PACKAGE_CHILD( Types )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Animation