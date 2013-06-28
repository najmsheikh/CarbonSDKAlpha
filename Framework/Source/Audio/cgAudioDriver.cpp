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
// Name : cgAudioDriver.cpp                                                  //
//                                                                           //
// Desc : The main device through which we may load / play sound effects and //
//        music.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAudioDriver Module Includes
//-----------------------------------------------------------------------------
#include <Audio/cgAudioDriver.h>
#include <Audio/Platform/cgDXAudioDriver.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgAudioBuffer.h>
#include <System/cgThreading.h>
#include <System/cgMessageTypes.h>
#include <Math/cgMathTypes.h>
#include <algorithm>
#include <math.h>

// Supported Codecs
#include <Audio/Codecs/cgAudioCodec_Ogg.h>
#include <Audio/Codecs/cgAudioCodec_Wav.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgAudioDriver * cgAudioDriver::mSingleton = CG_NULL;

// ToDo: We really need a cgAudioBuffer::GetLength() in seconds.

///////////////////////////////////////////////////////////////////////////////
// cgAudioDriver Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAudioDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver::cgAudioDriver( ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // initialize variables to sensible defaults
    mConfigLoaded     = false;
    mFadeOutTime      = 15.0f;
    mFadeInTime       = 6.0f;
    mUpdateThread     = CG_NULL;
    m3DUpdateDelay    = 0.1f; // Update our settings once every 100ms
    mResourceManager  = CG_NULL;

    // Clear structures
    memset( &mConfig, 0, sizeof(InitConfig) );

    // Create critical section for locking data
    mAmbientSection   = cgCriticalSection::createInstance();
    mStreamingSection = cgCriticalSection::createInstance();
}

//-----------------------------------------------------------------------------
//  Name : ~cgAudioDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver::~cgAudioDriver()
{
    // Release allocated memory
    dispose( false );

    // Delete critical section used for locking data
    delete mAmbientSection;
    delete mStreamingSection;

    // Clear variables
    mAmbientSection   = CG_NULL;
    mStreamingSection = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAudioDriver::dispose( bool bDisposeBase )
{
    // Close down update thread (if running)
    if ( mUpdateThread != CG_NULL )
    {
        mUpdateThread->terminate();
        delete mUpdateThread;
    
    } // End if thread created
    
    // Release any resources we're managing
    releaseOwnedResources();
    
    // Clear variables
    mConfigLoaded     = false;
    mFadeOutTime      = 15.0f;
    mFadeInTime       = 6.0f;
    mUpdateThread     = CG_NULL;
    mResourceManager  = CG_NULL;

    // Clear structures
    memset( &mConfig, 0, sizeof(InitConfig) );

    // Call base class implemenation if required.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AudioDriver )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::processMessage( cgMessage * pMessage )
{
    // We're only processing messages from ourself
    if ( pMessage->fromId != getReferenceId() )
        return cgReference::processMessage( pMessage );

    // Bail if this will cause a recursive loop by sending other messages to ourself,
    // in response to our own unregister event.
    if ( pMessage->sourceUnregistered == true )
        return cgReference::processMessage( pMessage );

    // What is the message?
    if ( pMessage->messageId == cgSystemMessages::AudioDriver_Apply3DSettings )
    {
        // Allow derived class to apply
        apply3DSettings();
        
        // Send this message again in a little while
        cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), pMessage, m3DUpdateDelay );
        return true;

    } // End if apply settings

    // Message was not processed, pass to base.
    return cgReference::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : releaseOwnedResources ()
/// <summary>
/// Release any resources that the render driver may have loaded via its
/// resource manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::releaseOwnedResources()
{
    // Bail if nothing to release
    if ( mAmbientTracks.empty() )
        return;

    // Lock ambient track data
    mAmbientSection->enter();

    // Stop and release all ambient tracks
    for ( AmbientTrackMap::iterator TrackIterator = mAmbientTracks.begin(); TrackIterator != mAmbientTracks.end(); ++TrackIterator )
    {
        AmbientBufferList & List = TrackIterator->second;

        // Loop through each item in the buffer list
        for ( AmbientBufferList::iterator BufferIterator = List.begin(); BufferIterator != List.end(); ++BufferIterator )
        {
            AmbientItem * pItem = (*BufferIterator);
            if ( !pItem ) continue;

            // Stop and release the audio resource
            cgAudioBuffer * pBuffer = (cgAudioBuffer*)pItem->buffer.getResource( false );
            if ( pBuffer != CG_NULL ) pBuffer->stop();
            pItem->buffer.close( true );
            
            // Delete the item from the list
            delete pItem;

        } // Next sound item
    
    } // Next ambient track
    mAmbientTracks.clear();

    // Unlock ambient track data
    mAmbientSection->exit();
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver * cgAudioDriver::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver * cgAudioDriver::createInstance()
{
    // Determine which driver we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        if ( Config.audioAPI == cgAudioAPI::DirectX )
            return new cgDXAudioDriver();

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::createSingleton( )
{
    // Allocate!
    if ( !mSingleton )
        mSingleton = createInstance();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the audio device.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver::InitConfig cgAudioDriver::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the audio device
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::initialize( cgResourceManager * pResources, cgAppWindow * pFocusWnd )
{
    // Attach to the specified resource manager.
    mResourceManager = pResources;
    if ( pResources == CG_NULL || pResources->setAudioDriver( this ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to attach to specified resource manager. The application will now exit.\n") );
        return false;
    
    } // End if no resources

    // Send a message to ourselves to update the 3D settings shortly
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::AudioDriver_Apply3DSettings;
    cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, m3DUpdateDelay );

    // Spawn a worker thread to trigger our processing function
    mUpdateThread = cgThread::createInstance();
    return mUpdateThread->start( updateAudioThread, this );    
}

//-----------------------------------------------------------------------------
//  Name : PCM16StereoToMono () (Static)
/// <summary>
/// Mix a set of stereo 16 bit PCM samples down to a mono buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::PCM16StereoToMono( cgInt16 pDestSamples[], cgInt16 pSrcSamples[], cgUInt32 nSampleCount, cgDouble fGain )
{
    cgUInt32 i;
    int      nMixedSample, nLeftSample, nRightSample;

    // Convert dB gain to a 0 through 1 scalar
    cgDouble fGainScalar = pow( 10.0, fGain / 20.0 );  

    // Loop through each sample
    for ( i = 0; i < nSampleCount; ++i )
    {
        // Retrieve samples
        nLeftSample  = (int)*pSrcSamples++;
        nRightSample = (int)*pSrcSamples++;

        // Add the two values together and apply the dB gain
        // Ensure that result is a 32 bit signed int.
        nMixedSample = (int)( ((cgDouble)nLeftSample + (cgDouble)nRightSample) * fGainScalar );

        // Limit the results
        if ( nMixedSample < -0x8000 )
            nMixedSample = -0x8000;
        else if ( nMixedSample > 0x8000 )
            nMixedSample = 0x8000;

        // Store result
        *pDestSamples++ = (cgInt16)nMixedSample;

    } // Next Sample
}

//-----------------------------------------------------------------------------
//  Name : PCM8StereoToMono () (Static)
/// <summary>
/// Mix a set of stereo 8 bit PCM samples down to a mono buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::PCM8StereoToMono( cgByte pDestSamples[], cgByte pSrcSamples[], cgUInt32 nSampleCount, cgDouble fGain )
{
    cgUInt32 i;
    cgInt16  nMixedSample, nLeftSample, nRightSample;

    // Convert dB gain to a 0 through 1 scalar
    cgDouble fGainScalar = pow( 10.0, fGain / 20.0 );  

    // Loop through each sample
    for ( i = 0; i < nSampleCount; ++i )
    {
        // Retrieve samples (convert to signed)
        nLeftSample  = ((cgInt16)*pSrcSamples++) - 0x80;
        nRightSample = ((cgInt16)*pSrcSamples++) - 0x80;

        // Add the two values together and apply the dB gain
        // Ensure that result is a 16 bit signed short.
        nMixedSample = (cgInt16)( ((cgDouble)nLeftSample + (cgDouble)nRightSample) * fGainScalar );

        // Limit the results
        if ( nMixedSample < -0x80 )
            nMixedSample = -0x80;
        else if ( nMixedSample > 0x80 )
            nMixedSample = 0x80;

        // Store result (convert back to unsigned value)
        *pDestSamples++ = (cgByte)(nMixedSample + 0x80);

    } // Next Sample
}

//-----------------------------------------------------------------------------
//  Name : createAudioCodec () (Static)
/// <summary>
/// Create an instance of one of the installed audio codecs.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioCodec * cgAudioDriver::createAudioCodec( cgUInt32 nIndex )
{
    // Validate requirements
    if ( nIndex > AudioCodec_Count )
        return CG_NULL;

    // Return correct codec
    switch ( nIndex )
    {
        case AudioCodec_Ogg:
            return new cgAudioCodec_Ogg();
        case AudioCodec_Wav:
            return new cgAudioCodec_Wav();
    
    } // End Index Switch

    // Unknown codec 
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : releaseAudioCodec () (Static)
/// <summary>
/// Release an instance of the specified codec.
/// Note : Provided in case this needs to be called across process boundaries.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::releaseAudioCodec( cgAudioCodec * pCodec )
{
    if ( pCodec )
    {
        // Ensure that the codec closes it's file
        pCodec->close();

        // Release memory
        delete pCodec;

    } // End if valid pointer passed
}

//-----------------------------------------------------------------------------
//  Name : stopAmbientTracks ()
/// <summary>
/// Completely stop all currently playing ambient tracks.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::stopAmbientTracks( )
{
    // Lock ambient track data
    mAmbientSection->enter();

    // Iterate through all ambient tracks and pause them.
    AmbientTrackMap::iterator itTrack;
    for ( itTrack = mAmbientTracks.begin(); itTrack != mAmbientTracks.end(); ++itTrack )
    {
        // Get the last item that was added and fade it.
        AmbientBufferList & List = itTrack->second;
        if ( List.empty() )
            continue;

        AmbientItem * pItem = List.back();
        if ( pItem != CG_NULL )
            pItem->state = Fade_Out;

    } // Next track

    // Unlock ambient track data
    mAmbientSection->exit();   
}

//-----------------------------------------------------------------------------
//  Name : pauseAmbientTracks ()
/// <summary>
/// Pause all currently playing ambient tracks.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::pauseAmbientTracks( )
{
    // Lock ambient track data
    mAmbientSection->enter();

    // Iterate through all ambient tracks and pause them.
    AmbientTrackMap::iterator itTrack;
    for ( itTrack = mAmbientTracks.begin(); itTrack != mAmbientTracks.end(); ++itTrack )
    {
        AmbientBufferList::iterator itBuffer;
        AmbientBufferList & Tracks = itTrack->second;
        for ( itBuffer = Tracks.begin(); itBuffer != Tracks.end(); ++itBuffer )
        {
            AmbientItem * pItem = *itBuffer;
            if ( pItem && pItem->buffer.isValid() )
                pItem->buffer->pause();

        } // Next buffer

    } // Next track

    // Unlock ambient track data
    mAmbientSection->exit();    
}

//-----------------------------------------------------------------------------
//  Name : resumeAmbientTracks ()
/// <summary>
/// Resume all previously paused ambient tracks.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::resumeAmbientTracks( )
{
    // Lock ambient track data
    mAmbientSection->enter();

    // Iterate through all ambient tracks and resume them.
    AmbientTrackMap::iterator itTrack;
    for ( itTrack = mAmbientTracks.begin(); itTrack != mAmbientTracks.end(); ++itTrack )
    {
        AmbientBufferList::iterator itBuffer;
        AmbientBufferList & Tracks = itTrack->second;
        for ( itBuffer = Tracks.begin(); itBuffer != Tracks.end(); ++itBuffer )
        {
            AmbientItem * pItem = *itBuffer;
            if ( pItem && pItem->buffer.isValid() )
                pItem->buffer->resume();

        } // Next buffer

    } // Next track

    // Unlock ambient track data
    mAmbientSection->exit();
}

//-----------------------------------------------------------------------------
//  Name : loadAmbientTrack ()
/// <summary>
/// Load and play the specified file as the current ambient clip in the
/// track specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::loadAmbientTrack( const cgString & strTrackName, cgInputStream Stream, cgFloat fInitialVolume /* = 1.0f */, cgFloat fRequestedVolume /* = 1.0f */ )
{
    return loadAmbientTrack( strTrackName, Stream, fInitialVolume, fRequestedVolume, true );
}

//-----------------------------------------------------------------------------
//  Name : loadAmbientTrack ()
/// <summary>
/// Load and play the specified file as the current ambient clip in the
/// track specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::loadAmbientTrack( const cgString & strTrackName, cgInputStream Stream, cgFloat fInitialVolume, cgFloat fRequestedVolume, bool loop )
{
    cgAudioBufferHandle hBuffer;
    
    // Attempt to load the specified ambient file as a streaming sound effect
    cgUInt32 nAudioFlags = cgAudioBufferFlags::Streaming | cgAudioBufferFlags::AllowVolume | cgAudioBufferFlags::AllowPitch;
    if ( mResourceManager->loadAudioBuffer( &hBuffer, Stream, nAudioFlags, 0, cgDebugSource() ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to load ambient sound '%s' into track '%s'.\n"), Stream.getName().c_str(), strTrackName.c_str() );
        return false;

    } // End if failed to load sound

    // Fade previously playing ambient sound out
    stopAmbientTrack( strTrackName );

    // Generate the new ambient item
    AmbientItem * pNewItem    = new AmbientItem();
    pNewItem->buffer          = hBuffer;
    pNewItem->state           = Fade_In;
    pNewItem->requestedVolume = fRequestedVolume;
    pNewItem->looping         = loop;

    // Lock ambient track data
    mAmbientSection->enter();

    // Add the new ambient item to the track's list
    mAmbientTracks[ strTrackName ].push_back( pNewItem );

    // Start playing at initial volume
    cgAudioBuffer * pSound = hBuffer.getResource(true);
    if ( pSound != CG_NULL )
    {
        pSound->setVolume( fInitialVolume );
        pSound->play( loop );

    } // End if sound loaded

    // Unlock ambient track data
    mAmbientSection->exit();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : stopAmbientTrack ()
/// <summary>
/// Stop the currently playing ambient track.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::stopAmbientTrack( const cgString & strTrackName )
{
    AmbientTrackMap::iterator it;

    // Lock ambient track data
    mAmbientSection->enter();

    // Get the specified ambient track data
    it = mAmbientTracks.find( strTrackName );
    if ( it == mAmbientTracks.end() )
    {
        // Unlock ambient track data
        mAmbientSection->exit();
        return false;
    
    } // End if no track found with this name

    // Other sound currently playing?
    AmbientBufferList & List = it->second;
    if ( !List.empty() )
    {
        // Get the last item that was added
        AmbientItem * pItem = List.back();

        // Set to fade out
        if ( pItem != CG_NULL )
            pItem->state = Fade_Out;

    } // End if previously playing sound

    // Unlock ambient track data
    mAmbientSection->exit();

    // No ambient sound playing in this track
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setAmbientTrackPitch ()
/// <summary>
/// Set the pitch of the specified ambient track.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::setAmbientTrackPitch( const cgString & strTrackName, cgFloat fPitch )
{
    AmbientTrackMap::iterator it;

    // Lock ambient track data
    mAmbientSection->enter();

    // Get the specified ambient track data
    it = mAmbientTracks.find( strTrackName );
    if ( it == mAmbientTracks.end() )
    {
        // Unlock ambient track data
        mAmbientSection->exit();
        return false;
    
    } // End if no track found with this name

    // Set the pitch of playing sounds.
    AmbientBufferList & List = it->second;
    AmbientBufferList::iterator itBuffer;
    for ( itBuffer = List.begin(); itBuffer != List.end(); ++itBuffer )
    {
        AmbientItem * pItem = *itBuffer;
        if ( pItem->buffer.isValid() )
            pItem->buffer->setPitch( fPitch );

    } // Next buffer

    // Unlock ambient track data
    mAmbientSection->exit();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setAmbientTrackVolume ()
/// <summary>
/// Set the volume of the specified ambient track.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::setAmbientTrackVolume( const cgString & strTrackName, cgFloat fVolume )
{
    AmbientTrackMap::iterator it;

    // Lock ambient track data
    mAmbientSection->enter();

    // Get the specified ambient track data
    it = mAmbientTracks.find( strTrackName );
    if ( it == mAmbientTracks.end() )
    {
        // Unlock ambient track data
        mAmbientSection->exit();
        return false;
    
    } // End if no track found with this name

    // Get the last item in the list and adjust its volume / fade
    AmbientBufferList & List = it->second;
    if ( !List.empty() )
    {
        AmbientItem * pItem = List.back();
        if ( pItem->state == Fade_In )
            pItem->requestedVolume = fVolume;
        else if ( pItem->state == Fade_None && pItem->buffer.isValid() )
            pItem->buffer->setVolume( fVolume );

    } // End if has items
    
    // Unlock ambient track data
    mAmbientSection->exit();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isAmbientTrackPlaying ()
/// <summary>
/// Stop the currently playing ambient track.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::isAmbientTrackPlaying( const cgString & strTrackName )
{
    AmbientTrackMap::iterator it;

    // Lock ambient track data
    mAmbientSection->enter();

    // Get the specified ambient track data
    it = mAmbientTracks.find( strTrackName );
    if ( it == mAmbientTracks.end() )
    {
        // Unlock ambient track data
        mAmbientSection->exit();
        return false;

    } // End if no track found with this name

    // Sounds in the list?
    AmbientBufferList & List = it->second;
    if ( !List.empty() )
    {
        // Unlock ambient track data
        mAmbientSection->exit();
        return true;
    
    } // End if no sounds in the list
    
    // Unlock ambient track data
    mAmbientSection->exit();

    // No ambient sound playing in this track
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setTrackFadeTimes ()
/// <summary>
/// Set the time it takes to fade ambient tracks in and out
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioDriver::setTrackFadeTimes( cgFloat fFadeOutTime, cgFloat fFadeInTime )
{
    mFadeOutTime = fFadeOutTime;
    mFadeInTime  = fFadeInTime;
}

//-----------------------------------------------------------------------------
//  Name : addStreamBuffer() (Private)
/// <summary>
/// Add a streaming buffer to the list to be processed during the audio 
/// device's update process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::addStreamBuffer( cgAudioBuffer * pSound )
{
    // Lock streaming sound data
    mStreamingSection->enter();
    
    // Already exists in the list?
    BufferList::iterator it;
    for ( it = mStreamingBuffers.begin(); it != mStreamingBuffers.end(); ++it )
    {
        if ( *it == pSound )
        {
            // Unlock streaming sound data
            mStreamingSection->exit();
            return false;
        
        } // End if already exists

    } // Next Iteration
    
    // Add it to the list
    mStreamingBuffers.push_back( pSound );

    // Unlock streaming sound data
    mStreamingSection->exit();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : removeStreamBuffer () (Private)
/// <summary>
/// Remove a streaming buffer from the list to be processed during the 
/// audio device's update process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioDriver::removeStreamBuffer( cgAudioBuffer * pSound )
{
    // Lock streaming sound data
    mStreamingSection->enter();

    // Exists in the list?
    BufferList::iterator it;
    for ( it = mStreamingBuffers.begin(); it != mStreamingBuffers.end(); ++it )
    {
        if ( *it == pSound )
        {
            // Remove it to the list
            mStreamingBuffers.erase( it );

            // Unlock streaming sound data
            mStreamingSection->exit();

            // Success!
            return true;

        } // End if found

    } // Next Buffer
    
    // Nothing found. Just unlock streaming sound data.
    mStreamingSection->exit();
    return false;
}

//-----------------------------------------------------------------------------
//  Name : updateAudioThread () (Static Worker Thread Function)
/// <summary>
/// Worker thread function that allows the audio system to process.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgAudioDriver::updateAudioThread( cgThread * pThread, void * pContext )
{
    cgAudioBuffer * pBuffer = CG_NULL;
    cgFloat         fNewVolume;
    cgTimer         UpdateTimer;

    // Cast the context pointer for easy access
    cgAudioDriver * pThis = (cgAudioDriver*)pContext;

    // Get initial timer tick
    UpdateTimer.tick();

    // Keep looping until we're done
    for ( ; ; )
    {
        // Application waiting for us to terminate?
        if ( pThread->terminateRequested() == true )
            break;
        
        // Lock ambient track data
        pThis->mAmbientSection->enter();

        // Time since we were last able to update
        UpdateTimer.tick();

        // Process active ambient tracks
        for ( AmbientTrackMap::iterator TrackIterator = pThis->mAmbientTracks.begin(); TrackIterator != pThis->mAmbientTracks.end(); ++TrackIterator )
        {
            AmbientBufferList & List = TrackIterator->second;

            // Loop through each item in the buffer list
            for ( AmbientBufferList::iterator itBuffer = List.begin(); itBuffer != List.end(); )
            {
                AmbientBufferList::iterator itCurrent = itBuffer;
                AmbientItem * pItem = *itBuffer++;
                if ( pItem == CG_NULL ) continue;

                // We can ignore this item if it's not fading, only if it is a looping track.
                if ( pItem->state == Fade_None && pItem->looping )
                    continue;

                // Retrieve underlying buffer resources
                if ( !(pBuffer = pItem->buffer.getResource(false)) )
                    continue;

                // Fading in or out?
                if ( pItem->state == Fade_In )
                {
                    // Increase the volume of the track
                    if ( pThis->mFadeInTime < CGE_EPSILON )
                        fNewVolume = pItem->requestedVolume;
                    else
                        fNewVolume = std::min<cgFloat>( pItem->requestedVolume, pBuffer->getVolume() + ((cgFloat)UpdateTimer.getTimeElapsed() / pThis->mFadeInTime) );

                    // Set the volume
                    pBuffer->setVolume( fNewVolume );

                    // Allow windows to process, AND ensure the loop doesn't happen too fast as
                    // to overwhelm our floating point precision.
                    pThread->sleep( 1 );

                    // Finished fading in?
                    if ( fNewVolume >= pItem->requestedVolume )
                        pItem->state = Fade_None;

                } // End if fading in
                else if ( pItem->state == Fade_Out )
                {
                    // Decrease the volume of the track
                    if ( pThis->mFadeOutTime < CGE_EPSILON )
                        fNewVolume = 0.0f;
                    else
                        fNewVolume = std::max<cgFloat>( 0.0f, pBuffer->getVolume() - ((cgFloat)UpdateTimer.getTimeElapsed() / pThis->mFadeOutTime ) );

                    // Set the volume
                    pBuffer->setVolume( fNewVolume );

                    // Allow windows to process, AND ensure the loop doesn't happen too fast as
                    // to overwhelm our floating point precision.
                    pThread->sleep( 1 );

                    // Kill the music track if we're faded out
                    if ( fNewVolume <= 0.0f )
                    {
                        // Stop and release the buffer
                        pBuffer->stop();
                        delete pItem;
                        List.erase( itCurrent );

                    } // End if completely faded

                } // End if fading out
                else if ( !pItem->looping && !pBuffer->isPlaying() )
                {
                    // Stop release the buffer
                    delete pItem;
                    List.erase( itCurrent );

                } // End if finished playing

            } // Next sound item

        } // Next ambient track

        // Unlock ambient track data
        pThis->mAmbientSection->exit();

        // Lock streaming sound data
        pThis->mStreamingSection->enter();

        // Loop through each streaming sound
        BufferList::iterator itStream, itCurrent;
        for ( itStream = pThis->mStreamingBuffers.begin(); itStream != pThis->mStreamingBuffers.end(); )
        { 
            itCurrent = itStream;
            cgAudioBuffer * pBuffer = *itStream++;
            if ( pBuffer == CG_NULL ) continue;

            // If the stream is no longer playing, it can be removed
            if ( pBuffer->isPlaying() == false )
            {
                // Stop first, in case the actual audio file was fully read, but the buffer is still playing
                pBuffer->stop();

                // Remove from our consideration
                pThis->mStreamingBuffers.erase( itCurrent );
                continue;
            
            } // End if no longer playing

			//printf( _T("Checking Stream Update on %s @ %i\n"), pBuffer->GetInputSource()->strFile, timeGetTime() );

            // Trigger the stream update if required
            pBuffer->checkStreamUpdate();

        } // Next streaming sound effect

        // Unlock streaming sound data
        pThis->mStreamingSection->exit();

        // Allow windows to process, AND ensure the loop doesn't happen too fast.
        pThread->sleep( 10 );

    } // Next Iteration

    // Done
    return 0;
}