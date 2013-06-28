//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : cgAudioDriver.h                                                    //
//                                                                           //
// Desc : The main device through which we may load / play sound effects and //
//        music.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAUDIODRIVER_H_ )
#define _CGE_CGAUDIODRIVER_H_

//-----------------------------------------------------------------------------
// cgAudioDriver Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Audio/cgAudioTypes.h>
#include <System/cgReference.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgAppWindow;
class cgThread;
class cgCriticalSection;
class cgVector3;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {CBC8F845-14CD-41B2-B421-4C11CA370EF1}
const cgUID RTID_AudioDriver = {0xCBC8F845, 0x14CD, 0x41B2, {0xB4, 0x21, 0x4C, 0x11, 0xCA, 0x37, 0xE, 0xF1}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAudioCodec (Class)
/// <summary>
/// Interface class provided for implementing support for additional
/// audio formats.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAudioCodec
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerations for This Class
    //-------------------------------------------------------------------------
    enum Errors
    {
        ReadError_Retry = -1,
        ReadError_Abort = -2,

        Force_32bit     = 0x7FFFFFFF
    };

    //-------------------------------------------------------------------------
    // Public Virtual Functions for This Class
    //-------------------------------------------------------------------------
    virtual bool    isValid     ( cgInputStream & stream ) = 0;
    virtual bool    open        ( cgInputStream & stream ) = 0;
    virtual bool    reset       ( ) = 0;
    virtual void    close       ( ) = 0;
    virtual bool    getPCMFormat( cgAudioBufferFormat & format ) = 0;
    virtual cgInt32 decodePCM   ( cgByte * buffer, cgUInt32 bufferLength ) = 0;
};


//-----------------------------------------------------------------------------
//  Name : cgAudioDriver (Class)
/// <summary>
/// Device through which sound and music is played and managed.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAudioDriver : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAudioDriver, cgReference, "AudioDriver" )

    //-------------------------------------------------------------------------
    // Friend declarations
    //-------------------------------------------------------------------------
    friend class cgResourceManager;
    friend class cgAudioBuffer;

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum AudioCodecs { AudioCodec_Ogg = 0, AudioCodec_Wav = 1, AudioCodec_Count };
    
    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct InitConfig           // The selected audio driver configuration options
    {
        cgUInt32 sampleRate;
        cgUInt16 channels;
        cgUInt16 bitRate;
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAudioDriver( );
    virtual ~cgAudioDriver( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgAudioDriver  * getInstance             ( );
    static cgAudioDriver  * createInstance          ( );
    static void             createSingleton         ( );
    static void             destroySingleton        ( );
    static void             PCM16StereoToMono       ( cgInt16 destSamples[], cgInt16 srcSamples[], cgUInt32 sampleCount, cgDouble gain = -6.0206f );
    static void             PCM8StereoToMono        ( cgByte destSamples[], cgByte srcSamples[], cgUInt32 sampleCount, cgDouble gain = -6.0206f );
    
    // Audio codecs
    static cgAudioCodec   * createAudioCodec        ( cgUInt32 index );
    static void             releaseAudioCodec       ( cgAudioCodec * codec );

    //-------------------------------------------------------------------------
    // Public Pure Virtual Methods
    //-------------------------------------------------------------------------
    // Configuration
    virtual cgConfigResult::Base    loadConfig              ( const cgString & fileName ) = 0;
    virtual cgConfigResult::Base    loadDefaultConfig       ( ) = 0;
    virtual bool                    saveConfig              ( const cgString & fileName ) = 0;

    // 3D Audio
    virtual void                    set3DWorldScale         ( cgFloat unitsPerMeter ) = 0;
    virtual void                    set3DRolloffFactor      ( cgFloat factor ) = 0;
    virtual void                    set3DListenerTransform  ( const cgTransform & t ) = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            initialize                  ( cgResourceManager * resourceManager, cgAppWindow * focusWindow );
    virtual void            releaseOwnedResources       ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    InitConfig              getConfig                   ( ) const;
    
    // Ambient Tracks
    bool                    loadAmbientTrack            ( const cgString & trackName, cgInputStream stream, cgFloat initialVolume = 1.0f, cgFloat requestedVolume = 1.0f );
    bool                    loadAmbientTrack            ( const cgString & trackName, cgInputStream stream, cgFloat initialVolume, cgFloat requestedVolume, bool loop );
    bool                    stopAmbientTrack            ( const cgString & trackname );
    bool                    setAmbientTrackPitch        ( const cgString & trackname, cgFloat pitch );
    bool                    setAmbientTrackVolume       ( const cgString & trackname, cgFloat volume );
    void                    stopAmbientTracks           ( );
    void                    pauseAmbientTracks          ( );
    void                    resumeAmbientTracks         ( );
    bool                    isAmbientTrackPlaying       ( const cgString & trackName );
    void                    setTrackFadeTimes           ( cgFloat fadeOutTime, cgFloat fadeInTime );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_AudioDriver; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;
    virtual bool            processMessage          ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Enumerations
    //-------------------------------------------------------------------------
    enum FadeState { Fade_None = 0, Fade_Out = 1, Fade_In = 2 };

    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct AmbientItem
    {
        cgAudioBufferHandle buffer;
        FadeState           state;
        cgFloat             requestedVolume;
        bool                looping;
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE         (cgAudioBuffer*, BufferList)
    CGE_LIST_DECLARE         (AmbientItem*, AmbientBufferList)
    CGE_UNORDEREDMAP_DECLARE (cgString, AmbientBufferList, AmbientTrackMap )
    
    //-------------------------------------------------------------------------
    // Protected Pure Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            apply3DSettings         ( ) = 0;

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    addStreamBuffer         ( cgAudioBuffer * buffer );
    bool                    removeStreamBuffer      ( cgAudioBuffer * buffer );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    InitConfig              mConfig;            // Audio configuration settings
    bool                    mConfigLoaded;      // Has the configuration been loaded yet?
    cgResourceManager     * mResourceManager;   // The manager that is in charge of resources tied to this device / audio driver
    AmbientTrackMap         mAmbientTracks;     // Tracks for ambient playback
    cgFloat                 mFadeOutTime;       // Amount of time it takes to fade a track out
    cgFloat                 mFadeInTime;        // Amount of time it takes to fade a track in
    cgFloat                 m3DUpdateDelay;     // Amount of time in seconds to delay each 3D update pass
    BufferList              mStreamingBuffers;  // List of streaming buffers that need updating every once in a while
    cgThread              * mUpdateThread;      // Main thread manager object for the audio update thread.
    cgCriticalSection     * mAmbientSection;    // Critical section for ambient track data
    cgCriticalSection     * mStreamingSection;  // Critical section for streaming sound data

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static cgUInt32         updateAudioThread       ( cgThread * thread, void * context );

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgAudioDriver  * mSingleton;         // Static singleton object instance.
};

#endif // !_CGE_CGAUDIODRIVER_H_