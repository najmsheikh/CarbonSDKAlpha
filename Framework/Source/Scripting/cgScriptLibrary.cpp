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
// Name : cgScriptLibrary.cpp                                                //
//                                                                           //
// Desc : Base class from which all script libraries should derive. These    //
//        library classes provide bindings and other support systems that    //
//        allow scripts to interact with the application.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScriptLibrary Module Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptLibrary.h>
#include <Scripting/cgScriptEngine.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgScriptLibrary::LibraryAllocTypeMap    cgScriptLibrary::mRegisteredLibs;

///////////////////////////////////////////////////////////////////////////////
// cgScriptLibrary Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScriptLibrary () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary::cgScriptLibrary( const cgString & strName )
{
    // Copy specified values
    mLibraryName = strName;
    mIsDeclared    = false;
    mIsLinked      = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgScriptLibrary () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary::~cgScriptLibrary()
{
    // Not used in this implementation
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Return the name of this script library.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgScriptLibrary::getName() const
{
    return mLibraryName;
}

//-----------------------------------------------------------------------------
//  Name : getDependencies () (Virtual)
/// <summary>
/// If this library has any dependencies, they should be listed here.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary::DependencyArray cgScriptLibrary::getDependencies() const
{
    // Default implementation returns an empty vector (no dependancies)
    return DependencyArray();
}

//-----------------------------------------------------------------------------
//  Name : declare () (Virtual)
/// <summary>
/// Provides only type declarations, but does not provide definitions
/// which is the responsibility of the 'link()' method.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary::LinkResult cgScriptLibrary::declare( cgScriptEngine * pScriptEngine )
{
    DependencyArray Dependencies = getDependencies();
    
    // Already declared?
    if ( mIsDeclared == true )
        return AlreadyProcessed;

    // Validate requirements
    if ( pScriptEngine == CG_NULL )
        return Failed;

    // Mark us as declared (even if it fails) to prevent circular dependancies
    // from causing a stack overflow.
    mIsDeclared = true;

    // Iterate through the dependencies and ensure they are declared as required.
    DependencyArray::iterator itDependency;
    for ( itDependency = Dependencies.begin(); itDependency != Dependencies.end(); ++itDependency )
    {
        const Dependency & Item = *itDependency;

        // Skip if this library has already provided its declarations to the engine
        if ( pScriptEngine->isLibraryDeclared( Item.library ) == true )
            continue;

        // Skip if this is a forward declaration, we'll defer this until the
        // final moment in the call to 'link'.
        if ( Item.declareOnly == true )
            continue;

        // Allow the library to declare.
        if ( pScriptEngine->declareLibrary( Item.library ) == false )
            return Failed;
        
    } // Next Dependency

    // Success!
    return Success;
}

//-----------------------------------------------------------------------------
//  Name : link () (Virtual)
/// <summary>
/// Default implementation of the link method. This base implementation
/// ensures that all dependencies have already been linked with the
/// engine. As a result, derived class method must call this first.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary::LinkResult cgScriptLibrary::link( cgScriptEngine * pScriptEngine )
{
    // Already linked ?
    if ( mIsLinked == true )
        return AlreadyProcessed;

    // Validate requirements
    if ( pScriptEngine == CG_NULL )
        return Failed;

    // If our own declaration process has not yet been run, run it before linking.
    if ( mIsDeclared == false )
    {
        if ( declare( pScriptEngine ) == Failed )
            return Failed;
    
    } // End if not declared

    // Mark us as linked (even if it fails) to prevent circular dependancies
    // from causing a stack overflow.
    mIsLinked = true;

    // First perform declaration of any deferred forward declared dependencies.
    DependencyArray::iterator itDependency;
    DependencyArray Dependencies = getDependencies();
    for ( itDependency = Dependencies.begin(); itDependency != Dependencies.end(); ++itDependency )
    {
        const Dependency & Item = *itDependency;

        // Forward declare?
        if ( Item.declareOnly == true )
        {
            // Skip if this library has already provided its declarations to the engine
            if ( pScriptEngine->isLibraryDeclared( Item.library ) == true )
                continue;

            // Allow the library to declare.
            if ( pScriptEngine->declareLibrary( Item.library ) == false )
                return Failed;

        } // End if declareOnly
        
    } // Next Dependency

    // Iterate through the dependencies and process any that require full linkage.
    for ( itDependency = Dependencies.begin(); itDependency != Dependencies.end(); ++itDependency )
    {
        const Dependency & Item = *itDependency;

        // Full linkage?
        if ( Item.declareOnly == false )
        {
            // Skip if this library has already been linked with the engine
            if ( pScriptEngine->isLibraryLinked( Item.library ) == true )
                continue;

            // Link the library
            if ( pScriptEngine->linkLibrary( Item.library ) == false )
                return Failed;

        } // End if !declareOnly
        
    } // Next Dependency

    // Success!
    return Success;
}

//-----------------------------------------------------------------------------
//  Name : isDeclared ()
/// <summary>
/// Has the 'declare' procedure been run on this script library?
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptLibrary::isDeclared( ) const
{
    return mIsDeclared;
}

//-----------------------------------------------------------------------------
//  Name : isLinked ()
/// <summary>
/// Has the 'link' procedure been run on this script library?
/// </summary>
//-----------------------------------------------------------------------------
bool cgScriptLibrary::isLinked( ) const
{
    return mIsLinked;
}

//-----------------------------------------------------------------------------
//  Name : registerType() (Static)
/// <summary>
/// Allows the application to register all of the various script library
/// types that are supported by the application. These types can then be 
/// referenced directly by the application by name.
/// </summary>
//-----------------------------------------------------------------------------
void cgScriptLibrary::registerType( const cgString & strTypeName, LibraryAllocFunc pFunction )
{
    // Store the function pointer
    mRegisteredLibs[ strTypeName ] = pFunction;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Call this function to create a script library of any given type, by
/// name.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptLibrary * cgScriptLibrary::createInstance( const cgString & strTypeName )
{
    LibraryAllocFunc Allocate = CG_NULL;
    LibraryAllocTypeMap::iterator itAlloc;

    // Find the allocation function based on the name specified
    itAlloc = mRegisteredLibs.find( strTypeName );
    if ( itAlloc == mRegisteredLibs.end() )
        return CG_NULL;

    // Extract the function pointer
    Allocate = itAlloc->second;
    if ( Allocate == CG_NULL )
        return CG_NULL;

    // Call the registered allocation function
    return Allocate( strTypeName );
}