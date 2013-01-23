#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgPointLight.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Lights {

// Package declaration
namespace PointLight
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgLightNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::Lights::Light::registerNodeMethods<type>( engine, typeName );

    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgLightObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::Lights::Light::registerObjectMethods<type>( engine, typeName );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights.PointLight" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PointLightObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "PointLightNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPointLightObject>( engine );
            registerHandleBehaviors<cgPointLightNode>( engine );

            // ToDo: Finish these types

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_PointLightObject", (void*)&RTID_PointLightObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_PointLightNode", (void*)&RTID_PointLightNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgPointLightObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgPointLightObject>( engine, "PointLightObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgPointLightNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgPointLightNode>( engine, "PointLightNode" );

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Lights::PointLight