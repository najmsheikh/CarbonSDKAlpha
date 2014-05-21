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
// Name : cgParticleEmitterObject.cpp                                        //
//                                                                           //
// Desc : Classes responsible for creating, managing and rendering particles //
//        used for scene special effects.                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgParticleEmitterObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgParticleEmitterObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgTargetObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Resources/cgSurfaceShader.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgParticleEmitterObject::mInsertEmitter;
cgWorldQuery cgParticleEmitterObject::mInsertEmitterLayer;
cgWorldQuery cgParticleEmitterObject::mDeleteEmitterLayer;
cgWorldQuery cgParticleEmitterObject::mUpdateConeAngles;
cgWorldQuery cgParticleEmitterObject::mUpdateEmissionRadii;
cgWorldQuery cgParticleEmitterObject::mUpdateParticleCounts;
cgWorldQuery cgParticleEmitterObject::mUpdateReleaseProperties;
cgWorldQuery cgParticleEmitterObject::mUpdateRenderingProperties;
cgWorldQuery cgParticleEmitterObject::mUpdateParticleProperties;
cgWorldQuery cgParticleEmitterObject::mUpdateColorKeys;
cgWorldQuery cgParticleEmitterObject::mUpdateScaleKeys;
cgWorldQuery cgParticleEmitterObject::mLoadEmitter;
cgWorldQuery cgParticleEmitterObject::mLoadEmitterLayers;

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterObject::cgParticleEmitterObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
}

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterObject::cgParticleEmitterObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgParticleEmitterObject * pObject = (cgParticleEmitterObject*)pInit;
    mLayers = pObject->mLayers;
}

//-----------------------------------------------------------------------------
// Name : ~cgParticleEmitterObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterObject::~cgParticleEmitterObject()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clean up layers
    mLayers.clear();

    // Call base class implementation
    if ( bDisposeBase == true )
        cgWorldObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgParticleEmitterObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgParticleEmitterObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgParticleEmitterObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgParticleEmitterObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgParticleEmitterObject::getLocalBoundingBox( )
{
    cgToDo( "Particles", "Compute maximum bounding box? Also override node's method to get ACTUAL bounding box." );
    return cgBoundingBox( -500, -500, -500, 500, 500, 500 );
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, cgUInt32 nFlags, cgFloat fWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    // Retrieve useful values
    float fZoomFactor  = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition( false ), 2.5f );
    float fTolerance   = 2.0f * fZoomFactor;
    
    // Compute vertices for the tip and base of the cone
    cgVector3 Points[10];
    const cgParticleEmitterProperties & properties = getLayerProperties( 0 );
    float fSize   = fZoomFactor * 25.0f * cosf(CGEToRadian(properties.outerCone* 0.5f));
    float fRadius = tanf(CGEToRadian(std::max<cgFloat>(0.5f,properties.outerCone) * 0.5f)) * fSize;
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
void cgParticleEmitterObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

    // Must have at least one layer.
    if ( !getLayerCount() )
        return;

    // Configuration
    cgFloat fRange = 2.0f;

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
    const cgParticleEmitterProperties & properties = getLayerProperties( 0 );
    cgFloat fSize      = fZoomFactor * 25.0f * cosf(CGEToRadian(properties.outerCone * 0.5f));
    cgFloat fRadius    = tanf(CGEToRadian(properties.outerCone * 0.5f)) * fSize;
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

            // If the emitter (or target) is selected show the inner/outer cones too
            // unless this is part of a selected group.
            cgTargetNode * pTarget = pIssuer->getTargetNode();
            if ( (bSelected || (pTarget && pTarget->isSelected())) && !pIssuer->isMergedAsGroup() )
            {
                // Inner cone first
                fRadius = tanf(CGEToRadian(properties.innerCone * 0.5f)) * fRange;
                nColor  = 0xFF99CCE5;
                
                // Generate line strip circle 
                for ( cgInt i = 0; i < 28; ++i )
                {
                    // Build vertex
                    Points[i].position.x = (sinf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                    Points[i].position.y = (cosf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                    Points[i].position.z = fRange;
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
                if ( properties.outerCone > properties.innerCone )
                {
                    fRadius = tanf(CGEToRadian(properties.outerCone * 0.5f)) * fRange;
                    nColor  = 0xFF4C6672;

                    // Generate line strip circle 
                    for ( cgInt i = 0; i < 28; ++i )
                    {
                        // Build vertex
                        Points[i].position.x = (sinf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                        Points[i].position.y = (cosf( (CGE_TWO_PI / 28.0f) * (float)i ) * fRadius);
                        Points[i].position.z = fRange;
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
    cgWorldObject::sandboxRender( flags, pCamera, pVisData, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::isRenderable() const
{
	// All emitters can render by default.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render the emitter object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::render( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer )
{
    // Render all emitters in order.
    cgParticleEmitterNode * pEmitterNode = static_cast<cgParticleEmitterNode*>(pIssuer);
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        cgParticleEmitter * pEmitter = pEmitterNode->getLayerEmitter( i );
        if ( pEmitter )
        {
            pEmitter->setEmitterMatrix( pIssuer->getWorldTransform(false) );
            pEmitter->render( pCamera );
        
        } // End if emitter
    
    } // Next layer
    
    // Drawn
    return true;
}

//-----------------------------------------------------------------------------
// Name : renderSubset ()
/// <summary>
/// Render only the section of this object that relates to the specified 
/// material (used for material batched / sorted rendering).
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::renderSubset( cgCameraNode * pCamera, cgVisibilitySet * pVisData, cgObjectNode * pIssuer, const cgMaterialHandle & hMaterial )
{
    // Route 'null' subset through to standard render.
    if ( !hMaterial.isValid() )
        return render( pCamera, pVisData, pIssuer );

    // Invalid subset
    return false;
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::applyObjectRescale( cgFloat fScale )
{
    /*// Apply the scale to object-space data
    cgFloat fNewWidth  = m_fWidth  * fScale;
    cgFloat fNewHeight = m_fHeight * fScale;
    cgFloat fNewLength = m_fLength * fScale;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        m_qUpdateProperties.bindParameter( 1, fNewWidth );
        m_qUpdateProperties.bindParameter( 2, fNewHeight );
        m_qUpdateProperties.bindParameter( 3, fNewLength );
        m_qUpdateProperties.bindParameter( 4, m_fTaper );
        m_qUpdateProperties.bindParameter( 5, mReferenceId );
        
        // Execute
        if ( !m_qUpdateProperties.step( true ) )
        {
            cgString strError;
            m_qUpdateProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update properties of bone object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local value.
    m_fWidth  = fNewWidth;
    m_fHeight = fNewHeight;
    m_fLength = fNewLength;

    // Sandbox mesh must be re-generated.
    m_hSandboxMesh.Close();*/

    cgToDo( "Particles", "ApplyRescale" );

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
bool cgParticleEmitterObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ParticleEmitterObject )
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
cgString cgParticleEmitterObject::getDatabaseTable( ) const
{
    return _T("Objects::ParticleEmitter");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Provide defaults if this is a new object (not cloned).
    if ( e->cloneMethod == cgCloneMethod::None )
    {
        // Create a default emitter layer.
        mLayers.resize( 1 );
        mLayers[0].databaseId = 0;
        mLayers[0].initialEmission = true;
        mLayers[0].applyGravity = false;
        cgParticleEmitterProperties & properties = mLayers[0].properties;

        // Default emitter properties
        properties.emitterType                 = cgParticleEmitterType::Billboards;
        properties.innerCone                   = 0;
        properties.outerCone                   = 50;
        properties.emissionRadius              = 0;
        properties.deadZoneRadius              = 0;
        properties.maxSimultaneousParticles    = 0;
        properties.initialParticles            = 0;
        properties.maxFiredParticles           = 0;
        properties.birthFrequency              = 60;
        properties.particleTexture             = _T("sys://Textures/Fire.xml");
        properties.sortedRender                = false;
        properties.emitterDirection            = cgVector3(0,0,0);
        properties.randomizeRotation           = true;
        properties.fireAmount                  = 0;
        properties.fireDelay                   = 0;
        properties.fireDelayOffset             = 0;
        properties.blendMethod                 = cgParticleBlendMethod::Additive;
        
        // Default particle properties
        properties.velocityAligned             = false;
        properties.velocityScaleStrength       = 0.0f;
        properties.speed.min                   = 2.5f;
        properties.speed.max                   = 3.0f;
        properties.mass.min                    = 10.0f;
        properties.mass.max                    = 10.0f;
        properties.angularSpeed.min            = -150.0f;
        properties.angularSpeed.max            = 150.0f;
        properties.baseScale.min               = 1.0f;
        properties.baseScale.max               = 1.0f;
        properties.lifetime.min                = 0.8f;
        properties.lifetime.max                = 1.0f;
        properties.airResistance               = 0.001f;
        properties.baseSize                    = cgSizeF(1.5f,1.5f);
        properties.hdrScale                    = 10.0f;

        // Default scale curves
        cgVector2 vPoint( 0.0f, 0.6f ), vOffset;
        vOffset = cgVector2(0.25f,0.70f) - vPoint;
        properties.scaleXCurve.setSplinePoint( 0, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
        vPoint = cgVector2( 1.0f, 1.0f );
        vOffset = vPoint - cgVector2(0.75f,0.90f);
        properties.scaleXCurve.setSplinePoint( 1, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
        properties.scaleYCurve = properties.scaleXCurve;
        
        // Default color curves
        properties.colorRCurve.setDescription( cgBezierSpline2::Maximum );
        properties.colorGCurve.setDescription( cgBezierSpline2::Maximum );
        properties.colorBCurve.setDescription( cgBezierSpline2::Maximum );
        
        // Default alpha curve
        properties.colorACurve.setDescription( cgBezierSpline2::Maximum );
        vPoint = cgVector2(0.25f, 1.0f);
        vOffset = cgVector2(0.3f, 1.0f) - vPoint;
        properties.colorACurve.insertPoint( 1, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
        vPoint = cgVector2(0.7f, 0.7f);
        vOffset = cgVector2(0.83728f, 0.43456f) - vPoint;
        properties.colorACurve.insertPoint( 2, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
        vPoint = cgVector2(1.0f, 0.0f);
        vOffset = vPoint - cgVector2(0.83728f, 0.43456f);
        properties.colorACurve.setSplinePoint( 3, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );

    } // End if new object

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
bool cgParticleEmitterObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ParticleEmitterObject::insertComponentData") );

        // Insert the emitter object properties.
        prepareQueries();

        mInsertEmitter.bindParameter( 1, mReferenceId );
        mInsertEmitter.bindParameter( 2, mSoftRefCount );

        // Execute
        if ( mInsertEmitter.step( true ) == false )
        {
            cgString strError;
            mInsertEmitter.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for particle emitter object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("ParticleEmitterObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Insert emitter layers.
        for ( size_t i = 0; i < mLayers.size(); ++i )
        {
            cgParticleEmitterProperties & properties = mLayers[i].properties;
            cgUInt32 blendMethod = (cgUInt32)properties.blendMethod;
            mInsertEmitterLayer.bindParameter( 1, mReferenceId );
            mInsertEmitterLayer.bindParameter( 2, (cgUInt32)i ); // LayerOrder
            mInsertEmitterLayer.bindParameter( 3, (cgUInt32)properties.emitterType );
            mInsertEmitterLayer.bindParameter( 4, cgString::Empty ); // ToDo: ScriptFile
            mInsertEmitterLayer.bindParameter( 5, properties.innerCone );
            mInsertEmitterLayer.bindParameter( 6, properties.outerCone );
            mInsertEmitterLayer.bindParameter( 7, properties.emissionRadius );
            mInsertEmitterLayer.bindParameter( 8, properties.deadZoneRadius );
            mInsertEmitterLayer.bindParameter( 9, properties.maxSimultaneousParticles );
            mInsertEmitterLayer.bindParameter( 10, properties.initialParticles );
            mInsertEmitterLayer.bindParameter( 11, properties.maxFiredParticles );
            mInsertEmitterLayer.bindParameter( 12, properties.birthFrequency );
            mInsertEmitterLayer.bindParameter( 13, properties.particleTexture );
            mInsertEmitterLayer.bindParameter( 14, properties.particleShader );
            mInsertEmitterLayer.bindParameter( 15, properties.sortedRender );
            mInsertEmitterLayer.bindParameter( 16, blendMethod );
            mInsertEmitterLayer.bindParameter( 17, properties.emitterDirection.x );
            mInsertEmitterLayer.bindParameter( 18, properties.emitterDirection.y );
            mInsertEmitterLayer.bindParameter( 19, properties.emitterDirection.z );
            mInsertEmitterLayer.bindParameter( 20, properties.randomizeRotation );
            mInsertEmitterLayer.bindParameter( 21, properties.fireAmount );
            mInsertEmitterLayer.bindParameter( 22, properties.fireDelay );
            mInsertEmitterLayer.bindParameter( 23, properties.fireDelayOffset );
            mInsertEmitterLayer.bindParameter( 24, properties.baseSize.width );
            mInsertEmitterLayer.bindParameter( 25, properties.baseSize.height );
            mInsertEmitterLayer.bindParameter( 26, properties.lifetime.min );
            mInsertEmitterLayer.bindParameter( 27, properties.lifetime.max );
            mInsertEmitterLayer.bindParameter( 28, properties.speed.min );
            mInsertEmitterLayer.bindParameter( 29, properties.speed.max );
            mInsertEmitterLayer.bindParameter( 30, properties.mass.min );
            mInsertEmitterLayer.bindParameter( 31, properties.mass.max );
            mInsertEmitterLayer.bindParameter( 32, properties.airResistance );
            mInsertEmitterLayer.bindParameter( 33, properties.angularSpeed.min );
            mInsertEmitterLayer.bindParameter( 34, properties.angularSpeed.max );
            mInsertEmitterLayer.bindParameter( 35, properties.baseScale.min );
            mInsertEmitterLayer.bindParameter( 36, properties.baseScale.max );
            mInsertEmitterLayer.bindParameter( 37, properties.hdrScale );

            // Curves
            cgUInt32 nCurveType = (cgUInt32)properties.scaleXCurve.getDescription();
            mInsertEmitterLayer.bindParameter( 38, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 39, properties.scaleXCurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 40, &properties.scaleXCurve.getSplinePoints()[0], properties.scaleXCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 39, 0 );
                mInsertEmitterLayer.bindParameter( 40, CG_NULL, 0 );
            
            } // End if described
            nCurveType = (cgUInt32)properties.scaleYCurve.getDescription();
            mInsertEmitterLayer.bindParameter( 41, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 42, properties.scaleYCurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 43, &properties.scaleYCurve.getSplinePoints()[0], properties.scaleYCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 42, 0 );
                mInsertEmitterLayer.bindParameter( 43, CG_NULL, 0 );
            
            } // End if described
            nCurveType = (cgUInt32)properties.colorRCurve.getDescription();
            mInsertEmitterLayer.bindParameter( 44, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 45, properties.colorRCurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 46, &properties.colorRCurve.getSplinePoints()[0], properties.colorRCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 45, 0 );
                mInsertEmitterLayer.bindParameter( 46, CG_NULL, 0 );
            
            } // End if described
            nCurveType = (cgUInt32)properties.colorGCurve.getDescription();
            mInsertEmitterLayer.bindParameter( 47, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 48, properties.colorGCurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 49, &properties.colorGCurve.getSplinePoints()[0], properties.colorGCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 48, 0 );
                mInsertEmitterLayer.bindParameter( 49, CG_NULL, 0 );
            
            } // End if described
            nCurveType = (cgUInt32)properties.colorBCurve.getDescription();
            mInsertEmitterLayer.bindParameter( 50, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 51, properties.colorBCurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 52, &properties.colorBCurve.getSplinePoints()[0], properties.colorBCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 51, 0 );
                mInsertEmitterLayer.bindParameter( 52, CG_NULL, 0 );
            
            } // End if described
            nCurveType = (cgUInt32)properties.colorACurve.getDescription();
            mInsertEmitterLayer.bindParameter( 53, nCurveType  );
            if ( nCurveType == cgBezierSpline2::Custom )
            {
                mInsertEmitterLayer.bindParameter( 54, properties.colorACurve.getPointCount() );
                mInsertEmitterLayer.bindParameter( 55, &properties.colorACurve.getSplinePoints()[0], properties.colorACurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
            
            } // End if custom
            else
            {
                mInsertEmitterLayer.bindParameter( 54, 0 );
                mInsertEmitterLayer.bindParameter( 55, CG_NULL, 0 );
            
            } // End if described
            
            // Final properties
            mInsertEmitterLayer.bindParameter( 56, mLayers[i].initialEmission );
            mInsertEmitterLayer.bindParameter( 57, mLayers[i].applyGravity );
            mInsertEmitterLayer.bindParameter( 58, mLayers[i].properties.velocityAligned );
            mInsertEmitterLayer.bindParameter( 59, mLayers[i].properties.velocityScaleStrength );

            // Execute
            if ( !mInsertEmitterLayer.step( true ) )
            {
                cgString strError;
                mInsertEmitterLayer.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert layer data for particle emitter object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
                mWorld->rollbackTransaction( _T("ParticleEmitterObject::insertComponentData") );
                return false;
            
            } // End if failed

            // Record the database ID for the layer.
            mLayers[i].databaseId = mInsertEmitterLayer.getLastInsertId();

        } // Next layer

        // Commit changes
        mWorld->commitTransaction( _T("ParticleEmitterObject::insertComponentData") );

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
bool cgParticleEmitterObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the emitter data.
    prepareQueries();
    mLoadEmitter.bindParameter( 1, e->sourceRefId );
    if ( !mLoadEmitter.step( ) || !mLoadEmitter.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadEmitter.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for particle emitter object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for particle emitter object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadEmitter.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadEmitter;

    // Load layers associated with this emitter.
    mLoadEmitterLayers.bindParameter( 1, e->sourceRefId );
    if ( !mLoadEmitterLayers.step( ) )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadEmitterLayers.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve layer data for particle emitter object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve layer data for particle emitter object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadEmitter.reset();
        mLoadEmitterLayers.reset();
        return false;

    } // End if failed
    for ( ; mLoadEmitterLayers.nextRow(); )
    {
        mLayers.resize( mLayers.size() + 1 );
        Layer & layer = mLayers.back();
        cgParticleEmitterProperties & properties = layer.properties;

        // Update our local members
        cgUInt32 blendMethod, emitterType;
        mLoadEmitterLayers.getColumn( _T("LayerId"), layer.databaseId );
        mLoadEmitterLayers.getColumn( _T("Type"), emitterType );
        mLoadEmitterLayers.getColumn( _T("InnerCone"), properties.innerCone );
        mLoadEmitterLayers.getColumn( _T("OuterCone"), properties.outerCone );
        mLoadEmitterLayers.getColumn( _T("EmissionRadius"), properties.emissionRadius );
        mLoadEmitterLayers.getColumn( _T("DeadZoneRadius"), properties.deadZoneRadius );
        mLoadEmitterLayers.getColumn( _T("MaxSimultaneousParticles"), properties.maxSimultaneousParticles );
        mLoadEmitterLayers.getColumn( _T("InitialParticles"), properties.initialParticles );
        mLoadEmitterLayers.getColumn( _T("MaxFiredParticles"), properties.maxFiredParticles );
        mLoadEmitterLayers.getColumn( _T("BirthFrequency"), properties.birthFrequency );
        mLoadEmitterLayers.getColumn( _T("ParticleTexture"), properties.particleTexture );
        mLoadEmitterLayers.getColumn( _T("ParticleShader"), properties.particleShader );
        mLoadEmitterLayers.getColumn( _T("SortedRender"), properties.sortedRender );
        mLoadEmitterLayers.getColumn( _T("BlendMethod"), blendMethod );
        mLoadEmitterLayers.getColumn( _T("FixedEmitDirX"), properties.emitterDirection.x );
        mLoadEmitterLayers.getColumn( _T("FixedEmitDirY"), properties.emitterDirection.y );
        mLoadEmitterLayers.getColumn( _T("FixedEmitDirZ"), properties.emitterDirection.z );
        mLoadEmitterLayers.getColumn( _T("RandomizeRotation"), properties.randomizeRotation );
        mLoadEmitterLayers.getColumn( _T("FireAmount"), properties.fireAmount );
        mLoadEmitterLayers.getColumn( _T("FireDelay"), properties.fireDelay );
        mLoadEmitterLayers.getColumn( _T("FireDelayOffset"), properties.fireDelayOffset );
        mLoadEmitterLayers.getColumn( _T("BaseSizeX"), properties.baseSize.width );
        mLoadEmitterLayers.getColumn( _T("BaseSizeY"), properties.baseSize.height );
        mLoadEmitterLayers.getColumn( _T("MinLifetime"), properties.lifetime.min );
        mLoadEmitterLayers.getColumn( _T("MaxLifetime"), properties.lifetime.max );
        mLoadEmitterLayers.getColumn( _T("MinSpeed"), properties.speed.min );
        mLoadEmitterLayers.getColumn( _T("MaxSpeed"), properties.speed.max );
        mLoadEmitterLayers.getColumn( _T("MinMass"), properties.mass.min );
        mLoadEmitterLayers.getColumn( _T("MaxMass"), properties.mass.max );
        mLoadEmitterLayers.getColumn( _T("AirResistance"), properties.airResistance );
        mLoadEmitterLayers.getColumn( _T("MinAngularSpeed"), properties.angularSpeed.min );
        mLoadEmitterLayers.getColumn( _T("MaxAngularSpeed"), properties.angularSpeed.max );
        mLoadEmitterLayers.getColumn( _T("MinBaseScale"), properties.baseScale.min );
        mLoadEmitterLayers.getColumn( _T("MaxBaseScale"), properties.baseScale.max );
        mLoadEmitterLayers.getColumn( _T("HDRScalar"), properties.hdrScale );
        mLoadEmitterLayers.getColumn( _T("EmissionEnabled"), layer.initialEmission );
        mLoadEmitterLayers.getColumn( _T("ApplyGravity"), layer.applyGravity );
        mLoadEmitterLayers.getColumn( _T("VelocityAligned"), properties.velocityAligned );
        mLoadEmitterLayers.getColumn( _T("VelocityScaleStrength"), properties.velocityScaleStrength );

        // Set enumerations
        properties.blendMethod = (cgParticleBlendMethod::Base)blendMethod;
        properties.emitterType = (cgParticleEmitterType::Base)emitterType;
        
        // Retrieve curve data. ScaleX Curve first.
        cgUInt32 nCurveType;
        mLoadEmitterLayers.getColumn( _T("ScaleXCurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ScaleXCurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ScaleXCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.scaleXCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.scaleXCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.scaleXCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

        // ScaleY Curve
        mLoadEmitterLayers.getColumn( _T("ScaleYCurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ScaleYCurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ScaleYCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.scaleYCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.scaleYCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.scaleYCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

        // ColorR Curve
        mLoadEmitterLayers.getColumn( _T("ColorRCurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ColorRCurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ColorRCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.colorRCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.colorRCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.colorRCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

        // ColorG Curve
        mLoadEmitterLayers.getColumn( _T("ColorGCurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ColorGCurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ColorGCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.colorGCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.colorGCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.colorGCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

        // ColorB Curve
        mLoadEmitterLayers.getColumn( _T("ColorBCurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ColorBCurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ColorBCurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.colorBCurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.colorBCurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.colorBCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

        // ColorA Curve
        mLoadEmitterLayers.getColumn( _T("ColorACurveType"), nCurveType );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            cgUInt32 nPointCount, nSize;
            cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
            mLoadEmitterLayers.getColumn( _T("ColorACurveSize"), nPointCount );
            mLoadEmitterLayers.getColumn( _T("ColorACurve"), (void**)&pPointData, nSize );
            if ( pPointData )
            {
                properties.colorACurve.clear();
                for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                    properties.colorACurve.addPoint( pPointData[nPoint] );

            } // End if valid point data

        } // End if custom
        else
        {
            properties.colorACurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

        } // End if described

    } // Next layer
    mLoadEmitterLayers.reset();

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
void cgParticleEmitterObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertEmitter.isPrepared() )
            mInsertEmitter.prepare( mWorld, _T("INSERT INTO 'Objects::ParticleEmitter' VALUES(?1,?2)"), true );
        if ( !mInsertEmitterLayer.isPrepared() )
            mInsertEmitterLayer.prepare( mWorld, _T("INSERT INTO 'Objects::ParticleEmitter::Layers' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,")
                                                 _T("?17,?18,?19,?20,?21,?22,?23,?24,?25,?26,?27,?28,?29,?30,?31,?32,?33,?34,?35,?36,?37,?38,?39,?40,?41,?42,?43,?44,?45,")
                                                 _T("?46,?47,?48,?49,?50,?51,?52,?53,?54,?55,?56,?57,?58,?59)"), true );
        if ( !mDeleteEmitterLayer.isPrepared() )
            mDeleteEmitterLayer.prepare( mWorld, _T("DELETE FROM 'Objects::ParticleEmitter::Layers' WHERE LayerId=?1"), true );
        if ( !mUpdateConeAngles.isPrepared() )
            mUpdateConeAngles.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET OuterCone=?1, InnerCone=?2 WHERE LayerId=?3"), true );
        if ( !mUpdateEmissionRadii.isPrepared() )
            mUpdateEmissionRadii.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET EmissionRadius=?1, DeadZoneRadius=?2 WHERE LayerId=?3"), true );
        if ( !mUpdateParticleCounts.isPrepared() )
            mUpdateParticleCounts.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET MaxSimultaneousParticles=?1, MaxFiredParticles=?2 WHERE LayerId=?3"), true );
        if ( !mUpdateReleaseProperties.isPrepared() )
            mUpdateReleaseProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET BirthFrequency=?1, RandomizeRotation=?2, EmissionEnabled=?3 WHERE LayerId=?4"), true );
        if ( !mUpdateRenderingProperties.isPrepared() )
            mUpdateRenderingProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET SortedRender=?1, BlendMethod=?2, HDRScalar=?3, VelocityAligned=?4, VelocityScaleStrength=?5 WHERE LayerId=?6"), true );
        if ( !mUpdateParticleProperties.isPrepared() )
            mUpdateParticleProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET MinSpeed=?1, MaxSpeed=?2, MinMass=?3, MaxMass=?4, MinAngularSpeed=?5,")
                                                       _T("MaxAngularSpeed=?6, MinBaseScale=?7, MaxBaseScale=?8, MinLifetime=?9, MaxLifetime=?10, BaseSizeX=?11,")
                                                       _T("BaseSizeY=?12, AirResistance=?13, ParticleTexture=?14, ApplyGravity=?15 WHERE LayerId=?16"), true );
        if ( !mUpdateColorKeys.isPrepared() )
            mUpdateColorKeys.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET ColorRCurveType=?1, ColorRCurveSize=?2, ColorRCurve=?3,")
                                              _T("ColorGCurveType=?4, ColorGCurveSize=?5, ColorGCurve=?6,")
                                              _T("ColorBCurveType=?7, ColorBCurveSize=?8, ColorBCurve=?9,")
                                              _T("ColorACurveType=?10, ColorACurveSize=?11, ColorACurve=?12 WHERE LayerId=?13"), true );
        if ( !mUpdateScaleKeys.isPrepared() )
            mUpdateScaleKeys.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter::Layers' SET ScaleXCurveType=?1, ScaleXCurveSize=?2, ScaleXCurve=?3,")
                                              _T("ScaleYCurveType=?4, ScaleYCurveSize=?5, ScaleYCurve=?6 WHERE LayerId=?7"), true );
    } // End if sandbox

    // Read queries
    if ( !mLoadEmitter.isPrepared() )
        mLoadEmitter.prepare( mWorld, _T("SELECT * FROM 'Objects::ParticleEmitter' WHERE RefId=?1"), true );
    if ( !mLoadEmitterLayers.isPrepared() )
        mLoadEmitterLayers.prepare( mWorld, _T("SELECT * FROM 'Objects::ParticleEmitter::Layers' WHERE EmitterId=?1 ORDER BY LayerOrder ASC"), true );
}

//-----------------------------------------------------------------------------
//  Name : getLayerCount()
/// <summary>
/// Retrieve the total number of emitter layers that have been defined.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgParticleEmitterObject::getLayerCount( ) const
{
    return (cgUInt32)mLayers.size();
}

//-----------------------------------------------------------------------------
//  Name : getLayerProperties()
/// <summary>
/// Retrieve the configuration structure that describes how the specified
/// emitter layer should be initialized and updated.
/// </summary>
//-----------------------------------------------------------------------------
const cgParticleEmitterProperties & cgParticleEmitterObject::getLayerProperties( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties;
}

//-----------------------------------------------------------------------------
//  Name : getInnerCone ()
/// <summary>
/// Get the inner cone (Theta) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getInnerCone( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.innerCone;
}

//-----------------------------------------------------------------------------
//  Name : getOuterCone ()
/// <summary>
/// Get the outer cone (Phi) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getOuterCone( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.outerCone;
}

//-----------------------------------------------------------------------------
//  Name : getEmissionRadius ()
/// <summary>
/// Get the radius around the emitter position from which particles can be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getEmissionRadius( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.emissionRadius;
}

//-----------------------------------------------------------------------------
//  Name : getDeadZoneRadius ()
/// <summary>
/// Get the radius around the emitter position from which particles cannot be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getDeadZoneRadius( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.deadZoneRadius;
}

//-----------------------------------------------------------------------------
//  Name : getMaxSimultaneousParticles ()
/// <summary>
/// Get the maximum number of particles that can be alive at any point in time 
/// for this emitter.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgParticleEmitterObject::getMaxSimultaneousParticles( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.maxSimultaneousParticles;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumParticles ()
/// <summary>
/// Get the maximum number of particles that this emitter can release before it
/// is automatically destroyed / unloaded.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgParticleEmitterObject::getMaximumParticles( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.maxFiredParticles;
}

//-----------------------------------------------------------------------------
//  Name : getBirthFrequency ()
/// <summary>
/// Get the rate at which the emitter will create and release particles each
/// second.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getBirthFrequency( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.birthFrequency;
}

//-----------------------------------------------------------------------------
//  Name : getHDRScale ()
/// <summary>
/// Get the amount to scale particle color values when HDR rendering is
/// enabled.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getHDRScale( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.hdrScale;
}

//-----------------------------------------------------------------------------
//  Name : getColorRCurve ()
/// <summary>
/// Get the bezier curve that describes the red component of the color to apply
/// to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getColorRCurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.colorRCurve;
}

//-----------------------------------------------------------------------------
//  Name : getColorGCurve ()
/// <summary>
/// Get the bezier curve that describes the green component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getColorGCurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.colorGCurve;
}

//-----------------------------------------------------------------------------
//  Name : getColorBCurve ()
/// <summary>
/// Get the bezier curve that describes the blue component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getColorBCurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.colorBCurve;
}

//-----------------------------------------------------------------------------
//  Name : getColorACurve ()
/// <summary>
/// Get the bezier curve that describes the alpha component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getColorACurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.colorACurve;
}

//-----------------------------------------------------------------------------
//  Name : getScaleXCurve ()
/// <summary>
/// Get the bezier curve that describes the X component of the scale to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getScaleXCurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.scaleXCurve;
}

//-----------------------------------------------------------------------------
//  Name : getScaleYCurve ()
/// <summary>
/// Get the bezier curve that describes the Y component of the scale to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgParticleEmitterObject::getScaleYCurve( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.scaleYCurve;
}

//-----------------------------------------------------------------------------
//  Name : getRandomizedRotation ()
/// <summary>
/// Get the property that, when enabled, causes each particle's rotation 
/// angle to be randomized upon its birth. Otherwise, all particles will 
/// default to a rotation angle of 0 degrees.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getRandomizedRotation( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.randomizeRotation;
}

//-----------------------------------------------------------------------------
//  Name : getSortedRender ()
/// <summary>
/// Get the property that indicates whether the emitter should sort the 
/// particles in a back to front order based on distance to the camera during
/// rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getSortedRender( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.sortedRender;
}

//-----------------------------------------------------------------------------
//  Name : getVelocityAlignment ()
/// <summary>
/// Get the property that indicates whether the particles should be aligned
/// and orientated to match their current velocities during rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getVelocityAlignment( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.velocityAligned;
}

//-----------------------------------------------------------------------------
//  Name : getVelocityScaleStrength ()
/// <summary>
/// Get the amount by which the velocity of each particle will affect its
/// scale (0 = no effect, 1 = full effect)
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getVelocityScaleStrength( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.velocityScaleStrength;
}

//-----------------------------------------------------------------------------
//  Name : getParticleSpeed ()
/// <summary>
/// Get the configured range of initial speed values that a particle can be 
/// assigned during its creation.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleSpeed( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.speed;
}

//-----------------------------------------------------------------------------
//  Name : getParticleTexture ()
/// <summary>
/// Get the name of the particle texture (or texture atlas) to map to the
/// billboards representing this emitters particles.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgParticleEmitterObject::getParticleTexture( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.particleTexture;
}

//-----------------------------------------------------------------------------
//  Name : getParticleMass ()
/// <summary>
/// Get the configured range of mass values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleMass( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.mass;
}

//-----------------------------------------------------------------------------
//  Name : getParticleAngularSpeed ()
/// <summary>
/// Get the configured range of angular (rotation) speed values that a particle 
/// can be assigned during its creation in degrees per second.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleAngularSpeed( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.angularSpeed;
}

//-----------------------------------------------------------------------------
//  Name : setParticleBaseScale ()
/// <summary>
/// Get the configured range of base scale values that a particle can be
/// assigned during its creation where 1 represents a particle whose initial
/// size matches that configured via the 'setParticleSize()' method.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleBaseScale( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.baseScale;
}

//-----------------------------------------------------------------------------
//  Name : getParticleLifetime ()
/// <summary>
/// Get the configured range of lifetime (maximum age) values that a particle
/// can be assigned during its creation in seconds.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleLifetime( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.lifetime;
}

//-----------------------------------------------------------------------------
//  Name : getParticleSize ()
/// <summary>
/// Get the configured initial size (pre-scale) of each particle in meters.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF  & cgParticleEmitterObject::getParticleSize( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.baseSize;
}

//-----------------------------------------------------------------------------
//  Name : GetAirResistance ()
/// <summary>
/// Get the configured air resistance co-efficient used to compute drag for
/// each particle during simulation.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getParticleAirResistance( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.airResistance;
}

//-----------------------------------------------------------------------------
//  Name : getParticleBlendMethod ()
/// <summary>
/// Get the technique used to blend the particles with the currently assigned
/// render target (i.e., additive, screen, linear, etc.)
/// </summary>
//-----------------------------------------------------------------------------
cgParticleBlendMethod::Base cgParticleEmitterObject::getParticleBlendMethod( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].properties.blendMethod;
}

//-----------------------------------------------------------------------------
//  Name : getInitialEnabledState ()
/// <summary>
/// Determine if the specified layer will begin to emit particles immediately
/// upon initial creation. If this value is set to false, the emitter for this
/// layer will need to be enabled manually before particles will be released.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getInitialEnabledState( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].initialEmission;
}

//-----------------------------------------------------------------------------
//  Name : isGravityEnabled ()
/// <summary>
/// Determine if gravity should be considered during particle updates.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::isGravityEnabled( cgUInt32 layerIndex ) const
{
    return mLayers[layerIndex].applyGravity;
}

//-----------------------------------------------------------------------------
//  Name : setInnerCone ()
/// <summary>
/// Set the inner cone (Theta) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setInnerCone( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.innerCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, layer.properties.outerCone );
        mUpdateConeAngles.bindParameter( 2, fValue );
        mUpdateConeAngles.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateConeAngles.step( true ) )
        {
            cgString strError;
            mUpdateConeAngles.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update cone angles for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
    layer.properties.innerCone = fabsf(fValue);
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("InnerCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer cone is at LEAST as large as the inner cone
    if ( layer.properties.innerCone > layer.properties.outerCone )
        setOuterCone( layerIndex, layer.properties.innerCone );
}

//-----------------------------------------------------------------------------
//  Name : setOuterCone ()
/// <summary>
/// Set the outer cone (Phi) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setOuterCone( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.outerCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, fValue );
        mUpdateConeAngles.bindParameter( 2, layer.properties.innerCone );
        mUpdateConeAngles.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateConeAngles.step( true ) )
        {
            cgString strError;
            mUpdateConeAngles.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update cone angles for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store the new emitter cone size
    layer.properties.outerCone = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("OuterCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner cone at MOST fits inside the outer cone
    if ( layer.properties.innerCone > layer.properties.outerCone )
        setInnerCone( layerIndex, layer.properties.outerCone );
}

//-----------------------------------------------------------------------------
//  Name : setDeadZoneRadius ()
/// <summary>
/// Set the radius around the emitter position from which particles cannot be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setDeadZoneRadius( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.deadZoneRadius == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateEmissionRadii.bindParameter( 1, layer.properties.emissionRadius );
        mUpdateEmissionRadii.bindParameter( 2, fValue );
        mUpdateEmissionRadii.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateEmissionRadii.step( true ) )
        {
            cgString strError;
            mUpdateEmissionRadii.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update emission radii for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
    layer.properties.deadZoneRadius = fabsf(fValue);
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("DeadZoneRadius");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer radius is at LEAST as large as the inner radius
    if ( layer.properties.deadZoneRadius > layer.properties.emissionRadius )
        setEmissionRadius( layerIndex, layer.properties.deadZoneRadius );
}

//-----------------------------------------------------------------------------
//  Name : setEmissionRadius ()
/// <summary>
/// Set the radius around the emitter position from which particles can be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setEmissionRadius( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.emissionRadius == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateEmissionRadii.bindParameter( 1, fValue );
        mUpdateEmissionRadii.bindParameter( 2, layer.properties.deadZoneRadius );
        mUpdateEmissionRadii.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateEmissionRadii.step( true ) )
        {
            cgString strError;
            mUpdateEmissionRadii.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update emission radii for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.emissionRadius = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("EmissionRadius");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner radius at MOST fits inside the outer radius
    if ( layer.properties.deadZoneRadius > layer.properties.emissionRadius )
        setDeadZoneRadius( layerIndex, layer.properties.emissionRadius );
}

//-----------------------------------------------------------------------------
//  Name : setMaxSimultaneousParticles ()
/// <summary>
/// Set the maximum number of particles that can be alive at any point in time 
/// for this emitter. Setting this value to 0 will cause the emitter to
/// automatically compute the required number of particles based on maximum
/// particle lifetime and rate of emission.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setMaxSimultaneousParticles( cgUInt32 layerIndex, cgUInt32 nAmount )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.maxSimultaneousParticles == nAmount )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleCounts.bindParameter( 1, nAmount );
        mUpdateParticleCounts.bindParameter( 2, layer.properties.maxFiredParticles );
        mUpdateParticleCounts.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleCounts.step( true ) )
        {
            cgString strError;
            mUpdateParticleCounts.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle count limits for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.maxSimultaneousParticles = nAmount;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MaxSimultaneousParticles");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setMaximumParticles ()
/// <summary>
/// Set the maximum number of particles that this emitter can release before it
/// is automatically destroyed / unloaded.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setMaximumParticles( cgUInt32 layerIndex, cgUInt32 amount )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.maxFiredParticles == amount )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleCounts.bindParameter( 1, layer.properties.maxSimultaneousParticles );
        mUpdateParticleCounts.bindParameter( 2, amount );
        mUpdateParticleCounts.bindParameter( 3, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleCounts.step( true ) )
        {
            cgString strError;
            mUpdateParticleCounts.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle count limits for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.maxFiredParticles = amount;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MaximumParticles");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setBirthFrequency ()
/// <summary>
/// Set the rate at which the emitter will create and release particles each
/// second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setBirthFrequency( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.birthFrequency == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateReleaseProperties.bindParameter( 1, fValue );
        mUpdateReleaseProperties.bindParameter( 2, layer.properties.randomizeRotation );
        mUpdateReleaseProperties.bindParameter( 3, layer.initialEmission );
        mUpdateReleaseProperties.bindParameter( 4, layer.databaseId );
        
        // Execute
        if ( !mUpdateReleaseProperties.step( true ) )
        {
            cgString strError;
            mUpdateReleaseProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle release properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.birthFrequency = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("BirthFrequency");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : enableRandomizedRotation ()
/// <summary>
/// Set the property that, when enabled, causes each particle's rotation 
/// angle to be randomized upon its birth. Otherwise, all particles will 
/// default to a rotation angle of 0 degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::enableRandomizedRotation( cgUInt32 layerIndex, bool bValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.randomizeRotation == bValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateReleaseProperties.bindParameter( 1, layer.properties.birthFrequency );
        mUpdateReleaseProperties.bindParameter( 2, bValue );
        mUpdateReleaseProperties.bindParameter( 3, layer.initialEmission );
        mUpdateReleaseProperties.bindParameter( 4, layer.databaseId );
        
        // Execute
        if ( !mUpdateReleaseProperties.step( true ) )
        {
            cgString strError;
            mUpdateReleaseProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle release properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.randomizeRotation = bValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("RandomizeRotation");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : enableSortedRender ()
/// <summary>
/// Set the property that indicates whether the emitter should sort the 
/// particles in a back to front order based on distance to the camera during
/// rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::enableSortedRender( cgUInt32 layerIndex, bool bValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.sortedRender == bValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, bValue );
        mUpdateRenderingProperties.bindParameter( 2, (cgUInt8)layer.properties.blendMethod );
        mUpdateRenderingProperties.bindParameter( 3, layer.properties.hdrScale );
        mUpdateRenderingProperties.bindParameter( 4, layer.properties.velocityAligned );
        mUpdateRenderingProperties.bindParameter( 5, layer.properties.velocityScaleStrength );
        mUpdateRenderingProperties.bindParameter( 6, layer.databaseId );
        
        // Execute
        if ( !mUpdateRenderingProperties.step( true ) )
        {
            cgString strError;
            mUpdateRenderingProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rendering properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.sortedRender = bValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("SortedRender");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : enableGravity ()
/// <summary>
/// Enable or disable the application of gravity forces to particles 
/// throughout their lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::enableGravity( cgUInt32 layerIndex, bool enable )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.applyGravity == enable )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, enable );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.applyGravity = enable;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ApplyGravity");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : enableVelocityAlignment ()
/// <summary>
/// Set the property that indicates whether the particles should be aligned
/// and orientated to match their current velocities during rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::enableVelocityAlignment( cgUInt32 layerIndex, bool enabled )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.velocityAligned == enabled )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, layer.properties.sortedRender );
        mUpdateRenderingProperties.bindParameter( 2, (cgUInt8)layer.properties.blendMethod );
        mUpdateRenderingProperties.bindParameter( 3, layer.properties.hdrScale );
        mUpdateRenderingProperties.bindParameter( 4, enabled );
        mUpdateRenderingProperties.bindParameter( 5, layer.properties.velocityScaleStrength );
        mUpdateRenderingProperties.bindParameter( 6, layer.databaseId );
        
        // Execute
        if ( !mUpdateRenderingProperties.step( true ) )
        {
            cgString strError;
            mUpdateRenderingProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rendering properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.velocityAligned = enabled;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("VelocityAligned");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setVelocityScaleStrength ()
/// <summary>
/// Set the amount by which the velocity of each particle will affect its
/// scale (0 = no effect, 1 = full effect)
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setVelocityScaleStrength( cgUInt32 layerIndex, cgFloat strength )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.velocityScaleStrength == strength )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, layer.properties.sortedRender );
        mUpdateRenderingProperties.bindParameter( 2, (cgUInt8)layer.properties.blendMethod );
        mUpdateRenderingProperties.bindParameter( 3, layer.properties.hdrScale );
        mUpdateRenderingProperties.bindParameter( 4, layer.properties.velocityAligned );
        mUpdateRenderingProperties.bindParameter( 5, strength );
        mUpdateRenderingProperties.bindParameter( 6, layer.databaseId );
        
        // Execute
        if ( !mUpdateRenderingProperties.step( true ) )
        {
            cgString strError;
            mUpdateRenderingProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rendering properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.velocityScaleStrength = strength;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("VelocityScaleStrength");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleTexture ()
/// <summary>
/// Set the name of the particle texture (or texture atlas) to map to the
/// billboards representing this emitters particles.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleTexture( cgUInt32 layerIndex, const cgString & textureFile )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.particleTexture == textureFile )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, textureFile );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.particleTexture = textureFile;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleTexture");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSpeed ()
/// <summary>
/// Set the range of initial speed values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSpeed( cgUInt32 layerIndex, cgFloat fMin, cgFloat fMax )
{
    setParticleSpeed( layerIndex, cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSpeed ()
/// <summary>
/// Set the range of initial speed values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSpeed( cgUInt32 layerIndex, const cgRangeF & Range )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.speed == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, Range.min );
        mUpdateParticleProperties.bindParameter( 2, Range.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.speed = Range;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleSpeed");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleMass ()
/// <summary>
/// Set the range of mass values that a particle can be assigned during its 
/// creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleMass( cgUInt32 layerIndex, cgFloat fMin, cgFloat fMax )
{
    setParticleMass( layerIndex, cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleMass ()
/// <summary>
/// Set the range of mass values that a particle can be assigned during its 
/// creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleMass( cgUInt32 layerIndex, const cgRangeF & Range )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.mass == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, Range.min );
        mUpdateParticleProperties.bindParameter( 4, Range.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.mass = Range;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleMass");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleAngularSpeed ()
/// <summary>
/// Set the range of angular (rotation) speed values that a particle can be 
/// assigned during its creation in degrees per second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleAngularSpeed( cgUInt32 layerIndex, cgFloat fMin, cgFloat fMax )
{
    setParticleAngularSpeed( layerIndex, cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleAngularSpeed ()
/// <summary>
/// Set the range of angular (rotation) speed values that a particle can be 
/// assigned during its creation in degrees per second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleAngularSpeed( cgUInt32 layerIndex, const cgRangeF & Range )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.angularSpeed == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, Range.min );
        mUpdateParticleProperties.bindParameter( 6, Range.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.angularSpeed = Range;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleAngularSpeed");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleBaseScale ()
/// <summary>
/// Set the range of base scale values that a particle can be assigned during 
/// its creation where 1 represents a particle whose initial size matches that
/// configured via the 'setParticleSize()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleBaseScale( cgUInt32 layerIndex, cgFloat fMin, cgFloat fMax )
{
    setParticleBaseScale( layerIndex, cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleBaseScale ()
/// <summary>
/// Set the range of base scale values that a particle can be assigned during 
/// its creation where 1 represents a particle whose initial size matches that
/// configured via the 'setParticleSize()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleBaseScale( cgUInt32 layerIndex, const cgRangeF & Range )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.baseScale == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, Range.min );
        mUpdateParticleProperties.bindParameter( 8, Range.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.baseScale = Range;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleBaseScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleLifetime ()
/// <summary>
/// Set the range of lifetime (maximum age) values that a particle can be 
/// assigned during its creation in seconds.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleLifetime( cgUInt32 layerIndex, cgFloat fMin, cgFloat fMax )
{
    setParticleLifetime( layerIndex, cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleLifetime ()
/// <summary>
/// Set the range of lifetime (maximum age) values that a particle can be 
/// assigned during its creation in seconds.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleLifetime( cgUInt32 layerIndex, const cgRangeF & Range )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.lifetime == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, Range.min );
        mUpdateParticleProperties.bindParameter( 10, Range.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.lifetime = Range;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleLifetime");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSize ()
/// <summary>
/// Set the initial size (pre-scale) of each particle in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSize( cgUInt32 layerIndex, cgFloat fWidth, cgFloat fHeight )
{
    setParticleSize( layerIndex, cgSizeF( fWidth, fHeight ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSize ()
/// <summary>
/// Set the initial size (pre-scale) of each particle in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSize( cgUInt32 layerIndex, const cgSizeF & Size )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.baseSize == Size )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, Size.width );
        mUpdateParticleProperties.bindParameter( 12, Size.height );
        mUpdateParticleProperties.bindParameter( 13, layer.properties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.baseSize = Size;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleBaseSize");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : SetAirResistance ()
/// <summary>
/// Set the air resistance co-efficient used to compute drag for each particle 
/// during simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleAirResistance( cgUInt32 layerIndex, cgFloat fValue )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.airResistance == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, layer.properties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, layer.properties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, layer.properties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, layer.properties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, layer.properties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, layer.properties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, layer.properties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, layer.properties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, layer.properties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, layer.properties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, layer.properties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, layer.properties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, fValue );
        mUpdateParticleProperties.bindParameter( 14, layer.properties.particleTexture );
        mUpdateParticleProperties.bindParameter( 15, layer.applyGravity );
        mUpdateParticleProperties.bindParameter( 16, layer.databaseId );
        
        // Execute
        if ( !mUpdateParticleProperties.step( true ) )
        {
            cgString strError;
            mUpdateParticleProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.airResistance = fValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleAirResistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setHDRScale ()
/// <summary>
/// Set the amount to scale particle color values when HDR rendering is
/// enabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setHDRScale( cgUInt32 layerIndex, cgFloat scale )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.hdrScale == scale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, layer.properties.sortedRender );
        mUpdateRenderingProperties.bindParameter( 2, (cgUInt8)layer.properties.blendMethod );
        mUpdateRenderingProperties.bindParameter( 3, scale );
        mUpdateRenderingProperties.bindParameter( 4, layer.properties.velocityAligned );
        mUpdateRenderingProperties.bindParameter( 5, layer.properties.velocityScaleStrength );
        mUpdateRenderingProperties.bindParameter( 6, layer.databaseId );
        
        // Execute
        if ( !mUpdateRenderingProperties.step( true ) )
        {
            cgString strError;
            mUpdateRenderingProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rendering properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.hdrScale = scale;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("HDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleBlendMethod ()
/// <summary>
/// Set the technique used to blend the particles with the currently assigned
/// render target (i.e., additive, screen, linear, etc.)
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleBlendMethod( cgUInt32 layerIndex, cgParticleBlendMethod::Base method )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.properties.blendMethod == method )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, layer.properties.sortedRender );
        mUpdateRenderingProperties.bindParameter( 2, (cgUInt8)method );
        mUpdateRenderingProperties.bindParameter( 3, layer.properties.hdrScale );
        mUpdateRenderingProperties.bindParameter( 4, layer.properties.velocityAligned );
        mUpdateRenderingProperties.bindParameter( 5, layer.properties.velocityScaleStrength );
        mUpdateRenderingProperties.bindParameter( 6, layer.databaseId );
        
        // Execute
        if ( !mUpdateRenderingProperties.step( true ) )
        {
            cgString strError;
            mUpdateRenderingProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rendering properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.properties.blendMethod = method;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleBlendMethod");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setInitialEnabledState ()
/// <summary>
/// Set the default emission state of the specified layer which will determine
/// if the emitter will begin to emit particles immediately upon initial 
/// creation. If this value is set to false, the emitter for this layer will 
/// need to be enabled manually before particles will be released.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setInitialEnabledState( cgUInt32 layerIndex, bool enabled )
{
    Layer & layer = mLayers[layerIndex];

    // Is this a no-op?
    if ( layer.initialEmission == enabled )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateReleaseProperties.bindParameter( 1, layer.properties.birthFrequency );
        mUpdateReleaseProperties.bindParameter( 2, layer.properties.randomizeRotation );
        mUpdateReleaseProperties.bindParameter( 3, enabled );
        mUpdateReleaseProperties.bindParameter( 4, layer.databaseId );
        
        // Execute
        if ( !mUpdateReleaseProperties.step( true ) )
        {
            cgString strError;
            mUpdateReleaseProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle release properties for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local member
    layer.initialEmission = enabled;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("InitialEnabledState");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setColorCurves ()
/// <summary>
/// Set the bezier curves that describe the four components of the color to 
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setColorCurves( cgUInt32 layerIndex, const cgBezierSpline2 & r, const cgBezierSpline2 & g, const cgBezierSpline2 & b, const cgBezierSpline2 & a )
{
    Layer & layer = mLayers[layerIndex];

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Red
        cgUInt32 curveType = (cgUInt32)r.getDescription();
        mUpdateColorKeys.bindParameter( 1, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateColorKeys.bindParameter( 2, r.getPointCount() );
            mUpdateColorKeys.bindParameter( 3, &r.getSplinePoints()[0], r.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateColorKeys.bindParameter( 2, 0 );
            mUpdateColorKeys.bindParameter( 3, CG_NULL, 0 );
        
        } // End if described

        // Green
        curveType = (cgUInt32)g.getDescription();
        mUpdateColorKeys.bindParameter( 4, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateColorKeys.bindParameter( 5, g.getPointCount() );
            mUpdateColorKeys.bindParameter( 6, &g.getSplinePoints()[0], g.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateColorKeys.bindParameter( 5, 0 );
            mUpdateColorKeys.bindParameter( 6, CG_NULL, 0 );
        
        } // End if described

        // Blue
        curveType = (cgUInt32)b.getDescription();
        mUpdateColorKeys.bindParameter( 7, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateColorKeys.bindParameter( 8, b.getPointCount() );
            mUpdateColorKeys.bindParameter( 9, &b.getSplinePoints()[0], b.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateColorKeys.bindParameter( 8, 0 );
            mUpdateColorKeys.bindParameter( 9, CG_NULL, 0 );
        
        } // End if described

        // Alpha
        curveType = (cgUInt32)a.getDescription();
        mUpdateColorKeys.bindParameter( 10, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateColorKeys.bindParameter( 11, a.getPointCount() );
            mUpdateColorKeys.bindParameter( 12, &a.getSplinePoints()[0], a.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateColorKeys.bindParameter( 11, 0 );
            mUpdateColorKeys.bindParameter( 12, CG_NULL, 0 );
        
        } // End if described
        mUpdateColorKeys.bindParameter( 13, layer.databaseId );
        
        // Execute
        if ( !mUpdateColorKeys.step( true ) )
        {
            cgString strError;
            mUpdateColorKeys.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle color animation curves for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local members
    layer.properties.colorRCurve = r;
    layer.properties.colorGCurve = g;
    layer.properties.colorBCurve = b;
    layer.properties.colorACurve = a;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ColorCurves");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setScaleCurves ()
/// <summary>
/// Set the bezier curves that describe the two components of the scale to 
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setScaleCurves( cgUInt32 layerIndex, const cgBezierSpline2 & x, const cgBezierSpline2 & y )
{
    Layer & layer = mLayers[layerIndex];

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();

        // X
        cgUInt32 curveType = (cgUInt32)x.getDescription();
        mUpdateScaleKeys.bindParameter( 1, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateScaleKeys.bindParameter( 2, x.getPointCount() );
            mUpdateScaleKeys.bindParameter( 3, &x.getSplinePoints()[0], x.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateScaleKeys.bindParameter( 2, 0 );
            mUpdateScaleKeys.bindParameter( 3, CG_NULL, 0 );
        
        } // End if described

        // Y
        curveType = (cgUInt32)y.getDescription();
        mUpdateScaleKeys.bindParameter( 4, curveType  );
        if ( curveType == cgBezierSpline2::Custom )
        {
            mUpdateScaleKeys.bindParameter( 5, y.getPointCount() );
            mUpdateScaleKeys.bindParameter( 6, &y.getSplinePoints()[0], y.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mUpdateScaleKeys.bindParameter( 5, 0 );
            mUpdateScaleKeys.bindParameter( 6, CG_NULL, 0 );
        
        } // End if described
        mUpdateScaleKeys.bindParameter( 7, layer.databaseId );
        
        // Execute
        if ( !mUpdateScaleKeys.step( true ) )
        {
            cgString strError;
            mUpdateScaleKeys.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update particle scale animation curves for particle emitter '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update local members
    layer.properties.scaleXCurve = x;
    layer.properties.scaleYCurve = y;
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ScaleCurves");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setColorRCurve ()
/// <summary>
/// Set the bezier curve that describes the red component of the color to apply
/// to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setColorRCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setColorCurves( layerIndex, curve, layer.properties.colorGCurve, layer.properties.colorBCurve, layer.properties.colorACurve );
}

//-----------------------------------------------------------------------------
//  Name : setColorGCurve ()
/// <summary>
/// Set the bezier curve that describes the green component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setColorGCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setColorCurves( layerIndex, layer.properties.colorRCurve, curve, layer.properties.colorBCurve, layer.properties.colorACurve );
}

//-----------------------------------------------------------------------------
//  Name : setColorBCurve ()
/// <summary>
/// Set the bezier curve that describes the blue component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setColorBCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setColorCurves( layerIndex, layer.properties.colorRCurve, layer.properties.colorGCurve, curve, layer.properties.colorACurve );
}

//-----------------------------------------------------------------------------
//  Name : setColorACurve ()
/// <summary>
/// Set the bezier curve that describes the alpha component of the color to
/// apply to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setColorACurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setColorCurves( layerIndex, layer.properties.colorRCurve, layer.properties.colorGCurve, layer.properties.colorBCurve, curve );
}

//-----------------------------------------------------------------------------
//  Name : setScaleXCurve ()
/// <summary>
/// Set the bezier curve that describes the X component of the scale to apply
/// to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setScaleXCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setScaleCurves( layerIndex, curve, layer.properties.scaleYCurve );
}

//-----------------------------------------------------------------------------
//  Name : setScaleYCurve ()
/// <summary>
/// Set the bezier curve that describes the Y component of the scale to apply
/// to each particle over its lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setScaleYCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
{
    Layer & layer = mLayers[layerIndex];
    setScaleCurves( layerIndex, layer.properties.scaleXCurve, curve );
}

///////////////////////////////////////////////////////////////////////////////
// cgParticleEmitterNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterNode::cgParticleEmitterNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize members to sensible defaults
    
    // Default the node color
    mColor = 0xFFAEBACB;

    // Default update rate to 'always' by default.
    mUpdateRate = cgUpdateRate::Always;

    // Automaticaly assign to the 'Effects' render class.
    mRenderClassId = pScene->getRenderClassId( _T("Effects") );

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Emitter%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterNode::cgParticleEmitterNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize members to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgParticleEmitterNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterNode::~cgParticleEmitterNode()
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
void cgParticleEmitterNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release particle manager(s).
    for ( size_t i = 0; i < mEmitters.size(); ++i )
    {
        if ( mEmitters[i] )
            mEmitters[i]->scriptSafeDispose();
    } // Next emitter
    mEmitters.clear();

    // Call base class implementation
    if ( bDisposeBase == true )
        cgObjectNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgParticleEmitterNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgParticleEmitterNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgParticleEmitterNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgParticleEmitterNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterNode::onComponentModified( cgReference * sender, cgComponentModifiedEventArgs * e )
{
    // Add / remove layers.
    size_t oldLayerCount = mEmitters.size();
    size_t newLayerCount = getLayerCount();
    if ( newLayerCount < oldLayerCount )
    {
        // Destroy un-used emitters
        for ( size_t i = newLayerCount; i < mEmitters.size(); ++i )
        {
            if ( mEmitters[i] )
                mEmitters[i]->scriptSafeDispose();

        } // Next emitter
        mEmitters.resize( newLayerCount );
    
    } // End if fewer layers
    else if ( newLayerCount > mEmitters.size() )
    {
        // Add and initialize new layers!
        mEmitters.resize(newLayerCount);
        for ( size_t i = oldLayerCount; i < newLayerCount; ++i )
        {
            mEmitters[i] = new cgParticleEmitter();
            if ( !mEmitters[i]->initialize( cgString::Empty, getLayerProperties(i), mParentScene->getRenderDriver() ) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for layer %i of particle emitter object '0x%x'.\n"), i + 1, mReferenceId );
                mEmitters[i]->scriptSafeDispose();
                mEmitters[i] = CG_NULL;
            
            } // End if failed
            else
            {
                // Initially enabled?
                mEmitters[i]->enableEmission( getInitialEnabledState(i) );

                // Use gravity?
                // ToDo: Needs to update if scene gravity changes.
                if ( isGravityEnabled(i) )
                    mEmitters[i]->setGravity( mParentScene->getPhysicsWorld()->getDefaultGravity() );
                else
                    mEmitters[i]->setGravity( cgVector3(0,0,0) );
            
            } // End if succeeded

        } // Next layer

    } // End if more layers

    // Update existing emitters
    size_t updateCount = min(oldLayerCount,newLayerCount);
    for ( size_t i = 0; i < updateCount; ++i )
    {
        if ( !mEmitters[i] )
        {
            mEmitters[i] = new cgParticleEmitter();
            if ( !mEmitters[i]->initialize( cgString::Empty, getLayerProperties(i), mParentScene->getRenderDriver() ) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for layer %i of particle emitter object '0x%x'.\n"), i + 1, mReferenceId );
                mEmitters[i]->scriptSafeDispose();
                mEmitters[i] = CG_NULL;
            
            } // End if failed
            else
            {
                // Initially enabled?
                mEmitters[i]->enableEmission( getInitialEnabledState(i) );

                // Use gravity?
                // ToDo: Needs to update if scene gravity changes.
                if ( isGravityEnabled(i) )
                    mEmitters[i]->setGravity( mParentScene->getPhysicsWorld()->getDefaultGravity() );
                else
                    mEmitters[i]->setGravity( cgVector3(0,0,0) );
            
            } // End if success

        } // End if not allocated
        else
        {
            if ( !mEmitters[i]->setEmitterProperties( getLayerProperties(i) ) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to update internal emitter manager for layer %i of particle emitter object '0x%x'.\n"), i + 1, mReferenceId );
                mEmitters[i]->scriptSafeDispose();
                mEmitters[i] = CG_NULL;
            
            } // End if failed
            else
            {
                // Update the enabled state to match the default state
                // only when in sandbox mode.
                if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
                    mEmitters[i]->enableEmission( getInitialEnabledState(i) );

                // Use gravity?
                // ToDo: Needs to update if scene gravity changes.
                if ( isGravityEnabled(i) )
                    mEmitters[i]->setGravity( mParentScene->getPhysicsWorld()->getDefaultGravity() );
                else
                    mEmitters[i]->setGravity( cgVector3(0,0,0) );
            
            } // End if succeeded

        } // End if already allocated

    } // Next layer

    // Call base class implementation last
    cgObjectNode::onComponentModified( sender, e );
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation.
    if ( !cgObjectNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    // Create an emitter manager for each layer and initialize it.
    mEmitters.resize(getLayerCount());
    for ( cgUInt32 i = 0; i < getLayerCount(); ++i )
    {
        mEmitters[i] = new cgParticleEmitter();
        if ( !mEmitters[i]->initialize( cgString::Empty, getLayerProperties(i), mParentScene->getRenderDriver() ) )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for layer %i of particle emitter object '0x%x'.\n"), i + 1, mReferenceId );
            mEmitters[i]->scriptSafeDispose();
            mEmitters[i] = CG_NULL;
        
        } // End if failed
        else
        {
            // Initially enabled?
            mEmitters[i]->enableEmission( getInitialEnabledState(i) );

            // Use gravity?
            // ToDo: Needs to update if scene gravity changes.
            if ( isGravityEnabled(i) )
                mEmitters[i]->setGravity( mParentScene->getPhysicsWorld()->getDefaultGravity() );
            else
                mEmitters[i]->setGravity( cgVector3(0,0,0) );
        
        } // End if succeeded

    } // Next layer

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
bool cgParticleEmitterNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Create an emitter manager for each layer and initialize it.
    mEmitters.resize(getLayerCount());
    for ( cgUInt32 i = 0; i < getLayerCount(); ++i )
    {
        mEmitters[i] = new cgParticleEmitter();
        if ( !mEmitters[i]->initialize( cgString::Empty, getLayerProperties(i), mParentScene->getRenderDriver() ) )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for layer %i of particle emitter object '0x%x'.\n"), i + 1, mReferenceId );
            mEmitters[i]->scriptSafeDispose();
            mEmitters[i] = CG_NULL;
        
        } // End if failed
        else
        {
            // Initially enabled?
            mEmitters[i]->enableEmission( getInitialEnabledState(i) );

            // Use gravity?
            // ToDo: Needs to update if scene gravity changes.
            if ( isGravityEnabled(i) )
                mEmitters[i]->setGravity( mParentScene->getPhysicsWorld()->getDefaultGravity() );
            else
                mEmitters[i]->setGravity( cgVector3(0,0,0) );
        
        } // End if succeeded

    } // Next layer

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ParticleEmitterNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : registerVisibility () (Virtual)
/// <summary>
/// This node has been deemed visible during testing, but this method gives the
/// node a final say on how it gets registered with the visibility set. The
/// default behavior is simply to insert directly into object visibility list,
/// paying close attention to filtering rules.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::registerVisibility( cgVisibilitySet * pSet )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    if ( !cgObjectNode::registerVisibility( pSet ) )
        return false;

    // Register with the 'default' (null) material so that this node 
    // can be included in standard material batched / queue based rendering.
    cgUInt32 nFlags = pSet->getSearchFlags();
    if ( nFlags & cgVisibilitySearchFlags::CollectMaterials )
        pSet->addVisibleMaterial( cgMaterialHandle::Null, this );
    
    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : particlesSpent ()
/// <summary>
/// Determine if all particles have been spent.
/// Note : If you want to ensure that all fired particles are dead, make sure
/// that you specify a value of true to the 'bIncludeAlive' parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::particlesSpent( bool includeAlive ) const
{
    for ( size_t i = 0; i < mEmitters.size(); ++i )
    {
        if ( mEmitters[i] )
        {
            if ( !mEmitters[i]->particlesSpent( includeAlive ) )
                return false;
        
        } // End if valid
    
    } // Next emitter

    // All particles spent!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLayerEmitter ()
/// <summary>
/// Retrieve the internal system particle emitter that manages the particle
/// data and oversees the update process for the specified layer.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitter * cgParticleEmitterNode::getLayerEmitter( cgUInt32 layerIndex ) const
{
    return mEmitters[layerIndex];
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Allow the object to perform necessary updates during the frame update 
/// phase.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterNode::update( cgFloat fElapsedTime )
{
    // Call base class implementation first to allow the node itself 
    // to update / move, etc.
    cgObjectNode::update( fElapsedTime );

    // Now allow the emitters to update.
    bool destroyEmitter = (cgGetSandboxMode() != cgSandboxMode::Enabled);
    for ( size_t i = 0; i < mEmitters.size(); ++i )
    {
        if ( mEmitters[i] )
        {
            mEmitters[i]->setEmitterMatrix( getWorldTransform(false) );
            mEmitters[i]->update( fElapsedTime, cgVector3(0,0,0), true );

            // Destroy the emitter when all particles are spent (except in sandbox mode).
            if ( !getMaximumParticles(i) || !mEmitters[i]->particlesSpent( true ) )
                destroyEmitter = false;    
        
        } // End if valid
        else
            destroyEmitter = false;
    
    } // Next emitter

    // Clean up the emitter?
    if ( !mEmitters.empty() && destroyEmitter )
        unload();
}

//-----------------------------------------------------------------------------
// Name : allowSandboxUpdate ( )
/// <summary>
/// Return true if the node should have its update method called even when
/// not running a preview in sandbox mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::allowSandboxUpdate( ) const
{
    // Allow the emitter's 'update' method to be called
    // irrespective of whether or not we're in sandbox 
    // preview mode so that we can see a visual representation
    // of the emitter's configured properties within the editor.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : enableLayerEmission ()
/// <summary>
/// Enable or disable the emission of new particles for the specified layer. 
/// Any existing particles that are alive will still be updated until the end
/// of their lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterNode::enableLayerEmission( cgUInt32 layerIndex, bool enable )
{
    if ( layerIndex < mEmitters.size() && mEmitters[layerIndex] )
        mEmitters[layerIndex]->enableEmission( enable );
}

//-----------------------------------------------------------------------------
//  Name : isLayerEmissionEnabled ()
/// <summary>
/// Determine if the emission of new particles is currently enabled for the
/// specified layer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterNode::isLayerEmissionEnabled( cgUInt32 layerIndex ) const
{
    if ( layerIndex >= mEmitters.size() || !mEmitters[layerIndex] )
        return false;
    return mEmitters[layerIndex]->isEmissionEnabled();
}