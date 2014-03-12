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
// Name : cgScript.cpp                                                       //
//                                                                           //
// Desc : Contains classes that provide the application with the ability to  //
//        interact with individual loaded script modules.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScript Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgScript.h>
#include <Scripting/cgScriptEngine.h>
#include <Scripting/cgScriptPreprocessor.h>
#include <System/cgStringUtility.h>

// Angelscript.
#include <angelscript.h>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgScriptInterop::Exceptions;

///////////////////////////////////////////////////////////////////////////////
// cgScript Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScript () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScript::cgScript( cgUInt32 nReferenceId, const cgInputStream & Stream, const cgString & strThisType, cgScriptEngine * pEngine ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mInputStream   = Stream;
    mThisTypeName   = strThisType;
    mScriptEngine = pEngine;
    mFailedState  = false;
    
    // Cached responses
    mResourceType  = cgResourceType::Script;
    mCanEvict     = ( Stream.getType() == cgStreamType::File || Stream.getType() == cgStreamType::MappedFile );
}

//-----------------------------------------------------------------------------
//  Name : ~cgScript () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScript::~cgScript( )
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
void cgScript::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Clear variables
    mInputStream.reset();
    mScriptEngine = CG_NULL;

    // Dispose base.
    if ( bDisposeBase )
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
bool cgScript::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ScriptResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getScriptEngine ()
/// <summary>
/// Retrieve the script engine that owns this script.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptEngine * cgScript::getScriptEngine( )
{
    return mScriptEngine;
}

//-----------------------------------------------------------------------------
//  Name : getSourceInfo ()
/// <summary>
/// Retrieve information about the source files which were used to compile any
/// loaded script.
/// </summary>
//-----------------------------------------------------------------------------
const cgScript::SourceFileArray & cgScript::getSourceInfo() const
{
    return mSourceFiles;
}

//-----------------------------------------------------------------------------
//  Name : isFailed()
/// <summary>
/// Determine whether or not the script failed to compile.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::isFailed( ) const
{
    return mFailedState;
}

//-----------------------------------------------------------------------------
//  Name : defineMacro()
/// <summary>
/// Define a macro word / macro to be used by the script pre-processor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::defineMacro( const cgString & strWord, const cgString & strValue )
{
    STRING_CONVERT;
    std::string strANSIWord( stringConvertT2CA(cgString::trim(strWord).c_str()) );
    std::string strANSIValue( stringConvertT2CA(strValue.c_str()) );

    // Valid name / value?
    if ( cgScriptPreprocessor::isValidMacro( strANSIWord, strANSIValue ) == false )
        return false;
    
    // Already exists?
    if ( mDefinitions.find( strANSIWord ) != mDefinitions.end() )
        return false;

    // Add to definitions
    mDefinitions[strANSIWord] = strANSIValue;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isMacroDefined()
/// <summary>
/// Determine if the specified macro word / macro is defined for used by the 
/// script pre-processor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::isMacroDefined( const cgString & strWord ) const
{
    STRING_CONVERT;
    std::string strANSIWord( stringConvertT2CA(cgString::trim(strWord).c_str()) );
    return ( mDefinitions.find( strANSIWord ) != mDefinitions.end() );
}

//-----------------------------------------------------------------------------
//  Name : getInputStream()
/// <summary>
/// Retrieve the original input stream from which this script resource is 
/// being loaded.
/// </summary>
//-----------------------------------------------------------------------------
const cgInputStream & cgScript::getInputStream( ) const
{
    return mInputStream;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::loadResource( )
{
    STRING_CONVERT;  // For string conversion macro

    // Already loaded?
    if ( mResourceLoaded )
        return true;
    if ( mFailedState )
        return false;

    // First convert the module name we're going to use to a standard ANSI
    // string if the application is currently using unicode.
    const cgChar * strModuleName = stringConvertT2CA( getResourceName().c_str() );
    
    // Retrieve the angelscript core engine
    asIScriptEngine * pInternalEngine = mScriptEngine->getInternalEngine();

    // Construct the module into which code will be placed.
    asIScriptModule * pModule = pInternalEngine->GetModule( strModuleName, asGM_ALWAYS_CREATE );

    // Pre-process the script (automatically adds to script module). Also populate
    // list of source files used in the script. This information will be used to 
    // determine a relevant hashing key for any shaders generated through this script 
    // (if it is a surface shader).
    cgScriptPreprocessor Preprocessor( this );
    mFailedState = true;
    if ( Preprocessor.process( mInputStream, mDefinitions, mSourceFiles ) )
    {
	    // Attempt to build the script
        cgAppLog::write( cgAppLog::Debug, _T("Building script '%s'.\n"), getResourceName().c_str() );
        if ( pModule->Build() < 0 )
        {
            cgAppLog::write( cgAppLog::Error, _T("An error occured while attempting to build script '%s'.\n"), getResourceName().c_str() );
            
            // If we're in sandbox mode (preview or full), we should treat a missing
            // file or failed load as a success irrespective of the fact that we aren't 
            // able to load it. This ensures that the script's information is not lost.
            if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
            {
                mResourceLoaded = true;
                mResourceLost   = false;
                return true;
            
            } // End if sandbox
            
            // Failed!
            return false;
        
        } // End if failed to build

        // Bind any imports.
        pModule->BindAllImportedFunctions();

        // Script is now loaded
        mScriptEngine->mLoadedScripts.insert(this);
        mResourceLoaded = true;
        mResourceLost   = false;
        mFailedState    = false;

        // Success!
        return true;

    } // End if succeeded.

    // If we're in sandbox mode (preview or full), we should treat a missing
    // file or failed load as a success irrespective of the fact that we aren't 
    // able to load it. This ensures that the script's information is not lost.
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        mResourceLoaded = true;
        mResourceLost   = false;
        return true;
    
    } // End if sandbox

    // Failed!
    return false;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::unloadResource( )
{
    // Have we been initialized?
    if ( mScriptEngine && mResourceLoaded == true )
    {
        STRING_CONVERT;  // For string conversion macro

        // Unloaded from engine.
        mScriptEngine->mLoadedScripts.erase( this );

        // Clear out old function handles
        mFunctionCache.clear();

        // Clear out source file list and their hashes.
        mSourceFiles.clear();

        // First convert the module name we're going to use to a standard ANSI
        // string if the application is currently using unicode.
        const cgChar * strModuleName = stringConvertT2CA( getResourceName().c_str() );

        // Retrieve the angelscript core engine
        asIScriptEngine * pInternalEngine = mScriptEngine->getInternalEngine();

        // Remove the loaded module from the scripting engine
        pInternalEngine->DiscardModule( strModuleName );

    } // End if attached to script engine

    // Try to build again next time loadResource() is called.
    mFailedState = false;

    // We are no longer loaded
    mResourceLoaded = false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionBool ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns a boolean result.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::executeFunctionBool( const cgString & strFunctionName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgScriptArgument::Array ScriptArgs;
    cgString strFunctionDecl = _T("bool ") + strFunctionName;

    // Get argument list from struct
    Struct.toArgumentList( ScriptArgs );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, ScriptArgs, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return ( pContext->GetReturnDWord() > 0 );
    else
        return false;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionBool ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns a boolean result.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::executeFunctionBool( const cgString & strFunctionName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgString strFunctionDecl = _T("bool ") + strFunctionName;

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, Arguments, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return ( pContext->GetReturnDWord() > 0 );
    else
        return false;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionBool ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns a boolean result.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::executeFunctionBool( cgScriptFunctionHandle pFunctionHandle, const cgScriptArgument::Array & Arguments, bool * pbSuccess /* = CG_NULL */ )
{
    // Execute the script
    asIScriptContext * pContext = scriptExecute( pFunctionHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return ( pContext->GetReturnDWord() > 0 );
    else
        return false;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionInt()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns an integer result.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScript::executeFunctionInt( const cgString & strFunctionName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgScriptArgument::Array ScriptArgs;
    cgString strFunctionDecl = _T("int ") + strFunctionName;

    // Get argument list from struct
    Struct.toArgumentList( ScriptArgs );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, ScriptArgs, bOptional );

    // Failed to execute?
    if ( pbSuccess )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext )
        return (cgInt32)pContext->GetReturnDWord();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionInt ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns an integer result.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScript::executeFunctionInt( const cgString & strFunctionName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgString strFunctionDecl = _T("int ") + strFunctionName;

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, Arguments, bOptional );

    // Failed to execute?
    if ( pbSuccess )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext )
        return (cgInt32)pContext->GetReturnDWord();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionInt ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns an integer result.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScript::executeFunctionInt( cgScriptFunctionHandle pFunctionHandle, const cgScriptArgument::Array & Arguments, bool * pbSuccess /* = CG_NULL */ )
{
    // Execute the script
    asIScriptContext * pContext = scriptExecute( pFunctionHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext )
        return (cgInt32)pContext->GetReturnDWord();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionString ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns a string result.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScript::executeFunctionString( const cgString & strFunctionName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgScriptArgument::Array ScriptArgs;
    cgString strFunctionDecl = _T("String ") + strFunctionName;

    // Get argument list from struct
    Struct.toArgumentList( ScriptArgs );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, ScriptArgs, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return *((cgString*)pContext->GetReturnObject());
    else
        return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionString ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns a string result.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScript::executeFunctionString( const cgString & strFunctionName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgString strFunctionDecl = _T("String ") + strFunctionName;

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, Arguments, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return *((cgString*)pContext->GetReturnObject());
    else
        return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionObject ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns an object result.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::executeFunctionObject( const cgString & strObjectType, const cgString & strFunctionName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgScriptArgument::Array ScriptArgs;
    cgString strFunctionDecl = strObjectType + _T("@ ") + strFunctionName;

    // Get argument list from struct
    Struct.toArgumentList( ScriptArgs );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, ScriptArgs, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return new cgScriptObject( this, pContext->GetReturnObject() );
    else
        return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionObject ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version returns an object result.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::executeFunctionObject( const cgString & strObjectType, const cgString & strFunctionName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgString strFunctionDecl = strObjectType + _T("@ ") + strFunctionName;

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, Arguments, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
    
    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return new cgScriptObject( this, pContext->GetReturnObject() );
    else
        return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionVoid ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version  returns no result.
/// </summary>
//-----------------------------------------------------------------------------
void cgScript::executeFunctionVoid( const cgString & strFunctionName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgScriptArgument::Array ScriptArgs;
    cgString strFunctionDecl = _T("void ") + strFunctionName;

    // Get argument list from struct
    Struct.toArgumentList( ScriptArgs );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, ScriptArgs, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : executeFunctionVoid ()
/// <summary>
/// Called in order to execute the specified function passing the
/// required list of arguments. This version  returns no result.
/// </summary>
//-----------------------------------------------------------------------------
void cgScript::executeFunctionVoid( const cgString & strFunctionName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    cgString strFunctionDecl = _T("void ") + strFunctionName;

    // Execute the script
    asIScriptContext * pContext = scriptExecute( strFunctionDecl, Arguments, bOptional );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : getFunctionHandle ()
/// <summary>
/// Retrieve the handle associated with a given function declaration.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptFunctionHandle cgScript::getFunctionHandle( const cgString & strFunctionDecl )
{
    // Have we precached the handle / existence of this function?
    cgScriptFunctionHandle pFuncHandle = CG_NULL;
    FunctionMap::iterator itHandle = mFunctionCache.find( strFunctionDecl );
    if ( itHandle != mFunctionCache.end() )
    {
        // We have previously cached the result of this function handle lookup.    
        pFuncHandle = itHandle->second;
    
    } // End if found
    else
    {
        STRING_CONVERT;  // For string conversion macro

        // Lookup the function handle.
        asIScriptEngine  * pInternalEngine  = mScriptEngine->getInternalEngine();

        // First convert the module name we're going to use to a standard ANSI
        // string if the application is currently using unicode.
        const cgChar * strModuleName = stringConvertT2CA( getResourceName().c_str() );

        // Get the module for this script.
        asIScriptModule * pModule = pInternalEngine->GetModule( strModuleName );

        // Find the function id for the function we want to execute.
        if ( pModule )
            pFuncHandle = cgScriptFunctionHandle(pModule->GetFunctionByDecl( stringConvertT2CA( strFunctionDecl.c_str() ) ));
        
        // Cache the result of this relative expensive operation.
        mFunctionCache[ strFunctionDecl ] = pFuncHandle;
    
    } // End if not found

    // Return the final function handle
    return pFuncHandle;
}

//-----------------------------------------------------------------------------
//  Name : scriptExecute ()
/// <summary>
/// Actually performs the setting up of arguments, and executing of the
/// script function.
/// Note : May throw a 'cgScriptInterop::Exceptions::ExecuteException' struct.
/// </summary>
//-----------------------------------------------------------------------------
asIScriptContext * cgScript::scriptExecute( const cgString & strFunctionDecl, const cgScriptArgument::Array & Arguments, bool bOptional )
{
    // Build declarator
    cgString strDeclaration = strFunctionDecl + _T("(");
    cgScriptArgument::Array::const_iterator itArguments;
    for ( itArguments = Arguments.begin(); itArguments != Arguments.end(); ++itArguments )
    {
        const cgScriptArgument & Argument = *itArguments;

        // Append type to declarator
        if ( itArguments != Arguments.begin() ) strDeclaration += _T(",");
        strDeclaration += Argument.declaration;

    } // Next Argument

    // Finish building declarator
    strDeclaration += _T(")");

    // Retrieve the function handle
    cgScriptFunctionHandle pFuncHandle = getFunctionHandle( strDeclaration );
    
    // A NULL handle indicates that the function was not found.
    if ( !pFuncHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strDeclaration.c_str() );
            throw ExecuteException( ("Failed to resolve script function with declaration '" + strDeclarationANSI + "'.").c_str(), getResourceName(), 0 );
        
        } // End if not optional
        return CG_NULL;

    } // End if invalid function

    // Pass through to the standard function
    return scriptExecute( pFuncHandle, Arguments );
}

//-----------------------------------------------------------------------------
//  Name : scriptExecute ()
/// <summary>
/// Actually performs the setting up of arguments, and executing of the
/// script function.
/// Note : May throw a 'cgScriptInterop::Exceptions::ExecuteException' struct.
/// </summary>
//-----------------------------------------------------------------------------
asIScriptContext * cgScript::scriptExecute( cgScriptFunctionHandle pFunctionHandle, const cgScriptArgument::Array & Arguments )
{
    cgAssert( pFunctionHandle != CG_NULL );

    // Get an idle context for execution
    asIScriptContext * pInternalContext = mScriptEngine->getIdleExecuteContext();

    // Prepare for execution
    pInternalContext->SetUserData( this );
    pInternalContext->Prepare( (asIScriptFunction*)pFunctionHandle );

    // Build all arguments that are required
    int nArgIndex = 0;
    cgScriptArgument::Array::const_iterator itArguments;
    for ( itArguments = Arguments.begin(); itArguments != Arguments.end(); ++itArguments )
    {
        const cgScriptArgument & Argument = *itArguments;

        // Set to internal engine
        switch ( Argument.type )
        {
            case cgScriptArgumentType::Object:
                pInternalContext->SetArgObject( nArgIndex++, (void*)Argument.data );
                break;

            case cgScriptArgumentType::Array:            
            case cgScriptArgumentType::Address:
                pInternalContext->SetArgAddress( nArgIndex++, (void*)Argument.data );
                break;

            case cgScriptArgumentType::Byte:
            case cgScriptArgumentType::Bool:
                pInternalContext->SetArgByte( nArgIndex++, *(asBYTE*)Argument.data );
                break;

            case cgScriptArgumentType::Word:
                pInternalContext->SetArgWord( nArgIndex++, *(asWORD*)Argument.data );
                break;
            
            case cgScriptArgumentType::DWord:
                pInternalContext->SetArgDWord( nArgIndex++, *(asDWORD*)Argument.data );
                break;

            case cgScriptArgumentType::Float:
                pInternalContext->SetArgFloat( nArgIndex++, *(cgFloat*)Argument.data );
                break;

            case cgScriptArgumentType::Double:
                pInternalContext->SetArgDouble( nArgIndex++, *(cgDouble*)Argument.data );
                break;

            case cgScriptArgumentType::QWord:
                pInternalContext->SetArgQWord( nArgIndex++, *(asQWORD*)Argument.data );
                break;

        } // End switch argument type

    } // Next Argument
    
    // Execute the function
    int nResult = pInternalContext->Execute();
    
    // An error occured?
    if ( nResult == asEXECUTION_EXCEPTION )
    {
        asIScriptFunction * pFunction = pInternalContext->GetExceptionFunction();
        if ( !pFunction )
            throw ExecuteException( pInternalContext->GetExceptionString(), getResourceName(), pInternalContext->GetExceptionLineNumber() );
        else
        {
            STRING_CONVERT;
            throw ExecuteException( pInternalContext->GetExceptionString(), stringConvertA2CT(pFunction->GetScriptSectionName()), pInternalContext->GetExceptionLineNumber() );
        
        } // End if has function
    
    } // End if exception
    else if ( nResult == asERROR )
        throw ExecuteException( "The script failed to execute due to previous errors. Check function call declaration.", getResourceName(), 0 );

    // Return the context we used.
    return pInternalContext;
}

//-----------------------------------------------------------------------------
//  Name : createObjectInstance ()
/// <summary>
/// Create an instance of the specific script or application registered class
/// or data type.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::createObjectInstance( const cgString & strObjectType )
{
    return createObjectInstance( strObjectType, false, cgScriptArgument::Array() );
}

//-----------------------------------------------------------------------------
//  Name : createObjectInstance ()
/// <summary>
/// Create an instance of the specific script or application registered class
/// or data type.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::createObjectInstance( const cgString & strObjectType, bool uninitialized )
{
    return createObjectInstance( strObjectType, uninitialized, cgScriptArgument::Array() );
}

//-----------------------------------------------------------------------------
//  Name : createObjectInstance ()
/// <summary>
/// Create an instance of the specific script or application registered class
/// or data type.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::createObjectInstance( const cgString & strObjectType, const cgScriptArgument::Array & aArgs )
{
    return createObjectInstance( strObjectType, false, aArgs );
}

//-----------------------------------------------------------------------------
//  Name : createObjectInstance ()
/// <summary>
/// Create an instance of the specific script or application registered class
/// or data type.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgScript::createObjectInstance( const cgString & strObjectType, bool uninitialized, const cgScriptArgument::Array & aArgs )
{
    STRING_CONVERT;
    
    // Catch exceptions
    if ( !mResourceLoaded )
    {
        cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Unable to instantiate object of type '%s' via script resource '%s' because the script is not currently resident.\n"), strObjectType.c_str(), getResourceName().c_str() );
        return false;
    
    } // End if !loaded

    // First convert the string values we're going to use to standard ANSI
    // if the application is currently using unicode.
    const cgChar * lpszModuleName = stringConvertT2CA( getResourceName().c_str() );
    const cgChar * lpszObjectType = stringConvertT2CA( strObjectType.c_str() );

    // Retrieve the angelscript core engine
    asIScriptEngine * pInternalEngine = mScriptEngine->getInternalEngine();

    // Retrieve the module in which the object type is declared.
    asIScriptModule * pModule = pInternalEngine->GetModule( lpszModuleName, asGM_CREATE_IF_NOT_EXISTS );

    // Get the object type
    int nTypeId = pModule->GetTypeIdByDecl(lpszObjectType);
    asIObjectType * pType = pInternalEngine->GetObjectTypeById( nTypeId );
    if ( !pType )
    {
        cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Unable to instantiate object of type '%s' via script resource '%s' because no matching type has been defined.\n"), strObjectType.c_str(), getResourceName().c_str() );
        return false;
    
    } // End if no type

    // Create a new object of the required type.
    cgScriptObject * pNewObject = CG_NULL;
    if ( !uninitialized )
    {
        // Build declarator
        cgString strFactoryDecl = strObjectType + _T(" @") + strObjectType + _T("(");
        cgScriptArgument::Array::const_iterator itArguments;
        for ( itArguments = aArgs.begin(); itArguments != aArgs.end(); ++itArguments )
        {
            const cgScriptArgument & Argument = *itArguments;

            // Append type to declarator
            if ( itArguments != aArgs.begin() ) strFactoryDecl += _T(",");
            strFactoryDecl += Argument.declaration;

        } // Next Argument

        // Finish building declarator
        strFactoryDecl += _T(")");

        // Get the factory function id from the declarator
        asIScriptFunction * pFactory = pType->GetFactoryByDecl(stringConvertT2CA(strFactoryDecl.c_str()));

        // Prepare the context to call the factory function
        asIScriptContext * pContext = pInternalEngine->CreateContext();
        pContext->Prepare( pFactory );

        // Build all arguments that are required
        int nArgIndex = 0;
        for ( itArguments = aArgs.begin(); itArguments != aArgs.end(); ++itArguments )
        {
            const cgScriptArgument & Argument = *itArguments;

            // Set to internal engine
            switch ( Argument.type )
            {
                case cgScriptArgumentType::Object:
                    pContext->SetArgObject( nArgIndex++, (void*)Argument.data );
                    break;

                case cgScriptArgumentType::Array:            
                case cgScriptArgumentType::Address:
                    pContext->SetArgAddress( nArgIndex++, (void*)Argument.data );
                    break;

                case cgScriptArgumentType::Byte:
                case cgScriptArgumentType::Bool:
                    pContext->SetArgByte( nArgIndex++, *(asBYTE*)Argument.data );
                    break;

                case cgScriptArgumentType::Word:
                    pContext->SetArgWord( nArgIndex++, *(asWORD*)Argument.data );
                    break;
                
                case cgScriptArgumentType::DWord:
                    pContext->SetArgDWord( nArgIndex++, *(asDWORD*)Argument.data );
                    break;

                case cgScriptArgumentType::Float:
                    pContext->SetArgFloat( nArgIndex++, *(cgFloat*)Argument.data );
                    break;

                case cgScriptArgumentType::Double:
                    pContext->SetArgDouble( nArgIndex++, *(cgDouble*)Argument.data );
                    break;

                case cgScriptArgumentType::QWord:
                    pContext->SetArgQWord( nArgIndex++, *(asQWORD*)Argument.data );
                    break;

            } // End switch argument type

        } // Next Argument

        // Execute the call
        if ( pContext->Execute() != asSUCCESS )
        {
            pContext->Release();
            cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Unable to execute factory function for object of type '%s' via script resource '%s'. An error occurred.\n"), strObjectType.c_str(), getResourceName().c_str() );
            return false;
        
        } // End if failed

        // Create a wrapper around the object that was created.
        pNewObject = new cgScriptObject( this, *(asIScriptObject**)pContext->GetAddressOfReturnValue() );

        // Clean up
        pContext->Release();
    
    } // End if initialized
    else
    {
        // Create uninitialized object.
        void * pObject = pInternalEngine->CreateUninitializedScriptObject( pType );

        // Create a wrapper around the object that was created.
        pNewObject = new cgScriptObject( this, (asIScriptObject*)pObject );

    } // End if uninitialized
    
    // Return new object.
    return pNewObject;
}

//-----------------------------------------------------------------------------
//  Name : setThisObject ()
/// <summary>
/// If requested, scripts can have access to a "this" global of the 
/// specified type. This method can be called to set the 'this' global
/// handle variable.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScript::setThisObject( void * pThis )
{
    bool bSucceeded;
    cgScriptArgument::Array Args;
    Args.push_back( cgScriptArgument( cgScriptArgumentType::Object, mThisTypeName + _T("@+"), pThis ) );
    executeFunctionVoid( _T("__GlobalSetThis"), Args, true, &bSucceeded );
    return bSucceeded;
}

//-----------------------------------------------------------------------------
//  Name : getThisType ()
/// <summary>
/// Retrieve the type name for the global 'this' object assigned to the script.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgScript::getThisType( ) const
{
    return mThisTypeName;
}

///////////////////////////////////////////////////////////////////////////////
// cgScriptObject Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScriptObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject::cgScriptObject( cgScript * pScript, void * pObject )
{
    // Initialize variables to sensible defaults
    mScript   = pScript;
    mObject   = (asIScriptObject*)pObject;
    mRefCount = 1;

    // Increment wrapped object reference count as necessary.
    if ( mObject != NULL )
        mObject->AddRef();
}

//-----------------------------------------------------------------------------
//  Name : ~cgScriptObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject::~cgScriptObject()
{
    // Release our reference to the script object (if valid).
    if ( mObject != CG_NULL )
        mObject->Release();
    
    // Clear variables
    mObject = CG_NULL;
    mScript = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : addRef ()
/// <summary>
/// Add a reference to this object's internal count. Items will only be
/// physically be destroyed when all references are released.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptObject::addRef( )
{
    // Increase the internal reference count
    mRefCount++;
}

//-----------------------------------------------------------------------------
//  Name : release ()
/// <summary>
/// Release a reference to this object. Items will only be physically be 
/// destroyed when all references are released.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgScriptObject::release( )
{
    // Decrease reference count
    mRefCount--;

    // Destroy when we hit 0.
    if ( mRefCount == 0 )
    {
        delete this;
        return 0;
    
    } // End if 0

    // Return current ref count.
    return mRefCount;
}

//-----------------------------------------------------------------------------
//  Name : executeMethod ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns the address of any
/// applicable return value (if any).
/// </summary>
//-----------------------------------------------------------------------------
void * cgScriptObject::executeMethod( cgScriptFunctionHandle pMethodHandle, const cgScriptCompatibleStruct & Struct, bool * pbSuccess /* = CG_NULL */ )
{
    // ToDo: 6767 - Is this needed?
    /*// Find the method.
    MethodDeclMap::const_iterator itDecl = mMethodDeclCache.find( pMethodHandle );
    if ( itDecl == mMethodDeclCache.end() )
    {
        // Failed to execute?
        if ( pbSuccess )
            *pbSuccess = false;
        return CG_NULL;
    
    } // End if no matching method*/

    // Get argument list from struct
    cgScriptArgument::Array Arguments;
    Struct.toArgumentList( Arguments );

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext )
        return pContext->GetAddressOfReturnValue();
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : executeMethod ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns the address of any
/// applicable return value (if any).
/// </summary>
//-----------------------------------------------------------------------------
void * cgScriptObject::executeMethod( cgScriptFunctionHandle pMethodHandle, const cgScriptArgument::Array & Arguments, bool * pbSuccess /* = CG_NULL */ )
{
    // ToDo: 6767 - Is this needed
    /*// Find the method.
    MethodDeclMap::const_iterator itDecl = mMethodDeclCache.find( nMethodHandle );
    if ( itDecl == mMethodDeclCache.end() )
    {
        // Failed to execute?
        if ( pbSuccess )
            *pbSuccess = false;
        return CG_NULL;
    
    } // End if no matching method*/

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext )
        return pContext->GetAddressOfReturnValue();
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodVoid ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns no result.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptObject::executeMethodVoid( const cgString & strMethodName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Get argument list from struct
    cgScriptArgument::Array Arguments;
    Struct.toArgumentList( Arguments );

    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("void");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : executeMethodVoid ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns no result.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptObject::executeMethodVoid( const cgString & strMethodName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("void");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );
 }

//-----------------------------------------------------------------------------
//  Name : executeMethodBool ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns a boolean result.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptObject::executeMethodBool( const cgString & strMethodName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Get argument list from struct
    cgScriptArgument::Array Arguments;
    Struct.toArgumentList( Arguments );

    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("bool");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return false;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return ( pContext->GetReturnDWord() > 0 );
    else
        return false;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodBool ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns a boolean result.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptObject::executeMethodBool( const cgString & strMethodName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("bool");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return false;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return ( pContext->GetReturnDWord() > 0 );
    else
        return false;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodInt ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns an integer result.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScriptObject::executeMethodInt( const cgString & strMethodName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Get argument list from struct
    cgScriptArgument::Array Arguments;
    Struct.toArgumentList( Arguments );

    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("int");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return 0;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return (cgInt32)pContext->GetReturnDWord();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodInt ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns a integer result.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScriptObject::executeMethodInt( const cgString & strMethodName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("int");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return 0;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return (cgInt32)pContext->GetReturnDWord();
    else
        return 0;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodString ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns a string result.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScriptObject::executeMethodString( const cgString & strMethodName, const cgScriptCompatibleStruct & Struct, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Get argument list from struct
    cgScriptArgument::Array Arguments;
    Struct.toArgumentList( Arguments );

    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("String");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return cgString::Empty;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return *((cgString*)pContext->GetReturnObject());
    else
        return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : executeMethodString ()
/// <summary>
/// Called in order to execute the specified object method passing the
/// required list of arguments. This version returns a string result.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScriptObject::executeMethodString( const cgString & strMethodName, const cgScriptArgument::Array & Arguments, bool bOptional /* = false */, bool * pbSuccess /* = CG_NULL */ )
{
    // Find the method.
    cgString strMethodDecl;
    static const cgString strReturnType = _T("String");
    buildMethodDeclaration( strReturnType, strMethodName, Arguments, strMethodDecl );
    cgScriptFunctionHandle pMethodHandle = getMethodHandle( strMethodDecl );

    // A NULL handle indicates that the function was not found.
    if ( !pMethodHandle )
    {
        // Exception should be thrown when not available?
        if ( !bOptional )
        {
            STRING_CONVERT;  // For string conversion macro
            std::string strDeclarationANSI = stringConvertT2CA( strMethodDecl.c_str() );
            throw ExecuteException( ("Failed to resolve script method with declaration '" + strDeclarationANSI + "'.").c_str(), mScript->getResourceName(), 0 );
        
        } // End if not optional
        return cgString::Empty;

    } // End if invalid function

    // Execute the script
    asIScriptContext * pContext = scriptExecute( pMethodHandle, Arguments );

    // Failed to execute?
    if ( pbSuccess != CG_NULL )
        *pbSuccess = ( pContext != CG_NULL );

    // Query the internal context so we can get the return result
    if ( pContext != CG_NULL )
        return *((cgString*)pContext->GetReturnObject());
    else
        return cgString::Empty;
}

//-----------------------------------------------------------------------------
//  Name : buildMethodDeclaration () (Protected)
/// <summary>
/// Construct a compatible method declaration based on the method details and 
/// arguments supplied.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptObject::buildMethodDeclaration( const cgString & strReturnType, const cgString & strMethodName, const cgScriptArgument::Array & Arguments, cgString & strDeclaration )
{
    // Start of method signature
    strDeclaration.clear();
    strDeclaration.append( strReturnType );
    strDeclaration.append( _T(" ") );
    strDeclaration.append( strMethodName );
    strDeclaration.append( _T("(") );
    
    // Arguments
    cgScriptArgument::Array::const_iterator itArguments;
    for ( itArguments = Arguments.begin(); itArguments != Arguments.end(); ++itArguments )
    {
        const cgScriptArgument & Argument = *itArguments;

        // Append type to declarator
        if ( itArguments != Arguments.begin() ) 
            strDeclaration.append( _T(",") );
        strDeclaration.append( Argument.declaration );

    } // Next Argument

    // End of method signature
    strDeclaration.append( _T(")") );
}

//-----------------------------------------------------------------------------
//  Name : scriptExecute () (Protected)
/// <summary>
/// Performs the setting up of arguments, and actual execution of the script 
/// method.
/// Note : May throw a 'cgScriptInterop::Exceptions::ExecuteException' struct.
/// </summary>
//-----------------------------------------------------------------------------
asIScriptContext * cgScriptObject::scriptExecute( cgScriptFunctionHandle pMethodHandle, const cgScriptArgument::Array & Arguments )
{
    // Get an idle context for execution
    cgScriptEngine   * pEngine = mScript->getScriptEngine();
    asIScriptContext * pInternalContext = pEngine->getIdleExecuteContext();

    // Prepare for execution
    pInternalContext->SetUserData( mScript );
    pInternalContext->Prepare( (asIScriptFunction*)pMethodHandle );
    pInternalContext->SetObject( mObject );

    // Build all arguments that are required
    cgInt nNumArgs = (cgInt)Arguments.size();
    for ( cgInt i = 0; i < nNumArgs; ++i )
    {
        const cgScriptArgument & Argument = Arguments[i];

        // Set to internal engine
        switch ( Argument.type )
        {
            case cgScriptArgumentType::Object:
                pInternalContext->SetArgObject( i, (void*)Argument.data );
                break;

            case cgScriptArgumentType::Array:
            case cgScriptArgumentType::Address:
                pInternalContext->SetArgAddress( i, (void*)Argument.data );
                break;

            case cgScriptArgumentType::Byte:
            case cgScriptArgumentType::Bool:
                pInternalContext->SetArgByte( i, *(asBYTE*)Argument.data );
                break;

            case cgScriptArgumentType::Word:
                pInternalContext->SetArgWord( i, *(asWORD*)Argument.data );
                break;
            
            case cgScriptArgumentType::DWord:
                pInternalContext->SetArgDWord( i, *(asDWORD*)Argument.data );
                break;

            case cgScriptArgumentType::Float:
                pInternalContext->SetArgFloat( i, *(cgFloat*)Argument.data );
                break;

            case cgScriptArgumentType::Double:
                pInternalContext->SetArgDouble( i, *(cgDouble*)Argument.data );
                break;

            case cgScriptArgumentType::QWord:
                pInternalContext->SetArgQWord( i, *(asQWORD*)Argument.data );
                break;

        } // End switch argument type

    } // Next Argument
    
    // Execute the function
    cgInt nResult = pInternalContext->Execute();
    
    // An error occured?
    if ( nResult == asEXECUTION_EXCEPTION )
    {
        asIScriptFunction * pFunction = pInternalContext->GetExceptionFunction();
        if ( !pFunction )
            throw ExecuteException( pInternalContext->GetExceptionString(), mScript->getResourceName(), pInternalContext->GetExceptionLineNumber() );
        else
        {
            STRING_CONVERT;
            const cgChar * sectionName = pFunction->GetScriptSectionName();
            throw ExecuteException( pInternalContext->GetExceptionString(), (sectionName) ? stringConvertA2CT(sectionName) : _T("[System]"), pInternalContext->GetExceptionLineNumber() );
        
        } // End if has function
    
    } // End if exception
    else if ( nResult == asERROR )
        throw ExecuteException( "The script method failed to execute due to previous errors. Check method call declaration.", mScript->getResourceName(), 0 );

    // Return the context we used.
    return pInternalContext;
}

//-----------------------------------------------------------------------------
//  Name : getMethodHandle ()
/// <summary>
/// Retrieve the unique integer identifier / handle for the specified method
/// declaration.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptFunctionHandle cgScriptObject::getMethodHandle( const cgString & strDeclaration )
{
    // Have we precached the handle / existance of this function?
    cgScriptFunctionHandle pMethodHandle = NULL;
    MethodMap::iterator itMethod = mMethodCache.find( strDeclaration );
    if ( itMethod != mMethodCache.end() )
    {
        // We have previously cached the result of this method lookup.    
        pMethodHandle = itMethod->second;
    
    } // End if found
    else
    {
        STRING_CONVERT;  // For string conversion macro

        // Get the type descriptor for this object.
        asIObjectType * pType = mObject->GetObjectType();

        // Find the method id for the function we want to execute.
        if ( pType )
            pMethodHandle = cgScriptFunctionHandle(pType->GetMethodByDecl( stringConvertT2CA( strDeclaration.c_str() ) ));
        
        // Cache the result of this relative expensive operation.
        mMethodCache[ strDeclaration ] = pMethodHandle;
        mMethodDeclCache[ pMethodHandle ] = strDeclaration;
    
    } // End if not found

    // Return handle (NULL indicates failure).
    return pMethodHandle;
}

//-----------------------------------------------------------------------------
//  Name : getTypeName ()
/// <summary>
/// Retrieve the script-side type / class name of the object.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScriptObject::getTypeName( ) const
{
    STRING_CONVERT;

    // Validate requirements
    if ( !mObject )
        return cgString::Empty;

    // Get the type descriptor for this object.
    const asIObjectType * pType = mObject->GetObjectType();
    return stringConvertA2CT( pType->GetName() );
}

//-----------------------------------------------------------------------------
//  Name : getInternalObject ()
/// <summary>
/// Retrieve the internal angelscript object being wrapped.
/// </summary>
//-----------------------------------------------------------------------------
asIScriptObject * cgScriptObject::getInternalObject( )
{
    return mObject;
}

//-----------------------------------------------------------------------------
//  Name : getAddressOfMember ()
/// <summary>
/// Retrieve the address of the specified object member variable.
/// </summary>
//-----------------------------------------------------------------------------
void * cgScriptObject::getAddressOfMember( const cgString & strMemberName )
{
    // ToDo: Consider caching the property index associated with this name.
    STRING_CONVERT;
    std::string strANSIName = stringConvertT2CA(strMemberName.c_str());

    // Find the index of the member by name.
    cgInt nPropertyCount = mObject->GetPropertyCount();
    for ( cgInt i = 0; i < nPropertyCount; ++i )
    {
        if ( strANSIName == mObject->GetPropertyName( i ) )
            return mObject->GetAddressOfProperty( i );

    } // Next Property

    // Not found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : setUserData ()
/// <summary>
/// Set custom data to the script object. Several different pieces of user
/// data can be specified by supplying a unique user data id.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptObject::setUserData( cgUInt32 id, void * value )
{
    if ( mObject )
        mObject->SetUserData( value, id );
}

//-----------------------------------------------------------------------------
//  Name : getUserData ()
/// <summary>
/// Retreive the script object custom data value associated with the unique 
/// user data id supplied .
/// </summary>
//-----------------------------------------------------------------------------
void * cgScriptObject::getUserData( cgUInt32 id )
{
    if ( mObject )
        return mObject->GetUserData( id );
    return CG_NULL;
}