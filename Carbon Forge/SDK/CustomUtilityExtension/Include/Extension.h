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
// File: Extension.h                                                         //
//                                                                           //
// Desc: Example utility extension for the Carbon Forge editing              //
//       environment.                                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2014 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// Extension Header Includes
//-----------------------------------------------------------------------------
#include "cfBaseExtension.h"

// CGE Includes
#include <cgBaseTypes.h>

namespace CarbonForge
{
namespace CustomUtilities
{
    //-------------------------------------------------------------------------
    // Constant Wrappers
    //-------------------------------------------------------------------------
    /// <summary>A list of GUIDs for the utility types supported by the extension.</summary>
    namespace UtilityIdentifiers
    {
        // {D1434AF8-372E-4c0e-99F2-C5280005662B}
        static const cgUID SimpleScatter   = { 0xd1434af8, 0x372e, 0x4c0e, { 0x99, 0xf2, 0xc5, 0x28, 0x0, 0x5, 0x66, 0x2b } };
    
    }; // End namespace UtilityIdentifiers
 
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : Extension (Class)
    /// <summary>
    /// Custom utility library extension for Carbon Forge. Demonstrates the
    /// addition of utilities to the 'Tools' menu.
    /// </summary>
    //-------------------------------------------------------------------------
    class Extension : public cfBaseExtension
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 Extension();
        virtual ~Extension();

        //---------------------------------------------------------------------
        // Public Virtual Methods (cfBaseExtension)
        //---------------------------------------------------------------------
        virtual bool    LoadExtension   ( );

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        
    }; // End Class Extension

} // End Namespace CustomUtilities

} // End Namespace CarbonForge