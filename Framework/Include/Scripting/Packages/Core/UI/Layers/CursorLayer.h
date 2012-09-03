#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUILayers.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Layers {

// Package declaration
namespace CursorLayer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Layers.CursorLayer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "CursorLayer", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgUILayer (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUICursorLayer>( engine );

            // Register base / system behaviors.
            Core::UI::Layers::UILayer::registerLayerMethods<cgUILayer>( engine, "CursorLayer" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "CursorLayer", "bool initialize( )", asMETHODPR(cgUICursorLayer, initialize, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CursorLayer", "void selectCursor( const String&in )", asMETHODPR(cgUICursorLayer, selectCursor, ( const cgString& ), void), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Layers::CursorLayer
