#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgMesh.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Mesh
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Mesh" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Mesh", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "MeshHandle", sizeof(cgMeshHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgMesh>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceComponentMethods<cgMesh>( engine, "Mesh" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgMeshHandle>( engine, "MeshHandle", "Mesh" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Mesh