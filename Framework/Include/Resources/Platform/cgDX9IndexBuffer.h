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

#pragma once
#if !defined( _CGE_CGDX9INDEXBUFFER_H_ )
#define _CGE_CGDX9INDEXBUFFER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9IndexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgIndexBuffer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DIndexBuffer9;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5E88D165-4B82-47AD-B0BA-592C63810162}
const cgUID RTID_DX9IndexBufferResource = {0x5E88D165, 0x4B82, 0x47AD, {0xB0, 0xBA, 0x59, 0x2C, 0x63, 0x81, 0x1, 0x62}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9IndexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D index buffer resource (DX9 implementation).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9IndexBuffer : public cgIndexBuffer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9IndexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool );
    virtual ~cgDX9IndexBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DIndexBuffer9 * GetD3DBuffer        ( ) const;

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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_DX9IndexBufferResource; }
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
    IDirect3DIndexBuffer9 * mIndexBuffer;   // Underlying Resource
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9INDEXBUFFER_H_