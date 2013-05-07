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

            // Does this format support filtering other than point?
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                     D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
            {
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMagnify;
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMinify;
            
            } // End if format supports filtering
            
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

            // Does this format support filtering other than point?
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                     D3DUSAGE_QUERY_FILTER, D3DRTYPE_VOLUMETEXTURE, (D3DFORMAT)NativeFormat ) ) )
            {
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMagnify;
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMinify;

            } // End if format supports filtering
        
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

            // Does this format support filtering other than point?
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                     D3DUSAGE_QUERY_FILTER, D3DRTYPE_CUBETEXTURE, (D3DFORMAT)NativeFormat ) ) )
            {
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMagnify;
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMinify;

            } // End if format supports filtering
        
        } // End if known format
        
    } // Next format

    // Render target texture formats
    cgString strLinearSupporting;
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

            // Does this format support filtering other than point?
            if ( SUCCEEDED( pD3D->CheckDeviceFormat( Params.AdapterOrdinal, Params.DeviceType, Mode.Format, 
                                                     D3DUSAGE_QUERY_FILTER | D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, (D3DFORMAT)NativeFormat ) ) )
            {
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMagnify;
                if ( (HardwareCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) )
                    Iterator->second |= cgBufferFormatCaps::CanLinearMinify;

            } // End if format supports filtering

            // Add to debug output if linear is supported.
            if ( Iterator->second & cgBufferFormatCaps::CanLinearFilter )
            {
                if ( !strLinearSupporting.empty() )
                    strLinearSupporting.append( _T(", ") );
                strLinearSupporting.append( cgBufferFormatEnum::formatToString( Iterator->first ) );
            
            } // End if can filter
            
        } // End if known format
        
    } // Next Format

    // Depth stencil surface formats
    // ToDo: Test linear sampling support for depth surfaces.
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

    // Output debug info regarding formats thats upport linear filter.
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Hardware supports linear texture filtering for render targets with the following formats: %s.\n"), strLinearSupporting.c_str() );

    // Release D3D objects.
    pD3DDevice->Release();
    pD3D->Release();

    // Success!
    return true;
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
                if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D32_Float, 0 ) )
                    return cgBufferFormat::D32_Float;
                else if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt, 0 ) )
                    return cgBufferFormat::D24_Float_S8_UInt;
            
            } // End if !bRequiresStencil
            else
            {
                if ( bAcceptFull && isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt, 0 ) )
                    return cgBufferFormat::D24_Float_S8_UInt;

            } // End if bRequiresStencil
            
            // Note: No floating point types have half precision support in D3D9!

        } // End if float
        else
        {
            if ( !bRequiresStencil )
            {
                if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_X8_Typeless, 0 ) )
                    return cgBufferFormat::D24_UNorm_X8_Typeless;
                else if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_S8_UInt, 0 ) )
                    return cgBufferFormat::D24_UNorm_S8_UInt;
                else if ( isFormatSupported( Type, cgBufferFormat::D16, 0 ) )
                    return cgBufferFormat::D16;
            
            } // End if !bRequiresStencil
            else
            {
                if ( isFormatSupported( Type, cgBufferFormat::D24_UNorm_S8_UInt, 0 ) )
                    return cgBufferFormat::D24_UNorm_S8_UInt;
                else if ( isFormatSupported( Type, cgBufferFormat::D24_Float_S8_UInt, 0 ) )
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

#endif // CGE_DX9_RENDER_SUPPORT