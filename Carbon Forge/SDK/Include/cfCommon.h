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
// File : cfCommon.h                                                         //
//                                                                           //
// Desc : The common header file used throughout the application. This       //
//        header contains all common includes, macros and utilities that may //
//        be required.                                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfCommon Header Includes
//-----------------------------------------------------------------------------
// System
#include "cfAPI.h"

// OS Interop
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501
#endif // !_WIN32_WINNT
#include <windows.h>
#include <D3DX9.h>
#include <dxerr.h>
#include <msclr\lock.h>
#undef MessageBoxEx
#undef GetCurrentDirectory

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgCriticalSection;

//-----------------------------------------------------------------------------
// Global Function Declarations
//-----------------------------------------------------------------------------
namespace CarbonForge
{
    //-----------------------------------------------------------------------------
    // Forward Declarations
    //-----------------------------------------------------------------------------
    ref class cfExtensionManager;
    ref class cfUIObjectBrowser;
    ref class frmMain;
    ref class frmWait;
    class cfToolManager;
    class cfUtilityManager;
    class cfPropertyManager;

    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : Program (Class)
    /// <summary>
    /// Contains application entry point, and very common application related 
    /// utility methods.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API Program
    {
    public:
        //---------------------------------------------------------------------
        // Public Static Functions
        //---------------------------------------------------------------------
        static void                         ShowSplashScreen    ( );
        static void                         CloseSplashScreen   ( bool bImmediate );
        static void                         ShowWaitScreen      ( const cgString & operation );
        static void                         ShowWaitScreen      ( const cgString & operation, cgCriticalSection * pSection );
        static void                         ShowWaitScreen      ( const cgString & operation, bool bThreaded );
        static void                         ShowWaitScreen      ( const cgString & operation, bool bThreaded, cgCriticalSection * pSection );
        static void                         CloseWaitScreen     ( );
        static gcrootx<System::Object^>     GetResource         ( const cgString & resourceName );
        static gcrootx<System::Object^>     GetResource         ( const cgString & resourceName, const cgString & resourceContainer );
        static cfToolManager              * GetToolManager      ( );
        static cfUtilityManager           * GetUtilityManager   ( );
        static cfPropertyManager          * GetPropertyManager  ( );
        static gcrootx<cfUIObjectBrowser^>  GetObjectBrowser    ( );
        static cgString                     GetRegistryString   ( const cgString & ValueName );
        static cgString                     GetRegistryString   ( const cgString & ValueName, const cgString & Default );
        static cgString                     GetRegistryString   ( const cgString & SubKey, const cgString & ValueName, const cgString & Default );
        static bool                         SetRegistryString   ( const cgString & ValueName, const cgString & Value );
        static bool                         SetRegistryString   ( const cgString & SubKey, const cgString & ValueName, const cgString & Value );
        static int                          Main                ( const cgStringArray & aArgs );
        static gcrootx<frmMain^>            GetMainForm         ( );
        static gcrootx<cfExtensionManager^> GetExtensions       ( );
        static gcrootx<System::String^>     GetAppPath          ( );
        static void                         ModalRepaint        ( );
        static bool                         ComputeFileHash     ( const cgString & strFile, cgByteArray & aHash );
        static bool                         IsInDesignMode      ( gcrootx<System::Windows::Forms::Control^> control );

        // General Utilities
        static void                         MouseEventToCGE     ( gcrootx<System::Windows::Forms::MouseEventArgs^> args, gcrootx<System::Windows::Forms::Keys> ModifierKeys, cgUInt8 & nButtons, cgPoint & Position, cgUInt32 & nModifiers );
    };

} // End Namespace CarbonForge