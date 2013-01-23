#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgNavigationPatrolPoint.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Navigation {

// Package declaration
namespace NavigationPatrolPoint
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Navigation.NavigationPatrolPoint" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "NavigationPatrolPointObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "NavigationPatrolPointNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgNavigationPatrolPointObject>( engine );
            registerHandleBehaviors<cgNavigationPatrolPointNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_NavigationPatrolPointObject", (void*)&RTID_NavigationPatrolPointObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_NavigationPatrolPointNode", (void*)&RTID_NavigationPatrolPointNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationPatrolPointObject (Class)
            ///////////////////////////////////////////////////////////////////////


            
            ///////////////////////////////////////////////////////////////////////
            // cgNavigationPatrolPointNode (Class)
            ///////////////////////////////////////////////////////////////////////

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Navigation::NavigationPatrolPoint