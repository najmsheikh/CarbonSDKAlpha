#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgAudioBuffer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace AudioBuffer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.AudioBuffer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AudioBuffer", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "AudioBufferHandle", sizeof(cgAudioBufferHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAudioBuffer>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgAudioBuffer>( engine, "AudioBuffer" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool supportsMode( AudioBufferFlags ) const", asMETHODPR(cgAudioBuffer,supportsMode, (cgAudioBufferFlags::Base) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "uint getBufferSize( ) const", asMETHODPR(cgAudioBuffer,getBufferSize, () const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "uint getBufferPosition( ) const", asMETHODPR(cgAudioBuffer,getBufferPosition, () const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool setBufferPosition( uint )", asMETHODPR(cgAudioBuffer,setBufferPosition, (cgUInt32), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "const AudioBufferFormat & getBufferFormat( ) const", asMETHODPR(cgAudioBuffer,getBufferFormat, () const, const cgAudioBufferFormat & ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "uint getCreationFlags( ) const", asMETHODPR(cgAudioBuffer,getCreationFlags, () const, cgUInt32 ), asCALL_THISCALL) );
            //ToDo:BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "SoundInputSource & getInputSource( )", asMETHODPR(cgAudioBuffer,getInputSource, (), cgAudioBuffer::InputSource& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool isPlaying( ) const", asMETHODPR(cgAudioBuffer,isPlaying, () const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool play( bool )", asMETHODPR(cgAudioBuffer,play, (bool), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool stop( )", asMETHODPR(cgAudioBuffer, stop, (), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool pause( )", asMETHODPR(cgAudioBuffer, pause, (), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool resume( )", asMETHODPR(cgAudioBuffer, resume, (), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool setVolume( float )", asMETHODPR(cgAudioBuffer, setVolume, (cgFloat), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "float getVolume( ) const", asMETHODPR(cgAudioBuffer, getVolume, () const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool setPan( float )", asMETHODPR(cgAudioBuffer, setPan, (cgFloat), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "float getPan( ) const", asMETHODPR(cgAudioBuffer, getPan, () const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "bool setPitch( float )", asMETHODPR(cgAudioBuffer, setPitch, (cgFloat), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "float getPitch( ) const", asMETHODPR(cgAudioBuffer, getPitch, () const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "void set3DSoundPosition( const Vector3 &in )", asMETHODPR(cgAudioBuffer, set3DSoundPosition, ( const cgVector3 & ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "void set3DSoundVelocity( const Vector3 &in )", asMETHODPR(cgAudioBuffer, set3DSoundVelocity, ( const cgVector3 & ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioBuffer", "void set3DRangeProperties( float,float )", asMETHODPR(cgAudioBuffer, set3DRangeProperties, ( cgFloat, cgFloat ), void ), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgAudioBufferHandle>( engine, "AudioBufferHandle", "AudioBuffer" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::AudioBuffer