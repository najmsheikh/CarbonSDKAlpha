#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStateBlocks.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace StateBlocks {

// Package declaration
namespace RasterizerState
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.StateBlocks.RasterizerState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RasterizerState", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "RasterizerStateHandle", sizeof(cgRasterizerStateHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgRasterizerState>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgRasterizerState>( engine, "RasterizerState" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "RasterizerState", "const RasterizerStateDesc & getValues( ) const", asMETHODPR(cgRasterizerState, getValues, () const, const cgRasterizerStateDesc&), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgRasterizerStateHandle>( engine, "RasterizerStateHandle", "RasterizerState" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::StateBlocks::RasterizerState