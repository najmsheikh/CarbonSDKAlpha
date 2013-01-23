#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <States/cgAppStateManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace States {

// Package declaration
namespace AppStateManager
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States.AppStateManager" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AppStateManager", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAppStateManager>( engine );

            // Register the object methods.
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "bool registerState( AppState@ )", asMETHODPR(cgAppStateManager, registerState, ( cgAppState* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "AppState@+ getActiveState( )", asMETHODPR(cgAppStateManager, getActiveState, ( ), cgAppState* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "bool setActiveState( const String &in )", asMETHODPR(cgAppStateManager, setActiveState, ( const cgString& ), bool ), asCALL_THISCALL) );
            // ToDo: bool            getStateDesc        ( const cgString & stateId, StateDesc * descriptionOut );
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "void update( )", asMETHODPR(cgAppStateManager, update, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "void render( )", asMETHODPR(cgAppStateManager, render, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AppStateManager", "void stop( )", asMETHODPR(cgAppStateManager, stop, ( ), void), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "AppStateManager@+ getAppStateManager( )", asFUNCTIONPR(cgAppStateManager::getInstance, ( ), cgAppStateManager*), asCALL_CDECL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::States::AppStateManager