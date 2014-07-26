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
// File: CustomDummyProperties.h                                             //
//                                                                           //
// Desc: Property definitions for CustomDummyObject & CustomDummyNode.       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// CustomDummyProperties Header Includes
//-----------------------------------------------------------------------------

namespace CarbonForge
{
namespace PropertyWrappers
{
    //-------------------------------------------------------------------------
    // Namespace Promotion
    //-------------------------------------------------------------------------
    using namespace System::Drawing;
    using namespace System::ComponentModel;

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : CustomDummy (Class)
    /// <summary>
    /// Property wrapper for CustomDummyObject & CustomDummyNode.
    /// </summary>
    //-------------------------------------------------------------------------
    public ref class CustomDummy : public cfBaseObjectProperties
	{
	public:
        //---------------------------------------------------------------------
        // Public Properties
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        // Name: Size (Property)
        //---------------------------------------------------------------------
        [DisplayName          (L"Size"),
         Category             (L"Display Options"),
         Description          (L"The object space size of the bounding box to display for the dummy object."),
         cfPropertyOrder      ( 0 ),                            // Order that this property appears in its category
         cfWorldObjectProperty( true ),                         // Indicates that this property belongs to the 'object', not the 'node' and thus may be instanced.
         TypeConverter        ( cfSceneUnitConverter::typeid ), // Automatically convert the value and display in whatever units are configured for the scene.
         cfSceneUnitFormat    ( 0, 65535, 3 )                   // Configuration for the above converter. Indicates that valid values are between 0 and 65535, with 3 decimal places.
        ]
        property float Size { float get(); void set( float value ); }

        // Hide the base 'NodeColor' property.
        [Browsable(false)]
        property Color NodeColor;

    }; // End Class CustomDummy

} // End Namespace PropertyWrappers

} // End Namespace CarbonForge