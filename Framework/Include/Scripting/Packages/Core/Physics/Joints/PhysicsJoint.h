#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsJoint.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Joints {

// Package declaration
namespace PhysicsJoint
{
    //-------------------------------------------------------------------------
    // Name : registerJointMethods ()
    // Desc : Register the base cgPhysicsJoint class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerJointMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableBodyCollision( bool )", asMETHODPR(type, enableBodyCollision, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isBodyCollisionEnabled( ) const", asMETHODPR(type, isBodyCollisionEnabled, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "PhysicsBody@+ getBody0( )", asMETHODPR(type, getBody0, ( ), cgPhysicsBody* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "PhysicsBody@+ getBody1( )", asMETHODPR(type, getBody1, ( ), cgPhysicsBody* ), asCALL_THISCALL) );
    
    } // End Method registerJointMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Joints.PhysicsJoint" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PhysicsJoint", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPhysicsJoint>( engine );

            // Register the object methods
            registerJointMethods<cgPhysicsJoint>( engine, "PhysicsJoint" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Joints::PhysicsJoint