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
// File : cgRandom.h                                                         //
//                                                                           //
// Desc : Random number generation utility classes.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRANDOM_H_ )
#define _CGE_CGRANDOM_H_

//-----------------------------------------------------------------------------
// cgRandom Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Math/cgMathTypes.h>

namespace cgRandom
{
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : ParkMiller (Class)
    /// <summary>
    /// Pseudo-random number generator based on the Park and Miller LCG
    /// implementation.
    /// </summary>
    //-------------------------------------------------------------------------
    class CGE_API ParkMiller
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
        ParkMiller  ( );
        ParkMiller  ( bool useStateTable );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        cgDouble    next    ( );
        cgDouble    next    ( cgDouble min, cgDouble max );
        cgUInt32    nextSeed( );
        cgUInt32    getSeed ( ) const;
        void        setSeed ( cgUInt32 seed );
        
    private:
        //---------------------------------------------------------------------
        // Private Variables
        //---------------------------------------------------------------------
        cgDoubleArray   mState;
        cgUInt32        mSeed;
        bool            mUseStateTable;
        
    }; // End Class ParkMiller

    //-------------------------------------------------------------------------
    // Name : NoiseGenerator (Class)
    /// <summary>
    /// Class responsible for generating various different types of noise such 
    /// as perlin, multi-fractal etc.
    /// </summary>
    //-------------------------------------------------------------------------
    class CGE_API NoiseGenerator
    {
    public:
        //---------------------------------------------------------------------
        // Public Enumerations
        //---------------------------------------------------------------------
        /// <summary>Noise generation method.</summary>
        enum NoiseType
        {
            Perlin
        };

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
        NoiseGenerator( );
        
        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        cgFloat             getValue        ( cgFloat x, cgFloat y ) const;
        cgFloat             getValue        ( const cgVector2 & pos ) const;
        NoiseType           getNoiseType    ( ) const;
        cgUInt32            getSeed         ( ) const;
        cgFloat             getPersistance  ( ) const;
        cgFloat             getFrequency    ( ) const;
        cgFloat             getOctaves      ( ) const;
        cgFloat             getAmplitude    ( ) const;
        void                setNoiseType    ( NoiseType type );
        void                setSeed         ( cgUInt32 seed );
        void                setPersistance  ( cgFloat value );
        void                setFrequency    ( cgFloat value );
        void                setOctaves      ( cgFloat value );
        void                setAmplitude    ( cgFloat value );
        
    protected:
        //---------------------------------------------------------------------
        // Protected Constants
        //---------------------------------------------------------------------
        static const cgInt  B  = 0x100;
        static const cgInt  BM = 0xFF;
        static const cgInt  N  = 0x1000;
        static const cgInt  NP = 12;
        static const cgInt  NM = 0xFFF;

        //---------------------------------------------------------------------
        // Protected Structures
        //---------------------------------------------------------------------
        struct NoiseTables
        {
            cgArray<cgInt>        p;
            cgArray<cgVector3>    g3;
            cgArray<cgVector2>    g2;
            cgArray<cgFloat>      g1;
            /// <summary>Exponent array. Used to store weights for each necessary noise frequency.</summary>
            cgArray<cgFloat>      exponents;
        
        }; // End Struct NoiseTables

        //---------------------------------------------------------------------
        // Protected Methods
        //---------------------------------------------------------------------
        void                initialize          ( );
        void                initializeExponents ( );
        cgFloat             generatePerlin      ( const cgVector2 & vec ) const;
        cgFloat             generateNoise3      ( const cgVector3 & vec ) const;
        cgFloat             generateNoise2      ( const cgVector2 & vec ) const;

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>Random number generator.</summary>
        ParkMiller      mRandom;
        /// <summary>Type of noise to generate.</summary>
        NoiseType       mType;
        /// <summary>Random seed to use during noise generation.</summary>
        cgUInt32        mSeed;
        /// <summary>Various noise generation parameters.</summary>
        cgFloat         mPersistance;
        cgFloat         mFrequency;
        cgFloat         mOctaves;
        cgFloat         mAmplitude;
        /// <summary>Internal lookup tables used for noise generation.</summary>
        NoiseTables     mTables;
        
        
    }; // End Class NoiseGenerator

} // End Namespace cgRandom

#endif // !_CGE_CGRANDOM_H_