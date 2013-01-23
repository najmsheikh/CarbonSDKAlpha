#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgDirectionalLight.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Lights {

// Package declaration
namespace DirectionalLight
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
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights.DirectionalLight" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DirectionalLightObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "DirectionalLightNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgDirectionalLightObject>( engine );
            registerHandleBehaviors<cgDirectionalLightNode>( engine );

            // ToDo: Finish these types

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_DirectionalLightObject", (void*)&RTID_DirectionalLightObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_DirectionalLightNode", (void*)&RTID_DirectionalLightNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDirectionalLightObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgDirectionalLightObject>( engine, "DirectionalLightObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgDirectionalLightNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgDirectionalLightNode>( engine, "DirectionalLightNode" );

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Lights::DirectionalLight