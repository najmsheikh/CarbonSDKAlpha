#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgDummyObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Dummy
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgGroupNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( float )", asMETHODPR(type, setSize, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSize( ) const", asMETHODPR(type, getSize, ( ) const, cgFloat), asCALL_THISCALL) );

    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgGroupObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( float )", asMETHODPR(type, setSize, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSize( ) const", asMETHODPR(type, getSize, ( ) const, cgFloat), asCALL_THISCALL) );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Dummy" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DummyObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "DummyNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgDummyObject>( engine );
            registerHandleBehaviors<cgDummyNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_DummyObject", (void*)&RTID_DummyObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_DummyNode", (void*)&RTID_DummyNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDummyObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgDummyObject>( engine, "DummyObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgDummyNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgDummyNode>( engine, "DummyNode" );            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Dummy