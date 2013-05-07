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
// Name : cgBoxCollisionShapeElement.cpp                                     //
//                                                                           //
// Desc : Class that provides a box shaped collision primitive exposed as an //
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
// cgBoxCollisionShapeElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgBoxCollisionShapeElement.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Physics/Shapes/cgBoxShape.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgBoxCollisionShapeElement::mInsertBoxCollisionShape;
cgWorldQuery cgBoxCollisionShapeElement::mLoadBoxCollisionShape;

///////////////////////////////////////////////////////////////////////////////
// cgBoxCollisionShapeElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBoxCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxCollisionShapeElement::cgBoxCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgCollisionShapeElement( nReferenceId, pParentObject )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgBoxCollisionShapeElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxCollisionShapeElement::cgBoxCollisionShapeElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgCollisionShapeElement( nReferenceId, pParentObject, pInit )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgBoxCollisionShapeElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBoxCollisionShapeElement::~cgBoxCollisionShapeElement()
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
void cgBoxCollisionShapeElement::dispose( bool bDisposeBase )
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
bool cgBoxCollisionShapeElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BoxCollisionShapeElement )
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
cgObjectSubElement * cgBoxCollisionShapeElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgBoxCollisionShapeElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgBoxCollisionShapeElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgBoxCollisionShapeElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgBoxCollisionShapeElement::getDatabaseTable( ) const
{
    return _T("ObjectSubElements::BoxCollisionShape");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoxCollisionShapeElement::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgBoxCollisionShapeElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("BoxCollisionShapeElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBoxCollisionShape.bindParameter( 1, mReferenceId );
        mInsertBoxCollisionShape.bindParameter( 2, mSoftRefCount );
        
        // Execute
        if ( !mInsertBoxCollisionShape.step( true ) )
        {
            cgString strError;
            mInsertBoxCollisionShape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for box collision shape object sub-element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("BoxCollisionShapeElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("BoxCollisionShapeElement::insertComponentData") );

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
bool cgBoxCollisionShapeElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the shape data.
    prepareQueries();
    mLoadBoxCollisionShape.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBoxCollisionShape.step( ) || !mLoadBoxCollisionShape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadBoxCollisionShape.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for box collision shape object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for box collision shape object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadBoxCollisionShape.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadBoxCollisionShape;

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
void cgBoxCollisionShapeElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBoxCollisionShape.isPrepared() )
            mInsertBoxCollisionShape.prepare( mWorld, _T("INSERT INTO 'ObjectSubElements::BoxCollisionShape' VALUES(?1,?2)"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadBoxCollisionShape.isPrepared() )
        mLoadBoxCollisionShape.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::BoxCollisionShape' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : autoFit () (Virtual)
/// <summary>
/// Automatically "best fit" the shape to the parent object.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoxCollisionShapeElement::autoFit( AutoFitType Type )
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

bool IntersectOOBB( const cgVector3 & minA, const cgVector3 & maxA, const cgMatrix & mtxA, const cgVector3 & minB, const cgVector3 & maxB, const cgMatrix & mtxB )
{
    
    // Generate a matrix to transform the planes for OOBB 'B' into the space of
    // OOBB 'A'. This allows us to treat this as an OOBB->AABB intersection test.
    cgMatrix mtxBtoA;
    cgMatrix::inverse( mtxBtoA, mtxA );
    mtxBtoA = mtxB * mtxBtoA;
    
    // Transform the center point of OOBB 'B' into our primary frame of reference 
    // for OOBB 'A' and compute the distance (separation) between the two.
    cgVector3 vCenterB;
    cgVector3::transformCoord( vCenterB, ((minB+maxB)*0.5f), mtxBtoA );
    cgVector3 vSeparation = vCenterB - ((minA+maxA)*0.5f);

    // Pre-compute the extents (half dimensions) of both boxes to simplify the test cases.
    cgVector3 vExtentsA = (maxA-minA)*0.5f;
    cgVector3 vExtentsB = (maxB-minB)*0.5f;

    // Pre-compute absolute versions of the matrix elements (only needed for axis
    // components -- upper left 3x3) again to simplify the test case code.
    cgMatrix mtxRotAbs = mtxBtoA;
    for ( int nRow = 0; nRow < 3; ++nRow )
    {
        for ( int nColumn = 0; nColumn < 3; ++nColumn )
            mtxRotAbs( nRow, nColumn ) = fabsf( mtxRotAbs( nRow, nColumn ) );
    
    } // Next Row
    
    // Test case 1 - X axis
    float r, r0, r1, r01;
    r = fabsf( vSeparation.x );
    r1 = cgVector3::dot( vExtentsB, cgVector3(mtxRotAbs._11, mtxRotAbs._21, mtxRotAbs._31) );
    r01 = vExtentsA.x + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 1 - Y axis
    r = fabsf( vSeparation.y );
    r1 = cgVector3::dot( vExtentsB, cgVector3(mtxRotAbs._12, mtxRotAbs._22, mtxRotAbs._32) );
    r01 = vExtentsA.y + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 1 - Z axis
    r = fabsf( vSeparation.z );
    r1 = cgVector3::dot( vExtentsB, cgVector3(mtxRotAbs._13, mtxRotAbs._23, mtxRotAbs._33) );
    r01 = vExtentsA.z + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 2 - X axis
    r = fabsf( cgVector3::dot( cgVector3(mtxBtoA._11, mtxBtoA._12, mtxBtoA._13), vSeparation) );
    r0 = cgVector3::dot( vExtentsA, cgVector3(mtxRotAbs._11, mtxRotAbs._12, mtxRotAbs._13) );
    r01 = r0 + vExtentsB.x;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 2 - Y axis
    r = fabsf( cgVector3::dot( cgVector3(mtxBtoA._21, mtxBtoA._22, mtxBtoA._23), vSeparation) );
    r0 = cgVector3::dot( vExtentsA, cgVector3(mtxRotAbs._21, mtxRotAbs._22, mtxRotAbs._23) );
    r01 = r0 + vExtentsB.y;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 2 - Z axis
    r = fabsf( cgVector3::dot( cgVector3(mtxBtoA._31, mtxBtoA._32, mtxBtoA._33), vSeparation) );
    r0 = cgVector3::dot( vExtentsA, cgVector3(mtxRotAbs._31, mtxRotAbs._32, mtxRotAbs._33) );
    r01 = r0 + vExtentsB.z;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 1
    r = fabsf( vSeparation.z * mtxBtoA._12 - vSeparation.y * mtxBtoA._13);
    r0 = vExtentsA.y * mtxRotAbs._13 + vExtentsA.z * mtxRotAbs._12;
    r1 = vExtentsB.y * mtxRotAbs._31 + vExtentsB.z * mtxRotAbs._21;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 2
    r = fabsf(vSeparation.z * mtxBtoA._22 - vSeparation.y * mtxBtoA._23);
    r0 = vExtentsA.y * mtxRotAbs._23 + vExtentsA.z * mtxRotAbs._22;
    r1 = vExtentsB.x * mtxRotAbs._31 + vExtentsB.z * mtxRotAbs._11;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 3
    r = fabsf(vSeparation.z * mtxBtoA._32 - vSeparation.y * mtxBtoA._33);
    r0 = vExtentsA.y * mtxRotAbs._33 + vExtentsA.z * mtxRotAbs._32;
    r1 = vExtentsB.x * mtxRotAbs._21 + vExtentsB.y * mtxRotAbs._11;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 4
    r = fabsf(vSeparation.x * mtxBtoA._13 - vSeparation.z * mtxBtoA._11);
    r0 = vExtentsA.x * mtxRotAbs._13 + vExtentsA.z * mtxRotAbs._11;
    r1 = vExtentsB.y * mtxRotAbs._32 + vExtentsB.z * mtxRotAbs._22;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 5
    r = fabsf(vSeparation.x * mtxBtoA._23 - vSeparation.z * mtxBtoA._21);
    r0 = vExtentsA.x * mtxRotAbs._23 + vExtentsA.z * mtxRotAbs._21;
    r1 = vExtentsB.x * mtxRotAbs._32 + vExtentsB.z * mtxRotAbs._12;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 6
    r = fabsf(vSeparation.x * mtxBtoA._33 - vSeparation.z * mtxBtoA._31);
    r0 = vExtentsA.x * mtxRotAbs._33 + vExtentsA.z * mtxRotAbs._31;
    r1 = vExtentsB.x * mtxRotAbs._22 + vExtentsB.y * mtxRotAbs._12;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 7
    r = fabsf(vSeparation.y * mtxBtoA._11 - vSeparation.x * mtxBtoA._12);
    r0 = vExtentsA.x * mtxRotAbs._12 + vExtentsA.y * mtxRotAbs._11;
    r1 = vExtentsB.y * mtxRotAbs._33 + vExtentsB.z * mtxRotAbs._23;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 8
    r = fabsf(vSeparation.y * mtxBtoA._21 - vSeparation.x * mtxBtoA._22);
    r0 = vExtentsA.x * mtxRotAbs._22 + vExtentsA.y * mtxRotAbs._21;
    r1 = vExtentsB.x * mtxRotAbs._33 + vExtentsB.z * mtxRotAbs._13;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // Test case 3 # 9
    r = fabsf(vSeparation.y * mtxBtoA._31 - vSeparation.x * mtxBtoA._32);
    r0 = vExtentsA.x * mtxRotAbs._32 + vExtentsA.y * mtxRotAbs._31;
    r1 = vExtentsB.x * mtxRotAbs._23 + vExtentsB.y * mtxRotAbs._13;
    r01 = r0 + r1;
    if (r > r01 + CGE_EPSILON)
        return false;

    // We have an intersection.
    return true;  

}

//-----------------------------------------------------------------------------
//  Name : createSandboxMesh ( ) (Virtual, Protected)
/// <summary>
/// Create the mesh that will be used to provide a visual representation of
/// this collision shape. It will also be used for shape picking.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBoxCollisionShapeElement::createSandboxMesh( )
{
    // Get access to required systems.
    cgResourceManager * pResources = mWorld->getResourceManager();
    
    // Build the sandbox representation if it needs to be constructed.
    if ( !mSandboxMesh.isValid() )
    {
        // Create a unit-size box (if it doesn't already exist) that will allow us
        // to represent the shape of the collision volume.
        if ( !pResources->getMesh( &mSandboxMesh, _T("Core::CollisionShapes::UnitBox") ) )
        {
            cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

            // Construct a unit-sized box mesh
            pMesh->createBox( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 1.0f, 1.0f, 1.0f, 
                              1, 1, 1, false, cgMeshCreateOrigin::Center, true, pResources );
            
            // Add to the resource manager
            pResources->addMesh( &mSandboxMesh, pMesh, 0, _T("Core::CollisionShapes::UnitBox"), cgDebugSource() );

        } // End if no existing mesh

    } // End if mesh not valid

    // Success?
    return mSandboxMesh.isValid();
}

//-----------------------------------------------------------------------------
//  Name : computeShapeTransform ( ) (Virtual, Protected)
/// <summary>
/// Generate the final transformation object used to transform the generated
/// sandbox mesh for both rendering and shape picking.
/// </summary>
//-----------------------------------------------------------------------------
void cgBoxCollisionShapeElement::computeShapeTransform( cgObjectNode * pIssuer, cgTransform & t )
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
cgPhysicsShape * cgBoxCollisionShapeElement::generatePhysicsShape( cgPhysicsWorld * pWorld )
{
    return new cgBoxShape( pWorld, mBounds, mTransform );
}