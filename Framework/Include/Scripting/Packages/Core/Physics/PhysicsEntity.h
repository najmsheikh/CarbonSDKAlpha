#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsEntity.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace PhysicsEntity
{
    //-------------------------------------------------------------------------
    // Name : registerEntityMethods ()
    // Desc : Register the base cgPhysicsEntity class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerEntityMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void clearForces( )", asMETHODPR(type, clearForces, (), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Transform getTransform( double ) const", asMETHODPR(type, getTransform, ( cgDouble ) const, cgTransform ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setTransform( const Transform &in )", asMETHODPR(type, setTransform, ( const cgTransform& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "PhysicsWorld@+ getPhysicsWorld( ) const", asMETHODPR(type, getPhysicsWorld, ( ) const, cgPhysicsWorld* ), asCALL_THISCALL) );
    
    } // End Method registerEntityMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.PhysicsEntity" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PhysicsEntity", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPhysicsEntity>( engine );

            // Register the object methods
            registerEntityMethods<cgPhysicsEntity>( engine, "PhysicsEntity" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::PhysicsEntity