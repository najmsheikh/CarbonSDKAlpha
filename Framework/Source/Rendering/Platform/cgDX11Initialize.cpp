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
// Name : cgDX11Initialize.cpp                                               //
//                                                                           //
// Desc : Provides support for rapidly enumerating hardware support and      //
//        initializing Direct3D 11 accordingly.                              //
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
// cgDX11Initialize Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX11Initialize.h>
#include <iostream>
#include <dxerr.h>

//-----------------------------------------------------------------------------
// cgDX11Initialize Specific Constants
//-----------------------------------------------------------------------------
const cgUInt32              ValidAdapterFormatCount = 3;
const cgUInt32              BackBufferFormatCount   = 3;
const cgUInt32              DriverTypeCount         = 3;
const cgUInt32              FeatureLevelCount       = 3;

const D3D_DRIVER_TYPE       DriverTypes[]           = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
const D3D_FEATURE_LEVEL     FeatureLevels[]         = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
const DXGI_FORMAT           ValidAdapterFormats[]   = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM };
const DXGI_FORMAT           BackBufferFormats[]     = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM };

//-----------------------------------------------------------------------------
// Name : ~cgDX11EnumAdapter () (Destructor)
// Desc : cgDX11EnumAdapter Class Destructor
//----------------------------------------------------------------------------
cgDX11EnumAdapter::~cgDX11EnumAdapter()
{
    // Release all devices
    for ( size_t i = 0; i < devices.size(); i++ )
        delete devices[i];

    // Release all outputs
    for ( size_t i = 0; i < outputs.size(); i++ )
        delete outputs[i];

    // Release all outputs
    for ( size_t i = 0; i < options.size(); i++ )
        delete options[i];
    
    // Clear Vectors
    outputs.clear();
    options.clear();
    devices.clear();

    // Release DXGI object
    if ( adapter )
        adapter->Release();
    adapter = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~cgDX11EnumOutput () (Destructor)
// Desc : cgDX11EnumOutput Class Destructor
//----------------------------------------------------------------------------
cgDX11EnumOutput::~cgDX11EnumOutput()
{
    // Release DXGI object
    if ( output )
        output->Release();
    output = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgDX11Initialize () (Constructor)
/// <summary>
/// cgDX11Initialize Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX11Initialize::cgDX11Initialize()
{
	// Reset / Clear all required values
    mFactory          = CG_NULL;
    mD3DDevice        = CG_NULL;
    mD3DDeviceContext = CG_NULL;
    mD3DSwapChain     = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11Initialize () (Destructor)
/// <summary>
/// cgDX11Initialize Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX11Initialize::~cgDX11Initialize()
{
    // Release all enumerated adapters
    for ( size_t i = 0; i < mAdapters.size(); i++ )
        delete mAdapters[i];

    // Clear Vectors
    mAdapters.clear();

	// Clean up D3D Objects
    if ( mD3DDeviceContext )
        mD3DDeviceContext->Release();
    if ( mD3DDevice )
        mD3DDevice->Release();
    if ( mD3DSwapChain )
        mD3DSwapChain->Release();
    if ( mFactory )
        mFactory->Release();
    mD3DDevice        = CG_NULL;
    mD3DDeviceContext = CG_NULL;
    mD3DSwapChain     = CG_NULL;
    mFactory          = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : enumerate ()
/// <summary>
/// This function must be called first to enumerate all available 
/// devices, adapters, modes and formats, prior to initialization.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerate( )
{
    // Create the DXGI factory object for enumeration
    HRESULT hRet = CreateDXGIFactory1( __uuidof(IDXGIFactory1), (void**)(&mFactory) );

    // Enumerate the adapters
    if ( FAILED( hRet = enumerateAdapters() ) )
        return hRet;

    // Success!
    return S_OK;
}

//-----------------------------------------------------------------------------
//  Name : createDisplay ()
/// <summary>
/// Creates the display devices ready for rendering.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::createDisplay( cgDX11Settings & D3DSettings, cgUInt32 Flags, HWND hFocusWnd, HWND hOutputWnd, WNDPROC pWndProc, LPCTSTR Title, cgUInt32 Width, cgUInt32 Height, LPVOID lParam, bool bModifyWindow, bool bPerfHUDAttach )
{
    cgDX11Settings::Settings *pSettings   = D3DSettings.getSettings();
    
    if ( !hFocusWnd )
    {
        // Register the new windows window class.
        WNDCLASS			wc;	
	    wc.style			= CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
	    wc.lpfnWndProc		= pWndProc;
	    wc.cbClsExtra		= 0;
	    wc.cbWndExtra		= 0;
	    wc.hInstance		= (HINSTANCE)GetModuleHandle(CG_NULL);
        wc.hIcon			= CG_NULL;
	    wc.hCursor			= LoadCursor(CG_NULL, IDC_ARROW);
	    wc.hbrBackground	= (HBRUSH )GetStockObject(BLACK_BRUSH);
	    wc.lpszMenuName		= CG_NULL;
	    wc.lpszClassName	= Title;
	    RegisterClass(&wc);

        cgUInt32 Left  = CW_USEDEFAULT, Top = CW_USEDEFAULT;
        cgUInt32 Style = WS_OVERLAPPEDWINDOW;

        // Create the rendering window
        if ( !D3DSettings.windowed )
        {
            Left   = 0; Top = 0;
            Width  = pSettings->displayMode.Width;
            Height = pSettings->displayMode.Height;
            Style  = WS_VISIBLE | WS_POPUP;
	        
        } // End if Fullscreen
        else
        {
            // Store the current window size in the settings for accountabilities sake
            D3DSettings.windowedSettings.displayMode.Width  = Width;
            D3DSettings.windowedSettings.displayMode.Height = Height;

            // Adjust width / height to take into account window decoration
            Width  += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
            Height += (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );
        
        } // End if windowed

        // Create the window
        mFocusWnd = CreateWindow( Title, Title, Style, Left, Top, Width, Height, CG_NULL, CG_NULL, wc.hInstance, lParam );
        
        // Bail on error
        if (!mFocusWnd) return E_FAIL;

    } // End if no Window Passed
    else
    {
        // Store HWND
        mFocusWnd = hFocusWnd;
        
        // Application allows us to modify the window?
        if ( bModifyWindow )
        {
            // Setup styles based on windowed / fullscreen mode
            if ( !D3DSettings.windowed )
            {
                SetMenu( mFocusWnd, CG_NULL );
                SetWindowLong( mFocusWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP );
                SetWindowPos( mFocusWnd, CG_NULL, 0, 0, pSettings->displayMode.Width, pSettings->displayMode.Height, SWP_NOZORDER );
            
            } // End if Fullscreen
            else
            {
                RECT rc;

                // Get the windows client rectangle
                GetWindowRect( mFocusWnd, &rc );

                // Setup the window properties
                SetWindowLong( mFocusWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );
                SetWindowPos( mFocusWnd, HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_SHOWWINDOW );

                // Store the current window size in the settings for accountabilities sake
                GetClientRect( mFocusWnd, &rc );
                D3DSettings.windowedSettings.displayMode.Width  = rc.right - rc.left;
                D3DSettings.windowedSettings.displayMode.Height = rc.bottom - rc.top;

            } // End if windowed

        } // End if can modify window
    
    } // End if window passed

    // Find the selected adapter.
    IDXGIAdapter1 * pDXGIAdapter = CG_NULL;
    D3D_DRIVER_TYPE DriverType = pSettings->driverType;
    if ( DriverType == D3D_DRIVER_TYPE_HARDWARE )
    {
        for ( size_t i = 0; i < mAdapters.size(); ++i )
        {
            if ( mAdapters[i]->ordinal == pSettings->adapterOrdinal )
            {
                pDXGIAdapter = mAdapters[i]->adapter;
                pDXGIAdapter->AddRef();
                break;
            
            } // End if match

        } // Next adapter

        // Driver type must default to 'UNKNOWN' when specifying an adapter.
        if ( pDXGIAdapter )
            DriverType = D3D_DRIVER_TYPE_UNKNOWN;
    
    } // End if hardware

    // Build the swap chain parameters
    if ( !hOutputWnd )
        hOutputWnd = mFocusWnd;
    DXGI_SWAP_CHAIN_DESC scDesc = buildSwapChainParameters( D3DSettings, hOutputWnd );

    // Enable debugging if applicable.
    #ifdef _DEBUG
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    // Note: D3D11 is not supported by PerfHUD, only D3D10. Included here for reference
    // in a D3D10 port if it becomes necessary.
    // If we are using nVidia's PerfHUD tool to analyze performance (nVidia HW only)
    // setup the device appropriately.
    /*if ( bPerfHUDAttach == true )
    {
        // Find the PerfHUD adapter
        IDXGIAdapter1 * pDXGIPerfHUDAdapter = CG_NULL;
        for ( cgUInt Adapter = 0; ; ++Adapter )
        {
            // Get next adapter (exit search on failure).
            if ( FAILED(mFactory->EnumAdapters1( Adapter, &pDXGIPerfHUDAdapter )) )
                break;

            if ( pDXGIPerfHUDAdapter )
            {
                DXGI_ADAPTER_DESC AdapterDesc;
                if ( SUCCEEDED( pDXGIPerfHUDAdapter->GetDesc(&AdapterDesc) ) )
                {
                    std::wcout << AdapterDesc.Description << std::endl;
                    if ( wcscmp( AdapterDesc.Description, L"NVIDIA PerfHUD" ) == 0 )
                    {
                        pDXGIAdapter = pDXGIPerfHUDAdapter;
                        DriverType = D3D_DRIVER_TYPE_REFERENCE;
                        std::cout << "PerfHUD adapter = " << Adapter << std::endl;
                        break;

                    } // End if PerfHUD

                } // End if success

                // Release temporary interface
                pDXGIPerfHUDAdapter->Release();

            } // End if Valid
        
        } // Next Adapter

    } // End if bPerfHUDAttach*/

    // Create the device
    HRESULT hRet;
    D3D_FEATURE_LEVEL SelectedFeatureLevel;
    hRet = D3D11CreateDeviceAndSwapChain( pDXGIAdapter, DriverType, CG_NULL, Flags, FeatureLevels, FeatureLevelCount,
                                          D3D11_SDK_VERSION, &scDesc, &mD3DSwapChain, &mD3DDevice, &SelectedFeatureLevel,
                                          &mD3DDeviceContext );

    // Release our temporary reference to the adapeter
    if ( pDXGIAdapter )
        pDXGIAdapter->Release();

    // Did the creation fail ?
    if ( FAILED( hRet ) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T( "Failed to create D3D device: %s\n" ), DXGetErrorString( hRet ) );

        if ( mD3DDevice )
            mD3DDevice->Release();
        if ( mD3DDeviceContext )
            mD3DDeviceContext->Release();
        if ( mD3DSwapChain )
            mD3DSwapChain->Release();
        mD3DDevice = CG_NULL;
        mD3DDeviceContext = CG_NULL;
        mD3DSwapChain = CG_NULL;
        return hRet;

    } // End if failed

    // Success
    return S_OK;
}

/*//-----------------------------------------------------------------------------
//  Name : ResetDisplay ()
/// <summary>
/// Reset the display device, and optionally the window etc.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::ResetDisplay( IDirect3DDevice9 * pD3DDevice, cgDX11Settings& D3DSettings, HWND hWnd, bool bCreateDepthBuffer )
{   
    D3DPRESENT_PARAMETERS    d3dpp     = BuildPresentParameters( D3DSettings, bCreateDepthBuffer );
    cgDX11Settings::Settings *pSettings = D3DSettings.getSettings();

    // Validate Requirements
    if ( !pD3DDevice )
        return S_OK;

    if ( hWnd )
    {
        // Setup styles based on windowed / fullscreen mode
        if ( !D3DSettings.windowed )
        {
            SetMenu( hWnd, CG_NULL );
            SetWindowLong( hWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP );
            SetWindowPos( hWnd, CG_NULL, 0, 0, pSettings->displayMode.Width, pSettings->displayMode.Height, SWP_NOZORDER );
        
        } // End if Fullscreen
        else
        {
            // Use fullscreen settings as the foundation for the window size
            cgDX11Settings::Settings * pSettings = &D3DSettings.Fullscreen_Settings;

            // Change back to an overlapped window
            SetWindowLong( hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );

            // Adjust width / height to take into account window decoration
            cgUInt32 Width = pSettings->displayMode.Width + GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
            cgUInt32 Height = pSettings->displayMode.Width + (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );

            // Update position
            SetWindowPos( hWnd, HWND_NOTOPMOST, 50, 50, Width, Height, SWP_NOACTIVATE | SWP_SHOWWINDOW );

        } // End if windowed

    } // End if

    // Reset the device
    HRESULT hRet = pD3DDevice->Reset( &d3dpp );
    if ( FAILED( hRet ) )
        return hRet;

    // Success
    return S_OK;
}*/

//-----------------------------------------------------------------------------
//  Name : enumerateAdapters () (Private)
/// <summary>
/// Enumerates the individual adapters contained within the user machine.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateAdapters()
{
    HRESULT hRet;

    // Loop through each adapter
    for ( cgUInt32 i = 0; ; ++i )
    {
        // DXGI_ERROR_NOT_FOUND is expected when the end of the list is hit
        IDXGIAdapter1 * pDXGIAdapter = CG_NULL;
        hRet = mFactory->EnumAdapters1( i, &pDXGIAdapter );
        if ( hRet == DXGI_ERROR_NOT_FOUND )
            break;
        else if ( FAILED( hRet ) )
            return hRet;

        // Create a new internal structure to store information about this adapter
        cgDX11EnumAdapter * pAdapter = new cgDX11EnumAdapter();
        pAdapter->ordinal = i;
        pAdapter->adapter = pDXGIAdapter;

        // Retrieve adapter description.
        if ( FAILED( pDXGIAdapter->GetDesc1( &pAdapter->details ) ) )
        {
            delete pAdapter;
            continue;
        
        } // End if failed

        // Enumerate the device driver types on the adapter.
        if ( FAILED( hRet = enumerateDevices( pAdapter ) ) ) 
        {
            delete pAdapter;
            if ( hRet == E_ABORT )
                continue;
            else
                return hRet;
        
        } // End if Failed Code

        // Enumerate the available outputs on the adapter (stores display modes).
        if ( FAILED( hRet = enumerateOutputs( pAdapter ) ) ) 
        {
            delete pAdapter;
            if ( hRet == E_ABORT )
                continue;
            else
                return hRet;
        
        } // End if Failed Code

        // Finally, enumerate all of the various combinations of available options 
        // for the different outputs and devices.
        if ( FAILED( hRet = enumerateDeviceOptions( pAdapter ) ) )
        {
            delete pAdapter;
            if ( hRet == E_ABORT )
                continue;
            else
                return hRet;
        
        } // End if Failed Code

        // Add this adapter to the list
        mAdapters.push_back( pAdapter );

    } // Next Adapter

    // Success!
    return S_OK;

}

//-----------------------------------------------------------------------------
//  Name : enumerateDevices () (Private)
/// <summary>
/// Enumerates the valid device types for the specified adapter.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateDevices( cgDX11EnumAdapter * pAdapter )
{
    HRESULT  hRet;
    
    // Loop through each driver type (HARDWARE, WARP, REFERENCE)
    for ( cgUInt32 i = 0; i < DriverTypeCount; i++ )
    {
        D3D_FEATURE_LEVEL MaxLevel;
        ID3D11Device * pD3DDevice = CG_NULL;
        ID3D11DeviceContext * pD3DDeviceContext = CG_NULL;

        // Use D3D11CreateDevice to ensure that this is a valid D3D11 device
        // and that it supports at least one of the required feature levels.
        // Driver type must be "UNKNOWN" when specifying an adapter with
        // hardware driver type. Otherwise, an adapter of NULL should be supplied.
        IDXGIAdapter1 * pDXGIAdapter = pAdapter->adapter;
        D3D_DRIVER_TYPE DriverType = DriverTypes[i];
        if ( pDXGIAdapter && DriverType == D3D_DRIVER_TYPE_HARDWARE )
            DriverType = D3D_DRIVER_TYPE_UNKNOWN;
        else
            pDXGIAdapter = CG_NULL;

        // Attempt to create.
        hRet = D3D11CreateDevice( pDXGIAdapter, DriverType, CG_NULL, 0, FeatureLevels, FeatureLevelCount, 
                                  D3D11_SDK_VERSION, &pD3DDevice, &MaxLevel, &pD3DDeviceContext );
        if ( FAILED( hRet ) )
            continue;

        // Supported by user ?
        bool bValid = validateDevice( pAdapter, DriverTypes[ i ], MaxLevel, pD3DDevice );
        
        // Clean up device objects (they were only temporary).
        pD3DDevice->Release();
        pD3DDeviceContext->Release();
        
        // Move on to next driver type if user rejected.
        if ( !bValid )
            continue;

        // Allocate a new device details structure
        cgDX11EnumDevice * pDevice = new cgDX11EnumDevice();

        // Store device information
        pDevice->driverType      = DriverTypes[i];
        pDevice->maximumFeatureLevel = MaxLevel;

        // Add it to our adapter device list
        pAdapter->devices.push_back( pDevice );

    } // Next Device Type

    // Success?
    return (pAdapter->devices.empty()) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateOutputs () (Private)
/// <summary>
/// Enumerates the valid outputs that are registered for the specified adapter.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateOutputs( cgDX11EnumAdapter * pAdapter )
{
    HRESULT  hRet;
    
    // Loop through each output found on the specified adapter.
    for ( cgUInt32 i = 0; ; i++ )
    {
        // DXGI_ERROR_NOT_FOUND is expected when the end of the list is hit
        IDXGIOutput * pDXGIOutput = CG_NULL;
        hRet = pAdapter->adapter->EnumOutputs( i, &pDXGIOutput );
        if ( hRet == DXGI_ERROR_NOT_FOUND )
            break;
        else if ( FAILED( hRet ) )
            return hRet;
        
        // Create a new internal structure to store information about this output
        cgDX11EnumOutput * pOutput = new cgDX11EnumOutput();
        pOutput->ordinal = i;
        pOutput->output = pDXGIOutput;

        // Retrieve output description.
        if ( FAILED( pDXGIOutput->GetDesc( &pOutput->details ) ) )
        {
            delete pOutput;
            continue;
        
        } // End if failed

        // Enumerate the available display modes for this output.
        if ( FAILED( hRet = enumerateDisplayModes( pDXGIOutput, pOutput ) ) ) 
        {
            delete pOutput;
            if ( hRet == E_ABORT )
                continue;
            else
                return hRet;
        
        } // End if Failed Code

        // Add it to our adapter output list
        pAdapter->outputs.push_back( pOutput );

    } // Next Device Type

    // Success?
    return (pAdapter->outputs.empty()) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateDisplayModes () (Private)
/// <summary>
/// Enumerates all of the display modes for the adapter specified.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateDisplayModes( IDXGIOutput * pDXGIOutput, cgDX11EnumOutput * pOutput )
{
    HRESULT     hRet;
    DXGI_FORMAT RemoteFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
   
    // Loop through each valid 'Adapter' format.
    for ( cgUInt32 i = 0; i < ValidAdapterFormatCount; i++ )
    {
        // First retrieve a list of up to 512 modes in one go. This prevents us from
        // having to call 'GetDisplayModeList' twice in most cases (once to retrieve the 
        // number of modes, and once more to get the actual list); this is an expensive call.
        cgUInt nNumModes = 512;
        DXGI_MODE_DESC * pModes = new DXGI_MODE_DESC[ nNumModes ];
        hRet = pDXGIOutput->GetDisplayModeList( ValidAdapterFormats[i], DXGI_ENUM_MODES_SCALING, &nNumModes, pModes );
        
        // What was the result?
        if( hRet == MAKE_DXGI_HRESULT( 34 ) && RemoteFormat == ValidAdapterFormats[i] )
        {
            if( GetSystemMetrics( 0x1000 ) != 0 ) // SM_REMOTESESSION
            {
                // This is a remote session. Since display modes can't be enumerated in
                // this case, we create a 'fake' display mode based on the current
                // remote session screen resolution.
                DEVMODE DeviceMode;
                DeviceMode.dmSize = sizeof(DEVMODE);
                if ( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &DeviceMode ) )
                {
                    nNumModes = 1;
                    pModes[0].Width  = DeviceMode.dmPelsWidth;
                    pModes[0].Height = DeviceMode.dmPelsHeight;
                    pModes[0].Format = RemoteFormat;
                    pModes[0].RefreshRate.Numerator   = 60;
                    pModes[0].RefreshRate.Denominator = 1;
                    pModes[0].ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
                    pModes[0].Scaling = DXGI_MODE_SCALING_CENTERED;
                    hRet = S_OK;

                } // End if success
                
            } // End if remote session

            // Fail if this situation was not recovered.
            if ( FAILED( hRet ) )
            {
                delete []pModes;
                continue;

            } // End if failed

        } // End if remote session failure
        else if ( hRet == DXGI_ERROR_MORE_DATA )
        {
            // More than the original number of modes requested are available.
            // Do it the hard way :) First delete the old mode array.
            delete []pModes;

            // Request the actual number of modes.
            hRet = pDXGIOutput->GetDisplayModeList( ValidAdapterFormats[i], DXGI_ENUM_MODES_SCALING, &nNumModes, CG_NULL );
            if ( FAILED( hRet ) )
                continue;

            // Allocate enough room for the mode list.
            pModes = new DXGI_MODE_DESC[ nNumModes ];

            // Get the full mode list
            hRet = pDXGIOutput->GetDisplayModeList( ValidAdapterFormats[i], DXGI_ENUM_MODES_SCALING, &nNumModes, pModes );
            if ( FAILED( hRet ) )
            {
                delete []pModes;
                continue;
            
            } // End if failed

        } // End if more modes
        else if ( FAILED( hRet ) )
        {
            delete []pModes;
            continue;
        
        } // End if general failure
        
        // Loop through each display mode for this format
        for ( cgUInt j = 0; j < nNumModes; j++ )
        {
            // Is supported by user ?
            if ( !validateDisplayMode( pModes[j] ) )
                continue;

            // Add this mode to the output.
            pOutput->modes.push_back( pModes[j] );

        } // Next Output Mode

        // Clean up.
        delete []pModes;

    } // Next Adapter Format

    // Success?
    return (pOutput->modes.empty()) ? E_ABORT : S_OK;

}

//-----------------------------------------------------------------------------
//  Name : enumerateDeviceOptions () (Private)
/// <summary>
/// Enumerates the various initialization options available for each output
/// and device found for the adapter specified, such as back buffer formats etc.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateDeviceOptions( cgDX11EnumAdapter * pAdapter )
{
    // Iterate through each combination of output and driver type.
    for ( size_t nOutput = 0; nOutput < pAdapter->outputs.size(); ++nOutput )
    {
        cgDX11EnumOutput * pOutput = pAdapter->outputs[nOutput];
        for ( size_t nDevice = 0; nDevice < pAdapter->devices.size(); ++nDevice )
        {
            cgDX11EnumDevice * pDevice = pAdapter->devices[nDevice];

            // Iterate through each supported valid back buffer format
            // Loop through all valid back buffer formats
            for ( size_t nBackBufferFormat = 0; nBackBufferFormat < BackBufferFormatCount; ++nBackBufferFormat )
            {
                DXGI_FORMAT BackBufferFormat = BackBufferFormats[nBackBufferFormat];

                // Test windowed / Fullscreen modes
                for ( size_t nWindowed = 0; nWindowed < 2; ++nWindowed )
                {
                    // Test to see if there are any modes that have a format that
                    // matches the back buffer format being tested.
                    cgUInt32 nMatchingModes = 0;
                    for( size_t i = 0; i < pOutput->modes.size(); ++i )
                    {
                        if ( BackBufferFormat == pOutput->modes[i].Format )
                            nMatchingModes++;

                    } // Next Mode
                    
                    // No valid display mode combination 
                    if ( nMatchingModes == 0 )
                        continue;

                    // Is this option set supported by the user ?
                    if ( !validateDeviceOptions( BackBufferFormat, (nWindowed != 0) ) )
                        continue;

                    // Allocate a new device options set
                    cgDX11EnumDeviceOptions * pDeviceOptions = new cgDX11EnumDeviceOptions();

                    // Store device option details
                    pDeviceOptions->adapterOrdinal   = pAdapter->ordinal;
                    pDeviceOptions->outputOrdinal    = pOutput->ordinal;
                    pDeviceOptions->driverType       = pDevice->driverType;
                    pDeviceOptions->backBufferFormat = BackBufferFormat;
                    pDeviceOptions->windowed         = (nWindowed != 0);

                    // Enumerate the multi sampling types.
                    HRESULT hRet;
                    if ( FAILED( hRet = enumerateMultiSampleTypes( pAdapter, pDevice, pDeviceOptions ) ) )
                    {
                        // Release our invalid options
                        delete pDeviceOptions;

                        // If returned anything other than abort, this is fatal
                        if ( hRet == E_ABORT )
                            continue;
                        else
                            return hRet;
            
                    } // End if any enumeration failed

                    // Add this to the adapter.
                    pAdapter->options.push_back( pDeviceOptions );

                } // Next Windowed State
            
            } // Next Back Buffer Format

        } // Next Device

    } // Next Output

    // Success?
    return (pAdapter->options.size() == 0) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateMultiSampleTypes () (Private)
/// <summary>
/// Enumerates multi-sample types available for this device set.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX11Initialize::enumerateMultiSampleTypes( cgDX11EnumAdapter * pAdapter, cgDX11EnumDevice * pDevice, cgDX11EnumDeviceOptions * pDeviceOptions )
{
    ID3D11Device * pD3DDevice = CG_NULL;
    ID3D11DeviceContext * pD3DDeviceContext = CG_NULL;
    D3D_FEATURE_LEVEL CurrentLevel;

    // Create a device of the required type so that we can query.
    // Driver type must be "UNKNOWN" when specifying an adapter with
    // hardware driver type. Otherwise, an adapter of NULL should be supplied.
    IDXGIAdapter1 * pDXGIAdapter = pAdapter->adapter;
    D3D_DRIVER_TYPE DriverType = pDeviceOptions->driverType;
    if ( pDXGIAdapter && DriverType == D3D_DRIVER_TYPE_HARDWARE )
        DriverType = D3D_DRIVER_TYPE_UNKNOWN;
    else
        pDXGIAdapter = CG_NULL;

    // Attempt to create
    HRESULT hRet = D3D11CreateDevice( pDXGIAdapter, DriverType, CG_NULL, 0, FeatureLevels, FeatureLevelCount,
                                      D3D11_SDK_VERSION, &pD3DDevice, &CurrentLevel, &pD3DDeviceContext );
    if ( FAILED( hRet ) )
        return S_OK;

    // Something went wrong? (feature level should match the original)
    if ( CurrentLevel != pDevice->maximumFeatureLevel )
    {
         pD3DDevice->Release();
         pD3DDeviceContext->Release();
         return E_ABORT;
    
    } // End if no match

    // Test for each sample count.
    for ( cgUInt32 i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i )
    {
        cgUInt nQualityLevels;
        if ( SUCCEEDED( pD3DDevice->CheckMultisampleQualityLevels( pDeviceOptions->backBufferFormat, i, &nQualityLevels ) ) && 
             nQualityLevels > 0 )
        {
            //From D3D10 docs: When multisampling a texture, the number of quality levels available for an adapter is dependent on the texture 
            //format used and the number of samples requested. The maximum sample count is defined by 
            //D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT in d3d10.h. If the returned value of pNumQualityLevels is 0, 
            //the format and sample count combination is not supported for the installed adapter.
            pDeviceOptions->multiSampleCount.push_back( i );
            pDeviceOptions->multiSampleQualityLevels.push_back( nQualityLevels );

        } // End if success

    } // Next sample count

    // Clean up
    pD3DDevice->Release();
    pD3DDeviceContext->Release();

    // Success!
    return S_OK;
}

//-----------------------------------------------------------------------------
//  Name : buildSwapChainParameters ()
/// <summary>
/// Builds a swap chain description structure based on the specified settings.
/// </summary>
//-----------------------------------------------------------------------------
DXGI_SWAP_CHAIN_DESC cgDX11Initialize::buildSwapChainParameters( cgDX11Settings & D3DSettings, HWND hWnd, cgUInt32 Flags )
{
    DXGI_SWAP_CHAIN_DESC    scDesc;
    cgDX11Settings::Settings *pSettings = D3DSettings.getSettings();
    
    memset( &scDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC) );

    // Fill out our common parameters
    scDesc.BufferDesc   = pSettings->displayMode;
    scDesc.BufferCount  = (pSettings->tripleBuffering == true) ? 2 : 1;
    scDesc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.OutputWindow = hWnd;
    scDesc.SampleDesc.Count = pSettings->multiSampleCount;
    scDesc.SampleDesc.Quality = pSettings->multiSampleQuality;
    scDesc.Windowed     = D3DSettings.windowed;

    // Is this windowed?
    if ( D3DSettings.windowed )
    {
        RECT rcClient;
        GetClientRect( hWnd, &rcClient );
        scDesc.BufferDesc.Width = rcClient.right - rcClient.left;
        scDesc.BufferDesc.Height = rcClient.bottom - rcClient.top;

    } // End if windowed

    // Success
    return scDesc;
}

//-----------------------------------------------------------------------------
//  Name : findBestWindowedMode ()
/// <summary>
/// Find the best windowed mode, and fill out our D3DSettings structure.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11Initialize::findBestWindowedMode( cgDX11Settings & D3DSettings, bool bRequireHAL, bool bRequireREF )
{
    DXGI_MODE_DESC            BestDisplayMode;
    cgDX11EnumAdapter        *pBestAdapter = CG_NULL;
    cgDX11EnumDeviceOptions  *pBestOptions = CG_NULL;
    cgDX11Settings::Settings *pSettings    = CG_NULL;

    // Loop through each adapter
    for ( cgUInt32 i = 0; i < getAdapterCount(); i++ )
    {
        cgDX11EnumAdapter * pAdapter = mAdapters[ i ];

        // Loop through each output on the adapter
        cgInt nBestOutput = -1;
        DXGI_MODE_DESC BestOutputMode;
        for ( size_t j = 0; j < pAdapter->outputs.size(); ++j )
        {
            cgDX11EnumOutput * pOutput = pAdapter->outputs[j];

            // Find mode that most closesly resembles the desktop for this output on this adapter.
            DXGI_MODE_DESC MatchMode, ClosestOutputMode;
            MatchMode.Width             = 0;
            MatchMode.Height            = 0;
            MatchMode.Format            = DXGI_FORMAT_R8G8B8A8_UNORM;
            MatchMode.RefreshRate.Numerator = 0;
            MatchMode.RefreshRate.Denominator = 0;
            MatchMode.Scaling           = DXGI_MODE_SCALING_UNSPECIFIED;
            MatchMode.ScanlineOrdering  = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            if ( FAILED( pOutput->output->FindClosestMatchingMode( &MatchMode, &ClosestOutputMode, CG_NULL ) ) )
                continue;

            // Use the first one we find (favor primary display output)
            if ( nBestOutput < 0 )
            {
                BestOutputMode = ClosestOutputMode;
                nBestOutput = j;
                break;

            } // End if no best

        } // Next Output

        // If no matching mode was found, try another adapter.
        if ( nBestOutput < 0 )
            continue;

        // Loop through each option set in the adapter for this output.
        for ( size_t j = 0; j < pAdapter->options.size(); ++j )
        {
            cgDX11EnumDeviceOptions * pOptions = pAdapter->options[ j ];
            
            // Skip if this is not a windowed option
            if ( !pOptions->windowed )
                continue;

            // Skip if the driver is not of the required type
            if ( bRequireHAL && pOptions->driverType != D3D_DRIVER_TYPE_HARDWARE )
                continue;
            if ( bRequireREF && pOptions->driverType != D3D_DRIVER_TYPE_REFERENCE )
                continue;

            // Skip if the back buffer format does not match the mode closest to that requested.
            if ( pOptions->backBufferFormat != BestOutputMode.Format )
                continue;

            // If we haven't found a compatible option set yet, or if this set
            // is better (because its HAL / format matches better) then save it.
            if ( pBestOptions == CG_NULL ||
                (pBestOptions->driverType != D3D_DRIVER_TYPE_HARDWARE && pOptions->driverType == D3D_DRIVER_TYPE_HARDWARE ) )
            {
                // Store best so far
                BestDisplayMode = BestOutputMode;
                pBestAdapter = pAdapter;
                pBestOptions = pOptions;
                
                // This fullscreen device option looks good?
                if ( pOptions->driverType == D3D_DRIVER_TYPE_HARDWARE )
                    goto EndWindowedDeviceOptionSearch;

            } // End if not a better match
        
        } // Next Option Set

    } // Next Adapter

EndWindowedDeviceOptionSearch:
    
    if ( !pBestOptions )
        return false;

    // Fill out passed settings details
    D3DSettings.windowed               = true;
    pSettings                          = D3DSettings.getSettings();
    pSettings->adapterOrdinal          = pBestOptions->adapterOrdinal;
    pSettings->outputOrdinal           = pBestOptions->outputOrdinal;
    pSettings->displayMode             = BestDisplayMode;
    pSettings->driverType              = pBestOptions->driverType;
    pSettings->backBufferFormat        = pBestOptions->backBufferFormat;
    pSettings->multiSampleCount        = pBestOptions->multiSampleCount[ 0 ];
    pSettings->multiSampleQuality      = 0;
    pSettings->tripleBuffering         = false; // Default option

    // We found a mode
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findBestFullScreenMode ()
/// <summary>
/// Find the best fullscreen mode, and fill out our D3DSettings structure.
/// </summary>
//-----------------------------------------------------------------------------
cgToDo( "Carbon General", "Revisit this whole thing to allow correct selection of specific outputs per display." )
bool cgDX11Initialize::findBestFullScreenMode( cgDX11Settings & D3DSettings, DXGI_MODE_DESC * pMatchMode, bool bRequireHAL, bool bRequireREF )
{
    DXGI_MODE_DESC            BestDisplayMode, BestOutputMode;
    cgDX11EnumAdapter        *pBestAdapter = CG_NULL;
    cgDX11EnumDeviceOptions  *pBestOptions = CG_NULL;
    cgDX11Settings::Settings *pSettings    = CG_NULL;
    cgInt                     nBestOutput  = -1;

    // Loop through each adapter and try and find
    for ( cgUInt32 i = 0; i < getAdapterCount(); i++ )
    {
        cgDX11EnumAdapter * pAdapter = mAdapters[ i ];
        
        // Loop through each output on the adapter
        for ( size_t j = 0; j < pAdapter->outputs.size(); ++j )
        {
            cgDX11EnumOutput * pOutput = pAdapter->outputs[j];

            // Find a good matching mode for this output on this adapter.
            DXGI_MODE_DESC ClosestOutputMode;
            if ( FAILED( pOutput->output->FindClosestMatchingMode( pMatchMode, &ClosestOutputMode, CG_NULL ) ) )
                continue;

            // Use the first one we find if we don't have one yet,
            // otherwise see if the resolution is better.
            if ( nBestOutput < 0 )
            {
                BestOutputMode = ClosestOutputMode;
                nBestOutput = j;
                pBestAdapter = pAdapter;

            } // End if no best
            else
            {
                // Does this more closesly match the requested resolution?
                if ( (abs((cgInt)ClosestOutputMode.Width - (cgInt)pMatchMode->Width) <
                      abs((cgInt)BestOutputMode.Width - (cgInt)pMatchMode->Width) ) ||
                     (abs((cgInt)ClosestOutputMode.Height - (cgInt)pMatchMode->Height) <
                      abs((cgInt)BestOutputMode.Height - (cgInt)pMatchMode->Height) ) )
                {
                    BestOutputMode = ClosestOutputMode;
                    nBestOutput = j;
                    pBestAdapter = pAdapter;

                } // End if closer?

            } // End if one exists

        } // Next Output

    } // Next adapter

    // If no matching mode was found, we're out of options.
    if ( !pBestAdapter )
        return false;

    // Loop through each option set in the adapter for this output.
    for ( size_t j = 0; j < pBestAdapter->options.size(); ++j )
    {
        cgDX11EnumDeviceOptions * pOptions = pBestAdapter->options[ j ];
        
        // Skip if this is not a fullscreen option
        if ( pOptions->windowed )
            continue;

        // Skip if these options aren't associated with the chosen output.
        if ( pOptions->outputOrdinal != nBestOutput )
            continue;

        // Skip if the driver is not of the required type
        if ( bRequireHAL && pOptions->driverType != D3D_DRIVER_TYPE_HARDWARE )
            continue;
        if ( bRequireREF && pOptions->driverType != D3D_DRIVER_TYPE_REFERENCE )
            continue;

        // Skip if the back buffer format does not match the mode closest to that requested.
        if ( pOptions->backBufferFormat != BestOutputMode.Format )
            continue;

        // If we haven't found a compatible option set yet, or if this set
        // is better (because its HAL / format matches better) then save it.
        if ( pBestOptions == CG_NULL ||
            (pBestOptions->driverType != D3D_DRIVER_TYPE_HARDWARE && pOptions->driverType == D3D_DRIVER_TYPE_HARDWARE ) )
        {
            // Store best so far
            BestDisplayMode = BestOutputMode;
            pBestOptions = pOptions;
            
            // This fullscreen device option looks good?
            if ( pOptions->driverType == D3D_DRIVER_TYPE_HARDWARE )
                break;

        } // End if not a better match
    
    } // Next Option Set

    // If we didn't find anything suitable, we must fail.
    if ( !pBestOptions )
        return false;

    // Fill out passed settings details
    D3DSettings.windowed               = false;
    pSettings                          = D3DSettings.getSettings();
    pSettings->adapterOrdinal          = pBestOptions->adapterOrdinal;
    pSettings->outputOrdinal           = pBestOptions->outputOrdinal;
    pSettings->displayMode             = BestDisplayMode;
    pSettings->driverType              = pBestOptions->driverType;
    pSettings->backBufferFormat        = pBestOptions->backBufferFormat;
    pSettings->multiSampleCount        = pBestOptions->multiSampleCount[ 0 ];
    pSettings->multiSampleQuality      = 0;
    pSettings->tripleBuffering         = false; // Default option

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getDirect3DDevice ()
/// <summary>
/// Return a copy of the Direct3D device pointer (adds ref on that iface)
/// </summary>
//-----------------------------------------------------------------------------
ID3D11Device * cgDX11Initialize::getDirect3DDevice( )
{ 
    // Bail if not created yet
    if ( !mD3DDevice )
        return CG_NULL;

    // AddRef on the device
    mD3DDevice->AddRef();

    // we've duplicated the pointer
    return mD3DDevice; 
}

//-----------------------------------------------------------------------------
//  Name : getDirect3DDeviceContext ()
/// <summary>
/// Return a copy of the Direct3D device context pointer (adds ref on that iface)
/// </summary>
//-----------------------------------------------------------------------------
ID3D11DeviceContext * cgDX11Initialize::getDirect3DDeviceContext( )
{ 
    // Bail if not created yet
    if ( !mD3DDeviceContext )
        return CG_NULL;

    // AddRef on the device
    mD3DDeviceContext->AddRef();

    // we've duplicated the pointer
    return mD3DDeviceContext; 
}

//-----------------------------------------------------------------------------
//  Name : getDirect3DSwapChain ()
/// <summary>
/// Return a copy of the Direct3D swap chain pointer (adds ref on that iface)
/// </summary>
//-----------------------------------------------------------------------------
IDXGISwapChain * cgDX11Initialize::getDirect3DSwapChain( )
{ 
    // Bail if not created yet
    if ( !mD3DSwapChain )
        return CG_NULL;

    // AddRef on the object
    mD3DSwapChain->AddRef();

    // we've duplicated the pointer
    return mD3DSwapChain; 
}

#endif // CGE_DX11_RENDER_SUPPORT