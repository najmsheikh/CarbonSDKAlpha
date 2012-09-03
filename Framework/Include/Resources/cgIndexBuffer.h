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
// Name : cgIndexBuffer.h                                                    //
//                                                                           //
// Desc : Contains classes responsible for loading and managing index        //
//        buffer resource data.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGINDEXBUFFER_H_ )
#define _CGE_CGINDEXBUFFER_H_

//-----------------------------------------------------------------------------
// cgIndexBuffer Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {E0372857-C943-4413-9098-7A0CE846E531}
const cgUID RTID_IndexBufferResource = {0xE0372857, 0xC943, 0x4413, {0x90, 0x98, 0x7A, 0xC, 0xE8, 0x46, 0xE5, 0x31}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgIndexBuffer (Class)
/// <summary>
/// Wrapper for managing a D3D index buffer resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgIndexBuffer : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgIndexBuffer, cgResource, "IndexBuffer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgIndexBuffer( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool );
    virtual ~cgIndexBuffer( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgIndexBuffer  * createInstance      ( cgUInt32 referenceId, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUInt32                getLength           ( ) const;
    cgUInt32                getUsage            ( ) const;
    cgBufferFormat::Base    getFormat           ( ) const;
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
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_IndexBufferResource; }
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
    cgBufferFormat::Base    mFormat;        // Format of the index buffer
    cgMemoryPool::Base      mPool;          // The pool in which the buffer will exist
};

#endif // !_CGE_CGINDEXBUFFER_H_