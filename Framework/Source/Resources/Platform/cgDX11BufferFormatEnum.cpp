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
// Name : cgDX11BufferFormatEnum.cpp                                         //
//                                                                           //
// Desc : A relatively simple class that provides simple enumeration and     //
//        and transport of data relating to buffer formats supported by the  //
//        end user hardware (DX11 class).                                    //
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
// cgDX11BufferFormatEnum Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11BufferFormatEnum.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <System/cgExceptions.h>
#include <D3D11.h>

//-----------------------------------------------------------------------------
//  Name : cgDX11BufferFormatEnum () (Constructor)
/// <summary>
/// cgDX11BufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX11BufferFormatEnum::cgDX11BufferFormatEnum( ) : cgBufferFormatEnum( )
{
}

//-----------------------------------------------------------------------------
//  Name : cgDX11BufferFormatEnum () (Constructor)
/// <summary>
/// Overloaded cgDX11BufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgDX11BufferFormatEnum::cgDX11BufferFormatEnum( const cgBufferFormatEnum & Format ) : cgBufferFormatEnum( Format )
{
}

//-----------------------------------------------------------------------------
//  Name : enumerate () (Virtual)
/// <summary>
/// Determine all supported texture formats.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11BufferFormatEnum::enumerate( cgRenderDriver * pDriver )
{
    // This only functions with the DX11 class driver.
    cgDX11RenderDriver * pDX11Driver = dynamic_cast<cgDX11RenderDriver*>(pDriver);
    if ( !pDX11Driver )
        return false;

    // Retrieve underlying D3D11 device object to perform format support enumeration.
    ID3D11Device * pDevice = pDX11Driver->getD3DDevice();
    if ( !pDevice )
        return false;

    // 1D Texture formats
    FormatEnumMap::iterator itFormat;
    for ( itFormat = mTexture1D.begin(); itFormat != mTexture1D.end(); ++itFormat )
    {
        // Test the format (must support shader sampling)
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_TEXTURE1D) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) )
            {
                itFormat->second |= cgBufferFormatCaps::CanSample;

                // What else is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if supports 1D sampling
        
        } // End if known format
        
    } // Next format

    // 2D Texture formats
    for ( itFormat = mTexture2D.begin(); itFormat != mTexture2D.end(); ++itFormat )
    {
        // Test the format (must support shader sampling)
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_TEXTURE2D) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) )
            {
                itFormat->second |= cgBufferFormatCaps::CanSample;

                // What else is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if supports 2D sampling
        
        } // End if known format
        
    } // Next format

    // 3D (Volume) texture formats
    for ( itFormat = mTexture3D.begin(); itFormat != mTexture3D.end(); ++itFormat )
    {
        // Test the format (must support shader sampling)
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_TEXTURE3D) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) )
            {
                itFormat->second |= cgBufferFormatCaps::CanSample;

                // What else is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if supports 3D sampling
        
        } // End if known format
        
    } // Next format

    // Cubemap texture formats
    for ( itFormat = mTextureCube.begin(); itFormat != mTextureCube.end(); ++itFormat )
    {
        // Test the format (must support shader sampling)
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_TEXTURECUBE) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) )
            {
                itFormat->second |= cgBufferFormatCaps::CanSample;

                // What else is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;

            } // End if supports cube sampling
        
        } // End if known format
        
    } // Next format

    // Render target texture formats
    for ( itFormat = mRenderTarget.begin(); itFormat != mRenderTarget.end(); ++itFormat )
    {
        // Test the format (must support shader sampling)
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_RENDER_TARGET) )
            {
                itFormat->second |= cgBufferFormatCaps::CanWrite;

                // What is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE )
                    itFormat->second |= cgBufferFormatCaps::CanSample;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON )
                    itFormat->second |= cgBufferFormatCaps::CanCompare;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_GATHER )
                    itFormat->second |= cgBufferFormatCaps::CanGather;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON )
                    itFormat->second |= cgBufferFormatCaps::CanGatherCompare;
            
            } // End if render target supported
        
        } // End if known format
        
    } // Next format

    // Depth stencil surface formats
    for ( itFormat = mDepthStencil.begin(); itFormat != mDepthStencil.end(); ++itFormat )
    {
        // Skip the DX9 FourCC shadow map formats.
        if ( itFormat->first == cgBufferFormat::DF16 || itFormat->first == cgBufferFormat::DF24 || 
			 itFormat->first == cgBufferFormat::INTZ || itFormat->first == cgBufferFormat::RAWZ )
            continue;

        // Test the format
        cgUInt32 NativeFormat = formatToNative(itFormat->first);
        if ( NativeFormat != DXGI_FORMAT_UNKNOWN )
        {
            cgUInt nSupportFlags = 0;
            if ( SUCCEEDED( pDevice->CheckFormatSupport( (DXGI_FORMAT)NativeFormat, &nSupportFlags ) ) &&
                 (nSupportFlags & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) )
            {
                itFormat->second |= cgBufferFormatCaps::CanWrite;

                // What is supported?
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE )
                    itFormat->second |= cgBufferFormatCaps::CanSample;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE_COMPARISON )
                    itFormat->second |= cgBufferFormatCaps::CanCompare;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN )
                    itFormat->second |= cgBufferFormatCaps::CanAutoGenMipMaps;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_GATHER )
                    itFormat->second |= cgBufferFormatCaps::CanGather;
                if ( nSupportFlags & D3D11_FORMAT_SUPPORT_SHADER_GATHER_COMPARISON )
                    itFormat->second |= cgBufferFormatCaps::CanGatherCompare;

            } // End if depth stencil supported
        
        } // End if known format
        
    } // Next format

    // Release D3D objects.
    pDevice->Release();

    // Test for linear filtering support.
    return testBilinearSupport( pDX11Driver );
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
size_t cgDX11BufferFormatEnum::estimateBufferSize( const cgImageInfo & Info ) const
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
            size_t BytesPerRow = (((BitsPerPixel * Width) + 31) * 0xFFFFFFE0) / 8;

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
/// are supported, you can test them directly via isFormatSupported().
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgDX11BufferFormatEnum::getBestFormat( cgBufferType::Base Type, cgUInt32 nSearchFlags ) const
{
    cgToDo( "DX11", "Go back over the list and find good formats for DX11" )

    // Get best format for the target case.
    if ( Type == cgBufferType::Texture1D || Type == cgBufferType::Texture2D || Type == cgBufferType::Texture3D ||
         Type == cgBufferType::TextureCube || Type == cgBufferType::Video || Type == cgBufferType::RenderTarget ||
         Type == cgBufferType::RenderTargetCube )
    {
        cgToDo( "DX11", "Try some of the new compressed formats (two channel, one channel, floating point, etc.)" )

        // Does the user prefer compressed textures in this case?
        // In D3D11, we will select compressed formats only in the
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
                    if ( isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
                        return cgBufferFormat::R8G8B8A8;
                    else if ( isFormatSupported( Type, cgBufferFormat::B8G8R8X8 ) )
                        return cgBufferFormat::B8G8R8X8;
                    else if ( isFormatSupported( Type, cgBufferFormat::B8G8R8A8 ) )
                        return cgBufferFormat::B8G8R8A8;
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
                    if ( isFormatSupported( Type, cgBufferFormat::R8G8B8A8 ) )
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
/// Convert the specified Carbon engine format to a native DX11 format.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDX11BufferFormatEnum::formatToNative( cgBufferFormat::Base Format )
{
    // BC7_SRGB is the last of the official D3D11 formats we support.
    if ( Format > cgBufferFormat::BC7_SRGB )
        return DXGI_FORMAT_UNKNOWN;

    // We can direct cast for the rest.
    return (cgUInt32)Format;
}

//-----------------------------------------------------------------------------
//  Name : formatFromNative () (Static)
/// <summary>
/// Convert the specified native DX11 format to a Carbon engine format.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgDX11BufferFormatEnum::formatFromNative( cgUInt32 Format )
{
    return (cgBufferFormat::Base)Format;
}

//-----------------------------------------------------------------------------
// Name : testBilinearSupport () (Protected)
// Desc : Tests all available color formats to determine if bilinear sampling 
//        is supported on the current device and lets the buffer format 
//        enumeration know which formats pass the test (for later querying).
//-----------------------------------------------------------------------------
bool cgDX11BufferFormatEnum::testBilinearSupport( cgDX11RenderDriver * pDriver )
{
    cgToDo( "DX11", "Need to revisit if it is safe to assume all formats can be linearly sampled." )

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

    // Local variables
    cgString strSupportedFormats;

    try
    {
        // Run the test for each format
        for ( cgUInt32 i = 0; i < TestFormatCount; i++ )
        {
            // Skip unknown formats.
            DXGI_FORMAT Format = (DXGI_FORMAT)formatToNative(TestFormats[ i ]);
            if ( Format == DXGI_FORMAT_UNKNOWN )
                continue;

            // Always assume the format can be linearly sampled if it can be sampled at all.
            FormatEnumMap::iterator it;
            it = mTexture1D.find( TestFormats[i] );
            if ( it != mTexture1D.end() && (it->second & cgBufferFormatCaps::CanSample) )
                it->second |= cgBufferFormatCaps::CanLinearFilter;
            it = mTexture2D.find( TestFormats[i] );
            if ( it != mTexture2D.end() && (it->second & cgBufferFormatCaps::CanSample) )
            {
                it->second |= cgBufferFormatCaps::CanLinearFilter;
                
                // Add to debug output
                if ( !strSupportedFormats.empty() )
                    strSupportedFormats.append( _T(", ") );
                strSupportedFormats.append( cgBufferFormatEnum::formatToString( TestFormats[i] ) );
            
            } // End if can sample
            it = mTexture3D.find( TestFormats[i] );
            if ( it != mTexture3D.end() && (it->second & cgBufferFormatCaps::CanSample) )
                it->second |= cgBufferFormatCaps::CanLinearFilter;
            it = mTextureCube.find( TestFormats[i] );
            if ( it != mTextureCube.end() && (it->second & cgBufferFormatCaps::CanSample) )
                it->second |= cgBufferFormatCaps::CanLinearFilter;
            it = mRenderTarget.find( TestFormats[i] );
            if ( it != mRenderTarget.end() && (it->second & cgBufferFormatCaps::CanSample) )
                it->second |= cgBufferFormatCaps::CanLinearFilter;
            
        } // Next format

    } // End try
    catch ( const cgExceptions::ResultException & e )
    {
        // Fail.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch
    
    // Success
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Hardware supports linear texture filtering with the following formats: %s.\n"), strSupportedFormats.c_str() );
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT