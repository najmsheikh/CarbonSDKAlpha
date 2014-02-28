#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/cgPhysicsTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "RigidBodyCreateParams", sizeof(cgRigidBodyCreateParams), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "CollisionContact"     , sizeof(cgCollisionContact), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "DefaultPhysicsShape" ) );
            BINDSUCCESS( engine->registerEnum( "PhysicsModel" ) );
            BINDSUCCESS( engine->registerEnum( "SimulationQuality" ) );
            BINDSUCCESS( engine->registerEnum( "DefaultPhysicsMaterialGroup" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgDefaultPhysicsShape (Enum)
            ///////////////////////////////////////////////////////////////////////
            const cgChar * typeName = "DefaultPhysicsShape";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Auto"       , cgDefaultPhysicsShape::Auto ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Box"        , cgDefaultPhysicsShape::Box ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Sphere"     , cgDefaultPhysicsShape::Sphere ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Cylinder"   , cgDefaultPhysicsShape::Cylinder ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Cone"       , cgDefaultPhysicsShape::Cone ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Capsule"    , cgDefaultPhysicsShape::Capsule ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ConvexHull" , cgDefaultPhysicsShape::ConvexHull ) );

            ///////////////////////////////////////////////////////////////////////
            // cgPhysicsModel (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "PhysicsModel";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"         , cgPhysicsModel::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CollisionOnly", cgPhysicsModel::CollisionOnly ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RigidStatic"  , cgPhysicsModel::RigidStatic ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RigidDynamic" , cgPhysicsModel::RigidDynamic ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Kinematic"    , cgPhysicsModel::Kinematic ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSimulationQuality (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "SimulationQuality";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Default", cgSimulationQuality::Default ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CCD"    , cgSimulationQuality::CCD ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDefaultPhysicsMaterialGroup (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "DefaultPhysicsMaterialGroup";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Standard"       , cgDefaultPhysicsMaterialGroup::Standard ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CastOnly"       , cgDefaultPhysicsMaterialGroup::CastOnly ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Character"      , cgDefaultPhysicsMaterialGroup::Character ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PlayerCharacter", cgDefaultPhysicsMaterialGroup::PlayerCharacter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Ragdoll"        , cgDefaultPhysicsMaterialGroup::Ragdoll ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgRigidBodyCreateParams (Struct)
            ///////////////////////////////////////////////////////////////////////
            typeName = "RigidBodyCreateParams";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgRigidBodyCreateParams>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "PhysicsModel model"        , offsetof(cgRigidBodyCreateParams,model) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "Transform initialTransform", offsetof(cgRigidBodyCreateParams,initialTransform) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "SimulationQuality quality" , offsetof(cgRigidBodyCreateParams,quality) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float mass"                , offsetof(cgRigidBodyCreateParams,mass) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "Vector3 centerofMass"      , offsetof(cgRigidBodyCreateParams,centerOfMass) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "Vector3 shapeOffset"       , offsetof(cgRigidBodyCreateParams,shapeOffset) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgCollisionContact (Struct)
            ///////////////////////////////////////////////////////////////////////
            typeName = "CollisionContact";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgCollisionContact>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "PhysicsBody @ body"   , offsetof(cgCollisionContact,body) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float intersectParam" , offsetof(cgCollisionContact,intersectParam) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "Vector3 contactNormal", offsetof(cgCollisionContact,contactNormal) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int collisionId"      , offsetof(cgCollisionContact,collisionId) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Physics::Types