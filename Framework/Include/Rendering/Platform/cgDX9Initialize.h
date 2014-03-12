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
// Name : cgDX9Initialize.h                                                  //
//                                                                           //
// Desc : Provides support for rapidly enumerating hardware support and      //
//        initializing Direct3D 9 accordingly.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9INITIALIZE_H_ )
#define _CGE_CGDX9INITIALIZE_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9Initialize Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <d3d9.h>	// Win8 SDK required
#include <d3dx9.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgDX9EnumDeviceOptions;
class cgDX9EnumDevice;
class cgDX9EnumAdapter;
struct cgRenderDriverConfig;

//-----------------------------------------------------------------------------
// Name: vertexProcessingType (Enum)
// Desc: Enumeration of all possible D3D vertex processing types.
//-----------------------------------------------------------------------------
enum VERTEXPROCESSING_TYPE
{
    SOFTWARE_VP         = 1,        // Software Vertex Processing
    MIXED_VP            = 2,        // Mixed Vertex Processing
    HARDWARE_VP         = 3,        // Hardware Vertex Processing
    PURE_HARDWARE_VP    = 4         // Pure Hardware Vertex Processing
};

//-----------------------------------------------------------------------------
// STL Vector Typedefs for Easy Access
//-----------------------------------------------------------------------------
typedef cgArray<D3DMULTISAMPLE_TYPE>     DX9VectorMSType;
typedef cgArray<D3DFORMAT>               DX9VectorFormat;
typedef cgArray<cgUInt32>                DX9VectorUInt32;
typedef cgArray<VERTEXPROCESSING_TYPE>   DX9VectorVPType;
typedef cgArray<D3DDISPLAYMODE>          DX9VectorDisplayMode;
typedef cgArray<cgDX9EnumDeviceOptions*> DX9VectorDeviceOptions;
typedef cgArray<cgDX9EnumDevice*>        DX9VectorDevice;
typedef cgArray<cgDX9EnumAdapter*>       DX9VectorAdapter;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9EnumDeviceOptions (Support Class)
/// <summary>
/// Stores the various device options available for any device.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX9EnumDeviceOptions
{
public:
    ~cgDX9EnumDeviceOptions();

    cgUInt32                adapterOrdinal;
    D3DDEVTYPE              deviceType;
    D3DCAPS9                caps;
    D3DFORMAT               adapterFormat;
    D3DFORMAT               backBufferFormat;
    bool                    windowed;
    DX9VectorMSType         multiSampleTypes;
    DX9VectorUInt32         multiSampleQuality;
    DX9VectorFormat         depthFormats;
    DX9VectorVPType         vertexProcessingTypes;
    DX9VectorUInt32         presentIntervals;
};

//-----------------------------------------------------------------------------
//  Name : cgDX9EnumDevice (Support Class)
/// <summary>
/// Stores the various capabilities etc for an individual device type.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX9EnumDevice
{
public:
    ~cgDX9EnumDevice();

    D3DDEVTYPE              deviceType;
    D3DCAPS9                caps;
    DX9VectorDeviceOptions  options;
};

//-----------------------------------------------------------------------------
//  Name : cgDX9EnumAdapter (Support Class)
/// <summary>
/// Stores the various adapter modes for a single enumerated adapter.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX9EnumAdapter
{
public:
    ~cgDX9EnumAdapter();

    cgUInt32                ordinal;
    D3DADAPTER_IDENTIFIER9  identifier;
    DX9VectorDisplayMode    modes;
    DX9VectorDevice         devices;
};

//-----------------------------------------------------------------------------
//  Name : cgDX9Settings (Support Class)
/// <summary>
/// Allows us to set up the various options we will be using.
/// Note : Also used internally by cgDX9SettingsDlg.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX9Settings 
{
public:
    
    struct Settings
    {
        cgUInt32                adapterOrdinal;
        D3DDISPLAYMODE          displayMode;
        D3DDEVTYPE              deviceType;
        D3DFORMAT               backBufferFormat;
        D3DFORMAT               depthStencilFormat;
        D3DMULTISAMPLE_TYPE     multiSampleType;
        cgUInt32                multiSampleQuality;
        VERTEXPROCESSING_TYPE   vertexProcessingType;
        cgUInt32                presentInterval;
        bool                    tripleBuffering;
    };

    bool        windowed;
    Settings    windowedSettings;
    Settings    fullScreenSettings;

    Settings      * getSettings() { return (windowed) ? &windowedSettings : &fullScreenSettings; }
    const Settings* getSettings() const { return (windowed) ? &windowedSettings : &fullScreenSettings; }

};

//-----------------------------------------------------------------------------
//  Name : cgDX9Initialize (Class)
/// <summary>
/// Direct3D Initialization class. Detects supported formats, modes and
/// capabilities, and initializes the devices based on the chosen details
/// </summary>
//-----------------------------------------------------------------------------
class cgDX9Initialize
{
public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
	         cgDX9Initialize();
	virtual ~cgDX9Initialize();

	//-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    HRESULT                 enumerate              ( IDirect3D9 * D3D );
    
    HRESULT                 createDisplay          ( cgDX9Settings& settings, cgUInt32 flags = 0, HWND focusWindow = CG_NULL, HWND outputWindow = CG_NULL, 
                                                     WNDPROC windowProcedure = CG_NULL, LPCTSTR windowTitle = CG_NULL, cgUInt32 width = CW_USEDEFAULT, 
                                                     cgUInt32 height = CW_USEDEFAULT, LPVOID lparam = CG_NULL, bool modifyWindow = true, bool vertexShaderDebug = false, 
                                                     bool pixelShaderDebug = false, bool attachPerfHUD = false, bool createDepthBuffer = true );

    HRESULT                 resetDisplay           ( IDirect3DDevice9 * device, cgDX9Settings& settings, HWND focusWindow = CG_NULL, HWND outputWindow = CG_NULL, 
                                                     bool createDepthBuffer = true );
    
    bool                    findBestWindowedMode   ( cgDX9Settings & settings, const cgRenderDriverConfig & currentConfig, bool requireHAL = false, bool requireREF = false, bool exactMatch = false );
    bool                    findBestFullScreenMode ( cgDX9Settings & settings, const cgRenderDriverConfig & currentConfig, bool requireHAL = false, bool requireREF = false, bool exactMatch = false );

    D3DPRESENT_PARAMETERS   buildPresentParameters ( cgDX9Settings& settings, cgUInt32 flags = 0, bool createDepthBuffer = true );
    
    cgUInt32                getAdapterCount         ( ) const { return (cgUInt32)mAdapters.size(); }
    const cgDX9EnumAdapter* getAdapter              ( cgUInt32 index ) { return (index > getAdapterCount() ) ? CG_NULL : mAdapters[index]; }
    HWND                    getWindowHandle         ( ) { return mFocusWnd; }
    const IDirect3D9      * getDirect3D             ( ) { return mD3D; }
    IDirect3DDevice9      * getDirect3DDevice       ( );
    
private:
    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------
    HRESULT             enumerateAdapters               ( );
    HRESULT             enumerateDisplayModes           ( cgDX9EnumAdapter * adapter );
    HRESULT             enumerateDevices                ( cgDX9EnumAdapter * adapter );
    HRESULT             enumerateDeviceOptions          ( cgDX9EnumDevice  * device, cgDX9EnumAdapter * adapter );
    HRESULT             enumerateDepthStencilFormats    ( cgDX9EnumDeviceOptions * deviceOptions );
    HRESULT             enumerateMultiSampleTypes       ( cgDX9EnumDeviceOptions * deviceOptions );
    HRESULT             enumerateVertexProcessingTypes  ( cgDX9EnumDeviceOptions * deviceOptions );
    HRESULT             enumeratePresentIntervals       ( cgDX9EnumDeviceOptions * deviceOptions );

    //-------------------------------------------------------------------------
	// Private Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool        validateDisplayMode          ( const D3DDISPLAYMODE& mode )                           { return true; }
    virtual bool        validateDevice               ( cgDX9EnumAdapter * adapter, const D3DDEVTYPE & type, const D3DCAPS9 & caps ) { return true; }
    virtual bool        validateDeviceOptions        ( const D3DFORMAT & backBufferFormat, bool isWindowed )  { return true; }
    virtual bool        validateDepthStencilFormat   ( const D3DFORMAT & depthStencilFormat )                 { return true; }
    virtual bool        validateMultiSampleType      ( const D3DMULTISAMPLE_TYPE& type )                      { return true; }
    virtual bool        validateVertexProcessingType ( const VERTEXPROCESSING_TYPE& type )                    { return true; }
    virtual bool        validatePresentInterval      ( const cgUInt32& interval )                             { return true; }
    
    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
	IDirect3D9        * mD3D;			// Primary Direct3D Object.
    IDirect3DDevice9  * mD3DDevice;     // Created Direct3D Device.
    HWND                mFocusWnd;      // Focus window (automatically created if not supplied).
    DX9VectorAdapter    mAdapters;      // Enumerated Adapters
    
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9INITIALIZE_H_