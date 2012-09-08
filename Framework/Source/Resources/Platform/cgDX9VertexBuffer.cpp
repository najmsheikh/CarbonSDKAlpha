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
// Name : cgDX9VertexBuffer.cpp                                              //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
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
// cgDX9VertexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9VertexBuffer.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Rendering/cgVertexFormats.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#undef WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////
// cgDX9VertexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9VertexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexBuffer::cgDX9VertexBuffer( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgVertexFormat * pFormat, cgMemoryPool::Base Pool ) :
    cgVertexBuffer( nReferenceId, Length, Usage, pFormat, Pool )
{
    // Initialize variables to sensible defaults
    mVertexBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9VertexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9VertexBuffer::~cgDX9VertexBuffer( )
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
void cgDX9VertexBuffer::dispose( bool bDisposeBase )
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
bool cgDX9VertexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9VertexBufferResource )
        return true;

    // Supported by base?
    return cgVertexBuffer::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DBuffer ()
/// <summary>
/// Retrieve the internal D3D9 vertex buffer object.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DVertexBuffer9 * cgDX9VertexBuffer::getD3DBuffer( ) const
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
void cgDX9VertexBuffer::deviceLost()
{
    // Nothing to do?
    if ( mPool != cgMemoryPool::Default )
        return;

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
void cgDX9VertexBuffer::deviceRestored()
{
    // Bail if this is not in the default pool
    if ( mPool != cgMemoryPool::Default )
        return;

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
void cgDX9VertexBuffer::releaseVertexBuffer( )
{
    // Unlock if we are still locked
    unlock();

    // We should release our underlying resource
    if ( mVertexBuffer != CG_NULL )
        mVertexBuffer->Release();
    mVertexBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createVertexBuffer ()
/// <summary>
/// Create the internal vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9VertexBuffer::createVertexBuffer( )
{
    // Already loaded?
    if ( mVertexBuffer )
        return true;

    // Validate requirements
    if ( !mManager )
        return false;

    // Retrieve D3D device for VB creation (required DX9 class driver)
    IDirect3DDevice9 * pDevice;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;
    
    // Resource manager should take the HardwareTnL flag from the render driver if applicable!
    if ( !pDriver->useHardwareTnL() )
        mUsage |= cgBufferUsage::SoftwareProcessing;

    // Create the vertex buffer
    if ( FAILED(pDevice->CreateVertexBuffer( mLength, mUsage, 0, (D3DPOOL)mPool, &mVertexBuffer, CG_NULL ) ) )
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
bool cgDX9VertexBuffer::updateBuffer( cgUInt32 DstOffset, cgUInt32 SrcSize, void * pSrcData )
{
    // Cannot update the resource if it is not loaded, or is already locked elsewhere
    if ( !mVertexBuffer || mLocked )
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
    if ( FAILED( mVertexBuffer->Lock( DstOffset, SrcSize, &pLockedData, nLockFlags )) )
        return false;

    // Copy the data in.
    memcpy( pLockedData, pSrcData, SrcSize );

    // We're done.
    mVertexBuffer->Unlock();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : lock ()
/// <summary>
/// Lock the specified area of the vertex buffer and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
void * cgDX9VertexBuffer::lock( cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mVertexBuffer || mLocked )
        return CG_NULL;

    // Cannot lock a resource unless it was marked as dynamic.
    if ( !(mUsage & cgBufferUsage::Dynamic) )
        return CG_NULL;

    // Strip 'WriteOnly' flag (not a D3D9 matching flag).
    Flags &= ~cgLockFlags::WriteOnly;

    // Lock the vertex buffer
    if ( FAILED( mVertexBuffer->Lock( OffsetToLock, SizeToLock, (void**)&mLockedBuffer, Flags )) )
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
void cgDX9VertexBuffer::unlock( )
{
    // Cannot unlock if it was not already locked
    if ( !mVertexBuffer || !mLocked )
        return;

    // Unlock!
    mVertexBuffer->Unlock();

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
bool cgDX9VertexBuffer::loadResource( )
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
bool cgDX9VertexBuffer::unloadResource( )
{
    // Release the vertex buffer
    releaseVertexBuffer();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT