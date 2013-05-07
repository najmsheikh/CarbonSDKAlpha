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
// Name : cgHullCollisionShapeElement.cpp                                    //
//                                                                           //
// Desc : Class that provides an automatically generated convex hull mesh    //
//        available for use as a collision primitive and exposed as an       //
//        object sub-element. This provides the integration between the      //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHullCollisionShapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgHullCollisionShapeElement.h>
#include <World/cgObjectNode.h>
#include <World/Objects/cgMeshObject.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Physics/Shapes/cgConvexHullShape.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgHullCollisionShapeElement::mInsertHullCollisionShape;
cgWorldQuery cgHullCollisionShapeElement::mUpdateSerializedData;
cgWorldQuery cgHullCollisionShapeElement::mLoadHullCollisionShape;

///////////////////////////////////////////////////////////////////////////////
// cgHullCollisionShapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHullCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHullCollisionShapeElement::cgHullCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgCollisionShapeElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
    mCollapseTolerance    = 0.0f;
    mSerializedDataSource = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgHullCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHullCollisionShapeElement::cgHullCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgCollisionShapeElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
    mSerializedDataSource = 0;

    // Clone values
    cgHullCollisionShapeElement * pElement = ((cgHullCollisionShapeElement*)pInit);
    mCollapseTolerance = pElement->mCollapseTolerance;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHullCollisionShapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHullCollisionShapeElement::~cgHullCollisionShapeElement()
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
void cgHullCollisionShapeElement::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgCollisionShapeElement::dispose( true );
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
bool cgHullCollisionShapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HullCollisionShapeElement )
        return true;

    // Supported by base?
    return cgCollisionShapeElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgHullCollisionShapeElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgHullCollisionShapeElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgHullCollisionShapeElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgHullCollisionShapeElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgHullCollisionShapeElement::getDatabaseTable( ) const
{
    return _T("ObjectSubElements::HullCollisionShape");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHullCollisionShapeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( CG_NULL, 0 ) )
        return false;

    // Call base class implementation last.
    return cgCollisionShapeElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHullCollisionShapeElement::insertComponentData( void * pSerializedData, cgUInt32 nDataSize )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("HullCollisionShapeElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertHullCollisionShape.bindParameter( 1, mReferenceId );
        mInsertHullCollisionShape.bindParameter( 2, pSerializedData, nDataSize );
        mInsertHullCollisionShape.bindParameter( 3, mCollapseTolerance );
        mInsertHullCollisionShape.bindParameter( 4, mSoftRefCount );
        
        // Execute
        if ( !mInsertHullCollisionShape.step( true ) )
        {
            cgString strError;
            mInsertHullCollisionShape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for convex hull collision shape object sub-element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("HullCollisionShapeElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("HullCollisionShapeElement::insertComponentData") );

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
bool cgHullCollisionShapeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the shape data.
    prepareQueries();
    mLoadHullCollisionShape.bindParameter( 1, e->sourceRefId );
    if ( !mLoadHullCollisionShape.step( ) || !mLoadHullCollisionShape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadHullCollisionShape.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for convex hull collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for convex hull collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadHullCollisionShape.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadHullCollisionShape;

    // Load parameters
    mLoadHullCollisionShape.getColumn( _T("CollapseTolerance"), mCollapseTolerance );

    // Any serialized data available?
    void   * pSerializedData = CG_NULL;
    cgUInt32 nDataSize = 0;
    mLoadHullCollisionShape.getColumn( _T("HullData"), &pSerializedData, nDataSize );
    mSerializedDataSource = (pSerializedData != CG_NULL) ? e->sourceRefId : 0;
    
    // Call base class implementation to read remaining data.
    if ( !cgCollisionShapeElement::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData( pSerializedData, nDataSize ) )
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
void cgHullCollisionShapeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertHullCollisionShape.isPrepared() )
            mInsertHullCollisionShape.prepare( mWorld, _T("INSERT INTO 'ObjectSubElements::HullCollisionShape' VALUES(?1,?2,?3,?4)"), true );
        if ( !mUpdateSerializedData.isPrepared() )
            mUpdateSerializedData.prepare( mWorld, _T("UPDATE 'ObjectSubElements::HullCollisionShape' SET HullData=?1 WHERE RefId=?2"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadHullCollisionShape.isPrepared() )
        mLoadHullCollisionShape.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::HullCollisionShape' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : setTransform()
/// <summary>
/// Update the parent relative (or offset) transform for this shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgHullCollisionShapeElement::setTransform( const cgTransform & t )
{
    // Transform adjustments are disallowed currently.
}

//-----------------------------------------------------------------------------
//  Name : autoFit () (Virtual)
/// <summary>
/// Automatically "best fit" the shape to the parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgHullCollisionShapeElement::autoFit( AutoFitType Type )
{
    // Auto-fit not supported for hull collision shape currently.
}

//-----------------------------------------------------------------------------
//  Name : createSandboxMesh ( ) (Virtual, Protected)
/// <summary>
/// Create the mesh that will be used to provide a visual representation of
/// this collision shape. It will also be used for shape picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHullCollisionShapeElement::createSandboxMesh( )
{
    // Mesh is available? (Generated automatically on the first call to
    // 'generatePhysicsShape()' if in sandbox mode).
    return mSandboxMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : computeShapeTransform ( ) (Virtual, Protected)
/// <summary>
/// Generate the final transformation object used to transform the generated
/// sandbox mesh for both rendering and shape picking.
/// </summary>
//-----------------------------------------------------------------------------
void cgHullCollisionShapeElement::computeShapeTransform( cgObjectNode * pIssuer, cgTransform & t )
{
    t = mTransform * pIssuer->getWorldTransform(false);
}

//-----------------------------------------------------------------------------
//  Name : generatePhysicsShape ( ) (Virtual)
/// <summary>
/// Retrieve a physics engine compatible shape object for this element type.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape * cgHullCollisionShapeElement::generatePhysicsShape( cgPhysicsWorld * pWorld )
{
    cgConvexHullShape * pNewShape = CG_NULL;

    // If serialized shape data is available, generate a new shape from it.
    if ( mSerializedDataSource )
    {
        // Load the shape data.
        prepareQueries();
        mLoadHullCollisionShape.bindParameter( 1, mSerializedDataSource );
        if ( !mLoadHullCollisionShape.step( ) || !mLoadHullCollisionShape.nextRow() )
        {
            // Log any error.
            cgString strError;
            if ( !mLoadHullCollisionShape.getLastError( strError ) )
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for convex hull collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for convex hull collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

            // Release any pending read operation.
            mLoadHullCollisionShape.reset();
            return CG_NULL;
        
        } // End if failed

        // Get serialized data.
        void   * pSerializedData = CG_NULL;
        cgUInt32 nDataSize = 0;
        mLoadHullCollisionShape.getColumn( _T("HullData"), &pSerializedData, nDataSize );
        
        // Generate the shape
        pNewShape = new cgConvexHullShape( pWorld, pSerializedData, nDataSize );

        // Clean up.
        mLoadHullCollisionShape.reset();

    } // End if serialized
    else
    {
        // Generate a new convex hull. Currently, this only works if the parent object is a mesh.
        if ( !mParentObject->queryReferenceType( RTID_MeshObject ) )
            return CG_NULL;

        // Retrieve the underlying mesh defined by the parent object
        cgMesh * pMesh = ((cgMeshObject*)mParentObject)->getMesh().getResource(true);
        if ( !pMesh || !pMesh->getVertexCount() )
            return CG_NULL;

        // Retrieve the mesh vertices to use as the convex hull point cloud.
        cgByte * pVertices      = pMesh->getSystemVB();
        cgUInt32 nVertexCount   = pMesh->getVertexCount();
        cgUInt32 nVertexStride  = pMesh->getVertexFormat()->getStride();

        // Construct the shape.
        pNewShape = new cgConvexHullShape( pWorld, pVertices, nVertexCount, nVertexStride, mCollapseTolerance, mTransform );

        // Cache the new shape data if applicable.
        if ( shouldSerialize() )
        {
            // Retrieve the serialized data for the collision shape if available.
            cgByteArray aSerializedData;
            pNewShape->serializeShape( aSerializedData );

            // Update database.
            prepareQueries();
            if ( aSerializedData.empty() )
                mUpdateSerializedData.bindParameter( 1, CG_NULL, 0 );
            else
                mUpdateSerializedData.bindParameter( 1, &aSerializedData.front(), aSerializedData.size() );
            mUpdateSerializedData.bindParameter( 2, mReferenceId );
            if ( !mUpdateSerializedData.step( true ) )
            {
                cgString strError;
                mUpdateSerializedData.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update convex hull data for collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                pNewShape->deleteReference();
                return CG_NULL;
            
            } // End if failed

            // Data can now be retrieved from the database?
            mSerializedDataSource = (aSerializedData.empty()) ? 0 : mReferenceId;

        } // End if serialize

    } // End if !serialized

    // If we're in sandbox mode, generate a representation of this
    // shape as a mesh for rendering.
    if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && !mSandboxMesh.isValid() && pNewShape )
        mSandboxMesh = pNewShape->buildRenderMesh( mWorld->getResourceManager() );

    // Return the newly generated shape.
    return pNewShape;
}