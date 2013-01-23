#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgReference.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace References {

// Package declaration
namespace Reference
{
    //-------------------------------------------------------------------------
    // Name : registerReferenceMethods()
    /// <summary>
    /// Register the base cgReference class methods. Can be called by derived 
    /// classes to re-register the behaviors
    /// </summary>
    //-------------------------------------------------------------------------
    template <class type>
    void registerReferenceMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::Events::EventDispatcher::registerDispatcherMethods<type>( engine, typeName );

        // Reference management
        BINDSUCCESS( engine->registerObjectMethod( typeName, "uint getReferenceId( ) const", asMETHODPR(type, getReferenceId, () const, cgUInt32 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "int getReferenceCount( bool ) const", asMETHODPR(type,getReferenceCount,( bool ) const,cgInt32), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void addReference( Reference@+ )", asMETHODPR(type, addReference,( cgReference*), void ), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void addReference( Reference@+, bool )", asMETHODPR(type, addReference,( cgReference*, bool), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "int removeReference( Reference@+ )", asMETHODPR(type, removeReference,( cgReference*), cgInt32 ), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod( typeName, "int removeReference( Reference@+, bool )", asMETHODPR(type, removeReference,( cgReference*, bool), cgInt32 ), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void deleteReference( )", asMETHODPR(type, deleteReference,( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void onReferenceAdded( Reference@+, int, bool )", asMETHODPR(type, onReferenceAdded,( cgReference*, cgInt32, bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void onReferenceRemoved( Reference@+, int, bool )", asMETHODPR(type, onReferenceRemoved,( cgReference*, cgInt32, bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isDisposed() const", asMETHODPR(type, isDisposed,( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isDisposing() const", asMETHODPR(type, isDisposing,( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isInternalReference() const", asMETHODPR(type, isInternalReference,( ) const, bool ), asCALL_THISCALL) );

        // Type queries
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const UID& getReferenceType( ) const", asMETHODPR(type, getReferenceType,() const, const cgUID& ), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool queryReferenceType( const UID &in ) const", asMETHODPR(type, queryReferenceType,( const cgUID&) const, bool ), asCALL_THISCALL) );        

        // Messaging
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool processMessage( Message &inout )", asMETHODPR(type, processMessage, (cgMessage*), bool   ), asCALL_THISCALL) );        
    
    } // End Method registerReferenceMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.References.Reference" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Reference", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgReference>( engine );

            // Register the object methods for the base class (template method)
            registerReferenceMethods<cgReference>( engine, "Reference" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::References::Reference