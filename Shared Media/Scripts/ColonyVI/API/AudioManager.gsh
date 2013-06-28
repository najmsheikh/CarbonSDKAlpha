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
// Name : AudioManager.gsh                                                   //
//                                                                           //
// Desc : Class that is responsible for managing the game's audio resources. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Local Includes
//-----------------------------------------------------------------------------
#include_once "../States/GamePlay.gs"

//-----------------------------------------------------------------------------
// Class Definitions
//-----------------------------------------------------------------------------
shared class AudioChannel
{
    int                 instanceId;
    AudioBufferHandle   resource;
    AudioBuffer@        buffer;
    bool                loop;
    bool                positional;
    float               pitch;
    float               volume;
    Vector3             offset;
    ObjectNode@         relativeNode;
}

shared class AudioBufferCache
{
    String              file;
    bool                positional;
    AudioBufferHandle   resource;
}

shared class SoundRef
{
    int channel;
    int instanceId;

    SoundRef( int _channel, int _instanceId )
    {
        channel = _channel;
        instanceId = _instanceId;
    }
}

shared class AmbientTrackChannel
{
    float                   volume;
    float                   pitch;
    bool                    ignoreVolumeScale;
    bool                    ignorePitchScale;
    String                  name;
    AmbientTrackChannel@    next;
    AmbientTrackChannel@    previous;
};

//-----------------------------------------------------------------------------
// Name : AudioManager (Class)
// Desc : Class that is responsible for managing the game's audio resources.
//-----------------------------------------------------------------------------
shared class AudioManager
{
    ///////////////////////////////////////////////////////////////////////////
	// Private Member Variables
	///////////////////////////////////////////////////////////////////////////
    private AudioDriver@                mDriver;
    private ResourceManager@            mResources;
    private array<AudioChannel@>        mChannels;
    private array<AudioBufferCache@>    mCache;
    private AmbientTrackChannel@        mAmbientTracks;
    private int                         mNextInstanceId;
    private float                       mPitchScale;
    private float                       mVolumeScale;

    ///////////////////////////////////////////////////////////////////////////
	// Constructors & Destructors
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
	// Name : AudioManager () (Constructor)
	// Desc : Constructor for this class.
	//-------------------------------------------------------------------------
    AudioManager( int maxChannels )
    {
        @mDriver        = getAppAudioDriver();
        @mResources     = getAppResourceManager();
        @mAmbientTracks = null;
        mPitchScale     = 1.0f;
        mVolumeScale    = 1.0f;
        mNextInstanceId = 1;

        // Allocate required number of channels.
        mChannels.resize( maxChannels );
        for ( int i = 0; i < maxChannels; ++i )
            @mChannels[i] = AudioChannel();

        // Long range sounds please!
        mDriver.set3DRolloffFactor( 0.15f );
        //mDriver.set3DRolloffFactor( 0.25f );
        //mDriver.set3DRolloffFactor( 0.75f );

        // Short (ish) ambient track cross-fades
        mDriver.setTrackFadeTimes( 2.0f, 2.0f );
    }

    ///////////////////////////////////////////////////////////////////////////
	// Public Methods
	///////////////////////////////////////////////////////////////////////////
    //-------------------------------------------------------------------------
    // Name : shutdown ()
    // Desc : Shut down the manager and clean up resources.
    //-------------------------------------------------------------------------
    void shutdown( )
    {
        // Stop all sounds
        stopSounds();

        // Stop all ambient tracks.
        mDriver.stopAmbientTracks();
        @mAmbientTracks = null;

        // Clear cache
        mCache.resize(0);
    }

    //-------------------------------------------------------------------------
    // Name : process ()
    // Desc : Allow the audio manager to perform runtime processing.
    //-------------------------------------------------------------------------
    void process( float elapsedTime )
    {
        // Process active audio channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            AudioChannel @ channel = mChannels[i];
            if ( @channel.buffer == null )
                continue;

            // Unload buffer when it finishes playing?
            if ( !channel.loop && !channel.buffer.isPlaying() )
            {
                stopSound( i );
                continue;
            
            } // End if finished

            // Auto update positional buffers in case the node moved.
            if ( channel.positional && @channel.relativeNode != null )
            {
                Vector3 pos = channel.offset;
                pos += channel.relativeNode.getPosition();
                channel.buffer.set3DSoundPosition( pos );

            } // End if positional
            
        } // Next channel

        // Remove any inactivate ambient tracks.
        AmbientTrackChannel @ channel = mAmbientTracks;
        while ( @channel != null )
        {
            // Remove any ambient track that is no longer playing.
            if ( !mDriver.isAmbientTrackPlaying( channel.name ) )
            {
                if ( @channel.next != null )
                    @channel.next.previous = channel.previous;
                if ( @channel.previous != null )
                    @channel.previous.next = channel.next;
                if ( @channel == @mAmbientTracks )
                    @mAmbientTracks = channel.next;
            
            } // End if stopped

            // Move on to next channel.
            @channel = channel.next;

        } // Next channel
    }

    void pauseSounds( )
    {
        // Process active audio channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            AudioChannel @ channel = mChannels[i];
            if ( @channel.buffer == null )
                continue;

            // Stop playback.
            channel.buffer.stop();
            
        } // Next channel

        // Pause ambient tracks.
        mDriver.pauseAmbientTracks();
    }

    void resumeSounds( )
    {
        // Process active audio channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            AudioChannel @ channel = mChannels[i];
            if ( @channel.buffer == null )
                continue;

            // Resume playback.
            channel.buffer.resume();
            
        } // Next channel

        // Resume ambient tracks.
        mDriver.resumeAmbientTracks();
    }

    int loadSound( const String & file, bool positional )
    {
        // Audio buffer already exists?
        for ( int i = 0; i < mCache.length(); ++i )
        {
            AudioBufferCache @ item = mCache[i];
            if ( item.file == file && item.positional == positional )
                return i;
        
        } // Next item

        // Build a new cache item
        AudioBufferCache @ item = AudioBufferCache();
        item.file = file;
        item.positional = positional;

        // Load the resource
        if ( !mResources.loadAudioBuffer( item.resource, file, (positional) ? AudioBufferFlags::Complex3D : AudioBufferFlags::Complex, 0, DebugSource() ) )
            return -1;

        // Add to cache list
        int cacheIndex = mCache.length();
        mCache.resize( cacheIndex + 1 );
        @mCache[cacheIndex] = item;
        
        // Loaded
        return cacheIndex;
    }

    SoundRef@ playSound( const String & file, bool duplicate, bool loop, float volume, float pan = 0.0f, float pitch = 1.0f )
    {
        // Find a free audio channel.
        int channelIndex = getFreeAudioChannel();
        if ( channelIndex < 0 )
            return null;

        // If we're not duplicating the buffer (i.e., simply re-using an
        // already resident buffer) attempt to retrieve it.
        AudioChannel @ channel = mChannels[channelIndex];
        if ( !duplicate )
        {
            // Get the existing resource if it already exists?
            mResources.getAudioBuffer( channel.resource, file, AudioBufferFlags::Complex );

        } // End if !duplicate

        // If there is no resource (because we're not duplicating, or
        // no existing buffer was found), load the buffer.
        if ( !channel.resource.isValid() )
        {
            if ( !mResources.loadAudioBuffer( channel.resource, file, AudioBufferFlags::Complex, 0, DebugSource() ) )
                return null;
        
        } // End if no resource

        // Cache the underlying resource to make things easy.
        @channel.buffer = channel.resource.getResource(true);
        if ( @channel.buffer == null || !channel.buffer.isLoaded() )
        {
            @channel.buffer = null;
            channel.resource.close();
            return null;
        
        } // End if failed

        // Set up remaining channel data.
        channel.instanceId = mNextInstanceId++;
        channel.positional = false;
        channel.loop = loop;
        channel.pitch = pitch;
        channel.volume = volume;

        // Set volume and begin playing
        channel.buffer.setPitch( pitch * mPitchScale );
        channel.buffer.setPan( pan );
        channel.buffer.setVolume( volume * mVolumeScale );
        channel.buffer.setBufferPosition( 0 );
        channel.buffer.play( loop );

        // We're playing!
        return SoundRef( channelIndex, channel.instanceId );
    }

    SoundRef @ playSound( const String & file, bool duplicate, bool loop, float volume, const Vector3 & offset, ObjectNode @ relativeNode )
    {
        // Find a free audio channel.
        int channelIndex = getFreeAudioChannel();
        if ( channelIndex < 0 )
            return null;

        // If we're not duplicating the buffer (i.e., simply re-using an
        // already resident buffer) attempt to retrieve it.
        AudioChannel @ channel = mChannels[channelIndex];
        if ( !duplicate )
        {
            // Get the existing resource if it already exists?
            mResources.getAudioBuffer( channel.resource, file, AudioBufferFlags::Complex3D );

        } // End if !duplicate

        // If there is no resource (because we're not duplicating, or
        // no existing buffer was found), load the buffer.
        if ( !channel.resource.isValid() )
        {
            if ( !mResources.loadAudioBuffer( channel.resource, file, AudioBufferFlags::Complex3D, 0, DebugSource() ) )
                return null;
        
        } // End if no resource

        // Cache the underlying resource to make things easy.
        @channel.buffer = channel.resource.getResource(true);
        if ( @channel.buffer == null || !channel.buffer.isLoaded() )
        {
            @channel.buffer = null;
            channel.resource.close();
            return null;
        
        } // End if failed

        // Set up remaining channel data.
        channel.instanceId = mNextInstanceId++;
        channel.positional = true;
        channel.loop = loop;
        channel.pitch = 1.0f;
        channel.volume = volume;
        channel.offset = offset;
        @channel.relativeNode = relativeNode;

        // Set initial 3D position.
        Vector3 pos = offset;
        if ( @relativeNode != null )
            pos += relativeNode.getPosition();
        channel.buffer.set3DSoundPosition( pos );

        // Set volume and begin playing
        channel.buffer.setPitch( mPitchScale );
        channel.buffer.setVolume( volume * mVolumeScale );
        channel.buffer.setBufferPosition( 0 );
        channel.buffer.play( loop );

        // We're playing!
        return SoundRef( channelIndex, channel.instanceId );
    }

    SoundRef @ playSound( int cacheHandle, bool duplicate, bool loop, float volume, float pan = 0.0f, float pitch = 1.0f )
    {
        // Valid cache handle?
        if ( cacheHandle < 0 )
            return null;

        // Get cache entry
        AudioBufferCache @ item = mCache[cacheHandle];
        if ( !item.resource.isValid() || item.positional ) // Must not be positional
            return null;

        // Find a free audio channel.
        int channelIndex = getFreeAudioChannel();
        if ( channelIndex < 0 )
            return null;

        // If we're not duplicating the buffer (i.e., simply re-using an
        // already resident buffer) re-use it.
        AudioChannel @ channel = mChannels[channelIndex];
        if ( !duplicate )
            channel.resource = item.resource;
        else
        {
            // Duplicate the buffer
            if ( !mResources.loadAudioBuffer( channel.resource, item.file, AudioBufferFlags::Complex, 0, DebugSource() ) )
                return null;
        
        } // End if duplicating

        // Cache the underlying resource to make things easy.
        @channel.buffer = channel.resource.getResource(true);
        if ( @channel.buffer == null || !channel.buffer.isLoaded() )
        {
            @channel.buffer = null;
            channel.resource.close();
            return null;
        
        } // End if failed

        // Set up remaining channel data.
        channel.instanceId = mNextInstanceId++;
        channel.positional = false;
        channel.loop = loop;
        channel.pitch = pitch;
        channel.volume = volume;

        // Set volume and begin playing
        channel.buffer.setPan( pan );
        channel.buffer.setPitch( pitch * mPitchScale );
        channel.buffer.setVolume( volume * mVolumeScale );
        channel.buffer.setBufferPosition( 0 );
        channel.buffer.play( loop );

        // We're playing!
        return SoundRef( channelIndex, channel.instanceId );
    }

    SoundRef @ playSound( int cacheHandle, bool duplicate, bool loop, float volume, const Vector3 & offset, ObjectNode @ relativeNode )
    {
        // Valid cache handle?
        if ( cacheHandle < 0 )
            return null;

        // Get cache entry
        AudioBufferCache @ item = mCache[cacheHandle];
        if ( !item.resource.isValid() || !item.positional ) // Must be positional
            return null;

        // Find a free audio channel.
        int channelIndex = getFreeAudioChannel();
        if ( channelIndex < 0 )
            return null;

        // If we're not duplicating the buffer (i.e., simply re-using an
        // already resident buffer) re-use it.
        AudioChannel @ channel = mChannels[channelIndex];
        if ( !duplicate )
            channel.resource = item.resource;
        else
        {
            // Duplicate the buffer
            if ( !mResources.loadAudioBuffer( channel.resource, item.file, AudioBufferFlags::Complex3D, 0, DebugSource() ) )
                return null;
        
        } // End if duplicating


        // Cache the underlying resource to make things easy.
        @channel.buffer = channel.resource.getResource(true);
        if ( @channel.buffer == null || !channel.buffer.isLoaded() )
        {
            @channel.buffer = null;
            channel.resource.close();
            return null;
        
        } // End if failed

        // Set up remaining channel data.
        channel.instanceId = mNextInstanceId++;
        channel.positional = true;
        channel.loop = loop;
        channel.pitch = 1.0f;
        channel.volume = volume;
        channel.offset = offset;
        @channel.relativeNode = relativeNode;

        // Set initial 3D position.
        Vector3 pos = offset;
        if ( @relativeNode != null )
            pos += relativeNode.getPosition();
        channel.buffer.set3DSoundPosition( pos );

        // Set volume and begin playing
        channel.buffer.setPitch( mPitchScale );
        channel.buffer.setVolume( volume * mVolumeScale );
        channel.buffer.setBufferPosition( 0 );
        channel.buffer.play( loop );

        // We're playing!
        return SoundRef( channelIndex, channel.instanceId );
    }

    bool isAmbientTrackPlaying( const String & ambientChannel )
    {
        return mDriver.isAmbientTrackPlaying( ambientChannel );
    }

    void playAmbientTrack( const String & ambientChannel, const String & file, float initialVolume, float desiredVolume, bool loop, bool ignoreVolumeScale = false, bool ignorePitchScale = false )
    {
        float scale = (ignoreVolumeScale) ? 1.0f : mVolumeScale;
        mDriver.loadAmbientTrack( ambientChannel, file, initialVolume * scale, desiredVolume * scale, loop );
        if ( !ignorePitchScale )
            mDriver.setAmbientTrackPitch( ambientChannel, mPitchScale );

        // Add to our ambient track list if it doesn't already exist.
        AmbientTrackChannel @ channel = mAmbientTracks;
        while ( @channel != null )
        {
            if ( channel.name == ambientChannel )
                return;
            @channel = channel.next;
        
        } // Next channel

        // Channel was not found, attach it to the front of the list.
        @channel            = AmbientTrackChannel();
        channel.name        = ambientChannel;
        channel.volume      = desiredVolume;
        channel.pitch       = 1.0f;
        channel.ignoreVolumeScale = ignoreVolumeScale;
        channel.ignorePitchScale  = ignorePitchScale;
        @channel.previous   = null;
        @channel.next       = mAmbientTracks;
        if ( @channel.next != null )
            @channel.next.previous = channel;
        @mAmbientTracks = channel;
    }

    float getPitchScale( )
    {
        return mPitchScale;
    }

    float getVolumeScale( )
    {
        return mVolumeScale;
    }

    void setPitchScale( float scale )
    {
        mPitchScale = scale;

        // Adjust the current pitch of all channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            AudioChannel @ channel = mChannels[i];
            if ( @channel.buffer == null )
                continue;

            // Reset the pitch
            channel.buffer.setPitch( channel.pitch * mPitchScale );
        
        } // Next channel

        // Adjust the pitch of ambient tracks that allow it.
        AmbientTrackChannel @ channel = mAmbientTracks;
        while ( @channel != null )
        {
            if ( !channel.ignorePitchScale )
                mDriver.setAmbientTrackPitch( channel.name, channel.pitch * mPitchScale );
            @channel = channel.next;

        } // Next track
    }

    void setAmbientTrackVolume( const String & ambientChannel, float volume, bool overrideScale = false )
    {
        // Fidn the track and update its volume.
        AmbientTrackChannel @ channel = mAmbientTracks;
        while ( @channel != null )
        {
            if ( channel.name == ambientChannel )
            {
                channel.volume = volume;
                float scale = ( channel.ignoreVolumeScale || overrideScale ) ? 1.0f : mVolumeScale;
                mDriver.setAmbientTrackVolume( channel.name, channel.volume * scale );
                return;
            
            } // End if matching name
            @channel = channel.next;

        } // Next track
    }

    void setVolumeScale( float scale )
    {
        mVolumeScale = scale;

        // Adjust the current volume of all channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            AudioChannel @ channel = mChannels[i];
            if ( @channel.buffer == null )
                continue;

            // Reset the volume
            channel.buffer.setVolume( channel.volume * mVolumeScale );
        
        } // Next channel

        // Adjust the volume of ambient tracks that allow it.
        AmbientTrackChannel @ channel = mAmbientTracks;
        while ( @channel != null )
        {
            if ( !channel.ignoreVolumeScale )
                mDriver.setAmbientTrackVolume( channel.name, channel.volume * mVolumeScale );
            @channel = channel.next;

        } // Next track
    }

    void stopSounds()
    {
         // Stop everything!
        for ( int i = 0; i < mChannels.length(); ++i )
            stopSound( i );
    }

    void stopSound( int channelIndex )
    {
        if ( channelIndex < 0 || @mChannels[channelIndex].buffer == null )
            return;

        // Stop the sound and unload.
        AudioChannel @ channel = mChannels[channelIndex];
        channel.buffer.stop();
        channel.instanceId = 0;
        @channel.buffer = null;
        channel.resource.close();
        @channel.relativeNode = null;
    }

    void stopSound( SoundRef @ reference )
    {
        if ( @reference == null || reference.channel < 0 )
            return;
        
        // Must be referencing the same, valid buffer.
        AudioChannel @ channel = mChannels[reference.channel];
        if ( @channel.buffer == null || 
             channel.instanceId != reference.instanceId )
            return;

        // Stop the sound and unload.
        channel.buffer.stop();
        channel.instanceId = 0;
        @channel.buffer = null;
        channel.resource.close();
        @channel.relativeNode = null;
    }

    void setSoundVolume( SoundRef @ reference, float volume )
    {
        if ( @reference == null || reference.channel < 0 )
            return;
        
        // Must be referencing the same, valid buffer.
        AudioChannel @ channel = mChannels[reference.channel];
        if ( @channel.buffer == null || 
             channel.instanceId != reference.instanceId )
            return;

        // Adjust the current 'requested' volume record for the channel.
        channel.volume = volume;

        // Set the buffer volume
        channel.buffer.setVolume( volume * mVolumeScale );
    }

    void setSoundPitch( SoundRef @ reference, float pitch )
    {
        if ( @reference == null || reference.channel < 0 )
            return;
        
        // Must be referencing the same, valid buffer.
        AudioChannel @ channel = mChannels[reference.channel];
        if ( @channel.buffer == null || 
             channel.instanceId != reference.instanceId )
            return;

        // Adjust the current 'requested' pitch record for the channel.
        channel.pitch = pitch;

        // Set the new buffer pitch
        channel.buffer.setPitch( pitch * mPitchScale );
    }

    void setSoundPan( SoundRef @ reference, float pan )
    {
        if ( @reference == null || reference.channel < 0 )
            return;
        
        // Must be referencing the same, valid buffer.
        AudioChannel @ channel = mChannels[reference.channel];
        if ( @channel.buffer == null || 
             channel.instanceId != reference.instanceId )
            return;

        // Set the buffer volume
        channel.buffer.setPan( pan );
    }

    void stopAmbientTrack( const String & ambientChannel )
    {
        mDriver.stopAmbientTrack( ambientChannel );
    }

    bool isChannelPlaying( int channelIndex )
    {
        if ( channelIndex < 0 || @mChannels[channelIndex].buffer == null )
            return false;
        return mChannels[channelIndex].buffer.isPlaying();
    }

    bool isSoundPlaying( SoundRef @ reference )
    {
        if ( @reference == null || reference.channel < 0 )
            return false;
        
        // Must be referencing the same, valid buffer.
        AudioChannel @ channel = mChannels[reference.channel];
        if ( @channel.buffer == null || 
             channel.instanceId != reference.instanceId )
            return false;

        return channel.buffer.isPlaying();
    }

    ///////////////////////////////////////////////////////////////////////////
	// Private Methods
	///////////////////////////////////////////////////////////////////////////
    private int getFreeAudioChannel()
    {
        // Process active audio channels.
        for ( int i = 0; i < mChannels.length(); ++i )
        {
            if ( @mChannels[i].buffer == null )
                return i;
   
        } // Next channel

        // No free channels.
        return -1;
    }
    
};

shared AudioManager @ getAudioManager( )
{
    AppStateManager @ stateManager = getAppStateManager();
    AppState @ state = stateManager.getState( "GamePlay" );
    if ( @state != null )
    {
        GamePlay @ gamePlay = cast<GamePlay>( state.getScriptObject() );
        if ( @gamePlay != null )
            return gamePlay.getAudioManager();

    } // End if valid state
    return null;
}