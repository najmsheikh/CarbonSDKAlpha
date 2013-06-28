#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Audio/cgAudioDriver.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Audio {

// Package declaration
namespace AudioDriver
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Audio.AudioDriver" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            //BINDSUCCESS( engine->registerObjectType( "AudioDriverConfig", sizeof(cgAudioDriver::InitConfig), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
            BINDSUCCESS( engine->registerObjectType( "AudioDriver", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            /*///////////////////////////////////////////////////////////////////////
            // cgAudioDriver::InitConfig (Struct)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "bool ignoreWindowsKey"  , offsetof(cgInputDriver::InitConfig,ignoreWindowsKey) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "float mouseSensitivity" , offsetof(cgInputDriver::InitConfig,mouseSensitivity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyRepeatDelay"    , offsetof(cgInputDriver::InitConfig,keyRepeatDelay) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyRepeatRate"     , offsetof(cgInputDriver::InitConfig,keyRepeatRate) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyboardBufferSize", offsetof(cgInputDriver::InitConfig,keyboardBufferSize) ) );*/

            ///////////////////////////////////////////////////////////////////////
            // cgAudioDriver (Class)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgAudioDriver>( engine );
            
            // Register base class object methods
            Core::System::References::Reference::registerReferenceMethods<cgAudioDriver>( engine, "AudioDriver" );
            
            // Register the object methods
            // ToDo: InitConfig              getConfig               ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void set3DWorldScale( float )", asMETHODPR(cgAudioDriver, set3DWorldScale, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void set3DRolloffFactor( float )", asMETHODPR(cgAudioDriver, set3DRolloffFactor, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void set3DListenerTransform( const Transform &in )", asMETHODPR(cgAudioDriver, set3DListenerTransform, ( const cgTransform& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool loadAmbientTrack( const String &in, InputStream, float, float )", asMETHODPR(cgAudioDriver, loadAmbientTrack, ( const cgString&, cgInputStream, cgFloat, cgFloat ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool loadAmbientTrack( const String &in, const String &in, float, float )", asFUNCTIONPR(loadAmbientTrack, ( const cgString&, const cgString&, cgFloat, cgFloat, cgAudioDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool loadAmbientTrack( const String &in, InputStream, float, float, bool )", asMETHODPR(cgAudioDriver, loadAmbientTrack, ( const cgString&, cgInputStream, cgFloat, cgFloat, bool ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool loadAmbientTrack( const String &in, const String &in, float, float, bool )", asFUNCTIONPR(loadAmbientTrack, ( const cgString&, const cgString&, cgFloat, cgFloat, bool, cgAudioDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool stopAmbientTrack( const String &in )", asMETHODPR(cgAudioDriver, stopAmbientTrack, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool setAmbientTrackPitch( const String &in, float )", asMETHODPR(cgAudioDriver, setAmbientTrackPitch, ( const cgString&, cgFloat ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool setAmbientTrackVolume( const String &in, float )", asMETHODPR(cgAudioDriver, setAmbientTrackVolume, ( const cgString&, cgFloat ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void pauseAmbientTracks( )", asMETHODPR(cgAudioDriver, pauseAmbientTracks, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void resumeAmbientTracks( )", asMETHODPR(cgAudioDriver, resumeAmbientTracks, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void stopAmbientTracks( )", asMETHODPR(cgAudioDriver, stopAmbientTracks, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "bool isAmbientTrackPlaying( const String &in )", asMETHODPR(cgAudioDriver, isAmbientTrackPlaying, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AudioDriver", "void setTrackFadeTimes( float, float )", asMETHODPR(cgAudioDriver, setTrackFadeTimes, ( cgFloat, cgFloat ), void), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "AudioDriver@+ getAppAudioDriver( )", asFUNCTIONPR(cgAudioDriver::getInstance, ( ), cgAudioDriver*), asCALL_CDECL) );
        }

		//---------------------------------------------------------------------
        //  Name : loadAmbientTrack () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// AudioDriver::loadAmbientTrack() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool loadAmbientTrack( const cgString & track, const cgString & stream, cgFloat initialVolume, cgFloat desiredVolume, cgAudioDriver * thisPointer )
        {
            return thisPointer->loadAmbientTrack( track, stream, initialVolume, desiredVolume );
        }

        //---------------------------------------------------------------------
        //  Name : loadAmbientTrack () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// AudioDriver::loadAmbientTrack() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool loadAmbientTrack( const cgString & track, const cgString & stream, cgFloat initialVolume, cgFloat desiredVolume, bool loop, cgAudioDriver * thisPointer )
        {
            return thisPointer->loadAmbientTrack( track, stream, initialVolume, desiredVolume, loop );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Audio::AudioDriver