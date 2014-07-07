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
// Name : cgSplineObject.cpp                                                 //
//                                                                           //
// Desc: Contains classes that allow for the design and placement of three   //
//       dimensional bezier splines. Shape of these splines can be designed  //
//       by manipulating individual control points. Can be used to provide   //
//       pathing / waypoint information, smooth animation, lofting or        //
//       collision data, etc.                                                //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSplineObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgSplineObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgSurfaceShader.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSplineObject::mInsertSpline;
cgWorldQuery cgSplineObject::mUpdateSplineData;
cgWorldQuery cgSplineObject::mUpdateDisplayOptions;
cgWorldQuery cgSplineObject::mLoadSpline;

///////////////////////////////////////////////////////////////////////////////
// cgSplineObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSplineObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineObject::cgSplineObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mDisplayInterpolationSteps  = 6;
}

//-----------------------------------------------------------------------------
//  Name : cgSplineObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineObject::cgSplineObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgSplineObject * pObject = (cgSplineObject*)pInit;
    mSpline                     = pObject->mSpline;
    mDisplayInterpolationSteps  = pObject->mDisplayInterpolationSteps;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSplineObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineObject::~cgSplineObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSplineObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgSplineObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSplineObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgSplineObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSplineObject::getLocalBoundingBox( )
{
    // Set to current position by default
    return mBounds;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgSplineObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, cgUInt32 nFlags, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Early out with a simple AABB test.
    cgFloat t;
    cgBoundingBox Bounds = pIssuer->getLocalBoundingBox();
    Bounds.inflate( pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, Bounds.getCenter(), pIssuer->getWorldTransform(false) ) );
    if ( Bounds.intersect( vOrigin, vDir, t, false ) == false )
        return false;

    // Iterate over spline and resolve at relevant divisions.
    cgInt32 nSegments = mSpline.getSegmentCount(), nPoint = 0;
    cgVector3 * Points = new cgVector3[(nSegments * (mDisplayInterpolationSteps+1)) + 1];
    for ( cgInt32 i = 0; i < nSegments; ++i )
    {
        for ( cgUInt32 j = 0; j < (mDisplayInterpolationSteps+1); ++j )
        {
            // Resolve
            cgFloat t = (cgFloat)j / (cgFloat)(mDisplayInterpolationSteps+1);
            Points[nPoint++] = mSpline.evaluateSegment( i, t );

        } // Next Iteration

    } // Next Segment

    // Add final point of final segment
    Points[nPoint++] = mSpline.evaluateSegment( nSegments-1, 1.0f );

    // Test each edge in the interpolated spline in order to determine 
    // if the ray passes close to one of them.
    bool bHit = false;
    cgFloat fClosestDistance = FLT_MAX;
    cgVector3 vWireTolerance;
    for ( cgInt32 nEdge = 0; nEdge < nPoint - 1; ++nEdge )
    {
        // Retrieve the two vertices that form this edge
        const cgVector3 & v1 = Points[nEdge];
        const cgVector3 & v2 = Points[nEdge+1];

        // Generate edge direction vector
        cgVector3 vAxis;
        cgVector3::normalize( vAxis, v2-v1 );

        // Skip if it runs parallel to the camera look vector
        if ( fabsf( vAxis.z ) >= 0.999f )
            continue;

        // Compute the plane for this edge on which we will test for ray intersection.
        // This should give us a point that exists close to the edge line segment (or not).
        cgVector3 vNormal;
        cgVector3::cross( vNormal, vAxis, cgVector3(0,0,1) );
        cgVector3::cross( vNormal, vNormal, vAxis );
        cgVector3::normalize( vNormal, vNormal );

        // Compute intersection point on plane
        cgCollision::rayIntersectPlane( vOrigin, vDir, vNormal, v1, fDistance, true, false );
        cgVector3 vIntersect = vOrigin + (vDir * fDistance);

        // Is the intersection point close to the axis line?
        if ( fDistance < fClosestDistance )
        {
            cgVector3 vClosestPoint = cgMathUtility::closestPointOnLineSegment( vIntersect, v1, v2 );
            vWireTolerance = pCamera->estimatePickTolerance( ViewportSize, fWireTolerance, vClosestPoint, pIssuer->getWorldTransform(false) );
            if ( fabsf(vIntersect.x - vClosestPoint.x) <= vWireTolerance.x &&
                 fabsf(vIntersect.y - vClosestPoint.y) <= vWireTolerance.y &&
                 fabsf(vIntersect.z - vClosestPoint.z) <= vWireTolerance.z )
            {
                fClosestDistance = fDistance;
                bHit = true;

            } // End if within tolerance
        
        } // End if closer than last test

    } // Next Edge

    // Clean up
    delete []Points;

    // Return result.
    fDistance = fClosestDistance;
    return bHit;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgSplineObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Get access to required systems
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Validate requirements
    if ( mSpline.getPointCount() < 2 )
        return;

    // Select appropriate rendering color.
    cgUInt32 nColor = (pIssuer->isSelected()) ? 0xFFFFFFFF : pIssuer->getNodeColor();

    // Iterate over spline and resolve at relevant divisions.
    cgInt32 nSegments = mSpline.getSegmentCount(), nPoint = 0;
    cgShadedVertex * Points = new cgShadedVertex[(nSegments * (mDisplayInterpolationSteps+1)) + 1];
    for ( cgInt32 i = 0; i < nSegments; ++i )
    {
        for ( cgUInt32 j = 0; j < (mDisplayInterpolationSteps+1); ++j )
        {
            cgFloat t = (cgFloat)j / (cgFloat)(mDisplayInterpolationSteps+1);
            Points[nPoint].position = mSpline.evaluateSegment( i, t );
            Points[nPoint].color = nColor;
            nPoint++;

        } // Next Iteration

    } // Next Segment

    // Add final point of final segment
    Points[nPoint].position = mSpline.evaluateSegment( nSegments-1, 1.0f );
    Points[nPoint].color = nColor;
    nPoint++;

    // Begin rendering
    bool bWireframe = (flags & cgSandboxRenderFlags::Wireframe);
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    pDriver->setWorldTransform( pIssuer->getWorldTransform( false ) );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
            pDriver->drawPrimitiveUP( cgPrimitiveType::LineStrip, nPoint-1, Points );

        // We have finished rendering
        pShader->endTechnique();

    } // End if begun technique
    
    // Clean up
    delete []Points;

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
void cgSplineObject::applyObjectRescale( cgFloat fScale )
{
    // Scale points
    cgBezierSpline3 data = mSpline;
    for ( cgInt32 i = 0; i < data.getPointCount(); ++i )
    {
        cgBezierSpline3::SplinePoint pt = data.getPoint( i );
        pt.point *= fScale;
        pt.controlPointOut *= fScale;
        pt.controlPointIn *= fScale;
        data.setPoint( i, pt );

    } // Next point

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSplineData.bindParameter( 1, data.getPointCount() );
        if ( data.getPointCount() > 0 )
            mUpdateSplineData.bindParameter( 2, &data.getPoints()[0], data.getPointCount() * sizeof(cgBezierSpline3::SplinePoint) );
        else
            mUpdateSplineData.bindParameter( 2, CG_NULL, 0 );
        mUpdateSplineData.bindParameter( 3, mReferenceId );

        // Execute
        if ( mUpdateSplineData.step( true ) == false )
        {
            cgString strError;
            mUpdateSplineData.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update point data for spline object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update local member value.
    mSpline = data;

    // Recompute the bounding box
    updateBoundingBox();
    
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
bool cgSplineObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SplineObject )
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
cgString cgSplineObject::getDatabaseTable( ) const
{
    return _T("Objects::Spline");
}

//-----------------------------------------------------------------------------
//  Name : getSplineData()
/// <summary>
/// Retrieve the three dimensional object space spline data being represented
/// by this object.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline3 & cgSplineObject::getSplineData( ) const
{
    return mSpline;
}

//-----------------------------------------------------------------------------
//  Name : getDisplaySteps()
/// <summary>
/// Get the number of interpolation steps that will be taken for each segment
/// during rendering.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgSplineObject::getDisplaySteps( ) const
{
    return mDisplayInterpolationSteps;
}

//-----------------------------------------------------------------------------
//  Name : setSplineData()
/// <summary>
/// Set the three dimensional object space spline data being represented by
/// this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgSplineObject::setSplineData( const cgBezierSpline3 & data )
{
    // Is this a no-op?
    if ( mSpline == data )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSplineData.bindParameter( 1, data.getPointCount() );
        if ( data.getPointCount() > 0 )
            mUpdateSplineData.bindParameter( 2, &data.getPoints()[0], data.getPointCount() * sizeof(cgBezierSpline3::SplinePoint) );
        else
            mUpdateSplineData.bindParameter( 2, CG_NULL, 0 );
        mUpdateSplineData.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateSplineData.step( true ) == false )
        {
            cgString strError;
            mUpdateSplineData.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update point data for spline object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mSpline = data;

    // Recompute the object space bounding box based on the new spline data.
    updateBoundingBox();

    // Notify listeners that property was altered
    static const cgString strContext = _T("SplineData");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : updateBoundingBox() (Protected)
/// <summary>
/// Recompute the object space bounding box based on the new spline data.
/// </summary>
//-----------------------------------------------------------------------------
void cgSplineObject::updateBoundingBox()
{
    mBounds.reset();
    if ( mSpline.getPointCount() > 0 )
    {
        for ( cgInt32 i = 0; i < mSpline.getPointCount() - 1; ++i )
        {
            const cgBezierSpline3::SplinePoint & pt1 = mSpline.getPoint( i );
            const cgBezierSpline3::SplinePoint & pt2 = mSpline.getPoint( i + 1 );
            mBounds.addPoint( pt1.point );
            mBounds.addPoint( pt1.controlPointOut );
            mBounds.addPoint( pt2.controlPointIn );

        } // Next Point

        // Final point
        const cgBezierSpline3::SplinePoint & pt = mSpline.getPoint( mSpline.getPointCount()-1 );
        mBounds.addPoint( pt.point );

    } // End if valid spline
}

//-----------------------------------------------------------------------------
//  Name : setDisplaySteps()
/// <summary>
/// Set the number of interpolation steps to take for each segment during
/// rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgSplineObject::setDisplaySteps( cgUInt32 steps )
{
    // Clamp
    if ( steps < 0 )
        steps = 0;

    // Is this a no-op?
    if ( mDisplayInterpolationSteps == steps )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateDisplayOptions.bindParameter( 1, steps );
        mUpdateDisplayOptions.bindParameter( 2, mReferenceId );

        // Execute
        if ( mUpdateDisplayOptions.step( true ) == false )
        {
            cgString strError;
            mUpdateDisplayOptions.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update display options for spline object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update value.
    mDisplayInterpolationSteps = steps;

    // Notify listeners that property was altered
    static const cgString strContext = _T("DisplaySteps");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSplineObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Recompute bounding box in case of clones.
    updateBoundingBox();

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSplineObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SplineObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSpline.bindParameter( 1, mReferenceId );
        mInsertSpline.bindParameter( 2, mDisplayInterpolationSteps );
        mInsertSpline.bindParameter( 3, mSpline.getPointCount() );
        if ( mSpline.getPointCount() > 0 )
            mInsertSpline.bindParameter( 4, &mSpline.getPoints()[0], mSpline.getPointCount() * sizeof(cgBezierSpline3::SplinePoint) );
        else
            mInsertSpline.bindParameter( 4, CG_NULL, 0 );
        mInsertSpline.bindParameter( 5, mSoftRefCount );

        // Execute
        if ( !mInsertSpline.step( true ) )
        {
            cgString strError;
            mInsertSpline.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for spline object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("SplineObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SplineObject::insertComponentData") );

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
bool cgSplineObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadSpline.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSpline.step( ) || !mLoadSpline.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadSpline.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for spline object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for spline object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSpline.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSpline;

    // Update our local members
    cgUInt32 nPointCount, nSize;
    cgBezierSpline3::SplinePoint * pPointData = CG_NULL;
    mLoadSpline.getColumn( _T("DisplaySteps"), mDisplayInterpolationSteps );
    mLoadSpline.getColumn( _T("SplineDataSize"), nPointCount );
    mLoadSpline.getColumn( _T("SplineData"), (void**)&pPointData, nSize );
    if ( pPointData )
    {
        mSpline.clear();
        for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
            mSpline.addPoint( pPointData[nPoint] );

    } // End if valid point data

    // Recompute bounding box based on loaded data.
    updateBoundingBox();

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
void cgSplineObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertSpline.isPrepared( mWorld ) )
            mInsertSpline.prepare( mWorld, _T("INSERT INTO 'Objects::Spline' VALUES(?1,?2,?3,?4,?5)"), true );
        if ( !mUpdateSplineData.isPrepared( mWorld ) )
            mUpdateSplineData.prepare( mWorld, _T("UPDATE 'Objects::Spline' SET SplineDataSize=?1, SplineData=?2 WHERE RefId=?3"), true );
        if ( !mUpdateDisplayOptions.isPrepared( mWorld ) )
            mUpdateDisplayOptions.prepare( mWorld, _T("UPDATE 'Objects::Spline' SET DisplaySteps=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadSpline.isPrepared( mWorld ) )
        mLoadSpline.prepare( mWorld, _T("SELECT * FROM 'Objects::Spline' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgSplineNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSplineNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineNode::cgSplineNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Spline%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgSplineNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineNode::cgSplineNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgSplineNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSplineNode::~cgSplineNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSplineNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgSplineNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSplineNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgSplineNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSplineNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SplineNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}