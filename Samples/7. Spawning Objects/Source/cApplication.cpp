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
#include "States/cGamePlayState.h"
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
    cgFileSystem::addPackage( cgFileSystem::getAppDirectory() + _T("../../Shared Media/Audio.pkg") );

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
    // Open a connection with the specified world database.
    cgWorld * world = cgWorld::getInstance();
    if ( !world->open( _T("SampleTower.cwm") ) )
        return false;
    
    // Begin to create the main viewer state
    cgAppStateManager * applicationStates = cgAppStateManager::getInstance();
    cgAppState * gamePlayState = new cGamePlayState( _T("GamePlayState") );
    applicationStates->registerState( gamePlayState );
    
    // Register exit action
    cgAppState::EventActionDesc actionDescription;
    actionDescription.actionType = cgAppState::ActionType_EndRoot;
    gamePlayState->registerEventAction( _T("Exit"), actionDescription );
    
    // Set the initial state
    applicationStates->setActiveState( _T("GamePlayState") );

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

    // Retrieve required application objects
    cgAppStateManager * applicationStates = cgAppStateManager::getInstance();
    cgUIManager       * interfaceManager  = cgUIManager::getInstance();

    // Allow the application state code to perform any necessary render operation
    applicationStates->render();
    
    // Render the user interface
    interfaceManager->render();
}

//-----------------------------------------------------------------------------
// Name : frameEnd () (Protected, Virtual)
// Desc : Called to signal that we have finished rendering the current frame.
//-----------------------------------------------------------------------------
void cApplication::frameEnd()
{
    cgRenderDriver * renderDriver     = cgRenderDriver::getInstance();
    cgUIManager    * interfaceManager = cgUIManager::getInstance();
    cgTimer        * timer            = cgTimer::getInstance();

    // Generate statistics information (i.e. FPS etc) ready for display
    cgString statistics = cgString::format( _T("MSPF = %.2f : FPS = %i"), timer->getTimeElapsed() * 1000.0f, timer->getFrameRate() );

    // Draw the text to the screen.
    cgSize screenSize = renderDriver->getScreenSize();
    cgRect textArea( 10, 10, screenSize.width - 10, screenSize.height - 10 );
    interfaceManager->selectFont( _T("fixed_v01_white") );
    interfaceManager->printText( textArea, statistics, cgTextFlags::VAlignBottom );
    interfaceManager->printText( textArea, mCopyright, cgTextFlags::VAlignBottom | cgTextFlags::AlignCenter );
    interfaceManager->printText( textArea, mVersion, cgTextFlags::VAlignBottom | cgTextFlags::AlignRight );
    interfaceManager->printText( textArea, _T("[c=#ffcccccc]<RMB>[/c] to pickup an object. [c=#ffcccccc]<LMB>[/c] to throw.\nPress [c=#ffff0000]<Tab>[/c] to show/hide spawn menu."), cgTextFlags::Multiline | cgTextFlags::AllowFormatCode, 0xFFFFFFFF, 0, 10 );

    // Allow base application to process.
    cgApplication::frameEnd();
}