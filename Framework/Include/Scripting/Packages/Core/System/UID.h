#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace UID
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.UID" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "UID", sizeof(cgUID), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "UID", "uint data1", offsetof(cgUID,data1) ) );
            BINDSUCCESS( engine->registerObjectProperty( "UID", "uint16 data2", offsetof(cgUID,data2) ) );
            BINDSUCCESS( engine->registerObjectProperty( "UID", "uint16 data3", offsetof(cgUID,data3) ) );
            BINDSUCCESS( engine->registerObjectProperty( "UID", "uint64 data4", offsetof(cgUID,data4) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::UID