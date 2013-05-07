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
// Name : cgPointLightObject.cpp                                             //
//                                                                           //
// Desc : Point / omni-directional light source classes.                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPointLightObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgPointLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgTexturePool.h>
#include <Resources/cgMesh.h>
#include <Math/cgMathUtility.h>
#include <Math/cgCollision.h>
#include <Math/cgExtrudedBoundingBox.h>
#include <System/cgStringUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgPointLightObject::mInsertPointLight;
cgWorldQuery cgPointLightObject::mUpdateRanges;
cgWorldQuery cgPointLightObject::mUpdateShadowConfigSources;
cgWorldQuery cgPointLightObject::mUpdateShadowRate;
cgWorldQuery cgPointLightObject::mLoadPointLight;

///////////////////////////////////////////////////////////////////////////////
// cgPointLightObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPointLightObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightObject::cgPointLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgLightObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mOuterRange             = 0.0f;
    mInnerRange             = 0.0f;
    mRangeAdjust            = 1.1f;
    mShadowUpdateRate       = 0;
    mShadowMapEdgeProcess   = CaptureBorder; //NoProcessing; //FullRepair; //CaptureBorder; // ToDo: Configure this in Carbon Forge
    
    // Clear arrays
    // ToDo: 6767 - Reintroduce for new shadow settings.
    //memset( mShadowFrustumIds, 0, 6 * sizeof(cgUInt32) );
}

//-----------------------------------------------------------------------------
//  Name : cgPointLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightObject::cgPointLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgLightObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgPointLightObject * pObject = (cgPointLightObject*)pInit;
    mOuterRange                = pObject->mOuterRange;
    mInnerRange                = pObject->mInnerRange;
    mShadowUpdateRate          = pObject->mShadowUpdateRate;
    mRangeAdjust               = pObject->mRangeAdjust;
    mShadowMapEdgeProcess       = pObject->mShadowMapEdgeProcess;

    // Duplicate shadow frustum configuration
    /*memset( mShadowFrustumIds, 0, 6 * sizeof(cgUInt32) );
    for ( cgInt i = 0; i < 6; ++i )
        m_pShadowConfig[i] = pObject->m_pShadowConfig[i];*/
}

//-----------------------------------------------------------------------------
//  Name : ~cgPointLightObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightObject::~cgPointLightObject()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( bDisposeBase )
        cgLightObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgPointLightObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgPointLightObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgPointLightObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgPointLightObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgPointLightObject::getDatabaseTable( ) const
{
    return _T("Objects::PointLight");
}

//-----------------------------------------------------------------------------
//  Name : setOuterRange()
/// <summary>
/// Set the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::setOuterRange( cgFloat fRange )
{
    // Is this a no-op?
    if ( mOuterRange == fRange )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, fRange );
        mUpdateRanges.bindParameter( 2, mInnerRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString strError;
            mUpdateRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store the new light source range
    mOuterRange = fabsf(fRange);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("OuterRange");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner range is less than the outer range
    if ( mInnerRange > mOuterRange )
        setInnerRange( mOuterRange );
}

//-----------------------------------------------------------------------------
//  Name : setInnerRange()
/// <summary>
/// Set the inner range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::setInnerRange( cgFloat fRange )
{
    // Is this a no-op?
    if ( mInnerRange == fRange )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, mOuterRange );
        mUpdateRanges.bindParameter( 2, fRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString strError;
            mUpdateRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store the new light source range
    mInnerRange = fabsf(fRange);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("InnerRange");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer range is at LEAST as large as the inner range
    if ( mInnerRange > mOuterRange )
        setOuterRange( mInnerRange );
}

//-----------------------------------------------------------------------------
//  Name : setRangeAdjust()
/// <summary>
/// Set the range adjustment scalar for this light source. This is used 
/// primarily to ensure that the light volume always fits within the boundaries
/// of the lower resolution light shape geometry which is expanded based on
/// this scalar.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::setRangeAdjust( cgFloat fAdjust )
{
    // Is this a no-op?
    if ( mRangeAdjust == fAdjust )
        return;

    /*// Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        m_qUpdateRange.bindParameter( 1, fRange );
        m_qUpdateRange.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !m_qUpdateRange.step( true ) )
        {
            cgString strError;
            m_qUpdateRange.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update range for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store the new light source range
    mRangeAdjust = fabsf(fRange);*/

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("RangeAdjust");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setShadowUpdateRate ()
/// <summary>
/// Set the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::setShadowUpdateRate( cgUInt32 nRate )
{
    // Is this a no-op?
    if ( mShadowUpdateRate == nRate )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateShadowRate.bindParameter( 1, nRate );
        mUpdateShadowRate.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateShadowRate.step( true ) )
        {
            cgString strError;
            mUpdateShadowRate.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow rate for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
    mShadowUpdateRate = nRate;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ShadowUpdateRate");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setShadowMapEdgeProcess ()
/// <summary>
/// Set the process that should be used to correctly blend shadow map edges
/// in order to remove unsightly border artifacts where necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::setShadowMapEdgeProcess( ShadowMapProcess Process )
{
    // Is this a no-op?
    if ( mShadowMapEdgeProcess == Process )
        return;

    // ToDo: 9999 - Update database

    // Update internal value.
    mShadowMapEdgeProcess = Process;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ShadowMapEdgeProcess");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

/*//-----------------------------------------------------------------------------
//  Name : SetShadowFrustumConfig ()
/// <summary>
/// Set the base shadow frustum configuration parameters.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::SetShadowFrustumConfig( cgUInt32 nFrustum, const cgShadowGeneratorConfig & Config )
{
    // Validate parameters
    if ( nFrustum >= 6 )
        return;

    // ToDo: No-Op Processing.

    // Update database first
    if ( m_bFrustumsLinked )
    {
        mWorld->beginTransaction( _T("setShadowFrustumConfig") );
        for ( cgInt i = 0; i < 6; ++i )
        {    
            if ( mShadowFrustumIds[i] != 0 && !UpdateShadowFrustum( mShadowFrustumIds[i], Config ) )
            {
                mWorld->rollbackTransaction( _T("setShadowFrustumConfig") );
                return;

            } // End if failed

        } // Next Frustum
        mWorld->commitTransaction( _T("setShadowFrustumConfig") );

    } // End if linked
    else
    {
        if ( mShadowFrustumIds[nFrustum] != 0 && !UpdateShadowFrustum( mShadowFrustumIds[nFrustum], Config ) )
            return;

    } // End if !linked
    
    // If frustums are linked we should update all 6 frustum configurations.
    if ( m_bFrustumsLinked )
    {
        for ( cgInt i = 0; i < 6; ++i )
            m_pShadowConfig[i] = Config;

        // Notify listeners that object data has changed.
        static const cgString strContext = _T("ShadowFrustumConfig");
        onComponentModified( &cgComponentModifiedEventArgs( strContext, cgInt32(-1) ) );
    
    } // End if linked
    else
    {
        m_pShadowConfig[nFrustum] = Config;

        // Notify listeners that object data has changed.
        static const cgString strContext = _T("ShadowFrustumConfig");
        onComponentModified( &cgComponentModifiedEventArgs( strContext, (cgInt32)nFrustum ) );
    
    } // End if !linked
}*/

//-----------------------------------------------------------------------------
//  Name : getOuterRange()
/// <summary>
/// Get the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPointLightObject::getOuterRange( ) const
{
    return mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerRange()
/// <summary>
/// Get the inner range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPointLightObject::getInnerRange( ) const
{
    return mInnerRange;
}

//-----------------------------------------------------------------------------
//  Name : getRangeAdjust ()
/// <summary>
/// Get the range adjustment scalar, used to apply tolerances in certain volume
/// testing and rendering situations.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPointLightObject::getRangeAdjust( ) const
{
    return mRangeAdjust;
}

//-----------------------------------------------------------------------------
//  Name : getShadowUpdateRate ()
/// <summary>
/// Get the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgPointLightObject::getShadowUpdateRate( ) const
{
    return mShadowUpdateRate;
}

//-----------------------------------------------------------------------------
//  Name : getShadowMapEdgeProcess ()
/// <summary>
/// Get the process that is being used to correctly blend shadow map edges
/// in order to remove unsightly border artifacts where necessary.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightObject::ShadowMapProcess cgPointLightObject::getShadowMapEdgeProcess(  ) const
{
    return mShadowMapEdgeProcess;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgPointLightObject::getLocalBoundingBox( )
{
    cgBoundingBox Bounds;

    // Compute light bounding box
    Bounds.min = cgVector3( -mOuterRange, -mOuterRange, -mOuterRange );
    Bounds.max = cgVector3(  mOuterRange,  mOuterRange,  mOuterRange );
    return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    cgFloat   fZoomFactor  = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition( false ), 2.5f );
    cgVector3 vRight       = cgVector3(1,0,0);
    cgVector3 vUp          = cgVector3(0,1,0);
    cgVector3 vLook        = cgVector3(0,0,1);
    
    // Compute vertices for the light source representation (central diamond)
    cgVector3 Points[6];
    Points[0] = vUp * (10.0f * fZoomFactor);
    Points[1] = vLook * (10.0f * fZoomFactor);
    Points[2] = vRight * (10.0f * fZoomFactor);
    Points[3] = -vLook * (10.0f * fZoomFactor);
    Points[4] = -vRight * (10.0f * fZoomFactor);
    Points[5] = -vUp * (10.0f * fZoomFactor);

    // Construct indices for the 8 triangles
    cgUInt32 Indices[24];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;
    Indices[12] = 5; Indices[13] = 2; Indices[14] = 1;
    Indices[15] = 5; Indices[16] = 3; Indices[17] = 2;
    Indices[18] = 5; Indices[19] = 4; Indices[20] = 3;
    Indices[21] = 5; Indices[22] = 1; Indices[23] = 4;

    // Determine if the ray intersects any of the triangles formed by the above points
    for ( cgUInt32 i = 0; i < 8; ++i )
    {
        // Compute plane for the current triangle
        cgPlane Plane;
        const cgVector3 & v1 = Points[Indices[(i*3)+0]];
        const cgVector3 & v2 = Points[Indices[(i*3)+1]];
        const cgVector3 & v3 = Points[Indices[(i*3)+2]];
        cgPlane::fromPoints( Plane, v1, v2, v3 );

        // Determine if (and where) the ray intersects the triangle's plane
        cgFloat t;
        if ( !cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, false, false ) )
            continue;

        // Compute the intersection point on the plane
        cgVector3 vIntersect = vOrigin + (vDir * t);

        // Determine if this point is within the triangles edges (within tolerance)
        if ( !cgCollision::pointInTriangle( vIntersect, v1, v2, v3, (cgVector3&)Plane, 2.0f * fZoomFactor ) )
            continue;

        // We intersected! Return the intersection distance.
        fDistance = t;
        return true;

    } // Next Triangle
    
    // No intersection with us.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;
    
    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    cgFloat  fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );
    cgFloat  fSize       = fZoomFactor * 10.0f;
    cgUInt32 nColor      = pIssuer->isSelected() ? 0xFFFFFFFF : 0xFFFFF600;
    
    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    cgShadedVertex Points[30];
    for ( cgInt i = 0; i < 6; ++i )
        Points[i].color = nColor;

    // Compute vertices for the light source representation (central diamond)
    Points[0].position = cgVector3( 0, fSize, 0 );
    Points[1].position = cgVector3( 0, 0, fSize );
    Points[2].position = cgVector3( fSize, 0, 0 );
    Points[3].position = cgVector3( 0, 0, -fSize );
    Points[4].position = cgVector3( -fSize, 0, 0 );
    Points[5].position = cgVector3( 0, -fSize, 0 );
    
    // Compute indices that will allow us to draw the diamond using a tri-list (wireframe)
    // so that we can easily take advantage of back-face culling.
    cgUInt32 Indices[31];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;
    Indices[12] = 5; Indices[13] = 2; Indices[14] = 1;
    Indices[15] = 5; Indices[16] = 3; Indices[17] = 2;
    Indices[18] = 5; Indices[19] = 4; Indices[20] = 3;
    Indices[21] = 5; Indices[22] = 1; Indices[23] = 4;
    
    // Begin rendering
    bool bWireframe = (flags & cgSandboxRenderFlags::Wireframe);
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    pDriver->setWorldTransform( pIssuer->getWorldTransform( false ) );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Draw the central diamond
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 6, 8, Indices, cgBufferFormat::Index32, Points );
        
            // If the light is REALLY selected (i.e. not just one of its parent groups), construct and 
            // render circles that depict the outer range of the light source.
            if ( pIssuer->isSelected() && !pIssuer->isMergedAsGroup() )
            {
                pDriver->setWorldTransform( CG_NULL );

                // Select the correct (clamped) color for rendering the axis.
                // Taken from the diffuse color to supply good visual feedback.
                nColor = mDiffuseColor;

                // A circle for each axis
                cgVector3 vPoint, vAxis, vOrthoAxis1, vOrthoAxis2, vPos = pIssuer->getPosition( false );
                for ( cgInt i = 0; i < 3; ++i )
                {
                    // Retrieve the three axis vectors necessary for constructing the
                    // circle representation for this axis.
                    switch ( i )
                    {
                        case 0:
                            vAxis       = cgVector3( 1, 0, 0 );
                            vOrthoAxis1 = cgVector3( 0, 1, 0 );
                            vOrthoAxis2 = cgVector3( 0, 0, 1 );
                            break;
                        case 1:
                            vAxis       = cgVector3( 0, 1, 0 );
                            vOrthoAxis1 = cgVector3( 0, 0, 1 );
                            vOrthoAxis2 = cgVector3( 1, 0, 0 );
                            break;
                        case 2:
                            vAxis       = cgVector3( 0, 0, 1 );
                            vOrthoAxis1 = cgVector3( 1, 0, 0 );
                            vOrthoAxis2 = cgVector3( 0, 1, 0 );
                            break;
                    
                    } // End Axis Switch

                    // Skip if this axis is more or less orthogonal to the camera (ortho view only)
                    if ( pCamera->getProjectionMode() == cgProjectionMode::Orthographic &&
                         fabsf( cgVector3::dot( vAxis, pCamera->getZAxis( false ) ) ) < 0.05f )
                        continue;
                    
                    // Generate line strip circle
                    for ( cgInt j = 0; j < 30; ++j )
                    {
                        // Build vertex
                        vPoint  = vPos;
                        vPoint += vOrthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);
                        vPoint += vOrthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);
                        Points[j] = cgShadedVertex( vPoint, nColor );

                        // Build index
                        Indices[j] = j;
                        
                    } // Next Segment

                    // Add last index to connect back to the start of the circle
                    Indices[30] = 0;

                    // Render the lines.
                    pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, Indices, cgBufferFormat::Index32, Points );

                    // Same for inner range (if >0 & <outer)
                    if ( mInnerRange > 0 && mInnerRange < mOuterRange )
                    {
                        // Generate line strip circle
                        for ( cgInt j = 0; j < 30; ++j )
                        {
                            // Build vertex
                            vPoint  = vPos;
                            vPoint += vOrthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            vPoint += vOrthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            Points[j].position = vPoint;
                            Points[j].color    = 0xFF99CCE5;
                            
                        } // Next Segment

                        // Render the lines.
                        pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, Indices, cgBufferFormat::Index32, Points );
                    
                    } // End if draw inner range

                } // Next Axis

            } // End if light source selected

        } // End if begun pass

        // We have finished rendering
        pShader->endTechnique();
    
    } // End if begun technique

    // Call base class implementation last.
    cgLightObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightObject::applyObjectRescale( cgFloat fScale )
{
    // Apply the scale to object-space data
    cgFloat fNewOuterRange  = mOuterRange * fScale;
    cgFloat fNewInnerRange  = mInnerRange * fScale;
    
    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, fNewOuterRange );
        mUpdateRanges.bindParameter( 2, fNewInnerRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateRanges.step( true ) )
        {
            cgString strError;
            mUpdateRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local values.
    mInnerRange = fNewInnerRange;
    mOuterRange = fNewOuterRange;

    // Notify listeners that property was altered
    static const cgString strContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Call base class implementation.
    cgLightObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PointLightObject )
        return true;

    // Supported by base?
    return cgLightObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getLightType () (Virtual)
/// <summary>
/// Determine the type of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::LightType cgPointLightObject::getLightType( ) const
{
    return Light_Point;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgLightObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("PointLightObject::insertComponentData") );

        // Write frustum configuration source to the database first of all.
        /*for ( cgInt i = 0; i < 6; ++i )
        {
            if ( !mShadowFrustumIds[i] )
            {
                mShadowFrustumIds[i] = InsertShadowFrustum( m_pShadowConfig[i] );
                if ( !mShadowFrustumIds[i] )
                {
                    mWorld->rollbackTransaction( _T("PointLightObject::insertComponentData") );
                    return false;

                } // End if failed

            } // End if does not exist

        } // Next Frustum*/

        // Insert new point light object
        prepareQueries();
        mInsertPointLight.bindParameter( 1, mReferenceId );
        mInsertPointLight.bindParameter( 2, mOuterRange );
        mInsertPointLight.bindParameter( 3, mInnerRange );
        mInsertPointLight.bindParameter( 4, (cgUInt32)0 ); // ToDo: DistanceAttenuationSplineId
        mInsertPointLight.bindParameter( 5, (cgUInt32)0 ); // ToDo: AttenuationMaskSamplerId
        // ToDo: 6767 - Reintroduce for new shadow settings.
        /*mInsertPointLight.bindParameter( 6, m_bFrustumsLinked );
        mInsertPointLight.bindParameter( 7, mShadowFrustumIds[0] );
        mInsertPointLight.bindParameter( 8, mShadowFrustumIds[1] );
        mInsertPointLight.bindParameter( 9, mShadowFrustumIds[2] );
        mInsertPointLight.bindParameter( 10, mShadowFrustumIds[3] );
        mInsertPointLight.bindParameter( 11, mShadowFrustumIds[4] );
        mInsertPointLight.bindParameter( 12, mShadowFrustumIds[5] );*/
        mInsertPointLight.bindParameter( 13, mShadowUpdateRate );
        
        // ToDo: 9999 - mRangeAdjust
        // ToDo: 9999 - Shadowmap edge process

        // Database ref count (just in case it has already been adjusted)
        mInsertPointLight.bindParameter( 14, mSoftRefCount );
        
        // Execute
        if ( !mInsertPointLight.step( true ) )
        {
            cgString strError;
            mInsertPointLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for point light object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("PointLightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("PointLightObject::insertComponentData") );

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
bool cgPointLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the light data.
    prepareQueries();
    mLoadPointLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadPointLight.step( ) || !mLoadPointLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadPointLight.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for point light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for point light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadPointLight.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadPointLight;

    // Update our local members
    mLoadPointLight.getColumn( _T("OuterRange"), mOuterRange );
    mLoadPointLight.getColumn( _T("InnerRange"), mInnerRange );
    // ToDo: 9999 - DistanceAttenuationSplineId
    // ToDo: 9999 - AttenuationMaskSamplerId
    // ToDo: 6767 - Reintroduce for new shadow settings.
    // ToDo: mLoadPointLight.getColumn( _T("ShadowFrustumsLinked"), m_bFrustumsLinked );
    mLoadPointLight.getColumn( _T("ShadowUpdateRate"), mShadowUpdateRate );
    /*ToDo: mLoadPointLight.getColumn( _T("ShadowFrustumPosX"), mShadowFrustumIds[0] );
    mLoadPointLight.getColumn( _T("ShadowFrustumNegX"), mShadowFrustumIds[1] );
    mLoadPointLight.getColumn( _T("ShadowFrustumPosY"), mShadowFrustumIds[2] );
    mLoadPointLight.getColumn( _T("ShadowFrustumNegY"), mShadowFrustumIds[3] );
    mLoadPointLight.getColumn( _T("ShadowFrustumPosZ"), mShadowFrustumIds[4] );
    mLoadPointLight.getColumn( _T("ShadowFrustumNegZ"), mShadowFrustumIds[5] );*/

    // Load frustum configuration source from the database.
    /*for ( cgInt i = 0; i < 6; ++i )
    {
        if ( mShadowFrustumIds[i] )
        {
            if ( !LoadShadowFrustum( mShadowFrustumIds[i], m_pShadowConfig[i] ) )
            {
                // Release any pending read operation.
                mLoadPointLight.reset();
                return false;
            
            } // End if failed
        
        } // End if has frustum

    } // Next Frustum*/

    // Call base class implementation to read remaining data.
    if ( !cgLightObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        // ToDo: 6767 - Reintroduce for new shadow settings.
        // Reset shadow frustum identifiers to ensure they are re-serialized as necessary.
        //memset( mShadowFrustumIds, 0, sizeof(mShadowFrustumIds) );

        // Insert
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
void cgPointLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertPointLight.isPrepared() )
            mInsertPointLight.prepare( mWorld, _T("INSERT INTO 'Objects::PointLight' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14)"), true );
        if ( !mUpdateRanges.isPrepared() )
            mUpdateRanges.prepare( mWorld, _T("UPDATE 'Objects::PointLight' SET OuterRange=?1, InnerRange=?2 WHERE RefId=?3"), true );
        if ( !mUpdateShadowConfigSources.isPrepared() )
            mUpdateShadowConfigSources.prepare( mWorld, _T("UPDATE 'Objects::PointLight' SET ShadowFrustumsLinked=?1 WHERE RefId=?2"), true );
        if ( !mUpdateShadowRate.isPrepared() )
            mUpdateShadowRate.prepare( mWorld, _T("UPDATE 'Objects::PointLight' SET ShadowUpdateRate=?1 WHERE RefId=?2"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadPointLight.isPrepared() )
        mLoadPointLight.prepare( mWorld, _T("SELECT * FROM 'Objects::PointLight' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgPointLightNode Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPointLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightNode::cgPointLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgLightNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults
    mShadowTimeSinceLast = 0.0f;
    memset( mFrustums, 0, 6 * sizeof(cgShadowGenerator*) );
    memset( mIndirectFrustums, 0, 6 * sizeof(cgReflectanceGenerator*) );

    // Allocate frustums
    for ( cgInt i = 0; i < 6; ++i )
    {
        mFrustums[i] = new cgShadowGenerator( this, i );
        mIndirectFrustums[i] = new cgReflectanceGenerator( this, i );
    
    } // Next Frustum
}

//-----------------------------------------------------------------------------
//  Name : cgPointLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightNode::cgPointLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    mShadowTimeSinceLast = 0.0f;
    memset( mFrustums, 0, 6 * sizeof(cgShadowGenerator*) );
    memset( mIndirectFrustums, 0, 6 * sizeof(cgReflectanceGenerator*) );

    // Allocate frustums
    for ( cgInt i = 0; i < 6; ++i )
    {
        mFrustums[i] = new cgShadowGenerator( this, i );
        mIndirectFrustums[i] = new cgReflectanceGenerator( this, i );
    
    } // Next Frustum
}

//-----------------------------------------------------------------------------
//  Name : ~cgPointLightNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPointLightNode::~cgPointLightNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    for ( cgInt i = 0; i < 6; ++i )
    {
        if ( mFrustums[i] )
            mFrustums[i]->scriptSafeDispose();
        if ( mIndirectFrustums[i] )
            mIndirectFrustums[i]->scriptSafeDispose();
    
    } // Next Frustum
    
    // Clear Variables
    memset( mFrustums, 0, 6 * sizeof(cgShadowGenerator*) );
    memset( mIndirectFrustums, 0, 6 * sizeof(cgReflectanceGenerator*) );
    
    // Release resources
    mFrustumMesh.close();
    
    // Dispose base.
    if ( bDisposeBase )
        cgLightNode::dispose( true );
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
bool cgPointLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PointLightNode )
        return true;

    // Supported by base?
    return cgLightNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgPointLightNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgPointLightNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgPointLightNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgPointLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

// ToDo: Consider adding a bool to the volume testing functions that tells 
//       us whether to do the test against the original volume or the error adjusted volume?
//       This way we can support both pure tests and shape rendering tests. 

//-----------------------------------------------------------------------------
//  Name : boundsInVolume () (Virtual)
/// <summary>
/// Does the specified bounding box fall within the volume of this light 
/// source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::boundsInVolume( const cgBoundingBox & AABB )
{
    return AABB.intersect( getBoundingBox() );
}

//-----------------------------------------------------------------------------
//  Name : pointInVolume () (Virtual)
/// <summary>
/// Does the specified point fall within the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::pointInVolume( const cgVector3 & Point, cgFloat fErrRadius /* = 0.0f */ )
{
    return ( cgVector3::length( getPosition( false ) - Point) < (getOuterRange() + fErrRadius) );
}

//-----------------------------------------------------------------------------
//  Name : frustumInVolume () (Virtual)
/// <summary>
/// Does the specified frustum intersect the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::frustumInVolume( const cgFrustum & Frustum )
{
    return Frustum.testSphere( getPosition( false ), getOuterRange() );
}

//-----------------------------------------------------------------------------
//  Name : testObjectShadowVolume( ) (Virtual)
/// <summary>
/// Determine if the specified object can potentially cast a shadow into
/// the view frustum based on the requirements of this light source.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::testObjectShadowVolume( cgObjectNode * pObject, const cgFrustum & ViewFrustum )
{
    cgBoundingBox ObjectAABB = pObject->getBoundingBox();

    // If the light source falls inside the bounding box of the object, 
    // this can automatically cast shadows.
    if ( ObjectAABB.containsPoint( getPosition( false ) ) )
        return true;

    // Compute an extruded bounding box for the object. This extrudes the box edges
    // away from the light source position (just like extruding an actual mesh shadow
    // volume when implementing stencil shadows).
    cgExtrudedBoundingBox ExtrudedAABB( ObjectAABB, getPosition( false ), getOuterRange() * 1.73205f /* sqrt((1*1)+(1*1)+(1*1)) */ );

    // Test to see if the box intersects the frustum.
    if ( ViewFrustum.testExtrudedAABB( ExtrudedAABB ) )
        return true;

    // Cannot cast a shadow into the view frustum
    return false;
}

//-----------------------------------------------------------------------------
//  Name : computeShadowSets( ) (Virtual)
/// <summary>
/// Creates the shadow mapping visibility sets that are also found within
/// the specified visibility set (i.e. main player camera). This 
/// essentially narrows down the list of visible objects from the
/// point of view of the light source to only those that can also be
/// seen by the camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::computeShadowSets( cgCameraNode * pCamera )
{
    // ToDo: 6767 - Don't do this if the camera is the same as the one used
    // previously on this frame (reset when the light's main computeVisibility is called)
    // on multiple passes.

    // ToDo: 6767 - Need to be able to re-activate mComputeShadows if
    // the camera changes (it may end up with something in the visibility set!)

    // Allow base class to process first.
    cgLightNode::computeShadowSets( pCamera );

    // Generate shadow sets if necessary.
    if ( isShadowSource() )
    {
        bool bCalcShadows = false;

        // Process each shadow frustum.
        for ( cgInt i = 0; i < 6; ++i )
        {
            // First allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            cgShadowGenerator * pFrustum = mFrustums[i];
            if ( pFrustum )
                bCalcShadows |= pFrustum->computeVisibilitySet( pCamera );

        } // Next frustum

        // Is there /still/ any need to assume that this is a shadow source?
        if ( !bCalcShadows )
            mComputeShadows = false;

    } // End if shadow caster
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail () (Virtual)
/// <summary>
/// Computes level of detail settings for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
{
    // Allow base class to compute default LOD settings.
    cgLightNode::computeLevelOfDetail( pCamera );

    // If shadows are enabled
    if ( mComputeShadows )
    {
        cgTexturePoolResourceDesc::Array aDescriptions;

        // Get the lighting manager
        cgLightingManager * pManager = getScene()->getLightingManager();

        // Ask the lighting manager for the best shadow technique given LOD
        cgInt32 nIndex = pManager->getShadowSettings( mShadowDetailLevels, false, mShadowSystemSettings, aDescriptions, mShadowFlags ); 
        if ( nIndex >= 0 )
        {
            // Get the light level settings based on the index selected by the lighting manager
            cgShadowSettingsLight & LightSettings = mShadowSettings[ nIndex ];

            // Update resource resolution(s) based on current LOD settings
            cgUInt32 nMaxResolution = pManager->getMaxShadowResolution( false );
            cgUInt32 nResolution = 1 << (nMaxResolution - (LightSettings.resolutionAdjust + mShadowSystemSettings.resolutionAdjust));
            for ( size_t i = 0; i < aDescriptions.size(); i++ )
            {
                aDescriptions[ i ].bufferDesc.width  = nResolution;
                aDescriptions[ i ].bufferDesc.height = nResolution;
            
            } // Next resource

            // Apply settings to the child shadow frustums.
            for ( cgInt i = 0; i < 6; ++i )
            {
                cgShadowGenerator * pFrustum = mFrustums[i];

                // Apply settings to the child shadow frustums.
                pFrustum->update( nResolution, mShadowSystemSettings, LightSettings, aDescriptions, mShadowFlags );

                // If we're blending shadow map edges, adjust the frustum's camera
                // to take into account a larger portion of the scene (the equivalent
                // of rendering an extra single texel border all the way around the 
                // shadow map).
                if ( getShadowMapEdgeProcess() != cgPointLightObject::NoProcessing )
                {
                    cgCameraNode * pFrustumCamera = pFrustum->getCamera();
                    pFrustumCamera->setFOV( CGEToDegree(atanf(1.0f / (1.0f - (1.0f / (pFrustum->getResolution() / 2)))) * 2.0f ) );

                } // End if blending edges

            } // Next frustum

        } // End valid settings
        else
        {
            // We could not get valid shadow settings, so turn off shadows
            mComputeShadows = false;
        
        } // End if invalid

    } // End shadows
}

//-----------------------------------------------------------------------------
//  Name : updateIndirectSettings () (Virtual)
/// <summary>
/// Updates settings for indirect lighting
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::updateIndirectSettings( )
{
    cgTexturePoolResourceDesc::Array aDescriptions;

    // Get the lighting manager
    cgLightingManager * pManager = getScene()->getLightingManager();

    // Ask the lighting manager for the best reflective shadow technique given LOD
    cgInt32 nIndex = pManager->getShadowSettings( mIndirectDetailLevels, true, mIndirectSystemSettings, aDescriptions, mIndirectFlags ); 
    if ( nIndex >= 0 )
    {
        // Get the light level settings based on the index selected by the lighting manager
        cgShadowSettingsLight & LightSettings = mIndirectSettings[ nIndex ];

        // Update resource resolution(s) based on current LOD settings
        cgUInt32 nMaxResolution = pManager->getMaxShadowResolution( false );
        cgUInt32 nResolution = 1 << (nMaxResolution - (LightSettings.resolutionAdjust + mShadowSystemSettings.resolutionAdjust));
        for ( size_t i = 0; i < aDescriptions.size(); i++ )
        {
            aDescriptions[ i ].bufferDesc.width  = nResolution;
            aDescriptions[ i ].bufferDesc.height = nResolution;
        
        } // Next resource

        // If we are downsampling the RSM, update the resolutions of the recipient targets based on the tap count
        if ( mIndirectSystemSettings.boxFilter && aDescriptions.size() > 4 )
        {
            cgAssert( mIndirectSystemSettings.primarySamples > 0 );
            for ( size_t i = 4; i < aDescriptions.size(); i++ )
            {
                aDescriptions[ i ].bufferDesc.width  = mIndirectSystemSettings.primarySamples;
                aDescriptions[ i ].bufferDesc.height = mIndirectSystemSettings.primarySamples;
            
            } // Next resource
        
        } // End if downsampling

        // Apply settings.
        for ( cgInt i = 0; i < 6; ++i )
        {
            if ( mIndirectFrustums[i] )
                mIndirectFrustums[i]->update( nResolution, mIndirectSystemSettings, LightSettings, aDescriptions, mIndirectFlags );

        } // Next frustum

    } // End valid settings 
    else
    {
        // Turn off indirect lighting
        mComputeIndirect = false;
    
    } // End if invalid settings
}

//-----------------------------------------------------------------------------
//  Name : setClipPlanes () (Virtual)
/// <summary>
/// Setup the clipping planes for this light source
/// ToDo : For some reason, this function is broken...
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::setClipPlanes( )
{
    cgPlane Planes[ 6 ];

	// Get access to required systems / objects
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();

	// Extract the clip planes from our bounding box
	cgBoundingBox bbox = getBoundingBox( );
	for ( cgInt i = 0; i < 6; i++ )
        Planes[ i ] = bbox.getPlane( (cgVolumePlane::Side)i );

	// Set the user-defined clip planes for the light
	pDriver->setUserClipPlanes( Planes, 6 );

    // Update state
	mSetClipPlanes = true;
}

/*//-----------------------------------------------------------------------------
//  Name : computeShadowSets( ) (Virtual)
/// <summary>
/// Creates the shadow mapping visibility sets that are also found within
/// the specified visibility set (i.e. main player camera). This 
/// essentially narrows down the list of visible objects from the
/// point of view of the light source to only those that can also be
/// seen by the camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::computeShadowSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::computeShadowSets( pCamera );

    // If it was determined that we should calculate shadows,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeShadows )
    {
        bool bCalcShadows = false;

        // Process each shadow frustum.
        for ( cgInt i = 0; i < 6; ++i )
        {
            cgShadowGenerator * pFrustum = mFrustums[i];
            if ( !pFrustum || !pFrustum->IsEnabled() )
                continue;

            // First allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            cgLightObject * pLight = (cgLightObject*)m_pReferencedObject;
            bCalcShadows |= pFrustum->computeShadowSets( pCamera );

        } // Next frustum

        // Is there /still/ any need to assume that this is a shadow source?
        if ( !bCalcShadows )
            mComputeShadows = false;

    } // End if mComputeShadows
}*/


/*//-----------------------------------------------------------------------------
//  Name : ComputeIndirectSets( ) (Virtual)
/// <summary>
/// Creates the rsm visibility sets
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::ComputeIndirectSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::ComputeIndirectSets( pCamera );

    // If it was determined that we should calculate rsms,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeIndirect )
    {
        bool bCalcIndirect = false;

        // Process each active frustum
        for ( cgInt i = 0; i < 6; ++i )
        {
            // First allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            bCalcIndirect |= mIndirectFrustums[i]->computeVisibilitySet( pCamera );

        } // Next frustum

        // Is there /still/ any need to assume that this is a reflective shadow source?
        if ( !bCalcIndirect )
            mComputeIndirect = false;

    } // End if mComputeIndirect
}*/

//-----------------------------------------------------------------------------
// Name : reassignShadowMaps() (Virtual)
/// <summary>
/// Allows the light to determine if it is necessary to partake in
/// the assignment of new shadow maps at this stage, or if it can re-use
/// shadow maps that it may have populated earlier.
/// Note : Returning true indicates that all resources were re-assigned and 
/// the light does not require further shadow map assignment processing 
/// (i.e. it reused pooled maps).
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::reassignShadowMaps( cgTexturePool * pPool )
{
    bool bNoFurtherAssignment = true;

    // Is there any need to re-assign?
    if ( !isShadowSource() )
    {
        // Release resources back to the pool.
        for ( cgInt i = 0; i < 6; ++i )
            mFrustums[i]->releaseResources();

        return false;
    
    } // End if not a caster

    // Attempt resource reassignment for each frustum
    for ( cgInt i = 0; i < 6; ++i )
    {
        // If the frustum has nothing to cast, release its resources
        // and skip further processing.
        cgShadowGenerator * pFrustum = mFrustums[i];
        if ( !pFrustum->containsRenderableObjects() )
        {
            pFrustum->releaseResources();
            continue;
        
        } // End if nothing to do
        
        // Attempt resource reassignment (if we cannot re-use the resources, process further)
        if ( !pFrustum->reassignResources( pPool ) )
            bNoFurtherAssignment = false;
        
    } // Next frustum

    // We need to assign further maps?
    return bNoFurtherAssignment;
}

//-----------------------------------------------------------------------------
//  Name : beginShadowFill() (Virtual)
/// <summary>
/// Assuming the light would like to opt in to the population of any
/// assigned shadow maps, this method is called to start that process
/// and should return the total number of fill passes that will be
/// required (i.e. perhaps because the light maintains multiple shadow
/// maps).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPointLightNode::beginShadowFill( cgTexturePool * pPool )
{
    // Call base class implementation (setup much of the pass tracking logic)
    if ( cgLightNode::beginShadowFill( pPool ) < 0 )
        return -1;

    // Set shadow update status variables.
    memset( mShadowFrustumUpdated, 0, 6 * sizeof(bool) );
    mShadowMapsUpdated = false;
    
    // Determine if it is an appropriate time for us to update based on
    // our shadow map update period limiting property.
    // ToDo: 6767 -- Should increment mShadowTimeSinceLast outside of this
    // method since it may be called multiple times in a frame.
    bool timeToUpdate = true;
    cgUInt32 shadowUpdateRate = getShadowUpdateRate();
    if ( shadowUpdateRate > 0 )
    {
        mShadowTimeSinceLast += cgTimer::getInstance()->getTimeElapsed();
        if ( mShadowTimeSinceLast < (1.0f / (cgFloat)shadowUpdateRate) )
            timeToUpdate = false;
        else
            mShadowTimeSinceLast = 0.0f;

    } // End if limit updates

    // For each frustum...
    mShadowPasses.reserve( 6 ); // Assume 1 pass per frustum as a minimum
    for ( cgInt i = 0; i < 6; i++ )
    {
        // ToDo: 6767 - Do nothing if the frustum has nothing to draw
        // (See Generator::containsRenderableObjects()?)
        cgShadowGenerator * pShadowFrustum = mFrustums[ i ];
        /*if ( pShadowFrustum->getVisibilitySet()->isEmpty() )
        {
            pShadowFrustum->releaseResources();
            continue;
        }*/

        // Attempt to find resources in the pool.
        // ToDo: 6767 - If later tests / frustums cause this method to fail,
        // should the resources assigned to other frustums be released?
        cgInt32 nFillStatus = pShadowFrustum->assignResources( pPool );

        // If there was a failure getting resources, bail
        // ToDo: 6767 - Is this really a "result", or a status / request?
        if ( nFillStatus & cgShadowGeneratorFillResult::DoNothing )
        {
            cgLightNode::endShadowFill();
            return -1;
        
        } // End if failed

        // We cannot fill now if we were assigned one or more default resource types.
        if( nFillStatus & cgShadowGeneratorFillResult::CannotFill )
            continue;

        // We can fill if we got previously assigned resources, but don't necessarily 
        // have to if the frustum doesn't require it.
        if( (nFillStatus & cgShadowGeneratorFillResult::CanFill) && !pShadowFrustum->shouldRegenerate() )
            continue;

        // Even if the frustum thinks we should regenerate, if it's not our time
        // then we can skip for this frustum.
        if ( (nFillStatus & cgShadowGeneratorFillResult::CanFill) && !timeToUpdate )
            continue;

        // How many passes are required to fill the assigned shadow resources?
        cgUInt32 nPassCount = pShadowFrustum->getWritePassCount();

        // Add new passes to list
        for ( cgUInt32 j = 0; j < nPassCount; j++ )
            mShadowPasses.push_back( LightingOpPass( Lighting_FillShadowMap, i, j ) );

    } // Next frustum

    // If there was literally nothing to do, return -1 (don't even process).
    // ToDo: 6767 - Check that the caller even cares if this is -1.
    if ( mShadowPasses.empty() )
    {
        cgLightNode::endShadowFill();
        return -1;
    
    } // End if nothing to do

    // Record pass count and return it.
    mShadowPassCount = (cgInt32)mShadowPasses.size();
    return mShadowPassCount;
}

//-----------------------------------------------------------------------------
//  Name : beginShadowFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::beginShadowFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation?
    if ( !cgLightNode::beginShadowFillPass( nPass, pRenderSetOut ) )
        return false;

    // Get the generator for the current shadow fill pass.
    const LightingOpPass & Pass = mShadowPasses[ nPass ];
    cgShadowGenerator * pFrustum = mFrustums[ Pass.frustum ];

    // If this is the first child pass for this frustum, begin writing
    if ( Pass.subPass == 0 )
        pFrustum->beginWrite( );

    // If we have not exited by now, a fill is either required or desired
    if ( pFrustum->beginWritePass( Pass.subPass ) )
    {
        // Render only objects that exist within frustum's shadow set.
        pRenderSetOut = pFrustum->getVisibilitySet();

        // We have begun this pass
        mCurrentShadowPass = nPass;
        mShadowFrustumUpdated[ Pass.frustum ] = true;
        return true;

    } // End if filling

    // Did not fill shadow map.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFillPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endShadowFillPass( )
{
    cgInt32 nCurrentPass = mCurrentShadowPass;

    // Valid end operation?
    if ( !cgLightNode::endShadowFillPass() )
        return false;

    // Finish filling shadow map for this pass.
    const LightingOpPass & Pass = mShadowPasses[ nCurrentPass ];
    cgShadowGenerator * pFrustum = mFrustums[ Pass.frustum ];
    if ( !pFrustum->endWritePass() )
        return false;

    // If this is the last child pass, end writing
    if ( Pass.subPass == (pFrustum->getWritePassCount() - 1) )
        pFrustum->endWrite();

    // Did we update /any/ of our frustums?
    mShadowMapsUpdated |= mShadowFrustumUpdated[ Pass.frustum ];

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFill() (Virtual)
/// <summary>
/// Called in order to signify that the shadow fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endShadowFill( )
{
    // Valid end operation?
    if ( !cgLightNode::endShadowFill( ) )
        return false;

    // Under the conditions of a full repair, we will need to copy portions of
    // neighboring shadow maps into the border texels of each of our six shadow
    // maps. We perform this operation here if requested.
    if ( getShadowMapEdgeProcess() == cgPointLightObject::FullRepair && mShadowMapsUpdated )
        repairShadowBorders( mShadowFrustumUpdated );

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : repairShadowBorders () (Protected)
/// <summary>
/// Repair the shadow map borders in order to completely eliminate the
/// possibility of bilinear filtering seams.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::repairShadowBorders( bool pFrustumInvalidated[] )
{
    cgToDoAssert( "Effect Overhaul", "Implement repairShadowBorders()" );
    /*
    cgUInt32     i, j;
    cgClipVertex Points[96];
    cgUInt16     Indices[144];
    cgByte       Quads[6] = {0};
    int          SourceFrustums[24], pNeighbors[4];
    cgUInt32     nNeighbor, nNeighborIndex, nBaseVertex;
    cgFloat      fBorderTexelSize, fBorderSampleOffset;

    // Clear all source frustum indices
    for ( i = 0; i < 24; ++i )
        SourceFrustums[i] = -1;

    // We need to repair any of the 6 shadow maps that may have been re-filled. 
    // We do this by effectively "pushing" color into any invalidated neighbor 
    // (rather than the more complex "pulling" operation) because it means we do
    // not need to know the orientation of the neighboring face.
    for ( i = 0; i < 6; ++i )
    {
        cgShadowGenerator * pFrustum = mFrustums[i], * pNeighbor;

        // Compute matrix that describes how to transform the quads we build
        // into "world" space (or more accurately "cube" space).
        cgMatrix m = pFrustum->getCamera()->getWorldTransform( false );
        (cgVector3&)m._41 = cgVector3(0,0,0);

        // Compute amount we should offset into the texture (in 0-1 range)
        // in order to duplicate the interior shadow map information to
        // any neighboring shadow map.
        fBorderSampleOffset = (1.5f / (cgFloat)pFrustum->getResolution());

        // Compute this face's neighbors so we know who
        // we are pushing shadow map data to.
        getShadowFaceNeighbors( i, pNeighbors );

        // Step through each neighbor who may need our information and construct a 
        // border quad in /their/ space (if we are able to blend with them).
        // First we begin with the top edge of the current shadow map face.
        nNeighborIndex = 0;
        nNeighbor      = pNeighbors[nNeighborIndex];
        pNeighbor      = mFrustums[nNeighbor];
        if ( pFrustumInvalidated[nNeighbor] && pNeighbor->CanBlend( pFrustum ) )
        {
            // We can and should blend into our neighbor, construct the quad in
            // our own local texture space first of all.
            SourceFrustums[((nNeighbor*4)+Quads[nNeighbor])] = i;
            nBaseVertex = ((nNeighbor*4)+Quads[nNeighbor]) * 4;
            Quads[nNeighbor]++;

            fBorderTexelSize = 1.0f / (cgFloat)pNeighbor->getResolution();
            Points[ nBaseVertex + 0 ] = cgClipVertex( -1.0f, 1.0f, 1.0f - (fBorderTexelSize * 2.0f), 0.0f, fBorderSampleOffset );
            Points[ nBaseVertex + 1 ] = cgClipVertex(  1.0f, 1.0f, 1.0f - (fBorderTexelSize * 2.0f), 1.0f, fBorderSampleOffset );
            Points[ nBaseVertex + 2 ] = cgClipVertex(  1.0f, 1.0f, 1.0f, 1.0f, fBorderSampleOffset );
            Points[ nBaseVertex + 3 ] = cgClipVertex( -1.0f, 1.0f, 1.0f, 0.0f, fBorderSampleOffset );
            
            // Transform all computed points into "world" space (or more accurately "cube" space)
            for ( j = nBaseVertex; j < nBaseVertex + 4; ++j )
                CGEVec3TransformCoord( &Points[j].position, &Points[j].position, &m );
            
        } // End if can blend

        // Right edge
        nNeighborIndex = 1;
        nNeighbor      = pNeighbors[nNeighborIndex];
        pNeighbor      = mFrustums[nNeighbor];
        if ( pFrustumInvalidated[nNeighbor] && pNeighbor->CanBlend( pFrustum ) )
        {
            // We can and should blend into our neighbor, construct the quad in
            // our own local texture space first of all.
            SourceFrustums[((nNeighbor*4)+Quads[nNeighbor])] = i;
            nBaseVertex = ((nNeighbor*4)+Quads[nNeighbor]) * 4;
            Quads[nNeighbor]++;

            fBorderTexelSize = 1.0f / (cgFloat)pNeighbor->getResolution();
            Points[ nBaseVertex + 0 ] = cgClipVertex( 1.0f,  1.0f, 1.0f, 1.0f - fBorderSampleOffset, 0.0f );
            Points[ nBaseVertex + 1 ] = cgClipVertex( 1.0f,  1.0f, 1.0f - (fBorderTexelSize * 2.0f), 1.0f - fBorderSampleOffset, 0.0f );
            Points[ nBaseVertex + 2 ] = cgClipVertex( 1.0f, -1.0f, 1.0f - (fBorderTexelSize * 2.0f), 1.0f - fBorderSampleOffset, 1.0f );
            Points[ nBaseVertex + 3 ] = cgClipVertex( 1.0f, -1.0f, 1.0f, 1.0f - fBorderSampleOffset, 1.0f );
            
            // Transform all computed points into "world" space (or more accurately "cube" space)
            for ( j = nBaseVertex; j < nBaseVertex + 4; ++j )
                CGEVec3TransformCoord( &Points[j].position, &Points[j].position, &m );
            
        } // End if can blend
        
        // Bottom edge
        nNeighborIndex = 2;
        nNeighbor      = pNeighbors[nNeighborIndex];
        pNeighbor      = mFrustums[nNeighbor];
        if ( pFrustumInvalidated[nNeighbor] && pNeighbor->CanBlend( pFrustum ) )
        {
            // We can and should blend into our neighbor, construct the quad in
            // our own local texture space first of all.
            SourceFrustums[((nNeighbor*4)+Quads[nNeighbor])] = i;
            nBaseVertex = ((nNeighbor*4)+Quads[nNeighbor]) * 4;
            Quads[nNeighbor]++;

            fBorderTexelSize = 1.0f / (cgFloat)pNeighbor->getResolution();
            Points[ nBaseVertex + 0 ] = cgClipVertex( -1.0f, -1.0f, 1.0f, 0.0f, 1.0f - fBorderSampleOffset );
            Points[ nBaseVertex + 1 ] = cgClipVertex(  1.0f, -1.0f, 1.0f, 1.0f, 1.0f - fBorderSampleOffset );
            Points[ nBaseVertex + 2 ] = cgClipVertex(  1.0f, -1.0f, 1.0f - (fBorderTexelSize * 2.0f), 1.0f, 1.0f - fBorderSampleOffset );
            Points[ nBaseVertex + 3 ] = cgClipVertex( -1.0f, -1.0f, 1.0f - (fBorderTexelSize * 2.0f), 0.0f, 1.0f - fBorderSampleOffset );
            
            // Transform all computed points into "world" space (or more accurately "cube" space)
            for ( j = nBaseVertex; j < nBaseVertex + 4; ++j )
                CGEVec3TransformCoord( &Points[j].position, &Points[j].position, &m );
            
        } // End if can blend

        // Left edge
        nNeighborIndex = 3;
        nNeighbor      = pNeighbors[nNeighborIndex];
        pNeighbor      = mFrustums[nNeighbor];
        if ( pFrustumInvalidated[nNeighbor] && pNeighbor->CanBlend( pFrustum ) )
        {
            // We can and should blend into our neighbor, construct the quad in
            // our own local texture space first of all.
            SourceFrustums[((nNeighbor*4)+Quads[nNeighbor])] = i;
            nBaseVertex = ((nNeighbor*4)+Quads[nNeighbor]) * 4;
            Quads[nNeighbor]++;

            fBorderTexelSize = 1.0f / (cgFloat)pNeighbor->getResolution();
            Points[ nBaseVertex + 0 ] = cgClipVertex( -1.0f,  1.0f, 1.0f - (fBorderTexelSize * 2.0f), fBorderSampleOffset, 0.0f );
            Points[ nBaseVertex + 1 ] = cgClipVertex( -1.0f,  1.0f, 1.0f, fBorderSampleOffset, 0.0f );
            Points[ nBaseVertex + 2 ] = cgClipVertex( -1.0f, -1.0f, 1.0f, fBorderSampleOffset, 1.0f );
            Points[ nBaseVertex + 3 ] = cgClipVertex( -1.0f, -1.0f, 1.0f - (fBorderTexelSize * 2.0f), fBorderSampleOffset, 1.0f );
            
            // Transform all computed points into "world" space (or more accurately "cube" space)
            for ( j = nBaseVertex; j < nBaseVertex + 4; ++j )
                CGEVec3TransformCoord( &Points[j].position, &Points[j].position, &m );
            
        } // End if can blend

    } // Next Shadow Face

    // Construct indices for rendering the quads
    for ( i = 0; i < 24; ++i )
    {
        // Vertices were constructed for this edge?
        if ( SourceFrustums[i] >= 0 )
        {
            Indices[(i*6)+0] = (cgUInt16)(i*4);
            Indices[(i*6)+1] = (cgUInt16)(i*4) + 1;
            Indices[(i*6)+2] = (cgUInt16)(i*4) + 3;

            Indices[(i*6)+3] = (cgUInt16)(i*4) + 1;
            Indices[(i*6)+4] = (cgUInt16)(i*4) + 2;
            Indices[(i*6)+5] = (cgUInt16)(i*4) + 3;
        
        } // End if needs quad

    } // Next Quad

    // Select the border repair technique prior to rendering.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    cgEffectFile   * pEffect = m_hEffect;
    pEffect->SetTechnique( _T("OmniShadowBorderRepair") );
    pDriver->SetEffect( m_hEffect );

    // Begin the effect
    pDriver->BeginEffect();

    // Select the correct vertex format for rendering our clip space quad.
    static cgVertexFormat * ClipSpaceFormat = cgVertexFormat::formatFromDeclarator( cgClipVertex::Declarator );
    pDriver->setVertexFormat( ClipSpaceFormat );

    // Render quads for each invalidated shadow map
    for ( i = 0; i < 6; ++i )
    {
        // Skip any which did not require updating.
        if ( !pFrustumInvalidated[i] )
            continue;

        // We're copying data from appropriate neighbors into /this/ face
        pDriver->BeginTargetRender( mFrustums[i]->GetShadowMap()->GetTarget() );

        // Select the correct matrix for this face in order to generate an accurate clip space quad.
        pEffect->SetMatrix( _T("LightViewMatrix"), mFrustums[i]->getCamera()->GetViewMatrix() );
        
        // Initialize the correct sampler states required for the copy.
        // These states are specified in the first pass (saves some time
        // setting these up for each quad that we render).
        if ( pDriver->BeginEffectPass(0) )
        {
            pDriver->EndEffectPass();

            // Draw each of the quads "pushed" into this face.
            for ( j = 0; j < Quads[i]; ++j )
            {
                // Select the shadow map to render from.
                cgShadowGenerator * pNeighbor  = mFrustums[SourceFrustums[(i*4) + j]];
                cgShadowMap     * pSourceMap = pNeighbor->GetShadowMap();
                cgRenderTarget  * pTarget    = pSourceMap->GetTarget();
                pEffect->SetTexture( _T("Texture0Texture"), pTarget->GetTarget()  );

                // Draw the quad.
                if ( pDriver->BeginEffectPass( 1 ) )
                {
                    pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, ((i*4)+j)*4, 4, 2, 
                                                     &Indices[((i*4)*6) + (j*6)], cgBufferFormat::Index16, Points );
                    pDriver->EndEffectPass();

                } // End if success

            } // Next Quad

        } // End if success

        // We've finished updating the shadow map.
        pDriver->EndTargetRender( );

    } // Next Shadow Face

    // Finish up
    pDriver->EndEffect();*/
}

//-----------------------------------------------------------------------------
//  Name : getShadowFaceNeighbors () (Protected)
/// <summary>
/// Compute the list of neighbors that connect to a given shadow cube
/// face in Top, Left, Bottom, Right order.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::getShadowFaceNeighbors( cgInt32 nFace, cgInt32 pNeighbors[] )
{
    // Which source face?
    switch ( nFace )
    {
        case cgPointLightObject::ShadowFace_PositiveX:
            pNeighbors[0] = cgPointLightObject::ShadowFace_PositiveY;
            pNeighbors[1] = cgPointLightObject::ShadowFace_NegativeZ;
            pNeighbors[2] = cgPointLightObject::ShadowFace_NegativeY;
            pNeighbors[3] = cgPointLightObject::ShadowFace_PositiveZ;
            break;

        case cgPointLightObject::ShadowFace_NegativeX:
            pNeighbors[0] = cgPointLightObject::ShadowFace_PositiveY;
            pNeighbors[1] = cgPointLightObject::ShadowFace_PositiveZ;
            pNeighbors[2] = cgPointLightObject::ShadowFace_NegativeY;
            pNeighbors[3] = cgPointLightObject::ShadowFace_NegativeZ;
            break;

        case cgPointLightObject::ShadowFace_PositiveY:
            pNeighbors[0] = cgPointLightObject::ShadowFace_NegativeZ;
            pNeighbors[1] = cgPointLightObject::ShadowFace_PositiveX;
            pNeighbors[2] = cgPointLightObject::ShadowFace_PositiveZ;
            pNeighbors[3] = cgPointLightObject::ShadowFace_NegativeX;
            break;

        case cgPointLightObject::ShadowFace_NegativeY:
            pNeighbors[0] = cgPointLightObject::ShadowFace_PositiveZ;
            pNeighbors[1] = cgPointLightObject::ShadowFace_PositiveX;
            pNeighbors[2] = cgPointLightObject::ShadowFace_NegativeZ;
            pNeighbors[3] = cgPointLightObject::ShadowFace_NegativeX;
            break;

        case cgPointLightObject::ShadowFace_PositiveZ:
            pNeighbors[0] = cgPointLightObject::ShadowFace_PositiveY;
            pNeighbors[1] = cgPointLightObject::ShadowFace_PositiveX;
            pNeighbors[2] = cgPointLightObject::ShadowFace_NegativeY;
            pNeighbors[3] = cgPointLightObject::ShadowFace_NegativeX;
            break;

        case cgPointLightObject::ShadowFace_NegativeZ:
            pNeighbors[0] = cgPointLightObject::ShadowFace_PositiveY;
            pNeighbors[1] = cgPointLightObject::ShadowFace_NegativeX;
            pNeighbors[2] = cgPointLightObject::ShadowFace_NegativeY;
            pNeighbors[3] = cgPointLightObject::ShadowFace_PositiveX;
            break;

    } // End switch
}

//-----------------------------------------------------------------------------
//  Name : updateLightConstants() (Virtual)
/// <summary>
/// Update the lighting base terms constant buffer as necessary whenever any
/// of the appropriate properties change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::updateLightConstants()
{
    // Update 'lighting terms' constant buffer.
    cgConstantBuffer * pLightBuffer = mLightConstants.getResource( true );
    cgAssert( pLightBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbLight * pLightData = CG_NULL;
    if ( !(pLightData = (_cbLight*)pLightBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock light data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed
    
    // Retrieve 'safe' ranges for attenuation computation.
    cgFloat fOuterRange = getOuterRange();
    cgFloat fInnerRange = getInnerRange();
    if ( fOuterRange <= 0.0f )
        fOuterRange = CGE_EPSILON_1MM;
    if ( fInnerRange == fOuterRange )
        fInnerRange -= CGE_EPSILON_1MM;

    // Setup the light source attenuation data.
    pLightData->direction      = getZAxis(false);
    pLightData->attenuation.x  = fInnerRange;
    pLightData->attenuation.y  = fOuterRange;
    pLightData->attenuation.z  = 1.0f / (fOuterRange - fInnerRange);
    pLightData->attenuation.w  = -(fInnerRange / (fOuterRange - fInnerRange));
    pLightData->clipDistance.x = 1.0f;
    pLightData->clipDistance.y = fOuterRange;
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pLightBuffer->unlock();

    // Call base class implementation LAST
    return cgLightNode::updateLightConstants();
}

//-----------------------------------------------------------------------------
//  Name : beginLighting() (Virtual)
/// <summary>
/// Assuming the light would like to opt in to the lighting process
/// this method is called to start that process and should return the 
/// total number of lighting passes that will be required (i.e. perhaps 
/// because the light process a single frustum at a time).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPointLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
{
    // Override shadow application settings just in case.
    if ( !isShadowSource() )
        bApplyShadows = false;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginLighting( pPool, bApplyShadows, bDeferred ) < 0 )
        return -1;

    // Shadowed or unshadowed?
    mLightingPasses.clear();
    if ( !bApplyShadows )
    {
        // Only a single (unshadowed) lighting pass is required.
        mLightingPasses.clear();
        mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, 0x7FFFFFFF, 0 ) );
    
    } // End if no shadows
    else
    {
        // Iterate through each frustum to build pass list.
        for ( cgInt32 i = 0; i < 6; ++i )
        {
            // If we are applying shadows, we will require multiple steps -- the first
            // step is to fill default shadow resources prior to lighting (which may take
            // more than one pass), and the second step is to perform the lighting process.
            cgShadowGenerator * pShadowFrustum = mFrustums[i];
            if ( pShadowFrustum->requiresDefaultResource() )
            {
                // How many passes will it take to populate our shadow map(s)?
                cgUInt32 nPassCount = pShadowFrustum->getWritePassCount();
                for ( cgUInt32 j = 0; j < nPassCount; j++ )
                    mLightingPasses.push_back( LightingOpPass( Lighting_FillShadowMap, i, j ) );
            
            } // End if default
            
            // 1 pass for lighting with this frustum.
            mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, i, 0 ) );

        } // Next frustum.

    } // End if shadowed

    // Return number of passes.
    mLightingPassCount = (cgInt32)mLightingPasses.size();
    return mLightingPassCount;
}

//-----------------------------------------------------------------------------
//  Name : beginLightingPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the lighting process.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::LightingOp cgPointLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( cgLightNode::beginLightingPass( nPass, pRenderSetOut ) == Lighting_Abort )
        return Lighting_Abort;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Filling (default) shadow resources
        // On the first shadow pass, begin writing
        cgShadowGenerator * pShadowFrustum = mFrustums[ Pass.frustum ];
        if ( Pass.subPass == 0 )
            pShadowFrustum->beginWrite( );

        // Begin the shadow pass
        if ( pShadowFrustum->beginWritePass( Pass.subPass ) )
        {
            // Render using the frustum's requested visibility set.
            pRenderSetOut = pShadowFrustum->getVisibilitySet();
            Pass.op = Lighting_FillShadowMap;

        } // End if drawing
        else
        {
            // Prevent final draw.
            Pass.op = Lighting_None;

        } // End if nothing to draw

    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        cgRenderDriver * pDriver = mParentScene->getRenderDriver();
        cgCameraNode   * pCamera = pDriver->getCamera();

        // Prevent final draw unless we're absolutely certain.
        Pass.op = Lighting_None;
        
        // Forward or deferred?
        if ( mLightingDeferred )
        {
            // Processing individual frustums or just rendering light shape?
            if ( Pass.frustum == 0x7FFFFFFF )
            {
                // Test the currently active camera against this light's volume in
                // order to determine if we can apply certain optimizations.
                // If the camera falls inside the lighting volume (sphere in this case)
                // then it is impossible for it to ever see anything but the inside faces.
                cgVolumeQuery::Class VolumeStatus = cgVolumeQuery::Inside;
                cgFloat fRange = getOuterRange() * getRangeAdjust();
                if ( !pointInVolume( pCamera->getPosition( false ), 0.0f ) )
                {
                    // If it does not fall inside, then this is the much more complex case.
                    // First retrieve the frustum that represents the space between the
                    // camera position and its near plane. This frustum represents the
                    // 'volume' that can end up clipping pieces of the shape geometry.
                    const cgFrustum & ClippingVolume = pCamera->getClippingVolume();

                    // Test to see if this clipping volume intersects the /adjusted/
                    // sphere range (the larger one designed to fully encompass the
                    // light shape geometry). If it does, we're intersecting.
                    if ( ClippingVolume.testSphere( getPosition(false), fRange ) )
                        VolumeStatus = cgVolumeQuery::Intersect;
                    else
                        VolumeStatus = cgVolumeQuery::Outside;

                } // End if !inside

                // Just render light shape
                cgTransform LightShapeTransform;
                cgTransform::scaling( LightShapeTransform, fRange, fRange, fRange );
                renderDeferred( LightShapeTransform * getWorldTransform( false ), VolumeStatus );

            } // End if light shape
            else
            {
                // Render individual frustum shape directly.
                renderFrustum( pCamera, Pass.frustum, (mLightingApplyShadows) ? mFrustums[ Pass.frustum ] : CG_NULL, true );

            } // End if individual frustum

        } // End if deferred
        else
        {
            // Processing individual frustums or just rendering geometry?
            if ( Pass.frustum == 0x7FFFFFFF  )
            {
                // Begin forward render process
	            if ( beginRenderForward( mLightShapeTransform * getWorldTransform( false ) ) )
                {
                    pRenderSetOut = CG_NULL;
                    Pass.op = Lighting_ProcessLight;
                
                } // End if success
                
            } // End if light shape
            else
            {
                // Processing individual frustums
                cgToDoAssert( false, "Reintroduce forward shadowing.");
                /*if ( renderFrustum( pCamera, Pass.frustum, (mLightingApplyShadows) ? mFrustums[ Pass.frustum ] : CG_NULL, false ) )
                {
                    // ToDo: 6767 -- If we have a shadow generator, we can use its visibility set as an illumination set.
                    pRenderSetOut = m_pReceiversSet;
                    Pass.op = Lighting_ProcessLight;
                
                } // End if success*/

            } // End if individual frustum*/

        } // End if forward

    } // End if ProcessLight

    // Process this pass.
    mCurrentLightingPass = nPass;
    return Pass.op;
}

//-----------------------------------------------------------------------------
//  Name : endLightingPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( !cgLightNode::endLightingPass() )
        return false;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Complete any shadow map fill for this frustum.
        cgShadowGenerator * pShadowFrustum = mFrustums[ Pass.frustum ];
        pShadowFrustum->endWritePass();

        // On the last shadow pass, end writing
        if ( Pass.subPass == (pShadowFrustum->getWritePassCount() - 1) )
            pShadowFrustum->endWrite();
        
    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        // Complete the forward rendering process.
        if ( !mLightingDeferred )
            endRenderForward( );

    } // End if ProcessLight

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endLighting() (Virtual)
/// <summary>
/// Called in order to signify that the lighting process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endLighting( )
{
    // Valid end operation?
    if ( !cgLightNode::endLighting( ) )
        return false;

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
// Name : reassignIndirectMaps() (Virtual)
/// <summary>
/// Allows the light to determine if it is necessary to partake in the 
/// assignment of new resources at this stage, or if it can re-use any 
/// maps that it may have populated earlier.
/// Note : Returning true indicates that the light does not require further
/// resource assignment processing (i.e. it reused pooled maps).
///
/// Note: At this stage the generator needs to understand the requirements that the
///       frustum has (e.g., method, resolution, filter size, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::reassignIndirectMaps( cgTexturePool * pPool )
{
    bool bNoFurtherAssignment = true;

    // Is there any need to re-assign?
    if ( !isIndirectSource() )
    {
        // Release resources back to the pool.
        for ( cgInt i = 0; i < 6; ++i )
            mIndirectFrustums[ i ]->releaseResources();

        return true;

    } // End if not a caster

    // Attempt resource reassignment for each frustum
    for ( cgInt i = 0; i < 6; ++i )
    {
        cgReflectanceGenerator * pFrustum = mIndirectFrustums[i];

        // If the frustum has nothing to cast, release its resources
        // and skip further processing.
        if ( !pFrustum->containsRenderableObjects() )
        {
            pFrustum->releaseResources();
            continue;

        } // End if nothing to do

        // Attempt resource reassignment (if we cannot re-use the resources, process further)
        if ( !pFrustum->reassignResources( pPool ) )
            bNoFurtherAssignment = false;

    } // Next frustum

    // We need to assign further maps?
    return bNoFurtherAssignment;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectFill() (Virtual)
/// <summary>
/// Assuming the light would like to opt in to the population of any
/// assigned reflective shadow maps, this method is called to start that process
/// and should return the total number of fill passes that will be
/// required (i.e. perhaps because the light maintains multiple shadow
/// maps).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPointLightNode::beginIndirectFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectFill( pPool ) < 0 )
        return -1;

    // Clear the current pass list and reset count
    // ToDo: 6767 - Performed by base class if not already!!
    /*// Clear the current pass list and reset count.
    mShadowPasses.clear();
    mShadowPassCount = 0;

    // If this is not an indirect lighting source, do nothing.
    if ( !isIndirectSource() )
        return 0;*/

    // ToDo: 6767 - Determine if it is an appropriate time for us to update based on
    // our update period limiting property.

    // For each frustum...
    mShadowPasses.reserve( 6 ); // Assume 1 pass per frustum as a minimum
    for ( cgInt i = 0; i < 6; i++ )
    {
        // Do nothing if the frustum has nothing to draw
        // ToDo: 6767 - Use containsRenderableObjects()?
        cgReflectanceGenerator * pFrustum = mIndirectFrustums[ i ];
        if ( pFrustum->getVisibilitySet()->isEmpty() )
            continue;

        // Attempt to find resources in the pool.
        cgInt32 nFillStatus = pFrustum->assignResources( pPool );

        // If there was a failure getting resources, bail
        if ( nFillStatus & cgShadowGeneratorFillResult::DoNothing )
            return -1;

        // We cannot fill now if we were assigned one or more default resource types.
        if( nFillStatus & cgShadowGeneratorFillResult::CannotFill )
            continue;

        // We can fill if we got previously assigned resources, but don't necessarily 
        // have to if the frustum doesn't require it.
        if( (nFillStatus & cgShadowGeneratorFillResult::CanFill) && !pFrustum->shouldRegenerate() )
            continue;

        // How many passes are required to fill the assigned shadow resources?
        cgUInt32 nPassCount = pFrustum->getWritePassCount();

        // Add new passes to list
        for ( cgUInt32 j = 0; j < nPassCount; j++ )
            mShadowPasses.push_back( LightingOpPass( Lighting_FillShadowMap, i, j ) );

    } // Next frustum

    // If there was literally nothing to do, return -1 (don't even process).
    // ToDo: 6767 - Check that the caller even cares if this is -1.
    if ( mShadowPasses.empty() )
        return -1;

    // Record pass count and return it.
    mShadowPassCount = (cgInt32)mShadowPasses.size();
    return mShadowPassCount;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the indirect reflectance 
/// fill process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::beginIndirectFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation?
    if ( !cgLightNode::beginIndirectFillPass( nPass, pRenderSetOut ) )
        return false;

    // If this is the first child pass for this frustum, begin writing
    LightingOpPass & Pass = mShadowPasses[ nPass ];
    cgReflectanceGenerator * pFrustum = mIndirectFrustums[ Pass.frustum ];
    if ( Pass.subPass == 0 )
        pFrustum->beginWrite();

    // If we have not exited by now, a fill is either required or desired
    if ( pFrustum->beginWritePass( Pass.subPass ) )
    {
        // Render only objects that exist within frustum's shadow set.
        pRenderSetOut = pFrustum->getVisibilitySet();

        // We have begun this pass
        mCurrentShadowPass = nPass;
        return true;

    } // End if filling

    // Did not fill.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectFillPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the rsm fill process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endIndirectFillPass( )
{
    cgInt32 nCurrentPass = mCurrentShadowPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectFillPass() )
        return false;

    // Finish filling for this pass.
    LightingOpPass & Pass = mShadowPasses[ nCurrentPass ];
    cgReflectanceGenerator * pFrustum = mIndirectFrustums[ Pass.frustum ];
    if ( !pFrustum->endWritePass() )
        return false;

    // If this is the last child pass, end writing.
    if ( Pass.subPass == (pFrustum->getWritePassCount() - 1) )
        pFrustum->endWrite();

    // Valid
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectFill() (Virtual)
/// <summary>
/// Called in order to signify that the Indirect rsm fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endIndirectFill( )
{
    // Valid end operation?
    if ( !cgLightNode::endIndirectFill() )
        return false;

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectLighting() (Virtual)
/// <summary>
/// Assuming the light would like to opt in to the indirect lighting process
/// this method is called to start that process and should return the 
/// total number of lighting passes that will be required (i.e. perhaps 
/// because the light process a single frustum at a time).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgPointLightNode::beginIndirectLighting( cgTexturePool * pPool )
{
    // ToDo: 6767 -- can we do this in the base class? I doubt it, but worth a shot.
    // Must be an indirect source for RSM to work
    if ( !isIndirectSource() )
        return -1;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectLighting( pPool ) < 0 )
        return -1;

    // For each frustum
    mLightingPasses.reserve( 6 ); // Assume 1 pass per frustum as a minimum.
    for ( cgInt32 i = 0; i < 6; ++i )
    {
        // If we are applying shadows, we potentially require 2 passes -- the first
        // to fill a default rsm prior to lighting, and the second to perform 
        // the actual lighting process.
        cgReflectanceGenerator * pFrustum = mIndirectFrustums[i];
        if ( pFrustum->requiresDefaultResource() )
        {
            // How many passes will it take to populate our resources?
            cgUInt32 nPassCount = pFrustum->getWritePassCount();
            for ( cgUInt32 j = 0; j < nPassCount; j++ )
                mLightingPasses.push_back( LightingOpPass( Lighting_FillShadowMap, i, j ) );
        
        } // End if requires default

        // One other pass required for actual lighting (rsm based).
        mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, i, 0 ) );

    } // Next frustum.

    // If there was literally nothing to do, return -1 (don't even process).
    // ToDo: 6767 - Check that the caller even cares if this is -1.
    if ( mLightingPasses.empty() )
        return -1;

    // Return number of passes.
    mLightingPassCount = (cgInt32)mLightingPasses.size();
    return mLightingPassCount;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectLightingPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the indirect lighting process.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::LightingOp cgPointLightNode::beginIndirectLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( cgLightNode::beginIndirectLightingPass( nPass, pRenderSetOut ) == Lighting_Abort )
        return Lighting_Abort;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nPass];

    // Filling (default) shadow resources
    if ( Pass.op == Lighting_FillShadowMap )
    {
        cgReflectanceGenerator * pFrustum = mIndirectFrustums[ Pass.frustum ];

        // On the first child pass, begin writing
        if ( Pass.subPass == 0 )
            pFrustum->beginWrite();

        // Begin the pass
        if ( pFrustum->beginWritePass( Pass.subPass ) )
        {
            // Render using the frustum's requested visibility set.
            pRenderSetOut = pFrustum->getVisibilitySet();
            Pass.op = Lighting_FillShadowMap;

        } // End if drawing
        else
        {
            // Prevent final draw.
            Pass.op = Lighting_None;

        } // End if nothing to draw

    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        // For indirect lighting, all we need to do is setup the RSM data
        mIndirectFrustums[ Pass.frustum ]->beginRead();

    } // End if ProcessLight

    // Process this pass.
    mCurrentLightingPass = nPass;
    return Pass.op;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectLightingPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the indirect lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endIndirectLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectLightingPass() )
        return false;

    // Complete any fill operations for this frustum.
    LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Complete fill for this pass.
        cgReflectanceGenerator * pFrustum = mIndirectFrustums[ Pass.frustum ];
        pFrustum->endWritePass();

        // On the last child pass, end writing.
        if ( Pass.subPass == (pFrustum->getWritePassCount() - 1) )
            pFrustum->endWrite();

    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        // For indirect lighting, all we need to do is setup the RSM data
        mIndirectFrustums[ Pass.frustum ]->endRead();

    } // End if ProcessLight

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectLighting() (Virtual)
/// <summary>
/// Called in order to signify that the indirect lighting process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::endIndirectLighting( )
{
    // Valid end operation?
    if ( !cgLightNode::endIndirectLighting( ) )
        return false;

    // Valid.
    mLightingPasses.clear();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setIndirectLightingMethod() (Virtual)
/// <summary>
/// Sets the current indirect lighting method for the generator to use
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::setIndirectLightingMethod( cgUInt32 nMethod )
{
    // Call the base class
    cgLightNode::setIndirectLightingMethod( nMethod );

    // Update the generators
    for ( cgInt i = 0; i < 6; ++i )
        mIndirectFrustums[i]->setMethod( nMethod );
}

//-----------------------------------------------------------------------------
//  Name : renderFrustum () (Protected)
/// <summary>
/// Process lighting & (potentially) shadows simultaneously. It is important
/// that NULL is specified for 'pShadowFrustum' if no shadows are to be
/// processed. This function may still be used to render an individual point
/// light frustum in either case however.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::renderFrustum( cgCameraNode * pRenderCamera, cgInt32 nFrustumIndex, cgShadowGenerator * pShadowFrustum, bool bDeferred )
{
    // We use a seperate mesh to store our six frustum shapes (used for deferred rendering)
	cgMesh * pFrustumMesh = mFrustumMesh.getResource(true);

	// Build a scaling matrix based purely on the range (i.e., no error tolerance).
    cgTransform LightShapeTransform;
    cgFloat fRange = getOuterRange();
    cgTransform::scaling( LightShapeTransform, fRange, fRange, fRange );

    // Compute the final world space shadow frustum shape matrix
    LightShapeTransform *= getWorldTransform( false );

    // Call the appropriate rendering method.
	if ( bDeferred )
	{
        // ToDo: This may be an expensive (relatively speaking) frustum generation.
        // Can / should we cache these?
        cgFrustum Frustum( (cgUInt8)nFrustumIndex, getPosition(false), fRange, 0.0f );

        // Test the currently active camera against this light's volume in
        // order to determine if we can apply certain optimizations.
        // If the camera falls inside the lighting volume (one frustum in this case)
        // then it is impossible for it to ever see anything but the inside faces.
        cgVolumeQuery::Class VolumeStatus = cgVolumeQuery::Inside;
        if ( !Frustum.testPoint( pRenderCamera->getPosition( false ) ) )
        {
            // If it does not fall inside, then this is the much more complex case.
            // First retrieve the frustum that represents the space between the
            // camera position and its near plane. This frustum represents the
            // 'volume' that can end up clipping pieces of the shape geometry.
            const cgFrustum & ClippingVolume = pRenderCamera->getClippingVolume();

            // Test to see if this clipping volume intersects the specified
            // frustum (represents the light shape to be rendered).
            if ( ClippingVolume.testFrustum( Frustum ) )
                VolumeStatus = cgVolumeQuery::Intersect;
            else
                VolumeStatus = cgVolumeQuery::Outside;

        } // End if !inside

        // Render the shape
        renderDeferred( LightShapeTransform, VolumeStatus, pShadowFrustum, CG_NULL, pFrustumMesh, nFrustumIndex );
        
	} // End if deferred
	else
	{
		// For forward rendering, ensure our 90 degree frustum values are as precise as possible (user clip planes really need this)
		cgFrustum PreciseFrustum( (cgUInt8)nFrustumIndex, getPosition( false ), fRange );
		return beginRenderForward( LightShapeTransform, pShadowFrustum, &PreciseFrustum );

	} // End if forward

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgPointLightNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What property was modified?
    if ( e->context == _T("OuterRange") || e->context == _T("RangeAdjust") || e->context == _T("ApplyRescale") )
    {
        cgFloat fRange = getOuterRange();
        cgFloat fRangeAdjust = getRangeAdjust();

        // Recompute render projection matrix.
        cgMatrix::perspectiveFovLH( mProjectionMatrix, CGEToRadian(90.0f), 1.0f, 1.0f, fRange );

        // Set the light's world matrix (sized to range with tolerance)
        cgTransform::scaling( mLightShapeTransform, fRange * fRangeAdjust, fRange * fRangeAdjust, fRange * fRangeAdjust );

        // Update far clip plane of shadow frustum cameras to match range.
        for ( cgInt i = 0; i < 6; ++i )
        {
            cgCameraNode * pFrustumCamera = mFrustums[i]->getCamera();
            pFrustumCamera->setFarClip( fRange );

        } // Next frustum

        // Light source node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );
        
    } // End if Range | RangeAdjust | ApplyRescale

    // Call base class implementation last
    cgLightNode::onComponentModified( e );

}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::postCreate( )
{
    // Call base class implementation.
    if ( !cgLightNode::postCreate( ) )
        return false;

    // Get access to required systems.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    cgRenderDriver    * pDriver    = mParentScene->getRenderDriver();

    // Create a unit-size sphere (if it doesn't already exist) that will allow us
    // to represent the shape of the light source.
    if ( !pResources->getMesh( &mLightShape, _T("Core::LightShapes::UnitSphere") ) )
    {
        cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Construct a unit-sized sphere mesh
        pMesh->createSphere( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 1.0f, 10, 10, false, 
                             cgMeshCreateOrigin::Center, true, pResources );
        
        // Add to the resource manager
        pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitSphere"), cgDebugSource() );

    } // End if no existing mesh

    // Create an omni-directional collection of frustums (box) that allows us to render 
    // frustum shapes relating to each of the 6 frustums during point light shadow generation.
    if ( !pResources->getMesh( &mFrustumMesh, _T("Core::LightShapes::Omni") ) )
    {
        cgUInt32  Indices[18];
        cgVector3 Vertices[9];
	    cgMesh  * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Set the mesh data format
        cgVertexFormat * pFormat = cgVertexFormat::formatFromFVF( D3DFVF_XYZ );
        pMesh->prepareMesh( pFormat, false, pResources );

	    // Frustum vertices
	    Vertices[0] = cgVector3( 0, 0, 0);
        Vertices[1] = cgVector3( -1.0f,  1.0f, -1.0f );
        Vertices[2] = cgVector3( -1.0f,  1.0f,  1.0f );
        Vertices[3] = cgVector3(  1.0f,  1.0f,  1.0f );
        Vertices[4] = cgVector3(  1.0f,  1.0f, -1.0f );
        Vertices[5] = cgVector3( -1.0f, -1.0f, -1.0f );
        Vertices[6] = cgVector3( -1.0f, -1.0f,  1.0f );
        Vertices[7] = cgVector3(  1.0f, -1.0f,  1.0f );
        Vertices[8] = cgVector3(  1.0f, -1.0f, -1.0f );
	    pMesh->setVertexSource( Vertices, 9, pFormat );

        // +X indices
        Indices[0]  = 4; Indices[1]  = 3; Indices[2]  = 7;  // Base Tri 1
        Indices[3]  = 4; Indices[4]  = 7; Indices[5]  = 8;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 8; Indices[11] = 7;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 7; Indices[14] = 3;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 4; Indices[17] = 8;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 0 );

        // -X indices
        Indices[0]  = 2; Indices[1]  = 1; Indices[2]  = 5;  // Base Tri 1
        Indices[3]  = 2; Indices[4]  = 5; Indices[5]  = 6;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 1; Indices[8]  = 2;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 6; Indices[11] = 5;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 5; Indices[14] = 1;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 2; Indices[17] = 6;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 1 );

        // +Y indices
        Indices[0]  = 1; Indices[1]  = 2; Indices[2]  = 3;  // Base Tri 1
        Indices[3]  = 1; Indices[4]  = 3; Indices[5]  = 4;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 1; Indices[8]  = 4;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 3; Indices[11] = 2;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 2; Indices[14] = 1;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 4; Indices[17] = 3;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 2 );

        // -Y indices
        Indices[0]  = 5; Indices[1]  = 7; Indices[2]  = 6;  // Base Tri 1
        Indices[3]  = 5; Indices[4]  = 8; Indices[5]  = 7;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 6; Indices[8]  = 7;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 8; Indices[11] = 5;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 5; Indices[14] = 6;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 7; Indices[17] = 8;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 3 );

        // +Z indices
        Indices[0]  = 2; Indices[1]  = 7; Indices[2]  = 3;  // Base Tri 1
        Indices[3]  = 2; Indices[4]  = 6; Indices[5]  = 7;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 2; Indices[8]  = 3;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 7; Indices[11] = 6;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 6; Indices[14] = 2;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 3; Indices[17] = 7;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 4 );

        // -Z indices
        Indices[0]  = 1; Indices[1]  = 4; Indices[2]  = 8;  // Base Tri 1
        Indices[3]  = 1; Indices[4]  = 8; Indices[5]  = 5;  // Base Tri 2
	    Indices[6]  = 0; Indices[7]  = 4; Indices[8]  = 1;  // Top   Tri
	    Indices[9]  = 0; Indices[10] = 5; Indices[11] = 8;  // Bottom Tri
	    Indices[12] = 0; Indices[13] = 8; Indices[14] = 4;  // Left   Tri
	    Indices[15] = 0; Indices[16] = 1; Indices[17] = 5;  // Right  Tri
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 18, cgMaterialHandle::Null, 5 );

	    // Build the mesh
        pMesh->endPrepare( );
        
        // Add to the resource manager
        pResources->addMesh( &mFrustumMesh, pMesh, 0, _T("Core::LightShapes::Omni"), cgDebugSource() );

    } // End if no mesh

    // Configurable axis vectors used to construct view matrices. In the 
    // case of the omni light, we align all frustums to the world axes.
    cgVector3 X( 1, 0, 0 );
	cgVector3 Y( 0, 1, 0 );
	cgVector3 Z( 0, 0, 1 );

    // Initialize shadow frustums ready for rendering
    const cgVector3 & vecPos = getPosition( false );
    cgFloat fRange = getOuterRange(), fRangeAdjust = getRangeAdjust();
    for ( cgInt i = 0; i < 6; ++i )
    {
        if ( !mFrustums[i]->initialize() )
            return false;

        // Compute the correct projection matrix to use. This will generally
        // always be a 90 degree frustum in the case of omni-directional
        // rendering, and will be applied to all 6 frustums. We may adjust this
        // at a later stage to repair shadow map borders, but it's a sensible default.
        cgCameraNode * pFrustumCamera = mFrustums[i]->getCamera();
        pFrustumCamera->setFOV( 90.0f );
        pFrustumCamera->setAspectRatio( 1.0f, true );
        pFrustumCamera->setNearClip( CGE_EPSILON_1CM * 10.0f );
        pFrustumCamera->setFarClip( fRange );
        
        // Frustum camera's position matches the light source position
        pFrustumCamera->setPosition( vecPos );

        // ToDo: Remove commented "LookAt" versions?
        // Generate the correct view matrix for the frustum
        switch ( i )
		{
			case 0: //pFrustumCamera->LookAt( vecPos, (vecPos + X), Y ); break;
                pFrustumCamera->setOrientation( -Z, +Y, +X ); break;
			case 1: // pFrustumCamera->LookAt( vecPos, (vecPos - X), Y ); break;
                pFrustumCamera->setOrientation( +Z, +Y, -X ); break;
			case 2: // pFrustumCamera->LookAt( vecPos, (vecPos + Y), -Z ); break;
                pFrustumCamera->setOrientation( +X, -Z, +Y ); break;
			case 3: // pFrustumCamera->LookAt( vecPos, (vecPos - Y), Z ); break;
                pFrustumCamera->setOrientation( +X, +Z, -Y ); break;
			case 4: // pFrustumCamera->LookAt( vecPos, (vecPos + Z), Y ); break;
                pFrustumCamera->setOrientation( +X, +Y, +Z ); break;
			case 5: // pFrustumCamera->LookAt( vecPos, (vecPos - Z), Y ); break;
                pFrustumCamera->setOrientation( -X, +Y, -Z ); break;
		
        } // End Switch
    
    } // Next frustum

    // Configure indirect lighting frustums (ToDo: Support Dual Paraboloid)
    // ToDo: 6767 - Reintroduce?
    /*for ( cgInt i = 0; i < 6; ++i )
    {
        if ( !mIndirectFrustums[i]->initialize( ) )
            return false;

        // Compute the correct projection matrix to use. This will generally
        // always be a 90 degree frustum in the case of omni-directional
        // rendering, and will be applied to all 6 frustums. We may adjust this
        // at a later stage to repair shadow map borders, but it's a sensible default.
        cgCameraNode * pFrustumCamera = mIndirectFrustums[i]->getCamera();
        pFrustumCamera->setFOV( 90.0f );
        pFrustumCamera->setAspectRatio( 1.0f, true );
        pFrustumCamera->setNearClip( 1.0f );
        pFrustumCamera->setFarClip( fRange );
        
        // Frustum camera's position matches the light source position
        pFrustumCamera->setPosition( vecPos );

        // ToDo: Remove commented "LookAt" versions?
        // Generate the correct view matrix for the frustum
        switch ( i )
		{
			case 0: //pFrustumCamera->LookAt( vecPos, (vecPos + X), Y ); break;
                pFrustumCamera->setOrientation( -Z, +Y, +X ); break;
			case 1: // pFrustumCamera->LookAt( vecPos, (vecPos - X), Y ); break;
                pFrustumCamera->setOrientation( +Z, +Y, -X ); break;
			case 2: // pFrustumCamera->LookAt( vecPos, (vecPos + Y), -Z ); break;
                pFrustumCamera->setOrientation( +X, -Z, +Y ); break;
			case 3: // pFrustumCamera->LookAt( vecPos, (vecPos - Y), Z ); break;
                pFrustumCamera->setOrientation( +X, +Z, -Y ); break;
			case 4: // pFrustumCamera->LookAt( vecPos, (vecPos + Z), Y ); break;
                pFrustumCamera->setOrientation( +X, +Y, +Z ); break;
			case 5: // pFrustumCamera->LookAt( vecPos, (vecPos - Z), Y ); break;
                pFrustumCamera->setOrientation( -X, +Y, -Z ); break;
		
        } // End Switch
   
    } // Next frustum*/

    // Compute render projection matrix.
    cgMatrix::perspectiveFovLH( mProjectionMatrix, CGEToRadian(90.0f), 1.0f, 1.0f, fRange );

    // Set the light's world matrix (sized to range with tolerance)
    cgTransform::scaling( mLightShapeTransform, fRange * fRangeAdjust, fRange * fRangeAdjust, fRange * fRangeAdjust );

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    cgUInt32 nShadowUpdateRate = getShadowUpdateRate();
    if ( nShadowUpdateRate > 0 )
        mShadowTimeSinceLast = -cgMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)nShadowUpdateRate );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::canScale( ) const
{
    // Point lights cannot be scaled
    return false;
}

//-----------------------------------------------------------------------------
// Name : canRotate ( )
/// <summary>Determine if rotation of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::canRotate( ) const
{
    // Point lights cannot be rotated
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can updatate our own properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPointLightNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgLightNode::setCellTransform( Transform, Source ) )
        return false;

    // Recompute frustum positions
    const cgVector3 & vecPos = getPosition( false );
    for ( cgInt i = 0; i < 6; ++i )
    {
        // ToDo: 9999 - Rotating the light source causes all havoc
        // to break loose. Are we sure we don't want the frustum's
        // to match the light source orientation?

        // Frustum camera's position matches the light source position
        // but its orientation remains the same.
        cgCameraNode * pFrustumCamera = mFrustums[i]->getCamera();
        pFrustumCamera->setPosition( vecPos );
        pFrustumCamera = mIndirectFrustums[i]->getCamera();
        pFrustumCamera->setPosition( vecPos );

    } // Next frustum

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getBoundingSphere () (Virtual)
/// <summary>
/// Retrieve the bounding sphere for this point light source as it exists in
/// world space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingSphere cgPointLightNode::getBoundingSphere( )
{
    return cgBoundingSphere( getPosition(false), getOuterRange() );
}