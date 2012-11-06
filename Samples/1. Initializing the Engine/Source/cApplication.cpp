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
// Name : cApplication.cpp                                                   //
//                                                                           //
// Desc : Game application class. This class is the central hub for all of   //
//        the main application processing. While no game specific logic is   //
//        contained in this class, it is primarily responsible for           //
//        initializing all of the systems that the application will rely on  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2012 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// cApplication Module Includes
//-----------------------------------------------------------------------------
#include "cApplication.h"
#include "Res/resource.h"

//-----------------------------------------------------------------------------
// Name : cApplication () (Constructor)
// Desc : cApplication Class Constructor
//-----------------------------------------------------------------------------
cApplication::cApplication()
{
}

//-----------------------------------------------------------------------------
// Name : ~cApplication () (Destructor)
// Desc : cApplication Class Destructor
//-----------------------------------------------------------------------------
cApplication::~cApplication()
{
}

//-----------------------------------------------------------------------------
// Name : initInstance () (Virtual)
// Desc : Initializes the entire engine here.
//-----------------------------------------------------------------------------
bool cApplication::initInstance( const cgString & commandLine )
{
    // Configure the application.
    loadConfig( _T("sys://Config/SampleConfig.ini") );

    // Set the root data path relative to which all provided file and
    // directory paths are assumed to be expressed. In the case of this
    // demo we want to load data from the shared media folder.
    setRootDataPath( cgFileSystem::getAppDirectory() + _T("../../Shared Media/") );

    // Setup the application and window display data.
    setVersionData( cgStringUtility::fromStringTable( CG_NULL, IDS_VERSION ) );
    setCopyrightData( cgStringUtility::fromStringTable( CG_NULL, IDS_COPYRIGHT ) );
    setWindowTitle( cgStringUtility::fromStringTable( CG_NULL, IDS_TITLE ) );
    setWindowIcon( IDI_ICON );

    // Initialize!
    return cgApplication::initInstance( commandLine );
}

//-----------------------------------------------------------------------------
// Name : initApplication() (Virtual)
// Desc : Initialize all required aspects of the application ready for us to
//        begin. This includes setting up all required states that the
//        application may enter, etc.
//-----------------------------------------------------------------------------
bool cApplication::initApplication( )
{    
    // Call base class implementation.
    return cgApplication::initApplication();
}

//-----------------------------------------------------------------------------
// Name : frameRender () (Protected, Virtual)
// Desc : Actually performs the default rendering of the current frame.
//-----------------------------------------------------------------------------
void cApplication::frameRender()
{
    // Allow base application to process first.
    cgApplication::frameRender();
}

//-----------------------------------------------------------------------------
// Name : frameEnd () (Protected, Virtual)
// Desc : Called to signal that we have finished rendering the current frame.
//-----------------------------------------------------------------------------
void cApplication::frameEnd()
{
    // Allow base application to process.
    cgApplication::frameEnd();
}