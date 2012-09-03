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
// Name : cgAudioBuffer.cpp                                                  //
//                                                                           //
// Desc : Contains classes responsible for loading and managing audio        //
//        buffer / sound effect resource data.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// Module Local Defines
//-----------------------------------------------------------------------------
#define DIRECTSOUND_VERSION 0x0800

//-----------------------------------------------------------------------------
// cgAudioBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgAudioBuffer.h>
#include <Audio/Platform/cgDXAudioDriver.h>
#include <Math/cgMathTypes.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Mmsystem.h>
#include <dsound.h>
#undef WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////
// cgAudioBuffer Class Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAudioBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioBuffer::cgAudioBuffer( cgUInt32 nReferenceId, cgAudioDriver * pDriver ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mAudioDriver          = pDriver;
    mBufferSize           = 0;
    mBuffer               = CG_NULL;
    m3DBuffer             = CG_NULL;
    mCodec                = CG_NULL;
    mCreationFlags        = cgAudioBufferFlags::Simple;
    mNotifyEvent          = CG_NULL;
    mNotifySize           = 0;
    mNotifyCount          = 16;
    mStreamBufferLength   = 10.0f; // 10 second buffer when streaming.
    mLooping              = false;
    mNextWriteOffset      = 0;
    mPlaybackComplete     = false;
    mFinalWriteOffset     = 0;
    m3DUpdateDelay        = 0.1f; // Update our settings once every 100ms
	mStreamDataPlayed     = 0;
	mLastStreamPosition   = 0;
	mStreamDataWritten    = 0;
    mInputSource.codecId  = 0;
    
    // Clear structures
    memset( &mBufferFormat, 0, sizeof(cgAudioBufferFormat) );

    // Cached resource responses
    mResourceType      = cgResourceType::AudioBuffer;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = false;
}

//-----------------------------------------------------------------------------
//  Name : cgAudioBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioBuffer::cgAudioBuffer( cgUInt32 nReferenceId, cgAudioDriver * pDriver, const cgInputStream & Stream, cgUInt32 nFlags ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mAudioDriver          = pDriver;
    mBufferSize           = 0;
    mBuffer               = CG_NULL;
    m3DBuffer             = CG_NULL;
    mCodec                = CG_NULL;
    mCreationFlags        = cgAudioBufferFlags::Simple;
    mNotifyEvent          = CG_NULL;
    mNotifySize           = 0;
    mNotifyCount          = 16;
    mStreamBufferLength   = 10.0f; // 10 second buffer when streaming.
    mLooping              = false;
    mNextWriteOffset      = 0;
    mPlaybackComplete     = false;
    mFinalWriteOffset     = 0;
    m3DUpdateDelay        = 0.1f; // Update our settings once every 100ms
	mStreamDataPlayed     = 0;
	mLastStreamPosition   = 0;
	mStreamDataWritten    = 0;
    mInputSource.codecId  = 0;

    // Store the resource load data
    mInputStream          = Stream;
    mInputFlags           = nFlags;
    
    // Clear structures
    memset( &mBufferFormat, 0, sizeof(cgAudioBufferFormat) );

    // Cached resource responses
    mResourceType      = cgResourceType::AudioBuffer;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = ( Stream.getType() == cgStreamType::File || Stream.getType() == cgStreamType::MappedFile );
}

//-----------------------------------------------------------------------------
//  Name : cgAudioBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioBuffer::cgAudioBuffer( cgUInt32 nReferenceId, cgAudioDriver * pDriver, cgAudioBuffer * pSourceBuffer ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mAudioDriver          = pDriver;
    mBufferSize           = 0;
    mBuffer               = CG_NULL;
    m3DBuffer             = CG_NULL;
    mCodec                = CG_NULL;
    mCreationFlags        = cgAudioBufferFlags::Simple;
    mNotifyEvent          = CG_NULL;
    mNotifySize           = 0;
    mNotifyCount          = 16;
    mStreamBufferLength   = 10.0f; // 10 second buffer when streaming.
    mLooping              = false;
    mNextWriteOffset      = 0;
    mPlaybackComplete     = false;
    mFinalWriteOffset     = 0;
    m3DUpdateDelay        = 0.1f; // Update our settings once every 100ms
	mStreamDataPlayed     = 0;
	mLastStreamPosition   = 0;
	mStreamDataWritten    = 0;
    mInputSource.codecId  = 0;
    mInputFlags           = 0;
    
    // Clear structures
    memset( &mBufferFormat, 0, sizeof(cgAudioBufferFormat) );

    // Cached resource responses
    mResourceType      = cgResourceType::AudioBuffer;
    mResourceLost     = false;
    mCanEvict         = false;

    // duplicate the source effect
    mResourceLoaded   = duplicate( pSourceBuffer );
}

//-----------------------------------------------------------------------------
//  Name : ~cgAudioBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioBuffer::~cgAudioBuffer()
{
    // Clean up
    dispose( false );

    // Reset variables
    mAudioDriver = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAudioBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgResource::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AudioBufferResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::loadResource( )
{
    bool bResult;

    // Is resource already loaded?
    if ( isLoaded() )
        return true;

    // Load the sound effect.
    bResult = load( mInputStream, mInputFlags );
    
    // Release unnecessary memory.
    if ( (supportsMode( cgAudioBufferFlags::Streaming ) == false) && 
         (mInputStream.getType() == cgStreamType::Memory) )
        mInputStream.reset();

    // Return result of the load
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::unloadResource( )
{
    // Remove us from the audio device stream list just in case
    if ( supportsMode( cgAudioBufferFlags::Streaming ) && mAudioDriver != CG_NULL )
        mAudioDriver->removeStreamBuffer( this );

    // Release memory
    cgAudioDriver::releaseAudioCodec( mCodec );
    if ( m3DBuffer != CG_NULL )
        m3DBuffer->Release();
    if ( mBuffer != CG_NULL )
        mBuffer->Release();

    // Release any handles
    if ( mNotifyEvent != CG_NULL )
        CloseHandle( mNotifyEvent );

    // Clear Variables
    mBufferSize           = 0;
    mBuffer               = CG_NULL;
    m3DBuffer             = CG_NULL;
    mCodec                = CG_NULL;
    mCreationFlags        = cgAudioBufferFlags::Simple;
    mNotifyEvent          = CG_NULL;
    mNotifySize           = 0;
    mNotifyCount          = 16;
    mStreamBufferLength   = 10.0f; // 10 second buffer when streaming.
    
    // Close down any input stream.
    mInputSource.stream    = cgInputStream();

    // Clear structures
    memset( &mBufferFormat, 0, sizeof(cgAudioBufferFormat) );

    // Resource is no longer loaded
    mResourceLoaded = false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsMode ()
/// <summary>
/// Determines if this sound buffers supports a particular mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::supportsMode( cgAudioBufferFlags::Base mode ) const
{
    // Was this mode specified at creation time?
    return ( mCreationFlags & mode ) != 0;
}

//-----------------------------------------------------------------------------
//  Name : getBufferFormat ()
/// <summary>
/// Retrieve the format of the audio buffer.
/// </summary>
//-----------------------------------------------------------------------------
const cgAudioBufferFormat & cgAudioBuffer::getBufferFormat( ) const
{
    return mBufferFormat;
}

//-----------------------------------------------------------------------------
//  Name : set3DSoundPosition ()
/// <summary>
/// Set the position of the sound relative to the world in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioBuffer::set3DSoundPosition( const cgVector3 & vecPos )
{
    // Store parameter
    m3DBuffer->SetPosition( vecPos.x, vecPos.y, vecPos.z, DS3D_DEFERRED );
}

//-----------------------------------------------------------------------------
//  Name : set3DSoundVelocity ()
/// <summary>
/// Set the velocity of the sound relative to the world in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioBuffer::set3DSoundVelocity( const cgVector3 & vecVelocity )
{
    // Store parameter
    m3DBuffer->SetVelocity( vecVelocity.x, vecVelocity.y, vecVelocity.z, DS3D_DEFERRED );
}

//-----------------------------------------------------------------------------
//  Name : set3DRangeProperties ()
/// <summary>
/// Set the details about when / how far away this can be heard
/// in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioBuffer::set3DRangeProperties( cgFloat fMinDistance, cgFloat fMaxDistance )
{
    // Store parameters
    m3DBuffer->SetMinDistance( fMinDistance, DS3D_DEFERRED );
    m3DBuffer->SetMaxDistance( fMaxDistance, DS3D_DEFERRED );
}

//-----------------------------------------------------------------------------
//  Name : get3DSoundInterface () (Private)
/// <summary>
/// Internal function make accessing the DirectSound 3D interfaces
/// much less painful.
/// </summary>
//-----------------------------------------------------------------------------
LPDIRECTSOUND3DBUFFER cgAudioBuffer::get3DSoundInterface()
{
    LPDIRECTSOUND3DBUFFER pDS3DBuffer = CG_NULL;

    // Validate requirements
    if ( mBuffer == CG_NULL || supportsMode( cgAudioBufferFlags::Positional ) == false )
        return CG_NULL;

    // Query for the 3D Buffer interface
    if ( FAILED( mBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void**)&pDS3DBuffer ) ) )
        return CG_NULL;

    // Return the interface
    return pDS3DBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getInputSource ()
/// <summary>
/// Returns information about our internal input source.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioBuffer::InputSource & cgAudioBuffer::getInputSource( )
{
    return mInputSource;
}

//-----------------------------------------------------------------------------
//  Name : duplicate ()
/// <summary>
/// Duplicate the specified buffer into this.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::duplicate( cgAudioBuffer * pBuffer )
{
    HRESULT             hRet;
    LPDIRECTSOUND       pDS        = CG_NULL;
    LPDIRECTSOUNDBUFFER pSrcBuffer = CG_NULL;

    // Validate source
    if ( pBuffer->supportsMode( cgAudioBufferFlags::Streaming ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Streaming audio buffers cannot be duplicated. Attempted to duplicate resource '%s' (Size: %i bytes).\n"), pBuffer->getResourceName().c_str(), pBuffer->getBufferSize() );
        return false;
    
    } // End if src buffer is a stream

    // Only 'cgDXAudioDriver' type is supported.
    cgDXAudioDriver * pDriver = dynamic_cast<cgDXAudioDriver*>(mAudioDriver);
    if ( pDriver == CG_NULL || (pDS = pDriver->getDirectSound()) == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access internal DirectSound object while duplicating resource '%s'.\n"), pBuffer->getResourceName().c_str() );
        return false;

    } // End if invalid cast

    // Retrieve the source sound buffer
    pSrcBuffer = pBuffer->getInternalBuffer();
    if ( pSrcBuffer == CG_NULL )
    {
        pDS->Release();
        return false;
    
    } // End if no buffer

    // Attempt to duplicate the buffer
    hRet = pDS->DuplicateSoundBuffer( pSrcBuffer, &mBuffer );

    // Release the interfaces
    pSrcBuffer->Release();
    pDS->Release();

    // Failed to duplicate?
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to duplicate audio buffer from resource '%s' (Size: %i bytes).\n"), pBuffer->getResourceName().c_str(), pBuffer->getBufferSize() );
        return false;

    } // End if failed to create temporary buffer

    // Duplicate source effect information
    mBufferSize    = pBuffer->getBufferSize();
    mBufferFormat  = pBuffer->getBufferFormat();
    mCreationFlags = pBuffer->getCreationFlags();
    mInputSource   = pBuffer->getInputSource();

    // Create a new codec for this input source
    mCodec = cgAudioDriver::createAudioCodec( mInputSource.codecId );
    if ( mCodec->open( mInputSource.stream ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("PCM decoding codec unable to open stream for duplication '%s'.\n"), mInputSource.stream.getName().c_str() );
        return false;

    } // End if failed to open
    
    // Cache a copy of the 3D buffer interface if applicable.
    if ( supportsMode( cgAudioBufferFlags::Positional ) == true )
    {
        m3DBuffer = get3DSoundInterface();
        if ( m3DBuffer == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("DirectSound buffer does not implement the 3D interface (Flags: 0x%x, Size: %i bytes).\n"), mCreationFlags, mBufferSize );
            return false;

        } // End if no 3D Interface

    } // End if positional buffer

    // Resource is now loaded
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : load ()
/// <summary>
/// Load the specified sound effect from file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::load( cgInputStream Stream, cgUInt32 nFlags )
{
    cgUInt32            i, CodecId;
    cgAudioBufferFormat Format;
    cgByte             *pAudioData  = CG_NULL;
    cgUInt32            nAudioSize  = 0;

    // Select the codec that supports the reading of this file (if any)
    cgAudioCodec * pCodec = CG_NULL;
    for ( i = 0; i < cgAudioDriver::AudioCodec_Count; ++i )
    {
        // Select a codec
        if ( (pCodec = cgAudioDriver::createAudioCodec( i )) == CG_NULL )
            continue;

        // Codec supports the specified file?
        if ( pCodec->isValid( Stream ) == true )
            break;

        // Unsupported, release the codec
        cgAudioDriver::releaseAudioCodec( pCodec );
        pCodec = CG_NULL;

    } // Next codec

    // No codec supports this file type?
    if ( pCodec == CG_NULL )
        return false;

    // Store the codec we're using
    mCodec = pCodec;
    CodecId  = i;

    // Store creation flags.
    mCreationFlags = nFlags;

    // Request that the codec open the specified audio data
    if ( pCodec->open( Stream ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Codec was unable to open the stream for PCM audio decoding '%s' (Flags: 0x%x).\n"), Stream.getName().c_str(), nFlags );
        return false;

    } // End if failed to open

    // Retrieve the format of the sample data
    if ( pCodec->getPCMFormat( Format ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to retrieve PCM audio format for stream '%s' (Flags: 0x%x).\n"), Stream.getName().c_str(), nFlags );
        return false;

    } // End if failed to get PCM format

    // Compatible format?
    if ( (Format.channels < 1 || Format.channels > 2) || (Format.bitsPerSample != 8 && Format.bitsPerSample != 16) )
    {
        cgAppLog::write( cgAppLog::Error, _T("The specified stream '%s' is of a supported type, but its PCM data format is incompatible (Flags: 0x%x).\n"), Stream.getName().c_str(), nFlags );
        return false;

    } // End if not compatible

	// Record information about the source of our audio data.
    mInputSource.stream  = Stream;
    mInputSource.codecId = CodecId;
    mInputSource.format  = Format;

    // If we're not streaming, we should decode the entire file
    if ( supportsMode( cgAudioBufferFlags::Streaming ) == false )
    {
        // Decode the full buffer
        if ( decodeFullAudioStream( pCodec, Format, nFlags, &pAudioData, &nAudioSize ) == false )
        {
            cgAppLog::write( cgAppLog::Error, _T("Decoding of PCM audio data failed for stream '%s' (Flags: 0x%x).\n"), Stream.getName().c_str(), nFlags );
            return false;
        
        } // End if full decode failed

        cgAppLog::write( cgAppLog::Debug, _T("BytesPerSecond=%i, BitsPerSample=%i, BlockAlign=%i, Channels=%i, FormatType=%i, SamplesPerSecond=%i, Size=%i.\n"), Format.averageBytesPerSecond, Format.bitsPerSample, Format.blockAlign, Format.channels, Format.formatType, Format.samplesPerSecond, Format.size );
        cgAppLog::write( cgAppLog::Debug, _T("Creating streaming audio buffer of size %i.\n"), nAudioSize );

        // Create the buffer(s)
        if ( createAudioBuffer( nFlags, Format, nAudioSize, pAudioData, nAudioSize ) == false )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to create audio buffer(s) for stream '%s' (Flags: 0x%x, Size:%i). See previous errors for more information.\n"), Stream.getName().c_str(), nFlags, nAudioSize );
            delete []pAudioData;
            return false;

        } // End if buffer creation failed

        // Clean up
        delete []pAudioData;
        
    } // End if not streaming
    else
    {
        // If this is a positional audio source, and the audio buffer contains stereo data,
        // we must convert it to mono on the fly in the streaming case (3D audio requires mono buffer).
        // Make a note of this requirement and adjust the format appropriately for that case.
        if ( (nFlags & cgAudioBufferFlags::Positional) && Format.channels == 2 )
        {
            // Since the second channel will be removed, data size will be halved
            Format.channels = 1;
            Format.averageBytesPerSecond /= 2;
            Format.blockAlign /= 2;

        } // End if requires mixing to mono

        // Determine the notification size (it should be multiples of the audio data block alignment)
        mNotifySize  = (cgInt32)(((cgFloat)Format.samplesPerSecond * (cgFloat)Format.blockAlign) * mStreamBufferLength) / mNotifyCount;
        mNotifySize -= mNotifySize % Format.blockAlign;
        cgAppLog::write( cgAppLog::Debug, _T("BytesPerSecond=%i, BitsPerSample=%i, BlockAlign=%i, Channels=%i, FormatType=%i, SamplesPerSecond=%i, Size=%i.\n"), Format.averageBytesPerSecond, Format.bitsPerSample, Format.blockAlign, Format.channels, Format.formatType, Format.samplesPerSecond, Format.size );
        cgAppLog::write( cgAppLog::Debug, _T("Creating streaming audio buffer of size %i (block size %i and block count %i).\n"), mNotifySize * mNotifyCount, mNotifySize, mNotifyCount );

        // Create the buffer(s)
        if ( createAudioBuffer( nFlags, Format, mNotifySize * mNotifyCount ) == false )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to create streaming audio buffer(s) for stream '%s' (Flags: 0x%x, Size:%i). See previous errors for more information.\n"), Stream.getName().c_str(), nFlags, nAudioSize );
            return false;

        } // End if buffer creation failed

    } // End if streaming data

    // Resource can now be considered loaded.
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : decodeFullAudioStream () (Private)
/// <summary>
/// Decode the entire contents of the file opened by the audio codec
/// and return the resulting PCM data in one continuous byte array.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::decodeFullAudioStream( cgAudioCodec * pCodec, cgAudioBufferFormat & Format, cgUInt32 nFlags, cgByte ** ppAudioData, cgUInt32 * pnAudioSize )
{
    cgByte * pBuffer = CG_NULL, * pAudioData = CG_NULL;
    cgUInt32 nAudioSize = 0, nAudioCapacity = 0;
    int      nBytesRead;

    // Statically sized read buffer of 8k (NB: Must be multiples of 4 bytes (16bit samples * 2 channels)).
    static const cgInt32 ReadBufferSize = (8 * 1024);
    cgByte               pReadBuffer[ReadBufferSize];

    // Empty passed variables (just to be poliate)
    if ( ppAudioData ) *ppAudioData = CG_NULL;
    if ( pnAudioSize ) *pnAudioSize = 0;

    // Read the decoded PCM audio data into memory
    for ( ; ; )
    {
        // Decode from the ogg vorbis file into our temporary buffer
        nBytesRead = pCodec->decodePCM( pReadBuffer, ReadBufferSize );
        if ( nBytesRead == 0 || nBytesRead == cgAudioCodec::ReadError_Abort ) break;
        if ( nBytesRead == cgAudioCodec::ReadError_Retry ) continue;

        // Only need to populate the output buffer if requested, otherwise
        // we will purely be decoding for the sake of generating a size.
        if ( ppAudioData )
        {
            // Enough room in our array?
            if ( nAudioSize + nBytesRead > nAudioCapacity )
            {
                // Select new capacity (grow in 128k increments)
                for ( ; nAudioSize + nBytesRead > nAudioCapacity; ) nAudioCapacity += (128 * 1024);

                // Allocate new audio data buffer
                pBuffer = new cgByte[ nAudioCapacity ];
                if ( !pBuffer )
                {
                    if ( pAudioData ) delete []pAudioData;
                    return false;

                } // End if failed to allocate

                // Copy over any old data
                if ( pAudioData && nAudioSize > 0 ) memcpy( pBuffer, pAudioData, nAudioSize );

                // Release old buffer and replace
                if ( pAudioData ) delete []pAudioData;
                pAudioData = pBuffer;

            } // End if growing is required

            // Copy audio data into buffer
            memcpy( pAudioData + nAudioSize, pReadBuffer, nBytesRead );
        
        } // End if require the actual data
        
        // Data added, increase size by appropriate amount
        nAudioSize += nBytesRead;

    } // Next block of data

    // Nothing read?
    if ( nAudioSize == 0 )
    {
        // Paranoia: Array should never have been allocated.
        if ( pAudioData ) delete [] pAudioData;
        return false;

    } // End if nothing read

    // Convert the data in the read buffer to mono if required (3D audio requires mono buffer)
    if ( (nFlags & cgAudioBufferFlags::Positional) && Format.channels == 2 )
    {
        if ( Format.bitsPerSample == 16 )
        {
            cgUInt32 nSampleCount = nAudioSize / 4; // (2 bytes per sample, per channel)

            // Mix the two channels into one
            cgAudioDriver::PCM16StereoToMono( (cgInt16*)pAudioData, (cgInt16*)pAudioData, nSampleCount );

        } // End if 16 bit samples
        else
        {
            cgUInt32 nSampleCount = nAudioSize / 2; // (1 byte per sample, per channel)

            // Mix the two channels into one
            cgAudioDriver::PCM8StereoToMono( pAudioData, pAudioData, nSampleCount );

        } // End if 8 bit samples

        // Since the second channel has been removed, data size is now halved
        nAudioSize            /= 2;
        Format.channels        = 1;
        Format.averageBytesPerSecond /= 2;
        Format.blockAlign     /= 2;

    } // End if requires mixing to mono

    // Return the resulting buffers
    if ( ppAudioData ) *ppAudioData = pAudioData;
    if ( pnAudioSize ) *pnAudioSize = nAudioSize;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createAudioBuffer () (Private)
/// <summary>
/// Create the actual audio buffers that will be used to play the sound
/// effects loaded.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::createAudioBuffer( cgUInt32 nFlags, cgAudioBufferFormat & Format, cgUInt32 nBufferSize, cgByte * pInitData /* = CG_NULL */, cgUInt32 nInitDataSize /* = 0 */ )
{
    //cgUInt32     i;
    LPDIRECTSOUND8  pDS = CG_NULL;
    DSBUFFERDESC    Desc;
    HRESULT         hRet;

    // Build the buffer description for our DirectSound buffer
    memset( &Desc, 0, sizeof(DSBUFFERDESC) );
    Desc.dwSize             = sizeof(DSBUFFERDESC);
    Desc.dwFlags            = DSBCAPS_GETCURRENTPOSITION2;
    Desc.lpwfxFormat        = (WAVEFORMATEX*)&Format;
    Desc.dwReserved         = 0;
    Desc.dwBufferBytes      = nBufferSize;

	// ToDo: Testing - Perform software mixing to ensure notifications work as intended?
	//Desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

    // Set flags
    if ( nFlags & cgAudioBufferFlags::AllowPan    ) Desc.dwFlags |= DSBCAPS_CTRLPAN;
    if ( nFlags & cgAudioBufferFlags::AllowVolume ) Desc.dwFlags |= DSBCAPS_CTRLVOLUME;
    if ( nFlags & cgAudioBufferFlags::AllowPitch  ) Desc.dwFlags |= DSBCAPS_CTRLFREQUENCY;
    //if ( nFlags & Streaming   ) Desc.dwFlags |= DSBCAPS_CTRLPOSITIONNOTIFY;

    // Select a 3D algorithm?
    if ( nFlags & cgAudioBufferFlags::Positional )
    {
        // Cannot support panning when using a positional buffer
        Desc.dwFlags &= ~DSBCAPS_CTRLPAN;

        // Use simple stereo 3D panning
        Desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION; //DS3DALG_HRTF_FULL;
        Desc.dwFlags |= DSBCAPS_CTRL3D;

        // Mute the sound entirely at max distance?
        if ( nFlags & cgAudioBufferFlags::MuteAtMaxDistance )
        {
            Desc.dwFlags |= DSBCAPS_MUTE3DATMAXDISTANCE;
            // ToDo: Muting at max distance only works reliably in software.
            Desc.dwFlags |= DSBCAPS_LOCSOFTWARE;

        } // End if mute outside of range

    } // End if positional

    // Only 'cgDXAudioDriver' type is supported.
    cgDXAudioDriver * pDriver = dynamic_cast<cgDXAudioDriver*>(mAudioDriver);
    if ( pDriver == CG_NULL || (pDS = pDriver->getDirectSound()) == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to access internal DirectSound object while creating resource '%s'.\n"), getResourceName().c_str() );
        return false;

    } // End if invalid cast

    // Create the buffer
    hRet = pDS->CreateSoundBuffer( &Desc, &mBuffer, CG_NULL );

    // Release temporary references.
    pDS->Release();

    // Validate
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("API reported creation of sound buffer failed (Flags: 0x%x, Size: %i bytes, Result: 0x%x).\n"), nFlags, nBufferSize, hRet );
        return false;

    } // End if failed to create temporary buffer

    // Cache a copy of the 3D buffer interface if applicable.
    if ( supportsMode( cgAudioBufferFlags::Positional ) == true )
    {
        m3DBuffer = get3DSoundInterface();
        if ( m3DBuffer == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("DirectSound buffer does not implement the 3D interface (Flags: 0x%x, Size: %i bytes).\n"), nFlags, nBufferSize );
            return false;

        } // End if no 3D Interface

    } // End if positional buffer

    // Store size of buffer
    mBufferSize  = nBufferSize;
    mBufferFormat = Format;

    //******************************************************************************
	// Notification support in hardware is sometimes poor (notifications get sent at 
	// the wrong times / incorrect event handles). Disabled in this implementation.
	//******************************************************************************
	/*// Are we creating a streaming buffer?
    if ( nFlags & Streaming )
    {
        DSBPOSITIONNOTIFY  *pNotifyPositions = CG_NULL; 
        LPDIRECTSOUNDNOTIFY pDSNotify        = CG_NULL;

        // Retrieve the notification interface
        if ( FAILED( hRet = mBuffer->QueryInterface( IID_IDirectSoundNotify, (void**)&pDSNotify ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("DirectSound buffer does not implement the notification interface (Flags: 0x%x, Size: %i bytes, Result: 0x%x).\n"), nFlags, nBufferSize, hRet );
            return false;
        
        } // End if no notification interface

        // Create the notification event handle
        mNotifyEvent = CreateEvent( CG_NULL, FALSE, FALSE, CG_NULL );

        // Allocate enough entries for each position in the buffer we would like
        // to receive notifications for.
        pNotifyPositions = new DSBPOSITIONNOTIFY[ mNotifyCount ];
        if ( !pNotifyPositions )
        {
            cgAppLog::write( cgAppLog::Error, _T("Out of memory whilst allocating stream notification array of %i elements in size.\n"), mNotifyCount );
            return false;

        } // End if out of memory

        // Populate the notification position array
        for( i = 0; i < mNotifyCount; i++ )
        {
            pNotifyPositions[i].dwOffset     = (mNotifySize * i) + mNotifySize - 1;
            pNotifyPositions[i].hEventNotify = mNotifyEvent;
        
        } // Next position

        // Inform DirectSound that it should notify us at these intervals.
        // The signaled event can be tested with a call to 'checkStreamUpdate()'.
        if( FAILED( hRet = pDSNotify->SetNotificationPositions( mNotifyCount, pNotifyPositions ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("DirectSound reported an error whilst setting %i stream notification positions (Flags: 0x%x, Size: %i bytes, Result: 0x%x).\n"), mNotifyCount, nFlags, nBufferSize, hRet );
            
            // Clean up and bail
            pDSNotify->Release();
            delete []pNotifyPositions;
            return false;
        
        } // End if failed to set positions

        // Clean up
        pDSNotify->Release();
        delete []pNotifyPositions;

    } // End if streaming*/
	//******************************************************************************

    // Any initial data?
    if ( pInitData != CG_NULL )
    {
        cgByte * pBuffer;
        cgUInt32 nLockSize;

        // Lock the entire buffer
        if ( FAILED( mBuffer->Lock( 0, 0, (void**)&pBuffer, &nLockSize, CG_NULL, CG_NULL, DSBLOCK_ENTIREBUFFER) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to lock sound buffer during initial population (Flags: 0x%x, Lock Size: %i bytes).\n"), mCreationFlags, mBufferSize );
            return false;

        } // End if failed to lock

        // Copy the decoded data into the buffer
        memcpy( pBuffer, pInitData, min( nLockSize, nInitDataSize ) );

        // If there was any slack data left at the end, fill it with silence.
        if ( nInitDataSize < nLockSize )
            FillMemory( pBuffer, nLockSize - nInitDataSize, (Format.bitsPerSample == 8) ? 128 : 0 );
        
        // Unlock the buffer
        mBuffer->Unlock( pBuffer, nLockSize, CG_NULL, CG_NULL );
    
    } // End if initial data
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : populateAudioBuffer () (Private)
/// <summary>
/// Populate the audio buffer with data from the audio stream.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::populateAudioBuffer( )
{
    cgByte * pLockedBuffer = CG_NULL;
    cgUInt32 nLockSize;
    cgInt32  nBytesRead;
    bool     bWasRestored;

    // ToDo: Keep the stereo remix buffer allocated to save us having to
    // recreate one each time.

    // Validate requirements
    if ( !mBuffer || !mCodec )
        return false;

    // First make sure that we have focus / a valid and available audio buffer
    if ( !restoreAudioBuffer( bWasRestored ) )
        return false;

    // Lock the entire buffer
    if ( FAILED( mBuffer->Lock( 0, mBufferSize, (void**)&pLockedBuffer, &nLockSize, CG_NULL, CG_NULL, 0) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to lock sound buffer during population for resource '%s' (Flags: 0x%x, Lock Size: %i bytes).\n"), getResourceName(), mCreationFlags, mBufferSize );
        return false;
        
    } // End if failed to lock

    // Reset the stream we're populating from
    mCodec->reset();

    // We'll need to convert the data in the read buffer to mono if it has two channels
    // and this is a positional audio buffer (3D audio requires mono buffer).
    cgByte * pReadBuffer = pLockedBuffer;
    cgUInt32 nReadAmount = nLockSize;
    bool bStereoToMono = (supportsMode(cgAudioBufferFlags::Positional) && mInputSource.format.channels == 2);
    if ( bStereoToMono )
    {
        // Data is twice the size of that in the locked buffer.
        nReadAmount *= 2;
        pReadBuffer = new cgByte[nReadAmount];

    } // End if requires mixing

    // Read the required number of bytes into the buffer
    for ( ; ; )
    {
        nBytesRead = mCodec->decodePCM( pReadBuffer, nReadAmount );
        if ( nBytesRead == cgAudioCodec::ReadError_Retry )
            continue;
        break;
    
    } // Next read attempt 

    // Did we read any data at all?
    if ( nBytesRead <= 0 )
    {
        // It's empty (or an error occurred, such as corrupt data), just fill the locked region with silence
        FillMemory( pReadBuffer, nReadAmount, (mInputSource.format.bitsPerSample == 8) ? 128 : 0 );
        nBytesRead = nReadAmount;
    
    } // End if no data
    else if ( nBytesRead < (signed)nReadAmount )
    {
        // Not enough data read (file completely decoded?), fill the remainder with silence
        FillMemory( &pReadBuffer[nBytesRead], nReadAmount - nBytesRead, (mInputSource.format.bitsPerSample == 8) ? 128 : 0 );

    } // End if not enough data read

    // Remix to mono if required.
    if ( bStereoToMono )
    {
        // Mix the two channels into one
        if ( mInputSource.format.bitsPerSample == 16 )
            cgAudioDriver::PCM16StereoToMono( (cgInt16*)pLockedBuffer, (cgInt16*)pReadBuffer, nReadAmount / 4 ); // (2 bytes per sample, per channel)
        else
            cgAudioDriver::PCM8StereoToMono( pLockedBuffer, pReadBuffer, nReadAmount / 2 ); // (1 byte per sample, per channel)

    } // End if remix

    // Clean up
    if ( pReadBuffer != pLockedBuffer )
        delete []pReadBuffer;

    // Move the write offset ahead (wrap it round once we pass the end of the buffer)
    mNextWriteOffset	+= nLockSize;
    mNextWriteOffset    %= mBufferSize;
	mStreamDataWritten  += nLockSize;
    
    // Unlock the buffer
    mBuffer->Unlock( pLockedBuffer, nLockSize, CG_NULL, CG_NULL );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreAudioBuffer () (Private)
/// <summary>
/// Determine if the buffer is / has been lost and needs restoring.
/// If it is ready to be restored, then do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::restoreAudioBuffer( bool & bWasRestored )
{
    cgUInt32 nStatus;

    // Initialize referenced return value
    bWasRestored = false;

    // Validate requirements
    if ( !mBuffer ) return false;

    // Get the status of the buffer
    if ( FAILED( mBuffer->GetStatus( &nStatus ) ) ) return false;

    // Is the buffer in a lost state?
    if ( nStatus & DSBSTATUS_BUFFERLOST )
    {
        // Keep attempting to restore until the device is no longer lost
        for ( ; mBuffer->Restore() == DSERR_BUFFERLOST ; )
        {
            // Wait for short while until we try again
            Sleep( 10 );
        
        } // Next attempt to restore

        // Buffer was restored
        bWasRestored = true;

    } // End if lost

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isPlaying ()
/// <summary>
/// Determine if the sound is currently playing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::isPlaying( ) const
{
    cgUInt32 Status;

    // Validate requirements and get status
    if ( mBuffer == CG_NULL || FAILED( mBuffer->GetStatus( &Status ) ) )
        return false;

    // Playing?
    if ( supportsMode( cgAudioBufferFlags::Streaming ) == false )
    {
        // Just return the playing status
        return (Status & DSBSTATUS_PLAYING);
    
    } // End if not streaming
    else
    {
        // Not playing at all?
        if ( !(Status & DSBSTATUS_PLAYING) )
            return false;

        // If we're looping, we'll just return true
        if ( mLooping == true )
            return true;

        // If not, let's actually check our playback complete indicator
        if ( mPlaybackComplete == true )
        {
            cgUInt32 nCurrentPlayCursor;
            if ( FAILED( mBuffer->GetCurrentPosition( &nCurrentPlayCursor, CG_NULL ) ) )
                return false;
            if ( nCurrentPlayCursor <= mFinalWriteOffset )
                return true;
        
        } // End if playback is listed as complete
        else
        {
            // Playback is not yet complete
            return true;
        
        } // End if playback is not complete

        // Otherwise, we're not playing
        return false;
    
    } // End if streaming
}

//-----------------------------------------------------------------------------
//  Name : getInternalBuffer ()
/// <summary>
/// Retrieve the underlying DirectSound buffer
/// </summary>
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER cgAudioBuffer::getInternalBuffer( ) const
{
    // Add a reference if we're about to return a valid pointer
    if ( mBuffer != NULL )
        mBuffer->AddRef();
    
    // Return buffer pointer
    return mBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getBufferSize ()
/// <summary>
/// Retrieve the size of the audio buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgAudioBuffer::getBufferSize( ) const
{
    return mBufferSize;
}

//-----------------------------------------------------------------------------
//  Name : getBufferPosition ()
/// <summary>
/// Get the current playback position within the buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgAudioBuffer::getBufferPosition( ) const
{
    cgUInt32 nPos = 0;
    
    // Validate requirements
    if ( !mBuffer ) return 0;

    // Retrieve actual position
    if ( FAILED( mBuffer->GetCurrentPosition( &nPos, CG_NULL ) ) ) return 0;

    // Return the position
    return nPos;
}

//-----------------------------------------------------------------------------
//  Name : setBufferPosition ()
/// <summary>
/// Set the current playback position within the buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::setBufferPosition( cgUInt32 nPos )
{
    
    // Validate requirements
    if ( !mBuffer ) return false;

    // Retrieve actual position
    if ( FAILED( mBuffer->SetCurrentPosition( nPos ) ) ) return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getCreationFlags ()
/// <summary>
/// Retrieve the flags that were used to create the audio buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgAudioBuffer::getCreationFlags( ) const
{
    return mCreationFlags;
}

//-----------------------------------------------------------------------------
//  Name : play ()
/// <summary>
/// Play the sound effect.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::play( bool bLoop /* = false */ )
{
    // Validate requirements
    if ( !mBuffer || !mCodec ) return false;

    cgUInt32 nFlags = 0;
    if ( bLoop ) nFlags = DSBPLAY_LOOPING;
    bool bIsPlaying = isPlaying();

    // Different looping status?
    if ( bIsPlaying && bLoop != mLooping )
        stop();

    // If this is a streaming sound, we must update some items
    if ( supportsMode( cgAudioBufferFlags::Streaming ) == true )
    {
        mNextWriteOffset		= 0;
        mFinalWriteOffset		= 0;
		mStreamDataPlayed		= 0;
		mLastStreamPosition	= 0;
		mStreamDataWritten	= 0;
        mPlaybackComplete		= false;

        // Populate audio buffer with initial data, this is needed because it will begin by
        // playing this data, before we start streaming the next set.
        populateAudioBuffer( );

    } // End if streaming sound

    // If we are already playing, simply reset it's position to the beginning
    if ( bIsPlaying )
    {
        // Move to beginning
        if ( FAILED( mBuffer->SetCurrentPosition( 0 ) ) )
            return false;        

        // Signal the play if we were changing loop status
        if ( bLoop != mLooping )
        {
            if ( FAILED( mBuffer->Play( 0, 0, nFlags ) ) )
                return false;
        
        } // End if changing
    
    } // End if already playing
    else
    {
        // Signal the play
        if ( FAILED( mBuffer->Play( 0, 0, nFlags ) ) )
            return false;
    
    } // End if not playing

    // Add this to the stream buffers that the audio device is monitoring
    if ( supportsMode( cgAudioBufferFlags::Streaming ) == true )
        mAudioDriver->addStreamBuffer( this );
    
    // Update looping flag
    mLooping = bLoop;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : stop ()
/// <summary>
/// Stop the currently playing sound effect.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::stop( )
{
    // Validate requirements
    if ( !mBuffer ) return false;

    // Signal the play
    if ( FAILED( mBuffer->Stop( ) ) ) return false;

    // Move to beginning
    mBuffer->SetCurrentPosition( 0 );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVolume ()
/// <summary>
/// Set the volume of the currently playing sound
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::setVolume( cgFloat fVolume )
{
    // Validate Requirements
    if ( mBuffer == CG_NULL || supportsMode( cgAudioBufferFlags::AllowVolume ) == false )
        return false;

    // Set the volume
    cgInt32 nVolume = (cgInt32)(DSBVOLUME_MIN + ((DSBVOLUME_MAX - DSBVOLUME_MIN) * fVolume));
    return SUCCEEDED( mBuffer->SetVolume( nVolume ) );
}

//-----------------------------------------------------------------------------
//  Name : getVolume ()
/// <summary>
/// Retrieve the volume of the currently playing sound.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgAudioBuffer::getVolume( ) const
{
    cgInt32 nVolume;

    // Validate requirements
    if ( mBuffer == CG_NULL )
        return 0.0f;
    if ( supportsMode( cgAudioBufferFlags::AllowVolume ) == false )
        return 1.0f;

    // Query actual volume
    if ( FAILED(mBuffer->GetVolume( &nVolume ) ) )
        return 1.0f;

    // Convert to our 0-1 scalar
    return ((cgFloat)nVolume - DSBVOLUME_MIN) / (DSBVOLUME_MAX - DSBVOLUME_MIN);
}

//-----------------------------------------------------------------------------
//  Name : checkStreamUpdate ()
/// <summary>
/// Determine the status of the stream and update the buffer if
/// necessary.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioBuffer::checkStreamUpdate( )
{
    //cgUInt32 nResult;
    bool     bWasRestored;
    cgByte * pLockedBuffer = CG_NULL, * pLockedBuffer2 = CG_NULL;
    cgUInt32 nLockSize, nLockSize2, nCurrentPlayCursor;
	cgInt32  nBytesRead;

    // ToDo: Keep the stereo remix buffer allocated to save us having to
    // recreate one each time.

    // Validate requirements
    if ( mBuffer == CG_NULL || mCodec == CG_NULL /*|| mNotifyEvent == CG_NULL*/ )
        return false;
    if ( supportsMode( cgAudioBufferFlags::Streaming ) == false )
        return false;

	//******************************************************************************
	// Notification support in hardware is sometimes poor (notifications get sent at 
	// the wrong times / incorrect event handles). Disabled in this implementation.
	//******************************************************************************
    // Determine if data update event has been signaled
    //nResult = ::WaitForSingleObject( mNotifyEvent, 0 );
    //if ( nResult != WAIT_OBJECT_0 ) return false;
	//******************************************************************************
	
	// Compute the current amount of data that has been played from the stream so far since it started playing (including looping).
	mBuffer->GetCurrentPosition( &nCurrentPlayCursor, CG_NULL );
	if ( nCurrentPlayCursor < mLastStreamPosition )
		mStreamDataPlayed += (signed)nCurrentPlayCursor - ((signed)mLastStreamPosition - (signed)mBufferSize);
	else
		mStreamDataPlayed += nCurrentPlayCursor - mLastStreamPosition;
	mLastStreamPosition = nCurrentPlayCursor;

	// Skip if we are not yet playing the last most recently written block
	if ( mStreamDataPlayed + mNotifySize < mStreamDataWritten )
		return false;
	
    // First make sure that we have focus / a valid and available audio buffer
    if ( !restoreAudioBuffer( bWasRestored ) )
        return false;

    // If the buffer was restored, we need to repopulate it with data
    if ( bWasRestored )
    {
        if ( !populateAudioBuffer() )
            return false;
    
    } // End if buffer was restored

    // Lock the buffer for the 'notification' area that we must update next
    if ( FAILED( mBuffer->Lock( mNextWriteOffset, mNotifySize, (void**)&pLockedBuffer, &nLockSize, (void**)&pLockedBuffer2, &nLockSize2, 0) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to lock sound buffer during stream update for resource '%s' (Flags: 0x%x, Lock Size: %i bytes, Lock Offset: %i bytes).\n"), getResourceName().c_str(), mCreationFlags, mNotifySize, mNextWriteOffset );
        return false;

    } // End if failed to lock
    
    // Since the size of the buffer and the next write offset are both multiples of the notify size
    // the second buffer area (wrap around) should never be valid. if it is, something went horribly wrong.
    if ( pLockedBuffer2 )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unexpected lock results were returned during stream update for resource '%s' (Flags: 0x%x, Lock Size: %i bytes, Lock Offset: %i bytes).\n"), getResourceName().c_str(), mCreationFlags, mNotifySize, mNextWriteOffset );
        
        // Clean up and bail
        mBuffer->Unlock( pLockedBuffer, nLockSize, pLockedBuffer2, nLockSize2 );
        return false;
    
    } // End if unexpected lock results
    
    // We'll need to convert the data in the read buffer to mono if it has two channels
    // and this is a positional audio buffer (3D audio requires mono buffer).
    bool bStereoToMono = (supportsMode(cgAudioBufferFlags::Positional) && mInputSource.format.channels == 2);
    cgByte * pReadBuffer = pLockedBuffer;
    cgUInt32 nReadAmount = nLockSize;
    if ( bStereoToMono )
    {
        // Data is twice the size of that in the locked buffer.
        nReadAmount *= 2;
        pReadBuffer = new cgByte[nReadAmount];

    } // End if requires mixing

    // Have we finished playing the file yet (if looping this should never happen)?
    if ( !mPlaybackComplete )
    {
        // Read the required number of bytes into the buffer
        for ( ; ; )
        {
            nBytesRead = mCodec->decodePCM( pReadBuffer, nReadAmount );
            if ( nBytesRead == cgAudioCodec::ReadError_Retry )
                continue;
            break;

        } // Next read attempt 

        // If an error occurred reading data (i.e. corrupt audio) then
        // we should fill the area with silence and simply mark it all as read
        if ( nBytesRead < 0 )
        {
            FillMemory( pReadBuffer, nReadAmount, (mInputSource.format.bitsPerSample == 8) ? 128 : 0 );
            nBytesRead = nReadAmount;
        
        } // End if read failure
    
    } // End if real data
    else
    {
        // No more audio data, fill with silence
        FillMemory( pReadBuffer, nReadAmount, (mInputSource.format.bitsPerSample == 8) ? 128 : 0 );
        nBytesRead = nReadAmount;

    } // End if silence
    
    // If we read less bytes than we actually needed to update in the buffer
    // then we have reached the end of the file and need to handle this case
    if ( nBytesRead < (signed)nReadAmount )
    {
        // We're not looping?
        if ( mLooping == false )
        {
            // Fill with silence for the remainder of the buffer
            FillMemory( &pReadBuffer[nBytesRead], nReadAmount - nBytesRead, (mInputSource.format.bitsPerSample == 8) ? 128 : 0 );

            // We're done, and we should simply play silence from now on.
            if ( mPlaybackComplete == false )
                mFinalWriteOffset = mNextWriteOffset + ((bStereoToMono) ? nBytesRead / 2 : nBytesRead);
            mPlaybackComplete = true;
        
        } // End if not looping
        else
        {
            // Keep reading until we run out of data. Remember, if the file is VERY small
            // then we may actually need to read it in several times to fill the notification
            // area.
            cgUInt32 nTotalBytesRead = nBytesRead;
            for ( ; nTotalBytesRead < nReadAmount; )
            {  
                // Reset the audio stream to start from the beginning again
                mCodec->reset();

                // Read the required number of bytes into the buffer
                for ( ; ; )
                {
                    nBytesRead = mCodec->decodePCM( pReadBuffer + nTotalBytesRead, nReadAmount - nTotalBytesRead );
                    if ( nBytesRead == cgAudioCodec::ReadError_Retry )
                        continue;
                    break;

                } // Next read attempt 

                // If an error occurred reading data (i.e. corrupt audio) then
                // we should fill the area with silence and simply mark it all as read
                if ( nBytesRead < 0 )
                {
                    FillMemory( pReadBuffer + nTotalBytesRead, nReadAmount - nTotalBytesRead, (mBufferFormat.bitsPerSample == 8) ? 128 : 0 );
                    nBytesRead = nReadAmount - nTotalBytesRead;

                } // End if read failure

                // Increment our running total
                nTotalBytesRead += nBytesRead;

            } // Next test for full population

        } // End if we are looping
    
    } // Finished reading from file?

    // Remix to mono if required.
    if ( bStereoToMono )
    {
        // Mix the two channels into one
        if ( mInputSource.format.bitsPerSample == 16 )
            cgAudioDriver::PCM16StereoToMono( (cgInt16*)pLockedBuffer, (cgInt16*)pReadBuffer, nReadAmount / 4 ); // (2 bytes per sample, per channel)
        else
            cgAudioDriver::PCM8StereoToMono( pLockedBuffer, pReadBuffer, nReadAmount / 2 ); // (1 byte per sample, per channel)

    } // End if remix

    // Clean up
    if ( pReadBuffer != pLockedBuffer )
        delete []pReadBuffer;

    // Unlock the buffer.
    mBuffer->Unlock( pLockedBuffer, nLockSize, pLockedBuffer2, nLockSize2 );
    
    // Move the write offset ahead (wrap it round once we pass the end of the buffer)
    mNextWriteOffset   += nLockSize;
    mNextWriteOffset   %= mBufferSize;
	mStreamDataWritten += nLockSize;
    
    // Success!
    return true;
}