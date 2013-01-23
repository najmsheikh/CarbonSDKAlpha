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
// File : cgDX11RenderDriver.cpp                                             //
//                                                                           //
// Desc : Rendering class wraps the properties, initialization and           //
//        management of our rendering device. This includes the enumeration, //
//        creation and destruction of our D3D device and associated          //
//        resources.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11RenderDriver Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <Rendering/Platform/cgDX11RenderingCapabilities.h>
#include <System/Platform/cgWinAppWindow.h>
#include <System/cgStringUtility.h>
#include <System/cgProfiler.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <Math/cgMathUtility.h>
#include <D3D11.h>
#include <dxerr.h>

// Platform specific resource types
#include <Resources/Platform/cgDX11BufferFormatEnum.h>
#include <Resources/Platform/cgDX11StateBlocks.h>
#include <Resources/Platform/cgDX11ConstantBuffer.h>
#include <Resources/Platform/cgDX11RenderTarget.h>
#include <Resources/Platform/cgDX11DepthStencilTarget.h>
#include <Resources/Platform/cgDX11IndexBuffer.h>
#include <Resources/Platform/cgDX11VertexBuffer.h>
#include <Resources/Platform/cgDX11HardwareShaders.h>

// ToDo: 9999 - Potential optimization is to move viewport and render area
// constants out of the camera buffer and into their own to avoid the need
// to reupload so many constants if / when it changes?

//-----------------------------------------------------------------------------
// Module Local Enumerations.
//-----------------------------------------------------------------------------
namespace DX11HardwareVendor
{
    enum Base
    {
        NVIDIA      = 0x10DE,
        AMD         = 0x1002,
        INTEL       = 0x8086
    };

}; // End Namespace : DX11HardwareVendor

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

///////////////////////////////////////////////////////////////////////////////
// cgDX11RenderDriver Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderDriver::cgDX11RenderDriver()
{
    // Initialize variables to sensible defaults
    mD3DDevice                = CG_NULL;
    mD3DDeviceContext         = CG_NULL;
    mD3DSwapChain             = CG_NULL;
    mD3DInitialize            = CG_NULL;
    mFrameBufferView          = CG_NULL;
    mNextQueryId              = 0;
    mNextIASignatureId        = 0;
    mCurrentLayoutSignature   = 0xFFFFFFFF;
    mSelectNewInputLayout     = true;

    // Initialize stacks with space for the default elements
    // NB: m_TargetStacks starts with an initial size of 0
    // unlike most other stacks. An implicit 'beginTargetRender()'
    // call will be made during driver initialization for this purpose.
    // (No stacks at this time).
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11RenderDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderDriver::~cgDX11RenderDriver()
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
void cgDX11RenderDriver::releaseOwnedResources()
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
    mVertexBlendingSampler.close( true );

    // Release internal vertex buffer caches.
    UserPointerVerticesMap::iterator itVertices;
    for ( itVertices = mUserPointerVertices.begin(); itVertices != mUserPointerVertices.end(); ++itVertices )
        itVertices->second.buffer.close(true);
    mUserPointerVertices.clear();
    
    // Release internal index buffer caches.
    UserPointerIndicesMap::iterator itIndices;
    for ( itIndices = mUserPointerIndices.begin(); itIndices != mUserPointerIndices.end(); ++itIndices )
        itIndices->second.buffer.close(true);
    mUserPointerIndices.clear();

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
void cgDX11RenderDriver::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release any active resources loaded via the resource manager.
    releaseOwnedResources();

    // Release any allocated memory
    delete mD3DInitialize;
    
    // Release any input layout objects
    InputSignatureLayoutMap::iterator itSignature;
    for ( itSignature = mInputSignatureFormats.begin(); itSignature != mInputSignatureFormats.end(); ++itSignature )
    {
        InputLayoutMap::iterator itLayout;
        InputLayoutMap & Layouts = itSignature->second;
        for ( itLayout = Layouts.begin(); itLayout != Layouts.end(); ++itLayout )
        {
            if ( itLayout->second != CG_NULL )
                itLayout->second->Release();

        } // Next Layout
    
    } // Next Signature
    mInputSignatureFormats.clear();
    mIAInputSignatureLUT.clear();

    // Release any active queries.
    clearQueries();

    // Release any DirectX objects.
    if ( mFrameBufferView )
        mFrameBufferView->Release();
    if ( mD3DSwapChain )
    {
        mSuppressResizeEvent = true;
        mD3DSwapChain->SetFullscreenState( FALSE, CG_NULL );
        mD3DSwapChain->Release();
        mSuppressResizeEvent = false;
    
    } // End if swap chain alive
    if ( mD3DDeviceContext )
        mD3DDeviceContext->Release();
    if ( mD3DDevice )
        mD3DDevice->Release();

    // Reset any variables
    mFrameBufferView          = CG_NULL;
    mD3DSwapChain             = CG_NULL;
    mD3DDeviceContext         = CG_NULL;
    mD3DDevice                = CG_NULL;
    mD3DInitialize            = CG_NULL;
    mNextQueryId              = 0;
    mNextIASignatureId        = 0;
    mCurrentLayoutSignature   = 0xFFFFFFFF;
    mSelectNewInputLayout     = true;

    // Clear containers
    while (mTargetStack.size() > 0)
        mTargetStack.pop();

    // Initialize stacks with space for the default elements
    // NB: m_TargetStacks starts with an initial size of 0
    // unlock most other stacks. An implcit 'beginTargetRender()'
    // call will be made during driver initialization for this purpose.
    // (No stacks at this time).

    // Call base if requested
    if ( bDisposeBase )
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
cgConfigResult::Base cgDX11RenderDriver::loadConfig( const cgString & strFileName )
{
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

    // 7777 - Get from enumeration object
	/*// Before we do any enumeration and choose a display mode, compute the default adapter's aspect ratio.
	// This is useful for handling widescreen displays in fullscreen mode.
	D3DDISPLAYMODE displayMode;
	if ( FAILED( m_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &displayMode ) ) )
	{
        cgAppLog::write( cgAppLog::Error, _T( "Could not get the adapter's original display mode during initialization.\n" ) );
        return cgConfigResult::Error;
	
    } // End if Failure
    m_fAdapterAspectRatio = (cgFloat)displayMode.Width / (cgFloat)displayMode.Height;*/
	
    // Release previous initialization object
    delete mD3DInitialize;

    // Create an init item for enumeration (the second parameter instructs the init
    // class as to whether it should strictly enforce the configuration options, or simply
    // let the system find a good match).
    mD3DInitialize = new cgDX11RenderDriverInit( mConfig, strFileName.empty() == false );
    
    // Enumerate the system graphics adapters    
    if ( FAILED(mD3DInitialize->enumerate( )) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Device enumeration failed. The application will now exit.\n" ) );
        return cgConfigResult::Error;

    } // End if Failure

    // Attempt to find a good default fullscreen set
    DXGI_MODE_DESC  MatchMode;
    MatchMode.Width             = mConfig.width;
    MatchMode.Height            = mConfig.height;
    MatchMode.Format            = DXGI_FORMAT_R8G8B8A8_UNORM;
    MatchMode.RefreshRate.Numerator = mConfig.refreshRate;
    MatchMode.RefreshRate.Denominator = (mConfig.refreshRate == 0) ? 0 : 1;
    MatchMode.Scaling           = DXGI_MODE_SCALING_UNSPECIFIED; // Stretched or centered.
    MatchMode.ScanlineOrdering  = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    bool bFoundMode             = mD3DInitialize->findBestFullScreenMode( mD3DSettings, &MatchMode, true, false );
    
    // Mismatched?
    if ( !mConfig.windowed && !bFoundMode )
        return cgConfigResult::Mismatch;

    // Attempt to find a good default windowed set
    bFoundMode = mD3DInitialize->findBestWindowedMode( mD3DSettings, true, false );
    
    // Mismatched?
    if ( mConfig.windowed && !bFoundMode )
        return cgConfigResult::Mismatch;

    // Enable triple buffering for fullscreen modes if requested.
    mD3DSettings.fullScreenSettings.tripleBuffering = mConfig.useTripleBuffering;

    // What mode are we using?
    mD3DSettings.windowed = mConfig.windowed;

    // Overwrite fullscreen options
    if ( !mConfig.windowed )
    {
        cgDX11Settings::Settings * pSettings = mD3DSettings.getSettings();
        mConfig.width       = pSettings->displayMode.Width;
        mConfig.height      = pSettings->displayMode.Height;
        cgUInt nDenominator  = max( 1, pSettings->displayMode.RefreshRate.Denominator );
        mConfig.refreshRate = (cgUInt32)(pSettings->displayMode.RefreshRate.Numerator / (cgDouble)nDenominator);
    
    } // End if not windowed

    // Retrieve selected adapter information and store it in the configuration
    STRING_CONVERT;
    cgDX11Settings::Settings * pSettings = mD3DSettings.getSettings();
    const cgDX11EnumAdapter  * pAdapter  = mD3DInitialize->getAdapter( pSettings->adapterOrdinal );
    mConfig.deviceName = cgString::trim(stringConvertW2CT(pAdapter->details.Description));

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
cgConfigResult::Base cgDX11RenderDriver::loadDefaultConfig( bool bWindowed /* = false  */ )
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
bool cgDX11RenderDriver::saveConfig( const cgString & strFileName )
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
bool cgDX11RenderDriver::initialize( cgResourceManager * pResources, cgAppWindow * pFocusWindow, cgAppWindow * pOutputWindow )
{
    HRESULT hRet;

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

    // If no output window is specified, use the device window.
    if ( !pOutputWindow )
        pOutputWindow = pFocusWindow;

    // Only 'cgWinAppWindow' type is supported.
    cgWinAppWindow * pWinFocusWindow  = dynamic_cast<cgWinAppWindow*>(pFocusWindow);
    cgWinAppWindow * pWinOutputWindow = dynamic_cast<cgWinAppWindow*>(pOutputWindow);
    if ( !pWinFocusWindow )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX11 render driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast

    // Get native window handles
    HWND hFocusWindow  = pWinFocusWindow->getWindowHandle();
    HWND hOutputWindow = pWinOutputWindow->getWindowHandle();

    // Create the direct 3d device etc.
    cgUInt32 nFlags = (cgGetEngineConfig().multiThreaded) ? 0 : D3D11_CREATE_DEVICE_SINGLETHREADED;
    if ( FAILED( mD3DInitialize->createDisplay( mD3DSettings, nFlags, hFocusWindow, hOutputWindow,
                                                CG_NULL, CG_NULL, 0, 0, 0, false, mConfig.usePerfHUD ) ))
    {
        cgAppLog::write( cgAppLog::Error, _T("Device creation failed. The application will now exit.\n") );
        return false;

    } // End if Failed

    // Retrieve created device
    mD3DDeviceContext = mD3DInitialize->getDirect3DDeviceContext( );
    mD3DDevice = mD3DInitialize->getDirect3DDevice( );
    mD3DSwapChain = mD3DInitialize->getDirect3DSwapChain( );

    // Create a render target view for the primary swap chain back buffer.
    ID3D11Texture2D * pBackBuffer = CG_NULL;
    if ( FAILED( hRet = mD3DSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed
    hRet = mD3DDevice->CreateRenderTargetView( pBackBuffer, CG_NULL, &mFrameBufferView );
    pBackBuffer->Release();
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create a render target view for primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed
    
    // Store the current window size in the settings for accountabilities sake
    if ( mConfig.windowed )
    {
        cgSize Size = pOutputWindow->getClientSize();
        mD3DSettings.windowedSettings.displayMode.Width  = Size.width;
        mD3DSettings.windowedSettings.displayMode.Height = Size.height;
    
    } // End if windowed

	// Perform an initial clear on the framebuffer targets
    cgColorValue ClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    mD3DDeviceContext->ClearRenderTargetView( mFrameBufferView, ClearColor );

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
bool cgDX11RenderDriver::initialize( cgResourceManager * pResources, const cgString & WindowTitle /* = _T("Render Output") */, cgInt32 IconResource /* = -1 */ )
{
    HRESULT hRet;

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
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("The DirectX11 render driver is only supported on the Windows(tm) platform.\n"));
        return false;

    } // End if invalid cast
    if ( !mFocusWindow->create( (mConfig.windowed == false), mConfig.width, mConfig.height, WindowTitle, IconResource ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Application window creation failed. The application will now exit.\n"));
        return false;

    } // End if failed

    // Create the direct 3d device etc.
    cgUInt32 nFlags = (cgGetEngineConfig().multiThreaded) ? 0 : D3D11_CREATE_DEVICE_SINGLETHREADED;
    if ( FAILED( mD3DInitialize->createDisplay( mD3DSettings, nFlags, pWinAppWindow->getWindowHandle(), pWinAppWindow->getWindowHandle(),
                                                  CG_NULL, CG_NULL, 0, 0, 0, false, mConfig.usePerfHUD ) ))
    {
        cgAppLog::write( cgAppLog::Error, _T("Device creation failed. The application will now exit.\n") );
        return false;

    } // End if Failed

    // Retrieve created device
    mD3DDeviceContext = mD3DInitialize->getDirect3DDeviceContext( );
    mD3DDevice = mD3DInitialize->getDirect3DDevice( );
    mD3DSwapChain = mD3DInitialize->getDirect3DSwapChain( );

    // Create a render target view for the primary swap chain back buffer.
    ID3D11Texture2D * pBackBuffer = CG_NULL;
    if ( FAILED( hRet = mD3DSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed
    hRet = mD3DDevice->CreateRenderTargetView( pBackBuffer, CG_NULL, &mFrameBufferView );
    pBackBuffer->Release();
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create a render target view for primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        return false;
    
    } // End if failed
    
    // Store the current window size in the settings for accountabilities sake
    if ( mConfig.windowed )
    {
        cgSize Size = mOutputWindow->getClientSize();
        mD3DSettings.windowedSettings.displayMode.Width  = Size.width;
        mD3DSettings.windowedSettings.displayMode.Height = Size.height;
    
    } // End if windowed

    // Perform an initial clear on the framebuffer targets
    cgColorValue ClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    mD3DDeviceContext->ClearRenderTargetView( mFrameBufferView, ClearColor );
    
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
bool cgDX11RenderDriver::updateDisplayMode( const cgDisplayMode & mode, bool windowed )
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
            mD3DSettings.fullScreenSettings.displayMode.RefreshRate.Numerator = (UINT)mode.refreshRate;
            mD3DSettings.fullScreenSettings.displayMode.RefreshRate.Denominator =  (mode.refreshRate == 0) ? 0 : 1;
            mD3DSettings.windowed = false;

            // Switch the focus window to the new mode and then set its size.
            mFocusWindow->setFullScreenMode( true );
            mFocusWindow->setClientSize( cgSize( mode.width, mode.height ) );

            // Release our references to the swap chain resources.
            mDeviceFrameBuffer.unloadResource();
            mFrameBufferView->Release();
            mFrameBufferView = CG_NULL;
            mD3DDeviceContext->ClearState();    

            // Resize swap chain's buffers.
            cgUInt bufferCount = (mD3DSettings.fullScreenSettings.tripleBuffering) ? 2 : 1;
            HRESULT hRet = mD3DSwapChain->ResizeBuffers( bufferCount, mode.width, mode.height, mD3DSettings.fullScreenSettings.displayMode.Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );
            if ( FAILED( hRet ) )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to resize swap chain after window resize. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                mLostDevice = true;

                cgToDo( "DX11", "Try again or exit?" );

            } // End if failed to reset

            // Switch device to full screen mode as necessary.
            BOOL fullScreenState;
            mD3DSwapChain->GetFullscreenState( &fullScreenState, CG_NULL );
            if ( fullScreenState != TRUE )
            {
                HRESULT hRet = mD3DSwapChain->SetFullscreenState( TRUE, CG_NULL );
                if ( FAILED( hRet ) )
                {
                    cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to switch display to full screen exclusive mode. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                    mLostDevice = true;

                    cgToDo( "DX11", "Try again or exit?" );

                } // End if failed to reset
            
            } // End if !fullscreen

            // Alter display mode.
            hRet = mD3DSwapChain->ResizeTarget( &mD3DSettings.fullScreenSettings.displayMode );
            if ( FAILED( hRet ) )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to alter display mode after window resize. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                mLostDevice = true;
            
            } // End if failed

            // Re-create a render target view for the primary swap chain back buffer.
            ID3D11Texture2D * pBackBuffer = CG_NULL;
            if ( FAILED( hRet = mD3DSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                mLostDevice = true;
            
            } // End if failed
            else
            {
                hRet = mD3DDevice->CreateRenderTargetView( pBackBuffer, CG_NULL, &mFrameBufferView );
                pBackBuffer->Release();
                if ( FAILED( hRet ) )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to create a render target view for primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                    mLostDevice = true;
                
                } // End if failed

                // Recreate device frame buffer target.
                mDeviceFrameBuffer.loadResource();
            
            } // End if success
            
            // Window resize events can continue.
            mSuppressResizeEvent = false;

            // Update the device configuration
            mConfig.width       = mode.width;
            mConfig.height      = mode.height;
            mConfig.refreshRate = (cgInt32)mode.refreshRate;
            mConfig.windowed    = windowed;

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
bool cgDX11RenderDriver::postInit()
{
    STRING_CONVERT;  // For string conversion macro

    // Retrieve selected adapter information
    cgDX11Settings::Settings * pSettings = mD3DSettings.getSettings();
    const cgDX11EnumAdapter  * pAdapter  = mD3DInitialize->getAdapter( pSettings->adapterOrdinal );

    // Build string for outputting selected device information to log
    cgString strDeviceName = cgString::trim(stringConvertW2CT(pAdapter->details.Description));
    cgString strDeviceInfo = _T("Selected D3D11 device '") + strDeviceName + _T("'.\n");

    // Output information
    cgAppLog::write( cgAppLog::Info, strDeviceInfo.c_str() );

    // Build component parts for the mode information required for log output
    strDeviceInfo = cgString::format( _T("Selected %s display mode of %ix%i @ %ibpp.\n"), (mD3DSettings.windowed) ? _T("windowed") : _T("fullscreen"), 
                                      pSettings->displayMode.Width, pSettings->displayMode.Height,
                                      cgBufferFormatEnum::formatBitsPerPixel( cgDX11BufferFormatEnum::formatFromNative(pSettings->displayMode.Format) ) );

    // Output information
    cgAppLog::write( cgAppLog::Info, strDeviceInfo.c_str() );

    // Store information about the type of hardware in use
    switch ( pAdapter->details.VendorId )
    {
        case DX11HardwareVendor::NVIDIA:
            mHardwareType = cgHardwareType::NVIDIA;
            break;

        case DX11HardwareVendor::AMD:
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
    dynamic_cast<cgDX11RenderingCapabilities*>(mCaps)->postInit( mD3DInitialize, mD3DSettings.fullScreenSettings.adapterOrdinal, mD3DSettings.fullScreenSettings.outputOrdinal );

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
        ImageDesc.width     = cgMathUtility::nextPowerOfTwo( cgDX11RenderingCapabilities::MaxVBTSlotsVTF * 3 ) * 2;
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

        // Create the sampler state.
        cgSamplerStateDesc SamplerDesc;
        SamplerDesc.addressU = cgAddressingMode::Clamp;
        SamplerDesc.addressV = cgAddressingMode::Clamp;
        SamplerDesc.minificationFilter  = cgFilterMethod::Point;
        SamplerDesc.magnificationFilter = cgFilterMethod::Point;
        SamplerDesc.mipmapFilter        = cgFilterMethod::None;
        if ( !mResourceManager->createSamplerState( &mVertexBlendingSampler, SamplerDesc, cgResourceFlags::ForceNew, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create vertex blending sampler states on the selected device.\n") );
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
bool cgDX11RenderDriver::cameraUpdated()
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
bool cgDX11RenderDriver::setWorldTransform( const cgMatrix * pMatrix )
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
bool cgDX11RenderDriver::setSamplerState( cgUInt32 nSamplerIndex, const cgSamplerStateHandle & hStates )
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
    cgDX11SamplerState * pState = (cgDX11SamplerState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    ID3D11SamplerState * pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified sampler state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    mD3DDeviceContext->PSSetSamplers( nSamplerIndex, 1, &pBlock );
    
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
void cgDX11RenderDriver::restoreSamplerState( cgUInt32 nSamplerIndex )
{
    cgSamplerStateHandle & hCurrentState = mSamplerStateStack[nSamplerIndex].top();
    cgDX11SamplerState * pState = (cgDX11SamplerState*)hCurrentState.getResource(true);
    ID3D11SamplerState * pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        mD3DDeviceContext->PSSetSamplers( nSamplerIndex, 1, &pBlock );
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
bool cgDX11RenderDriver::setDepthStencilState( const cgDepthStencilStateHandle & hStates, cgUInt32 nStencilRef )
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
    cgDepthStencilStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultDepthStencilState;
    cgDX11DepthStencilState * pState = (cgDX11DepthStencilState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    ID3D11DepthStencilState * pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified depth stencil state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    mD3DDeviceContext->OMSetDepthStencilState( pBlock, nStencilRef );
    
    // Clean up
    pBlock->Release();

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
void cgDX11RenderDriver::restoreDepthStencilState( )
{
    DepthStencilStateData & CurrentState = mDepthStencilStateStack.top();
    cgDX11DepthStencilState * pState = (cgDX11DepthStencilState*)CurrentState.handle.getResource(true);
    ID3D11DepthStencilState * pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        mD3DDeviceContext->OMSetDepthStencilState( pBlock, CurrentState.stencilRef );
        pBlock->Release();
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : setRasterizerState ()
/// <summary>
/// Apply the specified rasterizer state to the device. Supplying an invalid 
/// (NULL) handle will result in the default rasterizer states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setRasterizerState( const cgRasterizerStateHandle & hStates )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mRasterizerStateStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mRasterizerStateStack.top() == hStates )
        return true;

    // Retrieve the final commited state object.
    cgRasterizerStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultRasterizerState;
    cgDX11RasterizerState * pState = (cgDX11RasterizerState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    ID3D11RasterizerState * pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified rasterizer state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    mD3DDeviceContext->RSSetState( pBlock );
    
    // Clean up
    pBlock->Release();

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
void cgDX11RenderDriver::restoreRasterizerState( )
{
    cgRasterizerStateHandle & hCurrentState = mRasterizerStateStack.top();
    cgDX11RasterizerState * pState = (cgDX11RasterizerState*)hCurrentState.getResource(true);
    ID3D11RasterizerState * pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        mD3DDeviceContext->RSSetState( pBlock );
        pBlock->Release();
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : setBlendState ()
/// <summary>
/// Apply the specified blend state to the device. Supplying an invalid 
/// (NULL) handle will result in the default blend states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setBlendState( const cgBlendStateHandle & hStates )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mBlendStateStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mBlendStateStack.top() == hStates )
        return true;

    // Retrieve the final commited state object.
    cgBlendStateHandle hNewStates = hStates.isValid() ? hStates : mDefaultBlendState;
    cgDX11BlendState * pState = (cgDX11BlendState*)hNewStates.getResource(true);
    
    // Retrieve the D3D specific state block object that will be applied.
    ID3D11BlendState * pBlock = pState->getD3DStateBlock( );
    if ( !pBlock )
    {
        // Fail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind specified blend state to the selected device. No matching device state block was available.\n") );
        return false;
    
    } // End if invalid

    // Apply it.
    mD3DDeviceContext->OMSetBlendState( pBlock, cgColorValue(1.0f,1.0f,1.0f,1.0f), 0xFFFFFFFF );

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
void cgDX11RenderDriver::restoreBlendState( )
{
    cgBlendStateHandle & hCurrentState = mBlendStateStack.top();
    cgDX11BlendState * pState = (cgDX11BlendState*)hCurrentState.getResource(true);
    ID3D11BlendState * pBlock = pState->getD3DStateBlock( );
    if ( pBlock )
    {
        mD3DDeviceContext->OMSetBlendState( pBlock, cgColorValue(1.0f,1.0f,1.0f,1.0f), 0xFFFFFFFF );
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
bool cgDX11RenderDriver::setViewport( const cgViewport * pViewport )
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
    const cgViewport & NewViewport = mViewportStack.top();
    D3D11_VIEWPORT D3DViewport;
    D3DViewport.TopLeftX = (cgFloat)NewViewport.x;
    D3DViewport.TopLeftY = (cgFloat)NewViewport.y;
    D3DViewport.Width    = (cgFloat)NewViewport.width;
    D3DViewport.Height   = (cgFloat)NewViewport.height;
    D3DViewport.MinDepth = (cgFloat)NewViewport.minimumZ;
    D3DViewport.MaxDepth = (cgFloat)NewViewport.maximumZ;
    mD3DDeviceContext->RSSetViewports( 1, &D3DViewport );
    
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
//  Name : registerIAInputSignature()
/// <summary>
/// Given a 20 byte input signature hash (constructed based on a combination of
/// the shader's input types and semantics) create a unique single integer 
/// identifier that represents this specific combination of inputs -- the
/// unique shader input signature from the perspective of the D3D10/11 input
/// assembler. If a signature with this hash has already been registered, the
/// identifier that was originally associated with the hash the first time it
/// was registered will be returned.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11RenderDriver::registerIAInputSignature( cgUInt32 Hash[] )
{
    cgUInt32Array aHash( 5 );
    memcpy( &aHash[0], Hash, 5 * sizeof(cgUInt32) );
    InputSignatureHashMap::const_iterator itSignature = mIAInputSignatureLUT.find( aHash );
    if ( itSignature == mIAInputSignatureLUT.end() )
    {
        // No existing signature, add one!
        cgUInt32 nSignatureId = mNextIASignatureId++;
        mIAInputSignatureLUT[ aHash ] = nSignatureId;
        return nSignatureId;
    
    } // End if no signature
    return itSignature->second;
}

//-----------------------------------------------------------------------------
//  Name : setVertexFormat () (Virtual)
/// <summary>
/// Set the vertex format that should be assumed by any following drawing 
/// routine.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setVertexFormat( cgVertexFormat * pFormat )
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

    // Select a new input layout on the next primitive draw call.
    mSelectNewInputLayout = true;

    /*// If the old vertex format had a different stride to the new format, we need to
    // rebind vertex buffers.
    cgToDo( "Carbon General", "Vertex format system needs to be overhauled to maintain a stride per stream?" )
    if ( !pOldFormat || (pOldFormat->getStride() != pFormat->getStride()) )
    {
        ID3D11Buffer * ppBuffers[cgRenderDriver::MaxStreamSlots];
        UINT pOffsets[cgRenderDriver::MaxStreamSlots], pStrides[cgRenderDriver::MaxStreamSlots];
        mD3DDeviceContext->IAGetVertexBuffers( 0, cgRenderDriver::MaxStreamSlots, ppBuffers, pStrides, pOffsets );
        for ( cgUInt32 i = 0; i < cgRenderDriver::MaxStreamSlots; ++i )
            pStrides[i] = pFormat->getStride();
        mD3DDeviceContext->IASetVertexBuffers( 0, cgRenderDriver::MaxStreamSlots, ppBuffers, pStrides, pOffsets );

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
void cgDX11RenderDriver::restoreVertexFormat( )
{
    // Re-select input layout on the next primitive draw call.
    mSelectNewInputLayout = true;
}

//-----------------------------------------------------------------------------
//  Name : selectInputLayout () (Protected)
/// <summary>
/// Automatically find, construct and / or select the appropriate IA input
/// layout for the current vertex format / vertex shader combination.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::selectInputLayout( )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 );
    cgAssert( mVertexShaderStack.size() > 0 );

    // Retrieve the current vertex format. This forms part of the key by which
    // the correct input layout will be selected
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    if ( !pFormat )
        return false;

    // Retrieve the currently assigned vertex shader. We need this in order
    // to understand its registered IA input signature so that we can retrieve
    // the correct internal input layout object.
    cgDX11VertexShader * pShader = (cgDX11VertexShader*)mVertexShaderStack.top().getResource(true);
    if ( !pShader || !pShader->isLoaded() )
        return false;

    // Get the registered input signature identifier.
    cgUInt32 nSignatureId = pShader->getIAInputSignatureId();
    if ( nSignatureId == 0xFFFFFFFF )
        return false;

    // Retrieve the list of potential layouts for the currently assigned input signature identifier.
    InputLayoutMap & Layouts = mInputSignatureFormats[ nSignatureId ];
    
    // Retrieve the input layout associated with this format for this signature identifier.
    InputLayoutMap::iterator itItem = Layouts.find( (void*)pFormat );

    // Not yet created?
    HRESULT hRet;
    ID3D11InputLayout * pLayout = CG_NULL;
    if ( itItem == Layouts.end() )
    {
        // Translate the vertex format into an appropriate DX11 input layout.
        cgUInt nNumElements = pFormat->getElementCount();
        D3D11_INPUT_ELEMENT_DESC * pElements = new D3D11_INPUT_ELEMENT_DESC[ nNumElements ];
        for ( cgUInt i = 0; i < nNumElements; ++i )
        {
            D3D11_INPUT_ELEMENT_DESC * pOut = &pElements[i];
            const D3DVERTEXELEMENT9  * pIn  = &pFormat->getDeclarator()[ i ];
            pOut->SemanticIndex         = pIn->UsageIndex;
            pOut->AlignedByteOffset     = pIn->Offset;
            pOut->InputSlot             = pIn->Stream;
            pOut->InputSlotClass        = D3D11_INPUT_PER_VERTEX_DATA;
            pOut->InstanceDataStepRate  = 0;

            // Element format
            switch ( pIn->Type )
            {
                case D3DDECLTYPE_FLOAT1:
                    pOut->Format = DXGI_FORMAT_R32_FLOAT;
                    break;
                case D3DDECLTYPE_FLOAT2:
                    pOut->Format = DXGI_FORMAT_R32G32_FLOAT;
                    break;
                case D3DDECLTYPE_FLOAT3:
                    pOut->Format = DXGI_FORMAT_R32G32B32_FLOAT;
                    break;
                case D3DDECLTYPE_FLOAT4:
                    pOut->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;
                case D3DDECLTYPE_D3DCOLOR:
                    //pOut->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    pOut->Format = DXGI_FORMAT_B8G8R8A8_UNORM; // <--- DX11 only, works to maintain mapping of 32bit ULONG 'ARGB' with DX9
                    break;
                case D3DDECLTYPE_UBYTE4:
                    pOut->Format = DXGI_FORMAT_R8G8B8A8_UINT;
                    break;
                case D3DDECLTYPE_SHORT2:
                    pOut->Format = DXGI_FORMAT_R16G16_SINT;
                    break;
                case D3DDECLTYPE_SHORT4:
                    pOut->Format = DXGI_FORMAT_R16G16B16A16_SINT;
                    break;
                case D3DDECLTYPE_UBYTE4N:
                    pOut->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    break;
                case D3DDECLTYPE_SHORT2N:
                    pOut->Format = DXGI_FORMAT_R16G16_SNORM;
                    break;
                case D3DDECLTYPE_SHORT4N:
                    pOut->Format = DXGI_FORMAT_R16G16B16A16_SNORM;
                    break;
                case D3DDECLTYPE_USHORT2N:
                    pOut->Format = DXGI_FORMAT_R16G16_UNORM;
                    break;
                case D3DDECLTYPE_USHORT4N:
                    pOut->Format = DXGI_FORMAT_R16G16B16A16_UNORM;
                    break;
                case D3DDECLTYPE_UDEC3:
                    pOut->Format = DXGI_FORMAT_R10G10B10A2_UINT;
                    break;
                case D3DDECLTYPE_DEC3N:
                    pOut->Format = DXGI_FORMAT_R10G10B10A2_UNORM;
                    break;
                case D3DDECLTYPE_FLOAT16_2:
                    pOut->Format = DXGI_FORMAT_R16G16_FLOAT;
                    break;
                case D3DDECLTYPE_FLOAT16_4:
                    pOut->Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    break;

            } // End switch type

            // Element semantic name
            switch ( pIn->Usage )
            {
                case D3DDECLUSAGE_POSITION:
                    pOut->SemanticName = "POSITION";
                    break;
                case D3DDECLUSAGE_BLENDWEIGHT:
                    pOut->SemanticName = "BLENDWEIGHT";
                    break;
                case D3DDECLUSAGE_BLENDINDICES:
                    pOut->SemanticName = "BLENDINDICES";
                    break;
                case D3DDECLUSAGE_NORMAL:
                    pOut->SemanticName = "NORMAL";
                    break;
                case D3DDECLUSAGE_PSIZE:
                    pOut->SemanticName = "PSIZE";
                    break;
                case D3DDECLUSAGE_TEXCOORD:
                    pOut->SemanticName = "TEXCOORD";
                    break;
                case D3DDECLUSAGE_TANGENT:
                    pOut->SemanticName = "TANGENT";
                    break;
                case D3DDECLUSAGE_BINORMAL:
                    pOut->SemanticName = "BINORMAL";
                    break;
                case D3DDECLUSAGE_TESSFACTOR:
                    pOut->SemanticName = "TESSFACTOR";
                    break;
                case D3DDECLUSAGE_POSITIONT:
                    pOut->SemanticName = "POSITION";
                    break;
                case D3DDECLUSAGE_COLOR:
                    pOut->SemanticName = "COLOR";
                    break;
                case D3DDECLUSAGE_FOG:
                    pOut->SemanticName = "FOG";
                    break;
                case D3DDECLUSAGE_DEPTH:
                    pOut->SemanticName = "DEPTH";
                    break;
                case D3DDECLUSAGE_SAMPLE:
                    pOut->SemanticName = "SAMPLE";
                    break;

            } // End switch usage

        } // Next element

        // Create the input layout if not already created
        const cgByteArray & aByteCode = pShader->getByteCode();
        if ( FAILED( hRet = mD3DDevice->CreateInputLayout( pElements, nNumElements, &aByteCode[0], aByteCode.size(), &pLayout ) ) )
        {
            // Rollback and fail.
            delete []pElements;
            cgAppLog::write( cgAppLog::Error, _T("Failed to create vertex format input layout for first time use on the selected device for shader '%s'. The error reported was: (0x%x) %s - %s\n"), pShader->getResourceName().c_str(), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
            return false;
        
        } // End if failed

        // Clean up
        delete []pElements;

        // Store for later use.
        Layouts[ (void*)pFormat ] = pLayout;
        
    } // End if no D3D declaration yet
    else
    {
        // Retrieve declarator
        pLayout = itItem->second;

    } // End if exists

    // Set the input layout.
    mD3DDeviceContext->IASetInputLayout( pLayout );

    // Record the signature identifier of the most recently applied IA layout. This allows us to 
    // determine if we need to re-apply a layout when the shader changes (but the vertex format 
    // doesn't).
    mCurrentLayoutSignature = nSignatureId;
    mSelectNewInputLayout = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setMaterialTerms()
/// <summary>
/// Set the specified material terms to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setMaterialTerms( const cgMaterialTerms & Terms )
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

    // Update buffer.
    pMaterialData->diffuse              = Terms.diffuse;
    pMaterialData->ambient              = Terms.ambient;
    pMaterialData->specular             = Terms.specular;

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
    // ToDo: 6767 - This is not currently set in DX9 driver
    bool bDecodeSRGB = getSystemState( cgSystemState::HDRLighting ) > 0; 
	setSystemState( cgSystemState::DecodeSRGB, bDecodeSRGB ); 

	// ToDo: 6767 - Add proper support for new material flags. (Can create local bools to cache conditional results if desired.)
    // ToDo: 6767 - These system states should be set by the material, not the driver itself.
	bool bMetal = Terms.metalnessAmount > 0;
	setSystemState( cgSystemState::Metal, bMetal );

	bool bUseSurfaceFresnel = (Terms.fresnelDiffuse + Terms.fresnelSpecular + Terms.fresnelReflection + Terms.fresnelOpacity) > 0.0f;
	setSystemState( cgSystemState::SurfaceFresnel, bUseSurfaceFresnel );

	bool bCorrectNormals = true;
	setSystemState( cgSystemState::CorrectNormals, bCorrectNormals );

	bool bTranslucent = false;
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
bool cgDX11RenderDriver::setIndices( const cgIndexBufferHandle & hIndices )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mIndicesStack.size() > 0 );

    // Filter if this is a duplicate of the current index buffer.
    if ( mStateFilteringEnabled && mIndicesStack.top() == hIndices )
        return true;

    // Retrieve the actual underlying D3D resource (or NULL).
    cgIndexBufferHandle hNewIndices = hIndices;
    cgDX11IndexBuffer * pIndexBuffer = (cgDX11IndexBuffer*)hNewIndices.getResource(true);
    if ( pIndexBuffer )
    {
        ID3D11Buffer * pD3DIndexBuffer = pIndexBuffer->getD3DBuffer();

        // Pass through to device
        DXGI_FORMAT IndexBufferFormat = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(pIndexBuffer->getFormat());
        mD3DDeviceContext->IASetIndexBuffer( pD3DIndexBuffer, IndexBufferFormat, 0 );

        // Release our reference
        if ( pD3DIndexBuffer )
            pD3DIndexBuffer->Release();

    } // End if valid
    else
    {
        mD3DDeviceContext->IASetIndexBuffer( CG_NULL, DXGI_FORMAT_R32_UINT, 0 );

    } // End if clear

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
void cgDX11RenderDriver::restoreIndices( )
{
    cgDX11IndexBuffer * pBuffer = (cgDX11IndexBuffer*)mIndicesStack.top().getResource(true);
    ID3D11Buffer * pD3DIndexBuffer = (pBuffer) ? pBuffer->getD3DBuffer() : CG_NULL;
    if ( pD3DIndexBuffer )
    {
        DXGI_FORMAT IndexBufferFormat = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(pBuffer->getFormat());
        mD3DDeviceContext->IASetIndexBuffer( pD3DIndexBuffer, IndexBufferFormat, 0 );
        pD3DIndexBuffer->Release();
    
    } // End if valid
    else
    {
        mD3DDeviceContext->IASetIndexBuffer( CG_NULL, DXGI_FORMAT_R32_UINT, 0 );

    } // End if invalid
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource () (Virtual)
/// <summary>
/// Set the specified vertex buffer to the device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setStreamSource( cgUInt32 nStreamIndex, const cgVertexBufferHandle & hStream )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
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
    cgDX11VertexBuffer * pBuffer = (cgDX11VertexBuffer*)hNewStream.getResource(true);
    ID3D11Buffer * pD3DVertexBuffer = (pBuffer) ? pBuffer->getD3DBuffer() : CG_NULL;

    // Pass through to device
    const UINT Offset = 0;
    const UINT Stride = pFormat->getStride();
    mD3DDeviceContext->IASetVertexBuffers( nStreamIndex, 1, &pD3DVertexBuffer, &Stride, &Offset );
    
    // Release our reference
    if ( pD3DVertexBuffer )
        pD3DVertexBuffer->Release();

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
void cgDX11RenderDriver::restoreStreamSource( cgUInt32 nStreamIndex )
{
    VertexStreamData & Data = mVertexStreamStack[nStreamIndex].top();
    cgDX11VertexBuffer * pBuffer = (cgDX11VertexBuffer*)Data.handle.getResource(true);
    ID3D11Buffer * pD3DVertexBuffer = (pBuffer) ? pBuffer->getD3DBuffer() : CG_NULL;
    
    const UINT Offset = 0, Stride = Data.stride;
    mD3DDeviceContext->IASetVertexBuffers( nStreamIndex, 1, &pD3DVertexBuffer, &Stride, &Offset );
    if ( pD3DVertexBuffer )
        pD3DVertexBuffer->Release();
}

//-----------------------------------------------------------------------------
//  Name : setTexture ()
/// <summary>
/// Apply the specified texture to the indicated texture slot index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setTexture( cgUInt32 nTextureIndex, const cgTextureHandle & hTexture )
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
    ID3D11ShaderResourceView * pD3DView = CG_NULL;
    if ( pTexture )
    {
        if ( pTexture->getResourceType() == cgResourceType::RenderTarget )
            pD3DView = ((cgDX11RenderTarget*)pTexture)->getD3DShaderView();
        else if ( pTexture->getResourceType() == cgResourceType::Texture )
            pD3DView = ((cgDX11Texture<cgTexture>*)pTexture)->getD3DShaderView();
        else if ( pTexture->getResourceType() == cgResourceType::DepthStencilTarget )
            pD3DView = ((cgDX11DepthStencilTarget*)pTexture)->getD3DShaderView();

    } // End if valid
    
    // Pass through to device
    mD3DDeviceContext->PSSetShaderResources( nTextureIndex, 1, &pD3DView );
    
    // Release our reference
    if ( pD3DView )
        pD3DView->Release();

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
void cgDX11RenderDriver::restoreTexture( cgUInt32 nTextureIndex )
{
    cgTexture * pTexture = (cgTexture*)mTextureStack[nTextureIndex].top().getResource(true);
    ID3D11ShaderResourceView * pD3DView = CG_NULL;
    if ( pTexture )
    {
        if ( pTexture->getResourceType() == cgResourceType::RenderTarget )
            pD3DView = ((cgDX11RenderTarget*)pTexture)->getD3DShaderView();
        else if ( pTexture->getResourceType() == cgResourceType::Texture )
            pD3DView = ((cgDX11Texture<cgTexture>*)pTexture)->getD3DShaderView();
        else if ( pTexture->getResourceType() == cgResourceType::DepthStencilTarget )
            pD3DView = ((cgDX11DepthStencilTarget*)pTexture)->getD3DShaderView();

    } // End if valid

    mD3DDeviceContext->PSSetShaderResources( nTextureIndex, 1, &pD3DView );
    if ( pD3DView )
        pD3DView->Release();
}

//-----------------------------------------------------------------------------
//  Name : drawIndexedPrimitive () (Virtual)
/// <summary>
/// Render the primitives specified using the currently set stream
/// sources, and index buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11RenderDriver::drawIndexedPrimitive( cgPrimitiveType::Base Type, cgInt32 BaseVertexIndex, cgUInt32 MinIndex, cgUInt32 NumVertices, cgUInt32 StartIndex, cgUInt32 PrimitiveCount )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Set primitive topology
    cgToDo( "Carbon General", "Only modify topology if it has changed!"  );
    mD3DDeviceContext->IASetPrimitiveTopology( (D3D11_PRIMITIVE_TOPOLOGY)Type );
    
    // Select input layout if necessary.
    if ( mSelectNewInputLayout )
        selectInputLayout();

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX11ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX11ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( i, mD3DDeviceContext ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Compute number of indices to draw.
    cgUInt nIndexCount = 0;
    switch ( Type )
    {
        case cgPrimitiveType::PointList:
            nIndexCount = PrimitiveCount;
            break;
        case cgPrimitiveType::LineList:
            nIndexCount = PrimitiveCount * 2;
            break;
        case cgPrimitiveType::LineStrip:
            nIndexCount = PrimitiveCount + 1;
            break;
        case cgPrimitiveType::TriangleList:
            nIndexCount = PrimitiveCount * 3;
            break;
        case cgPrimitiveType::TriangleStrip:
            nIndexCount = PrimitiveCount + 2;
            break;
        default:
            return;
    
    } // End switch Type

    // Pass through to device
    mD3DDeviceContext->DrawIndexed( nIndexCount, StartIndex, BaseVertexIndex );
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
void cgDX11RenderDriver::drawPrimitive( cgPrimitiveType::Base Type, cgUInt32 StartVertex, cgUInt32 PrimitiveCount )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Set primitive topology
    cgToDo( "Carbon General", "Only modify topology if it has changed!"  );
    mD3DDeviceContext->IASetPrimitiveTopology( (D3D11_PRIMITIVE_TOPOLOGY)Type );

    // Select input layout if necessary.
    if ( mSelectNewInputLayout )
        selectInputLayout();

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX11ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX11ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( i, mD3DDeviceContext ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Compute number of vertices to draw.
    cgUInt nVertexCount = 0;
    switch ( Type )
    {
        case cgPrimitiveType::PointList:
            nVertexCount = PrimitiveCount;
            break;
        case cgPrimitiveType::LineList:
            nVertexCount = PrimitiveCount * 2;
            break;
        case cgPrimitiveType::LineStrip:
            nVertexCount = PrimitiveCount + 1;
            break;
        case cgPrimitiveType::TriangleList:
            nVertexCount = PrimitiveCount * 3;
            break;
        case cgPrimitiveType::TriangleStrip:
            nVertexCount = PrimitiveCount + 2;
            break;
        default:
            return;
    
    } // End switch Type

    // Pass through to device
    mD3DDeviceContext->Draw( nVertexCount, StartVertex );
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
void cgDX11RenderDriver::drawPrimitiveUP( cgPrimitiveType::Base Type, cgUInt32 PrimitiveCount, const void * pVertexStreamZeroData )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Select input layout if necessary.
    if ( mSelectNewInputLayout )
        selectInputLayout();

    // Set primitive topology
    cgToDo( "Carbon General", "Only modify topology if it has changed!"  );
    mD3DDeviceContext->IASetPrimitiveTopology( (D3D11_PRIMITIVE_TOPOLOGY)Type );

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX11ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX11ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( i, mD3DDeviceContext ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Compute number of vertices to draw.
    cgUInt nVertexCount = 0;
    switch ( Type )
    {
        case cgPrimitiveType::PointList:
            nVertexCount = PrimitiveCount;
            break;
        case cgPrimitiveType::LineList:
            nVertexCount = PrimitiveCount * 2;
            break;
        case cgPrimitiveType::LineStrip:
            nVertexCount = PrimitiveCount + 1;
            break;
        case cgPrimitiveType::TriangleList:
            nVertexCount = PrimitiveCount * 3;
            break;
        case cgPrimitiveType::TriangleStrip:
            nVertexCount = PrimitiveCount + 2;
            break;
        default:
            return;
    
    } // End switch Type

    // Is there a vertex buffer available for the requirest vertex format
    // that is large enough to contain the requires vertex count?
    UserPointerVertices * pVerticesContainer = CG_NULL;
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    UserPointerVertices & VerticesContainer = mUserPointerVertices[ pFormat ];
    if ( VerticesContainer.vertexCapacity < nVertexCount )
    {
        // Existing buffer (if there was one) is too small. Allocate a new one.
        if ( !mResourceManager->createVertexBuffer( &VerticesContainer.buffer, nVertexCount * pFormat->getStride(), 
                                                      cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic, pFormat, cgMemoryPool::Default, cgDebugSource() ) )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Warning, _T("Failed to allocate a vertex buffer large enough to contain the number of vertices specified during a user pointer drawPrimitive call. Future messages will be surpressed.\n") );
            bLog = false;

            // Cancel profile
#           if defined( CGE_PROFILEPRIMITIVES )
            mProfiler->endProcess();
#           endif
            return;

        } // End if failed

        // Store remaining values
        VerticesContainer.vertexFormat = pFormat;
        VerticesContainer.vertexCapacity  = nVertexCount;

    } // End if too small

    // Populate the buffer.
    void * pDstVertices = CG_NULL;
    cgVertexBuffer * pBuffer = VerticesContainer.buffer.getResource(true);
    if ( !pBuffer || !(pDstVertices = pBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Warning, _T("Failed to lock vertex buffer during a user pointer drawPrimitive call. Future messages will be surpressed.\n") );
        bLog = false;

        // Cancel profile
#       if defined( CGE_PROFILEPRIMITIVES )
        mProfiler->endProcess();
#       endif
        return;

    } // End if lock failure

    // Copy data over and unlock.
    memcpy( pDstVertices, pVertexStreamZeroData, nVertexCount * pFormat->getStride() );
    pBuffer->unlock( );

    // Temporarily set this as the stream source
    pushStreamSource( 0, VerticesContainer.buffer );

    // Render.
    mD3DDeviceContext->Draw( nVertexCount, 0 );
    mPrimitivesDrawn = PrimitiveCount;

    // Clean up
    popStreamSource( 0 );
     
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
void cgDX11RenderDriver::drawIndexedPrimitiveUP( cgPrimitiveType::Base Type, cgUInt32 MinVertexIndex, cgUInt32 NumVertices, cgUInt32 PrimitiveCount, const void * pIndexData, cgBufferFormat::Base IndexDataFormat, const void * pVertexStreamZeroData )
{
    cgToDo( "Effect Overhaul", "Duplicate constants into b and i registers as necessary (as indicated by the shaders)." );

    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );

    // Begin profiling draw primitive call on request.
#   if defined( CGE_PROFILEPRIMITIVES )
    mProfiler->beginProcess( _T("drawPrimitive") );
#   endif

    // Select input layout if necessary.
    if ( mSelectNewInputLayout )
        selectInputLayout();

    // Set primitive topology
    cgToDo( "Carbon General", "Only modify topology if it has changed!"  );
    mD3DDeviceContext->IASetPrimitiveTopology( (D3D11_PRIMITIVE_TOPOLOGY)Type );

    // Update any dirty constant buffers
    if ( mConstantsDirty )
    {
        cgDX11ConstantBuffer * pBuffer;
        for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
        {
            if ( mConstantsDirty & (1 << i)  )
            {
                // This buffer is dirty, update it.
                cgConstantBufferHandle & hBuffer = mConstantBufferStack[i].top();
                if ( (pBuffer = (cgDX11ConstantBuffer*)hBuffer.getResource(true)) && pBuffer->isLoaded() )
                {
                    if ( pBuffer->apply( i, mD3DDeviceContext ) )
                        mConstantsDirty &= ~(1 << i);
                
                } // End if valid buffer
                
            } // End if this buffer dirty

        } // Next buffer
    
    } // End if at least one buffer dirty

    // Compute number of indices to draw.
    cgUInt nIndexCount = 0;
    switch ( Type )
    {
        case cgPrimitiveType::PointList:
            nIndexCount = PrimitiveCount;
            break;
        case cgPrimitiveType::LineList:
            nIndexCount = PrimitiveCount * 2;
            break;
        case cgPrimitiveType::LineStrip:
            nIndexCount = PrimitiveCount + 1;
            break;
        case cgPrimitiveType::TriangleList:
            nIndexCount = PrimitiveCount * 3;
            break;
        case cgPrimitiveType::TriangleStrip:
            nIndexCount = PrimitiveCount + 2;
            break;
        default:
            return;
    
    } // End switch Type

    // Is there a vertex buffer available for the requested vertex format
    // that is large enough to contain the requires vertex count?
    UserPointerVertices * pVerticesContainer = CG_NULL;
    cgVertexFormat * pFormat = mVertexFormatStack.top();
    UserPointerVertices & VerticesContainer = mUserPointerVertices[ pFormat ];
    if ( VerticesContainer.vertexCapacity < NumVertices )
    {
        // Existing buffer (if there was one) is too small. Allocate a new one.
        if ( !mResourceManager->createVertexBuffer( &VerticesContainer.buffer, NumVertices * pFormat->getStride(), 
                                                      cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic, pFormat, cgMemoryPool::Default, cgDebugSource() ) )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Warning, _T("Failed to allocate a vertex buffer large enough to contain the number of vertices specified during a user pointer drawPrimitive call. Future messages will be surpressed.\n") );
            bLog = false;

            // Cancel profile
#           if defined( CGE_PROFILEPRIMITIVES )
            mProfiler->endProcess();
#           endif
            return;

        } // End if failed

        // Store remaining values
        VerticesContainer.vertexFormat = pFormat;
        VerticesContainer.vertexCapacity  = NumVertices;

    } // End if too small

    // Populate the buffer.
    void * pDstVertices = CG_NULL;
    cgVertexBuffer * pVertexBuffer = VerticesContainer.buffer.getResource(true);
    if ( !pVertexBuffer || !(pDstVertices = pVertexBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Warning, _T("Failed to lock vertex buffer during a user pointer drawPrimitive call. Future messages will be surpressed.\n") );
        bLog = false;

        // Cancel profile
#       if defined( CGE_PROFILEPRIMITIVES )
        mProfiler->endProcess();
#       endif
        return;

    } // End if lock failure

    // Copy data over and unlock.
    memcpy( pDstVertices, ((cgByte*)pVertexStreamZeroData) + MinVertexIndex * pFormat->getStride(), NumVertices * pFormat->getStride() );
    pVertexBuffer->unlock( );

    // Temporarily set this as the stream source
    pushStreamSource( 0, VerticesContainer.buffer );

    // Is there an index buffer available for the requirest index format
    // that is large enough to contain the required index count?
    cgInt32 nElementStride = (IndexDataFormat==cgBufferFormat::Index32) ? 4 : 2;
    UserPointerIndices * pIndicesContainer = CG_NULL;
    UserPointerIndices & IndicesContainer = mUserPointerIndices[ (cgUInt32)(IndexDataFormat) ];
    if ( IndicesContainer.indexCapacity < nIndexCount )
    {
        // Existing buffer (if there was one) is too small. Allocate a new one.
        if ( !mResourceManager->createIndexBuffer( &IndicesContainer.buffer, nIndexCount * nElementStride, 
                                                      cgBufferUsage::WriteOnly | cgBufferUsage::Dynamic, IndexDataFormat,
                                                      cgMemoryPool::Default, cgDebugSource() ) )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Warning, _T("Failed to allocate an index buffer large enough to contain the number of indices specified during a user pointer drawIndexedPrimitive call. Future messages will be surpressed.\n") );
            bLog = false;

            // Cancel profile
#           if defined( CGE_PROFILEPRIMITIVES )
            mProfiler->endProcess();
#           endif
            return;

        } // End if failed

        // Store remaining values
        IndicesContainer.format       = IndexDataFormat;
        IndicesContainer.indexCapacity  = nIndexCount;

    } // End if too small

    // Populate the buffer.
    void * pDstIndices = CG_NULL;
    cgIndexBuffer * pIndexBuffer = IndicesContainer.buffer.getResource(true);
    if ( !pIndexBuffer || !(pDstIndices = pIndexBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Warning, _T("Failed to lock index buffer during a user pointer drawIndexedPrimitive call. Future messages will be surpressed.\n") );
        bLog = false;

        // Cancel profile
#       if defined( CGE_PROFILEPRIMITIVES )
        mProfiler->endProcess();
#       endif
        return;

    } // End if lock failure

    // Copy data over and unlock.
    memcpy( pDstIndices, pIndexData, nIndexCount * nElementStride );
    pIndexBuffer->unlock( );

    // Temporarily set this as the index source
    pushIndices( IndicesContainer.buffer );

    // Pass through to device (note that we use a negative BaseVertexLocation
    // value because we only copied the *active* region of the vertex buffer)
    mD3DDeviceContext->DrawIndexed( nIndexCount, 0, -(INT)MinVertexIndex );
    mPrimitivesDrawn = PrimitiveCount;

    // Clean up
    popStreamSource( 0 );
    popIndices( );
     
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
bool cgDX11RenderDriver::setScissorRect( const cgRect * pRect )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mScissorRectStack.size() > 0 );

    // Call base class implementation.
    cgRect rcOld = mScissorRectStack.top();
    if ( !cgRenderDriver::setScissorRect( pRect ) )
        return false;

    // Pass through to D3D device
    mD3DDeviceContext->RSSetScissorRects( 1, (const D3D11_RECT*)&mScissorRectStack.top() );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onDeviceLost () (Protected Virtual)
/// <summary>
/// Notify all systems that the device has been lost.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11RenderDriver::onDeviceLost()
{
    // Destroy unmanaged resources
    //clearQueries();

    cgToDo( "DX11", "Not actually a 'lost' device, but refer to 'GetDeviceRemovedReason()' for implementation details." );
    
    // Call base class implementation.
    cgRenderDriver::onDeviceLost();
}

//-----------------------------------------------------------------------------
//  Name : clear () (Virtual)
/// <summary>
/// Clear the frame, depth and stencil buffers.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::clear( cgUInt32 nFlags, cgUInt32 nColor, cgFloat fDepth, cgUInt8 nStencil )
{
    // Get the current render target(s)
    TargetData & Data = mTargetStack.top();

    // Are we clearing render targets?
    if ( nFlags & cgClearFlags::Target )
    {
        cgColorValue c(nColor);
        for ( size_t i = 0; i < Data.renderTargets.size(); ++i )
        {
            cgDX11RenderTarget * pTarget = (cgDX11RenderTarget*)Data.renderTargets[i].getResource(true);
            if ( pTarget && pTarget->isLoaded() )
            {
                ID3D11RenderTargetView * pView = pTarget->getD3DTargetView();
                if ( pView )
                {
                    mD3DDeviceContext->ClearRenderTargetView( pView, c );
                    pView->Release();
                
                } // End if has view
            
            } // End if valid
        
        } // Next target

    } // End if clearing targets

    // Clear depth or stencil?
    if ( (nFlags & cgClearFlags::Depth) || (nFlags & cgClearFlags::Stencil) )
    {
        cgDX11DepthStencilTarget * pTarget = (cgDX11DepthStencilTarget*)Data.depthStencil.getResource(true);
        if ( pTarget && pTarget->isLoaded() )
        {
            ID3D11DepthStencilView * pView = pTarget->getD3DTargetView();
            if ( pView )
            {
                cgUInt nD3DFlags = 0;
                if ( nFlags & cgClearFlags::Depth )
                    nD3DFlags |= D3D11_CLEAR_DEPTH;
                if ( nFlags & cgClearFlags::Stencil )
                    nD3DFlags |= D3D11_CLEAR_STENCIL;
                mD3DDeviceContext->ClearDepthStencilView( pView, nD3DFlags, fDepth, nStencil );
                pView->Release();

            } // End if has view

        } // End if valid

    } // End if clear depth / stencil
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : clear () (Virtual)
/// <summary>
/// Clear the frame, depth and stencil buffers within the specified
/// rectangular areas.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::clear( cgUInt32 nRectCount, cgRect * pRectangles, cgUInt32 nFlags, cgUInt32 nColor, cgFloat fDepth, cgUInt8 nStencil )
{
    cgToDo( "DX11", "How can we clear a specific rectangle?" );
    clear( nFlags, nColor, fDepth, nStencil );

    /*cgAssert( isInitialized() == true );
    HRESULT hRet = mD3DDevice->Clear( nRectCount, (D3DRECT*)pRectangles, nFlags, nColor, fDepth, nStencil );
    return SUCCEEDED(hRet);*/
    // 7777
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginFrame () (Virtual)
/// <summary>
/// Begin rendering process for this frame
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::beginFrame( bool bClearTarget, cgUInt32 nTargetColor )
{
    static bool bSpewError = true;

    // Validate requirements
    cgAssert( isInitialized() == true );

    cgToDo( "Carbon General", "When device is reset (in all cases), potentially re-apply vertex shader, pixel shader, state blocks, vertex format, etc." )
    
    // Recover lost device if required
    // 7777
    /*if ( mLostDevice == true )
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
            if ( FAILED( hRet = mD3DInitialize->ResetDisplay( mD3DDevice, mD3DSettings, CG_NULL, false ) ) )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to reset display device. Waiting to try again. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                return false;
            
            } // End if failed
            else
            {
                // Restore post-reset
                OnDeviceReset();

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

    } // End if Device Lost*/

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
    if ( bClearTarget )
    {
        cgColorValue ClearColor( nTargetColor );
        mD3DDeviceContext->ClearRenderTargetView( mFrameBufferView, ClearColor );
    
    } // End if clear

    // Apply system constant buffers.
    setConstantBufferAuto( mSceneConstants );
    setConstantBufferAuto( mCameraConstants );
    setConstantBufferAuto( mShadowConstants );
    setConstantBufferAuto( mMaterialConstants );
    setConstantBufferAuto( mObjectConstants );
    setConstantBufferAuto( mWorldConstants );
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
bool cgDX11RenderDriver::beginTargetRender( cgRenderTargetHandle hRenderTarget, cgInt32 nCubeFace, bool bAutoUseMultiSample, cgDepthStencilTargetHandle hDepthStencilTarget )
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

    // Retrieve the new render target view
    cgDX11RenderTarget * pRenderTarget = (cgDX11RenderTarget*)hRenderTarget.getResource(true);
    ID3D11RenderTargetView * pTargetView = CG_NULL;
    if ( pRenderTarget )
    {
        // Bind to device
        pTargetView = pRenderTarget->getD3DTargetView( );
        
        // If user specified a cube face into which we would render, set it
        // here otherwise retain the original cube face.
        if ( Data.cubeFace >= 0 )
            pRenderTarget->setCurrentCubeFace( (cgTexture::CubeFace)Data.cubeFace );
        else
            Data.cubeFace = pRenderTarget->getCurrentCubeFace();

        // Render target data is no longer lost (if it was previously)
        // since it has been re-populated.
        pRenderTarget->setResourceLost(false);

    } // End if valid

    // Retrieve the depth stencil view
    cgDX11DepthStencilTarget * pDepthTarget = (cgDX11DepthStencilTarget*)Data.depthStencil.getResource(true);
    ID3D11DepthStencilView * pDepthView = CG_NULL;
    if ( pDepthTarget && pDepthTarget->isLoaded() )
    {
        pDepthView = pDepthTarget->getD3DTargetView();
        
        // Depth stencil data is no longer lost (if it was previously)
        // since it has been re-populated.
        pDepthTarget->setResourceLost(false);
    
    } // End if valid

    // Set to the device
    mD3DDeviceContext->OMSetRenderTargets( 1, &pTargetView, pDepthView );

    // Release temporary references
    if ( pTargetView )
        pTargetView->Release();
    if ( pDepthView )
        pDepthView->Release();

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
    //pCameraData->screenUVAdjustBias.x  = (pCameraData->viewportOffset.x + 0.5f) * pCameraData->targetSize.z;
    //pCameraData->screenUVAdjustBias.y  = (pCameraData->viewportOffset.y + 0.5f) * pCameraData->targetSize.w;
    // No half texel offset in DX11
    pCameraData->screenUVAdjustBias.x  = 0.0f; //0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.0f; //0.5f * pCameraData->targetSize.w;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Match D3D9 behavior and reset the viewport when setting a render target.
    cgViewport Viewport;
    Viewport.x      = 0;
    Viewport.y      = 0;
    Viewport.width  = mTargetSize.width;
    Viewport.height = mTargetSize.height;
    Viewport.minimumZ   = 0.0f;
    Viewport.maximumZ   = 1.0f;
    pushViewport( &Viewport );

    // Set a full size scissor rectangle by default.
    D3D11_RECT rcScissors;
    rcScissors.left   = (cgInt)Viewport.x;
    rcScissors.right  = rcScissors.left + (cgInt)Viewport.width;
    rcScissors.top    = (cgInt)Viewport.y;
    rcScissors.bottom = rcScissors.left + (cgInt)Viewport.height;
    mD3DDeviceContext->RSSetScissorRects( 1, &rcScissors );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTargetRender() (Virtual)
/// <summary>
/// Begin rendering to one or more render targets.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::beginTargetRender( cgRenderTargetHandleArray aRenderTargets, bool bAutoUseMultiSample, cgDepthStencilTargetHandle hDepthStencilTarget )
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
    cgDX11RenderTarget * pFirstTarget = CG_NULL;
    ID3D11RenderTargetView * ppTargetViews[8] = {0};
    for ( size_t i = 0; i < min( 8, aRenderTargets.size() ); ++i )
    {
        // Retrieve the render target
        cgDX11RenderTarget * pRenderTarget = (cgDX11RenderTarget*)aRenderTargets[i].getResource( true );
        if ( pRenderTarget )
        {
            // Retrieve the target surface and set
            ppTargetViews[i] = pRenderTarget->getD3DTargetView( );
            
            // Render target data is no longer lost (if it was previously)
            // since it has been re-populated.
            pRenderTarget->setResourceLost(false);

            // Record the reference to the first valid target encountered.
            // We'll need this later in order to extract some additional 
            // information such as the render target size, etc.
            if ( !pFirstTarget )
                pFirstTarget = pRenderTarget;

        } // End if valid target
	
    } // Next Render Target

    // Retrieve the depth stencil surface
    cgDX11DepthStencilTarget * pDepthTarget = (cgDX11DepthStencilTarget*)Data.depthStencil.getResource(true);
    ID3D11DepthStencilView * pDepthView = CG_NULL;
    if ( pDepthTarget && pDepthTarget->isLoaded() )
    {
        pDepthView = pDepthTarget->getD3DTargetView();
        
        // Depth stencil data is no longer lost (if it was previously)
        // since it has been re-populated.
        pDepthTarget->setResourceLost(false);
    
    } // End if valid

    // Set to the device
    mD3DDeviceContext->OMSetRenderTargets( aRenderTargets.size(), ppTargetViews, pDepthView );

    // Release temporary references
    for ( size_t i = 0; i < min( 8, aRenderTargets.size() ); ++i )
    {
        if ( ppTargetViews[i] )
            ppTargetViews[i]->Release();
    } // Next target
    if ( pDepthView )
        pDepthView->Release();

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
    //pCameraData->screenUVAdjustBias.x  = (pCameraData->viewportOffset.x + 0.5f) * pCameraData->targetSize.z;
    //pCameraData->screenUVAdjustBias.y  = (pCameraData->viewportOffset.y + 0.5f) * pCameraData->targetSize.w;
    // No half texel offset in DX11
    pCameraData->screenUVAdjustBias.x  = 0.0f; //0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.0f; //0.5f * pCameraData->targetSize.w;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'drawPrimitive*' is called.
    pCameraBuffer->unlock();

    // Match D3D9 behavior and reset the viewport when setting a render target.
    cgViewport Viewport;
    Viewport.x      = 0;
    Viewport.y      = 0;
    Viewport.width  = mTargetSize.width;
    Viewport.height = mTargetSize.height;
    Viewport.minimumZ   = 0.0f;
    Viewport.maximumZ   = 1.0f;
    pushViewport( &Viewport );

    // Set a full size scissor rectangle by default.
    D3D11_RECT rcScissors;
    rcScissors.left   = (cgInt)Viewport.x;
    rcScissors.right  = rcScissors.left + (cgInt)Viewport.width;
    rcScissors.top    = (cgInt)Viewport.y;
    rcScissors.bottom = rcScissors.left + (cgInt)Viewport.height;
    mD3DDeviceContext->RSSetScissorRects( 1, &rcScissors );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endTargetRender() (Virtual)
/// <summary>
/// Complete the rendering stage for the most recent render target.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::endTargetRender( )
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
    cgDX11RenderTarget * pFirstTarget = CG_NULL;
    ID3D11RenderTargetView * ppTargetViews[8] = {0};
    for ( size_t i = 0; i < min(8, Data.renderTargets.size()); ++i )
    {
        // Retrieve the prior render target
        cgDX11RenderTarget * pRenderTarget = (cgDX11RenderTarget*)Data.renderTargets[i].getResource( true );
        if ( pRenderTarget )
        {
            // Retrieve the target surface and set
            ppTargetViews[i] = pRenderTarget->getD3DTargetView( );

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

    // Retrieve the prior depth stencil surface
    cgDX11DepthStencilTarget * pDepthTarget = (cgDX11DepthStencilTarget*)Data.depthStencil.getResource(true);
    ID3D11DepthStencilView * pDepthView = CG_NULL;
    if ( pDepthTarget && pDepthTarget->isLoaded() )
        pDepthView = pDepthTarget->getD3DTargetView();

    // Set to the device
    mD3DDeviceContext->OMSetRenderTargets( Data.renderTargets.size(), ppTargetViews, pDepthView );

    // Release temporary references
    for ( size_t i = 0; i < min( 8, Data.renderTargets.size() ); ++i )
    {
        if ( ppTargetViews[i] )
            ppTargetViews[i]->Release();
    } // Next target
    if ( pDepthView )
        pDepthView->Release();
    
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
    // No half texel offset in DX11
    pCameraData->screenUVAdjustBias.x  = 0.0f; //0.5f * pCameraData->targetSize.z;
    pCameraData->screenUVAdjustBias.y  = 0.0f; //0.5f * pCameraData->targetSize.w;

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
void cgDX11RenderDriver::endFrame( cgAppWindow * pWndOverride, bool bPresent )
{
    // If we're in a lost state we should bail right here
    // 7777
    //if ( mLostDevice )
        //return;

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

    // Should we present?
    if ( bPresent )
    {
        // 7777
        /*cgRect  rcSource;
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
            hWndOverride = pWinAppWindow->getWindowHandle();*/

        // Present the buffer
        //mProfiler->beginProcess( _T("Presentation") );
        cgToDo( "DX11", "How to overlay onto a different window!" );
        //HRESULT hRet = mD3DDevice->Present( (RECT*)pSourceRect, CG_NULL, hWndOverride, CG_NULL );
        HRESULT hRet = mD3DSwapChain->Present( isVSyncEnabled() ? 1 : 0, 0 );
        //mProfiler->endProcess( );

        // 7777
        /*// Did we fail to present?
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
        
        } // End if failed*/

    } // End if present

    // Finish frame profiling
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
bool cgDX11RenderDriver::stretchRect( cgTextureHandle & hSource, const cgRect * pSrcRect, cgTextureHandle hDestination, const cgRect * pDstRect, cgFilterMethod::Base Filter )
{
    ID3D11Resource * pSrcResource = CG_NULL, * pDstResource = CG_NULL;

    // Valid source texture?
    cgTexture * pSrcTexture = hSource.getResource( true );
    if ( pSrcTexture )
    {
        // Update the texture (will automatically resolve any prior 
        // multi-sampled target data).
        pSrcTexture->update();

        // Retrieve the surface.
        if ( pSrcTexture->getResourceType() == cgResourceType::RenderTarget )
            pSrcResource = ((cgDX11RenderTarget*)pSrcTexture)->getD3DTexture();
        else if ( pSrcTexture->getResourceType() == cgResourceType::Texture )
            pSrcResource  = ((cgDX11Texture<cgTexture>*)pSrcTexture)->getD3DTexture();

    } // End if valid

    // Validate success
    if ( !pSrcResource )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve the source hardware surface when performing stretchRect operation.\n") );
        return false;
    
    } // End if !source

    // Retrieve destination surface.
    cgTexture * pDstTexture = hDestination.getResource( true );
    if ( pDstTexture )
    {
        // Retrieve the surface.
        if ( pDstTexture->getResourceType() == cgResourceType::RenderTarget )
            pDstResource = ((cgDX11RenderTarget*)pDstTexture)->getD3DTexture( );
        else if ( pDstTexture->getResourceType() == cgResourceType::Texture )
            pDstResource = ((cgDX11Texture<cgRenderTarget>*)pDstTexture)->getD3DTexture();
        
    } // End if valid texture

    // Validate success
    if ( !pDstResource )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve the destination hardware surface when performing stretchRect operation.\n") );
        pSrcResource->Release();
        return false;
    
    } // End if !source

    // We would like our version of stretch rect to automatically handle cases where
    // the destination rectangle falls outside the bounds of the destination surface.
    // First compute the full size of the destination surface.
    const cgImageInfo & DstInfo = pDstTexture->getInfo();
    cgRect rcDstBounds = cgRect( 0, 0, DstInfo.width, DstInfo.height );
    
    // Next we need to compute actual rectangles for any that have not already been
    // supplied by the caller. Start with the source rectangle.
    cgRect rcSrc;
    const cgImageInfo & SrcInfo = pSrcTexture->getInfo();
    if ( !pSrcRect )
        rcSrc = cgRect( 0, 0, SrcInfo.width, SrcInfo.height );
    else
        rcSrc = *pSrcRect;
    
    // Same for destination.
    cgRect rcDst;
    if ( !pDstRect )
        rcDst = rcDstBounds;
    else
        rcDst = *pDstRect;

    // Now attempt to clip the destination rectangle to the bounds of the destination surface.
    cgRect rcDstClipped;
    if ( IntersectRect( (RECT*)&rcDstClipped, (RECT*)&rcDst, (RECT*)&rcDstBounds ) == FALSE )
    {
        // No intersection occured, the destination is off the screen.
        pSrcResource->Release();
        pDstResource->Release();
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

    // If the source and destination rectangles are the same size, and the
    // formats are from the same group (i.e. float vs uint variations) perform 
    // a direct resource subregion copy. Otherwise, we need to use a quad
    // draw to perform the operation.
    bool bResult = false;
    if ( rcSrc.width() == rcDst.width() && rcSrc.height() == rcDst.height() &&
         mCaps->getBufferFormats().formatGroupMatches( SrcInfo.format, DstInfo.format ) )
    {
        D3D11_BOX SourceBox;
        SourceBox.left      = rcSrc.left;
        SourceBox.top       = rcSrc.top;
        SourceBox.right     = rcSrc.right;
        SourceBox.bottom    = rcSrc.bottom;
        SourceBox.front     = 0;
        SourceBox.back      = 1;
        mD3DDeviceContext->CopySubresourceRegion( pDstResource, 0, rcDst.left, rcDst.top, 0,
                                                    pSrcResource, 0, &SourceBox );
        bResult = true;

    } // End if same size
    else
    {
        cgToDo( "DX11", "Needs to utilize a shader draw if the source and destination rectangles are not the same size / format!" );
        bResult = false;

    } // End if stretching
    
    cgToDo( "Carbon General", "Need to filter destination texture to update mips?" )

    // Clean up
    pSrcResource->Release();
    pDstResource->Release();

    // Success?
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : activateQuery() (Virtual)
/// <summary>
/// Activate a new query ready to record occlusion results.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgDX11RenderDriver::activateQuery( )
{
    // Debug validation
    cgAssert( isInitialized() == true );

    // Build the query descriptor.
    D3D11_QUERY_DESC Desc;
    Desc.Query = D3D11_QUERY_OCCLUSION;
    Desc.MiscFlags = 0;

    // Create the new device query.
    HRESULT hRet;
    ID3D11Query * pQuery = CG_NULL;
    if ( FAILED( hRet = mD3DDevice->CreateQuery( &Desc, &pQuery ) ) )
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
void cgDX11RenderDriver::deactivateQuery( cgInt32 nQueryId )
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
bool cgDX11RenderDriver::validQuery( cgInt32 nQueryId )
{
    return ( mActiveQueries.find( nQueryId ) != mActiveQueries.end() );
}

//-----------------------------------------------------------------------------
//  Name : clearQueries() (Virtual)
/// <summary>
/// Remove all active queries.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11RenderDriver::clearQueries()
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
bool cgDX11RenderDriver::checkQueryResults( cgInt32 nQueryId, cgUInt32 & nPixelCount, bool bWaitForResults )
{
    // Debug validation
    cgAssert( isInitialized() == true );

    // Determine if query exists.
    ID3D11Query * pQuery = CG_NULL;
    QueryMap::iterator itQuery = mActiveQueries.find( nQueryId );
    if ( itQuery == mActiveQueries.end() )
        return false;
    else
        pQuery = itQuery->second;

	// If results are available, return success
    cgUInt64 nData;
	if ( mD3DDeviceContext->GetData( pQuery, &nData, sizeof(nData), D3D11_ASYNC_GETDATA_DONOTFLUSH ) == S_OK )
    {
        nPixelCount = (cgUInt32)nData;
		return true;
    
    } // End if found

	// Should we flush?
	if ( bWaitForResults )
	{
		while ( mD3DDeviceContext->GetData( pQuery, &nData, sizeof(nData), 0 ) == S_FALSE )
        {
            // Burn time!
            // ToDo: sleep(0)?
        
        } // Next Iteration
        nPixelCount = (cgUInt32)nData;
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
bool cgDX11RenderDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11RenderDriver )
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
bool cgDX11RenderDriver::processMessage( cgMessage * pMessage )
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
void cgDX11RenderDriver::windowResized( cgInt32 nWidth, cgInt32 nHeight )
{
    // If we're in fullscreen, or not yet initialized, this is a no-op
    if ( !isInitialized() || !isWindowed() )
        return;

    // Release our references to the swap chain resources.
    mDeviceFrameBuffer.unloadResource();
    mFrameBufferView->Release();
    mFrameBufferView = CG_NULL;
    mD3DDeviceContext->ClearState();

    // Update Settings
    cgDX11Settings::Settings *pSettings = mD3DSettings.getSettings();
    pSettings->displayMode.Width  = nWidth;
    pSettings->displayMode.Height = nHeight;

    // Resize swap chain's buffers.
    cgUInt nBufferCount = (pSettings->tripleBuffering) ? 2 : 1;
    HRESULT hRet = mD3DSwapChain->ResizeBuffers( nBufferCount, nWidth, nHeight, pSettings->displayMode.Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH );
    if ( FAILED( hRet ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to resize swap chain after window resize. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
        mLostDevice = true;

        cgToDo( "DX11", "Try again or exit?" );

    } // End if failed to reset
    else
    {
        // Re-create a render target view for the primary swap chain back buffer.
        ID3D11Texture2D * pBackBuffer = CG_NULL;
        if ( FAILED( hRet = mD3DSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
            mLostDevice = true;
        
        } // End if failed
        else
        {
            hRet = mD3DDevice->CreateRenderTargetView( pBackBuffer, CG_NULL, &mFrameBufferView );
            pBackBuffer->Release();
            if ( FAILED( hRet ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to create a render target view for primary swap chain back buffer on the selected device. The error reported was: (0x%x) %s - %s\n"), hRet, DXGetErrorString( hRet ), DXGetErrorDescription( hRet ) );
                mLostDevice = true;
            
            } // End if failed

            // Recreate device frame buffer target.
            mDeviceFrameBuffer.loadResource();
        
        } // End if success

    } // End if reset
    
    // Call base class implementation.
    cgRenderDriver::windowResized( nWidth, nHeight );
}

//-----------------------------------------------------------------------------
//  Name : setUserClipPlanes() 
/// <summary>
/// Sets user defined clip planes (maximum equivalent to MaxUDCPSlots) with 
/// optional negation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setUserClipPlanes( const cgPlane pWorldSpacePlanes[], cgUInt32 nNumPlanes, bool bNegatePlanes /* = true */ )
{
    cgToDoAssert( "DX11", "cgDX11RenderDriver::setUserClipPlanes()" );

    // 7777
    /*// Validate requirements
    cgAssert( isInitialized() == true );

    // Call base class implementation
    if ( !cgRenderDriver::setUserClipPlanes( pWorldSpacePlanes, nNumPlanes, bNegatePlanes ) )
        return false;

    // Planes need to be in clip space for use with shaders. Generate a duplicate set
    // of clip planes that have been transformed into this space.
	cgMatrix mtxITViewProj;
    const ClipPlaneData & Data = mClipPlaneStack.top();
    cgPlane ClipSpacePlanes[MaxUDCPSlots];
    memcpy( ClipSpacePlanes, Data.planes, Data.planeCount * sizeof(cgPlane) );
    if ( cgMatrix::inverse( &mtxITViewProj, CG_NULL, &mViewProjectionMatrix ) )
    {
        cgMatrix::transpose( &mtxITViewProj, &mtxITViewProj );
    	cgPlane::transformArray( ClipSpacePlanes, sizeof(cgPlane), ClipSpacePlanes, sizeof(cgPlane), &mtxITViewProj, Data.planeCount );

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
    return true;*/
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setVertexShader () (Virtual)
/// <summary>
/// Set the current vertex shader applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setVertexShader( const cgVertexShaderHandle & hShader )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mVertexShaderStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mVertexShaderStack.top() == hShader )
        return true;

    // Set to the device
    cgVertexShaderHandle hNewShader = hShader;
    cgDX11VertexShader * pShader = (cgDX11VertexShader*)hNewShader.getResource(true);
    ID3D11VertexShader * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    mD3DDeviceContext->VSSetShader( pD3DShader, CG_NULL, 0 );
    if ( pD3DShader )
        pD3DShader->Release();

    // If the input signature identifier of the vertex shader is different to the one
    // most recently applied then we need to apply the correct layout for this new
    // vertex shader.
    if ( pShader && pShader->getIAInputSignatureId() != mCurrentLayoutSignature )
        mSelectNewInputLayout = true;

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
void cgDX11RenderDriver::restoreVertexShader( )
{
    cgDX11VertexShader * pShader = (cgDX11VertexShader*)mVertexShaderStack.top().getResource(true);
    ID3D11VertexShader * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    mD3DDeviceContext->VSSetShader( pD3DShader, CG_NULL, 0 );
    if ( pD3DShader )
        pD3DShader->Release();

    // If the input signature identifier of the vertex shader is different to the one
    // most recently applied then we need to apply the correct layout for this new
    // vertex shader.
    if ( pShader && pShader->getIAInputSignatureId() != mCurrentLayoutSignature )
        mSelectNewInputLayout = true;
}

//-----------------------------------------------------------------------------
//  Name : setPixelShader () (Virtual)
/// <summary>
/// Set the current pixel shader applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setPixelShader( const cgPixelShaderHandle & hShader )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( mPixelShaderStack.size() > 0 );

    // Filter if this is a duplicate of the current.
    if ( mStateFilteringEnabled && mPixelShaderStack.top() == hShader )
        return true;

    // Set to the device
    cgPixelShaderHandle hNewShader = hShader;
    cgDX11PixelShader * pShader = (cgDX11PixelShader*)hNewShader.getResource(true);
    ID3D11PixelShader * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    mD3DDeviceContext->PSSetShader( pD3DShader, CG_NULL, 0 );
    if ( pD3DShader )
        pD3DShader->Release();

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
void cgDX11RenderDriver::restorePixelShader( )
{
    cgDX11PixelShader * pShader = (cgDX11PixelShader*)mPixelShaderStack.top().getResource(true);
    ID3D11PixelShader * pD3DShader = (pShader) ? pShader->getD3DShader() : CG_NULL;
    mD3DDeviceContext->PSSetShader( pD3DShader, CG_NULL, 0 );
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
bool cgDX11RenderDriver::setVertexBlendData( const cgMatrix pMatrices[], const cgMatrix pITMatrices[], cgUInt32 MatrixCount, cgInt32 nMaxBlendIndex /* = -1 */ )
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
            cgDX11Texture<cgTexture> * pTexture = (cgDX11Texture<cgTexture>*)mVertexBlendingTexture.getResource(true);
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
                ID3D11ShaderResourceView * pD3DView = pTexture->getD3DShaderView();
                mD3DDeviceContext->VSSetShaderResources( 0, 1, &pD3DView );
                if ( pD3DView )
                    pD3DView->Release();

                // Setup VTF sampler states.
                cgDX11SamplerState * pState = (cgDX11SamplerState*)mVertexBlendingSampler.getResource(true);
                ID3D11SamplerState * pBlock = pState->getD3DStateBlock( );
                if ( pBlock )
                {
                    mD3DDeviceContext->VSSetSamplers( 0, 1, &pBlock );
                    pBlock->Release();
                
                } // End if valid

                cgToDo( "Carbon General", "Output specific warning on failure." );
            
            } // End if valid

        } // End if matrices supplied

    } // End if VTF

    // Set the system script side max blend index member variable.
    *mSystemExportVars.maximumBlendIndex = nMaxBlendIndex;
    *mSystemExportVars.useVTFBlending = mConfig.useVTFBlending;

    /*// Update constant buffers only if any matrices are supplied.
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
        memcpy( pMatrixData->Transforms, pMatrices, MatrixCount * sizeof(cgMatrix) );

        // Unlock the buffer. If it is currently bound to the device
        // then the appropriate constants will be automatically updated
        // next time 'drawPrimitive*' is called.
        pMatrixBuffer->unlock();

        // Now update the second buffer containing any inverse transpose duplicates (where available).
        pMatrixBuffer = mVertexBlendingITConstants.getResource( true );
        cgAssert( pMatrixBuffer != CG_NULL );

        // Lock the buffer ready for population
        _cbVertexBlendingIT * pMatrixDataIT;
        if ( !(pMatrixDataIT = (_cbVertexBlendingIT*)pMatrixBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to lock inverse transpose vertex blending constant buffer in preparation for device update.\n") );
            return false;

        } // End if failed

        // Update buffer with matrix data.
        memcpy( pMatrixDataIT->ITTransforms, (pITMatrices) ? pITMatrices : pMatrices, MatrixCount * sizeof(cgMatrix) );

        // Unlock the buffer. If it is currently bound to the device
        // then the appropriate constants will be automatically updated
        // next time 'drawPrimitive*' is called.
        pMatrixBuffer->unlock();

    } // End if matrices supplied.*/
    
    // Set the system script side max blend index member variable.
    //*mSystemExportVars.maximumBlendIndex = nMaxBlendIndex;


    // 7777
    /*static LPDIRECT3DTEXTURE9 pTexture = CG_NULL;
    if ( !pTexture )
        mD3DDevice->CreateTexture( 512, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &pTexture, CG_NULL );
    D3DLOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, CG_NULL, D3DLOCK_DISCARD );
    cgVector4 * pRows = (cgVector4*)LockedRect.pBits;
    for ( int i = 0; i < MatrixCount; ++i )
    {
        const cgMatrix & m = pMatrices[i];
        pRows[i*4]   = cgVector4( m._11, m._12, m._13, m._41 );
        pRows[i*4+1] = cgVector4( m._21, m._22, m._23, m._42 );
        pRows[i*4+2] = cgVector4( m._31, m._32, m._33, m._43 );
        
        const cgMatrix & mit  = (pITMatrices) ? pITMatrices[i] : pMatrices[i];
        pRows[i*4+256] = cgVector4( mit._11, mit._12, mit._13, mit._41 );
        pRows[i*4+257] = cgVector4( mit._21, mit._22, mit._23, mit._42 );
        pRows[i*4+258] = cgVector4( mit._31, mit._32, mit._33, mit._43 );
    }
    pTexture->UnlockRect(0);

    mD3DDevice->setTexture( D3DVERTEXTEXTURESAMPLER0, pTexture );
    mD3DDevice->setSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
    mD3DDevice->setSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    mD3DDevice->setSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    mD3DDevice->setSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    mD3DDevice->setSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);*/

    /*cgDX9Settings::Settings * pSettings = mD3DSettings.getSettings();
    if (FAILED(m_pD3D->CheckDeviceFormat(pSettings->adapterOrdinal, pSettings->DeviceType, pSettings->displayMode.format,
                                         D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, D3DFMT_A32B32G32R32F)))
    {   
        MessageBox(NULL, L"D3DFMT_A32B32G32R32F cannot be used for vertex textures.", L"Failed", MB_OK | MB_ICONERROR);   
        return false;
    }*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVPLData ()
/// <summary>
/// Sets the depth and normal buffers needed for virtual point light injection.
/// ToDo: THIS FUNCTION NEEDS A PROPER DX11 APPROACH!
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::setVPLData( const cgTextureHandle & hDepth, const cgTextureHandle & hNormal )
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
            // Simply set sampler slot 0 with the merged texture
            mSampler2D0->apply( mVPLBuffer );
        }
        else
        {
            // ToDo 9999: R2VB
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
cgSize cgDX11RenderDriver::getScreenSize( ) const
{
    const cgDX11Settings::Settings * pSettings = mD3DSettings.getSettings();
    return cgSize( pSettings->displayMode.Width, pSettings->displayMode.Height );
}

//-----------------------------------------------------------------------------
//  Name : isWindowed () (Virtual)
/// <summary>
/// Determine if the device is currently functioning in a windowed mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderDriver::isWindowed( ) const
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
bool cgDX11RenderDriver::isVSyncEnabled( ) const
{
    return mConfig.useVSync;
}

//-----------------------------------------------------------------------------
//  Name : getD3DDevice()
/// <summary>
/// Retrieve the internal D3D device object specific to the Direct3D
/// platform implementation. This is not technically part of the public
/// interface and should not be called by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11Device * cgDX11RenderDriver::getD3DDevice( ) const
{
    if ( mD3DDevice )
        mD3DDevice->AddRef();
    return mD3DDevice;
}

//-----------------------------------------------------------------------------
//  Name : getD3DDeviceContext()
/// <summary>
/// Retrieve the internal D3D device context object specific to the Direct3D
/// platform implementation. This is not technically part of the public
/// interface and should not be called by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11DeviceContext * cgDX11RenderDriver::getD3DDeviceContext( ) const
{
    if ( mD3DDeviceContext )
        mD3DDeviceContext->AddRef();
    return mD3DDeviceContext;
}

//-----------------------------------------------------------------------------
//  Name : getD3DBackBuffer()
/// <summary>
/// Get the resource that represents the primary swap chain back buffer.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11Resource * cgDX11RenderDriver::getD3DBackBuffer( ) const
{
    ID3D11Resource * pBackBuffer = CG_NULL;
    mD3DSwapChain->GetBuffer( 0, __uuidof( ID3D11Resource ), (void**)&pBackBuffer );
    return pBackBuffer;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11RenderDriverInit Module Local Class
///////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Name : cgDX11RenderDriverInit () (Constructor)
// Desc : Constructor for this class.
//----------------------------------------------------------------------------
cgDX11RenderDriverInit::cgDX11RenderDriverInit( const cgRenderDriver::InitConfig & Config, bool bEnforceConfig ) : cgDX11Initialize( )
{
    // Store the configuration options
    mConfig   = Config;
    mEnforce = bEnforceConfig;
}

//----------------------------------------------------------------------------
// Name : validateDisplayMode () (Virtual)
// Desc : Allows us to validate and reject any adapter display modes.
//----------------------------------------------------------------------------
bool cgDX11RenderDriverInit::validateDisplayMode( const DXGI_MODE_DESC & Mode )
{
    // Compute final refresh rate.
    cgUInt nDenominator = max( 1, Mode.RefreshRate.Denominator );
    cgDouble fRefreshRate = Mode.RefreshRate.Numerator / (cgDouble)nDenominator;

    // Test display mode
    if ( Mode.Width < 640 || Mode.Height < 480 || fRefreshRate <= 30 )
        return false;

    // Disallow formats other than 32bpp
    cgUInt32 nBitsPerPixel = cgBufferFormatEnum::formatBitsPerPixel(cgDX11BufferFormatEnum::formatFromNative(Mode.Format));
    if ( nBitsPerPixel < 32 )
        return false;
    
    cgToDo( "Carbon General", "Ideally we need 'FindBestFullscreenDisplayMode()' to work based on the options it built, not ask DXGI to do it based on unvalidated modes." )
    // Enforce fullscreen config options?
    if ( mEnforce && !mConfig.windowed )
    {
        // All details must match exactly
        if ( Mode.Width != mConfig.width || Mode.Height != mConfig.height ||
            (mConfig.refreshRate != 0 && (cgUInt32)fRefreshRate != mConfig.refreshRate) )
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
bool cgDX11RenderDriverInit::validateDevice( cgDX11EnumAdapter * pAdapter, D3D_DRIVER_TYPE Type, D3D_FEATURE_LEVEL MaxLevel, ID3D11Device * pDevice )
{
    // Skip anything that isn't a hardware device.
    if ( Type != D3D_DRIVER_TYPE_HARDWARE )
        return false;

    // Enforce adapter / device selection?
    if ( mEnforce && !mConfig.deviceName.empty() )
    {
        // Ignore if the device name does not match the adapter description
        STRING_CONVERT;
        cgString strDeviceName = cgString::trim(stringConvertW2CT(pAdapter->details.Description));
        if ( !(strDeviceName == mConfig.deviceName)  )
            return false;

    } // End if enforce

    // Supported
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT