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
// Name : cgSpotLight.cpp                                                    //
//                                                                           //
// Desc : Spot light source classes.                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSpotLight Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgSpotLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgTargetObject.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h> // ToDo: Make sure this needs to stay.
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgMesh.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgTexturePool.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>
#include <Math/cgExtrudedBoundingBox.h>
#include <System/cgStringUtility.h>

// ToDo: 9999 - When properties change, we need to regenerate some things.
// For instance, when attenuation mask changes we may need to switch from unit cone
// to unit pyramid, etc.

// ToDo: 9999 - When properties change that affect the node's size, shape or other
//              attribute that would affect the 'dirty' status of the node for the
//              generation of shadow maps, we must call the cgObjectNode::nodeUpdated()
//              method.

// ToDo: 9999 - Timed shadow update rate doesn't seem to be limiting us when 
//              other objects within the shadow frustum are updated?

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgSpotLightObject::mInsertSpotLight;
cgWorldQuery cgSpotLightObject::mUpdateRanges;
cgWorldQuery cgSpotLightObject::mUpdateConeAngles;
cgWorldQuery cgSpotLightObject::mUpdateFalloff;
cgWorldQuery cgSpotLightObject::mUpdateShadowRate;
cgWorldQuery cgSpotLightObject::mLoadSpotLight;

///////////////////////////////////////////////////////////////////////////////
// cgSpotLightObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpotLightObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightObject::cgSpotLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgLightObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mOuterRange               = 0.0f;
    mInnerRange               = 0.0f;
    mOuterCone                = 62.0f;
    mInnerCone                = 60.0f;
    mFalloff                  = 1.0f;
    mShadowUpdateRate         = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgSpotLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightObject::cgSpotLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgLightObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgSpotLightObject * pObject = (cgSpotLightObject*)pInit;
    mOuterRange               = pObject->mOuterRange;
    mInnerRange               = pObject->mInnerRange;
    mOuterCone                = pObject->mOuterCone;
    mInnerCone                = pObject->mInnerCone;
    mFalloff                  = pObject->mFalloff;
    mShadowUpdateRate         = pObject->mShadowUpdateRate;

    // ToDo: 6767 - Reintroduce for new shadow settings.
    // Deep copy shadow frustum data
    //m_nShadowFrustumId        = 0;
    // ToDo: 6767 - Remove all of these in all lights
    //m_ShadowConfig              = pObject->m_ShadowConfig;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpotLightObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightObject::~cgSpotLightObject()
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
void cgSpotLightObject::dispose( bool bDisposeBase )
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
cgWorldObject * cgSpotLightObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgSpotLightObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgSpotLightObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgSpotLightObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSpotLightObject::getDatabaseTable( ) const
{
    return _T("Objects::SpotLight");
}

//-----------------------------------------------------------------------------
//  Name : setOuterRange()
/// <summary>
/// Set the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::setOuterRange( cgFloat fRange )
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
void cgSpotLightObject::setInnerRange( cgFloat fRange )
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
//  Name : setFalloff ()
/// <summary>
/// Set the spotlight cone falloff factor.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::setFalloff( cgFloat fValue )
{
    // Is this a no-op?
    if ( mFalloff == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateFalloff.bindParameter( 1, fValue );
        mUpdateFalloff.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateFalloff.step( true ) )
        {
            cgString strError;
            mUpdateFalloff.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update falloff for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
    mFalloff = fValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("Falloff");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setInnerCone ()
/// <summary>
/// Set the inner cone (Theta) angle of the spotlight in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::setInnerCone( cgFloat fValue )
{
    // Is this a no-op?
    if ( mInnerCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, mOuterCone );
        mUpdateConeAngles.bindParameter( 2, fValue );
        mUpdateConeAngles.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateConeAngles.step( true ) )
        {
            cgString strError;
            mUpdateConeAngles.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update cone angles for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
    mInnerCone = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("InnerCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer cone is at LEAST as large as the inner cone
    if ( mInnerCone > mOuterCone )
        setOuterCone( mInnerCone );
}

//-----------------------------------------------------------------------------
//  Name : setOuterCone ()
/// <summary>
/// Set the outer cone (Phi) angle of the spotlight in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::setOuterCone( cgFloat fValue )
{
    // Is this a no-op?
    if ( mOuterCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, fValue );
        mUpdateConeAngles.bindParameter( 2, mInnerCone );
        mUpdateConeAngles.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateConeAngles.step( true ) )
        {
            cgString strError;
            mUpdateConeAngles.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update cone angles for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store the new light source cone size
    mOuterCone = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("OuterCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner cone at MOST fits inside the outer cone
    if ( mInnerCone > mOuterCone )
        setInnerCone( mOuterCone );
}

//-----------------------------------------------------------------------------
//  Name : setShadowUpdateRate ()
/// <summary>
/// Set the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::setShadowUpdateRate( cgUInt32 nRate )
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

/*//-----------------------------------------------------------------------------
//  Name : SetShadowFrustumConfig ()
/// <summary>
/// Set the base shadow frustum configuration parameters.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::SetShadowFrustumConfig( const cgShadowGeneratorConfig & Config )
{
    // ToDo: 9999 - No-Op Processing.
    
    // Update database
    if ( m_nShadowFrustumId && !UpdateShadowFrustum( m_nShadowFrustumId, Config ) )
        return;

    // Update internal value.
    m_ShadowConfig = Config;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ShadowFrustumConfig");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}*/

//-----------------------------------------------------------------------------
//  Name : getOuterRange()
/// <summary>
/// Get the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSpotLightObject::getOuterRange( ) const
{
    return mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerRange()
/// <summary>
/// Get the inner range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSpotLightObject::getInnerRange( ) const
{
    return mInnerRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerCone ()
/// <summary>
/// Get the inner cone (Theta) angle of the spotlight in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSpotLightObject::getInnerCone( ) const
{
    return mInnerCone;
}

//-----------------------------------------------------------------------------
//  Name : getOuterCone ()
/// <summary>
/// Get the outer cone (Phi) angle of the spotlight in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSpotLightObject::getOuterCone( ) const
{
    return mOuterCone;
}

//-----------------------------------------------------------------------------
//  Name : getFalloff ()
/// <summary>
/// Get the spotlight cone falloff factor.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSpotLightObject::getFalloff( ) const
{
    return mFalloff;
}

//-----------------------------------------------------------------------------
//  Name : getShadowUpdateRate ()
/// <summary>
/// Get the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgSpotLightObject::getShadowUpdateRate( ) const
{
    return mShadowUpdateRate;
}

/*//-----------------------------------------------------------------------------
//  Name : GetShadowFrustumConfig ()
/// <summary>
/// Get the base shadow frustum configuration parameters.
/// </summary>
//-----------------------------------------------------------------------------
const cgShadowGeneratorConfig & cgSpotLightObject::GetShadowFrustumConfig( ) const
{
    return m_ShadowConfig;
}*/

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgSpotLightObject::getLocalBoundingBox( )
{
    cgBoundingBox Bounds;

    // Convert outer cone angle to a radius at the base.
    cgFloat fBaseRadius = tanf(CGEToRadian(mOuterCone)*0.5f) * mOuterRange;

    // Generate local bounding box from cone (light aligned to Z axis).
    Bounds.min = cgVector3( -fBaseRadius, -fBaseRadius, 0.0f );
    Bounds.max = cgVector3(  fBaseRadius,  fBaseRadius, mOuterRange );
	return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;
    
    // Retrieve useful values
    float fZoomFactor  = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition( false ), 2.5f );
    float fTolerance   = 2.0f * fZoomFactor;
    
    // Compute vertices for the tip and base of the cone
    cgVector3 Points[10];
    float fSize   = fZoomFactor * 25.0f * cosf(CGEToRadian(mOuterCone * 0.5f));
    float fRadius = tanf(CGEToRadian(mOuterCone * 0.5f)) * fSize;
    Points[0] = cgVector3( 0, 0, 0 );
    for ( size_t i = 0; i < 8; ++i )
    {
        // Build vertex
        Points[i+1].x = (sinf( (CGE_TWO_PI / 8.0f) * (float)i ) * fRadius);
        Points[i+1].y = (cosf( (CGE_TWO_PI / 8.0f) * (float)i ) * fRadius);
        Points[i+1].z = fSize;
    
    } // Next Base Vertex
    
    // Compute indices that will allow us to test each triangle of the cone.
    cgUInt32 Indices[24];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 5;
    Indices[12] = 0; Indices[13] = 5; Indices[14] = 6;
    Indices[15] = 0; Indices[16] = 6; Indices[17] = 7;
    Indices[18] = 0; Indices[19] = 7; Indices[20] = 8;
    Indices[21] = 0; Indices[22] = 8; Indices[23] = 1;

    // Test the triangles. Use a Bi-Directional test so that we don't
    // have to test the base of the cone.
    bool    bIntersect = false;
    cgFloat t, tMin = FLT_MAX;
    for ( size_t i = 0; i < 8; ++i )
    {
        // Compute plane for the triangle
        cgPlane Plane;
        const cgVector3 & v1 = Points[Indices[(i*3)+0]];
        const cgVector3 & v2 = Points[Indices[(i*3)+1]];
        const cgVector3 & v3 = Points[Indices[(i*3)+2]];
        cgPlane::fromPoints( Plane, v1, v2, v3 );

        // Ray intersects the triangle plane?
        if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, true, false ) )
        {
            // Check if it intersects the actual triangle (within a tolerance)
            if ( cgCollision::pointInTriangle( vOrigin + (vDir * t), v1, v2, v3, (cgVector3&)Plane, fTolerance ) )
            {
                // Closest hit?
                if ( t < tMin )
                {
                    tMin = t;
                    bIntersect = true;
                
                } // End if closest

            } // End if intersects triangle
        
        } // End if intersects plane

    } // Next Triangle

    // Intersection occurred?
    if ( bIntersect )
        fDistance = tMin;
    return bIntersect;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // ToDo: Light sources should not show additional cones / influences etc.
    // if they are selected only as part of a selected closed group.
    
    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);
    
    // Set the object's transformation matrix to the device (light
    // representation will be constructed in object space)
    cgTransform InverseObjectTransform;
    const cgTransform & ObjectTransform = pIssuer->getWorldTransform( false );
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    
    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    bool       bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);
    bool       bSelected   = pIssuer->isSelected();
    cgFloat    fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );
    cgUInt32   nColor      = (bSelected) ? 0xFFFFFFFF : 0xFFFFF600;

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    cgShadedVertex Points[29];
    for ( cgInt i = 0; i < 9; ++i )
        Points[i].color = nColor;

    // Compute vertices for the tip and base of the cone
    cgFloat fSize      = fZoomFactor * 25.0f * cosf(CGEToRadian(mOuterCone * 0.5f));
    cgFloat fRadius    = tanf(CGEToRadian(mOuterCone * 0.5f)) * fSize;
    Points[0].position = cgVector3( 0, 0, 0 );
    for ( cgInt i = 0; i < 8; ++i )
    {
        // Build vertex
        Points[i+1].position.x = (sinf( (CGE_TWO_PI / 8.0f) * (cgFloat)i ) * fRadius);
        Points[i+1].position.y = (cosf( (CGE_TWO_PI / 8.0f) * (cgFloat)i ) * fRadius);
        Points[i+1].position.z = fSize;
    
    } // Next Base Vertex
    
    // Compute indices that will allow us to draw the cone using a 
    // tri-list (wireframe) so that we can easily take advantage of back-face culling.
    cgUInt32 Indices[29];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 5;
    Indices[12] = 0; Indices[13] = 5; Indices[14] = 6;
    Indices[15] = 0; Indices[16] = 6; Indices[17] = 7;
    Indices[18] = 0; Indices[19] = 7; Indices[20] = 8;
    Indices[21] = 0; Indices[22] = 8; Indices[23] = 1;

    // Begin rendering
    bool bWireframe = (flags & cgSandboxRenderFlags::Wireframe);
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    pDriver->setWorldTransform( ObjectTransform );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Draw the cone
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 9, 8, Indices, cgBufferFormat::Index32, Points );

            // Now we'll render the base of the cone. So that we can construct in object space, 
            // transform camera culling details into object space too.
            cgVector3 vCameraLook, vCameraPos;
            InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis( false ) );
            InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition( false ) );
            cgVector3::normalize( vCameraLook, vCameraLook );

            // Now construct the "base" of the cone representation
            bool bCull = ( bOrtho ) ? (cgVector3::dot( cgVector3( 0,0,1 ), vCameraLook ) >= 0.0f ) :
                                      (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,1 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build base indices
            if ( !bCull )
            {
                for ( cgInt i = 0; i < 8; ++i )
                    Indices[i] = i+1;
                Indices[8] = 1;
                
                // Draw the base of the cone as a line-strip
                pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 9, 8, Indices, cgBufferFormat::Index32, Points );

            } // End if !cull

            // If the light source (or target) is selected show the inner/outer cones too
            // unless this is part of a selected group.
            cgTargetNode * pTarget = pIssuer->getTargetNode();
            if ( (bSelected || (pTarget && pTarget->isSelected())) && !pIssuer->isMergedAsGroup() )
            {
                // Inner cone first
                fRadius = tanf(CGEToRadian(mInnerCone * 0.5f)) * mOuterRange;
                nColor  = 0xFF99CCE5;
                
                // Generate line strip circle 
                for ( cgInt i = 0; i < 28; ++i )
                {
                    // Build vertex
                    Points[i].position.x = (sinf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                    Points[i].position.y = (cosf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                    Points[i].position.z = mOuterRange;
                    Points[i].color      = nColor;
                    
                    // Build index
                    Indices[i] = i;
                    
                } // Next Segment

                // Add last index to connect back to the start of the circle
                Indices[28] = 0;

                // Render the circle.
                pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 28, 28, Indices, cgBufferFormat::Index32, Points );

                // Render connecting lines from the tip to the four extreme points on the circle.
                Points[28].position = cgVector3(0,0,0);
                Points[28].color    = nColor;
                Indices[0] = 28; Indices[1] = 0;
                Indices[2] = 28; Indices[3] = 7;
                Indices[4] = 28; Indices[5] = 14;
                Indices[6] = 28; Indices[7] = 21;
                pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 29, 4, Indices, cgBufferFormat::Index32, Points );

                // Outer cone next if the cones aren't identical
                if ( mOuterCone > mInnerCone )
                {
                    fRadius = tanf(CGEToRadian(mOuterCone * 0.5f)) * mOuterRange;
                    nColor  = 0xFF4C6672;

                    // Generate line strip circle 
                    for ( cgInt i = 0; i < 28; ++i )
                    {
                        // Build vertex
                        Points[i].position.x = (sinf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                        Points[i].position.y = (cosf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                        Points[i].position.z = mOuterRange;
                        Points[i].color      = nColor;
                        
                        // Build index
                        Indices[i] = i;
                        
                    } // Next Segment

                    // Add last index to connect back to the start of the circle
                    Indices[28] = 0;

                    // Render the circle (Indices remain the same as for the inner cone).
                    pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 28, 28, Indices, cgBufferFormat::Index32, Points );

                    // Render connecting lines from the tip to the four extreme points on the circle.
                    Points[28].position = cgVector3(0,0,0);
                    Points[28].color    = nColor;
                    Indices[0] = 28; Indices[1] = 0;
                    Indices[2] = 28; Indices[3] = 7;
                    Indices[4] = 28; Indices[5] = 14;
                    Indices[6] = 28; Indices[7] = 21;
                    pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 29, 4, Indices, cgBufferFormat::Index32, Points );

                } // End if outer > inner

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
void cgSpotLightObject::applyObjectRescale( cgFloat fScale )
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
bool cgSpotLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SpotLightObject )
        return true;

    // Supported by base?
    return cgLightObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgSpotLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("SpotLightObject::insertComponentData") );

        // Write frustum configuration source to the database first of all.
        /*if ( !m_nShadowFrustumId )
            m_nShadowFrustumId = InsertShadowFrustum( m_ShadowConfig );
        if ( !m_nShadowFrustumId )
        {
            mWorld->rollbackTransaction( _T("SpotLightObject::insertComponentData") );
            return false;
        
        } // End if failed*/

        // Insert new spot light object
        prepareQueries();
        mInsertSpotLight.bindParameter( 1, mReferenceId );
        mInsertSpotLight.bindParameter( 2, mOuterRange );
        mInsertSpotLight.bindParameter( 3, mInnerRange );
        mInsertSpotLight.bindParameter( 4, mOuterCone );
        mInsertSpotLight.bindParameter( 5, mInnerCone );
        mInsertSpotLight.bindParameter( 6, mFalloff );
        mInsertSpotLight.bindParameter( 7, (cgUInt32)0 ); // ToDo: DistanceAttenuationSplineId
        mInsertSpotLight.bindParameter( 8, (cgUInt32)0 ); // ToDo: AttenuationMaskSamplerId
        mInsertSpotLight.bindParameter( 9, (cgUInt32)0 ); // ToDo: m_nShadowFrustumId
        mInsertSpotLight.bindParameter( 10, mShadowUpdateRate );

        // Database ref count (just in case it has already been adjusted)
        mInsertSpotLight.bindParameter( 11, mSoftRefCount );
        
        // Execute
        if ( !mInsertSpotLight.step( true ) )
        {
            cgString strError;
            mInsertSpotLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for spot light object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("SpotLightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("SpotLightObject::insertComponentData") );

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
bool cgSpotLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the light data.
    prepareQueries();
    mLoadSpotLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSpotLight.step( ) || !mLoadSpotLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadSpotLight.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for spot light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for spot light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSpotLight.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSpotLight;

    // Update our local members
    mLoadSpotLight.getColumn( _T("OuterRange"), mOuterRange );
    mLoadSpotLight.getColumn( _T("InnerRange"), mInnerRange );
    mLoadSpotLight.getColumn( _T("OuterCone"), mOuterCone );
    mLoadSpotLight.getColumn( _T("InnerCone"), mInnerCone );
    mLoadSpotLight.getColumn( _T("Falloff"), mFalloff );
    // ToDo: 9999 - DistanceAttenuationSplineId
    // ToDo: 9999 - AttenuationMaskSamplerId
    // ToDo: mLoadSpotLight.getColumn( _T("ShadowFrustumId"), m_nShadowFrustumId );
    mLoadSpotLight.getColumn( _T("ShadowUpdateRate"), mShadowUpdateRate );

    // Load frustum configuration source from the database.
    /*if ( m_nShadowFrustumId != 0 )
    {
        if ( !LoadShadowFrustum( m_nShadowFrustumId, m_ShadowConfig ) )
        {
            // Release any pending read operation.
            mLoadSpotLight.reset();
            return false;
        
        } // End if failed
    
    } // End if has frustum*/

    // Call base class implementation to read remaining data.
    if ( !cgLightObject::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        // ToDo: 6767 - Reintroduce for new shadow settings.
        // Reset shadow frustum identifiers to ensure they are re-serialized as necessary.
        //m_nShadowFrustumId = 0;

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
void cgSpotLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertSpotLight.isPrepared() )
            mInsertSpotLight.prepare( mWorld, _T("INSERT INTO 'Objects::SpotLight' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11)"), true );
        if ( !mUpdateRanges.isPrepared() )
            mUpdateRanges.prepare( mWorld, _T("UPDATE 'Objects::SpotLight' SET OuterRange=?1, InnerRange=?2 WHERE RefId=?3"), true );
        if ( !mUpdateConeAngles.isPrepared() )
            mUpdateConeAngles.prepare( mWorld, _T("UPDATE 'Objects::SpotLight' SET OuterCone=?1, InnerCone=?2 WHERE RefId=?3"), true );
        if ( !mUpdateFalloff.isPrepared() )
            mUpdateFalloff.prepare( mWorld, _T("UPDATE 'Objects::SpotLight' SET Falloff=?1 WHERE RefId=?2"), true );
        if ( !mUpdateShadowRate.isPrepared() )
            mUpdateShadowRate.prepare( mWorld, _T("UPDATE 'Objects::SpotLight' SET ShadowUpdateRate=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadSpotLight.isPrepared() )
        mLoadSpotLight.prepare( mWorld, _T("SELECT * FROM 'Objects::SpotLight' WHERE RefId=?1"), true );
}

/* ToDo: Remove on completion.
//-----------------------------------------------------------------------------
//  Name : Deserialize ()
/// <summary>
/// Initialize the object based on the XML data pulled from the 
/// environment / scene definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightObject::Deserialize( const cgXMLNode & InitData, cgSceneLoader * pLoader )
{
    cgXMLNode xChild, xAttenShadow;

    // Allow base class to process data first
    if ( !cgLightObject::Deserialize( InitData, pLoader ) )
        return false;

    // ToDo: Clean this concept up -- arbitrarily grows cones to fix lightmap issue.
    bool bIsDetail = (m_LightingStage == cgSceneProcessStage::Both) ? true : false;
	
    // Iterate through all child nodes to get custom light source data
    for ( cgUInt32 i = 0; i < InitData.GetChildNodeCount(); ++i )
    {
        xChild = InitData.GetChildNode( i );

        // What type of node is this?
        if ( xChild.IsOfType( _T("Range") ) )
        {
            cgFloat fValue;
            cgStringParser( xChild.GetText() ) >> fValue;
            SetRange( fValue );

        } // End if Range
        else if ( xChild.IsOfType( _T("Attenuation") ) )
        {
            // Parse the distance attenuation data (if any).
            xAttenShadow = xChild.GetChildNode( _T("Distance") );
            if ( !xAttenShadow.isEmpty() )
                m_DistanceAttenCurve.ParseXML( xAttenShadow );

            // Retrieve the attenuation mask data (if any)
            xAttenShadow     = xChild.GetChildNode( _T("Mask") );
            m_xAttenMaskData = xAttenShadow.GetChildNode( _T("Sampler") );

        } // End if Attenuation
        else if ( xChild.IsOfType( _T("InnerCone") ) )
        {
            cgFloat fValue;
            cgStringParser( xChild.GetText() ) >> fValue;
			setInnerCone( bIsDetail ? fValue - 30.0f : fValue );

        } // End if InnerCone
        else if ( xChild.IsOfType( _T("OuterCone") ) )
        {
            cgFloat fValue;
            cgStringParser( xChild.GetText() ) >> fValue;
            setOuterCone( bIsDetail ? fValue + 20.0f : fValue );

        } // End if OuterCone
        else if ( xChild.IsOfType( _T("Falloff") ) )
        {
            cgFloat fValue;
            cgStringParser( xChild.GetText() ) >> fValue;
            setFalloff( fValue );

        } // End if Falloff
        else if ( xChild.IsOfType( _T("Shadowing") ) )
        {
            // Iterate through all child nodes to get shadowing settings
            for ( cgUInt32 j = 0; j < xChild.GetChildNodeCount(); ++j )
            {
                xAttenShadow = xChild.GetChildNode( j );

                // What type of node is this?
                if ( xAttenShadow.IsOfType( _T("Frustum") ) )
                {
                    if ( !mShadowFrustum->Deserialize( xAttenShadow ) )
                        return false;
                
                } // End if Frustum
                else if ( xAttenShadow.IsOfType( _T("UpdateRate") ) )
                {
                    cgStringParser( xAttenShadow.GetText() ) >> mShadowUpdateRate;

                } // End if UpdateRate
                
            } // Next Child Node

        } // End if Shadowing

    } // Next Child Node

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : InitObject ()
/// <summary>
/// Initialize the object (after deserialization / setup) as required.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightObject::InitObject( bool bAutoAddObject = true )
{
    cgMatrix mtxView, mtxProj;

    // Allow base class to process
    if ( !cgLightObject::InitObject( bAutoAddObject ) )
        return false;

    // Get access to required systems.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    cgRenderDriver    * pDriver    = mParentScene->getRenderDriver();

    // Initialize the frustums as required
    const cgTexturePool::Config & Settings = mParentScene->GetShadowMaps()->GetConfig();

    // Compute the correct shadow frustum camera details.
    cgCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
    pFrustumCamera->SetObjectMatrix( GetObjectMatrix() );
    pFrustumCamera->setFOV( mOuterCone );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 1.0f );
    pFrustumCamera->setFarClip( m_fRange );
    
    // Set frustum properties
    mShadowFrustum->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution );

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    if ( mShadowUpdateRate > 0 )
        mShadowTimeSinceLast = -cgMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)mShadowUpdateRate );

    // Import the attenuation mask texture if supplied.
    if ( !m_xAttenMaskData.isEmpty() )
    {
        if ( (m_pAttenMaskSampler = pDriver->CreateSampler( _T("LightAtten"), m_hEffect ) ) )
        {
            // Parse sampler.
            if ( !m_pAttenMaskSampler->ParseXML( m_xAttenMaskData ) )
            {
                m_pAttenMaskSampler->ScriptSafeDispose();
                m_pAttenMaskSampler = CG_NULL;
            
            } // End if failed
            else
            {
                // Attenuation mask sampler MUST use border mode.
                cgSamplerStates & States = m_pAttenMaskSampler->GetStates();
                States.AddressU = D3DTADDRESS_BORDER;
                States.AddressV = D3DTADDRESS_BORDER;
                States.AddressW = D3DTADDRESS_BORDER;

            } // End if success

        } // End if valid

    } // End if Sampler

    // Create a 1D distance attenuation texture if the attenuation curve does
    // not describe standard linear attenuation.
    if ( !m_DistanceAttenCurve.IsLinear() || m_pAttenMaskSampler )
    {
        // Generate a texture for this curve unless one has already been generated
        cgString strCurveName = _T("Core::Curves::256::") + m_DistanceAttenCurve.ComputeHash( 2 );
        pResources->CreateTexture( &m_hDistanceAttenTex, m_DistanceAttenCurve, 256, 1, true, 0, strCurveName, cgDebugSource() );

        // Create a sampler for distance attenuation and bind the above texture
        if ( m_hDistanceAttenTex.IsValid() )
        {
            if ( (m_pDistanceAttenSampler = pDriver->CreateSampler( _T("LightDistanceAtten"), m_hEffect ) ) )
                m_pDistanceAttenSampler->SetTexture( m_hDistanceAttenTex );

        } // End if valid

    } // End if not linear

    // If we are using stand spot cone falloff, create a unit-size cone for the light shape
    // (if it doesn't already exist) that will allow us to represent the shape of the light source.
    // Otherwise, we need a 4 sided pyramid shape.
    if ( !m_pAttenMaskSampler )
    {
        // Cone shape
        if ( !pResources->getMesh( &mLightShape, _T("Core::LightShapes::UnitCone") ) )
        {
            cgMesh * pMesh = new cgMesh();

            // Construct a unit-sized cone mesh
            pMesh->prepareMesh( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), false, pResources );
            pMesh->CreateCone( 1.0f, 0.0f, 1.0f, 1, 15, true );
            pMesh->endPrepare( );

            // Add to the resource manager
            pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitCone"), cgDebugSource() );

        } // End if no existing mesh

    } // End if no mask
    else
    {
        // Pyramid shape
        if ( !pResources->getMesh( &mLightShape, _T("Core::LightShapes::UnitPyramid") ) )
        {
            cgUInt32  Indices[6];
            cgVector3 Vertices[5];
	        cgMesh  * pMesh = new cgMesh();

            // Set the mesh data format
            cgVertexFormat * pFormat = cgVertexFormat::formatFromFVF( D3DFVF_XYZ );
            pMesh->prepareMesh( pFormat, false, pResources );

	        // Pyramid vertices
            Vertices[0] = cgVector3(  0.0f,  0.0f,  0.0f );
            Vertices[1] = cgVector3( -1.0f, -1.0f,  1.0f );
            Vertices[2] = cgVector3(  1.0f, -1.0f,  1.0f );
            Vertices[3] = cgVector3(  1.0f, -1.0f, -1.0f );
            Vertices[4] = cgVector3( -1.0f, -1.0f, -1.0f );
	        pMesh->setVertexSource( Vertices, 5, pFormat );

            // +X indices
            Indices[0]  = 0; Indices[1]  = 2; Indices[2]  = 3;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, ResourceHandle(), 0 );

            // -X indices
            Indices[0]  = 0; Indices[1]  = 4; Indices[2]  = 1;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, ResourceHandle(), 0 );

            // +Z indices
            Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, ResourceHandle(), 0 );

            // -Z indices
            Indices[0]  = 0; Indices[1]  = 3; Indices[2]  = 4;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, ResourceHandle(), 0 );

            // -Y (base) indices
            Indices[0]  = 4; Indices[1]  = 2; Indices[2]  = 1;
            Indices[3]  = 4; Indices[4]  = 3; Indices[5]  = 2;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

	        // Build the mesh
            pMesh->endPrepare( );
            
            // Add to the resource manager
            pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitPyramid"), cgDebugSource() );

        } // End if no mesh

    } // End if using mask
    
    // Success
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : getLightType () (Virtual)
/// <summary>
/// Determine the type of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::LightType cgSpotLightObject::getLightType( ) const
{
    return Light_Spot;
}

///////////////////////////////////////////////////////////////////////////////
// cgSpotLightNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpotLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightNode::cgSpotLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgLightNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults
    mShadowFrustum            = new cgShadowGenerator( this, 0 );
    mIndirectFrustum          = new cgReflectanceGenerator( this, 0 );
    mShadowTimeSinceLast      = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgSpotLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightNode::cgSpotLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    mShadowFrustum            = new cgShadowGenerator( this, 0 );
    mIndirectFrustum          = new cgReflectanceGenerator( this, 0 );
    mShadowTimeSinceLast      = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpotLightNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpotLightNode::~cgSpotLightNode()
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
void cgSpotLightNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    if ( mShadowFrustum )
        mShadowFrustum->scriptSafeDispose();
    if ( mIndirectFrustum )
        mIndirectFrustum->scriptSafeDispose();
    
    // Clear variables
    mShadowFrustum = CG_NULL;
    mIndirectFrustum = CG_NULL;

    // Release resources
    mSpotLightConstants.close();
    
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
bool cgSpotLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SpotLightNode )
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
cgObjectNode * cgSpotLightNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgSpotLightNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgSpotLightNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgSpotLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::postCreate( )
{
    // Call base class implementation.
    if ( !cgLightNode::postCreate( ) )
        return false;

    // Retrieve the required properties from the referenced light object.
    cgFloat fRange     = getOuterRange();
    cgFloat fOuterCone = getOuterCone();
    
    // Always force at least a small range to prevent invalid
    // light shape matrix from being produced.
    if ( fRange < CGE_EPSILON_1MM )
        fRange = CGE_EPSILON_1MM;

    // Recompute projection matrix and frustum.
    cgMatrix::perspectiveFovLH( mProjectionMatrix, CGEToRadian(fOuterCone), 1.0f, 1.0f, fRange );
    updateFrustum();

    // Recompute the light shape's base transformation matrix. First compute 
    // the correct size for the end cap of the cone / pyramid (this is unit sized).
    cgFloat fBaseSize = tanf(CGEToRadian(fOuterCone)*0.5f) * fRange;

    // Compute correct scaling matrix for the geometry
    cgTransform::scaling( mLightShapeTransform, fBaseSize, fRange, fBaseSize );

    // Light is aligned to the Z axis, but cone is aligned to Y axis. Rotate.
    mLightShapeTransform.rotate( CGEToRadian(-90.0f), 0, 0 );
    
    // Initialize the shadow frustum as required
    if ( !mShadowFrustum->initialize() )
        return false;

    // Is this a dynamic ambient lighting source? (i.e., will create RSMs?)
    // ToDo: 6767 - Reintroduce?
    //if ( !mIndirectFrustum->initialize() )
        //return false;

    // Compute the correct shadow frustum camera details.
    cgCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
    pFrustumCamera->setWorldTransform( getWorldTransform( false ) );
    pFrustumCamera->setFOV( fOuterCone );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 1.0f );
    pFrustumCamera->setFarClip( fRange );
    
    // Compute the correct indirect frustum camera details.
    // ToDo: 6767 - Reintroduce?
    /*pFrustumCamera = mIndirectFrustum->getCamera();
    pFrustumCamera->setWorldTransform( getWorldTransform( false ) );
    pFrustumCamera->setFOV( fOuterCone );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 1.0f );
    pFrustumCamera->setFarClip( fRange );*/

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    if ( getShadowUpdateRate() > 0 )
        mShadowTimeSinceLast = -cgMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)getShadowUpdateRate() );

    // Create constant buffers
    cgResourceManager * pResources = mParentScene->getResourceManager();
    if ( !pResources->createConstantBuffer( &mSpotLightConstants, mShader, _T("_cbSpotLight"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required spot light source constant buffers for object node 0x%x in scene '%s'.\n"), mReferenceId, mParentScene->getName().c_str() );
        return false;
    
    } // End if failed
    cgAssert( mSpotLightConstants->getDesc().length == sizeof(_cbSpotLight ) );

    // Create the required light shape.
    return createLightShape();
}

//-----------------------------------------------------------------------------
//  Name : updateFrustum() (Protected)
/// <summary>
/// Update the frustum based on current light settings such as
/// FoV angles, range etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::updateFrustum()
{
    // Compute frustum from current view/projection combo.
    mFrustum.update( mViewMatrix, mProjectionMatrix );

    // Shift the near plane so that it sits at the light source origin
    // (the truest representation of our light shape volume).
    cgPlane & NearPlane = mFrustum.planes[cgVolumePlane::Near];
    NearPlane.d = -cgVector3::dot( (cgVector3&)NearPlane, mFrustum.position );

    // Shift the near plane points back too.
    mFrustum.points[cgVolumeGeometry::LeftTopNear] = mFrustum.position;
    mFrustum.points[cgVolumeGeometry::RightTopNear] = mFrustum.position;
    mFrustum.points[cgVolumeGeometry::RightBottomNear] = mFrustum.position;
    mFrustum.points[cgVolumeGeometry::LeftBottomNear] = mFrustum.position;
}

//-----------------------------------------------------------------------------
// Name : createLightShape () (Protected)
/// <summary>
/// Create the light shape mesh that is applicable to this spot light.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::createLightShape( )
{
    // Get access to required systems.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    cgRenderDriver    * pDriver    = mParentScene->getRenderDriver();

    /*// If we are using stand spot cone falloff, create a unit-size cone for the light shape
    // (if it doesn't already exist) that will allow us to represent the shape of the light source.
    // Otherwise, we need a 4 sided pyramid shape.
    cgSampler * pAttenMaskSampler = GetAttenMaskSampler();
    
    // Note: Spot lights now always use a frustum shape (unit pyramid) to represent
    // their light shape as geometry. This simplifies all manner things. The prior
    // code remains commented below for now however, in case we want to reverse this decision.
    if ( !pAttenMaskSampler || !pAttenMaskSampler->IsTextureValid() )
    {
        // Cone shape
        if ( !pResources->getMesh( &mLightShape, _T("Core::LightShapes::UnitCone") ) )
        {
            cgMesh * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

            // Construct a unit-sized cone mesh
            pMesh->CreateCone( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 1.0f, 0.0f, 1.0f, 1, 15, false, cgMeshCreateOrigin::Top, true, pResources );
            
            // Add to the resource manager
            pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitCone"), cgDebugSource() );

        } // End if no existing mesh

    } // End if no mask
    else
    {*/
        // Pyramid shape
        if ( !pResources->getMesh( &mLightShape, _T("Core::LightShapes::UnitPyramid") ) )
        {
            cgUInt32  Indices[6];
            cgVector3 Vertices[5];
	        cgMesh  * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

            // Set the mesh data format
            cgVertexFormat * pFormat = cgVertexFormat::formatFromFVF( D3DFVF_XYZ );
            pMesh->prepareMesh( pFormat, false, pResources );

	        // Pyramid vertices
            Vertices[0] = cgVector3(  0.0f,  0.0f,  0.0f );
            Vertices[1] = cgVector3( -1.0f, -1.0f,  1.0f );
            Vertices[2] = cgVector3(  1.0f, -1.0f,  1.0f );
            Vertices[3] = cgVector3(  1.0f, -1.0f, -1.0f );
            Vertices[4] = cgVector3( -1.0f, -1.0f, -1.0f );
	        pMesh->setVertexSource( Vertices, 5, pFormat );

            // +X indices
            Indices[0]  = 0; Indices[1]  = 2; Indices[2]  = 3;
            pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, cgMaterialHandle::Null, 0 );

            // -X indices
            Indices[0]  = 0; Indices[1]  = 4; Indices[2]  = 1;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, cgMaterialHandle::Null, 0 );

            // +Z indices
            Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, cgMaterialHandle::Null, 0 );

            // -Z indices
            Indices[0]  = 0; Indices[1]  = 3; Indices[2]  = 4;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 3, cgMaterialHandle::Null, 0 );

            // -Y (base) indices
            Indices[0]  = 4; Indices[1]  = 2; Indices[2]  = 1;
            Indices[3]  = 4; Indices[4]  = 3; Indices[5]  = 2;
	        pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

	        // Build the mesh
            pMesh->endPrepare( );
            
            // Add to the resource manager
            pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitPyramid"), cgDebugSource() );

        } // End if no mesh

    //} // End if using mask

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getFrustum ()
/// <summary>
/// Retrieve the frustum for this light source.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgSpotLightNode::getFrustum( ) const
{
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can updatate our own properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgLightNode::setCellTransform( Transform, Source ) )
        return false;

    // If this node has a target node associated with it, update 
    // the spot light's range to match the distance between the two.
    if ( mTargetNode )
        setOuterRange( cgVector3::length( mTargetNode->getPosition( ) - getPosition( ) ) );

    // Recompute frustum.
    updateFrustum();
    
    // Update shadow frustum camera
    if ( mShadowFrustum )
        mShadowFrustum->getCamera()->setWorldTransform( getWorldTransform( false ) );

    // Update indirect frustum camera
    if ( mIndirectFrustum )
        mIndirectFrustum->getCamera()->setWorldTransform( getWorldTransform( false ) );

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
void cgSpotLightNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What property was modified?
    if ( e->context == _T("OuterRange") || e->context == _T("OuterCone") || e->context == _T("ApplyRescale") )
    {
        // Retrieve the required properties from the referenced light object.
        cgFloat fRange     = getOuterRange();
        cgFloat fOuterCone = getOuterCone();
        
        // Always force at least a small range to prevent invalid
        // light shape matrix from being produced.
        if ( fRange < CGE_EPSILON_1MM )
            fRange = CGE_EPSILON_1MM;

        // Recompute projection matrix and frustum.
        cgMatrix::perspectiveFovLH( mProjectionMatrix, CGEToRadian(fOuterCone), 1.0f, 1.0f, fRange );
        updateFrustum();

        // Recompute the light shape's base transformation matrix. First compute 
        // the correct size for the end cap of the cone / pyramid (it is unit sized).
        cgFloat fBaseSize = tanf(CGEToRadian(fOuterCone)*0.5f) * fRange;

        // Compute correct scaling matrix for the geometry
        cgTransform::scaling( mLightShapeTransform, fBaseSize, fRange, fBaseSize );

        // Light is aligned to the Z axis, but shape is aligned to Y axis. Rotate.
        mLightShapeTransform.rotate( CGEToRadian(-90.0f), 0, 0 );
        
        // Update shadow frustum camera's far clip plane and 
        // FOV of to match range and outer cone of light source.
        cgCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
        pFrustumCamera->setFarClip( fRange );
        pFrustumCamera->setFOV( fOuterCone );

        // And the same for the indirect frustum camera.
        pFrustumCamera = mIndirectFrustum->getCamera();
        pFrustumCamera->setFarClip( fRange );
        pFrustumCamera->setFOV( fOuterCone );

        // Light source node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

        // If in target mode (and we're not already updating in response
        // to a target node update), push target to correct distance based
        // on the spot light's altered range.
        if ( mTargetNode && !mTargetNode->isUpdating() )
            mTargetNode->setPosition( getPosition() + (mTargetNode->getZAxis( ) * fRange) );
    
    } // End if OuterRange | OuterCone | ApplyRescale
    else if ( e->context == _T("AttenMaskSampler") )
    {
        // Regenerate our light shape mesh. Uses cone if no
        // mask is supplied, or pyramid if it is.
        cgToDo( "Carbon General", "Is this even necessary any more now that spot lights always use frustum shape?" );
        createLightShape();

    } // End if AttenMaskSampler

    // Call base class implementation last
    cgLightNode::onComponentModified( e );
    
}

//-----------------------------------------------------------------------------
//  Name : boundsInVolume () (Virtual)
/// <summary>
/// Does the specified bounding box fall within the volume of this light 
/// source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::boundsInVolume( const cgBoundingBox & AABB )
{
    return mFrustum.testAABB( AABB );
}

//-----------------------------------------------------------------------------
//  Name : pointInVolume () (Virtual)
/// <summary>
/// Does the specified point fall within the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::pointInVolume( const cgVector3 & Point, cgFloat fErrRadius /* = 0.0f */ )
{
	if ( fErrRadius > 0.0f )
		return mFrustum.testSphere( Point, fErrRadius );
	else
		return mFrustum.testPoint( Point );
}

//-----------------------------------------------------------------------------
//  Name : frustumInVolume () (Virtual)
/// <summary>
/// Does the specified frustum intersect the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::frustumInVolume( const cgFrustum & Frustum )
{
    return Frustum.testFrustum( mFrustum );
}

//-----------------------------------------------------------------------------
//  Name : testObjectShadowVolume( ) (Virtual)
/// <summary>
/// Determine if the specified object can potentially cast a shadow into
/// the view frustum based on the requirements of this light source.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::testObjectShadowVolume( cgObjectNode * pObject, const cgFrustum & ViewFrustum )
{
    cgBoundingBox ObjectAABB = pObject->getBoundingBox();

    // If the light source falls inside the bounding box of the object, 
    // this can automatically cast shadows.
    if ( ObjectAABB.containsPoint( getPosition( false ) ) )
        return true;

    // Compute an extruded bounding box for the object. This extrudes the box edges
    // away from the light source position (just like extruding an actual mesh shadow
    // volume when implementing stencil shadows).
    // ToDo: 9999 - Does this work property for spot lights?
    cgExtrudedBoundingBox ExtrudedAABB( ObjectAABB, getPosition( false ), getOuterRange()  * 1.73205f /* sqrt((1*1)+(1*1)+(1*1)) */ );

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
void cgSpotLightNode::computeShadowSets( cgCameraNode * pCamera )
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
        // Allow the frustum to compute its own local shadow set information.
        // This will return 'true' if it is deemed necessary to compute shadows
        // or at least render from a pool shadow map.
        if ( !mShadowFrustum->computeVisibilitySet( pCamera ) )
            mComputeShadows = false;

    } // End if shadow caster
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail () (Virtual)
/// <summary>
/// Computes level of detail settings for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
{
    // Allow base class to compute default LOD settings.
    cgLightNode::computeLevelOfDetail( pCamera );

    // If shadows are enabled
    if ( mComputeShadows )
    {
        cgTexturePoolResourceDesc::Array aDescriptions;

        // Ask the lighting manager for the best shadow technique given LOD
        cgLightingManager * pManager = getScene()->getLightingManager();
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
            mShadowFrustum->update( nResolution, mShadowSystemSettings, LightSettings, aDescriptions, mShadowFlags );

        } // End valid settings 
        else
        {
            // We could not get valid shadow settings, so turn off shadows
            mComputeShadows = false;
        
        } // End if no settings

    } // End shadows
}

//-----------------------------------------------------------------------------
//  Name : updateIndirectSettings () (Virtual)
/// <summary>
/// Updates settings for indirect lighting
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::updateIndirectSettings( )
{
    cgTexturePoolResourceDesc::Array aDescriptions;

    // Ask the lighting manager for the best reflective shadow technique given LOD
    cgLightingManager * pManager = getScene()->getLightingManager();
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
        mIndirectFrustum->update( nResolution, mIndirectSystemSettings, LightSettings, aDescriptions, mIndirectFlags );

    } // End valid settings 
    else
    {
        // Turn off indirect lighting
        mComputeIndirect = false;
    
    } // End if no settings
}

//-----------------------------------------------------------------------------
//  Name : setClipPlanes () (Virtual)
/// <summary>
/// Setup the clipping planes for this light source
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::setClipPlanes( )
{
    // Get access to required systems / objects
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();

	// Set the user-defined clip planes for the frustum
    pDriver->setUserClipPlanes( mFrustum.planes, 6 );

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
void cgSpotLightNode::computeShadowSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::computeShadowSets( pCamera );

    // If it was determined that we should calculate shadows,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeShadows )
    {
        bool bCalcShadows = false;

        // Process the shadow frustum.
        if ( mShadowFrustum && mShadowFrustum->IsEnabled() )
        {
            // Allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            cgLightObject * pLight = (cgLightObject*)m_pReferencedObject;
            bCalcShadows |= mShadowFrustum->computeShadowSets( pCamera );

        } // Next Frustum

        // Is there /still/ any need to assume that this is a shadow source?
        if ( !bCalcShadows )
            mComputeShadows = false;

    } // End if mComputeShadows
}*/

// ToDo: 6767 - For all light types, roll ComputeIndirectSets into compute visibility like we did for computeShadowSets
/*//-----------------------------------------------------------------------------
//  Name : ComputeIndirectSets( ) (Virtual)
/// <summary>
/// Creates the rsm visibility sets
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::ComputeIndirectSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::ComputeIndirectSets( pCamera );

    // If it was determined that we should calculate rsms,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeIndirect )
    {
        // Allow the frustum to compute its own local shadow set information.
        // This will return 'true' if it is deemed necessary to compute shadows
        // or at least render from a pool shadow map.
        if ( !mIndirectFrustum->computeVisibilitySet( pCamera ) )
            mComputeIndirect = false;

    } // End if mComputeIndirect
}*/

//-----------------------------------------------------------------------------
// Name : reassignShadowMaps() (Virtual)
/// <summary>
/// Allows the light to determine if it is necessary to partake in the 
/// assignment of new shadow maps at this stage, or if it can re-use shadow 
/// maps that it may have populated earlier.
/// Note : Returning true indicates that the light does not require further
/// shadow map assignment processing (i.e. it reused pooled maps).
///
/// Note: At this stage the generator needs to understand the requirements that the
///       frustum has (e.g., shadow method, resolution, filter size, etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::reassignShadowMaps( cgTexturePool * pPool )
{
    // Ignore the frustum if we are not a shadow source, or have
    // no visible objects to process.
    if ( !isShadowSource() || !mShadowFrustum->containsRenderableObjects() ) 
    {
        // Release shadow resources back to the pool.
        mShadowFrustum->releaseResources();
        return false;

    } // End if not a caster

    // Attempt resource reassignment
    return mShadowFrustum->reassignResources( pPool );
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
bool cgSpotLightNode::reassignIndirectMaps( cgTexturePool * pPool )
{
    // Ignore the frustum if we are not a shadow source, are disabled or have
    // no visible objects to process.
    if ( !isIndirectSource() || !mIndirectFrustum->containsRenderableObjects() ) 
    {
        // Release shadow resources back to the pool.
        mIndirectFrustum->releaseResources();
        return true;

    } // End if not a caster

    // Attempt resource reassignment
    return mIndirectFrustum->reassignResources( pPool );
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
cgInt32 cgSpotLightNode::beginShadowFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginShadowFill( pPool ) < 0 )
        return -1;

    // ToDo: 6767 - Do nothing if the frustum has nothing to draw
    // (See Generator::containsRenderableObjects()?)
    /*if ( mShadowFrustum->getVisibilitySet()->isEmpty() )
    {
        mShadowFrustum->releaseResources();
        cgLightNode::endShadowFill();
        return -1;
    }*/
    
    // ToDo: 6767 - Determine if it is an appropriate time for us to update based on
    // our shadow map update period limiting property.
    /*cgUInt32 nShadowUpdateRate = getShadowUpdateRate();
    if ( nShadowUpdateRate > 0 )
    {
        mShadowTimeSinceLast += cgTimer::getInstance()->GetTimeElapsed();
        if ( mShadowTimeSinceLast < (1.0f / (cgFloat)nShadowUpdateRate) )
            m_bShadowTimedUpdate = false;
        else
            mShadowTimeSinceLast = 0.0f;

    } // End if limit updates*/

    // Attempt to find resources for the generator.
    cgUInt32 nFillStatus = mShadowFrustum->assignResources( pPool );

    // If there was a failure getting resources, bail
    if ( nFillStatus & cgShadowGeneratorFillResult::DoNothing )
    {
        cgLightNode::endShadowFill();
        return -1;
    
    } // End if failed

    // We cannot fill now if we were assigned one or more default resource types
    if( nFillStatus & cgShadowGeneratorFillResult::CannotFill )
    {
        cgLightNode::endShadowFill();
        return -1;
    
    } // End if nothing to do

    // We can fill if we got previously assigned resources, but don't necessarily 
    // have to if the frustum doesn't require it.
    if( (nFillStatus & cgShadowGeneratorFillResult::CanFill) && !mShadowFrustum->shouldRegenerate() )
    {
        cgLightNode::endShadowFill();
        return -1;
    
    } // End if nothing to do

    // How many passes are required to fill the assigned resources?
    cgUInt32 nPassCount = mShadowFrustum->getWritePassCount();

    // If there is literally nothing to do, return -1 (don't even process).
    // ToDo: 6767 - Check that the caller even cares if this is -1.
    if ( !nPassCount )
    {
        cgLightNode::endShadowFill();
        return -1;
    
    } // End if nothing to do

    // Add new passes to list
    mShadowPasses.reserve( nPassCount );
    for ( cgUInt32 i = 0; i < nPassCount; ++i )
        mShadowPasses.push_back( LightingOpPass( Lighting_FillShadowMap, 0, i ) );

    // Bind the custom spot lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mSpotLightConstants );

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
bool cgSpotLightNode::beginShadowFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation?
    if ( !cgLightNode::beginShadowFillPass( nPass, pRenderSetOut ) )
        return false;

    // If this is the first child pass, begin writing.
    if ( mShadowPasses[ nPass ].subPass == 0 )
        mShadowFrustum->beginWrite();

    // Execute the current write pass.
    if ( mShadowFrustum->beginWritePass( nPass ) )
    {
        // Render only objects that exist within frustum's shadow set.
        pRenderSetOut = mShadowFrustum->getVisibilitySet();

        // We have begun this pass
        mCurrentShadowPass = nPass;
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
bool cgSpotLightNode::endShadowFillPass( )
{
    cgInt32 nCurrentPass = mCurrentShadowPass;

    // Valid end operation?
    if ( !cgLightNode::endShadowFillPass() )
        return false;

    // Finish this fill pass
    mShadowFrustum->endWritePass();

    // If this is the last child pass, end writing.
    if ( mShadowPasses[ nCurrentPass ].subPass == (mShadowFrustum->getWritePassCount() - 1) )
        mShadowFrustum->endWrite();

    // Valid
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFill() (Virtual)
/// <summary>
/// Called in order to signify that the shadow fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::endShadowFill( )
{
    // Valid end operation?
    if ( !cgLightNode::endShadowFill( ) )
        return false;

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : updateLightConstants() (Virtual)
/// <summary>
/// Update the lighting base terms constant buffer as necessary whenever any
/// of the appropriate properties change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::updateLightConstants()
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

    // Populate custom spot light terms
    pLightBuffer = mSpotLightConstants.getResource( true );
    cgAssert( pLightBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbSpotLight * pSpotLightData = CG_NULL;
    if ( !(pSpotLightData = (_cbSpotLight*)pLightBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock spot light data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed
    
    // Ensure that selected inner cone is never degenerate.
    // This can introduce a divide by zero in the shader.
    cgFloat fInnerCone = getInnerCone();
    cgFloat fOuterCone = getOuterCone();
	if ( fInnerCone == fOuterCone )
		fInnerCone = max( 0.0f, fInnerCone - CGE_EPSILON_1MM );
	
    // Compute spot light terms
    cgFloat cosTheta      = cosf( CGEToRadian(fInnerCone) * 0.5f );
	cgFloat cosPhi        = cosf( CGEToRadian(fOuterCone) * 0.5f );
	cgFloat recipDelta    = 1.0f / (cosPhi - cosTheta);
	pSpotLightData->spotTerms.x = -recipDelta;
	pSpotLightData->spotTerms.y = cosPhi * recipDelta;
	pSpotLightData->spotTerms.z = getFalloff();
	pSpotLightData->spotTerms.w = 0.0f;
    
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
cgInt32 cgSpotLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
{
    // Override shadow application settings just in case.
    if ( !isShadowSource() )
        bApplyShadows = false;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginLighting( pPool, bApplyShadows, bDeferred ) < 0 )
        return -1;

    // Bind the custom spot lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mSpotLightConstants );

    // If we are applying shadows, we require 2 (or more) passes -- the first set of
    // passes fill default shadow map resources prior to lighting, and the second set 
    // performs the actual lighting process.
    if ( bApplyShadows && mShadowFrustum->requiresDefaultResource() )
    {
        // How many passes will it take to populate our shadow map(s)?
        cgUInt32 nPassCount = mShadowFrustum->getWritePassCount();
        mLightingPasses.reserve( nPassCount + 1 );
        for ( cgUInt32 i = 0; i < nPassCount; ++i )
            mLightingPasses.push_back( LightingOpPass( Lighting_FillShadowMap, 0, i ) );
    
    } // End if default fill
            
    // One other pass required for actual lighting.
    mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, 0, 0 ) );
    
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
cgLightNode::LightingOp cgSpotLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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
        if ( Pass.subPass == 0 )
            mShadowFrustum->beginWrite();

        // Begin the shadow pass
        if ( mShadowFrustum->beginWritePass( Pass.subPass ) )
        {
            // Render using the frustum's requested visibility set.
            pRenderSetOut = mShadowFrustum->getVisibilitySet();
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
        // Prevent final draw unless we're absolutely certain.
        Pass.op = Lighting_None;

        // Deferred or forward lighting?
        if ( mLightingDeferred )
        {
            cgRenderDriver * pDriver = mParentScene->getRenderDriver();
            cgCameraNode   * pCamera = pDriver->getCamera();

            // Test the currently active camera against this light's volume in
            // order to determine if we can apply certain optimizations.
            // If the camera falls inside the lighting volume (pyramid in this case)
            // then it is impossible for it to ever see anything but the inside faces.
            cgVolumeQuery::Class VolumeStatus = cgVolumeQuery::Inside;
            if ( !pointInVolume( pCamera->getPosition( false ), 0.0f ) )
            {
                // If it does not fall inside, then this is the much more complex case.
                // First retrieve the frustum that represents the space between the
                // camera position and its near plane. This frustum represents the
                // 'volume' that can end up clipping pieces of the shape geometry.
                const cgFrustum & ClippingVolume = pCamera->getClippingVolume();

                // Test to see if this clipping volume intersects the spot
                // light's main frustum (represents the light shape).
                if ( ClippingVolume.testFrustum( mFrustum ) )
                    VolumeStatus = cgVolumeQuery::Intersect;
                else
                    VolumeStatus = cgVolumeQuery::Outside;

            } // End if !inside

            // Simply render relevant area of the screen using shape drawing.
            renderDeferred( mLightShapeTransform * getWorldTransform( false ), VolumeStatus, (mLightingApplyShadows) ? mShadowFrustum : CG_NULL );

        } // End if deferred
        else
        {
            // Begin the forward rendering process (caller should render using the light's illumination set
            if ( beginRenderForward( mLightShapeTransform * getWorldTransform( false ), (mLightingApplyShadows) ? mShadowFrustum : CG_NULL ) )
            {
                // ToDo: 6767 -- If we have a shadow generator, we can use its visibility set as an illumination set.
                pRenderSetOut = CG_NULL;
                Pass.op = Lighting_ProcessLight;
            
            } // End if success

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
bool cgSpotLightNode::endLightingPass( )
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
        mShadowFrustum->endWritePass();

        // On the last shadow pass, end writing
        if ( Pass.subPass == (mShadowFrustum->getWritePassCount() - 1) )
            mShadowFrustum->endWrite();
        
    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        // Complete any forward rendering process.
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
bool cgSpotLightNode::endLighting( )
{
    // Valid end operation?
    if ( !cgLightNode::endLighting( ) )
        return false;

    // Valid.
    return true;
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
cgInt32 cgSpotLightNode::beginIndirectFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectFill( pPool ) < 0 )
        return -1;

    // ToDo: 6767 - Can use the specialized method for this?
    // Do nothing if frustum has nothing to draw
    if ( mIndirectFrustum->getVisibilitySet()->isEmpty() )
        return 0;

    // ToDo: 6767 - Determine if it is an appropriate time for us to update based on
    // our update period limiting property.

    // Attempt to find resources for the generator.
    cgUInt32 nFillStatus = mIndirectFrustum->assignResources( pPool );

    // If there was a failure getting resources, bail
    if ( nFillStatus & cgShadowGeneratorFillResult::DoNothing )
        return -1;

    // We cannot fill now if we were assigned one or more default resource types
    if( nFillStatus & cgShadowGeneratorFillResult::CannotFill )
        return 0;

    // We can fill if we got previously assigned resources, but don't necessarily 
    // have to if the frustum doesn't require it.
    if( (nFillStatus & cgShadowGeneratorFillResult::CanFill) && !mIndirectFrustum->shouldRegenerate() )
        return 0;

    // Bind the custom spot lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mSpotLightConstants );

    // How many passes are required to fill the assigned resources?
    cgUInt32 nPassCount = mIndirectFrustum->getWritePassCount();

    // Add new passes to list
    mShadowPasses.reserve( nPassCount );
    for ( cgUInt32 i = 0; i < nPassCount; ++i )
        mShadowPasses.push_back( LightingOpPass( Lighting_FillShadowMap, 0, i ) );

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
bool cgSpotLightNode::beginIndirectFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation?
    if ( !cgLightNode::beginIndirectFillPass( nPass, pRenderSetOut ) )
        return false;

    // If this is the first child pass, begin writing.
    if ( mShadowPasses[ nPass ].subPass == 0 )
        mIndirectFrustum->beginWrite();

    // Begin writing for this pass.
    if ( mIndirectFrustum->beginWritePass( nPass ) )
    {
        // Render only objects that exist within frustum's visibility set.
        pRenderSetOut = mIndirectFrustum->getVisibilitySet();

        // We have begun this pass.
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
bool cgSpotLightNode::endIndirectFillPass( )
{
    cgInt32 nCurrentPass = mCurrentShadowPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectFillPass() )
        return false;

    // Finish this fill pass
    mIndirectFrustum->endWritePass();    

    // If this is the last child pass, end writing.
    if ( mShadowPasses[ nCurrentPass ].subPass == (mIndirectFrustum->getWritePassCount() - 1) )
        mIndirectFrustum->endWrite();

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectFill() (Virtual)
/// <summary>
/// Called in order to signify that the Indirect rsm fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpotLightNode::endIndirectFill( )
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
cgInt32 cgSpotLightNode::beginIndirectLighting( cgTexturePool * pPool )
{
    // ToDo: 6767 -- can we do this in the base class? I doubt it, but worth a shot.
    // Must be an indirect source for RSM to work
    if ( !isIndirectSource() )
        return -1;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectLighting( pPool ) < 0 )
        return -1;

    // Bind the custom spot lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mSpotLightConstants );

    // We need to fill a reflective shadow map (if default resources used).
    if ( mIndirectFrustum->requiresDefaultResource() )
    {
        // How many passes will it take to populate our resources?
        cgUInt32 nPassCount = mIndirectFrustum->getWritePassCount();
        mLightingPasses.reserve( nPassCount + 1 );
        for ( cgUInt32 i = 0; i < nPassCount; ++i )
            mLightingPasses.push_back( LightingOpPass( Lighting_FillShadowMap, 0, i ) );
    
    } // End if default fill

    // One other pass required for actual lighting (rsm based).
    mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, 0, 0 ) );

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
cgLightNode::LightingOp cgSpotLightNode::beginIndirectLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( cgLightNode::beginIndirectLightingPass( nPass, pRenderSetOut ) == Lighting_Abort )
        return Lighting_Abort;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Filling (default) resources
        // On the first child pass, begin writing
        if ( Pass.subPass == 0 )
            mIndirectFrustum->beginWrite();

        // Begin the write pass
        if ( mIndirectFrustum->beginWritePass( Pass.subPass ) )
        {
            // Render using the frustum's requested visibility set.
            pRenderSetOut = mIndirectFrustum->getVisibilitySet();
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
        mIndirectFrustum->beginRead();

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
bool cgSpotLightNode::endIndirectLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectLightingPass() )
        return false;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Complete the fill for this pass.
        mIndirectFrustum->endWritePass();

        // On the last child pass, end writing.
        if ( Pass.subPass == (mIndirectFrustum->getWritePassCount() - 1) )
            mIndirectFrustum->endWrite();

    } // End if FillShadowMap
    else if ( Pass.op == Lighting_ProcessLight )
    {
        mIndirectFrustum->endRead();

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
bool cgSpotLightNode::endIndirectLighting( )
{
    // Valid end operation?
    if ( !cgLightNode::endIndirectLighting( ) )
        return false;

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setIndirectLightingMethod() (Virtual)
/// <summary>
/// Sets the current indirect lighting method for the generator to use
/// </summary>
//-----------------------------------------------------------------------------
void cgSpotLightNode::setIndirectLightingMethod( cgUInt32 nMethod )
{
    mIndirectFrustum->setMethod( nMethod );
}
