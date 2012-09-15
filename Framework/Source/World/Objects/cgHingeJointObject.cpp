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
// Name : cgHingeJointObject.cpp                                             //
//                                                                           //
// Desc : Class implementing a single axis hinge joint. This joint can be    //
//        used to connect two bodies around a central axis and allow them    //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHingeJointObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgHingeJointObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgMeshObject.h>
#include <World/cgScene.h>
#include <Physics/Joints/cgHingeJoint.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>

// ToDo: 6767 -- needs database updates.

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgHingeJointObject::mInsertJoint;
cgWorldQuery cgHingeJointObject::mLoadJoint;
cgWorldQuery cgHingeJointObject::mUpdateLimits;
cgWorldQuery cgHingeJointNode::mInsertInstanceData;
cgWorldQuery cgHingeJointNode::mDeleteInstanceData;
cgWorldQuery cgHingeJointNode::mLoadInstanceData;

///////////////////////////////////////////////////////////////////////////////
// cgHingeJointObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHingeJointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::cgHingeJointObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgJointObject( nReferenceId, pWorld )
{
    mUseLimits      = true;
    mMinimumAngle   = -45.0f;
    mMaximumAngle   = 45.0f;
    mInitialAngle   = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgHingeJointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::cgHingeJointObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgJointObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgHingeJointObject * pObject = (cgHingeJointObject*)pInit;
    mUseLimits      = pObject->mUseLimits;
    mMinimumAngle   = pObject->mMinimumAngle;
    mMaximumAngle   = pObject->mMaximumAngle;
    mInitialAngle   = pObject->mInitialAngle;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHingeJointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::~cgHingeJointObject()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgHingeJointObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgHingeJointObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgHingeJointObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgHingeJointObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgHingeJointObject::getLocalBoundingBox( )
{
    // Set to current position by default
    return cgBoundingBox( 0,0,0,0,0,0 ); 
}

//-----------------------------------------------------------------------------
//  Name : enableLimits ()
/// <summary>
/// Enable angle of rotation limiting for this hinge joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::enableLimits( bool enabled )
{
    // Is this a no-op?
    if ( mUseLimits == enabled )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateLimits.bindParameter( 1, enabled );
        mUpdateLimits.bindParameter( 2, mMinimumAngle );
        mUpdateLimits.bindParameter( 3, mMaximumAngle );
        mUpdateLimits.bindParameter( 4, mReferenceId );

        // Execute
        if ( mUpdateLimits.step( true ) == false )
        {
            cgString strError;
            mUpdateLimits.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update limit configuration for hinge joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mUseLimits = enabled;

    // Notify listeners that property was altered
    static const cgString strContext = _T("UseLimits");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setLimits ()
/// <summary>
/// Set the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::setLimits( const cgRangeF & range )
{
    setLimits( range.min, range.max );
}

//-----------------------------------------------------------------------------
//  Name : setLimits ()
/// <summary>
/// Set the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::setLimits( cgFloat minDegrees, cgFloat maxDegrees )
{
    // Is this a no-op?
    if ( minDegrees == mMinimumAngle && maxDegrees == mMaximumAngle )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateLimits.bindParameter( 1, mUseLimits );
        mUpdateLimits.bindParameter( 2, minDegrees );
        mUpdateLimits.bindParameter( 3, maxDegrees );
        mUpdateLimits.bindParameter( 4, mReferenceId );

        // Execute
        if ( mUpdateLimits.step( true ) == false )
        {
            cgString strError;
            mUpdateLimits.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update limit configuration for hinge joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update local members
    mMinimumAngle = minDegrees;
    mMaximumAngle = maxDegrees;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("LimitRange");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : isLimited ()
/// <summary>
/// Is angle of rotation limiting for this hinge joint enabled?
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::isLimited( ) const
{
    return mUseLimits;
}

//-----------------------------------------------------------------------------
//  Name : getLimits ()
/// <summary>
/// Get the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgRangeF cgHingeJointObject::getLimits( ) const
{
    return cgRangeF(mMinimumAngle,mMaximumAngle);
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMesh() )
            return false;

    } // End if no mesh

    // Retrieve the underlying mesh resource and pick if available
    cgMesh * mesh = mSandboxMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return false;

    // Transform ray based on the scaling of the mesh itself.
    cgVector3 meshRayOrigin, meshRayDirection;
    cgRenderDriver * driver = mWorld->getRenderDriver();
    cgFloat zoomFactor = camera->estimateZoomFactor( viewportSize, issuer->getPosition( false ), 0.05f );
    cgTransform t, inverseT = issuer->getWorldTransform(false);
    t.rotateLocal( CGEToRadian(90.0f), CGEToRadian(90.0f), 0 );
    t.scaleLocal( zoomFactor, zoomFactor, zoomFactor );
    cgTransform::inverse( inverseT, t );
    inverseT.transformCoord( meshRayOrigin, rayOrigin );
    inverseT.transformNormal( meshRayDirection, rayDirection );
    cgVector3::normalize( meshRayDirection, meshRayDirection );

    // Pass through
    bool result = mesh->pick( meshRayOrigin, meshRayDirection, wireframe, wireTolerance, distance );

    // Scale distance back out
    if ( result )
    {
        cgVector3 intersection = meshRayOrigin + meshRayDirection * distance;
        t.transformCoord( intersection, intersection );
        distance = cgVector3::length( intersection - rayOrigin );

    } // End if intersected

    // Return result
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::sandboxRender( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // Get access to required systems.
    cgRenderDriver * driver = mWorld->getRenderDriver();

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMesh() )
            return;

    } // End if no mesh

    // Retrieve the underlying rendering resources if available
    cgMesh * mesh = mSandboxMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
    {
        cgJointObject::sandboxRender( camera, visibilityData, wireframe, gridPlane, issuer );
        return;

    } // End if no mesh

    // Setup constants.
    cgColorValue interiorColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0.7f, 0.2f, 0.1f, 0.4f );
    interiorColor.a = 0.4f;
    cgColorValue wireColor = (!issuer->isSelected()) ? issuer->getNodeColor() : cgColorValue( 0xFFFFFFFF );
    cgConstantBuffer * constants = driver->getSandboxConstantBuffer().getResource( true );
    constants->setVector( _T("shapeInteriorColor"), (cgVector4&)interiorColor );
    constants->setVector( _T("shapeWireColor"), (cgVector4&)wireColor );
    driver->setConstantBufferAuto( driver->getSandboxConstantBuffer() );

    // Setup world transform
    const cgViewport & viewport = driver->getViewport();
    cgFloat zoomFactor = camera->estimateZoomFactor( viewport.size, issuer->getPosition( false ), 0.05f );
    cgTransform t = issuer->getWorldTransform(false);
    t.rotateLocal( CGEToRadian(90.0f), CGEToRadian(90.0f), 0 );
    t.scaleLocal( zoomFactor, zoomFactor, zoomFactor );
    driver->setWorldTransform( t );

    // Execute technique.
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
    if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
    {
        while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mesh->draw( cgMeshDrawMode::Simple );
        shader->endTechnique();

    } // End if success
    
    // Render the connected nodes if the joint is selected
    if ( issuer->isSelected() )
    {
        // Get the referenced nodes to which the issuer is attached.
        cgHingeJointNode * hingeNode = static_cast<cgHingeJointNode*>(issuer);
        cgObjectNode * body0Node = hingeNode->getBody0Node();
        cgObjectNode * body1Node = hingeNode->getBody1Node();

        // Select new colors.
        constants->setVector( _T("shapeInteriorColor"), (cgVector4&)cgColorValue( 0.1f, 0.2f, 0.7f, 0.4f ) );
        constants->setVector( _T("shapeWireColor"), (cgVector4&)cgColorValue( 0.1f, 0.2f, 0.7f, 1.0f ) );

        // If the first referenced body is a mesh, draw a wireframe representation of it.
        if ( body0Node && body0Node->queryObjectType( RTID_MeshObject ) )
        {
            cgMeshObject * meshObject = static_cast<cgMeshObject*>(body0Node->getReferencedObject() );
            mesh = meshObject->getMesh().getResource(true);
            if ( mesh && mesh->isLoaded() )
            {
                if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
                {
                    driver->setWorldTransform( body0Node->getWorldTransform(false) );
                    while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
                        mesh->draw( cgMeshDrawMode::Simple );
                    shader->endTechnique();

                } // End if success
            
            } // End if valid mesh

        } // End if body0 = mesh

        // Select new colors.
        constants->setVector( _T("shapeInteriorColor"), (cgVector4&)cgColorValue( 0.1f, 0.7f, 0.2f, 0.4f ) );
        constants->setVector( _T("shapeWireColor"), (cgVector4&)cgColorValue( 0.1f, 0.7f, 0.2f, 1.0f ) );

        // If the second referenced body is a mesh, draw a wireframe representation of it.
        if ( body1Node && body1Node->queryObjectType( RTID_MeshObject ) )
        {
            cgMeshObject * meshObject = static_cast<cgMeshObject*>(body1Node->getReferencedObject() );
            mesh = meshObject->getMesh().getResource(true);
            if ( mesh && mesh->isLoaded() )
            {
                if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
                {
                    driver->setWorldTransform( body1Node->getWorldTransform(false) );
                    while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
                        mesh->draw( cgMeshDrawMode::Simple );
                    shader->endTechnique();

                } // End if success

            } // End if valid mesh

        } // End if body1 = mesh 

    } // End if selected

    // Call base class implementation last.
    cgJointObject::sandboxRender( camera, visibilityData, wireframe, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
// Name : createSandboxMesh() (Protected)
/// <summary>
/// Generate the physical representation of this joint for rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::createSandboxMesh( )
{
    // Compute a shape identifier that can be used to find duplicate meshes.
    cgString referenceName = cgString::format( _T("Core::SandboxShapes::HingeJoint") );

    // Does a mesh with this identifier already exist?
    cgResourceManager * resources = mWorld->getResourceManager();
    if ( !resources->getMesh( &mSandboxMesh, referenceName ) )
    {
        // Dispose of any previous mesh generated and
        // allocate a new one.
        mSandboxMesh.close( true );
        cgMesh * newMesh = new cgMesh( 0, CG_NULL );

        // Set the mesh data format and populate it.
        cgResourceManager * resources = mWorld->getResourceManager();
        cgVertexFormat * vertexFormat = cgVertexFormat::formatFromDeclarator( cgVertex::Declarator );
        if ( !newMesh->createCapsule( vertexFormat, 5.0f, 50.0f, 1, 10, false, cgMeshCreateOrigin::Center, true, resources ) )
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
    return mSandboxMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in this implementation

    // Call base class implementation.
    cgJointObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HingeJointObject )
        return true;

    // Supported by base?
    return cgJointObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgHingeJointObject::getDatabaseTable( ) const
{
    return _T("Objects::HingeJoint");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgJointObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("HingeJointObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertJoint.bindParameter( 1, mReferenceId );
        mInsertJoint.bindParameter( 2, mUseLimits );
        mInsertJoint.bindParameter( 3, mMinimumAngle );
        mInsertJoint.bindParameter( 4, mMaximumAngle );
        mInsertJoint.bindParameter( 5, mInitialAngle );
        mInsertJoint.bindParameter( 6, mSoftRefCount );

        // Execute
        if ( mInsertJoint.step( true ) == false )
        {
            cgString strError;
            mInsertJoint.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for hinge joint object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("HingeJointObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("HingeJointObject::insertComponentData") );

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
bool cgHingeJointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the joint data.
    prepareQueries();
    mLoadJoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadJoint.step( ) || !mLoadJoint.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadJoint.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hinge joint object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hinge joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadJoint.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadJoint;

    // Update our local members
    mLoadJoint.getColumn( _T("UseLimits"), mUseLimits );
    mLoadJoint.getColumn( _T("MinimumAngle"), mMinimumAngle );
    mLoadJoint.getColumn( _T("MaximumAngle"), mMaximumAngle );
    mLoadJoint.getColumn( _T("InitialAngle"), mInitialAngle );

    // Call base class implementation to read remaining data.
    if ( !cgJointObject::onComponentLoading( e ) )
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
void cgHingeJointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertJoint.isPrepared() == false )
            mInsertJoint.prepare( mWorld, _T("INSERT INTO 'Objects::HingeJoint' VALUES(?1,?2,?3,?4,?5,?6)"), true );
        if ( mUpdateLimits.isPrepared() == false )
            mUpdateLimits.prepare( mWorld, _T("UPDATE 'Objects::HingeJoint' SET UseLimits=?1, MinimumAngle=?2, MaximumAngle=?3 WHERE RefId=?4"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadJoint.isPrepared() == false )
        mLoadJoint.prepare( mWorld, _T("SELECT * FROM 'Objects::HingeJoint' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgHingeJointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHingeJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::cgHingeJointNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgJointNode( nReferenceId, pScene )
{
    // Initialize members to sensible defaults
    mJoint              = CG_NULL;
    mBody0RefId         = 0;
    mBody1RefId         = 0;
    mColor              = 0xFF22AAFF;
}

//-----------------------------------------------------------------------------
//  Name : cgHingeJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::cgHingeJointNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize members to sensible defaults
    mJoint              = CG_NULL;

    // Duplicate data
    cgHingeJointNode * hingeJoint = (cgHingeJointNode*)pInit;
    mBody0RefId = hingeJoint->mBody0RefId;
    mBody1RefId = hingeJoint->mBody1RefId;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHingeJointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::~cgHingeJointNode()
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
void cgHingeJointNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release internal references.
    if ( mJoint )
        mJoint->removeReference( this );
    mJoint = CG_NULL;
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgJointNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgHingeJointNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgHingeJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HingeJointNode )
        return true;

    // Supported by base?
    return cgJointNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame = _T("Joint");

    // Position icon
    vIconOrigin = getPosition(false);
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vIconOrigin, 0.05f );
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 17.0f * fZoomFactor);
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform()
/// <summary>
/// Update our internal cell matrix with that specified here. Optionally, you
/// can disable the recomputation of the local (parent relative) transformation
/// matrix (defaults to true). This is generally only used internally in order
/// to reduce introducing accumulated errors into the transformation matrix but
/// can be supplied if you know that this does not need to be performed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgJointNode::setCellTransform( Transform, Source ) )
        return false;

    // Update the transform of the underlying hinge joint as we move, unless we're
    // updating due to our attached physics body updates.
    if ( mJoint && Source != cgTransformSource::Dynamics )
        mJoint->setPivotTransform( getWorldTransform(false) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgHingeJointNode::getLocalBoundingBox( )
{
    // Pass through to the default handler.
    return cgJointNode::getLocalBoundingBox();
}

//-----------------------------------------------------------------------------
// Name : rebuildJoint( ) (Protected)
/// <summary>
/// Create the internal joint if all parent/child connections have 
/// been made, or destroy the joint if any are removed.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::rebuildJoint( )
{
    // Destroy any prior joint
    if ( mJoint )
        mJoint->removeReference( this );
    mJoint = CG_NULL;

    // Have all pre-requisites been met with regards child attachments?
    bool validConnections = (mBody0RefId && mBody1RefId);

    // If we have all valid connections, rebuild the joint.
    if ( validConnections )
    {
        cgObjectNode * body0Node = getBody0Node();
        cgObjectNode * body1Node = getBody1Node();
        cgPhysicsBody * body0 = (body0Node) ? body0Node->getPhysicsBody() : CG_NULL;
        cgPhysicsBody * body1 = (body1Node) ? body1Node->getPhysicsBody() : CG_NULL;
        if ( body0 && body1 )
        {
            mJoint = new cgHingeJoint( mParentScene->getPhysicsWorld(), body0, body1, getWorldTransform(false) );
            mJoint->addReference( this );
            mJoint->enableLimits( isLimited() );
            mJoint->setLimits( getLimits().min, getLimits().max );
            mJoint->enableBodyCollision( isBodyCollisionEnabled() );

            // Listen for changes (unregister first just in case we had previously registered).
            cgPhysicsBodyEventListener * listener = static_cast<cgPhysicsBodyEventListener*>(this);
            body0->unregisterEventListener( listener );
            body0->registerEventListener( listener );
            body1->unregisterEventListener( listener );
            body1->registerEventListener( listener );
        
        } // End if has physics bodies
    
    } // End if valid
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgJointNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    cgToDo( "Cloning", "Take care because the references may change in 'onNodeInit()'." )

    // Insert references to attached objects if there are any already defined (i.e. from cloning?)
    // Even if there are not, we should insert a row into the instance data table so that we can
    // simply update it as and one objects are attached / detached.
    if ( shouldSerialize() )
    {
        // Insert entry
        prepareQueries();
        mInsertInstanceData.bindParameter( 1, mReferencedObject->getReferenceId() );
        mInsertInstanceData.bindParameter( 2, mReferenceId );
        mInsertInstanceData.bindParameter( 3, mBody0RefId );
        mInsertInstanceData.bindParameter( 4, mBody1RefId );
        if ( mInsertInstanceData.step( true ) == false )
        {
            cgString error;
            mInsertInstanceData.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert instance data for hinge joint node '0x%x'. Error: %s"), mReferenceId, error.c_str() );
            return false;

        } // End if failed

    } // End if serialize

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeLoading ( ) (Virtual)
/// <summary>
/// Virtual method called when the node is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgJointNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Retrieve the *original* reference identifier we're loading from. We need
    // this so that we can load the instance data for the original node.
    cgUInt32 originalRefId = 0;
    pNodeData->getColumn( _T("RefId"), originalRefId );

    // Load the attached bone list.
    prepareQueries();
    mLoadInstanceData.bindParameter( 1, originalRefId );
    if ( !mLoadInstanceData.step( ) || !mLoadInstanceData.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( mLoadInstanceData.getLastError( error ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve instance data for hinge joint object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve instance data for hinge joint object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadInstanceData.reset();
        return false;

    } // End if failed

    // Get the loaded data.
    mLoadInstanceData.getColumn( _T("Body0Id"), mBody0RefId );
    mLoadInstanceData.getColumn( _T("Body1Id"), mBody1RefId );
    mLoadInstanceData.reset();

    cgToDo( "Spawning", "Insert cloned instance data? Take care because the references may change in 'onNodeInit()'." )

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the  node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeInit( const cgUInt32IndexMap & NodeReferenceRemap )
{
    // Remap object references appropriately.
    if ( !NodeReferenceRemap.empty() )
    {
        cgUInt32IndexMap::const_iterator itRemap = NodeReferenceRemap.find( mBody0RefId );
        if ( itRemap != NodeReferenceRemap.end() )
            mBody0RefId = itRemap->second;
        itRemap = NodeReferenceRemap.find( mBody1RefId );
        if ( itRemap != NodeReferenceRemap.end() )
            mBody1RefId = itRemap->second;

    } // End if remapping

    // Rebuild the joint if possible.
    rebuildJoint();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setBody0 () (Virtual)
/// <summary>
/// Set the reference identifier of the first of two nodes to which this hinge
/// joint will be anchored. In order for the joint to function, both attached
/// nodes must have valid physics bodies.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::setBody0( cgUInt32 nodeReferenceId )
{
    // Do nothing if this is a no-op.
    if ( mBody0RefId == nodeReferenceId )
        return;


    // Update local member
    mBody0RefId = nodeReferenceId;

    // Rebuild the underlying physics joint based on the new node.
    rebuildJoint();
}

//-----------------------------------------------------------------------------
// Name : getBody0 () (Virtual)
/// <summary>
/// Get the reference identifier of the first of two nodes to which this hinge
/// joint will be anchored.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgHingeJointNode::getBody0( ) const
{
    return mBody0RefId;
}

//-----------------------------------------------------------------------------
// Name : getBody0Node ()
/// <summary>
/// Get the object node referenced by the 'Body0' reference identifier set .
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::getBody0Node( ) const
{
    cgReference * refNode = cgReferenceManager::getReference( mBody0RefId );
    if ( refNode && refNode->queryReferenceType( RTID_ObjectNode ) )
        return static_cast<cgObjectNode*>(refNode);
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : setBody1 () (Virtual)
/// <summary>
/// Set the reference identifier of the second of two nodes to which this hinge
/// joint will be anchored. In order for the joint to function, both attached
/// nodes must have valid physics bodies.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::setBody1( cgUInt32 nodeReferenceId )
{
    // Do nothing if this is a no-op.
    if ( mBody1RefId == nodeReferenceId )
        return;


    // Update local member
    mBody1RefId = nodeReferenceId;

    // Rebuild the underlying physics joint based on the new node.
    rebuildJoint();
}

//-----------------------------------------------------------------------------
// Name : getBody1 () (Virtual)
/// <summary>
/// Get the reference identifier of the second of two nodes to which this hinge
/// joint will be anchored.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgHingeJointNode::getBody1( ) const
{
    return mBody1RefId;
}

//-----------------------------------------------------------------------------
// Name : getBody1Node ()
/// <summary>
/// Get the object node referenced by the 'Body1' reference identifier set .
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::getBody1Node( ) const
{
    cgReference * refNode = cgReferenceManager::getReference( mBody1RefId );
    if ( refNode && refNode->queryReferenceType( RTID_ObjectNode ) )
        return static_cast<cgObjectNode*>(refNode);
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// deleted in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeDeleted( )
{
    // Delete attached bone reference list.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mDeleteInstanceData.bindParameter( 1, mReferenceId );
        if ( !mDeleteInstanceData.step( true ) )
        {
            // Log any error.
            cgString error;
            if ( mDeleteInstanceData.getLastError( error ) == false )
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete instance data for hinge joint object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete instance data for hinge joint object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;

        } // End if failed

    } // End if should serialize

    // Call base class implementation last.
    return cgJointNode::onNodeDeleted( );
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // Rebuild the joint whenever a property is modified (whatever it is)
    rebuildJoint();

    // Call base class implementation last
    cgJointNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParentScene->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertInstanceData.isPrepared() )
            mInsertInstanceData.prepare( pWorld, _T("INSERT INTO 'Objects::HingeJoint::InstanceData' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( !mDeleteInstanceData.isPrepared() )
            mDeleteInstanceData.prepare( pWorld, _T("DELETE FROM 'Objects::HingeJoint::InstanceData' WHERE NodeRefId=?1"), true );

    } // End if sandbox

    // Read queries
    if ( !mLoadInstanceData.isPrepared() )
        mLoadInstanceData.prepare( pWorld, _T("SELECT * FROM 'Objects::HingeJoint::InstanceData' WHERE NodeRefId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyTransformed ( )
/// <summary>
/// Triggered whenever the physics body designed represent this object during
/// scene dynamics processing is transformed as a result of its simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e )
{
    // Due to the fact that we registered this node as a listener for events raised by
    // one or both of the attached physics bodies, this method will be triggered whenever 
    // the transform of either of those bodies is modified... /not/ as a result of this 
    // node being updated (since joints cannot be physics bodies).

    // Ignore updates that were not due to dynamics.
    if ( !e->dynamicsUpdate )
        return;

    // Update our transform relative to the joint if the joint's bodys are adjusted.
    if ( mJoint && (sender == mJoint->getBody0() || sender == mJoint->getBody1()) )
        setWorldTransform( mJoint->getPivotTransform( false ), cgTransformSource::Dynamics );
    
    // Call base class implementation.
    cgJointNode::onPhysicsBodyTransformed( sender, e );
}