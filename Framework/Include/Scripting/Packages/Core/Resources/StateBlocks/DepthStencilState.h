#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStateBlocks.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace StateBlocks {

// Package declaration
namespace DepthStencilState
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.StateBlocks.DepthStencilState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DepthStencilState", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "DepthStencilStateHandle", sizeof(cgDepthStencilStateHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgDepthStencilState>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgDepthStencilState>( engine, "DepthStencilState" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "DepthStencilState", "const DepthStencilStateDesc & getValues( ) const", asMETHODPR(cgDepthStencilState, getValues, () const, const cgDepthStencilStateDesc&), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgDepthStencilStateHandle>( engine, "DepthStencilStateHandle", "DepthStencilState" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::StateBlocks::DepthStencilState