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
// Name : cgDepthStencilTarget.cpp                                           //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgDepthStencilTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/Platform/cgDX9DepthStencilTarget.h>
#include <Resources/Platform/cgDX11DepthStencilTarget.h>

///////////////////////////////////////////////////////////////////////////////
// cgDepthStencilTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDepthStencilTarget () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilTarget::cgDepthStencilTarget( cgUInt32 nReferenceId, const cgImageInfo & Info ) :
    cgTexture( nReferenceId, Info )
{
    // Enforce buffer type (default to DepthStencil).
    if ( mInfo.type != cgBufferType::DepthStencil &&
         mInfo.type != cgBufferType::ShadowMap )
         mInfo.type = cgBufferType::DepthStencil;

    // Setup initial cached responses
    mResourceType  = cgResourceType::DepthStencilTarget;
    mCanEvict     = false;

    // Depth stencil target's start their life in a 'lost' state.
    mResourceLost = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDepthStencilTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilTarget::~cgDepthStencilTarget( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : CreateInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilTarget * cgDepthStencilTarget::CreateInstance( cgUInt32 nReferenceId, const cgImageInfo & Info )
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
                return new cgDX9DepthStencilTarget( nReferenceId, Info );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11DepthStencilTarget( nReferenceId, Info );

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
void cgDepthStencilTarget::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in this implementation (UnloadResource()
    // will eventually be called by base class).

    // Dispose base(s).
    if ( bDisposeBase == true )
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
bool cgDepthStencilTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DepthStencilTargetResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}