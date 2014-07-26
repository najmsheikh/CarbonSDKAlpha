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
// Desc: Custom object type extension example for the Carbon Forge editing   //
//       environment.                                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
namespace CustomObjects
{
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : Extension (Class)
    /// <summary>
    /// Custom object type extension example for the Carbon Forge editing
    /// environment.
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
        virtual void    OnNewDocument   ( cfWorldDoc * pDocument );
        virtual void    OnDocumentClosed( cfWorldDoc * pDocument );

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        
    }; // End Class Extension

} // End Namespace CustomObjects

} // End Namespace CarbonForge