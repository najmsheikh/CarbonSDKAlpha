#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgHardwareShaders.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace VertexShader
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.VertexShader" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "VertexShader", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "VertexShaderHandle", sizeof(cgVertexShaderHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgVertexShader>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgVertexShader>( engine, "VertexShader" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgVertexShaderHandle>( engine, "VertexShaderHandle", "VertexShader" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::VertexShader