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
// Name : cgCollisionShapeElement.cpp                                        //
//                                                                           //
// Desc : Class that provides a collision shape primitive exposed as an      //
//        object sub-element. This provides the integration between the      //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCollisionShapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCollisionShapeElement.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgCollisionShapeElement::mInsertBaseCollisionShape;
cgWorldQuery cgCollisionShapeElement::mUpdateTransformAndBounds;
cgWorldQuery cgCollisionShapeElement::mLoadBaseCollisionShape;

///////////////////////////////////////////////////////////////////////////////
// cgCollisionShapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCollisionShapeElement::cgCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgObjectSubElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCollisionShapeElement::cgCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgObjectSubElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
    cgCollisionShapeElement * pElement = (cgCollisionShapeElement*)pInit;
    mBounds        = pElement->mBounds;
    mTransform     = pElement->mTransform;
}

//-----------------------------------------------------------------------------
//  Name : ~cgCollisionShapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCollisionShapeElement::~cgCollisionShapeElement()
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
void cgCollisionShapeElement::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    mSandboxMesh.close();
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgObjectSubElement::dispose( true );
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
bool cgCollisionShapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CollisionShapeElement )
        return true;

    // Supported by base?
    return cgObjectSubElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollisionShapeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( ) )
        return false;

    // Automatically fit to parent object initially unless
    // we've already been initialized (i.e. cloned?)
    if ( !mBounds.isPopulated() )
        autoFit( YAxis );
    
    // Call base class implementation last.
    return cgObjectSubElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollisionShapeElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("CollisionShapeElement::insertComponentData") );

        // Decompose the offset transform for storage.
        cgQuaternion qRotation;
        cgVector3 vScale, vShear, vPosition;
        mTransform.decompose( vScale, vShear, qRotation, vPosition );

        // Update database.
        prepareQueries();
        mInsertBaseCollisionShape.bindParameter( 1, mReferenceId );
        mInsertBaseCollisionShape.bindParameter( 2, vPosition.x );
        mInsertBaseCollisionShape.bindParameter( 3, vPosition.y );
        mInsertBaseCollisionShape.bindParameter( 4, vPosition.z );
        mInsertBaseCollisionShape.bindParameter( 5, qRotation.x );
        mInsertBaseCollisionShape.bindParameter( 6, qRotation.y );
        mInsertBaseCollisionShape.bindParameter( 7, qRotation.z );
        mInsertBaseCollisionShape.bindParameter( 8, qRotation.w );
        mInsertBaseCollisionShape.bindParameter( 9, vShear.x );
        mInsertBaseCollisionShape.bindParameter( 10, vShear.y );
        mInsertBaseCollisionShape.bindParameter( 11, vShear.z );
        mInsertBaseCollisionShape.bindParameter( 12, vScale.x );
        mInsertBaseCollisionShape.bindParameter( 13, vScale.y );
        mInsertBaseCollisionShape.bindParameter( 14, vScale.z );
        mInsertBaseCollisionShape.bindParameter( 15, mBounds.min.x );
        mInsertBaseCollisionShape.bindParameter( 16, mBounds.min.y );
        mInsertBaseCollisionShape.bindParameter( 17, mBounds.min.z );
        mInsertBaseCollisionShape.bindParameter( 18, mBounds.max.x );
        mInsertBaseCollisionShape.bindParameter( 19, mBounds.max.y );
        mInsertBaseCollisionShape.bindParameter( 20, mBounds.max.z );
        
        // Execute
        if ( mInsertBaseCollisionShape.step( true ) == false )
        {
            cgString strError;
            mInsertBaseCollisionShape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert collision shape sub-element '0x%x' base data into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("CollisionShapeElement::insertComponentData") );
            return false;
        
        } // End if failed
        
        // Commit changes
        mWorld->commitTransaction( _T("CollisionShapeElement::insertComponentData") );

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
bool cgCollisionShapeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the base collision shape data.
    prepareQueries();
    mLoadBaseCollisionShape.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBaseCollisionShape.step( ) || !mLoadBaseCollisionShape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadBaseCollisionShape.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadBaseCollisionShape.reset();
        return false;
    
    } // End if failed
    
    // Update our local members
    cgQuaternion qRotation;
    cgVector3 vScale, vShear, vPosition;
    mLoadBaseCollisionShape.getColumn( _T("MinBoundsX"), mBounds.min.x );
    mLoadBaseCollisionShape.getColumn( _T("MinBoundsY"), mBounds.min.y );
    mLoadBaseCollisionShape.getColumn( _T("MinBoundsZ"), mBounds.min.z );
    mLoadBaseCollisionShape.getColumn( _T("MaxBoundsX"), mBounds.max.x );
    mLoadBaseCollisionShape.getColumn( _T("MaxBoundsY"), mBounds.max.y );
    mLoadBaseCollisionShape.getColumn( _T("MaxBoundsZ"), mBounds.max.z );
    mLoadBaseCollisionShape.getColumn( _T("OffsetPositionX"), vPosition.x );
    mLoadBaseCollisionShape.getColumn( _T("OffsetPositionY"), vPosition.y );
    mLoadBaseCollisionShape.getColumn( _T("OffsetPositionZ"), vPosition.z );
    mLoadBaseCollisionShape.getColumn( _T("OffsetRotationX"), qRotation.x );
    mLoadBaseCollisionShape.getColumn( _T("OffsetRotationY"), qRotation.y );
    mLoadBaseCollisionShape.getColumn( _T("OffsetRotationZ"), qRotation.z );
    mLoadBaseCollisionShape.getColumn( _T("OffsetRotationW"), qRotation.w );
    mLoadBaseCollisionShape.getColumn( _T("OffsetShearXY"), vShear.x );
    mLoadBaseCollisionShape.getColumn( _T("OffsetShearXZ"), vShear.y );
    mLoadBaseCollisionShape.getColumn( _T("OffsetShearYZ"), vShear.z );
    mLoadBaseCollisionShape.getColumn( _T("OffsetScaleX"), vScale.x );
    mLoadBaseCollisionShape.getColumn( _T("OffsetScaleY"), vScale.y );
    mLoadBaseCollisionShape.getColumn( _T("OffsetScaleZ"), vScale.z );
    mLoadBaseCollisionShape.reset();

    // Recompose the offset transform.
    mTransform.compose( vScale, vShear, qRotation, vPosition );

    // Call base class implementation to read remaining data.
    if ( !cgObjectSubElement::onComponentLoading( e ) )
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
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollisionShapeElement::createTypeTables( const cgUID & TypeIdentifier )
{
    // Ensure this base class table is created first.
    if ( cgObjectSubElement::createTypeTables( RTID_CollisionShapeElement ) == false )
        return false;

    // Then allow derived type to create as normal.
    return cgObjectSubElement::createTypeTables( TypeIdentifier );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollisionShapeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBaseCollisionShape.isPrepared() )
        {
            cgString sSQL = _T("INSERT INTO 'ObjectSubElements::Base::CollisionShape' VALUES(?1,?2,?3,?4,?5,?6,?7,")
                            _T("?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20)");
            mInsertBaseCollisionShape.prepare( mWorld, sSQL, true );
        
        } // End if !prepared
        if ( !mUpdateTransformAndBounds.isPrepared() )
        {
            cgString sSQL= _T("UPDATE 'ObjectSubElements::Base::CollisionShape' SET OffsetPositionX=?1,OffsetPositionY=?2,")
                           _T("OffsetPositionZ=?3,OffsetRotationX=?4,OffsetRotationY=?5,OffsetRotationZ=?6,OffsetRotationW=?7,")
                           _T("OffsetShearXY=?8,OffsetShearXZ=?9,OffsetShearYZ=?10,OffsetScaleX=?11,OffsetScaleY=?12,")
                           _T("OffsetScaleZ=?13,MinBoundsX=?14,MinBoundsY=?15,MinBoundsZ=?16,MaxBoundsX=?17,MaxBoundsY=?18,")
                           _T("MaxBoundsZ=?19 WHERE RefId=?20");
            mUpdateTransformAndBounds.prepare( mWorld, sSQL, true );

        } // End if !prepared
        
    } // End if sandbox

    // Read queries
    if ( mLoadBaseCollisionShape.isPrepared() == false )
        mLoadBaseCollisionShape.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::Base::CollisionShape' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getElementCategory () (Virtual)
/// <summary>
/// Retrieve the unique identifier for the sub-element category to which this
/// type belongs.
/// </summary>
//-----------------------------------------------------------------------------
const cgUID & cgCollisionShapeElement::getElementCategory( ) const
{
    return OSECID_CollisionShapes;
}

//-----------------------------------------------------------------------------
//  Name : getShapeBoundingBox()
/// <summary>
/// Retrieve the local space bounding box of this collision shape.
/// </summary>
//-----------------------------------------------------------------------------
const cgBoundingBox & cgCollisionShapeElement::getShapeBoundingBox( ) const
{
    return mBounds;
}

//-----------------------------------------------------------------------------
//  Name : getTransform()
/// <summary>
/// Retrieve the parent relative (or offset) transform for this shape.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgCollisionShapeElement::getTransform( ) const
{
    return mTransform;
}

//-----------------------------------------------------------------------------
//  Name : setTransform()
/// <summary>
/// Update the parent relative (or offset) transform for this shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollisionShapeElement::setTransform( const cgTransform & t )
{
    if ( shouldSerialize() )
    {
        // Decompose the transform for storage.
        cgQuaternion qRotation;
        cgVector3 vScale, vShear, vPosition;
        t.decompose( vScale, vShear, qRotation, vPosition );

        // Update database.
        prepareQueries();
        mUpdateTransformAndBounds.bindParameter( 1, vPosition.x );
        mUpdateTransformAndBounds.bindParameter( 2, vPosition.y );
        mUpdateTransformAndBounds.bindParameter( 3, vPosition.z );
        mUpdateTransformAndBounds.bindParameter( 4, qRotation.x );
        mUpdateTransformAndBounds.bindParameter( 5, qRotation.y );
        mUpdateTransformAndBounds.bindParameter( 6, qRotation.z );
        mUpdateTransformAndBounds.bindParameter( 7, qRotation.w );
        mUpdateTransformAndBounds.bindParameter( 8, vShear.x );
        mUpdateTransformAndBounds.bindParameter( 9, vShear.y );
        mUpdateTransformAndBounds.bindParameter( 10, vShear.z );
        mUpdateTransformAndBounds.bindParameter( 11, vScale.x );
        mUpdateTransformAndBounds.bindParameter( 12, vScale.y );
        mUpdateTransformAndBounds.bindParameter( 13, vScale.z );
        mUpdateTransformAndBounds.bindParameter( 14, mBounds.min.x );
        mUpdateTransformAndBounds.bindParameter( 15, mBounds.min.y );
        mUpdateTransformAndBounds.bindParameter( 16, mBounds.min.z );
        mUpdateTransformAndBounds.bindParameter( 17, mBounds.max.x );
        mUpdateTransformAndBounds.bindParameter( 18, mBounds.max.y );
        mUpdateTransformAndBounds.bindParameter( 19, mBounds.max.z );
        mUpdateTransformAndBounds.bindParameter( 20, mReferenceId );
        if ( !mUpdateTransformAndBounds.step( true ) )
        {
            cgString strError;
            mUpdateTransformAndBounds.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update offset transform and bounds for collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if serialize

    // Update local member
    mTransform = t;

    // Shear is not supported for collision shape transforms.
    mTransform.setLocalShear( 0, 0, 0 );
    
    // Notify all parent objects that the shape was was modified.
    ReferenceMap::iterator itRef;
    static const cgString strContext = _T("CollisionShapeModified");
    for ( itRef = mReferencedBy.begin(); itRef != mReferencedBy.end(); ++itRef )
    {
        if ( itRef->second.reference->queryReferenceType( RTID_WorldObject ) )
            ((cgWorldObject*)itRef->second.reference)->onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    } // Next reference holder
}

//-----------------------------------------------------------------------------
//  Name : applyElementRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this element. For instance,
/// in the case of a collision shape, its dimensions will be scaled.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollisionShapeElement::applyElementRescale( cgFloat fScale )
{
    // Scale the bounds.
    mBounds *= fScale;

    // Scale the offset transform.
    cgTransform NewTransform = mTransform;
    NewTransform.position() *= fScale;
    
    // Pass through to regular 'setTransform()' method to allow
    // it to perform any additional actions required.
    setTransform( NewTransform );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the sub element to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment assuming
/// element editing for this type is active.
/// </summary>
//-----------------------------------------------------------------------------
void cgCollisionShapeElement::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // Get access to required systems.
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Generate the sandbox mesh as necessary.
    if ( !mSandboxMesh.isValid() )
    {
        if ( !createSandboxMesh() )
            return;
    
    } // End if no mesh

    // Retrieve the underlying rendering resources if available
    cgMesh * pMesh = mSandboxMesh.getResource(true);
    if ( !pMesh || !pMesh->isLoaded() )
    {
        cgCollisionShapeElement::sandboxRender( pCamera, pVisData, bWireframe, GridPlane, pIssuer );
        return;
    
    } // End if no mesh

    // Setup constants.
    cgColorValue InteriorColor = (mSelected) ? cgColorValue( 0.7f, 0.2f, 0.1f, 0.4f ) : cgColorValue( 0.3f, 0.4f, 0.5f, 0.4f );
    cgColorValue WireColor = (mSelected) ? 0xFFFFF600 : 0xFF99CCE5;
    cgConstantBuffer * pConstants = pDriver->getSandboxConstantBuffer().getResource( true );
    pConstants->setVector( _T("shapeInteriorColor"), (cgVector4&)InteriorColor );
    pConstants->setVector( _T("shapeWireColor"), (cgVector4&)WireColor );
    pDriver->setConstantBufferAuto( pDriver->getSandboxConstantBuffer() );

    // Setup world transform.
    cgTransform ShapeTransform;
    computeShapeTransform( pIssuer, ShapeTransform );
    pDriver->setWorldTransform( ShapeTransform );

    // Execute technique.
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);
    if ( pShader->beginTechnique( _T("drawGhostedShapeMesh") ) )
    {
        while ( pShader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            pMesh->draw( cgMeshDrawMode::Simple );
        pShader->endTechnique();
    
    } // End if success
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified world space ray, determine if this collision shape is 
/// intersected and also compute the world space intersection distance.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCollisionShapeElement::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Broadphase test -- OOBB.
    // Retrieve the final world transformation for this shape
    // relative to the issuing node. Include the relevant shape offset 
    // transformation.
    cgTransform ObjectTransform = mTransform * pIssuer->getWorldTransform( false );

    // First transform ray to object space for picking.
    cgTransform InverseObjectTransform;
    cgVector3 vObjectOrigin, vObjectDir;
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    InverseObjectTransform.transformCoord( vObjectOrigin, vOrigin );
    InverseObjectTransform.transformNormal( vObjectDir, vDir );

    // Direction needs to be normalized in case node matrix contained scale
    cgVector3::normalize( vObjectDir, vObjectDir );

    // Perform ray->aabb intersection.
    if ( mBounds.intersect( vObjectOrigin, vObjectDir, fDistance, false ) )
    {
        // Intersects shape outer bounds.
        // Narrow-phase test -- mesh geometry.
        if ( !mSandboxMesh.isValid() )
            createSandboxMesh();
            
        // If no mesh was available, just return the OOBB intersection.
        cgMesh * pMesh = mSandboxMesh.getResource(true);
        if ( !pMesh || !pMesh->isLoaded() )
        {
            // Compute final object space intersection point.
            cgVector3 vIntersect = vObjectOrigin + (vObjectDir * fDistance);

            // Transform intersection point back into world space to compute
            // the final intersection distance.
            ObjectTransform.transformCoord( vIntersect, vIntersect );
            fDistance = cgVector3::length( vIntersect - vOrigin );
            return true;

        } // End if no mesh

        // Perform mesh picking. First retrieve the final transformation
        // matrix for the mesh based shape (same one used for rendering).
        // This matrix may be different than the original one used for the
        // broadphase since the mesh itself may be unit sized (for example).
        computeShapeTransform( pIssuer, ObjectTransform );

        // Transform ray to object space for picking.
        cgTransform::inverse( InverseObjectTransform, ObjectTransform );
        InverseObjectTransform.transformCoord( vObjectOrigin, vOrigin );
        InverseObjectTransform.transformNormal( vObjectDir, vDir );

        // Direction needs to be normalized in case node matrix contained scale
        cgVector3::normalize( vObjectDir, vObjectDir );

        // Pick mesh!
        if ( pMesh->pick( vObjectOrigin, vObjectDir, false, cgVector3(0,0,0), fDistance ) )
        {
            // Intersects mesh.
            // Compute final object space intersection point.
            cgVector3 vIntersect = vObjectOrigin + (vObjectDir * fDistance);

            // Transform intersection point back into world space to compute
            // the final intersection distance.
            ObjectTransform.transformCoord( vIntersect, vIntersect );
            fDistance = cgVector3::length( vIntersect - vOrigin );

            // Intersected
            return true;

        } // End if intersect mesh

    } // End if intersect OOBB

    // No intersection recorded.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getAutoFitBounds ( )
/// <summary>
/// Compute the best guess at the parent object's bounding box for the purposes
/// of auto-fitting the collision shape.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgCollisionShapeElement::getAutoFitBounds( )
{
    cgBoundingBox ParentBounds = mParentObject->getLocalBoundingBox();

    // If the bounding box is degenerate or invalid, it's possible that
    // the object is unable to supply one -- perhaps because it is dynamically
    // computed based on some information in its *node(s)*. If this is found
    // to be the case, we'll test some of the referencing nodes next.
    if ( ParentBounds.isDegenerate() || !ParentBounds.isPopulated() )
    {
        cgBoundingBox FirstValidBounds;
        bool          bFoundValid = false;

        // Search through referencing nodes. For sandbox mode, we'll
        // prefer a node which is currently selected. Otherwise, we'll
        // keep searching until we find a valid bounding box.
        cgReference::ReferenceMap::const_iterator itReference;
        const cgReference::ReferenceMap & ReferenceHolders = mParentObject->getReferenceHolders( );
        for ( itReference = ReferenceHolders.begin(); itReference != ReferenceHolders.end(); ++itReference )
        {
            cgReference * pReference = itReference->second.reference;
            
            // Is this a node?
            if ( pReference->queryReferenceType( RTID_ObjectNode ) )
            {
                cgObjectNode * pNode = (cgObjectNode*)pReference;

                // This *is* a node. Get its bounding box.
                ParentBounds = pNode->getLocalBoundingBox();

                // Keep searching if the bounding box is still invalid.
                if ( ParentBounds.isDegenerate() || !ParentBounds.isPopulated() )
                    continue;

                // In sandbox mode, keep searching if this node isn't selected
                // but keep track of the first valid bounding box we find
                // in case *nothing* is currently selected. Otherwise, break out.
                if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
                {
                    if ( !bFoundValid )
                    {
                        FirstValidBounds = ParentBounds;
                        bFoundValid = true;
                    
                    } // End if first found
                    if ( pNode->isSelected() && !pNode->isMergedAsGroup() )
                    {
                        // Overwrite whichever bounding box we may have previously
                        // found with this one and exit out of the search.
                        FirstValidBounds = ParentBounds;
                        bFoundValid = true;
                        break;

                    } // End if selected

                } // End if sandbox
                else
                    break;

            } // End if node

        } // Next reference holder

        // In sandbox mode we had a more involved search. Use the
        // bounding box that was selected.
        if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && bFoundValid )
            ParentBounds = FirstValidBounds;

        // If we still don't have a valid bounding box, just use a default size
        // of 1m cubed.
        if ( ParentBounds.isDegenerate() || !ParentBounds.isPopulated() )
            ParentBounds = cgBoundingBox( -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f );
        
    } // End if bounds invalid

    // Return selected bounds.
    return ParentBounds;
}