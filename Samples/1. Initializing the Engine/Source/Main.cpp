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
// Name : Main.cpp                                                           //
//                                                                           //
// Desc : Main application entry & handling source file.                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Main Module Includes
//-----------------------------------------------------------------------------
#include <cgAPI.h>
#include <windows.h>
#include <tchar.h>
#include "Main.h"
#include "cApplication.h"

//-----------------------------------------------------------------------------
// Name : WinMain() (Application Entry Point)
// Desc : Entry point for program, application flow starts here.
//-----------------------------------------------------------------------------
int WINAPI _tWinMain( HINSTANCE instance, HINSTANCE previousInstance, LPTSTR commandLine, int showCommand )
{
    // Set the location of the engine 'system' directory where all system 
    // resources reside. This would not generally be necessary in a real 
    // application because the engine defaults to finding its resources in
    // any 'System' folder found within the main application directory. In
    // this case however we want to share a common central system directory
    // between all sample applications.
    cgFileSystem::addPathProtocol( _T("sys"), cgFileSystem::getAppDirectory() + _T("../../System/") );

    // Select the requested outputs for application debug information.
    cgAppLog::registerOutput( new cgLogOutputFile( _T("./DebugLog.html") ) );
#   if defined(_DEBUG)
        cgAppLog::registerOutput( new cgLogOutputStd( true ) );
#   endif

    // Configure the framework
    CGEConfig config;
    config.highPriority  = true;
    config.multiThreaded = false;
    config.platform      = cgPlatform::Windows;
    config.audioAPI      = cgAudioAPI::DirectX;
    config.inputAPI      = cgInputAPI::DirectX;
    config.networkAPI    = cgNetworkAPI::Winsock;
    config.renderAPI     = cgRenderAPI::Null;

    // Select APIs
#   if defined( CGE_DX9_RENDER_SUPPORT )
        if ( config.renderAPI == cgRenderAPI::Null )
            config.renderAPI = cgRenderAPI::DirectX9;
#   endif // CGE_DX9_RENDER_SUPPORT
    
    // Initialize the framework
    if ( cgEngineInit( config ) == false )
    {
        // Write to debug spew
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize the application framework correctly! Fatal error, the application will now exit.\n") );
        MessageBox( 0, _T("Unable to initialize application framework. See 'AspeqtLog.html' for more information.\r\n\r\nIf the problem persists, please contact technical support."), _T("Fatal Error"), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );

        // Release the framework and exit
        cgEngineCleanup();
        return -1;

    } // End if failed to allocate game app

	// Initialize the engine.
    cApplication application;
	if ( application.initInstance( commandLine ) == false )
    {
        // Release the framework and exit
        cgEngineCleanup();
        return -1;
    
    } // End if failed to startup

    // Begin the gameplay process. Will return when app due to exit.
    cgInt returnCode = application.begin( CG_NULL );

    // Shut down the application, just to be polite, before exiting.
    if ( !application.shutDown() )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to cleanly shut down the application!\n") );
        MessageBox( 0, _T("Failed to shut system down correctly.\r\n\r\nIf the problem persists, please contact technical support."), _T("Fatal Error"), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );

    } // End if failed to shut down

    // Release the framework
    cgEngineCleanup();

    // Return the correct exit code.
    return returnCode;

}