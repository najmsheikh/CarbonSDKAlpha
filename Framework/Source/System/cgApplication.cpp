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
// Name : cgApplication.cpp                                                  //
//                                                                           //
// Desc : Game application class. This class is the central hub for all of   //
//        the main application processing. While no game specific logic is   //
//        contained in this class, it is primarily responsible for           //
//        initializing all of the systems that the application will rely on  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgApplication Module Includes
//-----------------------------------------------------------------------------
#include <System/cgApplication.h>
#include <System/cgAppWindow.h>
#include <System/cgThreading.h>
#include <System/cgStringUtility.h>
#include <System/cgProfiler.h>
#include <System/cgMessageTypes.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <Physics/cgPhysicsEngine.h>
#include <Audio/cgAudioDriver.h>
#include <Input/cgInputDriver.h>
#include <Interface/cgUIManager.h>
#include <States/cgAppStateManager.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // Warning: Portability
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
//  Name : cgApplication () (Constructor)
/// <summary>
/// cgApplication Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgApplication::cgApplication() : cgReference( cgReferenceManager::generateInternalRefId() )
{
    // Set members to sensible defaults
    mRootDataDir            = cgString::Empty;
    mWindowIcon             = 0;
    mAutoAddPackages        = true;
    mFocusWindow            = CG_NULL;
    mFocusWindowOverride    = CG_NULL;
    mOutputWindowOverride   = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgApplication () (Destructor)
/// <summary>
/// cgApplication Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgApplication::~cgApplication()
{
    if ( mFocusWindowOverride )
        mFocusWindowOverride->scriptSafeDispose();
    mFocusWindowOverride = CG_NULL;
    if ( mOutputWindowOverride )
        mOutputWindowOverride->scriptSafeDispose();
    mOutputWindowOverride = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : initInstance ()
/// <summary>
/// Initialises the entire engine here.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initInstance( const cgString & strCmdLine )
{
    // Initialize file system and search for all valid data packages
    cgFileSystem::setRootDirectory( mRootDataDir );
    cgFileSystem::index( mAutoAddPackages );

    // Initializing profiler
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Initializing application profiler.\n") );

    // Initialize the application profiler
    cgProfiler * pProfiler = cgProfiler::getInstance();
    if ( pProfiler->loadConfig( mSystemConfig ) == false || pProfiler->initialize( ) == false )
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to initialize application profiler.\n") );

    // Initializing resource manager
    cgAppLog::write( cgAppLog::Info, _T("Initializing resource manager.\n") );

    // Initialize the application resource manager with driver instances
    cgResourceManager * pResources = cgResourceManager::getInstance();
    if ( pResources->loadConfig( mSystemConfig ) == false || pResources->initialize( ) == false )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize resource manager.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize resource manager. The application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // Create and initialize the primary display device
    if ( !initDisplay( ) ) { shutDown(); return false; }

    // Create and initialize the audio devices
    if ( !initAudio( ) ) { shutDown(); return false; }

    // Initialize the physics subsystems
    if ( !initPhysics( ) ) { shutDown(); return false; }

    // Create and initialize the input devices
    if ( !initInput( ) ) { shutDown(); return false; }

    // Create and initialize the user interface
    if ( !initInterface( ) ) { shutDown(); return false; }

    // Initialize the application systems
    if ( !initApplication( ) ) { shutDown(); return false; }

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setWindowIcon ()
/// <summary>
/// Set the handle (usually a resource identifier) for the icon to display
/// on the window application when running in windowed mode. The information
/// provided via this method only takes effect during a subsequent call to 
/// 'initInstance()' and cannot be used to make changes after the fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::setWindowIcon( cgUInt32 iconHandle )
{
    mWindowIcon = iconHandle;
}

//-----------------------------------------------------------------------------
//  Name : setRootDataPath ()
/// <summary>
/// Set the path that represents the root from which all data will be loaded
/// or leave blank to use the process current directory. The information
/// provided via this method only takes effect during a subsequent call to 
/// 'initInstance()' and cannot be used to make changes after the fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::setRootDataPath( const cgString & path )
{
    mRootDataDir = path;
}

//-----------------------------------------------------------------------------
//  Name : setWindowTitle ()
/// <summary>
/// Set the initial caption to be displayed in the application window when
/// running in windowed mode. The information provided via this method only 
/// takes effect during a subsequent call to 'initInstance()' and cannot be 
/// used to make changes after the fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::setWindowTitle( const cgString & title )
{
    mWindowTitle = title;
}

//-----------------------------------------------------------------------------
//  Name : setCopyrightData ()
/// <summary>
/// Set the application copyright information as it should be displayed to the
/// user where applicable.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::setCopyrightData( const cgString & copyright )
{
    mCopyright = copyright;
}

//-----------------------------------------------------------------------------
//  Name : setVersionData ()
/// <summary>
/// Set the application version information as it should be displayed to the
/// user where applicable.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::setVersionData( const cgString & version )
{
    mVersion = version;
}

//-----------------------------------------------------------------------------
//  Name : initDisplay ()
/// <summary>
/// Initialize the display driver, create device window, load
/// render configuration etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initDisplay()
{
    cgConfigResult::Base Status;

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Initializing rendering driver.\n") );

    // Get singleton render driver instance.
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();

    // Instruct the primary render driver to load configuration
    Status = pDriver->loadConfig( mSystemConfig );
    
    // Were we able to match previous configuration data
    // to that now supported by the hardware?
    if( Status == cgConfigResult::Mismatch )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Warning, _T("Previous configuration mismatch found when selecting render hardware features. Selecting defaults.\n") );

        // Notify the user
        MessageBox( CG_NULL, _T("Previous configuration references features or modes not supported by the current graphics adapter.\r\n\r\n")
                          _T("It is possible that the hardware or device drivers were changed since last time the application was run.\r\n\r\n")
                          _T("Rendering configuration options will now be reset (this will not affect your profiles, keymappings or game options)."),
                          _T("Graphics Hardware Change"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );

        // Find sensible defaults
        Status = pDriver->loadDefaultConfig( );

    } // End if mismatch loading config

    // Was there an error loading the configuration
    if ( Status != cgConfigResult::Valid )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to load a valid render driver configuration.\n") );
        return false;
    
    } // End if error loading config

    // Save the render driver configuration
    pDriver->saveConfig( mSystemConfig );

    // Initialize the render driver
    bool bResult = false;
    if ( mFocusWindowOverride )
        bResult = pDriver->initialize( cgResourceManager::getInstance(), mFocusWindowOverride, mOutputWindowOverride );
    else
        bResult = pDriver->initialize( cgResourceManager::getInstance(), mWindowTitle, mWindowIcon );
    
    // Succeeded?
    if ( !bResult )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize render driver.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize rendering hardware. The application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // If a window was created, retrieve it so that we can listen in to messages.
    if ( !mFocusWindowOverride )
    {
        mFocusWindow = pDriver->getFocusWindow();
        cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow );
    
    } // End if new window

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initInterface ()
/// <summary>
/// Initialize the interface manager, loading required fonts, skins etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initInterface()
{
    cgUIManager * pInterface = cgUIManager::getInstance();

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Initializing user interface.\n") );

    // Initialize the user interface system using the resource manager
    if ( !pInterface->initialize( cgResourceManager::getInstance() ) )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize user interface.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize user interface. See the application execution log for more information.\r\n\r\nThe application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // ToDo: These need to be defined by the application

    // Add system fonts (last will be the default).
    pInterface->addFont( _T("sys://Interface/Fonts/FixedWhitePlain.fnt") );
    pInterface->addFont( _T("sys://Interface/Fonts/Nasalization.fnt") );
    pInterface->addFont( _T("sys://Interface/Fonts/FixedBlack.fnt") );
    pInterface->addFont( _T("sys://Interface/Fonts/FixedWhite.fnt") );

    // Add all required interface skins
    pInterface->addSkin( _T("sys://Interface/Skins/Carbon_Default.xml") );

    // Select the skin to use for the user interface
    pInterface->selectSkin( _T("Carbon Default") );

    // Start the user interface running
    if ( !pInterface->begin( ) )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to start user interface.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to start user interface. See the application execution log for more information.\r\n\r\nThe application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to start

    // Success!
    return true;
}


//-----------------------------------------------------------------------------
//  Name : initInput ()
/// <summary>
/// Initialize the input driver, and start capturing input data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initInput()
{
    cgConfigResult::Base Status;

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Initializing input driver.\n") );

    // Get singleton input driver and render driver instances.
    cgInputDriver  * pDriver       = cgInputDriver::getInstance();
    cgRenderDriver * pRenderDriver = cgRenderDriver::getInstance();
    
    // Instruct the primary input driver to load configuration
    Status = pDriver->loadConfig( mSystemConfig );
    
    // Were we able to match previous configuration data
    // to that now supported by the hardware?
    if( Status == cgConfigResult::Mismatch )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Warning, _T("Previous configuration mismatch found when selecting input hardware features. Selecting defaults.\n") );

        // Notify the user
        MessageBox( CG_NULL, _T("Previous configuration references features or modes not supported by the current input hardware.\r\n\r\n")
                          _T("It is possible that the hardware or device drivers were changed since last time the application was run.\r\n\r\n")
                          _T("Input configuration options will now be reset (this will not affect your profiles, video settings or game options)."),
                          _T("Input Hardware Change"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );

        // Find sensible defaults
        Status = pDriver->loadDefaultConfig( );

    } // End if mismatch loading config

    // Was there an error loading the configuration
    if ( Status != cgConfigResult::Valid )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to load a valid input driver configuration.\n") );
        return false;
    
    } // End if error loading config

    // Save the input driver configuration
    pDriver->saveConfig( mSystemConfig );

    // Initialize the input driver
    if ( !pDriver->initialize( pRenderDriver->getFocusWindow(), pRenderDriver->isWindowed() ) )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize input driver.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize required input devices. The application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initAudio ()
/// <summary>
/// Initialize the audio driver ready for playback of music and sound
/// effects etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initAudio()
{
    cgConfigResult::Base Status;

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Initializing audio driver.\n") );

    // Get singleton audio driver and render driver instances.
    cgAudioDriver  * pDriver       = cgAudioDriver::getInstance();
    cgRenderDriver * pRenderDriver = cgRenderDriver::getInstance();
    
    // Instruct the primary audio driver to load configurataion
    Status = pDriver->loadConfig( mSystemConfig );
    
    // Were we able to match previous configuration data
    // to that now supported by the hardware?
    if( Status == cgConfigResult::Mismatch )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Warning, _T("Previous configuration mismatch found when selecting audio hardware features. Selecting defaults.\n") );

        // Notify the user
        MessageBox( CG_NULL, _T("Previous configuration references features or modes not supported by the current audio hardware.\r\n\r\n")
                          _T("It is possible that the hardware or device drivers were changed since last time the application was run.\r\n\r\n")
                          _T("Audio configuration options will now be reset (this will not affect your profiles, video settings or game options)."),
                          _T("Audio Hardware Change"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );

        // Find sensible defaults
        Status = pDriver->loadDefaultConfig( );

    } // End if mismatch loading config

    // Was there an error loading the configuration
    if ( Status != cgConfigResult::Valid )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to load a valid audio driver configuration.\n") );
        return false;
    
    } // End if error loading config

    // Save the audio driver configuration
    pDriver->saveConfig( mSystemConfig );

    // Initialize the audio driver
    if ( pDriver->initialize( cgResourceManager::getInstance(), pRenderDriver->getFocusWindow() ) == false )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize audio driver.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize required audio devices. The application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initPhysics ()
/// <summary>
/// Initialize the physics subsystems ready for simulation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initPhysics()
{
    cgConfigResult::Base Status;

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Initializing physics engine.\n") );

    // Instruct the primary physics engine to load configurataion
    cgPhysicsEngine * pEngine = cgPhysicsEngine::getInstance();
    Status = pEngine->loadConfig( mSystemConfig );
    
    // Were we able to match previous configuration data
    // to that now supported by the system?
    if ( Status == cgConfigResult::Mismatch )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Warning, _T("Previous configuration mismatch found when selecting physics simulation or hardware features. Selecting defaults.\n") );

        // Notify the user
        MessageBox( CG_NULL, _T("Previous configuration references features or modes not supported by the available physics subsystem(s) or hardware.\r\n\r\n")
                          _T("It is possible that the software, hardware or device drivers were changed since last time the application was run.\r\n\r\n")
                          _T("Physics engine configuration options will now be reset (this will not affect your profiles, video settings or game options)."),
                          _T("Physics Subsystem Change"), MB_OK | MB_ICONINFORMATION | MB_APPLMODAL );

        // Find sensible defaults
        Status = pEngine->loadDefaultConfig( );

    } // End if mismatch loading config

    // Was there an error loading the configuration
    if ( Status != cgConfigResult::Valid )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to load valid physics subsystem configuration.\n") );
        return false;
    
    } // End if error loading config

    // Save the physics configuration
    pEngine->saveConfig( mSystemConfig );

    // Initialize the physics subsystem.
    if ( pEngine->initialize( ) == false )
    {
        // Write debug info
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize physics subsystems due to previous errors.\n") );

        // Failure to initialize!
        MessageBox( CG_NULL, _T("Failed to initialize required physics subsystems. The application must now exit."), _T("Fatal Error"), MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        return false;
    
    } // End if failed to initialize

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initApplication ()
/// <summary>
/// Initialize all required aspects of the application ready for us to begin.
/// This includes setting up all required states that the application may enter
/// etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::initApplication( )
{
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadConfig ()
/// <summary>
/// Load the application config from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::loadConfig( const cgString & strFileName )
{
    // Store the configuration file name so that other systems
    // have access to it during initialization.
    mSystemConfig = cgFileSystem::resolveFileLocation( strFileName );

    // Get application configuration
    mMaximumFPS          = cgStringUtility::getPrivateProfileFloat( _T("Application"), _T("MaxFrameRate"), 0, mSystemConfig.c_str() );
    mMaximumSmoothedFPS  = cgStringUtility::getPrivateProfileFloat( _T("Application"), _T("MaxSmoothedFrameRate"), 59.9f, mSystemConfig.c_str() );
    bool bTimerSmoothing = (GetPrivateProfileInt( _T("Application"), _T("TimerSmoothing"), 0, mSystemConfig.c_str() ) != 0);
    cgTimer::getInstance()->enableSmoothing( bTimerSmoothing );
}

//-----------------------------------------------------------------------------
//  Name : begin ()
/// <summary>
/// Signals the beginning of the physical post-initialization stage.
/// From here on, the game engine has control over processing.
/// </summary>
//-----------------------------------------------------------------------------
int cgApplication::begin( cgThread * pThread )
{
    MSG  msg;

    // Get access to required systems
    cgAppStateManager * pAppStates = cgAppStateManager::getInstance();
    cgTimer           * pTimer     = cgTimer::getInstance();

    // 'Ping' the timer for the first frame
    pTimer->tick();

    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Entered main application processing loop.\n") );

    // Start main loop
    for( ;; )
    {
        // Game has a currently active state?
        if ( pAppStates->getActiveState() == CG_NULL )
        {
            // Write debug info and exit
            cgAppLog::write( cgAppLog::Info, _T("Application in idle state. Exiting.\n") );
            break;
        
        } // End if inactive

        // Owner thread requests we exit?
        if ( pThread && pThread->terminateRequested() )
        {
            // Write debug info and exit
            cgAppLog::write( cgAppLog::Info, _T("Application thread terminated. Exiting.\n") );
            break;
        
        } // End if inactive

        // Did we recieve a message, or are we idling ?
        if ( PeekMessage(&msg, CG_NULL, 0, 0, PM_REMOVE) ) 
        {
            if (msg.message == WM_QUIT)
            {
                // Write debug info and exit message pump
                cgAppLog::write( cgAppLog::Info, _T("Quit message received.\n") );
                break;
            
            } // End if quit message received
            
            // Send this message to all relevant windows/systems
            TranslateMessage( &msg );
            DispatchMessage ( &msg );
        } 
        else 
        {
            // Advance and render frame.
            frameAdvance( );

        } // End If messages waiting
  
    } // Until quit message is received

    return 0;
}


//-----------------------------------------------------------------------------
//  Name : frameAdvance ()
/// <summary>
/// Process the next application frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::frameAdvance( bool bRunSimulation /* = true */ )
{
    // Advance Game Frame.
    if ( frameBegin( bRunSimulation ) == true )
    {
        frameRender();
        frameEnd();
        return true;
    
    } // End if frame rendering can commence
    return false;
}

//-----------------------------------------------------------------------------
//  Name : shutDown ()
/// <summary>
/// Shuts down the game engine, and frees up all resources.
/// Note : You must explicitly call this method PRIOR to cleaning up the
/// engine with a call to cgEngineCleanup()
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::shutDown()
{
    // Write debug info
    cgAppLog::write( cgAppLog::Info, _T("Shutting down main application.\n") );
    
    // Shutdown Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : frameBegin () (Protected, Virtual)
/// <summary>
/// Called to signal that we are just about to commence rendering of the
/// current frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::frameBegin( bool bRunSimulation /* = true */ )
{
    // Retrieve required application objects
    cgAppStateManager * pAppStates = cgAppStateManager::getInstance();
    cgRenderDriver    * pDriver    = cgRenderDriver::getInstance();
    cgTimer           * pTimer     = cgTimer::getInstance();
    cgInputDriver     * pInput     = cgInputDriver::getInstance();

	cgInputDriver * inputDriver = cgInputDriver::getInstance();
	if ( !inputDriver->wasKeyPressed( cgKeys::F ) && inputDriver->isKeyPressed( cgKeys::F ) )
	{
		mMaximumFPS += 30.0f;
		if ( mMaximumFPS > 120.0f )
			mMaximumFPS = 30.0f;
	}

    // Allowing simulation to run?
    if ( bRunSimulation == true )
    {
        // In order to help avoid input lag when VSync is enabled, it is sometimes
        // recommended that the application cap its frame-rate manually where possible.
        // This helps to ensure that there is a consistant delay between input polls
        // rather than the variable time when the hardware is waiting to draw.
        cgFloat fCap = mMaximumFPS;
        if ( pDriver->isVSyncEnabled() == true )
        {
            cgFloat fSmoothedCap = mMaximumSmoothedFPS;
            if ( fSmoothedCap < fCap || fCap == 0 )
                fCap = fSmoothedCap;
        
        } // End if cap frame rate

        // Advance timer.
        pTimer->setSimulationSpeed( 1.0f );
        pTimer->tick( fCap );

    } // End if bRunSimulation
    else
    {
        // Do not advance simulation time.
        pTimer->setSimulationSpeed( 0.0f );
        pTimer->tick();
    
    } // End if !bRunSimulation

    // Skip if app is inactive
    if ( pDriver->isWindowActive() == false )
        return false;

    // Increment the frame counter
    pTimer->incrementFrameCounter( );

    // Allow input driver to poll
    pInput->poll();

    // Allow reference manager to process messages
    cgReferenceManager::processMessages( );

    // Update application states
    pAppStates->update();

    // Render driver can begin its per-frame processing. Returns false on failure
    // or if the device was lost for instance.
    if ( pDriver->beginFrame( true ) == false )
        return false;
    
    // Success, continue on to render
    return true;
}

//-----------------------------------------------------------------------------
//  Name : frameRender () (Protected, Virtual)
/// <summary>
/// Actually performs the default rendering of the current frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::frameRender()
{
    // Nothing in base implementation
}

//-----------------------------------------------------------------------------
//  Name : frameEnd () (Protected, Virtual)
/// <summary>
/// Called to signal that we have finished rendering the current frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgApplication::frameEnd()
{
    // Finished rendering, cleanup and present the buffer.
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();
    pDriver->endFrame( );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Application )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgApplication::processMessage( cgMessage * pMessage )
{
    // Bail if the message source has been unregistered since it
    // was sent. It will no longer be of interest to us.
    if ( pMessage->sourceUnregistered == true )
        return cgReference::processMessage( pMessage );

    // Who is the source of the message?
    if ( mFocusWindow && pMessage->fromId == mFocusWindow->getReferenceId() )
    {
        // This is from our created window. What is the message?
        if ( pMessage->messageId == cgSystemMessages::AppWindow_OnClose )
        {
            // Application needs to shut down.
            PostQuitMessage( 0 );
            
            // We processed this message.
            return true;

        } // End if AppWindow_OnClose

    } // End if window message

    // Message was not processed, pass to base.
    return cgReference::processMessage( pMessage );
}