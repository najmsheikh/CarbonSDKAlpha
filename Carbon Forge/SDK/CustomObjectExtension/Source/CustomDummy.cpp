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
// Name : CustomDummy.cpp                                                    //
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
// CustomDummy Module Includes
//-----------------------------------------------------------------------------
#include "CustomDummy.h"

// CGE Includes
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgGroupObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery CustomDummyObject::mInsertDummy;
cgWorldQuery CustomDummyObject::mUpdateSize;
cgWorldQuery CustomDummyObject::mLoadDummy;

///////////////////////////////////////////////////////////////////////////////
// CustomDummyObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : CustomDummyObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyObject::CustomDummyObject( cgUInt32 referenceId, cgWorld * world ) : cgWorldObject( referenceId, world )
{
    // Initialize members to sensible defaults
    mSize = 3.0f;
}

//-----------------------------------------------------------------------------
//  Name : CustomDummyObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyObject::CustomDummyObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    CustomDummyObject * object = (CustomDummyObject*)init;
    mSize = object->mSize;
}

//-----------------------------------------------------------------------------
//  Name : ~CustomDummyObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyObject::~CustomDummyObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * CustomDummyObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new CustomDummyObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * CustomDummyObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    // Valid clone?
    return new CustomDummyObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox CustomDummyObject::getLocalBoundingBox( )
{
    // Set to current position by default
    const cgFloat halfSize = mSize * 0.5f;
    return cgBoundingBox( -halfSize, -halfSize, -halfSize, halfSize, halfSize, halfSize ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool CustomDummyObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Early out with a simple AABB test.
    cgFloat t;
    cgBoundingBox bounds = issuer->getLocalBoundingBox();
    /*Bounds.inflate( vWireTolerance );
    if ( Bounds.intersect( vOrigin, vDir, t, false ) == false )
        return false;*/

    // Check for intersection against each of the 6 planes of the box
    bool hit = false;
    cgVector3 points[4];
    cgFloat closestDistance = FLT_MAX;
    for ( size_t i = 0; i < 6; ++i )
    {
        cgPlane plane = bounds.getPlane( (cgVolumePlane::Side)i );
        
        // Determine if (and where) the ray intersects the box plane
        if ( !cgCollision::rayIntersectPlane( rayOrigin, rayDirection, plane, t, true, false ) )
            continue;

        // Compute the plane intersection point and the four corner points of the plane.
        const cgVector3 intersect = rayOrigin + (rayDirection * t);
        bounds.getPlanePoints( (cgVolumePlane::Side)i, points );

        // Test each edge in the box side in order to determine if the ray
        // passes close to the edge (could optimize this by skipping any
        // edges which have already been tested in adjacent triangles).
        for ( size_t edge = 0; edge < 4; ++edge )
        {
            // Retrieve the two vertices that form this edge
            const cgVector3 & e1 = points[edge];
            const cgVector3 & e2 = points[(edge+1)%4];

            // Test a ray, formed by the edge itself, to see if it intersects
            // a plane passing through the cursor position. This intersection
            // point can then be tested to see if it came close to the original
            // /plane/ intersection point.
            cgVector3 normal;
            cgVector3::normalize( normal, (e1-e2) );
            if ( cgCollision::rayIntersectPlane( e1, e2-e1, normal, intersect, t ) )
            {
                // Compute intersection point
                const cgVector3 edgeIntersect = e1 + ((e2-e1) * t);

                // Compute the correct wireframe tolerance to use at the edge.
                cgVector3 wt = camera->estimatePickTolerance( viewportSize, wireTolerance, edgeIntersect, issuer->getWorldTransform(false) );

                // Test to see if the cursor hit location (on the plane) is close to the edge.
                if ( fabsf( intersect.x - edgeIntersect.x ) <= wt.x &&
                     fabsf( intersect.y - edgeIntersect.y ) <= wt.y &&
                     fabsf( intersect.z - edgeIntersect.z ) <= wt.z )
                {
                    t = cgVector3::length( edgeIntersect - rayOrigin );

                    // Is this the closest intersection so far?
                    if ( t < closestDistance )
                    {
                        closestDistance = t;
                        hit             = true;

                    } // End if closest so far

                } // End if vectors match

            } // End if edge hit cursor plane

        } // Next Edge
    
    } // Next Plane

    // Hit registered?
    if ( hit )
        distanceOut = closestDistance;
    else
        return false;

    // Intersection occurred
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void CustomDummyObject::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // Only show them if it belongs to an open group (or no group at all)
    if ( issuer->getOwnerGroup() && !issuer->getOwnerGroup()->isOpen()  )
    {
        cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
        return;
    
    } // End if Hidden

    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
    {
        cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
        return;
    
    } // End if !PostDepthClear

    // Draw the bounding box (use "sealed" edges - i.e. no gaps).
    cgBoundingBox bounds = issuer->getLocalBoundingBox();
    cgRenderDriver * driver = cgRenderDriver::getInstance();
    driver->drawOOBB( bounds, 0.0f, issuer->getWorldTransform( false ), ((issuer->isSelected()) ? 0xFFFFFFFF : 0xFFFF0E02), true );

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void CustomDummyObject::applyObjectRescale( cgFloat scale )
{
    // Apply the scale to object-space data
    cgFloat newSize = mSize * scale;
    
    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, newSize );
        mUpdateSize.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateSize.step( true ) )
        {
            cgString error;
            mUpdateSize.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size of dummy object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local values.
    mSize = newSize;
    
    // Notify listeners that property was altered
    static const cgString context = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );

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
bool CustomDummyObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CustomDummyObject )
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
cgString CustomDummyObject::getDatabaseTable( ) const
{
    return _T("Objects::CustomDummy");
}

//-----------------------------------------------------------------------------
//  Name : getSize()
/// <summary>
/// Retrieve the local size of the dummy object (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat CustomDummyObject::getSize( ) const
{
    return mSize;
}

//-----------------------------------------------------------------------------
//  Name : setSize()
/// <summary>
/// Set the local size of the dummy object (pre-scale).
/// </summary>
//-----------------------------------------------------------------------------
void CustomDummyObject::setSize( cgFloat size )
{
    // Is this a no-op?
    if ( mSize == size )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, size );
        mUpdateSize.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateSize.step( true ) )
        {
            cgString error;
            mUpdateSize.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size of dummy object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mSize = size;

    // Notify listeners that property was altered
    static const cgString context = _T("Size");
    onComponentModified( &cgComponentModifiedEventArgs( context ) );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool CustomDummyObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool CustomDummyObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("CustomDummyObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertDummy.bindParameter( 1, mReferenceId );
        mInsertDummy.bindParameter( 2, mSize );
        mInsertDummy.bindParameter( 3, mSoftRefCount );

        // Execute
        if ( !mInsertDummy.step( true ) )
        {
            cgString error;
            mInsertDummy.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for dummy object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("CustomDummyObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("CustomDummyObject::insertComponentData") );

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
bool CustomDummyObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadDummy.bindParameter( 1, e->sourceRefId );
    if ( !mLoadDummy.step( ) || !mLoadDummy.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadDummy.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for dummy object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

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
void CustomDummyObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertDummy.isPrepared( mWorld ) )
            mInsertDummy.prepare( mWorld, _T("INSERT INTO 'Objects::CustomDummy' VALUES(?1,?2,?3)"), true );
        if ( !mUpdateSize.isPrepared( mWorld ) )
            mUpdateSize.prepare( mWorld, _T("UPDATE 'Objects::CustomDummy' SET DisplaySize=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadDummy.isPrepared( mWorld ) )
        mLoadDummy.prepare( mWorld, _T("SELECT * FROM 'Objects::CustomDummy' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// CustomDummyNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : CustomDummyNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyNode::CustomDummyNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("CustomDummy%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : CustomDummyNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyNode::CustomDummyNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~CustomDummyNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
CustomDummyNode::~CustomDummyNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * CustomDummyNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new CustomDummyNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * CustomDummyNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new CustomDummyNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool CustomDummyNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_CustomDummyNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}