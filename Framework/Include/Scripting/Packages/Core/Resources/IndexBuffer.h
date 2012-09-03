#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgIndexBuffer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace IndexBuffer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.IndexBuffer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "IndexBuffer", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "IndexBufferHandle", sizeof(cgIndexBufferHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgIndexBuffer>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgIndexBuffer>( engine, "IndexBuffer" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgIndexBufferHandle>( engine, "IndexBufferHandle", "IndexBuffer" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::IndexBuffer