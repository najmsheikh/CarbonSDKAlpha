#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Audio/cgAudioTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Audio {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Audio.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "AudioBufferFlags" ) );

            // Structures
            BINDSUCCESS( engine->registerObjectType( "AudioBufferFormat", sizeof(cgAudioBufferFormat), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAudioBufferFlags (Enum).
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "AudioBufferFlags";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "Simple"            , cgAudioBufferFlags::Simple ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Streaming"         , cgAudioBufferFlags::Streaming ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Positional"        , cgAudioBufferFlags::Positional ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AllowPan"          , cgAudioBufferFlags::AllowPan ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AllowVolume"       , cgAudioBufferFlags::AllowVolume ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AllowPitch"        , cgAudioBufferFlags::AllowPitch ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MuteAtMaxDistance" , cgAudioBufferFlags::MuteAtMaxDistance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Complex"           , cgAudioBufferFlags::Complex ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Complex3D"         , cgAudioBufferFlags::Complex3D ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAudioBufferFormat (Enum).
            ///////////////////////////////////////////////////////////////////////

            typeName = "AudioBufferFormat";

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint16 formatType"           , offsetof(cgAudioBufferFormat,formatType) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint16 channels"             , offsetof(cgAudioBufferFormat,channels) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint32 samplesPerSecond"     , offsetof(cgAudioBufferFormat,samplesPerSecond) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint32 averageBytesPerSecond", offsetof(cgAudioBufferFormat,averageBytesPerSecond) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint16 blockAlign"           , offsetof(cgAudioBufferFormat,blockAlign) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint16 bitsPerSample"        , offsetof(cgAudioBufferFormat,bitsPerSample) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint16 size"                 , offsetof(cgAudioBufferFormat,size) ) );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Audio::Types