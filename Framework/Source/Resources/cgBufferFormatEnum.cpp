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
// Name : cgBufferFormatEnum.cpp                                             //
//                                                                           //
// Desc : A relatively simple class that provides simple enumeration and     //
//        and transport of data relating to buffer formats supported by the  //
//        end user hardware.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBufferFormatEnum Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgBufferFormatEnum.h>
#include <Resources/Platform/cgDX9BufferFormatEnum.h>
#include <Resources/Platform/cgDX11BufferFormatEnum.h>

//-----------------------------------------------------------------------------
//  Name : cgBufferFormatEnum () (Constructor)
/// <summary>
/// cgBufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormatEnum::cgBufferFormatEnum()
{
    // Initialize the class
    initialize();
}

//-----------------------------------------------------------------------------
//  Name : cgBufferFormatEnum () (Constructor)
/// <summary>
/// Overloaded cgBufferFormatEnum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormatEnum::cgBufferFormatEnum( const cgBufferFormatEnum & Format )
{
    // Copy constructor
    *this = Format;
}

//-----------------------------------------------------------------------------
//  Name : cgBufferFormatEnum () (Destructor)
/// <summary>
/// cgBufferFormatEnum Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormatEnum::~cgBufferFormatEnum()
{
    // Nothing in this implementation
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormatEnum * cgBufferFormatEnum::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9BufferFormatEnum();

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11BufferFormatEnum();

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : operator = () (Operator)
/// <summary>
/// Assignment operator overload.
/// </summary>
//-----------------------------------------------------------------------------
const cgBufferFormatEnum & cgBufferFormatEnum::operator = ( const cgBufferFormatEnum & Format )
{
    // Perform copy of internal data
    mTexture1D             = Format.mTexture1D;
    mTexture2D             = Format.mTexture2D;
    mTexture3D             = Format.mTexture3D;
    mTextureCube           = Format.mTextureCube;
    mRenderTarget          = Format.mRenderTarget;
    mDepthStencil          = Format.mDepthStencil;

    // Return reference to ourselves (i.e. for a = b = c)
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize this class, all modes initially unsupported.
/// </summary>
//-----------------------------------------------------------------------------
void cgBufferFormatEnum::initialize()
{
    // First clear the internal maps if they've already been used.
    mTexture1D.clear();
    mTexture2D.clear();
    mTexture3D.clear();
    mTextureCube.clear();
    mRenderTarget.clear();
    mDepthStencil.clear();

    // Setup each of the map elements
    mTexture1D[ cgBufferFormat::R32G32B32A32_Typeless ]      = 0;
    mTexture1D[ cgBufferFormat::R32G32B32A32_Float ]         = 0;
    mTexture1D[ cgBufferFormat::R32G32B32A32_UInt ]          = 0;
    mTexture1D[ cgBufferFormat::R32G32B32A32_SInt ]          = 0;
    mTexture1D[ cgBufferFormat::R32G32B32_Typeless ]         = 0;
    mTexture1D[ cgBufferFormat::R32G32B32_Float ]            = 0;
    mTexture1D[ cgBufferFormat::R32G32B32_UInt ]             = 0;
    mTexture1D[ cgBufferFormat::R32G32B32_SInt ]             = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16_Typeless ]      = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16_Float ]         = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16 ]               = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16_UInt ]          = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16_Signed ]        = 0;
    mTexture1D[ cgBufferFormat::R16G16B16A16_SInt ]          = 0;
    mTexture1D[ cgBufferFormat::R32G32_Typeless ]            = 0;
    mTexture1D[ cgBufferFormat::R32G32_Float ]               = 0;
    mTexture1D[ cgBufferFormat::R32G32_UInt ]                = 0;
    mTexture1D[ cgBufferFormat::R32G32_SInt ]                = 0;
    mTexture1D[ cgBufferFormat::R32G8X24_Typeless ]          = 0;
    mTexture1D[ cgBufferFormat::R32_Float_X8X24_Typeless ]   = 0;
    mTexture1D[ cgBufferFormat::X32_Typeless_G8X24_UInt ]    = 0;
    mTexture1D[ cgBufferFormat::R10G10B10A2_Typeless ]       = 0;
    mTexture1D[ cgBufferFormat::R10G10B10A2 ]                = 0;
    mTexture1D[ cgBufferFormat::R10G10B10A2_UInt ]           = 0;
    mTexture1D[ cgBufferFormat::R11G11B10_Float ]            = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8_Typeless ]          = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8 ]                   = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8_SRGB ]              = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8_UInt ]              = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8_Signed ]            = 0;
    mTexture1D[ cgBufferFormat::R8G8B8A8_SInt ]              = 0;
    mTexture1D[ cgBufferFormat::R16G16_Typeless ]            = 0;
    mTexture1D[ cgBufferFormat::R16G16_Float ]               = 0;
    mTexture1D[ cgBufferFormat::R16G16 ]                     = 0;
    mTexture1D[ cgBufferFormat::R16G16_UInt ]                = 0;
    mTexture1D[ cgBufferFormat::R16G16_Signed ]              = 0;
    mTexture1D[ cgBufferFormat::R16G16_SInt ]                = 0;
    mTexture1D[ cgBufferFormat::R32_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::R32_Float ]                  = 0;
    mTexture1D[ cgBufferFormat::R32_UInt ]                   = 0;
    mTexture1D[ cgBufferFormat::R32_SInt ]                   = 0;
    mTexture1D[ cgBufferFormat::R24G8_Typeless ]             = 0;
    mTexture1D[ cgBufferFormat::R24_UNorm_X8_Typeless ]      = 0;
    mTexture1D[ cgBufferFormat::X24_Typeless_G8_UInt ]       = 0;
    mTexture1D[ cgBufferFormat::R8G8_Typeless ]              = 0;
    mTexture1D[ cgBufferFormat::R8G8 ]                       = 0;
    mTexture1D[ cgBufferFormat::R8G8_UInt ]                  = 0;
    mTexture1D[ cgBufferFormat::R8G8_Signed ]                = 0;
    mTexture1D[ cgBufferFormat::R8G8_SInt ]                  = 0;
    mTexture1D[ cgBufferFormat::R16_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::R16_Float ]                  = 0;
    mTexture1D[ cgBufferFormat::R16 ]                        = 0;
    mTexture1D[ cgBufferFormat::R16_UInt ]                   = 0;
    mTexture1D[ cgBufferFormat::R16_Signed ]                 = 0;
    mTexture1D[ cgBufferFormat::R16_SInt ]                   = 0;
    mTexture1D[ cgBufferFormat::R8_Typeless ]                = 0;
    mTexture1D[ cgBufferFormat::R8 ]                         = 0;
    mTexture1D[ cgBufferFormat::R8_UInt ]                    = 0;
    mTexture1D[ cgBufferFormat::R8_Signed ]                  = 0;
    mTexture1D[ cgBufferFormat::R8_SInt ]                    = 0;
    mTexture1D[ cgBufferFormat::A8 ]                         = 0;
    mTexture1D[ cgBufferFormat::R1 ]                         = 0;
    mTexture1D[ cgBufferFormat::R9G9B9E5_SharedExp ]         = 0;
    mTexture1D[ cgBufferFormat::R8G8_B8G8 ]                  = 0;
    mTexture1D[ cgBufferFormat::G8R8_G8B8 ]                  = 0;
    mTexture1D[ cgBufferFormat::BC1_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC1 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC1_SRGB ]                   = 0;
    mTexture1D[ cgBufferFormat::BC2_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC2 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC2_SRGB ]                   = 0;
    mTexture1D[ cgBufferFormat::BC3_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC3 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC3_SRGB ]                   = 0;
    mTexture1D[ cgBufferFormat::BC4_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC4 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC4_Signed ]                 = 0;
    mTexture1D[ cgBufferFormat::BC5_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC5 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC5_Signed ]                 = 0;
    mTexture1D[ cgBufferFormat::B5G6R5 ]                     = 0;
    mTexture1D[ cgBufferFormat::B5G5R5A1 ]                   = 0;
    mTexture1D[ cgBufferFormat::B8G8R8A8 ]                   = 0;
    mTexture1D[ cgBufferFormat::B8G8R8X8 ]                   = 0;
    //mTexture1D[ cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm ] = 0;
    mTexture1D[ cgBufferFormat::B8G8R8A8_Typeless ]          = 0;
    mTexture1D[ cgBufferFormat::B8G8R8A8_SRGB ]              = 0;
    mTexture1D[ cgBufferFormat::B8G8R8X8_Typeless ]          = 0;
    mTexture1D[ cgBufferFormat::B8G8R8X8_SRGB ]              = 0;
    mTexture1D[ cgBufferFormat::BC6H_Typeless ]              = 0;
    mTexture1D[ cgBufferFormat::BC6H_UF16 ]                  = 0;
    mTexture1D[ cgBufferFormat::BC6H_SF16 ]                  = 0;
    mTexture1D[ cgBufferFormat::BC7_Typeless ]               = 0;
    mTexture1D[ cgBufferFormat::BC7 ]                        = 0;
    mTexture1D[ cgBufferFormat::BC7_SRGB ]                   = 0;
    mTexture1D[ cgBufferFormat::B8G8R8 ]                     = 0;
    mTexture1D[ cgBufferFormat::B5G5R5X1 ]                   = 0;

    // Copy format items to the other relevant buffer type maps.
    mTexture2D     = mTexture1D;
    mTexture3D     = mTexture1D;
    mTextureCube   = mTexture1D;
    mRenderTarget  = mTexture1D;

    // Only very specific formats should be tested for depth buffers
    mDepthStencil[ cgBufferFormat::D32_Float_S8X24_UInt ]       = 0;
    mDepthStencil[ cgBufferFormat::D32_Float ]                  = 0;
    mDepthStencil[ cgBufferFormat::D24_UNorm_S8_UInt ]          = 0;
    mDepthStencil[ cgBufferFormat::D24_Float_S8_UInt ]          = 0;
    mDepthStencil[ cgBufferFormat::D24_UNorm_X8_Typeless ]      = 0;
    mDepthStencil[ cgBufferFormat::D16 ]                        = 0;
    mDepthStencil[ cgBufferFormat::DF16 ]                       = 0;  // Custom FourCC
    mDepthStencil[ cgBufferFormat::DF24 ]                       = 0;  // Custom FourCC
    mDepthStencil[ cgBufferFormat::INTZ ]                       = 0;  // Custom FourCC
    mDepthStencil[ cgBufferFormat::RAWZ ]                       = 0;  // Custom FourCC
}

//-----------------------------------------------------------------------------
//  Name : formatHasAlpha () (Static)
/// <summary>
/// Utility function to determine if the specified image format has
/// an alpha component.
/// Note : This function returns information about whether this format
/// explicitly contains an alpha channel, not necessarily whether one
/// can potentially be stored in the pixel or not (i.e. A8 vs X8).
/// </summary>
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::formatHasAlpha( cgBufferFormat::Base Format )
{
    // Switch based on the requested format
    switch ( Format )
    {
        case cgBufferFormat::R32G32B32A32_Typeless:
        case cgBufferFormat::R32G32B32A32_Float:
        case cgBufferFormat::R32G32B32A32_UInt:
        case cgBufferFormat::R32G32B32A32_SInt:
        case cgBufferFormat::R16G16B16A16_Typeless:
        case cgBufferFormat::R16G16B16A16_Float:
        case cgBufferFormat::R16G16B16A16:
        case cgBufferFormat::R16G16B16A16_UInt:
        case cgBufferFormat::R16G16B16A16_Signed:
        case cgBufferFormat::R16G16B16A16_SInt:
        case cgBufferFormat::R10G10B10A2_Typeless:
        case cgBufferFormat::R10G10B10A2:
        case cgBufferFormat::R10G10B10A2_UInt:
        case cgBufferFormat::R8G8B8A8_Typeless:
        case cgBufferFormat::R8G8B8A8:
        case cgBufferFormat::R8G8B8A8_SRGB:
        case cgBufferFormat::R8G8B8A8_UInt:
        case cgBufferFormat::R8G8B8A8_Signed:
        case cgBufferFormat::R8G8B8A8_SInt:
        case cgBufferFormat::A8:
        case cgBufferFormat::BC2_Typeless:
        case cgBufferFormat::BC2:
        case cgBufferFormat::BC2_SRGB:
        case cgBufferFormat::BC3_Typeless:
        case cgBufferFormat::BC3:
        case cgBufferFormat::BC3_SRGB:
        case cgBufferFormat::B5G5R5A1:
        case cgBufferFormat::B8G8R8A8:
        // case cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm:
        case cgBufferFormat::B8G8R8A8_Typeless:
        case cgBufferFormat::B8G8R8A8_SRGB:
        case cgBufferFormat::BC7_Typeless:
        case cgBufferFormat::BC7:
        case cgBufferFormat::BC7_SRGB:
            return true;
    
    } // End format switch

    // Format not recognized or is not alpha
    return false;
}

//-----------------------------------------------------------------------------
//  Name : formatIsFloatingPoint () (Static)
/// <summary>
/// Utility function to determine if the specified format is a floating
/// point texture format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::formatIsFloatingPoint( cgBufferFormat::Base Format )
{
    // Switch based on the requested format
    switch ( Format )
    {
        case cgBufferFormat::R32G32B32A32_Float:
        case cgBufferFormat::R32G32B32_Float:
        case cgBufferFormat::R16G16B16A16_Float:
        case cgBufferFormat::R32G32_Float:
        case cgBufferFormat::R32_Float_X8X24_Typeless:
        case cgBufferFormat::R11G11B10_Float:
        case cgBufferFormat::R16G16_Float:
        case cgBufferFormat::R32_Float:
        case cgBufferFormat::R16_Float:
        case cgBufferFormat::R9G9B9E5_SharedExp:
        case cgBufferFormat::BC6H_UF16:
        case cgBufferFormat::BC6H_SF16:
        
        // Depth formats too
        case cgBufferFormat::D32_Float_S8X24_UInt:
        case cgBufferFormat::D32_Float:
        case cgBufferFormat::DF16:
        case cgBufferFormat::DF24:
        case cgBufferFormat::INTZ:
        case cgBufferFormat::RAWZ:

            return true;
        
    } // End format switch

    // Unrecognized or non floating point
    return false;
}

//-----------------------------------------------------------------------------
// Name : formatBitsPerPixel () (Static)
// Desc : Utility function to determine the number of bits used for each pixel
//        in a texture created using the specified format.
// Note : This function returns information about the number of bits this 
//        format explicitly consumes, not necessarily whether data is
//        physically stored in each component (i.e. X8 vs A8).
//-----------------------------------------------------------------------------
cgUInt8 cgBufferFormatEnum::formatBitsPerPixel( cgBufferFormat::Base Format )
{
    // Switch based on the requested format
    switch ( Format )
    {
        case cgBufferFormat::R32G32B32A32_Typeless:
        case cgBufferFormat::R32G32B32A32_Float:
        case cgBufferFormat::R32G32B32A32_UInt:
        case cgBufferFormat::R32G32B32A32_SInt:
            return 128;

        case cgBufferFormat::R32G32B32_Typeless:
        case cgBufferFormat::R32G32B32_Float:
        case cgBufferFormat::R32G32B32_UInt:
        case cgBufferFormat::R32G32B32_SInt:
            return 96;

        case cgBufferFormat::R16G16B16A16_Typeless:
        case cgBufferFormat::R16G16B16A16_Float:
        case cgBufferFormat::R16G16B16A16:
        case cgBufferFormat::R16G16B16A16_UInt:
        case cgBufferFormat::R16G16B16A16_Signed:
        case cgBufferFormat::R16G16B16A16_SInt:
        case cgBufferFormat::R32G32_Typeless:
        case cgBufferFormat::R32G32_Float:
        case cgBufferFormat::R32G32_UInt:
        case cgBufferFormat::R32G32_SInt:
        case cgBufferFormat::R32G8X24_Typeless:
        case cgBufferFormat::R32_Float_X8X24_Typeless:
        case cgBufferFormat::X32_Typeless_G8X24_UInt:
        case cgBufferFormat::D32_Float_S8X24_UInt:
            return 64;

        case cgBufferFormat::R10G10B10A2_Typeless:
        case cgBufferFormat::R10G10B10A2:
        case cgBufferFormat::R10G10B10A2_UInt:
        case cgBufferFormat::R11G11B10_Float:
        case cgBufferFormat::R8G8B8A8_Typeless:
        case cgBufferFormat::R8G8B8A8:
        case cgBufferFormat::R8G8B8A8_SRGB:
        case cgBufferFormat::R8G8B8A8_UInt:
        case cgBufferFormat::R8G8B8A8_Signed:
        case cgBufferFormat::R8G8B8A8_SInt:
        case cgBufferFormat::R16G16_Typeless:
        case cgBufferFormat::R16G16_Float:
        case cgBufferFormat::R16G16:
        case cgBufferFormat::R16G16_UInt:
        case cgBufferFormat::R16G16_Signed:
        case cgBufferFormat::R16G16_SInt:
        case cgBufferFormat::R32_Typeless:
        case cgBufferFormat::R32_Float:
        case cgBufferFormat::R32_UInt:
        case cgBufferFormat::R32_SInt:
        case cgBufferFormat::R24G8_Typeless:
        case cgBufferFormat::R24_UNorm_X8_Typeless:
        case cgBufferFormat::X24_Typeless_G8_UInt:
        case cgBufferFormat::R9G9B9E5_SharedExp:
        case cgBufferFormat::R8G8_B8G8:
        case cgBufferFormat::G8R8_G8B8:
        case cgBufferFormat::B8G8R8A8:
        case cgBufferFormat::B8G8R8X8:
        //case cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm:
        case cgBufferFormat::B8G8R8A8_Typeless:
        case cgBufferFormat::B8G8R8A8_SRGB:
        case cgBufferFormat::B8G8R8X8_Typeless:
        case cgBufferFormat::B8G8R8X8_SRGB:
        case cgBufferFormat::D32_Float:
        case cgBufferFormat::D24_UNorm_S8_UInt:
        case cgBufferFormat::D24_Float_S8_UInt:
        case cgBufferFormat::D24_UNorm_X8_Typeless:
        case cgBufferFormat::INTZ:
        case cgBufferFormat::RAWZ:
            return 32;

        case cgBufferFormat::B8G8R8:
        case cgBufferFormat::DF24:
            return 24;

        case cgBufferFormat::R8G8_Typeless:
        case cgBufferFormat::R8G8:
        case cgBufferFormat::R8G8_UInt:
        case cgBufferFormat::R8G8_Signed:
        case cgBufferFormat::R8G8_SInt:
        case cgBufferFormat::R16_Typeless:
        case cgBufferFormat::R16_Float:
        case cgBufferFormat::R16:
        case cgBufferFormat::R16_UInt:
        case cgBufferFormat::R16_Signed:
        case cgBufferFormat::R16_SInt:
        case cgBufferFormat::B5G6R5:
        case cgBufferFormat::B5G5R5A1:
        case cgBufferFormat::B5G5R5X1:
        case cgBufferFormat::D16:
        case cgBufferFormat::DF16:
            return 16;

        case cgBufferFormat::R8_Typeless:
        case cgBufferFormat::R8:
        case cgBufferFormat::R8_UInt:
        case cgBufferFormat::R8_Signed:
        case cgBufferFormat::R8_SInt:
        case cgBufferFormat::A8:
        case cgBufferFormat::BC2:
        case cgBufferFormat::BC2_SRGB:
        case cgBufferFormat::BC2_Typeless:
        case cgBufferFormat::BC3:
        case cgBufferFormat::BC3_SRGB:
        case cgBufferFormat::BC3_Typeless:
        case cgBufferFormat::BC4:
        case cgBufferFormat::BC4_Signed:
        case cgBufferFormat::BC4_Typeless:
        case cgBufferFormat::BC5:
        case cgBufferFormat::BC5_Signed:
        case cgBufferFormat::BC5_Typeless:
        case cgBufferFormat::BC6H_UF16:
        case cgBufferFormat::BC6H_SF16:
        case cgBufferFormat::BC6H_Typeless:
        case cgBufferFormat::BC7:
        case cgBufferFormat::BC7_SRGB:
        case cgBufferFormat::BC7_Typeless:
            return 8;

        case cgBufferFormat::BC1:
        case cgBufferFormat::BC1_SRGB:
        case cgBufferFormat::BC1_Typeless:
            return 4;
        
        case cgBufferFormat::R1:
            return 1;
   
    } // End format switch

    // Format not recognized
    return 0;
}

//-----------------------------------------------------------------------------
// Name : formatGroupMatches () (Static)
// Desc : Determine whether or not the the two formats belong to the same
//        format group (i.e. R32G32B32_Float & R32G32B32_UINT)
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::formatGroupMatches( cgBufferFormat::Base Format1, cgBufferFormat::Base Format2 )
{
    cgToDo( "Carbon General", "Optimize with a lookup table?" )

    // Switch based on the requested format
    switch ( Format1 )
    {
        case cgBufferFormat::R32G32B32A32_Typeless:
        case cgBufferFormat::R32G32B32A32_Float:
        case cgBufferFormat::R32G32B32A32_UInt:
        case cgBufferFormat::R32G32B32A32_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R32G32B32A32_Typeless:
                case cgBufferFormat::R32G32B32A32_Float:
                case cgBufferFormat::R32G32B32A32_UInt:
                case cgBufferFormat::R32G32B32A32_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R32G32B32_Typeless:
        case cgBufferFormat::R32G32B32_Float:
        case cgBufferFormat::R32G32B32_UInt:
        case cgBufferFormat::R32G32B32_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R32G32B32_Typeless:
                case cgBufferFormat::R32G32B32_Float:
                case cgBufferFormat::R32G32B32_UInt:
                case cgBufferFormat::R32G32B32_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R16G16B16A16_Typeless:
        case cgBufferFormat::R16G16B16A16_Float:
        case cgBufferFormat::R16G16B16A16:
        case cgBufferFormat::R16G16B16A16_UInt:
        case cgBufferFormat::R16G16B16A16_Signed:
        case cgBufferFormat::R16G16B16A16_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R16G16B16A16_Typeless:
                case cgBufferFormat::R16G16B16A16_Float:
                case cgBufferFormat::R16G16B16A16:
                case cgBufferFormat::R16G16B16A16_UInt:
                case cgBufferFormat::R16G16B16A16_Signed:
                case cgBufferFormat::R16G16B16A16_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R32G32_Typeless:
        case cgBufferFormat::R32G32_Float:
        case cgBufferFormat::R32G32_UInt:
        case cgBufferFormat::R32G32_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R32G32_Typeless:
                case cgBufferFormat::R32G32_Float:
                case cgBufferFormat::R32G32_UInt:
                case cgBufferFormat::R32G32_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R32G8X24_Typeless:
        case cgBufferFormat::R32_Float_X8X24_Typeless:
        case cgBufferFormat::X32_Typeless_G8X24_UInt:
        case cgBufferFormat::D32_Float_S8X24_UInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R32G8X24_Typeless:
                case cgBufferFormat::R32_Float_X8X24_Typeless:
                case cgBufferFormat::X32_Typeless_G8X24_UInt:
                case cgBufferFormat::D32_Float_S8X24_UInt:
                    return true;
            
            } // End switch Format2
            break;
        
        case cgBufferFormat::R10G10B10A2_Typeless:
        case cgBufferFormat::R10G10B10A2:
        case cgBufferFormat::R10G10B10A2_UInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R10G10B10A2_Typeless:
                case cgBufferFormat::R10G10B10A2:
                case cgBufferFormat::R10G10B10A2_UInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R11G11B10_Float:
            switch ( Format2 )
            {
                case cgBufferFormat::R11G11B10_Float:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R8G8B8A8_Typeless:
        case cgBufferFormat::R8G8B8A8:
        case cgBufferFormat::R8G8B8A8_SRGB:
        case cgBufferFormat::R8G8B8A8_UInt:
        case cgBufferFormat::R8G8B8A8_Signed:
        case cgBufferFormat::R8G8B8A8_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R8G8B8A8_Typeless:
                case cgBufferFormat::R8G8B8A8:
                case cgBufferFormat::R8G8B8A8_SRGB:
                case cgBufferFormat::R8G8B8A8_UInt:
                case cgBufferFormat::R8G8B8A8_Signed:
                case cgBufferFormat::R8G8B8A8_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R16G16_Typeless:
        case cgBufferFormat::R16G16_Float:
        case cgBufferFormat::R16G16:
        case cgBufferFormat::R16G16_UInt:
        case cgBufferFormat::R16G16_Signed:
        case cgBufferFormat::R16G16_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R16G16_Typeless:
                case cgBufferFormat::R16G16_Float:
                case cgBufferFormat::R16G16:
                case cgBufferFormat::R16G16_UInt:
                case cgBufferFormat::R16G16_Signed:
                case cgBufferFormat::R16G16_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R32_Typeless:
        case cgBufferFormat::R32_Float:
        case cgBufferFormat::R32_UInt:
        case cgBufferFormat::R32_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R32_Typeless:
                case cgBufferFormat::R32_Float:
                case cgBufferFormat::R32_UInt:
                case cgBufferFormat::R32_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R24G8_Typeless:
        case cgBufferFormat::R24_UNorm_X8_Typeless:
        case cgBufferFormat::X24_Typeless_G8_UInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R24G8_Typeless:
                case cgBufferFormat::R24_UNorm_X8_Typeless:
                case cgBufferFormat::X24_Typeless_G8_UInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R9G9B9E5_SharedExp:
            switch ( Format2 )
            {
                case cgBufferFormat::R9G9B9E5_SharedExp:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R8G8_B8G8:
            switch ( Format2 )
            {
                case cgBufferFormat::R8G8_B8G8:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::G8R8_G8B8:
            switch ( Format2 )
            {
                case cgBufferFormat::G8R8_G8B8:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B8G8R8A8:
            switch ( Format2 )
            {
                case cgBufferFormat::B8G8R8A8:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B8G8R8X8:
            switch ( Format2 )
            {
                case cgBufferFormat::B8G8R8X8:
                    return true;
            
            } // End switch Format2
            break;

        //case cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm:
            //switch ( Format2 )
            //{
                //case cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm:
                    //return true;
            
            //} // End switch Format2
            //break;

        case cgBufferFormat::B8G8R8A8_Typeless:
        case cgBufferFormat::B8G8R8A8_SRGB:
            switch ( Format2 )
            {
                case cgBufferFormat::B8G8R8A8_Typeless:
                case cgBufferFormat::B8G8R8A8_SRGB:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B8G8R8X8_Typeless:
        case cgBufferFormat::B8G8R8X8_SRGB:
            switch ( Format2 )
            {
                case cgBufferFormat::B8G8R8X8_Typeless:
                case cgBufferFormat::B8G8R8X8_SRGB:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::D32_Float:
            switch ( Format2 )
            {
                case cgBufferFormat::D32_Float:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::D24_UNorm_S8_UInt:
        case cgBufferFormat::D24_Float_S8_UInt:
        case cgBufferFormat::D24_UNorm_X8_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::D24_UNorm_S8_UInt:
                case cgBufferFormat::D24_Float_S8_UInt:
                case cgBufferFormat::D24_UNorm_X8_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::INTZ:
            switch ( Format2 )
            {
                case cgBufferFormat::INTZ:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::RAWZ:
            switch ( Format2 )
            {
                case cgBufferFormat::RAWZ:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B8G8R8:
            switch ( Format2 )
            {
                case cgBufferFormat::B8G8R8:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::DF24:
            switch ( Format2 )
            {
                case cgBufferFormat::DF24:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R8G8_Typeless:
        case cgBufferFormat::R8G8:
        case cgBufferFormat::R8G8_UInt:
        case cgBufferFormat::R8G8_Signed:
        case cgBufferFormat::R8G8_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R8G8_Typeless:
                case cgBufferFormat::R8G8:
                case cgBufferFormat::R8G8_UInt:
                case cgBufferFormat::R8G8_Signed:
                case cgBufferFormat::R8G8_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R16_Typeless:
        case cgBufferFormat::R16_Float:
        case cgBufferFormat::R16:
        case cgBufferFormat::R16_UInt:
        case cgBufferFormat::R16_Signed:
        case cgBufferFormat::R16_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R16_Typeless:
                case cgBufferFormat::R16_Float:
                case cgBufferFormat::R16:
                case cgBufferFormat::R16_UInt:
                case cgBufferFormat::R16_Signed:
                case cgBufferFormat::R16_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B5G6R5:
            switch ( Format2 )
            {
                case cgBufferFormat::B5G6R5:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B5G5R5A1:
            switch ( Format2 )
            {
                case cgBufferFormat::B5G5R5A1:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::B5G5R5X1:
            switch ( Format2 )
            {
                case cgBufferFormat::B5G5R5X1:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::D16:
            switch ( Format2 )
            {
                case cgBufferFormat::D16:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::DF16:
            switch ( Format2 )
            {
                case cgBufferFormat::DF16:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R8_Typeless:
        case cgBufferFormat::R8:
        case cgBufferFormat::R8_UInt:
        case cgBufferFormat::R8_Signed:
        case cgBufferFormat::R8_SInt:
            switch ( Format2 )
            {
                case cgBufferFormat::R8_Typeless:
                case cgBufferFormat::R8:
                case cgBufferFormat::R8_UInt:
                case cgBufferFormat::R8_Signed:
                case cgBufferFormat::R8_SInt:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::A8:
            switch ( Format2 )
            {
                case cgBufferFormat::A8:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC2:
        case cgBufferFormat::BC2_SRGB:
        case cgBufferFormat::BC2_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC2:
                case cgBufferFormat::BC2_SRGB:
                case cgBufferFormat::BC2_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC3:
        case cgBufferFormat::BC3_SRGB:
        case cgBufferFormat::BC3_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC3:
                case cgBufferFormat::BC3_SRGB:
                case cgBufferFormat::BC3_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC4:
        case cgBufferFormat::BC4_Signed:
        case cgBufferFormat::BC4_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC4:
                case cgBufferFormat::BC4_Signed:
                case cgBufferFormat::BC4_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC5:
        case cgBufferFormat::BC5_Signed:
        case cgBufferFormat::BC5_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC5:
                case cgBufferFormat::BC5_Signed:
                case cgBufferFormat::BC5_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC6H_UF16:
        case cgBufferFormat::BC6H_SF16:
        case cgBufferFormat::BC6H_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC6H_UF16:
                case cgBufferFormat::BC6H_SF16:
                case cgBufferFormat::BC6H_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC7:
        case cgBufferFormat::BC7_SRGB:
        case cgBufferFormat::BC7_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC7:
                case cgBufferFormat::BC7_SRGB:
                case cgBufferFormat::BC7_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::BC1:
        case cgBufferFormat::BC1_SRGB:
        case cgBufferFormat::BC1_Typeless:
            switch ( Format2 )
            {
                case cgBufferFormat::BC1:
                case cgBufferFormat::BC1_SRGB:
                case cgBufferFormat::BC1_Typeless:
                    return true;
            
            } // End switch Format2
            break;

        case cgBufferFormat::R1:
            switch ( Format2 )
            {
                case cgBufferFormat::R1:
                    return true;
            
            } // End switch Format2
            break;
    
    } // End format switch

    // Format not recognized
    return false;
}

//-----------------------------------------------------------------------------
// Name : formatToString() (Static)
// Desc : Utility function to return a string name based on a valid format 
//        enum item.
//-----------------------------------------------------------------------------
cgString cgBufferFormatEnum::formatToString( cgBufferFormat::Base Format )
{
    switch ( Format )
    {
        case cgBufferFormat::R32G32B32A32_Typeless:
            return _T("R32G32B32A32_Typeless");
        case cgBufferFormat::R32G32B32A32_Float:
            return _T("R32G32B32A32_Float");
        case cgBufferFormat::R32G32B32A32_UInt:
            return _T("R32G32B32A32_UInt");
        case cgBufferFormat::R32G32B32A32_SInt:
            return _T("R32G32B32A32_SInt");
        case cgBufferFormat::R32G32B32_Typeless:
            return _T("R32G32B32_Typeless");
        case cgBufferFormat::R32G32B32_Float:
            return _T("R32G32B32_Float");
        case cgBufferFormat::R32G32B32_UInt:
            return _T("R32G32B32_UInt");
        case cgBufferFormat::R32G32B32_SInt:
            return _T("R32G32B32_SInt");
        case cgBufferFormat::R16G16B16A16_Typeless:
            return _T("R16G16B16A16_Typeless");
        case cgBufferFormat::R16G16B16A16_Float:
            return _T("R16G16B16A16_Float");
        case cgBufferFormat::R16G16B16A16:
            return _T("R16G16B16A16");
        case cgBufferFormat::R16G16B16A16_UInt:
            return _T("R16G16B16A16_UInt");
        case cgBufferFormat::R16G16B16A16_Signed:
            return _T("R16G16B16A16_Signed");
        case cgBufferFormat::R16G16B16A16_SInt:
            return _T("R16G16B16A16_SInt");
        case cgBufferFormat::R32G32_Typeless:
            return _T("R32G32_Typeless");
        case cgBufferFormat::R32G32_Float:
            return _T("R32G32_Float");
        case cgBufferFormat::R32G32_UInt:
            return _T("R32G32_UInt");
        case cgBufferFormat::R32G32_SInt:
            return _T("R32G32_SInt");
        case cgBufferFormat::R32G8X24_Typeless:
            return _T("R32G8X24_Typeless");
        case cgBufferFormat::R32_Float_X8X24_Typeless:
            return _T("R32_Float_X8X24_Typeless");
        case cgBufferFormat::X32_Typeless_G8X24_UInt:
            return _T("X32_Typeless_G8X24_UInt");
        case cgBufferFormat::R10G10B10A2_Typeless:
            return _T("R10G10B10A2_Typeless");
        case cgBufferFormat::R10G10B10A2:
            return _T("R10G10B10A2");
        case cgBufferFormat::R10G10B10A2_UInt:
            return _T("R10G10B10A2_UInt");
        case cgBufferFormat::R11G11B10_Float:
            return _T("R11G11B10_Float");
        case cgBufferFormat::R8G8B8A8_Typeless:
            return _T("R8G8B8A8_Typeless");
        case cgBufferFormat::R8G8B8A8:
            return _T("R8G8B8A8");
        case cgBufferFormat::R8G8B8A8_SRGB:
            return _T("R8G8B8A8_SRGB");
        case cgBufferFormat::R8G8B8A8_UInt:
            return _T("R8G8B8A8_UInt");
        case cgBufferFormat::R8G8B8A8_Signed:
            return _T("R8G8B8A8_Signed");
        case cgBufferFormat::R8G8B8A8_SInt:
            return _T("R8G8B8A8_SInt");
        case cgBufferFormat::R16G16_Typeless:
            return _T("R16G16_Typeless");
        case cgBufferFormat::R16G16_Float:
            return _T("R16G16_Float");
        case cgBufferFormat::R16G16:
            return _T("R16G16");
        case cgBufferFormat::R16G16_UInt:
            return _T("R16G16_UInt");
        case cgBufferFormat::R16G16_Signed:
            return _T("R16G16_Signed");
        case cgBufferFormat::R16G16_SInt:
            return _T("R16G16_SInt");
        case cgBufferFormat::R32_Typeless:
            return _T("R32_Typeless");
        case cgBufferFormat::R32_Float:
            return _T("R32_Float");
        case cgBufferFormat::R32_UInt:
            return _T("R32_UInt");
        case cgBufferFormat::R32_SInt:
            return _T("R32_SInt");
        case cgBufferFormat::R24G8_Typeless:
            return _T("R24G8_Typeless");
        case cgBufferFormat::R24_UNorm_X8_Typeless:
            return _T("R24_UNorm_X8_Typeless");
        case cgBufferFormat::X24_Typeless_G8_UInt:
            return _T("X24_Typeless_G8_UInt");
        case cgBufferFormat::R8G8_Typeless:
            return _T("R8G8_Typeless");
        case cgBufferFormat::R8G8:
            return _T("R8G8");
        case cgBufferFormat::R8G8_UInt:
            return _T("R8G8_UInt");
        case cgBufferFormat::R8G8_Signed:
            return _T("R8G8_Signed");
        case cgBufferFormat::R8G8_SInt:
            return _T("R8G8_SInt");
        case cgBufferFormat::R16_Typeless:
            return _T("R16_Typeless");
        case cgBufferFormat::R16_Float:
            return _T("R16_Float");
        case cgBufferFormat::R16:
            return _T("R16");
        case cgBufferFormat::R16_UInt:
            return _T("R16_UInt");
        case cgBufferFormat::R16_Signed:
            return _T("R16_Signed");
        case cgBufferFormat::R16_SInt:
            return _T("R16_SInt");
        case cgBufferFormat::R8_Typeless:
            return _T("R8_Typeless");
        case cgBufferFormat::R8:
            return _T("R8");
        case cgBufferFormat::R8_UInt:
            return _T("R8_UInt");
        case cgBufferFormat::R8_Signed:
            return _T("R8_Signed");
        case cgBufferFormat::R8_SInt:
            return _T("R8_SInt");
        case cgBufferFormat::A8:
            return _T("A8");
        case cgBufferFormat::R1:
            return _T("R1");
        case cgBufferFormat::R9G9B9E5_SharedExp:
            return _T("R9G9B9E5_SharedExp");
        case cgBufferFormat::R8G8_B8G8:
            return _T("R8G8_B8G8");
        case cgBufferFormat::G8R8_G8B8:
            return _T("G8R8_G8B8");
        case cgBufferFormat::BC1_Typeless:
            return _T("BC1_Typeless");
        case cgBufferFormat::BC1:
            return _T("BC1");
        case cgBufferFormat::BC1_SRGB:
            return _T("BC1_SRGB");
        case cgBufferFormat::BC2_Typeless:
            return _T("BC2_Typeless");
        case cgBufferFormat::BC2:
            return _T("BC2");
        case cgBufferFormat::BC2_SRGB:
            return _T("BC2_SRGB");
        case cgBufferFormat::BC3_Typeless:
            return _T("BC3_Typeless");
        case cgBufferFormat::BC3:
            return _T("BC3");
        case cgBufferFormat::BC3_SRGB:
            return _T("BC3_SRGB");
        case cgBufferFormat::BC4_Typeless:
            return _T("BC4_Typeless");
        case cgBufferFormat::BC4:
            return _T("BC4");
        case cgBufferFormat::BC4_Signed:
            return _T("BC4_Signed");
        case cgBufferFormat::BC5_Typeless:
            return _T("BC5_Typeless");
        case cgBufferFormat::BC5:
            return _T("BC5");
        case cgBufferFormat::BC5_Signed:
            return _T("BC5_Signed");
        case cgBufferFormat::B5G6R5:
            return _T("B5G6R5");
        case cgBufferFormat::B5G5R5A1:
            return _T("B5G5R5A1");
        case cgBufferFormat::B8G8R8A8:
            return _T("B8G8R8A8");
        case cgBufferFormat::B8G8R8X8:
            return _T("B8G8R8X8");
        //case cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm:
            //return _T("R10G10B10_XR_Bias_A2_UNorm");
        case cgBufferFormat::B8G8R8A8_Typeless:
            return _T("B8G8R8A8_Typeless");
        case cgBufferFormat::B8G8R8A8_SRGB:
            return _T("B8G8R8A8_SRGB");
        case cgBufferFormat::B8G8R8X8_Typeless:
            return _T("B8G8R8X8_Typeless");
        case cgBufferFormat::B8G8R8X8_SRGB:
            return _T("B8G8R8X8_SRGB");
        case cgBufferFormat::BC6H_Typeless:
            return _T("BC6H_Typeless");
        case cgBufferFormat::BC6H_UF16:
            return _T("BC6H_UF16");
        case cgBufferFormat::BC6H_SF16:
            return _T("BC6H_SF16");
        case cgBufferFormat::BC7_Typeless:
            return _T("BC7_Typeless");
        case cgBufferFormat::BC7:
            return _T("BC7");
        case cgBufferFormat::BC7_SRGB:
            return _T("BC7_SRGB");
        case cgBufferFormat::B8G8R8:
            return _T("B8G8R8");
        case cgBufferFormat::B5G5R5X1:
            return _T("B5G5R5X1");

        // Depth formats too
        case cgBufferFormat::D32_Float_S8X24_UInt:
            return _T("D32_Float_S8X24_UInt");
        case cgBufferFormat::D32_Float:
            return _T("D32_Float");
        case cgBufferFormat::D24_UNorm_S8_UInt:
            return _T("D24_UNorm_S8_UInt");
        case cgBufferFormat::D24_Float_S8_UInt:
            return _T("D24_Float_S8_UInt");
        case cgBufferFormat::D24_UNorm_X8_Typeless:
            return _T("D24_UNorm_X8_Typeless");
        case cgBufferFormat::D16:
            return _T("D16");
        case cgBufferFormat::DF16:
            return _T("DF16");
        case cgBufferFormat::DF24:
            return _T("DF24");
        case cgBufferFormat::INTZ:
            return _T("INTZ");
        case cgBufferFormat::RAWZ:
            return _T("RAWZ");
    
    } // End format switch

    // Unknown format
    return _T("Unknown");
}

//-----------------------------------------------------------------------------
//  Name : formatFromString() (Static)
/// <summary>
/// Utility function to return a valid D3D format enum item based on the
/// string specified.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBufferFormatEnum::formatFromString( const cgString & strFormat )
{
    if ( !strFormat.compare( _T("R32G32B32A32_Typeless"), true ) )
        return cgBufferFormat::R32G32B32A32_Typeless;
    else if ( !strFormat.compare( _T("R32G32B32A32_Float"), true ) )
        return cgBufferFormat::R32G32B32A32_Float;
    else if ( !strFormat.compare( _T("R32G32B32A32_UInt"), true ) )
        return cgBufferFormat::R32G32B32A32_UInt;
    else if ( !strFormat.compare( _T("R32G32B32A32_SInt"), true ) )
        return cgBufferFormat::R32G32B32A32_SInt;
    else if ( !strFormat.compare( _T("R32G32B32_Typeless"), true ) )
        return cgBufferFormat::R32G32B32_Typeless;
    else if ( !strFormat.compare( _T("R32G32B32_Float"), true ) )
        return cgBufferFormat::R32G32B32_Float;
    else if ( !strFormat.compare( _T("R32G32B32_UInt"), true ) )
        return cgBufferFormat::R32G32B32_UInt;
    else if ( !strFormat.compare( _T("R32G32B32_SInt"), true ) )
        return cgBufferFormat::R32G32B32_SInt;
    else if ( !strFormat.compare( _T("R16G16B16A16_Typeless"), true ) )
        return cgBufferFormat::R16G16B16A16_Typeless;
    else if ( !strFormat.compare( _T("R16G16B16A16_Float"), true ) )
        return cgBufferFormat::R16G16B16A16_Float;
    else if ( !strFormat.compare( _T("R16G16B16A16"), true ) )
        return cgBufferFormat::R16G16B16A16;
    else if ( !strFormat.compare( _T("R16G16B16A16_UInt"), true ) )
        return cgBufferFormat::R16G16B16A16_UInt;
    else if ( !strFormat.compare( _T("R16G16B16A16_Signed"), true ) )
        return cgBufferFormat::R16G16B16A16_Signed;
    else if ( !strFormat.compare( _T("R16G16B16A16_SInt"), true ) )
        return cgBufferFormat::R16G16B16A16_SInt;
    else if ( !strFormat.compare( _T("R32G32_Typeless"), true ) )
        return cgBufferFormat::R32G32_Typeless;
    else if ( !strFormat.compare( _T("R32G32_Float"), true ) )
        return cgBufferFormat::R32G32_Float;
    else if ( !strFormat.compare( _T("R32G32_UInt"), true ) )
        return cgBufferFormat::R32G32_UInt;
    else if ( !strFormat.compare( _T("R32G32_SInt"), true ) )
        return cgBufferFormat::R32G32_SInt;
    else if ( !strFormat.compare( _T("R32G8X24_Typeless"), true ) )
        return cgBufferFormat::R32G8X24_Typeless;
    else if ( !strFormat.compare( _T("R32_Float_X8X24_Typeless"), true ) )
        return cgBufferFormat::R32_Float_X8X24_Typeless;
    else if ( !strFormat.compare( _T("X32_Typeless_G8X24_UInt"), true ) )
        return cgBufferFormat::X32_Typeless_G8X24_UInt;
    else if ( !strFormat.compare( _T("R10G10B10A2_Typeless"), true ) )
        return cgBufferFormat::R10G10B10A2_Typeless;
    else if ( !strFormat.compare( _T("R10G10B10A2"), true ) )
        return cgBufferFormat::R10G10B10A2;
    else if ( !strFormat.compare( _T("R10G10B10A2_UInt"), true ) )
        return cgBufferFormat::R10G10B10A2_UInt;
    else if ( !strFormat.compare( _T("R11G11B10_Float"), true ) )
        return cgBufferFormat::R11G11B10_Float;
    else if ( !strFormat.compare( _T("R8G8B8A8_Typeless"), true ) )
        return cgBufferFormat::R8G8B8A8_Typeless;
    else if ( !strFormat.compare( _T("R8G8B8A8"), true ) )
        return cgBufferFormat::R8G8B8A8;
    else if ( !strFormat.compare( _T("R8G8B8A8_SRGB"), true ) )
        return cgBufferFormat::R8G8B8A8_SRGB;
    else if ( !strFormat.compare( _T("R8G8B8A8_UInt"), true ) )
        return cgBufferFormat::R8G8B8A8_UInt;
    else if ( !strFormat.compare( _T("R8G8B8A8_Signed"), true ) )
        return cgBufferFormat::R8G8B8A8_Signed;
    else if ( !strFormat.compare( _T("R8G8B8A8_SInt"), true ) )
        return cgBufferFormat::R8G8B8A8_SInt;
    else if ( !strFormat.compare( _T("R16G16_Typeless"), true ) )
        return cgBufferFormat::R16G16_Typeless;
    else if ( !strFormat.compare( _T("R16G16_Float"), true ) )
        return cgBufferFormat::R16G16_Float;
    else if ( !strFormat.compare( _T("R16G16"), true ) )
        return cgBufferFormat::R16G16;
    else if ( !strFormat.compare( _T("R16G16_UInt"), true ) )
        return cgBufferFormat::R16G16_UInt;
    else if ( !strFormat.compare( _T("R16G16_Signed"), true ) )
        return cgBufferFormat::R16G16_Signed;
    else if ( !strFormat.compare( _T("R16G16_SInt"), true ) )
        return cgBufferFormat::R16G16_SInt;
    else if ( !strFormat.compare( _T("R32_Typeless"), true ) )
        return cgBufferFormat::R32_Typeless;
    else if ( !strFormat.compare( _T("R32_Float"), true ) )
        return cgBufferFormat::R32_Float;
    else if ( !strFormat.compare( _T("Index32"), true ) )
        return cgBufferFormat::Index32;
    else if ( !strFormat.compare( _T("R32_UInt"), true ) )
        return cgBufferFormat::R32_UInt;
    else if ( !strFormat.compare( _T("R32_SInt"), true ) )
        return cgBufferFormat::R32_SInt;
    else if ( !strFormat.compare( _T("R24G8_Typeless"), true ) )
        return cgBufferFormat::R24G8_Typeless;
    else if ( !strFormat.compare( _T("R24_UNorm_X8_Typeless"), true ) )
        return cgBufferFormat::R24_UNorm_X8_Typeless;
    else if ( !strFormat.compare( _T("X24_Typeless_G8_UInt"), true ) )
        return cgBufferFormat::X24_Typeless_G8_UInt;
    else if ( !strFormat.compare( _T("R8G8_Typeless"), true ) )
        return cgBufferFormat::R8G8_Typeless;
    else if ( !strFormat.compare( _T("R8G8"), true ) )
        return cgBufferFormat::R8G8;
    else if ( !strFormat.compare( _T("R8G8_UInt"), true ) )
        return cgBufferFormat::R8G8_UInt;
    else if ( !strFormat.compare( _T("R8G8_Signed"), true ) )
        return cgBufferFormat::R8G8_Signed;
    else if ( !strFormat.compare( _T("R8G8_SInt"), true ) )
        return cgBufferFormat::R8G8_SInt;
    else if ( !strFormat.compare( _T("R16_Typeless"), true ) )
        return cgBufferFormat::R16_Typeless;
    else if ( !strFormat.compare( _T("R16_Float"), true ) )
        return cgBufferFormat::R16_Float;
    else if ( !strFormat.compare( _T("R16"), true ) )
        return cgBufferFormat::R16;
    else if ( !strFormat.compare( _T("Index16"), true ) )
        return cgBufferFormat::Index16;
    else if ( !strFormat.compare( _T("R16_UInt"), true ) )
        return cgBufferFormat::R16_UInt;
    else if ( !strFormat.compare( _T("R16_Signed"), true ) )
        return cgBufferFormat::R16_Signed;
    else if ( !strFormat.compare( _T("R16_SInt"), true ) )
        return cgBufferFormat::R16_SInt;
    else if ( !strFormat.compare( _T("R8_Typeless"), true ) )
        return cgBufferFormat::R8_Typeless;
    else if ( !strFormat.compare( _T("R8"), true ) )
        return cgBufferFormat::R8;
    else if ( !strFormat.compare( _T("R8_UInt"), true ) )
        return cgBufferFormat::R8_UInt;
    else if ( !strFormat.compare( _T("R8_Signed"), true ) )
        return cgBufferFormat::R8_Signed;
    else if ( !strFormat.compare( _T("R8_SInt"), true ) )
        return cgBufferFormat::R8_SInt;
    else if ( !strFormat.compare( _T("A8"), true ) )
        return cgBufferFormat::A8;
    else if ( !strFormat.compare( _T("R1"), true ) )
        return cgBufferFormat::R1;
    else if ( !strFormat.compare( _T("R9G9B9E5_SharedExp"), true ) )
        return cgBufferFormat::R9G9B9E5_SharedExp;
    else if ( !strFormat.compare( _T("R8G8_B8G8"), true ) )
        return cgBufferFormat::R8G8_B8G8;
    else if ( !strFormat.compare( _T("G8R8_G8B8"), true ) )
        return cgBufferFormat::G8R8_G8B8;
    else if ( !strFormat.compare( _T("BC1_Typeless"), true ) )
        return cgBufferFormat::BC1_Typeless;
    else if ( !strFormat.compare( _T("BC1"), true ) )
        return cgBufferFormat::BC1;
    else if ( !strFormat.compare( _T("BC1_SRGB"), true ) )
        return cgBufferFormat::BC1_SRGB;
    else if ( !strFormat.compare( _T("BC2_Typeless"), true ) )
        return cgBufferFormat::BC2_Typeless;
    else if ( !strFormat.compare( _T("BC2"), true ) )
        return cgBufferFormat::BC2;
    else if ( !strFormat.compare( _T("BC2_SRGB"), true ) )
        return cgBufferFormat::BC2_SRGB;
    else if ( !strFormat.compare( _T("BC3_Typeless"), true ) )
        return cgBufferFormat::BC3_Typeless;
    else if ( !strFormat.compare( _T("BC3"), true ) )
        return cgBufferFormat::BC3;
    else if ( !strFormat.compare( _T("BC3_SRGB"), true ) )
        return cgBufferFormat::BC3_SRGB;
    else if ( !strFormat.compare( _T("BC4_Typeless"), true ) )
        return cgBufferFormat::BC4_Typeless;
    else if ( !strFormat.compare( _T("BC4"), true ) )
        return cgBufferFormat::BC4;
    else if ( !strFormat.compare( _T("BC4_Signed"), true ) )
        return cgBufferFormat::BC4_Signed;
    else if ( !strFormat.compare( _T("BC5_Typeless"), true ) )
        return cgBufferFormat::BC5_Typeless;
    else if ( !strFormat.compare( _T("BC5"), true ) )
        return cgBufferFormat::BC5;
    else if ( !strFormat.compare( _T("BC5_Signed"), true ) )
        return cgBufferFormat::BC5_Signed;
    else if ( !strFormat.compare( _T("B5G6R5"), true ) )
        return cgBufferFormat::B5G6R5;
    else if ( !strFormat.compare( _T("B5G5R5A1"), true ) )
        return cgBufferFormat::B5G5R5A1;
    else if ( !strFormat.compare( _T("B8G8R8A8"), true ) )
        return cgBufferFormat::B8G8R8A8;
    else if ( !strFormat.compare( _T("B8G8R8X8"), true ) )
        return cgBufferFormat::B8G8R8X8;
    //else if ( !strFormat.compare( _T("R10G10B10_XR_Bias_A2_UNorm"), true ) )
        //return cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm;
    else if ( !strFormat.compare( _T("B8G8R8A8_Typeless"), true ) )
        return cgBufferFormat::B8G8R8A8_Typeless;
    else if ( !strFormat.compare( _T("B8G8R8A8_SRGB"), true ) )
        return cgBufferFormat::B8G8R8A8_SRGB;
    else if ( !strFormat.compare( _T("B8G8R8X8_Typeless"), true ) )
        return cgBufferFormat::B8G8R8X8_Typeless;
    else if ( !strFormat.compare( _T("B8G8R8X8_SRGB"), true ) )
        return cgBufferFormat::B8G8R8X8_SRGB;
    else if ( !strFormat.compare( _T("BC6H_Typeless"), true ) )
        return cgBufferFormat::BC6H_Typeless;
    else if ( !strFormat.compare( _T("BC6H_UF16"), true ) )
        return cgBufferFormat::BC6H_UF16;
    else if ( !strFormat.compare( _T("BC6H_SF16"), true ) )
        return cgBufferFormat::BC6H_SF16;
    else if ( !strFormat.compare( _T("BC7_Typeless"), true ) )
        return cgBufferFormat::BC7_Typeless;
    else if ( !strFormat.compare( _T("BC7"), true ) )
        return cgBufferFormat::BC7;
    else if ( !strFormat.compare( _T("BC7_SRGB"), true ) )
        return cgBufferFormat::BC7_SRGB;
    else if ( !strFormat.compare( _T("B8G8R8"), true ) )
        return cgBufferFormat::B8G8R8;
    else if ( !strFormat.compare( _T("B5G5R5X1"), true ) )
        return cgBufferFormat::B5G5R5X1;
    else if ( !strFormat.compare( _T("D32_Float_S8X24_UInt"), true ) )
        return cgBufferFormat::D32_Float_S8X24_UInt;
    else if ( !strFormat.compare( _T("D32_Float"), true ) )
        return cgBufferFormat::D32_Float;
    else if ( !strFormat.compare( _T("D24_UNorm_S8_UInt"), true ) )
        return cgBufferFormat::D24_UNorm_S8_UInt;
    else if ( !strFormat.compare( _T("D24_Float_S8_UInt"), true ) )
        return cgBufferFormat::D24_Float_S8_UInt;
    else if ( !strFormat.compare( _T("D24_UNorm_X8_Typeless"), true ) )
        return cgBufferFormat::D24_UNorm_X8_Typeless;
    else if ( !strFormat.compare( _T("D16"), true ) )
        return cgBufferFormat::D16;
    else if ( !strFormat.compare( _T("DF16"), true ) )
        return cgBufferFormat::DF16;
    else if ( !strFormat.compare( _T("DF24"), true ) )
        return cgBufferFormat::DF24;
    else if ( !strFormat.compare( _T("INTZ"), true ) )
        return cgBufferFormat::INTZ;
    else if ( !strFormat.compare( _T("RAWZ"), true ) )
        return cgBufferFormat::RAWZ;
    
    // Format not recognized
    return cgBufferFormat::Unknown;
}

//-----------------------------------------------------------------------------
//  Name : formatIsCompressed() (Static)
/// <summary>
/// Utility function to determine if the specified image format is a
/// compressed texture format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::formatIsCompressed( cgBufferFormat::Base Format )
{
    // Switch based on the requested format
    switch ( Format )
    {
        case cgBufferFormat::BC1_Typeless:
        case cgBufferFormat::BC1:
        case cgBufferFormat::BC1_SRGB:
        case cgBufferFormat::BC2_Typeless:
        case cgBufferFormat::BC2:
        case cgBufferFormat::BC2_SRGB:
        case cgBufferFormat::BC3_Typeless:
        case cgBufferFormat::BC3:
        case cgBufferFormat::BC3_SRGB:
        case cgBufferFormat::BC4_Typeless:
        case cgBufferFormat::BC4:
        case cgBufferFormat::BC4_Signed:
        case cgBufferFormat::BC5_Typeless:
        case cgBufferFormat::BC5:
        case cgBufferFormat::BC5_Signed:
        case cgBufferFormat::BC6H_Typeless:
        case cgBufferFormat::BC6H_UF16:
        case cgBufferFormat::BC6H_SF16:
        case cgBufferFormat::BC7_Typeless:
        case cgBufferFormat::BC7:
        case cgBufferFormat::BC7_SRGB:
            return true;

    } // End format switch

    // Format not compressed
    return false;
}

//-----------------------------------------------------------------------------
// Name : formatChannelCount() (Static)
// Desc : Utility function to return the number of channels available for
//        a valid format enum item.
// Note : Count is based on sampleable channels (i.e., those that could be
//        directly read in a shader)
//-----------------------------------------------------------------------------
cgUInt32 cgBufferFormatEnum::formatChannelCount( cgBufferFormat::Base Format )
{
    switch ( Format )
    {
		// Four Channel Formats
        case cgBufferFormat::R32G32B32A32_Typeless:
        case cgBufferFormat::R32G32B32A32_Float:
        case cgBufferFormat::R32G32B32A32_UInt:
        case cgBufferFormat::R32G32B32A32_SInt:
		case cgBufferFormat::R16G16B16A16_Typeless:
        case cgBufferFormat::R16G16B16A16_Float:
        case cgBufferFormat::R16G16B16A16:
        case cgBufferFormat::R16G16B16A16_UInt:
		case cgBufferFormat::R16G16B16A16_Signed:
        case cgBufferFormat::R16G16B16A16_SInt:
		case cgBufferFormat::R10G10B10A2_Typeless:
		case cgBufferFormat::R10G10B10A2:
		case cgBufferFormat::R10G10B10A2_UInt:
        case cgBufferFormat::B8G8R8A8_Typeless:
        case cgBufferFormat::B8G8R8A8_SRGB:
		case cgBufferFormat::R8G8B8A8_Typeless:
		case cgBufferFormat::R8G8B8A8:
		case cgBufferFormat::R8G8B8A8_SRGB:
		case cgBufferFormat::R8G8B8A8_UInt:
        case cgBufferFormat::R8G8B8A8_Signed:
        case cgBufferFormat::R8G8B8A8_SInt:
		case cgBufferFormat::R9G9B9E5_SharedExp:
        case cgBufferFormat::R8G8_B8G8:
        case cgBufferFormat::G8R8_G8B8:
		case cgBufferFormat::BC1_Typeless:
        case cgBufferFormat::BC1:
        case cgBufferFormat::BC1_SRGB:
        case cgBufferFormat::BC2_Typeless:
        case cgBufferFormat::BC2:
        case cgBufferFormat::BC2_SRGB:
        case cgBufferFormat::BC3_Typeless:
        case cgBufferFormat::BC3:
        case cgBufferFormat::BC3_SRGB:
        case cgBufferFormat::BC4_Typeless:
        case cgBufferFormat::BC4:
        case cgBufferFormat::BC4_Signed:
        case cgBufferFormat::BC5_Typeless:
        case cgBufferFormat::BC5:
        case cgBufferFormat::BC5_Signed:
		case cgBufferFormat::BC6H_Typeless:
        case cgBufferFormat::BC6H_UF16:
        case cgBufferFormat::BC6H_SF16:
        case cgBufferFormat::BC7_Typeless:
        case cgBufferFormat::BC7:
        case cgBufferFormat::BC7_SRGB:
        case cgBufferFormat::B5G5R5A1:
        case cgBufferFormat::B8G8R8A8:
			return 4;

		// Three Channel Formats
		case cgBufferFormat::R32G32B32_Typeless:
        case cgBufferFormat::R32G32B32_Float:
        case cgBufferFormat::R32G32B32_UInt:
        case cgBufferFormat::R32G32B32_SInt:
		case cgBufferFormat::R11G11B10_Float:
		case cgBufferFormat::B5G6R5:
        case cgBufferFormat::B8G8R8X8:
        case cgBufferFormat::B8G8R8X8_Typeless:
        case cgBufferFormat::B8G8R8X8_SRGB:
        case cgBufferFormat::B8G8R8:
        case cgBufferFormat::B5G5R5X1:
			return 3;

		// Two Channel Formats
		case cgBufferFormat::R32G32_Typeless:
        case cgBufferFormat::R32G32_Float:
        case cgBufferFormat::R32G32_UInt:
        case cgBufferFormat::R32G32_SInt:
		case cgBufferFormat::R32G8X24_Typeless:
		case cgBufferFormat::R16G16_Typeless:
        case cgBufferFormat::R16G16_Float:
        case cgBufferFormat::R16G16:
        case cgBufferFormat::R16G16_UInt:
        case cgBufferFormat::R16G16_Signed:
        case cgBufferFormat::R16G16_SInt:
		case cgBufferFormat::R24G8_Typeless:
		case cgBufferFormat::R8G8_Typeless:
        case cgBufferFormat::R8G8:
        case cgBufferFormat::R8G8_UInt:
        case cgBufferFormat::R8G8_Signed:
        case cgBufferFormat::R8G8_SInt:
			return 2;

		// One Channel Formats
		case cgBufferFormat::R32_Typeless:
        case cgBufferFormat::R32_Float:
        case cgBufferFormat::R32_UInt:
        case cgBufferFormat::R32_SInt:
		case cgBufferFormat::R16_Typeless:
        case cgBufferFormat::R16_Float:
        case cgBufferFormat::R16:
        case cgBufferFormat::R16_UInt:
        case cgBufferFormat::R16_Signed:
        case cgBufferFormat::R16_SInt:
        case cgBufferFormat::R8_Typeless:
        case cgBufferFormat::R8:
        case cgBufferFormat::R8_UInt:
        case cgBufferFormat::R8_Signed:
        case cgBufferFormat::R8_SInt:
        case cgBufferFormat::A8:
        case cgBufferFormat::R1:
		case cgBufferFormat::R24_UNorm_X8_Typeless:
        case cgBufferFormat::X24_Typeless_G8_UInt:
        case cgBufferFormat::D32_Float:
        case cgBufferFormat::D24_UNorm_X8_Typeless:
        case cgBufferFormat::R32_Float_X8X24_Typeless:
        case cgBufferFormat::X32_Typeless_G8X24_UInt:
        case cgBufferFormat::D32_Float_S8X24_UInt:
        case cgBufferFormat::D24_UNorm_S8_UInt:
        case cgBufferFormat::D24_Float_S8_UInt:
        case cgBufferFormat::D16:
        case cgBufferFormat::DF16:
        case cgBufferFormat::DF24:
        case cgBufferFormat::INTZ:
        case cgBufferFormat::RAWZ:
			return 1;

    } // End format switch

    // Unknown format
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : getBestDepthFormat () (Overload)
/// <summary>
/// Search for the ideal standard depth / depth stencil format given the
/// required options. The concept of 'best' format is somewhat dependant on the
/// usage scenario, but this method will return the correct format in most
/// standard cases.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgBufferFormatEnum::getBestDepthFormat( bool bFloatingPoint, bool bRequireStencil ) const
{
    cgUInt32 nFlags = 0;
    if ( bFloatingPoint )
        nFlags |= cgFormatSearchFlags::FloatingPoint;
    if ( bRequireStencil )
        nFlags |= cgFormatSearchFlags::RequireStencil;
    return getBestFormat( cgBufferType::DepthStencil, nFlags );
}

//-----------------------------------------------------------------------------
//  Name : getBestOneChannelFormat () (Overload)
/// <summary>
/// Search for the ideal standard single channel format given the required 
/// options. The concept of 'best' format is somewhat dependant on the usage
/// scenario, but this method will return the correct format in most standard
/// cases.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgBufferFormatEnum::getBestOneChannelFormat( cgBufferType::Base Type, bool bFloatingPoint, bool bRequireAlpha, bool bPreferCompressed, bool bAllowPadding ) const
{
    cgUInt32 nFlags = cgFormatSearchFlags::OneChannel;
    if ( bFloatingPoint )
        nFlags |= cgFormatSearchFlags::FloatingPoint;
    if ( bRequireAlpha )
        nFlags |= cgFormatSearchFlags::RequireAlpha;
    if ( bPreferCompressed )
        nFlags |= cgFormatSearchFlags::PreferCompressed;
    if ( bAllowPadding )
        nFlags |= cgFormatSearchFlags::AllowPaddingChannels;
    return getBestFormat( Type, nFlags );
}

//-----------------------------------------------------------------------------
//  Name : getBestTwoChannelFormat () (Overload)
/// <summary>
/// Search for the ideal standard two channel format given the required 
/// options. The concept of 'best' format is somewhat dependant on the usage
/// scenario, but this method will return the correct format in most standard
/// cases.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgBufferFormatEnum::getBestTwoChannelFormat( cgBufferType::Base Type, bool bFloatingPoint, bool bRequireAlpha, bool bPreferCompressed, bool bAllowPadding ) const
{
    cgUInt32 nFlags = cgFormatSearchFlags::TwoChannels;
    if ( bFloatingPoint )
        nFlags |= cgFormatSearchFlags::FloatingPoint;
    if ( bRequireAlpha )
        nFlags |= cgFormatSearchFlags::RequireAlpha;
    if ( bPreferCompressed )
        nFlags |= cgFormatSearchFlags::PreferCompressed;
    if ( bAllowPadding )
        nFlags |= cgFormatSearchFlags::AllowPaddingChannels;
    return getBestFormat( Type, nFlags );
}

//-----------------------------------------------------------------------------
//  Name : getBestFourChannelFormat () (Overload)
/// <summary>
/// Search for the ideal standard four channel format given the required 
/// options. The concept of 'best' format is somewhat dependant on the usage
/// scenario, but this method will return the correct format in most standard
/// cases.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgBufferFormatEnum::getBestFourChannelFormat( cgBufferType::Base Type, bool bFloatingPoint, bool bRequireAlpha, bool bPreferCompressed ) const
{
    cgUInt32 nFlags = cgFormatSearchFlags::FourChannels;
    if ( bFloatingPoint )
        nFlags |= cgFormatSearchFlags::FloatingPoint;
    if ( bRequireAlpha )
        nFlags |= cgFormatSearchFlags::RequireAlpha;
    if ( bPreferCompressed )
        nFlags |= cgFormatSearchFlags::PreferCompressed;
    return getBestFormat( Type, nFlags );
}

//-----------------------------------------------------------------------------
//  Name : isFormatSupported ()
/// <summary>
/// Returns status of whether or not this format is supported at all 
/// (irrespective of its physical capabilities).
/// </summary>
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::isFormatSupported( cgBufferType::Base Type, cgBufferFormat::Base Format ) const
{
    FormatEnumMap::const_iterator Item;
    const FormatEnumMap * pMap = CG_NULL;

    // How is it to be used?
    switch ( Type )
    {
        case cgBufferType::Texture1D:
            pMap = &mTexture1D;
            break;
        
        case cgBufferType::Video:
        case cgBufferType::Texture2D:
            pMap = &mTexture2D;
            break;
        
        case cgBufferType::Texture3D:
            pMap = &mTexture3D;
            break;

        case cgBufferType::TextureCube:
            pMap = &mTextureCube;
            break;

        case cgBufferType::RenderTarget:
        case cgBufferType::RenderTargetCube:
            pMap = &mRenderTarget;
            break;

        case cgBufferType::DepthStencil:
        case cgBufferType::ShadowMap:
            pMap = &mDepthStencil;
            break;

    } // End Switch Type

    // Get enumerated format
    Item = pMap->find( Format );

    // We don't support the storage of this format.
    if ( Item == pMap->end() )
        return false;

    // Item has any capabilities?
    return (Item->second != 0);
}

//-----------------------------------------------------------------------------
//  Name : getFormatCaps ()
/// <summary>
/// Return the capabilities exposed by the hardware for the specified buffer
/// type and format combination.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBufferFormatEnum::getFormatCaps( cgBufferType::Base Type, cgBufferFormat::Base Format ) const
{
    FormatEnumMap::const_iterator Item;
    const FormatEnumMap * pMap = CG_NULL;

    // How is it to be used?
    switch ( Type )
    {
        case cgBufferType::Texture1D:
            pMap = &mTexture1D;
            break;

        case cgBufferType::Video:
        case cgBufferType::Texture2D:
            pMap = &mTexture2D;
            break;

        case cgBufferType::Texture3D:
            pMap = &mTexture3D;
            break;

        case cgBufferType::TextureCube:
            pMap = &mTextureCube;
            break;

        case cgBufferType::RenderTarget:
        case cgBufferType::RenderTargetCube:
            pMap = &mRenderTarget;
            break;

        case cgBufferType::DepthStencil:
        case cgBufferType::ShadowMap:
            pMap = &mDepthStencil;
            break;

    } // End Switch Type

    // Get enumerated format
    Item = pMap->find( Format );

    // We don't support the storage of this format.
    if ( Item == pMap->end() )
        return 0;

    // Return capabilities.
    return Item->second;
}

/*//-----------------------------------------------------------------------------
//  Name : SetFormatSupported ()
/// <summary>
/// Forcably set / remove a format's supported flag.
/// </summary>
//-----------------------------------------------------------------------------
void cgBufferFormatEnum::SetFormatSupported( cgBufferType::Base Type, cgBufferFormat::Base Format, bool Supported )
{
    FormatEnumMap::iterator Item;
    FormatEnumMap * pMap = CG_NULL;

    // How is it to be used?
    switch ( Type )
    {
        case cgBufferType::Texture1D:
            pMap = &mTexture1D;
            break;
        
        case cgBufferType::Video:
        case cgBufferType::Texture2D:
            pMap = &mTexture2D;
            break;
        
        case cgBufferType::Texture3D:
            pMap = &mTexture3D;
            break;

        case cgBufferType::TextureCube:
            pMap = &mTextureCube;
            break;

        case cgBufferType::RenderTarget:
        case cgBufferType::RenderTargetCube:
            pMap = &mRenderTarget;
            break;

        case cgBufferType::DepthStencil:
        case cgBufferType::ShadowMap:
            pMap = &mDepthStencil;
            break;

    } // End Switch Type

    // Get enumerated format.
    Item = pMap->find( Format );

    // We don't support the storage of this format.
    if ( Item == pMap->end() )
        return;

    // Update the item (uses this method because we have already had to perform 
    // a 'find'. It saves us a little time over using the indexing operator
    // to find the key again).
    Item->second = Supported;
}*/

/*//-----------------------------------------------------------------------------
//  Name : SetLinearSupported ()
/// <summary>
/// Stores the linear filtering support status for a given format
/// </summary>
//-----------------------------------------------------------------------------
void cgBufferFormatEnum::SetLinearSupported( cgBufferFormat::Base Format, bool Supported )
{
	m_LinearFiltering[ Format ] = Supported;
}

//-----------------------------------------------------------------------------
//  Name : GetLinearSupported ()
/// <summary>
/// Return whether or not linear filtering is available for the requested format
/// </summary>
//-----------------------------------------------------------------------------
bool cgBufferFormatEnum::GetLinearSupported( cgBufferFormat::Base Format ) const
{
	FormatEnumMap::const_iterator itTable = m_LinearFiltering.find( Format );
	if ( itTable == m_LinearFiltering.end() )
		return false;
	return itTable->second;
}
*/