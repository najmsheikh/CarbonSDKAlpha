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
cgWorldQuery cgNavigationMeshElement::mInsertMeshParams;
cgWorldQuery cgNavigationMeshElement::mUpdateMeshParams;
cgWorldQuery cgNavigationMeshElement::mLoadMeshParams;

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
    if ( mNavMesh )
        mNavMesh->scriptSafeDispose();

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
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
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
// Name : buildMesh()
/// <summary>
/// Construct the navigation mesh based on the specified parameters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMeshElement::buildMesh( )
{
    // Release any previous navigation mesh.
    if ( mHandler )
        mHandler->scriptSafeDispose();
    mHandler = CG_NULL;
    if ( mNavMesh )
        mNavMesh->scriptSafeDispose();
    mNavMesh = CG_NULL;

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
    if ( !meshes.empty() )
    {
        // Allocate and build a new navigation mesh.
        mNavMesh = new cgNavigationMesh();
        if ( !mNavMesh->build( mParams, (cgUInt32)meshes.size(), &meshes.front(), &transforms.front() ) )
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
        mInsertMeshParams.bindParameter( 1, mReferenceId );
        mInsertMeshParams.bindParameter( 2, _T("") );
        mInsertMeshParams.bindParameter( 3, mParams.agentRadius );
        mInsertMeshParams.bindParameter( 4, mParams.agentHeight );
        mInsertMeshParams.bindParameter( 5, mParams.agentMaximumSlope );
        mInsertMeshParams.bindParameter( 6, mParams.agentMaximumStepHeight );
        mInsertMeshParams.bindParameter( 7, mParams.edgeMaximumLength );
        mInsertMeshParams.bindParameter( 8, mParams.edgeMaximumError );
        mInsertMeshParams.bindParameter( 9, mParams.regionMinimumSize );
        mInsertMeshParams.bindParameter( 10, mParams.regionMergedSize );
        mInsertMeshParams.bindParameter( 11, mParams.verticesPerPoly );
        mInsertMeshParams.bindParameter( 12, mParams.detailSampleDistance );
        mInsertMeshParams.bindParameter( 13, mParams.detailSampleMaximumError );
        mInsertMeshParams.bindParameter( 14, (cgUInt32)mSandboxRenderMethod );
        mInsertMeshParams.bindParameter( 15, (cgUInt32)0 ); // ManualRebuild
        mInsertMeshParams.bindParameter( 16, mSoftRefCount );
        
        // Execute
        if ( !mInsertMeshParams.step( true ) )
        {
            cgString error;
            mInsertMeshParams.getLastError( error );
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
    mLoadMeshParams.bindParameter( 1, e->sourceRefId );
    if ( !mLoadMeshParams.step( ) || !mLoadMeshParams.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadMeshParams.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh scene element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh scene element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadMeshParams.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadMeshParams;

    // Load parameters
    cgUInt32 valueUInt32;
    mParams = cgNavigationMeshCreateParams(); // Defaults
    mLoadMeshParams.getColumn( _T("AgentRadius"), mParams.agentRadius );
    mLoadMeshParams.getColumn( _T("AgentHeight"), mParams.agentHeight );
    mLoadMeshParams.getColumn( _T("AgentMaxSlope"), mParams.agentMaximumSlope );
    mLoadMeshParams.getColumn( _T("AgentMaxStepHeight"), mParams.agentMaximumStepHeight );
    mLoadMeshParams.getColumn( _T("EdgeMaxLength"), mParams.edgeMaximumLength );
    mLoadMeshParams.getColumn( _T("EdgeMaxError"), mParams.edgeMaximumError );
    mLoadMeshParams.getColumn( _T("RegionMinSize"), mParams.regionMinimumSize );
    mLoadMeshParams.getColumn( _T("RegionMergedSize"), mParams.regionMergedSize );
    mLoadMeshParams.getColumn( _T("VertsPerPoly"), mParams.verticesPerPoly );
    mLoadMeshParams.getColumn( _T("DetailSampleDistance"), mParams.detailSampleDistance );
    mLoadMeshParams.getColumn( _T("DetailSampleMaxError"), mParams.detailSampleMaximumError );
    mLoadMeshParams.getColumn( _T("SandboxRenderMethod"), valueUInt32 );
    mSandboxRenderMethod = (SandboxRenderMethod)valueUInt32;

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

    // Build the navigation mesh itself. Note: We don't
    // fail to create the element in this case, but warnings
    // may be output.
    buildMesh();

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
        if ( !mInsertMeshParams.isPrepared() )
            mInsertMeshParams.prepare( mWorld, _T("INSERT INTO 'SceneElements::NavigationMesh' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16)"), true );
        if ( !mUpdateMeshParams.isPrepared() )
            mUpdateMeshParams.prepare( mWorld, _T("UPDATE 'SceneElements::NavigationMesh' SET EditorName=?1, AgentRadius=?2, AgentHeight=?3, AgentMaxSlope=?4,")
                                               _T("AgentMaxStepHeight=?5, EdgeMaxLength=?6, EdgeMaxError=?7, RegionMinSize=?8, RegionMergedSize=?9, VertsPerPoly=?10,")
                                               _T("DetailSampleDistance=?11, DetailSampleMaxError=?12, SandboxRenderMethod=?13, ManualRebuild=?14 WHERE RefId=?15"), true );
    } // End if sandbox

    // Read queries
    if ( !mLoadMeshParams.isPrepared() )
        mLoadMeshParams.prepare( mWorld, _T("SELECT * FROM 'SceneElements::NavigationMesh' WHERE RefId=?1"), true );
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
        mUpdateMeshParams.bindParameter( 1, _T("") );
        mUpdateMeshParams.bindParameter( 2, mParams.agentRadius );
        mUpdateMeshParams.bindParameter( 3, mParams.agentHeight );
        mUpdateMeshParams.bindParameter( 4, mParams.agentMaximumSlope );
        mUpdateMeshParams.bindParameter( 5, mParams.agentMaximumStepHeight );
        mUpdateMeshParams.bindParameter( 6, mParams.edgeMaximumLength );
        mUpdateMeshParams.bindParameter( 7, mParams.edgeMaximumError );
        mUpdateMeshParams.bindParameter( 8, mParams.regionMinimumSize );
        mUpdateMeshParams.bindParameter( 9, mParams.regionMergedSize );
        mUpdateMeshParams.bindParameter( 10, mParams.verticesPerPoly );
        mUpdateMeshParams.bindParameter( 11, mParams.detailSampleDistance );
        mUpdateMeshParams.bindParameter( 12, mParams.detailSampleMaximumError );
        mUpdateMeshParams.bindParameter( 13, (cgUInt32)method );
        mUpdateMeshParams.bindParameter( 14, (cgUInt32)0 ); // ManualRebuild
        mUpdateMeshParams.bindParameter( 15, mReferenceId );

        // Execute
        if ( !mUpdateMeshParams.step( true ) )
        {
            cgString error;
            mUpdateMeshParams.getLastError( error );
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
    cgNavigationMeshCreateParams params = mParams;
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
    cgNavigationMeshCreateParams params = mParams;
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
    cgNavigationMeshCreateParams params = mParams;
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
    cgNavigationMeshCreateParams params = mParams;
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
    if ( memcmp( &params, &mParams, sizeof(cgNavigationMeshCreateParams) ) == 0 )
        return;

    // Update database.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateMeshParams.bindParameter( 1, _T("") );
        mUpdateMeshParams.bindParameter( 2, params.agentRadius );
        mUpdateMeshParams.bindParameter( 3, params.agentHeight );
        mUpdateMeshParams.bindParameter( 4, params.agentMaximumSlope );
        mUpdateMeshParams.bindParameter( 5, params.agentMaximumStepHeight );
        mUpdateMeshParams.bindParameter( 6, params.edgeMaximumLength );
        mUpdateMeshParams.bindParameter( 7, params.edgeMaximumError );
        mUpdateMeshParams.bindParameter( 8, params.regionMinimumSize );
        mUpdateMeshParams.bindParameter( 9, params.regionMergedSize );
        mUpdateMeshParams.bindParameter( 10, params.verticesPerPoly );
        mUpdateMeshParams.bindParameter( 11, params.detailSampleDistance );
        mUpdateMeshParams.bindParameter( 12, params.detailSampleMaximumError );
        mUpdateMeshParams.bindParameter( 13, (cgUInt32)mSandboxRenderMethod );
        mUpdateMeshParams.bindParameter( 14, (cgUInt32)0 ); // ManualRebuild
        mUpdateMeshParams.bindParameter( 15, mReferenceId );

        // Execute
        if ( !mUpdateMeshParams.step( true ) )
        {
            cgString error;
            mUpdateMeshParams.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update creation parameters for navigation mesh element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member.
    mParams = params;

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
    return mParams;
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