#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/Bodies/cgRigidBody.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Bodies {

// Package declaration
namespace RigidBody
{
    //-------------------------------------------------------------------------
    // Name : registerRigidBodyMethods ()
    // Desc : Register the base cgRigidBody class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerRigidBodyMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Physics::Bodies::PhysicsBody::registerBodyMethods<type>( engine, typeName );
        
        // Register the object methods
        // None at this time    

    } // End Method registerRigidBodyMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Bodies.RigidBody" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RigidBody", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgRigidBody>( engine );

            // Register the object methods
            registerRigidBodyMethods<cgRigidBody>( engine, "RigidBody" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Bodies::RigidBody