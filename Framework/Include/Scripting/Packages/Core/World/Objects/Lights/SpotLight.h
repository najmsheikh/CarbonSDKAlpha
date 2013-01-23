#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgSpotLight.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Lights {

// Package declaration
namespace SpotLight
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
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights.SpotLight" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SpotLightObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "SpotLightNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSpotLightObject>( engine );
            registerHandleBehaviors<cgSpotLightNode>( engine );

            // ToDo: Finish these types

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_SpotLightObject", (void*)&RTID_SpotLightObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_SpotLightNode", (void*)&RTID_SpotLightNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSpotLightObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgSpotLightObject>( engine, "SpotLightObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgSpotLightNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgSpotLightNode>( engine, "SpotLightNode" );

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Lights::SpotLight