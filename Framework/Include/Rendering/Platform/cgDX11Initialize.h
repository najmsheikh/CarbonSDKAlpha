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
// Name : cgDX11Initialize.h                                                 //
//                                                                           //
// Desc : Provides support for rapidly enumerating hardware support and      //
//        initializing Direct3D 11 accordingly.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11INITIALIZE_H_ )
#define _CGE_CGDX11INITIALIZE_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11Initialize Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <DXGI.h>
#include <D3D11.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgDX11EnumDeviceOptions;
class cgDX11EnumOutput;
class cgDX11EnumDevice;
class cgDX11EnumAdapter;

//-----------------------------------------------------------------------------
// STL Vector Typedefs for Easy Access
//-----------------------------------------------------------------------------
typedef std::vector<cgUInt32>                 DX11VectorUInt32;
typedef std::vector<DXGI_MODE_DESC>           DX11VectorDisplayMode;
typedef std::vector<cgDX11EnumDeviceOptions*> DX11VectorDeviceOptions;
typedef std::vector<cgDX11EnumOutput*>        DX11VectorOutput;
typedef std::vector<cgDX11EnumDevice*>        DX11VectorDevice;
typedef std::vector<cgDX11EnumAdapter*>       DX11VectorAdapter;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11EnumDeviceOptions (Support Class)
/// <summary>
/// Stores the various device options available for any device.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11EnumDeviceOptions
{
public:
    cgUInt32                adapterOrdinal;
    cgUInt32                outputOrdinal;
    D3D_DRIVER_TYPE         driverType;
    DXGI_FORMAT             backBufferFormat;
    bool                    windowed;
    DX11VectorUInt32        multiSampleCount;
    DX11VectorUInt32        multiSampleQualityLevels;
};

//-----------------------------------------------------------------------------
//  Name : cgDX11EnumOutput (Support Class)
/// <summary>
/// Stores the information about one of the available adapter outputs.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11EnumOutput
{
public:
    cgDX11EnumOutput() : output(CG_NULL) {}
    ~cgDX11EnumOutput();

    cgUInt32                ordinal;
    DXGI_OUTPUT_DESC        details;
    IDXGIOutput           * output;
    DX11VectorDisplayMode   modes;
};

//-----------------------------------------------------------------------------
//  Name : cgDX11EnumDevice (Support Class)
/// <summary>
/// Stores the various capabilities etc for an individual device type.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11EnumDevice
{
public:
    D3D_DRIVER_TYPE         driverType;
    D3D_FEATURE_LEVEL       maximumFeatureLevel;
};

//-----------------------------------------------------------------------------
//  Name : cgDX11EnumAdapter (Support Class)
/// <summary>
/// Stores the various adapter modes for a single enumerated adapter.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11EnumAdapter
{
public:
    cgDX11EnumAdapter() : adapter(CG_NULL) {}
    ~cgDX11EnumAdapter();

    cgUInt32                    ordinal;
    DXGI_ADAPTER_DESC1          details;
    IDXGIAdapter1             * adapter;
    DX11VectorDevice            devices;
    DX11VectorOutput            outputs;
    DX11VectorDeviceOptions     options;
};

//-----------------------------------------------------------------------------
//  Name : cgDX11Settings (Support Class)
/// <summary>
/// Allows us to set up the various options we will be using.
/// Note : Also used internally by cgDX11SettingsDlg.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11Settings 
{
public:
    
    struct Settings
    {
        cgUInt32                adapterOrdinal;
        cgUInt32                outputOrdinal;
        DXGI_MODE_DESC          displayMode;
        D3D_DRIVER_TYPE         driverType;
        DXGI_FORMAT             backBufferFormat;
        cgUInt32                multiSampleCount;
        cgUInt32                multiSampleQuality;
        bool                    tripleBuffering;
    };

    bool        windowed;
    Settings    windowedSettings;
    Settings    fullScreenSettings;

    Settings      * getSettings() { return (windowed) ? &windowedSettings : &fullScreenSettings; }
    const Settings* getSettings() const { return (windowed) ? &windowedSettings : &fullScreenSettings; }

};

//-----------------------------------------------------------------------------
//  Name : cgDX11Initialize (Class)
/// <summary>
/// Direct3D Initialization class. Detects supported formats, modes and
/// capabilities, and initializes the devices based on the chosen details
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11Initialize
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
	         cgDX11Initialize();
	virtual ~cgDX11Initialize();

	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    HRESULT                     enumerate               ( );
    
    HRESULT                     createDisplay           ( cgDX11Settings & settings, cgUInt32 flags = 0, HWND focusWindow = CG_NULL, HWND outputWindow = CG_NULL,
                                                          WNDPROC windowProcedure = CG_NULL, LPCTSTR windowTitle = CG_NULL, cgUInt32 width = CW_USEDEFAULT, 
                                                          cgUInt32 height = CW_USEDEFAULT, LPVOID lparam = CG_NULL, bool modifyWindow = true, bool attachPerfHUD = false );

    bool                        findBestWindowedMode    ( cgDX11Settings & settings, const cgRenderDriverConfig & currentConfig, bool requireHAL = false, bool requireREF = false, bool exactMatch = false );
    bool                        findBestFullScreenMode  ( cgDX11Settings & settings, const cgRenderDriverConfig & currentConfig, bool requireHAL = false, bool requireREF = false, bool exactMatch = false );

    DXGI_SWAP_CHAIN_DESC        buildSwapChainParameters( cgDX11Settings & settings, HWND hWnd, cgUInt32 flags = 0  );
    
    cgUInt32                    getAdapterCount         ( ) const           { return (cgUInt32)mAdapters.size(); }
    const cgDX11EnumAdapter   * getAdapter              ( cgUInt32 index )  { return (index > getAdapterCount() ) ? CG_NULL : mAdapters[index]; }
    HWND                        getWindowHandle         ( )                 { return mFocusWnd; }
    ID3D11Device              * getDirect3DDevice       ( );
    ID3D11DeviceContext       * getDirect3DDeviceContext( );
    IDXGISwapChain            * getDirect3DSwapChain    ( );

private:
    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------
    HRESULT             enumerateAdapters               ( );
    HRESULT             enumerateDevices                ( cgDX11EnumAdapter * adapter );
    HRESULT             enumerateOutputs                ( cgDX11EnumAdapter * adapter );
    HRESULT             enumerateDisplayModes           ( IDXGIOutput * outputDXGI, cgDX11EnumOutput * output );
    HRESULT             enumerateDeviceOptions          ( cgDX11EnumAdapter * adapter );
    HRESULT             enumerateMultiSampleTypes       ( cgDX11EnumAdapter * adapter, cgDX11EnumDevice * device, cgDX11EnumDeviceOptions * deviceOptions );

    //-------------------------------------------------------------------------
	// Private Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool        validateDisplayMode             ( const DXGI_MODE_DESC & mode ) { return true; }
    virtual bool        validateDevice                  ( cgDX11EnumAdapter * adapter, D3D_DRIVER_TYPE type, D3D_FEATURE_LEVEL maximumLevel, ID3D11Device * device ) { return true; }
    virtual bool        validateDeviceOptions           ( DXGI_FORMAT backBufferFormat, bool isWindowed ) { return true; }
    virtual bool        validateMultiSampleCount        ( cgUInt32 sampleCount, cgUInt32 qualityLevels ) { return true; }
    
    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    IDXGIFactory1         * mFactory;           // DXGI 1.1 factory object used for enumeration.
	ID3D11Device          * mD3DDevice;         // Primary Direct3D device object.
    ID3D11DeviceContext   * mD3DDeviceContext;  // Created Direct3D device context object.
    IDXGISwapChain        * mD3DSwapChain;      // The created Direct3D swap chain.
    HWND                    mFocusWnd;          // Focus window (automatically created if not supplied).
    DX11VectorAdapter       mAdapters;          // Enumerated Adapters
    
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11INITIALIZE_H_