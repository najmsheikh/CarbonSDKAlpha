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
// Name : cgHemisphereLightObject.cpp                                        //
//                                                                           //
// Desc : Hemisphere light source classes.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHemisphereLightObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgHemisphereLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <Math/cgCollision.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgMesh.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgHemisphereLightObject::mInsertHemisphereLight;
cgWorldQuery cgHemisphereLightObject::mUpdateRanges;
cgWorldQuery cgHemisphereLightObject::mUpdateHemisphereHDRScalars;
cgWorldQuery cgHemisphereLightObject::mUpdateDiffuseBackColor;
cgWorldQuery cgHemisphereLightObject::mUpdateSpecularBackColor;
cgWorldQuery cgHemisphereLightObject::mLoadHemisphereLight;

///////////////////////////////////////////////////////////////////////////////
// cgHemisphereLightObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHemisphereLightObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightObject::cgHemisphereLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgLightObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mOuterRange               = 0.0f;
    mRangeAdjust              = 1.1f;
    mInnerRange               = 0.0f;
    mDiffuseBackColor          = cgColorValue(0,0,0,0);
    mSpecularBackColor         = cgColorValue(0,0,0,0);
    mDiffuseBackHDRScale      = 1.0f;
    mSpecularBackHDRScale     = 1.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgHemisphereLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightObject::cgHemisphereLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgLightObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgHemisphereLightObject * pObject = (cgHemisphereLightObject*)pInit;
    mOuterRange               = pObject->mOuterRange;
    mRangeAdjust              = pObject->mRangeAdjust;
    mInnerRange               = pObject->mInnerRange;
    mDiffuseBackColor          = pObject->mDiffuseBackColor;
    mSpecularBackColor         = pObject->mSpecularBackColor;
    mDiffuseBackHDRScale      = pObject->mDiffuseBackHDRScale;
    mSpecularBackHDRScale     = pObject->mSpecularBackHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : ~cgHemisphereLightObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightObject::~cgHemisphereLightObject()
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
void cgHemisphereLightObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

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
cgWorldObject * cgHemisphereLightObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgHemisphereLightObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgHemisphereLightObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgHemisphereLightObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgHemisphereLightObject::getDatabaseTable( ) const
{
    return _T("Objects::HemisphereLight");
}

//-----------------------------------------------------------------------------
//  Name : setOuterRange()
/// <summary>
/// Set the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setOuterRange( cgFloat fRange )
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
void cgHemisphereLightObject::setInnerRange( cgFloat fRange )
{
    // Is this a no-op?
    if ( mInnerRange == fRange )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, mOuterRange );
        mUpdateRanges.bindParameter( 2, fRange );
        mUpdateRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateRanges.step( true ) == false )
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
void cgHemisphereLightObject::setRangeAdjust( cgFloat fAdjust )
{
    // Is this a no-op?
    if ( mRangeAdjust == fAdjust )
        return;

    /*// Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        m_qUpdateRange.bindParameter( 1, fRange );
        m_qUpdateRange.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( m_qUpdateRange.step( true ) == false )
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
//  Name : setDiffuseBackHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the diffuse back light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setDiffuseBackHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mDiffuseBackHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateHemisphereHDRScalars.bindParameter( 1, fScale );
        mUpdateHemisphereHDRScalars.bindParameter( 2, mSpecularBackHDRScale );
        mUpdateHemisphereHDRScalars.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateHemisphereHDRScalars.step( true ) == false )
        {
            cgString strError;
            mUpdateHemisphereHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars of light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mDiffuseBackHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("DiffuseBackHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularBackHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the specular back light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setSpecularBackHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mSpecularBackHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateHemisphereHDRScalars.bindParameter( 1, mDiffuseBackHDRScale );
        mUpdateHemisphereHDRScalars.bindParameter( 2, fScale );
        mUpdateHemisphereHDRScalars.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateHemisphereHDRScalars.step( true ) == false )
        {
            cgString strError;
            mUpdateHemisphereHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars of light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mSpecularBackHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularBackHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setDiffuseColor() (Virtual)
/// <summary>
/// Set the light's diffuse color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setDiffuseColor( const cgColorValue & Color )
{
    // Call base class implementation first.
    cgLightObject::setDiffuseColor( Color );

    // Since the hemisphere light is based on two diffuse colors
    // (front and back), the diffuse source bool should be based on both
    // rather than just the base class value.
    mIsDiffuseSource |= ( mDiffuseBackColor.r > CGE_EPSILON || mDiffuseBackColor.g > CGE_EPSILON || mDiffuseBackColor.b > CGE_EPSILON );
}

//-----------------------------------------------------------------------------
//  Name : setDiffuseBackColor() 
/// <summary>
/// Set the hemisphere light's diffuse back lit color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setDiffuseBackColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mDiffuseBackColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateDiffuseBackColor.bindParameter( 1, Color.r );
        mUpdateDiffuseBackColor.bindParameter( 2, Color.g );
        mUpdateDiffuseBackColor.bindParameter( 3, Color.b );
        mUpdateDiffuseBackColor.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( mUpdateDiffuseBackColor.step( true ) == false )
        {
            cgString strError;
            mUpdateDiffuseBackColor.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update diffuse back color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mDiffuseBackColor = Color;

    // Should we enable diffuse (includes both front and back diffuse colors)?
    mIsDiffuseSource = ( mDiffuseColor.r > CGE_EPSILON || mDiffuseColor.g > CGE_EPSILON || mDiffuseColor.b > CGE_EPSILON ||
                         mDiffuseBackColor.r > CGE_EPSILON || mDiffuseBackColor.g > CGE_EPSILON || mDiffuseBackColor.b > CGE_EPSILON );

    // Notify any listeners of this change.
    static const cgString strContext = _T("DiffuseBackColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularColor() (Virtual)
/// <summary>
/// Set the light's specular color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setSpecularColor( const cgColorValue & Color )
{
    // Call base class implementation first.
    cgLightObject::setSpecularColor( Color );

    // Since the hemisphere light is based on two specular colors
    // (front and back), the specular source bool should be based on both
    // rather than just the base class value.
    mIsSpecularSource |= ( mSpecularBackColor.r > CGE_EPSILON || mSpecularBackColor.g > CGE_EPSILON || mSpecularBackColor.b > CGE_EPSILON );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularBackColor() 
/// <summary>
/// Set the hemisphere light's specular back lit color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightObject::setSpecularBackColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mSpecularBackColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSpecularBackColor.bindParameter( 1, Color.r );
        mUpdateSpecularBackColor.bindParameter( 2, Color.g );
        mUpdateSpecularBackColor.bindParameter( 3, Color.b );
        mUpdateSpecularBackColor.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( mUpdateSpecularBackColor.step( true ) == false )
        {
            cgString strError;
            mUpdateSpecularBackColor.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update specular back color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mSpecularBackColor = Color;

    // Should we enable specular (includes both front and back specular colors)?
    mIsSpecularSource = ( mSpecularColor.r > CGE_EPSILON || mSpecularColor.g > CGE_EPSILON || mSpecularColor.b > CGE_EPSILON ||
                          mSpecularBackColor.r > CGE_EPSILON || mSpecularBackColor.g > CGE_EPSILON || mSpecularBackColor.b > CGE_EPSILON );

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getOuterRange()
/// <summary>
/// Get the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHemisphereLightObject::getOuterRange( ) const
{
    return mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerRange()
/// <summary>
/// Get the inner range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHemisphereLightObject::getInnerRange( ) const
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
cgFloat cgHemisphereLightObject::getRangeAdjust( ) const
{
    return mRangeAdjust;
}

//-----------------------------------------------------------------------------
//  Name : getDiffuseBackColor() 
/// <summary>
/// Retrieve the hemisphere light's diffuse back lit color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgHemisphereLightObject::getDiffuseBackColor( ) const
{
    return mDiffuseBackColor;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularBackColor() 
/// <summary>
/// Retrieve the hemisphere light's specular back lit color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgHemisphereLightObject::getSpecularBackColor( ) const
{
    return mSpecularBackColor;
}

//-----------------------------------------------------------------------------
//  Name : getDiffuseBackHDRScale() 
/// <summary>
/// Retrieve the HDR scale being applied to the diffuse back light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHemisphereLightObject::getDiffuseBackHDRScale( ) const
{
    return mDiffuseBackHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularBackHDRScale() 
/// <summary>
/// Retrieve the HDR scale being applied to the specular back light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHemisphereLightObject::getSpecularBackHDRScale( ) const
{
    return mSpecularBackHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgHemisphereLightObject::getLocalBoundingBox( )
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
bool cgHemisphereLightObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // Only valid in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return false;

    cgToDo( "Carbon General", "Update picking method to use that in Carbon Forge LT rather than the point light logic below" );

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
        if ( cgCollision::rayIntersectPlane( vOrigin, vDir, Plane, t, false, false ) == false )
            continue;

        // Compute the intersection point on the plane
        cgVector3 vIntersect = vOrigin + (vDir * t);

        // Determine if this point is within the triangles edges (within tolerance)
        if ( cgCollision::pointInTriangle( vIntersect, v1, v2, v3, (cgVector3&)Plane, 2.0f * fZoomFactor ) == false )
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
void cgHemisphereLightObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgShadedVertex  Points[30];
    cgUInt32        Indices[35];
    cgTransform     ObjectTransform, InverseObjectTransform;
    cgFloat         fZoomFactor;
    
    // Get access to required systems.
    cgRenderDriver  * pDriver = cgRenderDriver::getInstance();
    cgSurfaceShader * pShader = pDriver->getSandboxSurfaceShader().getResource(true);

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    cgUInt32    nColor      = (pIssuer->isSelected()) ? 0xFFFFFFFF : 0xFFFFF600;
    bool        bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);
    
    // Begin rendering
    pDriver->setVertexFormat( cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator ) );
    pShader->setBool( _T("wireViewport"), bWireframe );
    if ( pShader->beginTechnique( _T("drawWireframeNode") ) )
    {
        if ( pShader->executeTechniquePass() != cgTechniqueResult::Abort )
        {
            // Render both directional arrows.
            for ( cgInt i = 0; i < 2; ++i )
            {
                if ( i == 0 )
                {
                    // Set the object's transformation matrix to the device (light
                    // representation will be constructed in object space)
                    cgVector3 vAt  = pIssuer->getPosition(false);
                    cgVector3 vEye = vAt + pIssuer->getYAxis(false) * mOuterRange;
                    ObjectTransform.lookAt( vEye, vAt, pIssuer->getXAxis(false) );
                    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
                    pDriver->setWorldTransform( ObjectTransform );

                    // Compute zoom factor for this end point.
                    fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, vEye, 2.5f );
                
                } // End If Front Hemisphere
                else
                {
                    // Set the object's transformation matrix to the device (light
                    // representation will be constructed in object space)
                    cgVector3 vAt  = pIssuer->getPosition(false);
                    cgVector3 vEye = vAt - pIssuer->getYAxis(false) * mOuterRange;
                    ObjectTransform.lookAt( vEye, vAt, pIssuer->getXAxis(false) );
                    cgTransform::inverse( InverseObjectTransform, ObjectTransform );
                    pDriver->setWorldTransform( ObjectTransform );

                    // Compute zoom factor for this end point.
                    fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, vEye, 2.5f );
                
                } // End If Back Hemisphere

                // Set the color of each of the points first of all. This saves
                // us from having to set them during object construction.
                for ( cgInt j = 0; j < 16; ++j )
                    Points[j].color = nColor;

                // Compute vertices for the tip of the directional light source arrow
                cgFloat fSize      = fZoomFactor * 7.5f;
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
                bool bCull;
                if ( bOrtho )
                    bCull = (cgVector3::dot( cgVector3( 1,0,0 ), vCameraLook ) >= 0.0f );
                else
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 1,0,0 ), -fSize ) != cgPlaneQuery::Front );
                
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
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( -1,0,0 ), fSize ) != cgPlaneQuery::Front );
                
                // Build indices
                if ( !bCull )
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
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), 0.0f ) != cgPlaneQuery::Front );
                
                // Build indices
                if ( !bCull )
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
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,1,0 ), -fSize ) != cgPlaneQuery::Front );
                
                // Build indices
                if ( !bCull )
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
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,-1,0 ), fSize ) != cgPlaneQuery::Front );
                
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
                    bCull = (cgVector3::dot( cgVector3( 0,0,-1 ), vCameraLook ) >= 0.0f );
                else
                    bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 0,0,-1 ), fZoomFactor * 20.0f ) != cgPlaneQuery::Front );
                
                // Build base vertices / indices
                if ( !bCull )
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

                // Draw the rest of the light source representation
                pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 12, nCount / 2, Indices, cgBufferFormat::Index32, Points );

            } // Next Light Representation

            // Compute zoom factor for the central diamond.
            fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );

            // Set the color of each of the points first of all. This saves
            // us from having to set them during object construction.
            for ( cgInt i = 0; i < 6; ++i )
                Points[i].color = nColor;

            // Compute vertices for the light source representation (central diamond)
            cgFloat fSize      = fZoomFactor * 10.0f;
            Points[0].position = cgVector3( 0, fSize, 0 );
            Points[1].position = cgVector3( 0, 0, fSize );
            Points[2].position = cgVector3( fSize, 0, 0 );
            Points[3].position = cgVector3( 0, 0, -fSize );
            Points[4].position = cgVector3( -fSize, 0, 0 );
            Points[5].position = cgVector3( 0, -fSize, 0 );
            
            // Compute indices that will allow us to draw the diamond using a tri-list (wireframe)
            // so that we can easily take advantage of back-face culling.
            Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
            Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
            Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
            Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;
            Indices[12] = 5; Indices[13] = 2; Indices[14] = 1;
            Indices[15] = 5; Indices[16] = 3; Indices[17] = 2;
            Indices[18] = 5; Indices[19] = 4; Indices[20] = 3;
            Indices[21] = 5; Indices[22] = 1; Indices[23] = 4;

            // Set the object's transformation matrix to the device (light
            // representation will be constructed in object space)
            ObjectTransform = pIssuer->getWorldTransform(false);
            cgTransform::inverse( InverseObjectTransform, ObjectTransform );
            pDriver->setWorldTransform( ObjectTransform );

            // Draw the central diamond
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, 6, 8, Indices, cgBufferFormat::Index32, Points );

            // So that we can construct in object space, transform camera culling details
            // into object space too.
            cgVector3 vCameraLook, vCameraPos;
            InverseObjectTransform.transformNormal( vCameraLook, pCamera->getZAxis(false) );
            InverseObjectTransform.transformCoord( vCameraPos, pCamera->getPosition(false) );
            cgVector3::normalize( vCameraLook, vCameraLook );

            // If the light is REALLY selected (i.e. not just one of its parent groups), construct and 
            // render circles that depict the outer range of the light source.
            if ( pIssuer->isSelected() && !pIssuer->isMergedAsGroup() )
            {
                // A circle for each axis
                cgVector3 vPoint, vAxis, vOrthoAxis1, vOrthoAxis2;
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
                        fabsf( cgVector3::dot( vAxis, vCameraLook ) ) < 0.05f )
                        continue;
                    
                    // Generate line strip circle
                    for ( cgInt j = 0; j < 30; ++j )
                    {
                        // Build vertex
                        vPoint  = vOrthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);
                        vPoint += vOrthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mOuterRange);

                        // Generate the color for this new point
                        nColor = cgColorValue::lerp( mDiffuseBackColor, mDiffuseColor, ((vPoint.y / mOuterRange) * 0.5f) + 0.5f );

                        // Store and reference
                        Points[j] = cgShadedVertex( vPoint, nColor );
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
                            vPoint  = vOrthoAxis2 * (sinf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            vPoint += vOrthoAxis1 * (cosf( (CGE_TWO_PI / 30.0f) * (cgFloat)j ) * mInnerRange);
                            Points[j].position = vPoint;
                            Points[j].color    = 0xFF99CCE5;
                            
                        } // Next Segment

                        // Render the lines.
                        pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineStrip, 0, 30, 30, Indices, cgBufferFormat::Index32, Points );
                    
                    } // End if draw inner range

                } // Next Axis

            } // End if light source selected

             // Draw lines from the center of the light source to the directional indicators
            nColor             = (pIssuer->isSelected()) ? 0xFF4C6672 : 0xFF99CCE5;
            Points[0].position = cgVector3( 0.0f, 0.0f, 0.0f );
            Points[0].color    = nColor;
            Points[1].position = cgVector3( 0.0f, mOuterRange, 0.0f );
            Points[1].color    = nColor;
            Points[2].position = cgVector3( 0.0f, 0.0f, 0.0f );
            Points[2].color    = nColor;
            Points[3].position = cgVector3( 0.0f, -mOuterRange, 0.0f );
            Points[3].color    = nColor;
            pDriver->drawPrimitiveUP( cgPrimitiveType::LineList, 2, Points );

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
void cgHemisphereLightObject::applyObjectRescale( cgFloat fScale )
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
bool cgHemisphereLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HemisphereLightObject )
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
cgLightObject::LightType cgHemisphereLightObject::getLightType( ) const
{
    return Light_Hemisphere;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
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
bool cgHemisphereLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("HemisphereLightObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertHemisphereLight.bindParameter( 1, mReferenceId );
        mInsertHemisphereLight.bindParameter( 2, mDiffuseBackColor.r );
        mInsertHemisphereLight.bindParameter( 3, mDiffuseBackColor.g );
        mInsertHemisphereLight.bindParameter( 4, mDiffuseBackColor.b );
        mInsertHemisphereLight.bindParameter( 5, mDiffuseBackHDRScale );
        mInsertHemisphereLight.bindParameter( 6, mSpecularBackColor.r );
        mInsertHemisphereLight.bindParameter( 7, mSpecularBackColor.g );
        mInsertHemisphereLight.bindParameter( 8, mSpecularBackColor.b );
        mInsertHemisphereLight.bindParameter( 9, mSpecularBackHDRScale );
        mInsertHemisphereLight.bindParameter( 10, mOuterRange );
        mInsertHemisphereLight.bindParameter( 11, mInnerRange );
        mInsertHemisphereLight.bindParameter( 12, (cgUInt32)0 );   // ToDo: DistanceAttenuationSplineId
        mInsertHemisphereLight.bindParameter( 13, (cgUInt32)0 );   // ToDo: AttenuationMaskSamplerId
        
        cgToDo( "Carbon General", "All database inserts of references should do the following, even though most currently don't." );
        // Database ref count (just in case it has already been adjusted)
        mInsertHemisphereLight.bindParameter( 14, mSoftRefCount );
        
        // ToDo: 9999 - mRangeAdjust
        
        // Execute
        if ( mInsertHemisphereLight.step( true ) == false )
        {
            cgString strError;
            mInsertHemisphereLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for hemisphere light object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("HemisphereLightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("HemisphereLightObject::insertComponentData") );

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
bool cgHemisphereLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the light data.
    prepareQueries();
    mLoadHemisphereLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadHemisphereLight.step( ) || !mLoadHemisphereLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadHemisphereLight.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hemisphere light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for hemisphere light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadHemisphereLight.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadHemisphereLight;

    // Update our local members
    mLoadHemisphereLight.getColumn( _T("DiffuseBackR"), mDiffuseBackColor.r );
    mLoadHemisphereLight.getColumn( _T("DiffuseBackG"), mDiffuseBackColor.g );
    mLoadHemisphereLight.getColumn( _T("DiffuseBackB"), mDiffuseBackColor.b );
    mLoadHemisphereLight.getColumn( _T("DiffuseBackHDRScalar"), mDiffuseBackHDRScale );
    mLoadHemisphereLight.getColumn( _T("SpecularBackR"), mSpecularBackColor.r );
    mLoadHemisphereLight.getColumn( _T("SpecularBackG"), mSpecularBackColor.g );
    mLoadHemisphereLight.getColumn( _T("SpecularBackB"), mSpecularBackColor.b );
    mLoadHemisphereLight.getColumn( _T("SpecularBackHDRScalar"), mSpecularBackHDRScale );
    mLoadHemisphereLight.getColumn( _T("OuterRange"), mOuterRange );
    mLoadHemisphereLight.getColumn( _T("InnerRange"), mInnerRange );
    // ToDo: 9999 - DistanceAttenuationSplineId
    // ToDo: 9999 - AttenuationMaskSamplerId
    // ToDo: 9999 - RangeAdjust

    // Call base class implementation to read remaining data.
    if ( !cgLightObject::onComponentLoading( e ) )
        return false;

    // Should we enable diffuse / specular? Since the hemisphere light
    // includes two diffuse / specular colors (front and back), the appropriate
    // 'Source' bools should be based on both colors rather than just the
    // single color as computed by the base class method.
    mIsDiffuseSource  |= ( mDiffuseBackColor.r > CGE_EPSILON || mDiffuseBackColor.g > CGE_EPSILON || mDiffuseBackColor.b > CGE_EPSILON );
    mIsSpecularSource |= ( mSpecularBackColor.r > CGE_EPSILON || mSpecularBackColor.g > CGE_EPSILON || mSpecularBackColor.b > CGE_EPSILON );

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
void cgHemisphereLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertHemisphereLight.isPrepared() == false )
            mInsertHemisphereLight.prepare( mWorld, _T("INSERT INTO 'Objects::HemisphereLight' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14)"), true );
        if ( mUpdateRanges.isPrepared() == false )
            mUpdateRanges.prepare( mWorld, _T("UPDATE 'Objects::HemisphereLight' SET OuterRange=?1, InnerRange=?2 WHERE RefId=?3"), true );
        if ( mUpdateHemisphereHDRScalars.isPrepared() == false )
            mUpdateHemisphereHDRScalars.prepare( mWorld, _T("UPDATE 'Objects::HemisphereLight' SET DiffuseBackHDRScalar=?1, SpecularBackHDRScalar=?2 WHERE RefId=?3"), true );
        if ( mUpdateDiffuseBackColor.isPrepared() == false )
            mUpdateDiffuseBackColor.prepare( mWorld, _T("UPDATE 'Objects::HemisphereLight' SET DiffuseBackR=?1, DiffuseBackG=?2, DiffuseBackB=?3 WHERE RefId=?4"), true );
        if ( mUpdateSpecularBackColor.isPrepared() == false )
            mUpdateSpecularBackColor.prepare( mWorld, _T("UPDATE 'Objects::HemisphereLight' SET SpecularBackR=?1, SpecularBackG=?2, SpecularBackB=?3 WHERE RefId=?4"), true );
        
    } // End if sandbox

    // Read queries
    if ( mLoadHemisphereLight.isPrepared() == false )
        mLoadHemisphereLight.prepare( mWorld, _T("SELECT * FROM 'Objects::HemisphereLight' WHERE RefId=?1"), true );
}

///////////////////////////////////////////////////////////////////////////////
// cgHemisphereLightNode Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHemisphereLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightNode::cgHemisphereLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgLightNode( nReferenceId, pScene )
{
}

//-----------------------------------------------------------------------------
//  Name : cgHemisphereLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightNode::cgHemisphereLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgHemisphereLightNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHemisphereLightNode::~cgHemisphereLightNode()
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
void cgHemisphereLightNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
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
bool cgHemisphereLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HemisphereLightNode )
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
cgObjectNode * cgHemisphereLightNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgHemisphereLightNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgHemisphereLightNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgHemisphereLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
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
bool cgHemisphereLightNode::boundsInVolume( const cgBoundingBox & AABB )
{
    return AABB.intersect( getBoundingBox() );
}

//-----------------------------------------------------------------------------
//  Name : pointInVolume () (Virtual)
/// <summary>
/// Does the specified point fall within the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::pointInVolume( const cgVector3 & Point, cgFloat fErrRadius /* = 0.0f */ )
{
    return ( cgVector3::length( getPosition( false ) - Point ) < (getOuterRange() + fErrRadius) );
}

//-----------------------------------------------------------------------------
//  Name : frustumInVolume () (Virtual)
/// <summary>
/// Does the specified frustum intersect the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::frustumInVolume( const cgFrustum & Frustum )
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
bool cgHemisphereLightNode::testObjectShadowVolume( cgObjectNode * pObject, const cgFrustum & ViewFrustum )
{
    // Hemisphere light does not support shadow casting.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Virtual)
/// <summary>
/// Compute the visibility set from the point of view of this light.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightNode::computeVisibility( )
{
    // Shadow casting is not supported for hemisphere lights.
    mComputeShadows = false;
    
    // Call base class implementation last
    cgLightNode::computeVisibility();
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail () (Virtual)
/// <summary>
/// Computes level of detail settings for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
{
    // Allow base class to compute default LOD settings.
    cgLightNode::computeLevelOfDetail( pCamera );
}

//-----------------------------------------------------------------------------
//  Name : setClipPlanes () (Virtual)
/// <summary>
/// Setup the clipping planes for this light source
/// ToDo : For some reason, this function is broken...
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightNode::setClipPlanes( )
{
    cgPlane Planes[ 6 ];

	// Get access to required systems / objects
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();

	// Extract the clip planes from our bounding box
	cgBoundingBox bbox = getBoundingBox( );
	for ( cgUInt32 i = 0; i < 6; i++ )
        Planes[ i ] = bbox.getPlane( (cgVolumePlane::Side)i );

	// Set the user-defined clip planes for the light
	pDriver->setUserClipPlanes( Planes, 6 );

    // Update state
	mSetClipPlanes = true;
}

//-----------------------------------------------------------------------------
//  Name : updateLightConstants() (Virtual)
/// <summary>
/// Update the lighting base terms constant buffer as necessary whenever any
/// of the appropriate properties change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::updateLightConstants()
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
    pLightData->direction      = -getYAxis(false);
    pLightData->attenuation.x  = fInnerRange;
    pLightData->attenuation.y  = fOuterRange;
    pLightData->attenuation.z  = 1.0f / (fOuterRange - fInnerRange);
    pLightData->attenuation.w  = -(fInnerRange / (fOuterRange - fInnerRange));
    pLightData->clipDistance.x = 1.0f;
    pLightData->clipDistance.y = fOuterRange;

    // Setup light secondary colors
    pLightData->diffuse2       = getDiffuseBackColor();
    pLightData->specular2      = getSpecularBackColor();

    // If HDR lighting is enabled, apply intensity scalars
    cgRenderDriver * pRenderDriver = mParentScene->getRenderDriver();
    if ( pRenderDriver->getSystemState( cgSystemState::HDRLighting ) > 0 )
    {
        float fHDRIntensityDiffuse  = getDiffuseHDRScale();
        float fHDRIntensitySpecular = getSpecularHDRScale();

        pLightData->diffuse2.r  *= fHDRIntensityDiffuse;
        pLightData->diffuse2.g  *= fHDRIntensityDiffuse;
        pLightData->diffuse2.b  *= fHDRIntensityDiffuse;

        pLightData->specular2.r *= fHDRIntensitySpecular;
        pLightData->specular2.g *= fHDRIntensitySpecular;
        pLightData->specular2.b *= fHDRIntensitySpecular;
    
    } // End if HDR
    
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
cgInt32 cgHemisphereLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
{
    // Override shadow application settings just in case.
    if ( isShadowSource() == false )
        bApplyShadows = false;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginLighting( pPool, bApplyShadows, bDeferred ) < 0 )
        return -1;

    // Only a single (unshadowed) lighting pass is required.
    mLightingPasses.clear();
    mLightingPasses.push_back( LightingOpPass( Lighting_ProcessLight, 0x7FFFFFFF, 0 ) );
    
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
cgLightNode::LightingOp cgHemisphereLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Valid lighting operation
    if ( cgLightNode::beginLightingPass( nPass, pRenderSetOut ) == Lighting_Abort )
        return Lighting_Abort;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nPass];
    if ( Pass.op == Lighting_ProcessLight )
    {
        // Prevent final draw unless we're absolutely certain.
        Pass.op = Lighting_None;
        
        // Forward or deferred?
        if ( mLightingDeferred )
        {
            cgRenderDriver * pDriver = mParentScene->getRenderDriver();
            cgCameraNode   * pCamera = pDriver->getCamera();

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
            cgToDo( "Carbon General", "mLightShapeTransform should already be up-to date. Shouldn't it be used here (same for point lights)?" );
            cgTransform LightShapeTransform;
            LightShapeTransform.scale( fRange, fRange, fRange );
            renderDeferred( LightShapeTransform * getWorldTransform( false ), VolumeStatus );

        } // End if deferred
        else
        {
            // Begin forward render process
            if ( beginRenderForward( mLightShapeTransform * getWorldTransform( false ) ) )
            {
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
bool cgHemisphereLightNode::endLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( cgLightNode::endLightingPass() == false )
        return false;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_ProcessLight )
    {
        // Complete the forward rendering process.
        if ( mLightingDeferred == false )
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
bool cgHemisphereLightNode::endLighting( )
{
    // Valid end operation?
    if ( cgLightNode::endLighting( ) == false )
        return false;

    // Valid.
    mLightingPasses.clear();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgHemisphereLightNode::onComponentModified( cgComponentModifiedEventArgs * e )
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

        // Light source node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );
        
    } // End if Range | RangeAdjust | ApplyRescale

    // Call base class implementation last
    cgLightNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
//  Name : updateSystemConstants() (Virtual)
/// <summary>
/// Update the lighting system's constant buffer as necessary whenever any of the
/// appropriate matrices change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::updateSystemConstants()
{
    // Call base class implementation first.
    if ( !cgLightNode::updateSystemConstants() )
        return false;

    // Override the current direction
    cgVector3 direction = -getYAxis(false);
    static const cgString strConstantLightDirection = _T("_lightDirection");
    mParentScene->getLightingManager()->setConstant( strConstantLightDirection, &direction );

    /*// Are we using image based lighting? (ToDo 9999)
    cgSampler * pSampler = GetLightColorSampler();
    if ( pSampler )
    {
    	const cgTextureHandle & hTexture = pSampler->GetTexture();
    	if ( hTexture.IsValid() )
    	{
    		cgTexture * pTexture = hTexture.GetResourceSilent();
    		if ( pTexture->GetInfo().Type == cgBufferType::TextureCube )
    		{
    			cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    			pDriver->SetSystemState( cgSystemState::DiffuseIBL,  true );
    			pDriver->SetSystemState( cgSystemState::SpecularIBL, true );
    		
            } // End if cube map
    
        } // End valid texture

    } // End valid sampler*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::postCreate( )
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
        pMesh->createSphere( cgVertexFormat::formatFromFVF( D3DFVF_XYZ ), 1.0f, 10, 10,
                             false, cgMeshCreateOrigin::Center, true, pResources );
        
        // Add to the resource manager
        pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::UnitSphere"), cgDebugSource() );

    } // End if no existing mesh

    // Compute render projection matrix.
    cgFloat fRange = getOuterRange(), fRangeAdjust = getRangeAdjust();
    cgMatrix::perspectiveFovLH( mProjectionMatrix, CGEToRadian(90.0f), 1.0f, 1.0f, fRange );

    // Set the light's world matrix (sized to range with tolerance)
    cgTransform::scaling( mLightShapeTransform, fRange * fRangeAdjust, fRange * fRangeAdjust, fRange * fRangeAdjust );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform() (Override)
/// <summary>
/// Update our internal cell matrix with that specified here.
/// Note : Hooked into base class so that we can updatate our own properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgHemisphereLightNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgLightNode::setCellTransform( Transform, Source ) )
        return false;

    // Success!
    return true;
}