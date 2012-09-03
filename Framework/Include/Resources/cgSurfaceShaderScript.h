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
// Name : cgSurfaceShaderScript.h                                            //
//                                                                           //
// Desc : Contains a specialized (derived) version of the cgScript class     //
//        that provides support for features specific to surface shader      //
//        scripts.                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSURFACESHADERSCRIPT_H_ )
#define _CGE_CGSURFACESHADERSCRIPT_H_

//-----------------------------------------------------------------------------
// cgSurfaceShaderScript Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgScript.h>
#include <Resources/cgResourceTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgConstantBufferLinker;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {CB1173E3-B0DD-44D0-9DBA-12CA6364FABA}
const cgUID RTID_SurfaceShaderScriptResource = {0xCB1173E3, 0xB0DD, 0x44D0, {0x9D, 0xBA, 0x12, 0xCA, 0x63, 0x64, 0xFA, 0xBA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSurfaceShaderScript (Class)
/// <summary>
/// An individual script item, loaded from disk or otherwise, that
/// contains / manages all script code for execution by the engine.
/// Note : This is a specialized version of the base 'cgScript' class that
/// provides additional features specific to surface shader scripts.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSurfaceShaderScript : public cgScript
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSurfaceShaderScript, cgScript, "SurfaceShaderScript" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( cgConstantTypeDesc, ConstTypeArray )
    CGE_VECTOR_DECLARE( cgConstantBufferDesc, ConstBufferArray )
    CGE_VECTOR_DECLARE( cgSamplerBlockDesc, SamplerBlockArray )
    CGE_VECTOR_DECLARE( cgShaderCallFunctionDesc, ShaderCallFuncArray )
    CGE_MAP_DECLARE   ( cgString, cgInt32, HandleMap )
    CGE_MAP_DECLARE   ( cgString, cgInt32Array, HandleArrayMap )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSurfaceShaderScript( cgUInt32 referenceId, const cgInputStream & stream, const cgString & thisType, cgScriptEngine * engine );
    virtual ~cgSurfaceShaderScript( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt32                 addShaderCallFuncDesc   ( const cgShaderCallFunctionDesc & description );
    cgInt32                 addConstantBufferDesc   ( const cgConstantBufferDesc & description );
    cgInt32                 addConstantTypeDesc     ( const cgConstantTypeDesc & description );
    cgInt32                 addSamplerBlockDesc     ( const cgSamplerBlockDesc & description );
    cgInt32                 findSamplerRegister     ( const cgString & name ) const;
    cgInt32                 findSamplerRegister     ( const cgString & sourceNamespace, const cgString & name ) const;    
    bool                    getSamplerBlockDesc     ( cgInt32 handle, cgSamplerBlockDesc & description ) const;
    bool                    getSamplerBlockDesc     ( const cgString & name, cgSamplerBlockDesc & description ) const;
    bool                    getSamplerBlockDesc     ( const cgString & sourceNamespace, const cgString & name, cgSamplerBlockDesc & description ) const;
    cgInt32                 findConstantBuffer      ( const cgString & name ) const;
    cgInt32                 findConstantBuffer      ( const cgString & sourceNamespace, const cgString & name ) const;
    bool                    getConstantBufferDesc   ( cgInt32 handle, cgConstantBufferDesc & description ) const;
    bool                    getConstantBufferDesc   ( const cgString & name, cgConstantBufferDesc & description ) const;
    bool                    getConstantBufferDesc   ( const cgString & sourceNamespace, const cgString & name, cgConstantBufferDesc & description ) const;
    const ConstBufferArray& getConstantBuffers      ( ) const;
    cgInt32                 findConstantType        ( const cgString & name ) const;
    cgInt32                 findConstantType        ( const cgString & sourceNamespace, const cgString & name ) const;
    bool                    getConstantTypeDesc     ( cgInt32 handle, cgConstantTypeDesc & description ) const;
    bool                    getConstantTypeDesc     ( const cgString & name, cgConstantTypeDesc & description ) const;
    bool                    getConstantTypeDesc     ( const cgString & sourceNamespace, const cgString & name, cgConstantTypeDesc & description ) const;
    const ConstTypeArray  & getConstantTypes        ( ) const;
    cgConstantBufferLinker* getConstantBufferLinker ( ) const;
    cgInt32Array            findShaderCallFunc      ( const cgString & name ) const;
    cgInt32Array            findShaderCallFunc      ( const cgString & sourceNamespace, const cgString & name ) const;
    bool                    getShaderCallFuncDesc   ( cgInt32 handle, cgShaderCallFunctionDesc & description ) const;
    bool                    getShaderCallFuncDesc   ( const cgString & name, cgShaderCallFunctionDesc::Array & descriptions ) const;
    bool                    getShaderCallFuncDesc   ( const cgString & sourceNamespace, const cgString & name, cgShaderCallFunctionDesc::Array & descriptions ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource            ( );
    virtual bool            unloadResource          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_SurfaceShaderScriptResource; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ConstTypeArray          mConstantTypes;         // List of constant types / structures discovered within the script during pre-processing.
    HandleMap               mConstantTypeLUT;       // Lookup table providing a mapping between type names and the type handle.
    ConstBufferArray        mConstantBuffers;       // List of constant buffers discovered within the script during pre-processing.
    HandleMap               mConstantBufferLUT;     // Lookup table providing a mapping between buffer names and the type handle.
    SamplerBlockArray       mSamplerBlocks;         // List of sampler blocks discovered within the script during pre-processing.
    HandleMap               mSamplerBlockLUT;       // Lookup table providing a mapping between sampler block names and the type handle.
    HandleMap               mSamplerRegisters;      // Lookup table that directly maps named samplers (inc. namespace) to their register indices.
    ShaderCallFuncArray     mShaderCallFunctions;   // List of all shader call functions discovered in the script.
    HandleArrayMap          mShaderCallLUT;         // Lookup table that directly maps named functions to the description in the above array.
    cgConstantBufferLinker* mConstantBufferLinker;  // Provides support for generating hardware / API specific information relating to script defined constant buffers.
};

#endif // !_CGE_CGSURFACESHADERSCRIPT_H_