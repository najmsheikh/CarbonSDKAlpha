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
// Name : cgDX9HardwareShaders.h                                             //
//                                                                           //
// Desc : Contains classes responsible for loading and managing hardware     //
//        vertex, pixel and geometry shader resource data                    //
//        (DX9 implementation).                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX9HARDWARESHADERS_H_ )
#define _CGE_CGDX9HARDWARESHADERS_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9HardwareShaders Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgHardwareShaders.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct ID3DXConstantTable;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {3EEC738F-F123-4537-9047-FA3A22F8DB65}
const cgUID RTID_DX9VertexShaderResource   = {0x3EEC738F, 0xF123, 0x4537, {0x90, 0x47, 0xFA, 0x3A, 0x22, 0xF8, 0xDB, 0x65}};
// {40F0D60E-22C3-4289-A3BC-B392144F4A7A}
const cgUID RTID_DX9PixelShaderResource    = {0x40F0D60E, 0x22C3, 0x4289, {0xA3, 0xBC, 0xB3, 0x92, 0x14, 0x4F, 0x4A, 0x7A}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX9VertexShader (Class)
/// <summary>
/// Wrapper for managing a D3D vertex shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9VertexShader : public cgVertexShader
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgDX9VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgDX9VertexShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgDX9VertexShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgDX9VertexShader( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DVertexShader9* getD3DShader            ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost              ( );
    virtual void            deviceRestored          ( );
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_DX9VertexShaderResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    createShader            ( );
    void                    releaseShader           ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    IDirect3DVertexShader9* mShader;            // Underlying Resource
    ID3DXConstantTable    * mConstantTable;
};

//-----------------------------------------------------------------------------
//  Name : cgDX9PixelShader (Class)
/// <summary>
/// Wrapper for managing a D3D pixel shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX9PixelShader : public cgPixelShader
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX9PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgDX9PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgDX9PixelShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgDX9PixelShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgDX9PixelShader( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    IDirect3DPixelShader9 * GetD3DShader            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual void            deviceLost              ( );
    virtual void            deviceRestored          ( );
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_DX9PixelShaderResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    CreateShader            ( );
    void                    ReleaseShader           ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    IDirect3DPixelShader9 * m_pShader;              // Underlying Resource
    ID3DXConstantTable    * m_pConstantTable;
};

#endif // CGE_DX9_RENDER_SUPPORT

#endif // !_CGE_CGDX9HARDWARESHADERS_H_