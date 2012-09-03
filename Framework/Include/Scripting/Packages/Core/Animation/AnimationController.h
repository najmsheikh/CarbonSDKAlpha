#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Animation/cgAnimationController.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Animation {

// Package declaration
namespace AnimationController
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Animation.AnimationController" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AnimationTrackDesc", sizeof(cgAnimationController::TrackDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "AnimationController", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAnimationController::TrackDesc (Struct)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgAnimationController::TrackDesc>( engine, "AnimationTrackDesc" );
            
            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "double position" , offsetof(cgAnimationController::TrackDesc,position) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "float weight"    , offsetof(cgAnimationController::TrackDesc,weight) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "float speed"     , offsetof(cgAnimationController::TrackDesc,speed) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "int32 firstFrame", offsetof(cgAnimationController::TrackDesc,firstFrame) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "int32 lastFrame" , offsetof(cgAnimationController::TrackDesc,lastFrame) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AnimationTrackDesc", "bool enabled"    , offsetof(cgAnimationController::TrackDesc,enabled) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAnimationController (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAnimationController>( engine );

            // Register the object methods
            // ToDo: void                advanceTime         ( cgDouble timeElapsed, const TargetMap & targets );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "double getTime() const", asMETHODPR(cgAnimationController, getTime, () const, cgDouble ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "void resetTime()", asMETHODPR(cgAnimationController, resetTime, (), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "void setTrackLimit( uint16 )", asMETHODPR(cgAnimationController, setTrackLimit, ( cgUInt16 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackAnimationSet( uint16, const AnimationSetHandle &in )", asMETHODPR(cgAnimationController, setTrackAnimationSet, ( cgUInt16, const cgAnimationSetHandle& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "const AnimationSetHandle & getTrackAnimationSet( uint16 ) const", asMETHODPR(cgAnimationController, getTrackAnimationSet, ( cgUInt16 ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackWeight( uint16, float )", asMETHODPR(cgAnimationController, setTrackWeight, ( cgUInt16, cgFloat ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackSpeed( uint16, float )", asMETHODPR(cgAnimationController, setTrackSpeed, ( cgUInt16, cgFloat ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackPosition( uint16, double )", asMETHODPR(cgAnimationController, setTrackPosition, ( cgUInt16, cgDouble ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "double getTrackPosition( uint16 ) const", asMETHODPR(cgAnimationController, getTrackPosition, ( cgUInt16 ) const, cgDouble ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackFrameLimits( uint16, int32, int32 )", asMETHODPR(cgAnimationController, setTrackFrameLimits, ( cgUInt16, cgInt32, cgInt32 ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackDesc( uint16, const AnimationTrackDesc &in )", asMETHODPR(cgAnimationController, setTrackDesc, ( cgUInt16, const cgAnimationController::TrackDesc& ), bool ), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Animation::AnimationController
