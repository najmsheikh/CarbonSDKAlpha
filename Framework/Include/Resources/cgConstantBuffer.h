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
// File : cgConstantBuffer.h                                                 //
//                                                                           //
// Desc : Contains classes that represent blocks of shader constants that    //
//        can be applied to the device as a group. These constants most      //
//        often provide information to the shader library in order to        //
//        control their behavior.                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCONSTANTBUFFER_H_ )
#define _CGE_CGCONSTANTBUFFER_H_

//-----------------------------------------------------------------------------
// cgConstantBuffer Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Resources/cgResourceHandles.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector2;
class cgVector3;
class cgVector4;
class cgMatrix;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {B03B356E-B24C-4F6A-B6C8-6D020D39FAEA}
const cgUID RTID_ConstantBufferResource = {0xB03B356E, 0xB24C, 0x4F6A, {0xB6, 0xC8, 0x6D, 0x02, 0x0D, 0x39, 0xFA, 0xEA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgConstantBuffer (Base Class)
/// <summary>
/// Base constant buffer class used for managing and applying shader constant
/// values that can be uploaded to the hardware prior to rendering / processing.
/// This base class is designed to be inherited by API specific versions 
/// (in 'Platform' directory)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgConstantBuffer : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgConstantBuffer, cgResource, "ConstantBuffer" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgRenderDriver;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConstantBuffer( cgUInt32 referenceId, const cgConstantBufferDesc & desc );
             cgConstantBuffer( cgUInt32 referenceId, const cgConstantBufferDesc & desc, const cgConstantTypeDesc::Array & types );
             cgConstantBuffer( cgUInt32 referenceId, const cgSurfaceShaderHandle & shader, const cgString & bufferName );
    virtual ~cgConstantBuffer( );

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgConstantBuffer           * createInstance          ( cgUInt32 referenceId, const cgConstantBufferDesc & desc );
    static cgConstantBuffer           * createInstance          ( cgUInt32 referenceId, const cgConstantBufferDesc & desc, const cgConstantTypeDesc::Array & types );
    static cgConstantBuffer           * createInstance          ( cgUInt32 referenceId, const cgSurfaceShaderHandle & shader, const cgString & bufferName );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgConstantBufferDesc        & getDesc                 ( ) const;
    bool                                getConstantDesc         ( const cgString & name, cgConstantDesc & desc ) const;
    bool                                getConstantDesc         ( const cgString & name, cgConstantDesc & desc, cgUInt32 & offset ) const;
    const cgConstantTypeDesc::Array   & getTypes                ( ) const;
    bool                                setFloat                ( const cgString & name, float value );
    bool                                setVector               ( const cgString & name, const cgVector2 & value );
    bool                                setVector               ( const cgString & name, const cgVector3 & value );
    bool                                setVector               ( const cgString & name, const cgVector4 & value );
    bool                                setVector               ( const cgString & name, const cgColorValue & value );
    bool                                setMatrix               ( const cgString & name, const cgMatrix & value );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                        updateBuffer            ( cgUInt32 destinationOffset, cgUInt32 sourceSize, void * sourceData );
    virtual void                      * lock                    ( cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags );
    virtual void                        unlock                  ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID               & getReferenceType        ( ) const { return RTID_ConstantBufferResource; }
    virtual bool                        queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                        dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline cgByte const * const getReadOnlyBuffer( ) const
    {
        return mSystemBuffer;
    }
    inline cgUInt32 getBufferLength( ) const
    {
        return mDesc.length;
    }
    inline bool isBound( ) const
    {
        return (mBoundRegister >= 0);
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                        createConstantBuffer    ( );
    virtual void                        releaseConstantBuffer   ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    // Resource loading data.
    cgSurfaceShaderHandle       mShader;        // The shader to which this buffer is attached (if any).
    cgString                    mBufferName;    // The name of the buffer to attach to.

    // Constant buffer internals.
    cgConstantBufferDesc        mDesc;          // Description structure for this constant buffer.
    cgConstantTypeDesc::Array   mTypes;         // List of user types referenced by the above description.
    bool                        mLocked;        // Is the buffer locked?
    void                      * mLockedBuffer;  // Pointer to the locked buffer data area
    cgByte                    * mSystemBuffer;  // System memory buffer containing actual constant values using standard C++ alignment rules.
    cgInt32                     mBoundRegister; // The register to which this buffer is /currently/ bound. -1 if unbound.
    bool                        mDataUpdated;   // Was the buffer locked and updated?
};

//-----------------------------------------------------------------------------
//  Name : cgConstantBufferLinker (Base Class)
/// <summary>
/// Base utility class that is responsible for generating both the shader side
/// code declaration for referenced constant buffer descriptors as well as the
/// memory mapping instructions to generate an API specific constant mapping
/// from the engine side system memory cbuffer.
/// </summary>
//-----------------------------------------------------------------------------
class cgConstantBufferLinker
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConstantBufferLinker( cgConstantBufferDesc buffers[], size_t bufferCount, cgConstantTypeDesc types[], size_t typeCount );
    virtual ~cgConstantBufferLinker( );

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static cgConstantBufferLinker* createInstance               ( cgConstantBufferDesc buffers[], size_t bufferCount, cgConstantTypeDesc types[], size_t typeCount );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    generateBufferDeclarations  ( cgInt32 bufferReferences[], size_t bufferCount, cgString & declarationOut ) = 0;
    virtual void                    generateConstantMappings    ( ) = 0;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgConstantBufferDesc  * mBuffers;
    size_t                  mBufferCount;
    cgConstantTypeDesc    * mTypes;
    size_t                  mTypeCount;
};

#endif // !_CGE_CGCONSTANTBUFFER_H_