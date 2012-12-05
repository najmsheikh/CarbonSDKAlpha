#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "States/AppStateManager.h"
#include "States/AppState.h"
#include "States/AppTransitionState.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace States
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States" )
            DECLARE_PACKAGE_CHILD( AppStateManager )
            DECLARE_PACKAGE_CHILD( AppState )
            DECLARE_PACKAGE_CHILD( AppTransitionState )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::States