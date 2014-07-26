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
// File: Extension.cpp                                                       //
//                                                                           //
// Desc: Example utility extension for the Carbon Forge editing              //
//       environment.                                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2014 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Extension Module Includes
//-----------------------------------------------------------------------------
#include "Extension.h"

// Utility Types
#include "Utility Plugins/SimpleScatter.h"

// CarbonForge Includes
#include <cfCommon.h>
#include <cfWorldDoc.h>
#include <cfUtilityManager.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace System;
using namespace CarbonForge;
using namespace CarbonForge::CustomUtilities;

///////////////////////////////////////////////////////////////////////////////
// Extension Assemblies
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : CoreUtilities (Class)
// Desc : Provides installation of the utility extension with Carbon Forge.
//-----------------------------------------------------------------------------
public ref class CoreUtilities : public CarbonForge::cfExtensionAssembly
{
public:
    static IntPtr InstallExtension( ) { return IntPtr(new Extension()); }
    static void   UninstallExtension( IntPtr ptr ) { delete ((Extension*)ptr.ToPointer()); }
};

///////////////////////////////////////////////////////////////////////////////
// Extension Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : Extension() (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
Extension::Extension(  )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
// Name : ~Extension() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
Extension::~Extension( )
{
    // Clear Variables
}

//-----------------------------------------------------------------------------
// Name : LoadExtension()
/// <summary>
/// Allow the extension to initialize and install itself into the application
/// as necessary.
/// </summary>
//-----------------------------------------------------------------------------
bool Extension::LoadExtension( )
{
    // Add utility types
    cfUtilityManager * utilities = Program::GetUtilityManager();
    utilities->RegisterUtility( UtilityIdentifiers::SimpleScatter, SimpleScatter::Allocate, CG_NULL, _T("Simple Scatter"), _T("Example Utilities") );

    // Success!
    return true;
}