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
// Name : cgIndexBuffer.cpp                                                  //
//                                                                           //
// Desc : Contains classes responsible for loading and managing index        //
//        buffer resource data.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgIndexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgIndexBuffer.h>

// Platform specific implementations
#include <Resources/Platform/cgDX9IndexBuffer.h>
#include <Resources/Platform/cgDX11IndexBuffer.h>

///////////////////////////////////////////////////////////////////////////////
// cgIndexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgIndexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgIndexBuffer::cgIndexBuffer( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgBufferFormat::Base Format, cgMemoryPool::Base Pool ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mLocked       = false;
    mLockedBuffer = CG_NULL;

    // Store resource initialization vars
    mLength       = Length;
    mUsage        = Usage;
    mFormat        = Format;
    mPool          = Pool;

    // Setup initial cached responses
    mResourceType  = cgResourceType::IndexBuffer;
}

//-----------------------------------------------------------------------------
//  Name : ~cgIndexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgIndexBuffer::~cgIndexBuffer( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgIndexBuffer * cgIndexBuffer::createInstance( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgBufferFormat::Base Format, cgMemoryPool::Base Pool )
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
                return new cgDX9IndexBuffer( nReferenceId, Length, Usage, Format, Pool );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11IndexBuffer( nReferenceId, Length, Usage, Format, Pool );

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
void cgIndexBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in base implementation.

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgResource::dispose( true );
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
bool cgIndexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_IndexBufferResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getLength ()
/// <summary>
/// Retrieve the length of the buffer in bytes.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgIndexBuffer::getLength( ) const
{
    return mLength;
}

//-----------------------------------------------------------------------------
//  Name : getUsage ()
/// <summary>
/// Retrieve the usage flags specified when the buffer was created.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgIndexBuffer::getUsage( ) const
{
    return mUsage;
}

//-----------------------------------------------------------------------------
//  Name : getFormat ()
/// <summary>
/// Retrieve the requested format / layout of the buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgBufferFormat::Base cgIndexBuffer::getFormat( ) const
{
    return mFormat;
}

//-----------------------------------------------------------------------------
//  Name : getPool ()
/// <summary>
/// Retrieve the memory pool that was selected when the buffer was created.
/// </summary>
//-----------------------------------------------------------------------------
cgMemoryPool::Base cgIndexBuffer::getPool( ) const
{
    return mPool;
}