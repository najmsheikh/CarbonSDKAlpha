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
// File: cfProgressHandler.h                                                 //
//                                                                           //
// Desc: Interface through which progress is reported to the system by       //
//       various components such as processors and importers.                //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfProgressHandler Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    ref class frmProgress;
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfProgressHandler (Class)
    /// <summary>
    /// Interface through which progress is reported to the system by various 
    /// components such as processors and importers.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfProgressHandler
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfProgressHandler( gcrootx<frmProgress^> oProgressDialog );
        ~cfProgressHandler( );

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void    SetTotalProgress        ( cgUInt32 nStep, cgUInt32 nCount );
        void    SetOperationProgress    ( const cgString & strLabel, cgUInt32 nStep, cgUInt32 nCount );
        void    SetOperationProgress    ( cgUInt32 nStep, cgUInt32 nCount );
        void    SetTaskProgress         ( cgUInt32 nStep, cgUInt32 nCount );
        void    SetTaskProgress         ( const cgString & strLabel, cgUInt32 nStep, cgUInt32 nCount );
        void    SetTaskProgress         ( const cgString & strLabel, cgFloat fProgress );
        void    Completed               ( );

    protected:
        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The progress dialog displayed while importing / processing scene data.</summary>
        gcptr(frmProgress^) m_ProgressDialog;

    }; // End Class cfBaseExtension

} // End Namespace CarbonForge