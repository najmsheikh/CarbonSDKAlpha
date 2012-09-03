#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStateBlocks.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace StateBlocks {

// Package declaration
namespace SamplerState
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.StateBlocks.SamplerState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SamplerState", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "SamplerStateHandle", sizeof(cgSamplerStateHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSamplerState>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgSamplerState>( engine, "SamplerState" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "SamplerState", "const SamplerStateDesc & getValues( ) const", asMETHODPR(cgSamplerState, getValues, () const, const cgSamplerStateDesc&), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgSamplerStateHandle>( engine, "SamplerStateHandle", "SamplerState" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::StateBlocks::SamplerState