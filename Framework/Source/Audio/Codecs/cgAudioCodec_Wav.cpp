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
// Name : cgAudioCodec_Wav.cpp                                               //
//                                                                           //
// Desc : Provides support for loading and streaming audio data from wave    //
//        (.WAV) files.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAudioCodec_Wav Module Includes
//-----------------------------------------------------------------------------
#include <Audio/Codecs/cgAudioCodec_Wav.h>

//-----------------------------------------------------------------------------
//  Name : cgAudioCodec_Wav () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioCodec_Wav::cgAudioCodec_Wav()
{
    // Initialize variables to sensible defaults
    mInput     = CG_NULL;
    mChunkInfo = new MMCKINFO();
    mRIFFInfo  = new MMCKINFO();
}

//-----------------------------------------------------------------------------
//  Name : ~cgAudioCodec_Wav () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioCodec_Wav::~cgAudioCodec_Wav()
{
    // Close any opened files
    close();

    // Release memory
    delete mChunkInfo;
    delete mRIFFInfo;

    // Clear variables
    mChunkInfo = CG_NULL;
    mRIFFInfo  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : isValid ()
/// <summary>
/// Determine if the specified file is of a valid format
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Wav::isValid( cgInputStream & Stream )
{
    HMMIO         hInput;
    cgAudioBufferFormat Format;
    bool          bValid;

    // Open the specified file for reading
    cgInputStream TestStream = Stream;
    if ( !TestStream.open() )
        return false;

    // Attempt to open the multimedia IO stream.
    MMIOINFO Info;
    memset( &Info, 0, sizeof(MMIOINFO) );
    Info.pIOProc = ioProc;
    ((cgInputStream*&)Info.adwInfo[0]) = &TestStream;
    hInput = mmioOpenA( CG_NULL, &Info, MMIO_ALLOCBUF | MMIO_READ );
    if ( !hInput )
        return false;

    // Read the header information
    bValid = readMMIO( hInput, Format );

    // Close file
    mmioClose( hInput, 0 );
    TestStream.close();

    // Valid?
    return bValid;
}

//-----------------------------------------------------------------------------
//  Name : ioProc () (Private, Static)
/// <summary>
/// Callback mechanism for MMIO system that allows us to read from a custom
/// data stream.
/// </summary>
//-----------------------------------------------------------------------------
LRESULT WINAPI cgAudioCodec_Wav::ioProc( LPSTR lpmmioinfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2 )
{
    MMIOINFO * Info = (MMIOINFO*)lpmmioinfo;

    // Get the pointer to the stream.
    cgInputStream & Stream = *((cgInputStream*&)Info->adwInfo[0]);
    
    // What operation are we being asked to perform?
    switch ( uMsg )
    {
        case MMIOM_OPEN:
            Stream.seek( 0, cgInputStream::Begin );
            Info->lDiskOffset = 0;
            return MMSYSERR_NOERROR;

        case MMIOM_CLOSE:
            return MMSYSERR_NOERROR;

        case MMIOM_READ:
        {
            size_t BytesRead = Stream.read( (void*)lParam1, (size_t)((LPARAM)lParam2) );
            Info->lDiskOffset = (cgInt32)Stream.getPosition();
            return (LRESULT)BytesRead;
        
        } // End case READ
        case MMIOM_SEEK:
        {
            cgInputStream::SeekOrigin origin;
            switch ( (LPARAM)lParam2 )
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
            if ( !Stream.seek( (cgInt64)((LPARAM)lParam1), origin ) )
                return -1;
            Info->lDiskOffset = (cgInt32)Stream.getPosition();
            return (LRESULT)Info->lDiskOffset;
        
        } // End case SEEK
        case MMIOM_WRITE:
            // Unsupported
            break;

        case MMIOM_WRITEFLUSH:
            // Unsupported
            break;

    } // End switch message

    // Unrecognized message.
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : open ()
/// <summary>
/// Open the specified audio file from disk (assumes it has already
/// been validated with a call to 'isValid')
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Wav::open( cgInputStream & Stream )
{
    // Already open?
    if ( mInput )
        close();

    // Open the specified file for reading
    mFile = Stream;
    if ( !mFile.open() )
        return false;

    // Attempt to open the multimedia IO stream.
    MMIOINFO Info;
    memset( &Info, 0, sizeof(MMIOINFO) );
    Info.pIOProc = ioProc;
    ((cgInputStream*&)Info.adwInfo[0]) = &mFile;
    mInput = mmioOpenA( CG_NULL, &Info, MMIO_ALLOCBUF | MMIO_READ );
    if ( !mInput )
    {
        mFile.close();
        return false;
    
    } // End if failed

    // Read the header information
    if ( !readMMIO( mInput, mPCMFormat ) )
    {
        close();
        return false;
    
    } // End if failed
    
    // Reset the file ready for reading
    if( !reset() )
    {
        close();
        return false;
    
    } // End if failed
    
    // Opened successfully
    return true;
}

//-----------------------------------------------------------------------------
//  Name : close ()
/// <summary>
/// Close any file handles currently open.
/// </summary>
//-----------------------------------------------------------------------------
void cgAudioCodec_Wav::close( )
{
    // Close opened files
    if ( mInput )
        mmioClose( (HMMIO)mInput, 0 );
    mFile.close();

    // Clear variables
    mInput = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getPCMFormat ()
/// <summary>
/// Retrieve the format of the currently opened audio data in the PCM
/// format into which it will ultimately be decoded.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Wav::getPCMFormat( cgAudioBufferFormat & Format )
{
    // Validate requirements
    if ( !mInput ) return false;

    // Build the format structure for our audio buffer
    Format = mPCMFormat;
    
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
cgInt32 cgAudioCodec_Wav::decodePCM( cgByte * pBuffer, cgUInt32 BufferLength )
{
    MMIOINFO InputInfo;
    cgUInt32 nBytesRead = 0, nDataToRead = 0, i;

    // Validate requirements
    if ( mInput == CG_NULL )
        return ReadError_Abort;

    // Get IO info
    if ( mmioGetInfo( (HMMIO)mInput, &InputInfo, 0 ) != 0 )
        return ReadError_Abort;

    // Calculate the amount of data to read, and then subtract that from the data remaining
    nDataToRead = min( mChunkInfo->cksize, BufferLength );
    mChunkInfo->cksize -= nDataToRead;

    // Read data
    for( i = 0; i < nDataToRead; ++i )
    {
        // Copy the bytes from the io to the buffer.
        if( InputInfo.pchNext == InputInfo.pchEndRead )
        {
            // Attempt to advance, or fail
            if ( mmioAdvance( (HMMIO)mInput, &InputInfo, MMIO_READ ) != 0 )
                return ReadError_Abort;
            if ( InputInfo.pchNext == InputInfo.pchEndRead )
                return ReadError_Abort;

        } // End if reached the end

        // Actual copy of data.
        *(pBuffer + i) = *((cgByte*)InputInfo.pchNext);
        InputInfo.pchNext++;
    
    } // Next byte

    // Set the information
    if ( mmioSetInfo( (HMMIO)mInput, &InputInfo, 0 ) != 0 )
        return ReadError_Abort;

    // Return the actual amount of bytes read.
    return nDataToRead;
}

//-----------------------------------------------------------------------------
//  Name : readMMIO ()
/// <summary>
/// Support function for reading from a multimedia IO stream.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Wav::readMMIO( void * hInput, cgAudioBufferFormat & OutFormat )
{
    MMCKINFO        ChunkInfo;
    PCMWAVEFORMAT   pcmWaveFormat;

    // Descend into initial chunk
    if ( mmioDescend( (HMMIO)hInput, mRIFFInfo, CG_NULL, 0 ) != 0 )
        return false;

    // Check to make sure this is a valid wave file
    if ( ( mRIFFInfo->ckid != FOURCC_RIFF ) || ( mRIFFInfo->fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
        return false;

    // Search the input file for for the 'fmt ' chunk.
    ChunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if ( mmioDescend( (HMMIO)hInput, &ChunkInfo, mRIFFInfo, MMIO_FINDCHUNK ) != 0 )
        return false;

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
    if ( ChunkInfo.cksize < (cgInt32)sizeof(PCMWAVEFORMAT) ) return false;

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if ( mmioRead( (HMMIO)hInput, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        return false;

    // Populate the cgAudioBufferFormat structure
    memcpy( &OutFormat, &pcmWaveFormat, sizeof(pcmWaveFormat) );
    OutFormat.size = 0;

    // Ascend the input file out of the 'fmt ' chunk.
    if( mmioAscend( (HMMIO)hInput, &ChunkInfo, 0 ) != 0 )
        return false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : reset ()
/// <summary>
/// Resets the internal pointer so that our next read operations start
/// at the beginning of the actual audio data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAudioCodec_Wav::reset()
{
    // Validate requirements
    if( mInput == CG_NULL )
        return false;

    // Seek to the data
    if( mmioSeek( (HMMIO)mInput, mRIFFInfo->dwDataOffset + sizeof(FOURCC), SEEK_SET ) == -1 )
        return false;

    // Search the input file for the 'data' chunk.
    mChunkInfo->ckid = mmioFOURCC('d', 'a', 't', 'a');
    if ( mmioDescend( (HMMIO)mInput, mChunkInfo, mRIFFInfo, MMIO_FINDCHUNK ) != 0 )
        return false;

    // Success!
    return true;
}