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
// Name : cgSurfaceShader.h                                                  //
//                                                                           //
// Desc : Wrapper about the ISurfaceShader scriptable interface that         //
//        provides support for describing, compiling and applying vertex and //
//        pixel shader permutations, DirectX effect technique like script    //
//        method access, and configurable / dynamic material property        //
//        definitions. This is effectively Carbon's portable and extended    //
//        effect file architecture analogue.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSURFACESHADER_H_ )
#define _CGE_CGSURFACESHADER_H_

//-----------------------------------------------------------------------------
// cgSurfaceShader Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgScript.h>
#include <Resources/cgResourceHandles.h>
#include <Rendering/cgRenderingTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgRenderDriver;
struct cgConstantBufferDesc;
struct cgConstantTypeDesc;
class cgConstantBufferLinker;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {ADC78F59-97A1-45D3-96D8-F3B2B079B5C8}
const cgUID RTID_SurfaceShaderResource = {0xADC78F59, 0x97A1, 0x45D3, {0x96, 0xD8, 0xF3, 0xB2, 0xB0, 0x79, 0xB5, 0xC8}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSurfaceShader (Class)
/// <summary>
/// Wrapper about the ISurfaceShader scriptable interface that provides 
/// support for describing, compiling and applying vertex and pixel shader 
/// permutations, DirectX effect technique like script method access, and 
/// configurable / dynamic material property definitions. This is effectively 
/// Carbon's portable and extended effect file architecture analogue.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSurfaceShader : public cgResource
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSurfaceShader, cgResource, "SurfaceShader" )
    
public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgScriptArgument::Array DefaultShaderArgs;

    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( cgConstantTypeDesc, ConstTypeArray )
    CGE_VECTOR_DECLARE( cgConstantBufferDesc, ConstBufferArray )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSurfaceShader( cgUInt32 referenceId, const cgInputStream & shaderScript );
    virtual ~cgSurfaceShader( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgInt32                 findSamplerRegister     ( const cgString & name ) const;
    cgInt32                 findSamplerRegister     ( const cgString & sourceNamespace, const cgString & name ) const;
    bool                    getConstantBufferDesc   ( cgInt32 handle, cgConstantBufferDesc & description ) const;
    bool                    getConstantBufferDesc   ( const cgString & name, cgConstantBufferDesc & description ) const;
    bool                    getConstantBufferDesc   ( const cgString & sourceNamespace, const cgString & name, cgConstantBufferDesc & description ) const;
    const ConstBufferArray& getConstantBuffers      ( ) const;
    bool                    getConstantTypeDesc     ( cgInt32 handle, cgConstantTypeDesc & description ) const;
    bool                    getConstantTypeDesc     ( const cgString & name, cgConstantTypeDesc & description ) const;
    bool                    getConstantTypeDesc     ( const cgString & sourceNamespace, const cgString & name, cgConstantTypeDesc & description ) const;
    const ConstTypeArray  & getConstantTypes        ( ) const;
    cgConstantBufferLinker* getConstantBufferLinker ( ) const;

    // Permutation Selection
    bool                    selectVertexShader      ( const cgVertexShaderHandle & shader );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11 );
    bool                    selectVertexShader      ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11, const cgVariant & arg12 );
    bool                    selectVertexShader      ( const cgTChar * strName, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );
    bool                    selectVertexShader      ( const cgString & name, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );
    
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11, const cgVariant & arg12 );
    cgVertexShaderHandle    getVertexShader         ( const cgString & name, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );


    bool                    selectPixelShader       ( const cgPixelShaderHandle & shader );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11 );
    bool                    selectPixelShader       ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11, const cgVariant & arg12 );
    bool                    selectPixelShader       ( const cgTChar * strName, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );
    bool                    selectPixelShader       ( const cgString & name, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );

    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgVariant & arg1, const cgVariant & arg2, const cgVariant & arg3, const cgVariant & arg4, const cgVariant & arg5, const cgVariant & arg6, const cgVariant & arg7, const cgVariant & arg8, const cgVariant & arg9, const cgVariant & arg10, const cgVariant & arg11, const cgVariant & arg12 );
    cgPixelShaderHandle     getPixelShader          ( const cgString & name, const cgScriptArgument::Array & aArgs = DefaultShaderArgs );
    
    // Techniques
    cgScriptFunctionHandle  getTechnique            ( const cgString & techniqueName );
    bool                    selectTechnique         ( const cgString & techniqueName );
    bool                    selectTechnique         ( cgScriptFunctionHandle handle );
    bool                    beginTechnique          ( );
    bool                    beginTechnique          ( const cgString & techniqueName );
    bool                    beginTechnique          ( cgScriptFunctionHandle handle );
    bool                    commitChanges           ( );
    cgTechniqueResult::Base executeTechniquePass    ( );
    void                    endTechnique            ( );

    // Script variables
    bool                    setInt                  ( const cgString & name, cgInt32 value );
    bool                    setFloat                ( const cgString & name, cgFloat value );
    bool                    setDouble               ( const cgString & name, cgDouble value );
    bool                    setBool                 ( const cgString & name, bool value );
    bool                    setVector               ( const cgString & name, const cgVector2 & value );
    bool                    setVector               ( const cgString & name, const cgVector3 & value );
    bool                    setVector               ( const cgString & name, const cgVector4 & value );
    bool                    setMatrix               ( const cgString & name, const cgMatrix & value );
    bool                    setValue                ( const cgString & name, void * data, size_t size ); 
    
    bool                    getInt                  ( const cgString & name, cgInt32 & value );
    bool                    getFloat                ( const cgString & name, cgFloat & value );
    bool                    getDouble               ( const cgString & name, cgDouble & value );
    bool                    getBool                 ( const cgString & name, bool & value );
    bool                    getVector               ( const cgString & name, cgVector2 & value );
    bool                    getVector               ( const cgString & name, cgVector3 & value );
    bool                    getVector               ( const cgString & name, cgVector4 & value );
    bool                    getMatrix               ( const cgString & name, cgMatrix & value );
    bool                    getValue                ( const cgString & name, void * data, size_t size ); 
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_SurfaceShaderResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    cgToDo( "Carbon General", "Find a nice way to make these private/protected" )
    static void             emitCode            ( const cgString & code );
    static void             pushCodeOutput      ( cgString * output );
    static void             popCodeOutput       ( );
    static cgString         generateShaderCall  ( const cgString & declaration, const cgString & functionName, const cgString & parameters, const cgString & code );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_DEQUE_DECLARE   ( cgString*, OutputStack )
    CGE_SET_DECLARE     ( cgShaderIdentifier, ShaderSet )
    CGE_MAP_DECLARE     ( cgString, cgScriptFunctionHandle, TechniqueLUT );

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    clearGlobals        ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Resource Loading
    cgInputStream           mShaderScriptFile;          // File from which shader is being loaded.

    // Internal Resource Data
    cgRenderDriver        * mDriver;                    // Render driver associated with this surface shader.
    cgScriptHandle          mScript;                    // Handle to the script that defines this surface shader.
    cgScriptObject        * mScriptObject;              // Reference to the instantiated script side ISurfaceShader object.
    cgString                mShaderClass;               // Name of the surface shader class instantiated by the script.
    cgShaderIdentifier    * mIdentifier;                // Identifier structure used in the selection of a vertex / pixel shader.

    // Script globals
    cgString              * mShaderCode;                // Address of script-side global string variable that contains most recently generated shader code.
    cgString              * mShaderInputs;              // Address of script-side global string variable that contains most recently requested shader inputs.
    cgString              * mShaderOutputs;             // Address of script-side global string variable that contains most recently requested shader outputs.
    cgString              * mShaderGlobals;             // Address of script-side global string variable that contains most recently requested global shader code.
    cgString              * mConstantBufferRefs;        // Address of script-side global string variable that contains most recently requested cbuffer references.
    cgString              * mSamplerBlockRefs;          // Address of script-side global string variable that contains most recently requested sampler block references.

    // Techniques
    TechniqueLUT            mTechniqueLUT;              // Lookup table for technique method name to method identifier.
    cgScriptFunctionHandle  mCurrentTechnique;          // Handle of the currently selected class technique method.
    bool                    mTechniqueInProgress;       // Currently within 'beginTechnique' / 'endTechnique' calls?
    cgInt32                 mCurrentPass;               // Which pass of the technique are we currently executing?
    cgTechniqueResult::Base mLastResult;                // Last result returned from a technique pass execution.
    cgScriptArgument::Array mNoCommitArgs;              // Cached script argument array (saves having to repopulate each time)
    cgScriptArgument::Array mCommitArgs;                // Cached script argument array (saves having to repopulate each time)

    // General
    ShaderSet               mFailedVertexShaders;       // Set of vertex shaders that have failed to compile
    ShaderSet               mFailedPixelShaders;        // Set of pixel shaders that have failed to compile

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    static OutputStack      mShaderCodeOutputStack;     // Keeps track of the current __shx variable to which code is being output.
    static cgString       * mShaderGlobalCodeOutput;    // Current output string into which global code is placed.
};

#endif // !_CGE_CGSURFACESHADER_H_