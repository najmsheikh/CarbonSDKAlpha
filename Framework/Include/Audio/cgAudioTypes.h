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
// Name : cgAudioTypes.h                                                     //
//                                                                           //
// Desc : Common system file that defines various audio types and common     //
//        enumerations.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAUDIOTYPES_H_ )
#define _CGE_CGAUDIOTYPES_H_

//-----------------------------------------------------------------------------
// cgAudioTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
namespace cgAudioBufferFlags
{
    enum Base
    {
        Simple              = 0x0,
        Streaming           = 0x1,
        Positional          = 0x2,
        AllowPan            = 0x4,
        AllowVolume         = 0x8,
        AllowPitch          = 0x10,
        MuteAtMaxDistance   = 0x20,

        Complex             = AllowPan | AllowVolume | AllowPitch,
        Complex3D           = Positional | AllowVolume | AllowPitch
    };

}; // End Namespace : cgAudioBufferFlags

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
struct cgAudioBufferFormat
{
    cgUInt16    formatType;             // Format type
    cgUInt16    channels;               // Number of channels (i.e. mono, stereo, etc.)
    cgUInt32    samplesPerSecond;       // Sample rate
    cgUInt32    averageBytesPerSecond;  // For buffer estimation
    cgUInt16    blockAlign;             // Block size of data
    cgUInt16    bitsPerSample;          // Number of bits per sample of mono data
    cgUInt16    size;                   // Count in bytes of the size of extra information (after this member).

}; // End Struct : cgAudioBufferFormat

#endif // !_CGE_CGAUDIOTYPES_H_