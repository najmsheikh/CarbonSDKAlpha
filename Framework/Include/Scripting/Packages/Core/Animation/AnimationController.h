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
            BINDSUCCESS( engine->registerObjectType( "AnimationController", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

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
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "uint16 getTrackLimit( ) const", asMETHODPR(cgAnimationController, getTrackLimit, ( ) const, cgUInt16 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackAnimationSet( uint16, const AnimationSetHandle &in )", asMETHODPR(cgAnimationController, setTrackAnimationSet, ( cgUInt16, const cgAnimationSetHandle& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "const AnimationSetHandle & getTrackAnimationSet( uint16 ) const", asMETHODPR(cgAnimationController, getTrackAnimationSet, ( cgUInt16 ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackWeight( uint16, float )", asMETHODPR(cgAnimationController, setTrackWeight, ( cgUInt16, cgFloat ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "float getTrackWeight( uint16 ) const", asMETHODPR(cgAnimationController, getTrackWeight, ( cgUInt16 ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackSpeed( uint16, float )", asMETHODPR(cgAnimationController, setTrackSpeed, ( cgUInt16, cgFloat ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "float getTrackSpeed( uint16 ) const", asMETHODPR(cgAnimationController, getTrackSpeed, ( cgUInt16 ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackPosition( uint16, double )", asMETHODPR(cgAnimationController, setTrackPosition, ( cgUInt16, cgDouble ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "double getTrackPosition( uint16 ) const", asMETHODPR(cgAnimationController, getTrackPosition, ( cgUInt16 ) const, cgDouble ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackFrameLimits( uint16, int32, int32 )", asMETHODPR(cgAnimationController, setTrackFrameLimits, ( cgUInt16, cgInt32, cgInt32 ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackFrameLimits( uint16, const Range &in )", asMETHODPR(cgAnimationController, setTrackFrameLimits, ( cgUInt16, const cgRange& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "Range getTrackFrameLimits( uint16 ) const", asMETHODPR(cgAnimationController, getTrackFrameLimits, ( cgUInt16 ) const, cgRange ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackPlaybackMode( uint16, AnimationPlaybackMode )", asMETHODPR(cgAnimationController, setTrackPlaybackMode, ( cgUInt16, cgAnimationPlaybackMode::Base ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "AnimationPlaybackMode getTrackPlaybackMode( uint16 ) const", asMETHODPR(cgAnimationController, getTrackPlaybackMode, ( cgUInt16 ) const, cgAnimationPlaybackMode::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackDesc( uint16, const AnimationTrackDesc &in )", asMETHODPR(cgAnimationController, setTrackDesc, ( cgUInt16, const cgAnimationTrackDesc& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "const AnimationTrackDesc& getTrackDesc( uint16 ) const", asMETHODPR(cgAnimationController, getTrackDesc, ( cgUInt16 ) const, const cgAnimationTrackDesc& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool setTrackEnabled( uint16, bool )", asMETHODPR(cgAnimationController, setTrackEnabled, ( cgUInt16, bool ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AnimationController", "bool getTrackEnabled( uint16 ) const", asMETHODPR(cgAnimationController, getTrackEnabled, ( cgUInt16 ) const, bool ), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Animation::AnimationController
