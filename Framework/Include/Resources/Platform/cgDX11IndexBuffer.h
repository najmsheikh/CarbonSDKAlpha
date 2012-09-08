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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11INDEXBUFFER_H_ )
#define _CGE_CGDX11INDEXBUFFER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11IndexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgIndexBuffer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11Buffer;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {506FF387-8499-46D9-9C7D-E965AB48187D}
const cgUID RTID_DX11IndexBufferResource = {0x506FF387, 0x8499, 0x46D9, {0x9C, 0x7D, 0xE9, 0x65, 0xAB, 0x48, 0x18, 0x7D}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11IndexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D index buffer resource (DX11 implementation).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11IndexBuffer : public cgIndexBuffer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11IndexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool );
    virtual ~cgDX11IndexBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11Buffer          * getD3DBuffer        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgIndexBuffer)
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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX11IndexBufferResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    createIndexBuffer   ( );
    void                    releaseIndexBuffer  ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ID3D11Buffer  * mIndexBuffer;   // Underlying Resource
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11INDEXBUFFER_H_