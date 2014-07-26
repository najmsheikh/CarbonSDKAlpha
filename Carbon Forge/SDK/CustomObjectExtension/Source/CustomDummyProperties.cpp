//---------------------------------------------------------------------------//
//         ____           _                   _____                          //
//        / ___|__ _ _ __| |__   ___  _ __   |  ___|__  _ __ __ _  ___       //
//       | |   / _` | '__| '_ \ / _ \| '_ \  | |_ / _ \| '__/ _` |/ _ \      //
//       | |__| (_| | |  | |_) | (_) | | | | |  _| (_) | | | (_| |  __/      //
//        \____\__,_|_|  |_.__/ \___/|_| |_| |_|  \___/|_|  \__, |\___|      //
//                   Game Institute - Carbon Engine Sandbox |___/            //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// File: CustomDummyProperties.cpp                                           //
//                                                                           //
// Desc: Property definitions for CustomDummyObject & CustomDummyNode.       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// CustomDummyProperties Module Includes
//-----------------------------------------------------------------------------
#include "CustomDummyProperties.h"
#include "CustomDummy.h"

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace CarbonForge;
using namespace CarbonForge::PropertyWrappers;

///////////////////////////////////////////////////////////////////////////////
// Dummy Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : Size (Property)
/// <summary>
/// Get / set the size of the dummy object box (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
float CustomDummy::Size::get()
{
    // Pass through
    CustomDummyNode * node = (CustomDummyNode*)GetObjectNode().ToPointer();
    return node->getSize();
}
void CustomDummy::Size::set( float value )
{
    // Pass through
    CustomDummyNode * node = (CustomDummyNode*)GetObjectNode().ToPointer();
    node->setSize( value );
}
