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
// File : cgDX11ConstantBuffer.h                                             //
//                                                                           //
// Desc : Contains classes that represent blocks of shader constants that    //
//        can be applied to the device as a group. These constants most      //
//        often provide information to the shader library in order to        //
//        control their behavior (DX11 implementation).                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11CONSTANTBUFFER_H_ )
#define _CGE_CGDX11CONSTANTBUFFER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11ConstantBuffer Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgConstantBuffer.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11DeviceContext;
struct ID3D11Buffer;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {9C5AC94E-3C01-48EA-BBB4-28136BA2E88B}
const cgUID RTID_DX11ConstantBufferResource = {0x9C5AC94E, 0x3C01, 0x48EA, {0xBB, 0xB4, 0x28, 0x13, 0x6B, 0xA2, 0xE8, 0x8B}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11ConstantBuffer (Class)
/// <summary>
/// Base constant buffer class used for managing and applying shader constant
/// values that can be uploaded to the hardware prior to rendering / processing.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11ConstantBuffer : public cgConstantBuffer
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11ConstantBuffer( cgUInt32 referenceId, const cgConstantBufferDesc & description );
             cgDX11ConstantBuffer( cgUInt32 referenceId, const cgConstantBufferDesc & description, const cgConstantTypeDesc::Array & types );
             cgDX11ConstantBuffer( cgUInt32 referenceId, const cgSurfaceShaderHandle & shader, const cgString & bufferName );
    virtual ~cgDX11ConstantBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    apply                   ( cgUInt32 bufferIndex, ID3D11DeviceContext * device );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_DX11ConstantBufferResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct TransferItem
    {
        size_t      sourceOffset;
        size_t      destinationOffset;
        size_t      size;
        cgUInt32    baseType;
        bool        transposeData;
        bool        convertType;
        cgUInt8     rows;
        cgUInt8     columns;
    };
    CGE_VECTOR_DECLARE(TransferItem, TransferItemArray);

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    createConstantBuffer    ( );
    void                    releaseConstantBuffer   ( );
    void                    GenerateTransferMap     ( const cgConstantTypeDesc & typeDescription, cgUInt32 elementCount, cgUInt32 destinationRegisterOffset, bool rootType = true );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgConstantBufferLinker* mLinker;            // Object designed to generate mappings between this buffer and the hardware constant registers.
    bool                    mSharedLinker;      // Linker object belongs to the shader, or was locally assigned?
    TransferItemArray       mTransferMap;       // Contains a series of instructions that describes how to transfer data from the C++ aligned system member buffer, to the register buffer.
    ID3D11Buffer          * mBuffer;            // Actually D3D11 constant buffer object
};

//-----------------------------------------------------------------------------
//  Name : cgDX11ConstantBufferLinker (Class)
/// <summary>
/// Utility class that is responsible for generating both the shader side
/// code declaration for referenced constant buffer descriptors as well as the
/// memory mapping instructions to generate an API specific constant mapping
/// from the engine side system memory cbuffer (DX11 implementation).
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11ConstantBufferLinker : public cgConstantBufferLinker
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11ConstantBufferLinker( cgConstantBufferDesc buffers[], size_t bufferCount, cgConstantTypeDesc types[], size_t typeCount );
    virtual ~cgDX11ConstantBufferLinker( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool    generateBufferDeclarations      ( cgInt32 bufferRefs[], size_t bufferCount, cgString & declarationOut );
    virtual void    generateConstantMappings        ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgInt32, cgString, HandleCodeMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            generateTypeDeclaration         ( const cgConstantTypeDesc & description, cgString & declarationOut );
    void            collectReferencedUDTs           ( cgInt32Set & visitedTypeHandles, cgInt32Array & referencedTypeHandles, cgInt32 typeId );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    HandleCodeMap   mCachedTypeCode;            // Dictionary that represents a cache of all generated constant type shader code.
    cgInt32Array    mReferencedTypeHandles;     // Array containing a list of all types that are /actually/ referenced by the cbuffers.
    bool            mMappingsGenerated;         // A bool indicating whether or not register mappings have been generated yet.
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11CONSTANTBUFFER_H_