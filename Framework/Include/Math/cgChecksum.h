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
// File : cgChecksum.h                                                       //
//                                                                           //
// Desc : Checksum generation utility classes.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCHECKSUM_H_ )
#define _CGE_CGCHECKSUM_H_

//-----------------------------------------------------------------------------
// cgChecksum Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

namespace cgChecksum
{
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : SHA1 (Class)
    /// <summary>
    /// Compute the SHA-1 cryptographic hash for arbitrary data.
    /// </summary>
    //-------------------------------------------------------------------------
    class CGE_API SHA1
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
        SHA1( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void    beginMessage    ( );
        bool    messageData     ( const void * message, size_t length );
        void    endMessage      ( );
        void    getHash         ( cgUInt32 hash[] ) const;
        
    private:
        //---------------------------------------------------------------------
        // Private Methods
        //---------------------------------------------------------------------
        void    processBlock    ( cgByte messageBlock[], cgUInt32 hash[], cgUInt sequence[] );

        //-------------------------------------------------------------------------
        // Private Inline Static Functions
        //-------------------------------------------------------------------------
        static inline cgUInt circularShift( cgInt bits, cgUInt word )
        {
            return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32-bits));
        }

        //---------------------------------------------------------------------
        // Private Variables
        //---------------------------------------------------------------------
        cgUInt      mLengthLow;         // Message length in bits (low)
        cgUInt      mLengthHigh;        // Message length in bits (high)
        cgInt       mBlockIndex;        // Index into message block array
        cgByte      mMessageBlock[64];  // 512-bit message blocks
        cgUInt32    mDigest[5];         // Current hash / message digest
        
        //---------------------------------------------------------------------
        // Private Static Variables
        //---------------------------------------------------------------------
        static cgUInt   mSequence[80];  // Allocate word sequence memory in advance (used by processBlock())
        
    }; // End Class SHA1

} // End Namespace cgChecksum

#endif // !_CGE_CGCHECKSUM_H_