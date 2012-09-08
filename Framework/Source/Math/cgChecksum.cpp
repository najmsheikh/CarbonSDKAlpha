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
// File : cgChecksum.cpp                                                     //
//                                                                           //
// Desc : Checksum generation utility classes.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgChecksum Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgChecksum.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgChecksum;

///////////////////////////////////////////////////////////////////////////////
// Static Member Definitions
///////////////////////////////////////////////////////////////////////////////
cgUInt SHA1::mSequence[80];

///////////////////////////////////////////////////////////////////////////////
// SHA1 Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : SHA1() (Constructor)
/// <summary>Default class constructor.</summary>
//-----------------------------------------------------------------------------
SHA1::SHA1( )
{
    mLengthLow = 0;    // Message length in bits (low)
    mLengthHigh = 0;   // Message length in bits (high)
    mBlockIndex = 0;   // Index into message block array
    
    // Initialize the digest
    mDigest[0] = 0x67452301;
    mDigest[1] = 0xEFCDAB89;
    mDigest[2] = 0x98BADCFE;
    mDigest[3] = 0x10325476;
    mDigest[4] = 0xC3D2E1F0;
}

//-----------------------------------------------------------------------------
// Name : beginMessage()
/// <summary>
/// Clear out the existing digest and prepare for a new set of checksum
/// computations.
/// </summary>
//-----------------------------------------------------------------------------
void SHA1::beginMessage( )
{
    mLengthLow = 0;    // Message length in bits (low)
    mLengthHigh = 0;   // Message length in bits (high)
    mBlockIndex = 0;   // Index into message block array
    
    // Initialize the digest
    mDigest[0] = 0x67452301;
    mDigest[1] = 0xEFCDAB89;
    mDigest[2] = 0x98BADCFE;
    mDigest[3] = 0x10325476;
    mDigest[4] = 0xC3D2E1F0;
}

//-----------------------------------------------------------------------------
// Name : messageData()
/// <summary>
/// Add data to the message from which the SHA-1 cryptographic hash will be
/// computed.
/// </summary>
//-----------------------------------------------------------------------------
bool SHA1::messageData( const void * pMessage, size_t nLength )
{
    // Compute the SHA-1 for input data.
    for ( size_t i = 0; i < nLength; ++i )
    {
        mMessageBlock[mBlockIndex++] = ((cgByte*)pMessage)[i];

        // Increment message length in bits (part of the hash)
        mLengthLow += 8;
        mLengthLow &= 0xFFFFFFFF;
        if ( mLengthLow == 0 )
        {
            mLengthHigh++;
            mLengthHigh &= 0xFFFFFFFF;
            if (mLengthHigh == 0 )
            {
                // Message is too long, hash is corrupt.
                return false;
            
            } // End if corrupt

        } // End if overflow

        // If the block is full (all 512 bits), we need to process.
        if ( mBlockIndex == 64 )
        {
            processBlock( mMessageBlock, &mDigest[0], mSequence );
            mBlockIndex = 0;

        } // End if process full block

    } // Next byte

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : endMessage()
/// <summary>
/// Perform final computations and append the message length to the digest.
/// </summary>
//-----------------------------------------------------------------------------
void SHA1::endMessage( )
{
    // Pad the message up to the 64 byte boundary (if necessary) in order
    // to process the remaining block data. We also want to encode the message
    // length at the end as necessary so first we need to check to see if there
    // is enough room at the end of the current block for the length data.
    if ( mBlockIndex > 55 )
    {
        // There is not enough room. Pad out the rest of the current block.
        mMessageBlock[mBlockIndex++] = 0x80;
        while ( mBlockIndex < 64)
            mMessageBlock[mBlockIndex++] = 0;
        
        // Update the hash based on this block.
        processBlock( mMessageBlock, &mDigest[0], mSequence );
        mBlockIndex = 0;
    
        // Generate a new (empty) block up to the start of 
        // the length data to follow (byte 57 through 64).
        while ( mBlockIndex < 56 )
            mMessageBlock[mBlockIndex++] = 0;
    
    } // End if not enough space for length
    else
    {
        // There is enough space for the length so 
        // just pad up to the start of the length data
        // to follow (byte 57 through 64)
        mMessageBlock[mBlockIndex++] = 0x80;
        while ( mBlockIndex < 56 )
            mMessageBlock[mBlockIndex++] = 0;
    
    } // End if enough space

    // Store the message length
    mMessageBlock[56] = (mLengthHigh >> 24) & 0xFF;
    mMessageBlock[57] = (mLengthHigh >> 16) & 0xFF;
    mMessageBlock[58] = (mLengthHigh >> 8) & 0xFF;
    mMessageBlock[59] = (mLengthHigh) & 0xFF;
    mMessageBlock[60] = (mLengthLow >> 24) & 0xFF;
    mMessageBlock[61] = (mLengthLow >> 16) & 0xFF;
    mMessageBlock[62] = (mLengthLow >> 8) & 0xFF;
    mMessageBlock[63] = (mLengthLow) & 0xFF;

    // Process final block.
    processBlock( mMessageBlock, &mDigest[0], mSequence );
}

//-----------------------------------------------------------------------------
//  Name : processBlock () (Private)
/// <summary>
/// Internal method for processing the currently populated 64 byte (512) 
/// message block when constructing an SHA1 hash.
/// </summary>
//-----------------------------------------------------------------------------
void SHA1::processBlock( cgByte MessageBlock[], cgUInt32 Hash[], cgUInt Sequence[] )
{
    static const cgUInt Constants[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    cgUInt nTemp;
    cgInt  i;

    // Initialize the first 16 words in the sequence array.
    for ( i = 0; i < 16; ++i )
    {
        Sequence[i]  = ((unsigned)MessageBlock[i * 4]) << 24;
        Sequence[i] |= ((unsigned)MessageBlock[i * 4 + 1]) << 16;
        Sequence[i] |= ((unsigned)MessageBlock[i * 4 + 2]) << 8;
        Sequence[i] |= ((unsigned)MessageBlock[i * 4 + 3]);
    
    } // Next Word

    for ( i = 16; i < 80; ++i )
        Sequence[i] = circularShift( 1, Sequence[i-3] ^ Sequence[i-8] ^ Sequence[i-14] ^ Sequence[i-16] );

    // Extract current state
    cgUInt A = Hash[0];
    cgUInt B = Hash[1];
    cgUInt C = Hash[2];
    cgUInt D = Hash[3];
    cgUInt E = Hash[4];

    // Update hash based on sequence
    for ( i = 0; i < 20; ++i )
    {
        nTemp = circularShift(5,A) + ((B & C) | ((~B) & D)) + E + Sequence[i] + Constants[0];
        nTemp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = nTemp;
    }

    for ( i = 20; i < 40; ++i )
    {
        nTemp = circularShift(5,A) + (B ^ C ^ D) + E + Sequence[i] + Constants[1];
        nTemp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = nTemp;
    }

    for ( i = 40; i < 60; ++i )
    {
        nTemp = circularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + Sequence[i] + Constants[2];
        nTemp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = nTemp;
    }

    for ( i = 60; i < 80; ++i )
    {
        nTemp = circularShift(5,A) + (B ^ C ^ D) + E + Sequence[i] + Constants[3];
        nTemp &= 0xFFFFFFFF;
        E = D;
        D = C;
        C = circularShift(30,B);
        B = A;
        A = nTemp;
    }

    // Write update state.
    Hash[0] = (Hash[0] + A) & 0xFFFFFFFF;
    Hash[1] = (Hash[1] + B) & 0xFFFFFFFF;
    Hash[2] = (Hash[2] + C) & 0xFFFFFFFF;
    Hash[3] = (Hash[3] + D) & 0xFFFFFFFF;
    Hash[4] = (Hash[4] + E) & 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : getHash ()
/// <summary>
/// Retrieve a copy of the computed message digest / hash.
/// </summary>
//-----------------------------------------------------------------------------
void SHA1::getHash( cgUInt32 Hash[] ) const
{
    memcpy( Hash, mDigest, sizeof(mDigest) );
}