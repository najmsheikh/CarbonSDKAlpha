#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgNavigationWaypoint.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Navigation {

// Package declaration
namespace NavigationWaypoint
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Navigation.NavigationWaypoint" )
        END_SCRIPT_PACKAGE( )

        //-------------------------------------------------------------------------
        // Name : registerNodeMethods ()
        // Desc : Register the base cgNavigationWaypointNode's class methods. Can 
        //        be called by derived classes to re-register the behaviors.
        //-------------------------------------------------------------------------
        template <class type>
        void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
        {
            // Register base class object methods
            Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

            // Register methods.
            //BINDSUCCESS( engine->registerObjectMethod( typeName, "void setOpen( bool )", asMETHODPR(type, setOpen, ( bool ), void), asCALL_THISCALL) );
            //BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isOpen( ) const", asMETHODPR(type, isOpen, ( ) const, bool), asCALL_THISCALL) );

        } // End Method registerNodeMethods<>

        //-------------------------------------------------------------------------
        // Name : registerObjectMethods ()
        // Desc : Register the base cgNavigationWaypointObject's class methods. Can 
        //        be called by derived classes to re-register the behaviors.
        //-------------------------------------------------------------------------
        template <class type>
        void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
        {
            // Register base class object methods
            Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

            // Register methods.
            //BINDSUCCESS( engine->registerObjectMethod( typeName, "void setOpen( bool )", asMETHODPR(type, setOpen, ( bool ), void), asCALL_THISCALL) );
            //BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isOpen( ) const", asMETHODPR(type, isOpen, ( ) const, bool), asCALL_THISCALL) );

        } // End Method registerObjectMethods<>


        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "NavigationWaypointObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "NavigationWaypointNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgNavigationWaypointObject>( engine );
            registerHandleBehaviors<cgNavigationWaypointNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_NavigationWaypointObject", (void*)&RTID_NavigationWaypointObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_NavigationWaypointNode", (void*)&RTID_NavigationWaypointNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationWaypointObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgNavigationWaypointObject>( engine, "NavigationWaypointObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgNavigationWaypointNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgNavigationWaypointNode>( engine, "NavigationWaypointNode" );      
        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Navigation::NavigationWaypoint