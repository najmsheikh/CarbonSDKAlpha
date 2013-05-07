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
// Name : cgNavigationWaypoint.cpp                                           //
//                                                                           //
// Desc : Base navigation waypoint object from which different types of      //
//        waypoint can be derived (i.e. patrol, cover, etc.)                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationWaypoint Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgNavigationWaypoint.h>
#include <World/Objects/cgCameraObject.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgSurfaceShader.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgNavigationWaypointObject::mInsertBaseWaypoint;
cgWorldQuery cgNavigationWaypointObject::mLoadBaseWaypoint;

///////////////////////////////////////////////////////////////////////////////
// cgNavigationWaypointObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationWaypointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointObject::cgNavigationWaypointObject( cgUInt32 referenceId, cgWorld * world ) : cgWorldObject( referenceId, world )
{
    // Initialize variables to sensible defaults    
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationWaypointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointObject::cgNavigationWaypointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgWorldObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgNavigationWaypointObject * object = (cgNavigationWaypointObject*)init;
    mAvailability = object->mAvailability;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationWaypointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointObject::~cgNavigationWaypointObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgNavigationWaypointObject::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    
    // Clear variables
    mAvailability = 0;
    
    // Dispose base.
    if ( disposeBase )
        cgWorldObject::dispose( true );
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
bool cgNavigationWaypointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationWaypointObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
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
bool cgNavigationWaypointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("NavigationWaypointObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBaseWaypoint.bindParameter( 1, mReferenceId );
        mInsertBaseWaypoint.bindParameter( 2, mAvailability );
        
        // Execute
        if ( !mInsertBaseWaypoint.step( true ) )
        {
            cgString error;
            mInsertBaseWaypoint.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert navigation waypoint object '0x%x' base data into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("NavigationWaypointObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("NavigationWaypointObject::insertComponentData") );

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
bool cgNavigationWaypointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the base waypoint data.
    prepareQueries();
    mLoadBaseWaypoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBaseWaypoint.step( ) || !mLoadBaseWaypoint.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadBaseWaypoint.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for navigation waypoint object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for navigation waypoint object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadBaseWaypoint.reset();
        return false;
    
    } // End if failed
    
    // Update our local members
    mLoadBaseWaypoint.getColumn( _T("Availability"), mAvailability );
    mLoadBaseWaypoint.reset();

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
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointObject::createTypeTables( const cgUID & typeIdentifier )
{
    // Ensure this base class table is created first.
    if ( !cgWorldObject::createTypeTables( RTID_NavigationWaypointObject ) )
        return false;

    // Then allow derived type to create as normal.
    return cgWorldObject::createTypeTables( typeIdentifier );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationWaypointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBaseWaypoint.isPrepared() )
            mInsertBaseWaypoint.prepare( mWorld, _T("INSERT INTO 'Objects::Base::NavigationWaypoint' VALUES(?1,?2)"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadBaseWaypoint.isPrepared() )
        mLoadBaseWaypoint.prepare( mWorld, _T("SELECT * FROM 'Objects::Base::NavigationWaypoint' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationWaypointObject::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;
    
    // Get access to required systems.
    cgRenderDriver  * driver = cgRenderDriver::getInstance();
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);

    // Retrieve useful values
    const cgViewport & viewport = driver->getViewport();
    cgFloat  zoomFactor = camera->estimateZoomFactor( viewport.size, issuer->getPosition( false ), 2.5f );
    cgFloat  size       = zoomFactor * 10.0f;
    cgUInt32 color      = issuer->isSelected() ? 0xFFFFFFFF : issuer->getNodeColor();
    
    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    cgShadedVertex points[6];
    for ( cgInt i = 0; i < 6; ++i )
        points[i].color = color;

    // Compute vertices for the object representation (central diamond)
    points[0].position = cgVector3( 0, size, 0 );
    points[1].position = cgVector3( 0, 0, size );
    points[2].position = cgVector3( size, 0, 0 );
    points[3].position = cgVector3( 0, 0, -size );
    points[4].position = cgVector3( -size, 0, 0 );
    points[5].position = cgVector3( 0, -size, 0 );
    
    // Compute indices that will allow us to draw the diamond using a tri-list (wireframe)
    // so that we can easily take advantage of back-face culling.
    cgUInt32 indices[31];
    indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
    indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;
    indices[6]  = 0; indices[7]  = 3; indices[8]  = 4;
    indices[9]  = 0; indices[10] = 4; indices[11] = 1;
    indices[12] = 5; indices[13] = 2; indices[14] = 1;
    indices[15] = 5; indices[16] = 3; indices[17] = 2;
    indices[18] = 5; indices[19] = 4; indices[20] = 3;
    indices[21] = 5; indices[22] = 1; indices[23] = 4;
    
    // Begin rendering
    bool wireframe = (flags & cgSandboxRenderFlags::Wireframe);
    driver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    shader->setBool( _T("wireViewport"), wireframe );
    driver->setWorldTransform( issuer->getWorldTransform( false ) );
    if ( shader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( shader->executeTechniquePass() != cgTechniqueResult::Abort )
            driver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 6, 8, indices, cgBufferFormat::Index32, points );

        // We have finished rendering
        shader->endTechnique();
    
    } // End if begun technique

    // Call base class implementation last.
    cgWorldObject::sandboxRender( flags, camera, visibilityData, gridPlane, issuer );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgNavigationWaypointObject::getLocalBoundingBox( )
{
    // Set to current position by default
    return cgBoundingBox( 0,0,0,0,0,0 ); 
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointObject::pick( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat fWireTolerance, cgFloat & distance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    cgFloat   zoomFactor  = camera->estimateZoomFactor( viewportSize, issuer->getPosition( false ), 2.5f );
    cgVector3 right       = cgVector3(1,0,0);
    cgVector3 up          = cgVector3(0,1,0);
    cgVector3 look        = cgVector3(0,0,1);
    
    // Compute vertices for the light source representation (central diamond)
    cgVector3 points[6];
    points[0] = up * (10.0f * zoomFactor);
    points[1] = look * (10.0f * zoomFactor);
    points[2] = right * (10.0f * zoomFactor);
    points[3] = -look * (10.0f * zoomFactor);
    points[4] = -right * (10.0f * zoomFactor);
    points[5] = -up * (10.0f * zoomFactor);

    // Construct indices for the 8 triangles
    cgUInt32 indices[24];
    indices[0]  = 0; indices[1]  = 1; indices[2]  = 2;
    indices[3]  = 0; indices[4]  = 2; indices[5]  = 3;
    indices[6]  = 0; indices[7]  = 3; indices[8]  = 4;
    indices[9]  = 0; indices[10] = 4; indices[11] = 1;
    indices[12] = 5; indices[13] = 2; indices[14] = 1;
    indices[15] = 5; indices[16] = 3; indices[17] = 2;
    indices[18] = 5; indices[19] = 4; indices[20] = 3;
    indices[21] = 5; indices[22] = 1; indices[23] = 4;

    // Determine if the ray intersects any of the triangles formed by the above points
    for ( cgUInt32 i = 0; i < 8; ++i )
    {
        // Compute plane for the current triangle
        cgPlane plane;
        const cgVector3 & v1 = points[indices[(i*3)+0]];
        const cgVector3 & v2 = points[indices[(i*3)+1]];
        const cgVector3 & v3 = points[indices[(i*3)+2]];
        cgPlane::fromPoints( plane, v1, v2, v3 );

        // Determine if (and where) the ray intersects the triangle's plane
        cgFloat t;
        if ( !cgCollision::rayIntersectPlane( rayOrigin, rayDirection, plane, t, false, false ) )
            continue;

        // Compute the intersection point on the plane
        cgVector3 intersect = rayOrigin + (rayDirection * t);

        // Determine if this point is within the triangles edges (within tolerance)
        if ( !cgCollision::pointInTriangle( intersect, v1, v2, v3, (cgVector3&)plane, 2.0f * zoomFactor ) )
            continue;

        // We intersected! Return the intersection distance.
        distance = t;
        return true;

    } // Next Triangle
    
    // No intersection with us.
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cgNavigationWaypointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationWaypointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointNode::cgNavigationWaypointNode( cgUInt32 referenceId, cgScene * scene ) : cgObjectNode( referenceId, scene )
{
    // Initialize variables to sensible defaults
	
    // Set default color and instance identifier
    mColor              = 0xFF00F6FF;
    mInstanceIdentifier = cgString::format( _T("Waypoint%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationWaypointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointNode::cgNavigationWaypointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgObjectNode( referenceId, scene, init, initMethod, initTransform )
{
    // Initialize variables to sensible defaults
    cgNavigationWaypointNode * node = (cgNavigationWaypointNode*)init;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationWaypointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationWaypointNode::~cgNavigationWaypointNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgNavigationWaypointNode::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear variables
    
    // Dispose base.
    if ( disposeBase )
        cgObjectNode::dispose( true );
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
bool cgNavigationWaypointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationWaypointNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointNode::onNodeCreated( const cgUID & objectType, cgCloneMethod::Base cloneMethod )
{
    // Call base class implementation.
    if ( !cgObjectNode::onNodeCreated( objectType, cloneMethod ) )
        return false;

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
bool cgNavigationWaypointNode::onNodeLoading( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( objectType, nodeData, parentCell, cloneMethod ) )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointNode::canScale( ) const
{
    // Waypoints cannot be scaled.
    return false;
}

/*//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationWaypointNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas  = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame  = _T("Light");
    
    // Position icon
    vIconOrigin = getPosition(false);
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vIconOrigin, 2.5f );
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 17.0f * fZoomFactor);
    return true;
}*/