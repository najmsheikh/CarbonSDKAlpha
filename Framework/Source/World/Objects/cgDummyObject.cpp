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
// Name : cgDummyObject.cpp                                                  //
//                                                                           //
// Desc : Contains simple classes responsible for providing dummy hierarchy  //
//        frames. These can be useful for providing alternative              //
//        transformation points of reference for animation (i.e. spinning a  //
//        wheel on more than one axis).                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgDummyObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgDummyObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgDummyObject::mInsertDummy;
cgWorldQuery cgDummyObject::mUpdateSize;
cgWorldQuery cgDummyObject::mLoadDummy;

///////////////////////////////////////////////////////////////////////////////
// cgDummyObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDummyObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyObject::cgDummyObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mSize = 3.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgDummyObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyObject::cgDummyObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgDummyObject * pObject = (cgDummyObject*)pInit;
    mSize = pObject->mSize;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDummyObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyObject::~cgDummyObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgDummyObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgDummyObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgDummyObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgDummyObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgDummyObject::getLocalBoundingBox( )
{
    // Set to current position by default
    cgFloat fHalfSize = mSize * 0.5f;
    return cgBoundingBox( -fHalfSize, -fHalfSize, -fHalfSize, fHalfSize, fHalfSize, fHalfSize ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgDummyObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Early out with a simple AABB test.
    cgFloat t;
    cgBoundingBox Bounds = pIssuer->getLocalBoundingBox();
    /*Bounds.inflate( vWireTolerance );
    if ( Bounds.intersect( vOrigin, vDir, t, false ) == false )
        return false;*/

    // Check for intersection against each of the 6 planes of the box
    bool bHit = false;
    cgVector3 vPoints[4], vWireTolerance;
    cgFloat fClosestDistance = FLT_MAX;
    for ( size_t i = 0; i < 6; ++i )
    {
        cgPlane Plane = Bounds.getPlane( (cgVolumePlane::Side)i );
        
        // Determine if (and where) the ray intersects the box plane
        if ( !cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, true, false ) )
            continue;

        // Compute the plane intersection point and the four corner points of the plane.
        const cgVector3 vIntersect = vOrigin + (vDir * t);
        Bounds.getPlanePoints( (cgVolumePlane::Side)i, vPoints );

        // Test each edge in the box side in order to determine if the ray
        // passes close to the edge (could optimize this by skipping any
        // edges which have already been tested in adjacent triangles).
        for ( size_t nEdge = 0; nEdge < 4; ++nEdge )
        {
            // Retrieve the two vertices that form this edge
            const cgVector3 & e1 = vPoints[nEdge];
            const cgVector3 & e2 = vPoints[(nEdge+1)%4];

            // Test a ray, formed by the edge itself, to see if it intersects
            // a plane passing through the cursor position. This intersection
            // point can then be tested to see if it came close to the original
            // /plane/ intersection point.
            cgVector3 vNormal;
            cgVector3::normalize( vNormal, (e1-e2) );
            if ( cgCollision::rayIntersectPlane( e1, e2-e1, vNormal, vIntersect, t ) )
            {
                // Compute intersection point
                const cgVector3 vEdgeIntersect = e1 + ((e2-e1) * t);

                // Compute the correct wireframe tolerance to use at the edge.
                vWireTolerance = pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, vEdgeIntersect, pIssuer->getWorldTransform(false) );

                // Test to see if the cursor hit location (on the plane) is close to the edge.
                if ( fabsf( vIntersect.x - vEdgeIntersect.x ) <= vWireTolerance.x &&
                     fabsf( vIntersect.y - vEdgeIntersect.y ) <= vWireTolerance.y &&
                     fabsf( vIntersect.z - vEdgeIntersect.z ) <= vWireTolerance.z )
                {
                    t = cgVector3::length( vEdgeIntersect - vOrigin );

                    // Is this the closest intersection so far?
                    if ( t < fClosestDistance )
                    {
                        fClosestDistance = t;
                        bHit             = true;

                    } // End if closest so far

                } // End if vectors match

            } // End if edge hit cursor plane

        } // Next Edge
    
    } // Next Plane

    // Hit registered?
    if ( bHit )
        fDistance = fClosestDistance;
    else
        return false;

    // Intersection occurred
    return true;
    
    /*// Check for intersection against each of the 6 planes of the box
    for ( size_t i = 0; i < 6; ++i )
    {
        cgPlane Plane = Bounds.getPlane( (cgVolumePlane::Side)i );
        
        // Skip if the ray and the plane are effectively orthogonal
        if ( fabsf(cgVector3::dot( vDir, (cgVector3&)Plane )) < CGE_EPSILON )
            continue;

        // Determine if (and where) the ray intersects the box plane
        if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, true, false ) == false )
            continue;

        // Compute distance from center of this box face to the intersection point
        cgVector3 vPos;
        cgVector3 vIntersect = vOrigin + (vDir * t);
        vPos.x = fabsf(vIntersect.x)-(mSize*0.5f);
        vPos.y = fabsf(vIntersect.y)-(mSize*0.5f);
        vPos.z = fabsf(vIntersect.z)-(mSize*0.5f);

        // If this intersects close to the edge of the box, we'll consider this picked.
        // (This is a relatively simple test because we are working in object space).
        if ( ((fabsf(Plane.a) < 1e-3f) && (fabsf(vPos.x) < vWireTolerance.x)) ||
             ((fabsf(Plane.b) < 1e-3f) && (fabsf(vPos.y) < vWireTolerance.y)) ||
             ((fabsf(Plane.c) < 1e-3f) && (fabsf(vPos.z) < vWireTolerance.z)) )
        {

            fDistance = t;
            return true;

        } // End if close to edge

    } // Next Plane

    // No intersection with us
    return false;*/
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgDummyObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Draw the bounding box (use "sealed" edges - i.e. no gaps).
    cgBoundingBox Bounds = pIssuer->getLocalBoundingBox();
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();
    pDriver->drawOOBB( Bounds, 0.0f, pIssuer->getWorldTransform( false ), ((pIssuer->isSelected()) ? 0xFFFFFFFF : 0xFF0EFF02), true );

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgDummyObject::applyObjectRescale( cgFloat fScale )
{
    // Apply the scale to object-space data
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
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDummyObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DummyObject )
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
cgString cgDummyObject::getDatabaseTable( ) const
{
    return _T("Objects::Dummy");
}

//-----------------------------------------------------------------------------
//  Name : getSize()
/// <summary>
/// Retrieve the local size of the dummy object (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgDummyObject::getSize( ) const
{
    return mSize;
}

//-----------------------------------------------------------------------------
//  Name : setSize()
/// <summary>
/// Set the local size of the dummy object (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
void cgDummyObject::setSize( cgFloat fSize )
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
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDummyObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgDummyObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("DummyObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertDummy.bindParameter( 1, mReferenceId );
        mInsertDummy.bindParameter( 2, mSize );
        mInsertDummy.bindParameter( 3, mSoftRefCount );

        // Execute
        if ( !mInsertDummy.step( true ) )
        {
            cgString strError;
            mInsertDummy.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for dummy object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("DummyObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("DummyObject::insertComponentData") );

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
bool cgDummyObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadDummy.bindParameter( 1, e->sourceRefId );
    if ( !mLoadDummy.step( ) || !mLoadDummy.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadDummy.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadDummy.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadDummy;

    // Update our local members
    mLoadDummy.getColumn( _T("DisplaySize"), mSize );

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
void cgDummyObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertDummy.isPrepared() == false )
            mInsertDummy.prepare( mWorld, _T("INSERT INTO 'Objects::Dummy' VALUES(?1,?2,?3)"), true );
        if ( mUpdateSize.isPrepared() == false )
            mUpdateSize.prepare( mWorld, _T("UPDATE 'Objects::Dummy' SET DisplaySize=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadDummy.isPrepared() == false )
        mLoadDummy.prepare( mWorld, _T("SELECT * FROM 'Objects::Dummy' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgDummyNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDummyNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyNode::cgDummyNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Dummy%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgDummyNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyNode::cgDummyNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgDummyNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDummyNode::~cgDummyNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgDummyNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgDummyNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgDummyNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgDummyNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDummyNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DummyNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}