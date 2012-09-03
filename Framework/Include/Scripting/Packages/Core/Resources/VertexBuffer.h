#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgVertexBuffer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace VertexBuffer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.VertexBuffer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "VertexBuffer", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "VertexBufferHandle", sizeof(cgVertexBufferHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgVertexBuffer>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgVertexBuffer>( engine, "VertexBuffer" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgVertexBufferHandle>( engine, "VertexBufferHandle", "VertexBuffer" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::VertexBuffer