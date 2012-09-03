#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStateBlocks.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace StateBlocks {

// Package declaration
namespace BlendState
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.StateBlocks.BlendState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "BlendState", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "BlendStateHandle", sizeof(cgBlendStateHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgBlendState>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgBlendState>( engine, "BlendState" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "BlendState", "const BlendStateDesc & getValues( ) const", asMETHODPR(cgBlendState, getValues, () const, const cgBlendStateDesc&), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgBlendStateHandle>( engine, "BlendStateHandle", "BlendState" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::StateBlocks::BlendState