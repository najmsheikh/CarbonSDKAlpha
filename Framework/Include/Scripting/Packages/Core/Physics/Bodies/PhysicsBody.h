#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsBody.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Bodies {

// Package declaration
namespace PhysicsBody
{
    //-------------------------------------------------------------------------
    // Name : registerBodyMethods ()
    // Desc : Register the base cgPhysicsBody class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerBodyMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Physics::PhysicsEntity::registerEntityMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Vector3 getVelocity( ) const", asMETHODPR(type, getVelocity, () const, cgVector3 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Vector3 getAngularVelocity( ) const", asMETHODPR(type, getAngularVelocity, () const, cgVector3 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setVelocity( const Vector3 &in )", asMETHODPR(type, setVelocity, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAngularVelocity( const Vector3 &in )", asMETHODPR(type, setAngularVelocity, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMass( float )", asMETHODPR(type, setMass, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMass( ) const", asMETHODPR(type, getMass, ( ) const, cgFloat ), asCALL_THISCALL) );
        // ToDo: BINDSUCCESS( engine->registerObjectMethod( typeName, "PhysicsShape @+ getShape( ) const", asMETHODPR(type, getShape, ( ) const, cgPhysicsShape* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyForce( const Vector3 &in )", asMETHODPR(type, applyForce, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyForce( const Vector3 &in, const Vector3 &in )", asMETHODPR(type, applyForce, ( const cgVector3&, const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyImpulse( const Vector3 &in )", asMETHODPR(type, applyImpulse, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyImpulse( const Vector3 &in, const Vector3 &in )", asMETHODPR(type, applyImpulse, ( const cgVector3&, const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyTorque( const Vector3 &in )", asMETHODPR(type, applyTorque, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyTorqueImpulse( const Vector3 &in )", asMETHODPR(type, applyTorqueImpulse, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCustomGravity( const Vector3 &in )", asMETHODPR(type, setCustomGravity, ( const cgVector3& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Vector3 getCustomGravity( ) const", asMETHODPR(type, getCustomGravity, ( ) const, cgVector3 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableCustomGravity( bool )", asMETHODPR(type, enableCustomGravity, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isCustomGravityEnabled( ) const", asMETHODPR(type, isCustomGravityEnabled, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableAutoSleep( bool )", asMETHODPR(type, enableAutoSleep, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isAutoSleepEnabled( ) const", asMETHODPR(type, isAutoSleepEnabled, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableContinuousCollision( bool )", asMETHODPR(type, enableContinuousCollision, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isContinuousCollisionEnabled( ) const", asMETHODPR(type, isContinuousCollisionEnabled, ( ) const, bool ), asCALL_THISCALL) );
        
        // ToDo: ? virtual void                onPhysicsBodyTransformed( cgPhysicsBodyTransformedEventArgs * e );
    
    } // End Method registerBodyMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Bodies.PhysicsBody" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PhysicsBody", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPhysicsBody>( engine );

            // Register the object methods
            registerBodyMethods<cgPhysicsBody>( engine, "PhysicsBody" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Bodies::PhysicsBody