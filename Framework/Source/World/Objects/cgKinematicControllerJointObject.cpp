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
// Name : cgKinematicControllerJointObject.cpp                               //
//                                                                           //
// Desc : Class implementing a simple kinematic controller joint as a scene  //
//        object. This joint can be used to anchor a body to a specific      //
//        point in space. This anchor point can then be manipulated          //
//        dynamically at runtime resulting in the body following.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgKinematicControllerJointObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgKinematicControllerJointObject.h>
#include <Rendering/cgRenderDriver.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgKinematicControllerJointObject::mInsertJoint;
cgWorldQuery cgKinematicControllerJointObject::mUpdateFrictions;
cgWorldQuery cgKinematicControllerJointObject::mUpdateConstraintMode;
cgWorldQuery cgKinematicControllerJointObject::mLoadJoint;

///////////////////////////////////////////////////////////////////////////////
// cgKinematicControllerJointObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointObject::cgKinematicControllerJointObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgJointObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mMaxLinearFriction    = 1.0f;
    mMaxAngularFriction   = 1.0f;
    mConstraintMode        = PreserveOrientation;
}

//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointObject::cgKinematicControllerJointObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgJointObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgKinematicControllerJointObject * pObject = (cgKinematicControllerJointObject*)pInit;
    mMaxLinearFriction    = pObject->mMaxLinearFriction;
    mMaxAngularFriction   = pObject->mMaxAngularFriction;
    mConstraintMode        = pObject->mConstraintMode;
}

//-----------------------------------------------------------------------------
//  Name : ~cgKinematicControllerJointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointObject::~cgKinematicControllerJointObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgKinematicControllerJointObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgKinematicControllerJointObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgKinematicControllerJointObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgKinematicControllerJointObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgKinematicControllerJointObject::getLocalBoundingBox( )
{
    // Set to current position by default
    cgFloat fHalfSize = 5.0f;
    return cgBoundingBox( -fHalfSize, -fHalfSize, -fHalfSize, fHalfSize, fHalfSize, fHalfSize ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgKinematicControllerJointObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, cgUInt32 nFlags, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    /*cgFloat     t;
    cgVector3 vIntersect, vPos;
    
    // Early out with a simple AABB test.
    cgBoundingBox Bounds = getLocalBoundingBox();
    Bounds.Grow( vWireTolerance );
    if ( Bounds.Intersect( vOrigin, vDir, t, false ) == false )
        return false;
    
    // Check for intersection against each of the 6 planes of the box
    for ( cgUInt32 i = 0; i < 6; ++i )
    {
        cgPlane Plane = Bounds.GetPlane( (cgVolumePlane::Side)i );
        
        // Skip if the ray and the plane are effectively orthogonal
        if ( fabsf(CGEVec3Dot( &vDir, (cgVector3*)&Plane ))  < 1e-3f )
            continue;

        // Determine if (and where) the ray intersects the box plane
        if ( cgCollision::RayIntersectPlane( vOrigin, vDir, Plane, t, true, false ) == false )
            continue;

        // Compute distance from center of this box face to the intersection point
        vIntersect = vOrigin + (vDir * t);
        vPos.x     = fabsf(vIntersect.x)-(m_fSize*0.5f);
        vPos.y     = fabsf(vIntersect.y)-(m_fSize*0.5f);
        vPos.z     = fabsf(vIntersect.z)-(m_fSize*0.5f);

        // If this intersects close to the edge of the box, we'll consider this picked.
        // (This is a relatively simple test because we are working in object space).
        if ( ((fabsf(Plane.a) < 1e-3f) && (fabsf(vPos.x) < vWireTolerance.x)) ||
             ((fabsf(Plane.b) < 1e-3f) && (fabsf(vPos.y) < vWireTolerance.y)) ||
             ((fabsf(Plane.c) < 1e-3f) && (fabsf(vPos.z) < vWireTolerance.z)) )
        {
            fDistance = t;
            return true;

        } // End if close to edge

    } // Next Plane*/

    // No intersection with us
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJointObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    /*cgRenderDriver * pDriver = cgRenderDriver::GetInstance();
        
    // Draw the bounding box (use "sealed" edges - i.e. no gaps).
    pDriver->DrawOOBB( getLocalBoundingBox(), 0.0f, pIssuer->GetWorldTransform( false ), ((pIssuer->IsSelected()) ? 0xFFFFFFFF : 0xFF0EFF02), true );*/

    // Call base class implementation last.
    cgJointObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJointObject::applyObjectRescale( cgFloat fScale )
{
    cgToDoAssert( "Carbon General", "cgKinematicControllerJointObject::applyObjectRescale()" );

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
bool cgKinematicControllerJointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_KinematicControllerJointObject )
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
cgString cgKinematicControllerJointObject::getDatabaseTable( ) const
{
    return _T("Objects::KinematicControllerJoint");
}

//-----------------------------------------------------------------------------
//  Name : setConstraintMode ()
/// <summary>
/// Set the mechanism that will be used to constrain the body to which this
/// joint is attached. The body can either be anchored to a single point in
/// space with freedom to rotate about it, or the orientation can be entirely
/// fixed.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJointObject::setConstraintMode( ConstraintMode Mode )
{
    // Is this a no-op?
    if ( mConstraintMode == Mode )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateConstraintMode.bindParameter( 1, (cgInt)Mode );
        mUpdateConstraintMode.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateConstraintMode.step( true ) == false )
        {
            cgString strError;
            mUpdateConstraintMode.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update constraint mode for kinematic controller joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mConstraintMode = Mode;

    // Notify listeners that property was altered
    static const cgString strContext = _T("ConstraintMode");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setMaxLinearFriction ()
/// <summary>
/// Set the maximum amount of linear friction that should be considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJointObject::setMaxLinearFriction( cgFloat fFriction )
{
    // Is this a no-op?
    if ( mMaxLinearFriction == fFriction )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateFrictions.bindParameter( 1, fFriction );
        mUpdateFrictions.bindParameter( 2, mMaxAngularFriction );
        mUpdateFrictions.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateFrictions.step( true ) == false )
        {
            cgString strError;
            mUpdateFrictions.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update maximum frictions for kinematic controller joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mMaxLinearFriction = fFriction;

    // Notify listeners that property was altered
    static const cgString strContext = _T("MaxLinearFriction");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setMaxAngularFriction ()
/// <summary>
/// Set the maximum amount of angular friction that should be considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJointObject::setMaxAngularFriction( cgFloat fFriction )
{
    // Is this a no-op?
    if ( mMaxAngularFriction == fFriction )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateFrictions.bindParameter( 1, mMaxLinearFriction );
        mUpdateFrictions.bindParameter( 2, fFriction );
        mUpdateFrictions.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateFrictions.step( true ) == false )
        {
            cgString strError;
            mUpdateFrictions.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update maximum frictions for kinematic controller joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mMaxAngularFriction = fFriction;

    // Notify listeners that property was altered
    static const cgString strContext = _T("MaxAngularFriction");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getConstraintMode ()
/// <summary>
/// Get the mechanism that is in use for constraining the body to which this
/// joint is attached. The body can either be anchored to a single point in
/// space with freedom to rotate about it, or the orientation can be entirely
/// fixed.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointObject::ConstraintMode cgKinematicControllerJointObject::getConstraintMode( ) const
{
    return mConstraintMode;
}

//-----------------------------------------------------------------------------
//  Name : getMaxAngularFriction ()
/// <summary>
/// Get the maximum amount of angular friction that is being considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgKinematicControllerJointObject::getMaxAngularFriction( ) const
{
    return mMaxAngularFriction;
}

//-----------------------------------------------------------------------------
//  Name : getMaxLinearFriction ()
/// <summary>
/// Get the maximum amount of linear friction that is being considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgKinematicControllerJointObject::getMaxLinearFriction( ) const
{
    return mMaxLinearFriction;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgKinematicControllerJointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
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
bool cgKinematicControllerJointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("KinematicControllerJointObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertJoint.bindParameter( 1, mReferenceId );
        mInsertJoint.bindParameter( 2, (cgInt)mConstraintMode );
        mInsertJoint.bindParameter( 3, mMaxLinearFriction );
        mInsertJoint.bindParameter( 4, mMaxAngularFriction );
        mInsertJoint.bindParameter( 5, mSoftRefCount );

        // Execute
        if ( mInsertJoint.step( true ) == false )
        {
            cgString strError;
            mInsertJoint.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for kinematic controller joint object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("KinematicControllerJointObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("KinematicControllerJointObject::insertComponentData") );

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
bool cgKinematicControllerJointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the joint data.
    prepareQueries();
    mLoadJoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadJoint.step( )  || !mLoadJoint.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadJoint.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for kinematic controller joint object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for kinematic controller joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadJoint.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadJoint;

    // Update our local members
    cgInt nValue;
    mLoadJoint.getColumn( _T("ConstraintMode"), nValue );
    mConstraintMode = (ConstraintMode)nValue;
    mLoadJoint.getColumn( _T("MaxLinearFriction"), mMaxLinearFriction );
    mLoadJoint.getColumn( _T("MaxAngularFriction"), mMaxAngularFriction );

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
void cgKinematicControllerJointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertJoint.isPrepared() == false )
            mInsertJoint.prepare( mWorld, _T("INSERT INTO 'Objects::KinematicControllerJoint' VALUES(?1,?2,?3,?4,?5)"), true );
        if ( mUpdateConstraintMode.isPrepared() == false )
            mUpdateConstraintMode.prepare( mWorld, _T("UPDATE 'Objects::KinematicControllerJoint' SET ConstraintMode=?1 WHERE RefId=?2"), true );
        if ( mUpdateFrictions.isPrepared() == false )
            mUpdateFrictions.prepare( mWorld, _T("UPDATE 'Objects::KinematicControllerJoint' SET MaxLinearFriction=?1, MaxAngularFriction=?2 WHERE RefId=?3"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadJoint.isPrepared() == false )
        mLoadJoint.prepare( mWorld, _T("SELECT * FROM 'Objects::KinematicControllerJoint' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgKinematicControllerJointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointNode::cgKinematicControllerJointNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgJointNode( nReferenceId, pScene )
{
}

//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointNode::cgKinematicControllerJointNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgKinematicControllerJointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJointNode::~cgKinematicControllerJointNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgKinematicControllerJointNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgKinematicControllerJointNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgKinematicControllerJointNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgKinematicControllerJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgKinematicControllerJointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_KinematicControllerJointNode )
        return true;

    // Supported by base?
    return cgJointNode::queryReferenceType( type );
}