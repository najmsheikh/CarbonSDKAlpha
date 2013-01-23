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
// Name : cgDX9RenderingCapabilities.cpp                                     //
//                                                                           //
// Desc : DX9 implementation of interface through which rendering            //
//        capabilities can be queried. Tested capabilities include supported //
//        buffer formats, shader model support, format filtering support     //
//        and so on.                                                         //
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
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9RenderingCapabilities Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Platform/cgDX9RenderingCapabilities.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Rendering/Platform/cgDX9Initialize.h>
#include <Resources/Platform/cgDX9BufferFormatEnum.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9RenderingCapabilities Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9RenderingCapabilities () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderingCapabilities::cgDX9RenderingCapabilities( cgRenderDriver * pDriver ) : cgRenderingCapabilities( pDriver )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9RenderingCapabilities () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderingCapabilities::~cgDX9RenderingCapabilities( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9RenderingCapabilities::dispose( bool bDisposeBase )
{
    // Release allocated memory
    mDisplayModes.clear();

    // Call base class implementation on request
    if ( bDisposeBase )
        cgRenderingCapabilities::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : enumerate ()
/// <summary>
/// Run the capabilities enumeration for the specified device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::enumerate( )
{
    // Call base class implementation first.
    if ( !cgRenderingCapabilities::enumerate( ))
        return false;

    // Get the DX9 driver interface
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mDriver);
    if ( !pDriver )
    {
        cgAppLog::write( cgAppLog::Error, _T("Incorrect use of the DirectX9 capabilities object on a non-DirectX9 render driver.\n"));
        dispose( true );
        return false;

    } // End if failed
    
    // Get the actual D3D device.
    IDirect3DDevice9 * pDevice = pDriver->getD3DDevice();
    if ( !pDevice )
    {
        cgAppLog::write( cgAppLog::Error, _T("Direct3D device must be initialized before capabailities can be determined.\n"));
        dispose( true );
        return false;

    } // End if no device.

    // Get the hardware caps
    pDevice->GetDeviceCaps( &mHardwareCaps );
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : postInit ()
/// <summary>
/// Perform any remaining post render driver initialization tasks.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::postInit( cgDX9Initialize * data, cgUInt32 fullScreenAdapter )
{
    // Populate the list of available full screen display modes for this device.
    const cgDX9EnumAdapter * pAdapter = data->getAdapter( fullScreenAdapter );
    DX9VectorDisplayMode::const_iterator itMode;
    for ( itMode = pAdapter->modes.begin(); itMode != pAdapter->modes.end(); ++itMode )
    {
        cgDisplayMode mode;
        mode.width          = itMode->Width;
        mode.height         = itMode->Height;
        mode.refreshRate    = (cgDouble)itMode->RefreshRate;
        mode.bitDepth       = cgBufferFormatEnum::formatBitsPerPixel(cgDX9BufferFormatEnum::formatFromNative(itMode->Format));
        mDisplayModes.push_back( mode );

    } // Next mode

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMaxBlendTransforms() (Virtual)
/// <summary>
/// Retrieve maximum number of vertex blending transformation matrices
/// supported by this render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX9RenderingCapabilities::getMaxBlendTransforms( ) const
{
    return (mDriver->getConfig().useVTFBlending) ? MaxVBTSlotsVTF : MaxVBTSlotsCB;
}

//-----------------------------------------------------------------------------
//  Name : getMaxAnisotropySamples() (Virtual)
/// <summary>
/// Retrieve maximum number of anisotropic samples that can be taken during rendering.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX9RenderingCapabilities::getMaxAnisotropySamples( ) const
{
    cgToDo( "Carbon General", "Properly support Anisotropic Sampling." )
    return mHardwareCaps.MaxAnisotropy;
}

//-----------------------------------------------------------------------------
//  Name : supportsFastStencilFill() (Virtual)
/// <summary>
/// Hardware supports two sided (automatic CW vs. CCW) stencil fill & test 
/// operations?
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::supportsFastStencilFill() const
{    
    return ((mHardwareCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) != 0);
}

//-----------------------------------------------------------------------------
//  Name : supportsNonPow2Textures() (Virtual)
/// <summary>
/// Supports non power of two textures unconditionally?
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::supportsNonPow2Textures() const
{
    if ( ((mHardwareCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0 ) && 
        ((mHardwareCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) == 0) )
        return false;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsShaderModel () (Virtual)
/// <summary>
/// Determine if the specified shader model is supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::supportsShaderModel( cgShaderModel::Base Model ) const
{
    // Supported?
    switch( Model )
    {
        case cgShaderModel::SM_2_0:
            return ( mHardwareCaps.PixelShaderVersion >= D3DPS_VERSION(2,0) );

        case cgShaderModel::SM_2_a:
            return ( mDriver->getHardwareType() == cgHardwareType::AMD && (mHardwareCaps.PixelShaderVersion > D3DPS_VERSION(2,0)) );

        case cgShaderModel::SM_2_b:
            return ( mDriver->getHardwareType() == cgHardwareType::NVIDIA && (mHardwareCaps.PixelShaderVersion > D3DPS_VERSION(2,0)) );

        case cgShaderModel::SM_3_0:
            return ( mHardwareCaps.PixelShaderVersion >= D3DPS_VERSION(3,0) ); break;

    } // End Switch Model

    // Unsupported shader model
    return false;
}

//-----------------------------------------------------------------------------
//  Name : supportsDepthStencilReading () (Virtual)
/// <summary>
/// Determine if the hardware supports depth-stencil buffer reading. For DX9,
/// this will only be true if the hardware supports the INTZ format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::supportsDepthStencilReading ( ) const
{
	return mBufferFormats->isFormatSupported( cgBufferType::DepthStencil, cgBufferFormat::INTZ, cgBufferFormatCaps::CanSample );
}

//-----------------------------------------------------------------------------
//  Name : getDisplayModes () (Virtual)
/// <summary>
/// Retrieve a list of all full screen display modes supported by this
/// device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderingCapabilities::getDisplayModes( cgDisplayMode::Array & modes ) const
{
    modes = mDisplayModes;
    return true;
}


#endif // CGE_DX9_RENDER_SUPPORT