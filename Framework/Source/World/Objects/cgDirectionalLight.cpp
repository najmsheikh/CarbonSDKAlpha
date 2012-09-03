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
// Name : cgDirectionalLight.cpp                                             //
//                                                                           //
// Desc : Directional light source classes.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgDirectionalLight Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgDirectionalLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgTexturePool.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Math/cgMathUtility.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgDirectionalLightObject::mInsertDirectionalLight;
cgWorldQuery cgDirectionalLightObject::mUpdateSplitOverlap;
cgWorldQuery cgDirectionalLightObject::mUpdateShadowConfigSources;
cgWorldQuery cgDirectionalLightObject::mUpdateShadowRate;
cgWorldQuery cgDirectionalLightObject::mLoadDirectionalLight;

///////////////////////////////////////////////////////////////////////////////
// cgDirectionalLightObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgDirectionalLightObject () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgDirectionalLightObject::cgDirectionalLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgLightObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mSplitOverlapSize         = 0.0f;
    mShadowUpdateRate         = 0;
}

//-----------------------------------------------------------------------------
//  Name : cgDirectionalLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDirectionalLightObject::cgDirectionalLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgLightObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgDirectionalLightObject * pObject = (cgDirectionalLightObject*)pInit;
    mSplitOverlapSize     = pObject->mSplitOverlapSize;
    mShadowUpdateRate     = pObject->mShadowUpdateRate;
    mSplitDistances       = pObject->mSplitDistances;
    //m_aShadowConfig         = pObject->m_aShadowConfig;
}

//-----------------------------------------------------------------------------
// Name : ~cgDirectionalLightObject () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
cgDirectionalLightObject::~cgDirectionalLightObject()
{
    // Clean up
    dispose( false );

}

//-----------------------------------------------------------------------------
// Name : dispose () (Virtual)
// Desc : Release any resources allocated by this object.
//-----------------------------------------------------------------------------
void cgDirectionalLightObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear containers.
    //m_aShadowConfig.clear();
    mSplitDistances.clear();

    // Dispose base.
    if ( bDisposeBase == true )
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
cgWorldObject * cgDirectionalLightObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgDirectionalLightObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgDirectionalLightObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgDirectionalLightObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgDirectionalLightObject::getDatabaseTable( ) const
{
    return _T("Objects::DirectionalLight");
}

//-----------------------------------------------------------------------------
//  Name : setShadowUpdateRate ()
/// <summary>
/// Set the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgDirectionalLightObject::setShadowUpdateRate( cgUInt32 nRate )
{
    // Is this a no-op?
    if ( mShadowUpdateRate == nRate )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateShadowRate.bindParameter( 1, nRate );
        mUpdateShadowRate.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateShadowRate.step( true ) == false )
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
//  Name : getShadowUpdateRate ()
/// <summary>
/// Get the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDirectionalLightObject::getShadowUpdateRate( ) const
{
    return mShadowUpdateRate;
}

//-----------------------------------------------------------------------------
//  Name : getShadowSplitCount ()
/// <summary>
/// Retrieve the number of split frustums that are configured for this
/// light source.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgDirectionalLightObject::getShadowSplitCount( ) const
{
    return 0; //(cgUInt32)m_aShadowConfig.size();
}

//-----------------------------------------------------------------------------
//  Name : getShadowSplitDistance ()
/// <summary>
/// Retrieve the configured distance for the specified PSSM shadow split 
/// frustum.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgDirectionalLightObject::getShadowSplitDistance( cgUInt32 nFrustum ) const
{
    return mSplitDistances[nFrustum];
}

/*//-----------------------------------------------------------------------------
//  Name : GetShadowFrustumConfig ()
/// <summary>
/// Get the base shadow frustum configuration parameters.
/// </summary>
//-----------------------------------------------------------------------------
const cgShadowGeneratorConfig & cgDirectionalLightObject::GetShadowFrustumConfig( cgUInt32 nFrustum ) const
{
    return m_aShadowConfig[nFrustum];
}*/

//-----------------------------------------------------------------------------
//  Name : getShadowSplitOverlap ()
/// <summary>
/// Retrieve the amount of overlap that will be included for PSSM shadow
/// split frustums in world space units.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgDirectionalLightObject::getShadowSplitOverlap( ) const
{
    return mSplitOverlapSize;
}

//-----------------------------------------------------------------------------
// Name : getLocalBoundingBox ()
// Desc : Retrieve the bounding box of this object.
//-----------------------------------------------------------------------------
cgBoundingBox cgDirectionalLightObject::getLocalBoundingBox( )
{
    // Always entire scene
    return cgBoundingBox( -FLT_MAX, -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DirectionalLightObject )
        return true;

    // Supported by base?
    return cgLightObject::queryReferenceType( type );
}

/* ToDo: Remove on completion.
//-----------------------------------------------------------------------------
// Name : Deserialize ()
// Desc : Initialize the object based on the XML data pulled from the 
//        environment / scene definition file.
//-----------------------------------------------------------------------------
bool cgDirectionalLightObject::Deserialize( const cgXMLNode & InitData, cgSceneLoader * pLoader )
{
    // Allow base class to process data first
    if ( cgLightObject::Deserialize( InitData, pLoader ) == false )
        return false;

    // ToDo: Temporarily hardcode frustum update rate to 30fps.
    mShadowUpdateRate = 30;

    // ToDo: Temporary - setup an initial test frustum split
    m_nSplitCount     = 3;
    m_pSplitDistances = new cgFloat[m_nSplitCount];
    m_ppFrustums      = new cgShadowGenerator*[m_nSplitCount];
    m_ppFrustums[0]   = new cgShadowGenerator( this, 0 );
    m_ppFrustums[1]   = new cgShadowGenerator( this, 1 );
    m_ppFrustums[2]   = new cgShadowGenerator( this, 2 );

    const cgTexturePool::Config & Settings = mParentScene->GetShadowMaps()->GetConfig();
	m_ppFrustums[0]->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution );    
    m_ppFrustums[1]->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution / 2 );
    m_ppFrustums[2]->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution / 2 );

    // ToDo: Get these values from PSF file
	bool bTestClose = false;
	if ( bTestClose )
	{
		m_pSplitDistances[0] = 10.0f;
		m_pSplitDistances[1] = 50.0f;
		m_pSplitDistances[2] = 100.0f;
		m_fShadowMinDistance = 80.0f;
		m_fShadowMaxDistance = 100.0f;
		mSplitOverlapSize  = 2.0f;
	}
	else
	{
		m_pSplitDistances[0] = 300.0f;
		m_pSplitDistances[1] = 600.0f;
		m_pSplitDistances[2] = 1500.0f;
		m_fShadowMinDistance = 1250.0f;
		m_fShadowMaxDistance = 1499.0f;
		mSplitOverlapSize  = 20.0f;
	}

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : InitObject ()
// Desc : Initialize the object (after deserialization / setup) as required.
//-----------------------------------------------------------------------------
bool cgDirectionalLightObject::InitObject( bool bAutoAddObject = true )
{
    // Allow base class to process
    if ( cgLightObject::InitObject( bAutoAddObject ) == false )
        return false;
    
    // Initialize the frustums as required
    const cgTexturePool::Config & Settings = mParentScene->GetShadowMaps()->GetConfig();

    // Initialize shadow frustums ready for rendering
    for ( cgUInt32 i = 0; i < m_nSplitCount; ++i )
    {
        cgCameraNode * pFrustumCamera = m_ppFrustums[i]->getCamera();

        // Initialize view matrix only for parallel split frustums.
        // Projection matrix will be calculated based on light source visibility
        // set at a later point.
        pFrustumCamera->SetObjectMatrix( GetObjectMatrix() );
        
        // Set additional frustum properties
        // ToDo: Uncomment
        //m_ppFrustums[i]->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution );
    
    } // Next Frustum

    // ToDo: Needs to recompute shadow frustums on SetObjectMatrix() (surely this isn't required for directionals?)

    // Success!
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;
    
    // Retrieve useful values
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, pIssuer->getPosition( false ), 2.5f );
    cgFloat fTolerance  = 2.0f * fZoomFactor;
    
    // Compute vertices for the tip of the directional light source arrow
    cgVector3 Points[5];
    cgFloat fSize = fZoomFactor * 7.5f;
    Points[0] = cgVector3( 0, 0, fZoomFactor * 30.0f );
    Points[1] = cgVector3( -fSize,  fSize, fZoomFactor * 20.0f );
    Points[2] = cgVector3( -fSize, -fSize, fZoomFactor * 20.0f );
    Points[3] = cgVector3(  fSize, -fSize, fZoomFactor * 20.0f );
    Points[4] = cgVector3(  fSize,  fSize, fZoomFactor * 20.0f );

    // ...and indices.
    cgUInt32 Indices[12];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;

    // First check to see if the ray intersects the arrow tip pyramid
    cgFloat   t, tMin = FLT_MAX;
    bool      bIntersect = false;
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
    cgPlane   Plane;
    cgPlane::fromPointNormal( Plane, cgVector3( 0, 0, fZoomFactor * 20.0f ), cgVector3( 0, 0, -1 ) );
    if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, false, false ) == true )
    {
        // Check to see if it falls within the quad at the base of the pyramid
        // (Simple to do when working in object space as we are here).
        cgVector3 vIntersect = vOrigin + (vDir * t);
        if ( fabsf( vIntersect.x ) < fSize + fTolerance && fabsf( vIntersect.y ) < fSize + fTolerance )
        {
            bIntersect = true;
            if ( t < tMin ) tMin = t;
        
        } // End if intersects quad

    } // End if intersects plane

    // Now check to see if the ray intersects the arrow "stalk" (simple AABB test when in object space)
    fSize = (fZoomFactor * 4.0f) + fTolerance;
    cgBoundingBox StalkAABB( cgVector3( -fSize, -fSize, -fTolerance ), cgVector3( fSize, fSize, (fZoomFactor * 20.0f) + fTolerance ) ); 
    if ( StalkAABB.intersect( vOrigin, vDir, t, false ) == true )
    {
        bIntersect = true;
        if ( t < tMin ) tMin = t;

    } // End if intersects stalk aabb

    // Finally check for intersection against the base plate / plane.
    // In this case we can intersect from either side, so enable "bidirectional" test.
    fSize = fZoomFactor * 20.0f;
    Plane = cgPlane( 0.0f, 0.0f, 1.0f, 0.0f );
    if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, true, false ) == true )
    {
        // Check to see if it falls within the quad of the base plate / plane
        // (Simple to do when working in object space as we are here).
        cgVector3 vIntersect = vOrigin + (vDir * t);
        if ( fabsf( vIntersect.x ) < (fSize * 2.0f) + fTolerance && fabsf( vIntersect.y ) < fSize + fTolerance )
        {
            bIntersect = true;
            if ( t < tMin ) tMin = t;
        
        } // End if intersects quad

    } // End if intersects plane

    // Return final intersection distance (if we hit anything)
    if ( bIntersect == true )
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
void cgDirectionalLightObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgShadedVertex  Points[16];
    cgUInt32        Indices[35];
    bool            bCull;
    cgUInt32        i;

    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Set the object's transformation matrix to the driver (light
    // representation will be constructed in object space)
    cgTransform InverseObjectTransform;
    const cgTransform & ObjectTransform = pIssuer->getWorldTransform( false );
    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
    pDriver->setWorldTransform( ObjectTransform );
    
    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    cgFloat  fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );
    cgUInt32 nColor      = (pIssuer->isSelected() == true) ? 0xFFFFFFFF : 0xFFFFF600;
    bool     bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    for ( i = 0; i < 16; ++i )
        Points[i].color = nColor;

    // Compute vertices for the tip of the directional light source arrow
    cgFloat fSize = fZoomFactor * 7.5f;
    Points[0].position = cgVector3( 0, 0, fZoomFactor * 30.0f );
    Points[1].position = cgVector3( -fSize,  fSize, fZoomFactor * 20.0f );
    Points[2].position = cgVector3( -fSize, -fSize, fZoomFactor * 20.0f );
    Points[3].position = cgVector3(  fSize, -fSize, fZoomFactor * 20.0f );
    Points[4].position = cgVector3(  fSize,  fSize, fZoomFactor * 20.0f );
    
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

            // Now we'll render the base of the arrow, the "stalk" and the back plate.
            // So that we can construct in object space, transform camera culling details
            // into object space too.
            cgVector3 vCameraLook, vCameraPos;
            InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis(false) );
            InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition(false) );
            cgVector3::normalize( vCameraLook, vCameraLook );
            
            // Construct the vertices. First the "stalk" / cube vertices.
            // Render each of the cube faces as line lists (manual back face culling)
            fSize              = fZoomFactor * 4.0f;
            Points[0].position = cgVector3( -fSize,  fSize, 0.0f );
            Points[1].position = cgVector3( -fSize,  fSize, fZoomFactor * 20.0f );
            Points[2].position = cgVector3(  fSize,  fSize, fZoomFactor * 20.0f );
            Points[3].position = cgVector3(  fSize,  fSize, 0.0f );
            Points[4].position = cgVector3( -fSize, -fSize, 0.0f );
            Points[5].position = cgVector3( -fSize, -fSize, fZoomFactor * 20.0f );
            Points[6].position = cgVector3(  fSize, -fSize, fZoomFactor * 20.0f );
            Points[7].position = cgVector3(  fSize, -fSize, 0.0f );

            // Stalk +X Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 1,0,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 1,0,0 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            cgUInt32 nCount = 0;
            if ( bCull == false )
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
            if ( bCull == false )
            {
                Indices[nCount++] = 1; Indices[nCount++] = 0;
                Indices[nCount++] = 0; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 1;

            } // End if !cull

            // -Z Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,0,-1 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), 0.0f ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 0; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 7;
                Indices[nCount++] = 7; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 0;

            } // End if !cull

            // +Y Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,1,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,1,0 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 0; Indices[nCount++] = 1;
                Indices[nCount++] = 1; Indices[nCount++] = 2;
                Indices[nCount++] = 2; Indices[nCount++] = 3;
                Indices[nCount++] = 3; Indices[nCount++] = 0;

            } // End if !cull

            // -Y Face
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,-1,0 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,-1,0 ), fSize ) != cgPlaneQuery::Front);
            
            // Build indices
            if ( bCull == false )
            {
                Indices[nCount++] = 7; Indices[nCount++] = 6;
                Indices[nCount++] = 6; Indices[nCount++] = 5;
                Indices[nCount++] = 5; Indices[nCount++] = 4;
                Indices[nCount++] = 4; Indices[nCount++] = 7;

            } // End if !cull

            // Now construct the "base" of the arrow tip pyramid.
            if ( bOrtho )
                bCull = (cgVector3::dot( cgVector3( 0,0,-1 ), vCameraLook ) >= 0.0f );
            else
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), fZoomFactor * 20.0f ) != cgPlaneQuery::Front);
            
            // Build base vertices / indices
            if ( bCull == false )
            {
                fSize               = fZoomFactor * 7.5f;
                Points[8].position  = cgVector3( -fSize, -fSize, fZoomFactor * 20.0f );
                Points[9].position  = cgVector3( -fSize,  fSize, fZoomFactor * 20.0f );
                Points[10].position = cgVector3(  fSize,  fSize, fZoomFactor * 20.0f );
                Points[11].position = cgVector3(  fSize, -fSize, fZoomFactor * 20.0f );

                Indices[nCount++] = 8 ; Indices[nCount++] = 9;
                Indices[nCount++] = 9 ; Indices[nCount++] = 10;
                Indices[nCount++] = 10; Indices[nCount++] = 11;
                Indices[nCount++] = 11; Indices[nCount++] = 8;

            } // End if !cull

            // Always add the plate / plane
            fSize               = fZoomFactor * 20.0f;
            Points[12].position = cgVector3( -fSize * 2.0f, -fSize, 0.0f );
            Points[13].position = cgVector3( -fSize * 2.0f,  fSize, 0.0f );
            Points[14].position = cgVector3(  fSize * 2.0f,  fSize, 0.0f );
            Points[15].position = cgVector3(  fSize * 2.0f, -fSize, 0.0f );
            
            Indices[nCount++] = 12; Indices[nCount++] = 13;
            Indices[nCount++] = 13; Indices[nCount++] = 14;
            Indices[nCount++] = 14; Indices[nCount++] = 15;
            Indices[nCount++] = 15; Indices[nCount++] = 12;

            // Draw the rest of the light source representation
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 16, nCount / 2, Indices, cgBufferFormat::Index32, Points );

        } // End if begun pass

        // We have finished rendering
        pShader->endTechnique();
    
    } // End if begun technique

    // Call base class implementation last.
    cgLightObject::sandboxRender( pCamera, pVisData, bWireframe, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgDirectionalLightObject::applyObjectRescale( cgFloat fScale )
{
    cgToDoAssert( "Carbon General", "cgDirectionalLightObject::applyObjectRescale()" );
    // mSplitOverlapSize ??
    // mSplitDistances ??

    // Call base class implementation.
    cgLightObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
// Name : getLightType () (Virtual)
// Desc : Determine the type of this light source.
//-----------------------------------------------------------------------------
cgLightObject::LightType cgDirectionalLightObject::getLightType( ) const
{
    return Light_Directional;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgDirectionalLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("DirectionalLightObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertDirectionalLight.bindParameter( 1, mReferenceId );
        mInsertDirectionalLight.bindParameter( 2, mSplitOverlapSize );
        mInsertDirectionalLight.bindParameter( 3, mShadowUpdateRate );
        
        // ToDo: 9999 - Frustum configuration sources
        
        // Execute
        if ( !mInsertDirectionalLight.step( true ) )
        {
            cgString strError;
            mInsertDirectionalLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for directional light object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("DirectionalLightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("DirectionalLightObject::insertComponentData") );

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
bool cgDirectionalLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the light data.
    prepareQueries();
    mLoadDirectionalLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadDirectionalLight.step( ) || !mLoadDirectionalLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadDirectionalLight.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for directional light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for directional light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadDirectionalLight.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadDirectionalLight;

    // Update our local members
    mLoadDirectionalLight.getColumn( _T("SplitOverlapSize"), mSplitOverlapSize );
    mLoadDirectionalLight.getColumn( _T("ShadowUpdateRate"), mShadowUpdateRate );

    // Call base class implementation to read remaining data.
    if ( !cgLightObject::onComponentLoading( e ) )
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
void cgDirectionalLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertDirectionalLight.isPrepared() == false )
            mInsertDirectionalLight.prepare( mWorld, _T("INSERT INTO 'Objects::DirectionalLight' VALUES(?1,?2,?3,NULL)"), true );
        if ( mUpdateSplitOverlap.isPrepared() == false )
            mUpdateSplitOverlap.prepare( mWorld, _T("UPDATE 'Objects::DirectionalLight' SET SplitOverlapSize=?1 WHERE RefId=?2"), true );
        //if ( mUpdateShadowConfigSources.isPrepared() == false )
            //mUpdateShadowConfigSources.prepare( mWorld, _T("UPDATE 'Objects::PointLight' SET ShadowFrustumsLinked=?1 WHERE RefId=?2"), true );
        if ( mUpdateShadowRate.isPrepared() == false )
            mUpdateShadowRate.prepare( mWorld, _T("UPDATE 'Objects::DirectionalLight' SET ShadowUpdateRate=?1 WHERE RefId=?2"), true );

        // ToDo: 9999 - Shadow Config
    
    } // End if sandbox

    // Read queries
    if ( mLoadDirectionalLight.isPrepared() == false )
        mLoadDirectionalLight.prepare( mWorld, _T("SELECT * FROM 'Objects::DirectionalLight' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgDirectionalLightNode Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgDirectionalLightNode () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgDirectionalLightNode::cgDirectionalLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgLightNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults
    m_fShadowTimeSinceLast = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgDirectionalLightNode () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDirectionalLightNode::cgDirectionalLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    m_fShadowTimeSinceLast = 0.0f;
}

//-----------------------------------------------------------------------------
// Name : ~cgDirectionalLightNode () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
cgDirectionalLightNode::~cgDirectionalLightNode()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
// Name : dispose () (Virtual)
// Desc : Release any resources allocated by this object.
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    for ( cgUInt32 i = 0; i < mFrustums.size(); ++i )
    {
        if ( mFrustums[i] )
            mFrustums[i]->scriptSafeDispose();
    
    } // Next frustum
    mFrustums.clear();
    
    // dispose base.
    if ( bDisposeBase == true )
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
bool cgDirectionalLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DirectionalLightNode )
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
cgObjectNode * cgDirectionalLightNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgDirectionalLightNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgDirectionalLightNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgDirectionalLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::postCreate( )
{
    // Call base class implementation.
    if ( !cgLightNode::postCreate( ) )
        return false;

	// Is this a dynamic ambient lighting source? (i.e., will create RSMs?)
	bool bGenerateRSMs = isIndirectSource();

    // Create required number of shadow frustums.
    cgUInt32 nSplitCount = getShadowSplitCount();
    mFrustums.resize( nSplitCount );

    // Initialize shadow frustums ready for rendering
    for ( cgUInt32 i = 0; i < nSplitCount; ++i )
    {
        cgShadowGenerator * pFrustum = new cgShadowGenerator( this, i );
        
        // Allow frustum to initialize internal resources as necessary.
        if ( !pFrustum->initialize() )
            return false;

        // Initialize view matrix only for parallel split frustums.
        // Projection matrix will be calculated based on light source visibility
        // set at a later point.
        cgCameraNode * pFrustumCamera = pFrustum->getCamera();
        pFrustumCamera->setWorldTransform( getWorldTransform( false ) );
        
        // Store
        mFrustums[i] = pFrustum;
    
    } // Next Frustum

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    cgUInt32 nShadowUpdateRate = getShadowUpdateRate();
    if ( nShadowUpdateRate > 0 )
        m_fShadowTimeSinceLast = -cgMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)nShadowUpdateRate );

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
void cgDirectionalLightNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : boundsInVolume () (Virtual)
// Desc : Does the specified bounding box fall within the volume of this light 
//        source?
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::boundsInVolume( const cgBoundingBox & AABB )
{
    // Bounding box always intersects directional light volume
    return true;
}

//-----------------------------------------------------------------------------
// Name : pointInVolume () (Virtual)
// Desc : Does the specified point fall within the volume of this light source?
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::pointInVolume( const cgVector3 & Point, cgFloat fErrRadius /* = 0.0f */ )
{
    // Point is always within directional light volume.
    return true;
}

//-----------------------------------------------------------------------------
// Name : frustumInVolume () (Virtual)
// Desc : Does the specified frustum intersect the volume of this light source?
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::frustumInVolume( const cgFrustum & Frustum )
{
    // Frustum always intersects directional light volume.
    return true;
}

//-----------------------------------------------------------------------------
// Name : testObjectShadowVolume( ) (Virtual)
// Desc : Determine if the specified object can potentially cast a shadow into
//        the view frustum based on the requirements of this light source.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::testObjectShadowVolume( cgObjectNode * pObject, const cgFrustum & ViewFrustum )
{
    cgBoundingBox ObjectAABB = pObject->getBoundingBox();

    // Construct a bounding sphere around the object (this will
    // be swept and tested against the frustum).
    cgVector3 vSphereCenter = ObjectAABB.getCenter();
    cgFloat   fRadius       = cgVector3::length( ObjectAABB.getExtents() );

    // Test to see if the sphere, swept along the light direction, intersects the frustum.
    if ( ViewFrustum.testSweptSphere( vSphereCenter, fRadius, getZAxis( false ) ) == true )
        return true;

    // Cannot cast a shadow into the view frustum
    return false;
}

//-----------------------------------------------------------------------------
// Name : computeVisibility () (Virtual)
// Desc : Compute the visibility set from the point of view of this light.
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::computeVisibility( )
{
    // Shadow casting is not *currently* supported for directional lights.
    mComputeShadows = false;
    
    // Call base class implementation last
    cgLightNode::computeVisibility();
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
bool cgDirectionalLightNode::registerVisibility( cgVisibilitySet * pSet, cgUInt32 nFlags )
{
    // First test filters.
    if ( (nFlags & cgVisibilitySearchFlags::MustCastShadows) )
        return false;
    if ( (nFlags & cgVisibilitySearchFlags::MustRender) && !isRenderable() )
        return false;

    // We're always visible. Add to the set.
    pSet->addVisibleLight( this );

    // We modified the visibility set.
    return true;
}

//-----------------------------------------------------------------------------
// Name : computeLevelOfDetail () (Virtual)
// Desc : Computes level of detail settings for this light source.
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
{
    // Set initial values
    cgLightObject * pData = (cgLightObject*)mReferencedObject;
    mComputeShadows  = pData->isShadowSource();
	mComputeSpecular = pData->isSpecularSource();

    // LOD is disabled.
	mShadowAttenuation   = 1.0f;
	mSpecularAttenuation = 1.0f;
	mShadowLODScale      = 1.0f;

    // Disable shadows if not required
    if ( mFrustums.size() == 0 )
        mComputeShadows = false;

    // If shadows are enabled
	if ( mComputeShadows )
	{
        cgTexturePoolResourceDesc::Array aDescriptions;

		// Get the lighting manager
		cgLightingManager * pManager = mParentScene->getLightingManager();

		// Ask the lighting manager for the best shadow technique given LOD
		cgInt32 nIndex = pManager->getShadowSettings( mShadowDetailLevels, false, mShadowSystemSettings, aDescriptions, mShadowFlags ); 
		if ( nIndex >= 0 )
		{
			// Get the light level settings based on the index selected by the lighting manager
			const cgShadowSettingsLight & SettingsLight = mShadowSettings[ nIndex ];

			// Update resource resolution(s) based on current LOD settings
			cgUInt32 nMaxResolution = pManager->getMaxShadowResolution( false );
			cgUInt32 nResolution = 1 << (nMaxResolution - (SettingsLight.resolutionAdjust + mShadowSystemSettings.resolutionAdjust));
			for ( cgUInt32 i = 0; i < aDescriptions.size(); i++ )
			{
				aDescriptions[ i ].bufferDesc.width  = nResolution;
				aDescriptions[ i ].bufferDesc.height = nResolution;

            } // Next resource

			// Apply settings to the child shadow frustums / generators.
			for ( cgUInt32 i = 0; i < mFrustums.size(); ++i )
				mFrustums[i]->update( nResolution, mShadowSystemSettings, SettingsLight, aDescriptions, mShadowFlags );

		} // End valid settings 
		else
		{
			// We could not get valid shadow settings, so turn off shadows
			mComputeShadows = false;
		
        } // End if invalid

	} // End shadows
}

/*//-----------------------------------------------------------------------------
// Name : ComputeShadowSets( ) (Virtual)
// Desc : Creates the shadow mapping visibility sets that are also found within
//        the specified visibility set (i.e. main player camera). This 
//        essentially narrows down the list of visible objects from the
//        point of view of the light source to only those that can also be
//        seen by the camera.
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::ComputeShadowSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::ComputeShadowSets( pCamera );

    // If it was determined that we should calculate shadows,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeShadows == true )
    {
        bool bCalcShadows = false;

        // Get access to required systems / objects
        cgRenderDriver * pDriver = mParentScene->getRenderDriver();
        cgCameraNode   * pCamera = pDriver->getCamera();

        // Process each shadow frustum.
        for ( cgUInt32 i = 0; i < mFrustums.size(); ++i )
        {
            cgShadowGenerator * pFrustum = mFrustums[i];

            // Select the frustum split's near and far plane (camera space distance)
            cgFloat fZFar  = getShadowSplitDistance( i );
            cgFloat fZNear = ( i == 0 ) ? fZNear = pCamera->GetNearClip() : getShadowSplitDistance( i - 1 );

		    // Adjust the near/far planes to account for overlap zones when blending splits
            cgFloat fOverlap = getShadowSplitOverlap();
            if ( fOverlap > 0.0f )
            {
                cgFloat fHalfRegion = fOverlap * 0.5f;
                if ( i == 0 )
                {
                    fZFar += fHalfRegion;
                
                } // End if first split
                else if ( i == (mFrustums.size() - 1) )
                {
                    fZNear -= fHalfRegion;
                
                } // End if last split
                else    
                {
                    fZFar  += fHalfRegion;
                    fZNear -= fHalfRegion;
                
                } // End if other splits
    		
            } // End if blending

            // First allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            // ToDo: Support fixed split distances mixed with non-fixed split distances or one or the other?
            cgLightObject * pLight = (cgLightObject*)mReferencedObject;
            bCalcShadows |= pFrustum->ComputeVisibilitySet( pCamera, true, fZNear, fZFar );

        } // Next frustum

        // Is there /still/ any need to assume that this is a shadow source?
        if ( bCalcShadows == false )
            mComputeShadows = false;

    } // End if mComputeShadows
}*/

/*//-----------------------------------------------------------------------------
//  Name : ComputeIndirectSets( ) (Virtual)
/// <summary>
/// Creates the rsm visibility sets
/// </summary>
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::ComputeIndirectSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::ComputeIndirectSets( pCamera );

    // If it was determined that we should calculate rsms,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( mComputeIndirect )
    {
		// Get the list of grid AABBs from the lighting manager.
		//cgLightingManager * pManager = mParentScene->getLightingManager();
		//pManager->GetGrids();
		
		// Each frustum should map up to one grid AABB?
		// This can be a problem if we want to support randomly placed (outdoor)
		// static grids. A better bet would seem to be to somehow figure out a 
		// way to take advantange of PSSM? The trick here is how to deal with 
		// cases where, if a grid reads two or more PSSM maps, how to avoid sampling
		// the same RSM location more than once and overlighting the RH sample point. 
		// The problem here is that due to split frustum overlaps, the same shadowmap
		// can contain some of the same samples (where some objects exist in more than one
		// shadow map). 
		// SOLUTION 1: Perhaps an atlased PSSM system works best here? That is, assume
		// 4 maps and select the "best" map from the atlas for a given sample point at
		// the time of processing/injecting the RSM samples. This should guarantee that
		// a given location does not oversample the same objects from the RSM. An alternative
		// in a multipass situation (as is currently the case) is to, for each RH when sampling,
		// determine whether the current PSSM is the best one for it. If not, reject all samples
		// from it, making the assumption that the next one in line will contain what it wants. 
		// SOLUTION 2: Just use a single RSM that covers the largest grid. Grid points will 
		//             project to the closest locations and sample in a disc. Seems the best bet.
        bool bCalcIndirect = false;

		// Process each active frustum
        for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); ++i )
        {
            cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator * )mIndirectFrustums[i];

            // First allow the frustum to compute its own local shadow set information.
            // This will return 'true' if it is deemed necessary to compute shadows
            // or at least render from a pool shadow map.
            bCalcIndirect |= pFrustum->ComputeVisibilitySet( pCamera, false );

        } // Next frustum

        // Is there /still/ any need to assume that this is a reflective shadow source?
        if ( !bCalcIndirect )
            mComputeIndirect = false;

    } // End if mComputeIndirect
}*/

//-----------------------------------------------------------------------------
//  Name : updateIndirectSettings () (Virtual)
/// <summary>
/// Updates settings for indirect lighting
/// </summary>
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::updateIndirectSettings( )
{
    cgTexturePoolResourceDesc::Array aDescriptions;

	// Get the lighting manager
	cgLightingManager * pManager = getScene()->getLightingManager();

	// Ask the lighting manager for the best reflective shadow technique given LOD
	cgInt32 nIndex = pManager->getShadowSettings( mIndirectDetailLevels, true, mIndirectSystemSettings, aDescriptions, mIndirectFlags ); 
	if ( nIndex >= 0 )
	{
		// Get the light level settings based on the index selected by the lighting manager
		const cgShadowSettingsLight & SettingsLight = mIndirectSettings[ nIndex ];

		// Update resource resolution(s) based on current LOD settings
		cgUInt32 nMaxResolution = pManager->getMaxShadowResolution( false );
		cgUInt32 nResolution = 1 << (nMaxResolution - (SettingsLight.resolutionAdjust + mShadowSystemSettings.resolutionAdjust));
		for ( cgUInt32 i = 0; i < aDescriptions.size(); i++ )
		{
			aDescriptions[ i ].bufferDesc.width  = nResolution;
			aDescriptions[ i ].bufferDesc.height = nResolution;
		
        } // Next resource

		// If we are downsampling the RSM, update the resolutions of the recipient targets based on the tap count
		if ( mIndirectSystemSettings.boxFilter && aDescriptions.size() > 4 )
		{
			cgAssert( mIndirectSystemSettings.primarySamples > 0 );
			for ( cgUInt32 i = 4; i < aDescriptions.size(); i++ )
			{
				aDescriptions[ i ].bufferDesc.width  = mIndirectSystemSettings.primarySamples;
				aDescriptions[ i ].bufferDesc.height = mIndirectSystemSettings.primarySamples;
			
            } // Next recipient
		
        } // End if downsample

		// Apply settings.
		for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); ++i )
		{
			if ( mIndirectFrustums[i] )
				mIndirectFrustums[i]->update( nResolution, mIndirectSystemSettings, SettingsLight, aDescriptions, mIndirectFlags );

		} // Next frustum

	} // End valid settings 
	else
	{
		// Turn off indirect lighting
		mComputeIndirect = false;
	
    } // End if invalid
}

//-----------------------------------------------------------------------------
// Name : reassignShadowMaps() (Virtual)
// Desc : Allows the light to determine if it is necessary to partake in
//        the assignment of new shadow maps at this stage, or if it can re-use
//        shadow maps that it may have populated earlier.
// Note : Returning true indicates that the light does not require further
//        shadow map assignment processing (i.e. it reused pooled maps).
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::reassignShadowMaps( cgTexturePool * pPool )
{
    bool bNoFurtherAssignment = true;

    // Is there any need to re-assign?
    if ( !isShadowSource() )
    {
        // Release any resources back to the pool.
        for ( cgUInt32 i = 0; i < mFrustums.size(); ++i )
            mFrustums[ i ]->releaseResources();
		return true;
    
    } // End if not a caster
 	
	// Attempt resource reassignment for each frustum
    for ( cgUInt32 i = 0; i < mFrustums.size(); ++i )
    {
        cgShadowGenerator * pFrustum = mFrustums[i];

		// Attempt resource reassignment (if we cannot re-use the resources, process further)
        if ( !pFrustum->reassignResources( pPool ) )
            bNoFurtherAssignment = false;
        
    } // Next frustum

    // We need to assign further maps?
    return bNoFurtherAssignment;
}

//-----------------------------------------------------------------------------
// Name : beginShadowFill() (Virtual)
// Desc : Assuming the light would like to opt in to the population of any
//        assigned shadow maps, this method is called to start that process
//        and should return the total number of fill passes that will be
//        required (i.e. perhaps because the light maintains multiple shadow
//        maps).
//-----------------------------------------------------------------------------
cgInt32 cgDirectionalLightNode::beginShadowFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic).
    // Base class will return -1 if this is not a shadow source.
    if ( cgLightNode::beginShadowFill( pPool ) < 0 )
        return -1;

    // ToDo: 6767 - Determine if it is an appropriate time for us to update based on
    // our shadow map update period limiting property.
    /*cgUInt32 nShadowUpdateRate = getShadowUpdateRate();
    if ( nShadowUpdateRate > 0 )
    {
        m_fShadowTimeSinceLast += cgTimer::getInstance()->GetTimeElapsed();
        if ( m_fShadowTimeSinceLast < (1.0f / (cgFloat)nShadowUpdateRate) )
            m_bShadowTimedUpdate = false;
        else
            m_fShadowTimeSinceLast = 0.0f;

    } // End if limit updates*/

    // For each frustum...
    mShadowPasses.reserve( mFrustums.size() ); // Assume 1 pass per frustum as a minimum
	for ( cgUInt32 i = 0; i < mFrustums.size(); i++ )
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
// Name : beginShadowFillPass() (Virtual)
// Desc : Called in order to begin an individual pass of the shadow fill
//        process.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::beginShadowFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( !cgLightNode::beginShadowFillPass( nPass, pRenderSetOut ) )
        return false;

    // Get the generator for the current shadow fill pass.
    const LightingOpPass & Pass = mShadowPasses[ nPass ];
    cgShadowGenerator * pFrustum = mFrustums[ Pass.frustum ];

    // If this is the first child pass for this frustum, begin writing
	if ( Pass.subPass == 0 )
        pFrustum->beginWrite( true, true );

    // Begin the requested pass.
	if ( pFrustum->beginWritePass( Pass.subPass ) )
    {
        // Render only objects that exist within frustum's shadow set.
        pRenderSetOut = pFrustum->getVisibilitySet();

        // We have begun this pass
        mCurrentShadowPass = nPass;
        return true;

    } // End if filling

    // Did not fill shadow map.
    return false;
}

//-----------------------------------------------------------------------------
// Name : endShadowFillPass() (Virtual)
// Desc : Called in order to end an individual pass of the shadow fill
//        process.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::endShadowFillPass( )
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

    // Valid
    return true;
}

//-----------------------------------------------------------------------------
// Name : endShadowFill() (Virtual)
// Desc : Called in order to signify that the shadow fill process is complete.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::endShadowFill( )
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
bool cgDirectionalLightNode::updateLightConstants()
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
    
    // Setup the light source attenuation data.
    pLightData->direction      = getZAxis(false);
    pLightData->attenuation.x  = 0;
    pLightData->attenuation.y  = 1;
    pLightData->attenuation.z  = 1;
    pLightData->attenuation.w  = 0;
    pLightData->clipDistance.x = 0;
    pLightData->clipDistance.y = 1;

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pLightBuffer->unlock();

    // Call base class implementation LAST
    return cgLightNode::updateLightConstants();
}

//-----------------------------------------------------------------------------
// Name : beginLighting() (Virtual)
// Desc : Assuming the light would like to opt in to the lighting process
//        this method is called to start that process and should return the 
//        total number of lighting passes that will be required (i.e. perhaps 
//        because the light process a single frustum at a time).
//-----------------------------------------------------------------------------
cgInt32 cgDirectionalLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
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
        cgToDoAssert( "Effect Overhaul", "I'm assuming this can be removed now that it is passed in beginRead" );
        /*// Set the min/max shadow fade distances (for shadow attenuation over distance)
        cgEffectFile * pEffect = m_hEffect;
        pEffect->SetFloat( _T("ShadowMinDistance"), GetShadowMinDistance() );
        pEffect->SetFloat( _T("ShadowMaxDistance"), GetShadowMaxDistance() );*/

        // Add first pass to process the far distance (outside of the range of the last split)
        // Note: 0x7FFFFFFF is used to denote this particular 'frustum'.
        mLightingPasses.reserve( mFrustums.size() + 1 ); // Assume 1 pass per frustum as a minimum (plus the far split).
        mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, 0x7FFFFFFF, 0 ) );

        // Iterate through each frustum to build pass list.
        // Note: Splits must be drawn in back to front order.
        for ( cgInt32 i = ((cgInt32)mFrustums.size() - 1); i >= 0; --i )
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

    // If there was literally nothing to do, return -1 (don't even process).
    // ToDo: 6767 - Check that the caller even cares if this is -1.
    if ( mLightingPasses.empty() )
        return -1;

    // Return number of passes.
    mLightingPassCount = (cgInt32)mLightingPasses.size();
    return mLightingPassCount;
}

//-----------------------------------------------------------------------------
//  Name : buildStateBlocks () (Protected Virtual)
/// <summary>
/// Build state blocks necessary for lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::buildStateBlocks( )
{
    cgResourceManager * pResources = mParentScene->getResourceManager();

    ///////////////////////////////////////////////////////////////////////////
    // Camera Inside Case (Full Lighting)
    ///////////////////////////////////////////////////////////////////////////
    {
        //*********************************************************************
        // Process standard lighting case first.
        //*********************************************************************
        StateGroup * pStates = &mInLightStates;

        // Depth stencil state
        cgDepthStencilStateDesc dsState;
        dsState.depthEnable        = true;
        dsState.depthWriteEnable   = false;
        dsState.depthFunction          = cgComparisonFunction::Greater;
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;
        
        // Rasterizer state
        cgRasterizerStateDesc rsState;
        rsState.cullMode = cgCullMode::Back;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;
        
        // Blend state
        cgBlendStateDesc blState;
        blState.renderTarget[0].blendEnable = true;
        blState.renderTarget[0].sourceBlend  = cgBlendMode::One;
        blState.renderTarget[0].destinationBlend = cgBlendMode::One;	    
        if ( !pResources->createBlendState( &pStates->blend, blState, 0, cgDebugSource() ) )
            return false;

    } // End Inside Case

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : beginLightingPass() (Virtual)
// Desc : Called in order to begin an individual pass of the lighting process.
//-----------------------------------------------------------------------------
cgLightNode::LightingOp cgDirectionalLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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
	        pShadowFrustum->beginWrite( true, true );

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
        // Prevent final draw unless we're absolutely certain.
        Pass.op = Lighting_None;

        // Forward or deferred?
        if ( mLightingDeferred )
        {
            // Processing individual frustums or just rendering light shape?
            if ( Pass.frustum == 0x7FFFFFFF )
            {
                cgSurfaceShader * pShader = mShader.getResource(true);
                if ( !pShader || !pShader->isLoaded() )
                    return Lighting_Abort;

                // Get access to our required systems
                cgRenderDriver * pDriver = mParentScene->getRenderDriver();

                // Select states for lighting
                pDriver->setDepthStencilState( mInLightStates.depthStencil );
                pDriver->setRasterizerState( mInLightStates.rasterizer );
                pDriver->setBlendState( mInLightStates.blend );

                // Select vertex / pixel shaders
                if ( !pShader->selectVertexShader( _T("transform"), mLightingVSArgs ) ||
                     !pShader->selectPixelShader( _T("drawDirectLighting"), mLightingPSArgs ) )
                    return Lighting_Abort;
        	    
                // Light all pixels with a quad draw
		        pDriver->drawClipQuad( 1.0f );

            } // End if light shape
            else
            {
                // Processing individual frustums.
                cgShadowGenerator * pShadowFrustum = (Pass.frustum != 0x7FFFFFFF) ? mFrustums[ Pass.frustum ] : CG_NULL;
                beginRenderFrustum( Pass.frustum, pShadowFrustum, true, mLightingApplyShadows );

            } // End if individual frustum

        } // End if deferred
        else
        {
            // Processing individual frustums or just rendering light shape?
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
                // Processing individual frustums (caller should render using the computed receiver set).
                cgToDoAssert( "Carbon General", "Reintroduce forward shadowing.");
                cgShadowGenerator * pShadowFrustum = (Pass.frustum != 0x7FFFFFFF) ? mFrustums[ Pass.frustum ] : CG_NULL;
                if ( beginRenderFrustum( Pass.frustum, pShadowFrustum, false, mLightingApplyShadows ) == true )
                {
                    // ToDo: 6767 -- If we have a shadow generator, we can use its visibility set as an illumination set.
                    pRenderSetOut = CG_NULL;
                    Pass.op = Lighting_ProcessLight;
                
                } // End if success

            } // End if individual frustum*/

        } // End if forward

    } // End if ProcessLight

    // Process this pass.
    mCurrentLightingPass = nPass;
    return Pass.op;
}

//-----------------------------------------------------------------------------
// Name : endLightingPass() (Virtual)
// Desc : Called in order to end an individual pass of the lighting process.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::endLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( cgLightNode::endLightingPass() == false )
        return false;

    // What are we doing?
    const LightingOpPass & Pass = mLightingPasses[nCurrentPass];
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
        // Complete any rendering process
        if ( Pass.frustum == 0x7FFFFFFF )
        {
            if ( mLightingDeferred == false )
                endRenderForward();

        } // End if shape draw
        else
        {
            cgShadowGenerator * pShadowFrustum = (Pass.frustum != 0x7FFFFFFF) ? mFrustums[ Pass.frustum ] : CG_NULL;
            endRenderFrustum( Pass.frustum, pShadowFrustum, mLightingDeferred, mLightingApplyShadows );

        } // End if frustum draw
        
    } // End if ProcessLight

    // Valid.
    return true;
}

//-----------------------------------------------------------------------------
// Name : endLighting() (Virtual)
// Desc : Called in order to signify that the lighting process is complete.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::endLighting( )
{
    // Valid end operation?
    if ( cgLightNode::endLighting( ) == false )
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
bool cgDirectionalLightNode::reassignIndirectMaps( cgTexturePool * pPool )
{
    bool bNoFurtherAssignment = true;

    // Is there any need to re-assign?
    if ( !isIndirectSource() )
    {
        // Release resources back to the pool.
        for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); ++i )
            mIndirectFrustums[ i ]->releaseResources();

		return true;
    
    } // End if not a caster
 	
	// Attempt resource reassignment for each frustum
    for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); ++i )
    {
        cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator *)mIndirectFrustums[i];

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
cgInt32 cgDirectionalLightNode::beginIndirectFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectFill( pPool ) < 0 )
        return -1;

	// Clear the current pass list and reset count
    // ToDo: 6767 - Performed by base class if not already!!
    /*mShadowPasses.clear();
	mShadowPassCount = 0;

	// If this is not an indirect lighting source, do nothing
    if ( !isIndirectSource() )
		return 0;*/
    
    // ToDo: 6767 - Determine if it is an appropriate time for us to update based on
    // our shadow map/rsm update period limiting property.

	// For each frustum...
    mShadowPasses.reserve( mIndirectFrustums.size() ); // Assume 1 pass per frustum as a minimum
	for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); i++ )
	{
		// Do nothing if the frustum has nothing to draw
        // ToDo: 6767 - Use containsRenderableObjects()?
        cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator *)mIndirectFrustums[ i ];
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

		// How many passes are required to fill the assigned resources?
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
//  Name : beginShadowFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::beginIndirectFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation?
    if ( !cgLightNode::beginIndirectFillPass( nPass, pRenderSetOut ) )
        return false;

	// If this is the first child pass for this frustum, begin writing
    const LightingOpPass & Pass = mShadowPasses[ nPass ];
    cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator *)mIndirectFrustums[ Pass.frustum ];
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
bool cgDirectionalLightNode::endIndirectFillPass( )
{
    cgInt32 nCurrentPass = mCurrentShadowPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectFillPass() )
        return false;

    // Finish filling for this pass.
    const LightingOpPass & Pass = mShadowPasses[ nCurrentPass ];
	cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator *)mIndirectFrustums[ Pass.frustum ];
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
bool cgDirectionalLightNode::endIndirectFill( )
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
cgInt32 cgDirectionalLightNode::beginIndirectLighting( cgTexturePool * pPool )
{
    // ToDo: 6767 -- can we do this in the base class? I doubt it, but worth a shot.
    // ToDo: 6767 -- The other light types (i.e. cgPointLight) use '!isIndirectSource()'
    // Must be a shadow source for RSM to work
    if ( !isShadowSource() )
        return -1;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectLighting( pPool ) < 0 )
        return -1;

    // We need to fill and set reflective shadow maps.
    mLightingPasses.reserve( mFrustums.size() ); // Assume 1 pass per frustum as a minimum.
    for ( cgInt32 i = 0; i < (cgInt32)mFrustums.size(); ++i )
    {
        // If we are applying shadows, we potentially require 2 passes -- the first
        // to fill a default rsm prior to lighting, and the second to perform 
        // the actual lighting process.
        cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator * )mIndirectFrustums[i];
        if ( pFrustum->requiresDefaultResource() )
		{
			// How many passes will it take to populate our resources?
			cgUInt32 nPassCount = pFrustum->getWritePassCount();
			for ( cgUInt32 j = 0; j < nPassCount; j++ )
                mLightingPasses.push_back( LightingOpPass( Lighting_FillShadowMap, i, j ) );
		
        } // End if requires default
        
        // 1 pass for lighting with this frustum.
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
cgLightNode::LightingOp cgDirectionalLightNode::beginIndirectLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( cgLightNode::beginIndirectLightingPass( nPass, pRenderSetOut ) == Lighting_Abort )
        return Lighting_Abort;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nPass];

 	// Filling default resources
   if ( Pass.op == Lighting_FillShadowMap )
    {
        cgReflectanceGenerator * pFrustum = (cgReflectanceGenerator * )mIndirectFrustums[ Pass.frustum ];

		// On the first child pass, begin writing
		if ( Pass.subPass == 0 )
            pFrustum->beginWrite( true, true );
			
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
bool cgDirectionalLightNode::endIndirectLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectLightingPass() )
        return false;

    // Complete any shadow map fill for this frustum.
    const LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Complete fill for this pass.
        cgShadowGenerator * pFrustum = mIndirectFrustums[ Pass.frustum ];
        pFrustum->endWritePass();
        
		// On the last child pass, end writing.
		if ( Pass.subPass == (pFrustum->getWritePassCount() - 1) )
			pFrustum->endWrite();

    } // End if FillShadowMap
	else if ( Pass.op == Lighting_ProcessLight )
	{
		// For indirect lighting, all we need to do is setup the RSM data
		mIndirectFrustums[ Pass.frustum ]->endRead( );

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
bool cgDirectionalLightNode::endIndirectLighting( )
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
void cgDirectionalLightNode::setIndirectLightingMethod( cgUInt32 nMethod )
{
	// Call the base class
	cgLightNode::setIndirectLightingMethod( nMethod );

	// Update the frustums with the method
    for ( cgUInt32 i = 0; i < mIndirectFrustums.size(); ++i )
		mIndirectFrustums[i]->setMethod( nMethod );
}

//-----------------------------------------------------------------------------
// Name : beginRenderFrustum () (Protected)
// Desc : Begin to process lighting & (potentially) shadows simultaneously.
//-----------------------------------------------------------------------------
bool cgDirectionalLightNode::beginRenderFrustum( cgInt32 nFrustumIndex, cgShadowGenerator * pShadowFrustum, bool bDeferred, bool bApplyShadows )
{
    cgToDoAssert( "Effect Overhaul", "Implement beginRenderFrustum()" );
    /*
    const cgInt32 nFirstSplit  = 0, nLastSplit = ((cgInt32)mFrustums.size() - 1);

    // Get access to required systems / objects.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    cgCameraNode   * pCamera = pDriver->getCamera();

    // Should we use the stencil buffer and/or user clip planes? (Force stencil for deferred cases, clip planes for forward.)
    bool bUseStencil    = (bDeferred);
    bool bUseClipPlanes = (!bDeferred || m_bUseLightClipPlanes);

    // Is split blending enabled?
    bool bBlendSplits = (getShadowSplitOverlap() > 0.0f) ? true : false; // ToDo: Note: Should check that the library supports this feature or make it controllable via a uniform bool. 

    // Get camera near/far data to prepare for projection of split plane distance values
    const cgVector3 & CameraPosition  = pCamera->getPosition( false );
    const cgVector3 & CameraDirection = pCamera->getZAxis( false );
    cgFloat fCameraNear = pCamera->GetNearClip();
    cgFloat fCameraFar  = pCamera->GetFarClip();

    // Select technique base name based on lighting type
    cgString strMethod = GetLightingTechnique();

    // Is this a specific valid frustum, or is this the initialization
    // pass that pre-lights the scene past the range of the light source?
    if ( nFrustumIndex == 0x7FFFFFFF )
    {
        // This pass draws the (optional) non-shadowed region between the far split plane and the 
        // camera's far plane. This reduces the number of shadowing computations and makes sure 
        // everything that needs to be lit (without shadows) can still get a chance to run their 
        // calculations.

        // Select the frustum split's near and far plane (world/camera space distance)
        cgFloat fZNear = getShadowSplitDistance( nLastSplit );
        cgFloat fZFar  = fCameraFar;

        // Is this pass even necessary?
        if ( fZNear >= fZFar )
            return false;

        // Run lighting (deferred) or create a stencil mask.
        cgEffectFile * pEffect = m_hEffect;
	    if ( bUseStencil == true )
	    {
            cgFloat Q          = fCameraFar / ( fCameraFar - fCameraNear );
            cgFloat QN         = Q * fCameraNear;
            cgFloat fDepthNear = ( -1.0f / fZNear ) * QN + Q;
            cgFloat fDepthFar  = ( -1.0f / fZFar  ) * QN + Q;

		    pEffect->SetInt( _T("System_ShadowType"), 0 );
		    pEffect->SetTechnique( strMethod + _T("PSSMFar") );
		    pEffect->SetFloat( _T("PSSM_StencilPlane"), fDepthFar );
		    pDriver->drawClipQuad( m_hEffect, fDepthNear );
		
        } // End if bUseStencil

	    // Setup user clip planes
	    if ( bUseClipPlanes == true )
	    {
            cgPlane ClipPlaneNear;
		    cgVector3 p0 = CameraPosition + CameraDirection * fZNear;
		    cgPlane::fromPointNormal( &ClipPlaneNear, &p0, &CameraDirection );
		    pDriver->EnableUserClipPlane( 0, ClipPlaneNear );
		
        } // End if bUseClipPlanes

        // When forward rendering, build a local illumination set 
        // for the current frustum's receivers.
        m_pReceiversSet->Clear();
        if ( bDeferred == false )
        {
            // ToDo: Concerns with modifying the camera directly, build a new frustum?
            cgFloat fPrevClip = pCamera->GetNearClip();
            pCamera->SetNearClip( fZNear );
            m_pIlluminationSet->intersect( pCamera->GetFrustum(), m_pReceiversSet );
            pCamera->SetNearClip( fPrevClip );
        
        } // End if deferred
        
        // If split blending is active, make sure it doesn't do anything to this subfrustum
        if ( bBlendSplits )
            pEffect->SetVector( _T("SplitBlend"), cgVector4( 0, 0, 1, 0 ) );

        // Caller will draw geometry in m_pReceiversSet after this point assuming forward lighting.

    } // End if initialize pass
    else
    {
        cgFloat fCenter, fMin, fMax, ooRange, fHalfRegion;

        // Disable shadow rendering if the frustum still has no shadow map.
        if ( pShadowFrustum->GetShadowMap() == CG_NULL )
            bApplyShadows = false;

        // Select the frustum split's near and far plane (world/camera space distance)
        cgFloat fZFar  = getShadowSplitDistance( nFrustumIndex );
        cgFloat fZNear = ( nFrustumIndex == nFirstSplit ) ? fCameraNear : getShadowSplitDistance( nFrustumIndex - 1 );

        // If we are blending our results between splits, compute blend equation values 
        // and do any plane adjustment needed to account for the overlap region.
        cgEffectFile * pEffect = m_hEffect;
        if ( bBlendSplits == true )
        {
            fHalfRegion = getShadowSplitOverlap() * 0.5f;
            if ( nFrustumIndex == nFirstSplit )
            {
                fMin      = fZFar - fHalfRegion;
                fMax      = fZFar + fHalfRegion;
                ooRange   = 1.0f / (fMax - fMin);
                fZFar     = fMax;
			    pEffect->SetVector( _T("SplitBlend"), cgVector4( 0.0f, -ooRange, fMin * ooRange + 1.0f, 0 ) );
            
            } // End if first split
            else if ( nFrustumIndex == nLastSplit )
            {
                fMax      = fZNear + fHalfRegion;
                fMin      = fZNear - fHalfRegion;
                ooRange   = 1.0f / (fMax - fMin);
                fZNear    = fMin;
                pEffect->SetVector( _T("SplitBlend"), cgVector4( 0.0f, ooRange, fMin * -ooRange, 0 ) );
            
            } // End if last split
            else    
            {
                fCenter   = (fZFar + fZNear) * 0.5f;
                fMin      = fZFar - fHalfRegion - fCenter;
                fMax      = fZFar + fHalfRegion - fCenter;
                ooRange   = 1.0f / (fMax - fMin);
                fZFar    += fHalfRegion;
                fZNear   -= fHalfRegion;
                pEffect->SetVector( _T("SplitBlend"), cgVector4( fCenter, -ooRange, fMin * ooRange + 1.0f, 0 ) );
            
            } // End if other splits
        
        } // End if blending splits

        // Transform the split plane distances to depth values
        cgFloat Q          = fCameraFar / ( fCameraFar - fCameraNear );
        cgFloat QN         = Q * fCameraNear;
        cgFloat fDepthNear = ( -1.0f / fZNear ) * QN + Q;
        cgFloat fDepthFar  = ( -1.0f / fZFar  ) * QN + Q;

	    // Setup the system ready for rendering shadowed lighting.
	    pShadowFrustum->beginRead( pDriver, m_hEffect, mShadowAttenuation );

        // Update the stencil buffer to ensure only required pixels in the frustum are processed.
	    if ( bUseStencil == true )
	    {
		    if ( nFrustumIndex == nFirstSplit )
		    {
			    pEffect->SetTechnique( strMethod + _T("PSSMNear") );
			    pDriver->drawClipQuad( m_hEffect, fDepthFar );
    		
            } // End if first split
		    else
		    {
			    pEffect->SetTechnique( strMethod + _T("PSSM") );
			    pEffect->SetFloat( _T("PSSM_StencilPlane"), fDepthFar );
			    pDriver->drawClipQuad( m_hEffect, fDepthNear );
    		
            } // End if other split
    	
        } // End if bUseStencil

	    // Setup near/far clip planes
	    if ( bUseClipPlanes == true )
	    {
            cgPlane ClipPlaneNear, ClipPlaneFar;
		    cgVector3 p0 = CameraPosition + CameraDirection * fZNear;
		    cgVector3 p1 = CameraPosition + CameraDirection * fZFar;
		    cgPlane::fromPointNormal( &ClipPlaneNear, &p0, &-CameraDirection );
		    cgPlane::fromPointNormal( &ClipPlaneFar,  &p1, &CameraDirection );
		    pDriver->EnableUserClipPlane( 0, -ClipPlaneNear );
		    pDriver->EnableUserClipPlane( 1, -ClipPlaneFar );
    	
        } // End if bUseClipPlanes

        // When forward rendering, build a local illumination set 
        // for the current frustum's receivers.
        m_pReceiversSet->Clear();
        if ( bDeferred == false )
	        m_pIlluminationSet->intersect( pShadowFrustum->getCamera()->GetFrustum(), m_pReceiversSet );

	    // Caller will draw geometry in m_pReceiversSet after this point assuming forward lighting.

    } // End if frustum pass.

    // Process!
    return true;*/
    return false;
}

//-----------------------------------------------------------------------------
// Name : endRenderFrustum () (Protected)
// Desc : Complete lighting & (potentially) shadowing process.
//-----------------------------------------------------------------------------
void cgDirectionalLightNode::endRenderFrustum( cgInt32 nFrustumIndex, cgShadowGenerator * pShadowFrustum, bool bDeferred, bool bApplyShadows )
{
    cgToDoAssert( "Effect Overhaul", "Implement endRenderFrustum()" );
    /*
    const cgInt32 nFirstSplit  = 0, nLastSplit = ((cgInt32)mFrustums.size() - 1);

    // Get access to required systems / objects.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    cgCameraNode   * pCamera = pDriver->getCamera();
    
    // Did we use the stencil buffer and/or user clip planes? (Force stencil for deferred cases, clip planes for forward.)
    bool bUseStencil    = (bDeferred);
    bool bUseClipPlanes = (!bDeferred || m_bUseLightClipPlanes);

    // Is this a specific valid frustum, or is this the initialization
    // pass that pre-lights the scene past the range of the light source?
    if ( nFrustumIndex == 0x7FFFFFFF )
    {
        // This pass drew the (optional) non-shadowed region between the far split plane and the 
        // camera's far plane. This reduces the number of shadowing computations and makes sure 
        // everything that needs to be lit (without shadows) can still get a chance to run their 
        // calculations.

        // Clear the clip planes if we used them
	    if ( bUseClipPlanes == true )
		    pDriver->DisableUserClipPlane( 0 );

	    // Clear the stencil buffer if we used it
	    if ( bUseStencil == true )
	    {
            cgEffectFile * pEffect = m_hEffect;
		    pEffect->SetTechnique( _T("StencilMaskClear") );
		    pEffect->SetFloat( _T("PSSM_StencilPlane"), 1.0f );
		    pDriver->drawClipQuad( m_hEffect, 1.0f );
		
        } // End if bUseStencil

    } // End if initialize pass
    else
    {
        // Clear the clip planes if we used them
	    if ( bUseClipPlanes == true )
	    {
		    pDriver->DisableUserClipPlane( 0 );
		    pDriver->DisableUserClipPlane( 1 );
    	
        } // End if bUseClipPlanes

	    // Clear the stencil buffer if we used it
	    if ( bUseStencil == true && nFrustumIndex != nFirstSplit )
	    {
            // Compute the distance at which to clear.
            cgFloat fCameraNear = pCamera->GetNearClip();
            cgFloat fCameraFar  = pCamera->GetFarClip();
            cgFloat Q           = fCameraFar / ( fCameraFar - fCameraNear );
            cgFloat QN          = Q * fCameraNear;
            cgFloat fDepthFar   = ( -1.0f / getShadowSplitDistance( nFrustumIndex ) ) * QN + Q;

            // Run clear operation.
            cgEffectFile * pEffect = m_hEffect;
		    pEffect->SetTechnique( _T("StencilMaskClear") );
		    pEffect->SetFloat( _T("PSSM_StencilPlane"), fDepthFar );
		    pDriver->drawClipQuad( m_hEffect, fDepthFar );
    	
        } // End if clear stencil

        // Reset states to appropriate defaults
        if ( pShadowFrustum != CG_NULL )
            pShadowFrustum->endRead( pDriver );

    } // End if frustum pass.*/
}
