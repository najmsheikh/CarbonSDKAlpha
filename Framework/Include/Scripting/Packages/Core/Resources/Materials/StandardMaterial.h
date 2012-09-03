#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStandardMaterial.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace Materials {

// Package declaration
namespace StandardMaterial
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Materials.StandardMaterial" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "StandardMaterial", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgStandardMaterial>( engine );

            // Register the object methods
            Core::Resources::Materials::Material::registerMaterialMethods<cgStandardMaterial>( engine, "StandardMaterial" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::Materials::StandardMaterial