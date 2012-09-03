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
// Name : cgHingeJointObject.cpp                                             //
//                                                                           //
// Desc : Class implementing a single axis hinge joint. This joint can be    //
//        used to connect two bodies around a central axis and allow them    //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHingeJointObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgHingeJointObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <Physics/Joints/cgHingeJoint.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>

// ToDo: 6767 -- needs database updates and onComponentModified() so that joint in the node can be updated when properties change.

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgHingeJointObject::mInsertJoint;
cgWorldQuery cgHingeJointObject::mLoadJoint;
cgWorldQuery cgHingeJointNode::mInsertInstanceData;
cgWorldQuery cgHingeJointNode::mDeleteInstanceData;
cgWorldQuery cgHingeJointNode::mLoadInstanceData;

///////////////////////////////////////////////////////////////////////////////
// cgHingeJointObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHingeJointObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::cgHingeJointObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgJointObject( nReferenceId, pWorld )
{
    mUseLimits    = true;
    mMinimumAngle = -45.0f;
    mMaximumAngle = 45.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgHingeJointObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::cgHingeJointObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgJointObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgHingeJointObject * pObject = (cgHingeJointObject*)pInit;
    mUseLimits      = pObject->mUseLimits;
    mMinimumAngle   = pObject->mMinimumAngle;
    mMaximumAngle   = pObject->mMaximumAngle;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHingeJointObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointObject::~cgHingeJointObject()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgHingeJointObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgHingeJointObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgHingeJointObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgHingeJointObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgHingeJointObject::getLocalBoundingBox( )
{
    // Set to current position by default
    return cgBoundingBox( 0,0,0,0,0,0 ); 
}

//-----------------------------------------------------------------------------
//  Name : enableLimits ()
/// <summary>
/// Enable angle of rotation limiting for this hinge joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::enableLimits( bool enabled )
{
    // ToDo: 6767 -- Update database
    mUseLimits = enabled;
}

//-----------------------------------------------------------------------------
//  Name : setLimits ()
/// <summary>
/// Set the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::setLimits( const cgRangeF & range )
{
    setLimits( range.min, range.max );
}

//-----------------------------------------------------------------------------
//  Name : setLimits ()
/// <summary>
/// Set the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::setLimits( cgFloat minDegrees, cgFloat maxDegrees )
{
    // ToDo: 6767 -- Clamp min/max etc. and update database.
    mMinimumAngle = CGEToRadian( minDegrees );
    mMaximumAngle = CGEToRadian( maxDegrees );
}

//-----------------------------------------------------------------------------
//  Name : isLimited ()
/// <summary>
/// Is angle of rotation limiting for this hinge joint enabled?
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::isLimited( ) const
{
    return mUseLimits;
}

//-----------------------------------------------------------------------------
//  Name : getLimits ()
/// <summary>
/// Get the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgRangeF cgHingeJointObject::getLimits( ) const
{
    return cgRangeF(mMinimumAngle,mMaximumAngle);
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Compute the distance from the origin of the joint to the edge of
    // the parent object oriented box.
    cgVector3 vJointPos(0, 0, 0);
    cgFloat fZoomFactor = 0.0f;
    cgFloat fArrowStalkLength = 0.0f;
    cgFloat fCircleRadius = 0.0f;
    if ( false && pIssuer->getParent() )
    {
        // Joint will be rendered at the center of the parent' bounds.
        cgTransform ParentTransform = pIssuer->getParent()->getWorldTransform( false );
        cgBoundingBox LocalBounds = pIssuer->getParent()->getLocalBoundingBox( );
        cgBoundingBox WorldBounds = LocalBounds;
        WorldBounds.transform( ParentTransform );
        vJointPos = WorldBounds.getCenter();

        // Compute zoom factor for the selected joint location.
        fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vJointPos, 2.5f );

        // We want to find out the distance to the edge of the parent's
        // OOBB from the joint along the joint's fixed axis direction.
        // This will allow us to draw an arrow that is long enough such
        // that is always visible out of the 'top' of the object. We
        // achieve this using ray-casting, so first transform the relevant
        // position and direction into the space of the parent's OOBB.
        cgTransform InverseParentTransform;
        cgTransform::inverse( InverseParentTransform, ParentTransform );
        cgVector3 vEdgeSearchOrigin = vJointPos;
        cgVector3 vEdgeSearchDir    = pIssuer->getYAxis( false );
        InverseParentTransform.transformCoord( vEdgeSearchOrigin, vEdgeSearchOrigin );
        InverseParentTransform.transformNormal( vEdgeSearchDir, vEdgeSearchDir );
        cgVector3::normalize( vEdgeSearchDir, vEdgeSearchDir );
        
        // Cast ray from a point known to be *outside* of the OOBB
        // toward the *interior* in order to find the edge intersection point
        cgFloat t;
        vEdgeSearchOrigin = vEdgeSearchOrigin + vEdgeSearchDir * 1000.0f;
        vEdgeSearchDir    = -vEdgeSearchDir;
        if ( LocalBounds.intersect( vEdgeSearchOrigin, vEdgeSearchDir, t, false ) )
        {
            cgVector3 vIntersect = vEdgeSearchOrigin + vEdgeSearchDir * t;
            ParentTransform.transformCoord( vIntersect, vIntersect );
            fArrowStalkLength = cgVector3::length( vIntersect - vJointPos );
        
        } // End if intersecting

        // Compute the radius necessary to draw a circle around the parent node
        // designed to represent the axis about which it can yaw.
        cgTransform InverseObjectTransform;
        cgTransform ObjectTransform = pIssuer->getWorldTransform( false );
        ObjectTransform.setPosition( vJointPos );
        cgTransform::inverse( InverseObjectTransform, ObjectTransform );
        LocalBounds.transform( ParentTransform * InverseObjectTransform );
        cgVector3 vExtents = LocalBounds.getExtents();
        fCircleRadius = ((vExtents.x > vExtents.z) ? vExtents.x : vExtents.z);

        // Transform the final center position into a local space offset
        // so that we can pick appropriately.
        pIssuer->getWorldTransform( false ).inverseTransformCoord( vJointPos, vJointPos );
    
    } // End if has parent
    else
    {
        // Compute zoom factor for the selected joint location.
        fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition(false), 2.5f );
    
    } // End if no parent
    
    // Add the arrow and radius padding (this is also the default
    // length and radius in cases where there is no parent).
    fArrowStalkLength += 20.0f * fZoomFactor;
    fCircleRadius += 10.0f * fZoomFactor;

    // Compute vertices for the tip of the directional light source arrow
    cgVector3 Points[5];
    cgFloat fSize = fZoomFactor * 7.5f;
    Points[0] = cgVector3( 0, fArrowStalkLength + fZoomFactor * 10.0f, 0 ) + vJointPos;
    Points[1] = cgVector3( -fSize, fArrowStalkLength,  fSize ) + vJointPos;
    Points[2] = cgVector3( -fSize, fArrowStalkLength, -fSize ) + vJointPos;
    Points[3] = cgVector3(  fSize, fArrowStalkLength, -fSize ) + vJointPos;
    Points[4] = cgVector3(  fSize, fArrowStalkLength,  fSize ) + vJointPos;

    // ...and indices.
    cgUInt32  Indices[12];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;

    // First check to see if the ray intersects the arrow tip pyramid
    cgFloat   t, tMin = FLT_MAX;
    bool      bIntersect = false;
    cgFloat fTolerance = 2.0f * fZoomFactor;
    for ( size_t i = 0; i < 4; ++i )
    {
        // Compute plane for the current triangle
        cgPlane Plane;
        const cgVector3 & v1 = Points[Indices[(i*3)+0]];
        const cgVector3 & v2 = Points[Indices[(i*3)+1]];
        const cgVector3 & v3 = Points[Indices[(i*3)+2]];
        cgPlane::fromPoints( Plane, v1, v2, v3 );

        // Determine if (and where) the ray intersects the triangle's plane
        if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, false, false ) == false )
            continue;
        
        // Check if it intersects the actual triangle (within a tolerance)
        cgVector3 vIntersect = vOrigin + (vDir * t);
        if ( cgCollision::pointInTriangle( vIntersect, v1, v2, v3, (cgVector3&)Plane, fTolerance ) == false )
            continue;

        // We intersected! Record the intersection distance.
        bIntersect = true;
        if ( t < tMin ) tMin = t;

    } // Next Triangle

    // Also check to see if the ray intersects the outward facing base of the pyramid
    cgPlane Plane;
    cgPlane::fromPointNormal( Plane,
                              cgVector3( 0, fArrowStalkLength, 0 ) + vJointPos, 
                              cgVector3( 0, -1, 0 ) );
    if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, false, false ) == true )
    {
        // Check to see if it falls within the quad at the base of the pyramid
        // (Simple to do when working in object space as we are here).
        cgVector3 vIntersect = vOrigin + (vDir * t);
        if ( fabsf( vIntersect.x ) < fSize + fTolerance && fabsf( vIntersect.z ) < fSize + fTolerance )
        {
            bIntersect = true;
            if ( t < tMin ) tMin = t;
        
        } // End if intersects quad

    } // End if intersects plane

    // Now check to see if the ray intersects the arrow "stalk" (simple AABB test when in object space)
    fSize = (fZoomFactor * 4.0f) + fTolerance;
    cgBoundingBox StalkAABB( cgVector3( -fSize, -fTolerance, -fSize ) + vJointPos,
                             cgVector3( fSize, fArrowStalkLength + fTolerance, fSize ) + vJointPos ); 
    if ( StalkAABB.intersect( vOrigin, vDir, t, false ) == true )
    {
        bIntersect = true;
        if ( t < tMin ) tMin = t;

    } // End if intersects stalk aabb

    // Finally, check the rotation axis circle. For this, we will first 
    // determine if the ray intersects a sphere with the same radius. 
    if ( cgCollision::rayIntersectSphere( vOrigin, vDir, vJointPos, fCircleRadius, t ) )
    {
        cgVector3 vSphereIntersect = vOrigin + (vDir * t);

        // It intersected the sphere. Now we check to see if the 
        // intersection normal is close to this axis' circle.
        if ( fabsf( cgVector3::dot( (vSphereIntersect-vJointPos), cgVector3(0,1,0) ) ) < (5.0f * fZoomFactor) )
        {
            // Intersection found
            bIntersect = true;
            if ( t < tMin ) tMin = t;
        
        } // End if close to circle
    
    } // End if intersected sphere

    // Return final intersection distance (if we hit anything)
    if ( bIntersect )
    {
        fDistance = tMin;
        return true;
    
    } // End if intersected

    // No intersection.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgShadedVertex  Points[30];
    cgUInt32        Indices[35];
    bool            bCull;
    cgUInt32        i;

    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    cgUInt32 nColor      = (pIssuer->isSelected() == true) ? 0xFFFFFFFF : 0xFF22AAFF;
    bool     bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);

    // Compute the distance from the origin of the joint to the edge of
    // the parent object oriented box.
    cgVector3 vJointPos(0, 0, 0);
    cgFloat fZoomFactor = 0.0f;
    cgFloat fArrowStalkLength = 0.0f;
    cgFloat fCircleRadius = 0.0f;
    cgTransform ObjectTransform = pIssuer->getWorldTransform( false );
    if ( false && pIssuer->getParent() )
    {
        // Joint will be rendered at the center of the parent' bounds.
        cgTransform ParentTransform = pIssuer->getParent()->getWorldTransform( false );
        cgBoundingBox LocalBounds = pIssuer->getParent()->getLocalBoundingBox( );
        cgBoundingBox WorldBounds = LocalBounds;
        WorldBounds.transform( ParentTransform );
        vJointPos = WorldBounds.getCenter();

        // Compute zoom factor for the selected joint location.
        fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, vJointPos, 2.5f );

        // We want to find out the distance to the edge of the parent's
        // OOBB from the joint along the joint's fixed axis direction.
        // This will allow us to draw an arrow that is long enough such
        // that is always visible out of the 'top' of the object. We
        // achieve this using ray-casting, so first transform the relevant
        // position and direction into the space of the parent's OOBB.
        cgTransform InverseParentTransform;
        cgTransform::inverse( InverseParentTransform, ParentTransform );
        cgVector3 vEdgeSearchOrigin = vJointPos;
        cgVector3 vEdgeSearchDir    = pIssuer->getYAxis( false );
        InverseParentTransform.transformCoord( vEdgeSearchOrigin, vEdgeSearchOrigin );
        InverseParentTransform.transformNormal( vEdgeSearchDir, vEdgeSearchDir );
        cgVector3::normalize( vEdgeSearchDir, vEdgeSearchDir );
        
        // Cast ray from a point known to be *outside* of the OOBB
        // toward the *interior* in order to find the edge intersection point
        cgFloat t;
        vEdgeSearchOrigin = vEdgeSearchOrigin + vEdgeSearchDir * 1000.0f;
        vEdgeSearchDir    = -vEdgeSearchDir;
        if ( LocalBounds.intersect( vEdgeSearchOrigin, vEdgeSearchDir, t, false ) )
        {
            cgVector3 vIntersect = vEdgeSearchOrigin + vEdgeSearchDir * t;
            ParentTransform.transformCoord( vIntersect, vIntersect );
            fArrowStalkLength = cgVector3::length( vIntersect - vJointPos );
        
        } // End if intersecting

        // Render the joint at the center of the parent's bounds
        ObjectTransform.setPosition( vJointPos );

        // Compute the radius necessary to draw a circle around the parent node
        // designed to represent the axis about which it can yaw.
        cgTransform InverseObjectTransform;
        cgTransform::inverse( InverseObjectTransform, ObjectTransform );
        LocalBounds.transform( ParentTransform * InverseObjectTransform );
        cgVector3 vExtents = LocalBounds.getExtents();
        fCircleRadius = ((vExtents.x > vExtents.z) ? vExtents.x : vExtents.z);

        //pDriver->DrawOOBB( LocalBounds, 0.0f, ObjectTransform, 0xFFFF00FF, true );
    
    } // End if has parent
    else
    {
        // Compute zoom factor for the selected joint location.
        fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition(false), 2.5f );
    
    } // End if no parent

    // Add the arrow and radius padding (this is also the default
    // length and radius in cases where there is no parent).
    fArrowStalkLength += 20.0f * fZoomFactor;
    fCircleRadius += 10.0f * fZoomFactor;

    // Set the object's transformation matrix to the driver (joint
    // representation will be constructed in object space along +Y)
    cgTransform InverseObjectTransform;
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    pDriver->setWorldTransform( ObjectTransform );

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    for ( i = 0; i < 30; ++i )
        Points[i].color = nColor;

    // Compute vertices for the tip of the axis arrow
    cgFloat fSize = fZoomFactor * 7.5f;
    Points[0].position = cgVector3( 0, fArrowStalkLength + fZoomFactor * 10.0f, 0 );
    Points[1].position = cgVector3( -fSize, fArrowStalkLength,  fSize );
    Points[2].position = cgVector3( -fSize, fArrowStalkLength, -fSize );
    Points[3].position = cgVector3(  fSize, fArrowStalkLength, -fSize );
    Points[4].position = cgVector3(  fSize, fArrowStalkLength,  fSize );
    
    // Compute indices that will allow us to draw the arrow tip using a 
    // tri-list (wireframe) so that we can easily take advantage of back-face culling.
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;

    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    pDriver->setWorldTransform( ObjectTransform );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Draw the arrow tip
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 5, 4, Indices, cgBufferFormat::Index32, Points );

            // Now we'll render the base of the arrow and the "stalk".
            // So that we can construct in object space, transform camera culling details
            // into object space too.
            cgVector3 vCameraLook, vCameraPos;
            InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis(false) );
            InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition(false) );
            cgVector3::normalize( vCameraLook, vCameraLook );
            
            // Construct the vertices. First the "stalk" / cube vertices.
            // Render each of the cube faces as line lists (manual back face culling)
            fSize              = fZoomFactor * 4.0f;
            Points[0].position = cgVector3( -fSize, 0.0f,               fSize );
            Points[1].position = cgVector3( -fSize, fArrowStalkLength,  fSize );
            Points[2].position = cgVector3(  fSize, fArrowStalkLength,  fSize );
            Points[3].position = cgVector3(  fSize, 0.0f,               fSize );
            Points[4].position = cgVector3( -fSize, 0.0f,              -fSize );
            Points[5].position = cgVector3( -fSize, fArrowStalkLength, -fSize );
            Points[6].position = cgVector3(  fSize, fArrowStalkLength, -fSize );
            Points[7].position = cgVector3(  fSize, 0.0f,              -fSize );

            // Stalk +X Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 1,0,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 1,0,0 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            cgUInt32 nCount = 0;
            if ( !bCull )
            {
                Indices[nCount++] = 3; Indices[nCount++] = 2;
                Indices[nCount++] = 2; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 3;

            } // End if !cull

            // -X Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( -1,0,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( -1,0,0 ), fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( !bCull )
            {
                Indices[nCount++] = 1; Indices[nCount++] = 0;
                Indices[nCount++] = 0; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 1;

            } // End if !cull

            // -Y Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,-1,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,-1,0 ), 0.0f ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( !bCull )
            {
                Indices[nCount++] = 0; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 0;

            } // End if !cull

            // +Z Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,0,1 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,1 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( !bCull )
            {
                Indices[nCount++] = 0; Indices[nCount++] = 1;
                Indices[nCount++] = 1; Indices[nCount++] = 2;
                Indices[nCount++] = 2; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 0;

            } // End if !cull

            // -Z Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,0,-1 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( !bCull )
            {
                Indices[nCount++] = 7; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 7;

            } // End if !cull

            // Now construct the "base" of the arrow tip pyramid.
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,-1,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,-1, 0 ), fZoomFactor * 20.0f ) != cgPlaneQuery::Front);
            
            // Build base vertices / indices
            if ( !bCull )
            {
                fSize               = fZoomFactor * 7.5f;
                Points[8].position  = cgVector3( -fSize, fArrowStalkLength, -fSize );
                Points[9].position  = cgVector3( -fSize, fArrowStalkLength,  fSize );
                Points[10].position = cgVector3(  fSize, fArrowStalkLength,  fSize );
                Points[11].position = cgVector3(  fSize, fArrowStalkLength, -fSize );

                Indices[nCount++] = 8 ; Indices[nCount++] = 9;
                Indices[nCount++] = 9 ; Indices[nCount++] = 10;
                Indices[nCount++] = 10; Indices[nCount++] = 11;
                Indices[nCount++] = 11; Indices[nCount++] = 8;

            } // End if !cull

            // Draw the rest of the light source representation
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 16, nCount / 2, Indices, cgBufferFormat::Index32, Points );

            // Generate line strip circle to show the axis of rotation.
            for ( i = 0; i < 30; ++i )
            {
                // Build vertex
                cgVector3 vPoint;
                vPoint.x = (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)i ) * fCircleRadius);
                vPoint.y = 0.0f;
                vPoint.z = (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)i ) * fCircleRadius);
                Points[i] = cgShadedVertex( vPoint, nColor );

                // Build index
                Indices[i] = i;
                
            } // Next Segment

            // Add last index to connect back to the start of the circle
            Indices[30] = 0;

            // Render the lines.
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, Indices, cgBufferFormat::Index32, Points );

        } // End if begun pass

        // We have finished rendering
        pShader->endTechnique();
    
    } // End if begun technique

    // Call base class implementation last.
    cgJointObject::sandboxRender( pCamera, pVisData, bWireframe, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in this implementation

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
bool cgHingeJointObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HingeJointObject )
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
cgString cgHingeJointObject::getDatabaseTable( ) const
{
    return _T("Objects::HingeJoint");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
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
bool cgHingeJointObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("HingeJointObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertJoint.bindParameter( 1, mReferenceId );
        mInsertJoint.bindParameter( 2, mUseLimits );
        mInsertJoint.bindParameter( 3, mMinimumAngle );
        mInsertJoint.bindParameter( 4, mMaximumAngle );
        mInsertJoint.bindParameter( 5, mSoftRefCount );

        // Execute
        if ( mInsertJoint.step( true ) == false )
        {
            cgString strError;
            mInsertJoint.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for hinge joint object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("HingeJointObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("HingeJointObject::insertComponentData") );

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
bool cgHingeJointObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the joint data.
    prepareQueries();
    mLoadJoint.bindParameter( 1, e->sourceRefId );
    if ( !mLoadJoint.step( ) || !mLoadJoint.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadJoint.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hinge joint object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hinge joint object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadJoint.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadJoint;

    // Update our local members
    mLoadJoint.getColumn( _T("UseLimits"), mUseLimits );
    mLoadJoint.getColumn( _T("MinimumAngle"), mMinimumAngle );
    mLoadJoint.getColumn( _T("MaximumAngle"), mMaximumAngle );

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
void cgHingeJointObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertJoint.isPrepared() == false )
            mInsertJoint.prepare( mWorld, _T("INSERT INTO 'Objects::HingeJoint' VALUES(?1,?2,?3,?4,?5)"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadJoint.isPrepared() == false )
        mLoadJoint.prepare( mWorld, _T("SELECT * FROM 'Objects::HingeJoint' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgHingeJointNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHingeJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::cgHingeJointNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgJointNode( nReferenceId, pScene )
{
    // Initialize members to sensible defaults
    mJoint      = CG_NULL;
    mBody0RefId = 0;
    mBody1RefId = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgHingeJointNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::cgHingeJointNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize members to sensible defaults
    mJoint = CG_NULL;

    // Duplicate data
    cgHingeJointNode * hingeJoint = (cgHingeJointNode*)pInit;
    mBody0RefId = hingeJoint->mBody0RefId;
    mBody1RefId = hingeJoint->mBody1RefId;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHingeJointNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJointNode::~cgHingeJointNode()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgHingeJointNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release internal references.
    if ( mJoint )
        mJoint->removeReference( this );
    mJoint = CG_NULL;
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgJointNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgHingeJointNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHingeJointNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgHingeJointNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HingeJointNode )
        return true;

    // Supported by base?
    return cgJointNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame = _T("Joint");

    // Compute the distance from the origin of the joint to the edge of
    // the parent object oriented box.
    cgFloat fZoomFactor = 0.0f;
    cgFloat fArrowStalkLength = 0.0f;
    cgVector3 vJointPos = getPosition(false);
    if ( false && mParentNode )
    {
        // Joint will be rendered at the center of the parent' bounds.
        cgTransform ParentTransform = mParentNode->getWorldTransform( false );
        cgBoundingBox LocalBounds = mParentNode->getLocalBoundingBox( );
        cgBoundingBox WorldBounds = LocalBounds;
        WorldBounds.transform( ParentTransform );
        vJointPos = WorldBounds.getCenter();

        // We want to find out the distance to the edge of the parent's
        // OOBB from the joint along the joint's fixed axis direction.
        // This will allow us to draw an arrow that is long enough such
        // that is always visible out of the 'top' of the object. We
        // achieve this using ray-casting, so first transform the relevant
        // position and direction into the space of the parent's OOBB.
        cgTransform InverseParentTransform;
        cgTransform::inverse( InverseParentTransform, ParentTransform );
        cgVector3 vEdgeSearchOrigin = vJointPos;
        cgVector3 vEdgeSearchDir    = getYAxis( false );
        InverseParentTransform.transformCoord( vEdgeSearchOrigin, vEdgeSearchOrigin );
        InverseParentTransform.transformNormal( vEdgeSearchDir, vEdgeSearchDir );
        cgVector3::normalize( vEdgeSearchDir, vEdgeSearchDir );
        
        // Cast ray from a point known to be *outside* of the OOBB
        // toward the *interior* in order to find the edge intersection point
        cgFloat t;
        vEdgeSearchOrigin = vEdgeSearchOrigin + vEdgeSearchDir * 10000.0f;
        vEdgeSearchDir    = -vEdgeSearchDir;
        if ( LocalBounds.intersect( vEdgeSearchOrigin, vEdgeSearchDir, t, false ) )
        {
            cgVector3 vIntersect = vEdgeSearchOrigin + vEdgeSearchDir * t;
            ParentTransform.transformCoord( vIntersect, vIntersect );
            fArrowStalkLength = cgVector3::length( vIntersect - vJointPos );
        
        } // End if intersecting
    
    } // End if has parent
    
    // Compute zoom factor for the selected joint location.
    fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vJointPos, 2.5f );
    
    // Add the arrow padding, and the distance from the base to the tip of the pyramid.
    fArrowStalkLength += 30.0f * fZoomFactor;

    // Position icon
    vIconOrigin  = vJointPos + (getYAxis(false) * fArrowStalkLength);
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 13.0f * fZoomFactor);
    
    // Draw icon!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform()
/// <summary>
/// Update our internal cell matrix with that specified here. Optionally, you
/// can disable the recomputation of the local (parent relative) transformation
/// matrix (defaults to true). This is generally only used internally in order
/// to reduce introducing accumulated errors into the transformation matrix but
/// can be supplied if you know that this does not need to be performed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgJointNode::setCellTransform( Transform, Source ) )
        return false;

    // Update the transform of the underlying hinge joint as we move.
    if ( mJoint )
        mJoint->setPivotTransform( getWorldTransform(false) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgHingeJointNode::getLocalBoundingBox( )
{
    if ( mParentNode )
    {
        // Use the parent's bounding box transformed into the
        // space of the rendered joint.
        cgBoundingBox LocalBounds = mParentNode->getLocalBoundingBox();
        cgTransform InverseObjectTransform;
        cgTransform::inverse( InverseObjectTransform, getWorldTransform( false ) );
        LocalBounds.transform( mParentNode->getWorldTransform( false ) * InverseObjectTransform );
        return LocalBounds;

    } // End if has parent

    // Pass through to the default handler.
    return cgJointNode::getLocalBoundingBox();
}

//-----------------------------------------------------------------------------
// Name : rebuildJoint( ) (Protected)
/// <summary>
/// Create the internal joint if all parent/child connections have 
/// been made, or destroy the joint if any are removed.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::rebuildJoint( )
{
    // Destroy any prior joint
    if ( mJoint )
        mJoint->removeReference( this );
    mJoint = CG_NULL;

    // Have all pre-requisites been met with regards child attachments?
    bool validConnections = (mBody0RefId && mBody1RefId);

    // If we have all valid connections, rebuild the joint.
    if ( validConnections )
    {
        cgObjectNode * body0Node = static_cast<cgObjectNode*>(cgReferenceManager::getReference(mBody0RefId));
        cgObjectNode * body1Node = static_cast<cgObjectNode*>(cgReferenceManager::getReference(mBody1RefId));
        cgPhysicsBody * body0 = (body0Node) ? body0Node->getPhysicsBody() : CG_NULL;
        cgPhysicsBody * body1 = (body1Node) ? body1Node->getPhysicsBody() : CG_NULL;
        if ( body0 && body1 )
        {
            mJoint = new cgHingeJoint( mParentScene->getPhysicsWorld(), body0, body1, getWorldTransform(false) );
            mJoint->addReference( this );
            mJoint->enableLimits( isLimited() );
            mJoint->setLimits( getLimits().min, getLimits().max );
        
        } // End if has physics bodies
    
    } // End if valid
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgJointNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    cgToDo( "Cloning", "Take care because the references may change in 'onNodeInit()'." )

    // Insert references to attached objects if there are any already defined (i.e. from cloning?)
    // Even if there are not, we should insert a row into the instance data table so that we can
    // simply update it as and one objects are attached / detached.
    if ( shouldSerialize() )
    {
        // Insert entry
        prepareQueries();
        mInsertInstanceData.bindParameter( 1, mReferencedObject->getReferenceId() );
        mInsertInstanceData.bindParameter( 2, mReferenceId );
        mInsertInstanceData.bindParameter( 3, mBody0RefId );
        mInsertInstanceData.bindParameter( 4, mBody1RefId );
        if ( mInsertInstanceData.step( true ) == false )
        {
            cgString error;
            mInsertInstanceData.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert instance data for hinge joint node '0x%x'. Error: %s"), mReferenceId, error.c_str() );
            return false;

        } // End if failed

    } // End if serialize

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
bool cgHingeJointNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgJointNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Retrieve the *original* reference identifier we're loading from. We need
    // this so that we can load the instance data for the original node.
    cgUInt32 originalRefId = 0;
    pNodeData->getColumn( _T("RefId"), originalRefId );

    // Load the attached bone list.
    prepareQueries();
    mLoadInstanceData.bindParameter( 1, originalRefId );
    if ( !mLoadInstanceData.step( ) || !mLoadInstanceData.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( mLoadInstanceData.getLastError( error ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve instance data for hinge joint object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve instance data for hinge joint object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadInstanceData.reset();
        return false;

    } // End if failed

    // Get the loaded data.
    mLoadInstanceData.getColumn( _T("Body0Id"), mBody0RefId );
    mLoadInstanceData.getColumn( _T("Body1Id"), mBody1RefId );
    mLoadInstanceData.reset();

    cgToDo( "Spawning", "Insert cloned instance data? Take care because the references may change in 'onNodeInit()'." )

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the  node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeInit( const cgUInt32IndexMap & NodeReferenceRemap )
{
    // Remap object references appropriately.
    if ( !NodeReferenceRemap.empty() )
    {
        cgUInt32IndexMap::const_iterator itRemap = NodeReferenceRemap.find( mBody0RefId );
        if ( itRemap != NodeReferenceRemap.end() )
            mBody0RefId = itRemap->second;
        itRemap = NodeReferenceRemap.find( mBody1RefId );
        if ( itRemap != NodeReferenceRemap.end() )
            mBody1RefId = itRemap->second;

    } // End if remapping

    // Rebuild the joint if possible.
    rebuildJoint();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// deleted in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHingeJointNode::onNodeDeleted( )
{
    // Delete attached bone reference list.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mDeleteInstanceData.bindParameter( 1, mReferenceId );
        if ( !mDeleteInstanceData.step( true ) )
        {
            // Log any error.
            cgString error;
            if ( mDeleteInstanceData.getLastError( error ) == false )
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete instance data for hinge joint object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Failed to delete instance data for hinge joint object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;

        } // End if failed

    } // End if should serialize

    // Call base class implementation last.
    return cgJointNode::onNodeDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJointNode::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParentScene->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertInstanceData.isPrepared() )
            mInsertInstanceData.prepare( pWorld, _T("INSERT INTO 'Objects::HingeJoint::InstanceData' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( !mDeleteInstanceData.isPrepared() )
            mDeleteInstanceData.prepare( pWorld, _T("DELETE FROM 'Objects::HingeJoint::InstanceData' WHERE NodeRefId=?1"), true );

    } // End if sandbox

    // Read queries
    if ( !mLoadInstanceData.isPrepared() )
        mLoadInstanceData.prepare( pWorld, _T("SELECT * FROM 'Objects::HingeJoint::InstanceData' WHERE NodeRefId=?1"), true );
}