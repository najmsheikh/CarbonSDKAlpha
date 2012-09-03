#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace Events {

// Package declaration
namespace EventDispatcher
{
    //-------------------------------------------------------------------------
    // Name : registerDispatcherMethods ()
    // Desc : Register the base cgEventDispatcher class methods. Can be called 
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerDispatcherMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool registerEventListner( EventListener@+ )", asMETHODPR(type, registerEventListener, ( cgEventListener* ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void unregisterEventListner( EventListener@+ )", asMETHODPR(type, unregisterEventListener, ( cgEventListener* ), void), asCALL_THISCALL) );
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Events.EventDispatcher" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "EventDispatcher", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgEventDispatcher>( engine );

            // Register the object methods for the base class (template method)
            registerDispatcherMethods<cgEventDispatcher>( engine, "EventDispatcher" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Events::EventDispatcher