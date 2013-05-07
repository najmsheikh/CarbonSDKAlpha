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
// Name : cgDX11IndexBuffer.h                                                //
//                                                                           //
// Desc : Contains classes responsible for loading and managing index        //
//        buffer resource data (DX11 implementation).                        //
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
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11IndexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11IndexBuffer.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <D3D11.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11IndexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11IndexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11IndexBuffer::cgDX11IndexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool ) :
    cgIndexBuffer( referenceId, length, usage, format, pool )
{
    // Initialize variables to sensible defaults
    mIndexBuffer  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11IndexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11IndexBuffer::~cgDX11IndexBuffer( )
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
void cgDX11IndexBuffer::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( disposeBase == true )
        cgIndexBuffer::dispose( true );
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
bool cgDX11IndexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11IndexBufferResource )
        return true;

    // Supported by base?
    return cgIndexBuffer::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DBuffer ()
/// <summary>
/// Retrieve the internal D3D11 index buffer object.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11Buffer * cgDX11IndexBuffer::getD3DBuffer( ) const
{
    // Add reference, we're returning a pointer
    if ( mIndexBuffer )
        mIndexBuffer->AddRef();

    // Return the resource item
    return mIndexBuffer;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11IndexBuffer::deviceLost()
{
    // ToDo: 7777 - Is this even necessary any more?
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseIndexBuffer();

    // Resource data is now lost
    setResourceLost( true );
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11IndexBuffer::deviceRestored()
{
    // ToDo: 7777 - Is this even necessary any more?
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createIndexBuffer();
}

//-----------------------------------------------------------------------------
//  Name : releaseIndexBuffer ()
/// <summary>
/// Release the internal vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11IndexBuffer::releaseIndexBuffer( )
{
    // Unlock if we are still locked
    unlock();

    // We should release our underlying resource
    if ( mIndexBuffer )
        mIndexBuffer->Release();
    mIndexBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createIndexBuffer ()
/// <summary>
/// Create the internal vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11IndexBuffer::createIndexBuffer( )
{
    // Already loaded?
    if ( mIndexBuffer )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for IB creation (required DX11 class driver)
    ID3D11Device * device;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(device = driver->getD3DDevice()) )
        return false;

    // Populate the D3D buffer description buffer ready for creation.
    D3D11_BUFFER_DESC bufferDesc;
    memset( &bufferDesc, 0, sizeof(D3D11_BUFFER_DESC) );
    bufferDesc.Usage     = (mUsage & cgBufferUsage::Dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = mLength;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    
    // Set dynamic resource CPU access flags.
    if ( mUsage & cgBufferUsage::Dynamic )
    {
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if ( !(mUsage & cgBufferUsage::WriteOnly) )
            bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    
    } // End if dynamic

    // Create the index buffer
    if ( FAILED(device->CreateBuffer( &bufferDesc, CG_NULL, &mIndexBuffer ) ) )
    {
        // Cleanup
        device->Release();
        return false;

    } // End if failed to create

    // Release the D3D device, we're all done with it
    device->Release();

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
bool cgDX11IndexBuffer::updateBuffer( cgUInt32 destinationOffset, cgUInt32 sourceSize, void * sourceData )
{
    // Cannot update the resource if it is not loaded, or is already locked elsewhere
    if ( !mIndexBuffer || mLocked )
        return CG_NULL;

    // If SrcSize is 0, update entire buffer from the offset onewards.
    if ( !sourceSize )
        sourceSize = mLength - destinationOffset;

    // Retrieve D3D device context for IB mapping (required DX11 class driver)
    ID3D11DeviceContext * deviceContext;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(deviceContext = driver->getD3DDeviceContext()) )
        return false;

    // If this is a dynamic buffer, we have to use a lock / map strategy.
    // Otherwise we can populate it with UpdateSubresource.
    if ( mUsage & cgBufferUsage::Dynamic )
    {
        // Can we reliably discard the data in the existing buffer?
        D3D11_MAP mapType = D3D11_MAP_WRITE;
        if ( !destinationOffset && sourceSize == mLength )
            mapType = D3D11_MAP_WRITE_DISCARD;

        // Lock the vertex buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if ( FAILED( deviceContext->Map( mIndexBuffer, 0, mapType, 0, &mappedResource ) ) )
        {
            deviceContext->Release();
            return false;
        
        } // End if failed

        // Copy the data in and unlock.
        memcpy( mappedResource.pData, sourceData, sourceSize );
        deviceContext->Unmap( mIndexBuffer, 0 );

    } // End if dynamic
    else
    {
        // Construct the update region information.
        D3D11_BOX Region;
        Region.left   = destinationOffset;
        Region.right  = destinationOffset + sourceSize;
        Region.top    = 0;
        Region.bottom = 1;
        Region.front  = 0;
        Region.back   = 1;

        // Trigger the resource update.
        deviceContext->UpdateSubresource( mIndexBuffer, 0, &Region, sourceData, sourceSize, sourceSize );

    } // End if default

    // We're done.
    deviceContext->Release();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : lock ()
/// <summary>
/// Lock the specified area of the index buffer and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
void * cgDX11IndexBuffer::lock( cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mIndexBuffer || mLocked )
        return CG_NULL;

    // Cannot lock a resource unless it was marked as dynamic.
    if ( !(mUsage & cgBufferUsage::Dynamic) )
        return CG_NULL;

    // Retrieve D3D device context for IB mapping (required DX11 class driver)
    ID3D11DeviceContext * deviceContext;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(deviceContext = driver->getD3DDeviceContext()) )
        return CG_NULL;

    // Generate the D3D11 compatible lock / map operation.
    D3D11_MAP mapType;
    if ( flags & cgLockFlags::ReadOnly )
        mapType = D3D11_MAP_READ;
    else if ( flags & cgLockFlags::WriteOnly )
    {
        mapType = D3D11_MAP_WRITE;
        if ( flags & cgLockFlags::Discard )
            mapType = D3D11_MAP_WRITE_DISCARD;
        else if ( flags & cgLockFlags::NoOverwrite )
            mapType = D3D11_MAP_WRITE_NO_OVERWRITE;
    
    } // End if write only
    else
        mapType = D3D11_MAP_READ_WRITE;

    // And the lock / map flags
    cgUInt32 mapFlags = 0;
    if ( flags & cgLockFlags::DoNotWait )
        mapFlags |= D3D11_MAP_FLAG_DO_NOT_WAIT;

    // Lock the index buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if ( FAILED( deviceContext->Map( mIndexBuffer, 0, mapType, mapFlags, &mappedResource ) ) )
    {
        deviceContext->Release();
        return CG_NULL;
    
    } // End if failed

    // Clean up
    deviceContext->Release();

    // Store lock data.
    mLockedBuffer = mappedResource.pData;
    mLocked       = true;

    // Adjust the offset of the locked data to match their requested region.
    mLockedBuffer = ((cgByte*)mLockedBuffer) + offsetToLock;
    
    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : unlock ()
/// <summary>
/// Unlock the buffer if previously locked.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11IndexBuffer::unlock( )
{
    // Cannot unlock if it was not already locked
    if ( !mIndexBuffer || !mLocked )
        return;

    // Validate requirements
    if ( !mManager )
        return;

    // Retrieve D3D device context for IB unmapping (required DX11 class driver)
    ID3D11DeviceContext * deviceContext;
    cgDX11RenderDriver * driver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !driver || !(deviceContext = driver->getD3DDeviceContext()) )
        return;

    // Unmap
    deviceContext->Unmap( mIndexBuffer, 0 );

    // Clean up
    deviceContext->Release();

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
bool cgDX11IndexBuffer::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the index buffer
    if ( !createIndexBuffer() )
        return false;

    // We're now loaded
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX11IndexBuffer::unloadResource( )
{
    // Release the index buffer
    releaseIndexBuffer();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT