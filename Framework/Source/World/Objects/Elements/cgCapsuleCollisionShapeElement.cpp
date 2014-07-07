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
// Name : cgCapsuleCollisionShapeElement.cpp                                 //
//                                                                           //
// Desc : Class that provides a capsule shaped collision primitive exposed   //
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
// cgCapsuleCollisionShapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgCapsuleCollisionShapeElement.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Physics/Shapes/cgCapsuleShape.h>
#include <Math/cgMathUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgCapsuleCollisionShapeElement::mInsertCapsuleCollisionShape;
cgWorldQuery cgCapsuleCollisionShapeElement::mLoadCapsuleCollisionShape;

///////////////////////////////////////////////////////////////////////////////
// cgCapsuleCollisionShapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCapsuleCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleCollisionShapeElement::cgCapsuleCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgCollisionShapeElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgCapsuleCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleCollisionShapeElement::cgCapsuleCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgCollisionShapeElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgCapsuleCollisionShapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCapsuleCollisionShapeElement::~cgCapsuleCollisionShapeElement()
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
void cgCapsuleCollisionShapeElement::dispose( bool bDisposeBase )
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
bool cgCapsuleCollisionShapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CapsuleCollisionShapeElement )
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
cgObjectSubElement * cgCapsuleCollisionShapeElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgCapsuleCollisionShapeElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgCapsuleCollisionShapeElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgCapsuleCollisionShapeElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgCapsuleCollisionShapeElement::getDatabaseTable( ) const
{
    return _T("ObjectSubElements::CapsuleCollisionShape");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCapsuleCollisionShapeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgCapsuleCollisionShapeElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("CapsuleCollisionShapeElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertCapsuleCollisionShape.bindParameter( 1, mReferenceId );
        mInsertCapsuleCollisionShape.bindParameter( 2, mSoftRefCount );
        
        // Execute
        if ( !mInsertCapsuleCollisionShape.step( true ) )
        {
            cgString strError;
            mInsertCapsuleCollisionShape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for capsule collision shape object sub-element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("CapsuleCollisionShapeElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("CapsuleCollisionShapeElement::insertComponentData") );

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
bool cgCapsuleCollisionShapeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the shape data.
    prepareQueries();
    mLoadCapsuleCollisionShape.bindParameter( 1, e->sourceRefId );
    if ( !mLoadCapsuleCollisionShape.step( ) || !mLoadCapsuleCollisionShape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadCapsuleCollisionShape.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for capsule collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for capsule collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadCapsuleCollisionShape.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadCapsuleCollisionShape;

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
void cgCapsuleCollisionShapeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertCapsuleCollisionShape.isPrepared( mWorld ) )
            mInsertCapsuleCollisionShape.prepare( mWorld, _T("INSERT INTO 'ObjectSubElements::CapsuleCollisionShape' VALUES(?1,?2)"), true );
        
    } // End if sandcapsule

    // Read queries
    if ( !mLoadCapsuleCollisionShape.isPrepared( mWorld ) )
        mLoadCapsuleCollisionShape.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::CapsuleCollisionShape' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : setDimensions ()
/// <summary>
/// Manually set the dimensions of this element.
/// </summary>
//-----------------------------------------------------------------------------
void cgCapsuleCollisionShapeElement::setDimensions( cgFloat fRadius, cgFloat fHeight )
{
    mBounds.min.x = -fRadius;
    mBounds.min.y = -fHeight * 0.5f;
    mBounds.min.z = -fRadius;
    mBounds.max.x =  fRadius;
    mBounds.max.y =  fHeight * 0.5f;
    mBounds.max.z =  fRadius;

    // Re-trigger the 'setTransform()' method to allow
    // it to perform any additional actions required.
    setTransform( mTransform );

    // Trash the existing sandbox mesh if there is one. Capsules
    // can't be easily rendered using scaling tricks.
    mSandboxMesh.close();
}

//-----------------------------------------------------------------------------
//  Name : setTransform()
/// <summary>
/// Update the parent relative (or offset) transform for this shape.
/// </summary>
//-----------------------------------------------------------------------------
void cgCapsuleCollisionShapeElement::setTransform( const cgTransform & t )
{
    cgVector3 vOldScale = mTransform.localScale();

    // Call base class implementation
    cgCollisionShapeElement::setTransform( t );

    // If the scale of the old transform changed in any way, we
    // need to trash the existing sandbox mesh if there is one. 
    // Capsules can't be easily rendered using scaling tricks.
    if ( cgMathUtility::compareVectors( mTransform.localScale(), vOldScale, CGE_EPSILON ) != 0 )
        mSandboxMesh.close();
}

//-----------------------------------------------------------------------------
//  Name : autoFit () (Virtual)
/// <summary>
/// Automatically "best fit" the shape to the parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgCapsuleCollisionShapeElement::autoFit( AutoFitType Type )
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

    // Trash the existing sandbox mesh if there is one. Capsules
    // can't be easily rendered using scaling tricks.
    mSandboxMesh.close();
}

//-----------------------------------------------------------------------------
//  Name : applyElementRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this element. For instance,
/// in the case of a collision shape, its dimensions will be scaled.
/// </summary>
//-----------------------------------------------------------------------------
void cgCapsuleCollisionShapeElement::applyElementRescale( cgFloat fScale )
{
    // Call base class implementation.
    cgCollisionShapeElement::applyElementRescale( fScale );

    // Trash the existing sandbox mesh if there is one. Capsules
    // can't be easily rendered using scaling tricks.
    mSandboxMesh.close();
}

//-----------------------------------------------------------------------------
//  Name : createSandboxMesh ( ) (Virtual, Protected)
/// <summary>
/// Create the mesh that will be used to provide a visual representation of
/// this collision shape. It will also be used for shape picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCapsuleCollisionShapeElement::createSandboxMesh( )
{
    // Get access to required systems.
    cgResourceManager * pResources = mWorld->getResourceManager();
    
    // Build the sandbox representation if it needs to be constructed.
    if ( !mSandboxMesh.isValid() )
    {
        // Apply shape scale to the capsule dimensions.
        cgVector3 Dimensions = mBounds.getDimensions();
        cgVector3 vScale = mTransform.localScale();
        Dimensions.x *= vScale.x;
        Dimensions.y *= vScale.y;
        Dimensions.z *= vScale.z;

        // Compute the required capsule properties
        cgFloat fRadius = ((Dimensions.x > Dimensions.z) ? Dimensions.x : Dimensions.z) * 0.5f;
        cgFloat fHeight = max( Dimensions.y, fRadius * 2.0f );
        cgString strCapsuleId = cgString::format( _T("Core::CollisionShapes::Capsule(r:%.4f,h:%.4f)"), fRadius, fHeight );

        // Create a capsule with these properties (if it doesn't already exist) that will allow us
        // to represent the shape of the collision volume.
        if ( !pResources->getMesh( &mSandboxMesh, strCapsuleId ) )
        {
            cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

            // Construct a unit-sized capsule mesh
            pMesh->createCapsule( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), fRadius, fHeight,
                                   1, 20, false, cgMeshCreateOrigin::Center, true, pResources );
            
            // Add to the resource manager
            pResources->addMesh( &mSandboxMesh, pMesh, 0, strCapsuleId, cgDebugSource() );

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
void cgCapsuleCollisionShapeElement::computeShapeTransform( cgObjectNode * pIssuer, cgTransform & t )
{
    t = mTransform;
    t.setLocalScale( 1, 1, 1 );
    t *= pIssuer->getWorldTransform(false);
}

//-----------------------------------------------------------------------------
//  Name : generatePhysicsShape ( ) (Virtual)
/// <summary>
/// Retrieve a physics engine compatible shape object for this element type.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape * cgCapsuleCollisionShapeElement::generatePhysicsShape( cgPhysicsWorld * pWorld )
{
    return new cgCapsuleShape( pWorld, mBounds, mTransform );
}