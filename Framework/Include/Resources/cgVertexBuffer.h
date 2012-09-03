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
// Name : cgVertexBuffer.h                                                   //
//                                                                           //
// Desc : Contains classes responsible for loading and managing vertex       //
//        buffer resource data.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGVERTEXBUFFER_H_ )
#define _CGE_CGVERTEXBUFFER_H_

//-----------------------------------------------------------------------------
// cgVertexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVertexFormat;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {1A7E3B74-B8F5-42B5-8C15-F0A639D8AA12}
const cgUID RTID_VertexBufferResource = {0x1A7E3B74, 0xB8F5, 0x42B5, {0x8C, 0x15, 0xF0, 0xA6, 0x39, 0xD8, 0xAA, 0x12}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVertexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D vertex buffer resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVertexBuffer : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgVertexBuffer, cgResource, "VertexBuffer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgVertexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgVertexFormat * format, cgMemoryPool::Base pool );
    virtual ~cgVertexBuffer( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgVertexBuffer * createInstance      ( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgVertexFormat * format, cgMemoryPool::Base pool );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUInt32                getLength           ( ) const;
    cgUInt32                getUsage            ( ) const;
    cgVertexFormat        * getFormat           ( ) const;
    cgMemoryPool::Base      getPool             ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            updateBuffer        ( cgUInt32 destinationOffset, cgUInt32 sourceSize, void * sourceData ) = 0;
    virtual void          * lock                ( cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags ) = 0;
    virtual void            unlock              ( ) = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_VertexBufferResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool                    mLocked;        // Is the buffer locked?
    void                  * mLockedBuffer;  // Pointer to the locked buffer data area
    cgUInt32                mLength;        // Length of the buffer
    cgUInt32                mUsage;         // Usage flags for the buffer
    cgVertexFormat        * mFormat;        // Format of the vertex buffer
    cgMemoryPool::Base      mPool;          // The pool in which the buffer will exist
};

#endif // !_CGE_CGVERTEXBUFFER_H_