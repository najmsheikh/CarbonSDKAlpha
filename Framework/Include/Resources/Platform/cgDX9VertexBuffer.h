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
// Name : cgDX9VertexBuffer.h                                                //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
//        buffer resource data (DX9 implementation).                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9VERTEXBUFFER_H_ )
#define _CGE_CGDX9VERTEXBUFFER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9VertexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgVertexBuffer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DVertexBuffer9;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {9B71A394-2908-4D4D-ABB5-D5B955F9B71C}
const cgUID RTID_DX9VertexBufferResource = {0x9B71A394, 0x2908, 0x4D4D, {0xAB, 0xB5, 0xD5, 0xB9, 0x55, 0xF9, 0xB7, 0x1C}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9VertexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D vertex buffer resource (DX9 implementation).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9VertexBuffer : public cgVertexBuffer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9VertexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgVertexFormat * format, cgMemoryPool::Base pool );
    virtual ~cgDX9VertexBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DVertexBuffer9* getD3DBuffer        ( ) const;

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9VertexBufferResource; }
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
    IDirect3DVertexBuffer9* mVertexBuffer;  // Underlying Resource
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9VERTEXBUFFER_H_