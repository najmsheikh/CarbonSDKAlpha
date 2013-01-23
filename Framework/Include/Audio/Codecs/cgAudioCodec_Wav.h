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
// Name : cgAudioCodec_Wav.h                                                 //
//                                                                           //
// Desc : Provides support for loading and streaming audio data from wave    //
//        (.WAV) files.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAUDIOCODEC_WAV_H_ )
#define _CGE_CGAUDIOCODEC_WAV_H_

//-----------------------------------------------------------------------------
// cgAudioCodec_Wav Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Audio/cgAudioDriver.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // Warning: Portability
#include <mmsystem.h>    // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct _MMCKINFO;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAudioCodec_Wav (Class)
/// <summary>
/// Codec class implementation providing support for decoding WAV files.
/// </summary>
//-----------------------------------------------------------------------------
class cgAudioCodec_Wav : public cgAudioCodec
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAudioCodec_Wav( );
    virtual ~cgAudioCodec_Wav( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool            readMMIO    ( void * input, cgAudioBufferFormat & formatOut );

    //-------------------------------------------------------------------------
    // Public Virtual Functions (Overrides cgAudioCodec)
    //-------------------------------------------------------------------------
    virtual bool    isValid     ( cgInputStream & stream );
    virtual bool    open        ( cgInputStream & stream );
    virtual bool    reset       ( );
    virtual void    close       ( );
    virtual bool    getPCMFormat( cgAudioBufferFormat & format );
    virtual cgInt32 decodePCM   ( cgByte * buffer, cgUInt32 bufferLength );

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static LRESULT WINAPI ioProc( LPSTR lpmmioinfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2 );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    void              * mInput;         // A handle to the Multimedia IO object
    _MMCKINFO         * mChunkInfo;     // Multimedia RIFF chunk info
    _MMCKINFO         * mRIFFInfo;      // Used in opening a WAVE file
    cgAudioBufferFormat mPCMFormat;     // The format read from the file
    cgInputStream       mFile;          // File from which data is being read.
};

#endif // !_CGE_CGAUDIOCODEC_WAV_H_