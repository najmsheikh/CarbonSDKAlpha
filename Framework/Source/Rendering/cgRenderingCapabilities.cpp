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
// Name : cgRenderingCapabilities.cpp                                        //
//                                                                           //
// Desc : Base interface through which rendering capabilities can be queried //
//        Tested capabilities include supported buffer formats, shader model //
//        support, format filtering support and so on.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRenderingCapabilities Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgRenderingCapabilities.h>
#include <Resources/cgBufferFormatEnum.h>

// Platform specific implementations
#include <Rendering/Platform/cgDX9RenderingCapabilities.h>
#include <Rendering/Platform/cgDX11RenderingCapabilities.h>

///////////////////////////////////////////////////////////////////////////////
// cgRenderingCapabilities Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRenderingCapabilities () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderingCapabilities::cgRenderingCapabilities( cgRenderDriver * pDriver )
{
    // Initialize variables to sensible defaults
    mDriver           = pDriver;
    mBufferFormats    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgRenderingCapabilities () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderingCapabilities::~cgRenderingCapabilities( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderingCapabilities * cgRenderingCapabilities::createInstance( cgRenderDriver * pDriver )
{
    // Determine which driver we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9RenderingCapabilities( pDriver );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11RenderingCapabilities( pDriver );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : enumerate ()
/// <summary>
/// Run the capabilities enumeration for the specified device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderingCapabilities::enumerate( )
{
    // If we're already enumerated, just return.
    if ( mBufferFormats )
        return true;

    // Enumerate buffer formats the easy way :)
    mBufferFormats = cgBufferFormatEnum::createInstance();
    if ( !mBufferFormats->enumerate( mDriver ) )
    {
        dispose( true );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRenderingCapabilities::dispose( bool bDisposeBase )
{
    delete mBufferFormats;
    mBufferFormats = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getBufferFormats ()
/// <summary>
/// Retrieve information about the supported texture / surface formats
/// supported by this device.
/// </summary>
//-----------------------------------------------------------------------------
const cgBufferFormatEnum & cgRenderingCapabilities::getBufferFormats( ) const
{
    return *mBufferFormats;
}