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
// File: SimpleScatter.h                                                     //
//                                                                           //
// Desc: Simple example utility plugin. Displays a form, and adds some       //
//       objects to the currently active scene (if any).                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2014 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// SimpleScatter Header Includes
//-----------------------------------------------------------------------------
// CarbonForge Includes
#include <cfBaseUtility.h>

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;

namespace CarbonForge
{
namespace CustomUtilities
{
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : SimpleScatter (Class)
    /// <summary>
    /// Scatter some objects around the scene.
    /// </summary>
    //-------------------------------------------------------------------------
    class SimpleScatter : public cfBaseUtility
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         SimpleScatter( cfWorldDoc * document );
        ~SimpleScatter();

        //---------------------------------------------------------------------
        // Public Static Functions
        //---------------------------------------------------------------------
        static cfBaseUtility * Allocate     ( const cgUID & identifier, cfWorldDoc * document );

        //---------------------------------------------------------------------
        // Public Virtual Methods (Overrides cfBaseUtility)
        //---------------------------------------------------------------------
        virtual bool        Launch          ( );
        
    protected:
        //---------------------------------------------------------------------
        // Protected Methods
        //---------------------------------------------------------------------
        
        //---------------------------------------------------------------------
        // Protected Virtual Methods (Overrides cfBaseUtility)
        //---------------------------------------------------------------------
        virtual bool        UtilityThread   ( cgThread * pThread, cfProgressHandler * pProgress );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
		cgScene   * m_pScene;
        int         m_nObjectType;
        int         m_nObjectCount;

    }; // End Class SimpleScatter

} // End Namespace CustomUtilities

} // End Namespace CarbonForge