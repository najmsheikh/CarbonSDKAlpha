#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgGroupObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Group
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
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setOpen( bool )", asMETHODPR(type, setOpen, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isOpen( ) const", asMETHODPR(type, isOpen, ( ) const, bool), asCALL_THISCALL) );

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
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setOpen( bool )", asMETHODPR(type, setOpen, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isOpen( ) const", asMETHODPR(type, isOpen, ( ) const, bool), asCALL_THISCALL) );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Group" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "GroupObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "GroupNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgGroupObject>( engine );
            registerHandleBehaviors<cgGroupNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_GroupObject", (void*)&RTID_GroupObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_GroupNode", (void*)&RTID_GroupNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgGroupObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgGroupObject>( engine, "GroupObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgGroupNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgGroupNode>( engine, "GroupNode" );            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Group