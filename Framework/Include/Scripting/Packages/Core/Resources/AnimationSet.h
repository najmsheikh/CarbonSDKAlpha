#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgAnimationSet.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace AnimationSet
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.AnimationSet" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AnimationSet", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "AnimationSetHandle", sizeof(cgAnimationSetHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAnimationSet>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgAnimationSet>( engine, "AnimationSet" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgAnimationSetHandle>( engine, "AnimationSetHandle", "AnimationSet" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::AnimationSet