#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgActor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Actor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Actor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ActorObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "ActorNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgActorObject>( engine );
            registerHandleBehaviors<cgActorNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_ActorObject", (void*)&RTID_ActorObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_ActorNode", (void*)&RTID_ActorNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgActorObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::Group::registerObjectMethods<cgActorObject>( engine, "ActorObject" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "ActorObject", "const AnimationSetHandle & getAnimationSetByName( const String &in ) const", asMETHODPR(cgActorObject, getAnimationSetByName, ( const cgString& ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ActorObject", "uint32 getAnimationSetCount( ) const", asMETHODPR(cgActorObject, getAnimationSetCount, ( ) const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ActorObject", "const AnimationSetHandle & getAnimationSet( uint32 ) const", asMETHODPR(cgActorObject, getAnimationSet, ( cgUInt32 ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgActorNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::Group::registerNodeMethods<cgActorNode>( engine, "ActorNode" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "ActorNode", "AnimationController@+ getAnimationController( ) const", asMETHODPR(cgActorNode, getAnimationController, ( ) const, cgAnimationController*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ActorNode", "const AnimationSetHandle & getAnimationSetByName( const String &in ) const", asMETHODPR(cgActorNode, getAnimationSetByName, ( const cgString& ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ActorNode", "uint32 getAnimationSetCount( ) const", asMETHODPR(cgActorNode, getAnimationSetCount, ( ) const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ActorNode", "const AnimationSetHandle & getAnimationSet( uint32 ) const", asMETHODPR(cgActorNode, getAnimationSet, ( cgUInt32 ) const, const cgAnimationSetHandle& ), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Actor