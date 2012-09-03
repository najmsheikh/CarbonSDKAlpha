#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgScript.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Script
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Script" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Script", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "ScriptHandle", sizeof(cgScriptHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgScript>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgScript>( engine, "Script" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgScriptHandle>( engine, "ScriptHandle", "Script" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Script