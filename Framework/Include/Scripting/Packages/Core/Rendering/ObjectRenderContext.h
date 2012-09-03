#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgObjectRenderContext.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace ObjectRenderContext
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.ObjectRenderContext" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ObjectRenderContext", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgObjectRenderContext (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgObjectRenderContext>( engine );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderContext", "bool step( )", asMETHODPR(cgObjectRenderContext, step, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderContext", "void render( )", asMETHODPR(cgObjectRenderContext, render, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderContext", "void light( )", asMETHODPR(cgObjectRenderContext, light, ( ), void), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::ObjectRenderContext