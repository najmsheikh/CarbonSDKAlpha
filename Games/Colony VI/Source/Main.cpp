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
#include <Carbon.h>
#include <windows.h>
#include <tchar.h>
#include "Main.h"
#include "cApplication.h"

// Enable leak check in debug mode.
#if defined(_DEBUG)
#define _PERFORM_LEAK_CHECK 1
#define _BREAK_ON_ALLOC 0
#endif
#if (_PERFORM_LEAK_CHECK)
#include <crtdbg.h>
#endif

void buildPackage( cgDataPackage & package, const cgString & path, const cgString & refPath )
{
    // Process entries in this directory.
    WIN32_FIND_DATA entry;
    HANDLE findHandle = FindFirstFile( (path + _T("*")).c_str(), &entry);
    while ( findHandle != INVALID_HANDLE_VALUE )
    {
        if ( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            cgString directoryName = entry.cFileName;
            if ( directoryName != _T(".") && directoryName != _T("..") )
            {
                cgString newPath = path + _T("/") + directoryName + _T("/");
                cgString newRefPath = refPath + directoryName + _T("/");
                buildPackage( package, newPath, newRefPath );
            
            } // End if valid directory

        } // End if directory
        else
        {
            cgString fileName = entry.cFileName;
            cgString fileRef = refPath + fileName;
            cgString filePath = path + fileName;
            package.appendFile( filePath, fileRef );

        } // End if file

        // More entries?
        if ( !FindNextFile(findHandle, &entry) )
            break;

    } // Next file
    FindClose( findHandle );
}

//-----------------------------------------------------------------------------
// Name : WinMain() (Application Entry Point)
// Desc : Entry point for program, application flow starts here.
//-----------------------------------------------------------------------------
int WINAPI _tWinMain( HINSTANCE instance, HINSTANCE previousInstance, LPTSTR commandLine, int showCommand )
{
    // Stop execution at a given allocation?
#   if ( _BREAK_ON_ALLOC > 0 )
        _CrtSetBreakAlloc( _BREAK_ON_ALLOC );
#   endif // _BREAK_ON_ALLOC > 0

    /*cgDataPackage syspackage( _T("System.pkg") );
    buildPackage( syspackage, _T("SystemPackage/"), _T("sys://") );

    cgDataPackage datapackage( _T("Data.pkg") );
    buildPackage( datapackage, _T("DataPackage/"), _T("") );
    return 0;*/

    // Set the location of the engine 'system' directory where all system 
    // resources reside. This would not generally be necessary in a real 
    // application because the engine defaults to finding its resources in
    // any 'System' folder found within the main application directory. In
    // this case however we want to share a common central system directory
    // between all sample applications.
    cgFileSystem::addPathProtocol( _T("sys"), cgFileSystem::getAppDirectory() + _T("../../System/") );
    //cgFileSystem::addPathProtocol( _T("sys"), cgFileSystem::getAppDirectory() + _T("System/") );

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
    cgTChar renderingAPI[128];
    cgString configFile = cgFileSystem::resolveFileLocation( _T("sys://Config/SampleConfig.ini") );
    GetPrivateProfileString( _T("Drivers"), _T("Rendering"), _T("D3D9"), renderingAPI, 128, configFile.c_str() );

#   if defined( CGE_DX11_RENDER_SUPPORT )
        if ( _tcsicmp( renderingAPI, _T("D3D11") ) == 0 )
            config.renderAPI = cgRenderAPI::DirectX11;
#   endif // CGE_DX11_RENDER_SUPPORT

#   if defined( CGE_DX9_RENDER_SUPPORT )
        if ( config.renderAPI == cgRenderAPI::Null )
            config.renderAPI = cgRenderAPI::DirectX9;
#   endif // CGE_DX9_RENDER_SUPPORT
    
    // Initialize the framework
    if ( cgEngineInit( config ) == false )
    {
        // Write to debug spew
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize the application framework correctly! Fatal error, the application will now exit.\n") );
        MessageBox( 0, _T("Unable to initialize application framework. See 'DebugLog.html' for more information.\r\n\r\nIf the problem persists, please contact technical support."), _T("Fatal Error"), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL );

        // Release the framework and exit
        cgEngineCleanup();

        // Perform heap memory leak check?
#       if (_PERFORM_LEAK_CHECK)
            _CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );
#       endif
        return -1;

    } // End if failed to allocate game app

	// Initialize the engine.
    cApplication application;
	if ( application.initInstance( commandLine ) == false )
    {
        // Release the framework and exit
        cgEngineCleanup();

        // Perform heap memory leak check?
#       if (_PERFORM_LEAK_CHECK)
            _CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );
#       endif
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

    // Perform heap memory leak check?
#   if (_PERFORM_LEAK_CHECK)
        _CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF );
#   endif

    // Return the correct exit code.
    return returnCode;

}