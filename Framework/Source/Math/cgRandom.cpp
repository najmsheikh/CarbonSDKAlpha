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
// File : cgRandom.cpp                                                       //
//                                                                           //
// Desc : Random number generation utility classes.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRandom Module Includes
//-----------------------------------------------------------------------------
#include <Math/cgRandom.h>
#include <math.h>
#include <sys/timeb.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgRandom;

///////////////////////////////////////////////////////////////////////////////
// ParkMiller Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : ParkMiller() (Constructor)
/// <summary>Default class constructor.</summary>
//-----------------------------------------------------------------------------
ParkMiller::ParkMiller( )
{
    // Initialize variables to sensible defaults
    mUseStateTable = true;

    // Initialize with default seed (millisecond system time)
    setSeed( 0 );
}

//-----------------------------------------------------------------------------
// Name : ParkMiller() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
ParkMiller::ParkMiller( bool bUseStateTable )
{
    // Initialize variables to sensible defaults
    mUseStateTable = bUseStateTable;

    // Initialize with default seed (millisecond system time)
    setSeed( 0 );
}

//-----------------------------------------------------------------------------
// Name : next()
/// <summary>Retrieve the next value in the sequence.</summary>
//-----------------------------------------------------------------------------
cgDouble ParkMiller::next( )
{
    return next( 0.0, 1.0 );
}

//-----------------------------------------------------------------------------
// Name : next()
/// <summary>
/// Retrieve the next value in the sequence scaled into the specified range.
/// </summary>
//-----------------------------------------------------------------------------
cgDouble ParkMiller::next( cgDouble min, cgDouble max )
{
    cgDouble fResult;

    if ( mUseStateTable == true )
    {
        // Get next result from state table.
        cgUInt32 nSeed   = nextSeed();
        cgDouble fRandom = ((cgDouble)nSeed * 4.6566128750000002e-010);
        cgInt    nIndex  = (cgInt)(fRandom * (cgDouble)mState.size());
        fResult          = mState[nIndex];

        // Refresh this entry in the state table
        nSeed   = nextSeed();
        fRandom = ((cgDouble)nSeed * 4.6566128750000002e-010);
        mState[nIndex] = fRandom;
    
    } // End if accumulate state
    else
    {
        cgUInt32 nSeed = nextSeed();
        fResult =((cgDouble)nSeed * 4.6566128750000002e-010);

    } // End if no state

    // Return result, scaled into supplied range
    return min + (fResult * (max-min));
}

//-----------------------------------------------------------------------------
// Name : nextSeed()
/// <summary>
/// Retrieve the next seed in the sequence (Park and Miller).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ParkMiller::nextSeed()
{
    cgInt64 const a = 16807;
    cgInt64 const m = 2147483647;
    mSeed = (cgUInt32)(((cgInt64)mSeed * a) % m );
    return mSeed;
}

//-----------------------------------------------------------------------------
// Name : getSeed ( )
/// <summary>
/// Get the current seed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ParkMiller::getSeed( ) const
{
    return mSeed;
}

//-----------------------------------------------------------------------------
// Name : setSeed ( )
/// <summary>
/// Set the current seed.
/// </summary>
//-----------------------------------------------------------------------------
void ParkMiller::setSeed( cgUInt32 value )
{
    if ( value == 0 )
    {
        // Select default seed as system tick count
        // NB: Portable solution but rolls over ever 12 days or so.
        timeb tb;
        ftime( &tb );
        mSeed = tb.millitm + (tb.time & 0xfffff) * 1000;

    } // End if default
    else
    {
        // Store new seed.
        mSeed = value;

    } // End if specified.

    // Refresh state tables
    if ( mUseStateTable == true )
    {
        if ( mState.empty() == true )
            mState.resize( 128 );
        for ( size_t i = 0; i < 128; ++i )
            mState[i] = ((cgDouble)nextSeed() * 4.6566128750000002e-010);
    
    } // End if use state tables
}

///////////////////////////////////////////////////////////////////////////////
// NoiseGenerator Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : NoiseGenerator() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
NoiseGenerator::NoiseGenerator( )
{
    // Store the initial seed used to generate the random numbers.
    // (The random number generator will change its internal seed
    // each time a new number is generated).
    mSeed = mRandom.getSeed();

    // Initialize tables based on this random seed.
    initialize();

    // Select default type to initialize any additional properties
    // required (also initializes parameters to good defaults).
    setNoiseType( Perlin );
}

//-----------------------------------------------------------------------------
// Name : initialize( ) (Protected)
/// <summary>
/// Initialize noise generation tables.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::initialize()
{
    cgInt i, j, k;

    // Grab references to tables just to make things a little more readable.
    std::vector<cgInt>     & p  = mTables.p;
    std::vector<cgVector3> & g3 = mTables.g3;
    std::vector<cgVector2> & g2 = mTables.g2;
    std::vector<cgFloat>   & g1 = mTables.g1;

    // Clear original tables.
    p.clear();
    g3.clear();
    g2.clear();
    g1.clear();
        
    // Allocate tables
    p.resize( B + B + 2 );
    g3.resize( B + B + 2 );
    g2.resize( B + B + 2 );
    g1.resize( B + B + 2 );
    
    // Generate tables
    for ( i = 0; i < B; ++i )
	{
		p[i]  = i;
		g1[i] = (cgFloat)(mRandom.next() * 2.0 - 1.0);  // -1.0 to 1.0
		g2[i] = cgVector2( (cgFloat)(mRandom.next() * 2.0 - 1.0), 
                           (cgFloat)(mRandom.next() * 2.0 - 1.0) );
		cgVector2::normalize( g2[i], g2[i] );
        g3[i] = cgVector3( (cgFloat)(mRandom.next() * 2.0 - 1.0), 
                           (cgFloat)(mRandom.next() * 2.0 - 1.0),
                           (cgFloat)(mRandom.next() * 2.0 - 1.0));
        cgVector3::normalize( g3[i], g3[i] );
		
	} // Next Entry

    while ((--i) > 0)
	{
		j = (cgInt)(mRandom.next() * B);
		k = p[i];
		p[i] = p[j];
		p[j] = k;
	
    } // Next Entry

    // Make symmetrical
    for ( i = 0; i < B + 2; ++i )
	{
		p[B + i]  = p[i];
		g1[B + i] = g1[i];
        g2[B + i] = g2[i];
        g3[B + i] = g3[i];
	
    } // Next Entry
}

//-----------------------------------------------------------------------------
// Name : initializeExponents( ) (Protected)
/// <summary>
/// Initialize spectral weights table.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::initializeExponents()
{
    // Precompute and store spectral weights
    mTables.exponents.clear();
    mTables.exponents.resize( (size_t)mOctaves + 1 );
    cgFloat frequency = 1.0f;
    for ( cgInt i = 0; i <= (cgInt)mOctaves; ++i )
    {
        // Compute weight for each frequency
        mTables.exponents[i] = (cgFloat)powf( (cgFloat)frequency, (cgFloat)-mPersistance );
        frequency *= mFrequency;

    } // Next Frequency
}

//-----------------------------------------------------------------------------
// Name : getValue( )
/// <summary>
/// Retrieve the noise value at the given location.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::getValue( cgFloat x, cgFloat y ) const
{
    return getValue( cgVector2( x, y ) );
}
cgFloat NoiseGenerator::getValue( const cgVector2 & vec ) const
{
    // Select the relevant type
    switch (mType)
    {
        case Perlin:
            return generatePerlin( vec );
    
    } // End switch type

    // Unknown type.
    return 0.0f;
}

//-----------------------------------------------------------------------------
// Name : generatePerlin ( ) (Protected)
/// <summary>
/// Generate perlin noise at the given location.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::generatePerlin( const cgVector2 & vec ) const
{
    // Properties of the perlin noise
    cgFloat P           = 0.5f;
    cgFloat octaves     = mOctaves;
    cgFloat amplitude   = mAmplitude;
    cgFloat freq_factor = mFrequency;
    cgFloat persistance = mPersistance;

    // Generate
    cgFloat total = 0.0f;
    for( cgInt i = 0; i < (cgInt)octaves; ++i ) 
    {
        total       += generateNoise2( cgVector2(vec.x * freq_factor, vec.y * freq_factor) ) * (amplitude + persistance);
        persistance *= P;
        amplitude   *= persistance;
        freq_factor *= 2;
    
    } // Next Frequency
    total = total * 0.5f + 0.5f;
    return total;
}

//-----------------------------------------------------------------------------
// Name : generateNoise3 ( ) (Protected)
/// <summary>
/// Generate correct noise for 3D position.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::generateNoise3( const cgVector3 & vec ) const
{
	cgInt   bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	cgFloat rx0, rx1, ry0, ry1, rz0, rz1, sy, sz, a, b, c, d, t, u, v;
	cgInt   i, j;

	t = vec.x + N;
	bx0 = ((cgInt)t) & BM;
	bx1 = (bx0+1) & BM;
	rx0 = t - (cgInt)t;
	rx1 = rx0 - 1.0f;

	t = vec.y + N;
	by0 = ((cgInt)t) & BM;
	by1 = (by0+1) & BM;
	ry0 = t - (cgInt)t;
	ry1 = ry0 - 1.0f;

	t = vec.z + N;
	bz0 = ((int)t) & BM;
	bz1 = (bz0+1) & BM;
	rz0 = t - (int)t;
	rz1 = rz0 - 1.0f;

    // Reference tables for easy access.
    const std::vector<cgInt> & p = mTables.p;
    const std::vector<cgVector3> & g3 = mTables.g3;
    
    // Generate
	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = rx0 * rx0 * (3.0f - 2.0f * rx0); // S_Curve
	sy = ry0 * ry0 * (3.0f - 2.0f * ry0); // S_Curve
	sz = rz0 * rz0 * (3.0f - 2.0f * rz0); // S_Curve
    
	u = ( rx0 * g3[ b00 + bz0 ].x + ry0 * g3[ b00 + bz0 ].y + rz0 * g3[ b00 + bz0 ].z );
	v = ( rx1 * g3[ b10 + bz0 ].x + ry0 * g3[ b10 + bz0 ].y + rz0 * g3[ b10 + bz0 ].z );
	a = u + t * (v - u); // Lerp

	u = ( rx0 * g3[ b01 + bz0 ].x + ry1 * g3[ b01 + bz0 ].y + rz0 * g3[ b01 + bz0 ].z );
	v = ( rx1 * g3[ b11 + bz0 ].x + ry1 * g3[ b11 + bz0 ].y + rz0 * g3[ b11 + bz0 ].z );
	b = u + t * (v - u); // Lerp

	c = a + sy * (b - a); // Lerp

	u = ( rx0 * g3[ b00 + bz1 ].x + ry0 * g3[ b00 + bz1 ].y + rz1 * g3[ b00 + bz1 ].z );
	v = ( rx1 * g3[ b10 + bz1 ].x + ry0 * g3[ b10 + bz1 ].y + rz1 * g3[ b10 + bz1 ].z );
	a = u + t * (v - u); // Lerp

	u = ( rx0 * g3[ b01 + bz1 ].x + ry1 * g3[ b01 + bz1 ].y + rz1 * g3[ b01 + bz1 ].z );
	v = ( rx1 * g3[ b11 + bz1 ].x + ry1 * g3[ b11 + bz1 ].y + rz1 * g3[ b11 + bz1 ].z );
	b = u + t * (v - u); // Lerp

	d = a + sy * (b - a); // Lerp

	return c + sz * (d - c); // Lerp
}

//-----------------------------------------------------------------------------
// Name : generateNoise2 ( ) (Protected)
/// <summary>
/// Generate correct noise for 2D position.
/// </summary>
//-----------------------------------------------------------------------------
float NoiseGenerator::generateNoise2( const cgVector2 & vec ) const
{
	cgInt   bx0, bx1, by0, by1, b00, b10, b01, b11;
	cgFloat rx0, rx1, ry0, ry1, sx, sy, a, b, t, u, v;
	cgInt   i, j;
	
	t = vec.x + N;
	bx0 = ((cgInt)t) & BM;
	bx1 = (bx0+1) & BM;
	rx0 = t - (cgInt)t;
	rx1 = rx0 - 1.0f;

	t = vec.y + N;
	by0 = ((cgInt)t) & BM;
	by1 = (by0+1) & BM;
	ry0 = t - (cgInt)t;
	ry1 = ry0 - 1.0f;
	
    // Reference tables for easy access.
    const std::vector<cgInt> & p = mTables.p;
    const std::vector<cgVector2> & g2 = mTables.g2;
    
    // Generate
	i = p[ bx0 ];
	j = p[ bx1 ];
	
	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];
	
	sx = rx0 * rx0 * (3.0f - 2.0f * rx0); // S_Curve
	sy = ry0 * ry0 * (3.0f - 2.0f * ry0); // S_Curve
    
	u = ( rx0 * g2[ b00 ].x + ry0 * g2[ b00 ].y );
	v = ( rx1 * g2[ b10 ].x + ry0 * g2[ b10 ].y );
	a = u + sx * (v - u); // Lerp
	
	u = ( rx0 * g2[ b01 ].x + ry1 * g2[ b01 ].y );
	v = ( rx1 * g2[ b11 ].x + ry1 * g2[ b11 ].y );
	b = u + sx * (v - u); // Lerp
	
	return a + sy * (b - a); // Lerp
}

//-----------------------------------------------------------------------------
// Name : getNoiseType ( )
/// <summary>
/// Get the type of noise to generate.
/// </summary>
//-----------------------------------------------------------------------------
NoiseGenerator::NoiseType NoiseGenerator::getNoiseType( ) const
{
    return mType;
}

//-----------------------------------------------------------------------------
// Name : setNoiseType ( )
/// <summary>
/// Set the type of noise to generate.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setNoiseType( NoiseType value )
{
    mType = value;

    // Select good defaults
    switch ( value )
    {
        case Perlin:
            mOctaves          = 8;
            mFrequency        = 0.04f;
            mPersistance      = 0.5f;
            mAmplitude        = 0.65f;
            break;

    } // End Switch Type

    // Recompute spectral weights table
    initializeExponents();
}

//-----------------------------------------------------------------------------
// Name : getSeed ( )
/// <summary>
/// Get the random seed and re-initialize noise generation tables.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 NoiseGenerator::getSeed( ) const
{
    return mSeed;
}

//-----------------------------------------------------------------------------
// Name : setSeed ( )
/// <summary>
/// Set the random seed and re-initialize noise generation tables.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setSeed( cgUInt32 value )
{
    mSeed = value;
    mRandom.setSeed( value );

    // Reinitialize all tables
    initialize();

    // Recompute spectral weights table
    initializeExponents();
}

//-----------------------------------------------------------------------------
// Name : getPersistance ( )
/// <summary>
/// Get the noise persistance.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::getPersistance( ) const
{
    return mPersistance;
}

//-----------------------------------------------------------------------------
// Name : setPersistance ( )
/// <summary>
/// Set the noise persistance.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setPersistance( cgFloat value )
{
    mPersistance = value;

    // Recompute spectral weights table
    initializeExponents();
}

//-----------------------------------------------------------------------------
// Name : getFrequency ( )
/// <summary>
/// Get the noise frequency.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::getFrequency( ) const
{
    return mFrequency;
}

//-----------------------------------------------------------------------------
// Name : setFrequency ( )
/// <summary>
/// Set the noise frequency.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setFrequency( cgFloat value )
{
    mFrequency = value;

    // Reinitialize spectral weights table
    initializeExponents();
}

//-----------------------------------------------------------------------------
// Name : getOctaves ( )
/// <summary>
/// Get the number of noise octaves.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::getOctaves( ) const
{
    return mOctaves;
}

//-----------------------------------------------------------------------------
// Name : setOctaves ( )
/// <summary>
/// Set the number of noise octaves.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setOctaves( cgFloat value )
{
    mOctaves = value;

    // Recompute spectral weights table
    initializeExponents();
}

//-----------------------------------------------------------------------------
// Name : getAmplitude ( )
/// <summary>
/// Get the noise amplitude.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat NoiseGenerator::getAmplitude( ) const
{
    return mAmplitude;
}

//-----------------------------------------------------------------------------
// Name : setAmplitude ( )
/// <summary>
/// Set the noise amplitude.
/// </summary>
//-----------------------------------------------------------------------------
void NoiseGenerator::setAmplitude( cgFloat value )
{
    mAmplitude = value;
}