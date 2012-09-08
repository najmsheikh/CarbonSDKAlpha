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
// Name : cgDX9BufferFormatEnum.cpp                                          //
//                                                                           //
// Desc : A relatively simple class that provides simple enumeration and     //
//        and transport of data relating to buffer formats supported by the  //
//        end user hardware (DX9 class).                                     //
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
// cgDX9BufferFormatEnum Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9BufferFormatEnum.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgResourceHandles.h>
#include <System/cgExceptions.h>

//-----------------------------------------------------------------------------
//  Name : cgDX9BufferFormatEnum () (Constructor)
/// <summary>
/// cgDX9BufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX9BufferFormatEnum::cgDX9BufferFormatEnum( ) : cgBufferFormatEnum( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgDX9BufferFormatEnum () (Constructor)
/// <summary>
/// Overloaded cgDX9BufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX9BufferFormatEnum::cgDX9BufferFormatEnum( const cgBufferFormatEnum & Format ) : cgBufferFormatEnum( Format )
{
}

//-----------------------------------------------------------------------------
//  Name : enumerate () (Virtual)
/// <summary>
/// Determine all supported texture formats.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9BufferFormatEnum::enumerate( cgRenderDriver * pDriver )
{
    IDirect3D9       * pD3D = CG_NULL;
    IDirect3DDevice9 * pD3DDevice = CG_NULL;
    FormatEnumMap::iterator Iterator;

    // This only functions with the DX9 class driver.
    cgDX9RenderDriver * pDX9Driver = dynamic_cast<cgDX9RenderDriver*>(pDriver);
    if ( !pDX9Driver )
        return false;

    // Retrieve underlying D3D9 & Device objects to perform format enumeration.
    if ( !(pD3D = pDX9Driver->getD3D()) )
        return false;
    if ( !(pD3DDevice = pDX9Driver->getD3DDevice()) )
    {
        pD3D->Release();
        return false;
    
    } // End if no device

    // Retrieve the device creation parameters so that we can
    // determine the device ordinal, type, etc.
    D3DDEVICE_CREATION_PARAMETERS Params;
    pD3DDevice->GetCreationParameters( &Params );

    // Similarly, retrieve the device's current display mode in
    // order to retrieve the current adapter format.
    D3DDISPLAYMODE Mode;
    pD3DDevice->GetDisplayMode( 0, &Mode );

    // And the device capabilities so we can reject certain
    // test if a feature is not available on a broad scale.
    D3DCAPS9 HardwareCaps;
    pD3DDevice->GetDeviceCaps( &HardwareCaps );

    // Retrieve the hardware type for this driver so that
    // we can perform vendor specific tests.
    cgHardwareType::Base HardwareType = pDriver->getHardwareType();
    
    // 2D Texture formats
    for ( Iterator = mTexture2D.begin(); Iterator != mTexture2D.end(); ++Iterator )
    {
        // Test the format
        cgUInt32 NativeFormat = formatToNative(Iterator->first);
        if ( NativeFormat != D3DFMT_UNKNOWN )
        {
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                     0, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                Iterator->second = cgBufferFormatCaps::CanSample;

            // Hardware supports automatic mip map generation at a broad level?
            if ( (HardwareCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) )
            {
                // Can this specific format be used in conjunction with auto mip map generation?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                         D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanAutoGenMipMaps;
            
            } // End if hardware supports
            
        } // End if known format
        
    } // Next format

    // In D3D9, 1D and 2D formats are the same thing.
    mTexture1D = mTexture2D;

    // Volume (3D) texture formats
    for ( Iterator = mTexture3D.begin(); Iterator != mTexture3D.end(); ++Iterator )
    {
        // Test the format
        cgUInt32 NativeFormat = formatToNative(Iterator->first);
        if ( NativeFormat != D3DFMT_UNKNOWN )
        {
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                     0, D3DRTYPE_VOLUMETEXTURE, (D3DFORMAT)NativeFormat ) ) )
                Iterator->second = cgBufferFormatCaps::CanSample;

            // Hardware supports automatic mip map generation at a broad level?
            if ( (HardwareCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) )
            {
                // Can this specific format be used in conjunction with auto mip map generation?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                         D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_VOLUMETEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if hardware supports
        
        } // End if known format
        
    } // Next format

    // Cubemap texture formats
    for ( Iterator = mTextureCube.begin(); Iterator != mTextureCube.end(); ++Iterator )
    {
        // Test the format
        cgUInt32 NativeFormat = formatToNative(Iterator->first);
        if ( NativeFormat != D3DFMT_UNKNOWN )
        {
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                     0, D3DRTYPE_CUBETEXTURE, (D3DFORMAT)NativeFormat ) ) )
                Iterator->second = cgBufferFormatCaps::CanSample;

            // Hardware supports automatic mip map generation at a broad level?
            if ( (HardwareCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) )
            {
                // Can this specific format be used in conjunction with auto mip map generation?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                         D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_CUBETEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if hardware supports
        
        } // End if known format
        
    } // Next format

    // Render target texture formats
    for ( Iterator = mRenderTarget.begin(); Iterator != mRenderTarget.end(); ++Iterator )
    {
        // Test the format
        cgUInt32 NativeFormat = formatToNative(Iterator->first);
        if ( NativeFormat != D3DFMT_UNKNOWN )
        {
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                     D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                Iterator->second = cgBufferFormatCaps::CanSample | cgBufferFormatCaps::CanWrite;

            // Hardware supports automatic mip map generation at a broad level?
            if ( (HardwareCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) )
            {
                // Can this specific format be used in conjunction with auto mip map generation?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                         D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, 
                                                         (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if hardware supports
            
        } // End if known format
        
    } // Next Format

    // Depth stencil surface formats
    for ( Iterator = mDepthStencil.begin(); Iterator != mDepthStencil.end(); ++Iterator )
    {
        cgUInt32 NativeFormat = formatToNative(Iterator->first);
        if ( NativeFormat != D3DFMT_UNKNOWN )
        {
            if ( HardwareType == cgHardwareType::AMD && (Iterator->first == cgBufferFormat::DF16 || Iterator->first == cgBufferFormat::DF24) )
            {
                // Supports depth sampling?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                         D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanSample | cgBufferFormatCaps::CanWrite | cgBufferFormatCaps::CanGather;

            } // End if ATI DF16 | DF24
            else if ( HardwareType == cgHardwareType::NVIDIA && (Iterator->first == cgBufferFormat::D24_UNorm_S8_UInt || Iterator->first == cgBufferFormat::D24_UNorm_X8_Typeless || Iterator->first == cgBufferFormat::D16 ) )
            {
                // Supports depth sampling?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                         D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanSample | cgBufferFormatCaps::CanWrite | cgBufferFormatCaps::CanCompare;

            } // End if NVIDIA D24S8 | D24X8 | D16
            else if ( HardwareType != cgHardwareType::AMD && (Iterator->first == cgBufferFormat::INTZ || Iterator->first == cgBufferFormat::RAWZ) )
            {
                // Supports depth sampling?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                         D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanSample | cgBufferFormatCaps::CanWrite;

            } // End if !ATI INTZ | RAWZ
            else
            {
                // Supports writing (as regular surface)?
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                         D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanWrite;

                // Supports regular depth sample
                if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format,
                                                         D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
                    Iterator->second |= cgBufferFormatCaps::CanSample;

            } // End if other
        
        } // Next known format
        
    } // Next format

    // Release D3D objects.
    pD3DDevice->Release();
    pD3D->Release();

    // Run bilinear filtering tests.
    return testBilinearSupport( pDX9Driver );
}

//-----------------------------------------------------------------------------
//  Name : estimateBufferSize () (Virtual)
/// <summary>
/// Determines an approximate amount of memory (in bytes) required to
/// store the buffer whose details are defined by the Info parameter.
/// Note : This is only a rough estimate and should not be used as a precise
/// size for buffer allocation.
/// </summary>
//-----------------------------------------------------------------------------
size_t cgDX9BufferFormatEnum::estimateBufferSize( const cgImageInfo & Info ) const
{
    size_t TotalSize = 0;
    
    // Copy specifications directly from the info structure
    cgUInt Width     = Info.width;
    cgUInt Height    = Info.height;
    cgUInt Depth     = Info.depth;
    cgUInt MipLevels = Info.mipLevels;

    // Depth == 1 if anything other than a volume texture
    if ( Info.type != cgBufferType::Texture3D )
        Depth = 1;

    // Compute maximum number of mip levels if 0 is specified
    if ( !MipLevels )
    {
        MipLevels = 1;

        // Loop until smallest dimension is 1
        if ( Info.type == cgBufferType::Texture3D )
        {
            while ( Width > 1 && Height > 1 && Depth > 1 )
            {
                ++MipLevels;
                Width >>= 1;
                Height >>= 1;
                Depth >>= 1;

            } // Next iteration

        } // End if 3D
        else if ( Info.type == cgBufferType::Texture1D )
        {
            while ( Width > 1 )
            {
                ++MipLevels;
                Width >>= 1;

            } // Next iteration

        } // End if 1D
        else
        {
            while ( Width > 1 && Height > 1 )
            {
                ++MipLevels;
                Width >>= 1;
                Height >>= 1;

            } // Next iteration

        } // End if 2D

        // Reset dimensions to top level surface.
        Width = Info.width;
        Height = Info.height;
        Depth = Info.depth;

    } // End if max levels

    // Is this a compressed format?
    if ( formatIsCompressed( Info.format ) )
    {
        // If this is a compressed format, width and height values MUST be multiples of four
        // Round UP width and height to nearest multiple of four
        Width  = (cgUInt32)(ceil( (cgDouble)Width / 4.0 ) * 4.0);
        Height = (cgUInt32)(ceil( (cgDouble)Height / 4.0 ) * 4.0);

        // Compute the number of bytes per compressed block
        size_t BytesPerElement = 0;
        switch ( Info.format )
        {
            case cgBufferFormat::BC1:
            case cgBufferFormat::BC1_SRGB:
            case cgBufferFormat::BC1_Typeless:
            case cgBufferFormat::BC4:
            case cgBufferFormat::BC4_Signed:
            case cgBufferFormat::BC4_Typeless:
                BytesPerElement = 8;
                break;

            case cgBufferFormat::BC2:
            case cgBufferFormat::BC2_SRGB:
            case cgBufferFormat::BC2_Typeless:
            case cgBufferFormat::BC3:
            case cgBufferFormat::BC3_SRGB:
            case cgBufferFormat::BC3_Typeless:
            case cgBufferFormat::BC5:
            case cgBufferFormat::BC5_Signed:
            case cgBufferFormat::BC5_Typeless:
            case cgBufferFormat::BC7:
            case cgBufferFormat::BC7_SRGB:
            case cgBufferFormat::BC7_Typeless:
                BytesPerElement = 16;
                break;
                
        } // End format switch

        // For each mip level
        for ( cgUInt i = 0; i < MipLevels; ++i )
        {
            // Add correct number of bytes
            size_t ElementCount = (Width >> 2) * (Height >> 2) * Depth;
            TotalSize += ElementCount * BytesPerElement;

            // Shift sizes to lower mip level
            if ( Width  > 1 ) Width  >>= 1;
            if ( Height > 1 ) Height >>= 1;
            if ( Depth  > 1 ) Depth  >>= 1;

        } // Next Mip Level

    } // End if format is compressed
    else
    {
        size_t BitsPerPixel = formatBitsPerPixel( Info.format );

        // For each mip level
        for ( cgUInt i = 0; i < MipLevels; ++i )
        {
            // Compute the number of bytes per row
            size_t BytesPerRow = (((BitsPerPixel * Width) + 31) & 0xFFFFFFE0) / 8;

            // Add correct number of bytes for entire mip level
            TotalSize += (Height * Depth) * BytesPerRow;

            // Shift sizes to lower mip level
            if ( Width  > 1 ) Width  >>= 1;
            if ( Height > 1 ) Height >>= 1;
            if ( Depth  > 1 ) Depth  >>= 1;

        } // Next Mip Level

    } // End if format is not compressed

    // Return the final size
    return TotalSize;
}

//-----------------------------------------------------------------------------
//  Name : getBestFormat ()
/// <summary>
/// Retrieve the best format given the required options.
/// Note : We only test for the 'normal' texture formats here as a quick helper
/// function. If you want to determine whether more complicated modes
/// are supported, you can test them directly via GetFormatSupported().
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgDX9BufferFormatEnum::getBestFormat( cgBufferType::Base Type, cgUInt32 nSearchFlags ) const
{
    // Get best format for the target case.
    if ( Type == cgBufferType::Texture1D || Type == cgBufferType::Texture2D || Type == cgBufferType::Texture3D ||
         Type == cgBufferType::TextureCube || Type == cgBufferType::Video || Type == cgBufferType::RenderTarget ||
         Type == cgBufferType::RenderTargetCube )
    {
        // Does the user prefer compressed textures in this case?
        // In D3D9, we will select compressed formats only in the
        // four channel non-floating point cases.
        if ( (nSearchFlags & cgFormatSearchFlags::PreferCompressed) &&
             (nSearchFlags & cgFormatSearchFlags::FourChannels) &&
            !(nSearchFlags & cgFormatSearchFlags::FloatingPoint) )
        {
            // Alpha is required?
            if ( (nSearchFlags & cgFormatSearchFlags::RequireAlpha) )
            {
                if ( isFormatSupported( Type, cgBufferFormat::BC2 ) )
                    return cgBufferFormat::BC2;
                if ( isFormatSupported( Type, cgBufferFormat::BC3 ) )
                    return cgBufferFormat::BC3;
        
            } // End if alpha required
            else
            {
                if ( isFormatSupported( Type, cgBufferFormat::BC1 ) )
                    return cgBufferFormat::BC1;

            } // End if no alpha required
        
        } // End if prefer compressed formats

        // Standard formats, and fallback for compression unsupported case
        bool bAcceptPadding = ((nSearchFlags & cgFormatSearchFlags::AllowPaddingChannels) != 0);
        bool bRequiresAlpha = ((nSearchFlags & cgFormatSearchFlags::RequireAlpha) != 0);
        if ( (nSearchFlags & cgFormatSearchFlags::FloatingPoint) )
        {
            // Floating point formats ONLY!
            bool bAcceptHalf = ((nSearchFlags & cgFormatSearchFlags::HalfPrecisionFloat) != 0);
            bool bAcceptFull = ((nSearchFlags & cgFormatSearchFlags::FullPrecisionFloat) != 0);

            // How many channels?
            if ( (nSearchFlags & cgFormatSearchFlags::FourChannels) )
            {
                if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32B32A32_Float ) )
                    return cgBufferFormat::R32G32B32A32_Float;
                else if ( bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16B16A16_Float ) )
                    return cgBufferFormat::R16G16B16A16_Float;
            
            } // End if FourChannel
            else if ( (nSearchFlags & cgFormatSearchFlags::TwoChannels) )
            {
                if ( !bRequiresAlpha )
                {
                    if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32_Float ) )
                        return cgBufferFormat::R32G32_Float;
                    else if ( bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16_Float ) )
                        return cgBufferFormat::R16G16_Float;
                    else if ( bAcceptPadding && bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16B16A16_Float ) )
                        return cgBufferFormat::R16G16B16A16_Float;
                    else if ( bAcceptPadding && bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32B32A32_Float ) )
                        return cgBufferFormat::R32G32B32A32_Float;
                
                } // End if !bRequiresAlpha
                else
                {
                    if ( bAcceptPadding && bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16B16A16_Float ) )
                        return cgBufferFormat::R16G16B16A16_Float;
                    else if ( bAcceptPadding && bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32B32A32_Float ) )
                        return cgBufferFormat::R32G32B32A32_Float;

                } // End if bRequiresAlpha
            
            } // End if TwoChannel
            else if ( (nSearchFlags & cgFormatSearchFlags::OneChannel ) )
            {
                if ( !bRequiresAlpha )
                {
                    if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32_Float ) )
                        return cgBufferFormat::R32_Float;
                    else if ( bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16_Float ) )
                        return cgBufferFormat::R16_Float;
                    else if ( bAcceptPadding && bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16_Float ) )
                        return cgBufferFormat::R16G16_Float;
                    else if ( bAcceptPadding && bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32_Float ) )
                        return cgBufferFormat::R32G32_Float;
                    else if ( bAcceptPadding && bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16B16A16_Float ) )
                        return cgBufferFormat::R16G16B16A16_Float;
                    else if ( bAcceptPadding && bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32B32A32_Float ) )
                        return cgBufferFormat::R32G32B32A32_Float;
                
                } // End if !bRequiresAlpha
                else
                {
                    if ( bAcceptPadding && bAcceptHalf && isFormatSupported( Type, cgBufferFormat::R16G16B16A16_Float ) )
                        return cgBufferFormat::R16G16B16A16_Float;
                    else if ( bAcceptPadding && bAcceptFull && isFormatSupported( Type, cgBufferFormat::R32G32B32A32_Float ) )
                        return cgBufferFormat::R32G32B32A32_Float;

                } // End if bRequiresAlpha
            
            } // End if OneChannel
        
        } // End if float
        else
        {
            // How many channels?
            if ( (nSearchFlags & cgFormatSearchFlags::FourChannels) )
            {
                if ( !bRequiresAlpha )
                {
                    if ( isFormatSupported( Type, cgBufferFormat::B8G8R8X8 ) )
                        return cgBufferFormat::B8G8R8X8;
                    else if ( isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( isFormatSupported( Type, cgBufferFormat::B5G6R5 ) )
                        return cgBufferFormat::B5G6R5;
                    else if ( isFormatSupported( Type, cgBufferFormat::B5G5R5X1 ) )
                        return cgBufferFormat::B5G5R5X1;
                    else if ( isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;
                
                } // End if !bRequiresAlpha
                else
                {
                    if ( isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;

                } // End if bRequiresAlpha
            
            } // End if FourChannel
            else if ( (nSearchFlags & cgFormatSearchFlags::TwoChannels) )
            {
                if ( !bRequiresAlpha )
                {
                    if ( isFormatSupported( Type, cgBufferFormat::R16G16 ) )
                        return cgBufferFormat::R16G16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8 ) )
                        return cgBufferFormat::B8G8R8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8X8 ) )
                        return cgBufferFormat::B8G8R8X8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G6R5 ) )
                        return cgBufferFormat::B5G6R5;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5X1 ) )
                        return cgBufferFormat::B5G5R5X1;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;
                
                } // End if !bRequiresAlpha
                else
                {
                    if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;

                } // End if bRequiresAlpha
            
            } // End if TwoChannel
            else if ( (nSearchFlags & cgFormatSearchFlags::OneChannel ) )
            {
                if ( !bRequiresAlpha )
                {
                    if ( isFormatSupported( Type, cgBufferFormat::R8 ) )
                        return cgBufferFormat::R8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R16G16 ) )
                        return cgBufferFormat::R16G16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8 ) )
                        return cgBufferFormat::B8G8R8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8X8 ) )
                        return cgBufferFormat::B8G8R8X8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G6R5 ) )
                        return cgBufferFormat::B5G6R5;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5X1 ) )
                        return cgBufferFormat::B5G5R5X1;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;
                
                } // End if !bRequiresAlpha
                else
                {
                    if ( isFormatSupported( Type, cgBufferFormat::A8 ) )
                        return cgBufferFormat::A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R16G16B16A16 ) )
                        return cgBufferFormat::R16G16B16A16;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::R10G10B10A2 ) )
                        return cgBufferFormat::R10G10B10A2;
                    else if ( bAcceptPadding && isFormatSupported( Type, cgBufferFormat::B5G5R5A1 ) )
                        return cgBufferFormat::B5G5R5A1;

                } // End if bRequiresAlpha
            
            } // End if OneChannel
        
        } // End if !float

    } // End if color formats
    else if ( Type == cgBufferType::DepthStencil )
    {
        bool bRequiresStencil = ((nSearchFlags & cgFormatSearchFlags::RequireStencil) != 0);
        if ( (nSearchFlags & cgFormatSearchFlags::FloatingPoint) )
        {
            // Floating point formats ONLY!
            bool bAcceptHalf = ((nSearchFlags & cgFormatSearchFlags::HalfPrecisionFloat) != 0);
            bool bAcceptFull = ((nSearchFlags & cgFormatSearchFlags::FullPrecisionFloat) != 0);
            if ( !bRequiresStencil )
            {
                if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D32_Float ) )
                    return cgBufferFormat::D32_Float;
                else if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt ) )
                    return cgBufferFormat::D24_Float_S8_UInt;
            
            } // End if !bRequiresStencil
            else
            {
                if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt ) )
                    return cgBufferFormat::D24_Float_S8_UInt;

            } // End if bRequiresStencil
            
            // Note: No floating point types have half precision support in D3D9!

        } // End if float
        else
        {
            if ( !bRequiresStencil )
            {
                if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_X8_Typeless ) )
                    return cgBufferFormat::D24_UNorm_X8_Typeless;
                else if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_S8_UInt ) )
                    return cgBufferFormat::D24_UNorm_S8_UInt;
                else if ( isFormatSupported( Type, cgBufferFormat::D16 ) )
                    return cgBufferFormat::D16;
            
            } // End if !bRequiresStencil
            else
            {
                if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_S8_UInt ) )
                    return cgBufferFormat::D24_UNorm_S8_UInt;
                else if ( isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt ) )
                    return cgBufferFormat::D24_Float_S8_UInt;

            } // End if bRequiresStencil

        } // End if !float
        
    } // End if depth formats

    // Unsupported format.
    return cgBufferFormat::Unknown;
}

//-----------------------------------------------------------------------------
//  Name : formatToNative () (Static)
/// <summary>
/// Convert the specified Carbon engine format to a native DX9 format.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX9BufferFormatEnum::formatToNative( cgBufferFormat::Base Format )
{
    switch ( Format )
    {
        case cgBufferFormat::R32G32B32A32_Float:
            return D3DFMT_A32B32G32R32F;
        case cgBufferFormat::R16G16B16A16_Float:
            return D3DFMT_A16B16G16R16F;
        case cgBufferFormat::R16G16B16A16:
            return D3DFMT_A16B16G16R16;
        case cgBufferFormat::R16G16B16A16_Signed:
            return D3DFMT_Q16W16V16U16;
        case cgBufferFormat::R32G32_Float:
            return D3DFMT_G32R32F;
        case cgBufferFormat::R10G10B10A2:
            return D3DFMT_A2B10G10R10;
        case cgBufferFormat::R8G8B8A8:
            return D3DFMT_A8B8G8R8;
        case cgBufferFormat::R8G8B8A8_SRGB:
            return D3DFMT_A8B8G8R8;
        case cgBufferFormat::R8G8B8A8_Signed:
            return D3DFMT_Q8W8V8U8;
        case cgBufferFormat::R16G16_Float:
            return D3DFMT_G16R16F;
        case cgBufferFormat::R16G16:
            return D3DFMT_G16R16;
        case cgBufferFormat::R16G16_Signed:
            return D3DFMT_V16U16;
        case cgBufferFormat::R32_Float:
            return D3DFMT_R32F;
        case cgBufferFormat::R32_UInt:
            return D3DFMT_INDEX32;
        case cgBufferFormat::R8G8_Signed:
            return D3DFMT_V8U8;
        case cgBufferFormat::R16_Float:
            return D3DFMT_R16F;
        case cgBufferFormat::R16:
            return D3DFMT_L16;
        case cgBufferFormat::R16_UInt:
            return D3DFMT_INDEX16;
        case cgBufferFormat::R8:
            return D3DFMT_L8;
        case cgBufferFormat::A8:
            return D3DFMT_A8;
        case cgBufferFormat::R8G8_B8G8:
            return D3DFMT_G8R8_G8B8;
        case cgBufferFormat::G8R8_G8B8:
            return D3DFMT_R8G8_B8G8;
        case cgBufferFormat::BC1:
            return D3DFMT_DXT1;
        case cgBufferFormat::BC1_SRGB:
            return D3DFMT_DXT1;
        case cgBufferFormat::BC2:
            return D3DFMT_DXT3;
        case cgBufferFormat::BC2_SRGB:
            return D3DFMT_DXT3;
        case cgBufferFormat::BC3:
            return D3DFMT_DXT5;
        case cgBufferFormat::BC3_SRGB:
            return D3DFMT_DXT5;
        case cgBufferFormat::B5G6R5:
            return D3DFMT_R5G6B5;
        case cgBufferFormat::B5G5R5A1:
            return D3DFMT_A1R5G5B5;
        case cgBufferFormat::B8G8R8A8:
            return D3DFMT_A8R8G8B8;
        case cgBufferFormat::B8G8R8X8:
            return D3DFMT_X8R8G8B8;
        case cgBufferFormat::DF16:
            return (D3DFORMAT)cgBufferFormat::DF16; // FourCC
        case cgBufferFormat::DF24:
            return (D3DFORMAT)cgBufferFormat::DF24; // FourCC
        case cgBufferFormat::INTZ:
            return (D3DFORMAT)cgBufferFormat::INTZ; // FourCC
        case cgBufferFormat::RAWZ:
            return (D3DFORMAT)cgBufferFormat::RAWZ; // FourCC
        case cgBufferFormat::B8G8R8:
            return D3DFMT_R8G8B8;
        case cgBufferFormat::B5G5R5X1:
            return D3DFMT_X1R5G5B5;

        // Depth formats too
        case cgBufferFormat::D32_Float:
            return D3DFMT_D32_LOCKABLE;
        case cgBufferFormat::D24_UNorm_S8_UInt:
            return D3DFMT_D24S8;    // Note: This actually maps to an equivalent of D3DFMT_S8D24 but we have no choices :)
        case cgBufferFormat::D24_Float_S8_UInt:
            return D3DFMT_D24FS8;
        case cgBufferFormat::D24_UNorm_X8_Typeless:
            return D3DFMT_D24X8;
        case cgBufferFormat::D16:
            return D3DFMT_D16;
    
    } // End format switch

    // Unsupported / unknown format.
    return D3DFMT_UNKNOWN;
}

//-----------------------------------------------------------------------------
//  Name : formatFromNative () (Static)
/// <summary>
/// Convert the specified native DX9 format to a Carbon engine format.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgDX9BufferFormatEnum::formatFromNative( cgUInt32 Format )
{
    switch ( Format )
    {
        case D3DFMT_A32B32G32R32F:
            return cgBufferFormat::R32G32B32A32_Float;
        case D3DFMT_A16B16G16R16F:
            return cgBufferFormat::R16G16B16A16_Float;
        case D3DFMT_A16B16G16R16:
            return cgBufferFormat::R16G16B16A16;
        case D3DFMT_Q16W16V16U16:
            return cgBufferFormat::R16G16B16A16_Signed;
        case D3DFMT_G32R32F:
            return cgBufferFormat::R32G32_Float;
        case D3DFMT_A2B10G10R10:
            return cgBufferFormat::R10G10B10A2;
        case D3DFMT_A8B8G8R8:
            return cgBufferFormat::R8G8B8A8;
       case D3DFMT_Q8W8V8U8:
            return cgBufferFormat::R8G8B8A8_Signed;
        case D3DFMT_G16R16F:
            return cgBufferFormat::R16G16_Float;
        case D3DFMT_G16R16:
            return cgBufferFormat::R16G16;
        case D3DFMT_V16U16:
            return cgBufferFormat::R16G16_Signed;
        case D3DFMT_R32F:
            return cgBufferFormat::R32_Float;
        case D3DFMT_INDEX32:
            return cgBufferFormat::R32_UInt;
        case D3DFMT_V8U8:
            return cgBufferFormat::R8G8_Signed;
        case D3DFMT_R16F:
            return cgBufferFormat::R16_Float;
        case D3DFMT_L16:
            return cgBufferFormat::R16;
        case D3DFMT_INDEX16:
            return cgBufferFormat::R16_UInt;
        case D3DFMT_L8:
            return cgBufferFormat::R8;
        case D3DFMT_A8:
            return cgBufferFormat::A8;
        case D3DFMT_G8R8_G8B8:
            return cgBufferFormat::R8G8_B8G8;
        case D3DFMT_R8G8_B8G8:
            return cgBufferFormat::G8R8_G8B8;
        case D3DFMT_DXT1:
            return cgBufferFormat::BC1;
        case D3DFMT_DXT2:
            return cgBufferFormat::BC2;
        case D3DFMT_DXT3:
            return cgBufferFormat::BC2;
        case D3DFMT_DXT4:
            return cgBufferFormat::BC3;
        case D3DFMT_DXT5:
            return cgBufferFormat::BC3;
        case D3DFMT_R5G6B5:
            return cgBufferFormat::B5G6R5;
        case D3DFMT_A1R5G5B5:
            return cgBufferFormat::B5G5R5A1;
        case D3DFMT_A8R8G8B8:
            return cgBufferFormat::B8G8R8A8;
        case D3DFMT_X8R8G8B8:
            return cgBufferFormat::B8G8R8X8;
        case cgBufferFormat::DF16:
            return cgBufferFormat::DF16; // FourCC
        case cgBufferFormat::DF24:
            return cgBufferFormat::DF24; // FourCC
        case cgBufferFormat::INTZ:
            return cgBufferFormat::INTZ; // FourCC
        case cgBufferFormat::RAWZ:
            return cgBufferFormat::RAWZ; // FourCC
        case D3DFMT_R8G8B8:
            return cgBufferFormat::B8G8R8;
        case D3DFMT_X1R5G5B5:
            return cgBufferFormat::B5G5R5X1;

        // Depth formats too
        case D3DFMT_D32_LOCKABLE:
            return cgBufferFormat::D32_Float;
        case D3DFMT_D24S8:
            return cgBufferFormat::D24_UNorm_S8_UInt;    // Note: This actually maps to an equivalent of D3DFMT_S8D24 but we have no choices :)
        case D3DFMT_D24FS8:
            return cgBufferFormat::D24_Float_S8_UInt;
        case D3DFMT_D24X8:
            return cgBufferFormat::D24_UNorm_X8_Typeless;
        case D3DFMT_D16:
            return cgBufferFormat::D16;
    
    } // End format switch

    // Unsupported / unknown format.
    return cgBufferFormat::Unknown;
}

//-----------------------------------------------------------------------------
// Name : fillLinearTestTexture () (Callback)
// Desc : Support callback function for populating the textures we are going to 
//        be testing for linear filtering support via testBilinearSupport. 
// Note : The test pattern used is to fill texel 0 with 0.0 and texel 1 with 1.0
//        (the textures are 2x1) and the shader samples at texcoord 0.5.
//-----------------------------------------------------------------------------
void __stdcall cgDX9BufferFormatEnum::fillLinearTestTexture( D3DXVECTOR4 * pOut, const D3DXVECTOR2 * pTexCoord, const D3DXVECTOR2 * pTexelSize, void* pData )
{	
    if( pTexCoord->x < 0.5 )
        *pOut = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
    else
        *pOut = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
}

//-----------------------------------------------------------------------------
// Name : testBilinearSupport () (Protected)
// Desc : Tests all available color formats to determine if bilinear sampling 
//        is supported on the current device and lets the buffer format 
//        enumeration know which formats pass the test (for later querying).
//-----------------------------------------------------------------------------
bool cgDX9BufferFormatEnum::testBilinearSupport( cgDX9RenderDriver * pDriver )
{
    // Supported test formats.
    static const cgBufferFormat::Base TestFormats[] = {
        cgBufferFormat::R32G32B32A32_Typeless, cgBufferFormat::R32G32B32A32_Float, cgBufferFormat::R32G32B32A32_UInt, cgBufferFormat::R32G32B32A32_SInt,
        cgBufferFormat::R32G32B32_Typeless, cgBufferFormat::R32G32B32_Float, cgBufferFormat::R32G32B32_UInt, cgBufferFormat::R32G32B32_SInt, 
        cgBufferFormat::R16G16B16A16_Typeless, cgBufferFormat::R16G16B16A16_Float, cgBufferFormat::R16G16B16A16, cgBufferFormat::R16G16B16A16_UInt,
        cgBufferFormat::R16G16B16A16_Signed, cgBufferFormat::R16G16B16A16_SInt, cgBufferFormat::R32G32_Typeless, cgBufferFormat::R32G32_Float,
        cgBufferFormat::R32G32_UInt, cgBufferFormat::R32G32_SInt, cgBufferFormat::R32G8X24_Typeless, cgBufferFormat::R32_Float_X8X24_Typeless,
        cgBufferFormat::X32_Typeless_G8X24_UInt, cgBufferFormat::R10G10B10A2_Typeless, cgBufferFormat::R10G10B10A2, cgBufferFormat::R10G10B10A2_UInt,
        cgBufferFormat::R11G11B10_Float, cgBufferFormat::R8G8B8A8_Typeless, cgBufferFormat::R8G8B8A8, cgBufferFormat::R8G8B8A8_SRGB,
        cgBufferFormat::R8G8B8A8_UInt, cgBufferFormat::R8G8B8A8_Signed, cgBufferFormat::R8G8B8A8_SInt, cgBufferFormat::R16G16_Typeless,
        cgBufferFormat::R16G16_Float, cgBufferFormat::R16G16, cgBufferFormat::R16G16_UInt, cgBufferFormat::R16G16_Signed,
        cgBufferFormat::R16G16_SInt, cgBufferFormat::R32_Typeless, cgBufferFormat::R32_Float, cgBufferFormat::R32_UInt,
        cgBufferFormat::R32_SInt, cgBufferFormat::R24G8_Typeless, cgBufferFormat::X24_Typeless_G8_UInt, cgBufferFormat::R8G8_Typeless,
        cgBufferFormat::R8G8, cgBufferFormat::R8G8_UInt, cgBufferFormat::R8G8_Signed, cgBufferFormat::R8G8_SInt,
        cgBufferFormat::R16_Typeless, cgBufferFormat::R16_Float, cgBufferFormat::R16, cgBufferFormat::R16_UInt,
        cgBufferFormat::R16_Signed, cgBufferFormat::R16_SInt, cgBufferFormat::R8_Typeless, cgBufferFormat::R8,
        cgBufferFormat::R8_UInt, cgBufferFormat::R8_Signed, cgBufferFormat::R8_SInt, cgBufferFormat::A8,
        cgBufferFormat::R1, cgBufferFormat::R9G9B9E5_SharedExp, cgBufferFormat::R8G8_B8G8, cgBufferFormat::G8R8_G8B8,
        cgBufferFormat::BC1_Typeless, cgBufferFormat::BC1, cgBufferFormat::BC1_SRGB, cgBufferFormat::BC2_Typeless,
        cgBufferFormat::BC2, cgBufferFormat::BC2_SRGB, cgBufferFormat::BC3_Typeless, cgBufferFormat::BC3,
        cgBufferFormat::BC3_SRGB, cgBufferFormat::BC4_Typeless, cgBufferFormat::BC4, cgBufferFormat::BC4_Signed,
        cgBufferFormat::BC5_Typeless, cgBufferFormat::BC5, cgBufferFormat::BC5_Signed, cgBufferFormat::B5G6R5,
        cgBufferFormat::B5G5R5A1, cgBufferFormat::B8G8R8A8, cgBufferFormat::B8G8R8X8, cgBufferFormat::B8G8R8A8_Typeless,
        cgBufferFormat::B8G8R8A8_SRGB, cgBufferFormat::B8G8R8X8_Typeless, cgBufferFormat::B8G8R8X8_SRGB, cgBufferFormat::BC6H_Typeless,
        cgBufferFormat::BC6H_UF16, cgBufferFormat::BC6H_SF16, cgBufferFormat::BC7_Typeless, cgBufferFormat::BC7,
        cgBufferFormat::BC7_SRGB, cgBufferFormat::B8G8R8, cgBufferFormat::B5G5R5X1 };
    static const cgUInt32 TestFormatCount = sizeof(TestFormats) / sizeof(TestFormats[0]);

    // Local Variables
    cgString strSupportedFormats;
    IDirect3DDevice9 * pDevice = CG_NULL;
    IDirect3DSurface9 * pScratchSurface = CG_NULL, * pRenderTarget = CG_NULL, * pReadbackTarget = CG_NULL;
    HRESULT hRet;

    // Get access to required systems.
    cgSurfaceShader * pShader = pDriver->getSystemShader().getResource(true);
    if ( !pShader || !(pDevice = pDriver->getD3DDevice()) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access compatible Direct3D device or its associated system shader during the linear filtering support test.\n") );
        return false;
    
    } // End if no device

    try
    {
        // Create the two pixel shaders we will use during testing (alpha vs. red channels)
        cgScriptArgument::Array psArgs( 1 );
        static const bool bFalse = false, bTrue = true;
        psArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bFalse );
        cgPixelShaderHandle hPSTestRed   = pShader->getPixelShader( _T("testLinearFiltering"), psArgs );
        psArgs[0] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bTrue );
        cgPixelShaderHandle hPSTestAlpha = pShader->getPixelShader( _T("testLinearFiltering"), psArgs );
        if ( !hPSTestRed.isValid() || !hPSTestAlpha.isValid() )
            throw cgExceptions::ResultException( _T("Failed to load or compile the 'testLinearFiltering' pixel shader used for the linear filtering support test."), cgDebugSource() );

        // Create a system memory buffer for filling compressed textures with our test pattern
        if ( FAILED( hRet = pDevice->CreateOffscreenPlainSurface( 2, 1, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pScratchSurface, NULL ) ) )
            throw cgExceptions::ResultException( hRet, _T("Failed to create the source test pattern surface for the linear filtering support test. IDirect3DDevice9::CreateOffscreenplainSurface"), cgDebugSource() );

        // Lock and fill the test pattern buffer
        D3DLOCKED_RECT LockedRect;
        if ( FAILED( hRet = pScratchSurface->LockRect( &LockedRect, NULL, 0 ) ) )
            throw cgExceptions::ResultException( hRet, _T("Failed to lock the source test pattern surface during the linear filtering support test. IDirect3DSurface9::LockRect"), cgDebugSource() );
        ((cgUInt32*)LockedRect.pBits)[0] = 0;
        ((cgUInt32*)LockedRect.pBits)[1] = 0xffffffff;
        pScratchSurface->UnlockRect();

        // Create a 1x1 render target (RGBA8) for primary rendering
        if ( FAILED( hRet = pDevice->CreateRenderTarget( 1, 1, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &pRenderTarget, NULL ) ) )
            throw cgExceptions::ResultException( hRet, _T("Failed to create a render target for the linear filtering support test. IDirect3DDevice9::CreateRenderTarget"), cgDebugSource() );
        
        // Create a 1x1 system memory target (RGBA8) for render target readback to CPU
        if ( FAILED( hRet = pDevice->CreateOffscreenPlainSurface( 1, 1, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pReadbackTarget, NULL ) ) )
            throw cgExceptions::ResultException( hRet, _T("Failed to create a readback surface for the linear filtering support test. IDirect3DDevice9::CreateOffscreenPlainSurface"), cgDebugSource() );
        
        // Begin scene
        if ( SUCCEEDED( pDevice->BeginScene() ) )
        {
            // Create a 1x1 viewport
            D3DVIEWPORT9 Viewport;
            Viewport.X      = 0;
            Viewport.Y      = 0;
            Viewport.Width  = 1;
            Viewport.Height = 1;
            Viewport.MinZ   = 0.0f;
            Viewport.MaxZ   = 1.0f;

            // Build a screen quad for drawing
            cgScreenVertex Quad[4];
            cgFloat fX      = -0.5f;
            cgFloat fY      = -0.5f;
            cgFloat fWidth  = 1.0f;
            cgFloat fHeight = 1.0f;
            cgFloat	fDepth  = 0.5f;
            memset( Quad, 0, sizeof(cgScreenVertex) * 4 ); 
            Quad[0].position  = cgVector4(fX, fY, fDepth, 1.0f);
            Quad[1].position  = cgVector4(fX + fWidth, fY, fDepth, 1.0f);
            Quad[2].position  = cgVector4(fX, fY + fHeight, fDepth, 1.0f);
            Quad[3].position  = cgVector4(fX + fWidth, fY + fHeight, fDepth, 1.0f);

            // Backup the current device values so that we can restore them when we are done.
            D3DVIEWPORT9 OldViewport;
            IDirect3DTexture9 * pOldTexture = CG_NULL;
            IDirect3DSurface9 * pOldRenderTarget = CG_NULL, * pOldDepthStencil = CG_NULL;
            pDevice->GetTexture( 0, (LPDIRECT3DBASETEXTURE9*)&pOldTexture );
            pDevice->GetRenderTarget( 0, &pOldRenderTarget );
            pDevice->GetDepthStencilSurface( &pOldDepthStencil );
            pDevice->GetViewport( &OldViewport );

            // Set the vertex format for quad drawing
            pDriver->pushVertexFormat( cgVertexFormat::formatFromDeclarator( cgScreenVertex::Declarator ) );

            // Get the current values for states we will modify
            cgUInt32 nOldZEnable, nOldZWriteEnable, nOldScissorTest, nOldAlphaBlend, nOldCullMode, nOldMin, nOldMag, nOldMip;
            pDevice->GetRenderState( D3DRS_ZENABLE, &nOldZEnable );
            pDevice->GetRenderState( D3DRS_ZWRITEENABLE, &nOldZWriteEnable );                 
            pDevice->GetRenderState( D3DRS_SCISSORTESTENABLE, &nOldScissorTest );
            pDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &nOldAlphaBlend );
            pDevice->GetRenderState( D3DRS_CULLMODE, &nOldCullMode );
            pDevice->GetSamplerState( 0, D3DSAMP_MAGFILTER, &nOldMin );
            pDevice->GetSamplerState( 0, D3DSAMP_MINFILTER, &nOldMag );
            pDevice->GetSamplerState( 0, D3DSAMP_MIPFILTER, &nOldMip );

            // Set required states for the test (e.g., linear filtering, no depth test, etc.)
            pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );                 
            pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );                 
            pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
            pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
            pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
            pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
            pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
            pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

            // Run the test for each format
            for ( cgUInt32 i = 0; i < TestFormatCount; i++ )
            {
                // Skip unknown formats.
                D3DFORMAT Format = (D3DFORMAT)formatToNative(TestFormats[ i ]);
                if ( Format == D3DFMT_UNKNOWN )
                    continue;

                // Skip formats that can't be sampled.
                cgUInt32 FormatCaps = getFormatCaps( cgBufferType::Texture1D, TestFormats[i] );
                if ( !(FormatCaps & cgBufferFormatCaps::CanSample) )
                    continue;

                // Create a 2x1 texture using the current format (1 mip level)
                IDirect3DTexture9 * pTexture = CG_NULL;
                if ( FAILED( hRet = D3DXCreateTexture ( pDevice, 2, 1, 1, 0, Format, D3DPOOL_MANAGED, &pTexture ) ) )
                {
                    cgAppLog::write( cgAppLog::Warning, _T("Failed to create a texture of format %s during linear filtering support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                    continue;
                
                } // End if failed
                
                // Is this a compressed texture format?
                if ( formatIsCompressed( TestFormats[i] ) )
                {
                    IDirect3DSurface9 * pDstSurface = CG_NULL;
                    if ( FAILED( hRet = pTexture->GetSurfaceLevel( 0, &pDstSurface ) ) )
                    {
                        cgAppLog::write( cgAppLog::Warning, _T("Failed to access top level surface for texture of format %s during linear filtering support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                        pTexture->Release();
                        continue;
                    
                    } // End if no surface

                    // Fill using the compressed texture scratch buffer
                    if ( FAILED( hRet = D3DXLoadSurfaceFromSurface( pDstSurface, NULL, NULL, pScratchSurface, NULL, NULL, D3DX_FILTER_POINT, 0 ) ) )
                    {
                        cgAppLog::write( cgAppLog::Warning, _T("Failed to populate texture of format %s during linear filtering support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                        pDstSurface->Release();
                        pTexture->Release();
                        continue;
                    
                    } // End if failed

                    // Release the surface
                    pDstSurface->Release();
                
                } // End if compressed
                else
                {
                    // Fill the texture with our testing data using D3DXFillTexture
                    if ( FAILED( D3DXFillTexture( pTexture, fillLinearTestTexture, NULL ) ) )
                    {
                        cgAppLog::write( cgAppLog::Warning, _T("Unable to fill texture of format %s during linear filtering support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                        pTexture->Release();
                        continue;
                    
                    } // End if failed

                } // End if not compressed

                // Set the texture and sampler state
                pDevice->SetTexture( 0, pTexture );

                // Select the appropriate pixel shader for the test
                if ( !pDriver->pushPixelShader( (Format == cgBufferFormat::A8) ? hPSTestAlpha : hPSTestRed ) )
                {
                    cgAppLog::write( cgAppLog::Warning, _T("Failed to set pixel shader when testing format %s during linear filtering support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                    pTexture->Release();
                    continue;
                
                } // End if failed
                
                // Set the render target and viewport
                pDevice->SetRenderTarget( 0, pRenderTarget );
                pDevice->SetDepthStencilSurface( CG_NULL );
                pDevice->SetViewport( &Viewport );

                // Clear the render target to 0 (i.e., not supported)
                pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0, 1, 0 );

                // Draw a quad to execute the test (ps returns 0 or 1)
                pDriver->drawPrimitiveUP( cgPrimitiveType::TriangleStrip, 2, Quad );

                // Clear the render target
                pDevice->SetRenderTarget( 0, CG_NULL );

                // Release the texture, we are done with it
                pTexture->Release();

                // Restore pixel shader
                pDriver->popPixelShader();

                // Transfer the test results from GPU to CPU
                if ( FAILED( pDevice->GetRenderTargetData( pRenderTarget, pReadbackTarget ) ) )
                {
                    cgAppLog::write( cgAppLog::Warning, _T("Failed to read linear filtering results back from GPU for format %s during linear format support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                    continue;
                
                } // End if failed

                // Lock the scratch surface and check the result
                if ( FAILED( hRet = pReadbackTarget->LockRect( &LockedRect, NULL, 0 ) ) )
                {
                    cgAppLog::write( cgAppLog::Warning, _T("Failed to lock read back target for final testing of texture format %s during linear format support test. Will assume no support.\n"), cgBufferFormatEnum::formatToString( TestFormats[i] ).c_str() );
                    continue;
                
                } // End if failed

                // If final result is > 0, filtering is supported, so add to table(s)
                cgUInt32 nTestResults = *(cgUInt32 *)LockedRect.pBits;
                if ( nTestResults > 0 )
                {
                    FormatEnumMap::iterator it;
                    it = mTexture1D.find( TestFormats[i] );
                    if ( it != mTexture1D.end() )
                        it->second |= cgBufferFormatCaps::CanLinearFilter;
                    it = mTexture2D.find( TestFormats[i] );
                    if ( it != mTexture2D.end() )
                        it->second |= cgBufferFormatCaps::CanLinearFilter;
                    it = mTexture3D.find( TestFormats[i] );
                    if ( it != mTexture3D.end() )
                        it->second |= cgBufferFormatCaps::CanLinearFilter;
                    it = mTextureCube.find( TestFormats[i] );
                    if ( it != mTextureCube.end() )
                        it->second |= cgBufferFormatCaps::CanLinearFilter;
                    it = mRenderTarget.find( TestFormats[i] );
                    if ( it != mRenderTarget.end() )
                        it->second |= cgBufferFormatCaps::CanLinearFilter;
                    
                    // Add to debug output
                    if ( !strSupportedFormats.empty() )
                        strSupportedFormats.append( _T(", ") );
                    strSupportedFormats.append( cgBufferFormatEnum::formatToString( TestFormats[i] ) );
                
                } // End if supported

                // Unlock the scratch target
                pReadbackTarget->UnlockRect();
                
            } // Next format

            // Restore vertex format
            pDriver->popVertexFormat( );

            // Restore targets and viewport
            pDevice->SetTexture( 0, pOldTexture );
            pDevice->SetRenderTarget( 0, pOldRenderTarget );
            pDevice->SetDepthStencilSurface( pOldDepthStencil );
            pDevice->SetViewport( &OldViewport );
            if ( pOldTexture )
                pOldTexture->Release();
            if ( pOldRenderTarget )
                pOldRenderTarget->Release();
            if ( pOldDepthStencil )
                pOldDepthStencil->Release();

            // Restore states
            pDevice->SetRenderState( D3DRS_ZENABLE, nOldZEnable );                 
            pDevice->SetRenderState( D3DRS_ZWRITEENABLE, nOldZWriteEnable );                 
            pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, nOldScissorTest );
            pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, nOldAlphaBlend );
            pDevice->SetRenderState( D3DRS_CULLMODE, nOldCullMode );
            pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, nOldMin );
            pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, nOldMag );
            pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, nOldMip );

            // End scene
            pDevice->EndScene();

        } // End begin scene

        // Clean up what we need to
        if ( pDevice )
            pDevice->Release();
        if ( pScratchSurface )
            pScratchSurface->Release();
        if ( pRenderTarget )
            pRenderTarget->Release();
        if ( pReadbackTarget )
            pReadbackTarget->Release();

    } // End try
    catch ( const cgExceptions::ResultException & e )
    {
        // Clean up what we need to
        if ( pDevice )
            pDevice->Release();
        if ( pScratchSurface )
            pScratchSurface->Release();
        if ( pRenderTarget )
            pRenderTarget->Release();
        if ( pReadbackTarget )
            pReadbackTarget->Release();

        // Fail.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch
    
    // Success
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Hardware supports linear texture filtering with the following formats: %s.\n"), strSupportedFormats.c_str() );
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT