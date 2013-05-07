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
// Name : cgAudioCodec_Ogg.cpp                                               //
//                                                                           //
// Desc : Provides support for loading and streaming audio data from ogg     //
//        vorbis (.OGG) files.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAudioCodec_Ogg Module Includes
//-----------------------------------------------------------------------------
#include <Audio/Codecs/cgAudioCodec_Ogg.h>

//-----------------------------------------------------------------------------
//  Name : cgAudioCodec_Ogg () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioCodec_Ogg::cgAudioCodec_Ogg()
{
    // Initialize variables to sensible defaults
    mVorbisOpened   = false;
    mCurrentSection = 0;

    // Clear structures
    memset( &mVorbisFile, 0, sizeof(OggVorbis_File) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgAudioCodec_Ogg () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioCodec_Ogg::~cgAudioCodec_Ogg()
{
    // Close any opened files
    close();
}

//-----------------------------------------------------------------------------
//  Name : isValid ()
/// <summary>
/// Determine if the specified file is of a valid format
/// Note : Should not affect any already opened file!
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Ogg::isValid( cgInputStream & Stream )
{
    FILE         * pFile = CG_NULL;
    OggVorbis_File VorbisFile;
    bool           bValid = true;

    // Open the specified file (read mode!)
    cgInputStream TestStream = Stream;
    if ( !TestStream.open() )
        return false;

    // Attempt to open the bitstream
    ov_callbacks Callbacks;
    Callbacks.read_func = readStream;
    Callbacks.seek_func = seekStream;
    Callbacks.tell_func = tellStream;
    Callbacks.close_func = closeStream;
    if ( ov_test_callbacks( &TestStream, &VorbisFile, CG_NULL, 0, Callbacks ) < 0 )
        bValid = false;
    
    // Finish up
    if ( bValid )
        ov_clear( &VorbisFile );
    TestStream.close();

    // Valid?
    return bValid;
}

//-----------------------------------------------------------------------------
//  Name : readStream() (Private, Static)
/// <summary>
/// Callback to read data from the opened ogg vorbis stream.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgAudioCodec_Ogg::readStream( void *ptr, size_t size, size_t nmemb, void *dataSource )
{
    cgInputStream & Stream = *(cgInputStream*)dataSource;
    return Stream.read( ptr, size );
}

//-----------------------------------------------------------------------------
//  Name : seekStream() (Private, Static)
/// <summary>
/// Callback to reposition the current pointer in the opened ogg vorbis stream.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgAudioCodec_Ogg::seekStream( void *dataSource, ogg_int64_t offset, cgInt whence )
{
    cgInputStream & Stream = *(cgInputStream*)dataSource;
    cgInputStream::SeekOrigin origin;
    switch ( whence )
    {
        case SEEK_SET:
            origin = cgInputStream::Begin;
            break;
        case SEEK_CUR:
            origin = cgInputStream::Current;
            break;
        case SEEK_END:
            origin = cgInputStream::End;
            break;
    } // End switch whence
    if ( !Stream.seek( offset, origin ) )
        return -1;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : closeStream() (Private, Static)
/// <summary>
/// Callback to close the currently opened ogg vorbis stream.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgAudioCodec_Ogg::closeStream( void *dataSource )
{
    // Currently unsupported, we'll clean up.
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : tellStream() (Private, Static)
/// <summary>
/// Callback to retrieve the current position of the currently opened ogg 
/// vorbis stream.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgAudioCodec_Ogg::tellStream( void *dataSource )
{
    cgInputStream & Stream = *(cgInputStream*)dataSource;
    return (cgInt32)Stream.getPosition();
}

//-----------------------------------------------------------------------------
//  Name : open ()
/// <summary>
/// Open the specified audio file from disk (assumes it has already
/// been validated with a call to 'isValid')
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Ogg::open( cgInputStream & Stream )
{
    // Already open?
    if ( mFile.isOpen() || mVorbisOpened )
        close();

    // Open the specified file (read mode!)
    mFile = Stream;
    if ( !mFile.open() )
        return false;

    // Attempt to open the bitstream
    ov_callbacks Callbacks;
    Callbacks.read_func = readStream;
    Callbacks.seek_func = seekStream;
    Callbacks.tell_func = tellStream;
    Callbacks.close_func = closeStream;
    if ( ov_open_callbacks( &mFile, &mVorbisFile, CG_NULL, 0, Callbacks ) < 0 )
    {
        close();
        return false;
    } // End if failed
    mVorbisOpened = true;

    // Setup some values
    mCurrentSection = 0;

    // Opened succesfully
    return true;
}

//-----------------------------------------------------------------------------
//  Name : close ()
/// <summary>
/// Close any file handles currently open.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioCodec_Ogg::close( )
{
    // Close opened files
    if ( mVorbisOpened )
        ov_clear( &mVorbisFile );
    mFile.close();
    
    // Clear variables
    mVorbisOpened = false;

    // Clear structures
    memset( &mVorbisFile, 0, sizeof(OggVorbis_File) );
}

//-----------------------------------------------------------------------------
//  Name : getPCMFormat ()
/// <summary>
/// Retrieve the format of the currently opened audio data in the PCM
/// format into which it will ultimately be decoded.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Ogg::getPCMFormat( cgAudioBufferFormat & Format )
{
    // Validate requirements
    if ( !mVorbisOpened ) return false;

    // Retrieve the information structure for the file
    vorbis_info * pInfo = ov_info( &mVorbisFile, -1 );

    // Build the format structure for our DirectSound buffer
    memset( &Format, 0, sizeof(cgAudioBufferFormat) );
    Format.channels         = pInfo->channels;
    Format.bitsPerSample    = 16;   // Ogg Vorbis always uses a bitrate of 16
    Format.samplesPerSecond    = pInfo->rate;
    Format.averageBytesPerSecond   = pInfo->rate * pInfo->channels * 2;
    Format.blockAlign       = 2 * pInfo->channels;
    Format.formatType       = 1;

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : decodePCM ()
/// <summary>
/// Read the specified number of bytes from the bitstream decoded into
/// the raw PCM format.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgAudioCodec_Ogg::decodePCM( cgByte * pBuffer, cgUInt32 BufferLength )
{
    cgInt32 nBytesRead, nTotalRead = 0;

    // Validate requirements
    if ( !mVorbisOpened ) return ReadError_Abort;

    // Read all the requested data until we reach the required length, or run out of data
    for ( ; nTotalRead < (signed)BufferLength; )
    {
        // Decode from the ogg vorbis file into the specified buffer
        nBytesRead = ov_read( &mVorbisFile, (cgChar*)pBuffer + nTotalRead, BufferLength - nTotalRead, 0, 2, 1, &mCurrentSection );
        if ( nBytesRead == OV_HOLE ) return ReadError_Retry;
        if ( nBytesRead == OV_EBADLINK ) return ReadError_Abort;
        if ( nBytesRead == 0 ) return nTotalRead;

        // Some data read
        nTotalRead += nBytesRead;
    
    } // Next block read

    // Return the total amount of data read
    return nTotalRead;
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Resets the internal pointer so that our next read operations start
/// at the beginning of the actual audio data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Ogg::reset()
{
    // Validate requirements
    if( !mVorbisOpened ) return false;

    // Seek to the absolute beginning of the audio data
    if ( ov_pcm_seek( &mVorbisFile, 0 ) ) return false;

    // Success!
    return true;
}