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
// File: cfBaseExtension.h                                                   //
//                                                                           //
// Desc: Contains base class interface through which new functionality can   //
//       be added to Carbon Forge by providing additional extension DLLs.    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseExtension Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfWorldDoc;
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfBaseExtension (Base Class)
    /// <summary>
    /// Base interface through which Carbon Forge extensions can be accessed.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseExtension
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 cfBaseExtension( );
        virtual ~cfBaseExtension( );

        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual bool    LoadExtension   ( ) = 0;
        virtual void    OnNewDocument   ( cfWorldDoc * pDocument );
        virtual void    OnDocumentClosed( cfWorldDoc * pDocument );

    protected:
        //---------------------------------------------------------------------
        // Protected Member Variables
        //---------------------------------------------------------------------
        /// <summary>The currently open application document.</summary>
        cfWorldDoc * m_pDocument;

    }; // End Class cfBaseExtension

} // End Namespace CarbonForge