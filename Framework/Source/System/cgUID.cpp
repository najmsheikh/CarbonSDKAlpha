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
// Name : cgUID.cpp                                                          //
//                                                                           //
// Desc : Globally unique identifier.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgUID Module Includes
//-----------------------------------------------------------------------------
#include <System/cgUID.h>
#include <Math/cgRandom.h>

//-----------------------------------------------------------------------------
// Static Constant Definitions
//-----------------------------------------------------------------------------
const cgUID cgUID::Empty = { 0 };

//-----------------------------------------------------------------------------
//  Name : generateRandom () (Static)
/// <summary>
/// Generate a UID using a random number generator.
/// </summary>
//-----------------------------------------------------------------------------
cgUID cgUID::generateRandom( )
{
    cgUID NewUID;

    // We'll use the park miller RNG
    cgRandom::ParkMiller Random;

    // Populate with random numbers.
    unsigned __int32* p = (unsigned __int32*)&NewUID.data1;
    *p++ = Random.nextSeed();
    *p++ = Random.nextSeed();
    *p++ = Random.nextSeed();
    *p++ = Random.nextSeed();

    // Set UUID variant (Binary:10xxxxxx)
    NewUID.data4[0] &= 0xBF;
    NewUID.data4[0] |= 0x80;

    // Set UUID version (Binary:0100xxxx)
    NewUID.data3 &= 0x4FFF;
    NewUID.data3 |= 0x4000;

    // Return new random UID
    return NewUID;
}