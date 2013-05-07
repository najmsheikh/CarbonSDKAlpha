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
// Name : cgVertexBuffer.cpp                                                 //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
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
// cgVertexBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgVertexBuffer.h>
#include <Rendering/cgVertexFormats.h>

// Platform specific implementations
#include <Resources/Platform/cgDX9VertexBuffer.h>
#include <Resources/Platform/cgDX11VertexBuffer.h>

///////////////////////////////////////////////////////////////////////////////
// cgVertexBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgVertexBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexBuffer::cgVertexBuffer( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgVertexFormat * pFormat, cgMemoryPool::Base Pool ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mLocked       = false;
    mLockedBuffer = CG_NULL;

    // Store resource initialization vars
    mLength       = Length;
    mUsage        = Usage;
    mFormat       = cgVertexFormat::formatFromDeclarator( pFormat->getDeclarator() );
    mPool          = Pool;

    // Setup initial cached responses
    mResourceType  = cgResourceType::VertexBuffer;
}

//-----------------------------------------------------------------------------
//  Name : ~cgVertexBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexBuffer::~cgVertexBuffer( )
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
cgVertexBuffer * cgVertexBuffer::createInstance( cgUInt32 nReferenceId, cgUInt32 Length, cgUInt32 Usage, cgVertexFormat * pFormat, cgMemoryPool::Base Pool )
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
                return new cgDX9VertexBuffer( nReferenceId, Length, Usage, pFormat, Pool );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11VertexBuffer( nReferenceId, Length, Usage, pFormat, Pool );

#endif // CGE_DX11_RENDER_SUPPORT

            default:
                return CG_NULL;

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
void cgVertexBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Nothing in this implementation.

    // Dispose base(s).
    if ( bDisposeBase )
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
bool cgVertexBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_VertexBufferResource )
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
cgUInt32 cgVertexBuffer::getLength( ) const
{
    return mLength;
}

//-----------------------------------------------------------------------------
//  Name : getUsage ()
/// <summary>
/// Retrieve the usage flags specified when the buffer was created.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVertexBuffer::getUsage( ) const
{
    return mUsage;
}

//-----------------------------------------------------------------------------
//  Name : getFormat ()
/// <summary>
/// Retrieve the requested format / layout of the buffer.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgVertexBuffer::getFormat( ) const
{
    return mFormat;
}

//-----------------------------------------------------------------------------
//  Name : getPool ()
/// <summary>
/// Retrieve the memory pool that was selected when the buffer was created.
/// </summary>
//-----------------------------------------------------------------------------
cgMemoryPool::Base cgVertexBuffer::getPool( ) const
{
    return mPool;
}