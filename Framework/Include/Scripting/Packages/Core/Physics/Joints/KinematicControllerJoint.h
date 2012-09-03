#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/Joints/cgKinematicControllerJoint.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Joints {

// Package declaration
namespace KinematicControllerJoint
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Joints.KinematicControllerJoint" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerEnum( "KinematicControllerMode" ) );
            BINDSUCCESS( engine->registerObjectType( "KinematicControllerJoint", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgKinematicControllerJoint::ConstraintMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "KinematicControllerMode", "SinglePoint", cgKinematicControllerJoint::SinglePoint ) );
            BINDSUCCESS( engine->registerEnumValue( "KinematicControllerMode", "PreserveOrientation", cgKinematicControllerJoint::PreserveOrientation ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgKinematicControllerJoint (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgKinematicControllerJoint>( engine );

            // Register the object behaviors
            cgChar * typeName = "KinematicControllerJoint";
            BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_FACTORY, "KinematicControllerJoint@ f( PhysicsWorld@+, PhysicsBody@+, Vector3 )", asFUNCTIONPR(kinematicControllerJointFactory, ( cgPhysicsWorld*, cgPhysicsBody*, cgVector3), cgKinematicControllerJoint*), asCALL_CDECL) );

            // Register the base object methods
            Core::Physics::Joints::PhysicsJoint::registerJointMethods<cgKinematicControllerJoint>( engine, typeName );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setConstraintMode( KinematicControllerMode )", asMETHODPR(cgKinematicControllerJoint,setConstraintMode,( cgKinematicControllerJoint::ConstraintMode ),void), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaxAngularFriction( float )", asMETHODPR(cgKinematicControllerJoint,setMaxAngularFriction,( cgFloat ),void), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaxLinearFriction( float )", asMETHODPR(cgKinematicControllerJoint,setMaxLinearFriction,( cgFloat ),void), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPosition( const Vector3 &in )", asMETHODPR(cgKinematicControllerJoint,setPosition,( const cgVector3& ),void), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setOrientation( const Quaternion &in )", asMETHODPR(cgKinematicControllerJoint,setOrientation,( const cgQuaternion& ),void), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setTransform( const Transform &in )", asMETHODPR(cgKinematicControllerJoint,setTransform,( const cgTransform& ),void), asCALL_THISCALL ) );

            BINDSUCCESS( engine->registerObjectMethod( typeName, "KinematicControllerMode getConstraintMode( ) const", asMETHODPR(cgKinematicControllerJoint,getConstraintMode,( ) const, cgKinematicControllerJoint::ConstraintMode ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaxAngularFriction( ) const", asMETHODPR(cgKinematicControllerJoint,getMaxAngularFriction,( ) const, float ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaxLinearFriction( ) const", asMETHODPR(cgKinematicControllerJoint,getMaxLinearFriction,( ) const, float ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "Vector3 getPosition( ) const", asMETHODPR(cgKinematicControllerJoint,getPosition,( ) const, cgVector3 ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "const Quaternion & getOrientation( ) const", asMETHODPR(cgKinematicControllerJoint,getOrientation,( ) const, const cgQuaternion& ), asCALL_THISCALL ) );
        }

        //---------------------------------------------------------------------
        //  Name : kinematicControllerJointFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgKinematicControllerJoint class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgKinematicControllerJoint * kinematicControllerJointFactory( cgPhysicsWorld * world, cgPhysicsBody * body, cgVector3 worldSpaceHandle )
        {
            return new cgKinematicControllerJoint( world, body, worldSpaceHandle );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Joints::KinematicControllerJoint