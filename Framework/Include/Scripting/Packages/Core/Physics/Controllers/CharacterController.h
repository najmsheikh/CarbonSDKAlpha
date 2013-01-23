#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/Controllers/cgCharacterController.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Controllers {

// Package declaration
namespace CharacterController
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Controllers.CharacterController" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "CharacterController", 0, asOBJ_REF ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "CharacterStandingMode" ) );
            BINDSUCCESS( engine->registerEnum( "CharacterState" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgCharacterController::StandingMode (Enum)
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "CharacterStandingMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Standing" , cgCharacterController::Standing ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Crouching", cgCharacterController::Crouching ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Prone"    , cgCharacterController::Prone ) );

            ///////////////////////////////////////////////////////////////////////
            // cgCharacterController::CharacterState (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "CharacterState";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Airborne", cgCharacterController::Airborne ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "OnFloor" , cgCharacterController::OnFloor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "OnRamp"  , cgCharacterController::OnRamp ) );

            ///////////////////////////////////////////////////////////////////////
            // cgCharacterController (Class)
            ///////////////////////////////////////////////////////////////////////

            typeName = "CharacterController";

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgCharacterController>( engine );

            // Register base class object methods
            Core::Physics::Controllers::PhysicsController::registerControllerMethods<cgCharacterController>( engine, typeName );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "CharacterController", asBEHAVE_FACTORY, "CharacterController @ f( PhysicsWorld@+ )", asFUNCTIONPR(characterControllerFactory, ( cgPhysicsWorld* ), cgCharacterController*), asCALL_CDECL) );
            BINDSUCCESS( engine->registerObjectBehavior( "CharacterController", asBEHAVE_FACTORY, "CharacterController @ f( PhysicsWorld@+, bool )", asFUNCTIONPR(characterControllerFactory, ( cgPhysicsWorld*, bool ), cgCharacterController*), asCALL_CDECL) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( typeName, "CharacterState getCharacterState( ) const", asMETHODPR(cgCharacterController, getCharacterState, ( ) const, cgCharacterController::CharacterState ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "NavigationTargetState getNavigationState( ) const", asMETHODPR(cgCharacterController, getNavigationState, ( ) const, cgNavigationTargetState::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setKinematicCushion( float )", asMETHODPR(cgCharacterController, setKinematicCushion, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCharacterMass( float )", asMETHODPR(cgCharacterController, setCharacterMass, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCharacterHeight( float )", asMETHODPR(cgCharacterController, setCharacterHeight, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCharacterRadius( float )", asMETHODPR(cgCharacterController, setCharacterRadius, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setCharacterOffset( float )", asMETHODPR(cgCharacterController, setCharacterOffset, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumSlope( float )", asMETHODPR(cgCharacterController, setMaximumSlope, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumStepHeight( float )", asMETHODPR(cgCharacterController, setMaximumStepHeight, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumWalkSpeed( float )", asMETHODPR(cgCharacterController, setMaximumWalkSpeed, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumFlySpeed( float )", asMETHODPR(cgCharacterController, setMaximumFlySpeed, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setWalkAcceleration( float )", asMETHODPR(cgCharacterController, setWalkAcceleration, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setFlyAcceleration( float )", asMETHODPR(cgCharacterController, setFlyAcceleration, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setGravity( const Vector3 &in )", asMETHODPR(cgCharacterController, setGravity, ( const cgVector3& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setJumpImpulse( float )", asMETHODPR(cgCharacterController, setJumpImpulse, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAirborneWalkDamping( float )", asMETHODPR(cgCharacterController, setAirborneWalkDamping, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRampWalkDamping( float )", asMETHODPR(cgCharacterController, setRampWalkDamping, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRampJumpDamping( float )", asMETHODPR(cgCharacterController, setRampJumpDamping, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "bool requestStandingMode( CharacterStandingMode )", asMETHODPR(cgCharacterController, requestStandingMode, ( cgCharacterController::StandingMode ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableFlyMode( bool )", asMETHODPR(cgCharacterController, enableFlyMode, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void enableFlyMode( bool, bool )", asMETHODPR(cgCharacterController, enableFlyMode, ( bool, bool ), void ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getKinematicCushion( ) const", asMETHODPR(cgCharacterController, getKinematicCushion, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getCharacterMass( ) const", asMETHODPR(cgCharacterController, getCharacterMass, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getCharacterHeight( ) const", asMETHODPR(cgCharacterController, getCharacterHeight, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getCharacterHeight( bool ) const", asMETHODPR(cgCharacterController, getCharacterHeight, ( bool ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getCharacterRadius( ) const", asMETHODPR(cgCharacterController, getCharacterRadius, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getCharacterOffset( ) const", asMETHODPR(cgCharacterController, getCharacterOffset, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaximumSlope( ) const", asMETHODPR(cgCharacterController, getMaximumSlope, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaximumStepHeight( ) const", asMETHODPR(cgCharacterController, getMaximumStepHeight, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaximumWalkSpeed( ) const", asMETHODPR(cgCharacterController, getMaximumWalkSpeed, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getMaximumFlySpeed( ) const", asMETHODPR(cgCharacterController, getMaximumFlySpeed, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getWalkAcceleration( ) const", asMETHODPR(cgCharacterController, getWalkAcceleration, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getFlyAcceleration( ) const", asMETHODPR(cgCharacterController, getFlyAcceleration, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "const Vector3& getGravity( ) const", asMETHODPR(cgCharacterController, getGravity, ( ) const, const cgVector3& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "const Vector3& getVelocity( ) const", asMETHODPR(cgCharacterController, getVelocity, ( ) const, const cgVector3& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getJumpImpulse( ) const", asMETHODPR(cgCharacterController, getJumpImpulse, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getAirborneWalkDamping( ) const", asMETHODPR(cgCharacterController, getAirborneWalkDamping, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRampWalkDamping( ) const", asMETHODPR(cgCharacterController, getRampWalkDamping, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRampJumpDamping( ) const", asMETHODPR(cgCharacterController, getRampJumpDamping, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "CharacterStandingMode getRequestedStandingMode( ) const", asMETHODPR(cgCharacterController, getRequestedStandingMode, ( ) const, cgCharacterController::StandingMode ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "CharacterStandingMode getActualStandingMode( ) const", asMETHODPR(cgCharacterController, getActualStandingMode, ( ) const, cgCharacterController::StandingMode ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "const Quaternion & getSuggestedHeading( ) const", asMETHODPR(cgCharacterController, getSuggestedHeading, ( ) const, const cgQuaternion& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isFlyModeEnabled( ) const", asMETHODPR(cgCharacterController, isFlyModeEnabled, ( ) const, bool ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( typeName, "bool navigateTo( const Vector3 &in )", asMETHODPR(cgCharacterController, navigateTo, ( const cgVector3& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyImpulse( const Vector3 &in )", asMETHODPR(cgCharacterController, applyImpulse, ( const cgVector3& ), void ), asCALL_THISCALL) );

        }

        //---------------------------------------------------------------------
        //  Name : characterControllerFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgCharacterController class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgCharacterController * characterControllerFactory( cgPhysicsWorld * world )
        {
            return new cgCharacterController( world, true );
        }

        //---------------------------------------------------------------------
        //  Name : characterControllerFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgCharacterController class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgCharacterController * characterControllerFactory( cgPhysicsWorld * world, bool playerControlled )
        {
            return new cgCharacterController( world, playerControlled );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Controllers::CharacterController