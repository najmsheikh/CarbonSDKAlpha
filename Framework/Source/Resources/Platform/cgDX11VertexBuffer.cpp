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
// Name : cgDX11VertexBuffer.cpp                                             //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
//        buffer resource data (DX11 implementation).                        //
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
// cgDX11VertexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11VertexBuffer.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <D3D11.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11VertexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11VertexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexBuffer::cgDX11VertexBuffer( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgVertexFormat * pFormat, cgMemoryPool::Base Pool ) :
    cgVertexBuffer( nReferenceId, Length, Usage, pFormat, Pool )
{
    // Initialize variables to sensible defaults
    mVertexBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11VertexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11VertexBuffer::~cgDX11VertexBuffer( )
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
void cgDX11VertexBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase )
        cgVertexBuffer::dispose( true );
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
bool cgDX11VertexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11VertexBufferResource )
        return true;

    // Supported by base?
    return cgVertexBuffer::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DBuffer ()
/// <summary>
/// Retrieve the internal D3D11 vertex buffer object.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11Buffer * cgDX11VertexBuffer::getD3DBuffer( ) const
{
    // Add reference, we're returning a pointer
    if ( mVertexBuffer )
        mVertexBuffer->AddRef();

    // Return the resource item
    return mVertexBuffer;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexBuffer::deviceLost()
{
    // 7777 - Is this even necessary any more?
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseVertexBuffer();

    // Resource data is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexBuffer::deviceRestored()
{
    // 7777 - Is this even necessary any more?
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createVertexBuffer();
}

//-----------------------------------------------------------------------------
//  Name : releaseVertexBuffer ()
/// <summary>
/// Release the internal vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexBuffer::releaseVertexBuffer( )
{
    // Unlock if we are still locked
    unlock();

    // We should release our underlying resource
    if ( mVertexBuffer )
        mVertexBuffer->Release();
    mVertexBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createVertexBuffer ()
/// <summary>
/// Create the internal vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexBuffer::createVertexBuffer( )
{
    // Already loaded?
    if ( mVertexBuffer )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for VB creation (required DX11 class driver)
    ID3D11Device * pDevice;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Populate the D3D buffer description buffer ready for creation.
    D3D11_BUFFER_DESC bDesc;
    memset( &bDesc, 0, sizeof(D3D11_BUFFER_DESC) );
    bDesc.Usage          = (mUsage & cgBufferUsage::Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bDesc.ByteWidth      = mLength;
    bDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    
    // Set dynamic resource CPU access flags.
    if ( mUsage & cgBufferUsage::Dynamic )
    {
        bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if ( !(mUsage & cgBufferUsage::WriteOnly) )
            bDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    
    } // End if dynamic
        
    // Create the vertex buffer
    if ( FAILED(pDevice->CreateBuffer( &bDesc, CG_NULL, &mVertexBuffer ) ) )
    {
        // Cleanup
        pDevice->Release();
        return false;

    } // End if failed to create

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : updateBuffer ()
/// <summary>
/// Populate the specified region of the buffer with new data. For optimal
/// performance it is recommended that the entire buffer is replaced where
/// possible. Specifying a SrcSize of 0 is equivalent to specifying a region 
/// starting at DstOffset up to the end of the buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexBuffer::updateBuffer( cgUInt32 DstOffset, cgUInt32 SrcSize, void * pSrcData )
{
    // Cannot update the resource if it is not loaded, or is already locked elsewhere
    if ( !mVertexBuffer || mLocked )
        return CG_NULL;

    // If SrcSize is 0, update entire buffer from the offset onewards.
    if ( !SrcSize )
        SrcSize = mLength - DstOffset;

    // Retrieve D3D device context for VB mapping (required DX11 class driver)
    ID3D11DeviceContext * pDeviceContext;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDeviceContext = pDriver->getD3DDeviceContext()) )
        return false;

    // If this is a dynamic buffer, we have to use a lock / map strategy.
    // Otherwise we can populate it with UpdateSubresource.
    if ( mUsage & cgBufferUsage::Dynamic )
    {
        // Can we reliably discard the data in the existing buffer?
        D3D11_MAP MapType = D3D11_MAP_WRITE;
        if ( !DstOffset && SrcSize == mLength )
            MapType = D3D11_MAP_WRITE_DISCARD;

        // Lock the vertex buffer
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        if ( FAILED( pDeviceContext->Map( mVertexBuffer, 0, MapType, 0, &MappedResource ) ) )
        {
            pDeviceContext->Release();
            return false;
        
        } // End if failed

        // Copy the data in and unlock.
        memcpy( MappedResource.pData, pSrcData, SrcSize );
        pDeviceContext->Unmap( mVertexBuffer, 0 );

    } // End if dynamic
    else
    {
        // Construct the update region information.
        D3D11_BOX Region;
        Region.left   = DstOffset;
        Region.right  = DstOffset + SrcSize;
        Region.top    = 0;
        Region.bottom = 1;
        Region.front  = 0;
        Region.back   = 1;

        // Trigger the resource update.
        pDeviceContext->UpdateSubresource( mVertexBuffer, 0, &Region, pSrcData, SrcSize, SrcSize );

    } // End if default

    // We're done.
    pDeviceContext->Release();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : lock ()
/// <summary>
/// Lock the specified area of the vertex buffer and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
void * cgDX11VertexBuffer::lock( cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mVertexBuffer || mLocked )
        return CG_NULL;

    // Cannot lock a resource unless it was marked as dynamic.
    if ( !(mUsage & cgBufferUsage::Dynamic) )
        return CG_NULL;

    // Retrieve D3D device context for VB mapping (required DX11 class driver)
    ID3D11DeviceContext * pDeviceContext;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDeviceContext = pDriver->getD3DDeviceContext()) )
        return CG_NULL;

    // Generate the D3D11 compatible lock / map operation.
    D3D11_MAP MapType;
    if ( Flags & cgLockFlags::ReadOnly )
        MapType = D3D11_MAP_READ;
    else if ( Flags & cgLockFlags::WriteOnly )
    {
        MapType = D3D11_MAP_WRITE;
        if ( Flags & cgLockFlags::Discard )
            MapType = D3D11_MAP_WRITE_DISCARD;
        else if ( Flags & cgLockFlags::NoOverwrite )
            MapType = D3D11_MAP_WRITE_NO_OVERWRITE;
    
    } // End if write only
    else
        MapType = D3D11_MAP_READ_WRITE;

    // And the lock / map flags
    cgUInt32 MapFlags = 0;
    if ( Flags & cgLockFlags::DoNotWait )
        MapFlags |= D3D11_MAP_FLAG_DO_NOT_WAIT;

    // Lock the vertex buffer
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    if ( FAILED( pDeviceContext->Map( mVertexBuffer, 0, MapType, MapFlags, &MappedResource ) ) )
    {
        pDeviceContext->Release();
        return CG_NULL;
    
    } // End if failed

    // Clean up
    pDeviceContext->Release();

    // Store lock data.
    mLockedBuffer = MappedResource.pData;
    mLocked       = true;

    // Adjust the offset of the locked data to match their requested region.
    mLockedBuffer = ((cgByte*)mLockedBuffer) + OffsetToLock;
    
    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : unlock ()
/// <summary>
/// Unlock the buffer if previously locked.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11VertexBuffer::unlock( )
{
    // Cannot unlock if it was not already locked
    if ( !mVertexBuffer || !mLocked )
        return;

    // Validate requirements
    if ( !mManager )
        return;

    // Retrieve D3D device context for VB unmapping (required DX11 class driver)
    ID3D11DeviceContext * pDeviceContext;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDeviceContext = pDriver->getD3DDeviceContext()) )
        return;

    // Unmap
    pDeviceContext->Unmap( mVertexBuffer, 0 );

    // Clean up
    pDeviceContext->Release();

    // Item is no longer locked
    mLocked       = false;
    mLockedBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexBuffer::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;
    
    // Attempt to create the vertex buffer
    if ( !createVertexBuffer( ) )
        return false;

    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11VertexBuffer::unloadResource( )
{
    // Release the vertex buffer
    releaseVertexBuffer();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT