#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgParticleEmitterObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace ParticleEmitter
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.ParticleEmitter" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ParticleEmitterObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "ParticleEmitterNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgParticleEmitterObject>( engine );
            registerHandleBehaviors<cgParticleEmitterNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_ParticleEmitterObject", (void*)&RTID_ParticleEmitterObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_ParticleEmitterNode", (void*)&RTID_ParticleEmitterNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgParticleEmitterObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::WorldObject::registerObjectMethods<cgParticleEmitterObject>( engine, "ParticleEmitterObject" );

            // Register methods.
            
            ///////////////////////////////////////////////////////////////////////
            // cgParticleEmitterNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register base class object methods
            Core::World::Objects::ObjectNode::registerNodeMethods<cgParticleEmitterNode>( engine, "ParticleEmitterNode" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setInitialEnabledState( uint, bool )", asMETHODPR(cgParticleEmitterNode, setInitialEnabledState, ( cgUInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setInnerCone( uint, float )", asMETHODPR(cgParticleEmitterNode, setInnerCone, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setOuterCone( uint, float )", asMETHODPR(cgParticleEmitterNode, setOuterCone, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setEmissionRadius( uint, float )", asMETHODPR(cgParticleEmitterNode, setEmissionRadius, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setDeadZoneRadius( uint, float )", asMETHODPR(cgParticleEmitterNode, setDeadZoneRadius, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setMaxSimultaneousParticles( uint, uint )", asMETHODPR(cgParticleEmitterNode, setMaxSimultaneousParticles, ( cgUInt32, cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setBirthFrequency( uint, float )", asMETHODPR(cgParticleEmitterNode, setBirthFrequency, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setHDRScale( uint, float )", asMETHODPR(cgParticleEmitterNode, setHDRScale, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void enableRandomizedRotation( uint, bool )", asMETHODPR(cgParticleEmitterNode, enableRandomizedRotation, ( cgUInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void enableSortedRender( uint, bool )", asMETHODPR(cgParticleEmitterNode, enableSortedRender, ( cgUInt32, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleSpeed( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleSpeed, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleSpeed( uint, const RangeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleSpeed, ( cgUInt32, const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleMass( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleMass, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleMass( uint, const RangeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleMass, ( cgUInt32, const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleAngularSpeed( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleAngularSpeed, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleAngularSpeed( uint, const RangeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleAngularSpeed, ( cgUInt32, const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleBaseScale( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleBaseScale, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleBaseScale( uint, const RangeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleBaseScale, ( cgUInt32, const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleLifetime( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleLifetime, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleLifetime( uint, const RangeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleLifetime, ( cgUInt32, const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleSize( uint, float, float )", asMETHODPR(cgParticleEmitterNode, setParticleSize, ( cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleSize( uint, const SizeF &in )", asMETHODPR(cgParticleEmitterNode, setParticleSize, ( cgUInt32, const cgSizeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void setParticleAirResistance( uint, float )", asMETHODPR(cgParticleEmitterNode, setParticleAirResistance, ( cgUInt32, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "void enableLayerEmission( uint, bool )", asMETHODPR(cgParticleEmitterNode, enableLayerEmission, ( cgUInt32, bool ), void), asCALL_THISCALL) );
            // ToDo: inline void setParticleBlendMethod( cgUInt32 layerIndex, cgParticleBlendMethod::Base value )

            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "uint getLayerCount( ) const", asMETHODPR(cgParticleEmitterNode, getLayerCount, ( ) const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "bool isLayerEmissionEnabled( uint ) const", asMETHODPR(cgParticleEmitterNode, isLayerEmissionEnabled, ( cgUInt32 ) const, bool), asCALL_THISCALL) );
            // ToDo: inline const cgParticleEmitterProperties & getLayerProperties( cgUInt32 layerIndex ) const
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "bool getInitialEnabledState( uint ) const", asMETHODPR(cgParticleEmitterNode, getInitialEnabledState, ( cgUInt32 ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getInnerCone( uint ) const", asMETHODPR(cgParticleEmitterNode, getInnerCone, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getOuterCone( uint ) const", asMETHODPR(cgParticleEmitterNode, getOuterCone, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getEmissionRadius( uint ) const", asMETHODPR(cgParticleEmitterNode, getEmissionRadius, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getDeadZoneRadius( uint ) const", asMETHODPR(cgParticleEmitterNode, getDeadZoneRadius, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "uint getMaxSimultaneousParticles( uint ) const", asMETHODPR(cgParticleEmitterNode, getMaxSimultaneousParticles, ( cgUInt32 ) const, cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getBirthFrequency( uint ) const", asMETHODPR(cgParticleEmitterNode, getBirthFrequency, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getHDRScale( uint ) const", asMETHODPR(cgParticleEmitterNode, getHDRScale, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "bool getRandomizedRotation( uint ) const", asMETHODPR(cgParticleEmitterNode, getRandomizedRotation, ( cgUInt32 ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "bool getSortedRender( uint ) const", asMETHODPR(cgParticleEmitterNode, getSortedRender, ( cgUInt32 ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const RangeF& getParticleSpeed( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleSpeed, ( cgUInt32 ) const, const cgRangeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const RangeF& getParticleMass( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleMass, ( cgUInt32 ) const, const cgRangeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const RangeF& getParticleAngularSpeed( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleAngularSpeed, ( cgUInt32 ) const, const cgRangeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const RangeF& getParticleBaseScale( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleBaseScale, ( cgUInt32 ) const, const cgRangeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const RangeF& getParticleLifetime( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleLifetime, ( cgUInt32 ) const, const cgRangeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "const SizeF& getParticleSize( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleSize, ( cgUInt32 ) const, const cgSizeF&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ParticleEmitterNode", "float getParticleAirResistance( uint ) const", asMETHODPR(cgParticleEmitterNode, getParticleAirResistance, ( cgUInt32 ) const, cgFloat), asCALL_THISCALL) );
            // ToDo: cgParticleBlendMethod::Base getParticleBlendMethod( cgUInt32 layerIndex ) const
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::ParticleEmitter