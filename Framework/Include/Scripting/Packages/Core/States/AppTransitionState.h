#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <States/cgAppStateManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace States {

// Package declaration
namespace AppTransitionState
{
    //-------------------------------------------------------------------------
    // Name : registerTransitionStateMethods ()
    // Desc : Register the base cgAppTransitionState class methods. Can be
    //        called by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerTransitionStateMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::States::AppState::registerStateMethods<type>( engine, typeName );
        
        // Register the object methods
        //BINDSUCCESS( engine->registerObjectMethod( typeName, "void clearForces( )", asMETHODPR(type, clearForces, (), void ), asCALL_THISCALL) );
        // ToDo
    
    } // End Method registerTransitionStateMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States.AppTransitionState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AppTransitionState", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAppTransitionState (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAppTransitionState>( engine );

            // Register the object methods
            registerTransitionStateMethods<cgAppTransitionState>( engine, "AppTransitionState" );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::States::AppTransitionState