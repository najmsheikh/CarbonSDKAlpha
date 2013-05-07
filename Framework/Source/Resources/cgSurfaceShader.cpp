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
// Name : cgSurfaceShader.cpp                                                //
//                                                                           //
// Desc : Wrapper about the ISurfaceShader scriptable interface that         //
//        provides support for describing, compiling and applying vertex and //
//        pixel shader permutations, DirectX effect technique like script    //
//        method access, and configurable / dynamic material property        //
//        definitions. This is effectively Carbon's portable and extended    //
//        effect file architecture analogue.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSurfaceShader Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgHardwareShaders.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShaderScript.h>
#include <Resources/cgConstantBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Scripting/cgScriptEngine.h>
#include <System/cgStringUtility.h>
#include <Math/cgChecksum.h>

// Angelscript.
#include <angelscript.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
const cgScriptArgument::Array cgSurfaceShader::DefaultShaderArgs;
cgSurfaceShader::OutputStack cgSurfaceShader::mShaderCodeOutputStack;
cgString * cgSurfaceShader::mShaderGlobalCodeOutput = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgSurfaceShader Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSurfaceShader () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShader::cgSurfaceShader( cgUInt32 nReferenceId, const cgInputStream & ShaderScript ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mShaderScriptFile     = ShaderScript;
    mIdentifier           = CG_NULL;
    mScriptObject         = CG_NULL;
    mShaderCode           = CG_NULL;
    mShaderInputs         = CG_NULL;
    mShaderOutputs        = CG_NULL;
    mShaderGlobals        = CG_NULL;
    mConstantBufferRefs   = CG_NULL;
    mSamplerBlockRefs     = CG_NULL;
    mCurrentTechnique     = CG_NULL;
    mTechniqueInProgress  = false;
    mCurrentPass          = 0;

    // Populate pre-cached script technique method argument arrays
    // for both 'commitChanges' and standard 'executeTechniquePass' calls.
    static const bool bCommit   = true;
    static const bool bNoCommit = false;
    mNoCommitArgs.reserve( 2 );
    mNoCommitArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &mCurrentPass ) );
    mNoCommitArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bNoCommit ) );
    mCommitArgs.reserve( 2 );
    mCommitArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &mCurrentPass ) );
    mCommitArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bCommit ) );

    // Cached responses
    mResourceType      = cgResourceType::SurfaceShader;
    mCanEvict         = ( ShaderScript.getType() == cgStreamType::File || ShaderScript.getType() == cgStreamType::MappedFile );
}

//-----------------------------------------------------------------------------
//  Name : ~cgScript () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShader::~cgSurfaceShader( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSurfaceShader::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base.
    if ( bDisposeBase == true )
        cgResource::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SurfaceShaderResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::loadResource( )
{
    STRING_CONVERT;  // For string conversion macro

    // Bail if we are already loaded
    if ( mResourceLoaded == true )
        return true;

    // Load the script
    if ( mManager->loadSurfaceShaderScript( &mScript, mShaderScriptFile, 0, cgDebugSource() ) == false )
    {
        // Only bail at this point if we're in sandbox mode.
        if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
            return false;

    } // End if failed to load script

    // Script can only be invalid at this point when in sandbox mode if the
    // file was not found (to ensure that information about the resource file
    // is not lost).
    cgScript * pScript = mScript.getResource(true);
    if ( pScript )
    {
        // Convert the module name we're going to use to a standard ANSI
        // string if the application is currently using unicode.
        const cgChar * strModuleName = stringConvertT2CA( pScript->getResourceName().c_str() );

        // Retrieve the module resposible for managing this script.
        cgScriptEngine  * pScriptEngine = pScript->getScriptEngine();
        asIScriptEngine * pInternalEngine = pScriptEngine->getInternalEngine();
        asIScriptModule * pModule = pInternalEngine->GetModule( strModuleName );
        
        // Retrieve indices for the global shader compilation variables
        // (__shx, __shi, __sho etc.)
        int nSHX = pModule->GetGlobalVarIndexByName( "__shx" ); // Shader code
        int nSHI = pModule->GetGlobalVarIndexByName( "__shi" ); // Shader inputs
        int nSHO = pModule->GetGlobalVarIndexByName( "__sho" ); // Shader outputs
        int nSHG = pModule->GetGlobalVarIndexByName( "__shg" ); // Shader globals
        int nCBR = pModule->GetGlobalVarIndexByName( "__cbr" ); // Constant buffer references
        int nSBR = pModule->GetGlobalVarIndexByName( "__sbr" ); // Sampler block references

        // Resolve to their addresses.
        mShaderCode           = (cgString*)pModule->GetAddressOfGlobalVar( nSHX );
        mShaderInputs         = (cgString*)pModule->GetAddressOfGlobalVar( nSHI );
        mShaderOutputs        = (cgString*)pModule->GetAddressOfGlobalVar( nSHO );
        mShaderGlobals        = (cgString*)pModule->GetAddressOfGlobalVar( nSHG );
        mConstantBufferRefs   = (cgString*)pModule->GetAddressOfGlobalVar( nCBR );
        mSamplerBlockRefs     = (cgString*)pModule->GetAddressOfGlobalVar( nSBR );
        
        // Ask the script to create the script side ISurfaceShader object via the 
        // (non-optional) global 'createSurfaceShader' function.
        try
        {
            cgScriptArgument::Array ScriptArgs;
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("SurfaceShader@+"), (void*)this ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("RenderDriver@+"), (void*)mManager->getRenderDriver() ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("ResourceManager@+"), (void*)mManager ) );
            mScriptObject = pScript->executeFunctionObject( _T("ISurfaceShader"), _T("createSurfaceShader"), ScriptArgs );

        } // End try to execute

        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute createSurfaceShader() function in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;

        } // End catch exception

        if ( mScriptObject == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("No valid shader object was returned from createSurfaceShader() function in '%s'.\n"), pScript->getResourceName().c_str() );
            return false;
        
        } // End if failed

        // Cache pixel/vertex shader selection data to save having to do it
        // constantly at runtime.
        mDriver = mManager->getRenderDriver();
        mShaderClass = mScriptObject->getTypeName();
        mIdentifier = new cgShaderIdentifier();
        const cgScript::SourceFileArray & ScriptFiles = pScript->getSourceInfo();
        mIdentifier->sourceFiles.resize( ScriptFiles.size() );
        for ( size_t i = 0; i < ScriptFiles.size(); ++i )
        {
            mIdentifier->sourceFiles[i].name = ScriptFiles[i].name;
            memcpy( mIdentifier->sourceFiles[i].hash, ScriptFiles[i].hash, 5 * sizeof(cgUInt32) );
        
        } // Next File

    } // End if valid script

    // Resource is now loaded
    mResourceLoaded = true;
    mResourceLost   = false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::unloadResource( )
{
    // Release any script objects we retain.
    if ( mScriptObject != CG_NULL )
        mScriptObject->release();

    // Destroy objects.
    delete mIdentifier;

    // Clear out failed shader list for next load attempt.
    mFailedVertexShaders.clear();
    mFailedPixelShaders.clear();

    // Clear out look up tables.
    mTechniqueLUT.clear();
    
    // Allow the script to be released as necessary.
    mScript.close();

    // Clear variables
    mIdentifier           = CG_NULL;
    mScriptObject         = CG_NULL;
    mShaderCode           = CG_NULL;
    mShaderInputs         = CG_NULL;
    mShaderOutputs        = CG_NULL;
    mShaderGlobals        = CG_NULL;
    mConstantBufferRefs   = CG_NULL;
    mSamplerBlockRefs     = CG_NULL;
    mCurrentTechnique     = CG_NULL;
    mTechniqueInProgress  = false;
    mCurrentPass          = 0;

    // Resource is no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getShaderIdentifier ()
/// <summary>
/// Retrieve the unique shader identifier that was generated for this
/// surface shader, based on the hash of the source file and all of its
/// included dependencies. Returns NULL if the shader was not successfully
/// created.
/// </summary>
//-----------------------------------------------------------------------------
cgShaderIdentifier * cgSurfaceShader::getShaderIdentifier( ) const
{
    return mIdentifier;
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader by handle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgVertexShaderHandle & hShader )
{
    cgAssert( mDriver != CG_NULL );
    return mDriver->setVertexShader( hShader );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1 )
{
    cgScriptArgument::Array aArgs(1);
    aArgs[0] = cgScriptArgument( Arg1 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2 )
{
    cgScriptArgument::Array aArgs(2);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3 )
{
    cgScriptArgument::Array aArgs(3);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4 )
{
    cgScriptArgument::Array aArgs(4);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5 )
{
    cgScriptArgument::Array aArgs(5);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6 )
{
    cgScriptArgument::Array aArgs(6);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7 )
{
    cgScriptArgument::Array aArgs(7);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8 )
{
    cgScriptArgument::Array aArgs(8);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9 )
{
    cgScriptArgument::Array aArgs(9);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10 )
{
    cgScriptArgument::Array aArgs(10);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11 )
{
    cgScriptArgument::Array aArgs(11);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11, const cgVariant & Arg12 )
{
    cgScriptArgument::Array aArgs(12);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    aArgs[11] = cgScriptArgument( Arg12 );
    return selectVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgTChar * strName, const cgScriptArgument::Array & aArgs )
{
    // Validate requirements.
    cgAssert( mDriver != CG_NULL );

    // Retrieve the vertex shader from the cache or compile on demand.
    cgVertexShaderHandle hShader = getVertexShader( strName, aArgs );
    if ( hShader.isValid() )
    {
        // Set to the driver.
        mDriver->setVertexShader( hShader );
        return true;
    
    } // End if success

    // Failed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : selectVertexShader ()
/// <summary>
/// Select the vertex shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectVertexShader( const cgString & strName, const cgScriptArgument::Array & aArgs )
{
    // Validate requirements.
    cgAssert( mDriver != CG_NULL );

    // Retrieve the vertex shader from the cache or compile on demand.
    cgVertexShaderHandle hShader = getVertexShader( strName, aArgs );
    if ( hShader.isValid() )
    {
        // Set to the driver.
        mDriver->setVertexShader( hShader );
        return true;
    
    } // End if success

    // Failed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1 )
{
    cgScriptArgument::Array aArgs(1);
    aArgs[0] = cgScriptArgument( Arg1 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2 )
{
    cgScriptArgument::Array aArgs(2);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3 )
{
    cgScriptArgument::Array aArgs(3);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4 )
{
    cgScriptArgument::Array aArgs(4);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5 )
{
    cgScriptArgument::Array aArgs(5);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6 )
{
    cgScriptArgument::Array aArgs(6);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7 )
{
    cgScriptArgument::Array aArgs(7);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8 )
{
    cgScriptArgument::Array aArgs(8);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9 )
{
    cgScriptArgument::Array aArgs(9);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10 )
{
    cgScriptArgument::Array aArgs(10);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11 )
{
    cgScriptArgument::Array aArgs(11);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Get the vertex shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11, const cgVariant & Arg12 )
{
    cgScriptArgument::Array aArgs(12);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    aArgs[11] = cgScriptArgument( Arg12 );
    return getVertexShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Retrieve the vertex shader with the specified name / compilation arguments. 
/// The shader will be compiled on-demand if it has not already been compiled, 
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexShaderHandle cgSurfaceShader::getVertexShader( const cgString & strName, const cgScriptArgument::Array & aArgs )
{
    // Finalize the shader identifier that will allow us to find the specified shader. 
    // Build the shader identifier first. Note: We append the surface shader class name 
    // to the end of the shader function name to allow us to differentiate between two
    // shader functions that have the same name but come from different shader classes. 
    // Note: The following 'optimized' set of string copies works out faster than the
    // original string concatenation
    //
    // mIdentifier->shaderIdentifier = strName + _T("::") + mShaderClass;
    //
    // Optimized version below (almost twice as fast).
    const size_t nNameSize  = strName.size();
    const size_t nClassSize = mShaderClass.size();
    mIdentifier->shaderIdentifier.resize( nNameSize + 2 + nClassSize );
    memcpy( &mIdentifier->shaderIdentifier[0], &strName[0], nNameSize * sizeof(cgTChar) );
    memcpy( &mIdentifier->shaderIdentifier[nNameSize], _T("::"), 2 * sizeof(cgTChar) );
    memcpy( &mIdentifier->shaderIdentifier[nNameSize+2], &mShaderClass[0], nClassSize * sizeof(cgTChar) );

    // Now copy the shader argument data (always assumed to be 32bit)
    size_t nCurrent = 0;
    mIdentifier->parameterData.resize( aArgs.size() );
    for ( size_t i = 0; i < aArgs.size(); ++i )
    {
        switch ( aArgs[i].type )
        {
            case cgScriptArgumentType::Bool:
            case cgScriptArgumentType::Byte:
                mIdentifier->parameterData[nCurrent++] = *((cgUInt8*)aArgs[i].data);
                break;
            case cgScriptArgumentType::Word:
                mIdentifier->parameterData[nCurrent++] = *((cgUInt16*)aArgs[i].data);
                break;
            case cgScriptArgumentType::Array:
            {
                // Iterate through and copy data as required. We acknowledge and accept
                // the fact that the std::vector<> casts we do here may not match the
                // original types; but in each case except the bool we do not allow for
                // specializations anyway and thus the class should still function.
                switch ( aArgs[i].subType )
                {
                    case cgScriptArgumentType::Bool:
                    {
                        const std::vector<bool> & aValues = *((std::vector<bool>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array to account for the
                        // number of elements in the array (1 less than the actual array size
                        // given the fact that the array itself was considered a parameter when
                        // the array was first resized).
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j] ? 1 : 0;
                        break;
                    
                    } // End case Bool
                    case cgScriptArgumentType::Byte:
                    {
                        const std::vector<cgByte> & aValues = *((std::vector<cgUInt8>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;
                    
                    } // End case Byte
                    case cgScriptArgumentType::Word:
                    {
                        const std::vector<cgUInt16> & aValues = *((std::vector<cgUInt16>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;
                    
                    } // End case Word
                    case cgScriptArgumentType::QWord:
                    {
                        const std::vector<cgUInt64> & aValues = *((std::vector<cgUInt64>*)aArgs[i].data);
                        
                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = (cgUInt32)aValues[j]; // TRUNCATES!
                        break;
                    
                    } // End case QWord
                    default:
                    {
                        const std::vector<cgUInt32> & aValues = *((std::vector<cgUInt32>*)aArgs[i].data);
                        
                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;

                    } // End default case

                } // End switch sub type

                break;
            
            } // End case Array
            default:
                // Assume 32bit
                mIdentifier->parameterData[nCurrent++] = *((cgUInt32*)aArgs[i].data);
                break;

        } // End switch type

    } // Next argument

    // Shrink parameter data to the size that was actually required.
    if ( mIdentifier->parameterData.size() != nCurrent )
        mIdentifier->parameterData.resize(nCurrent);

    // With the identifier set up, now attempt to find this shader permutation
    // within the resource manager's internal shader dictionary or on-disk cache.
    cgVertexShaderHandle hShader;
    static const cgString strResourceFile = CGTEXT(__FILE__);
    if ( mManager->loadVertexShader( &hShader, mIdentifier, 0, cgDebugSourceInfo(strResourceFile,__LINE__) ) )
    {
        // Set the automatic management time-out to 60 seconds
        // to ensure it is unloaded if no longer used.
        // ToDo: Don't hardcode. Or choose an alternate management method.
        hShader->setDestroyDelay( 60.0f );

        // The shader was already available. Just retrieve it.
        return hShader;

    } // End if shader loaded
    else
    {
        // Already in the failed list?
        if ( mFailedVertexShaders.find( *mIdentifier ) != mFailedVertexShaders.end() )
            return cgVertexShaderHandle::Null;

        // This shader / permutation has not been encountered before. Compile it.
        // First clear out the script globals that will be populated by the shader generator
        clearGlobals();

        // Also set the current shader code output string to the default
        // *top level* code string (__shadercall support).
        mShaderCodeOutputStack.clear();
        mShaderCodeOutputStack.push_back( mShaderCode );

        // Same for current global code output
        mShaderGlobalCodeOutput = mShaderGlobals;

        // Call the referenced function.
        bool bResult = false;
        try
        {
            bResult = mScriptObject->executeMethodBool( strName, aArgs );
        
        } // End try to execute

        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute surface shader method '%s()' when compiling vertex shader from '%s'. The engine reported the following error: %s.\n"), strName.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;

        } // End catch exception

        // If the shader generation process failed, we're done.
        if ( !bResult )
            return false;
        
        // Also retrieve the common shader code by calling any available '__shcmn' 
        // method which returns a string.
        cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResource(true);
        cgString strCommon( pScript->executeFunctionString( _T("__shsyscmnvs"), cgScriptArgument::Array(), true ) );
        strCommon.append( mScriptObject->executeMethodString( _T("__shcmn"), cgScriptArgument::Array(), true ) );

        // Which shader model are we targeting?
        bool bModel4Plus = (pScript->isMacroDefined( _T("DX10") ) | pScript->isMacroDefined( _T("DX11") ));
        
        // Build shader function signature.
        cgString strSignature;
        strSignature.append( _T("void ") );
        strSignature.append( strName );
        strSignature.append( _T("(") );
        
        // Add inputs to shader function signature first
        cgStringArray aTokens;
        cgString strInputSignature;
        size_t nInputParams = 0, nOutputParams = 0;
        cgStringUtility::tokenize( *mShaderInputs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strInput = aTokens[i].trim();
            if ( strInput.empty() || strInput.beginsWith(_T("//") ) )
                continue;
                
            // Extract the type name and semantic of the input to use as a special 'input signature' 
            // string. This will be turned into a hash that is eventually used to identify all shaders
            // that use precisely the same inputs, in the same order. The ability to detect this is 
            // necessary for optimal usage with DX11's input layout system. First, find the type.
            size_t nTypeEnd = strInput.find_first_of( _T(" ") );
            if ( nTypeEnd != cgString::npos )
            {
                cgString strInputType = strInput.substr( 0, nTypeEnd );

                // Has an array declaration?
                size_t nArrayStart = strInput.find_last_of( _T("[") );
                if ( nArrayStart != cgString::npos )
                {
                    size_t nArrayEnd = strInput.find_first_of( _T("]"), nArrayStart );
                    if ( nArrayEnd != cgString::npos )
                        strInputType.append( strInput.substr( nArrayStart, (nArrayEnd - nArrayStart) + 1 ) );

                } // End if array start
                
                // Now find the semantic.
                size_t nSemanticStart = strInput.find_last_of( _T(":") );
                if ( nSemanticStart != cgString::npos )
                {
                    cgString strInputSemantic = strInput.substr( nSemanticStart + 1 );

                    // Append to the input type and semantic (but not name) to
                    // the input signature.
                    if ( !strInputSignature.empty() )
                        strInputSignature.append( _T(",") );
                    strInputSignature.append( strInputType );
                    strInputSignature.append( _T(" ") );
                    strInputSignature.append( cgString::trim(strInputSemantic) );

                } // End if found ':'

            } // End if found '<space>'

            // Add full input (including name) to shader function signature.
            if ( nInputParams++ != 0 )
                strSignature.append( _T(", ") );
            strSignature.append( strInput );
        
        } // Next Input

        // Generate the unique hash based on the input signature.
        {
            STRING_CONVERT;
            cgChecksum::SHA1 HashGenerator;
            HashGenerator.beginMessage( );
            HashGenerator.messageData( stringConvertT2CA( cgString::toUpper(strInputSignature).c_str() ), strInputSignature.size() );
            HashGenerator.endMessage( );
            HashGenerator.getHash( mIdentifier->inputSignatureHash );
        
        } // End scope

        // Now outputs
        cgStringUtility::tokenize( *mShaderOutputs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strOutput = aTokens[i].trim();
            if ( strOutput.empty() || strOutput.beginsWith(_T("//") ) )
                continue;
            
            // Add to signature
            if ( ((nOutputParams++) + nInputParams) != 0 )
                strSignature.append( _T(", out ") );
            else
                strSignature.append( _T("out ") );
            strSignature.append( strOutput );
        
        } // Next Output
        
        // Finish up signature
        strSignature += _T(")");

        // Now process referenced constant-buffers in order to inject the 
        // appropriate constant buffer declarations. Our first job is to 
        // construct a set of referenced CBuffers for this shader.
        cgInt32Array aBufferHandles;
        cgStringSet ReferencedBufferNames;
        cgStringUtility::tokenize( *mConstantBufferRefs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strCBufferName = aTokens[i].trim();
            if ( strCBufferName.empty() )
                continue;

            // Skip if we have already processed a buffer by this name.
            if ( ReferencedBufferNames.find(strCBufferName) != ReferencedBufferNames.end() )
                continue;

            // Execute the method that returns the cbuffer identifier handle.
            bResult = false;
            cgInt32 nCBufferHandle = mScriptObject->executeMethodInt( _T("__shcb_") + strCBufferName, cgScriptArgument::Array(), true, &bResult );
            if ( !bResult )
            {
                // Try as a global function?
                nCBufferHandle = pScript->executeFunctionInt( _T("__shcb_") + strCBufferName, cgScriptArgument::Array(), true, &bResult );
                if ( !bResult )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Unresolved constant buffer symbol '%s' referenced by shader method '%s()' in '%s'.\n"), strCBufferName.c_str(), strName.c_str(), getResourceName().c_str() );
                    return false;
                
                } // End if !global cbuffer
            
            } // End if !class cbuffer

            // Record buffer reference.
            ReferencedBufferNames.insert( strCBufferName );
            aBufferHandles.push_back( nCBufferHandle );

        } // Next CBuffer

        // Now generate the shader code (or retrieve from the cache) for each of the
        // above constant buffers. 
        cgString strConstants;
        if ( !aBufferHandles.empty() )
        {
            // In order to generate the correct code for the current  shader API 
            // (i.e. DX9 HLSL vs. DX10 HLSL vs GLSL) we need access to the API
            // sensitive constant buffer linker object. This object parses type and
            // / cbuffer  descriptors in order to generate a tightly packed set of 
            // shader constant variables and / or structures.
            cgConstantBufferLinker * pLinker = pScript->getConstantBufferLinker();
            
            // Generate shader code only for the /specific/ C-buffers that have been referenced.
            if ( !pLinker->generateBufferDeclarations( &aBufferHandles[0], aBufferHandles.size(), strConstants ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to generate final constant buffer declarations referenced by shader method '%s()' in '%s'.\n"), strName.c_str(), getResourceName().c_str() );
                return false;

            } // End if failed

        } // End if any buffers referenced

        // Build the final shader code.
        cgString strShader = (!strCommon.empty()) ? strCommon + _T("\n") : cgString::Empty;
        if ( !strConstants.empty() )
        {
            strShader.append( strConstants );
            strShader.append( _T("\n") );
        } // End if valid constants
        if ( !mShaderGlobals->empty() )
        {
            strShader.append( *mShaderGlobals );
            strShader.append( _T("\n") );

        } // End if valid globals
        strShader.append( strSignature );
        strShader.append( _T("\n{\n") );
        strShader.append( *mShaderCode );
        strShader.append( _T("\n}") );

        #if defined(_DEBUG)
            // Dump original HLSL code
            STRING_CONVERT;
            static int nf = 0;
            nf++;
            cgChar lpszFName[256];
            const cgChar * lpszShaderName = stringConvertT2CA(strName.c_str());
            sprintf( lpszFName, "Build/vs_%s_%04i_HLSL.txt", lpszShaderName, nf );
            FILE * pf = fopen( lpszFName, "w" );
            if ( pf )
            {
                fwrite( stringConvertT2CA( strShader.c_str() ), strShader.size(), 1, pf );
                fclose( pf );
                cgAppLog::write( cgAppLog::Debug, _T("Emitting generated HLSL source code to %s\n"), stringConvertA2CT(lpszFName) );
            }
        #endif

        // Create and attempt to load the vertex shader.
        if ( !mManager->createVertexShader( &hShader, mIdentifier, strShader, strName, 0, cgDebugSource() ) )
        {
            // Insert into the failed list
            mFailedVertexShaders.insert( *mIdentifier );
            return cgVertexShaderHandle::Null;
        
        } // End if failed

        // Set the automatic management time-out to 60 seconds
        // to ensure it is unloaded if no longer used.
        // ToDo: Don't hardcode. Or choose an alternate management method.
        cgVertexShader * pShader = hShader.getResource( false );
        pShader->setDestroyDelay( 60.0f );

        // Return the new handle.
        return hShader;

    } // End if no shader

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader by handle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgPixelShaderHandle & hShader )
{
    cgAssert( mDriver != CG_NULL );
    return mDriver->setPixelShader( hShader );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1 )
{
    cgScriptArgument::Array aArgs(1);
    aArgs[0] = cgScriptArgument( Arg1 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2 )
{
    cgScriptArgument::Array aArgs(2);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3 )
{
    cgScriptArgument::Array aArgs(3);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4 )
{
    cgScriptArgument::Array aArgs(4);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5 )
{
    cgScriptArgument::Array aArgs(5);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6 )
{
    cgScriptArgument::Array aArgs(6);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7 )
{
    cgScriptArgument::Array aArgs(7);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8 )
{
    cgScriptArgument::Array aArgs(8);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9 )
{
    cgScriptArgument::Array aArgs(9);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10 )
{
    cgScriptArgument::Array aArgs(10);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11 )
{
    cgScriptArgument::Array aArgs(11);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11, const cgVariant & Arg12 )
{
    cgScriptArgument::Array aArgs(12);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    aArgs[11] = cgScriptArgument( Arg12 );
    return selectPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgTChar * strName, const cgScriptArgument::Array & aArgs )
{
    // Validate requirements
    cgAssert( mDriver != CG_NULL );

    // Retrieve the pixel shader from the cache or compile on demand.
    cgPixelShaderHandle hShader = getPixelShader( strName, aArgs );
    if ( hShader.isValid() )
    {
        // Set to the driver.
        mDriver->setPixelShader( hShader );
        return true;
    
    } // End if success

    // Failed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : selectPixelShader ()
/// <summary>
/// Select the pixel shader with the specified name / compilation arguments
/// into the render device. The shader will be compiled on-demand if it has not
/// already been compiled, and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectPixelShader( const cgString & strName, const cgScriptArgument::Array & aArgs )
{
    // Validate requirements
    cgAssert( mDriver != CG_NULL );

    // Retrieve the pixel shader from the cache or compile on demand.
    cgPixelShaderHandle hShader = getPixelShader( strName, aArgs );
    if ( hShader.isValid() )
    {
        // Set to the driver.
        mDriver->setPixelShader( hShader );
        return true;
    
    } // End if success

    // Failed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1 )
{
    cgScriptArgument::Array aArgs(1);
    aArgs[0] = cgScriptArgument( Arg1 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2 )
{
    cgScriptArgument::Array aArgs(2);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3 )
{
    cgScriptArgument::Array aArgs(3);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4 )
{
    cgScriptArgument::Array aArgs(4);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5 )
{
    cgScriptArgument::Array aArgs(5);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6 )
{
    cgScriptArgument::Array aArgs(6);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7 )
{
    cgScriptArgument::Array aArgs(7);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8 )
{
    cgScriptArgument::Array aArgs(8);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9 )
{
    cgScriptArgument::Array aArgs(9);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10 )
{
    cgScriptArgument::Array aArgs(10);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11 )
{
    cgScriptArgument::Array aArgs(11);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments.
/// The shader will be compiled on-demand if it has not already been compiled
/// and the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgVariant & Arg1, const cgVariant & Arg2, const cgVariant & Arg3, const cgVariant & Arg4, const cgVariant & Arg5, const cgVariant & Arg6, const cgVariant & Arg7, const cgVariant & Arg8, const cgVariant & Arg9, const cgVariant & Arg10, const cgVariant & Arg11, const cgVariant & Arg12 )
{
    cgScriptArgument::Array aArgs(12);
    aArgs[0] = cgScriptArgument( Arg1 );
    aArgs[1] = cgScriptArgument( Arg2 );
    aArgs[2] = cgScriptArgument( Arg3 );
    aArgs[3] = cgScriptArgument( Arg4 );
    aArgs[4] = cgScriptArgument( Arg5 );
    aArgs[5] = cgScriptArgument( Arg6 );
    aArgs[6] = cgScriptArgument( Arg7 );
    aArgs[7] = cgScriptArgument( Arg8 );
    aArgs[8] = cgScriptArgument( Arg9 );
    aArgs[9] = cgScriptArgument( Arg10 );
    aArgs[10] = cgScriptArgument( Arg11 );
    aArgs[11] = cgScriptArgument( Arg12 );
    return getPixelShader( strName, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Get the pixel shader with the specified name / compilation arguments. The 
/// shader will be compiled on-demand if it has not already been compiled, and 
/// the result will be cached.
/// </summary>
//-----------------------------------------------------------------------------
cgPixelShaderHandle cgSurfaceShader::getPixelShader( const cgString & strName, const cgScriptArgument::Array & aArgs )
{
    // Finalize the shader identifier that will allow us to find the specified shader. 
    // Build the shader identifier first. Note: We append the surface shader class name 
    // to the end of the shader function name to allow us to differentiate between two
    // shader functions that have the same name but come from different shader classes. 
    // Note: The following 'optimized' set of string copies works out faster than the
    // original string concatenation
    //
    // mIdentifier->shaderIdentifier = strName + _T("::") + mShaderClass;
    //
    // Optimized version below (almost twice as fast).
    const size_t nNameSize  = strName.size();
    const size_t nClassSize = mShaderClass.size();
    mIdentifier->shaderIdentifier.resize( nNameSize + 2 + nClassSize );
    memcpy( &mIdentifier->shaderIdentifier[0], &strName[0], nNameSize * sizeof(cgTChar) );
    memcpy( &mIdentifier->shaderIdentifier[nNameSize], _T("::"), 2 * sizeof(cgTChar) );
    memcpy( &mIdentifier->shaderIdentifier[nNameSize+2], &mShaderClass[0], nClassSize * sizeof(cgTChar) );
    
    // Now copy the shader argument data (always assumed to be 32bit)
    size_t nCurrent = 0;
    mIdentifier->parameterData.resize( aArgs.size() );
    for ( size_t i = 0; i < aArgs.size(); ++i )
    {
        switch ( aArgs[i].type )
        {
            case cgScriptArgumentType::Bool:
            case cgScriptArgumentType::Byte:
                mIdentifier->parameterData[nCurrent++] = *((cgUInt8*)aArgs[i].data);
                break;
            case cgScriptArgumentType::Word:
                mIdentifier->parameterData[nCurrent++] = *((cgUInt16*)aArgs[i].data);
                break;
            case cgScriptArgumentType::Array:
            {
                // Iterate through and copy data as required. We acknowledge and accept
                // the fact that the std::vector<> casts we do here may not match the
                // original types; but in each case except the bool we do not allow for
                // specializations anyway and thus the class should still function.
                switch ( aArgs[i].subType )
                {
                    case cgScriptArgumentType::Bool:
                    {
                        const std::vector<bool> & aValues = *((std::vector<bool>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array to account for the
                        // number of elements in the array (1 less than the actual array size
                        // given the fact that the array itself was considered a parameter when
                        // the array was first resized).
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j] ? 1 : 0;
                        break;
                    
                    } // End case Bool
                    case cgScriptArgumentType::Byte:
                    {
                        const std::vector<cgByte> & aValues = *((std::vector<cgUInt8>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;
                    
                    } // End case Byte
                    case cgScriptArgumentType::Word:
                    {
                        const std::vector<cgUInt16> & aValues = *((std::vector<cgUInt16>*)aArgs[i].data);

                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;
                    
                    } // End case Word
                    case cgScriptArgumentType::QWord:
                    {
                        const std::vector<cgUInt64> & aValues = *((std::vector<cgUInt64>*)aArgs[i].data);
                        
                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = (cgUInt32)aValues[j]; // TRUNCATES!
                        break;
                    
                    } // End case QWord
                    default:
                    {
                        const std::vector<cgUInt32> & aValues = *((std::vector<cgUInt32>*)aArgs[i].data);
                        
                        // Resize the parameter data (permutation key) array for the expanded data
                        size_t nArraySize = aValues.size();
                        if ( nArraySize > 0 )
                            mIdentifier->parameterData.resize( mIdentifier->parameterData.size() + (nArraySize - 1) );

                        // Copy values
                        for ( size_t j = 0; j < nArraySize; ++j )
                            mIdentifier->parameterData[nCurrent++] = aValues[j];
                        break;

                    } // End default case

                } // End switch sub type

                break;
            
            } // End case Array
            default:
                // Assume 32bit
                mIdentifier->parameterData[nCurrent++] = *((cgUInt32*)aArgs[i].data);
                break;

        } // End switch type

    } // Next argument

    // Shrink parameter data to the size that was actually required.
    if ( mIdentifier->parameterData.size() != nCurrent )
        mIdentifier->parameterData.resize(nCurrent);

    // With the identifier set up, now attempt to find this shader permutation
    // within the resource manager's internal shader dictionary or on-disk cache.
    cgPixelShaderHandle hShader;
    static const cgString strResourceFile = CGTEXT(__FILE__);
    if ( mManager->loadPixelShader( &hShader, mIdentifier, 0, cgDebugSourceInfo(strResourceFile,__LINE__) ) )
    {
        // Set the automatic management time-out to 60 seconds
        // to ensure it is unloaded if no longer used.
        // ToDo: Don't hardcode. Or choose an alternate management method.
        hShader->setDestroyDelay( 60.0f );

        // The shader was already available. Just retrieve it.
        return hShader;

    } // End if shader loaded
    else
    {
        // Already in the failed list?
        if ( mFailedPixelShaders.find( *mIdentifier ) != mFailedPixelShaders.end() )
            return cgPixelShaderHandle::Null;
        
        // This shader / permutation has not been encountered before. Compile it.
        // First clear out the script globals that will be populated by the shader generator
        clearGlobals();

        // Also set the current shader code output string to the default
        // *top level* code string (__shadercall support).
        mShaderCodeOutputStack.clear();
        mShaderCodeOutputStack.push_back( mShaderCode );

        // Same for current global code output
        mShaderGlobalCodeOutput = mShaderGlobals;

        // Call the referenced function.
        bool bResult = false;
        try
        {
            bResult = mScriptObject->executeMethodBool( strName, aArgs );
        
        } // End try to execute

        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute surface shader method '%s()' when compiling pixel shader from '%s'. The engine reported the following error: %s.\n"), strName.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
            return false;

        } // End catch exception

        // If the shader generation process failed, we're done.
        if ( !bResult )
            return false;
        
        // Retrieve the common shader code by calling any available '__shcmn' 
        // method which returns a string.
        cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResource(true);
        cgString strCommon( pScript->executeFunctionString( _T("__shsyscmnps"), cgScriptArgument::Array(), true ) );
        strCommon.append( mScriptObject->executeMethodString( _T("__shcmn"), cgScriptArgument::Array(), true ) );

        // Same for common samplers by calling any available '__shsmp__default_' 
        // method first of all. This handles the unnamed sampler block that may 
        // have been included, with explicit sampler references handled separately below.
        cgInt32Array aSamplerBlockHandles;
        cgInt32 nDefaultSamplerBlock = mScriptObject->executeMethodInt( _T("__shsmp__default_"), cgScriptArgument::Array(), true, &bResult );
        if ( bResult )
            aSamplerBlockHandles.push_back( nDefaultSamplerBlock );

        // Which shader model are we targeting?
        bool bModel4Plus = (pScript->isMacroDefined( _T("DX10") ) | pScript->isMacroDefined( _T("DX11") ));
        
        // Build shader function signature.
        cgString strSignature;
        strSignature.append( _T("void ") );
        strSignature.append( strName );
        strSignature.append( _T("(") );
        
        // Add inputs to shader function signature first
        cgStringArray aTokens;
        cgString strInputSignature;
        size_t nInputParams = 0, nOutputParams = 0;
        cgStringUtility::tokenize( *mShaderInputs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strInput = aTokens[i].trim();
            if ( strInput.empty() || strInput.beginsWith(_T("//") ) )
                continue;

            // Extract the type name and semantic of the input to use as a special 'input signature' 
            // string. This will be turned into a hash that is eventually used to identify all shaders
            // that use precisely the same inputs, in the same order. The ability to detect this is 
            // necessary for optimal usage with DX11's input layout system. First, find the type.
            size_t nTypeEnd = strInput.find_first_of( _T(" ") );
            if ( nTypeEnd != cgString::npos )
            {
                cgString strInputType = strInput.substr( 0, nTypeEnd );

                // Has an array declaration?
                size_t nArrayStart = strInput.find_last_of( _T("[") );
                if ( nArrayStart != cgString::npos )
                {
                    size_t nArrayEnd = strInput.find_first_of( _T("]"), nArrayStart );
                    if ( nArrayEnd != cgString::npos )
                        strInputType.append( strInput.substr( nArrayStart, (nArrayEnd - nArrayStart) + 1 ) );

                } // End if array start
                
                // Now find the semantic.
                size_t nSemanticStart = strInput.find_last_of( _T(":") );
                if ( nSemanticStart != cgString::npos )
                {
                    cgString strInputSemantic = strInput.substr( nSemanticStart + 1 );

                    // Append to the input type and semantic (but not name) to
                    // the input signature.
                    if ( !strInputSignature.empty() )
                        strInputSignature.append( _T(",") );
                    strInputSignature.append( strInputType );
                    strInputSignature.append( _T(" ") );
                    strInputSignature.append( cgString::trim(strInputSemantic) );

                } // End if found ':'

            } // End if found '<space>'
        
            // Add full input (including name) to shader function signature.
            if ( nInputParams++ != 0 )
                strSignature.append( _T(", ") );
            strSignature.append( strInput );
        
        } // Next Input

        // Generate the unique hash based on the input signature.
        {
            STRING_CONVERT;
            cgChecksum::SHA1 HashGenerator;
            HashGenerator.beginMessage( );
            HashGenerator.messageData( stringConvertT2CA( cgString::toUpper(strInputSignature).c_str() ), strInputSignature.size() );
            HashGenerator.endMessage( );
            HashGenerator.getHash( mIdentifier->inputSignatureHash );
        
        } // End scope

        // Now outputs
        cgStringUtility::tokenize( *mShaderOutputs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strOutput = aTokens[i].trim();
            if ( strOutput.empty() || strOutput.beginsWith(_T("//") ) )
                continue;
            
            // Add to signature
            if ( ((nOutputParams++) + nInputParams) != 0 )
                strSignature.append( _T(", out ") );
            else
                strSignature.append( _T("out ") );
            strSignature.append( strOutput );
        
        } // Next Output
        
        // Finish up signature
        strSignature += _T(")");

        // Now process referenced constant-buffers in order to inject the 
        // appropriate constant buffer declarations. Our first job is to 
        // construct a set of referenced CBuffers for this shader.
        cgInt32Array aBufferHandles;
        cgStringSet ReferencedBufferNames;
        cgStringUtility::tokenize( *mConstantBufferRefs, aTokens, _T(";\n") );
        for ( size_t i = 0; i < aTokens.size(); ++i )
        {
            // Skip empty entries
            cgString strCBufferName = aTokens[i].trim();
            if ( strCBufferName.empty() )
                continue;

            // Skip if we have already processed a buffer by this name.
            if ( ReferencedBufferNames.find(strCBufferName) != ReferencedBufferNames.end() )
                continue;

            // Execute the method that returns the cbuffer identifier handle.
            bResult = false;
            cgInt32 nCBufferHandle = mScriptObject->executeMethodInt( _T("__shcb_") + strCBufferName, cgScriptArgument::Array(), true, &bResult );
            if ( !bResult )
            {
                // Try as a global function?
                nCBufferHandle = pScript->executeFunctionInt( _T("__shcb_") + strCBufferName, cgScriptArgument::Array(), true, &bResult );
                if ( !bResult )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Unresolved constant buffer symbol '%s' referenced by shader method '%s()' in '%s'.\n"), strCBufferName.c_str(), strName.c_str(), getResourceName().c_str() );
                    return false;
                
                } // End if !global cbuffer
            
            } // End if !class cbuffer

            // Record buffer reference.
            ReferencedBufferNames.insert( strCBufferName );
            aBufferHandles.push_back( nCBufferHandle );

        } // Next CBuffer

        // Now generate the shader code (or retrieve from the cache) for each of the
        // above constant buffers. 
        cgString strConstants;
        if ( !aBufferHandles.empty() )
        {
            // In order to generate the correct code for the current  shader API 
            // (i.e. DX9 HLSL vs. DX10 HLSL vs GLSL) we need access to the API
            // sensitive constant buffer linker object. This object parses type and
            // / cbuffer  descriptors in order to generate a tightly packed set of 
            // shader constant variables and / or structures.
            cgConstantBufferLinker * pLinker = pScript->getConstantBufferLinker();
            
            // Generate shader code only for the /specific/ C-buffers that have been referenced.
            if ( !pLinker->generateBufferDeclarations( &aBufferHandles[0], aBufferHandles.size(), strConstants ) )
            {
                cgAppLog::write( cgAppLog::Error, _T("Failed to generate final constant buffer declarations referenced by shader method '%s()' in '%s'.\n"), strName.c_str(), getResourceName().c_str() );
                return false;

            } // End if failed

        } // End if any buffers referenced

        // Now process any explicitly referenced sampler blocks.
        cgStringSet aReferencedSamplerBlocks;
        cgStringArray aSamplerBlocks;
        cgStringUtility::tokenize( *mSamplerBlockRefs, aSamplerBlocks, _T(";\n") );
        for ( size_t i = 0; i < aSamplerBlocks.size(); ++i )
        {
            // Skip empty entries
            cgString strSamplerBlockName = aSamplerBlocks[i].trim();
            if ( strSamplerBlockName.empty() )
                continue;

            // Skip if we have already processed a buffer by this name.
            if ( aReferencedSamplerBlocks.find(strSamplerBlockName) != aReferencedSamplerBlocks.end() )
                continue;

            // Execute the method that returns the sampler block handle
            cgInt32 nSamplerBlock = mScriptObject->executeMethodInt( _T("__shsmp_") + strSamplerBlockName, cgScriptArgument::Array(), true, &bResult );
            if ( !bResult )
            {
                // Try as a global function?
                nSamplerBlock = pScript->executeFunctionInt( _T("__shsmp_") + strSamplerBlockName, cgScriptArgument::Array(), true, &bResult );
                if ( !bResult )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Unresolved sampler block symbol '%s' referenced by shader method '%s()' in '%s'.\n"), strSamplerBlockName.c_str(), strName.c_str(), pScript->getResourceName().c_str() );
                    return false;
                
                } // End if !global sampler block
            
            } // End if !class sampler block

            // Record sampler block references
            aReferencedSamplerBlocks.insert( strSamplerBlockName );
            aSamplerBlockHandles.push_back( nSamplerBlock );
        
        } // Next sampler block

        // Now generate the shader code for each of the referenced sampler blocks.
        cgString strSamplers;
        if ( !aSamplerBlockHandles.empty() )
        {
            cgToDo( "Effect Overhaul", "Shader code can be cached for each sampler block handle" );
            for ( size_t i = 0; i < aSamplerBlockHandles.size(); ++ i )
            {
                cgSamplerBlockDesc sbDesc;
                if ( !pScript->getSamplerBlockDesc( aSamplerBlockHandles[i], sbDesc ) )
                    continue;

                // Process each referenced sampler
                for ( size_t j = 0; j < sbDesc.samplers.size(); ++j )
                {
                    const cgSamplerDesc & sDesc = sbDesc.samplers[j];

                    // Insert an implicit texture object with matching name 
                    // followed by 'Tex' (i.e. 'sDiffuseTex'). First detect / insert 
                    // the texture type if necessary.
                    bool bAppendTexture = true;
                    cgString strFinalTypeName = sDesc.typeName;
                    if ( bModel4Plus )
                    {
                        // Final sampler will be of type 'SamplerState' by default
                        strFinalTypeName = _T("SamplerState");

                        // What action do we need to take for the specified sampler type?
                        if ( sDesc.typeName.compare( _T("Sampler1D"), true ) == 0 )
                            strSamplers.append( _T("Texture1D ") );
                        else if ( sDesc.typeName.compare( _T("Sampler2D"), true ) == 0 )
                            strSamplers.append( _T("Texture2D ") );
                        else if ( sDesc.typeName.compare( _T("Sampler3D"), true ) == 0 )
                            strSamplers.append( _T("Texture3D ") );
                        else if ( sDesc.typeName.compare( _T("SamplerCube"), true ) == 0 )
                            strSamplers.append( _T("TextureCube ") );
                        else if ( sDesc.typeName.compare( _T("Sampler2DCmp"), true ) == 0 )
                        {
                            bAppendTexture = false;
                            strFinalTypeName = _T("SamplerComparisonState");
                        
                        } // End if Sampler2DCmp
                        else
                        {
                            // ToDo: 9999 - Unrecognized sampler type '$strSamplerType'
                            continue;

                        } // End if unknown sampler

                        // ToDo: Support sampler2DMS, sampler2DArray, etc.?
                        // These will be replaced with just Sampler2D, but the texture
                        // inserted will be of the appropriate type.

                    } // End if >= Model4
                    else
                    {
                        strSamplers.append( _T("texture ") );

                    } // End if < Model4

                    // Append texture name (${sDesc.name}Tex)
                    if ( bAppendTexture )
                    {
                        strSamplers.append( sDesc.name );
                        strSamplers.append( _T("Tex") );

                        // If the sampler was bound, bind the texture to the
                        // associated texture register.
                        if ( sDesc.registerIndex >= 0 )
                            strSamplers.append( cgString::format( _T(" : register(t%i)"), sDesc.registerIndex ) );

                        // End texture line.
                        strSamplers.append( _T(";\n") );
                    
                    } // End if append texture
    
                    // Append actual sampler.
                    strSamplers.append( strFinalTypeName );
                    strSamplers.append( _T(" ") );
                    strSamplers.append( sDesc.name );
                    if ( sDesc.registerIndex >= 0 )
                        strSamplers.append( cgString::format( _T(" : register(s%i)"), sDesc.registerIndex ) );
                    strSamplers.append( _T(";\n") );

                } // Next Sampler

            } // Next Sampler Block

        } // End if samplers referenced

        cgToDo( "Effect Overhaul", "Remove when complete." );
        /*// Parse referenced (individual) samplers.
        if ( !strSamplers.empty() )
        {
            // Extract samplers (re-output to original sampler string)
            cgStringArray aSamplers;
            cgStringUtility::tokenize( strSamplers, aSamplers, _T(";\n") );
            strSamplers.clear();
            for ( size_t i = 0; i < aSamplers.size(); ++i )
            {
                // Skip empty entries
                cgString strSampler = aSamplers[i].trim();
                if ( strSampler.empty() || strSampler.beginsWith(_T("//") ) )
                    continue;

                // Extract up to the first white-space (should be type)
                cgInt nCurTokenPos = strSampler.find_first_of( _T(" \t") );
                if ( nCurTokenPos == cgString::npos )
                    continue;

                // Retrieve the type.
                cgString strSamplerType = strSampler.substr( 0, nCurTokenPos );

                // Move on to the next non-whitespace character which should
                // signal the start of the identifier / name.
                nCurTokenPos = strSampler.find_first_not_of( _T(" \t"), nCurTokenPos );
                if ( nCurTokenPos == cgString::npos )
                    continue;

                // Find the end of the identifier / name which should end
                // at the next colon or the end of the string itself.
                cgInt nEndPos = strSampler.find_first_of( _T(":"), nCurTokenPos );
                if ( nEndPos == cgString::npos )
                    nEndPos = strSampler.size();

                // Extract the identifier name (including array size if any).
                cgString strSamplerIdentifier = cgString::trim( strSampler.substr( nCurTokenPos, nEndPos - nCurTokenPos ) );
                nCurTokenPos = nEndPos + 1;

                // Now we need to determine which sampler register this is bound to (if any).
                // Find the opening parenthesis.
                cgInt nSamplerRegister = -1; // -1 = not bound
                cgInt nOpenParen = strSampler.find_first_of( _T("("), nCurTokenPos );
                if ( nOpenParen != cgString::npos )
                {
                    cgString strBindType = cgString::trim(strSampler.substr( nCurTokenPos, nOpenParen - nCurTokenPos ));
                    nCurTokenPos = nOpenParen;
                    if ( strBindType.compare( _T("register"), true ) == 0 )
                    {
                        // This is a register binding. Find the closing paren.
                        cgInt nCloseParen = strSampler.find_last_of( _T(")") );
                        if ( nCloseParen != cgString::npos )
                        {
                            // Extract the value from between the parens
                            cgString strValue = cgString::trim(strSampler.substr( nOpenParen + 1, (nCloseParen - nOpenParen) - 1 ));

                            // Find the first numeric character (we'll ignore any starting characters like 's5').
                            cgInt nIndexStart = strValue.find_first_of( _T("0123456789") );
                            if ( nIndexStart != std::string::npos )
                            {
                                // Convert from string to an integer number.
                                cgStringParser Parser( strValue.substr( nIndexStart ) );
                                Parser >> nSamplerRegister;

                            } // End if found numeric char
                            else
                            {
                                // ToDo: 9999 - Expected numeric register identifier
                                continue;

                            } // End if !number

                        } // End if found ')'

                    } // End if 'register'

                } // End if found '('

                // Insert an implicit texture object with matching name
                // appended with 'Texture' (i.e. 'sDiffuseTexture'). First
                // detect / insert the texture type.
                #if (COMPILERMODE==DX11 || COMPILERMODE==DX10)
                    if ( strSamplerType.compare( _T("Sampler1D"), true ) == 0 )
                        strSamplers.append( _T("Texture1D ") );
                    else if ( strSamplerType.compare( _T("Sampler2D"), true ) == 0 )
                        strSamplers.append( _T("Texture2D ") );
                    else if ( strSamplerType.compare( _T("Sampler3D"), true ) == 0 )
                        strSamplers.append( _T("Texture3D ") );
                    else if ( strSamplerType.compare( _T("SamplerCUBE"), true ) == 0 )
                        strSamplers.append( _T("TextureCUBE ") );
                    else
                    {
                        // ToDo: 9999 - Unrecognized sampler type '$strSamplerType'
                        continue;
                    } // End if unknown sampler

                    // ToDo: Support sampler2DMS, sampler2DArray, etc.?
                    // These will be replaced with just Sampler2D, but the texture
                    // inserted will be of the appropriate type.

                #else
                    strSamplers.append( _T("texture ") );
                #endif

                // Append texture name ({$strSamplerIdentifier}Tex)
                strSamplers.append( strSamplerIdentifier );
                strSamplers.append( _T("Tex") );

                // If the sampler was bound, bind the texture to the
                // associated texture register.
                if ( nSamplerRegister >= 0 )
                    strSamplers.append( cgString::format( _T(" : register(t%i)"), nSamplerRegister ) );

                // End texture line.
                strSamplers.append( _T(";\n") );
    
                // Append actual sampler.
                #if (COMPILERMODE==DX11 || COMPILERMODE==DX10)
                    strSamplers.append( _T("SamplerState ") );
                    strSamplers.append( strSamplerIdentifier );
                    if ( nSamplerRegister >= 0 )
                        strSamplers.append( cgString::format( _T(" : register(s%i)"), nSamplerRegister ) );
                    strSamplers.append( _T(";\n") );
                #else
                    strSamplers.append( strSampler );
                    strSamplers.append( _T(";\n") );
                #endif
                
            } // Next sampler
        
        } // End if samplers provided*/

        // Build the final shader code.
        cgString strShader = (!strCommon.empty()) ? strCommon + _T("\n") : cgString::Empty;
        if ( !strConstants.empty() )
        {
            strShader.append( strConstants );
            strShader.append( _T("\n") );
        } // End if valid constants
        strShader.append( strSamplers );
        if ( !mShaderGlobals->empty() )
        {
            strShader.append( *mShaderGlobals );
            strShader.append( _T("\n") );

        } // End if valid globals
        strShader.append( strSignature );
        strShader.append( _T("\n{\n") );
        strShader.append( *mShaderCode );
        strShader.append( _T("\n}") );

        #if defined(_DEBUG)
            // Dump original HLSL code
            STRING_CONVERT;
            static int nf = 0;
            nf++;
            cgChar lpszFName[256];
            const cgChar * lpszShaderName = stringConvertT2CA(strName.c_str());
            sprintf( lpszFName, "Build/ps_%s_%04i_HLSL.txt", lpszShaderName, nf );
            FILE * pf = fopen( lpszFName, "w" );
            if ( pf )
            {
                fwrite( stringConvertT2CA( strShader.c_str() ), strShader.size(), 1, pf );
                fclose( pf );
                cgAppLog::write( cgAppLog::Debug, _T("Emitting generated HLSL source code to %s\n"), stringConvertA2CT(lpszFName) );
            }
        #endif

        // Create and attempt to load the pixel shader.
        if ( !mManager->createPixelShader( &hShader, mIdentifier, strShader, strName, 0, cgDebugSource() ) )
        {
            // Add to the list of failed shaders.
            mFailedPixelShaders.insert( *mIdentifier );
            return cgPixelShaderHandle::Null;
        
        } // End if failed

        // Set the automatic management time-out to 60 seconds
        // to ensure it is unloaded if no longer used.
        // ToDo: Don't hardcode, or choose an alternate management method.
        cgPixelShader * pShader = hShader.getResource( false );
        pShader->setDestroyDelay( 60.0f );

        // Return the new handle.
        return hShader;

    } // End if no shader

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : clearGlobals () (Protected)
/// <summary>
/// Shader code generation methods place their data into several global script 
/// variables. Prior to calling these methods, the globals must be cleared.
/// </summary>
//-----------------------------------------------------------------------------
void cgSurfaceShader::clearGlobals( )
{
    if ( mShaderCode )
        mShaderCode->clear();
    if ( mShaderInputs )
        mShaderInputs->clear();
    if ( mShaderOutputs )
        mShaderOutputs->clear();
    if ( mShaderGlobals )
        mShaderGlobals->clear();
    if ( mConstantBufferRefs )
        mConstantBufferRefs->clear();
    if ( mSamplerBlockRefs )
        mSamplerBlockRefs->clear();
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantBufferDesc( cgInt32 nHandle, cgConstantBufferDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantBufferDesc( nHandle, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantBufferDesc( const cgString & strName, cgConstantBufferDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantBufferDesc( mShaderClass, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant buffer as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantBufferDesc( const cgString & strSourceNamespace, const cgString & strName, cgConstantBufferDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantBufferDesc( strSourceNamespace, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantBuffers ()
/// <summary>
/// Retrieve a list of all constant buffer description structures parsed during 
/// script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
const cgSurfaceShader::ConstBufferArray & cgSurfaceShader::getConstantBuffers( ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantBuffers();
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantTypeDesc( cgInt32 nHandle, cgConstantTypeDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantTypeDesc( nHandle, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantTypeDesc( const cgString & strName, cgConstantTypeDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantTypeDesc( mShaderClass, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypeDesc ()
/// <summary>
/// Retrieve description structure for the referenced constant type as it
/// was parsed during script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getConstantTypeDesc( const cgString & strSourceNamespace, const cgString & strName, cgConstantTypeDesc & Desc ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantTypeDesc( strSourceNamespace, strName, Desc );
}

//-----------------------------------------------------------------------------
//  Name : getConstantTypes ()
/// <summary>
/// Retrieve a list of all constant type description structures parsed during 
/// script pre-processing.
/// </summary>
//-----------------------------------------------------------------------------
const cgSurfaceShader::ConstTypeArray & cgSurfaceShader::getConstantTypes( ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantTypes();
}

//-----------------------------------------------------------------------------
//  Name : findSamplerRegister ()
/// <summary>
/// Find the register index for the sampler with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShader::findSamplerRegister( const cgString & strName ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->findSamplerRegister( mShaderClass, strName );
}

//-----------------------------------------------------------------------------
//  Name : findSamplerRegister ()
/// <summary>
/// Find the register index for the sampler with the specified name, in the 
/// current or global namespace.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgSurfaceShader::findSamplerRegister( const cgString & strSourceNamespace, const cgString & strName ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->findSamplerRegister( strSourceNamespace, strName );
}

//-----------------------------------------------------------------------------
//  Name : getTechnique ()
/// <summary>
/// Retrieve the handle of the surface shader class technique with the 
/// specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptFunctionHandle cgSurfaceShader::getTechnique( const cgString & strTechniqueName )
{
    // Validate requirements
    cgAssert( mScriptObject != CG_NULL );

    // Already available?
    TechniqueLUT::const_iterator itTechnique = mTechniqueLUT.find( strTechniqueName );
    if ( itTechnique == mTechniqueLUT.end() )
    {
        // Retrieve the handle of the specified technique
        static const cgString strReturnType = _T("TechniqueResult ");
        static const cgString strArgumentList = _T("(int,bool)");
        cgString strMethodDecl = strReturnType;
        strMethodDecl.append( strTechniqueName );
        strMethodDecl.append( strArgumentList );
        cgScriptFunctionHandle pTechnique = mScriptObject->getMethodHandle( strMethodDecl );

        // Store in the look up table.
        mTechniqueLUT[ strTechniqueName ] = pTechnique;
        return pTechnique;

    } // End if not found
    else
    {
        return itTechnique->second;

    } // End if found
}

//-----------------------------------------------------------------------------
//  Name : selectTechnique ()
/// <summary>
/// Select the technique method with the specified name as the current 
/// technique for execution in following calls to 'beginTechnique' and
/// 'executeTechniquePass'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectTechnique( const cgString & strTechniqueName )
{
    cgScriptFunctionHandle pHandle = getTechnique( strTechniqueName );
    if ( !pHandle )
        return false;

    // Complete any currently open technique execution.
    endTechnique();

    // Cache the handle.
    mCurrentTechnique = pHandle;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : selectTechnique ()
/// <summary>
/// Select the technique method with the specified handle as the current 
/// technique for execution in following calls to 'beginTechnique' and
/// 'executeTechniquePass'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::selectTechnique( cgScriptFunctionHandle pHandle )
{
    // Complete any currently open technique execution.
    endTechnique();

    // Cache the handle.
    mCurrentTechnique = pHandle;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTechnique ()
/// <summary>
/// Begin the execution of the technique previously selected with a call to
/// 'selectTechnique()' or a prior 'beginTechnique( Name/Handle )'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::beginTechnique( )
{
    // End any prior technique execution (just in case).
    endTechnique();

    // Reset auto-execution details.
    mCurrentPass = 0;
    mLastResult = cgTechniqueResult::Continue;

    // Is a valid technique selected?
    if ( !mCurrentTechnique )
        return false;

    // Execution has begin.
    mTechniqueInProgress = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTechnique ()
/// <summary>
/// Automatically select the technique with the specified name and begin its
/// execution.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::beginTechnique( const cgString & strName )
{
    // First select the technique with the specified name.
    if ( !selectTechnique( strName ) )
        return false;

    // Reset auto-execution details.
    mCurrentPass = 0;
    mLastResult = cgTechniqueResult::Continue;

    // Is a valid technique selected?
    if ( !mCurrentTechnique )
        return false;

    // Execution has begin.
    mTechniqueInProgress = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTechnique ()
/// <summary>
/// Automatically select the technique with the specified handle and begin its
/// execution.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::beginTechnique( cgScriptFunctionHandle pHandle )
{
    // First select the technique with the specified handle
    if ( !selectTechnique( pHandle ) )
        return false;

    // Reset auto-execution details.
    mCurrentPass = 0;
    mLastResult = cgTechniqueResult::Continue;

    // Is a valid technique selected?
    if ( !mCurrentTechnique )
        return false;

    // Execution has begin.
    mTechniqueInProgress = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : commitChanges ()
/// <summary>
/// Notify the surface shader that some parameters may have changed and it
/// should re-apply any appropriate states based on those variables for the 
/// current pass.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::commitChanges( )
{
    // Validate requirements
    cgAssert( mScriptObject != CG_NULL );
    if ( !mTechniqueInProgress )
        return false;

    // Re-execute the most recent pass with the 'commit changes' flag set.
    bool bSuccess;
    void * pResult = mScriptObject->executeMethod( mCurrentTechnique, mCommitArgs, &bSuccess );

    // Success?
    if ( bSuccess == true )
    {
        cgTechniqueResult::Base Result = *(cgTechniqueResult::Base*)pResult;
        return ( Result != cgTechniqueResult::Abort );

    } // End if success

    // Failed!
    return false;
}

//-----------------------------------------------------------------------------
//  Name : executeTechniquePass ()
/// <summary>
/// Execute the next pass of the script technique method. Automatically
/// increments the current pass counter on completion.
/// </summary>
//-----------------------------------------------------------------------------
cgTechniqueResult::Base cgSurfaceShader::executeTechniquePass( )
{
    // Validate requirements
    cgAssert( mScriptObject != CG_NULL );

    // Do nothing if all passes have been executed (or abort occurred)
    if ( mLastResult != cgTechniqueResult::Continue )
        return mLastResult;
    
    // Execute the next pass
    bool bSuccess;
    void * pResult = mScriptObject->executeMethod( mCurrentTechnique, mNoCommitArgs, &bSuccess );

    // Success?
    if ( bSuccess )
    {
        // Move on to the next pass
        ++mCurrentPass;

        // Store the result.
        mLastResult = *(cgTechniqueResult::Base*)pResult;

        // If the result was 'abort', bail immediately. If the result
        // is 'complete', signal 'continue' one last time to allow
        // the interior of the execution loop to complete before we exit.
        switch ( mLastResult )
        {
            case cgTechniqueResult::Abort:
                return cgTechniqueResult::Abort;
            case cgTechniqueResult::Complete:
                return cgTechniqueResult::Continue;
            default:
                return cgTechniqueResult::Continue;
        
        } // End switch LastResult
        
    } // End if success

    // Failed!
    mLastResult = cgTechniqueResult::Abort;
    return mLastResult;
}

//-----------------------------------------------------------------------------
//  Name : endTechnique ()
/// <summary>
/// Allow the shader to perform houskeeping tasks one shader technique 
/// execution is completed.
/// </summary>
//-----------------------------------------------------------------------------
void cgSurfaceShader::endTechnique( )
{
    // We have finished executing the technique.
    mTechniqueInProgress = false;
}

//-----------------------------------------------------------------------------
//  Name : setInt ()
/// <summary>
/// Set the value of the integer variable identified by the supplied 
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setInt( const cgString & strName, cgInt32 Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgInt32 * pVar = (cgInt32*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setFloat ()
/// <summary>
/// Set the value of the single precision floating point variable identified by
/// the supplied name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setFloat( const cgString & strName, cgFloat Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgFloat * pVar = (cgFloat*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setDouble ()
/// <summary>
/// Set the value of the double precision floating point variable identified by
/// the supplied name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setDouble( const cgString & strName, cgDouble Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgDouble * pVar = (cgDouble*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setBool ()
/// <summary>
/// Set the value of the boolean variable identified by the supplied 
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setBool( const cgString & strName, bool Value )
{
    cgAssert( mScriptObject != CG_NULL );
    bool * pVar = (bool*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setVector( const cgString & strName, const cgVector2 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector2 * pVar = (cgVector2*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setVector( const cgString & strName, const cgVector3 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector3 * pVar = (cgVector3*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setVector( const cgString & strName, const cgVector4 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector4 * pVar = (cgVector4*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setMatrix ()
/// <summary>
/// Set the value of the matrix variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setMatrix( const cgString & strName, const cgMatrix & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgMatrix * pVar = (cgMatrix*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    *pVar = Value;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setValue ()
/// <summary>
/// Set the value of the specified variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::setValue( const cgString & strName, void * pData, size_t nSize )
{
    cgAssert( mScriptObject != CG_NULL );
    void * pVar = (void*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    memcpy( pVar, pData, nSize );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getInt ()
/// <summary>
/// Get the value of the integer variable identified by the supplied 
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getInt( const cgString & strName, cgInt32 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgInt32 * pVar = (cgInt32*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getFloat ()
/// <summary>
/// Get the value of the single precision floating point variable identified by
/// the supplied name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getFloat( const cgString & strName, cgFloat & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgFloat * pVar = (cgFloat*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getDouble ()
/// <summary>
/// Get the value of the double precision floating point variable identified by
/// the supplied name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getDouble( const cgString & strName, cgDouble & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgDouble * pVar = (cgDouble*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getBool ()
/// <summary>
/// Get the value of the boolean variable identified by the supplied 
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getBool( const cgString & strName, bool & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    bool * pVar = (bool*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getVector ()
/// <summary>
/// Get the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getVector( const cgString & strName, cgVector2 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector2 * pVar = (cgVector2*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getVector ()
/// <summary>
/// Get the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getVector( const cgString & strName, cgVector3 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector3 * pVar = (cgVector3*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getVector ()
/// <summary>
/// Get the value of the vector variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getVector( const cgString & strName, cgVector4 & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgVector4 * pVar = (cgVector4*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMatrix ()
/// <summary>
/// Get the value of the matrix variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getMatrix( const cgString & strName, cgMatrix & Value )
{
    cgAssert( mScriptObject != CG_NULL );
    cgMatrix * pVar = (cgMatrix*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    Value = *pVar;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getValue ()
/// <summary>
/// Get the value of the specified variable identified by the supplied
/// name/identifier string.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSurfaceShader::getValue( const cgString & strName, void * pData, size_t nSize )
{
    cgAssert( mScriptObject != CG_NULL );
    void * pVar = (void*)mScriptObject->getAddressOfMember( strName );
    if ( !pVar )
        return false;
    memcpy( pData, pVar, nSize );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBufferLinker ()
/// <summary>
/// Retrieve the common / shared constant linker that provides hardare register 
/// mappings and shader code declarations for the constant buffers declared 
/// within the script.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferLinker * cgSurfaceShader::getConstantBufferLinker( ) const
{
    cgSurfaceShaderScript * pScript = (cgSurfaceShaderScript*)mScript.getResourceSilent();
    cgAssert( pScript != CG_NULL );
    return pScript->getConstantBufferLinker( );
}

//-----------------------------------------------------------------------------
//  Name : pushCodeOutput () (Static)
/// <summary>
/// Internal surface shader script function (bound to __sh_push_code_output())
/// that provides the system with the current output string into which code
/// is placed by the 'generateShaderCall()' and 'emitCode()' functions. This is
/// implicitly called by a shader call function at the start of the code
/// generation process in order to ensure that recursive shadercall function
/// calls are supported.
/// </summary>
//-----------------------------------------------------------------------------
void cgSurfaceShader::pushCodeOutput( cgString * pOutput )
{
    mShaderCodeOutputStack.push_back( pOutput );
}

//-----------------------------------------------------------------------------
//  Name : popCodeOutput () (Static)
/// <summary>
/// Internal surface shader script function (bound to __sh_pop_code_output())
/// that restores the current output string into which code is placed by 
/// the 'generateShaderCall()' function. This is implicitly called by a shader 
/// call function at the end of the code generation process in order to ensure 
/// that recursive shadercall function calls are supported.
/// </summary>
//-----------------------------------------------------------------------------
void cgSurfaceShader::popCodeOutput( )
{
    mShaderCodeOutputStack.pop_back();
}

//-----------------------------------------------------------------------------
//  Name : generateShaderCall () (Static)
/// <summary>
/// Generates a new (or re-uses an existing) HLSL function that can be called
/// by a surface shader generated vertex or pixel shader (or other utility
/// function). In addition to generating the function itself where necessary,
/// this function also returns a code string that represents the physical call 
/// to the HLSL function.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSurfaceShader::generateShaderCall( const cgString & strDeclaration, const cgString & strFunction, const cgString & strParams, const cgString & strCode )
{
    // ToDo: Track if the generated code is identical to a previously
    // output function to avoid bloating the shader output.

    // Generate final declaration, inserting the function instance number 
    // where appropriate; i.e. void myFunc_10( params )
    static cgInt nCurrentFunc = 0;
    size_t nParenPos = strDeclaration.find( _T("(") );
    cgString strFinalDeclaration = strDeclaration.substr( 0, nParenPos );
    strFinalDeclaration.append( cgString::format( _T("_%i"), nCurrentFunc ) );
    strFinalDeclaration.append( strDeclaration.substr( nParenPos ) );
    
    // Output function code.
    mShaderGlobalCodeOutput->append( strFinalDeclaration );
    mShaderGlobalCodeOutput->append( _T("\n{\n") );
    mShaderGlobalCodeOutput->append( strCode );
    mShaderGlobalCodeOutput->append( _T("\n}\n\n") );

    // Generate code for the actual function call.
    cgString strFunctionCall = cgString::format( _T("%s_%i"), strFunction.c_str(), nCurrentFunc );
    strFunctionCall.append( _T("(") );
    strFunctionCall.append( strParams );
    strFunctionCall.append( _T(")") );
    
    // Next function
    nCurrentFunc++;

    // Return the code that we want to emit to the actual shader for the CALL.
    return strFunctionCall;
}

//-----------------------------------------------------------------------------
//  Name : emitCode () (Static)
/// <summary>
/// Emit the specified code to the currently assigned output string.
/// </summary>
//-----------------------------------------------------------------------------
void cgSurfaceShader::emitCode( const cgString & strCode )
{
    cgAssert( mShaderCodeOutputStack.empty() == false );
    cgString & strOutput = *mShaderCodeOutputStack.back();
    strOutput.append( strCode );
}