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
// Name : cgAudioBuffer.h                                                    //
//                                                                           //
// Desc : Contains classes responsible for loading and managing audio        //
//        buffer / sound effect resource data.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAUDIOBUFFER_H_ )
#define _CGE_CGAUDIOBUFFER_H_

//-----------------------------------------------------------------------------
// cgAudioBuffer Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Audio/cgAudioTypes.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResource.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgAudioDriver;
class cgAudioCodec;
struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;
class cgVector3;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {474684C4-5DEE-4FB4-B535-A99C43A38ACF}
const cgUID RTID_AudioBufferResource = {0x474684C4, 0x5DEE, 0x4FB4, {0xB5, 0x35, 0xA9, 0x9C, 0x43, 0xA3, 0x8A, 0xCF}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAudioBuffer (Class)
/// <summary>
/// Actual audio buffer resource object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAudioBuffer : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAudioBuffer, cgResource, "AudioBuffer" )

public:
    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct InputSource
    {
        cgInputStream       stream;     // The stream from which data is being loaded / streamed.
        cgUInt32            codecId;    // The identifier of the codec used to load the data.
        cgAudioBufferFormat format;     // PCM format of data in the source file.
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAudioBuffer( cgUInt32 referenceId, cgAudioDriver * driver );
             cgAudioBuffer( cgUInt32 referenceId, cgAudioDriver * driver, const cgInputStream & stream, cgUInt32 flags );
             cgAudioBuffer( cgUInt32 referenceId, cgAudioDriver * driver, cgAudioBuffer * sourceBuffer );
    virtual ~cgAudioBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        load                    ( cgInputStream stream, cgUInt32 flags );
    bool                        duplicate               ( cgAudioBuffer * buffer );
    bool                        supportsMode            ( cgAudioBufferFlags::Base mode ) const;
    cgUInt32                    getBufferSize           ( ) const;
    cgUInt32                    getBufferPosition       ( ) const;
    bool                        setBufferPosition       ( cgUInt32 position );
    const cgAudioBufferFormat & getBufferFormat         ( ) const;
    cgUInt32                    getCreationFlags        ( ) const;
    InputSource               & getInputSource          ( );
    
    // Playback
    bool                        isPlaying               ( ) const;
    bool                        play                    ( bool loop = false );
    bool                        stop                    ( );

    // Audio Control
    bool                        setVolume               ( cgFloat volume );
    cgFloat                     getVolume               ( ) const;

    // Streaming support
    bool                        checkStreamUpdate       ( );

    // 3D Positional Support
    void                        set3DSoundPosition      ( const cgVector3 & position );
    void                        set3DSoundVelocity      ( const cgVector3 & velocity );
    void                        set3DRangeProperties    ( cgFloat minimumDistance, cgFloat maximumDistance );

    // Internal Methods
    IDirectSoundBuffer        * getInternalBuffer       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool                loadResource            ( );
    virtual bool                unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_AudioBufferResource; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                        decodeFullAudioStream   ( cgAudioCodec * codec, cgAudioBufferFormat & format, cgUInt32 flags, cgByte ** audioData, cgUInt32 * audioSize );
    bool                        createAudioBuffer       ( cgUInt32 flags, cgAudioBufferFormat & format, cgUInt32 bufferSize, cgByte * initData = CG_NULL, cgUInt32 initDataSize = 0 );
    bool                        populateAudioBuffer     ( );
    bool                        restoreAudioBuffer      ( bool & wasRestored );
    IDirectSound3DBuffer      * get3DSoundInterface     ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgInputStream           mInputStream;           // The source stream from which the sound effect data will be loaded.
    cgUInt32                mInputFlags;            // Flags to use during resource loading.

    cgAudioDriver         * mAudioDriver;           // Parent audio driver through which we will play
    IDirectSoundBuffer    * mBuffer;                // The DirectSound buffer containing the audio data
    IDirectSound3DBuffer  * m3DBuffer;              // Cached pointer to the 3D buffer interface.
    cgUInt32                mBufferSize;            // Size of the DirectSound buffer in bytes.
    cgUInt32                mCreationFlags;         // Flags used to create the 
    cgAudioCodec          * mCodec;                 // The codec being used to load the data.
    InputSource             mInputSource;           // Information about the source file / memory buffer used to load the audio data
    cgAudioBufferFormat     mBufferFormat;          // The format information for the data in the buffer
    bool                    mLooping;               // Is this buffer looping

    // Stream Handling
    cgFloat                 mStreamBufferLength;    // Number of seconds of available audio data to buffer whilst streaming
    cgUInt32                mNotifyCount;           // Number of notification events we want to receive during the buffer period
    cgUInt32                mNotifySize;            // Calculated at creation time... Total number of bytes in each notification block.
    void                  * mNotifyEvent;           // The event that is signaled when streaming data is available for us.
    cgUInt32                mNextWriteOffset;       // Position within the buffer that we are currently writing to
    bool                    mPlaybackComplete;      // Playback of the streamed file is complete.
    cgUInt32                mFinalWriteOffset;      // The offset to the final piece of data that was written if mPlaybackComplete == true;
	cgInt64					mStreamDataPlayed;	    // Amount of data played for this stream so far.
	cgInt64					mStreamDataWritten;	    // Amount of data written to the stream so far.
	cgUInt32                mLastStreamPosition;	// Temporary member used for storing the previous position within the stream buffer.

    // 3D / Positional Parameters
    cgFloat                 m3DUpdateDelay;         // Amount of time in seconds to delay each 3D update pass
};

#endif // !_CGE_CGAUDIOBUFFER_H_