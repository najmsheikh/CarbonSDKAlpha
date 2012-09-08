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
// Name : cgDX9DepthStencilTarget.cpp                                        //
//                                                                           //
// Desc : Contains classes responsible for managing, updating and granting   //
//        the application access to depth stencil surface resource data.     //
//        (DX9 implementation).                                              //
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
// cgDX9DepthStencilTarget Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9DepthStencilTarget.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Resources/Platform/cgDX9BufferFormatEnum.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9DepthStencilTarget Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9DepthStencilTarget () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9DepthStencilTarget::cgDX9DepthStencilTarget( cgUInt32 nReferenceId, const cgImageInfo & Info ) : cgDX9Texture<cgDepthStencilTarget>( nReferenceId, Info )
{
    // Initialize variables to sensible defaults
    mSurface = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9DepthStencilTarget () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9DepthStencilTarget::~cgDX9DepthStencilTarget( )
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
void cgDX9DepthStencilTarget::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // In order to ensure that the depth stencil target's overriden 'releaseTexture()'
    // method  is called correctly during class destruction (it is a virtual method) we
    // call it in advance here, even though the base class 'dispose()' method will
    // potentially call it again during a non-destruction call.
    releaseTexture();

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgDX9Texture<cgDepthStencilTarget>::dispose( true );
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
bool cgDX9DepthStencilTarget::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9DepthStencilTargetResource )
        return true;

    // Supported by base?
    return cgDX9Texture<cgDepthStencilTarget>::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : createTexture () (Protected, Virtual)
/// <summary>
/// Create the internal texture resource(s).
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9DepthStencilTarget::createTexture( )
{
    // ToDo: 9999 - Validate the INFO structure!

    // Already loaded?
    if ( mSurface || mTexture )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Enforce fixed surface details. Remember that cgDepthStencilTarget
    // constructor forces invalid buffer type to cgBufferType::DepthStencil.
    mInfo.autoGenerateMipmaps   = false;
    mInfo.autoDetectFormat      = false;
    mInfo.depth                 = 1;
    mInfo.mipLevels             = 1;
    mInfo.pool                  = cgMemoryPool::Default;

    // If this is a compatible 'texture' type (required for shadow maps)
    // then use the base class texture implementation.
    if ( mInfo.type == cgBufferType::ShadowMap || 
		mInfo.format == cgBufferFormat::DF16 || 
		mInfo.format == cgBufferFormat::DF24 || 
		mInfo.format == cgBufferFormat::INTZ || 
		mInfo.format == cgBufferFormat::RAWZ )
        return cgDX9Texture<cgDepthStencilTarget>::createTexture();

    // Otherwise, we do NOT call base class implementation for non texture
    // depth stencil targets. Only the surface created below is valid.

    // Retrieve D3D device for texture creation (required DX9 class driver)
    IDirect3DDevice9 * pDevice;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Create hardware depth stencil surface.
    HRESULT hRet = D3DERR_INVALIDCALL;
    hRet = pDevice->CreateDepthStencilSurface( mInfo.width, mInfo.height, 
                                              (D3DFORMAT)cgDX9BufferFormatEnum::formatToNative(mInfo.format), 
                                              (D3DMULTISAMPLE_TYPE)mInfo.multiSampleType, mInfo.multiSampleQuality,
                                              false, &mSurface, CG_NULL );

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success?
    return SUCCEEDED( hRet );
}

//-----------------------------------------------------------------------------
//  Name : releaseTexture () (Protected, Virtual)
/// <summary>
/// Release the internal texture data.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9DepthStencilTarget::releaseTexture( )
{
    // Call base class implementation first.
    cgDX9Texture<cgDepthStencilTarget>::releaseTexture();

    // We should release our underlying depth stencil surface 
    // resource if one was created.
    if ( mSurface )
        mSurface->Release();
    mSurface = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getD3DTargetSurface ()
/// <summary>
/// Retrieve an underlying D3D surface for this depth stencil target.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DSurface9 * cgDX9DepthStencilTarget::getD3DTargetSurface( )
{
    if ( mTexture )
    {
        // Depth stencil texture
        LPDIRECT3DSURFACE9 pSurface;
        LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)mTexture;
        pTexture->GetSurfaceLevel( 0, &pSurface );
        return pSurface;
    
    } // End if texture
    else if ( mSurface )
    {
        // Regular surface.
        mSurface->AddRef();
        return mSurface;
    
    } // End if surface

    // Invalid target
    return CG_NULL;
}

#endif // CGE_DX9_RENDER_SUPPORT