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
// Name : cgDX9RenderTarget.cpp                                              //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to render target (color buffer) resource    //
//        data (DX9 implementation).                                         //
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
// cgDX9RenderTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9RenderTarget.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9RenderTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9RenderTarget () (Overload Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderTarget::cgDX9RenderTarget( cgUInt32 nReferenceId, const cgImageInfo & Info ) : 
    cgDX9Texture<cgRenderTarget>( nReferenceId, Info )
{
    // Initialize variables to sensible defaults.
	mMSAADirty    = false;
    mMSAASurface  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9RenderTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RenderTarget::~cgDX9RenderTarget( )
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
void cgDX9RenderTarget::dispose( bool bDisposeBase )
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
        cgDX9Texture<cgRenderTarget>::dispose( true );
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
bool cgDX9RenderTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9RenderTargetResource )
        return true;

    // Supported by base?
    return cgDX9Texture<cgRenderTarget>::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allows the texture to update if it is decoding media perhaps.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderTarget::update()
{
    // If we are using a multisample surface, we need to transfer the data to our
    // non-multisampled surface for use as a texture.
    if ( mInfo.multiSampleType != cgMultiSampleType::None && mMSAADirty == true && 
         mInfo.type != cgBufferType::DeviceBackBuffer )
    {
        IDirect3DDevice9  * pDevice;
        LPDIRECT3DSURFACE9  pDstSurface;
        HRESULT             hRet = D3DERR_INVALIDCALL;

        // Retrieve the top level surface of the underlying render target texture.
        if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
            ((LPDIRECT3DCUBETEXTURE9)mTexture)->GetCubeMapSurface( (D3DCUBEMAP_FACES)mCurrentCubeFace, 0, &pDstSurface );
        else
            ((LPDIRECT3DTEXTURE9)mTexture)->GetSurfaceLevel( 0, &pDstSurface );

        // Validate
        if ( !pDstSurface )
            return;
            
        // Retrieve D3D device for manual 'StretchRect' (required DX9 class driver)
        cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
        if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        {
            pDstSurface->Release();
            return;
        
        } // End if no device
        
        // Copy MSAA surface into top level texture surface.
        hRet = pDevice->StretchRect( mMSAASurface, CG_NULL, pDstSurface, CG_NULL, (D3DTEXTUREFILTERTYPE)cgFilterMethod::None );

        // We're done with device data.
        pDstSurface->Release();
        pDevice->Release();

        // MSAA data has been resolved?
        if ( SUCCEEDED(hRet) )
        {
            // If automatic generation of mip maps is required, we should hint to the driver 
            // that we need new mip levels generating at this point.
            if ( mInfo.autoGenerateMipmaps )
                updateMipLevels();

            // Resolved!
            mMSAADirty = false;

        } // End if success

    } // End if multi-sample resolve required

    // Call base class implementation
    cgDX9Texture<cgRenderTarget>::update();
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RenderTarget::createTexture( )
{
    IDirect3DDevice9 * pDevice;
    HRESULT            hRet = D3DERR_INVALIDCALL;

    // Enforce fixed surface details. Remember that cgRenderTarget constructor 
    // forces invalid buffer type to cgBufferType::RenderTarget.
    mInfo.autoDetectFormat = false;
    mInfo.pool             = cgMemoryPool::Default;

    // Call base class implementation first
    if ( !cgDX9Texture<cgRenderTarget>::createTexture() )
        return false;

    // When using 'DeviceBackBuffer' target type, the actual D3D device
    // surface is retrieved on calls to 'getD3DTargetSurface()'. This
    // ensures that device double / triple buffering still continues to
    // function, but prevents use of render target as a physical texture.
    if ( mInfo.type == cgBufferType::DeviceBackBuffer )
        return true;

    // If multi-sampling was requested, we also need a proxy
    // multi-sample enabled surface into which the device will
    // render. This data will then be transferred to the physical
    // texture resource as and when necessary (MSAA resolve).
    if ( mInfo.multiSampleType == cgMultiSampleType::None )
        return true;
    
    // Retrieve D3D device for surface creation (required DX9 class driver)
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
        return false;

    // Create the surface (cgTexture::createTexture() will already have resolved
    // the scalar size -> pixel size appropriately).
    hRet = pDevice->CreateRenderTarget( mInfo.width, mInfo.height, 
                                       (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format), 
                                       (D3DMULTISAMPLE_TYPE)mInfo.multiSampleType, mInfo.multiSampleQuality, 
                                       false, &mMSAASurface, CG_NULL );
    // Release D3D Device
    pDevice->Release();

    // Success?
    if (FAILED(hRet))
        return false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : releaseTexture () (Protected, Virtual)
/// <summary>
/// Release the internal texture data.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RenderTarget::releaseTexture( )
{
    // Call base class implementation first.
    cgDX9Texture<cgRenderTarget>::releaseTexture();

    // Now release our own underlying resource
    if ( mMSAASurface != CG_NULL )
        mMSAASurface->Release();
    mMSAASurface = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getD3DTargetSurface ()
/// <summary>
/// Retrieve an underlying D3D surface for this render target.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DSurface9 * cgDX9RenderTarget::getD3DTargetSurface( bool bAutoUseMultiSample /* = true */ )
{
    LPDIRECT3DSURFACE9 pSurface = CG_NULL;

    // Cannot use multi-sample surface if this is device target
    if ( mInfo.type == cgBufferType::DeviceBackBuffer )
        bAutoUseMultiSample = false;

    // Override with multi-sample surface?
    if ( bAutoUseMultiSample && mInfo.multiSampleType != cgMultiSampleType::None )
    {
        mMSAASurface->AddRef();
        return mMSAASurface;
    
    } // End if MSAA surface

    // Target may be texture or surface, dereference as necessary.
    if ( mInfo.type == cgBufferType::TextureCube || mInfo.type == cgBufferType::RenderTargetCube )
    {
        LPDIRECT3DCUBETEXTURE9 pTexture = (LPDIRECT3DCUBETEXTURE9)mTexture;
        pTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)mCurrentCubeFace, 0, &pSurface );
        return pSurface;
    
    } // End if cube render target
    else if ( mInfo.type == cgBufferType::RenderTarget )
    {
        LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)mTexture;
        pTexture->GetSurfaceLevel( 0, &pSurface );
        return pSurface;

    } // End if 2D render target
    else if ( mInfo.type == cgBufferType::DeviceBackBuffer )
    {
        // Retrieve the surface from the D3D device (required DX9 class driver).
        IDirect3DDevice9 * pDevice;
        cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
        if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
            return CG_NULL;
        pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface );
        pDevice->Release();
        return pSurface;

    } // End if device frame buffer

    // Invalid target type.
    return CG_NULL;

}

#endif // CGE_DX9_RENDER_SUPPORT