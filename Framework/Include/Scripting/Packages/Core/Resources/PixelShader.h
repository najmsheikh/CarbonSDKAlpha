#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgHardwareShaders.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace PixelShader
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.PixelShader" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PixelShader", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "PixelShaderHandle", sizeof(cgPixelShaderHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPixelShader>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgPixelShader>( engine, "PixelShader" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgPixelShaderHandle>( engine, "PixelShaderHandle", "PixelShader" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::PixelShader