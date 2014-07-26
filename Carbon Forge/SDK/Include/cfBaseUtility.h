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
// File: cfBaseUtility.h                                                     //
//                                                                           //
// Desc: Custom utility base class. Utilities are essentially custom         //
//       processors designed to perform some 'one off' scene related task    //
//       such as compiling effects, lightmaps, etc.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfBaseUtility Header Includes
//-----------------------------------------------------------------------------
#include "cfAPI.h"

// CGE Includes
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgThread;
class cgCriticalSection;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    class cfWorldDoc;
    class cfProgressHandler;

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfBaseUtility (Base Class)
    /// <summary>
    /// This is the base interface class through which the application will
    /// access custom utility / processor functionality.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfBaseUtility
	{
	public:
        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
                 cfBaseUtility( cfWorldDoc * pDocument );
        virtual ~cfBaseUtility();

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void                        Dispose             ( );

        //---------------------------------------------------------------------
        // Public Virtual Methods
        //---------------------------------------------------------------------
        virtual bool                Launch              ( ) = 0;

    protected:
        //---------------------------------------------------------------------
        // Protected Virtual Methods
        //---------------------------------------------------------------------
        virtual bool                StartThreadedProcess( );
        virtual bool                UtilityThread       ( cgThread * pThread, cfProgressHandler * pProgress );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The document on which we are working.</summary>
        cfWorldDoc        * m_pDocument;
        /// <summary>Progress handler for threaded process.</summary>
        cfProgressHandler * m_pProgress;

    private:
        //---------------------------------------------------------------------
        // Private Static Methods
        //---------------------------------------------------------------------
        static cgUInt32             UtilityThreadStub   ( cgThread * pThread, void * pContext );

        //---------------------------------------------------------------------
        // Private Variables
        //---------------------------------------------------------------------
        /// <summary>Critical section for handling wait screen on thread shutdown.</summary>
        cgCriticalSection * m_pWaitScreenSection;
        /// <summary>Result of the threaded process.</summary>
        volatile bool       m_bProcessSuccess;


    }; // End Class cfBaseUtility

} // End Namespace CarbonForge