#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <States/cgAppStateManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace States {

// Package declaration
namespace AppStateManager
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States.AppStateManager" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AppStateManager", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAppStateManager>( engine );

            // Register the object methods.
            // ToDo:
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::States::AppStateManager