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
// File : cgDX9RenderDriver.cpp                                              //
//                                                                           //
// Desc : Rendering class wraps the properties, initialization and           //
//        management of our rendering device. This includes the enumeration, //
//        creation and destruction of our D3D device and associated          //
//        resources.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9RenderDriver Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Rendering/Platform/cgDX9RenderingCapabilities.h>
#include <System/Platform/cgWinAppWindow.h>
#include <System/cgStringUtility.h>
#include <System/cgProfiler.h>
#include <System/cgMessageTypes.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/cgHardwareShaders.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>
#include <Input/cgInputDriver.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <Math/cgMathUtility.h>
#include <dxerr.h>

// Platform specific resource types
#include <Resources/Platform/cgDX9Texture.h>
#include <Resources/Platform/cgDX9StateBlocks.h>
#include <Resources/Platform/cgDX9ConstantBuffer.h>
#include <Resources/Platform/cgDX9RenderTarget.h>
#include <Resources/Platform/cgDX9DepthStencilTarget.h>
#include <Resources/Platform/cgDX9VertexBuffer.h>
#include <Resources/Platform/cgDX9IndexBuffer.h>
#include <Resources/Platform/cgDX9HardwareShaders.h>

// ToDo: 9999 - Potential optimization is to move viewport and render area
// constants out of the camera buffer and into their own to avoid the need
// to reupload so many constants if / when it changes?

//-----------------------------------------------------------------------------
// Module Local Enumerations.
//-----------------------------------------------------------------------------
namespace DX9HardwareVendor
{
    enum Base
    {
        NVIDIA      = 0x10DE,
        AMD         = 0x1002,
        INTEL       = 0x8086
    };

}; // End Namespace : DX9HardwareVendor

// ToDo: 9999 - Print Post initialization
/* // Write debug information
    if ( m_ShaderModel == cgShaderModel::SM_2_0 )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 2.0 communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_2_a )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 2.a (ATI) communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_2_b )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 2.b (NVIDIA) communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_3_0 )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 3.0 communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_4_0 )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 4.0 communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_4_1 )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 4.1 communication layer.\n") );
    else if ( m_ShaderModel == cgShaderModel::SM_5_0 )
        cgAppLog::write( cgAppLog::Info, _T("Successfully initialized shader model 5.0 communication layer.\n") );*/

//-----------------------------------------------------------------------------
// Module Local Definitions.
//-----------------------------------------------------------------------------
#define FETCH4_ENABLE  ((cgUInt32)MAKEFOURCC( 'G', 'E', 'T', '4' ))
#define FETCH4_DISABLE ((cgUInt32)MAKEFOURCC( 'G', 'E', 'T', '1' ))

///////////////////////////////////////////////////////////////////////////////
// cgDX9RenderDriver Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9RenderDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderDriver::cgDX9RenderDriver()
{
    // Initialize variables to sensible defaults
    mD3D                      = CG_NULL;
    mD3DDevice                = CG_NULL;
    mD3DInitialize            = CG_NULL;
    mNextQueryId              = 0;
    mFrontCounterClockwise    = false;

    // Initialize stacks with space for the default elements
    // NB: m_TargetStacks starts with an initial size of 0
    // unlike most other stacks. An implcit 'beginTargetRender()'
    // call will be made during driver initialization for this purpose.
    // (No stacks at this time).
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9RenderDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderDriver::~cgDX9RenderDriver()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : releaseOwnedResources () (Virtual)
/// <summary>
/// Release any resources that the render driver may have loaded via its
/// resource manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::releaseOwnedResources()
{
    // Release internal constant buffers
    mSceneConstants.close(true);
    mCameraConstants.close(true);
    mShadowConstants.close(true);
    mMaterialConstants.close(true);
    mObjectConstants.close(true);
    mWorldConstants.close(true);
    mVertexBlendingConstants.close(true);
    mVertexBlendingTexture.close( true );
    
    // Release all stack held resources.
    while (mTargetStack.size() > 0)
    {
        TargetData & Data = mTargetStack.top();
        for ( size_t i = 0; i < Data.renderTargets.size(); ++i )
            Data.renderTargets[i].close(true);
        Data.depthStencil.close(true);
        mTargetStack.pop();

    } // Next TargetData

    // Call base class.
    cgRenderDriver::releaseOwnedResources();
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::dispose( bool bDisposeBase )
{
    DeclarationMap::iterator itDeclaration;

    // We are in the process of disposing?
    mDisposing = true;

    // Release any active resources loaded via the resource manager.
    releaseOwnedResources();

    // Release any allocated memory
    delete mD3DInitialize;
    
    // Release any vertex declarators
    for ( itDeclaration = mVertexDeclarators.begin(); itDeclaration != mVertexDeclarators.end(); ++itDeclaration )
    {
        if ( itDeclaration->second != CG_NULL )
            itDeclaration->second->Release();

    } // Next Declarator
    mVertexDeclarators.clear();

    // Release any active queries.
    clearQueries();

    // Release any DirectX objects.
    if ( mD3DDevice )
        mD3DDevice->Release();
    if ( mD3D )
        mD3D->Release();

    // Reset any variables
    mD3DDevice                = CG_NULL;
    mD3D                      = CG_NULL;
    mD3DInitialize            = CG_NULL;
    mNextQueryId              = 0;
    mFrontCounterClockwise    = false;

    // Clear containers
    while (mTargetStack.size() > 0)
        mTargetStack.pop();

    // Initialize stacks with space for the default elements
    // NB: m_TargetStacks starts with an initial size of 0
    // unlock most other stacks. An implcit 'beginTargetRender()'
    // call will be made during driver initialization for this purpose.
    // (No stacks at this time).

    // Call base if requested
    if ( bDisposeBase == true )
        cgRenderDriver::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : loadConfig () (Virtual)
/// <summary>
/// Load the render driver configuration from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgDX9RenderDriver::loadConfig( const cgString & strFileName )
{
    STRING_CONVERT;
    D3DDISPLAYMODE  MatchMode;
    bool            bFoundMode = false;
    
    // Fail if config already loaded
    if ( mConfigLoaded )
        return cgConfigResult::Error;

    // Retrieve configuration options if provided
    if ( !strFileName.empty() )
    {
        cgTChar Buffer[256];
        LPCTSTR strSection          = _T("RenderDriver");
        GetPrivateProfileString( strSection, _T("DeviceName"), _T(""), Buffer, 256, strFileName.c_str() );
        mConfig.deviceName         = cgString::trim(Buffer);
        mConfig.windowed           = GetPrivateProfileInt( strSection, _T("Windowed"), 0, strFileName.c_str() ) > 0;
        mConfig.useHardwareTnL     = GetPrivateProfileInt( strSection, _T("UseHardwareTnL"), 1, strFileName.c_str() ) > 0;
        mConfig.useVSync           = GetPrivateProfileInt( strSection, _T("UseVSync"), 1, strFileName.c_str() ) > 0;
        mConfig.useTripleBuffering = GetPrivateProfileInt( strSection, _T("UseTripleBuffering"), 0, strFileName.c_str() ) > 0;
        mConfig.primaryDepthBuffer = GetPrivateProfileInt( strSection, _T("PrimaryDepthBuffer"), 0, strFileName.c_str() ) > 0;
        mConfig.width              = (cgUInt32)GetPrivateProfileInt( strSection, _T("Width"), 0, strFileName.c_str() );
        mConfig.height             = (cgUInt32)GetPrivateProfileInt( strSection, _T("Height"), 0, strFileName.c_str() );
        mConfig.refreshRate        = (cgUInt32)GetPrivateProfileInt( strSection, _T("RefreshRate"), 0, strFileName.c_str() );
        mConfig.debugVShader       = GetPrivateProfileInt( strSection, _T("DebugVShader"), 0, strFileName.c_str() ) > 0;
        mConfig.debugPShader       = GetPrivateProfileInt( strSection, _T("DebugPShader"), 0, strFileName.c_str() ) > 0;
        mConfig.usePerfHUD         = GetPrivateProfileInt( strSection, _T("UsePerfHUD"), 0, strFileName.c_str() ) > 0;
        mConfig.useVTFBlending     = GetPrivateProfileInt( strSection, _T("UseVTFBlending"), 0, strFileName.c_str() ) > 0;

    } // End if config provided

    // Release previous D3D object if already created
    if ( mD3D != CG_NULL )
        mD3D->Release();
    
    // Create a D3D Object (This is needed by the enumeration etc).
    mD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if ( mD3D == CG_NULL ) 
    {
        cgAppLog::write( cgAppLog::Error, _T( "No compatible Direct3D object could be created.\n" ) );
        return cgConfigResult::Error;

    } // End if failure

	// Before we do any enumeration and choose a display mode, compute the default adapter's aspect ratio.
	// This is useful for handling widescreen displays in fullscreen mode.
	D3DDISPLAYMODE DisplayMode;
	if ( FAILED( mD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &DisplayMode ) ) )
	{
        cgAppLog::write( cgAppLog::Error, _T( "Could not get the adapter's original display mode during initialization.\n" ) );
        return cgConfigResult::Error;
	
    } // End if Failure
    mAdapterAspectRatio = (cgFloat)DisplayMode.Width / (cgFloat)DisplayMode.Height;
	
    // Release previous initialization object
    delete mD3DInitialize;

    // Create an init item for enumeration (the second parameter instructs the init
    // class as to whether it should strictly enforce the configuration options, or simply
    // let the system find a good match).
    mD3DInitialize = new cgDX9RenderDriverInit( mConfig, strFileName.empty() == false );
    
    // Enumerate the system graphics adapters    
    if ( FAILED(mD3DInitialize->enumerate( mD3D )) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Device enumeration failed. The application will now exit.\n" ) );
        return cgConfigResult::Error;

    } // End if Failure

    // Attempt to find a good default fullscreen set
    MatchMode.Width       = mConfig.width;
    MatchMode.Height      = mConfig.height;
    MatchMode.Format      = D3DFMT_UNKNOWN;
    MatchMode.RefreshRate = mConfig.refreshRate;
    bFoundMode            = mD3DInitialize->findBestFullScreenMode( mD3DSettings, &MatchMode, true, false );
    
    // Mismatched?
    if ( mConfig.windowed == false && bFoundMode == false )
        return cgConfigResult::Mismatch;

    // Attempt to find a good default windowed set
    bFoundMode = mD3DInitialize->findBestWindowedMode( mD3DSettings, true, false );
    
    // Mismatched?
    if ( mConfig.windowed == true && bFoundMode == false )
        return cgConfigResult::Mismatch;

    // Enable triple buffering for fullscreen modes if requested.
    mD3DSettings.fullScreenSettings.tripleBuffering = mConfig.useTripleBuffering;

    // What mode are we using?
    mD3DSettings.windowed = mConfig.windowed;
    
    // Set config to this closest match (in case no filename was specified)
    cgDX9Settings::Settings * pSettings = mD3DSettings.getSettings();
    mConfig.useVSync       = (pSettings->presentInterval == D3DPRESENT_INTERVAL_ONE);
    mConfig.useHardwareTnL = (pSettings->vertexProcessingType != SOFTWARE_VP);

    // Overwrite fullscreen options
    if ( mConfig.windowed == false )
    {
        mConfig.width          = pSettings->displayMode.Width;
        mConfig.height         = pSettings->displayMode.Height;
        mConfig.refreshRate    = pSettings->displayMode.RefreshRate;
    
    } // End if not windowed

    // Retrieve selected adapter information and store it in the configuration
    D3DADAPTER_IDENTIFIER9 Identifier;
    mD3D->GetAdapterIdentifier( pSettings->adapterOrdinal, 0, &Identifier );
    mConfig.deviceName = cgString::trim(stringConvertA2CT(Identifier.Description));

    // Signal that we have settled on good config options
    mConfigLoaded = true;
    
    // Options are valid. Success!!
    return cgConfigResult::Valid;
}

//-----------------------------------------------------------------------------
//  Name : loadDefaultConfig () (Virtual)
/// <summary>
/// Load a default configuration for the render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgDX9RenderDriver::loadDefaultConfig( bool bWindowed /* = false  */ )
{
    // Pick sensible defaults for the mode matching
    mConfig.deviceName         = cgString::Empty;
    mConfig.windowed           = bWindowed;
    mConfig.useHardwareTnL     = true;
    mConfig.useVSync           = false;
    mConfig.useTripleBuffering = false;
    mConfig.primaryDepthBuffer = false;
    mConfig.width              = 800;
    mConfig.height             = 600;
    mConfig.refreshRate        = 0;
    mConfig.debugPShader       = false;
    mConfig.debugVShader       = false;
    mConfig.usePerfHUD         = false;
    mConfig.useVTFBlending     = false;
    
    // Pass through to the LoadConfig function
    return loadConfig( _T("") );
}

//-----------------------------------------------------------------------------
//  Name : saveConfig () (Virtual)
/// <summary>
/// Save the render driver configuration from the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::saveConfig( const cgString & strFileName )
{
    LPCTSTR strSection = _T("RenderDriver");

    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Save configuration options
    WritePrivateProfileString( strSection, _T("DeviceName"), mConfig.deviceName.c_str(), strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("Windowed"), mConfig.windowed, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UseHardwareTnL"), mConfig.useHardwareTnL, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UseVSync"), mConfig.useVSync, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UseTripleBuffering"), mConfig.useTripleBuffering, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("PrimaryDepthBuffer"), mConfig.primaryDepthBuffer, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("Width"), mConfig.width, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("Height"), mConfig.height, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("RefreshRate"), mConfig.refreshRate, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("DebugVShader"), mConfig.debugVShader, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("DebugPShader"), mConfig.debugPShader, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UsePerfHUD"), mConfig.usePerfHUD, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UseVTFBlending"), mConfig.useVTFBlending, strFileName.c_str() );

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Initialize the render driver using the window specified.
/// Note : In this case, you will be responsible for informing the render driver
/// about any important messages (i.e. window resize events).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::initialize( cgResourceManager * pResources, cgAppWindow * pFocusWindow, cgAppWindow * pOutputWindow )
{
    // Configuration must be loaded
    if ( !mConfigLoaded )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Render driver configuration must be loaded prior to initialization!\n"));
        return false;

    } // End if no config loaded
    
    if ( isInitialized() )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Render driver must be released before it can be initialized for a second time!\n"));
        return false;
    
    } // End if already initialized

    // If no output window is specified, use the focus window.
    if ( !pOutputWindow )
        pOutputWindow = pFocusWindow;

    // Only 'cgWinAppWindow' type is supported.
    cgWinAppWindow * pWinFocusWindow  = dynamic_cast<cgWinAppWindow*>(pFocusWindow);
    cgWinAppWindow * pWinOutputWindow = dynamic_cast<cgWinAppWindow*>(pOutputWindow);
    if ( !pWinFocusWindow )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX9 render driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast

    // Get native window handles
    HWND hFocusWindow  = pWinFocusWindow->getWindowHandle();
    HWND hOutputWindow = pWinOutputWindow->getWindowHandle();

    // Create the direct 3d device etc.
    cgUInt32 nFlags = (cgGetEngineConfig().multiThreaded) ? D3DCREATE_MULTITHREADED : 0;
    if ( FAILED( mD3DInitialize->createDisplay( mD3DSettings, nFlags, hFocusWindow, hOutputWindow, CG_NULL, CG_NULL, 0, 0,
                                                  0, false, mConfig.debugVShader, mConfig.debugPShader,
                                                  mConfig.usePerfHUD, false ) ))
    {
        cgAppLog::write( cgAppLog::Error, _T("Device creation failed. The application will now exit.\n") );
        return false;

    } // End if Failed

    // Retrieve created device
    mD3DDevice = mD3DInitialize->getDirect3DDevice( );

    // Store the current window size in the settings for accountabilities sake
    if ( mConfig.windowed )
    {
        cgSize Size = pOutputWindow->getClientSize();
        mD3DSettings.windowedSettings.displayMode.Width  = Size.width;
        mD3DSettings.windowedSettings.displayMode.Height = Size.height;
    
    } // End if windowed

	// Perform an initial clear on the framebuffer targets
    if ( mConfig.primaryDepthBuffer )
	    mD3DDevice->Clear( 0, CG_NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0 );
    else
        mD3DDevice->Clear( 0, CG_NULL, D3DCLEAR_TARGET, 0, 1.0f, 0 );

    // Call base class implementation.
    return cgRenderDriver::initialize( pResources, pFocusWindow, pOutputWindow );
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Initialize the render driver
/// Note : This overload will automatically create the window on your behalf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::initialize( cgResourceManager * pResources, const cgString & WindowTitle /* = _T("Render Output") */, cgInt32 IconResource /* = -1 */ )
{
    // Configuration must be loaded
    if ( !mConfigLoaded )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Render driver configuration must be loaded prior to initialization!\n"));
        return false;
    } // End if no config loaded
    
    if ( isInitialized() )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Render driver must be released before it can be initialized for a second time!\n"));
        return false;
    
    } // End if already initialized

    // Create a new application window.
    mFocusWindow  = cgAppWindow::createInstance();
    mOutputWindow = mFocusWindow;
    mOwnsWindow   = true;
    cgWinAppWindow * pWinAppWindow = dynamic_cast<cgWinAppWindow*>(mFocusWindow);
    if ( !pWinAppWindow )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX9 render driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast
    if ( !mFocusWindow->create( (mConfig.windowed == false), mConfig.width, mConfig.height, WindowTitle, IconResource ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Application window creation failed. The application will now exit.\n"));
        return false;

    } // End if failed

    // Create the direct 3d device etc.
    cgUInt32 nFlags = (cgGetEngineConfig().multiThreaded) ? D3DCREATE_MULTITHREADED : 0;
    if ( FAILED( mD3DInitialize->createDisplay( mD3DSettings, nFlags, pWinAppWindow->getWindowHandle(), pWinAppWindow->getWindowHandle(), 
                                                CG_NULL, CG_NULL, 0, 0, 0, false, mConfig.debugVShader, mConfig.debugPShader,
                                                mConfig.usePerfHUD, false ) ))
    {
        cgAppLog::write( cgAppLog::Error, _T("Device creation failed. The application will now exit.\n") );
        return false;

    } // End if Failed

    // Retrieve created device
    mD3DDevice = mD3DInitialize->getDirect3DDevice( );

    // Store the current window size in the settings for accountabilities sake
    if ( mConfig.windowed )
    {
        cgSize Size = mOutputWindow->getClientSize();
        mD3DSettings.windowedSettings.displayMode.Width  = Size.width;
        mD3DSettings.windowedSettings.displayMode.Height = Size.height;
    
    } // End if windowed

    // Perform an initial clear on the framebuffer targets
    if ( mConfig.primaryDepthBuffer )
	    mD3DDevice->Clear( 0, CG_NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0 );
    else
        mD3DDevice->Clear( 0, CG_NULL, D3DCLEAR_TARGET, 0, 1.0f, 0 );

    // Show the window
    mFocusWindow->show();

    // Call base class implementation to finish up.
    return cgRenderDriver::initialize( pResources, WindowTitle, IconResource );
}

//-----------------------------------------------------------------------------
//  Name : updateDisplayMode () (Virtual)
/// <summary>
/// Alter the display mode after initialization, and optionally switch between
/// windowed and fullscreen mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::updateDisplayMode( const cgDisplayMode & mode, bool windowed, bool verticalSync )
{
    // Is this a no-op?
    if ( mConfig.width == mode.width && mConfig.height == mode.height && mConfig.refreshRate == mode.refreshRate &&
         mConfig.windowed == windowed )
         return true;

    // Log the changes being made.
    cgString modeInfo = cgString::format( _T("Application selected a new %s display mode of %ix%ix%ibpp @ %ihz."), 
                                         (windowed) ? _T("windowed") : _T("fullscreen"), mode.width, mode.height, 
                                         mode.bitDepth, mode.refreshRate );
    cgAppLog::write( cgAppLog::Info, _T("%s\n"), modeInfo.c_str() );

    // If we're currently in windowed mode and the caller is not
    // requesting that we switch to fullscreen, just resize the window.
    bool success = true;
    if ( isWindowed() && windowed )
    {
        // Update the device configuration
        mConfig.width       = mode.width;
        mConfig.height      = mode.height;
        mConfig.refreshRate = (cgInt32)mode.refreshRate;
        mConfig.windowed    = windowed;
        mConfig.useVSync    = verticalSync;

        // Update settings.
        mD3DSettings.windowedSettings.presentInterval = (verticalSync) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

        // Update the size of the focus window.
        mFocusWindow->setClientSize( cgSize( mode.width, mode.height ) );

    } // End if remaining windowed
    else
    {
        // Switching to windowed from fullscreen?
        if ( windowed )
        {
            // Update the device configuration
            mConfig.width       = mode.width;
            mConfig.height      = mode.height;
            mConfig.refreshRate = (cgInt32)mode.refreshRate;
            mConfig.windowed    = windowed;
            mConfig.useVSync    = verticalSync;

            // Update settings.
            mD3DSettings.windowedSettings.presentInterval = (verticalSync) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

            // Switch the focus window to the new mode and then set its size.
            // This will automatically trigger a window resize event that will
            // cause the device to be reset.
            mD3DSettings.windowed = true;
            mFocusWindow->setFullScreenMode( false );
            mFocusWindow->setClientSize( cgSize( mode.width, mode.height ) );

        } // End if windowed
        else
        {
            // We're switching to or remaining in fullscreen.
            // First make sure that window resize events are suppressed.
            mSuppressResizeEvent = true;

            // Update settings.
            mD3DSettings.fullScreenSettings.displayMode.Width       = mode.width;
            mD3DSettings.fullScreenSettings.displayMode.Height      = mode.height;
            mD3DSettings.fullScreenSettings.displayMode.RefreshRate = (UINT)mode.refreshRate;
            mD3DSettings.fullScreenSettings.presentInterval         = (verticalSync) ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
            mD3DSettings.windowed = false;

            // Switch the focus window to the new mode and then set its size.
            mFocusWindow->setFullScreenMode( true );
            mFocusWindow->setClientSize( cgSize( mode.width, mode.height ) );
        
            // Clean up prior to reset
            onDeviceLost();

            // Reset the display
            HRESULT hRet = mD3DInitialize->resetDisplay( mD3DDevice, mD3DSettings, CG_NULL, CG_NULL, false );
            if ( FAILED( hRet ) )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to reset display device after window resize. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                mLostDevice = true;

            } // End if failed to reset
            else
            {
                // Restore post-reset
                onDeviceReset();
            
            } // End if reset success

            // Window resize events can continue.
            mSuppressResizeEvent = false;

            // Update the device configuration
            mConfig.width       = mode.width;
            mConfig.height      = mode.height;
            mConfig.refreshRate = (cgInt32)mode.refreshRate;
            mConfig.windowed    = windowed;
            mConfig.useVSync    = verticalSync;

            // Perform final updates and notify listeners.
            cgRenderDriver::windowResized( mode.width, mode.height );

        } // End if fullscreen

    } // End if other

    // Done
    return success;
}

//-----------------------------------------------------------------------------
//  Name : postInit () (Protected Virtual)
/// <summary>
/// Complete the render driver initialization process
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::postInit()
{
    STRING_CONVERT;  // For string conversion macro

    // Retrieve selected adapter information
    D3DADAPTER_IDENTIFIER9 Identifier;
    cgDX9Settings::Settings * pSettings = mD3DSettings.getSettings();
    mD3D->GetAdapterIdentifier( pSettings->adapterOrdinal, 0, &Identifier );

    // Build component parts of the information required for log output
    cgString strDriver    = cgString(stringConvertA2CT(Identifier.Driver));
    cgString strCertified = (Identifier.WHQLLevel == 0) ? _T(" whql") : _T("");
    cgString strDriverVersion = cgString::format( _T("%i.%i.%i.%i"), HIWORD(Identifier.DriverVersion.HighPart), LOWORD(Identifier.DriverVersion.HighPart), 
                                                  HIWORD(Identifier.DriverVersion.LowPart), LOWORD(Identifier.DriverVersion.LowPart) );
    
    // Build string for outputting selected device information to log
    cgString strDeviceName = cgString::trim(stringConvertA2CT(Identifier.Description));
    cgString strDeviceInfo = _T("Selected D3D9 device '") + strDeviceName + _T("' (Driver: ") + 
                             strDriver + _T(" - ") + strDriverVersion + strCertified + _T(").\n");

    // Output information
    cgAppLog::write( cgAppLog::Info, strDeviceInfo.c_str() );

    // Build component parts for the mode information required for log output
    strDeviceInfo = cgString::format( _T("Selected %s display mode of %ix%ix%ibpp @ %ihz.\n"), (mD3DSettings.windowed) ? _T("windowed") : _T("fullscreen"), 
                                      pSettings->displayMode.Width, pSettings->displayMode.Height,
                                      cgBufferFormatEnum::formatBitsPerPixel( cgDX9BufferFormatEnum::formatFromNative(pSettings->displayMode.Format) ),
                                      pSettings->displayMode.RefreshRate );

    // Output information
    cgAppLog::write( cgAppLog::Info, strDeviceInfo.c_str() );

    // Store information about the type of hardware in use
    switch ( Identifier.VendorId )
    {
        case DX9HardwareVendor::NVIDIA:
            mHardwareType = cgHardwareType::NVIDIA;
            break;

        case DX9HardwareVendor::AMD:
            mHardwareType = cgHardwareType::AMD;
            break;

        default:
            mHardwareType = cgHardwareType::Generic;
            break;

    } // End Switch VendorId

    // Retrieve a reference to the profiler so that we don't have to retrieve it on the fly.
    mProfiler = cgProfiler::getInstance();

    // Call base class implementation.
    if ( !cgRenderDriver::postInit( ) )
        return false;

    // Populate remaining capabilities and display modes.
    dynamic_cast<cgDX9RenderingCapabilities*>(mCaps)->postInit( mD3DInitialize, mD3DSettings.fullScreenSettings.adapterOrdinal );

    // Create internal constant buffers
    bool bSuccess = true;
    if ( !mResourceManager->createConstantBuffer( &mSceneConstants, mDriverShaderHandle, _T("_cbScene"), cgDebugSource() ) ||
         !mResourceManager->createConstantBuffer( &mCameraConstants, mDriverShaderHandle, _T("_cbCamera"), cgDebugSource() ) ||
         !mResourceManager->createConstantBuffer( &mMaterialConstants, mDriverShaderHandle, _T("_cbMaterial"), cgDebugSource() ) ||
         !mResourceManager->createConstantBuffer( &mObjectConstants, mDriverShaderHandle, _T("_cbObject"), cgDebugSource() ) ||
         !mResourceManager->createConstantBuffer( &mWorldConstants, mDriverShaderHandle, _T("_cbWorld"), cgDebugSource() ) ||
         !mResourceManager->createConstantBuffer( &mVertexBlendingConstants, mDriverShaderHandle, _T("_cbVertexBlending"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create internal render driver constant buffers on the selected device.\n") );
        return false;

    } // End if failed

    // Debugging
    cgAssert( mSceneConstants->getDesc().length == sizeof(_cbScene) );
    cgAssert( mCameraConstants->getDesc().length == sizeof(_cbCamera) );
    cgAssert( mMaterialConstants->getDesc().length == sizeof(_cbMaterial) );
    cgAssert( mObjectConstants->getDesc().length == sizeof(_cbObject) );
    cgAssert( mWorldConstants->getDesc().length == sizeof(_cbWorld) );
    cgAssert( mVertexBlendingConstants->getDesc().length == sizeof(_cbVertexBlending) );

    /*cgDX9Settings::Settings * pSettings = mD3DSettings.getSettings();
    if (FAILED(mD3D->CheckDeviceFormat(pSettings->adapterOrdinal, pSettings->DeviceType, pSettings->displayMode.format,
                                         D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
    {   
        MessageBox(NULL, L"D3DFMT_A32B32G32R32F cannot be used for vertex textures.", L"Failed", MB_OK | MB_ICONERROR);   
        return false;
    }*/

    // Create a texture large enough to contain vertex blending matrices
    // for reading via vertex texture fetch.
    if ( mConfig.useVTFBlending )
    {
        cgImageInfo ImageDesc;
        ImageDesc.width     = cgMathUtility::nextPowerOfTwo( cgDX9RenderingCapabilities::MaxVBTSlotsVTF * 3 ) * 2;
        ImageDesc.height    = 1;
        ImageDesc.mipLevels = 1;
        ImageDesc.format    = cgBufferFormat::R32G32B32A32_Float;
        ImageDesc.pool      = cgMemoryPool::Default;
        ImageDesc.type      = cgBufferType::Texture2D;
        ImageDesc.dynamic   = true;
        if ( !mResourceManager->createTexture( &mVertexBlendingTexture, ImageDesc, cgResourceFlags::ForceNew, _T("Core::Textures::VertexBlending"), cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create vertex blending matrix buffer texture on the selected device.\n") );
            return false;

        } // End if failed

    } // End if useVTFBlending

    // Create initial 'default' render target data stack entry.
    if ( !beginTargetRender( mDeviceFrameBuffer, mDeviceDepthStencilTarget ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind internal device render and depth stencil targets to the selected device.\n") );
        return false;

    } // End if failed
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : cameraUpdated() 
/// <summary>
/// This function should be called by the application if the currently
/// active camera has changed any of it's properties (Viewport, view or
/// projection matrix) after a call to 'beginFrame'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::cameraUpdated()
{
    // Call base class implementation
    if ( !cgRenderDriver::cameraUpdated() )
        return false;

    // Update camera constant buffer
    cgCameraNode * pCamera = mCameraStack.back();
    if ( pCamera != CG_NULL ) 
    {
        cgConstantBuffer * pCameraBuffer = mCameraConstants.getResource( true );
        cgAssert( pCameraBuffer != CG_NULL );

        // Lock the buffer ready for population
        _cbCamera * pCameraData = CG_NULL;
        if ( !(pCameraData = (_cbCamera*)pCameraBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to lock camera constant buffer in preparation for device update.\n") );
            return false;

        } // End if failed

        // Compute additional matrices
        cgMatrix mtxInvView, mtxInvProj;
        cgMatrix::inverse( mtxInvView, mViewMatrix );
        cgMatrix::inverse( mtxInvProj, mProjectionMatrix );
        
        // Compute the maximum length of a ray from the camera to the far plane given the current FOV
        cgToDo( "Carbon General", "Compute mathematically" );
        using namespace cgVolumeGeometry;
        cgVector3 vRay = pCamera->getFrustum().points[ LeftTopFar ] - pCamera->getPosition();
        cgFloat fMaxDistance = cgVector3::length( vRay );
        
        // Update buffer.
        pCameraData->viewMatrix                 = mViewMatrix;
        pCameraData->projectionMatrix           = mProjectionMatrix;
        pCameraData->viewProjectionMatrix       = mViewProjectionMatrix;
        pCameraData->inverseViewMatrix          = mtxInvView;
        pCameraData->inverseProjectionMatrix    = mtxInvProj;
        pCameraData->position                   = pCamera->getPosition();
        pCameraData->direction                  = pCamera->getZAxis();
        pCameraData->nearClip                   = pCamera->getNearClip();
        pCameraData->farClip                    = pCamera->getFarClip();
        pCameraData->nearRecip                  = 1.0f / pCameraData->nearClip;
        pCameraData->farRecip                   = 1.0f / pCameraData->farClip;
        pCameraData->maximumDistance            = fMaxDistance;
        pCameraData->maximumDistanceRecip       = 1.0f / fMaxDistance;
        pCameraData->rangeScale                 = 1.0f / (pCameraData->farClip - pCameraData->nearClip);
        pCameraData->rangeBias                  = -pCameraData->nearClip / (pCameraData->farClip - pCameraData->nearClip);
        pCameraData->inverseRangeScale          = (pCameraData->farClip - pCameraData->nearClip);
        pCameraData->inverseRangeBias           = pCameraData->inverseRangeScale * -pCameraData->rangeBias;
        pCameraData->screenToViewScale.x        =   2.0f / mProjectionMatrix._11;
        pCameraData->screenToViewScale.y        =  -2.0f / mProjectionMatrix._22;
        pCameraData->screenToViewBias.x         =  (-1.0f / mProjectionMatrix._11) + (-mProjectionMatrix._41 / mProjectionMatrix._11);
        pCameraData->screenToViewBias.y         =  ( 1.0f / mProjectionMatrix._22) + (-mProjectionMatrix._42 / mProjectionMatrix._22);
		pCameraData->jitterAA                   = pCamera->getJitterAA( );

        // Note: The following variables are not updated. When unlocked, the values 
        // already currently stored in the internal system memory buffer will be used.
        //pCameraData->viewportSize;
        //pCameraData->viewportOffset;
        //pCameraData->targetSize;
        //pCameraData->screenUVAdjustBias;
        
        // Unlock the buffer. If it is currently bound to the device
        // then the appropriate constants will be automatically updated
        // next time 'drawPrimitive*' is called.
        pCameraBuffer->unlock();

    } // End if camera available

    // Clip planes need to be re-transformed.
    ClipPlaneData & ClipPlanes = mClipPlaneStack.top();
    if ( ClipPlanes.planeCount > 0 )
        setUserClipPlanes( ClipPlanes.planes, ClipPlanes.planeCount, false );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setWorldTransform ()
/// <summary>
/// Set the world matrix state to the device. Providing a NULL argument will
/// result in an identity matrix being assumed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setWorldTransform( const cgMatrix * pMatrix )
{
    // ToDo: 9999 - Rollback on failure?

    // Validate requirements
    cgAssert( isInitialized() == true );

    // Call base class implementation
    if ( !cgRenderDriver::setWorldTransform( pMatrix ) )
        return false;
    
    // Update 'world' constant buffer.
    cgConstantBuffer * pWorldBuffer = mWorldConstants.getResource( true );
    cgAssert( pWorldBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbWorld * pWorldData = CG_NULL;
    if ( !(pWorldData = (_cbWorld*)pWorldBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock world transformation constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Compute additional matrices
    cgMatrix mtxIT = mWorldTransformStack.top();
    mtxIT._41 = 0; mtxIT._42 = 0; mtxIT._43 = 0; mtxIT._44 = 1; // Remove translation component.
    if ( cgMatrix::inverse( mtxIT, mtxIT ) )
        cgMatrix::transpose( mtxIT, mtxIT );
    
    // Update buffer.
    pWorldData->transform   = mWorldTransformStack.top();
    pWorldData->inverseTransposeTransform = mtxIT;
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pWorldBuffer->unlock();

    // If the constant buffer is not currently bound to the device, do so now.
    if ( !pWorldBuffer->isBound() )
        setConstantBufferAuto( mWorldConstants );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setSamplerState ()
/// <summary>
/// Apply the specified sampler state to the indicated sampler register.
/// Supplying an invalid (NULL) handle will result in the default sampler 
/// states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setSamplerState( cgUInt32 nSamplerIndex, const cgSamplerStateHandle & hStates )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( nSamplerIndex < MaxSamplerSlots );
    cgAssert( mSamplerStateStack[nSamplerIndex].size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mSamplerStateStack[nSamplerIndex].top() == hStates )
        return true;

    // Retrieve the final commited state object.
    cgSamplerStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultSamplerState;
    cgDX9SamplerState * pState = (cgDX9SamplerState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( nSamplerIndex );
    if ( !pBlock )
    {
        // Fail.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified sampler state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    HRESULT hRet;
    if ( FAILED( hRet = pBlock->Apply() ) )
    {
        // Rollback and fail.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified sampler state to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        pBlock->Release();
        return false;
    
    } // End if failed

    // Clean up
    pBlock->Release();

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setSamplerState( nSamplerIndex, hStates );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreSamplerState () (Protected, Virtual)
/// <summary>
/// Re-bind the current sampler state object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreSamplerState( cgUInt32 nSamplerIndex )
{
    cgSamplerStateHandle & hCurrentState = mSamplerStateStack[nSamplerIndex].top();
    cgDX9SamplerState * pState = (cgDX9SamplerState*)hCurrentState.getResource(true);
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( nSamplerIndex );
    if ( pBlock )
    {
        pBlock->Apply();
        pBlock->Release();
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : setDepthStencilState ()
/// <summary>
/// Apply the specified depth stencil state to the device. Supplying an invalid 
/// (NULL) handle will result in the default depth stencil states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setDepthStencilState( const cgDepthStencilStateHandle & hStates, cgUInt32 nStencilRef )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mDepthStencilStateStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && 
         mDepthStencilStateStack.top().handle == hStates &&
         mDepthStencilStateStack.top().stencilRef == nStencilRef )
        return true;

    // Retrieve the final commited state object.
    cgDepthStencilStateHandle hNewState = hStates.isValid() ? hStates : mDefaultDepthStencilState;
    cgDX9DepthStencilState * pState = (cgDX9DepthStencilState*)hNewState.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( mFrontCounterClockwise );
    if ( !pBlock )
    {
        // Fail.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified depth stencil state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    HRESULT hRet;
    if ( FAILED( hRet = pBlock->Apply() ) )
    {
        // Fail.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified depth stencil state to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        pBlock->Release();
        return false;
    
    } // End if failed

    // Clean up
    pBlock->Release();

    // Setup stencil reference value.
    mD3DDevice->SetRenderState( D3DRS_STENCILREF, nStencilRef );

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setDepthStencilState( hStates, nStencilRef );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreDepthStencilState () (Protected, Virtual)
/// <summary>
/// Re-bind the current depth stencil state object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreDepthStencilState( )
{
    DepthStencilStateData & CurrentState = mDepthStencilStateStack.top();
    cgDX9DepthStencilState * pState = (cgDX9DepthStencilState*)CurrentState.handle.getResource(true);
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( mFrontCounterClockwise );
    if ( pBlock )
    {
        pBlock->Apply();
        pBlock->Release();
    
    } // End if valid
    mD3DDevice->SetRenderState( D3DRS_STENCILREF, CurrentState.stencilRef );
}

//-----------------------------------------------------------------------------
//  Name : setRasterizerState ()
/// <summary>
/// Apply the specified rasterizer state to the device. Supplying an invalid 
/// (NULL) handle will result in the default rasterizer states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setRasterizerState( const cgRasterizerStateHandle & hStates )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mRasterizerStateStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mRasterizerStateStack.top() == hStates )
        return true;

    // Retrieve the final commited state object.
    cgRasterizerStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultRasterizerState;
    cgDX9RasterizerState * pState = (cgDX9RasterizerState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified rasterizer state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    HRESULT hRet;
    if ( FAILED( hRet = pBlock->Apply() ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified rasterizer state to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        pBlock->Release();
        return false;
    
    } // End if failed

    // Clean up
    pBlock->Release();

    // Retrieve new 'frontCounterClockwise' state. Since the depth stencil state 
    // relies on knowing the current value of this state then we must re-bind the 
    // depth stencil state should this value ever change.
    bool bFrontCounterClockwise = pState->getValues().frontCounterClockwise;
    if ( bFrontCounterClockwise != mFrontCounterClockwise )
        restoreDepthStencilState();
    mFrontCounterClockwise = bFrontCounterClockwise;

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setRasterizerState( hStates );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreRasterizerState () (Protected, Virtual)
/// <summary>
/// Re-bind the current rasterizer state object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreRasterizerState( )
{
    cgRasterizerStateHandle & hCurrentState = mRasterizerStateStack.top();
    cgDX9RasterizerState * pState = (cgDX9RasterizerState*)hCurrentState.getResource(true);
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        pBlock->Apply();
        pBlock->Release();
    
    } // End if valid

    // Retrieve new 'frontCounterClockwise' state. Since the depth stencil state 
    // relies on knowing the current value of this state then we must re-bind the 
    // depth stencil state should this value ever change.
    bool bFrontCounterClockwise = pState->getValues().frontCounterClockwise;
    if ( bFrontCounterClockwise != mFrontCounterClockwise )
        restoreDepthStencilState();
    mFrontCounterClockwise = bFrontCounterClockwise;
}

//-----------------------------------------------------------------------------
//  Name : setBlendState ()
/// <summary>
/// Apply the specified blend state to the device. Supplying an invalid 
/// (NULL) handle will result in the default blend states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setBlendState( const cgBlendStateHandle & hStates )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mBlendStateStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mBlendStateStack.top() == hStates )
        return true;

    // Retrieve the final commited state object.
    cgBlendStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultBlendState;
    cgDX9BlendState * pState = (cgDX9BlendState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified blend state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    HRESULT hRet;
    if ( FAILED( hRet = pBlock->Apply() ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified blend state to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        pBlock->Release();
        return false;
    
    } // End if failed

    // Clean up
    pBlock->Release();

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setBlendState( hStates );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreBlendState () (Protected, Virtual)
/// <summary>
/// Re-bind the current blend state object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreBlendState( )
{
    cgBlendStateHandle & hCurrentState = mBlendStateStack.top();
    cgDX9BlendState * pState = (cgDX9BlendState*)hCurrentState.getResource(true);
    LPDIRECT3DSTATEBLOCK9 pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        pBlock->Apply();
        pBlock->Release();
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : setViewport() (Virtual)
/// <summary>
/// Set the specified viewport details to the device. Specify a value of
/// CG_NULL to automatically revert to full size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setViewport( const cgViewport * pViewport )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mViewportStack.size() > 0 );

    // Lock the camera constant buffer ready for population
    cgConstantBuffer * pCameraBuffer = mCameraConstants.getResource( true );
    cgAssert( pCameraBuffer != CG_NULL );
    _cbCamera * pCameraData = CG_NULL;
    if ( !(pCameraData = (_cbCamera*)pCameraBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock camera constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Call base class implementation to select.
    cgViewport OldViewport = mViewportStack.top();
    if ( !cgRenderDriver::setViewport( pViewport ) )
    {
        pCameraBuffer->unlock();
        return false;
    
    } // End if failed

    // Pass through to D3D device
    HRESULT hRet;
    const cgViewport & NewViewport = mViewportStack.top();
    if ( FAILED( hRet = mD3DDevice->SetViewport( (const D3DVIEWPORT9*)&NewViewport ) ) )
    {
        // Rollback and fail
        pCameraBuffer->unlock();
        cgRenderDriver::setViewport( &OldViewport );
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind viewport to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Update buffer.
    cgFloat fInvWidth  = (NewViewport.width != 0) ? 1.0f / (cgFloat)NewViewport.width : 0;
    cgFloat fInvHeight = (NewViewport.height != 0) ? 1.0f / (cgFloat)NewViewport.height : 0;
    pCameraData->viewportSize.x = (cgFloat)NewViewport.width;
    pCameraData->viewportSize.y = (cgFloat)NewViewport.height;
    pCameraData->viewportSize.z = fInvWidth;
    pCameraData->viewportSize.w = fInvHeight;
    pCameraData->viewportOffset = cgVector2( (cgFloat)NewViewport.x, (cgFloat)NewViewport.y );
        
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVertexFormat () (Virtual)
/// <summary>
/// Set the vertex format that should be assumed by any following drawing 
/// routine.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setVertexFormat( cgVertexFormat * pFormat )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 );

    // Filter duplicates.
    cgVertexFormat * pOldFormat = mVertexFormatStack.top();
    if ( mStateFilteringEnabled && pOldFormat == pFormat )
        return true;

    // No-op?
    if ( !pFormat )
        return true;

    // Retrieve the D3D declarator for this format if it exists.
    DeclarationMap::iterator itItem = mVertexDeclarators.find( (void*)pFormat );

    // Not yet created?
    HRESULT hRet;
    LPDIRECT3DVERTEXDECLARATION9 pDeclarator = CG_NULL;
    if ( itItem == mVertexDeclarators.end() )
    {
        // Create the vertex declarator if not already created
        if ( FAILED( hRet = mD3DDevice->CreateVertexDeclaration( pFormat->getDeclarator(), &pDeclarator ) ) )
        {
            // Rollback and fail.
            cgRenderDriver::setVertexFormat( pOldFormat );
            cgAppLog::write( cgAppLog::Error, _T("Failed to create vertex format for first time use on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
            return false;
        
        } // End if failed

        // Store for later use.
        mVertexDeclarators[ (void*)pFormat ] = pDeclarator;
        
    } // End if no D3D declaration yet
    else
    {
        // Retrieve declarator
        pDeclarator = itItem->second;

    } // End if exists

    // Set the vertex declarator
    if ( FAILED( hRet = mD3DDevice->SetVertexDeclaration( pDeclarator ) ) )
    {
        // Rollback and fail.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind vertex format to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // If the old vertex format had a different stride to the new format, we need to
    // rebind vertex buffers.
    /*cgToDo( "Carbon General", "Vertex format system needs to be overhauled to maintain a stride per stream?" )
    if ( !pOldFormat || (pOldFormat->getStride() != pFormat->getStride()) )
    {
        for ( cgUInt32 i = 0; i < cgRenderDriver::MaxStreamSlots; ++i )
        {
            IDirect3DVertexBuffer9 * pBuffer = CG_NULL;
            UINT o, s;
            mD3DDevice->GetStreamSource( i, &pBuffer, &o, &s );
            if ( pBuffer )
            {
                mD3DDevice->setStreamSource( i, pBuffer, 0, pFormat->getStride() );
                pBuffer->Release();
            }

        } // Next vertex stream

    } // End if stride changed*/

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setVertexFormat( pFormat );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreVertexFormat () (Protected, Virtual)
/// <summary>
/// Re-bind the current vertex format object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreVertexFormat( )
{
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    if ( pFormat )
    {
        // Retrieve the D3D declarator for this format if it exists and set it.
        DeclarationMap::iterator itItem = mVertexDeclarators.find( (void*)pFormat );
        if ( itItem != mVertexDeclarators.end() )
            mD3DDevice->SetVertexDeclaration( itItem->second );

    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : setMaterialTerms()
/// <summary>
/// Set the specified material terms to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setMaterialTerms( const cgMaterialTerms & Terms )
{
    // ToDo: 9999 - Merge (or at least align) 'cgMaterialTerms' and '_cbMaterial' 
    // structure concept so that we don't have to copy the variables one at a time
    // below -- memcpy instead.

    // Update 'Material' constant buffer.
    cgConstantBuffer * pMaterialBuffer = mMaterialConstants.getResource( true );
    cgAssert( pMaterialBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbMaterial * pMaterialData = CG_NULL;
    if ( !(pMaterialData = (_cbMaterial*)pMaterialBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock material data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

	// Copy reflectance into the buffer
	pMaterialData->diffuse          = Terms.diffuse;
	pMaterialData->ambient          = Terms.ambient;
	pMaterialData->specular         = Terms.specular;

    // Linearize and scale emissive input if HDR lighting is enabled.
    if ( getSystemState( cgSystemState::HDRLighting ) > 0 )
    {
        pMaterialData->emissive.r       = powf( Terms.emissive.r, 2.2f ) * Terms.emissiveHDRScale;
        pMaterialData->emissive.g       = powf( Terms.emissive.g, 2.2f ) * Terms.emissiveHDRScale;
        pMaterialData->emissive.b       = powf( Terms.emissive.b, 2.2f ) * Terms.emissiveHDRScale;
        pMaterialData->emissive.a       = Terms.emissiveHDRScale;
		pMaterialData->emissiveTint     = cgColorValue( Terms.emissiveHDRScale, Terms.emissiveHDRScale, Terms.emissiveHDRScale, 1 );
    }
    else
    {
        pMaterialData->emissive         = Terms.emissive;
        pMaterialData->emissiveTint     = cgColorValue( 1, 1, 1, 1 );
    }

    // ToDo: 6767 - Convert SG Power to gloss. Can remove the following conversion afterwards. This is for backwards compatibility.
	if ( Terms.gloss > 1.0f )
		pMaterialData->gloss            = (log( Terms.gloss ) / log( 2.0f )) / 13.0f; //<- Matches Config.shh MAX_GLOSS_LEVELS
	else
		pMaterialData->gloss            = Terms.gloss;

	// ToDo: 9999 - pMaterialData->alphaTestValue -- Currently unsupported, set to default.
    pMaterialData->alphaTestValue       = 0.5f;
    pMaterialData->reflectionIntensity  = Terms.reflectionIntensity;
    pMaterialData->reflectionBumpiness  = Terms.reflectionBumpiness;
    pMaterialData->reflectionMipLevel   = Terms.reflectionMipLevel;
    pMaterialData->fresnelExponent      = Terms.fresnelExponent;
    pMaterialData->fresnelDiffuse       = Terms.fresnelDiffuse;
    pMaterialData->fresnelSpecular      = Terms.fresnelSpecular;
    pMaterialData->fresnelReflection    = Terms.fresnelReflection;
    pMaterialData->fresnelTransparent   = Terms.fresnelOpacity;
    pMaterialData->metalnessAmount      = Terms.metalnessAmount;
    pMaterialData->metalnessSpecular    = Terms.metalnessSpecular;
    pMaterialData->metalnessDiffuse     = Terms.metalnessDiffuse;
    pMaterialData->opacityMapStrength.x = Terms.diffuseOpacityMapStrength;
    pMaterialData->opacityMapStrength.y = Terms.specularOpacityMapStrength;
    // ToDo: 9999 - pMaterialData->parallaxHeightScale
    pMaterialData->parallaxHeightScale  = 1.0f;
    // ToDo: 9999 - pMaterialData->parallaxHeightBias
    pMaterialData->parallaxHeightBias   = 0.0f;
    pMaterialData->rimExponent          = Terms.rimExponent;
    pMaterialData->rimIntensity         = Terms.rimExponent;
    // ToDo: 9999 - pMaterialData->ILRStrength
    pMaterialData->ILRStrength                  = 1.0f;
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pMaterialBuffer->unlock();

	// Set manual sRGB decoding flag (Only true if not using a sampling state.)
	bool bDecodeSRGB = getSystemState( cgSystemState::HDRLighting ) > 0; 
	setSystemState( cgSystemState::DecodeSRGB, bDecodeSRGB ? 1 : 0 ); 

    // ToDo: 6767 - Add proper support for new material flags. (Can create local bools to cache conditional results if desired.)
    // ToDo: 6767 - These system states should be set by the material, not the driver itself.
	bool bMetal = Terms.metalnessAmount > 0;
	setSystemState( cgSystemState::Metal, bMetal );

	bool bUseSurfaceFresnel = (Terms.fresnelDiffuse + Terms.fresnelSpecular + Terms.fresnelReflection + Terms.fresnelOpacity) > 0.0f;
	setSystemState( cgSystemState::SurfaceFresnel, bUseSurfaceFresnel );

	bool bCorrectNormals = true;
	setSystemState( cgSystemState::CorrectNormals, bCorrectNormals );

	// ToDo: 6767 - Make sure this flags gets set using a more robust approach
	bool bTranslucent = ( pMaterialData->diffuse.a < 1.0f || pMaterialData->specular.a < 1.0f );
	setSystemState( cgSystemState::Translucent, bTranslucent );

	bool bTransmissive = false;
	setSystemState( cgSystemState::Transmissive, bTransmissive );
	
	bool bEmissive = (Terms.emissive.r + Terms.emissive.g + Terms.emissive.b) > 0.0f;
	setSystemState( cgSystemState::Emissive, bEmissive );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setIndices () (Virtual)
/// <summary>
/// Bind the specified index buffer to the device. This index buffer will be
/// utilized in any subsequent calls to 'drawIndexedPrimitive()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setIndices( const cgIndexBufferHandle & hIndices )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mIndicesStack.size() > 0 );

    // Filter if this is a duplicate of the current index buffer.
    if ( mStateFilteringEnabled && mIndicesStack.top() == hIndices )
        return true;

    // Retrieve the actual underlying D3D resource (or NULL).
    cgIndexBufferHandle hNewIndices = hIndices;
    cgDX9IndexBuffer * pBuffer = (cgDX9IndexBuffer*)hNewIndices.getResource(true);
    IDirect3DIndexBuffer9 * pD3DIndexBuffer = (pBuffer) ? pBuffer->GetD3DBuffer() : CG_NULL;

    // Pass through to device
    HRESULT hRet = mD3DDevice->SetIndices( pD3DIndexBuffer );
    
    // Release our reference
    if ( pD3DIndexBuffer )
        pD3DIndexBuffer->Release();

    // Failed to bind?
    if ( FAILED( hRet ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified index buffer data to the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setIndices( hIndices );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreIndices () (Protected, Virtual)
/// <summary>
/// Re-bind the current index buffer object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreIndices( )
{
    cgDX9IndexBuffer * pBuffer = (cgDX9IndexBuffer*)mIndicesStack.top().getResource(true);
    IDirect3DIndexBuffer9 * pD3DIndexBuffer = (pBuffer) ? pBuffer->GetD3DBuffer() : CG_NULL;
    if ( pD3DIndexBuffer )
    {
        mD3DDevice->SetIndices( pD3DIndexBuffer );
        pD3DIndexBuffer->Release();
    
    } // End if valid
    else
    {
        mD3DDevice->SetIndices( CG_NULL );

    } // End if invalid
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource () (Virtual)
/// <summary>
/// Set the specified vertex buffer to the device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setStreamSource( cgUInt32 nStreamIndex, const cgVertexBufferHandle & hStream )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );
    cgAssert( nStreamIndex < MaxStreamSlots );
    cgAssert( mVertexStreamStack[nStreamIndex].size() > 0 );

    // Filter if this is a duplicate of the current stream.
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    if ( mStateFilteringEnabled && 
        mVertexStreamStack[nStreamIndex].top().handle == hStream &&
        mVertexStreamStack[nStreamIndex].top().stride == pFormat->getStride() )
        return true;

    // Retrieve the actual underlying D3D resource (or NULL).
    cgVertexBufferHandle hNewStream = hStream;
    cgDX9VertexBuffer * pBuffer = (cgDX9VertexBuffer*)hNewStream.getResource(true);
    IDirect3DVertexBuffer9 * pD3DVertexBuffer = (pBuffer) ? pBuffer->getD3DBuffer() : CG_NULL;

    // Pass through to device
    HRESULT hRet = mD3DDevice->SetStreamSource( nStreamIndex, pD3DVertexBuffer, 0, pFormat->getStride() );
    
    // Release our reference
    if ( pD3DVertexBuffer )
        pD3DVertexBuffer->Release();

    // Failed to bind?
    if ( FAILED( hRet ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified vertex buffer data to stream %i of the selected device. The error reported was: (0x%x) %s - %s\n"), nStreamIndex, hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setStreamSource( nStreamIndex, hStream );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreStreamSource () (Protected, Virtual)
/// <summary>
/// Re-bind the current stream source object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreStreamSource( cgUInt32 nStreamIndex )
{
    VertexStreamData & Data = mVertexStreamStack[nStreamIndex].top();
    cgDX9VertexBuffer * pBuffer = (cgDX9VertexBuffer*)Data.handle.getResource(true);
    IDirect3DVertexBuffer9 * pD3DVertexBuffer = (pBuffer) ? pBuffer->getD3DBuffer() : CG_NULL;
    if ( pD3DVertexBuffer )
    {
        mD3DDevice->SetStreamSource( nStreamIndex, pD3DVertexBuffer, 0, Data.stride );
        pD3DVertexBuffer->Release();
    
    } // End if valid
    else
    {
        mD3DDevice->SetStreamSource( nStreamIndex, CG_NULL, 0, 0 );

    } // End if invalid
}

//-----------------------------------------------------------------------------
//  Name : setTexture ()
/// <summary>
/// Apply the specified texture to the indicated texture slot index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setTexture( cgUInt32 nTextureIndex, const cgTextureHandle & hTexture )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( nTextureIndex < MaxTextureSlots );
    cgAssert( mTextureStack[nTextureIndex].size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mTextureStack[nTextureIndex].top() == hTexture )
        return true;

    // Retrieve the actual underlying D3D resource (or NULL).
    cgTextureHandle hNewTexture = hTexture;
    cgTexture * pTexture = hNewTexture.getResource(true);
    IDirect3DBaseTexture9 * pD3DTexture = CG_NULL;
    if ( pTexture )
    {
        if ( pTexture->getResourceType() == cgResourceType::RenderTarget )
            pD3DTexture = ((cgDX9RenderTarget*)pTexture)->getD3DTexture();
        else if ( pTexture->getResourceType() == cgResourceType::Texture )
            pD3DTexture = ((cgDX9Texture<cgTexture>*)pTexture)->getD3DTexture();
        else if ( pTexture->getResourceType() == cgResourceType::DepthStencilTarget )
            pD3DTexture = ((cgDX9DepthStencilTarget*)pTexture)->getD3DTexture();

    } // End if valid
    
    // Pass through to device
    HRESULT hRet = mD3DDevice->SetTexture( nTextureIndex, pD3DTexture );
    
    // Release our reference
    if ( pD3DTexture )
        pD3DTexture->Release();

    // Failed to bind?
    if ( FAILED( hRet ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified texture data to stage %i of the selected device. The error reported was: (0x%x) %s - %s\n"), nTextureIndex, hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setTexture( nTextureIndex, hTexture );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreTexture () (Protected, Virtual)
/// <summary>
/// Re-bind the current texture object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreTexture( cgUInt32 nTextureIndex )
{
    cgTexture * pTexture = (cgTexture*)mTextureStack[nTextureIndex].top().getResource(true);
    IDirect3DBaseTexture9 * pD3DTexture = CG_NULL;
    if ( pTexture )
    {
        if ( pTexture->getResourceType() == cgResourceType::RenderTarget )
            pD3DTexture = ((cgDX9RenderTarget*)pTexture)->getD3DTexture();
        else if ( pTexture->getResourceType() == cgResourceType::Texture )
            pD3DTexture = ((cgDX9Texture<cgTexture>*)pTexture)->getD3DTexture();
        else if ( pTexture->getResourceType() == cgResourceType::DepthStencilTarget )
            pD3DTexture = ((cgDX9DepthStencilTarget*)pTexture)->getD3DTexture();

    } // End if valid

    if ( pD3DTexture )
    {
        mD3DDevice->SetTexture( nTextureIndex, pD3DTexture );
        pD3DTexture->Release();
    
    } // End if valid
    else
    {
        mD3DDevice->SetTexture( nTextureIndex, CG_NULL );

    } // End if invalid
}

//-----------------------------------------------------------------------------
//  Name : drawIndexedPrimitive () (Virtual)
/// <summary>
/// Render the primitives specified using the currently set stream
/// sources, and index buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::drawIndexedPrimitive( cgPrimitiveType::Base Type, cgInt32 BaseVertexIndex, cgUInt32 MinIndex, cgUInt32 NumVertices, cgUInt32 StartIndex, cgUInt32 PrimitiveCount )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX9ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX9ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( mD3DDevice ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Pass through to device
    mD3DDevice->DrawIndexedPrimitive( (D3DPRIMITIVETYPE)Type, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount );
    mPrimitivesDrawn += PrimitiveCount;

    // Record triangle drawing.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->endProcess();
#   endif

}

//-----------------------------------------------------------------------------
//  Name : drawPrimitive () (Virtual)
/// <summary>
/// Render the primitives specified using the currently set stream
/// sources.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::drawPrimitive( cgPrimitiveType::Base Type, cgUInt32 StartVertex, cgUInt32 PrimitiveCount )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("DrawPrimitive") );
#   endif

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX9ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX9ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( mD3DDevice ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Pass through to device
    mD3DDevice->DrawPrimitive( (D3DPRIMITIVETYPE)Type, StartVertex, PrimitiveCount );
    mPrimitivesDrawn = PrimitiveCount;

    // Record triangle drawing.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->endProcess();
#   endif

}

//-----------------------------------------------------------------------------
//  Name : drawPrimitiveUP () (Virtual)
/// <summary>
/// Render the primitives specified using the user provided data specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::drawPrimitiveUP( cgPrimitiveType::Base Type, cgUInt32 PrimitiveCount, const void * pVertexStreamZeroData )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX9ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX9ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( mD3DDevice ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Pass through to device
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    mD3DDevice->DrawPrimitiveUP( (D3DPRIMITIVETYPE)Type, PrimitiveCount, pVertexStreamZeroData, pFormat->getStride() );
    mPrimitivesDrawn = PrimitiveCount;

    // 'drawPrimitiveUP' clears out vertex stream [0] on the device. Ensure that
    // state filtering is deactivated for this slot.
    mVertexStreamStack[0].top() = VertexStreamData();
     
    // Record primitive render.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->endProcess();
#   endif
    
}

//-----------------------------------------------------------------------------
//  Name : DrawIndexedPrimitiveUp () (Virtual)
/// <summary>
/// Render the primitives specified using the user provided data specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::drawIndexedPrimitiveUP( cgPrimitiveType::Base Type, cgUInt32 MinVertexIndex, cgUInt32 NumVertices, cgUInt32 PrimitiveCount, const void * pIndexData, cgBufferFormat::Base IndexDataFormat, const void * pVertexStreamZeroData )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX9ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX9ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( mD3DDevice ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Pass through to device
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    D3DFORMAT NativeFormat = (IndexDataFormat == cgBufferFormat::Index32) ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    mD3DDevice->DrawIndexedPrimitiveUP( (D3DPRIMITIVETYPE)Type, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, 
                                          NativeFormat, pVertexStreamZeroData, pFormat->getStride() );
    mPrimitivesDrawn = PrimitiveCount;
    

    // 'drawIndexedPrimitiveUP' clears out vertex stream [0] on the device
    // as well as the index buffer. Ensure that state filtering is deactivated 
    // for these.
    mVertexStreamStack[0].top() = VertexStreamData();
    mIndicesStack.top() = cgIndexBufferHandle::Null;
     
    // Record primitive render.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->endProcess();
#   endif
    
}

//-----------------------------------------------------------------------------
//  Name : setScissorRect () (Virtual)
/// <summary>
/// Select the screen space scissor rectangle clipping area outside of which
/// pixels will be clipped away. Specifying a NULL pointer argument will 
/// cause the entire frame buffer to be selected.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setScissorRect( const cgRect * pRect )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mScissorRectStack.size() > 0 );

    // Call base class implementation.
    cgRect rcOld = mScissorRectStack.top();
    if ( !cgRenderDriver::setScissorRect( pRect ) )
        return false;

    // Pass through to D3D device
    HRESULT hRet;
    if ( FAILED( hRet = mD3DDevice->SetScissorRect( (const RECT*)&mScissorRectStack.top() ) ) )
    {
        // Rollback and fail
        cgRenderDriver::setScissorRect( &rcOld );
        cgAppLog::write( cgAppLog::Error, _T("Failed to select specified scissor rectangle area on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onDeviceLost () (Protected Virtual)
/// <summary>
/// Notify all systems that the device has been lost.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::onDeviceLost()
{
    // Destroy unmanaged resources
    clearQueries();
    
    // Call base class implementation.
    cgRenderDriver::onDeviceLost();
}

//-----------------------------------------------------------------------------
//  Name : clear () (Virtual)
/// <summary>
/// Clear the frame, depth and stencil buffers.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::clear( cgUInt32 nFlags, cgUInt32 nColor, cgFloat fDepth, cgUInt8 nStencil )
{
    cgAssert( isInitialized() == true );
    HRESULT hRet = mD3DDevice->Clear( 0, CG_NULL, nFlags, nColor, fDepth, nStencil );
    return SUCCEEDED(hRet);
}

//-----------------------------------------------------------------------------
//  Name : clear () (Virtual)
/// <summary>
/// Clear the frame, depth and stencil buffers within the specified
/// rectangular areas.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::clear( cgUInt32 nRectCount, cgRect * pRectangles, cgUInt32 nFlags, cgUInt32 nColor, cgFloat fDepth, cgUInt8 nStencil )
{
    cgAssert( isInitialized() == true );
    HRESULT hRet = mD3DDevice->Clear( nRectCount, (D3DRECT*)pRectangles, nFlags, nColor, fDepth, nStencil );
    return SUCCEEDED(hRet);
}

//-----------------------------------------------------------------------------
//  Name : beginFrame () (Virtual)
/// <summary>
/// Begin rendering process for this frame
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::beginFrame( bool bClearTarget, cgUInt32 nTargetColor )
{
    static bool bSpewError = true;

    // Validate requirements
    cgAssert( isInitialized() == true );

    cgToDo( "Carbon General", "When device is reset (in all cases), potentially re-apply vertex shader, pixel shader, state blocks, vertex format, etc." )
    
    // Recover lost device if required
    if ( mLostDevice == true )
    {
        // Can we reset the device yet ?
        HRESULT hRet = mD3DDevice->TestCooperativeLevel();
        if ( hRet == D3DERR_DEVICENOTRESET )
        {
            // Log that the device is now to be reset.
            if ( bSpewError == true )
                cgAppLog::write( cgAppLog::Info, _T("Resetting render device after it was found to be in a lost state.\n") );
            bSpewError = false;

            // Clean up prior to reset
            onDeviceLost();

            // Reset the display
            if ( FAILED( hRet = mD3DInitialize->resetDisplay( mD3DDevice, mD3DSettings, CG_NULL, CG_NULL, false ) ) )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to reset display device. Waiting to try again. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                return false;
            
            } // End if failed
            else
            {
                // Restore post-reset
                onDeviceReset();

                // Device is no longer in a lost state
                mLostDevice = false;
                bSpewError    = true;
            
            } // End if success

        } // End if can reset
        else
        {
            // Cannot yet reset
            return false;

        } // End if cannot reset

    } // End if Device Lost

    // Begin frame profiling
    cgProfiler * pProfiler = cgProfiler::getInstance();
    pProfiler->beginFrame();

    // Set 'scene' parameters.
    cgConstantBuffer * pSceneBuffer = mSceneConstants.getResource( true );
    cgAssert( pSceneBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbScene * pSceneData = CG_NULL;
    if ( !(pSceneData = (_cbScene*)pSceneBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock scene constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Set atmospherics data (ToDo: 6767 - Set proper values, probably best done by the SCENE class.)
	pSceneData->fogColor     = cgColorValue( 0.12f, 0.12f, 0.12f, 0 );
	pSceneData->fogDensity   = 0.008f;
	pSceneData->fogRange     = cgVector4( 50.0f, 2500.0f, 1.0f / (2500.f - 200.f), -50.f / (2500.f - 200.f) );
	pSceneData->skyIntensity = 1.0f;

    // Set timing data
    pSceneData->currentTime = (cgFloat)cgTimer::getInstance()->getTime();
    pSceneData->elapsedTime = (cgFloat)cgTimer::getInstance()->getTimeElapsed();

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pSceneBuffer->unlock();

    // Reset the device viewport (for the clear)
    setViewport( CG_NULL );

    // Reset the scissor rectangle
    setScissorRect( CG_NULL );

    // Clear the frame buffer on request ready for drawing
    if ( bClearTarget == true )
	    mD3DDevice->Clear( 0, CG_NULL, D3DCLEAR_TARGET, nTargetColor, 1.0f, 0 );

    // Begin Scene Rendering
    mD3DDevice->BeginScene();

    // Apply system constant buffers.
    setConstantBufferAuto( mSceneConstants );
    setConstantBufferAuto( mCameraConstants );
    setConstantBufferAuto( mShadowConstants );
    setConstantBufferAuto( mMaterialConstants );
    setConstantBufferAuto( mObjectConstants );
    setConstantBufferAuto( mWorldConstants );
    if ( !mConfig.useVTFBlending )
        setConstantBufferAuto( mVertexBlendingConstants );

    // Call base class implementation
    return cgRenderDriver::beginFrame( bClearTarget, nTargetColor );
}

//-----------------------------------------------------------------------------
//  Name : beginTargetRender() (Virtual)
/// <summary>
/// Prepares the render target and depth stencil target to receive data. 
/// If a null handle is supplied to 'hDepthStencilTarget', then any active 
/// depth stencil buffer will be unset from the device. A cube face of -1 will 
/// cause the original render target's selected cube face to be retained.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::beginTargetRender( cgRenderTargetHandle hRenderTarget, cgInt32 nCubeFace, bool bAutoUseMultiSample, cgDepthStencilTargetHandle hDepthStencilTarget )
{
    // Debug validations
    cgAssert( isInitialized() == true );

    // Lock the camera constant buffer ready for target size constant update.
    // We do this first in order to simplify any failure rollback process.
    cgConstantBuffer * pCameraBuffer = mCameraConstants.getResource( true );
    cgAssert( pCameraBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbCamera * pCameraData = CG_NULL;
    if ( !(pCameraData = (_cbCamera*)pCameraBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock camera constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Remove and record any targets that may currently be bound to shader resource slots.
    for ( size_t i = 0; i < 16; ++i )
    {
        const cgTextureHandle & hTexture = mTextureStack[i].top();
        if ( hTexture.isValid() && (hTexture == hRenderTarget || hTexture == hDepthStencilTarget) )
        {
            // Clear the slot
            pushTexture( i, cgTextureHandle::Null );

            // Record the modified texture slot so that it can be restored when we're done.
            mTargetStack.top().boundTextures.push_back( i );

        } // End if already bound
    
    } // Next slot

    // Push the new target information onto the top of the stack.
    mTargetStack.push( TargetData() );
    TargetData & Data = mTargetStack.top();
    Data.renderTargets.push_back( hRenderTarget );
    Data.autoUseMultiSample = bAutoUseMultiSample;
    Data.cubeFace = nCubeFace;
    Data.depthStencil = hDepthStencilTarget;

    // Set the new render target
    cgDX9RenderTarget * pRenderTarget = (cgDX9RenderTarget*)hRenderTarget.getResource(true);
    if ( !pRenderTarget )
        mD3DDevice->SetRenderTarget( 0, CG_NULL );
    else
    {
        // Bind to device
        LPDIRECT3DSURFACE9 pSurface = pRenderTarget->getD3DTargetSurface( Data.autoUseMultiSample );
        mD3DDevice->SetRenderTarget( 0, pSurface );
        if ( pSurface ) pSurface->Release();

        // If user specified a cube face into which we would render, set it
        // here otherwise retain the original cube face.
        if ( Data.cubeFace >= 0 )
            pRenderTarget->setCurrentCubeFace( (cgTexture::CubeFace)Data.cubeFace );
        else
            Data.cubeFace = pRenderTarget->getCurrentCubeFace();

        // Render target data is no longer lost (if it was previously)
        // since it has been re-populated.
        pRenderTarget->setResourceLost(false);

        // Mark target as dirty for (possible) future msaa resolve 
        pRenderTarget->mMSAADirty = true;

    } // End if valid

    // Clear all other remaining render targets
    for ( cgUInt32 i = 1; i < 4; ++i )
        mD3DDevice->SetRenderTarget( i, CG_NULL );

    // Retrieve the depth stencil surface and set
    cgDX9DepthStencilTarget * pDepthTarget = (cgDX9DepthStencilTarget*)Data.depthStencil.getResource(true);
    if ( !pDepthTarget || !pDepthTarget->isLoaded() )
    {
        mD3DDevice->SetDepthStencilSurface( CG_NULL );

    } // End if invalid
    else
    {
        LPDIRECT3DSURFACE9 pSurface = pDepthTarget->getD3DTargetSurface();
        mD3DDevice->SetDepthStencilSurface( pSurface );
        if ( pSurface ) pSurface->Release();

        // Depth stencil data is no longer lost (if it was previously)
        // since it has been re-populated.
        pDepthTarget->setResourceLost(false);
    
    } // End if valid

    // Update the target size shader constant data.
    if ( pRenderTarget )
    {
        const cgImageInfo & TargetInfo = pRenderTarget->getInfo();
        cgFloat fInvWidth  = (TargetInfo.width != 0) ? 1.0f / (cgFloat)TargetInfo.width : 0;
        cgFloat fInvHeight = (TargetInfo.height != 0) ? 1.0f / (cgFloat)TargetInfo.height : 0;
        pCameraData->targetSize.x = (float)TargetInfo.width;
        pCameraData->targetSize.y = (float)TargetInfo.height;
        pCameraData->targetSize.z = fInvWidth;
        pCameraData->targetSize.w = fInvHeight;

        // Update internal target size status
        mTargetSize.width  = TargetInfo.width;
        mTargetSize.height = TargetInfo.height;
    
    } // End if valid target
    else
    {
        pCameraData->targetSize = cgVector4(0,0,0,0);

        // Update internal target size status
        mTargetSize.width  = 0;
        mTargetSize.height = 0;
    
    } // End if no target

    // Pre-compute final screen UV adjust scale and bias (based on current
    // viewport and render target size) to prevent the need to compute this
    // in the shader for each pixel / vertex.
    pCameraData->screenUVAdjustBias.x  = 0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.5f * pCameraData->targetSize.w;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Viewport is reset by D3D. Push this viewport onto the viewport stack to be 
    // restored when target rendering ends.
    cgViewport Viewport;
    mD3DDevice->GetViewport( (D3DVIEWPORT9*)&Viewport );
    pushViewport( &Viewport );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTargetRender() (Virtual)
/// <summary>
/// Begin rendering to one or more render targets.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::beginTargetRender( cgRenderTargetHandleArray aRenderTargets, bool bAutoUseMultiSample, cgDepthStencilTargetHandle hDepthStencilTarget )
{
    // Debug validations
    cgAssert( isInitialized() == true );
    
    // Lock the camera constant buffer ready for target size constant update.
    // We do this first in order to simplify any failure rollback process.
    cgConstantBuffer * pCameraBuffer = mCameraConstants.getResource( true );
    cgAssert( pCameraBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbCamera * pCameraData = CG_NULL;
    if ( !(pCameraData = (_cbCamera*)pCameraBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock camera constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Remove and record any targets that may currently be bound to shader resource slots.
    for ( size_t i = 0; i < 16; ++i )
    {
        const cgTextureHandle & hTexture = mTextureStack[i].top();
        if ( hTexture.isValid() )
        {
            if ( hTexture == hDepthStencilTarget )
            {
                // Clear the slot
                pushTexture( i, cgTextureHandle::Null );

                // Record the modified texture slot so that it can be restored when we're done.
                mTargetStack.top().boundTextures.push_back( i );
            
            } // End if matches depth target
            else
            {
                // Matches one of the specified render targets?
                for ( size_t j = 0; j < aRenderTargets.size(); ++j )
                {
                    if ( hTexture == aRenderTargets[j] )
                    {
                        // Clear the slot
                        pushTexture( i, cgTextureHandle::Null );

                        // Record the modified texture slot so that it can be restored when we're done.
                        mTargetStack.top().boundTextures.push_back( i );

                        // Slot cleared, move on to the next slot check.
                        break;
                    
                    } // End if matches depth target

                } // Next target

            } // End if !depth target

        } // End if already bound
    
    } // Next slot

    // Push the new target information onto the top of the stack.
    mTargetStack.push( TargetData() );
    TargetData & Data = mTargetStack.top();
    Data.renderTargets = aRenderTargets;
    Data.autoUseMultiSample = bAutoUseMultiSample;
    Data.cubeFace = -1;
    Data.depthStencil = hDepthStencilTarget;

    // Iterate through each target we need to set (up to a maximum of 4)
    cgDX9RenderTarget * pFirstTarget = CG_NULL;
    for ( size_t i = 0; i < min( 4, aRenderTargets.size() ); ++i )
    {
        // Set the new render target
        cgDX9RenderTarget * pRenderTarget = (cgDX9RenderTarget*)aRenderTargets[i].getResource( true );
        if ( !pRenderTarget )
            mD3DDevice->SetRenderTarget( (DWORD)i, CG_NULL );
        else
        {
            // Retrieve the target surface and set
            LPDIRECT3DSURFACE9 pSurface = pRenderTarget->getD3DTargetSurface( Data.autoUseMultiSample );
            mD3DDevice->SetRenderTarget( (DWORD)i, pSurface );
            if ( pSurface ) pSurface->Release();

            // Render target data is no longer lost (if it was previously)
            // since it has been re-populated.
            pRenderTarget->setResourceLost(false);

            // Record the reference to the first valid target encountered.
            // We'll need this later in order to extract some additional 
            // information such as the render target size, etc.
            if ( !pFirstTarget )
                pFirstTarget = pRenderTarget;

            // Mark target as dirty for (possible) future msaa resolve 
            pRenderTarget->mMSAADirty = true;

        } // End if valid target
	
    } // Next Render Target

    // Clear all other remaining render targets
    for ( size_t i = aRenderTargets.size(); i < 4; ++i )
        mD3DDevice->SetRenderTarget( (DWORD)i, CG_NULL );

    // Retrieve the depth stencil surface and set
    cgDX9DepthStencilTarget * pDepthTarget = (cgDX9DepthStencilTarget*)Data.depthStencil.getResource(true);
    if ( !pDepthTarget || !pDepthTarget->isLoaded() )
    {
        mD3DDevice->SetDepthStencilSurface( CG_NULL );

    } // End if invalid
    else
    {
        LPDIRECT3DSURFACE9 pSurface = pDepthTarget->getD3DTargetSurface();
        mD3DDevice->SetDepthStencilSurface( pSurface );
        if ( pSurface ) pSurface->Release();

        // Depth stencil data is no longer lost (if it was previously)
        // since it has been re-populated.
        pDepthTarget->setResourceLost(false);
    
    } // End if valid

    // Update the target size shader constant data.
    if ( pFirstTarget )
    {
        const cgImageInfo & TargetInfo = pFirstTarget->getInfo();
        cgFloat fInvWidth  = (TargetInfo.width != 0) ? 1.0f / (cgFloat)TargetInfo.width : 0;
        cgFloat fInvHeight = (TargetInfo.height != 0) ? 1.0f / (cgFloat)TargetInfo.height : 0;
        pCameraData->targetSize.x = (float)TargetInfo.width;
        pCameraData->targetSize.y = (float)TargetInfo.height;
        pCameraData->targetSize.z = fInvWidth;
        pCameraData->targetSize.w = fInvHeight;

        // Update internal target size status
        mTargetSize.width  = TargetInfo.width;
        mTargetSize.height = TargetInfo.height;
    
    } // End if valid target
    else
    {
        pCameraData->targetSize = cgVector4(0,0,0,0);

        // Update internal target size status
        mTargetSize.width  = 0;
        mTargetSize.height = 0;
    
    } // End if no target

    // Pre-compute final screen UV adjust scale and bias (based on current
    // viewport and render target size) to prevent the need to compute this
    // in the shader for each pixel / vertex.
    pCameraData->screenUVAdjustBias.x  = 0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.5f * pCameraData->targetSize.w;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Viewport is reset by D3D. Push this viewport onto the viewport stack to be 
    // restored when target rendering ends.
    cgViewport Viewport;
    mD3DDevice->GetViewport( (D3DVIEWPORT9*)&Viewport );
    pushViewport( &Viewport );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endTargetRender() (Virtual)
/// <summary>
/// Complete the rendering stage for the most recent render target.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::endTargetRender( )
{
    // Should always be at least 1 format entry in the stack.
    // This last element cannot be removed.
    cgAssert( mTargetStack.size() > 1 );

    // Lock the camera constant buffer ready for target size constant update.
    // We do this first in order to simplify any failure rollback process.
    cgConstantBuffer * pCameraBuffer = mCameraConstants.getResource( true );
    cgAssert( pCameraBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbCamera * pCameraData = CG_NULL;
    if ( !(pCameraData = (_cbCamera*)pCameraBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock camera constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Shrink the stack.
    mTargetStack.pop();

    // Re-apply the prior state.
    TargetData & Data = mTargetStack.top();
    cgDX9RenderTarget * pFirstTarget = CG_NULL;
    for ( size_t i = 0; i < min(4, Data.renderTargets.size()); ++i )
    {
        // Restore the render target
        cgDX9RenderTarget * pRenderTarget = (cgDX9RenderTarget*)Data.renderTargets[i].getResource( true );
        if ( !pRenderTarget )
            mD3DDevice->SetRenderTarget( (DWORD)i, CG_NULL );
        else
        {
            // Retrieve the target surface and set
            LPDIRECT3DSURFACE9 pSurface = pRenderTarget->getD3DTargetSurface( Data.autoUseMultiSample );
            mD3DDevice->SetRenderTarget( (DWORD)i, pSurface );
            if ( pSurface ) pSurface->Release(); 

            // Restore the specified cube map face as it existed 
            // at the point of application.
            if ( Data.cubeFace >= 0 )
                pRenderTarget->setCurrentCubeFace( (cgTexture::CubeFace)Data.cubeFace );
            
            // Record the reference to the first valid target encountered.
            // We'll need this later in order to extract some additional 
            // information such as the render target size, etc.
            if ( !pFirstTarget )
                pFirstTarget = pRenderTarget;

        } // End if valid target
    
    } // Next Target

    // Clear all other remaining render targets
    for ( size_t i = Data.renderTargets.size(); i < 4; ++i )
        mD3DDevice->SetRenderTarget( (DWORD)i, CG_NULL );

    // Restore the prior depth stencil surface
    cgDX9DepthStencilTarget * pDepthTarget = (cgDX9DepthStencilTarget*)Data.depthStencil.getResource(true);
    if ( !pDepthTarget || !pDepthTarget->isLoaded() )
    {
        mD3DDevice->SetDepthStencilSurface( CG_NULL );

    } // End if invalid
    else
    {
        LPDIRECT3DSURFACE9 pSurface = pDepthTarget->getD3DTargetSurface();
        mD3DDevice->SetDepthStencilSurface( pSurface );
        if ( pSurface ) pSurface->Release();
    
    } // End if valid

    // Update the target size shader constant data.
    if ( pFirstTarget )
    {
        const cgImageInfo & TargetInfo = pFirstTarget->getInfo();
        cgFloat fInvWidth  = (TargetInfo.width != 0) ? 1.0f / (cgFloat)TargetInfo.width : 0;
        cgFloat fInvHeight = (TargetInfo.height != 0) ? 1.0f / (cgFloat)TargetInfo.height : 0;
        pCameraData->targetSize.x = (float)TargetInfo.width;
        pCameraData->targetSize.y = (float)TargetInfo.height;
        pCameraData->targetSize.z = fInvWidth;
        pCameraData->targetSize.w = fInvHeight;

        // Update internal target size status
        mTargetSize.width  = TargetInfo.width;
        mTargetSize.height = TargetInfo.height;
    
    } // End if found a target
    else
    {
        pCameraData->targetSize = cgVector4(0,0,0,0);

        // Update internal target size status
        mTargetSize.width  = 0;
        mTargetSize.height = 0;
    
    } // End if no target

    // Pre-compute final screen UV adjust scale and bias (based on current
    // viewport and render target size) to prevent the need to compute this
    // in the shader for each pixel / vertex.
    pCameraData->screenUVAdjustBias.x  = 0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.5f * pCameraData->targetSize.w;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Restore prior viewport.
    popViewport( );

    // Restore any prior shader resource slots that may have been unbound.
    for ( size_t i = 0; i < Data.boundTextures.size(); ++i )
        popTexture( Data.boundTextures[i] );
    Data.boundTextures.clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endFrame () (Virtual)
/// <summary>
/// End rendering process for this frame and present.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::endFrame( cgAppWindow * pWndOverride, bool bPresent )
{
    // If we're in a lost state we should bail right here
    if ( mLostDevice )
        return;

    // Call base class implementation
    cgRenderDriver::endFrame( pWndOverride, bPresent ); 

    // ToDo: 9999 - Consider making this a debug assert with
    // perhaps optional silent recovery in release?
    // Ensure that we have completed all render target drawing
    if ( mTargetStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'beginTargetRender' / 'endTargetRender' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mTargetStack.size() > 2 )
            mTargetStack.pop();
        
        // Perform actual restoration of final element.
        endTargetRender();
        
    } // End if not empty

    // End Scene Rendering
    mD3DDevice->EndScene();

    // Should we present?
    if ( bPresent == true )
    {
        cgRect  rcSource;
        cgRect* pSourceRect = CG_NULL;

        // In windowed mode, present only the relevant part of the buffer.
        if ( isWindowed() == true )
        {
            if ( pWndOverride == CG_NULL )
            {
                // Copy the entire frame buffer contents
                cgSize Size = getScreenSize();
                rcSource = cgRect( 0, 0, Size.width, Size.height );
            
            } // End if no override
            else
            {
                cgSize Size = pWndOverride->getClientSize();
                rcSource = cgRect( 0, 0, Size.width, Size.height );

            } // End if override

            // Use this rectangle
            pSourceRect = &rcSource;

        } // End if camera available

        // Are we using an override window?
        HWND hWndOverride = CG_NULL;
        cgWinAppWindow * pWinAppWindow = dynamic_cast<cgWinAppWindow*>(pWndOverride);
        if ( pWinAppWindow != NULL )
            hWndOverride = pWinAppWindow->getWindowHandle();

        // Present the buffer
        mProfiler->beginProcess( _T("Presentation") );
        HRESULT hRet = mD3DDevice->Present( (RECT*)pSourceRect, CG_NULL, hWndOverride, CG_NULL );
        mProfiler->endProcess( );

        // Did we fail to present?
        if ( FAILED( hRet ) )
        {
            // Was the device lost?
            if ( hRet == D3DERR_DEVICELOST && mLostDevice == false )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Device has been put into a lost state, possibly as a result of a minimizing or Alt+Tab action.\n") );
                mLostDevice = true;
            
            } // End if device was lost
            else
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to present frame buffer. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
            
            } // End if failed to present
        
        } // End if failed

    } // End if present
    
    // End frame profiling
    cgProfiler * pProfiler = cgProfiler::getInstance();
    pProfiler->primitivesDrawn( mPrimitivesDrawn );
    pProfiler->endFrame();
}

//-----------------------------------------------------------------------------
//  Name : stretchRect () (Virtual)
/// <summary>
/// Copy data from one texture or render target to another.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::stretchRect( cgTextureHandle & hSource, const cgRect * pSrcRect, cgTextureHandle hDestination, const cgRect * pDstRect, cgFilterMethod::Base Filter )
{    
    IDirect3DSurface9 * pSrcSurface = CG_NULL, * pDstSurface = CG_NULL;

    // Valid source texture?
    cgTexture * pSrcTexture = hSource.getResource( true );
    if ( pSrcTexture )
    {
        // Update the texture (will automatically resolve any prior 
        // multi-sampled target data).
        pSrcTexture->update();

        // Retrieve the surface.
        if ( pSrcTexture->getResourceType() == cgResourceType::RenderTarget )
        {
            pSrcSurface = ((cgDX9RenderTarget*)pSrcTexture)->getD3DTargetSurface( /*bAutoMultiSampleResolve*/ );
        
        } // End if RenderTarget   
        else if ( pSrcTexture->getResourceType() == cgResourceType::Texture )
        {
            IDirect3DTexture9 * pTexture;
            IDirect3DBaseTexture9 * pBaseTexture = ((cgDX9Texture<cgTexture>*)pSrcTexture)->getD3DTexture();
            if ( FAILED( pBaseTexture->QueryInterface( IID_IDirect3DTexture9, (void**)&pTexture ) ) )
            {
                pBaseTexture->Release();
        
            } // End if not 2D texture
            else
            {
                // Retrieve the top level surface
                pTexture->GetSurfaceLevel( 0, &pSrcSurface );
        
                // Clean up
                pBaseTexture->Release();
                pTexture->Release();
        
            } // End if 2D texture

        } // End if texture

    } // End if valid

    // Validate success
    if ( !pSrcSurface )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve the source hardware surface when performing StretchRect operation.\n") );
        return false;
    
    } // End if !source

    // Retrieve destination surface.
    cgTexture * pDstTexture = hDestination.getResource( true );
    if ( pDstTexture )
    {
        // Retrieve the surface.
        if ( pDstTexture->getResourceType() == cgResourceType::RenderTarget )
        {
            pDstSurface = ((cgDX9RenderTarget*)pDstTexture)->getD3DTargetSurface( /*bAutoMultiSampleResolve*/ );
        
        } // End if RenderTarget   
        else if ( pDstTexture->getResourceType() == cgResourceType::Texture )
        {
            IDirect3DTexture9 * pTexture;
            IDirect3DBaseTexture9 * pBaseTexture = ((cgDX9Texture<cgTexture>*)pDstTexture)->getD3DTexture();
            if ( FAILED( pBaseTexture->QueryInterface( IID_IDirect3DTexture9, (void**)&pTexture ) ) )
            {
                pBaseTexture->Release();
        
            } // End if not 2D texture
            else
            {
                // Retrieve the top level surface
                pTexture->GetSurfaceLevel( 0, &pDstSurface );
        
                // Clean up
                pBaseTexture->Release();
                pTexture->Release();
        
            } // End if 2D texture

        } // End if texture

    } // End if valid texture

    // Validate success
    if ( !pDstSurface )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve the destination hardware surface when performing StretchRect operation.\n") );
        pSrcSurface->Release();
        return false;
    
    } // End if !source

    // We would like our version of stretch rect to automatically handle cases where
    // the destination rectangle falls outside the bounds of the destination surface.
    // First compute the full size of the destination surface.
    D3DSURFACE_DESC Desc;
    pDstSurface->GetDesc( &Desc );
    cgRect rcDstBounds = cgRect( 0, 0, Desc.Width, Desc.Height );
    bool bDstSystem = (Desc.Pool == D3DPOOL_SYSTEMMEM);
    
    // Next we need to compute actual rectangles for any that have not already been
    // supplied by the caller. Start with the source rectangle.
    cgRect rcSrc;
    if ( !pSrcRect )
    {   
        pSrcSurface->GetDesc( &Desc );
        rcSrc = cgRect( 0, 0, Desc.Width, Desc.Height );
    
    } // End if no explicit rectangle
    else
    {
        rcSrc = *pSrcRect;
    
    } // End if source provided

    // Same for destination.
    cgRect rcDst;
    if ( !pDstRect )
    {
        pDstSurface->GetDesc( &Desc );
        rcDst = cgRect( 0, 0, Desc.Width, Desc.Height );
    
    } // End if no explicit rectangle
    else
    {
        rcDst = *pDstRect;
    
    } // End if dest provided

    // Now attempt to clip the destination rectangle to the bounds of the destination surface.
    cgRect rcDstClipped;
    if ( !IntersectRect( (RECT*)&rcDstClipped, (RECT*)&rcDst, (RECT*)&rcDstBounds ) )
    {
        // No intersection occured, the destination is off the screen.
        pSrcSurface->Release();
        pDstSurface->Release();
        return false;
    
    } // End if off screen.

    // If the destination rectangle was modified, we need to adjust the source
    // rectangle appropriately.
    if ( memcmp( &rcDstClipped, &rcDst, sizeof(cgRect) ) != 0 )
    {
        cgFloat fUScale = (cgFloat)(rcSrc.right - rcSrc.left) / (cgFloat)(rcDst.right - rcDst.left);
        cgFloat fVScale = (cgFloat)(rcSrc.bottom - rcSrc.top) / (cgFloat)(rcDst.bottom - rcDst.top);

        // Offset src rectangle as required
        rcSrc.left   += (int)((cgFloat)(rcDstClipped.left   - rcDst.left )  * fUScale );
        rcSrc.top    += (int)((cgFloat)(rcDstClipped.top    - rcDst.top  )  * fVScale );
        rcSrc.right  += (int)((cgFloat)(rcDstClipped.right  - rcDst.right)  * fUScale );
        rcSrc.bottom += (int)((cgFloat)(rcDstClipped.bottom - rcDst.bottom) * fUScale );

    } // End if clipped.

    // Copy data over. If the destination exists in system memory then
    // perform a manual 'load'. Otherwise, use hardware stretch.
    bool bResult = false;
    if ( bDstSystem )
        bResult = SUCCEEDED( D3DXLoadSurfaceFromSurface( pDstSurface, CG_NULL, (RECT*)&rcDstClipped, pSrcSurface, CG_NULL, 
                                                         (RECT*)&rcSrc, ((Filter != cgFilterMethod::None) ? D3DX_DEFAULT : D3DX_FILTER_NONE), 0 ) );
    else
        bResult = SUCCEEDED(mD3DDevice->StretchRect( pSrcSurface, (RECT*)&rcSrc, pDstSurface, (RECT*)&rcDstClipped, (D3DTEXTUREFILTERTYPE)Filter ));

    cgToDo( "Carbon General", "Need to filter destination texture to update mips?" )

    // Clean up
    pSrcSurface->Release();
    pDstSurface->Release();

    // Success?
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : activateQuery() (Virtual)
/// <summary>
/// Activate a new query ready to record occlusion results.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgDX9RenderDriver::activateQuery( )
{
    // Debug validation
    cgAssert( isInitialized() == true );

    // Create a new device query.
    HRESULT hRet;
    LPDIRECT3DQUERY9 pQuery = CG_NULL;
    if ( FAILED( hRet = mD3DDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &pQuery ) ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Failed to create new occlusion query. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return -1;

    } // End if failed

    // Store query.
    mActiveQueries[ mNextQueryId++ ] = pQuery;
    return mNextQueryId - 1;
}

//-----------------------------------------------------------------------------
//  Name : deactivateQuery() (Virtual)
/// <summary>
/// Removes an active query 
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::deactivateQuery( cgInt32 nQueryId )
{
    QueryMap::iterator itQuery = mActiveQueries.find( nQueryId );
    if ( itQuery != mActiveQueries.end() )
    {
        // Release query
        if ( itQuery->second != CG_NULL )
            itQuery->second->Release();
        
        // Remove from active list
        mActiveQueries.erase( itQuery );

    } // End if query available
}

//-----------------------------------------------------------------------------
//  Name : validQuery() (Virtual)
/// <summary>
/// Determine if the specified query is valid.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::validQuery( cgInt32 nQueryId )
{
    return ( mActiveQueries.find( nQueryId ) != mActiveQueries.end() );
}

//-----------------------------------------------------------------------------
//  Name : clearQueries() (Virtual)
/// <summary>
/// Remove all active queries.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::clearQueries()
{
    QueryMap::iterator itQuery;
    for ( itQuery = mActiveQueries.begin(); itQuery != mActiveQueries.end(); ++itQuery )
    {
        if ( itQuery->second != CG_NULL )
            itQuery->second->Release();

    } // Next Declarator
    mActiveQueries.clear();
}

//-----------------------------------------------------------------------------
//  Name : checkQueryResults() (Virtual)
/// <summary>
/// Attempts to retrieve results of an active query.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::checkQueryResults( cgInt32 nQueryId, cgUInt32 & nPixelCount, bool bWaitForResults )
{
    // Debug validation
    cgAssert( isInitialized() == true );

    // Determine if query exists.
    LPDIRECT3DQUERY9 pQuery = CG_NULL;
    QueryMap::iterator itQuery = mActiveQueries.find( nQueryId );
    if ( itQuery == mActiveQueries.end() )
        return false;
    else
        pQuery = itQuery->second;

	// If results are available, return success
	if ( pQuery->GetData( &nPixelCount, sizeof(cgUInt32), 0 ) == S_OK )
		return true;

	// Should we flush?
	if ( bWaitForResults == true )
	{
		while( pQuery->GetData( &nPixelCount, sizeof(cgUInt32), D3DGETDATA_FLUSH ) == S_FALSE )
        {
            // Burn time!
            // ToDo: sleep(0)?
        
        } // Next Iteration
		return true;
	
    } // End if block

	// No results yet
	return false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9RenderDriver )
        return true;

    // Supported by base?
    return cgRenderDriver::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::processMessage( cgMessage * pMessage )
{
    // No messages currently processed in this implementation, pass to base.
    return cgRenderDriver::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : windowResized() (Virtual)
/// <summary>
/// Notify the render driver that the render window was resized.
/// Note : This should only be called if the render driver was initialized using
/// a custom HWND.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::windowResized( cgInt32 nWidth, cgInt32 nHeight )
{
    // If we're in fullscreen, or not yet initialized, this is a no-op
    if ( isInitialized() == false || isWindowed() == false )
        return;

    // Update Settings
    mD3DSettings.windowedSettings.displayMode.Width  = nWidth;
    mD3DSettings.windowedSettings.displayMode.Height = nHeight;

    // Clean up prior to reset
    onDeviceLost();

    // Reset the display
    HRESULT hRet = mD3DInitialize->resetDisplay( mD3DDevice, mD3DSettings, CG_NULL, CG_NULL, false );
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to reset display device after window resize. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        mLostDevice = true;

    } // End if failed to reset
    else
    {
        // Restore post-reset
        onDeviceReset();
    
    } // End if reset success

    // Call base class implementation.
    cgRenderDriver::windowResized( nWidth, nHeight );
}

//-----------------------------------------------------------------------------
//  Name : setUserClipPlanes() 
/// <summary>
/// Sets user defined clip planes (maximum equivalent to MaxClipPlaneSlots) with 
/// optional negation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setUserClipPlanes( const cgPlane pWorldSpacePlanes[], cgUInt32 nNumPlanes, bool bNegatePlanes /* = true */ )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Call base class implementation
    if ( !cgRenderDriver::setUserClipPlanes( pWorldSpacePlanes, nNumPlanes, bNegatePlanes ) )
        return false;

    // Planes need to be in clip space for use with shaders. Generate a duplicate set
    // of clip planes that have been transformed into this space.
	cgMatrix mtxITViewProj;
    const ClipPlaneData & Data = mClipPlaneStack.top();
    cgPlane ClipSpacePlanes[MaxClipPlaneSlots];
    memcpy( ClipSpacePlanes, Data.planes, Data.planeCount * sizeof(cgPlane) );
    if ( cgMatrix::inverse( mtxITViewProj, mViewProjectionMatrix ) )
    {
        cgMatrix::transpose( mtxITViewProj, mtxITViewProj );
    	cgPlane::transformArray( ClipSpacePlanes, sizeof(cgPlane), ClipSpacePlanes, sizeof(cgPlane), mtxITViewProj, Data.planeCount );

    } // End if transform

	// Set the planes
    cgUInt32 nEnabledStatus = 0;
	for ( cgUInt32 i = 0; i < Data.planeCount; ++i )
    {
        mD3DDevice->SetClipPlane( i, ClipSpacePlanes[i] );
        nEnabledStatus |= 1 << i;
    
    } // Next Plane

    // Enable the appropriate planes
    mD3DDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, nEnabledStatus );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVertexShader () (Virtual)
/// <summary>
/// Set the current vertex shader applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setVertexShader( const cgVertexShaderHandle & hShader )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexShaderStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mVertexShaderStack.top() == hShader )
        return true;

    // Set to the device
    cgVertexShaderHandle hNewShader = hShader;
    cgDX9VertexShader * pShader = (cgDX9VertexShader*)hNewShader.getResource(true);
    IDirect3DVertexShader9 * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    HRESULT hRet = mD3DDevice->SetVertexShader( pD3DShader );
    if ( pD3DShader )
        pD3DShader->Release();

    // Validate success.
    if ( FAILED( hRet ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to select specified vertex shader on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setVertexShader( hShader );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreVertexShader () (Protected, Virtual)
/// <summary>
/// Re-bind the current vertex shader object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restoreVertexShader( )
{
    cgDX9VertexShader * pShader = (cgDX9VertexShader*)mVertexShaderStack.top().getResource(true);
    IDirect3DVertexShader9 * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    mD3DDevice->SetVertexShader( pD3DShader );
    if ( pD3DShader )
        pD3DShader->Release();
}

//-----------------------------------------------------------------------------
//  Name : setPixelShader () (Virtual)
/// <summary>
/// Set the current pixel shader applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setPixelShader( const cgPixelShaderHandle & hShader )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mPixelShaderStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mPixelShaderStack.top() == hShader )
        return true;

    // Set to the device
    cgPixelShaderHandle hNewShader = hShader;
    cgDX9PixelShader * pShader = (cgDX9PixelShader*)hNewShader.getResource(true);
    IDirect3DPixelShader9 * pD3DShader = (pShader) ? pShader->GetD3DShader() : CG_NULL;
    HRESULT hRet = mD3DDevice->SetPixelShader( pD3DShader );
    if ( pD3DShader )
        pD3DShader->Release();

    // Validate success.
    if ( FAILED( hRet ) )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to select specified pixel shader on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed

    // Call base class implementation to set it to the internal stack.
    cgRenderDriver::setPixelShader( hShader );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restorePixelShader () (Protected, Virtual)
/// <summary>
/// Re-bind the current pixel shader object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderDriver::restorePixelShader( )
{
    cgDX9PixelShader * pShader = (cgDX9PixelShader*)mPixelShaderStack.top().getResource(true);
    IDirect3DPixelShader9 * pD3DShader = (pShader) ? pShader->GetD3DShader() : CG_NULL;
    mD3DDevice->SetPixelShader( pD3DShader );
    if ( pD3DShader )
        pD3DShader->Release();
}

//-----------------------------------------------------------------------------
//  Name : setVertexBlendData ()
/// <summary>
/// Set the matrix array and associated data required for performing 
/// vertex blending.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setVertexBlendData( const cgMatrix pMatrices[], const cgMatrix pITMatrices[], cgUInt32 MatrixCount, cgInt32 nMaxBlendIndex /* = -1 */ )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( MatrixCount ? pMatrices != CG_NULL : true );
    cgAssert( MatrixCount <= mCaps->getMaxBlendTransforms() );

    // Using vertex texture fetch or constants for blending?
    if ( !mConfig.useVTFBlending )
    {
        // Update constant buffers only if any matrices are supplied.
        if ( MatrixCount != 0 )
        {
            // Update the first of the two constant buffers designed to contain the
            // standard and inverse transpose matrices. Standard matrix buffer first.
            cgConstantBuffer * pMatrixBuffer = mVertexBlendingConstants.getResource( true );
            cgAssert( pMatrixBuffer != CG_NULL );

            // Lock the buffer ready for population
            _cbVertexBlending * pMatrixData;
            if ( !(pMatrixData = (_cbVertexBlending*)pMatrixBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to lock vertex blending constant buffer in preparation for device update.\n") );
                return false;

            } // End if failed

            // Update buffer with matrix data.
            cgVector3 * pRows = pMatrixData->transforms;
            for ( cgUInt32 i = 0; i < MatrixCount; ++i )
            {
                *pRows++ = (cgVector3&)pMatrices[i]._11;
                *pRows++ = (cgVector3&)pMatrices[i]._21;
                *pRows++ = (cgVector3&)pMatrices[i]._31;
                *pRows++ = (cgVector3&)pMatrices[i]._41;
            
            } // Next matrix

            // Unlock the buffer. If it is currently bound to the device
            // then the appropriate constants will be automatically updated
            // next time 'drawPrimitive*' is called.
            pMatrixBuffer->unlock();

            // If the constant buffer is not currently bound to the device, do so now.
            if ( !pMatrixBuffer->isBound() )
                setConstantBufferAuto( mVertexBlendingConstants );

        } // End if matrices supplied

    } // End if !VTF
    else
    {
        // Populate the VTF texture.
        if ( MatrixCount )
        {
            cgDX9Texture<cgTexture> * pTexture = (cgDX9Texture<cgTexture>*)mVertexBlendingTexture.getResource(true);
            if ( pTexture )
            {
                cgUInt32 nMatrixBlockSize = cgMathUtility::nextPowerOfTwo( mCaps->getMaxBlendTransforms() * 3 );

                // Pack matrix data into the texture.
                cgUInt32 nPitch;
                cgVector4 * pRows = (cgVector4*)pTexture->lock( nPitch, cgLockFlags::WriteOnly | cgLockFlags::Discard );
                for ( cgUInt32 i = 0; i < MatrixCount; ++i )
                {
                    const cgMatrix & m = pMatrices[i];
                    pRows[i*4]   = cgVector4( m._11, m._12, m._13, m._41 );
                    pRows[i*4+1] = cgVector4( m._21, m._22, m._23, m._42 );
                    pRows[i*4+2] = cgVector4( m._31, m._32, m._33, m._43 );
                    
                    const cgMatrix & mit  = (pITMatrices) ? pITMatrices[i] : pMatrices[i];
                    pRows[i*4+nMatrixBlockSize]   = cgVector4( mit._11, mit._12, mit._13, mit._41 );
                    pRows[i*4+nMatrixBlockSize+1] = cgVector4( mit._21, mit._22, mit._23, mit._42 );
                    pRows[i*4+nMatrixBlockSize+2] = cgVector4( mit._31, mit._32, mit._33, mit._43 );
                
                } // Next matrix
                pTexture->unlock(0);

                // Apply the texture to the device for use with vertex texture fetch
                LPDIRECT3DBASETEXTURE9 pD3DTexture = pTexture->getD3DTexture();
                mD3DDevice->SetTexture( D3DVERTEXTEXTURESAMPLER0, pD3DTexture );
                if ( pD3DTexture )
                    pD3DTexture->Release();

                // Setup VTF sampler states.
                mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
                mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
                mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
                mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
                mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

                cgToDo( "Carbon General", "Output specific warning on failure." );
            
            } // End if valid

        } // End if matrices supplied

    } // End if VTF

    // Set the system script side max blend index member variable.
    *mSystemExportVars.maximumBlendIndex = nMaxBlendIndex;
    *mSystemExportVars.useVTFBlending = mConfig.useVTFBlending;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVPLData ()
/// <summary>
/// Sets the depth and normal buffers needed for virtual point light injection.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::setVPLData( const cgTextureHandle & hDepth, const cgTextureHandle & hNormal )
{
	// Are we using VTF or R2VB? (ToDo 9999)
	bool bUseVTF = true;
	
	// Can only do VTF for the moment
	if ( !bUseVTF )
		return false;

	// Get the required texture resources
	const cgTexture * pDepth  = hDepth.getResourceSilent();
	const cgTexture * pNormal = hNormal.getResourceSilent();
	const cgTexture * pOutput = mVPLBuffer.getResource(false);

	// Set the active size of the vpl buffer
	mCurrentVPLSize.width = max( pDepth->getInfo().width,  pNormal->getInfo().width  );
	mCurrentVPLSize.height = max( pDepth->getInfo().height, pNormal->getInfo().height );

    // If the current buffer is too small to accomodate this data, grow it
	if ( pOutput->getInfo().width < (cgUInt32)mCurrentVPLSize.width || pOutput->getInfo().height < (cgUInt32)mCurrentVPLSize.height )
	{
		// Clear the old buffer and create a new one
		cgImageInfo TargetDesc = pOutput->getInfo();
        TargetDesc.width  = mCurrentVPLSize.width;
        TargetDesc.height = mCurrentVPLSize.height;
		mVPLBuffer.close(true);
		if ( !mResourceManager->createRenderTarget( &mVPLBuffer, TargetDesc, 0, _T("VPL_Texture_Shared"), cgDebugSource() ) )
		{
			cgAppLog::write( cgAppLog::Error, _T("Failed to create internal render driver's shared VPL render target on the selected device.\n") );
			return false;
		
        } // End if failed
	
    } // End if grow

	// Merge the textures into a single buffer for reading
    cgSurfaceShader * pShader = mDriverShaderHandle.getResource(true);
    if ( pShader->selectPixelShader( _T("mergeDepthAndNormalBuffers") ) && 
         beginTargetRender( mVPLBuffer, cgDepthStencilTargetHandle::Null ) )
    {
		// Set rendering states
		setDepthStencilState( mDisabledDepthState, 0 );
		setRasterizerState( cgRasterizerStateHandle::Null );
		setBlendState( cgBlendStateHandle::Null );

		// Set the input textures
		mSampler2D0->apply( hDepth  );
		mSampler2D1->apply( hNormal );

		// Set a viewport to match the dimensions of the inputs
		cgViewport Viewport;
		Viewport.x      = 0;
		Viewport.y      = 0;
		Viewport.width  = mCurrentVPLSize.width;
		Viewport.height = mCurrentVPLSize.height;
		Viewport.minimumZ   = 0.0f;
		Viewport.maximumZ   = 1.0f;
		setViewport( &Viewport );

        // Draw a fullscreen quad
        drawScreenQuad();

        // End rendering to target
        endTargetRender();

        // Set the merged buffer for reading
        if ( bUseVTF )
        {
            // Retrieve and set the underlying D3D texture resource for vertex texture fetching
            mD3DDevice->SetTexture( D3DVERTEXTEXTURESAMPLER0, ((cgDX9RenderTarget*)pOutput)->getD3DTexture() );
            mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
            mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
            mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
            mD3DDevice->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        }
        else
        {
            //ToDo 9999: R2VB
        }

        // Success!
        return true;

    } // End render

	// Failed
	return false;
}

//-----------------------------------------------------------------------------
//  Name : getScreenSize () (Virtual)
/// <summary>
/// Retrieve the current size of the 'screen' or frame buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgDX9RenderDriver::getScreenSize( ) const
{
    const cgDX9Settings::Settings * pSettings = mD3DSettings.getSettings();
    return cgSize( pSettings->displayMode.Width, pSettings->displayMode.Height );
}

//-----------------------------------------------------------------------------
//  Name : isWindowed () (Virtual)
/// <summary>
/// Determine if the device is currently functioning in a windowed mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::isWindowed( ) const
{
    return mD3DSettings.windowed;
}

//-----------------------------------------------------------------------------
//  Name : isVSyncEnabled () (Virtual)
/// <summary>
/// Determine if the driver is currently synchronizing with the monitor
/// vertical blank during frame presentation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderDriver::isVSyncEnabled( ) const
{
    return (mD3DSettings.getSettings()->presentInterval != D3DPRESENT_INTERVAL_IMMEDIATE);
}

//-----------------------------------------------------------------------------
//  Name : getD3D()
/// <summary>
/// Retrieve the internal D3D object specific to the Direct3D platform
/// implementation. This is not technically part of the public interface 
/// and should not be called by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3D9 * cgDX9RenderDriver::getD3D( ) const
{
    if ( mD3D != CG_NULL )
        mD3D->AddRef();
    return mD3D;
}

//-----------------------------------------------------------------------------
//  Name : getD3DDevice()
/// <summary>
/// Retrieve the internal D3D device object specific to the Direct3D
/// platform implementation. This is not technically part of the public
/// interface and should not be called by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DDevice9 * cgDX9RenderDriver::getD3DDevice( ) const
{
    if ( mD3DDevice != CG_NULL )
        mD3DDevice->AddRef();
    return mD3DDevice;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9RenderDriverInit Module Local Class
///////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Name : cgDX9RenderDriverInit () (Constructor)
// Desc : Constructor for this class.
//----------------------------------------------------------------------------
cgDX9RenderDriverInit::cgDX9RenderDriverInit( const cgRenderDriver::InitConfig & Config, bool bEnforceConfig ) : cgDX9Initialize( )
{
    // Store the configuration options
    mConfig   = Config;
    mEnforce = bEnforceConfig;
}

//----------------------------------------------------------------------------
// Name : validateDisplayMode () (Virtual)
// Desc : Allows us to validate and reject any adapter display modes.
//----------------------------------------------------------------------------
bool cgDX9RenderDriverInit::validateDisplayMode( const D3DDISPLAYMODE &Mode )
{
    // Test display mode
    if ( Mode.Width < 640 || Mode.Height < 480 || Mode.RefreshRate <= 30 )
        return false;

    // Disallow formats other than 32bpp
    cgUInt32 nBitsPerPixel = cgBufferFormatEnum::formatBitsPerPixel(cgDX9BufferFormatEnum::formatFromNative(Mode.Format));
    if ( nBitsPerPixel < 32 )
        return false;

    // Enforce fullscreen config options?
    if ( mEnforce && !mConfig.windowed )
    {
        // All details must match exactly
        if ( Mode.Width != mConfig.width || Mode.Height != mConfig.height ||
             (mConfig.refreshRate != 0 && Mode.RefreshRate != mConfig.refreshRate) )
             return false;
    
    } // End if enforce config

    // Supported
    return true;
}

//----------------------------------------------------------------------------
// Name : validateDevice () (Virtual)
// Desc : Allows us to validate and reject any devices that do not have
//        certain capabilities, or does not allow hardware rendering etc.
//----------------------------------------------------------------------------
bool cgDX9RenderDriverInit::validateDevice( cgDX9EnumAdapter * pAdapter, const D3DDEVTYPE &Type, const D3DCAPS9 &Caps )
{
    // Test Capabilities (All device types supported)
    // ToDo: Are we required to support 3 render targets in final implementation?
    if ( Caps.NumSimultaneousRTs < 3 )
        return false;

    // Enforce adapter / device selection?
    if ( mEnforce && !mConfig.deviceName.empty() )
    {
        // Ignore if the device name does not match the adapter description
        STRING_CONVERT;
        cgString strDeviceName = cgString::trim(stringConvertA2CT(pAdapter->identifier.Description));
        if ( !(strDeviceName == mConfig.deviceName)  )
            return false;

    } // End if enforce

    // Supported
    return true;
}

//----------------------------------------------------------------------------
// Name : validateVertexProcessingType () (Virtual)
// Desc : Allows us to validate and reject any vertex processing types we
//        do not wish to allow access to.
//----------------------------------------------------------------------------
bool cgDX9RenderDriverInit::validateVertexProcessingType( const VERTEXPROCESSING_TYPE &Type )
{
    // Note: Uncomment relevant lines below to fall back to HARDWARE_VP or MIXED_VP.

    // Hardware, mixed or software VP
    if ( Type == PURE_HARDWARE_VP ) return false;
    
    // Test Type, we must only use either mixed or software to allow our fallback paths
    // to correctly function (methods such as per-susbet software skinning)
    //if ( Type == HARDWARE_VP ) return false;
    //else if ( Type == PURE_HARDWARE_VP ) return false;

    // Enforce vertex processing options?
    if ( mEnforce )
    {
        // Must support a hardware TnL mode?
        if ( (Type != SOFTWARE_VP) && mConfig.useHardwareTnL == false )
            return false;

    } // End if enforce config

    // Supported
    return true;
}

//----------------------------------------------------------------------------
// Name : validatePresentInterval () (Virtual)
// Desc : Allows us to validate and reject any presentation interval types
//        we do not wish to allow access to.
//----------------------------------------------------------------------------
bool cgDX9RenderDriverInit::validatePresentInterval( const cgUInt32& Interval )
{
    // Support only VSync ON or OFF in this application
    if ( Interval != D3DPRESENT_INTERVAL_ONE && Interval != D3DPRESENT_INTERVAL_IMMEDIATE )
        return false;

    // Are we enforcing our configuration options?
    if ( mEnforce )
    {
        // Presentation interval must match
        if ( mConfig.useVSync == true && Interval != D3DPRESENT_INTERVAL_ONE )
            return false;

    } // End if enforce config

    // Supported
    return true;
}

//-----------------------------------------------------------------------------
// Name : validateDepthStencilFormat () (Private)
// Desc : Allows us to validate and reject any depth stencil formats that we
//        do not wish to allow access to.
//-----------------------------------------------------------------------------
bool cgDX9RenderDriverInit::validateDepthStencilFormat( const D3DFORMAT& DepthStencilFormat )
{
    // Engine requires 24bit depth with 8bit stencil
    if ( DepthStencilFormat != D3DFMT_D24S8 )
        return false;
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT