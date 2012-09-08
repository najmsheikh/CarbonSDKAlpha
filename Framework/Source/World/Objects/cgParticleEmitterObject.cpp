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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
cgWorldQuery cgParticleEmitterObject::mUpdateConeAngles;
cgWorldQuery cgParticleEmitterObject::mUpdateEmissionRadii;
cgWorldQuery cgParticleEmitterObject::mUpdateParticleCounts;
cgWorldQuery cgParticleEmitterObject::mUpdateReleaseProperties;
cgWorldQuery cgParticleEmitterObject::mUpdateRenderingProperties;
cgWorldQuery cgParticleEmitterObject::mUpdateParticleProperties;
cgWorldQuery cgParticleEmitterObject::mLoadEmitter;

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitterObject::cgParticleEmitterObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Default emitter properties
    mProperties.innerCone                  = 0;
    mProperties.outerCone                  = 50;
    mProperties.emissionRadius             = 0;
    mProperties.deadZoneRadius             = 0;
    mProperties.maxSimultaneousParticles   = 0;
    mProperties.initialParticles           = 0;
    mProperties.maxFiredParticles          = 0;
    mProperties.birthFrequency             = 60;
    mProperties.textureFile                = _T("sys://Textures/Fire.xml");
    mProperties.shaderSource               = _T("sys://Shaders/ParticleScreenBlend.sh");
    mProperties.sortedRender               = false;
    mProperties.emitterDirection           = cgVector3(0,0,0);
    mProperties.randomizeRotation          = true;
    mProperties.fireAmount                 = 0;
    mProperties.fireDelay                  = 0;
    mProperties.fireDelayOffset            = 0;
    
    // Default particle properties
    mProperties.speed.min                  = 2.5f;
    mProperties.speed.max                  = 3.0f;
    mProperties.mass.min                   = 10.0f;
    mProperties.mass.max                   = 10.0f;
    mProperties.angularSpeed.min           = -150.0f;
    mProperties.angularSpeed.max           = 150.0f;
    mProperties.baseScale.min              = 1.0f;
    mProperties.baseScale.max              = 1.0f;
    mProperties.lifetime.min               = 0.8f;
    mProperties.lifetime.max               = 1.0f;
    mProperties.airResistance              = 0.001f;
    mProperties.baseSize                   = cgSizeF(1.5f,1.5f);

    // Default scale curves
    cgVector2 vPoint( 0.0f, 0.6f ), vOffset;
    vOffset = cgVector2(0.25f,0.70f) - vPoint;
    mProperties.scaleXCurve.setSplinePoint( 0, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
    vPoint = cgVector2( 1.0f, 1.0f );
    vOffset = vPoint - cgVector2(0.75f,0.90f);
    mProperties.scaleXCurve.setSplinePoint( 1, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
    mProperties.scaleYCurve = mProperties.scaleXCurve;
    
    // Default color curves
    mProperties.colorRCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorGCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorBCurve.setDescription( cgBezierSpline2::Maximum );

    // Default alpha curve
    mProperties.colorACurve.setDescription( cgBezierSpline2::Maximum );
    vPoint = cgVector2(0.25f, 1.0f);
    vOffset = cgVector2(0.3f, 1.0f) - vPoint;
    mProperties.colorACurve.insertPoint( 1, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
    vPoint = cgVector2(0.7f, 0.7f);
    vOffset = cgVector2(0.83728f, 0.43456f) - vPoint;
    mProperties.colorACurve.insertPoint( 2, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
    vPoint = cgVector2(1.0f, 0.0f);
    vOffset = vPoint - cgVector2(0.83728f, 0.43456f);
    mProperties.colorACurve.setSplinePoint( 3, cgBezierSpline2::SplinePoint( vPoint - vOffset, vPoint, vPoint + vOffset ) );
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
    mScriptFile = pObject->mScriptFile;
    mProperties = pObject->mProperties;
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
    cgBoundingBox Bounds;

    // Compute bounding box for emitter
    cgToDo( "Particles", "Compute bounding box" );
    return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    return false;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgShadedVertex Points[29];
    cgUInt32       Indices[29];
    cgUInt32       i;
    cgFloat        fRange = 2.0f;

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
    for ( i = 0; i < 9; ++i )
        Points[i].color = nColor;

    // Compute vertices for the tip and base of the cone
    cgFloat fSize      = fZoomFactor * 25.0f * cosf(CGEToRadian(mProperties.outerCone * 0.5f));
    cgFloat fRadius    = tanf(CGEToRadian(mProperties.outerCone * 0.5f)) * fSize;
    Points[0].position = cgVector3( 0, 0, 0 );
    for ( i = 0; i < 8; ++i )
    {
        // Build vertex
        Points[i+1].position.x = (sinf( (CGE_TWO_PI / 8.0f) * (cgFloat)i ) * fRadius);
        Points[i+1].position.y = (cosf( (CGE_TWO_PI / 8.0f) * (cgFloat)i ) * fRadius);
        Points[i+1].position.z = fSize;
    
    } // Next Base Vertex
    
    // Compute indices that will allow us to draw the cone using a 
    // tri-list (wireframe) so that we can easily take advantage of back-face culling.
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 5;
    Indices[12] = 0; Indices[13] = 5; Indices[14] = 6;
    Indices[15] = 0; Indices[16] = 6; Indices[17] = 7;
    Indices[18] = 0; Indices[19] = 7; Indices[20] = 8;
    Indices[21] = 0; Indices[22] = 8; Indices[23] = 1;

    // Begin rendering
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
                for ( i = 0; i < 8; ++i )
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
                fRadius = tanf(CGEToRadian(mProperties.innerCone * 0.5f)) * fRange;
                nColor  = 0xFF99CCE5;
                
                // Generate line strip circle 
                for ( i = 0; i < 28; ++i )
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
                if ( mProperties.outerCone > mProperties.innerCone )
                {
                    fRadius = tanf(CGEToRadian(mProperties.outerCone * 0.5f)) * fRange;
                    nColor  = 0xFF4C6672;

                    // Generate line strip circle 
                    for ( i = 0; i < 28; ++i )
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
    cgWorldObject::sandboxRender( pCamera, pVisData, bWireframe, GridPlane, pIssuer );
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
    cgParticleEmitter * pEmitter = ((cgParticleEmitterNode*)pIssuer)->getEmitter();
    if ( !pEmitter )
        return false;

    // Render the emitter.
    pEmitter->render( pCamera );
    
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

        // Update database.
        prepareQueries();
        mInsertEmitter.bindParameter( 1, mReferenceId );
        mInsertEmitter.bindParameter( 2, 0 ); // ToDo: Type
        mInsertEmitter.bindParameter( 3, mScriptFile );
        mInsertEmitter.bindParameter( 4, mProperties.innerCone );
        mInsertEmitter.bindParameter( 5, mProperties.outerCone );
        mInsertEmitter.bindParameter( 6, mProperties.emissionRadius );
        mInsertEmitter.bindParameter( 7, mProperties.deadZoneRadius );
        mInsertEmitter.bindParameter( 8, mProperties.maxSimultaneousParticles );
        mInsertEmitter.bindParameter( 9, mProperties.initialParticles );
        mInsertEmitter.bindParameter( 10, mProperties.maxFiredParticles );
        mInsertEmitter.bindParameter( 11, mProperties.birthFrequency );
        mInsertEmitter.bindParameter( 12, mProperties.textureFile );
        mInsertEmitter.bindParameter( 13, mProperties.shaderSource );
        mInsertEmitter.bindParameter( 14, mProperties.sortedRender );
        mInsertEmitter.bindParameter( 15, mProperties.emitterDirection.x );
        mInsertEmitter.bindParameter( 16, mProperties.emitterDirection.y );
        mInsertEmitter.bindParameter( 17, mProperties.emitterDirection.z );
        mInsertEmitter.bindParameter( 18, mProperties.randomizeRotation );
        mInsertEmitter.bindParameter( 19, mProperties.fireAmount );
        mInsertEmitter.bindParameter( 20, mProperties.fireDelay );
        mInsertEmitter.bindParameter( 21, mProperties.fireDelayOffset );
        mInsertEmitter.bindParameter( 22, mProperties.baseSize.width );
        mInsertEmitter.bindParameter( 23, mProperties.baseSize.height );
        mInsertEmitter.bindParameter( 24, mProperties.lifetime.min );
        mInsertEmitter.bindParameter( 25, mProperties.lifetime.max );
        mInsertEmitter.bindParameter( 26, mProperties.speed.min );
        mInsertEmitter.bindParameter( 27, mProperties.speed.max );
        mInsertEmitter.bindParameter( 28, mProperties.mass.min );
        mInsertEmitter.bindParameter( 29, mProperties.mass.max );
        mInsertEmitter.bindParameter( 30, mProperties.airResistance );
        mInsertEmitter.bindParameter( 31, mProperties.angularSpeed.min );
        mInsertEmitter.bindParameter( 32, mProperties.angularSpeed.max );
        mInsertEmitter.bindParameter( 33, mProperties.baseScale.min );
        mInsertEmitter.bindParameter( 34, mProperties.baseScale.max );

        // Curves
        cgUInt32 nCurveType = (cgUInt32)mProperties.scaleXCurve.getDescription();
        mInsertEmitter.bindParameter( 35, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 36, mProperties.scaleXCurve.getPointCount() );
            mInsertEmitter.bindParameter( 37, &mProperties.scaleXCurve.getSplinePoints()[0], mProperties.scaleXCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 36, 0 );
            mInsertEmitter.bindParameter( 37, CG_NULL, 0 );
        
        } // End if described
        nCurveType = (cgUInt32)mProperties.scaleYCurve.getDescription();
        mInsertEmitter.bindParameter( 38, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 39, mProperties.scaleYCurve.getPointCount() );
            mInsertEmitter.bindParameter( 40, &mProperties.scaleYCurve.getSplinePoints()[0], mProperties.scaleYCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 39, 0 );
            mInsertEmitter.bindParameter( 40, CG_NULL, 0 );
        
        } // End if described
        nCurveType = (cgUInt32)mProperties.colorRCurve.getDescription();
        mInsertEmitter.bindParameter( 41, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 42, mProperties.colorRCurve.getPointCount() );
            mInsertEmitter.bindParameter( 43, &mProperties.colorRCurve.getSplinePoints()[0], mProperties.colorRCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 42, 0 );
            mInsertEmitter.bindParameter( 43, CG_NULL, 0 );
        
        } // End if described
        nCurveType = (cgUInt32)mProperties.colorGCurve.getDescription();
        mInsertEmitter.bindParameter( 44, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 45, mProperties.colorGCurve.getPointCount() );
            mInsertEmitter.bindParameter( 46, &mProperties.colorGCurve.getSplinePoints()[0], mProperties.colorGCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 45, 0 );
            mInsertEmitter.bindParameter( 46, CG_NULL, 0 );
        
        } // End if described
        nCurveType = (cgUInt32)mProperties.colorBCurve.getDescription();
        mInsertEmitter.bindParameter( 47, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 48, mProperties.colorBCurve.getPointCount() );
            mInsertEmitter.bindParameter( 49, &mProperties.colorBCurve.getSplinePoints()[0], mProperties.colorBCurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 48, 0 );
            mInsertEmitter.bindParameter( 49, CG_NULL, 0 );
        
        } // End if described
        nCurveType = (cgUInt32)mProperties.colorACurve.getDescription();
        mInsertEmitter.bindParameter( 50, nCurveType  );
        if ( nCurveType == cgBezierSpline2::Custom )
        {
            mInsertEmitter.bindParameter( 51, mProperties.colorACurve.getPointCount() );
            mInsertEmitter.bindParameter( 52, &mProperties.colorACurve.getSplinePoints()[0], mProperties.colorACurve.getPointCount() * sizeof(cgBezierSpline2::SplinePoint) );
        
        } // End if custom
        else
        {
            mInsertEmitter.bindParameter( 51, 0 );
            mInsertEmitter.bindParameter( 52, CG_NULL, 0 );
        
        } // End if described
        
        mInsertEmitter.bindParameter( 53, mSoftRefCount );

        // Execute
        if ( mInsertEmitter.step( true ) == false )
        {
            cgString strError;
            mInsertEmitter.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for particle emitter object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("ParticleEmitterObject::insertComponentData") );
            return false;
        
        } // End if failed

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

    // Update our local members
    mLoadEmitter.getColumn( _T("ScriptFile"), mScriptFile );
    mLoadEmitter.getColumn( _T("InnerCone"), mProperties.innerCone );
    mLoadEmitter.getColumn( _T("OuterCone"), mProperties.outerCone );
    mLoadEmitter.getColumn( _T("EmissionRadius"), mProperties.emissionRadius );
    mLoadEmitter.getColumn( _T("DeadZoneRadius"), mProperties.deadZoneRadius );
    mLoadEmitter.getColumn( _T("MaxSimultaneousParticles"), mProperties.maxSimultaneousParticles );
    mLoadEmitter.getColumn( _T("InitialParticles"), mProperties.initialParticles );
    mLoadEmitter.getColumn( _T("MaxFiredParticles"), mProperties.maxFiredParticles );
    mLoadEmitter.getColumn( _T("BirthFrequency"), mProperties.birthFrequency );
    mLoadEmitter.getColumn( _T("TextureFile"), mProperties.textureFile );
    mLoadEmitter.getColumn( _T("ShaderSource"), mProperties.shaderSource );
    mLoadEmitter.getColumn( _T("SortedRender"), mProperties.sortedRender );
    mLoadEmitter.getColumn( _T("FixedEmitDirX"), mProperties.emitterDirection.x );
    mLoadEmitter.getColumn( _T("FixedEmitDirY"), mProperties.emitterDirection.y );
    mLoadEmitter.getColumn( _T("FixedEmitDirZ"), mProperties.emitterDirection.z );
    mLoadEmitter.getColumn( _T("RandomizeRotation"), mProperties.randomizeRotation );
    mLoadEmitter.getColumn( _T("FireAmount"), mProperties.fireAmount );
    mLoadEmitter.getColumn( _T("FireDelay"), mProperties.fireDelay );
    mLoadEmitter.getColumn( _T("FireDelayOffset"), mProperties.fireDelayOffset );
    mLoadEmitter.getColumn( _T("BaseSizeX"), mProperties.baseSize.width );
    mLoadEmitter.getColumn( _T("BaseSizeY"), mProperties.baseSize.height );
    mLoadEmitter.getColumn( _T("MinLifetime"), mProperties.lifetime.min );
    mLoadEmitter.getColumn( _T("MaxLifetime"), mProperties.lifetime.max );
    mLoadEmitter.getColumn( _T("MinSpeed"), mProperties.speed.min );
    mLoadEmitter.getColumn( _T("MaxSpeed"), mProperties.speed.max );
    mLoadEmitter.getColumn( _T("MinMass"), mProperties.mass.min );
    mLoadEmitter.getColumn( _T("MaxMass"), mProperties.mass.max );
    mLoadEmitter.getColumn( _T("AirResistance"), mProperties.airResistance );
    mLoadEmitter.getColumn( _T("MinAngularSpeed"), mProperties.angularSpeed.min );
    mLoadEmitter.getColumn( _T("MaxAngularSpeed"), mProperties.angularSpeed.max );
    mLoadEmitter.getColumn( _T("MinBaseScale"), mProperties.baseScale.min );
    mLoadEmitter.getColumn( _T("MaxBaseScale"), mProperties.baseScale.max );
    
    // Retrieve curve data. ScaleX Curve first.
    cgUInt32 nCurveType;
    mLoadEmitter.getColumn( _T("ScaleXCurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ScaleXCurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ScaleXCurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.scaleXCurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.scaleXCurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.scaleXCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

    // ScaleY Curve
    mLoadEmitter.getColumn( _T("ScaleYCurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ScaleYCurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ScaleYCurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.scaleYCurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.scaleYCurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.scaleYCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

    // ColorR Curve
    mLoadEmitter.getColumn( _T("ColorRCurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ColorRCurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ColorRCurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.colorRCurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.colorRCurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.colorRCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

    // ColorG Curve
    mLoadEmitter.getColumn( _T("ColorGCurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ColorGCurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ColorGCurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.colorGCurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.colorGCurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.colorGCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

    // ColorB Curve
    mLoadEmitter.getColumn( _T("ColorBCurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ColorBCurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ColorBCurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.colorBCurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.colorBCurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.colorBCurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

    // ColorA Curve
    mLoadEmitter.getColumn( _T("ColorACurveType"), nCurveType );
    if ( nCurveType == cgBezierSpline2::Custom )
    {
        cgUInt32 nPointCount, nSize;
        cgBezierSpline2::SplinePoint * pPointData = CG_NULL;
        mLoadEmitter.getColumn( _T("ColorACurveSize"), nPointCount );
        mLoadEmitter.getColumn( _T("ColorACurve"), (void**)&pPointData, nSize );
        if ( pPointData )
        {
            mProperties.colorACurve.clear();
            for ( cgUInt32 nPoint = 0; nPoint < nPointCount; ++nPoint )
                mProperties.colorACurve.addPoint( pPointData[nPoint] );

        } // End if valid point data

    } // End if custom
    else
    {
        mProperties.colorACurve.setDescription( (cgBezierSpline2::SplineDescription)nCurveType );

    } // End if described

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
            mInsertEmitter.prepare( mWorld, _T("INSERT INTO 'Objects::ParticleEmitter' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,")
                                                _T("?20,?21,?22,?23,?24,?25,?26,?27,?28,?29,?30,?31,?32,?33,?34,?35,?36,?37,?38,?39,?40,?41,?42,?43,?44,?45,?46,?47,")
                                                _T("?48,?49,?50,?51,?52,?53)"), true );
        if ( !mUpdateConeAngles.isPrepared() )
            mUpdateConeAngles.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET OuterCone=?1, InnerCone=?2 WHERE RefId=?3"), true );
        if ( !mUpdateEmissionRadii.isPrepared() )
            mUpdateEmissionRadii.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET EmissionRadius=?1, DeadZoneRadius=?2 WHERE RefId=?3"), true );
        if ( !mUpdateParticleCounts.isPrepared() )
            mUpdateParticleCounts.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET MaxSimultaneousParticles=?1 WHERE RefId=?2"), true );
        if ( !mUpdateReleaseProperties.isPrepared() )
            mUpdateReleaseProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET BirthFrequency=?1, RandomizeRotation=?2 WHERE RefId=?3"), true );
        if ( !mUpdateRenderingProperties.isPrepared() )
            mUpdateRenderingProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET SortedRender=?1 WHERE RefId=?2"), true );
        if ( !mUpdateParticleProperties.isPrepared() )
            mUpdateParticleProperties.prepare( mWorld, _T("UPDATE 'Objects::ParticleEmitter' SET MinSpeed=?1, MaxSpeed=?2, MinMass=?3, MaxMass=?4, MinAngularSpeed=?5,")
                                                           _T("MaxAngularSpeed=?6, MinBaseScale=?7, MaxBaseScale=?8, MinLifetime=?9, MaxLifetime=?10, BaseSizeX=?11,")
                                                           _T("BaseSizeY=?12, AirResistance=?13 WHERE RefId=?14"), true );
    } // End if sandbox

    // Read queries
    if ( !mLoadEmitter.isPrepared() )
        mLoadEmitter.prepare( mWorld, _T("SELECT * FROM 'Objects::ParticleEmitter' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : getProperties()
/// <summary>
/// Retrieve the configuration structure that describes how this emitter should
/// be initialized and updated.
/// </summary>
//-----------------------------------------------------------------------------
const cgParticleEmitterProperties & cgParticleEmitterObject::getProperties( ) const
{
    return mProperties;
}

//-----------------------------------------------------------------------------
//  Name : getScriptFile()
/// <summary>
/// Retrieve the file name of the particle emitter update script that has been 
/// assigned to this emitter (if any).
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgParticleEmitterObject::getScriptFile( ) const
{
    return mScriptFile;
}

//-----------------------------------------------------------------------------
//  Name : getInnerCone ()
/// <summary>
/// Get the inner cone (Theta) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getInnerCone( ) const
{
    return mProperties.innerCone;
}

//-----------------------------------------------------------------------------
//  Name : getOuterCone ()
/// <summary>
/// Get the outer cone (Phi) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getOuterCone( ) const
{
    return mProperties.outerCone;
}

//-----------------------------------------------------------------------------
//  Name : getEmissionRadius ()
/// <summary>
/// Get the radius around the emitter position from which particles can be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getEmissionRadius( ) const
{
    return mProperties.emissionRadius;
}

//-----------------------------------------------------------------------------
//  Name : getDeadZoneRadius ()
/// <summary>
/// Get the radius around the emitter position from which particles cannot be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getDeadZoneRadius( ) const
{
    return mProperties.deadZoneRadius;
}

//-----------------------------------------------------------------------------
//  Name : getMaxSimultaneousParticles ()
/// <summary>
/// Get the maximum number of particles that can be alive at any point in time 
/// for this emitter.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgParticleEmitterObject::getMaxSimultaneousParticles( ) const
{
    return mProperties.maxSimultaneousParticles;
}

//-----------------------------------------------------------------------------
//  Name : getBirthFrequency ()
/// <summary>
/// Get the rate at which the emitter will create and release particles each
/// second.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getBirthFrequency( ) const
{
    return mProperties.birthFrequency;
}

//-----------------------------------------------------------------------------
//  Name : getRandomizedRotation ()
/// <summary>
/// Get the property that, when enabled, causes each particle's rotation 
/// angle to be randomized upon its birth. Otherwise, all particles will 
/// default to a rotation angle of 0 degrees.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getRandomizedRotation( ) const
{
    return mProperties.randomizeRotation;
}

//-----------------------------------------------------------------------------
//  Name : getSortedRender ()
/// <summary>
/// Get the property that indicates whether the emitter should sort the 
/// particles in a back to front order based on distance to the camera during
/// rendering.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitterObject::getSortedRender( ) const
{
    return mProperties.sortedRender;
}

//-----------------------------------------------------------------------------
//  Name : getParticleSpeed ()
/// <summary>
/// Get the configured range of initial speed values that a particle can be 
/// assigned during its creation.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleSpeed( ) const
{
    return mProperties.speed;
}

//-----------------------------------------------------------------------------
//  Name : getParticleMass ()
/// <summary>
/// Get the configured range of mass values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleMass( ) const
{
    return mProperties.mass;
}

//-----------------------------------------------------------------------------
//  Name : getParticleAngularSpeed ()
/// <summary>
/// Get the configured range of angular (rotation) speed values that a particle 
/// can be assigned during its creation in degrees per second.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleAngularSpeed( ) const
{
    return mProperties.angularSpeed;
}

//-----------------------------------------------------------------------------
//  Name : setParticleBaseScale ()
/// <summary>
/// Get the configured range of base scale values that a particle can be
/// assigned during its creation where 1 represents a particle whose initial
/// size matches that configured via the 'setParticleSize()' method.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleBaseScale( ) const
{
    return mProperties.baseScale;
}

//-----------------------------------------------------------------------------
//  Name : getParticleLifetime ()
/// <summary>
/// Get the configured range of lifetime (maximum age) values that a particle
/// can be assigned during its creation in seconds.
/// </summary>
//-----------------------------------------------------------------------------
const cgRangeF & cgParticleEmitterObject::getParticleLifetime( ) const
{
    return mProperties.lifetime;
}

//-----------------------------------------------------------------------------
//  Name : getParticleSize ()
/// <summary>
/// Get the configured initial size (pre-scale) of each particle in meters.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF  & cgParticleEmitterObject::getParticleSize( ) const
{
    return mProperties.baseSize;
}

//-----------------------------------------------------------------------------
//  Name : GetAirResistance ()
/// <summary>
/// Get the configured air resistance co-efficient used to compute drag for
/// each particle during simulation.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgParticleEmitterObject::getParticleAirResistance( ) const
{
    return mProperties.airResistance;
}

//-----------------------------------------------------------------------------
//  Name : setInnerCone ()
/// <summary>
/// Set the inner cone (Theta) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setInnerCone( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.innerCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, mProperties.outerCone );
        mUpdateConeAngles.bindParameter( 2, fValue );
        mUpdateConeAngles.bindParameter( 3, mReferenceId );
        
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
    mProperties.innerCone = fabsf(fValue);
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("InnerCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer cone is at LEAST as large as the inner cone
    if ( mProperties.innerCone > mProperties.outerCone )
        setOuterCone( mProperties.innerCone );
}

//-----------------------------------------------------------------------------
//  Name : setOuterCone ()
/// <summary>
/// Set the outer cone (Phi) angle of the emitter in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setOuterCone( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.outerCone == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateConeAngles.bindParameter( 1, fValue );
        mUpdateConeAngles.bindParameter( 2, mProperties.innerCone );
        mUpdateConeAngles.bindParameter( 3, mReferenceId );
        
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
    mProperties.outerCone = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("OuterCone");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner cone at MOST fits inside the outer cone
    if ( mProperties.innerCone > mProperties.outerCone )
        setInnerCone( mProperties.outerCone );
}

//-----------------------------------------------------------------------------
//  Name : setDeadZoneRadius ()
/// <summary>
/// Set the radius around the emitter position from which particles cannot be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setDeadZoneRadius( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.deadZoneRadius == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateEmissionRadii.bindParameter( 1, mProperties.emissionRadius );
        mUpdateEmissionRadii.bindParameter( 2, fValue );
        mUpdateEmissionRadii.bindParameter( 3, mReferenceId );
        
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
    mProperties.deadZoneRadius = fabsf(fValue);
    
    // Notify listeners that object data has changed.
    static const cgString strContext = _T("DeadZoneRadius");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure outer radius is at LEAST as large as the inner radius
    if ( mProperties.deadZoneRadius > mProperties.emissionRadius )
        setEmissionRadius( mProperties.deadZoneRadius );
}

//-----------------------------------------------------------------------------
//  Name : setEmissionRadius ()
/// <summary>
/// Set the radius around the emitter position from which particles can be 
/// released.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setEmissionRadius( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.emissionRadius == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateEmissionRadii.bindParameter( 1, fValue );
        mUpdateEmissionRadii.bindParameter( 2, mProperties.deadZoneRadius );
        mUpdateEmissionRadii.bindParameter( 3, mReferenceId );
        
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
    mProperties.emissionRadius = fabsf(fValue);

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("EmissionRadius");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure inner radius at MOST fits inside the outer radius
    if ( mProperties.deadZoneRadius > mProperties.emissionRadius )
        setDeadZoneRadius( mProperties.emissionRadius );
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
void cgParticleEmitterObject::setMaxSimultaneousParticles( cgUInt32 nAmount )
{
    // Is this a no-op?
    if ( mProperties.maxSimultaneousParticles == nAmount )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleCounts.bindParameter( 1, nAmount );
        mUpdateParticleCounts.bindParameter( 2, mReferenceId );
        
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
    mProperties.maxSimultaneousParticles = nAmount;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("MaxSimultaneousParticles");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setBirthFrequency ()
/// <summary>
/// Set the rate at which the emitter will create and release particles each
/// second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setBirthFrequency( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.birthFrequency == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateReleaseProperties.bindParameter( 1, fValue );
        mUpdateReleaseProperties.bindParameter( 2, mProperties.randomizeRotation );
        mUpdateReleaseProperties.bindParameter( 3, mReferenceId );
        
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
    mProperties.birthFrequency = fabsf(fValue);

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
void cgParticleEmitterObject::enableRandomizedRotation( bool bValue )
{
    // Is this a no-op?
    if ( mProperties.randomizeRotation == bValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateReleaseProperties.bindParameter( 1, mProperties.birthFrequency );
        mUpdateReleaseProperties.bindParameter( 2, bValue );
        mUpdateReleaseProperties.bindParameter( 3, mReferenceId );
        
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
    mProperties.randomizeRotation = bValue;

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
void cgParticleEmitterObject::enableSortedRender( bool bValue )
{
    // Is this a no-op?
    if ( mProperties.sortedRender == bValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRenderingProperties.bindParameter( 1, bValue );
        mUpdateRenderingProperties.bindParameter( 2, mReferenceId );
        
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
    mProperties.sortedRender = bValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("SortedRender");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSpeed ()
/// <summary>
/// Set the range of initial speed values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSpeed( cgFloat fMin, cgFloat fMax )
{
    setParticleSpeed( cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSpeed ()
/// <summary>
/// Set the range of initial speed values that a particle can be assigned 
/// during its creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSpeed( const cgRangeF & Range )
{
    // Is this a no-op?
    if ( mProperties.speed == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, Range.min );
        mUpdateParticleProperties.bindParameter( 2, Range.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.speed = Range;

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
void cgParticleEmitterObject::setParticleMass( cgFloat fMin, cgFloat fMax )
{
    setParticleMass( cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleMass ()
/// <summary>
/// Set the range of mass values that a particle can be assigned during its 
/// creation.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleMass( const cgRangeF & Range )
{
    // Is this a no-op?
    if ( mProperties.mass == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, Range.min );
        mUpdateParticleProperties.bindParameter( 4, Range.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.mass = Range;

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
void cgParticleEmitterObject::setParticleAngularSpeed( cgFloat fMin, cgFloat fMax )
{
    setParticleAngularSpeed( cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleAngularSpeed ()
/// <summary>
/// Set the range of angular (rotation) speed values that a particle can be 
/// assigned during its creation in degrees per second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleAngularSpeed( const cgRangeF & Range )
{
    // Is this a no-op?
    if ( mProperties.angularSpeed == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, Range.min );
        mUpdateParticleProperties.bindParameter( 6, Range.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.angularSpeed = Range;

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
void cgParticleEmitterObject::setParticleBaseScale( cgFloat fMin, cgFloat fMax )
{
    setParticleBaseScale( cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleBaseScale ()
/// <summary>
/// Set the range of base scale values that a particle can be assigned during 
/// its creation where 1 represents a particle whose initial size matches that
/// configured via the 'setParticleSize()' method.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleBaseScale( const cgRangeF & Range )
{
    // Is this a no-op?
    if ( mProperties.baseScale == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, Range.min );
        mUpdateParticleProperties.bindParameter( 8, Range.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.baseScale = Range;

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
void cgParticleEmitterObject::setParticleLifetime( cgFloat fMin, cgFloat fMax )
{
    setParticleLifetime( cgRangeF( fMin, fMax ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleLifetime ()
/// <summary>
/// Set the range of lifetime (maximum age) values that a particle can be 
/// assigned during its creation in seconds.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleLifetime( const cgRangeF & Range )
{
    // Is this a no-op?
    if ( mProperties.lifetime == Range )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, Range.min );
        mUpdateParticleProperties.bindParameter( 10, Range.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.lifetime = Range;

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
void cgParticleEmitterObject::setParticleSize( cgFloat fWidth, cgFloat fHeight )
{
    setParticleSize( cgSizeF( fWidth, fHeight ) );
}

//-----------------------------------------------------------------------------
//  Name : setParticleSize ()
/// <summary>
/// Set the initial size (pre-scale) of each particle in meters.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitterObject::setParticleSize( const cgSizeF & Size )
{
    // Is this a no-op?
    if ( mProperties.baseSize == Size )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, Size.width );
        mUpdateParticleProperties.bindParameter( 12, Size.height );
        mUpdateParticleProperties.bindParameter( 13, mProperties.airResistance );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.baseSize = Size;

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
void cgParticleEmitterObject::setParticleAirResistance( cgFloat fValue )
{
    // Is this a no-op?
    if ( mProperties.airResistance == fValue )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParticleProperties.bindParameter( 1, mProperties.speed.min );
        mUpdateParticleProperties.bindParameter( 2, mProperties.speed.max );
        mUpdateParticleProperties.bindParameter( 3, mProperties.mass.min );
        mUpdateParticleProperties.bindParameter( 4, mProperties.mass.max );
        mUpdateParticleProperties.bindParameter( 5, mProperties.angularSpeed.min );
        mUpdateParticleProperties.bindParameter( 6, mProperties.angularSpeed.max );
        mUpdateParticleProperties.bindParameter( 7, mProperties.baseScale.min );
        mUpdateParticleProperties.bindParameter( 8, mProperties.baseScale.max );
        mUpdateParticleProperties.bindParameter( 9, mProperties.lifetime.min );
        mUpdateParticleProperties.bindParameter( 10, mProperties.lifetime.max );
        mUpdateParticleProperties.bindParameter( 11, mProperties.baseSize.width );
        mUpdateParticleProperties.bindParameter( 12, mProperties.baseSize.height );
        mUpdateParticleProperties.bindParameter( 13, fValue );
        mUpdateParticleProperties.bindParameter( 14, mReferenceId );
        
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
    mProperties.airResistance = fValue;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("ParticleAirResistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
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
    mEmitter = CG_NULL;

    // Default the node color
    mColor = 0xFFAEBACB;

    // Default update rate to 'always' by default.
    mUpdateRate = cgUpdateRate::Always;

    // Automaticaly assign to the 'Transparent' render class.
    mRenderClassId = pScene->getRenderClassId( _T("Transparent") );

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
    mEmitter = CG_NULL;
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

    // Release particle manager.
    if ( mEmitter )
        mEmitter->scriptSafeDispose();
    mEmitter = CG_NULL;

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
void cgParticleEmitterNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // Update emitter properties.
    if ( mEmitter )
        mEmitter->setEmitterProperties( getProperties() );

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
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

    // Create the emitter manager and initialize it.
    mEmitter = new cgParticleEmitter();
    if ( !mEmitter->initialize( getScriptFile(), getProperties(), mParentScene->getRenderDriver() ) )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for particle emitter object '0x%x'.\n"), mReferenceId );
        mEmitter->scriptSafeDispose();
        mEmitter = CG_NULL;
    
    } // End if failed

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

    // Create the emitter manager and initialize it.
    mEmitter = new cgParticleEmitter();
    if ( !mEmitter->initialize( getScriptFile(), getProperties(), mParentScene->getRenderDriver() ) )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Failed to initialize internal emitter manager for particle emitter object '0x%x'.\n"), mReferenceId );
        mEmitter->scriptSafeDispose();
        mEmitter = CG_NULL;
    
    } // End if failed

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
bool cgParticleEmitterNode::registerVisibility( cgVisibilitySet * pSet, cgUInt32 nFlags )
{
    // Allow base class to perform basic tests against filters and
    // add itself to the list of visible objects where necessary.
    if ( !cgObjectNode::registerVisibility( pSet, nFlags ) )
        return false;

    // Register with the 'default' (null) material so that this node 
    // can be included in standard material batched / queue based rendering.
    if ( nFlags & cgVisibilitySearchFlags::CollectMaterials )
        pSet->addVisibleMaterial( cgMaterialHandle::Null, this );
    
    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getEmitter ()
/// <summary>
/// Retrieve the internal system particle emitter that manages the particle
/// data and oversees the update process.
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitter * cgParticleEmitterNode::getEmitter( ) const
{
    return mEmitter;
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
    // Call base class implementation first to allow
    // the node itself to update / move, etc.
    cgObjectNode::update( fElapsedTime );

    // Now allow the emitter to update.
    if ( mEmitter )
    {
        mEmitter->setEmitterMatrix( getWorldTransform(false) );
        mEmitter->update( fElapsedTime, cgVector3(0,0,0), false );

    } // End if valid emitter
}
