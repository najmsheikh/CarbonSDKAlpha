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
// Name : cgScene.cpp                                                        //
//                                                                           //
// Desc : Provides classes responsible for the loading, management, update   //
//        and rendering of an individual scene.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScene Module Includes
//-----------------------------------------------------------------------------
#include <World/cgScene.h>
#include <World/cgWorld.h>
#include <World/cgWorldConfiguration.h>
#include <World/cgSceneController.h>
#include <World/cgSceneCell.h>
#include <World/cgObjectNode.h>
#include <World/cgLandscape.h>
#include <World/Lighting/cgLightingManager.h>
#include <World/Objects/cgSpatialTreeObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <World/Objects/cgGroupObject.h>
#include <World/Objects/cgActor.h>
#include <World/Objects/cgMeshObject.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgStandardMaterial.h>
#include <Resources/cgScript.h>
#include <Resources/cgHeightMap.h> // ToDo: 9999 - Remove when we have completed testing
#include <Physics/cgPhysicsEngine.h>
#include <Physics/cgPhysicsWorld.h>
#include <Rendering/cgRenderDriver.h>
#include <Audio/cgAudioDriver.h>
#include <System/cgStringUtility.h>
#include <System/cgMessageTypes.h>
#include <System/cgExceptions.h>
#include <System/cgProfiler.h>
#include <Math/cgMathUtility.h>
#include <algorithm>

// ToDo: 9999 - Replace all 'scriptSafeDispose()' with 'deleteReference()' calls solution wide.

// ToDo: 9999 - Should we just addReference() all nodes to prevent cases where
// the node is prematurely destroyed? If so, we can get rid of the 'isValidReference()'
// call in cgScene::deleteObjectNodes()

// ToDo: 9999 - When an object is deleted, it leaves the cell in which it existed
// in-tact even if that cell is now empty. Care has to be taken here because 
// the cell system needs to understand when an object has been truly deleted
// versus the object simply having been unloaded

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgExceptions;

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgScene::mInsertMaterialUsage;
cgWorldQuery cgScene::mDeleteMaterialUsage;
cgWorldQuery cgScene::mUpdateLandscapeRef;

///////////////////////////////////////////////////////////////////////////////
// cgSceneDescriptor Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSceneDescriptor() (Constructor)
/// <summary>Default class constructor.</summary>
//-----------------------------------------------------------------------------
cgSceneDescriptor::cgSceneDescriptor( )
{
    // Initialize variables to sensible defaults.
    sceneId                 = 0;
    type                    = cgSceneType::Standard;
    flags                   = 0;
    renderContext           = cgSceneRenderContext::Runtime;
    distanceDisplayUnits    = cgUnitType::Meters;
    sceneBounds.min         = cgVector3( -250, -250, -250 );
    sceneBounds.max         = cgVector3(  250,  250,  250 );
    cellDimensions          = cgVector3( 58.5216f, 58.5216f, 58.5216f ); // 192ft, 6ft per heightmap sample
    landscapeId             = 0;

    // Sandbox?
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
        renderContext = cgSceneRenderContext::SandboxRender;
}

///////////////////////////////////////////////////////////////////////////////
// cgScene Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgScene() (Constructor)
/// <summary>Class constructor.</summary>
/// <param name="world">The world in which this scene is to exist.</param>
/// <param name="description">Description of the high-level properties that 
/// should be assumed for the created scene.</param>
//-----------------------------------------------------------------------------
cgScene::cgScene( cgWorld * world, const cgSceneDescriptor * description )  : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mPhysicsWorld         = CG_NULL;
    mActiveCamera         = CG_NULL;
    mScriptObject         = CG_NULL;
    mLandscape            = CG_NULL;
    mPassBegun            = false;
    mIsLoading            = false;
    mIsDirty              = false;
    mDynamicsEnabled      = true;
    mSceneWritesEnabled   = true;
    mNextSelectionId      = 0;
    mActiveElementType    = cgUID::Empty;
    mOnSceneRenderMethod  = CG_NULL;

    // Allocate the lighting manager on the heap
    mLightingManager      = new cgLightingManager( this );

    // Store required values
    mWorld                = world;
    mSceneDescriptor      = *description;

    // Reset the update rate structures
    for ( cgUInt32 i = 0; i < cgUpdateRate::Count; ++i )
    {
        mUpdateBuckets[i].lastUpdateTime = -1.0f;
        mUpdateBuckets[i].nextUpdateTime = -1.0f;
    
    } // Next interval

    // Subscribe to the resource manager messaging group so that we can 
    // listen in for script reloads (surface shader script).
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_ResourceManager );
}

//-----------------------------------------------------------------------------
//  Name : ~cgScene() (Destructor)
/// <summary>
/// Class destructor. Falls through to the class' <see cref="dispose()" />
/// method.
/// </summary>
//-----------------------------------------------------------------------------
cgScene::~cgScene()
{
    // Release allocated memory
    dispose( false );

    // Detroy constructor allocated lighting manager.
    if ( mLightingManager )
        mLightingManager->scriptSafeDispose();
    mLightingManager = CG_NULL;

}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgScene::dispose( bool disposeBase )
{
    // We are in the process of disposing.
    mDisposing = true;

    // Finish any rendering operations.
    endRenderPass();

    // Access required systems
    cgRenderDriver * driver = getRenderDriver();

    // Unset the active camera if any
    if ( driver->getCamera() == mActiveCamera )
        driver->setCamera( CG_NULL );

    // Release any script objects we retain.
    if ( mLightingManager )
        mLightingManager->setRenderControl( CG_NULL );
    if ( mScriptObject )
        mScriptObject->release();
    mScriptObject = CG_NULL;
    
    // Render control must be destroyed first in
    // case it maintains references to scene managed objects.
    mRenderScript.close( true );

    // Remove any scene controllers
    for ( size_t i = 0; i < mSceneControllers.size(); ++i )
        mSceneControllers[i]->scriptSafeDispose();
    mSceneControllers.clear();

    // Destroy nodes.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mObjectNodes.begin(); itNode != mObjectNodes.end(); ++itNode )
        itNode->second->scriptSafeDispose();

    // Clear the update rate structures (objects are "dead")
    for ( cgUInt32 i = 0; i < cgUpdateRate::Count; ++i )
    {
        mUpdateBuckets[i].lastUpdateTime = -1.0f;
        mUpdateBuckets[i].nextUpdateTime = -1.0f;
        mUpdateBuckets[i].nodes.clear();
    
    } // Next interval

    // Destroy any active landscape.
    if ( mLandscape )
        mLandscape->scriptSafeDispose();

    // Reset the lighting manager.
    if ( mLightingManager )
        mLightingManager->dispose( false );

    // Destroy selection sets
    SelectionSetMap::iterator itSet;
    for ( itSet = mSelectionSets.begin(); itSet != mSelectionSets.end(); ++itSet )
        delete itSet->second;

    // Disconnect from any scene materials (not a full
    // reference release. This ensures that the materials
    // remain referenced in the database appropriately).
    SceneMaterialMap::iterator itMaterial;
    for ( itMaterial = mActiveMaterials.begin(); itMaterial != mActiveMaterials.end(); ++itMaterial )
        itMaterial->second->removeReference( CG_NULL, true );
    
    // Clean up cell data.
    cgSceneCellMap::iterator itCell;
    for ( itCell = mCells.begin(); itCell != mCells.end(); ++itCell )
        itCell->second->scriptSafeDispose();

    // Clear variables
    mPhysicsWorld         = CG_NULL;
    mActiveCamera         = CG_NULL;
    mLandscape            = CG_NULL;
    mActiveElementType    = cgUID::Empty;
    mDynamicsEnabled      = true;
    mSceneWritesEnabled   = true;
    mOnSceneRenderMethod  = CG_NULL;

    // Clear containers
    mRootSpatialTrees.clear();
    mCells.clear();
    mOrphanNodes.clear();
    mObjectNodes.clear();
    mNameUsage.clear();
    mSelectedNodes.clear();
    mSelectedNodesOrdered.clear();
    mSelectionSets.clear();
    mActiveMaterials.clear();

    // Call base class implementation as required
    if ( disposeBase )
        cgReference::dispose( true );
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
bool cgScene::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Scene )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::processMessage( cgMessage * message )
{
    // What message is this?
    switch ( message->messageId )
    {
        case cgSystemMessages::Resources_ReloadScripts:

            // Recreate the scripted render control class. There is no
            // need to reload the script because the resource manager
            // will already have taken care of that aspect.
            reloadRenderControl( false );
            
            // Processed message
            return true;
    
    } // End message type switch
    
    // Message was not processed, pass to base.
    return cgReference::processMessage( message );
}

//-----------------------------------------------------------------------------
//  Name : reloadRenderControl () (Protected)
/// <summary>
/// Reload and re-initialize the render control script and the instantitated
/// script object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::reloadRenderControl( bool reloadScript )
{
    // unload the old render control object.
    if ( mScriptObject )
        mScriptObject->release();
    mScriptObject = CG_NULL;

    // Force lighting manager to clean up its data.
    if ( mLightingManager )
        mLightingManager->dispose( false );

    // Should the script itself be reloaded?
    if ( reloadScript )
    {
        mRenderScript.unloadResource();
        mRenderScript.loadResource();
    
    } // End if reload script

    // Instantiate a new scripted render control class.
    try
    {
        cgScript * script = mRenderScript.getResource(true);
        if ( !script || !script->isLoaded() )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to reload render control script for scene '%s'.\n"), getName().c_str() );
            return false;

        } // End if !loaded

        // Create the new script object.
        cgScriptArgument::Array scriptArgs;
        scriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("Scene@+"), (void*)this ) );
        scriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("SceneRenderContext"), &mSceneDescriptor.renderContext ) );
        mScriptObject = script->executeFunctionObject( _T("IScriptedRenderControl"), _T("createRenderControlScript"), scriptArgs );
        if ( !mScriptObject )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create necessary IScriptedRenderControl object in scene '%s'. No error was reported.\n"), getName().c_str() );
            return false;
        
        } // End if no object

    } // End try to execute

    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute createRenderControlScript() function in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        return true;

    } // End catch exception

    // Re-execute render control script functions to allow it to re-initialize.
    try
    {
        bool callSucceeded = false, result = false;
        
        // Attempt to execute the specified (optional) script event functions.
        result = mScriptObject->executeMethodBool( _T("onScenePreLoad"), cgScriptArgument::Array(), true, &callSucceeded );
        if ( callSucceeded && !result )
        {
            mScriptObject->release();
            mScriptObject = CG_NULL;
            return false;
        
        } // End if failure

        result = mScriptObject->executeMethodBool( _T("onSceneLoaded"), cgScriptArgument::Array(), true, &callSucceeded );
        if ( callSucceeded && !result )
        {
            mScriptObject->release();
            mScriptObject = CG_NULL;
            return false;
        
        } // End if failure

    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to re-initialize render control script '%s' during script reload event. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        mScriptObject->release();
        mScriptObject = CG_NULL;
        return false;
    
    } // End catch exception

    // Re-initialize the lighting manager.
    if ( mLightingManager )
        mLightingManager->initialize( mScriptObject );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : reload ()
/// <summary>Unload and then immediately reload the scene data.</summary>
//-----------------------------------------------------------------------------
bool cgScene::reload( )
{
    // Backup necessary settings
    bool oldDynamics = mDynamicsEnabled;

    // Clear out scene data.
    dispose(false);

    // Restore necessary settings
    mDynamicsEnabled = oldDynamics;

    // Reload the scene
    return load();
}

//-----------------------------------------------------------------------------
//  Name : load ()
/// <summary>Initialize the scene and begin the loading process.</summary>
/// <remarks>
/// The scene that will be loaded is based on the information provided within
/// the <see cref="cgSceneDescriptor" /> object instance passed to the
/// scene class constructor.
/// <para>
/// This method cannot be called directly. Instead, the application
/// should load a new scene via the <see cref="cgWorld::LoadScene()" /> 
/// method.</para>
/// </remarks>
/// <seealso cref="cgScene()"/>
/// <seealso cref="cgWorld::LoadScene()"/>
//-----------------------------------------------------------------------------
bool cgScene::load( )
{
    // ToDo: 9999 - When render control script fails to compile / load
    // the application crashes unexpectedly. This may now have been fixed
    // after moving to the interface script driven approach but I want to confirm it.

    // First select the scene information from the database if not an internal scene.
    cgUInt32 sceneId = mSceneDescriptor.sceneId;
    if ( sceneId )
    {
        cgString queryString = cgString::format( _T("SELECT * FROM 'Scenes' WHERE SceneId=%i LIMIT 1"), sceneId );
        cgWorldQuery sceneQuery( mWorld, queryString );
        if ( !sceneQuery.step() || !sceneQuery.nextRow() )
        {
            cgString error;
            if ( !sceneQuery.getLastError( error ) )
                cgAppLog::write( cgAppLog::Error, _T("Unable to retrieve scene information for scene id %i. World database has potentially become corrupt.\n"), sceneId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Unable to retrieve scene information for scene id %i. Error: %s\n"), sceneId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if !internal

    // We are in the process of loading.
    mIsLoading = true;

    // Get access to required systems.
    cgResourceManager * resources  = cgResourceManager::getInstance();
    cgPhysicsEngine   * physics    = cgPhysicsEngine::getInstance();

    // Create and initialize a new physics world for this scene
    mPhysicsWorld = physics->createWorld( );
    if ( !mPhysicsWorld->initialize( mSceneDescriptor.sceneBounds ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize physics world while loading scene '%s'.\n"), getName().c_str() );
        mIsLoading = false;
        return false;

    } // End if failed

    // Initialize the physics unit conversion system.
    mPhysicsWorld->setSystemScale( 1.0f );
    mPhysicsWorld->setDefaultGravity( cgVector3(0, -15.0f, 0) );

    // Select the correct render control definition
    // ToDo: 9999 - Get default script from world configuration.
    cgString renderScriptFile = mSceneDescriptor.renderControl;
    if ( renderScriptFile.empty() )
        renderScriptFile = _T("sys://Scripts/Render Control/Default.gs");

    // Create the render control script
    if ( !resources->loadScript( &mRenderScript, renderScriptFile, 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Failed to compile scene render control script '%s' while loading scene '%s'.\n"), renderScriptFile.c_str(), getName().c_str() );
        mIsLoading = false;
        return false;

    } // End if failed to load render control script

    // Instantiate the scripted render control class.
    try
    {
        cgScript * script = mRenderScript.getResource(true);
        cgScriptArgument::Array scriptArgs;
        scriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("Scene@+"), (void*)this ) );
        scriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("SceneRenderContext"), &mSceneDescriptor.renderContext ) );
        mScriptObject = script->executeFunctionObject( _T("IScriptedRenderControl"), _T("createRenderControlScript"), scriptArgs );
        if ( !mScriptObject )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create necessary IScriptedRenderControl object in '%s'. No error was reported.\n"), renderScriptFile.c_str() );
            return false;
        
        } // End if no object

    } // End try to execute

    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute createRenderControlScript() function in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        return false;

    } // End catch exception

    // Initialize the lighting system
    if ( !mLightingManager->initialize( mScriptObject ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize the lighting manager for scene '%s'.\n"), getName().c_str() );
        return false;
    
    } // End if failed

    // Notify the script that we're about to load the scene.
    try
    {
        bool callSucceeded = false, result = false;
        
        // Attempt to execute the specified (optional) script event function.
        result = mScriptObject->executeMethodBool( _T("onScenePreLoad"), cgScriptArgument::Array(), true, &callSucceeded );

        // If the call succeeded, but it requested that we exit, do so.
        if ( callSucceeded && !result )
        {
            mIsLoading = false;
            return false;
        
        } // End if failure
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute onScenePreLoad() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        mIsLoading = false;
        return false;
    
    } // End catch exception

    // Begin loading the scene data
    try
    {
        // Process material usage data for this scene if we're 
        // in sandbox mode and this is not an internal scene.
        if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && sceneId )
        {
            cgString queryString = cgString::format( _T("SELECT * FROM 'Scenes::MaterialUsage' WHERE SceneId=%i"), sceneId );
            cgWorldQuery query( mWorld, queryString );
            if ( !query.step() )
            {
                cgString strError;
                query.getLastError( strError );
                throw ResultException( cgString::format( _T("Unable to retrieve material usage data for scene id %i. Error: %s"), sceneId, strError.c_str() ), cgDebugSource() );
            
            } // End if failed
            for ( ; query.nextRow(); )
            {
                // Load the specified material data.
                cgUInt32 materialId, materialTypeId;
                cgMaterialHandle materialHandle;
                query.getColumn( _T("MaterialType"), materialTypeId );
                query.getColumn( _T("MaterialId"), materialId );
                if ( !resources->loadMaterial( &materialHandle, mWorld, (cgMaterialType::Base)materialTypeId, materialId, false, 0, cgDebugSource() ) )
                    throw ResultException( cgString::format( _T("Unable to load material marked as required for scene id %i."), sceneId ), cgDebugSource() );

                // Attach this scene as an owner (reconnecting).
                cgMaterial * material = materialHandle.getResource(true);
                material->addReference( CG_NULL, true );
                mActiveMaterials[ material->getReferenceId() ] = material;

            } // Next Node

        } // End if sandbox mode

        // Load any landscape associated with this scene.
        if ( mSceneDescriptor.landscapeId )
        {
            mLandscape = new cgLandscape( this );
            if ( !mLandscape->load( mSceneDescriptor.landscapeId ) )
                throw ResultException( cgString::format( _T("Unable to load landscape data for scene id %i. World database has potentially become corrupt."), sceneId ), cgDebugSource() );

        } // End if has landscape

        // Load the data associated with this scene (if not internal).
        if ( sceneId && !loadAllCells( ) )
            throw ResultException( cgString::format( _T("Unable to load initial scene cell data for scene id %i. World database has potentially become corrupt."), sceneId ), cgDebugSource() );

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        mIsLoading = false;
        return false;
    
    } // End catch

    // Notify the script that the load has completed
    try
    {
        bool callSucceeded = false, result = false;
        
        // Attempt to execute the specified (optional) script event function.
        result = mScriptObject->executeMethodBool( _T("onSceneLoaded"), cgScriptArgument::Array(), true, &callSucceeded );

        // If the call succeeded, and it requested that we exit, do so.
        if ( callSucceeded && !result )
        {
            mIsLoading = false;
            return false;
        } // End if failure
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute onSceneLoaded() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        mIsLoading = false;
        return false;
    
    } // End catch exception

    // We are no longer loading.
    mIsLoading = false;

    // If this is an exterior scene but it does not yet have a 
    // landscape, generate a default one (full sandbox mode only).
    if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && sceneId && mSceneDescriptor.type == cgSceneType::Exterior && !mSceneDescriptor.landscapeId )
    {
        // Compute the maximum cell count for the scene.
        cgVector3 sceneDimensions = mSceneDescriptor.sceneBounds.getDimensions();
        cgVector3 cellDimensions  = mSceneDescriptor.cellDimensions;
        cgUInt32 maxCellsX = (cgUInt32)ceilf( sceneDimensions.x / cellDimensions.x );
        cgUInt32 maxCellsZ = (cgUInt32)ceilf( sceneDimensions.z / cellDimensions.z );

        // Since the world cells are symmetrical starting with a
        // single initial center cell, maximum cell count should always
        // be an odd number (i.e. 10 on the left of the center cell and 
        // 10 on the right = 21).
        if ( cgMathUtility::isEven( maxCellsX ) )
            maxCellsX++;
        if ( cgMathUtility::isEven( maxCellsZ ) )
            maxCellsZ++;

        // Create an empty landscape.
        cgLandscapeImportParams importParams;
        importParams.dimensions        = cgVector3( maxCellsX * cellDimensions.x, sceneDimensions.y, maxCellsZ * cellDimensions.z );
        importParams.offset            = cgVector3( importParams.dimensions.x * -0.5f, 0, importParams.dimensions.z * 0.5f );
        importParams.blockGrid         = cgSize(32,32);
        importParams.blockBlendMapSize = cgSize(32,32);
        importParams.initFlags         = cgLandscapeFlags::Dynamic | cgLandscapeFlags::LODIgnoreY;
        importParams.heightMap         = new cgHeightMap( cgSize((maxCellsX * importParams.blockGrid.width) + 1, 
                                                          (maxCellsZ * importParams.blockGrid.height) + 1) );
        importLandscape( importParams );
        
        // The landscape is also our main scene spatial tree.
        /*setSceneTree( mLandscape );*/
        
        // Mark the scene as immediately dirty.
        setDirty( true );
    
    } // End if generate landscape
    
    // ToDo: 9999 - Original Loading Code
    /*cgInputStream      Stream;
    cgSceneLoader      Loader;
    
    // Validate requirements
    if ( mSceneDescriptor == CG_NULL )
        return false;

    // ToDo: Prevent loading twice.

    // We are now in the process of loading.
    mIsLoading = true;

    // Get access to required systems.
    cgResourceManager * resources  = cgResourceManager::getInstance();
    cgPhysicsEngine   * physics    = cgPhysicsEngine::getInstance();

    // Create and initialize a new physics world for this scene
    mPhysicsWorld = physics->createWorld( getName() );
    if ( mPhysicsWorld->initialize() == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize physics world while loading scene '%s'.\n"), getName().c_str() );
        mIsLoading = false;
        return false;

    } // End if failed

    // ToDo: Hardcoded currently.
    // Initialize the physics unit conversion system.
    mPhysicsWorld->SetUnitConversion( cgUnitType::Meters, 15.0f );

    // ToDo: Add a System/Scripts/Render Control/Default.gs fallback render control script.

    // Select the correct render control definition
    cgString renderScriptFile = mSceneDescriptor.strRenderControl;
    if ( renderScriptFile.empty() == true )
        renderScriptFile = _T("sys://Scripts/Render Control/Default.gs");

    // Create the render control script
    if ( resources->loadScript( &mRenderScript, renderScriptFile, _T("Scene"), getName(), 0, cgDebugSource() ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Failed to compile scene render control script '%s' while loading scene '%s'.\n"), renderScriptFile.c_str(), getName().c_str() );
        mIsLoading = false;
        return false;

    } // End if failed to load render control script

    // Set the script's global "this" handle.
    cgScript * script = mRenderScript;
    script->SetThisObject( this );

    // Notify the script that we're about to load the scene.
    try
    {
        bool callSucceeded = false, result = false;
        
        // Attempt to execute the specified (optional) script event function.
        result = script->ExecuteFunctionBool( _T("onScenePreLoad"), cgScriptArgument::Array(), true, &callSucceeded );

        // If the call succeeded, and it requested that we exit, do so.
        if ( callSucceeded == true && result == false )
        {
            mIsLoading = true;
            return false;
        
        } // End if failure
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute onScenePreLoad() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        mIsLoading = false;
        return false;
    
    } // End catch exception

    // Begin loading the scene data
    if ( Loader.load( mSceneDescriptor.strSceneData, resources, this ) == false )
    {
        mIsLoading = false;
        return false;
    
    } // End if failed

    // Notify the script that the load has completed
    try
    {
        bool callSucceeded = false, result = false;
        
        // Attempt to execute the specified (optional) script event function.
        result = script->ExecuteFunctionBool( _T("onSceneLoaded"), cgScriptArgument::Array(), true, &callSucceeded );

        // If the call succeeded, and it requested that we exit, do so.
        if ( callSucceeded == true && result == false )
        {
            mIsLoading = false;
            return false;
        } // End if failure
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute onSceneLoaded() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        mIsLoading = false;
        return false;
    
    } // End catch exception

    // Create the default material used for the 'CollapseMaterials' batching method.
    // ToDo: Error handling.
    ResourceHandle hDefaultEffect;
    cgMaterial * pDefaultMaterial = new cgMaterial( 0 );
    pDefaultMaterial->BeginPopulate( resources );
    resources->LoadEffectFile( &hDefaultEffect, resources->GetDefaultEffectFile(), 0, cgDebugSource() );
    pDefaultMaterial->SetEffectFile( hDefaultEffect );
    pDefaultMaterial->EndPopulate();
    resources->AddMaterial( &m_hDefaultMaterial, pDefaultMaterial, 0, cgDebugSource() );

    // ToDo: Allow for script to register other collapse materials referencable by name,
    // or perhaps just have the BeginDrawBatch() accept a resource handle to a material.

    // ToDo: Until we decide how to handle "action" definitions in Prescience, we'll
    //       just create a new animation controller for each animation set loaded from
    //       the file and leave it running.
    const ResourceArray & LoadedSets = Loader.GetLoadedAnimationSets();
    for ( size_t i = 0; i < LoadedSets.size(); ++i )
    {
        cgAnimationController * controller = new cgAnimationController( this );
        controller->SetControllerEnabled( true );
        controller->SetTrackAnimationSet( 0, LoadedSets[i] );
        addController( controller );
        
    } // Next Animation Set
    
    // ToDo: Supply scene tree?
    //m_pCollisionDB->SetSceneTree( m_pSpatialTree );*/

    //cgToDoAssert( "Carbon Testing", "Applying a local / data level scale to all elements in the scene." );
    //applySceneRescale( 1.0f / 90.0f );
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadAllCells()
/// <summary>
/// Load the scene data from the world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::loadAllCells( )
{
    // Begin loading the scene data
    try
    {
        cgWorldConfiguration * config = mWorld->getConfiguration();

        // Retrieve the cells associated with this scene if not an internal scene.
        cgUInt32 sceneId = getSceneId();
        if ( sceneId != 0 )
        {
            // Select all cells
            cgString queryString = cgString::format( _T("SELECT * FROM 'Cells' WHERE SceneId=%i"), sceneId );
            cgWorldQuery cellQuery( mWorld, queryString );
            if ( cellQuery.step() == false )
            {
                cgString strError;
                if ( cellQuery.getLastError( strError ) == false )
                    throw ResultException( cgString::format( _T("Unable to retrieve cell data for scene id %i. World database has potentially become corrupt."), sceneId ), cgDebugSource() );
                else
                    throw ResultException( cgString::format( _T("Unable to retrieve cell data for scene id %i. Error: %s"), sceneId, strError.c_str() ), cgDebugSource() );
            
            } // End if failed

            // Process all cells
            for ( ; cellQuery.nextRow(); )
            {
                // ToDo: 9999 - What if it's already been dynamically created?
                // We need a way to merge temporary cell with loaded cell.

                // Has this cell already been loaded?
                cgSceneCellKey cellKey;
                cellQuery.getColumn( _T("LocationX"), cellKey.cellX );
                cellQuery.getColumn( _T("LocationY"), cellKey.cellY );
                cellQuery.getColumn( _T("LocationZ"), cellKey.cellZ );
                if ( mCells.find( cellKey ) != mCells.end() )
                    continue;

                // Create and load the new cell.
                cgSceneCell * cell = new cgSceneCell( this );
                if ( cell->load( &cellQuery ) == false )
                {
                    cell->scriptSafeDispose();
                    continue;
                
                } // End if failed

                // Insert into the cell database
                mCells[ cellKey ] = cell;

                // Proceed to load all nodes associated with this cell.
                queryString = cgString::format( _T("SELECT * FROM 'nodes' WHERE CellId=%i ORDER BY Level ASC"), cell->getCellId() );
                cgWorldQuery query( mWorld, queryString );
                if ( query.step() == false )
                {
                    cgString error;
                    query.getLastError( error );
                    throw ResultException( cgString::format( _T("Unable to retrieve nodes from cell id %i for scene id %i. Error: %s"), cell->getCellId(), sceneId, error.c_str() ), cgDebugSource() );
                
                } // End if failed
                for ( ; query.nextRow(); )
                {
                    // Get the reference identifier of the node to load.
                    cgUInt32 referenceId = 0;
                    query.getColumn( _T("RefId"), referenceId );

                    // Get the reference identifier of the parent of this node (if any) and
                    // find the parent node in the loaded list.
                    cgUInt32 parentReferenceId = 0;
                    cgObjectNode * parentNode = CG_NULL;
                    query.getColumn( _T("ParentRefId"), parentReferenceId );
                    cgObjectNodeMap::iterator itNode = mObjectNodes.find( parentReferenceId );
                    if ( itNode != mObjectNodes.end() )
                        parentNode = itNode->second;

                    // Instantiate the object node
                    cgObjectNode * node = loadObjectNode( referenceId, referenceId, &query, cgCloneMethod::None, cell, parentNode, mObjectNodes, false );
                    if ( !node )
                        continue;

                    // Insert node into cell.
                    cell->addNode( node );

                    // Notify whoever is interested that the scene was modified
                    onNodeAdded( &cgNodeUpdatedEventArgs( this, node ) );

                } // Next Node

            } // Next Cell

        } // End if !internal scene

        // Resolve any remaining information.
        resolvePendingUpdates();

        // Process all loaded nodes and allow them to initialize now that the entire scene
        // has been loaded.
        for ( cgObjectNodeMap::iterator itNode = mObjectNodes.begin(); itNode != mObjectNodes.end(); ++itNode )
        {
            if ( !itNode->second->onNodeInit( cgUInt32IndexMap() ) )
                throw ResultException( cgString::format( _T("Unable to load cell data for scene %x because at least one if its nodes reported a failure during initialization. Refer to any previous errors for more information.\n"), getSceneId() ), cgDebugSource() );
        
        } // Next node

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : resetScene () (Virtual)
/// <summary>
/// Reload the transforms and cell ownership information for the nodes in the
/// scene as they exist in the current scene database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::resetScene( )
{
    // Disable database updates.
    bool oldWrites = isSceneWritingEnabled();
    enableSceneWrites( false );

    // Ask all nodes to reload their transformations
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mRootNodes.begin(); itNode != mRootNodes.end(); ++itNode )
        itNode->second->reloadTransforms( true );

    // Resolve any deferred node updates.
    resolvePendingUpdates();

    // Re-enable database updates
    enableSceneWrites( oldWrites );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : enableSceneWrites () (Virtual)
/// <summary>
/// Enable / disable the ability for nodes to serialize their transformation
/// updates to the scene database.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::enableSceneWrites( bool enable )
{
    mSceneWritesEnabled = enable;
}

//-----------------------------------------------------------------------------
//  Name : isSceneWritingEnabled () (Virtual)
/// <summary>
/// Are nodes allowed to serialize their transformation updates to the scene 
/// database?
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::isSceneWritingEnabled( ) const
{
    return mSceneWritesEnabled;
}

//-----------------------------------------------------------------------------
//  Name : updateObjectOwnership () (Virtual)
/// <summary>
/// Allows the various spatial (and scene) tree components to determine if it 
/// can / wants to own the specified object and optionally inserts it into the 
/// relevant cells / leaves.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::updateObjectOwnership( cgObjectNode * node )
{
    // Objects with a reference identifier of 0 (reserved, no scene) cannot
    // be owned. They are temporary internal objects that cannot / should
    // not be referenced.
    if ( !node->getReferenceId() )
        return;

    // If this is a root level object (has no parent) then we need
    // to determine if the object still exists within the same scene 
    // cell as it did previously. To do so, we retrieve the current world 
    // space position of the object node and use this to determine which
    // scene cell grid reference this falls into. If any update to the 
    // position pushes it outside of its current scene cell, then we need 
    // to adjust it. Note: Node parent cells are fixed if scene writing is
    // disabled. We do not want cells to be created and / or destroyed so
    // they remain relative to their original cells.
    if ( !node->getParent() && !mIsLoading && isSceneWritingEnabled() )
    {
        // Make a note of the cell in which the node currently exists.
        // We may need to clean up this cell data if it moves out.
        cgSceneCell * currentCell = node->getCell();

        // ToDo: 9999 - Stop creating scene cells at scene boundary.
        // ToDo: 9999 - Protect against Int16 overflow.
        // ToDo: 9999 - Should this use the object's PIVOT position?
        const cgVector3 & position = node->getPosition( false );
        const cgVector3 & cellSize = mSceneDescriptor.cellDimensions;
        cgSceneCellKey cellKey( (cgInt16)( (position.x / cellSize.x) + ((position.x >= 0) ? 0.5f : -0.5f) ), 
                            (cgInt16)( (position.y / cellSize.y) + ((position.y >= 0) ? 0.5f : -0.5f) ), 
                            (cgInt16)( (position.z / cellSize.z) + ((position.z >= 0) ? 0.5f : -0.5f) ) );

        // Does any cell exist at this location?
        cgSceneCellMap::iterator itCell = mCells.find( cellKey );
        if ( itCell == mCells.end() )
        {
            // ToDo: 9999 - If we're creating a new cell, and destroying an old one
            // then we may as well just update the cell's location rather than chewing
            // up another auto-increment rowId

            // Create a new cell at this location.
            cgSceneCell * newCell = new cgSceneCell( this, cellKey.cellX, cellKey.cellY, cellKey.cellZ );
            if ( newCell->insert( mWorld, getSceneId() ) == true )
            {
                // Insert into the scene cell map based on its location.
                mCells[ cellKey ] = newCell;

                // Swap node's cell.
                node->setCell( newCell, false );

            } // End if insert success
            else
            {
                delete newCell;
            
            } // End if failure

        } // End if no cell
        else
        {
            // If the node current position now exists within a
            // different cell, change it.
            cgSceneCell * cell = itCell->second;
            if ( cell != currentCell )
                node->setCell( cell, false );
            
        } // End if cell exists

        // If the cell in which the object previously exists
        // is now obsolete (because it is both empty and does
        // not define any custom properties) then destroy it.
        if ( currentCell && currentCell->isEmpty() )
        {
            // Retrieve necessary data and then remove from database and clean up.
            currentCell->getGridOffsets( cellKey.cellX, cellKey.cellY, cellKey.cellZ );
            currentCell->remove( mWorld, getSceneId() );
            currentCell->scriptSafeDispose();

            // Remove from in-memory grid map.
            mCells.erase( cellKey );

        } // End if remove cell

    } // End if no parent

    // Ask all root level spatial trees if they want to take ownership of this object.
    // Each tree will perform the task for each of their child trees (if any).
    bool found = false, previouslyManaged = (node->getSpatialTree() != CG_NULL);
    for ( cgObjectNodeSet::iterator itTree = mRootSpatialTrees.begin(); 
          itTree != mRootSpatialTrees.end() && !found; 
          ++itTree )
    {
        cgSpatialTreeNode * tree = (cgSpatialTreeNode*)(*itTree);
        
        // Skip self
        if ( tree == node )
            continue;

        // Test to see if this spatial tree (or one of its children) wants ownership
        if ( tree->updateObjectOwnership( node ) )
            found = true;

    } // Next Tree

    // If the object was not previously managed within a spatial
    // tree (i.e. an orphan), but a tree has now taken ownership
    // then remove it from the orphan list. Otherwise, add it
    // to the orphan list.
    if ( found && !previouslyManaged )
    {
        // Remove from orphan list
        mOrphanNodes.erase( node );

    } // End if newly managed
    else if ( !found && previouslyManaged )
    {
        // Add to orphan list
        node->setSpatialTree( CG_NULL );
        mOrphanNodes.insert( node );
    
    } // End if !managed
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Virtual)
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::computeVisibility( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags )
{
    // First register all orphan nodes.
    cgObjectNodeSet::iterator itNode;
    for ( itNode = mOrphanNodes.begin(); itNode != mOrphanNodes.end(); ++itNode )
    {
        // Register only if it passes simple bounding box test.
        cgObjectNode * node = (*itNode);
        if ( frustum.testAABB( node->getBoundingBox() ) )
            node->registerVisibility( visibilityData, flags );
    
    } // Next Orphan
    
    // Now request that all spatial trees compute their respective visibility.
    if ( mLandscape )
        mLandscape->computeVisibility( frustum, visibilityData, flags, CG_NULL );
    cgObjectNodeSet::iterator itTree;
    for ( itTree = mRootSpatialTrees.begin(); itTree != mRootSpatialTrees.end(); ++itTree )
    {
        cgSpatialTreeNode * tree = (cgSpatialTreeNode*)(*itTree);
        tree->computeVisibility( frustum, visibilityData, flags );
        
    } // Next Tree
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Virtual)
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::computeVisibility( const cgBoundingBox & bounds, cgVisibilitySet * visibilityData, cgUInt32 flags )
{
    // First register all orphan nodes.
    cgObjectNodeSet::iterator itNode;
    for ( itNode = mOrphanNodes.begin(); itNode != mOrphanNodes.end(); ++itNode )
    {
        // Register only if it passes simple bounding box test.
        cgObjectNode * node = (*itNode);
        if ( bounds.intersect( node->getBoundingBox() ) )
            node->registerVisibility( visibilityData, flags );
    
    } // Next Orphan

    // Now request that all spatial trees compute their respective visibility.
    cgObjectNodeSet::iterator itTree;
    for ( itTree = mRootSpatialTrees.begin(); itTree != mRootSpatialTrees.end(); ++itTree )
    {
        cgSpatialTreeNode * tree = (cgSpatialTreeNode*)(*itTree);
        tree->computeVisibility( bounds, visibilityData, flags );
        
    } // Next Tree
}

//-----------------------------------------------------------------------------
// Name : loadObjectNode() (Protected, Recursive)
/// <summary>
/// Recreate a previously existing node, loading it from the database based on
/// the specified data query. Can also recurse through any available child
/// hierarchy on request.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::loadObjectNode( cgUInt32 rootReferenceId, cgUInt32 referenceId, cgWorldQuery * nodeData, cgCloneMethod::Base cloneMethod, cgSceneCell * parentCell, cgObjectNode * parentNode, cgObjectNodeMap & loadedNodes, bool loadChildren )
{
    cgObjectNode * newNode = CG_NULL;

    // Scope locals
    {
        // Check to see if this node is already resident in *any* scene
        // (i.e. it could be a character in a different building).
        cgReference * reference = cgReferenceManager::getReference( referenceId );
        if ( reference )
        {
            // *Something* was already resident in memory with a matching reference identifier.
            // If the user isn't cloning, this is a failure for the entire hierarchy currently.
            if ( cloneMethod != cgCloneMethod::None )
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' into scene %i because part or all of its child hierarchy, beginning at object node '0x%x', was already resident in memory.\n"), rootReferenceId, getSceneId(), referenceId ), cgDebugSource() );

            // We're cloning, but we don't yet know what the reference we've found actually is. 
            // If it's not an object node, clearly  there is something wrong with the supplied 
            // reference identifier.
            if ( !reference->queryReferenceType( RTID_ObjectNode ) )
            {
                cgString identifierString = cgStringUtility::toString( reference->getReferenceType(), _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x'. A reference with the id '0x%x' was found in its hierarchy, but its type of '%s' did not match that expected for an object node.\n"), rootReferenceId, referenceId, identifierString.c_str() ), cgDebugSource() );
            
            } // End if conflicted

            // Our only option in this case is a clone.
            cgObjectNode * existingNode = (cgObjectNode*)reference;
            if ( !existingNode->clone( cloneMethod, this, true, newNode, cgTransform() ) )
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' using the specified cloning method. Refer to earlier errors for more information.\n"), referenceId ), cgDebugSource() );

        } // End if already resident
        else
        {
            // Find the type descriptor that corresponds to the specified local (world database) 
            // integer type identifier for this object type. This reverse lookup can be achieved
            // via the world's configuration object.
            cgUInt32 typeId = 0;
            nodeData->getColumn( _T("ObjectTypeId"), typeId );
            const cgWorldObjectTypeDesc * objectTypeDesc = mWorld->getConfiguration()->getObjectTypeByLocalId( typeId );
            if ( !objectTypeDesc )
            {
                // Silent fail. The object type was not recognized, but the user was notified 
                // about this during the initial world configuration import stage.
                return CG_NULL;

            } // End if unknown type
            
            // If the user will be ultimately cloning the node and (optionally) its data, we 
            // need to generate a new internal reference identifier for the new node itself.
            cgUInt32 finalRefId = ( cloneMethod == cgCloneMethod::None ) ? referenceId : mWorld->generateRefId( true );
            
            // Now create a new node / object instance to represent the data being loaded.
            if ( !(newNode = objectTypeDesc->nodeAllocNew( objectTypeDesc->globalIdentifier, finalRefId, this )) )
            {
                cgString identifierString = cgStringUtility::toString( objectTypeDesc->globalIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x'. Failed to create a physical node instance (or underlying world object instance) of type '%s'.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );
            
            } // End if 

            // Allow node to load its data from the provided world query. It should clone
            // its internal data appropriately based on the supplied cloning method.
            if ( !newNode->onNodeLoading( objectTypeDesc->globalIdentifier, nodeData, parentCell, cloneMethod ) )
            {
                newNode->deleteReference();
                cgString identifierString = cgStringUtility::toString( objectTypeDesc->globalIdentifier, _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x'. Failed to initialize physical node instance (or underlying world object instance) of type '%s'. Refer to previous errors for more information.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );
            
            } // End if failed

        } // End if not already resident

        // Add this node to the lookup table that maps *original* reference
        // identifiers to the newly created nodes (which may or may not have
        // the same id.)
        loadedNodes[ referenceId ] = newNode;

    } // End scope

    // If supplied, automatically re-attach to the referenced parent node.
    if ( parentNode )
        newNode->setParent( (cgObjectNode*)parentNode, true );

    // If node belonged to a group, re-attach where possible.
    {
        cgUInt32 groupId;
        if ( nodeData->getColumn( _T("OwnerGroup"), groupId ) && groupId )
        {
            // Search for it in the loaded node lookup table. Recall that
            // this contains nodes mapped against their ORIGINAL reference
            // identifier as it existed in the database, not their new ids. 
            cgObjectNodeMap::iterator itGroup = loadedNodes.find( groupId );
            if ( itGroup != loadedNodes.end() )
                newNode->setOwnerGroup( (cgGroupNode*)itGroup->second );

        } // End if has owner group

    } // End scope

    // Add the new node to main node list (contains all scene nodes)
    mObjectNodes[ newNode->getReferenceId() ] = newNode;

    // Add to the orphan node list initially as necessary.
    if ( !newNode->getSpatialTree() )
        mOrphanNodes.insert( newNode );

    // Add as a root level node in the node hierarchy unless
    // it already exists or already has a parent.
    if ( !newNode->getParent() )
    {
        mRootNodes[ newNode->getReferenceId() ] = newNode;

        // If this is a spatial tree, we need to keep track of these too for
        // the purposes of visibility computation and object ownership management.
        if ( newNode->queryReferenceType( RTID_SpatialTreeNode ) )
            mRootSpatialTrees.insert( newNode );
    
    } // End if no parent

    // Add the node to the relevant update bucket if required
    cgUpdateRate::Base updateRate = newNode->getUpdateRate();
    if ( updateRate != cgUpdateRate::Never )
        mUpdateBuckets[ updateRate ].nodes.push_back( newNode );

    // If it was requested that we load children, do so now.
    if ( loadChildren )
    {
        static const cgString queryString = _T("SELECT * FROM 'nodes' WHERE ParentRefId=?1");
        cgWorldQuery query( mWorld, queryString );
        query.bindParameter( 1, referenceId );
        if ( !query.step() )
        {
            cgString error;
            query.getLastError( error );
            throw ResultException( cgString::format( _T("Unable to load children of object node '0x%x' from the world database. Error: %s"), referenceId, error.c_str() ), cgDebugSource() );
        
        } // End if failed

        // Process any children discovered.
        while ( query.nextRow() )
        {
            // Get the original reference identifier of the node.
            cgUInt32 childRefId;
            query.getColumn( _T("RefId"), childRefId );

            // Recurse into child.
            loadObjectNode( rootReferenceId, childRefId, &query, cloneMethod, parentCell, newNode, loadedNodes, true );

        } // Next row

    } // End if load children

    // Success!
    return newNode;

}

//-----------------------------------------------------------------------------
// Name : loadObjectNode()
/// <summary>
/// Recreate a previously existing node, loading it from the database based on
/// the specified reference identifier. The node did not originally have to
/// belong to this scene, and can optionally be cloned into a new node (with a
/// new internal identifier) if required. This is essentially the primary
/// method by which the application 'spawn in' new entities.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::loadObjectNode( cgUInt32 referenceId, cgCloneMethod::Base cloneMethod, bool loadChildren )
{
    cgToDo( "Spawning", "This method currently always creates internal nodes when cloning." )

    cgObjectNodeMap loadedNodes;
    cgObjectNode * newNode = CG_NULL;

    // Begin loading the node data
    try
    {
        // Check to see if this node is already resident in *any* scene
        // (i.e. it could be a character in a different building).
        cgReference * reference = cgReferenceManager::getReference( referenceId );
        if ( reference )
        {
            // *Something* was already resident in memory with a matching reference
            // identifier but we don't yet know what it is. If it's not an object node,
            // clearly there is something wrong with the supplied identifier.
            if ( !reference->queryReferenceType( RTID_ObjectNode ) )
            {
                cgString identifierString = cgStringUtility::toString( reference->getReferenceType(), _T("B") );
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x'. A reference with the specified identifier was found to exist but its type of '%s' did not match that expected for an object node.\n"), referenceId, identifierString.c_str() ), cgDebugSource() );
            
            } // End if conflicted

            // An *object node* with this identifier already exists in memory. It may or 
            // may not exist in this scene however so we need to test for that next.
            cgObjectNode * existingNode = (cgObjectNode*)reference;
            if ( existingNode->getScene() == this )
            {
                // The node existed in this scene to begin with. If the user opted 
                // not to clone the node, we can just return the existing reference.
                if ( cloneMethod == cgCloneMethod::None )
                    return existingNode;

            } // End if this scene
            else
            {
                // The node existed in some other scene. If the user opted not to
                // clone the node then the loading operation must fail.
                if ( cloneMethod == cgCloneMethod::None )
                    throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' into scene %i because it is currently resident in scene %i.\n"), referenceId, getSceneId(), existingNode->getScene()->getSceneId() ), cgDebugSource() );

            } // End if other scene

            cgToDo( "Spawning Logic", "Clone entire hierarchy if requested. Currently only the top level node is cloned if it is already resident in memory." );

            // The only remaining case is the case where the user requested a clone
            // of an already resident node. This method is available whether the node
            // currently exists in this scene or not.
            if ( !existingNode->clone( cloneMethod, this, true, newNode, cgTransform() ) )
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' using the specified cloning method. Refer to earlier errors for more information.\n"), referenceId ), cgDebugSource() );

            // Add this node to the lookup table that maps *original* reference
            // identifiers to the newly created nodes (which may or may not have
            // the same id.)
            loadedNodes[ referenceId ] = newNode;

        } // End if already resident
        else
        {
            // Mark scene as loading (automatically disables serialization in loading nodes).
            mIsLoading = true;

            // Find the object node with this reference identifier.
            static const cgString queryString = _T("SELECT * FROM 'nodes' WHERE RefId=?1");
            cgWorldQuery query( mWorld, queryString );
            query.bindParameter( 1, referenceId );
            if ( !query.step() )
            {
                cgString error;
                query.getLastError( error );
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' from the world database. Error: %s"), referenceId, error.c_str() ), cgDebugSource() );
            
            } // End if failed
            if ( !query.nextRow() )
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' because it was not found in the world database.\n"), referenceId ), cgDebugSource() );

            // Load the object node.
            newNode = loadObjectNode( referenceId, referenceId, &query, cloneMethod, CG_NULL, CG_NULL, loadedNodes, true );

            // Loading is complete.
            mIsLoading = false;
        
        } // End if not already resident

        // Process all loaded nodes and allow them to initialize. We provide a 'remap'
        // dictionary for cases where node reference identifiers may have been different
        // to those in the original database.
        cgUInt32IndexMap referenceRemap;
        if ( cloneMethod != cgCloneMethod::None )
        {
            for ( cgObjectNodeMap::iterator itNode = loadedNodes.begin(); itNode != loadedNodes.end(); ++itNode )
                referenceRemap[itNode->first] = itNode->second->getReferenceId();
        
        } // End if cloning
        for ( cgObjectNodeMap::iterator itNode = loadedNodes.begin(); itNode != loadedNodes.end(); ++itNode )
        {
            if ( !itNode->second->onNodeInit( referenceRemap ) )
                throw ResultException( cgString::format( _T("Unable to load requested object node '0x%x' because it reported a failure during initialization. Refer to any previous errors for more information.\n"), referenceId ), cgDebugSource() );
        
        } // Next node

    } // End try

    catch ( const ResultException & e )
    {
        // Unset loading state
        mIsLoading = false;

        // Remap original reference identifiers to final values so that
        // we can be sure that clean-up is successful.
        cgObjectNodeMap cleanupNodes;
        for ( cgObjectNodeMap::iterator itNode = loadedNodes.begin(); itNode != loadedNodes.end(); ++itNode )
            cleanupNodes[itNode->second->getReferenceId()] = itNode->second;

        // Destroy nodes that were loaded so far.
        deleteObjectNodes( cleanupNodes );

        // Print thrown error and bail
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return CG_NULL;
    
    } // End catch

    // Success!
    return newNode;
}

//-----------------------------------------------------------------------------
// Name : unloadObjectNode ()
/// <summary>
/// Call in order to remove the specified node from the in-memory scene
/// but /not/ from the scene database.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::unloadObjectNode( cgObjectNode * node )
{
    // Skip if it cannot be deleted
    if ( !node )
        return;

    // Allow the node to resolve any final pending updates in order
    // to fully serialize its data into its final state, or to update
    // its children, before removal.
    node->resolvePendingUpdates( cgDeferredUpdateFlags::All );

    // Remove from sandbox node list(s) if required.
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        // De-select.
        if ( node->isSelected() )
        {
            mSelectedNodes.erase( node->getReferenceId() );
            mSelectedNodesOrdered.erase( node->mSelectionId );
    
        } // End if selected

        // ToDo: 9999 - When these are serialized, we probably shouldn't do this
        // quite so blindly and instead should have the set function in a database
        // aware fashion
        
        // Remove from any active selection sets.
        SelectionSetMap::iterator itSet;
        for ( itSet = mSelectionSets.begin(); itSet != mSelectionSets.end(); )
        {
            // Retrieve the set and then immediately increment the iterator
            // (the selection set map may be adjusted if nothing remains in the set).
            cgSelectionSet * selectionSet = itSet->second;
            ++itSet;

            // Find the identifier associated with the set if valid.
            cgSelectionSet::RefIdIndexMap::iterator itRef = selectionSet->nodeLUT.find( node->getReferenceId() );
            if ( itRef != selectionSet->nodeLUT.end() )
            {
                cgUInt32 referenceId = itRef->second;
                selectionSet->nodeLUT.erase( itRef );
                selectionSet->orderedNodes.erase( referenceId );
            
            } // End if contained

            // Nothing left in set?
            if ( selectionSet->orderedNodes.empty() == true )
                removeSelectionSet( selectionSet->name );

        } // Next Set

    } // End if sandbox

    // Remove node from orphan list if it existed there.
    mOrphanNodes.erase( node );

    // Remove the object from the appropriate update bucket.
    cgObjectNodeList * bucket = &mUpdateBuckets[ node->mUpdateRate ].nodes;
    for ( cgObjectNodeList::iterator itBucketNode = bucket->begin(); itBucketNode != bucket->end(); ++itBucketNode )
    {
        if ( *itBucketNode == node )
        {
            bucket->erase( itBucketNode );
            break;
        
        } // End if matches

    } // Next Node

    // If this node exists at the root level in the hierarchy,
    // remove it from the root node list first of all.
    if ( node->getParent() == CG_NULL )
    {
        mRootNodes.erase( node->getReferenceId() );
        mRootSpatialTrees.erase( node );
    
    } // End if no parent

    // Remove from the scene. Will automatically be removed
    // from name usage map when node is disposed.
    mObjectNodes.erase( node->getReferenceId() );

    // Now dispose of the node from memory.
    node->scriptSafeDispose();
}

//-----------------------------------------------------------------------------
// Name : unloadObjectNodes ()
/// <summary>
/// Call in order to remove a whole list of nodes from the in-memory scene
/// in one go, but /not/ from the scene database.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::unloadObjectNodes( cgObjectNodeMap & nodes )
{
    // Duplicate the node list. If the specified list should be
    // modified during deletion, this will prevent correct iteration.
    cgObjectNodeMap processedNodes( nodes );

    // Iterate through each node to remove
    cgObjectNodeMap::iterator itNode;
    for ( itNode = processedNodes.begin(); itNode != processedNodes.end(); ++itNode )
    {
        // Skip if the node was deleted elsewhere. For instance, a deleted
        // node might also destroy its target object automatically or a group
        // may destroy itself when there are no more child objects.
        if ( cgReferenceManager::isValidReference( itNode->second ) == false )
            continue;

        // Allow the node to resolve any final pending updates in order
        // to fully serialize its data into its final state, or to update
        // its children, before removal.
        cgObjectNode * node = itNode->second;
        node->resolvePendingUpdates( cgDeferredUpdateFlags::All );
        
        // Remove from sandbox node list(s) if required.
        if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
        {
            // De-select.
            if ( node->isSelected() )
            {
                mSelectedNodes.erase( node->getReferenceId() );
                mSelectedNodesOrdered.erase( node->mSelectionId );
            
            } // End if selected

            // ToDo: 9999 - When these are serialized, we probably shouldn't do this
            // quite so blindly and instead should have the set function in a database
            // aware fashion

            // Remove from any active selection sets.
            SelectionSetMap::iterator itSet;
            for ( itSet = mSelectionSets.begin(); itSet != mSelectionSets.end(); )
            {
                // Retrieve the set and then immediately increment the iterator
                // (the selection set map may be adjusted if nothing remains in the set).
                cgSelectionSet * selectionSet = itSet->second;
                ++itSet;

                // Find the identifier associated with the set if valid.
                cgSelectionSet::RefIdIndexMap::iterator itRef = selectionSet->nodeLUT.find( node->getReferenceId() );
                if ( itRef != selectionSet->nodeLUT.end() )
                {
                    cgUInt32 referenceId = itRef->second;
                    selectionSet->nodeLUT.erase( itRef );
                    selectionSet->orderedNodes.erase( referenceId );
                
                } // End if contained

                // Nothing left in set?
                if ( selectionSet->orderedNodes.empty() == true )
                    removeSelectionSet( selectionSet->name );

            } // Next Set

        } // End if sandbox

        // Remove node from orphan list if it existed there.
        mOrphanNodes.erase( node );

        // Remove the object from the appropriate update bucket.
        cgObjectNodeList * bucket = &mUpdateBuckets[ node->mUpdateRate ].nodes;
        for ( cgObjectNodeList::iterator itBucketNode = bucket->begin(); itBucketNode != bucket->end(); ++itBucketNode )
        {
            if ( *itBucketNode == node )
            {
                bucket->erase( itBucketNode );
                break;
            
            } // End if matches

        } // Next Node

        // If this node exists at the root level in the hierarchy,
        // remove it from the root node list first of all.
        if ( node->getParent() == CG_NULL )
        {
            mRootNodes.erase( node->getReferenceId() );
            mRootSpatialTrees.erase( node );
        
        } // End if no parent

        // Remove from the scene. Will automatically be removed
        // from name usage map when node is disposed.
        mObjectNodes.erase( node->getReferenceId() );

        // Now dispose of the node from memory.
        node->scriptSafeDispose();
        node = CG_NULL;
            
    } // Next Node
}

//-----------------------------------------------------------------------------
// Name : importLandscape()
/// <summary>
/// Generate a new scene landscape based on the import parameters specified.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscape * cgScene::importLandscape( const cgLandscapeImportParams & params )
{
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::importLandscape() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return CG_NULL;
    
    } // End if !sandbox

    // Destroy any existing landscape
    if ( mLandscape )
    {
        // ToDo: 9999 - Delete the landscape from the database
        mLandscape->scriptSafeDispose();
    
    } // End if exists
    mLandscape = NULL;

    // Allocate a new landscape.
    mLandscape = new cgLandscape( this );
    if ( !mLandscape->import( params ) )
    {
        mLandscape->scriptSafeDispose();
        mLandscape = CG_NULL;
        return CG_NULL;
    
    } // End if failed

    // Update the scene's landscape reference identifier.
    if ( getSceneId() != 0 && mLandscape->getDatabaseId() != 0 )
    {
        prepareQueries();
        mUpdateLandscapeRef.bindParameter( 1, mLandscape->getDatabaseId() );
        mUpdateLandscapeRef.bindParameter( 2, getSceneId() );
        
        // Execute
        if ( mUpdateLandscapeRef.step( true ) == false )
        {
            cgString error;
            mUpdateLandscapeRef.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update landscape reference identifier for scene '0x%x' during import process. Error: %s\n"), getSceneId(), error.c_str() );
            return false;
        
        } // End if failed
    
    } // End if serialize

    // Success!
    return mLandscape;
}

//-----------------------------------------------------------------------------
// Name : createObjectNode()
/// <summary>
/// Create a new node of the specified type and add it to the scene's
/// node database.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::createObjectNode( bool internalNode, const cgUID & objectTypeIdentifier, bool autoAssignName )
{
    return createObjectNode( internalNode, objectTypeIdentifier, autoAssignName, cgCloneMethod::None, CG_NULL, cgTransform() );
}

//-----------------------------------------------------------------------------
// Name : createObjectNode()
/// <summary>
/// Create a new node of the specified type and add it to the scene's
/// node database. The new node should represent a clone of the specified
/// initializing node, cloned using the provided method.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::createObjectNode( bool internalNode, const cgUID & objectTypeIdentifier, bool autoAssignName, cgCloneMethod::Base cloneMethod, cgObjectNode * nodeInit, const cgTransform & initTransform )
{
    cgObjectNode * newNode = CG_NULL;

    // If the engine is not in sandbox mode, or the scene is internal, all new nodes are internal.
    if ( (cgGetSandboxMode() != cgSandboxMode::Enabled) || !getSceneId() )
        internalNode = true;
    
    // Find the data associated with the underlying element type for this object.
    // Note: We use the main static 'mRegisteredObjectTypes' map for node
    // creation rather than the world's active list (which is only populated
    // when relevant /objects/ are created for the first time).
    const cgWorldObjectTypeDesc::Map & types = cgWorldObject::getRegisteredTypes();
    cgWorldObjectTypeDesc::Map::const_iterator itType = types.find( objectTypeIdentifier );
    if ( itType == types.end() )
        return CG_NULL;
    const cgWorldObjectTypeDesc & type = itType->second;
    
    // Catch exceptions
    try
    {
        // Ensure we can roll back on failure.
        if ( !internalNode )
            mWorld->beginTransaction( _T("createObjectNode") );

        // Generate a new identifier for this node.
        cgUInt32 referenceId = mWorld->generateRefId( internalNode );

        // Create new instance (with relevant constructor parameters)
        if ( cloneMethod == cgCloneMethod::None )
        {
            // No clone, just allocate a new node.
            if ( type.nodeAllocNew )
                newNode = type.nodeAllocNew( type.globalIdentifier, referenceId, this );
        
        } // End if no clone
        else if ( type.nodeAllocClone )
        {
            // Clone based on the specified node.
            newNode = type.nodeAllocClone( type.globalIdentifier, referenceId, this, nodeInit, cloneMethod, initTransform );
        
        } // End if clone

        // Success?
        if ( !newNode )
        {
            cgString strIdentifier = cgStringUtility::toString( objectTypeIdentifier, _T("B") );
            throw ResultException( cgString::format( _T("Unable to create new world object / node. Failed to create instance for type '%s'."), strIdentifier.c_str() ), cgDebugSource() );
        
        } // End if failed

        // Allow node to create any necessary data / object references.
        if ( !newNode->onNodeCreated( objectTypeIdentifier, cloneMethod ) )
        {
            // Destroy the node.
            newNode->deleteReference();

            // Throw
            cgString strIdentifier = cgStringUtility::toString( objectTypeIdentifier, _T("B") );
            throw ResultException( cgString::format( _T("Failed to initialize new object node of type '%s'."), strIdentifier.c_str() ), cgDebugSource() );
        
        } // End if failed

        // Generate a unique name for this node if requested.
        if ( autoAssignName )
            newNode->setName( makeUniqueName( type.name, 1 ) );

        // Commit the changes.
        if ( !internalNode )
            mWorld->commitTransaction( L"createObjectNode" );

    } // End try

    catch ( const ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        if ( !internalNode )
            mWorld->rollbackTransaction( L"createObjectNode" );
        return CG_NULL;
    
    } // End catch
    
    // Add to main node list (contains all scene nodes)
    mObjectNodes[ newNode->getReferenceId() ] = newNode;

    // Add to the orphan node list initially as necessary.
    if ( !newNode->getSpatialTree() )
        mOrphanNodes.insert( newNode );

    // Add as a root level node in the node hierarchy unless
    // it already exists or already has a parent.
    if ( !newNode->getParent() )
    {
        mRootNodes[ newNode->getReferenceId() ] = newNode;

        // If this is a spatial tree, we need to keep track of these too for
        // the purposes of visibility computation and object ownership management.
        if ( newNode->queryReferenceType( RTID_SpatialTreeNode ) )
            mRootSpatialTrees.insert( newNode );
    
    } // End if no parent

    // Add the node to the relevant update bucket if required
    cgUpdateRate::Base updateRate = newNode->getUpdateRate();
    if ( updateRate != cgUpdateRate::Never )
        mUpdateBuckets[ updateRate ].nodes.push_back( newNode );
    
    // Automatically resolve any information which was not initially computed.
    newNode->resolvePendingUpdates( cgDeferredUpdateFlags::All );
    
    // Scene has been modified
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        setDirty( true );

    // Notify whoever is interested that the scene was modified
    onNodeAdded( &cgNodeUpdatedEventArgs( this, newNode ) );

    // ToDo: 9999 - Original Framework
    /*// Insert into the main scene object list.
    m_Objects.push_back( object );

    // Add to the root object list ONLY if it has no parent.
    if ( object->getParent() == CG_NULL )
        m_RootObjects.push_back( object );

    // Also add to the lookup table that indicates if an object exists.
    m_ValidObjectTable.insert( object );
    
    // Also insert into group sorted object list
    m_ObjectTypes[ object->GetObjectType() ].push_back( object );

    // And finally, the name sorted object map
    m_NamedObjects[ object->GetInstanceName() ] = object;

    // Insert us into the spatial tree if required
    if ( m_pSpatialTree != NULL )
        m_pSpatialTree->updateObjectOwnership( object );*/
    
    // Success?
    return newNode;
}

//-----------------------------------------------------------------------------
//  Name : addController ()
/// <summary>
/// Add a new scene controller that, when enabled, will be triggered in
/// the scene's update call.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::addController( cgSceneController * controller )
{
    mSceneControllers.push_back( controller );
}

//-----------------------------------------------------------------------------
//  Name : unload ()
/// <summary>
/// Unload this scene from the parent manager with a convenient method.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::unload()
{
    // Pass up to manager if available
    if ( mWorld != CG_NULL )
        mWorld->unloadScene( this );
}

//-----------------------------------------------------------------------------
//  Name : isLoading ()
/// <summary>
/// Determine if the scene is currently in the process of loading.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::isLoading( ) const
{
    return mIsLoading;
}

//-----------------------------------------------------------------------------
//  Name : getPhysicsWorld ()
/// <summary>
/// Retrieve the scene's internal physics simulation world.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsWorld * cgScene::getPhysicsWorld( ) const
{
    return mPhysicsWorld;
}

//-----------------------------------------------------------------------------
//  Name : getLightingManager ()
/// <summary>
/// Retrieve the scene's internal lighting manager.
/// </summary>
//-----------------------------------------------------------------------------
cgLightingManager * cgScene::getLightingManager( ) const
{
    return mLightingManager;
}

//-----------------------------------------------------------------------------
//  Name : getActiveCamera ()
/// <summary>
/// Retrieve the currently active scene camera object.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode * cgScene::getActiveCamera( ) const
{
    return mActiveCamera;
}

//-----------------------------------------------------------------------------
//  Name : getSceneDescriptor ()
/// <summary>
/// Retrieve the full descriptor for the scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneDescriptor * cgScene::getSceneDescriptor( ) const
{
    return &mSceneDescriptor;
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Simply retrieve the render driver through which the scene will be
/// rendered.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgScene::getRenderDriver( ) const
{
    // Scenes always currently use the main singleton render driver.
    return cgRenderDriver::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : getResourceManager ()
/// <summary>
/// Simply retrieve the resource manager that will manage the scene's
/// resources.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager * cgScene::getResourceManager( ) const
{
    // Scenes always currently use the main singleton resource manager.
    return cgResourceManager::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : getLandscape ()
/// <summary>
/// Simply retrieve any landscape associated with this scene.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscape * cgScene::getLandscape( ) const
{
    return mLandscape;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Simply retrieve the name of this loaded scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgScene::getName( ) const
{
    return mSceneDescriptor.name;
}

//-----------------------------------------------------------------------------
//  Name : getSceneId ()
/// <summary>
/// Retrieve the unique integer identifier associated with this scene.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgScene::getSceneId( ) const
{
    return mSceneDescriptor.sceneId;
}

//-----------------------------------------------------------------------------
//  Name : getDistanceDisplayUnits ()
/// <summary>
/// Retrieve the unit type that the artist has selected for displaying distance
/// values.
/// </summary>
//-----------------------------------------------------------------------------
cgUnitType::Base cgScene::getDistanceDisplayUnits( ) const
{
    return mSceneDescriptor.distanceDisplayUnits;
}

//-----------------------------------------------------------------------------
//  Name : setActiveCamera ()
/// <summary>
/// Set the currently active camera.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::setActiveCamera( cgCameraNode * pCamera )
{
    mActiveCamera = pCamera;

    // Success!
    return true;
}

// ToDo: To be replaced with 'CreateNode' / 'LoadNode'
/*//-----------------------------------------------------------------------------
//  Name : AddObject ()
/// <summary>
/// Add the specified object to the scene.
/// Note : Should not ideally be called explicitly unless you can guarantee
/// object lifetime (scene will take ownership).
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::AddObject( cgSceneObject * object )
{
    cgSceneObjectList::iterator itObject;

    // This exact object already exists?
    if ( SceneObjectExists( object ) == true )
        return true;

    // Something with the same instance name already exists?
    if ( m_NamedObjects.find( object->GetInstanceName() ) != m_NamedObjects.end() ) 
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("An object with name '%s' already exists in the object database for this scene. Object instance names must be unique.\n"), object->GetInstanceName().c_str() );
        return false;
    
    } // End if already exists

    // Insert into the main scene object list.
    m_Objects.push_back( object );

    // Add to the root object list ONLY if it has no parent.
    if ( object->getParent() == CG_NULL )
        m_RootObjects.push_back( object );

    // Also add to the lookup table that indicates if an object exists.
    m_ValidObjectTable.insert( object );
    
    // Also insert into group sorted object list
    m_ObjectTypes[ object->GetObjectType() ].push_back( object );

    // And finally, the name sorted object map
    m_NamedObjects[ object->GetInstanceName() ] = object;

    // Insert us into the spatial tree if required
    if ( m_pSpatialTree != NULL )
        m_pSpatialTree->updateObjectOwnership( object );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : RemoveObject ()
/// <summary>
/// Remove the specified object from the scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::RemoveObject( cgSceneObject * object )
{
    // Validate requirements
    if ( object == CG_NULL )
        return;

    // Remove the tree object from the spatial tree
    object->setSpatialTree( CG_NULL );
    
    // Remove from all lists
    m_Objects.remove( object );
    m_RootObjects.remove( object );
    m_ValidObjectTable.erase( object );
    m_ObjectTypes[ object->GetObjectType() ].remove( object );
    m_NamedObjects.erase( object->GetInstanceName() );

    // Set the update rate to 'Never' to remove it from any active
    // update list.
    setObjectUpdateRate( object, cgUpdateRate::Never );
}*/

//-----------------------------------------------------------------------------
//  Name : addRootNode ()
/// <summary>
/// Add the specified object node to the scene as a root node. This is 
/// simply a list that outlines all of the nodes that exist at a root 
/// level in the hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::addRootNode( cgObjectNode * node )
{
    if ( mRootNodes.find( node->getReferenceId() ) == mRootNodes.end() )
    {
        mRootNodes[ node->getReferenceId() ] = node;
        setDirty( true );

        // If this is a spatial tree, we need to keep track of these too for
        // the purposes of visibility computation and object ownership management.
        if ( node->queryReferenceType( RTID_SpatialTreeNode ) == true )
            mRootSpatialTrees.insert( node );
    
    } // End if not found
}

//-----------------------------------------------------------------------------
//  Name : removeRootNode ()
/// <summary>
/// Remove the specified node from the scene's root node list.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::removeRootNode( cgObjectNode * node )
{
    cgObjectNodeMap::iterator itNode = mRootNodes.find( node->getReferenceId() );
    if ( itNode != mRootNodes.end() )
    {
        mRootNodes.erase( itNode );
        setDirty( true );
    
    } // End if found

    // Remove from the root spatial tree list used for visibility
    // computation and object ownership management.
    if ( node->queryReferenceType( RTID_SpatialTreeNode ) == true )
        mRootSpatialTrees.erase( node );
}

// ToDo: Fix Me
/*//-----------------------------------------------------------------------------
//  Name : GetNodesByType ()
/// <summary>
/// Returns a list of objects matching input cgUID type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::GetObjectsByType( cgUID ObjectType, cgSceneObjectList **ppObjects )
{
	// Validate output parameter
	if ( !ppObjects ) return false;

	// Search for the object list matching the UID
    ObjectTypeMap::iterator itObject = m_ObjectTypes.find( ObjectType );
    
	// If a valid list was found, return it.
	if( itObject != m_ObjectTypes.end( ) )
	{
        // Return the object list
		*ppObjects = &(itObject->second);
		return true;
	
    } // End if found type

	// No matches. 
	return false;
}

//-----------------------------------------------------------------------------
//  Name : GetObjectsByDistance ()
/// <summary>
/// Returns a list of objects with the specified distance.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::GetObjectsByDistance( cgSceneObjectList & objects, const cgVector3 & vecPos, cgFloat distance )
{
    //ISpatialTree::LeafList::iterator    itLeaf;
    //ISpatialTree::LeafList              Leaves;
    //ITreeLeaf::TreeObjectList::iterator itObject;
    //ITreeLeaf::TreeObjectList           objects;
    bool                                bFoundObjects = false;

    // ToDo : Complete this

    // Validate requirements
    if ( !m_pSpatialTree )
        return false;

    // Generate a bounding box for retrieving data from the spatial tree
    cgBoundingBox AABB( vecPos.x - distance, vecPos.y - distance, vecPos.z - distance,
                        vecPos.x + distance, vecPos.y + distance, vecPos.z + distance );

    // Retrieve a list of all leaves in the vicinity
    m_pSpatialTree->CollectLeavesAABB( Leaves, AABB );

    // Loop through all leaves in the list
    for ( itLeaf = Leaves.begin(); itLeaf != Leaves.end(); ++itLeaf )
    {
        ITreeLeaf * pLeaf = *itLeaf;
        if ( !pLeaf ) continue;

        // Retrieve objects in this leaf
        objects = pLeaf->GetTreeObjectList();

        // Loop through all objects
        for ( itObject = objects.begin(); itObject != objects.end(); ++itObject )
        {
            ISpatialTree::TreeObject * pTreeObject = *itObject;
            if ( !pTreeObject ) continue;

            // Is this a scene object?
            if ( pTreeObject->type != ISpatialTree::TreeObject::GameObject ) continue;

            // If this is not already in the list
            if ( std::find( ObjectList.begin(), ObjectList.end(), (cgSceneObject*)pTreeObject->pContext ) != ObjectList.end() ) continue;

            // Within the specified distance?
            sqVector3 vecObjectPos = ((cgSceneObject*)pTreeObject->pContext)->getPosition();
            if ( (vecObjectPos - vecPos).SquareLength() > (distance * distance) ) continue;

            // Add it to the list
            ObjectList.push_back( (cgSceneObject*)pTreeObject->pContext );
            bFoundObjects = true;

        } // Next Object

    } // Next leaf

    // Did we find any objects?
    return bFoundObjects;
}

//-----------------------------------------------------------------------------
//  Name : GetObjectInBounds ()
/// <summary>
/// Returns a list of objects with the specified bounding box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::GetObjectsInBounds( cgSceneObjectList & objects, const cgBoundingBox & AABB )
{
    //ISpatialTree::LeafList::iterator    itLeaf;
    //ISpatialTree::LeafList              Leaves;
    //ITreeLeaf::TreeObjectList::iterator itObject;
    //ITreeLeaf::TreeObjectList           objects;
    bool                                bFoundObjects = false;

    // ToDo : Complete this.

    // Validate requirements
    if ( !m_pSpatialTree )
        return false;

    // Retrieve a list of all leaves in the vicinity
    m_pSpatialTree->CollectLeavesAABB( Leaves, AABB );

    // Loop through all leaves in the list
    for ( itLeaf = Leaves.begin(); itLeaf != Leaves.end(); ++itLeaf )
    {
        ITreeLeaf * pLeaf = *itLeaf;
        if ( !pLeaf ) continue;

        // Retrieve objects in this leaf
        objects = pLeaf->GetTreeObjectList();

        // Loop through all objects
        for ( itObject = objects.begin(); itObject != objects.end(); ++itObject )
        {
            ISpatialTree::TreeObject * pTreeObject = *itObject;
            if ( !pTreeObject ) continue;

            // Is this a scene object?
            if ( pTreeObject->type != ISpatialTree::TreeObject::GameObject ) continue;

            // If this is not already in the list
            if ( std::find( ObjectList.begin(), ObjectList.end(), (cgSceneObject*)pTreeObject->pContext ) != ObjectList.end() ) continue;

            // Bounding boxes intersect?
            sqBounds3 AABBObject = ((cgSceneObject*)pTreeObject->pContext)->getBoundingBox( );
            if ( !cgCollision::AABBIntersectAABB( AABB, AABBObject ) ) continue;

            // Add it to the list
            ObjectList.push_back( (cgSceneObject*)pTreeObject->pContext );
            bFoundObjects = true;

        } // Next Object

    } // Next leaf

    // Did we find any objects?
    return bFoundObjects;
}*/

//-----------------------------------------------------------------------------
//  Name : setObjectUpdateRate ()
/// <summary>
/// Each update list has a different rate that can be used in order
/// to reduce the amount of nodes which potentially have to be updated
/// each frame. This function simply adds the node to the relevant 
/// internal update list based on the specified rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::setObjectUpdateRate( cgObjectNode * node, cgUpdateRate::Base rate )
{
    // Is this a no-op?
    if ( node == NULL || rate == node->mUpdateRate )
        return;

    // First remove the object from its old list.
    cgObjectNodeList::iterator itNode;
    cgObjectNodeList * bucket = &mUpdateBuckets[ node->mUpdateRate ].nodes;
    for ( itNode = bucket->begin(); itNode != bucket->end(); ++itNode )
    {
        if ( *itNode == node )
        {
            bucket->erase( itNode );
            break;
        
        } // End if matches
    } // Next Node
    
    // Update the node's internal rate record
    node->mUpdateRate = rate;
    node->serializeUpdateRate();

    // Add the node to the new update list if required
    if ( rate != cgUpdateRate::Never )
    {
        bucket = &mUpdateBuckets[ node->mUpdateRate ].nodes;
        bucket->push_back( node );
    
    } // End if not the 'never' interval
}

//-----------------------------------------------------------------------------
//  Name : getObjectNodes ()
/// <summary>
/// Retrieve a complete list of all scene object nodes.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeMap & cgScene::getObjectNodes( )
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : GetSceneNodes () (const modifier)
/// <summary>
/// Retrieve a complete list of all scene object nodes.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeMap & cgScene::getObjectNodes( ) const
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : getSceneCells () (const modifier)
/// <summary>
/// Retrieve a complete list of all allocated scene cells.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneCellMap & cgScene::getSceneCells( ) const
{
    return mCells;
}

//-----------------------------------------------------------------------------
//  Name : getCellSize ()
/// <summary>
/// Retrieve the specified size of each individual cell used for scene
/// management.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgScene::getCellSize( ) const
{
    return mSceneDescriptor.cellDimensions;
}

//-----------------------------------------------------------------------------
//  Name : getParentWorld ( )
/// <summary>
/// Retrieve the world to which this scene belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld * cgScene::getParentWorld( ) const
{
    return mWorld;
}

//-----------------------------------------------------------------------------
//  Name : getSelectedAABB ()
/// <summary>
/// Retrieve a world space axis aligned bounding box for each of the currently
/// selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::getSelectedAABB( cgBoundingBox & bounds, cgFloat growAmount )
{
    // Bail if nothing is selected
    if ( mSelectedNodes.empty() == true )
        return false;

    // Initialize the bounding box output values ready for growing
    // as we encounter each selected node.
    bounds.reset();
    
    // Iterate through each selected node
    bool boundsGenerated = false;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        // Retrieve the node ready to retrieve its bounding box
        cgObjectNode * node = itNode->second;
        if ( node == CG_NULL )
            continue;

        // Skip if the node belongs to a closed group. The group's bounding
        // box (also automatically selected) will contain all the necessary information .
        if ( node->isMergedAsGroup() == true )
            continue;
        
        // Retrieve the world space bounding box of this node. Skip
        // if it was not fully populated.
        cgBoundingBox nodeBounds = node->getBoundingBox( );
        if ( nodeBounds.isPopulated() == false )
            continue;
        
        // Grow the output bounding box values
        bounds.addPoint( nodeBounds.min );
        bounds.addPoint( nodeBounds.max );
        boundsGenerated = true;
        
    } // Next Node

    // If no bounding boxes were available, but nodes WERE selected
    // just return a degenerate bounding box.
    if ( !boundsGenerated )
    {
        bounds = cgBoundingBox::Empty;
    
    } // End if no bounds available
    else
    {
        // If requested, inflate the bounding box by the specified amount.
        bounds.inflate( growAmount );
        
    } // End if bounds available

    // nodes selected!
    return true;
}

//-----------------------------------------------------------------------------
// Name : getSelectedPivot ()
/// <summary>
/// Retrieve a world space point about which selected nodes should pivot
/// during rotation for instance. If one node is select, this will be its
/// origin, otherwise it will be the average of all selected node origins.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::getSelectedPivot( cgVector3 & pivotOut ) const
{
    return getObjectNodesPivot( mSelectedNodes, pivotOut );
}

//-----------------------------------------------------------------------------
// Name : getObjectNodesPivot ()
/// <summary>
/// Retrieve a world space point about which specified nodes should pivot
/// during rotation for instance. If one node is select, this will be its
/// origin, otherwise it will be the average of all selected node origins.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::getObjectNodesPivot( const cgObjectNodeMap & nodes, cgVector3 & pivotOut ) const
{
    cgVector3 summedPivots( 0, 0, 0 );
    cgInt     originCount = 0;
    
    // Bail if nothing is selected
    if ( nodes.empty() )
        return false;

    // Iterate through each selected node and sum their origins
    cgObjectNodeMap::const_iterator itNode;
    for ( itNode = nodes.begin(); itNode != nodes.end(); ++itNode )
    {
        // Skip if the node belongs to a closed group. The group's bounding
        // box (also automatically selected) will contain all the necessary 
        // information. Note: It is currently the caller's responsibility to 
        // ensure that any owner group is also in the supplied node list.
        cgObjectNode * node = itNode->second;
        if ( node->isMergedAsGroup() == true )
            continue;

        // Sum pivot of all nodes.
        summedPivots += node->getPosition();
        originCount++;

    } // Next Node

    // No average available?
    if ( originCount == 0 )
        return false;

    // Return the summed average.
    pivotOut = summedPivots * (1.0f / (cgFloat)originCount);
    return true;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Allows the scene to perform its update pass prior to rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::update( )
{
    cgFloat  timeDelta   = cgTimer::getInstance()->getTimeElapsed();
    cgDouble currentTime = cgTimer::getInstance()->getTime();

    // Begin profiling scene update method.
    cgProfiler * profiler = cgProfiler::getInstance();
    profiler->beginProcess( cgString::format( _T("Scene update [0x%x]"), getSceneId() ) );

    // Allow any enabled scene controllers to update.
    for ( size_t i = 0; i < mSceneControllers.size(); ++i )
    {
        cgSceneController * controller = mSceneControllers[i];
        if ( controller != CG_NULL && controller->isControllerEnabled() == true )
            controller->update( timeDelta );
    
    } // Next Controller
    
    // Allow the physics world to update if enabled.
    if ( mPhysicsWorld && mDynamicsEnabled )
        mPhysicsWorld->update( timeDelta );

    // Allow scene nodes to update. First iterate through the 'Always Update' list.
    cgObjectNodeList::iterator itNode;
    cgObjectNodeList * bucketNodes = &mUpdateBuckets[ cgUpdateRate::Always ].nodes;
    for ( itNode = bucketNodes->begin(); itNode != bucketNodes->end(); )
    {
        // Increment as we go as the 'update' call may delete the node
        cgObjectNode * node = *itNode++;
        
        // Trigger the node's update process
        node->update( timeDelta );    

    } // Next scene node

    // Now iterate through all other update rate buckets
    for ( cgUInt32 i = cgUpdateRate::FPS1; i < cgUpdateRate::Count; ++i )
    {
        UpdateBucket * bucket = &mUpdateBuckets[i];

        // If this is the first update, we should trigger it
        if ( bucket->lastUpdateTime < 0 )
            bucket->lastUpdateTime = currentTime;
        if ( bucket->nextUpdateTime < 0 )
            bucket->nextUpdateTime = currentTime;

        // Are we due to execute the update process for this?
        if ( currentTime >= bucket->nextUpdateTime )
        {
            cgFloat finalDelta = std::min<cgFloat>(1.0f,(cgFloat)(currentTime - bucket->lastUpdateTime));

            // Step through the list and update
            bucketNodes = &mUpdateBuckets[i].nodes;
            for ( itNode = bucketNodes->begin(); itNode != bucketNodes->end(); )
            {
                // Increment as we go as the 'update' call may delete the node
                cgObjectNode * node = *itNode++;
                
                // Trigger the node's update process
                node->update( finalDelta );

            } // Next scene object

            // Just for housekeeping purposes, record the last time an update was run
            bucket->lastUpdateTime = currentTime;

            // Schedule the next update of this list
            switch ( i )
            {
                case cgUpdateRate::FPS1:
                    bucket->nextUpdateTime += 1.0;              // 1.0 / 1.0;
                    break;
                case cgUpdateRate::FPS2:
                    bucket->nextUpdateTime += 0.5;              // 1.0 / 2.0;
                    break;
                case cgUpdateRate::FPS5:
                    bucket->nextUpdateTime += 0.2;              // 1.0 / 5.0;
                    break;
                case cgUpdateRate::FPS10:
                    bucket->nextUpdateTime += 0.1;              // 1.0 / 10.0;
                    break;
                case cgUpdateRate::FPS15:
                    bucket->nextUpdateTime += 0.0666666666;     // 1.0 / 15.0;
                    break;
                case cgUpdateRate::FPS30:
                    bucket->nextUpdateTime += 0.0333333333;     // 1.0 / 30.0;
                    break;
                case cgUpdateRate::FPS60:
                    bucket->nextUpdateTime += 0.0166666666;     // 1.0 / 60.0;
                    break;
                case cgUpdateRate::FPS100:
                    bucket->nextUpdateTime += 0.001;            // 1.0 / 100.0;
                    break;
                case cgUpdateRate::FPS120:
                    bucket->nextUpdateTime += 0.0083333333;     // 1.0 / 120.0;
                    break;
                case cgUpdateRate::NTSC:                        // 29.97fps
                    bucket->nextUpdateTime += 0.0333667000;     // 1.0 / 29.97;
                    break;
                case cgUpdateRate::PAL:                         // 25fps
                    bucket->nextUpdateTime += 0.04;             // 1.0 / 25.0;
                    break;

            } // End Switch Interval Type

        } // End if time to update nodes

    } // Next interval item

    // Update audio driver 3D listener with current camera position.
    cgAudioDriver * audioDriver = cgAudioDriver::getInstance();
    if ( mActiveCamera && audioDriver )
        audioDriver->set3DListenerTransform( mActiveCamera->getWorldTransform() );

    // Resolve any deferred node updates.
    resolvePendingUpdates();
    
    // End profiling scene update method.
    profiler->endProcess( );
}

//-----------------------------------------------------------------------------
//  Name : resolvePendingUpdates ()
/// <summary>
/// Resolve all pending node updates that opted to defer the computation of
/// certain information until the entire update process was complete.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::resolvePendingUpdates( )
{
    // Keep pulling from the front of the map until it is empty (the resolution 
    // process will remove each node as it is processed).
    while ( !mPendingUpdates.empty() )
        (*mPendingUpdates.begin())->resolvePendingUpdates( cgDeferredUpdateFlags::All );
}

//-----------------------------------------------------------------------------
//  Name : queueNodeUpdates ()
/// <summary>
/// Called internally when an object node has been updated in order to queue
/// that node for additional processing in cases where certain update processes
/// need to be deferred until a later time. An example might be the deferral
/// of child transform updates until all nodes in a hierarchy have been 
/// processed (such as during the animation of a character's full bone 
/// hierarchy).
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::queueNodeUpdates( cgObjectNode * node )
{
    // ToDo: Automatically remove from queue when a node is deleted / unloaded.
    mPendingUpdates.insert( node );
}

//-----------------------------------------------------------------------------
//  Name : resolvedNodeUpdates ()
/// <summary>
/// Specified node's pending updates have been resolved and it can be removed
/// from the pending update set.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::resolvedNodeUpdates( cgObjectNode * node )
{
    mPendingUpdates.erase( node );
}

//-----------------------------------------------------------------------------
//  Name : enableDynamics ()
/// <summary>
/// Set the state which determines whether scene dynamics processing (physics 
/// updates) are currently enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::enableDynamics( bool enabled )
{
    mDynamicsEnabled = enabled;
}

//-----------------------------------------------------------------------------
//  Name : isDynamicsEnabled ()
/// <summary>
/// Get the state which determines whether scene dynamics processing (physics 
/// updates) are currently enabled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::isDynamicsEnabled( ) const
{
    return mDynamicsEnabled;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Allows the scene to perform the rendering of an individual scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::render( )
{
    // Validate requirements
    if ( !mActiveCamera || !mScriptObject )
        return;

    // Access required systems / values
    cgRenderDriver * driver   = getRenderDriver();
    //cgProfiler     * profiler = cgProfiler::getInstance();

    // Begin profiling scene render method.
    //profiler->beginProcess( cgString::format( _T("Scene Render [0x%x]"), getSceneId() ) );
    
    // Set currently active camera / transform states
    cgCameraNode * oldCamera = driver->getCamera();
    driver->setWorldTransform( CG_NULL );
    driver->setCamera( mActiveCamera );

    // Compute the visibility set for the active camera and apply it initially.
    // Make sure that it collects materials from the objects for runtime batching.
    //profiler->beginProcess( _T("Visibility") );
    //profiler->beginProcess( _T("Camera") );
    mActiveCamera->computeVisibility( cgVisibilitySearchFlags::MustRender | cgVisibilitySearchFlags::CollectMaterials, true );
    //profiler->endProcess( );

    // Compute level of detail settings for each visible object.
    //profiler->beginProcess( _T("LoD Computation") );
    cgVisibilitySet * visibilityData = mActiveCamera->getVisibilitySet();
    cgObjectNodeArray & visibleLights = visibilityData->getVisibleLights();
    cgObjectNodeArray & visibleObjects = visibilityData->getVisibleObjects();
    for ( size_t i = 0; i < visibleObjects.size(); ++i )
        visibleObjects[i]->computeLevelOfDetail( mActiveCamera );
    for ( size_t i = 0; i < visibleLights.size(); ++i )
        visibleLights[i]->computeLevelOfDetail( mActiveCamera );
    //profiler->endProcess( );

    // Also allow light sources to compute visibility as they deem necessary
    // (this will largely only be the case if they are shadow casters)
    //profiler->beginProcess( _T("Light Sources") );
    for ( size_t i = 0; i < visibleLights.size(); ++i )
        ((cgLightNode*)visibleLights[i])->computeVisibility();
    //profiler->endProcess( );

    /*// Combine sets to produce a list of lights that influence each object.
    //profiler->beginProcess( _T("Illumation Set") );
    selectionSet->GenerateIlluminationSet();
    //profiler->endProcess( );*/

    // Update the lighting manager. This will include updating any indirect
	// lighting data that is needed for the frame
    // ToDo: 6767 - Reintroduce.
	//mLightingManager->update( mActiveCamera );

    // Notify the script that it should render.
    try
    {
        cgToDo( "Carbon General", "When reloading scripts, the method handle needs to be reset." );
        
        // Generate script argument array and cache method handle during first time population.
        static cgFloat timeDelta;
        if ( !mOnSceneRenderMethod )
        {
            mOnSceneRenderArgs.clear();
            mOnSceneRenderArgs.push_back( cgScriptArgument( cgScriptArgumentType::Float, _T("float"), &timeDelta ) );
            mOnSceneRenderArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object, _T("RenderView@+"), NULL ) );
            mOnSceneRenderMethod = mScriptObject->getMethodHandle( _T("void onSceneRender( float, RenderView@+ )") );

        } // End if first time

        // Populate argument array.
        timeDelta = cgTimer::getInstance()->getTimeElapsed();
        mOnSceneRenderArgs[1].data = driver->getActiveRenderView();

        // Execute script method.
        //profiler->beginProcess( _T("Render Script") );
        mScriptObject->executeMethod( mOnSceneRenderMethod, mOnSceneRenderArgs );
        //profiler->endProcess( );
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute onSceneRender() method in '%s'. The engine reported the following error: %s.\n"), e.getExceptionSource().c_str(), e.description.c_str() );
        driver->setCamera( oldCamera );
        //profiler->endProcess( );
    
    } // End catch exception

    // Retore prior camera
    driver->setCamera( oldCamera );

    // Begin profiling scene render method.
    //profiler->endProcess( );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow all nodes to render their 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::sandboxRender( bool wireframe, const cgPlane & gridPlane )
{
    // Validate requirements
    if ( mActiveCamera == CG_NULL )
        return;

    // Access required systems / values
    cgRenderDriver * driver = getRenderDriver();
    
    // Set currently active camera / transform states
    cgCameraNode * oldCamera = driver->getCamera();
    driver->setWorldTransform( CG_NULL );
    driver->setCamera( mActiveCamera );

    // Compute the visibility set containing /all/ objects, not just
    // the traditionally 'renderable' ones.
    mActiveCamera->computeVisibility( 0, true );

    // ToDo: 9999
    /*// Add all orphan nodes to the visibility set if there is no scene tree (default).
    if ( m_oSceneTree == nullptr )
        oVis->AddNodes( mOrphanNodes );*/

    // Begin sandbox render pass.
    beginRenderPass( _T("Sandbox") );

    // Visit each node and ask them to draw.
    cgVisibilitySet * visibilityData = mActiveCamera->getVisibilitySet();
    cgObjectNodeArray & visibleObjects = visibilityData->getVisibleObjects();
    cgObjectNodeArray & visibleLights  = visibilityData->getVisibleLights();
    for ( size_t i = 0; i < visibleObjects.size(); ++i )
        visibleObjects[i]->sandboxRender( mActiveCamera, visibilityData, wireframe, gridPlane );
    for ( size_t i = 0; i < visibleLights.size(); ++i )
        visibleLights[i]->sandboxRender( mActiveCamera, visibilityData, wireframe, gridPlane );

    // End sandbox render pass.
    endRenderPass( );

    // Retore prior camera
    driver->setCamera( oldCamera );
}

//-----------------------------------------------------------------------------
//  Name : beginRenderPass ()
/// <summary>
/// Called in order to set up the rendering subsystem ready for the
/// caller to take a single draw pass over the scene. Once complete,
/// the caller should call 'endRenderPass()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::beginRenderPass( const cgString & passName )
{
    // Update the driver's current pass name in order to allow
    // for subsequent automatic effect technique selection
    // to occur.
    cgRenderDriver * driver = getRenderDriver();
    driver->pushRenderPass( passName );

    // We're rendering!
    mPassBegun = true;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endRenderPass ()
/// <summary>
/// Called once the current rendering pass has been completed.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::endRenderPass( )
{
    // Erroneous call?
    if ( mPassBegun == false )
        return;

    // Restore the previous driver render pass name (if any).
    cgRenderDriver * driver = getRenderDriver();
    driver->popRenderPass();

    // We've finished rendering.
    mPassBegun = false;
}

//-----------------------------------------------------------------------------
// Name : setActiveElementType( )
/// <summary>
/// The sandbox environment is manipulating the specified element type 
/// (i.e. object sub-element category). This property can be queried by objects
/// and other scene elements to determine if they should be considered the
/// 'active' element type. Set to 'cgUID::Empty' to disable this.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::setActiveElementType( const cgUID & typeIdentifier )
{
    mActiveElementType = typeIdentifier;
}

//-----------------------------------------------------------------------------
// Name : getActiveElementType( )
/// <summary>
/// Get the unique identifier of the element type / category which the 
/// application / sandbox environment considers 'active'. An identifier of 
/// cgUID::Empty means that standard top level nodes are considered 'active'.
/// </summary>
//-----------------------------------------------------------------------------
const cgUID & cgScene::getActiveElementType( ) const
{
    return mActiveElementType;
}

//-----------------------------------------------------------------------------
// Name : createSelectionSet ( )
/// <summary>
/// Create a selection set from the current list of selected nodes. This
/// entire set of nodes can then be re-selected later in a single step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::createSelectionSet( const cgString & name, bool internalSet, bool overwrite )
{
    // Validate requirements
    if ( name.empty() || mSelectedNodesOrdered.empty() )
        return false;

    // Set already exists with this name?
    SelectionSetMap::iterator itSet;
    cgString searchKey = cgString::toLower( name );
    if ( (itSet = mSelectionSets.find( searchKey )) != mSelectionSets.end() )
    {
        if ( overwrite == true )
            mSelectionSets.erase( itSet );
        else
            return false;
    
    } // End if exists

    // Create a new selection set entry from those nodes currently selected.
    cgSelectionSet * selectionSet = new cgSelectionSet();
    selectionSet->name        = name;
    selectionSet->internalSet = internalSet;
    
    // Build the node tables so that nodes can be rapidly searched, etc.
    cgUInt32 i = 0;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodesOrdered.begin(); itNode != mSelectedNodesOrdered.end(); ++itNode, ++i )
    {
        cgObjectNode * node = itNode->second;
        selectionSet->orderedNodes[ i ] = node;
        selectionSet->nodeLUT[ node->getReferenceId() ] = i;
    
    } // Next Node

    // Store using name as key.
    mSelectionSets[ searchKey ] = selectionSet;

    // Scene has been modified
    setDirty( true );

    // Notify whoever is listening that we created a new selection set
    onSelectionSetAdded( &cgSelectionSetEventArgs( this, selectionSet ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : removeSelectionSet ( )
/// <summary>
/// Remove a previously created selection set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::removeSelectionSet( const cgString & name )
{
    // Retrieve the specified selection set
    SelectionSetMap::iterator itSet;
    cgString strKey = cgString::toLower(name);
    if ( (itSet = mSelectionSets.find( strKey )) == mSelectionSets.end() )
        return false;
    
    // Retrieve pointer to set and remove from our dictionary.
    cgSelectionSet * selectionSet = itSet->second;
    mSelectionSets.erase( itSet );

    // Scene has been modified
    setDirty( true );

    // Notify whoever is listening that we're removing this selection set
    onSelectionSetRemoved( &cgSelectionSetEventArgs( this, selectionSet ) );

    // Finally, clean up allocated memory.
    delete selectionSet;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : applySelectionSet ( )
/// <summary>
/// Apply the specified selection set, selecting all referenced nodes.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::applySelectionSet( const cgString & name, bool clearCurrent )
{
    // Retrieve the specified selection set
    SelectionSetMap::iterator itSet;
    cgString searchKey = cgString::toLower( name );
    if ( (itSet = mSelectionSets.find( searchKey )) == mSelectionSets.end() )
        return false;

    // Select the nodes.
    cgSelectionSet * selectionSet = itSet->second;
    selectNodes( selectionSet->orderedNodes, clearCurrent );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : getNextSelectionId ( )
/// <summary>
/// Retrieve the next consecutive integer identifier that can be used to
/// determine the order in which nodes were selected.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgScene::getNextSelectionId( )
{
    return mNextSelectionId++;
}

//-----------------------------------------------------------------------------
// Name : getSelectedNodes ( )
/// <summary>
/// Retrieve a list of the currently selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeMap & cgScene::getSelectedNodes( )
{
    return mSelectedNodes;
}

//-----------------------------------------------------------------------------
// Name : getSelectedNodesOrdered ( )
/// <summary>
/// Retrieve a list of the currently selected nodes in the order in which
/// they were selected.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeMap & cgScene::getSelectedNodesOrdered( )
{
    return mSelectedNodesOrdered;
}

//-----------------------------------------------------------------------------
// Name : getSelectionSets ( )
/// <summary>
/// Retrieve a list of the currently available selection sets.
/// </summary>
//-----------------------------------------------------------------------------
const cgScene::SelectionSetMap & cgScene::getSelectionSets( ) const
{
    return mSelectionSets;
}

//-----------------------------------------------------------------------------
// Name : clearSelection ( )
/// <summary>
/// Simply clear out the selected nodes list.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::clearSelection( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::clearSelection() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Set the selected status of each node to false.
    // Note: We specify 'false' to the second parameter of
    // 'SetSelect()' which prevents the scene's internal selection
    // lists from being updated and instead only sets the node's flag.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        // Retrieve the node and deselect
        cgObjectNode * node = itNode->second;
        if ( node )
            node->setSelected( false, false );
        
    } // Next Node
    mSelectedNodes.clear();
    mSelectedNodesOrdered.clear();
   
    // Notify whoever is listening that we cleared the current selection.
    onSelectionCleared( &cgSceneEventArgs( this ) );
}

//-----------------------------------------------------------------------------
// Name : selectAllNodes ( )
/// <summary>
/// Simply select all nodes contained in the currently open scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::selectAllNodes( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::selectAllNodes() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    selectNodes( mObjectNodes, false );
}

//-----------------------------------------------------------------------------
// Name : selectNodes ( )
/// <summary>
/// Select a specific collection of nodes.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::selectNodes( cgObjectNodeMap & nodes, bool replaceSelection )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::selectNodes() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return false;
    
    } // End if !sandbox

    // Deselect all prior nodes if requested.
    if ( replaceSelection )
    {
        cgObjectNodeMap::iterator itNode;
        for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
        {
            cgObjectNode * node = itNode->second;
            
            // Mark as de-selected without notifying the scene.
            if ( node && !node->isDisposed() )
                node->setSelected( false, false );

        } // Next Node

        // Clear selection lists
        mSelectedNodes.clear();
        mSelectedNodesOrdered.clear();

    } // End if replace selection

    // Iterate through all referenced nodes and select them.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = nodes.begin(); itNode != nodes.end(); )
    {
        cgObjectNode * node = itNode->second;
        
        // Increment iterator immediately in case list is modified.
        ++itNode;

        // Validate
        if ( node == CG_NULL || node->isDisposed() == true || node->isSelected() == true )
            continue;

        // Mark as selected without notifying the scene.
        node->setSelected( true, false );
        node->mSelectionId = mNextSelectionId++;
        mSelectedNodes[ node->getReferenceId() ] = node;
        mSelectedNodesOrdered[ node->mSelectionId ] = node;

    } // Next Node

    // Notify whoever is listening that we're applying this selection set
    onSelectionUpdated( &cgSelectionUpdatedEventArgs( this, &nodes ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : cloneSelected ( )
/// <summary>
/// Clone the current selection and create new copies, references or instances.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::cloneSelected( cgCloneMethod::Base method, cgObjectNodeMap & nodes, bool internalNode )
{
    return cloneSelected( method, nodes, internalNode, 1, cgTransform(), cgOperationSpace::World );
}

//-----------------------------------------------------------------------------
// Name : cloneSelected ( )
/// <summary>
/// Clone the current selection and create new copies, references or instances.
/// Each new clone will be transformed based on the delta matrix specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::cloneSelected( cgCloneMethod::Base method, cgObjectNodeMap & nodes, bool internalNode, cgUInt32 cloneCount, const cgTransform & transformDelta, cgOperationSpace::Base transformSpace )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::cloneSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return false;
    
    } // End if !sandbox

    // Wrap the removal in a transaction to speed up the process and
    // to automatically handle failure.
    mWorld->beginTransaction( _T("cloneSelected") );

    // Duplicate the node list. If the specified list should be
    // modified during cloning, this will prevent correct iteration.
    cgObjectNodeMap ProcessedNodes( mSelectedNodes );
    
    // Process each of the currently selected nodes.
    cgUInt32 index = 0;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = ProcessedNodes.begin(); itNode != ProcessedNodes.end(); ++itNode )
    {
        // Apply first delta offset.
        cgObjectNode * node = itNode->second;
        cgTransform currentTransform = node->getWorldTransform();
        if ( transformSpace == cgOperationSpace::World )
        {
            currentTransform *= transformDelta;
        
        } // End if world
        else if ( transformSpace == cgOperationSpace::Local )
        {
            // Remove any scaling from the matrix before we 
            // apply the transformation.
            cgVector3 scale = currentTransform.localScale();
            currentTransform.setLocalScale( 1, 1, 1 );
            
            // Apply transformation
            currentTransform = transformDelta * currentTransform;
            
            // Re-apply scale.
            currentTransform.setLocalScale( scale );
            
        } // End if local

        // Create the requested number of copies.
        for ( cgUInt32 i = 0; i < cloneCount; ++i )
        {
            // Create a new cloned node
            cgObjectNode * newNode = CG_NULL;
            if ( node->clone( method, this, internalNode, newNode, currentTransform ) == true )
            {
                cgString baseName = cgStringUtility::stripTrailingNumbers( cgString::trim( node->getName() ) );
                newNode->setName( makeUniqueName( baseName ) );
                
                // Add to the output list of cloned nodes
                nodes[ index++ ] = newNode;
            
                // Offset current matrix by delta transform
                // Apply first delta offset.
                if ( transformSpace == cgOperationSpace::World )
                {
                    currentTransform *= transformDelta;
                
                } // End if world
                else if ( transformSpace == cgOperationSpace::Local )
                {
                    // Remove any scaling from the matrix before we 
                    // apply the transformation.
                    cgVector3 scale = currentTransform.localScale();
                    currentTransform.setLocalScale( 0, 0, 0 );
                    
                    // Apply transformation
                    currentTransform = transformDelta * currentTransform;
                    
                    // Re-apply scale.
                    currentTransform.setLocalScale( scale );

                } // End if local

            } // End if Success

        } // Next Copy

    } // Next Node

    // Commit changes.
    mWorld->commitTransaction( _T("cloneSelected") );

    // NB: isDirty update automatically handled by createObjectNode() during the clone.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : applyMaterialToSelected ( )
/// <summary>
/// Apply the specified material to all selected mesh nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::applyMaterialToSelected( cgMaterial * material )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::applyMaterialToSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // We cannot apply if this is not an 'active' material for this scene.
    if ( !isActiveMaterial( material ) )
        return;

    // Speed up modification by wrapping in a transaction.
    mWorld->beginTransaction( _T("applyMaterialToSelected") );

    // Process each of the currently selected nodes.
    bool modified = false;
    std::set<cgMeshHandle> appliedMeshes;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;
        
        // Is a mesh?
        if ( node->queryObjectType( RTID_MeshObject ) == true )
        {
            cgMeshObject * object = (cgMeshObject*)node->getReferencedObject();
            
            // Have we already applied to this mesh in cases where the mesh data is
            // referenced by multiple objects?
            cgMeshHandle meshData = object->getMesh();
            if ( appliedMeshes.find( meshData ) != appliedMeshes.end() )
                continue;

            // Replace the material.
            modified |= object->setMeshMaterial( material );

            // Mark as applied
            appliedMeshes.insert( meshData );

        } // End if mesh node

    } // Next Node
    appliedMeshes.clear();

    // Commit changes.
    mWorld->commitTransaction( _T("applyMaterialToSelected") );

    // Scene now dirty?
    if ( modified )
        setDirty( true );
}

//-----------------------------------------------------------------------------
// Name : applySceneRescale ( )
/// <summary>
/// Rescale all elements in the scene by the desired amount at the local / data
/// level. For instance, a light source will scale its range properties and a
/// mesh will rescale all of its vertices.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::applySceneRescale( cgFloat scale )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::applySceneRescale() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    cgToDo( "Carbon General", "Note: this feature is not yet fully complete. In some cases, nodes that exist in one scene may represent references / instances of objects that are *also* used in other scenes in the world. In this case, the 'applySceneRescale()' method must clone the object (once -- that single clone will replace every instance of that object in that specific scene). This behavior is not currently implemented." );
    cgToDo( "Carbon General", "Needs to eventually rescale cells and cell sizes too." );

    // Build a list of all of the unique objects that exist
    // in the scene (actual objects, not nodes).
    cgWorldObjectSet objects;
    for ( cgObjectNodeMap::iterator itNode = mObjectNodes.begin(); itNode != mObjectNodes.end(); ++itNode )
        objects.insert( itNode->second->getReferencedObject() );

    // Wrap all changes in a transaction for efficiency, and in case we need to roll back.
    mWorld->beginTransaction( _T("applyObjectRescale") );

    // Request that each object rescales.
    for ( cgWorldObjectSet::iterator itObject = objects.begin(); itObject != objects.end(); ++itObject )
        (*itObject)->applyObjectRescale( scale );

    // Reposition all the objects in the scene accordingly.
    for ( cgObjectNodeMap::iterator itNode = mObjectNodes.begin(); itNode != mObjectNodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;

        // Do *not* allow child nodes to be affected. They will be visited
        // in their own right.
        node->setTransformMethod( cgTransformMethod::NoChildUpdate );

        // Apply a position only scale
        node->scale( scale, scale, scale, true );

        // Switch back to standard transform method.
        node->setTransformMethod( cgTransformMethod::Standard );
    
    } // Next Node

    // Commit changes
    mWorld->commitTransaction( _T("applyObjectRescale") );
    
    // Scene is now dirty
    setDirty( true );
}

//-----------------------------------------------------------------------------
// Name : resetSelectedScale ( )
/// <summary>
/// Reset the scale transforms of all selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::resetSelectedScale( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::resetSelectedScale() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Process each of the currently selected nodes.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;
        node->resetScale();
        
    } // Next Node

    // Scene now dirty.
    setDirty( true );
}

//-----------------------------------------------------------------------------
// Name : resetSelectedOrientation ( )
/// <summary>
/// Reset the orientation transforms of all selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::resetSelectedOrientation( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::resetSelectedOrientation() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Process each of the currently selected nodes.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;
        node->resetOrientation();
        
    } // Next Node

    // Scene now dirty.
    setDirty( true );
}

//-----------------------------------------------------------------------------
// Name : resetSelectedPivot ( )
/// <summary>
/// Reset the pivot point of all selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::resetSelectedPivot( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::resetSelectedPivot() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Process each of the currently selected nodes.
    cgObjectNodeMap::iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;
        node->resetPivot();
        
    } // Next Node

    // Scene now dirty.
    setDirty( true );
}

//-----------------------------------------------------------------------------
// Name : deleteSelected ( )
/// <summary>
/// Delete any currently selected nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::deleteSelected( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::deleteSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Delete all selected nodes.
    deleteObjectNodes( mSelectedNodes );

    // Clear the current selection
    clearSelection();

    // Notify whoever is listening that we deleted the current selection
    onDeleteSelection( &cgSceneEventArgs( this ) );
}

//-----------------------------------------------------------------------------
// Name : deleteObjectNode ()
/// <summary>
/// Call in order to delete a single specified node from the scene database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::deleteObjectNode( cgObjectNode * node )
{
    // Reflect through to 'unloadObjectNode' if we are not in full sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        unloadObjectNode( node );
        return true;
    
    } // End if !sandbox

    // Skip if it cannot be deleted
    if ( !node || !node->canDelete() )
        return false;

    // Allow the node to resolve any final pending updates in order
    // to fully serialize its data into its final state, or to update
    // its children, before removal.
    node->resolvePendingUpdates( cgDeferredUpdateFlags::All );

    // Allow us to roll back this specific removal on failure.
    bool serialize = ((cgGetSandboxMode() == cgSandboxMode::Enabled) && !node->isInternalReference());
    if ( serialize )
        mWorld->beginTransaction( _T("deleteObjectNode") );

    // Instruct node that it is being physically destroyed
    // and should be removed from the database or perform
    // any necessary clean up.
    if ( !node->onNodeDeleted( ) )
    {
        if ( serialize )
            mWorld->rollbackTransaction( _T("deleteObjectNode") );
        return false;
    
    } // End if failed

    // Commit changes.
    if ( serialize )
        mWorld->commitTransaction( _T("deleteObjectNode") );

    // Remove from sandbox node list(s) if required.
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        // De-select.
        if ( node->isSelected() )
        {
            mSelectedNodes.erase( node->getReferenceId() );
            mSelectedNodesOrdered.erase( node->mSelectionId );
    
        } // End if selected

        // Remove from any active selection sets.
        SelectionSetMap::iterator itSet;
        for ( itSet = mSelectionSets.begin(); itSet != mSelectionSets.end(); )
        {
            // Retrieve the set and then immediately increment the iterator
            // (the selection set map may be adjusted if nothing remains in the set).
            cgSelectionSet * selectionSet = itSet->second;
            ++itSet;

            // Find the identifier associated with the set if valid.
            cgSelectionSet::RefIdIndexMap::iterator itRef = selectionSet->nodeLUT.find( node->getReferenceId() );
            if ( itRef != selectionSet->nodeLUT.end() )
            {
                cgUInt32 referenceId = itRef->second;
                selectionSet->nodeLUT.erase( itRef );
                selectionSet->orderedNodes.erase( referenceId );
            
            } // End if contained

            // Nothing left in set?
            if ( selectionSet->orderedNodes.empty() )
                removeSelectionSet( selectionSet->name );

        } // Next Set

        // Scene has been modified
        setDirty( true );

    } // End if sandbox

    // Remove node from orphan list if it existed there.
    mOrphanNodes.erase( node );

    // Remove the object from the appropriate update bucketNodes.
    cgObjectNodeList * bucketNodes = &mUpdateBuckets[ node->mUpdateRate ].nodes;
    for ( cgObjectNodeList::iterator itBucketNode = bucketNodes->begin(); itBucketNode != bucketNodes->end(); ++itBucketNode )
    {
        if ( *itBucketNode == node )
        {
            bucketNodes->erase( itBucketNode );
            break;
        
        } // End if matches

    } // Next Node

    // If this node exists at the root level in the hierarchy,
    // remove it from the root node list first of all.
    if ( node->getParent() == CG_NULL )
    {
        mRootNodes.erase( node->getReferenceId() );
        mRootSpatialTrees.erase( node );
    
    } // End if no parent

    // Remove from the scene. Will automatically be removed
    // from name usage map when node is disposed.
    mObjectNodes.erase( node->getReferenceId() );

    // Notify whoever is listening that the specified nodes were removed.
    onNodeDeleted( &cgNodeUpdatedEventArgs( this, node ) );

    // Now dispose of the node from memory.
    node->scriptSafeDispose();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : deleteObjectNodes ()
/// <summary>
/// Call in order to delete a whole list of specified nodes from the scene 
/// database in one go.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::deleteObjectNodes( cgObjectNodeMap & nodes )
{
    // Reflect through to 'unloadObjectNodes' if we are not in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
    {
        unloadObjectNodes( nodes );
        return true;
    
    } // End if !sandbox

    // Wrap the removal in a transaction to speed up the process and
    // to automatically handle failure.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        mWorld->beginTransaction( _T("deleteObjectNodes") );

    // Duplicate the node list. If the specified list should be
    // modified during deletion, this will prevent correct iteration.
    cgObjectNodeMap processedNodes( nodes );

    // Iterate through each node to remove
    cgObjectNodeMap::iterator itNode;
    for ( itNode = processedNodes.begin(); itNode != processedNodes.end(); ++itNode )
    {
        // Skip if the node was deleted elsewhere. For instance, a deleted
        // node might also destroy its target object automatically or a group
        // may destroy itself when there are no more child objects.
        if ( !cgReferenceManager::isValidReference( itNode->second ) )
            continue;
        
        // Skip if it cannot be deleted
        cgObjectNode * node = itNode->second;
        if ( !node->canDelete() )
            continue;

        // Allow the node to resolve any final pending updates in order
        // to fully serialize its data into its final state, or to update
        // its children, before removal.
        node->resolvePendingUpdates( cgDeferredUpdateFlags::All );

        // Allow us to roll back this specific removal on failure.
        if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && !node->isInternalReference() )
        {
            mWorld->beginTransaction( _T("deleteObjectNodesInner") );

            // Instruct node that it is being physically destroyed
            // and should be removed from the database or perform
            // any necessary clean up.
            if ( !node->onNodeDeleted( ) )
            {
                mWorld->rollbackTransaction( _T("deleteObjectNodesInner") );
                continue;
            
            } // End if failed

            // We're done.
            mWorld->commitTransaction( _T("deleteObjectNodesInner") );

        } // End if !internal
        else
        {
            // Instruct node that it is being physically destroyed
            // and should be removed from the database or perform
            // any necessary clean up.
            if ( !node->onNodeDeleted( ) )
                continue;
            
        } // End if internal

        // Remove from sandbox node list(s) if required.
        if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
        {
            // De-select.
            if ( node->isSelected() )
            {
                mSelectedNodes.erase( node->getReferenceId() );
                mSelectedNodesOrdered.erase( node->mSelectionId );
            
            } // End if selected

            // Remove from any active selection sets.
            SelectionSetMap::iterator itSet;
            for ( itSet = mSelectionSets.begin(); itSet != mSelectionSets.end(); )
            {
                // Retrieve the set and then immediately increment the iterator
                // (the selection set map may be adjusted if nothing remains in the set).
                cgSelectionSet * selectionSet = itSet->second;
                ++itSet;

                // Find the identifier associated with the set if valid.
                cgSelectionSet::RefIdIndexMap::iterator itRef = selectionSet->nodeLUT.find( node->getReferenceId() );
                if ( itRef != selectionSet->nodeLUT.end() )
                {
                    cgUInt32 referenceId = itRef->second;
                    selectionSet->nodeLUT.erase( itRef );
                    selectionSet->orderedNodes.erase( referenceId );
                
                } // End if contained

                // Nothing left in set?
                if ( selectionSet->orderedNodes.empty() )
                    removeSelectionSet( selectionSet->name );

            } // Next Set

        } // End if sandbox

        // Remove node from orphan list if it existed there.
        mOrphanNodes.erase( node );

        // Remove the object from the appropriate update bucketNodes.
        cgObjectNodeList * bucketNodes = &mUpdateBuckets[ node->mUpdateRate ].nodes;
        for ( cgObjectNodeList::iterator itBucketNode = bucketNodes->begin(); itBucketNode != bucketNodes->end(); ++itBucketNode )
        {
            if ( *itBucketNode == node )
            {
                bucketNodes->erase( itBucketNode );
                break;
            
            } // End if matches

        } // Next Node

        // If this node exists at the root level in the hierarchy,
        // remove it from the root node list first of all.
        if ( !node->getParent() )
        {
            mRootNodes.erase( node->getReferenceId() );
            mRootSpatialTrees.erase( node );
    
        } // End if no parent

        // Remove from the scene. Will automatically be removed
        // from name usage map when node is disposed.
        mObjectNodes.erase( node->getReferenceId() );

        // Now dispose of the node from memory.
        node->scriptSafeDispose();
        node = CG_NULL;
            
    } // Next Node

    // Commit changes
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        mWorld->commitTransaction( _T("deleteObjectNodes") );

    // Scene has been modified
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        setDirty( true );

    // Notify whoever is listening that the specified nodes were removed.
    onNodesDeleted( &cgNodesUpdatedEventArgs( this, &processedNodes ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pickClosestNode ( )
/// <summary>
/// Given the specified ray, find the closest node that is intersected by
/// that ray and return it. Also return the intersection point on the node.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::pickClosestNode( const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgVector3 & intersectionOut )
{
    return pickClosestNode( viewportSize, rayOrigin, rayDirection, false, 0.0f, intersectionOut );
}

//-----------------------------------------------------------------------------
//  Name : pickClosestNode ( )
/// <summary>
/// Given the specified ray, find the closest node that is intersected by
/// that ray and return it. Also return the intersection point on the node.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgScene::pickClosestNode( const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgVector3 & intersectionOut )
{
    // Validate requirements
    if ( !mActiveCamera )
        return CG_NULL;

    // Iterate through root level nodes and check for intersection.
    // nodes will also check for intersection against their children
    // and return the closest node found.
    cgObjectNodeMap::iterator itNode;
    cgFloat closestDistance = FLT_MAX;
    cgObjectNode * closestNode = CG_NULL;
    for ( itNode = mRootNodes.begin(); itNode != mRootNodes.end(); ++itNode )
    {
        // Check for intersection
        cgObjectNode * hitNode;
        cgFloat distance = FLT_MAX;
        if ( itNode->second->pick( mActiveCamera, viewportSize, rayOrigin, rayDirection, wireframe, wireTolerance, distance, hitNode ) )
        {
            // Is this the closest so far?
            if ( distance < closestDistance )
            {
                closestNode     = hitNode;
                closestDistance = distance;
            
            } // End if closest so far
        
        } // End if intersection occurred
        
    } // Next Node

    // Return the world space point of intersection (if hit occurred)
    if ( closestNode )
        intersectionOut = rayOrigin + (rayDirection * closestDistance);

    // Return the closest node (if any)
    return closestNode;
}

//-----------------------------------------------------------------------------
// Name : groupSelected ( )
/// <summary>
/// Group together any currently selected nodes into either a standard 
/// collection group, or optionally as an actor.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::groupSelected( bool asActor )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::groupSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Group the selected objects if possible.
    cgGroupNode * group = groupObjectNodes( mSelectedNodes, asActor );
    if ( group )
    {
        // Select the new group (automatically notifies listeners about selection change).
        clearSelection();
        group->setSelected( true );

        // NB: Scene isDirty automatically handled by createObjectNode()

    } // End if success

}

//-----------------------------------------------------------------------------
// Name : groupSelected ( )
/// <summary>
/// Group together the specified nodes into either a standard collection group, 
/// or optionally as an actor.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupNode * cgScene::groupObjectNodes( cgObjectNodeMap & nodes, bool asActor )
{
    // Skip if this is a no-op.
    if ( !canGroupObjectNodes( nodes ) )
        return CG_NULL;

    // Create a new group ready to contain the selected nodes
    cgGroupNode * newGroup = (cgGroupNode*)createObjectNode( false, (asActor) ? RTID_ActorObject : RTID_GroupObject, true );
    if ( !newGroup )
        return CG_NULL;
    
    // Position the group in the center of the specified nodes initially
    cgVector3 center;
    getObjectNodesPivot( nodes, center );
    newGroup->setPosition( center );

    // Make a duplicate of the node list in case it
    // is altered while the group is being constructed.
    cgObjectNodeMap processedNodes( nodes );

    // Find the common 'shared' parent for the selected nodes. Because
    // canGroupObjectNodes() will have bailed if these don't all match, 
    // we can just do this for the first selected node.
    cgObjectNodeMap::iterator itNode = processedNodes.begin();
    cgObjectNode * sharedParent = itNode->second->getParent();
    for ( ; sharedParent && sharedParent->isSelected(); )
    {
        // Step up a level in the hierarchy.
        sharedParent = sharedParent->getParent();
    
    } // Next parent

    // Now attach the group as a child of the shared parent (if applicable)
    newGroup->setParent( sharedParent );

    // If the shared parent is a group, or belongs to a group itself then
    // this new group should also be attached there.
    if ( sharedParent )
    {
        if ( sharedParent->queryReferenceType( RTID_GroupNode ) )
            newGroup->setOwnerGroup( (cgGroupNode*)sharedParent );
        else if ( sharedParent->getOwnerGroup() )
            newGroup->setOwnerGroup( sharedParent->getOwnerGroup() );
    
    } // End if not root

    // Attach each of the specified nodes that connect to the shared
    // parent to the newly created group
    for ( itNode = processedNodes.begin(); itNode != processedNodes.end(); ++itNode )
    {
        // Attach as a child of the group if this is the root level of the selection
        cgObjectNode * node = itNode->second;
        if ( node->getParent() == sharedParent )
            node->setParent( newGroup );

        // All selected nodes however belong to the parent group 
        // unless they are already assigned to one that is also being 
        // added to this new group.
        if ( !node->getOwnerGroup() || (processedNodes.find( node->getOwnerGroup()->getReferenceId() ) == processedNodes.end()) )
            node->setOwnerGroup( newGroup );

    } // Next Object

    // NB: Scene isDirty automatically handled by createObjectNode()

    // Return the created group to the caller.
    return newGroup;
}


//-----------------------------------------------------------------------------
// Name : ungroupSelected ( )
/// <summary>
/// Ungroup any currently selected groups.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::ungroupSelected( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::ungroupSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Skip if this is a no-op.
    if ( !canUngroupSelected() )
        return;

    // Make a duplicate of the selection in case the node list
    // is altered while any groups are being removed.
    cgObjectNodeMap selectedNodes( mSelectedNodes );

    // Find all currently valid groups.
    cgObjectNodeMap markedNodes;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = selectedNodes.begin(); itNode != selectedNodes.end(); ++itNode )
    {
        // Is this a group?
        cgObjectNode * node = itNode->second;
        if ( node->queryReferenceType( RTID_GroupNode ) )
        {
            // Ignore if this group is not the top level "selected" group and
            // its owner group is closed (i.e. this group is not directly selectable).
            cgGroupNode * group = (cgGroupNode*)node;
            if ( group->isMergedAsGroup() && group->getOwnerGroup()->isSelected() )
                continue;
            
            // Mark group for removal. 
            markedNodes[ group->getReferenceId() ] = group;
            
        } // End if is group

    } // Next Object

    // Remove all specified groups (will automatically detach all children).
    if ( !markedNodes.empty() )
        deleteObjectNodes( markedNodes );
    
    // NB: Scene isDirty automatically handled by deleteObjectNodes()
}

//-----------------------------------------------------------------------------
// Name : openSelectedGroups ( )
/// <summary>
/// Open any currently selected groups such that their child nodes become
/// available for selection / editing as if they were detached.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::openSelectedGroups( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::openSelectedGroups() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Skip if this is a no-op.
    if ( !canOpenSelectedGroups() )
        return;

    // Make a duplicate of the selection in case the node list
    // is altered while any groups are being opened.
    cgObjectNodeMap selectedNodes( mSelectedNodes );

    // Find all currently closed groups.
    bool modified = false;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = selectedNodes.begin(); itNode != selectedNodes.end(); ++itNode )
    {
        // Is this a group?
        cgObjectNode * node = itNode->second;
        if ( !node->queryReferenceType( RTID_GroupNode ) )
            continue;

        // Is the group already open?
        cgGroupNode * group = (cgGroupNode*)node;
        if ( group->isOpen() )
            continue;

        // If this group belongs to a closed group it should not be opened
        // at this time.
        if ( group->isMergedAsGroup() )
            continue;

        // This group can be opened
        group->setOpen( true );
        modified = true;

    } // Next Node

    // Scene is now dirty?
    if ( modified )
    {
        setDirty( true );
        onModifySelection( &cgSceneEventArgs( this ) );
    
    } // End if modified
}

//-----------------------------------------------------------------------------
// Name : closeSelectedGroups ( )
/// <summary>
/// Close any currently selected groups such that their child nodes are no
/// longer available for selection / editing.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::closeSelectedGroups( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::closeSelectedGroups() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Skip if this is a no-op.
    if ( !canCloseSelectedGroups() )
        return;

    // Make a duplicate of the selection in case the node list
    // is altered while any groups are being closed.
    cgObjectNodeMap selectedNodes( mSelectedNodes );

    // Find all currently opened groups.
    bool modified = false;
    cgObjectNodeMap::iterator itNode;
    for ( itNode = selectedNodes.begin(); itNode != selectedNodes.end(); ++itNode )
    {
        // Is this a group?
        bool closed = false;
        cgObjectNode * node = itNode->second;
        if ( node->queryReferenceType( RTID_GroupNode ) )
        {
            // Close the group if open
            cgGroupNode * group = (cgGroupNode*)node;
            if ( group->isOpen() )
            {
                group->setOpen( false );
                closed = true;
                modified = true;
            
            } // End if group open

        } // End if is group
        
        // If we didn't close a group above (not a group or was already closed?), 
        // check if we're supposed to be closing a parent group.
        if ( !closed )
        {
            // If the owner group of this node is open, we should close it.
            if ( node->getOwnerGroup() && node->getOwnerGroup()->isOpen() )
            {
                node->getOwnerGroup()->setOpen( false );
                modified = true;
            
            } // End if owner open
        
        } // End if !closed

    } // Next Node

    // Scene is now dirty?
    if ( modified )
    {
        setDirty( true );
        onModifySelection( &cgSceneEventArgs( this ) );
    
    } // End if modified
}

//-----------------------------------------------------------------------------
// Name : canGroupSelected ( )
/// <summary>
/// Determine if the currently selected nodes can be grouped.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::canGroupSelected( ) const
{
    return canGroupObjectNodes( mSelectedNodes );
}

//-----------------------------------------------------------------------------
// Name : canGroupObjectNodes ( )
/// <summary>
/// Determine if the specified nodes can be grouped.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::canGroupObjectNodes( const cgObjectNodeMap & nodes ) const
{
    // Is anything selected for grouping?
    if ( nodes.empty() )
        return false;

    // Selection Rules - Nodes can only be grouped if the following
    // criteria are met:
    //
    // 1) Nodes must be children of the same parent... or
    // 2) Nodes must have that parent somewhere above it in the
    //    node hierarchy with every node between them selected.
    //
    // In summary, the group node needs to be inserted just underneath
    // the first de-selected parent in the hierarchy, and above all the
    // selected nodes. These rules also handle the case where grouping
    // nodes that already belong to a group with some that do not must
    // be disallowed.

    // Iterate through each of the specified nodes and find their first
    // "de-selected" parent (one that is not in the list) or the root 
    // 'NULL' parent indicator.
    bool foundParent = false;
    const cgObjectNode * sharedParent = CG_NULL;
    cgObjectNodeMap::const_iterator itNode;
    for ( itNode = nodes.begin(); itNode != nodes.end(); ++itNode )
    {
        // Find the first de-selected parent
        const cgObjectNode * node = itNode->second, * parent = CG_NULL;
        for ( parent = node->getParent(); parent && (nodes.find( parent->getReferenceId() ) != nodes.end()); )
        {
            // Step up a level in the hierarchy.
            parent = parent->getParent();
        
        } // Next parent

        // If this is the first time we're identifying a parent, just record
        // it. Otherwise, check to make sure we match.
        if ( !foundParent )
        {
            sharedParent = parent;
            foundParent  = true;

        } // End if first time
        else
        {
            // The first time we find a different shared parent then
            // we cannot group these nodes together.
            if ( parent != sharedParent )
                return false;

        } // End if validate

    } // Next Node

    // We can group these nodes.
    return true;
}

//-----------------------------------------------------------------------------
// Name : canUngroupSelected ( )
/// <summary>
/// Determine if the currently selected nodes can be ungrouped.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::canUngroupSelected( ) const
{
    // Is anything selected for grouping?
    if ( mSelectedNodes.empty() )
        return false;

    // Selection Rules - Groups can only be removed if the following
    // criteria are met:
    //
    // 1) Any valid group is selected.
    
    // Iterate through each of the selected nodes and find any groups.
    cgObjectNodeMap::const_iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
        // Is this a group?
        cgObjectNode * node = itNode->second;
        if ( node->queryReferenceType( RTID_GroupNode ) )
            return true;
        
    } // Next Node

    // We cannot ungroup any of these nodes.
    return false;
}

//-----------------------------------------------------------------------------
// Name : canOpenSelectedGroups ( )
/// <summary>
/// Determine if any groups are currently selected and if they can be opened.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::canOpenSelectedGroups( ) const
{
    // Is anything selected for grouping?
    if ( mSelectedNodes.empty() )
        return false;

    // Selection Rules - Groups can only be opened if the following
    // criteria are met:
    //
    // 1) Any closed group is selected.
    
    // Iterate through each of the selected nodes and find any closed groups.
    cgObjectNodeMap::const_iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
         // Is this a group?
        cgObjectNode * node = itNode->second;
        if ( node->queryReferenceType( RTID_GroupNode ) )
        {
            // If it's closed then we can enable the "Open" selection.
            if ( !((cgGroupNode*)node)->isOpen() )
                return true;
        
        } // End if group

    } // Next Node

    // We cannot open any of these nodes.
    return false;
}

//-----------------------------------------------------------------------------
// Name : canCloseSelectedGroups ( )
/// <summary>
/// Determine if any groups are currently selected and if they can be closed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::canCloseSelectedGroups( ) const
{
    // Is anything selected for grouping?
    if ( mSelectedNodes.empty() )
        return false;

    // Selection Rules - Groups can only be closed if the following
    // criteria are met:
    //
    // 1) Any open group is selected.
    // 2) Any node belonging to an open group is selected.
    
    // Iterate through each of the selected nodes and find 
    // any open groups or nodes owned by open groups.
    cgObjectNodeMap::const_iterator itNode;
    for ( itNode = mSelectedNodes.begin(); itNode != mSelectedNodes.end(); ++itNode )
    {
         // Is this a group?
        cgObjectNode * node = itNode->second;
        if ( node->queryReferenceType( RTID_GroupNode ) )
        {
            // If it's open then we can enable the "Close" selection.
            if ( ((cgGroupNode*)node)->isOpen() )
                return true;
        
        } // End if group
        
        // If the owner group of this node is open we can also close.
        if ( node->getOwnerGroup() && node->getOwnerGroup()->isOpen() )
            return true;

    } // Next Node

    // We cannot close any of these groups.
    return false;
}

//-----------------------------------------------------------------------------
// Name : detachSelected ( )
/// <summary>
/// Delete any currently selected nodes from their parents.
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::detachSelected( )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::detachSelected() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return false;
    
    } // End if !sandbox

    // First, duplicate the selection list (so we can iterate through it
    // without fear of any nodes becoming de-selected when they are detached.)
    cgObjectNodeMap selectedNodes( mSelectedNodes );

    // Iterate through each selected node
    bool detached = false;
    cgObjectNodeMap::iterator itSelected;
    for ( itSelected = selectedNodes.begin(); itSelected != selectedNodes.end(); ++itSelected )
    {
        // Ignore this node if it is part of a closed group
        cgObjectNode * node = itSelected->second;
        if ( node->isMergedAsGroup() )
            continue;

        // Detach if it has a parent.
        if ( node->getParent() )
        {
            node->setParent( CG_NULL );
            detached = true;
        
        } // End if has parent

    } // Next Node

    // Scene has been modified?
    if ( detached )
    {
        setDirty( true );
        onModifySelection( &cgSceneEventArgs( this ) );
    
    } // End if modified

    // Success!
    return detached;
}

//-----------------------------------------------------------------------------
// Name : isDirty ( )
/// <summary>Get the scene dirty status.</summary>
//-----------------------------------------------------------------------------
bool cgScene::isDirty( ) const
{
    return mIsDirty;
}

//-----------------------------------------------------------------------------
// Name : isDirty ( )
/// <summary>Set the scene dirty status.</summary>
//-----------------------------------------------------------------------------
void cgScene::setDirty( bool dirty )
{
    // Is this a no-op?
    if ( mIsDirty == dirty )
        return;

    // Apply change
    mIsDirty = dirty; 
    onSceneDirtyChange( &cgSceneEventArgs(this) );
}

//-----------------------------------------------------------------------------
// Name : makeUniqueName ( )
/// <summary>
/// Given the specified suggested name, determine if it is unique. If it is
/// not unique, append a number to the end such that it will become unique.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScene::makeUniqueName( const cgString & name )
{
    // Pass through to main overload with no postfix initially
    return makeUniqueName( name, 0xFFFFFFFF );
}

//-----------------------------------------------------------------------------
// Name : makeUniqueName ( )
/// <summary>
/// Given the specified suggested name, determine if it is unique. If it is
/// not unique, append a number to the end such that it will become unique.
/// Note: Specify 0xFFFFFFFF to suffixNumber to initially search for name with
/// no appended number.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScene::makeUniqueName( const cgString & name, cgUInt32 suffixNumber )
{
    cgUInt32 counter = 1;
    cgString uniqueName  = name;

    // Was a suffix value supplied?
    if ( suffixNumber < 0xFFFFFFFF )
    {
         uniqueName = cgString::format( _T("%s%02i"), uniqueName.c_str(), suffixNumber );
         counter      = suffixNumber;
    
    } // End if 
    
    // Keep searching until we find a unique name
    NameUsageMap::iterator itName;
    cgString searchKey = cgString::toLower( uniqueName );
    while( (itName = mNameUsage.find(searchKey)) != mNameUsage.end() )
    {
        // Name matches. Try a new name and search again
        uniqueName = cgString::format( _T("%s%02i"), name.c_str(), counter );
        searchKey = cgString::toLower( uniqueName );
        counter++;

    } // Next name

    // Return the unique version of the name
    return uniqueName;
}

//-----------------------------------------------------------------------------
// Name : getNameUsage ( )
/// <summary>
/// Retrieve the map that contains a list of all the node names currently in
/// use within the scene at this time.
/// </summary>
//-----------------------------------------------------------------------------
cgScene::NameUsageMap & cgScene::getNameUsage( )
{
    return mNameUsage;
}

//-----------------------------------------------------------------------------
// Name : addSceneMaterial ( )
/// <summary>
/// Add the specified material to the scene for ownership. This means that
/// the material will show up in the material editor for this scene, and
/// also ensures that the material will never be removed from the database
/// unless it is physically removed from the scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::addSceneMaterial( cgMaterial * material )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::addSceneMaterial() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    // Add a full reference (live and soft) to this material to denote that
    // this scene actually owns a reference to this material. This will
    // increment any database reference count and ensures that the material
    // is never physically removed from the database unless it is removed
    // from the scene itself (removeSceneMaterial()).
    if ( material )
    {
        // Must not already exist.
        SceneMaterialMap::iterator itMaterial = mActiveMaterials.find( material->getReferenceId() );
        if ( itMaterial == mActiveMaterials.end() )
        {
            material->addReference( CG_NULL, false );
            mActiveMaterials[ material->getReferenceId() ] = material;
            cgAppLog::write( cgAppLog::Debug, _T("%i materials created.\n"), mActiveMaterials.size() );

            // Add usage information to the database if not an internal scene.
            if ( getSceneId() != 0 )
            {
                prepareQueries();
                mInsertMaterialUsage.bindParameter( 1, material->getLocalTypeId() );
                mInsertMaterialUsage.bindParameter( 2, material->getReferenceId() );
                mInsertMaterialUsage.bindParameter( 3, getSceneId() );
                mInsertMaterialUsage.step( true );

                // ToDo: 9999 - Error Check
            
            } // End if !internal

            // Notify anyone interested.
            onMaterialAdded( &cgSceneMaterialEventArgs( this, material ) );

            // Scene is now dirty
            setDirty( true );
        
        } // End if !exists
    
    } // End if valid
}

//-----------------------------------------------------------------------------
// Name : removeSceneMaterial ( )
/// <summary>
/// Remove the specified material from the scene's material ownership list.
/// If the scene was the last live item reference that material, and if it
/// held the last 'soft' reference it will be deleted from the database.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::removeSceneMaterial( cgMaterial * material )
{
    if ( cgGetSandboxMode() == cgSandboxMode::Disabled )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("cgScene::removeSceneMaterial() is a sandbox function and can only be called when sandbox mode is enabled.\n") );
        return;
    
    } // End if !sandbox

    if ( material != CG_NULL )
    {
        SceneMaterialMap::iterator itMaterial = mActiveMaterials.find( material->getReferenceId() );
        if ( itMaterial != mActiveMaterials.end() )
        {
            // Remove this scene's ownership reference.
            material->removeReference( CG_NULL, false );
            mActiveMaterials.erase( itMaterial );

            // Remove usage information to the database if not an internal scene.
            if ( getSceneId() != 0 )
            {
                prepareQueries();
                mDeleteMaterialUsage.bindParameter( 1, material->getReferenceId() );
                mDeleteMaterialUsage.bindParameter( 2, getSceneId() );
                mDeleteMaterialUsage.step( true );

                // ToDo: 9999 - Error Check
            
            } // End if !internal

            // Notify anyone interested.
            onMaterialRemoved( &cgSceneMaterialEventArgs( this, material ) );

            // Scene is now dirty
            setDirty( true );

        } // End if active
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : getMaterialPropertyIdentifiers ()
/// <summary>
/// Retrieve the pre-constructed list of identifiers (compatible with 
/// cgFilterExpression) for the various user defined material properties types.
/// </summary>
//-----------------------------------------------------------------------------
const cgFilterExpression::IdentifierArray & cgScene::getMaterialPropertyIdentifiers( ) const
{
    cgAssert( mWorld != CG_NULL );
    return mWorld->getConfiguration()->getMaterialPropertyIdentifiers();
}

//-----------------------------------------------------------------------------
//  Name : getRenderClassId ()
/// <summary>
/// Retrieve the integer identifier associated with the specified user defined
/// render class name string.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgScene::getRenderClassId( const cgString & className ) const
{
    cgAssert( mWorld != CG_NULL );
    return mWorld->getConfiguration()->getRenderClassId( className );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertMaterialUsage.isPrepared() == false )
            mInsertMaterialUsage.prepare( mWorld, _T("INSERT INTO 'Scenes::MaterialUsage' VALUES(NULL,?1,?2,?3)"), true );
        if ( mDeleteMaterialUsage.isPrepared() == false )
            mDeleteMaterialUsage.prepare( mWorld, _T("DELETE FROM 'Scenes::MaterialUsage' WHERE MaterialId=?1 AND SceneId=?2"), true );
        if ( mUpdateLandscapeRef.isPrepared() == false )
            mUpdateLandscapeRef.prepare( mWorld, _T("UPDATE 'Scenes' SET LandscapeId=?1 WHERE SceneId=?2"), true );
    
    } // End if sandbox
}

//-----------------------------------------------------------------------------
// Name : getSceneMaterials ( )
/// <summary>
/// Retrieve the list of all active materials associated with this scene.
/// </summary>
//-----------------------------------------------------------------------------
const cgScene::SceneMaterialMap & cgScene::getSceneMaterials( ) const
{
    return mActiveMaterials;
}

//-----------------------------------------------------------------------------
// Name : isActiveMaterial ( )
/// <summary>
/// Determine if the specified material is one of this scene's active
/// materials (those that should show up in the material editor).
/// </summary>
//-----------------------------------------------------------------------------
bool cgScene::isActiveMaterial( cgMaterial * material ) const
{
    if ( material )
        return ( mActiveMaterials.find( material->getReferenceId() ) != mActiveMaterials.end() );
    return false;
}

//-----------------------------------------------------------------------------
// Name : onNodeAdded () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onNodeAdded( cgNodeUpdatedEventArgs * e )
{
    // Trigger 'onNodeAdded' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onNodeAdded( e );
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted() (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onNodeDeleted( cgNodeUpdatedEventArgs * e )
{
    // Trigger 'onNodeDeleted' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onNodeDeleted( e );
}

//-----------------------------------------------------------------------------
// Name : onNodesDeleted () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onNodesDeleted( cgNodesUpdatedEventArgs * e )
{
    // Trigger 'onNodesDeleted' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onNodesDeleted( e );
}

//-----------------------------------------------------------------------------
// Name : onSceneDirtyChange () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onSceneDirtyChange( cgSceneEventArgs * e )
{
    // Trigger 'onSceneDirtyChange' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onSceneDirtyChange( e );
}

//-----------------------------------------------------------------------------
// Name : onSelectionUpdated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onSelectionUpdated( cgSelectionUpdatedEventArgs * e )
{
    // Trigger 'onSelectionUpdated' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onSelectionUpdated( e );
}

//-----------------------------------------------------------------------------
// Name : onSelectionCleared () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onSelectionCleared( cgSceneEventArgs * e )
{
    // Trigger 'onSelectionCleared' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onSelectionCleared( e );
}

//-----------------------------------------------------------------------------
// Name : onModifySelection () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onModifySelection( cgSceneEventArgs * e )
{
    // Trigger 'onModifySelection' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onModifySelection( e );
}

//-----------------------------------------------------------------------------
// Name : onDeleteSelection () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onDeleteSelection( cgSceneEventArgs * e )
{
    // Trigger 'onDeleteSelection' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onDeleteSelection( e );
}

//-----------------------------------------------------------------------------
// Name : onSelectionSetAdded () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onSelectionSetAdded( cgSelectionSetEventArgs * e )
{
    // Trigger 'onSelectionSetAdded' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onSelectionSetAdded( e );
}

//-----------------------------------------------------------------------------
// Name : onSelectionSetRemoved () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onSelectionSetRemoved( cgSelectionSetEventArgs * e )
{
    // Trigger 'onSelectionSetRemoved' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onSelectionSetRemoved( e );
}

//-----------------------------------------------------------------------------
// Name : onMaterialAdded () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onMaterialAdded( cgSceneMaterialEventArgs * e )
{
    // Trigger 'onMaterialAdded' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onMaterialAdded( e );
}

//-----------------------------------------------------------------------------
// Name : onMaterialRemoved () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgScene::onMaterialRemoved( cgSceneMaterialEventArgs * e )
{
    // Trigger 'onMaterialRemoved' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgSceneEventListener*)(*itListener))->onMaterialRemoved( e );
}

///////////////////////////////////////////////////////////////////////////////
// Global Operator Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : operator < () (cgSceneCellKey&, cgSceneCellKey&)
/// <summary>
/// Perform less than comparison on the SceneCellKey structure.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgSceneCellKey& key1, const cgSceneCellKey& key2)
{
    cgInt32 difference = key1.cellX - key2.cellX;
    if ( difference != 0 ) return (difference < 0);
    difference = key1.cellY - key2.cellY;
    if ( difference != 0 ) return (difference < 0);
    difference = key1.cellZ - key2.cellZ;
    if ( difference != 0 ) return (difference < 0);
    
    // Exact match (for the purposes of combining)
    return false;
}