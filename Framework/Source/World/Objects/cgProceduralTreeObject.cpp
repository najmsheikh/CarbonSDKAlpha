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
// Name : cgProceduralTreeObject.cpp                                         //
//                                                                           //
// Desc : Provides support for procedurally generated tree that can be       //
//        placed into a scene as an standalone object.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgProceduralTreeObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgProceduralTreeObject.h>
#include <World/cgVisibilitySet.h>
#include <World/cgSphereTree.h>
#include <Tools/Generators/cgProceduralTreeGenerator.h>
#include <Resources/cgMesh.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Rendering/cgRenderDriver.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgProceduralTreeObject::mInsertTree;
cgWorldQuery cgProceduralTreeObject::mLoadTree;

///////////////////////////////////////////////////////////////////////////////
// cgProceduralTreeObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeObject::cgProceduralTreeObject( cgUInt32 referenceId, cgWorld * world ) : cgWorldObject( referenceId, world )
{
    // Initialize members to sensible defaults
    mShadowStage = cgSceneProcessStage::Both;
}

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeObject::cgProceduralTreeObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgProceduralTreeObject * object = (cgProceduralTreeObject*)init;
    mShadowStage = object->mShadowStage;
}

//-----------------------------------------------------------------------------
//  Name : ~cgProceduralTreeObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeObject::~cgProceduralTreeObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgProceduralTreeObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgProceduralTreeObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgProceduralTreeObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new cgProceduralTreeObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeObject::applyObjectRescale( cgFloat scale )
{
    /*// Apply the scale to object-space data
    cgFloat fNewSize = mSize * fScale;
    
    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, fNewSize );
        mUpdateSize.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateSize.step( true ) == false )
        {
            cgString strError;
            mUpdateSize.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size of dummy object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local values.
    mSize = fNewSize;
    
    // Notify listeners that property was altered
    static const cgString strContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );*/

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( scale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ProceduralTreeObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgProceduralTreeObject::getDatabaseTable( ) const
{
    return _T("Objects::ProceduralTree");
}

/*//-----------------------------------------------------------------------------
//  Name : setSize()
/// <summary>
/// Set the local size of the dummy object (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeObject::setSize( cgFloat fSize )
{
    // Is this a no-op?
    if ( mSize == fSize )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, fSize );
        mUpdateSize.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateSize.step( true ) == false )
        {
            cgString strError;
            mUpdateSize.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size of dummy object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mSize = fSize;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Size");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}*/

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Create a new tree generator
    mGenerator = new cgProceduralTreeGenerator( mWorld->generateRefId( isInternalReference() ), mWorld );
    if ( !mGenerator->onComponentCreated( &cgComponentCreatedEventArgs( 0, e->cloneMethod ) ) )
    {
        mGenerator->deleteReference();
        mGenerator = CG_NULL;
        return false;
    
    } // End if failed
    
    // Add us as a reference to this object (don't increment true DB based
    // reference count if this is an internal element).
    mGenerator->addReference( this, isInternalReference() );

    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Generate the tree (we don't fail to create the object if this fails,
    // but it may spew errors).
    regenerate();

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ProceduralTreeObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertTree.bindParameter( 1, mReferenceId );
        mInsertTree.bindParameter( 2, (mGenerator) ? mGenerator->getReferenceId() : 0 );
        mInsertTree.bindParameter( 3, (cgInt32)0 ); // LightingStage
        mInsertTree.bindParameter( 4, (cgInt32)mShadowStage );
        mInsertTree.bindParameter( 5, (cgInt32)0 ); // ShadowReceiveStage        
        mInsertTree.bindParameter( 6, mSoftRefCount );

        // Execute
        if ( !mInsertTree.step( true ) )
        {
            cgString error;
            mInsertTree.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for procedural tree object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("ProceduralTreeObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("ProceduralTreeObject::insertComponentData") );

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
bool cgProceduralTreeObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the tree construction data.
    prepareQueries();
    mLoadTree.bindParameter( 1, e->sourceRefId );
    if ( !mLoadTree.step( ) || !mLoadTree.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadTree.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for procedural tree object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for procedural tree object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadTree.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadTree;

    // Load parameters
    cgUInt32 valueUInt32, dataSourceId;
    mLoadTree.getColumn( _T("DataSourceId"), dataSourceId );
    mLoadTree.getColumn( L"ShadowCastStage", valueUInt32 );
    mShadowStage = (cgSceneProcessStage::Base)valueUInt32;

    // Load the referenced generator data source.
    if ( dataSourceId != 0 )
    {
        // Create a new generator item.
        if ( e->cloneMethod == cgCloneMethod::Copy )
            mGenerator = new cgProceduralTreeGenerator( mWorld->generateRefId( isInternalReference() ), mWorld );
        else
            mGenerator = new cgProceduralTreeGenerator( dataSourceId, mWorld );

        // Instruct the data source to load its data.
        if ( !mGenerator->onComponentLoading( &cgComponentLoadingEventArgs( dataSourceId, 0, e->cloneMethod, CG_NULL ) ) )
        {
            mGenerator->deleteReference();
            mGenerator = CG_NULL;
            mLoadTree.reset();
            return false;
        
        } // End if failed

        // Reconnect to this component.
        mGenerator->addReference( this, true );

    } // End if valid identifier
    else
    {
        // Create a nav mesh if one was not specified.
        mGenerator = new cgProceduralTreeGenerator( mWorld->generateRefId( isInternalReference() ), mWorld );
        if ( !mGenerator->onComponentCreated( &cgComponentCreatedEventArgs( 0, cgCloneMethod::None ) ) )
        {
            mGenerator->deleteReference();
            mGenerator = CG_NULL;
            return false;

        } // End if failed

        // Add us as a reference to this object (don't increment true DB based
        // reference count if this is an internal element).
        mGenerator->addReference( this, isInternalReference() );

    } // End if no identifier

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned

    // Regenerate the mesh.
    regenerate();

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
void cgProceduralTreeObject::onComponentDeleted( )
{
    // Remove us as a valid reference from the object we're referencing (don't
    // decrement true DB based reference count if this is an internal node).
    if ( mGenerator )
        mGenerator->removeReference( this, isInternalReference() );
    mGenerator = CG_NULL;

    // Call base class implementation last.
    cgWorldObject::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertTree.isPrepared( mWorld ) )
            mInsertTree.prepare( mWorld, _T("INSERT INTO 'Objects::ProceduralTree' VALUES(?1,?2,?3,?4,?5,?6)"), true );
        /*if ( !mUpdateProcessStages.isPrepared( mWorld ) )
            mUpdateProcessStages.prepare( mWorld, _T("UPDATE 'Objects::ProceduralTree' SET LightStage=?1, ShadowCastStage=?2, ShadowReceiveStage=?3 WHERE RefId=?4"), true );*/
    
    } // End if sandbox

    // Read queries
    if ( !mLoadTree.isPrepared( mWorld ) )
        mLoadTree.prepare( mWorld, _T("SELECT * FROM 'Objects::ProceduralTree' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the world object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::render( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer )
{
    // Retrieve the underlying mesh resource and render if available
    cgMesh * mesh = mTreeMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return false;

    // Signal a straight draw of the object
    cgRenderDriver * driver = mWorld->getRenderDriver();
    driver->setConstantBufferAuto( issuer->getRenderTransformBuffer() );
    mesh->setDefaultColor( issuer->getNodeColor() );
    mesh->draw( );
    
    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
// Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified 
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::renderSubset( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & material )
{
    // Retrieve the underlying mesh resource and render if available
    cgMesh * mesh = mTreeMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return false;
    
    // Signal a straight draw of the object
    cgRenderDriver * driver = mWorld->getRenderDriver();
    driver->setConstantBufferAuto( issuer->getRenderTransformBuffer() );
    mesh->setDefaultColor( issuer->getNodeColor() );
    mesh->drawSubset( material, cgMeshDrawMode::Simple );

    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeObject::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Sandbox render is only necessary in wireframe mode unless the
    // node is hidden. Standard scene rendering behavior applies in other modes.
    if ( issuer->isRenderable() && !(flags & cgSandboxRenderFlags::Wireframe) )
    {
        cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
        return;
    
    } // End if !wireframe

    // Retrieve the underlying mesh resource if available
    cgMesh * mesh = mTreeMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
    {
        cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
        return;
    
    } // End if no mesh

    // Setup constants.
    cgRenderDriver * driver = cgRenderDriver::getInstance();
    cgColorValue color = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0xFFFFFFFF );
    cgConstantBuffer * constants = driver->getSandboxConstantBuffer().getResource( true );
    constants->setVector( _T("diffuseReflectance"), (cgVector4&)color );
    driver->setConstantBufferAuto( issuer->getRenderTransformBuffer() );
    driver->setConstantBufferAuto( driver->getSandboxConstantBuffer() );
    
    // Execute technique.
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
    if ( shader->beginTechnique( _T("drawWireframeMesh") ) )
    {
        while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mesh->draw( cgMeshDrawMode::Simple );
        shader->endTechnique();
    
    } // End if success

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
// Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distance )
{
    // Retrieve the underlying mesh resource and pick if available
    cgMesh * mesh = mTreeMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return false;
    
    // Pass through
    return mesh->pick( camera, viewportSize, issuer->getWorldTransform(false), rayOrigin, rayDirection, flags, wireTolerance, distance );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgProceduralTreeObject::getLocalBoundingBox( )
{
    // Retrieve the underlying mesh resource (returns
    // degenerate local origin <0,0,0> if not loaded)
    cgMesh * mesh = mTreeMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() ) 
        return cgBoundingBox::Empty;

    // Retrieve the bounding box properties
    return mesh->getBoundingBox( );
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::isRenderable() const
{
	// All meshes can render by default (assuming there is any data).
    return mTreeMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : isShadowCaster () (Virtual)
/// <summary>
/// Is the object capable of casting shadows? (i.e. a camera may not be)
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::isShadowCaster() const
{
	// Can this mesh cast shadows at runtime (assuming there is any data).
    return mTreeMesh.isValid() && (mShadowStage == cgSceneProcessStage::Runtime || mShadowStage == cgSceneProcessStage::Both);
}

//-----------------------------------------------------------------------------
//  Name : setGrowthProperties ()
/// <summary>
/// Update the properties that describe how this tree should be generated, and
/// if necessary regenerate the renderable representation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::setGrowthProperties( const cgProceduralTreeGrowthProperties & properties, bool regenerate )
{
    // Update local properties.
    mGenerator->setParameters(properties);

    // Notify listeners that property was altered
    static const cgString strContext = _T("GrowthProperties");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Regenerate the renderable mesh if requested.
    if ( regenerate )
        return this->regenerate();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getGrowthProperties ()
/// <summary>
/// Retrieve the properties that describe how this tree should be generated.
/// </summary>
//-----------------------------------------------------------------------------
const cgProceduralTreeGrowthProperties & cgProceduralTreeObject::getGrowthProperties ( ) const
{
    return mGenerator->getParameters();
}

//-----------------------------------------------------------------------------
//  Name : getMesh ()
/// <summary>
/// Retrieve the procedurally generated mesh resource being managed by this 
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgMeshHandle cgProceduralTreeObject::getMesh( ) const
{
    return mTreeMesh;
}

//-----------------------------------------------------------------------------
//  Name : regenerate ()
/// <summary>
/// Regenerate the renderable representation based on the current supplied
/// growth properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeObject::regenerate( )
{
    // Generate a new mesh based on current properties.
    cgMeshHandle newMesh = mGenerator->generate( );
    if ( !newMesh.isValid() )
        return false;

    // Replace the generated mesh.
    mTreeMesh = newMesh;

    // Notify listeners that mesh data was altered
    static const cgString strContext = _T("MeshData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgProceduralTreeNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeNode::cgProceduralTreeNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Tree%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeNode::cgProceduralTreeNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgProceduralTreeNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProceduralTreeNode::~cgProceduralTreeNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgProceduralTreeNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgProceduralTreeNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgProceduralTreeNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgProceduralTreeNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ProceduralTreeNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : registerVisibility () (Virtual)
/// <summary>
/// This node has been deemed visible during testing, but this method gives the
/// node a final say on how it gets registered with the visibility set. The
/// default behavior is simply to insert directly into object visibility list,
/// paying close attention to filtering rules.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProceduralTreeNode::registerVisibility( cgVisibilitySet * visibilityData )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    if ( !cgObjectNode::registerVisibility( visibilityData ) )
        return false;

    // Since this node type supports material based subset rendering,
    // we should also register ourselves with the visible material list
    // in the visibility set. First get the list of materials used by
    // the referenced mesh object.
    cgUInt32 flags = visibilityData->getSearchFlags();
    if ( flags & cgVisibilitySearchFlags::CollectMaterials )
    {
        cgMesh * mesh = (cgMesh*)getMesh().getResource(false);
        if ( mesh )
        {
            // Register each of these materials with the visibility set.
            const cgMaterialHandleArray & materials = mesh->getMaterials();
            for ( size_t i = 0, count = materials.size(); i < count; ++i )
                visibilityData->addVisibleMaterial( materials[i], this );
        
        } // End if valid / loaded

    } // End if materials required
    
    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgProceduralTreeNode::onComponentModified( cgReference * sender, cgComponentModifiedEventArgs * e )
{
    // What was modified?
    if ( sender == mReferencedObject )
    {
        if ( e->context == _T("MeshData") )
        {
            // Tree node is now 'dirty' and should always 
            // re-trigger a shadow map fill during the next render
            // process if necessary.
            nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

            // Invalidate visibility in the sphere tree in case 
            // material data was altered (will affect frame coherent 
            // batching similar to the show/hide case).
            if ( mSceneTreeNode )
                mSceneTreeNode->invalidateVisibility();

        } // End if MeshData

    } // End if from our object

    // Call base class implementation last
    cgObjectNode::onComponentModified( sender, e );
}