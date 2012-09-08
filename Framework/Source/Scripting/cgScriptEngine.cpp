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
// Name : cgScriptEngine.cpp                                                 //
//                                                                           //
// Desc : Contains classes which provides the core of the scripting support  //
//        in the engine.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScriptEngine Module Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptEngine.h>
#include <Scripting/cgScriptLibrary.h>
#include <Scripting/cgScriptInterop.h>
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgScript.h>
#include <System/cgStringUtility.h>

// Angelscript
#include <angelscript.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgScriptEngine * cgScriptEngine::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgScriptEngine Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScriptEngine () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptEngine::cgScriptEngine()
{
    // Initialize variables to sensible defaults
    mEngine = CG_NULL;
    mVerboseOutput = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgScriptEngine () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptEngine::~cgScriptEngine()
{
    // Unload all active scripts
    unloadScripts();

    // Destroy all linked custom (scripted) libraries
    ScriptedLibraryMap::iterator itScript;
    for ( itScript = mScriptedLibraries.begin(); itScript != mScriptedLibraries.end(); ++itScript )
        (itScript->second).close( true );
    mScriptedLibraries.clear();

    // Destroy all linked C++ libraries
    LinkedLibraryMap::iterator itLibrary;
    for ( itLibrary = mLinkedLibraries.begin(); itLibrary != mLinkedLibraries.end(); ++itLibrary )
    {
        cgScriptLibrary * pLib = itLibrary->second;
        if ( pLib != CG_NULL )
            delete pLib;

    } // Next Library
    mLinkedLibraries.clear();

    // Release the angelscript engine
    if ( mEngine != CG_NULL )
        mEngine->Release();
    mEngine = CG_NULL;

    // Destroy all linked C++ scripting packages.
    unbindPackage( &mRootPackage );
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptEngine * cgScriptEngine::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgScriptEngine;
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::destroySingleton( )
{
    // Destroy!
    if ( mSingleton != CG_NULL )
        delete mSingleton;
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : messageCallback () (Private, Static)
/// <summary>
/// Called by angelscript whenever an error occured during compilation.
/// This will contain information about the type of error that occured
/// in addition to the location etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::messageCallback(const asSMessageInfo *msg, void *param )
{
    // Output enabled?
    if ( !((cgScriptEngine*)param)->mVerboseOutput )
        return;
    
    // Build output message.
    STRING_CONVERT; // For string conversion
    const cgTChar * strMessageType = _T("Error  ");
    cgUInt32        nType          = cgAppLog::Debug | cgAppLog::Error;

    // Determine the message type
    if ( msg->type == asMSGTYPE_WARNING ) 
    {
        strMessageType = _T("Warning");
        nType          = cgAppLog::Debug | cgAppLog::Warning;
    }
    else if( msg->type == asMSGTYPE_INFORMATION ) 
    {
        strMessageType = _T("Info   ");
        nType          = cgAppLog::Debug;
    }

    // Write to the registered output streams
    cgAppLog::write( nType, _T("%s(%d,%d) : %s : %s.\n"), stringConvertA2CT(msg->section), msg->row, msg->col, strMessageType, stringConvertA2CT(msg->message) );
}

//-----------------------------------------------------------------------------
//  Name : enableVerboseOutput ()
/// <summary>
/// Output compiler failures to log?
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::enableVerboseOutput( bool bEnable )
{
    mVerboseOutput = bEnable;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the scripting engine.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::initialize( )
{
    // Already initialized?
    if ( mEngine != CG_NULL )
        return false;

    // Create the internal scripting engine
    mEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    if ( mEngine == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Error, _T("Fatal error while attempting to instantiate core scripting engine object.\n") );
        return false;
    
    } // End if failed to allocate

    // Set the compilation error message callback.
    mEngine->SetMessageCallback( asFUNCTION(cgScriptEngine::messageCallback), this, asCALL_CDECL );

    // Allow unsafe references (disables the need for in/out/inout keywords)
    mEngine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true );

    // Require that enums are used exclusively with the correct scope (i.e. ResourceType::Texture)
    mEngine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, true );

    // Register the inbuilt array template type.
    cgScriptInterop::Types::ScriptArray::Register( mEngine );

    // Mark the above array type as the default implementation.
     mEngine->RegisterDefaultArrayType( "array<T>" );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : declarePackage ()
/// <summary>
/// Add the specified scripting package (collection of type bindings) to
/// the scripting engine and allow it to declare its types in advance, optionally 
/// including its child packages.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::declarePackage( cgScriptPackage * pPackage, bool bDeclareChildren )
{
    // Namespace promotion
    using namespace cgScriptInterop::Utils;
    using namespace cgScriptInterop::Exceptions;

    // Retrieve the namespace of the package (i.e. Core.Math.Matrix).
    cgString strNamespace = pPackage->getName();

    // Tokenize the namespace into its component parts.
    cgStringArray aTokens;
    cgStringUtility::tokenize( strNamespace, aTokens, _T(".") );

    // Find the correct spot in the namespace hierarchy for the package.
    PackageEntry * pEntry = &mRootPackage;
    for ( size_t i = 0; i < aTokens.size(); ++i )
        pEntry = &pEntry->childPackages[aTokens[i]];

    // Cannot exist at the root.
    if ( pEntry == &mRootPackage )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Attempted to declare a scripting package at the root level (no namespace was provided). This is not a valid operation and the package has been ignored.\n") );
        delete pPackage;
        return false;

    } // End if root

    // Destroy old package if one already existed here.
    if ( pEntry->package )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("A duplicate scripting package within the namespace '%s' was detected. The former package has been replaced.\n"), strNamespace.c_str() );
        unbindPackage( pEntry );
        
    } // End if replace

    // Begin configuration group for this package.
    cgToDo( "Scripting Packages", "A problem with AngelScript config group reference counting causes assert on shutdown." );
    //STRING_CONVERT;
    //const cgChar * lpszConfigGroup = StringConvertT2CA( strNamespace.c_str() );
    //mEngine->BeginConfigGroup( lpszConfigGroup );

    try
    {
        // Allow package to declare.
        pPackage->declare( this );

    } // End Try Block

    catch ( const BindException & e )
    {
        // Close the configuration group and remove it (roll back).
        //mEngine->EndConfigGroup( );
        //mEngine->RemoveConfigGroup( lpszConfigGroup );

        // Log error and clean up.
        cgAppLog::write( cgAppLog::Error, _T("Failed to declare types for script package '%s'. Engine reported an error of %s in %s.\n"), strNamespace.c_str(), getResultName(e.error).c_str(), e.getExceptionSource().c_str() );
        delete pPackage;
        return false;

    } // End registration failure

    catch ( ... )
    {
        // Close the configuration group and remove it (roll back).
        //mEngine->EndConfigGroup( );
        //mEngine->RemoveConfigGroup( lpszConfigGroup );

        // Log error and clean up.
        cgAppLog::write( cgAppLog::Error, _T("Failed to declare types for script package '%s'.\n"), strNamespace.c_str() );
        delete pPackage;
        return false;
    
    } // End generic exception

    // Close the configuration group.
    //mEngine->EndConfigGroup( );

    // Process children if requested.
    if ( bDeclareChildren && !pPackage->processChildPackages( this, true ) )
    {
        delete pPackage;
        return false;
    
    } // End if failed

    // ToDo: Remove when complete
    cgAppLog::write( cgAppLog::Debug, _T("Declared scripting package: %s\n"), strNamespace.c_str() );

    // Store new package at this location in the hierarchy.
    pEntry->package = pPackage;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : bindPackage ()
/// <summary>
/// Allow the specified scripting package (collection of type bindings) to bind
/// its types with  the scripting engine, optionally including its child 
/// packages.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::bindPackage( const cgString & strPackageNamespace, bool bBindChildren )
{
    // Namespace promotion
    using namespace cgScriptInterop::Utils;
    using namespace cgScriptInterop::Exceptions;

    // Tokenize the namespace into its component parts.
    cgStringArray aTokens;
    cgStringUtility::tokenize( strPackageNamespace, aTokens, _T(".") );

    // Find the package in the existing (declared) package hierarchy.
    PackageEntry * pEntry = &mRootPackage;
    for ( size_t i = 0; i < aTokens.size(); ++i )
    {
        PackageEntry::Map::iterator itPackage = pEntry->childPackages.find( aTokens[i] );
        if ( itPackage == pEntry->childPackages.end() )
            return false;
        pEntry = &itPackage->second;
    
    } // Next token

    // Valid package?
    cgScriptPackage * pPackage = pEntry->package;
    if ( !pPackage )
        cgAppLog::write( cgAppLog::Warning, _T("No scripting package was declared within the specified namespace '%s'. Request to bind package was ignored.\n"), strPackageNamespace.c_str() );    

    // Begin configuration group for this package.
    cgToDo( "Scripting Packages", "A problem with AngelScript config group reference counting causes assert on shutdown." );
    //STRING_CONVERT;
    //const cgChar * lpszConfigGroup = StringConvertT2CA( strPackageNamespace.c_str() );
    //mEngine->BeginConfigGroup( lpszConfigGroup );

    try
    {
        // Allow package to bind.
        pPackage->bind( this );

    } // End Try Block

    catch ( const BindException & e )
    {
        // Close the configuration group
        //mEngine->EndConfigGroup( );

        // Log error and clean up.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind types for script package '%s'. Engine reported an error of %s in %s.\n"), strPackageNamespace.c_str(), getResultName(e.error).c_str(), e.getExceptionSource().c_str() );
        return false;

    } // End registration failure

    catch ( ... )
    {
        // Close the configuration group and remove it (roll back).
        //mEngine->EndConfigGroup( );

        // Log error and clean up.
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind types for script package '%s'.\n"), strPackageNamespace.c_str() );
        return false;
    
    } // End generic exception

    // Close the configuration group.
    //mEngine->EndConfigGroup( );

    // Bind children if requested.
    if ( bBindChildren && !pPackage->processChildPackages( this, false ) )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unbindPackage () (Protected)
/// <summary>
/// Remove the specified package and its children from the hierarchy in 
/// addition to removing the bindings from the scripting engine.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::unbindPackage( PackageEntry * pEntry )
{
    // Recurse and release from the bottom up.
    PackageEntry::Map::iterator itChild;
    for ( itChild = pEntry->childPackages.begin(); itChild != pEntry->childPackages.end(); ++itChild )
        unbindPackage( &itChild->second );
    pEntry->childPackages.clear();

    // Remove the configuration group (if possible).
    if ( pEntry->package && mEngine )
    {
        cgToDo( "Scripting Packages", "A problem with AngelScript config group reference counting causes assert on shutdown." );
        //STRING_CONVERT;
        //const cgChar * lpszConfigGroup = StringConvertT2CA( pEntry->package->getName() );
        //mEngine->RemoveConfigGroup( lpszConfigGroup );
    
    } // End if remove bindings

    // Destroy any supplied package object.
    delete pEntry->package;
    pEntry->package = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : isLibraryLinked ()
/// <summary>
/// Determine if the specified script library has already been linked
/// with this engine instance.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::isLibraryLinked( const cgString & strLib ) const
{
    // Contained in linked library array?
    LinkedLibraryMap::const_iterator itLib = mLinkedLibraries.find( strLib );
    if ( itLib == mLinkedLibraries.end() )
        return false;

    // Has the link process run, or perhaps just the declare?
    return itLib->second->isLinked();
}

//-----------------------------------------------------------------------------
//  Name : isLibraryDeclared ()
/// <summary>
/// Determine if the specified script library has already provided its
/// public declarations with this engine instance.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::isLibraryDeclared( const cgString & strLib ) const
{
    // Contained in linked library array?
    LinkedLibraryMap::const_iterator itLib = mLinkedLibraries.find( strLib );
    if ( itLib == mLinkedLibraries.end() )
        return false;

    // Has the declaration process run?
    return itLib->second->isDeclared();
}

//-----------------------------------------------------------------------------
//  Name : declareLibrary ()
/// <summary>
/// Allow the specified script library to add declarations for its
/// defined types to this engine (definitions / linkage may be set up
/// later via a call to LinkLibrar()).
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::declareLibrary( const cgString & strLib )
{
    cgScriptLibrary * pLib = CG_NULL;
    
    // Already linked / declared?
    LinkedLibraryMap::iterator itLib;
    itLib = mLinkedLibraries.find( strLib );
    if ( itLib != mLinkedLibraries.end() )
        pLib = itLib->second;

    // Allocate the specified library if not yet processed
    if ( pLib == CG_NULL )
    {
        pLib = cgScriptLibrary::createInstance( strLib );
        if ( pLib == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to link script library '%s'. The library did not exist or was not registered correctly.\n"), strLib.c_str() );
            return false;
        
        } // End if didn't exist

        // This library is now linked
        mLinkedLibraries[ strLib ] = pLib;

    } // End if new

    // Ask the library to declare
    cgScriptLibrary::LinkResult Result = pLib->declare( this );
    if ( Result == cgScriptLibrary::Failed )
        return false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : linkLibrary ()
/// <summary>
/// Allow the specified script library to add linkage definitions to
/// this engine.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::linkLibrary( const cgString & strLib )
{
    cgScriptLibrary * pLib = CG_NULL;
    
    // Already linked / declared?
    LinkedLibraryMap::iterator itLib;
    itLib = mLinkedLibraries.find( strLib );
    if ( itLib != mLinkedLibraries.end() )
        pLib = itLib->second;

    // Allocate the specified library if not yet processed
    if ( pLib == CG_NULL )
    {
        pLib = cgScriptLibrary::createInstance( strLib );
        if ( pLib == CG_NULL )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to link script library '%s'. The library did not exist or was not registered correctly.\n"), strLib.c_str() );
            return false;
        
        } // End if didn't exist

        // This library is now linked
        mLinkedLibraries[ strLib ] = pLib;

    } // End if new

    // Ask the library to link with us
    cgScriptLibrary::LinkResult Result = pLib->link( this );
    if ( Result == cgScriptLibrary::Failed )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : linkLibrary ()
/// <summary>
/// Load the specified script to use as a scripted library file.
/// Functions exposed by this library can be imported by other scripts.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptEngine::linkLibrary( const cgInputStream & Stream, const cgString & strLibName, cgResourceManager * pManager /* = CG_NULL */ )
{
    // Already linked?
    if ( isLibraryLinked( strLibName ) == true )
        return true;

    // Use main resource manager to load the script if none supplied.
    if ( pManager == CG_NULL )
        pManager = cgResourceManager::getInstance();

    // Load the specified library (use deferred load so that we can change the name
    // of the resource prior to compiling the script).
    cgScriptHandle hScript;
    if ( pManager->loadScript( &hScript, Stream, cgResourceFlags::DeferredLoad, cgDebugSource() ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to link script library '%s' from '%s'. The library did not exist or could not be compiled correctly.\n"), strLibName.c_str(), Stream.getName().c_str() );
        return false;
    
    } // End if failed

    // Change the name of the resource and then load.
    cgScript * pScript = hScript.getResource( false );
    pScript->setResourceName( strLibName );
    if ( pScript->loadResource() == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to link script library '%s' from '%s'. The library did not exist or could not be compiled correctly.\n"), strLibName.c_str(), Stream.getName().c_str() );
        return false;
    
    } // End if failed
    
    // Add to the list of available library scripts.
    mScriptedLibraries[strLibName] = hScript;

    // This library is now linked (this is a scripted library rather than 
    // a C++ one so we just store CG_NULL in the list).
    mLinkedLibraries[strLibName] = CG_NULL;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getIdleExecuteContext ()
/// <summary>
/// Return an AngelScript execution context that is free to be executed
/// (i.e. is not currently running).
/// </summary>
//-----------------------------------------------------------------------------
asIScriptContext * cgScriptEngine::getIdleExecuteContext( )
{
    asIScriptContext * pContext = CG_NULL;

    // Any idle contexts already exist?
    ContextList::iterator itContext;
    for ( itContext = mExecuteContexts.begin(); itContext != mExecuteContexts.end(); ++itContext )
    {
        pContext = *itContext;
        if ( pContext != CG_NULL )
        {
            // Query the state, if it is in any way executing, or being prepared
            // to be executed, do not return it.
            int nState = pContext->GetState();
            if ( nState != asEXECUTION_SUSPENDED && nState != asEXECUTION_PREPARED &&
                 nState != asEXECUTION_ACTIVE )
            return pContext;

        } // End if context valid

    } // Next context

    // No idle contexts found so create one.
    pContext = mEngine->CreateContext();
    mExecuteContexts.push_back( pContext );
    
    // Return the new context
    return pContext;
}

//-----------------------------------------------------------------------------
//  Name : unloadScripts ()
/// <summary>
/// Unload any resident scripts to release any active handles.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptEngine::unloadScripts( )
{
    // First, destroy any currently active execution contexts.
    ContextList::iterator itContext;
    for ( itContext = mExecuteContexts.begin(); itContext != mExecuteContexts.end(); ++itContext )
    {
        if ( (*itContext) != CG_NULL )
        {
            (*itContext)->Unprepare();
            (*itContext)->Release();
        
        } // End if valid

    } // Next context
    mExecuteContexts.clear();

    // Unload all scripts loaded into this engine. The call to unloadResource should 
    // automatically remove it from the loaded script list if it was previously loaded.
    // Beware that unloading a script may cause other scripts to be unloaded and removed
    // from the list during this process, so we use a while!empty approach.
    for ( ; !mLoadedScripts.empty(); )
        ((cgResource*)(*mLoadedScripts.begin()))->unloadResource();    
}

//-----------------------------------------------------------------------------
//  Name : setDefaultNamespace ()
/// <summary>
/// Set the namespace in which all following registrations should be placed.
/// Use an empty string to clear the default namespace.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::setDefaultNamespace( const cgChar * lpszNamespaceNames )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->SetDefaultNamespace( lpszNamespaceNames );
}

//-----------------------------------------------------------------------------
//  Name : registerObjectMethod ()
/// <summary>
/// Registers a method for the object type.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerObjectMethod( const cgChar * lpszObject, const cgChar * lpszDecl, const asSFuncPtr & FunctionPointer, cgUInt32 nCallingConvention )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterObjectMethod( lpszObject, lpszDecl, FunctionPointer, (asDWORD)nCallingConvention );
}

//-----------------------------------------------------------------------------
//  Name : registerObjectProperty ()
/// <summary>
/// Registers a property for the object type.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerObjectProperty( const cgChar * lpszObject, const cgChar * lpszDecl, cgInt nByteOffset )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterObjectProperty( lpszObject, lpszDecl, nByteOffset );
}

//-----------------------------------------------------------------------------
//  Name : registerObjectType ()
/// <summary>
/// Registers a new object type.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerObjectType( const cgChar * lpszObject, cgInt nByteSize, cgUInt32 nFlags )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterObjectType( lpszObject, nByteSize, (asDWORD)nFlags );
}

//-----------------------------------------------------------------------------
//  Name : registerObjectBehavior ()
/// <summary>
/// Registers a behavior for the object type.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerObjectBehavior( const cgChar * lpszObject, asEBehaviours Behavior, const cgChar * lpszDecl, const asSFuncPtr & FunctionPointer, cgUInt32 nCallingConvention )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterObjectBehaviour( lpszObject, Behavior, lpszDecl, FunctionPointer, (asDWORD)nCallingConvention );
}

//-----------------------------------------------------------------------------
//  Name : registerInterface ()
/// <summary>
/// Registers an interface.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerInterface( const cgChar * lpszName )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterInterface( lpszName );
}

//-----------------------------------------------------------------------------
//  Name : registerInterfaceMethod ()
/// <summary>
/// Registers an interface method.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerInterfaceMethod( const cgChar * lpszInterface, const cgChar * lpszDecl )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterInterfaceMethod( lpszInterface, lpszDecl );
}

//-----------------------------------------------------------------------------
//  Name : registerStringFactory ()
/// <summary>
/// Registers the string factory.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerStringFactory( const cgChar * lpszDataType, const asSFuncPtr & FactoryFunctionPointer, cgUInt32 nCallingConvention )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterStringFactory( lpszDataType, FactoryFunctionPointer, (asDWORD)nCallingConvention  );
}

//-----------------------------------------------------------------------------
//  Name : registerGlobalFunction ()
/// <summary>
/// Registers a global function.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerGlobalFunction( const cgChar * lpszDecl, const asSFuncPtr & FunctionPointer, cgUInt32 nCallingConvention )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterGlobalFunction( lpszDecl, FunctionPointer, (asDWORD)nCallingConvention  );
}

//-----------------------------------------------------------------------------
//  Name : registerGlobalProperty ()
/// <summary>
/// Registers a global property accessible to all scripts.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerGlobalProperty( const cgChar * lpszDecl, void * value )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterGlobalProperty( lpszDecl, value  );
}

//-----------------------------------------------------------------------------
//  Name : registerEnum ()
/// <summary>
/// Registers an enum type.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerEnum( const cgChar * lpszName )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterEnum( lpszName );
}

//-----------------------------------------------------------------------------
//  Name : registerEnumValue ()
/// <summary>
/// Registers an enum value.
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgScriptEngine::registerEnumValue( const cgChar * lpszDataType, const cgChar * lpszValueName, cgInt nValue )
{
    cgAssertEx( mEngine != CG_NULL, "Ensure that the scripting engine has been initialized before beginning binding types." );
    return mEngine->RegisterEnumValue( lpszDataType, lpszValueName, nValue );
}