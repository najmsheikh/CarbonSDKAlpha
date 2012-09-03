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
// Name : cgDX11RenderTarget.cpp                                             //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data (DX11 implementation).                                        //
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
// cgDX11RenderTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11RenderTarget.h>
#include <Resources/Platform/cgDX11BufferFormatEnum.h>
#include <Resources/cgResourceManager.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11RenderTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderTarget () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderTarget::cgDX11RenderTarget( cgUInt32 nReferenceId, const cgImageInfo & Info ) : 
    cgDX11Texture<cgRenderTarget>( nReferenceId, Info )
{
    // Initialize variables to sensible defaults.
    mView = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11RenderTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RenderTarget::~cgDX11RenderTarget( )
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
void cgDX11RenderTarget::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // In order to ensure that the render target's overriden 'releaseTexture()' method 
    // is called correctly during class destruction (it is a virtual method) we call it 
    // in advance here, even though the base class 'dispose()' method will potentially
    // call it again during a non-destruction call.
    releaseTexture();

    // Dispose base(s).
    if ( bDisposeBase )
        cgDX11Texture<cgRenderTarget>::dispose( true );
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
bool cgDX11RenderTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11RenderTargetResource )
        return true;

    // Supported by base?
    return cgDX11Texture<cgRenderTarget>::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allows the texture to update if it is decoding media perhaps.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11RenderTarget::update()
{
    // Call base class implementation
    cgDX11Texture<cgRenderTarget>::update();
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11RenderTarget::createTexture( )
{
    // Enforce fixed surface details. Remember that cgRenderTarget constructor 
    // forces invalid buffer type to cgBufferType::RenderTarget.
    mInfo.autoDetectFormat = false;
    mInfo.pool             = cgMemoryPool::Default;

    // Call base class implementation first
    if ( !cgDX11Texture<cgRenderTarget>::createTexture() )
        return false;

    // Create view of this resource as a render target.
    if ( !mView )
    {
        // Build the view description.
        D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
        memset( &rtDesc, 0, sizeof(D3D11_RENDER_TARGET_VIEW_DESC) );
        rtDesc.Format = (DXGI_FORMAT)cgDX11BufferFormatEnum::formatToNative(mInfo.format);
        
        // Cube map or regular 2D?
        if ( mInfo.type == cgBufferType::RenderTargetCube )
        {
            if ( mInfo.multiSampleType != cgMultiSampleType::None )
            {
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtDesc.Texture2DMSArray.ArraySize = 6;    
            
            } // End if Multisampling
            else
            {
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtDesc.Texture2DMSArray.ArraySize = 6;    
            
            } // End if !Multisampling

        } // End if Cube
        else if ( mInfo.type == cgBufferType::DeviceBackBuffer )
        {
            rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        } // End if DeviceBackBuffer
        else
        {
            if ( mInfo.multiSampleType != cgMultiSampleType::None )
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            else
                rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        } // End if standard 2D

        // Retrieve D3D device for view creation (required DX11 class driver)
        ID3D11Device * pDevice = CG_NULL;
        cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
        if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
            return false;

        // Create the actual view
        if ( mInfo.type == cgBufferType::DeviceBackBuffer )
        {
            ID3D11Resource * pBackBuffer = pDriver->getD3DBackBuffer( );
            if ( FAILED( pDevice->CreateRenderTargetView( pBackBuffer, &rtDesc, &mView ) ) )
            {
                pDevice->Release();
                pBackBuffer->Release();
                releaseTexture();
                return false;
            
            } // End if failed
            pBackBuffer->Release();

        } // End if DeviceBackBuffer
        else
        {
            if ( FAILED( pDevice->CreateRenderTargetView( mTexture, &rtDesc, &mView ) ) )
            {
                pDevice->Release();
                releaseTexture();
                return false;
            
            } // End if failed
        
        } // End if !DeviceBackBuffer

        // Clean up.
        pDevice->Release();

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
void cgDX11RenderTarget::releaseTexture( )
{
    // Release view of this resource.
    if ( mView )
        mView->Release();
    mView = CG_NULL;

    // Call base class implementation first.
    cgDX11Texture<cgRenderTarget>::releaseTexture();
}

//-----------------------------------------------------------------------------
//  Name : getD3DTargetView ()
/// <summary>
/// Retrieve an underlying D3D resource view for this render target.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11RenderTargetView * cgDX11RenderTarget::getD3DTargetView( ) const
{
    if ( mView )
        mView->AddRef();
    return mView;
}

#endif // CGE_DX11_RENDER_SUPPORT