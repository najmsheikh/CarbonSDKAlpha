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
// Name : cgBSPVisTreeElement.cpp                                            //
//                                                                           //
// Desc : Class that provides configuration and management of BSP based      //
//        scene visibility data, exposed as a scene element type. This       //
//        provides the integration between the application (such as the      //
//        editing environment) and the relevant components of the navigation //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBSPVisTreeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Elements/cgBSPVisTreeElement.h>
#include <World/Objects/cgMeshObject.h>
#include <World/cgBSPVisTree.h>
#include <World/cgScene.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgBSPVisTreeElement::mInsertCompileParams;
cgWorldQuery cgBSPVisTreeElement::mInsertGeometryRef;
cgWorldQuery cgBSPVisTreeElement::mClearGeometryRefs;
cgWorldQuery cgBSPVisTreeElement::mUpdateCompileParams;
cgWorldQuery cgBSPVisTreeElement::mLoadCompileParams;
cgWorldQuery cgBSPVisTreeElement::mLoadGeometryRefs;

///////////////////////////////////////////////////////////////////////////////
// cgBSPVisTreeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBSPVisTreeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBSPVisTreeElement::cgBSPVisTreeElement( cgUInt32 referenceId, cgScene * scene ) : cgSceneElement( referenceId, scene )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgBSPVisTreeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBSPVisTreeElement::~cgBSPVisTreeElement()
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
void cgBSPVisTreeElement::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear variables.
    mOcclusionGeometryIds.clear();
    
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
cgSceneElement * cgBSPVisTreeElement::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgBSPVisTreeElement( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPVisTreeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BSPVisTreeElement )
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
cgString cgBSPVisTreeElement::getDatabaseTable( ) const
{
    return _T("SceneElements::BSPVisTree");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPVisTreeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( ) )
        return false;

    // Call base class implementation last.
    return cgSceneElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the element to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPVisTreeElement::sandboxRender( cgUInt32 flags, cgCameraNode * camera )
{
    // No post-clear operation.
    //if ( flags & cgSandboxRenderFlags::PostDepthClear || mSandboxRenderMethod == Hidden )
        //return;

    // Render the navigation mesh.
    /*if ( mNavMesh )
        mNavMesh->debugDraw( mParentScene->getRenderDriver() );*/
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPVisTreeElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("BSPVisTreeElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertCompileParams.bindParameter( 1, mReferenceId );
        mInsertCompileParams.bindParameter( 2, (cgUInt32)0 ); // Type
        mInsertCompileParams.bindParameter( 3, (cgUInt32)0 ); // SandboxRenderMethod
        mInsertCompileParams.bindParameter( 4, (cgUInt32)0 ); // ManualRebuild
        mInsertCompileParams.bindParameter( 5, mSoftRefCount );
        
        // Execute
        if ( !mInsertCompileParams.step( true ) )
        {
            cgString error;
            mInsertCompileParams.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for BSPVisTree scene element '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("BSPVisTreeElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("BSPVisTreeElement::insertComponentData") );

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
bool cgBSPVisTreeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the tree construction data.
    prepareQueries();
    mLoadCompileParams.bindParameter( 1, e->sourceRefId );
    if ( !mLoadCompileParams.step( ) || !mLoadCompileParams.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadCompileParams.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for BSPVisTree scene element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for BSPVisTree scene element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadCompileParams.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadCompileParams;

    // Now load the geometery references
    mLoadGeometryRefs.reset();
    mLoadGeometryRefs.bindParameter( 1, e->sourceRefId );
    if ( !mLoadGeometryRefs.step() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadGeometryRefs.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve list of geometery references for BSPVisTree scene element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve list of geometery references for BSPVisTree scene element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadGeometryRefs.reset();
        return false;

    } // End if failed

    // Iterate through each row returned and process the data.
    for ( ; mLoadGeometryRefs.nextRow(); )
    {
        // Get the target details.
        cgUInt32 refId;
        mLoadGeometryRefs.getColumn( _T("GeometryId"), refId );
        mOcclusionGeometryIds.push_back( refId );

    } // Next reference

    // Close pending read operations
    mLoadGeometryRefs.reset();
    
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
void cgBSPVisTreeElement::onComponentDeleted( )
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
void cgBSPVisTreeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertCompileParams.isPrepared( mWorld ) )
            mInsertCompileParams.prepare( mWorld, _T("INSERT INTO 'SceneElements::BSPVisTree' VALUES(?1,?2,?3,?4,?5)"), true );
        if ( !mInsertGeometryRef.isPrepared( mWorld ) )
            mInsertGeometryRef.prepare( mWorld, _T("INSERT INTO 'SceneElements::BSPVisTree::Occluders' VALUES(NULL,?1,?2)"), true );
        if ( !mClearGeometryRefs.isPrepared( mWorld ) )
            mClearGeometryRefs.prepare( mWorld, _T("DELETE FROM 'SceneElements::BSPVisTree::Occluders' WHERE ElementId=?1"), true );
        //if ( !mUpdateCompileParams.isPrepared( mWorld ) )
            //mUpdateCompileParams.prepare( mWorld, _T("UPDATE 'SceneElements::BSPVisTree' SET OccluderGeometryId=?1 WHERE RefId=?2"), true );

    } // End if sandbox

    // Read queries
    if ( !mLoadCompileParams.isPrepared( mWorld ) )
        mLoadCompileParams.prepare( mWorld, _T("SELECT * FROM 'SceneElements::BSPVisTree' WHERE RefId=?1"), true );
    if ( !mLoadGeometryRefs.isPrepared( mWorld ) )
        mLoadGeometryRefs.prepare( mWorld, _T("SELECT * FROM 'SceneElements::BSPVisTree::Occluders' WHERE ElementId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : addOcclusionGeometryId ( )
/// <summary>
/// Add a new reference ID for a mesh node that houses occlusion geometry used 
/// to construct this visibility tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPVisTreeElement::addOcclusionGeometryId( cgUInt32 meshNodeRefId )
{
    // Already contained?
    for ( size_t i = 0; i < mOcclusionGeometryIds.size(); ++i )
    {
        if ( mOcclusionGeometryIds[i] == meshNodeRefId )
            return;
    
    } // Next ref
    
    // Update database.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mInsertGeometryRef.bindParameter( 1, mReferenceId );
        mInsertGeometryRef.bindParameter( 2, meshNodeRefId );

        // Execute
        if ( !mInsertGeometryRef.step( true ) )
        {
            cgString error;
            mInsertGeometryRef.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert occlusion geometery reference for BSPVisTree element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member.
    mOcclusionGeometryIds.push_back( meshNodeRefId );

    // Notify any listeners of this change.
    static const cgString strContext = _T("AddOcclusionGeometry");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : clearOcclusionGeometry ( )
/// <summary>
/// Clear out the list of meshes that provide occluding geometery for this tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPVisTreeElement::clearOcclusionGeometry()
{
    // Update database first.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mClearGeometryRefs.bindParameter( 1, mReferenceId );

        // Execute
        if ( !mClearGeometryRefs.step( true ) )
        {
            cgString error;
            mClearGeometryRefs.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to clear occlusion geometery references for BSPVisTree element '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Clear local members
    mOcclusionGeometryIds.clear();
}

//-----------------------------------------------------------------------------
// Name : getOcclusionGeometryIds ( )
/// <summary>
/// Retrieve the list of reference identifiers for mesh nodes that house the 
/// occlusion geometry used to construct this visibility tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgUInt32Array & cgBSPVisTreeElement::getOcclusionGeometryIds( ) const
{
    return mOcclusionGeometryIds;
}

//-----------------------------------------------------------------------------
// Name : buildTree ( )
/// <summary>
/// Compile / populate the specified visibility tree based on the compilation
/// parameters specified within this element.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPVisTreeElement::buildTree( cgBSPTree * tree )
{
    // Valid?
    if ( !tree || mOcclusionGeometryIds.empty() )
        return false;

    // Build the data needed for compilation.
    cgArray<cgMeshHandle> meshes;
    cgArray<cgTransform> transforms;
    for ( size_t i = 0; i < mOcclusionGeometryIds.size(); ++i )
    {
        cgObjectNode * node = mParentScene->getObjectNodeById( mOcclusionGeometryIds[i] );
        if ( !node->queryObjectType( RTID_MeshObject ) )
            continue;

        // Get mesh data
        meshes.push_back( ((cgMeshNode*)node)->getMesh() );
        transforms.push_back( node->getWorldTransform(false) );

        // Unload the compilation objects.
        mParentScene->unloadObjectNode( node );

    } // Next occluder mesh

    // Data added?
    if ( meshes.empty() )
        return false;

    // Compile the tree.
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Begining BSP/PVS construction.\n") );
    if ( !tree->buildTree( meshes.size(), &meshes[0], &transforms[0] ) )
        return false;
    if ( !tree->compilePVS() )
        return false;
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Completed BSP/PVS construction.\n") );

    // Debug tree data.
    const cgBSPTree::NodeArray & nodes = tree->getNodes();
    const cgBSPTree::PlaneArray & nodePlanes = tree->getNodePlanes();
    const cgBSPTree::LeafArray & leaves = tree->getLeaves();
    const cgBSPTree::PortalArray & portals = tree->getPortals();
    const cgBSPTree::PointArray & portalPoints = tree->getPortalVertices();
    const cgByteArray & visibilityData = tree->getPVSData();
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Nodes: %d\n"), nodes.size() );
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Leaves: %d\n"), leaves.size() );
    cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Visibility Set: %d bytes\n"), visibilityData.size() );

    // Build a mesh to represent the portals.
//#define DEBUGPORTALS
#if defined(DEBUGPORTALS)
    cgMesh * newMesh = new cgMesh( mWorld->generateRefId( true ), mWorld );
    cgVertexFormat * destFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
    newMesh->prepareMesh( destFormat, false, mParentScene->getResourceManager() );

    // Add the portal triangle data.
    cgVertexFormat * srcFormat = cgVertexFormat::formatFromFVF( D3DFVF_XYZ );
    newMesh->setVertexSource( (void*)&portalPoints[0], portalPoints.size(), srcFormat );
    for ( size_t j = 0; j < portals.size(); ++j )
    {
        cgUInt32 indices[3];
        const cgBSPTree::Portal & portal = portals[j];
        for ( size_t k = 0; k < portal.vertexCount - 2; ++k )
        {
            indices[0] = portal.firstVertex;
            indices[1] = portal.firstVertex + k + 1;
            indices[2] = portal.firstVertex + k + 2;
            newMesh->addPrimitives( cgPrimitiveType::TriangleList, indices, 0, 3, cgMaterialHandle::Null, 0 );

            indices[0] = portal.firstVertex;
            indices[1] = portal.firstVertex + k + 2;
            indices[2] = portal.firstVertex + k + 1;
            newMesh->addPrimitives( cgPrimitiveType::TriangleList, indices, 0, 3, cgMaterialHandle::Null, 0 );
        
        } // Next triangle.

    } // Next portal

    // Finish preparation
    cgMeshHandle meshHandle;
    newMesh->endPrepare( true, true, true );
    mParentScene->getResourceManager()->addMesh( &meshHandle, newMesh, cgResourceFlags::ForceNew, _T("PortalData"), cgDebugSource() );
    cgMeshNode * meshNode = (cgMeshNode*)mParentScene->createObjectNode( true, RTID_MeshObject, false );
    meshNode->setMesh( meshHandle );
    meshNode->setNodeColor( 0xFF880000 );
#endif

    // Success!
    return true;
}