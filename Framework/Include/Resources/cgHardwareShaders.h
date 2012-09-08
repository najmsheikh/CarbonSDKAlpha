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
// Name : cgHardwareShaders.h                                                //
//                                                                           //
// Desc : Contains classes responsible for loading and managing hardware     //
//        vertex, pixel and geometry shader resource data.                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHARDWARESHADERS_H_ )
#define _CGE_CGHARDWARESHADERS_H_

//-----------------------------------------------------------------------------
// cgHardwareShaders Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResource.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {532570B5-62A6-4D28-BDAB-AD69549E1C8D}
const cgUID RTID_VertexShaderResource   = {0x532570B5, 0x62A6, 0x4D28, {0xBD, 0xAB, 0xAD, 0x69, 0x54, 0x9E, 0x1C, 0x8D}};
// {47393B29-2D12-4FAC-95A3-226A5AE9B062}
const cgUID RTID_PixelShaderResource    = {0x47393B29, 0x2D12, 0x4FAC, {0x95, 0xA3, 0x22, 0x6A, 0x5A, 0xE9, 0xB0, 0x62}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVertexShader (Class)
/// <summary>
/// Wrapper for managing a vertex shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVertexShader : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgVertexShader, cgResource, "VertexShader" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgVertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgVertexShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgVertexShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgVertexShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgVertexShader( );

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static cgVertexShader     * createInstance      ( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
    static cgVertexShader     * createInstance      ( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
    static cgVertexShader     * createInstance      ( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
    static cgVertexShader     * createInstance      ( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgShaderIdentifier  & getIdentifier       ( ) const;
    const cgByteArray         & getByteCode         ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_VertexShaderResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool bDisposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    readCompiledShader      ( cgInputStream stream );
    void                    writePermutationCache   ( const cgString & cacheFile );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Resource Loading
    cgInputStream           mSourceFile;
    cgString                mSourceCode;
    cgString                mSourceEntryPoint;
    cgByteArray             mByteCode;
    bool                    mCompiled;

    // Resource Data
    cgShaderIdentifier      mIdentifier;
};

//-----------------------------------------------------------------------------
//  Name : cgPixelShader (Class)
/// <summary>
/// Wrapper for managing a pixel shader resource
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPixelShader : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPixelShader, cgResource, "PixelShader" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
             cgPixelShader( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
             cgPixelShader( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
             cgPixelShader( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );
    virtual ~cgPixelShader( );

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static cgPixelShader      * createInstance      ( cgUInt32 referenceId, const cgInputStream & sourceFile, const cgString & entryPoint );
    static cgPixelShader      * createInstance      ( cgUInt32 referenceId, const cgInputStream & sourceFile, bool compiled );
    static cgPixelShader      * createInstance      ( cgUInt32 referenceId, const cgString & sourceCode, const cgString & entryPoint, const cgShaderIdentifier * identifier );
    static cgPixelShader      * createInstance      ( cgUInt32 referenceId, const cgByteArray & byteCode, const cgShaderIdentifier * identifier );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgShaderIdentifier  & getIdentifier       ( ) const;
    const cgByteArray         & getByteCode         ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_PixelShaderResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                    readCompiledShader      ( cgInputStream stream );
    void                    writePermutationCache   ( const cgString & cacheFile );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Resource Loading
    cgInputStream           mSourceFile;
    cgString                mSourceCode;
    cgString                mSourceEntryPoint;
    cgByteArray             mByteCode;
    bool                    mCompiled;

    // Resource Data
    cgShaderIdentifier      mIdentifier;
};

#endif // !_CGE_CGHARDWARESHADERS_H_