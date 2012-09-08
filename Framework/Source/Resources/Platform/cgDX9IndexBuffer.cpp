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
// Name : cgDX9IndexBuffer.h                                                 //
//                                                                           //
// Desc : Contains classes responsible for loading and managing index        //
//        buffer resource data (DX9 implementation).                         //
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
// cgDX9IndexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9IndexBuffer.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9IndexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9IndexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9IndexBuffer::cgDX9IndexBuffer( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgBufferFormat::Base Format, cgMemoryPool::Base Pool ) :
    cgIndexBuffer( nReferenceId, Length, Usage, Format, Pool )
{
    // Initialize variables to sensible defaults
    mIndexBuffer  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9IndexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9IndexBuffer::~cgDX9IndexBuffer( )
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
void cgDX9IndexBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase == true )
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
bool cgDX9IndexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9IndexBufferResource )
        return true;

    // Supported by base?
    return cgIndexBuffer::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : GetD3DBuffer ()
/// <summary>
/// Retrieve the internal D3D9 index buffer object.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DIndexBuffer9 * cgDX9IndexBuffer::GetD3DBuffer( ) const
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
void cgDX9IndexBuffer::deviceLost()
{
    // Nothing to do?
    if ( mPool != cgMemoryPool::Default )
        return;

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
void cgDX9IndexBuffer::deviceRestored()
{
    // Bail if this is not in the default pool
    if ( mPool != cgMemoryPool::Default )
        return;

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
void cgDX9IndexBuffer::releaseIndexBuffer( )
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
bool cgDX9IndexBuffer::createIndexBuffer( )
{
    // Already loaded?
    if ( mIndexBuffer )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for IB creation (required DX9 class driver)
    IDirect3DDevice9 * pDevice;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Resource manager should take the HardwareTnL flag from the render driver if applicable!
    if ( !pDriver->useHardwareTnL() )
        mUsage |= cgBufferUsage::SoftwareProcessing;

    // Create the index buffer
    D3DFORMAT IndexBufferFormat = (mFormat == cgBufferFormat::Index32) ? D3DFMT_INDEX32 : D3DFMT_INDEX16;
    if ( FAILED(pDevice->CreateIndexBuffer( mLength, mUsage, IndexBufferFormat, (D3DPOOL)mPool, &mIndexBuffer, CG_NULL ) ) )
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
bool cgDX9IndexBuffer::updateBuffer( cgUInt32 DstOffset, cgUInt32 SrcSize, void * pSrcData )
{
    // Cannot update the resource if it is not loaded, or is already locked elsewhere
    if ( !mIndexBuffer || mLocked )
        return CG_NULL;

    // If SrcSize is 0, update entire buffer from the offset onewards.
    if ( !SrcSize )
        SrcSize = mLength - DstOffset;

    // Can we reliably discard the data in the existing buffer?
    cgUInt32 nLockFlags = 0;
    if ( (mUsage & cgBufferUsage::Dynamic) && !DstOffset && SrcSize == mLength )
        nLockFlags |= D3DLOCK_DISCARD;

    // Lock the vertex buffer
    void * pLockedData = CG_NULL;
    if ( FAILED( mIndexBuffer->Lock( DstOffset, SrcSize, &pLockedData, nLockFlags )) )
        return false;

    // Copy the data in.
    memcpy( pLockedData, pSrcData, SrcSize );

    // We're done.
    mIndexBuffer->Unlock();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : lock ()
/// <summary>
/// Lock the specified area of the index buffer and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
void * cgDX9IndexBuffer::lock( cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mIndexBuffer || mLocked )
        return CG_NULL;

    // Cannot lock a resource unless it was marked as dynamic.
    if ( !(mUsage & cgBufferUsage::Dynamic) )
        return CG_NULL;

    // Strip 'WriteOnly' flag (not a D3D9 matching flag).
    Flags &= ~cgLockFlags::WriteOnly;

    // Lock the index buffer
    if ( FAILED( mIndexBuffer->Lock( OffsetToLock, SizeToLock, (void**)&mLockedBuffer, Flags )) )
        return CG_NULL;
    mLocked = true;

    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : unlock ()
/// <summary>
/// Unlock the buffer if previously locked.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9IndexBuffer::unlock( )
{
    // Cannot unlock if it was not already locked
    if ( !mIndexBuffer || !mLocked )
        return;

    // Unlock!
    mIndexBuffer->Unlock();

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
bool cgDX9IndexBuffer::loadResource( )
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
bool cgDX9IndexBuffer::unloadResource( )
{
    // Release the index buffer
    releaseIndexBuffer();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT