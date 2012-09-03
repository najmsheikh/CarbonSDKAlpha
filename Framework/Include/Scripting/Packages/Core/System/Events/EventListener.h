#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace Events {

// Package declaration
namespace EventListener
{
    //-------------------------------------------------------------------------
    // Name : registerListenerMethods ()
    // Desc : Register the base cgEventListener class methods. Can be called 
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerListenerMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register the object methods
        // Nothing at the current time.
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Events.EventListener" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "EventListener", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgEventListener>( engine );

            // Register the object methods for the base class (template method)
            registerListenerMethods<cgEventListener>( engine, "EventListener" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Events::EventListener