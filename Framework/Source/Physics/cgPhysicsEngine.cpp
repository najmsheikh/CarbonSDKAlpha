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
// Name : cgPhysicsEngine.cpp                                                //
//                                                                           //
// Desc : Provides all of the physical interaction and movement functions    //
//        used for manipulating movable objects within the world.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsEngine Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsEngine.h>
#include <Physics/cgPhysicsWorld.h>
#include <System/cgStringUtility.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>    // GetPrivateProfileInt() -- Warning: Portability
#include <Mmsystem.h>   // timeGetTime() -- Warning: Portability
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgPhysicsEngine * cgPhysicsEngine::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsEngine Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsEngine () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEngine::cgPhysicsEngine()
{
    // Initialize variables to sensible defaults
    mConfigLoaded     = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsEngine () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEngine::~cgPhysicsEngine()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPhysicsEngine::dispose( bool bDisposeBase )
{
    // Shut down all registered worlds.
    for ( size_t i = 0; i < mWorlds.size(); ++i )
        mWorlds[i]->scriptSafeDispose();
    mWorlds.clear();

    // Clear variables
    mConfigLoaded     = false;
    mConfig            = InitConfig();
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEngine * cgPhysicsEngine::getInstance( )
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
void cgPhysicsEngine::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgPhysicsEngine;
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsEngine::destroySingleton( )
{
    // Destroy (unless script still referencing)!
    if ( mSingleton )
        mSingleton->scriptSafeDispose( );
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : loadConfig ()
// Desc : Load the physics engine configuration from the file specified.
// Note : This function can return several status codes. The meanings follow:
//
//        Error    - An error occured whilst attempting to load and / or
//                   validate the configuration options specified. The
//                   application should probably exit.
//
//        Mismatch - The configuration information could not be matched
//                   to any valid modes supported by the physics engine.
//                   In this case, it's most likely that the software, 
//                   hardware or drivers were changed since last time 
//                   the application was run. If this value is returned,
//                   then the application should notify the user, and 
//                   then call the loadDefaultConfig function to have 
//                   it pick a sensible set of default options.
//
//        Valid    - The configuration options were loaded sucessfully,
//                   and they were also matched exactly to a supported
//                   mode.
//-----------------------------------------------------------------------------
cgConfigResult::Base cgPhysicsEngine::loadConfig( const cgString & strFileName )
{
    // Fail if config already loaded
    if ( mConfigLoaded == true )
        return cgConfigResult::Error;

    // Retrieve configuration options if provided
    if ( strFileName.empty() == false )
    {
        const cgTChar * strSection   = _T("Physics");
        mConfig.useVDB              = GetPrivateProfileInt( strSection, _T("UseVDB"), 0, strFileName.c_str() ) > 0;
        mConfig.disableDeactivation = GetPrivateProfileInt( strSection, _T("DisableDeactivation"), 0, strFileName.c_str() ) > 0;
        mConfig.verboseOutput       = GetPrivateProfileInt( strSection, _T("VerboseOutput"), 0, strFileName.c_str() ) > 0;
        
    } // End if config provided

    // Signal that we have settled on good config options
    mConfigLoaded = true;
    
    // Options are valid. Success!!
    return cgConfigResult::Valid;
}

//-----------------------------------------------------------------------------
//  Name : loadDefaultConfig ()
/// <summary>
/// Load a default configuration for the render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgConfigResult::Base cgPhysicsEngine::loadDefaultConfig( )
{
    // Pick sensible defaults
    mConfig.useVDB = false;
    
    // Pass through to the loadConfig function
    return loadConfig( _T("") );
}

//-----------------------------------------------------------------------------
//  Name : saveConfig ()
/// <summary>
/// Save the render driver configuration from the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsEngine::saveConfig( const cgString & strFileName )
{
    const cgTChar * strSection = _T("Physics");

    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Save configuration options
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("UseVDB"), mConfig.useVDB, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("DisableDeactivation"), mConfig.disableDeactivation, strFileName.c_str() );
    cgStringUtility::writePrivateProfileIntEx( strSection, _T("VerboseOutput"), mConfig.verboseOutput, strFileName.c_str() );

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsEngine::InitConfig cgPhysicsEngine::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Initialize the physics subsystems ready for simulation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsEngine::initialize( )
{
    // Set the memory allocators
	//NewtonSetMemorySystem (AllocMemory, FreeMemory);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createWorld()
/// <summary>
/// Allocate a new world object with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld * cgPhysicsEngine::createWorld( )
{
    // Allocate a new world object.
    cgPhysicsWorld * pNewWorld = new cgPhysicsWorld( this);

    // Add to the active world containers.
    mWorlds.push_back( pNewWorld );

    // Success!!
    return pNewWorld;
}

//-----------------------------------------------------------------------------
//  Name : removeWorld ()
/// <summary>
/// Remove the specified world from the physics engine. Optionally, you
/// can prevent the world from being implicitly disposed by specifying
/// false to the final parameter.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsEngine::removeWorld( cgPhysicsWorld * pWorld, bool bDispose /* = true */ )
{
    // Paranoia
    if ( pWorld == CG_NULL )
        return;

    // Remove from the loaded world containers
   for ( size_t i = 0; i < mWorlds.size(); ++i )
    {
        if ( mWorlds[i] == pWorld )
        {
            mWorlds.erase( mWorlds.begin() + i );
            break;
        
        } // End if match
    } // Next World
    
    // Cleanup the world if requested, but pay attention to script
    // reference count on delete.
    if ( bDispose == true )
        pWorld->scriptSafeDispose( );
}

//-----------------------------------------------------------------------------
//  Name : getWorldCount ()
/// <summary>
/// Retrieve the total number of active worlds.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgPhysicsEngine::getWorldCount( ) const
{
    return (cgUInt32)mWorlds.size();
}

//-----------------------------------------------------------------------------
//  Name : getWorld ()
/// <summary>
/// Retrieve the referenced world object from the engine's internal
/// database. This particular method allows you to retrieve a world
/// by index (in conjunction with getWorldCount()).
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld * cgPhysicsEngine::getWorld( cgUInt32 nIndex )
{
    // Out of range?
    if ( nIndex >= mWorlds.size() )
        return CG_NULL;

    // Return the world at this location
    return mWorlds[ nIndex ];
}