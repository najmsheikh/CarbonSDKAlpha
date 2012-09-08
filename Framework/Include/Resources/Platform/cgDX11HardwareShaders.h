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
// Name : cgDX11HardwareShaders.h                                            //
//                                                                           //
// Desc : Contains classes responsible for loading and managing hardware     //
//        vertex, pixel and geometry shader resource data                    //
//        (DX11 implementation).                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11HARDWARESHADERS_H_ )
#define _CGE_CGDX11HARDWARESHADERS_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11HardwareShaders Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgHardwareShaders.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct ID3D11VertexShader;
struct ID3D11PixelShader;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {8F6CFA2F-EB14-4925-A471-A1B1FBF16CD3}
const cgUID RTID_DX11VertexShaderResource   = {0x8F6CFA2F, 0xEB14, 0x4925, {0xA4, 0x71, 0xA1, 0xB1, 0xFB, 0xF1, 0x6C, 0xD3}};
// {4F04FC4D-883D-4DFE-BEFD-FA9FC7949ADC}
const cgUID RTID_DX11PixelShaderResource    = {0x4F04FC4D, 0x883D, 0x4DFE, {0xBE, 0xFD, 0xFA, 0x9F, 0xC7, 0x94, 0x9A, 0xDC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11VertexShader (Class)
/// <summary>
/// Wrapper for managing a D3D vertex shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11VertexShader : public cgVertexShader
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgDX11VertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgDX11VertexShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgDX11VertexShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgDX11VertexShader( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11VertexShader    * getD3DShader            ( ) const;
    cgUInt32                getIAInputSignatureId   ( ) const;
    
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
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_DX11VertexShaderResource; }
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
    ID3D11VertexShader* mShader;            // Underlying Resource
    cgUInt32            mIASignatureId;     // The unique input signature identifier for this shader permutation.
};

//-----------------------------------------------------------------------------
//  Name : cgDX11PixelShader (Class)
/// <summary>
/// Wrapper for managing a D3D pixel shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgDX11PixelShader : public cgPixelShader
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgDX11PixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgDX11PixelShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgDX11PixelShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgDX11PixelShader( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    ID3D11PixelShader     * getD3DShader            ( ) const;
    cgUInt32                getIAInputSignatureId   ( ) const;

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
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_DX11PixelShaderResource; }
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
    ID3D11PixelShader * mShader;            // Underlying Resource
    cgUInt32            mIASignatureId;     // The unique input signature identifier for this shader permutation.
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11HARDWARESHADERS_H_