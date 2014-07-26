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
// Desc: Custom object type extension example for the Carbon Forge editing   //
//       environment.                                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Extension Module Includes
//-----------------------------------------------------------------------------
#include "Extension.h"

// Custom Object Types
#include "CustomDummy.h"
#include "ToolCreateCustomDummy.h"

// CarbonForge Includes
#include <cfCommon.h>
#include <cfWorldDoc.h>
#include <cfPropertyManager.h>
#include <cfToolManager.h>

// CGE Includes
#include <Carbon.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace System;
using namespace CarbonForge;
using namespace CarbonForge::CustomObjects;

///////////////////////////////////////////////////////////////////////////////
// Extension Assemblies
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : CustomObjects (Class)
// Desc : Provides installation of the extension with Carbon Forge.
//-----------------------------------------------------------------------------
public ref class CustomObjects : public CarbonForge::cfExtensionAssembly
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
    // Register our custom object type(s) with the core engine.
    cgWorldObject::registerType( RTID_CustomDummyObject, L"Custom Dummy", CustomDummyObject::allocateNew, CustomDummyObject::allocateClone, CustomDummyNode::allocateNew, CustomDummyNode::allocateClone );

    // Register our custom object type(s) with Carbon Forge's property manager
    // so that its properties can be displayed in the property explorer.
    cfPropertyManager * propertyManager = Program::GetPropertyManager();
    propertyManager->AddType( RTID_CustomDummyObject, L"Custom Dummy", L"CarbonForge.PropertyWrappers.CustomDummy", nullptr );

    // Register our custom creation tool with Carbon Forge's tool manager.
    cfToolManager * toolManager = Program::GetToolManager();
    toolManager->AddMode( TMID_CreateCustomDummy, ToolCreateCustomDummy::Allocate, CG_NULL, L"Create Custom Dummy", nullptr );

    // Add button to the object browser.
    cfUIObjectBrowser ^ objectBrowser = Program::GetObjectBrowser();
    objectBrowser->AddToolModeButton( cfAPI::ToManaged(TMID_CreateCustomDummy), L"Custom Dummy", cfUIObjectBrowser::ToolPanel::Objects, L"Helper Objects", nullptr, nullptr );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : OnNewDocument() (Virtual)
/// <summary>
/// Triggered whenever a new document has been created in the application.
/// </summary>
//-----------------------------------------------------------------------------
void Extension::OnNewDocument( CarbonForge::cfWorldDoc * pDocument )
{
    // Call base class implementation first
    cfBaseExtension::OnNewDocument( pDocument );
}

//-----------------------------------------------------------------------------
// Name : OnDocumentClosed () (Virtual)
/// <summary>
/// Triggered whenever the system closes / disposes of a document.
/// </summary>
//-----------------------------------------------------------------------------
void Extension::OnDocumentClosed( CarbonForge::cfWorldDoc * pDocument )
{
    // Call base class implementation last
    cfBaseExtension::OnDocumentClosed( pDocument );
}