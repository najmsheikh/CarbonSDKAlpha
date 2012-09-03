#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUILayers.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Layers {

// Package declaration
namespace ControlLayer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Layers.ControlLayer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ControlLayer", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgUILayer (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUIControlLayer>( engine );

            // Register base / system behaviors.
            Core::UI::Layers::UILayer::registerLayerMethods<cgUILayer>( engine, "ControlLayer" );

            // Register object methods
            // ToDo: bool RegisterSkinElement ( const cgUISkinElement & Element, const cgString & strElementName );
            BINDSUCCESS( engine->registerObjectMethod( "ControlLayer", "void attachControl( UIControl@+ )", asMETHODPR(cgUIControlLayer, attachControl, ( cgUIControl* ), void), asCALL_THISCALL) );
            // ToDo: const cgUISkinElement     * GetSkinElement      ( const cgString & strName );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Layers::ControlLayer
