#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <States/cgAppStateManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace States {

// Package declaration
namespace AppState
{
    //-------------------------------------------------------------------------
    // Name : registerStateMethods ()
    // Desc : Register the base cgAppState class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerStateMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isActive( ) const", asMETHODPR(type, isActive, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isSuspended( ) const", asMETHODPR(type, isSuspended, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isTransitionState( ) const", asMETHODPR(type, isTransitionState, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isBegun( ) const", asMETHODPR(type, isBegun, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String & getStateId( ) const", asMETHODPR(type, getStateId, ( ) const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppState @+ getTerminalState( )", asMETHODPR(type, getTerminalState, ( ), cgAppState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppState @+ getRootState( )", asMETHODPR(type, getRootState, ( ), cgAppState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppTransitionState @+ getOutgoingTransition( )", asMETHODPR(type, getOutgoingTransition, ( ), cgAppTransitionState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppTransitionState @+ getIncomingTransition( )", asMETHODPR(type, getIncomingTransition, ( ), cgAppTransitionState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool initialize( )", asMETHODPR(type, initialize, ( ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool begin( )", asMETHODPR(type, begin, ( ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void end( )", asMETHODPR(type, end, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void update( )", asMETHODPR(type, update, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void render( )", asMETHODPR(type, render, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void suspend( )", asMETHODPR(type, suspend, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resume( )", asMETHODPR(type, resume, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void raiseEvent( const String&in )", asMETHODPR(type, raiseEvent, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void raiseEvent( const String&in, bool )", asMETHODPR(type, raiseEvent, ( const cgString&,bool), void ), asCALL_THISCALL) );
        
        // ToDo: registerEventAction & unregisterEventAction if needed
    
    } // End Method registerStateMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States.AppState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AppState", 0, asOBJ_REF ) );

            // Interface Types
            BINDSUCCESS( engine->registerInterface("IScriptedAppState" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAppState (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAppState>( engine );

            // Register the object methods
            registerStateMethods<cgAppState>( engine, "AppState" );

            ///////////////////////////////////////////////////////////////////////
            // IScriptedAppState (Interface)
            ///////////////////////////////////////////////////////////////////////

            // Register the /required/ interface methods
            // ToDo: Are any actually required?
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::States::AppState