#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsController.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Controllers {

// Package declaration
namespace PhysicsController
{
    //-------------------------------------------------------------------------
    // Name : registerControllerMethods ()
    // Desc : Register the base cgPhysicsController class methods. Can be
    //        called by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerControllerMethods( cgScriptEngine * engine, const cgChar * typeName )
    {   
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void preStep( float )", asMETHODPR(type, preStep, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void postStep( float )", asMETHODPR(type, postStep, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool initialize( )", asMETHODPR(type, initialize, ( ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyForce( const Vector3& )", asMETHODPR(type, applyForce, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool supportsInputChannels( ) const", asMETHODPR(type, supportsInputChannels, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCollidable( bool )", asMETHODPR(type, setCollidable, ( bool ), void ), asCALL_THISCALL) );        
    
    } // End Method registerControllerMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Controllers.PhysicsController" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PhysicsController", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPhysicsController>( engine );

            // Register the object methods
            registerControllerMethods<cgPhysicsController>( engine, "PhysicsController" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Controllers::PhysicsController