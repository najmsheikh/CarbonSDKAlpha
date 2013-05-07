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
// Name : cgSphereCollisionShapeElement.cpp                                  //
//                                                                           //
// Desc : Class that provides a sphere shaped collision primitive exposed    //
//        as an object sub-element. This provides the integration between    //
//        the application (such as the editing environment) and the relevant //
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
// cgSphereCollisionShapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgSphereCollisionShapeElement.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Physics/Shapes/cgSphereShape.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSphereCollisionShapeElement::mInsertSphereCollisionShape;
cgWorldQuery cgSphereCollisionShapeElement::mLoadSphereCollisionShape;

///////////////////////////////////////////////////////////////////////////////
// cgSphereCollisionShapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSphereCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereCollisionShapeElement::cgSphereCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgCollisionShapeElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgSphereCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereCollisionShapeElement::cgSphereCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgCollisionShapeElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgSphereCollisionShapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereCollisionShapeElement::~cgSphereCollisionShapeElement()
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
void cgSphereCollisionShapeElement::dispose( bool bDisposeBase )
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
bool cgSphereCollisionShapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SphereCollisionShapeElement )
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
cgObjectSubElement * cgSphereCollisionShapeElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgSphereCollisionShapeElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgSphereCollisionShapeElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgSphereCollisionShapeElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSphereCollisionShapeElement::getDatabaseTable( ) const
{
    return _T("ObjectSubElements::SphereCollisionShape");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSphereCollisionShapeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( ) )
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
bool cgSphereCollisionShapeElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SphereCollisionShapeElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSphereCollisionShape.bindParameter( 1, mReferenceId );
        mInsertSphereCollisionShape.bindParameter( 2, mSoftRefCount );
        
        // Execute
        if ( !mInsertSphereCollisionShape.step( true ) )
        {
            cgString strError;
            mInsertSphereCollisionShape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for sphere collision shape object sub-element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("SphereCollisionShapeElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SphereCollisionShapeElement::insertComponentData") );

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
bool cgSphereCollisionShapeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the shape data.
    prepareQueries();
    mLoadSphereCollisionShape.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSphereCollisionShape.step( ) || !mLoadSphereCollisionShape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadSphereCollisionShape.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sphere collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sphere collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSphereCollisionShape.reset();
        return false;
    
    } // End if failed

    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSphereCollisionShape;

    // Call base class implementation to read remaining data.
    if ( !cgCollisionShapeElement::onComponentLoading( e ) )
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
void cgSphereCollisionShapeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertSphereCollisionShape.isPrepared() == false )
            mInsertSphereCollisionShape.prepare( mWorld, _T("INSERT INTO 'ObjectSubElements::SphereCollisionShape' VALUES(?1,?2)"), true );
        
    } // End if sandsphere

    // Read queries
    if ( mLoadSphereCollisionShape.isPrepared() == false )
        mLoadSphereCollisionShape.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::SphereCollisionShape' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : autoFit () (Virtual)
/// <summary>
/// Automatically "best fit" the shape to the parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereCollisionShapeElement::autoFit( AutoFitType Type )
{
    if ( Type == XAxis )
    {
        mTransform.identity( );
        mTransform.setOrientation( cgVector3( 0, -1, 0 ),
                                   cgVector3( 1, 0, 0 ),
                                   cgVector3( 0, 0, 1 ) );
    
    } // End if X axis
    else if ( Type == YAxis )
    {
        mTransform.identity( );
        mTransform.setOrientation( cgVector3( 1, 0, 0 ),
                                   cgVector3( 0, 1, 0 ),
                                   cgVector3( 0, 0, 1 ) );
    
    } // End if Z axis
    else if ( Type == ZAxis )
    {
        mTransform.identity( );
        mTransform.setOrientation( cgVector3( 1, 0, 0 ),
                                   cgVector3( 0, 0, 1 ),
                                   cgVector3( 0, -1, 0 ) );
    
    } // End if X axis

    // Retrieve bounds.
    cgBoundingBox ParentBounds = getAutoFitBounds();

    // Center on the shape origin
    cgTransform NewTransform = mTransform;
    cgVector3 vCenter = ParentBounds.getCenter();
    ParentBounds -= vCenter;
    NewTransform.setPosition( 0, 0, 0 );

    // Transform the parent bounding box into the space of the shape
    // to get our 'best fit' AABB in local space.
    cgTransform InverseTransform;
    cgTransform::inverse( InverseTransform, NewTransform );
    ParentBounds.transform( InverseTransform );
    mBounds = ParentBounds;

    // Reposition the shape at the center of the parent object.
    NewTransform.setPosition( vCenter );

    // Pass through to regular 'setTransform()' method to allow
    // it to perform any additional actions required.
    setTransform( NewTransform );
}

//-----------------------------------------------------------------------------
//  Name : createSandboxMesh ( ) (Virtual, Protected)
/// <summary>
/// Create the mesh that will be used to provide a visual representation of
/// this collision shape. It will also be used for shape picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSphereCollisionShapeElement::createSandboxMesh( )
{
    // Get access to required systems.
    cgResourceManager * pResources = mWorld->getResourceManager();
    
    // Build the sandbox representation if it needs to be constructed.
    if ( !mSandboxMesh.isValid() )
    {
        // Create a unit-size sphere (if it doesn't already exist) that will allow us
        // to represent the shape of the collision volume.
        if ( !pResources->getMesh( &mSandboxMesh, _T("Core::CollisionShapes::UnitSphere") ) )
        {
            cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

            // Construct a unit-sized box mesh
            pMesh->createSphere( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 0.5f, 10, 20,
                                 false, cgMeshCreateOrigin::Center, true, pResources );
            
            // Add to the resource manager
            pResources->addMesh( &mSandboxMesh, pMesh, 0, _T("Core::CollisionShapes::UnitSphere"), cgDebugSource() );

        } // End if no existing mesh

    } // End if mesh not valid

    // Success?
    return mSandboxMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : computeShapeTransform ( ) (Virtual, Protected)
/// <summary>
/// Generate the final transformation matrix used to transform the generated
/// sandbox mesh for both rendering and shape picking.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereCollisionShapeElement::computeShapeTransform( cgObjectNode * pIssuer, cgTransform & t )
{
    cgVector3 vDimensions = mBounds.getDimensions();
    t = mTransform * pIssuer->getWorldTransform(false);
    t.scaleLocal( vDimensions.x, vDimensions.y, vDimensions.z );
}

//-----------------------------------------------------------------------------
//  Name : generatePhysicsShape ( ) (Virtual)
/// <summary>
/// Retrieve a physics engine compatible shape object for this element type.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape * cgSphereCollisionShapeElement::generatePhysicsShape( cgPhysicsWorld * pWorld )
{
    return new cgSphereShape( pWorld, mBounds, mTransform );
}