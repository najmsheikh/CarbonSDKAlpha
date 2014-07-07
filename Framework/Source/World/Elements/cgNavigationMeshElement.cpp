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
// Name : cgNavigationMeshElement.cpp                                        //
//                                                                           //
// Desc : Class that provides configuration and management of scene          //
//        navigation data, exposed as a scene element type. This provides    //
//        the integration between the application (such as the editing       //
//        environment) and the relevant components of the navigation system. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationMeshElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Elements/cgNavigationMeshElement.h>
#include <World/Objects/cgMeshObject.h>
#include <Navigation/cgNavigationMesh.h>
#include <Navigation/cgNavigationHandler.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgNavigationMeshElement::mInsertElement;
cgWorldQuery cgNavigationMeshElement::mUpdateParams;
cgWorldQuery cgNavigationMeshElement::mLoadElement;

///////////////////////////////////////////////////////////////////////////////
// cgNavigationMeshElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationMeshElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMeshElement::cgNavigationMeshElement( cgUInt32 referenceId, cgScene * scene ) : cgSceneElement( referenceId, scene )
{
    // Initialize variables to sensible defaults
    mNavMesh                = CG_NULL;
    mHandler                = CG_NULL;
    mSandboxRenderMethod    = ShowAll;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationMeshElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMeshElement::~cgNavigationMeshElement()
{
    // Clean up object resources.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clean up allocated items.
    if ( mHandler )
        mHandler->scriptSafeDispose();

    // Disconnect from our navigation mesh (do not allow soft reference
    // count to decrement -- we want to reconnect later).
    if ( mNavMesh )
        mNavMesh->removeReference( this, true );

    // Clear variables.
    mNavMesh                = CG_NULL;
    mHandler                = CG_NULL;
    mSandboxRenderMethod    = ShowAll;
    
    // Dispose base class if requested.
    if ( disposeBase == true )
        cgSceneElement::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a scene element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneElement * cgNavigationMeshElement::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgNavigationMeshElement( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationMeshElement )
        return true;

    // Supported by base?
    return cgSceneElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgNavigationMeshElement::getDatabaseTable( ) const
{
    return _T("SceneElements::NavigationMesh");
}

//-----------------------------------------------------------------------------
// Name : buildMesh()
/// <summary>
/// Construct the navigation mesh based on the specified parameters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::buildMesh( )
{
    // Validate requirements.
    if ( !mNavMesh )
        return false;

    // Release any previous navigation handler.
    if ( mHandler )
        mHandler->scriptSafeDispose();
    mHandler = CG_NULL;

    // Get a list of all static meshes currently defined 
    // in the scene.
    cgArray<cgMeshHandle> meshes;
    cgArray<cgTransform>  transforms;
    cgObjectNodeMap::const_iterator itNode;
    const cgObjectNodeMap & nodes = mParentScene->getObjectNodes();
    for ( itNode = nodes.begin(); itNode != nodes.end(); ++itNode )
    {
        cgObjectNode * node = itNode->second;

        // Ignore anything but static collidables.
        cgPhysicsModel::Base model = node->getPhysicsModel();
        if ( model != cgPhysicsModel::CollisionOnly && model != cgPhysicsModel::RigidStatic )
            continue;

        // Ignore anything that doesn't have a mesh representaiton.
        if ( !node->queryObjectType( RTID_MeshObject ) )
            continue;

        // Grab the mesh 
        cgMeshObject * meshObject = (cgMeshObject*)node->getReferencedObject();
        meshes.push_back( meshObject->getMesh() );
        transforms.push_back( node->getWorldTransform(false) );
    
    } // Next object node

    // Build the navigation mesh
    if ( !meshes.empty() || mParentScene->getLandscape() )
    {
        // Build a new navigation mesh.
        if ( !mNavMesh->build( (cgUInt32)meshes.size(), (meshes.empty()) ? CG_NULL : &meshes.front(), 
                               (transforms.empty()) ? CG_NULL : &transforms.front(), mParentScene->getLandscape() ) )
            return false;

        // Create a new handler for this navigation mesh.
        mHandler = new cgNavigationHandler();
        mHandler->initialize( mNavMesh, 256 ); // ToDo: configurable

        // Success!
        return true;

    } // End if has meshes

    // Nothing to do. This is not an error.
    return true;
}

//-----------------------------------------------------------------------------
// Name : createAgent()
/// <summary>
/// Create a new navigation agent to be associated with this navigation mesh.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgent * cgNavigationMeshElement::createAgent( const cgNavigationAgentCreateParams & params, const cgVector3 & position )
{
    if ( !mHandler )
        return CG_NULL;
    return mHandler->createAgent( params, position );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the element to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::sandboxRender( cgUInt32 flags, cgCameraNode * camera )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear || mSandboxRenderMethod == Hidden )
        return;

    // Render the navigation mesh.
    if ( mNavMesh )
        mNavMesh->debugDraw( mParentScene->getRenderDriver() );
}

//-----------------------------------------------------------------------------
//  Name : update () (Virtual)
/// <summary>
/// Allow the element to perform any necessary updates during the scene's
/// update process.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::update( cgFloat timeDelta )
{
    if ( mHandler )
        mHandler->update( timeDelta );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Create a new navigation mesh item.
    mNavMesh = new cgNavigationMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
    if ( !mNavMesh->onComponentCreated( &cgComponentCreatedEventArgs( 0, cgCloneMethod::None ) ) )
    {
        mNavMesh->deleteReference();
        mNavMesh = CG_NULL;
        return false;
    
    } // End if failed
    
    // Add us as a reference to this object (don't increment true DB based
    // reference count if this is an internal element).
    mNavMesh->addReference( this, isInternalReference() );

    // Insert the new object sub-element BEFORE it has been built.
    if ( !insertComponentData( ) )
        return false;

    // Build the navigation mesh itself. Note: We don't
    // fail to create the element in this case, but warnings
    // may be output.
    buildMesh();

    // Call base class implementation last.
    return cgSceneElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("NavigationMeshElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertElement.bindParameter( 1, mReferenceId );
        mInsertElement.bindParameter( 2, _T("") );
        mInsertElement.bindParameter( 3, (mNavMesh) ? mNavMesh->getReferenceId() : 0 );
        mInsertElement.bindParameter( 4, (cgUInt32)mSandboxRenderMethod );
        mInsertElement.bindParameter( 5, (cgUInt32)0 ); // ManualRebuild
        mInsertElement.bindParameter( 6, mSoftRefCount );
        
        // Execute
        if ( !mInsertElement.step( true ) )
        {
            cgString error;
            mInsertElement.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for navigation mesh scene element '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("NavigationMeshElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("NavigationMeshElement::insertComponentData") );

    } // End if !internal

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentLoading() (Virtual)
/// <summary>
/// Virtual method called when the component is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the navigation mesh construction data.
    prepareQueries();
    mLoadElement.bindParameter( 1, e->sourceRefId );
    if ( !mLoadElement.step( ) || !mLoadElement.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadElement.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh scene element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh scene element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadElement.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadElement;

    // Load parameters
    cgUInt32 valueUInt32, dataSourceId;
    mLoadElement.getColumn( _T("DataSourceId"), dataSourceId );
    mLoadElement.getColumn( _T("SandboxRenderMethod"), valueUInt32 );
    mSandboxRenderMethod = (SandboxRenderMethod)valueUInt32;

    // Load the referenced navigation mesh data source.
    if ( dataSourceId != 0 )
    {
        // Create a new navigation mesh item.
        if ( e->cloneMethod == cgCloneMethod::Copy )
            mNavMesh = new cgNavigationMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            mNavMesh = new cgNavigationMesh( dataSourceId, mWorld );

        // Instruct the data source to load its data.
        if ( !mNavMesh->onComponentLoading( &cgComponentLoadingEventArgs( dataSourceId, 0, e->cloneMethod, CG_NULL ) ) )
        {
            mNavMesh->deleteReference();
            mNavMesh = CG_NULL;
            mLoadElement.reset();
            return false;
        
        } // End if failed

        // Reconnect to this component.
        mNavMesh->addReference( this, true );

    } // End if valid identifier
    else
    {
        // Create a nav mesh if one was not specified.
        mNavMesh = new cgNavigationMesh( mWorld->generateRefId( isInternalReference() ), mWorld );
        if ( !mNavMesh->onComponentCreated( &cgComponentCreatedEventArgs( 0, cgCloneMethod::None ) ) )
        {
            mNavMesh->deleteReference();
            mNavMesh = CG_NULL;
            return false;

        } // End if failed

        // Add us as a reference to this object (don't increment true DB based
        // reference count if this is an internal element).
        mNavMesh->addReference( this, isInternalReference() );

    } // End if no identifier

    // Call base class implementation to read remaining data.
    if ( !cgSceneElement::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned

    // If the navigation mesh has no tiles, attempt to rebuild it.
    if ( !mNavMesh->getTileCount() )
        buildMesh();
    else
    {
        // Create a new handler for this navigation mesh.
        mHandler = new cgNavigationHandler();
        mHandler->initialize( mNavMesh, 256 ); // ToDo: configurable
    
    } // End if has tiles

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::onComponentDeleted( )
{
    // Remove us as a valid reference from the object we're referencing (don't
    // decrement true DB based reference count if this is an internal node).
    if ( mNavMesh )
        mNavMesh->removeReference( this, isInternalReference() );
    mNavMesh = CG_NULL;

    // Call base class implementation last.
    cgSceneElement::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertElement.isPrepared( mWorld ) )
            mInsertElement.prepare( mWorld, _T("INSERT INTO 'SceneElements::NavigationMesh' VALUES(?1,?2,?3,?4,?5,?6)"), true );
        if ( !mUpdateParams.isPrepared( mWorld ) )
            mUpdateParams.prepare( mWorld, _T("UPDATE 'SceneElements::NavigationMesh' SET EditorName=?1, ")
                                               _T("SandboxRenderMethod=?2, ManualRebuild=?3 WHERE RefId=?4"), true );
    } // End if sandbox

    // Read queries
    if ( !mLoadElement.isPrepared( mWorld ) )
        mLoadElement.prepare( mWorld, _T("SELECT * FROM 'SceneElements::NavigationMesh' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : setSandboxRenderMethod ( )
/// <summary>
/// Set the approach used when rendering this navigation mesh element within
/// the sandbox / world editor.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setSandboxRenderMethod( SandboxRenderMethod method )
{
    // Is this a no-op?
    if ( mSandboxRenderMethod == method )
        return;
    
    // Update database.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParams.bindParameter( 1, _T("") );
        mUpdateParams.bindParameter( 2, (cgUInt32)method );
        mUpdateParams.bindParameter( 3, (cgUInt32)0 ); // ManualRebuild
        mUpdateParams.bindParameter( 4, mReferenceId );

        // Execute
        if ( !mUpdateParams.step( true ) )
        {
            cgString error;
            mUpdateParams.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update sandbox render method for navigation mesh element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member.
    mSandboxRenderMethod = method;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SandboxRenderMethod");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : getSandboxRenderMethod ( )
/// <summary>
/// Retrieve the approach used when rendering this navigation mesh element
/// within the sandbox / world editor.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMeshElement::SandboxRenderMethod cgNavigationMeshElement::getSandboxRenderMethod( ) const
{
    return mSandboxRenderMethod;
}

//-----------------------------------------------------------------------------
// Name : setAgentRadius ( )
/// <summary>
/// Set the radius of the navigation agents that will be assigned to this mesh.
/// The mesh will be eroded by this amount. Specify 'true' to the 'rebuild'
/// parameter to automatically rebuild the mesh immediately, or call 
/// 'buildMesh()' once all changes have been made.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setAgentRadius( cgFloat radius, bool rebuild /* = false */ )
{
    cgNavigationMeshCreateParams params = getParameters();
    params.agentRadius = radius;
    setParameters( params, rebuild );
}

//-----------------------------------------------------------------------------
// Name : setAgentHeight ( )
/// <summary>
/// Set the height of the navigation agents that will be assigned to this mesh.
/// Navigable areas must be at least this tall. Specify 'true' to the 'rebuild'
/// parameter to automatically rebuild the mesh immediately, or call 
/// 'buildMesh()' once all changes have been made.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setAgentHeight( cgFloat height, bool rebuild /* = false */ )
{
    cgNavigationMeshCreateParams params = getParameters();
    params.agentHeight = height;
    setParameters( params, rebuild );
}

//-----------------------------------------------------------------------------
// Name : setAgentMaximumSlope ( )
/// <summary>
/// Set the maximum angle of incline that agents assigned to this mesh will be
/// able to navigate. Specify 'true' to the 'rebuild' parameter to 
/// automatically rebuild the mesh immediately, or call 'buildMesh()' once all
/// changes have been made.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setAgentMaximumSlope( cgFloat degrees, bool rebuild /* = false */ )
{
    cgNavigationMeshCreateParams params = getParameters();
    params.agentMaximumSlope = degrees;
    setParameters( params, rebuild );
}

//-----------------------------------------------------------------------------
// Name : setAgentMaximumStepHeight ( )
/// <summary>
/// Set the maximum height of (non-slope) obstacles that agents assigned to
/// this mesh will be able to climb; i.e. the height of a single step of a
/// staircase. Specify 'true' to the 'rebuild' parameter to automatically
/// rebuild the mesh immediately, or call 'buildMesh()' once all changes have
/// been made.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setAgentMaximumStepHeight( cgFloat height, bool rebuild /* = false */ )
{
    cgNavigationMeshCreateParams params = getParameters();
    params.agentMaximumStepHeight = height;
    setParameters( params, rebuild );
}

//-----------------------------------------------------------------------------
// Name : setParameters ( )
/// <summary>
/// Set the creation parameters that describe how the navigation mesh should
/// be generated based on current scene data. Specify 'true' to the 'rebuild'
/// parameter to automatically rebuild the mesh immediately, or call 
/// 'buildMesh()' once all changes have been made.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMeshElement::setParameters( const cgNavigationMeshCreateParams & params, bool rebuild /* = false */ )
{
    // Is this a no-op?
    if ( memcmp( &params, &mNavMesh->getParameters(), sizeof(cgNavigationMeshCreateParams) ) == 0 )
        return;

    // Update parameters.
    mNavMesh->setParameters( params );

    // Notify any listeners of this change.
    static const cgString strContext = _T("CreationParameters");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Re build the navigation mesh if requested. Note: We don't
    // fail to update the element in this case, but warnings
    // may be output.
    if ( rebuild )
        buildMesh();
}

//-----------------------------------------------------------------------------
// Name : getParameters ( )
/// <summary>
/// Retrieve the creation parameters that describe how the navigation mesh
/// should be generated based on current scene data.
/// </summary>
//-----------------------------------------------------------------------------
const cgNavigationMeshCreateParams & cgNavigationMeshElement::getParameters( ) const
{
    return mNavMesh->getParameters();
}

//-----------------------------------------------------------------------------
// Name : getNavigationHandler ( )
/// <summary>
/// Retrieve the navigation handler (crowd) that is responsible for managing
/// the agents that will navigate this mesh.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationHandler * cgNavigationMeshElement::getNavigationHandler( ) const
{
    return mHandler;
}