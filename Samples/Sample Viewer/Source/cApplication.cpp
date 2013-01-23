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
    // Begin to add application states
    cgAppStateManager * applicationStates = cgAppStateManager::getInstance();

    // Search the 'Scripts' directory for a list of sample folders.
    cgString scriptDirectory = cgFileSystem::getAppDirectory() + L"Scripts";
    cgAppLog::write( cgAppLog::Info, _T("Searching for sample scripts in directory: %s\n"), scriptDirectory.c_str() );

    // Process entries in this directory.
    WIN32_FIND_DATA entry;
    HANDLE findHandle = FindFirstFile( (scriptDirectory + _T("\\*")).c_str(), &entry);
    while ( findHandle != INVALID_HANDLE_VALUE )
    {
        if ( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            cgString sampleDirectory = entry.cFileName;
            if ( sampleDirectory != _T(".") && sampleDirectory != _T("..") )
            {
                // If 'Main.gs' exists in this folder, use it.
                cgString scriptLocation = scriptDirectory + _T("\\") + sampleDirectory + _T("\\Main.gs");
                if ( cgFileSystem::fileExists( scriptLocation ) )
                {
                    // We found a sample.
                    cgAppLog::write( cgAppLog::Info, _T("Found sample '%s'.\n"), sampleDirectory.c_str() );

                    // Create a state for this sample
                    cgAppState * sampleState = new cgAppState( sampleDirectory, scriptLocation, cgResourceManager::getInstance() );
                    applicationStates->registerState( sampleState );
                    
                    // Register exit action
                    cgAppState::EventActionDesc actionDescription;
                    actionDescription.actionType = cgAppState::ActionType_EndRoot;
                    sampleState->registerEventAction( _T("Exit"), actionDescription );

                    // Record a list of all added samples
                    mSamples.push_back( sampleDirectory );

                } // End if found script
            
            } // End if valid directory

        } // End if directory

        // More entries?
        if ( !FindNextFile(findHandle, &entry) )
            break;

    } // Next file
    FindClose( findHandle );

    // Activate the last added state.
    if ( !mSamples.empty() )
        applicationStates->setActiveState( mSamples.back() );

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

    // Output top level instructions
    cgAppStateManager * applicationStates = cgAppStateManager::getInstance();
    cgAppState * activeState = applicationStates->getActiveState();
    cgString currentSample = (activeState) ? activeState->getStateId() : _T("None");
    cgString instructions = cgString::format( _T("Current Sample: [c=#ff5555ff]%s[/c]\nPress 1 through %i to select a sample."), currentSample.c_str(), mSamples.size() );
    interfaceManager->printText( textArea, instructions, cgTextFlags::Multiline | cgTextFlags::AllowFormatCode, 0xFFFFFFFF, 0, 10 );

    // Switch samples when a number key is pressed (temporary logic).
    cgInputDriver * input = cgInputDriver::getInstance();
    for ( cgInt i = cgKeys::D1; i < cgKeys::D0; ++i )
    {
        cgInt sampleIndex = i - cgKeys::D1;
        if ( input->isKeyPressed( i, true ) && (cgInt)mSamples.size() > sampleIndex )
            applicationStates->setActiveState( mSamples[sampleIndex] );
    
    } // Next number key

    // Allow base application to process.
    cgApplication::frameEnd();
}