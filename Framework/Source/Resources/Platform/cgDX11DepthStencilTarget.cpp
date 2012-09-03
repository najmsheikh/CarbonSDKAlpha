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
// Name : cgDX11DepthStencilTarget.cpp                                       //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//        (DX11 implementation).                                             //
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
// cgDX11DepthStencilTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11DepthStencilTarget.h>
#include <Resources/Platform/cgDX11BufferFormatEnum.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11DepthStencilTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11DepthStencilTarget () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11DepthStencilTarget::cgDX11DepthStencilTarget( cgUInt32 nReferenceId, const cgImageInfo & description ) : cgDX11Texture<cgDepthStencilTarget>( nReferenceId, description )
{
    // Initialize variables to sensible defaults
    mView = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11DepthStencilTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11DepthStencilTarget::~cgDX11DepthStencilTarget( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX11DepthStencilTarget::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // In order to ensure that the depth stencil target's overridden 'releaseTexture()'
    // method  is called correctly during class destruction (it is a virtual method) we
    // call it in advance here, even though the base class 'dispose()' method will
    // potentially call it again during a non-destruction call.
    releaseTexture();

    // Dispose base(s).
    if ( disposeBase == true )
        cgDX11Texture<cgDepthStencilTarget>::dispose( true );
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
bool cgDX11DepthStencilTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11DepthStencilTargetResource )
        return true;

    // Supported by base?
    return cgDX11Texture<cgDepthStencilTarget>::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11DepthStencilTarget::createTexture( )
{
    // Enforce fixed surface details. Remember that cgDepthStencilTarget
    // constructor forces invalid buffer type to cgBufferType::DepthStencil.
    mInfo.autoGenerateMipmaps = false;
    mInfo.autoDetectFormat    = false;
    mInfo.depth               = 1;
    mInfo.mipLevels           = 1;
    mInfo.pool                = cgMemoryPool::Default;

    // Call base class implementation first
    if ( !cgDX11Texture<cgDepthStencilTarget>::createTexture() )
        return false;

    // Create view of this resource as a render target.
    if ( !mView )
    {
        // Build the view description.
        D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
        memset( &viewDesc, 0, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );
        viewDesc.Format = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(mInfo.format);
        if ( mInfo.multiSampleType != cgMultiSampleType::None )
            viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        else
            viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        // Retrieve D3D device for view creation (required DX11 class driver)
        ID3D11Device * device = CG_NULL;
        cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
        if ( !driver || !(device = driver->getD3DDevice()) )
            return false;

        // Create the actual view
        if ( FAILED( device->CreateDepthStencilView( mTexture, &viewDesc, &mView ) ) )
        {
            device->Release();
            releaseTexture();
            return false;
        
        } // End if failed

        // Clean up
        device->Release();

    } // End if !exists

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : releaseTexture () (Protected, Virtual)
/// <summary>
/// Release the internal texture data.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11DepthStencilTarget::releaseTexture( )
{
    // Release view of this resource.
    if ( mView )
        mView->Release();
    mView = CG_NULL;

    // Call base class implementation first.
    cgDX11Texture<cgDepthStencilTarget>::releaseTexture();
}

//-----------------------------------------------------------------------------
//  Name : getD3DTargetView ()
/// <summary>
/// Retrieve an underlying D3D resource view for this depth stencil target.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11DepthStencilView * cgDX11DepthStencilTarget::getD3DTargetView( ) const
{
    if ( mView )
        mView->AddRef();
    return mView;
}

#endif // CGE_DX11_RENDER_SUPPORT