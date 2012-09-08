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
// Name : cgDX9Initialize.cpp                                                //
//                                                                           //
// Desc : Provides support for rapidly enumerating hardware support and      //
//        initializing Direct3D 9 accordingly.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
//
// Adapters (Array)
//    |
//    |---- Adapter ordinal
//    |
//    |---- Adapter Identifier
//    |
//    |---- Display modes (Array)
//    |           |
//    |           |------- Resolution
//    |           |
//    |           |------- Format / ColorDepth
//    |           |
//    |           |------- Refresh Rate
//    |           
//    |           
//    |---- Device Types (Array)
//                |
//                |------- Device Type - HAL / SW / REF
//                |
//                |------- Device Capabilities
//                |
//                |------- Device Options (Array)
//                               |
//                               |------- Adapter Format
//                               |
//                               |------- Back Buffer Format
//                               |
//                               |------- Is Windowed Mode
//                               |
//                               |------- Depth / Stencil Formats (Array)
//                               |
//                               |------- Multi-Sample Types (Array)   <--\
//                               |                                         > Linked
//                               |------- Multi-Sample Quality (Array) <--/
//                               |
//                               |------- Vertex Processing Types (Array)
//                               |
//                               |------- Presentation Intervals (Array)
//           
//-----------------------------------------------------------------------------

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
// cgDX9Initialize Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX9Initialize.h>
#include <iostream>
#include <dxerr.h>

//-----------------------------------------------------------------------------
// cgDX9Initialize Specific Constants
//-----------------------------------------------------------------------------
const cgUInt32              ValidAdapterFormatCount = 3;
const cgUInt32              BackBufferFormatCount   = 11;
const cgUInt32              DeviceTypeCount         = 3;
const cgUInt32              MultiSampleTypeCount    = 17;
const cgUInt32              DepthStencilFormatCount = 6;
const cgUInt32              PresentIntervalCount    = 6;

const D3DFORMAT             ValidAdapterFormats[3]  = { D3DFMT_X8R8G8B8, D3DFMT_X1R5G5B5, D3DFMT_R5G6B5 };
const D3DDEVTYPE            DeviceTypes[3]          = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };

const D3DFORMAT             BackBufferFormats[11]   = { D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, D3DFMT_R8G8B8,
                                                        D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5,
                                                        D3DFMT_R3G3B2, D3DFMT_A8R3G3B2, D3DFMT_X4R4G4B4, 
                                                        D3DFMT_A4R4G4B4, D3DFMT_A2B10G10R10 };

const D3DMULTISAMPLE_TYPE   MultiSampleTypes[17]    = { D3DMULTISAMPLE_NONE      , D3DMULTISAMPLE_NONMASKABLE,
                                                        D3DMULTISAMPLE_2_SAMPLES , D3DMULTISAMPLE_3_SAMPLES,
                                                        D3DMULTISAMPLE_4_SAMPLES , D3DMULTISAMPLE_5_SAMPLES,
                                                        D3DMULTISAMPLE_6_SAMPLES , D3DMULTISAMPLE_7_SAMPLES,
                                                        D3DMULTISAMPLE_8_SAMPLES , D3DMULTISAMPLE_9_SAMPLES,
                                                        D3DMULTISAMPLE_10_SAMPLES, D3DMULTISAMPLE_11_SAMPLES,
                                                        D3DMULTISAMPLE_12_SAMPLES, D3DMULTISAMPLE_13_SAMPLES,
                                                        D3DMULTISAMPLE_14_SAMPLES, D3DMULTISAMPLE_15_SAMPLES,
                                                        D3DMULTISAMPLE_16_SAMPLES };

const D3DFORMAT             DepthStencilFormats[6]  = { D3DFMT_D24S8, D3DFMT_D32, D3DFMT_D24X4S4, 
                                                        D3DFMT_D24X8, D3DFMT_D16, D3DFMT_D15S1 };

const cgUInt32              PresentIntervals[6]     = { D3DPRESENT_INTERVAL_IMMEDIATE, D3DPRESENT_INTERVAL_DEFAULT,
                                                        D3DPRESENT_INTERVAL_ONE, D3DPRESENT_INTERVAL_TWO,
                                                        D3DPRESENT_INTERVAL_THREE, D3DPRESENT_INTERVAL_FOUR };

//-----------------------------------------------------------------------------
// Name : ~cgDX9EnumAdapter () (Destructor)
// Desc : cgDX9EnumAdapter Class Destructor
//----------------------------------------------------------------------------
cgDX9EnumAdapter::~cgDX9EnumAdapter()
{
    // Release all devices
    for ( size_t i = 0; i < devices.size(); i++ )
    {
        // Release Device
        if ( devices[i] )
            delete devices[i];

    } // Next options

    // Clear Vectors
    modes.clear();
    devices.clear();
}

//-----------------------------------------------------------------------------
// Name : ~cgDX9EnumDevice () (Destructor)
// Desc : cgDX9EnumDevice Class Destructor
//----------------------------------------------------------------------------
cgDX9EnumDevice::~cgDX9EnumDevice()
{
    for ( size_t i = 0; i < options.size(); i++ )
    {
        // Release options
        if ( options[i] )
            delete options[i];

    } // Next options

    // Clear Vectors
    options.clear();
}

//-----------------------------------------------------------------------------
// Name : ~cgDX9EnumDeviceOptions () (Destructor)
// Desc : cgDX9EnumDeviceOptions Class Destructor
//----------------------------------------------------------------------------
cgDX9EnumDeviceOptions::~cgDX9EnumDeviceOptions()
{
    // Clear Vectors
    depthFormats.clear();
    multiSampleTypes.clear();
    presentIntervals.clear();
    vertexProcessingTypes.clear();
}

//-----------------------------------------------------------------------------
//  Name : cgDX9Initialize () (Constructor)
/// <summary>
/// cgDX9Initialize Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX9Initialize::cgDX9Initialize()
{
	// Reset / Clear all required values
    mD3D          = CG_NULL;
    mD3DDevice    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9Initialize () (Destructor)
/// <summary>
/// cgDX9Initialize Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX9Initialize::~cgDX9Initialize()
{
	if ( mD3DDevice ) mD3DDevice->Release();
    mD3DDevice = CG_NULL;

    // Clean up D3D Objects
    if ( mD3D ) mD3D->Release();
    mD3D = CG_NULL;

    // Release all enumerated adapters
    for ( size_t i = 0; i < mAdapters.size(); i++ )
    {
        // Release Adapter
        if ( mAdapters[i] )
            delete mAdapters[i];

    } // Next options

    // Clear Vectors
    mAdapters.clear();
}

//-----------------------------------------------------------------------------
//  Name : enumerate ()
/// <summary>
/// This function must be called first to enumerate all available 
/// devices, adapters, modes and formats, prior to initialization.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerate( IDirect3D9 * pD3D )
{
    HRESULT hRet;
 
    // Store the D3D Object
    mD3D = pD3D;
    if ( !mD3D ) return E_FAIL;

    // AddRef on the D3D9 Object so we can auto cleanup
    mD3D->AddRef();

    // Enumerate the adapters
    if ( FAILED( hRet = enumerateAdapters() ) ) return hRet;

    // Success!
    return S_OK;
}

//-----------------------------------------------------------------------------
//  Name : createDisplay ()
/// <summary>
/// Creates the display devices ready for rendering.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::createDisplay( cgDX9Settings& D3DSettings, cgUInt32 Flags, HWND hFocusWnd, HWND hOutputWnd, WNDPROC pWndProc, LPCTSTR Title, cgUInt32 Width, cgUInt32 Height, LPVOID lParam, bool bModifyWindow, bool bDebugVS, bool bDebugPS, bool bPerfHUDAttach, bool bCreateDepthBuffer )
{
    cgUInt32                 CreateFlags = 0;
    cgDX9Settings::Settings *pSettings   = D3DSettings.getSettings();
    
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
        if (!mFocusWnd)
            return E_FAIL;

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

    // Build our present parameters
    D3DPRESENT_PARAMETERS d3dpp = buildPresentParameters( D3DSettings, 0, bCreateDepthBuffer );
    if ( !hOutputWnd )
        hOutputWnd = mFocusWnd;
    d3dpp.hDeviceWindow = hOutputWnd;
    
    // Build our creation flags
    CreateFlags = Flags;
    if ( pSettings->vertexProcessingType == PURE_HARDWARE_VP )
        CreateFlags |= D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if ( pSettings->vertexProcessingType == HARDWARE_VP )
        CreateFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else if ( pSettings->vertexProcessingType == MIXED_VP )
        CreateFlags |= D3DCREATE_MIXED_VERTEXPROCESSING;
    else if ( pSettings->vertexProcessingType == SOFTWARE_VP )
        CreateFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Shader debugging flags

	/*Note: Legacy debugging (pre-Pix). No longer necessary.
    // To debug vertex shaders, we want software processing
	if ( bDebugVS )
	{
		if( pSettings->deviceType != D3DDEVTYPE_REF )
		{
			CreateFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
			CreateFlags &= ~D3DCREATE_PUREDEVICE;                            
			CreateFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;                            
			CreateFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}
	}

	// To debug pixel shaders, we need a ref device
	if ( bDebugPS ) pSettings->deviceType = D3DDEVTYPE_REF;*/

    // If we are using nVidia's PerfHUD tool to analyze performance (nVidia HW only)
    // setup the device appropriately.
    if ( bPerfHUDAttach == true )
    {
        // Find the PerfHUD adapter
        for ( cgUInt32 Adapter = 0; Adapter < mD3D->GetAdapterCount(); Adapter++ ) 
        { 
            D3DADAPTER_IDENTIFIER9 Identifier; 
            HRESULT Res; 
            Res = mD3D->GetAdapterIdentifier( Adapter, 0, &Identifier ); 
            if ( strstr( Identifier.Description, "PerfHUD" ) != 0 )
            {
                pSettings->adapterOrdinal = Adapter; 
                pSettings->deviceType     = D3DDEVTYPE_REF; 
                std::cout << "PerfHUD adapter = " << Adapter << std::endl;
                break; 

            } // End if PerfHUD
        
        } // Next Adapter

    } // End if bPerfHUDAttach

    // Create the device
    mD3DDevice = CG_NULL;
    HRESULT hRet = mD3D->CreateDevice( pSettings->adapterOrdinal, pSettings->deviceType,
                                       mFocusWnd, CreateFlags, &d3dpp, &mD3DDevice );
    // Did the creation fail ?
    if ( FAILED( hRet ) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T( "Failed to create D3D device: %s\n" ), DXGetErrorString( hRet ) );

        if ( mD3DDevice ) mD3DDevice->Release();
        mD3DDevice = CG_NULL;
        return hRet;

    } // End if failed

    // Success
    return S_OK;
}

//-----------------------------------------------------------------------------
//  Name : resetDisplay ()
/// <summary>
/// Reset the display device, and optionally the window etc.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::resetDisplay( IDirect3DDevice9 * pD3DDevice, cgDX9Settings& D3DSettings, HWND hFocusWnd, HWND hOutputWnd, bool bCreateDepthBuffer )
{   
    cgDX9Settings::Settings *pSettings = D3DSettings.getSettings();
    D3DPRESENT_PARAMETERS d3dpp = buildPresentParameters( D3DSettings, 0, bCreateDepthBuffer );
    
    // Use focus window as output window if none supplied.
    if ( !hOutputWnd )
        hOutputWnd = hFocusWnd;
    d3dpp.hDeviceWindow = hOutputWnd;

    // Validate Requirements
    if ( !pD3DDevice )
        return S_OK;

    // Update focus window if supplied.
    if ( hFocusWnd )
    {
        // Setup styles based on windowed / fullscreen mode
        if ( !D3DSettings.windowed )
        {
            SetMenu( hFocusWnd, CG_NULL );
            SetWindowLong( hFocusWnd, GWL_STYLE, WS_VISIBLE | WS_POPUP );
            SetWindowPos( hFocusWnd, CG_NULL, 0, 0, pSettings->displayMode.Width, pSettings->displayMode.Height, SWP_NOZORDER );
        
        } // End if Fullscreen
        else
        {
            // Use fullscreen settings as the foundation for the window size
            cgDX9Settings::Settings * pSettings = &D3DSettings.fullScreenSettings;

            // Change back to an overlapped window
            SetWindowLong( hFocusWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW );

            // Adjust width / height to take into account window decoration
            cgUInt32 Width = pSettings->displayMode.Width + GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
            cgUInt32 Height = pSettings->displayMode.Width + (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );

            // Update position
            SetWindowPos( hFocusWnd, HWND_NOTOPMOST, 50, 50, Width, Height, SWP_NOACTIVATE | SWP_SHOWWINDOW );

        } // End if Windowed

    } // End if

    // Reset the device
    HRESULT hRet = pD3DDevice->Reset( &d3dpp );
    if ( FAILED( hRet ) )
        return hRet;

    // Success
    return S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateAdapters () (Private)
/// <summary>
/// Enumerates the individual adapters contained within the user machine.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateAdapters()
{
    HRESULT hRet;

    // Store the number of available adapters
    cgUInt32 nAdapterCount = mD3D->GetAdapterCount();

    // Loop through each adapter
    for ( cgUInt32 i = 0; i < nAdapterCount; i++ )
    {
        cgDX9EnumAdapter * pAdapter = new cgDX9EnumAdapter;

        // Store adapter ordinal
        pAdapter->ordinal = i;

        // Retrieve adapter identifier
        mD3D->GetAdapterIdentifier( i, 0, &pAdapter->identifier );

        // Enumerate all display modes for this adapter
        if ( FAILED( hRet = enumerateDisplayModes( pAdapter ) ) ||
             FAILED( hRet = enumerateDevices( pAdapter ) )) 
        {
            delete pAdapter;
            if ( hRet == E_ABORT ) continue; else return hRet;
        
        } // End if Failed Code

        // Add this adapter the list
        try { mAdapters.push_back( pAdapter ); } catch ( ... )
        {
            delete pAdapter;
            return E_OUTOFMEMORY;

        } // End Try / Catch Block

    } // Next Adapter

    // Success!
    return S_OK;

}

//-----------------------------------------------------------------------------
//  Name : enumerateDisplayModes () (Private)
/// <summary>
/// Enumerates all of the display modes for the adapter specified.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateDisplayModes( cgDX9EnumAdapter * pAdapter )
{
    HRESULT         hRet;
    cgUInt32        i, j;
    D3DDISPLAYMODE  Mode;
   
    // Loop through each valid 'Adapter' format.
    for ( i = 0; i < ValidAdapterFormatCount; i++ )
    {
        // Retrieve the number of valid modes for this format
        cgUInt32 nModeCount = mD3D->GetAdapterModeCount( pAdapter->ordinal, ValidAdapterFormats[i] );
        if ( nModeCount == 0 ) continue;

        // Loop through each display mode for this format
        for ( j = 0; j < nModeCount; j++ )
        {
            // Retrieve the display mode
            hRet = mD3D->EnumAdapterModes( pAdapter->ordinal, ValidAdapterFormats[i], j, &Mode );
            if ( FAILED( hRet ) ) return hRet;

            // Is supported by user ?
            if ( !validateDisplayMode( Mode ) ) continue;

            // Add this mode to the adapter
            try { pAdapter->modes.push_back( Mode ); } catch( ... ) 
            { 
                return E_OUTOFMEMORY;
            } // End Try / Catch block

        } // Next Adapter Mode

    } // Next Adapter Format

    // Success?
    return (pAdapter->modes.size() == 0) ? E_ABORT : S_OK;

}
  
//-----------------------------------------------------------------------------
//  Name : enumerateDevices () (Private)
/// <summary>
/// Enumerates the valid device types for the specified adapter.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateDevices( cgDX9EnumAdapter * pAdapter )
{
    HRESULT  hRet;
    D3DCAPS9 Caps;

    // Loop through each device type (HAL, SW, REF)
    for ( cgUInt32 i = 0; i < DeviceTypeCount; i++ )
    {
        // Retrieve device caps (on failure, device not generally available)
        if ( FAILED( mD3D->GetDeviceCaps( pAdapter->ordinal, DeviceTypes[i], &Caps ) ) ) continue;

        // Supported by user ?
        if ( !validateDevice( pAdapter, DeviceTypes[ i ], Caps ) ) continue;

        // Allocate a new device
        cgDX9EnumDevice * pDevice = new cgDX9EnumDevice;

        // Store device information
        pDevice->deviceType = DeviceTypes[i];
        pDevice->caps       = Caps;

        // Retrieve various init options for this device
        if ( FAILED( hRet = enumerateDeviceOptions( pDevice, pAdapter ) ) ) 
        { 
            delete pDevice; 
            if ( hRet == E_ABORT ) continue; else return hRet; 
    
        } // End if failed to enumerate

        // Add it to our adapter list
        try { pAdapter->devices.push_back( pDevice ); } catch ( ... )
        {
            delete pDevice;
            return E_OUTOFMEMORY;

        } // End Try / Catch Block

    } // Next Device Type

    // Success?
    return (pAdapter->devices.size() == 0) ? E_ABORT : S_OK;

}

//-----------------------------------------------------------------------------
//  Name : enumerateDeviceOptions () (Private)
/// <summary>
/// Enumerates the various initialization options available for the
/// device specified, such as back buffer formats etc.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateDeviceOptions( cgDX9EnumDevice * pDevice, cgDX9EnumAdapter * pAdapter )
{
    HRESULT     hRet;
    bool        Windowed;
    D3DFORMAT   AdapterFormats[ ValidAdapterFormatCount ];
    cgUInt32    AdapterFormatCount = 0;
    D3DFORMAT   AdapterFormat, BackBufferFormat;

    // Build a list of all the formats used by the adapter
    for ( size_t i = 0; i < pAdapter->modes.size(); i++ )
    {
        // Already added to the list ?
        cgUInt32 j;
        for ( j = 0; j < AdapterFormatCount; j++ ) 
            if ( pAdapter->modes[i].Format == AdapterFormats[j] ) break;

        // Add it to the list if not existing.
        if ( j == AdapterFormatCount )
            AdapterFormats[ AdapterFormatCount++ ] = pAdapter->modes[i].Format;

    } // Next Adapter Mode

    // Loop through each adapter format available
    for ( cgUInt32 i = 0; i < AdapterFormatCount; i++ )
    {
        // Store Adapter Format 
        AdapterFormat = AdapterFormats[i];

        // Loop through all valid back buffer formats
        for ( cgUInt32 j = 0; j < BackBufferFormatCount; j++ )
        {
            // Store Back Buffer Format 
            BackBufferFormat = BackBufferFormats[j];

            // Test Windowed / Fullscreen modes
            for ( cgInt k = 0; k < 2; k++ )
            {
                // Select windowed / fullscreen
                Windowed = ( k == 0 ) ? false : true;

                // Skip if this is not a valid device type
                if ( FAILED( mD3D->CheckDeviceType( pAdapter->ordinal, pDevice->deviceType, 
                                                      AdapterFormat, BackBufferFormat, Windowed ) ) ) continue;
                // Allocate a new device options set
                cgDX9EnumDeviceOptions * pDeviceOptions = new cgDX9EnumDeviceOptions;

                // Store device option details
                pDeviceOptions->adapterOrdinal   = pAdapter->ordinal;
                pDeviceOptions->deviceType       = pDevice->deviceType;
                pDeviceOptions->adapterFormat    = AdapterFormat;
                pDeviceOptions->backBufferFormat = BackBufferFormat;
                pDeviceOptions->caps             = pDevice->caps;
                pDeviceOptions->windowed         = Windowed;

                // Is this option set supported by the user ?
                if ( !validateDeviceOptions( BackBufferFormat, Windowed ) )
                {
                    delete pDeviceOptions;
                    continue;
                
                } // End if user-unsupported

                // Enumerate the various options components
                if ( FAILED( hRet = enumerateDepthStencilFormats  ( pDeviceOptions ) ) ||
                     FAILED( hRet = enumerateMultiSampleTypes     ( pDeviceOptions ) ) ||
                     FAILED( hRet = enumerateVertexProcessingTypes( pDeviceOptions ) ) ||
                     FAILED( hRet = enumeratePresentIntervals     ( pDeviceOptions ) ) )
                {
                    // Release our invalid options
                    delete pDeviceOptions;

                    // If returned anything other than abort, this is fatal
                    if ( hRet == E_ABORT ) continue; else return hRet;
            
                } // End if any enumeration failed

                // Add this to our device
                try { pDevice->options.push_back( pDeviceOptions ); } catch ( ... )
                {
                    delete pDeviceOptions;
                    return E_OUTOFMEMORY;

                } // End Try / Catch Block

            } // Next windowed State

        } // Next BackBuffer Format

    } // Next Adapter Format
        
    // Success?
    return (pDevice->options.size() == 0) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateDepthStencilFormats () (Private)
/// <summary>
/// Enumerates all the valid depth / stencil formats for this device set.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateDepthStencilFormats( cgDX9EnumDeviceOptions * pDeviceOptions )
{
    try
    {
        // Loop through each depth stencil format
        for ( cgUInt32 i = 0; i < DepthStencilFormatCount; i++ )
        {
            // Test to see if this is a valid depth surface format
            if ( SUCCEEDED( mD3D->CheckDeviceFormat( pDeviceOptions->adapterOrdinal, pDeviceOptions->deviceType, 
                                                       pDeviceOptions->adapterFormat, D3DUSAGE_DEPTHSTENCIL,
                                                       D3DRTYPE_SURFACE, DepthStencilFormats[ i ] ) ) )
            {
                // Test to see if this is a valid depth / stencil format for this mode
                if ( SUCCEEDED( mD3D->CheckDepthStencilMatch( pDeviceOptions->adapterOrdinal, pDeviceOptions->deviceType, 
                                                                pDeviceOptions->adapterFormat, pDeviceOptions->backBufferFormat,
                                                                DepthStencilFormats[ i ] ) ) )
                {

                    // Is this supported by the user ?
                    if ( validateDepthStencilFormat( DepthStencilFormats[ i ] ) )
                    {
                        // Add this as a valid depthstencil format
                        pDeviceOptions->depthFormats.push_back( DepthStencilFormats[ i ] );

                    } // End if User-Supported

                } // End if valid for this mode

            } // End if valid DepthStencil format

        } // Next DepthStencil Format

    } // End Try Block

    catch ( ... ) { return E_OUTOFMEMORY; }

    // Success ?
    return ( pDeviceOptions->depthFormats.size() == 0 ) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateMultiSampleTypes () (Private)
/// <summary>
/// Enumerates multi-sample types available for this device set.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateMultiSampleTypes( cgDX9EnumDeviceOptions * pDeviceOptions )
{
    try
    {
        cgUInt32 Quality;

        // Loop through each multi-sample type
        for ( cgUInt32 i = 0; i < MultiSampleTypeCount; i++ )
        {
            // Check if this multi-sample type is supported
            if ( SUCCEEDED( mD3D->CheckDeviceMultiSampleType( pDeviceOptions->adapterOrdinal, pDeviceOptions->deviceType,
                                                                pDeviceOptions->backBufferFormat, pDeviceOptions->windowed,
                                                                MultiSampleTypes[ i ], &Quality ) ) )
            {
                // Is this supported by the user ?
                if ( validateMultiSampleType( MultiSampleTypes[ i ] ) )
                {
                    // Supported, add these to our list
                    pDeviceOptions->multiSampleTypes.push_back( MultiSampleTypes[i] );
                    pDeviceOptions->multiSampleQuality.push_back( Quality );

                } // End if User-Supported

            } // End if valid for this mode

        } // Next Sample Type

    } // End try Block

    catch ( ... ) { return E_OUTOFMEMORY; }

    // Success ?
    return ( pDeviceOptions->multiSampleTypes.size() == 0 ) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumerateVertexProcessingTypes () (Private)
/// <summary>
/// Enumerates all the types of vertex processing available.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumerateVertexProcessingTypes( cgDX9EnumDeviceOptions * pDeviceOptions )
{
    try
    {
        // If the device supports Hardware T&L
        if ( pDeviceOptions->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
        {
            // If the device can be created as 'Pure'
            if ( pDeviceOptions->caps.DevCaps & D3DDEVCAPS_PUREDEVICE )
            {
                // Supports Pure hardware device ?
                if ( validateVertexProcessingType( PURE_HARDWARE_VP ) )
                    pDeviceOptions->vertexProcessingTypes.push_back( PURE_HARDWARE_VP );

            } // End if

            // Supports hardware T&L and Mixed by definitiion ?
            if ( validateVertexProcessingType( HARDWARE_VP ) )
                pDeviceOptions->vertexProcessingTypes.push_back( HARDWARE_VP );

            if ( validateVertexProcessingType( MIXED_VP ) )
                pDeviceOptions->vertexProcessingTypes.push_back( MIXED_VP );

        } // End if HW T&L

        // Always supports software
        if ( validateVertexProcessingType( SOFTWARE_VP ) )
            pDeviceOptions->vertexProcessingTypes.push_back( SOFTWARE_VP );

    } // End try Block

    catch ( ... ) { return E_OUTOFMEMORY; }

    // Success ?
    return ( pDeviceOptions->vertexProcessingTypes.size() == 0 ) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : enumeratePresentIntervals () (Private)
/// <summary>
/// Enumerates all the valid present intervals available for this set.
/// </summary>
//-----------------------------------------------------------------------------
HRESULT cgDX9Initialize::enumeratePresentIntervals( cgDX9EnumDeviceOptions * pDeviceOptions )
{
    try
    {
        cgUInt32 Interval;

        // Loop through each presentation interval
        for ( cgUInt32 i = 0; i < PresentIntervalCount; i++ )
        {
            // Store for easy access
            Interval = PresentIntervals[i];

            // If device is windowed, skip anything above ONE
            if ( pDeviceOptions->windowed )
            {
                if ( Interval == D3DPRESENT_INTERVAL_TWO   ||
                     Interval == D3DPRESENT_INTERVAL_THREE ||
                     Interval == D3DPRESENT_INTERVAL_FOUR ) continue;

            } // End if Windowed

            // Supported by the device ?
            if ( pDeviceOptions->caps.PresentationIntervals & Interval )
            {
                if ( validatePresentInterval( Interval ) )
                    pDeviceOptions->presentIntervals.push_back( Interval );

            } // End if Supported

        } // Next Interval Type

    } // End try Block

    catch ( ... ) { return E_OUTOFMEMORY; }

    // Success ?
    return ( pDeviceOptions->presentIntervals.size() == 0 ) ? E_ABORT : S_OK;
}

//-----------------------------------------------------------------------------
//  Name : buildPresentParameters ()
/// <summary>
/// Builds a set of present parameters from the Settings passed.
/// </summary>
//-----------------------------------------------------------------------------
D3DPRESENT_PARAMETERS cgDX9Initialize::buildPresentParameters( cgDX9Settings& D3DSettings, cgUInt32 Flags, bool bCreateDepthBuffer )
{
    D3DPRESENT_PARAMETERS    d3dpp;
    cgDX9Settings::Settings *pSettings = D3DSettings.getSettings();
    
    memset( &d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS) );

    // Fill out our common present parameters
    d3dpp.BackBufferCount           = (pSettings->tripleBuffering == true) ? 2 : 1;
    d3dpp.BackBufferFormat          = pSettings->backBufferFormat;
    d3dpp.Windowed                  = D3DSettings.windowed;
    d3dpp.MultiSampleType           = pSettings->multiSampleType;
    d3dpp.MultiSampleQuality        = pSettings->multiSampleQuality;
    d3dpp.AutoDepthStencilFormat    = pSettings->depthStencilFormat;
    d3dpp.PresentationInterval      = pSettings->presentInterval;
    d3dpp.Flags                     = Flags;
    d3dpp.SwapEffect                = D3DSWAPEFFECT_DISCARD;

    // Automatically create a primary (device) depth buffer?
    if ( bCreateDepthBuffer )
        d3dpp.EnableAutoDepthStencil = true;
    
    // Is this fullscreen ?
    if ( !d3dpp.Windowed )
    {
        d3dpp.FullScreen_RefreshRateInHz = pSettings->displayMode.RefreshRate;
        d3dpp.BackBufferWidth            = pSettings->displayMode.Width;
        d3dpp.BackBufferHeight           = pSettings->displayMode.Height;

    } // End if fullscreen
    
    // Success
    return d3dpp;
}

//-----------------------------------------------------------------------------
//  Name : findBestWindowedMode ()
/// <summary>
/// Find the best windowed mode, and fill out our D3DSettings structure.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9Initialize::findBestWindowedMode( cgDX9Settings & D3DSettings, bool bRequireHAL, bool bRequireREF )
{
    D3DDISPLAYMODE           DisplayMode;
    cgDX9EnumAdapter        *pBestAdapter = CG_NULL;
    cgDX9EnumDevice         *pBestDevice  = CG_NULL;
    cgDX9EnumDeviceOptions  *pBestOptions = CG_NULL;
    cgDX9Settings::Settings *pSettings    = CG_NULL;

    // Retrieve the primary adapters display mode.
    mD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &DisplayMode);

    // Loop through each adapter
    for ( cgUInt32 i = 0; i < getAdapterCount(); i++ )
    {
        cgDX9EnumAdapter * pAdapter = mAdapters[ i ];
        
        // Loop through each device
        for ( size_t j = 0; j < pAdapter->devices.size(); j++ )
        {
            cgDX9EnumDevice * pDevice = pAdapter->devices[ j ];

            // Skip if this is not of the required type
            if ( bRequireHAL && pDevice->deviceType != D3DDEVTYPE_HAL ) continue;
            if ( bRequireREF && pDevice->deviceType != D3DDEVTYPE_REF ) continue;
            
            // Loop through each option set
            for ( size_t k = 0; k < pDevice->options.size(); k++ )
            {
                cgDX9EnumDeviceOptions * pOptions = pDevice->options[ k ];

                // Determine if back buffer format matches adapter 
                bool MatchedBB = (pOptions->backBufferFormat == pOptions->adapterFormat );

                // Skip if this is not windowed, and formats don't match
                if (!pOptions->windowed) continue;
                if ( pOptions->adapterFormat != DisplayMode.Format) continue;

                // If we haven't found a compatible option set yet, or if this set
                // is better (because it's HAL / formats match better) then save it.
                if( pBestOptions == CG_NULL || (pOptions->deviceType == D3DDEVTYPE_HAL && MatchedBB ) ||
                    (pBestOptions->deviceType != D3DDEVTYPE_HAL && pOptions->deviceType == D3DDEVTYPE_HAL) )
                {
                    // Store best so far
                    pBestAdapter = pAdapter;
                    pBestDevice  = pDevice;
                    pBestOptions = pOptions;
                    
                    if ( pOptions->deviceType == D3DDEVTYPE_HAL && MatchedBB )
                    {
                        // This windowed device option looks great -- take it
                        goto EndWindowedDeviceOptionSearch;
                    }
                    
                } // End if not a better match
            
            } // Next Option Set
        
        } // Next Device Type
    
    } // Next Adapter

EndWindowedDeviceOptionSearch:
    
    if ( pBestOptions == CG_NULL ) return false;

    // Fill out passed settings details
    D3DSettings.windowed               = true;
    pSettings                          = D3DSettings.getSettings();
    pSettings->adapterOrdinal          = pBestOptions->adapterOrdinal;
    pSettings->displayMode             = DisplayMode;
    pSettings->deviceType              = pBestOptions->deviceType;
    pSettings->backBufferFormat        = pBestOptions->backBufferFormat;
    pSettings->depthStencilFormat      = pBestOptions->depthFormats[ 0 ];
    pSettings->multiSampleType         = pBestOptions->multiSampleTypes[ 0 ];
    pSettings->multiSampleQuality      = 0;
    pSettings->vertexProcessingType    = pBestOptions->vertexProcessingTypes[ 0 ];
    pSettings->presentInterval         = pBestOptions->presentIntervals[ 0 ];
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
bool cgDX9Initialize::findBestFullScreenMode( cgDX9Settings & D3DSettings, D3DDISPLAYMODE * pMatchMode, bool bRequireHAL, bool bRequireREF )
{
    // For fullscreen, default to first HAL option that supports the current desktop 
    // display mode, or any display mode if HAL is not compatible with the desktop mode, or 
    // non-HAL if no HAL is available
    
    D3DDISPLAYMODE           AdapterDisplayMode;
    D3DDISPLAYMODE           BestAdapterDisplayMode;
    D3DDISPLAYMODE           BestDisplayMode;
    cgDX9EnumAdapter        *pBestAdapter = CG_NULL;
    cgDX9EnumDevice         *pBestDevice  = CG_NULL;
    cgDX9EnumDeviceOptions  *pBestOptions = CG_NULL;
    cgDX9Settings::Settings *pSettings    = CG_NULL;
    
    BestAdapterDisplayMode.Width  = 0;
    BestAdapterDisplayMode.Height = 0;
    BestAdapterDisplayMode.Format = D3DFMT_UNKNOWN;
    BestAdapterDisplayMode.RefreshRate = 0;

    // Loop through each adapter
    for ( cgUInt32 i = 0; i < getAdapterCount(); i++ )
    {
        cgDX9EnumAdapter * pAdapter = mAdapters[ i ];
        
        // Retrieve the desktop display mode
        mD3D->GetAdapterDisplayMode( pAdapter->ordinal, &AdapterDisplayMode );

        // If any settings were passed, overwrite to test for matches
        if ( pMatchMode ) 
        {
            if ( pMatchMode->Width  != 0 ) AdapterDisplayMode.Width  = pMatchMode->Width;
            if ( pMatchMode->Height != 0 ) AdapterDisplayMode.Height = pMatchMode->Height;
            if ( pMatchMode->Format != D3DFMT_UNKNOWN ) AdapterDisplayMode.Format = pMatchMode->Format;
            if ( pMatchMode->RefreshRate != 0 ) AdapterDisplayMode.RefreshRate = pMatchMode->RefreshRate;

        } // End if match mode passed

        // Loop through each device
        for ( size_t j = 0; j < pAdapter->devices.size(); j++ )
        {
            cgDX9EnumDevice * pDevice = pAdapter->devices[ j ];
            
            // Skip if this is not of the required type
            if ( bRequireHAL && pDevice->deviceType != D3DDEVTYPE_HAL ) continue;
            if ( bRequireREF && pDevice->deviceType != D3DDEVTYPE_REF ) continue;
            
            // Loop through each option set
            for ( size_t k = 0; k < pDevice->options.size(); k++ )
            {
                cgDX9EnumDeviceOptions * pOptions = pDevice->options[ k ];

                // Determine if back buffer format matches adapter 
                bool MatchedBB = (pOptions->backBufferFormat == pOptions->adapterFormat );
                bool MatchedDesktop = (pOptions->adapterFormat == AdapterDisplayMode.Format);
                
                // Skip if this is not fullscreen
                if ( pOptions->windowed ) continue;

                // If we haven't found a compatible option set yet, or if this set
                // is better (because it's HAL / formats match better) then save it.
                if ( pBestOptions == CG_NULL ||
                    (pBestOptions->deviceType != D3DDEVTYPE_HAL && pDevice->deviceType == D3DDEVTYPE_HAL ) ||
                    (pOptions->deviceType == D3DDEVTYPE_HAL && pBestOptions->adapterFormat != AdapterDisplayMode.Format && MatchedDesktop ) ||
                    (pOptions->deviceType == D3DDEVTYPE_HAL && MatchedDesktop && MatchedBB) )
                {
                    // Store best so far
                    BestAdapterDisplayMode = AdapterDisplayMode;
                    pBestAdapter = pAdapter;
                    pBestDevice  = pDevice;
                    pBestOptions = pOptions;
                    
                    if ( pOptions->deviceType == D3DDEVTYPE_HAL && MatchedDesktop && MatchedBB )
                    {
                        // This fullscreen device option looks great -- take it
                        goto EndFullscreenDeviceOptionSearch;
                    }

                } // End if not a better match
            
            } // Next Option Set
        
        } // Next Device Type
    
    } // Next Adapter

EndFullscreenDeviceOptionSearch:
    
    if ( pBestOptions == CG_NULL) return false;

    // Need to find a display mode on the best adapter that uses pBestOptions->adapterFormat
    // and is as close to BestAdapterDisplayMode's res as possible
    BestDisplayMode.Width       = 0;
    BestDisplayMode.Height      = 0;
    BestDisplayMode.Format      = D3DFMT_UNKNOWN;
    BestDisplayMode.RefreshRate = 0;

    // Loop through valid display modes
    for( size_t i = 0; i < pBestAdapter->modes.size(); i++ )
    {
        D3DDISPLAYMODE Mode = pBestAdapter->modes[ i ];
        
        // Skip if it doesn't match our best format
        if( Mode.Format != pBestOptions->adapterFormat ) continue;

        // Determine how good a match this is
        if( Mode.Width == BestAdapterDisplayMode.Width &&
            Mode.Height == BestAdapterDisplayMode.Height && 
            Mode.RefreshRate == BestAdapterDisplayMode.RefreshRate )
        {
            // found a perfect match, so stop
            BestDisplayMode = Mode;
            break;

        } // End if Perfect Match
        else if( Mode.Width == BestAdapterDisplayMode.Width &&
                 Mode.Height == BestAdapterDisplayMode.Height && 
                 Mode.RefreshRate > BestDisplayMode.RefreshRate )
        {
            // refresh rate doesn't match, but width/height match, so keep this
            // and keep looking
            BestDisplayMode = Mode;
        }
        else if( Mode.Width == BestAdapterDisplayMode.Width )
        {
            // width matches, so keep this and keep looking
            BestDisplayMode = Mode;
        }
        else if( BestDisplayMode.Width == 0 )
        {
            // we don't have anything better yet, so keep this and keep looking
            BestDisplayMode = Mode;
        
        } // End if 
    
    } // Next Mode

        // Fill out passed settings details
    D3DSettings.windowed               = false;
    pSettings                          = D3DSettings.getSettings();
    pSettings->adapterOrdinal          = pBestOptions->adapterOrdinal;
    pSettings->displayMode             = BestDisplayMode;
    pSettings->deviceType              = pBestOptions->deviceType;
    pSettings->backBufferFormat        = pBestOptions->backBufferFormat;
    pSettings->depthStencilFormat      = pBestOptions->depthFormats[ 0 ];
    pSettings->multiSampleType         = pBestOptions->multiSampleTypes[ 0 ];
    pSettings->multiSampleQuality      = 0;
    pSettings->vertexProcessingType    = pBestOptions->vertexProcessingTypes[ 0 ];
    pSettings->presentInterval         = pBestOptions->presentIntervals[ 0 ];
    pSettings->tripleBuffering         = false; // Default option

    // Success!
    return true;
}


//-----------------------------------------------------------------------------
//  Name : getDirect3DDevice ()
/// <summary>
/// Return a copy of the Direct3DDevice pointer (adds ref on that iface)
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DDevice9 * cgDX9Initialize::getDirect3DDevice( )
{ 
    // Bail if not created yet
    if ( !mD3DDevice )
        return CG_NULL;

    // AddRef on the device
    mD3DDevice->AddRef();

    // we've duplicated the pointer
    return mD3DDevice; 
}

#endif // CGE_DX9_RENDER_SUPPORT