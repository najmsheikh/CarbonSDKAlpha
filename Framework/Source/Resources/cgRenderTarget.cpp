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
// Name : cgRenderTarget.cpp                                                 //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data.                                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRenderTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgRenderTarget.h>
#include <Resources/Platform/cgDX9RenderTarget.h>
#include <Resources/Platform/cgDX11RenderTarget.h>

///////////////////////////////////////////////////////////////////////////////
// cgRenderTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRenderTarget () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderTarget::cgRenderTarget( cgUInt32 nReferenceId, const cgImageInfo & Info ) : 
    cgTexture( nReferenceId, Info )
{
    // Valid buffer type is either RenderTarget, CubeRenderTarget or
    // DeviceBackBuffer. If it is not, force it to 'RenderTarget' (default).
    if ( mInfo.type != cgBufferType::RenderTarget && mInfo.type != cgBufferType::RenderTargetCube &&
        mInfo.type != cgBufferType::DeviceBackBuffer )
        mInfo.type = cgBufferType::RenderTarget;

    // Cached responses
    mResourceType  = cgResourceType::RenderTarget;
    mCanEvict     = false;

    // Render targets start their life in a 'lost' state.
    mResourceLost = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgRenderTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderTarget::~cgRenderTarget( )
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
cgRenderTarget * cgRenderTarget::createInstance( cgUInt32 nReferenceId, const cgImageInfo & Info )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9RenderTarget( nReferenceId, Info );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11RenderTarget( nReferenceId, Info );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRenderTarget::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in this implementation (UnloadResource()
    // will eventually be called by base class).

    // Dispose base(s).
    if ( bDisposeBase )
        cgTexture::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_RenderTargetResource )
        return true;

    // Supported by base?
    return cgTexture::queryReferenceType( type );
}