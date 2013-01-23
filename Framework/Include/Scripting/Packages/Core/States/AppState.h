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
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isScripted( ) const", asMETHODPR(type, isScripted, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String & getStateId( ) const", asMETHODPR(type, getStateId, ( ) const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppStateManager @+ getManager( )", asMETHODPR(type, getManager, ( ), cgAppStateManager* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppState @+ getTerminalState( )", asMETHODPR(type, getTerminalState, ( ), cgAppState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppState @+ getRootState( )", asMETHODPR(type, getRootState, ( ), cgAppState* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "AppState @+ getParentState( )", asMETHODPR(type, getParentState, ( ), cgAppState* ), asCALL_THISCALL) );
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
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool registerEventAction( const String&in, const AppStateEventActionDesc &in )", asMETHODPR(type, registerEventAction, ( const cgString& , const cgAppState::EventActionDesc& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void unregisterEventAction( const String&in )", asMETHODPR(type, unregisterEventAction, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool spawnChildState( const String&in, bool )", asMETHODPR(type, spawnChildState, ( const cgString&, bool ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "IScriptedAppState@ getScriptObject( )", asFUNCTIONPR(getScriptObject,( type* ), asIScriptObject*), asCALL_CDECL_OBJLAST ) );
    
    } // End Method registerStateMethods<>

    //-------------------------------------------------------------------------
    // Name : getScriptObject ()
    // Desc : Wrapper function that returns a reference to the script based
    //        'IScriptedAppState' interface rather than the C++ side 
    //        'cgScriptObject' that is returned by the 'cgAppState' 
    //        native method of the same name.
    //-------------------------------------------------------------------------
    template <class type>
    asIScriptObject * getScriptObject( type* thisPointer )
    {
        cgScriptObject * object = thisPointer->getScriptObject();
        if ( object && object->getInternalObject() )
        {
            object->getInternalObject()->AddRef();
            return object->getInternalObject();
        
        } // End if valid
        return CG_NULL;
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.States.AppState" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AppState", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "AppStateEventActionDesc", sizeof(cgAppState::EventActionDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "AppStateEventActionType" ) );
            BINDSUCCESS( engine->registerEnum( "AppStateEventActionFlags" ) );

            // Interface Types
            BINDSUCCESS( engine->registerInterface("IScriptedAppState" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAppState::EventActionType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "Transition", cgAppState::ActionType_Transition ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "TransitionRoot", cgAppState::ActionType_TransitionRoot ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "SpawnChild", cgAppState::ActionType_SpawnChild ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "EndState", cgAppState::ActionType_EndState ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "EndRoot", cgAppState::ActionType_EndRoot ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "PassUp", cgAppState::ActionType_PassUp ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionType", "PassDown", cgAppState::ActionType_PassDown ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAppState::EventActionFlags (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionFlags", "None", cgAppState::ActionFlag_None ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionFlags", "SuspendParent", cgAppState::ActionFlag_SuspendParent ) );
            BINDSUCCESS( engine->registerEnumValue( "AppStateEventActionFlags", "UseTransitionState", cgAppState::ActionFlag_UseTransitionState ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAppState::EventActionDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgAppState::EventActionDesc>( engine, "AppStateEventActionDesc" );

            // Register Values
            BINDSUCCESS( engine->registerObjectProperty( "AppStateEventActionDesc", "AppStateEventActionType actionType", offsetof(cgAppState::EventActionDesc,actionType) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AppStateEventActionDesc", "uint flags", offsetof(cgAppState::EventActionDesc,flags) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AppStateEventActionDesc", "String toStateId", offsetof(cgAppState::EventActionDesc,toStateId) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AppStateEventActionDesc", "String transitionStateId", offsetof(cgAppState::EventActionDesc,transitionStateId) ) );            

            ///////////////////////////////////////////////////////////////////////
            // cgAppState (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAppState>( engine );

            // Register the object methods
            registerStateMethods<cgAppState>( engine, "AppState" );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "AppState", asBEHAVE_FACTORY, "AppState @ f( const String &in, const InputStream &in, ResourceManager@+ )", asFUNCTIONPR(appStateFactory, ( const cgString&, const cgInputStream&, cgResourceManager* ), cgAppState*), asCALL_CDECL) );
            BINDSUCCESS( engine->registerObjectBehavior( "AppState", asBEHAVE_FACTORY, "AppState @ f( const String &in, const String &in, ResourceManager@+ )", asFUNCTIONPR(appStateFactory, ( const cgString&, const cgString&, cgResourceManager* ), cgAppState*), asCALL_CDECL) );

            ///////////////////////////////////////////////////////////////////////
            // IScriptedAppState (Interface)
            ///////////////////////////////////////////////////////////////////////

            // Register the /required/ interface methods
            // ToDo: Are any actually required?
        }

        //---------------------------------------------------------------------
        //  Name : appStateFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgAppState class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgAppState * appStateFactory( const cgString & stateId, const cgInputStream & scriptStream, cgResourceManager * resourceManager )
        {
            return new cgAppState( stateId, scriptStream, resourceManager );
        }

        //---------------------------------------------------------------------
        //  Name : appStateFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgAppState class (string as
        /// input stream).
        /// </summary>
        //---------------------------------------------------------------------
        static cgAppState * appStateFactory( const cgString & stateId, const cgString & scriptStream, cgResourceManager * resourceManager )
        {
            return new cgAppState( stateId, scriptStream, resourceManager );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::States::AppState