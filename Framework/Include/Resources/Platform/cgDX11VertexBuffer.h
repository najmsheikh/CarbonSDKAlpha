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
// Name : cgDX11VertexBuffer.h                                               //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
//        buffer resource data (DX11 implementation).                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11VERTEXBUFFER_H_ )
#define _CGE_CGDX11VERTEXBUFFER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11VertexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgVertexBuffer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11Buffer;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {1F0C1F51-D977-4D74-A339-2793B77C56CC}
const cgUID RTID_DX11VertexBufferResource = {0x1F0C1F51, 0xD977, 0x4D74, {0xA3, 0x39, 0x27, 0x93, 0xB7, 0x7C, 0x56, 0xCC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11VertexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D vertex buffer resource (DX11 implementation).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11VertexBuffer : public cgVertexBuffer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11VertexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgVertexFormat * format, cgMemoryPool::Base pool );
    virtual ~cgDX11VertexBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11Buffer          * getD3DBuffer        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgVertexBuffer)
    //-------------------------------------------------------------------------
    virtual bool            updateBuffer        ( cgUInt32 destinationOffset, cgUInt32 sourceSize, void * sourceData );
    virtual void          * lock                ( cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags );
    virtual void            unlock              ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost          ( );
    virtual void            deviceRestored      ( );
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11VertexBufferResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    bool                    createVertexBuffer  ( );
    void                    releaseVertexBuffer ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    ID3D11Buffer  * mVertexBuffer;  // Underlying Resource
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11VERTEXBUFFER_H_