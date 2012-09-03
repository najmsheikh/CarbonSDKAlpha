#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Input/cgInputDriver.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Input {

// Package declaration
namespace InputListener
{
    //-------------------------------------------------------------------------
    // Name : registerListenerMethods ()
    // Desc : Register the base cgInputListener class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerListenerMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class methods.
        Core::System::Events::EventListener::registerListenerMethods<type>( engine, typeName );

        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onMouseMove( const Point &in, const PointF &in )", asMETHODPR(type,onMouseMove,( const cgPoint&, const cgPointF& ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onMouseButtonDown( int, const Point &in )", asMETHODPR(type,onMouseButtonDown,( cgInt32, const cgPoint&),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onMouseButtonUp( int, const Point &in )", asMETHODPR(type,onMouseButtonUp,( cgInt32, const cgPoint&),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onMouseWheelScroll( int, const Point &in )", asMETHODPR(type,onMouseWheelScroll,( cgInt32, const cgPoint&),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onKeyDown( int, uint )", asMETHODPR(type,onKeyDown,( cgInt32, cgUInt32),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onKeyUp( int, uint )", asMETHODPR(type,onKeyUp,( cgInt32, cgUInt32),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void onKeyPressed( int, uint )", asMETHODPR(type,onKeyPressed,( cgInt32, cgUInt32),void), asCALL_THISCALL) );

    } // End Method RegisterObjectBehaviorMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Input.InputListener" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "InputListener", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgInputListener>( engine );

            // Register the object methods for the base class (template method in header)
            registerListenerMethods<cgInputListener>( engine, "InputListener" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Input::InputListener