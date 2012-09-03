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
// Name : cgAudioCodec_Ogg.h                                                 //
//                                                                           //
// Desc : Provides support for loading and streaming audio data from ogg     //
//        vorbis (.OGG) files.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAUDIOCODEC_OGG_H_ )
#define _CGE_CGAUDIOCODEC_OGG_H_

//-----------------------------------------------------------------------------
// cgAudioCodec_Ogg Header Includes
//-----------------------------------------------------------------------------
#include <Audio/cgAudioDriver.h>

//-----------------------------------------------------------------------------
// libVorbis - Ogg Vorbis (.OGG) support.
//-----------------------------------------------------------------------------
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAudioCodec_Ogg (Class)
/// <summary>
/// Codec class implementation providing support for decoding OGG files.
/// </summary>
//-----------------------------------------------------------------------------
class cgAudioCodec_Ogg : public cgAudioCodec
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAudioCodec_Ogg( );
    virtual ~cgAudioCodec_Ogg( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgAudioCodec)
    //-------------------------------------------------------------------------
    virtual bool    isValid     ( cgInputStream & stream );
    virtual bool    open        ( cgInputStream & stream );
    virtual bool    reset       ( );
    virtual void    close       ( );
    virtual bool    getPCMFormat( cgAudioBufferFormat & format );
    virtual cgInt32 decodePCM   ( cgByte * buffer, cgUInt32 bufferLength );

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    OggVorbis_File      mVorbisFile;        // The ogg vorbis file identifier (libVorbis)
    FILE              * mFile;              // A handle to the physical file on disk.
    bool                mVorbisOpened;      // Has libVorbis opened the bitstream?
    int                 mCurrentSection;    // Section information for bitstream reading.
};

#endif // !_CGE_CGAUDIOCODEC_OGG_H_