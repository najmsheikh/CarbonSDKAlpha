#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace RenderControl
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.RenderControl" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerInterface("IScriptedRenderControl") );
        }

        // Member bindings
        void bind( cgScriptEngine * pEngine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the /required/ interface methods
            // ToDo: Are any actually required?
            /*BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptRenderControl", "bool onScenePreLoad()" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptRenderControl", "bool onSceneLoaded()" ) );
            BINDSUCCESS( engine->RegisterInterfaceMethod( "IScriptRenderControl", "void onSceneRender( float )" ) );*/
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::RenderControl