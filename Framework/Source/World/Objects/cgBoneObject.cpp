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
// Name : cgBoneObject.cpp                                                   //
//                                                                           //
// Desc : Contains classes that allow for rigging of skinned meshes with     //
//        bone information. These bones can then be animated independantly   //
//        using the animation controller, in order to affect the skinned     //
//        mesh to which they are assigned.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBoneObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgBoneObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/Elements/cgCollisionShapeElement.h>
#include <World/Objects/Elements/cgCapsuleCollisionShapeElement.h>
#include <World/cgScene.h>
#include <Math/cgMathUtility.h>
#include <Physics/Bodies/cgRigidBody.h>
#include <Physics/cgPhysicsWorld.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>
#include <Resources/cgResourceManager.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgBoneObject::mInsertBone;
cgWorldQuery cgBoneObject::mUpdateProperties;
cgWorldQuery cgBoneObject::mLoadBone;

//-----------------------------------------------------------------------------
//  Name : cgBoneObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::cgBoneObject( cgUInt32 referenceId, cgWorld * world ) : cgWorldObject( referenceId, world )
{
    // Initialize members to sensible defaults
    mInitialCollisionState = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBoneObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::cgBoneObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgBoneObject * pObject = (cgBoneObject*)init;
    mInitialCollisionState = pObject->mInitialCollisionState;
}

//-----------------------------------------------------------------------------
// Name : ~cgBoneObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneObject::~cgBoneObject()
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
void cgBoneObject::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources.
    mSandboxMesh.close();
    mSandboxLinkMesh.close();

    // Dispose base.
    if ( disposeBase )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBoneObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgBoneObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgBoneObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new cgBoneObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgBoneObject::getLocalBoundingBox( )
{
    // ToDo: cache.

    // Compute bounding box for bone based on any child collision shapes.
    const cgObjectSubElementArray & elements = getSubElements( OSECID_CollisionShapes );
    if ( elements.empty() )
        return cgBoundingBox( 0, 0, 0, 0, 0, 0 );
    else
    {
        cgBoundingBox bounds;
        for ( size_t i = 0; i < elements.size(); ++i )
        {
            cgBoundingBox shapeBounds = ((cgCollisionShapeElement*)elements[i])->getShapeBoundingBox();
            shapeBounds = cgBoundingBox::transform( shapeBounds, ((cgCollisionShapeElement*)elements[i])->getTransform() );
            bounds.addPoint( shapeBounds.min );   
            bounds.addPoint( shapeBounds.max );

        } // Next sub element
        return bounds;

    } // End if has elements
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxLinkMesh.isValid() || !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMeshes( ) )
            return false;
    
    } // End if no mesh

    // Compute zoom factor for the mesh.
    cgFloat zoomFactor = camera->estimateZoomFactor( viewportSize, issuer->getPosition( false ), 0.05f );

    // Test link to child bones.
    cgTransform t;
    const cgTransform & objectTransform = issuer->getWorldTransform( false );
    cgMesh * mesh = mSandboxLinkMesh.getResource(true);
    if ( mesh && mesh->isLoaded() )
    {
        cgObjectNodeList::iterator itChild;
        cgObjectNodeList & children = issuer->getChildren();
        for ( itChild = children.begin(); itChild != children.end(); ++itChild )
        {
            cgObjectNode * childNode = *itChild;
            if ( !childNode->queryObjectType( RTID_BoneObject ) )
                continue;

            // Setup world transform
            cgVector3 targetPosition = childNode->getPosition(false), bonePosition = objectTransform.position();
            t.lookAt( bonePosition, targetPosition );
            t.scaleLocal( 5.0f * zoomFactor, cgVector3::length(bonePosition-targetPosition), 5.0f * zoomFactor );
            t.rotateLocal( CGEToRadian(90.0f), 0, 0 );

            // Transform ray from object space of the bone into
            // the space of the bone mesh.
            cgVector3 meshRayOrigin, meshRayDirection;
            cgTransform inverseObjectTransform, diffTransform;
            cgTransform::inverse( inverseObjectTransform, objectTransform );
            diffTransform = t * inverseObjectTransform;
            diffTransform.invert();
            diffTransform.transformCoord( meshRayOrigin, rayOrigin );
            diffTransform.transformNormal( meshRayDirection, rayDirection );
            cgVector3::normalize( meshRayDirection, meshRayDirection );

            // Pass through
            if ( mesh->pick( camera, viewportSize, t, meshRayOrigin, meshRayDirection, wireframe, wireTolerance, distance ) )
            {
                cgVector3 intersection = meshRayOrigin + meshRayDirection * distance;
                diffTransform.inverseTransformCoord( intersection, intersection );
                distance = cgVector3::length( intersection - rayOrigin );
                return true;
            
            } // End if hit

        } // Next child

    } // End if has link mesh.

    // Now check standard representation
    mesh = mSandboxMesh.getResource(true);
    if ( mesh && mesh->isLoaded() )
    {
        // Setup world transform
        t.identity();
        t.setPosition( objectTransform.position() );
        t.scaleLocal( 10.0f * zoomFactor, 10.0f * zoomFactor, 10.0f * zoomFactor );

        // Transform ray from object space of the bone into
        // the space of the bone mesh.
        cgVector3 meshRayOrigin, meshRayDirection;
        cgTransform inverseObjectTransform, diffTransform;
        cgTransform::inverse( inverseObjectTransform, objectTransform );
        diffTransform = t * inverseObjectTransform;
        diffTransform.invert();
        diffTransform.transformCoord( meshRayOrigin, rayOrigin );
        diffTransform.transformNormal( meshRayDirection, rayDirection );
        cgVector3::normalize( meshRayDirection, meshRayDirection );

        // Pass through
        if ( mesh->pick( camera, viewportSize, t, meshRayOrigin, meshRayDirection, wireframe, wireTolerance, distance ) )
        {
            cgVector3 intersection = meshRayOrigin + meshRayDirection * distance;
            diffTransform.inverseTransformCoord( intersection, intersection );
            distance = cgVector3::length( intersection - rayOrigin );
            return true;
        
        } // End if hit

    } // End if has link mesh.

    // No hit
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // ONLY post-clear rendering.
    if ( !(flags & cgSandboxRenderFlags::PostDepthClear) )
    {
        cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
        return;
    
    } // End if !PostDepthClear

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxLinkMesh.isValid() || !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMeshes( ) )
        {
            cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
            return;
        
        } // End if failed
    
    } // End if no mesh

    // Compute zoom factor for the mesh.
    cgRenderDriver * driver = mWorld->getRenderDriver();
    cgFloat zoomFactor = camera->estimateZoomFactor( driver->getViewport().size, issuer->getPosition( false ), 0.05f );

    // Setup constants.
    cgColorValue interiorColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0.7f, 0.2f, 0.1f, 0.4f );
    interiorColor.a = 0.2f;
    cgColorValue wireColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0xFFFFFFFF );
    wireColor.a = 0.4f;
    cgConstantBuffer * constants = driver->getSandboxConstantBuffer().getResource( true );
    constants->setVector( _T("shapeInteriorColor"), (cgVector4&)interiorColor );
    constants->setVector( _T("shapeWireColor"), (cgVector4&)wireColor );
    driver->setConstantBufferAuto( driver->getSandboxConstantBuffer() );

    // Draw links to all child BONES!
    cgTransform t;
    cgMesh * mesh = mSandboxLinkMesh.getResource(true);
    const cgTransform & objectTransform = issuer->getWorldTransform( false );
    if ( mesh && mesh->isLoaded() )
    {
        cgObjectNodeList::iterator itChild;
        cgObjectNodeList & children = issuer->getChildren();
        for ( itChild = children.begin(); itChild != children.end(); ++itChild )
        {
            cgObjectNode * childNode = *itChild;
            if ( !childNode->queryObjectType( RTID_BoneObject ) )
                continue;

            // Setup world transform
            cgVector3 targetPosition = childNode->getPosition(false), bonePosition = objectTransform.position();
            t.lookAt( bonePosition, targetPosition );
            t.scaleLocal( 5.0f * zoomFactor, cgVector3::length(bonePosition-targetPosition), 5.0f * zoomFactor );
            t.rotateLocal( CGEToRadian(90.0f), 0, 0 );
            driver->setWorldTransform( t );

            // Execute technique.
            cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
            if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
            {
                while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
                    mesh->draw( cgMeshDrawMode::Simple );
                shader->endTechnique();

            } // End if success
        
        } // Next child        

    } // End if mesh valid

    // Now render standard representation.
    mesh = mSandboxMesh.getResource(true);
    if ( mesh && mesh->isLoaded() )
    {
        // Set new constants
        interiorColor.a = 0.4f;
        wireColor.a = 0.9f;
        constants->setVector( _T("shapeInteriorColor"), (cgVector4&)interiorColor );
        constants->setVector( _T("shapeWireColor"), (cgVector4&)wireColor );

        // Setup world transform
        t.identity();
        t.setPosition( objectTransform.position() );
        t.setOrientation( objectTransform.orientation() );
        t.scaleLocal( 10.0f * zoomFactor, 10.0f * zoomFactor, 10.0f * zoomFactor );
        driver->setWorldTransform( t );

        // Execute technique.
        cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
        if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
        {
            while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
                mesh->draw( cgMeshDrawMode::Simple );
            shader->endTechnique();

        } // End if success

    } // End if has link mesh.

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
// Name : createSandboxMeshes() (Protected)
/// <summary>
/// Generate the physical representations of this bone for rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::createSandboxMeshes( )
{
    // Compute a shape identifier that can be used to find duplicate meshes.
    cgString referenceName = cgString::format( _T("Core::SandboxShapes::BoneLink") );

    // Does a mesh with this identifier already exist?
    cgResourceManager * resources = mWorld->getResourceManager();
    if ( !resources->getMesh( &mSandboxLinkMesh, referenceName ) )
    {
        // Dispose of any previous mesh generated and
        // allocate a new one.
        mSandboxLinkMesh.close( true );
        cgMesh * newMesh = new cgMesh( 0, CG_NULL );

        // Set the mesh data format and populate it.
        cgResourceManager * resources = mWorld->getResourceManager();
        cgVertexFormat * vertexFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( !newMesh->createCone( vertexFormat, 1, 0.1f, 1, 1, 10, false, cgMeshCreateOrigin::Bottom, true, resources ) )
        {
            delete newMesh;
            return false;

        } // End if failed
            
        // Add to the resource manager
        if ( !resources->addMesh( &mSandboxLinkMesh, newMesh, 0, referenceName, cgDebugSource() ) )
        {
            delete newMesh;
            return false;

        } // End if failed

    } // End if no existing mesh

    referenceName = cgString::format( _T("Core::SandboxShapes::Bone") );

    // Does a mesh with this identifier already exist?
    if ( !resources->getMesh( &mSandboxMesh, referenceName ) )
    {
        // Dispose of any previous mesh generated and
        // allocate a new one.
        mSandboxMesh.close( true );
        cgMesh * newMesh = new cgMesh( 0, CG_NULL );

        // Set the mesh data format and populate it.
        cgResourceManager * resources = mWorld->getResourceManager();
        cgVertexFormat * vertexFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( !newMesh->createSphere( vertexFormat, 1, 2, 4, false, cgMeshCreateOrigin::Center, true, resources ) )
        {
            delete newMesh;
            return false;

        } // End if failed
            
        // Add to the resource manager
        if ( !resources->addMesh( &mSandboxMesh, newMesh, 0, referenceName, cgDebugSource() ) )
        {
            delete newMesh;
            return false;

        } // End if failed

    } // End if no existing mesh
    
    // Success?
    return mSandboxMesh.isValid() && mSandboxLinkMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : getInitialCollisionState()
/// <summary>
/// Determine if the collision volume is enabled for this bone for the purposes
/// of ray testing, etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::getInitialCollisionState( ) const
{
    return mInitialCollisionState;
}

//-----------------------------------------------------------------------------
//  Name : setInitialCollisionState()
/// <summary>
/// Enable / disable the collision volume for this bone for the purposes
/// of ray testing, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::setInitialCollisionState( bool enable )
{
    // Is this a no-op?
    if ( mInitialCollisionState == enable )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProperties.bindParameter( 1, enable );
        mUpdateProperties.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateProperties.step( true ) )
        {
            cgString error;
            mUpdateProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mInitialCollisionState = enable;

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("InitialCollisionState");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoneObject )
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
cgString cgBoneObject::getDatabaseTable( ) const
{
    return _T("Objects::Bone");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("BoneObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBone.bindParameter( 1, mReferenceId );
        mInsertBone.bindParameter( 2, mInitialCollisionState );
        mInsertBone.bindParameter( 6, mSoftRefCount );

        // Execute
        if ( !mInsertBone.step( true ) )
        {
            cgString error;
            mInsertBone.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for bone object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("BoneObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("BoneObject::insertComponentData") );

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
bool cgBoneObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the bone data.
    prepareQueries();
    mLoadBone.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBone.step( ) || !mLoadBone.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadBone.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for bone object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for bone object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadBone.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadBone;

    // Update our local members
    mLoadBone.getColumn( _T("HasCollisionVolume"), mInitialCollisionState );
    
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

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertBone.isPrepared() == false )
            mInsertBone.prepare( mWorld, _T("INSERT INTO 'Objects::Bone' VALUES(?1,?2,?3)"), true );
        if ( mUpdateProperties.isPrepared() == false )
            mUpdateProperties.prepare( mWorld, _T("UPDATE 'Objects::Bone' SET HasCollisionVolume=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadBone.isPrepared() == false )
        mLoadBone.prepare( mWorld, _T("SELECT * FROM 'Objects::Bone' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgBoneNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBoneNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::cgBoneNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Default the node color
    mColor                = 0xFFAEBACB;
    mCollisionEnabled     = false;
}

//-----------------------------------------------------------------------------
//  Name : cgBoneNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::cgBoneNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
    // Clone properties
    cgBoneNode * node = (cgBoneNode*)init;
    mCollisionEnabled = node->mCollisionEnabled;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Bone%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : ~cgBoneNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoneNode::~cgBoneNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBoneNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgBoneNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgBoneNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgBoneNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What was modified?
    if ( e->context == _T("InitialCollisionState") )
    {
        // If we're in sandbox mode, update our own internal 
        // collision enabled state, and rebuild physics body immediately.
        if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        {
            mCollisionEnabled = getInitialCollisionState();
            
            // Add / remove physics body.
            buildPhysicsBody();
        
        } // End if sandbox mode

    } // End if HasCollisionVolume
    
    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : setBoneOrientation() (Virtual)
/// <summary>
/// Set the orientation of the bone based on the specified source and
/// destination world space positions.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::setBoneOrientation( const cgVector3 & source, const cgVector3 & destination, const cgVector3 & up )
{
    // Compute new "look at" (Z direction) between the source and destination points.
    cgVector3 newDirection = destination - source;

    // Normalize (if possible)
    cgFloat length = cgVector3::length( newDirection );
    if ( length < CGE_EPSILON )
        return;
    newDirection /= length;

    // Generate the new node transform
    cgTransform t;
    cgVector3 currentPosition = getPosition();
    t.lookAt( currentPosition, currentPosition + newDirection, up );

    // Rotate the transform to ensure our bone is aligned correctly
    // (bone is aligned to the X axis, not the Z as with the lookAt method).
    t.rotateLocal( 0, CGEToRadian(-90.0f), 0 );
    
    // Maintain the original X/Y/Z scale of the bone.
    t.setLocalScale( getScale() );
    
    // Adjust bone transform.
    this->setWorldTransform( t );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoneNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDirection ()
/// <summary>
/// Get the primary direction of the bone (defaults to X axis).
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgBoneNode::getDirection( )
{
    return getXAxis();
}

//-----------------------------------------------------------------------------
// Name : move() (Virtual)
/// <summary>Move the object by the specified amount in "parent" space.</summary>
//-----------------------------------------------------------------------------
void cgBoneNode::move( const cgVector3 & amount )
{
    /*// If this object has a parent bone, the parent should be
    // adjusted instead of moving the child. Otherwise, move as normal.
    if ( mParentNode && mParentNode->queryObjectType( RTID_BoneObject ) )
    {
        cgBoneNode * parentBone = (cgBoneNode*)mParentNode;
        
        // Compute new "look at" direction for parent to this bone's newly proposed position.
        cgVector3 tipPosition = parentBone->getPosition();
        tipPosition += parentBone->getDirection() * fabsf(parentBone->getLength());
        tipPosition += amount;
        parentBone->setBoneOrientation( parentBone->getPosition(), tipPosition, parentBone->getYAxis() );
        return;
    
    } // End if parent bone*/

    // Call base class implementation if parent is not a bone.
    cgObjectNode::move( amount );
}

//-----------------------------------------------------------------------------
// Name : moveLocal() (Virtual)
/// <summary>Move the object by the specified amount in "local" space.</summary>
//-----------------------------------------------------------------------------
void cgBoneNode::moveLocal( const cgVector3 & amount )
{
    /*// If this object has a parent bone, the parent should be
    // adjusted instead of moving the child. Otherwise, move as normal.
    if ( mParentNode && mParentNode->queryObjectType( RTID_BoneObject ) )
    {
        cgBoneNode * parentBone = (cgBoneNode*)mParentNode;

        // Compute new position for the end of the parent bone
        cgVector3 tipPosition = parentBone->getPosition() + (parentBone->getDirection() * fabsf(parentBone->getLength()));
        tipPosition += getXAxis() * amount.x;
        tipPosition += getYAxis() * amount.y;
        tipPosition += getZAxis() * amount.z;
        
        // Adjust parent.
        parentBone->setBoneOrientation( parentBone->getPosition(), tipPosition, parentBone->getYAxis() );
        return;
    
    } // End if parent bone*/
    
    // Call base class implementation if parent is not a bone.
    cgObjectNode::moveLocal( amount );
}

//-----------------------------------------------------------------------------
// Name : generateCollisionShape()
/// <summary>
/// Automatically generate an initial collision shape that optimally surrounds 
/// the bone's influence of the specified skin's vertex data as it exists in 
/// the original reference pose. Returns false if no dimensions could be 
/// computed because no vertices could be found that were influenced by this 
/// bone.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::generateCollisionShape( cgMesh * mesh, cgUInt32 boneIndex /* = cgUInt32(-1) */ )
{
    // Find the binding data for this bone unless one was supplied.
    cgSkinBindData * bindData = mesh->getSkinBindData(); 
    const cgSkinBindData::BoneArray & boneData = bindData->getBones();
    for ( cgUInt32 i = 0; i < boneData.size() && (boneIndex == cgUInt32(-1)); ++i )
    {
        if ( boneData[i]->boneIdentifier == getInstanceIdentifier() )
            boneIndex = i;
    
    } // Next bone

    // Any binding data found for this node?
    if ( boneIndex == cgUInt32(-1) )
        return false;
    
    // Get the vertex data we need from the mesh.
    cgVertexFormat * format         = mesh->getVertexFormat();
    size_t           positionOffset = (size_t)format->getElementOffset( D3DDECLUSAGE_POSITION );
    size_t           weightOffset   = (size_t)format->getElementOffset( D3DDECLUSAGE_BLENDWEIGHT );
    size_t           indicesOffset  = (size_t)format->getElementOffset( D3DDECLUSAGE_BLENDINDICES );
    size_t           vertexStride   = (size_t)format->getStride();
    cgByte         * vertices       = mesh->getSystemVB();

    // Format must have at least a position component
    if ( !vertices || positionOffset == size_t(-1) )
        return false;

    // If there are influences available in the skin binding data, use those.
    // Otherwise build a list of vertices that are influenced by this bone.
    cgSkinBindData::VertexInfluenceArray customInfluences;
    const cgSkinBindData::VertexInfluenceArray * finalInfluences;
    const cgSkinBindData::BoneInfluence & bone = *boneData[boneIndex];
    if ( bone.influences.empty() )
    {
        // Format must have all required components
        if ( weightOffset == size_t(-1) || indicesOffset == size_t(-1) )
            return false;

        // Use the custom influences list.
        finalInfluences = &customInfluences;

        // Search through each bone palette to get influence data.
        const cgMesh::BonePaletteArray & bonePalettes = mesh->getBonePalettes();
        for ( size_t i = 0; i < bonePalettes.size(); ++i )
        {
            cgBonePalette * palette = bonePalettes[i];

            // Find the subset associated with this bone palette.
            const cgMesh::MeshSubset * subset = mesh->getSubset( palette->getMaterial(), palette->getDataGroup() );
            if ( !subset )
                continue;

            // Process faces in this subset to retrieve referenced vertex data.
            cgUInt32 * indices = mesh->getSystemIB();
            cgByte * vertices = mesh->getSystemVB();
            cgInt32 maxBlendIndex = palette->getMaximumBlendIndex();
            const cgUInt32Array & boneReferences = palette->getBones();
            for ( cgInt32 j = subset->faceStart; j < (subset->faceStart + subset->faceCount); ++j )
            {
                for ( size_t k = 0; k < 3; ++k )
                {
                    cgInt vertexIndex = indices[j*3+k];
                    cgFloat * blendWeights = (cgFloat*)(vertices + weightOffset + vertexIndex * vertexStride);
                    cgByte  * blendIndices = (cgByte*)(vertices + indicesOffset + vertexIndex * vertexStride);

                    // Search the blend indices for data matching this bone.
                    for ( cgInt32 l = 0; l <= maxBlendIndex; ++l )
                    {
                        if ( blendIndices[l] != 0xFF && boneReferences[blendIndices[l]] == boneIndex )
                            customInfluences.push_back( cgSkinBindData::VertexInfluence( vertexIndex, blendWeights[l] ) );

                    } // Next blend reference

                } // Next triangle index

            } // Next face

        } // Next Palette

    } // End if no influences
    else
    {
        // Use the original influences list.
        finalInfluences = &bone.influences;

    } // End if influences exist

    // Process the influences for this bone and collect the bounding box.
    cgBoundingBox bounds;
    for ( size_t i = 0; i < finalInfluences->size(); ++i )
    {
        // Retrieve the position of this vertex in it's correct reference pose
        // and test it against the frame bounding box and update if necessary
        cgVector3 position = *(cgVector3*)(vertices + positionOffset + finalInfluences->at(i).vertexIndex * vertexStride);
        cgVector3::transformCoord( position, position, bone.bindPoseTransform );
        bounds.addPoint( position );

    } // Next influence

    // If we found data, generate an appropriate collision shape
    // to surround the influenced geometry.
    if ( bounds.isPopulated() )
    {
        // Delete any existing collision shapes.
        const cgObjectSubElementArray & collisionShapes = getSubElements( OSECID_CollisionShapes );
        if ( !collisionShapes.empty() )
            deleteSubElements( collisionShapes );

        // Generate a new shape
        cgCollisionShapeElement * collisionShape = (cgCollisionShapeElement*)createSubElement( OSECID_CollisionShapes, RTID_CapsuleCollisionShapeElement );
        if ( collisionShape )
            collisionShape->fitToBounds( bounds, cgCollisionShapeElement::XAxis ); // Bones are aligned to the X axis.

        // We computed shape
        return true;

    } // End if data

    // No dimensions were computed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::getSubElementCategories( cgObjectSubElementCategory::Map & categoriesOut ) const
{
    // Ask the referenced object for its list of supported sub-elements.
    if ( !mReferencedObject->getSubElementCategories( categoriesOut ) )
        return false;

    // Override base node functionality, and DO NOT remove collision shape
    // categories, even if no physics model is selected.

    // Any sub-elements remaining?
    return (!categoriesOut.empty());
}

//-----------------------------------------------------------------------------
// Name : buildPhysicsBody ( ) (Virtual, Protected)
/// <summary>
/// Construct the internal physics body designed to represent this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::buildPhysicsBody( )
{
    // Override 'None' case for bones if we should build a collision volume.
    if ( isCollisionEnabled() && mPhysicsModel == cgPhysicsModel::None )
    {
        // Release the previous physics body (if any).
        if ( mPhysicsBody )
        {
            mPhysicsBody->unregisterEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );
            mPhysicsBody->removeReference( this );
            mPhysicsBody = CG_NULL;
        
        } // End if exists

        // Search through collision shapes in the referenced object if any.
        const cgObjectSubElementArray & objectShapes = getSubElements( OSECID_CollisionShapes );

        // Iterate through the shape sub elements and construct a list
        // of matching physics shape objects.
        std::vector<cgPhysicsShape*> physicsShapes;
        for ( size_t i = 0; i < objectShapes.size(); ++i )
        {
            cgPhysicsShape * shape = ((cgCollisionShapeElement*)objectShapes[i])->generatePhysicsShape( mParentScene->getPhysicsWorld() );
            if ( shape )
                physicsShapes.push_back( shape );

        } // Next sub-element

        // If there are no shapes, just bail.
        if ( physicsShapes.empty() )
            return;
        
        // If there was more than one shape, we need to build a compound shape.
        // If there were *no* shapes however, assign a 'null' shape that allows 
        // the object to act like a rigid body without any collision geometry.
        cgPhysicsShape * primaryShape = CG_NULL;
        if ( physicsShapes.size() > 1 )
        {
            cgToDoAssert( "Physics", "Compound shape!" );
            for ( size_t i = 0; i < physicsShapes.size(); ++i )
                physicsShapes[i]->deleteReference();
            mPhysicsBody = CG_NULL;
            return;
        
        } // End if needs compound
        else
        {
            primaryShape = physicsShapes[0];
        
        } // End if single shape

        // Configure new rigid body.
        cgRigidBodyCreateParams cp;
        cp.model            = cgPhysicsModel::RigidStatic;
        cp.initialTransform = getWorldTransform(false);
        cp.quality          = cgSimulationQuality::Default;
        cp.mass             = 0; // IMPORTANT -- STATIC NON COLLIDABLE

        // Construct a new rigid body object. Copy any necessary
        // simulation details from the existing rigid body such as
        // current velocity, torque, etc.
        cgRigidBody * rigidBody = new cgRigidBody( mParentScene->getPhysicsWorld(), primaryShape, cp, mPhysicsBody );

        // Take ownership of the new rigid body.
        mPhysicsBody = rigidBody;
        mPhysicsBody->addReference( this );

        // Assign to the 'cast only' material group so that it cannot collide with anything else.
        mPhysicsBody->setMaterialGroupId( cgDefaultPhysicsMaterialGroup::CastOnly );

    } // End if no collision
    else
    {
        // Fall through to base class implementation
        cgObjectNode::buildPhysicsBody( );

    } // End if other model

}

//-----------------------------------------------------------------------------
//  Name : isCollisionEnabled()
/// <summary>
/// Determine if the collision volume is enabled for this bone for the purposes
/// of ray testing, etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::isCollisionEnabled( ) const
{
    return mCollisionEnabled;
}

//-----------------------------------------------------------------------------
//  Name : enableCollision()
/// <summary>
/// Enable / disable the collision volume for this bone for the purposes
/// of ray testing, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoneNode::enableCollision( bool enable )
{
    // Is this a no-op?
    if ( mCollisionEnabled == enable )
        return;

    // Update local member and then rebuild physics body if necessary.
    mCollisionEnabled = enable;
    buildPhysicsBody();
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoneNode::onNodeInit( const cgUInt32IndexMap & nodeReferenceRemap )
{
    // Collision initially enabled?
    mCollisionEnabled = getInitialCollisionState();

    // Call base class implementation.
    return cgObjectNode::onNodeInit( nodeReferenceRemap );
}