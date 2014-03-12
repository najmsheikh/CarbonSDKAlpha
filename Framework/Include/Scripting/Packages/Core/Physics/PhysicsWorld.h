#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsWorld.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace PhysicsWorld
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.PhysicsWorld" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PhysicsWorld", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgPhysicsWorld>( engine );

            // Register the base methods
            Core::System::Events::EventDispatcher::registerDispatcherMethods<cgPhysicsWorld>( engine, "PhysicsWorld" );

            // Register the object methods.
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "bool initialize( const BoundingBox &in )", asMETHODPR(cgPhysicsWorld, initialize, (const cgBoundingBox&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void update( float )", asMETHODPR(cgPhysicsWorld, update, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void setSystemScale( float )", asMETHODPR(cgPhysicsWorld, setSystemScale, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void clearForces( )", asMETHODPR(cgPhysicsWorld, clearForces, (), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void addEntity( PhysicsEntity@+ )", asMETHODPR(cgPhysicsWorld, addEntity, ( cgPhysicsEntity* ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void removeEntity( PhysicsEntity@+ )", asMETHODPR(cgPhysicsWorld, removeEntity, ( cgPhysicsEntity* ), void), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void addController( PhysicsController@+ )", asMETHODPR(cgPhysicsWorld, addController, ( cgPhysicsController* ), void), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void removeController( PhysicsController@+ )", asMETHODPR(cgPhysicsWorld, removeController, ( cgPhysicsController* ), void), asCALL_THISCALL) );
            // ToDo: cgPhysicsShape    * getExistingShape    ( const cgPhysicsShapeCacheKey & key ) const;
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "float toPhysicsScale( ) const", asMETHODPR(cgPhysicsWorld, toPhysicsScale, () const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "float toPhysicsScale( float ) const", asMETHODPR(cgPhysicsWorld, toPhysicsScale, ( cgFloat ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "Vector3 toPhysicsScale( const Vector3&in ) const", asMETHODPR(cgPhysicsWorld, toPhysicsScale, ( const cgVector3& ) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "BoundingBox toPhysicsScale( const BoundingBox &in ) const", asMETHODPR(cgPhysicsWorld, toPhysicsScale, ( const cgBoundingBox& ) const, cgBoundingBox), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "float fromPhysicsScale( ) const", asMETHODPR(cgPhysicsWorld, fromPhysicsScale, () const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "float fromPhysicsScale( float ) const", asMETHODPR(cgPhysicsWorld, fromPhysicsScale, ( cgFloat ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "Vector3 fromPhysicsScale( const Vector3&in ) const", asMETHODPR(cgPhysicsWorld, fromPhysicsScale, ( const cgVector3& ) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "BoundingBox fromPhysicsScale( const BoundingBox &in ) const", asMETHODPR(cgPhysicsWorld, fromPhysicsScale, ( const cgBoundingBox& ) const, cgBoundingBox), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "bool rayCastClosest( const Vector3 &in, const Vector3 &in, RayCastContact &inout )", asMETHODPR(cgPhysicsWorld, rayCastClosest, ( const cgVector3&, const cgVector3&, cgRayCastContact& ), bool), asCALL_THISCALL) );
            // ToDo: bool                rayCast             ( const cgVector3 & from, const cgVector3 & to, bool sortContacts, cgRayCastContact::Array & contacts );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "int getDefaultMaterialGroupId( DefaultPhysicsMaterialGroup ) const", asMETHODPR(cgPhysicsWorld, getDefaultMaterialGroupId, ( cgDefaultPhysicsMaterialGroup::Base ) const, cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "int createMaterialGroup( )", asMETHODPR(cgPhysicsWorld, createMaterialGroup, ( ), cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "PhysicsWorld", "void enableMaterialCollision( int, int, bool )", asMETHODPR(cgPhysicsWorld, enableMaterialCollision, ( cgInt32, cgInt32, bool ), void), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::PhysicsWorld