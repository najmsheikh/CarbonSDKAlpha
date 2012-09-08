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
// Name : cgProjectorLight.cpp                                               //
//                                                                           //
// Desc : Projector / area light source classes.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgProjectorLight Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgProjectorLight.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgTargetObject.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgTexture.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgMesh.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgTexturePool.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>
#include <Math/cgExtrudedBoundingBox.h>
#include <System/cgStringUtility.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgProjectorLightObject::mInsertProjectorLight;
cgWorldQuery cgProjectorLightObject::mUpdateRanges;
cgWorldQuery cgProjectorLightObject::mUpdateSize;
cgWorldQuery cgProjectorLightObject::mUpdateTiling;
cgWorldQuery cgProjectorLightObject::mUpdateFOV;
cgWorldQuery cgProjectorLightObject::mUpdatePlaybackRate;
cgWorldQuery cgProjectorLightObject::mUpdateShadowRate;
cgWorldQuery cgProjectorLightObject::mUpdateColorSamplerId;
cgWorldQuery cgProjectorLightObject::mLoadProjectorLight;

///////////////////////////////////////////////////////////////////////////////
// cgProjectorLightObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProjectorLightObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightObject::cgProjectorLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgLightObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mOuterRange               = 0.0f;
    mInnerRange               = 0.0f;
    mNearSize                 = cgSizeF(0,0);
    mFarSize                  = cgSizeF(0,0);
    mTiling                   = cgSizeF(1,1);
    mFOV                      = cgSizeF(62,62);
    mShadowUpdateRate         = 0;
    mProjectionPlaybackRate   = 0;
    mProjectionTotalFrames    = 1;
    mLightColorSampler        = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgProjectorLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightObject::cgProjectorLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgLightObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgProjectorLightObject * pObject = (cgProjectorLightObject*)pInit;
    mOuterRange               = pObject->mOuterRange;
    mInnerRange               = pObject->mInnerRange;
    mNearSize                  = pObject->mNearSize;
    mFarSize                   = pObject->mFarSize;
    mTiling                    = pObject->mTiling;
    mFOV                       = pObject->mFOV;
    mShadowUpdateRate         = pObject->mShadowUpdateRate;
    mProjectionTotalFrames    = pObject->mProjectionTotalFrames;
    mProjectionPlaybackRate   = pObject->mProjectionPlaybackRate;

    // ToDo: 6767 - Reintroduce for new shadow settings.
    // Deep copy shadow frustum data
    //m_nShadowFrustumId          = 0;
    //m_ShadowConfig              = pObject->m_ShadowConfig;

    // Duplicate sampler data.
    mLightColorSampler = CG_NULL;
    if ( pObject->mLightColorSampler )
    {
        cgResourceManager * pResources = pWorld->getResourceManager();
        mLightColorSampler = pResources->cloneSampler( pWorld, isInternalReference(), pObject->mLightColorSampler );
        
        // We now own a reference to this sampler
        mLightColorSampler->addReference( this, isInternalReference() );

    } // End if sampler exists
}

//-----------------------------------------------------------------------------
//  Name : ~cgProjectorLightObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightObject::~cgProjectorLightObject()
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
void cgProjectorLightObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    if ( mLightColorSampler )
        mLightColorSampler->removeReference( this, true );
    
    // Clear variables
    mLightColorSampler = CG_NULL;

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
cgWorldObject * cgProjectorLightObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgProjectorLightObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a world object of this specific type, cloned from the provided
/// object.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgProjectorLightObject::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod )
{
    // Valid clone?
    return new cgProjectorLightObject( nReferenceId, pWorld, pInit, InitMethod );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgProjectorLightObject::getDatabaseTable( ) const
{
    return _T("Objects::ProjectorLight");
}

//-----------------------------------------------------------------------------
//  Name : setSize()
/// <summary>
/// Set the new size of the projector light source at the near plane.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setSize( cgFloat fWidth, cgFloat fHeight )
{
    setSize( cgSizeF( fWidth, fHeight ) );
}
void cgProjectorLightObject::setSize( const cgSizeF & Size )
{
    // Is this a no-op?
    if ( mNearSize == Size )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateSize.bindParameter( 1, Size.width );
        mUpdateSize.bindParameter( 2, Size.height );
        mUpdateSize.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateSize.step( true ) == false )
        {
            cgString strError;
            mUpdateSize.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store new near plane size
    mNearSize = Size;

    // Compute the length of the opposite edge of the triangle
    // formed by the rectangle corner point projected onto the far plane
    // and the unknown point describing the far corner of the frustum
    // given the outer spot angle PHI (Tan(a) = Opposite/Adjacent)
    cgFloat fOppositeLengthU = tanf( CGEToRadian(mFOV.width) / 2.0f ) * mOuterRange;
    cgFloat fOppositeLengthV = tanf( CGEToRadian(mFOV.height) / 2.0f ) * mOuterRange;

    // Compute the size of the far plane
    mFarSize = cgSizeF( Size.width + (fOppositeLengthU * 2.0f), Size.height + (fOppositeLengthV * 2.0f) );

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("Size");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setOuterRange()
/// <summary>
/// Set the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setOuterRange( cgFloat fRange )
{
    // Is this a no-op?
    if ( mOuterRange == fRange )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateRanges.bindParameter( 1, fRange );
        mUpdateRanges.bindParameter( 2, mInnerRange );
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
    mOuterRange = fabsf(fRange);

    // Compute the length of the opposite edge of the triangle
    // formed by the rectangle corner point projected onto the far plane
    // and the unknown point describing the far corner of the frustum
    // given the outer spot angle PHI (Tan(a) = Opposite/Adjacent)
    cgFloat fOppositeLengthU = tanf( CGEToRadian(mFOV.width) / 2.0f ) * mOuterRange;
    cgFloat fOppositeLengthV = tanf( CGEToRadian(mFOV.height) / 2.0f ) * mOuterRange;

    // Compute the size of the far plane
    mFarSize = cgSizeF( mNearSize.width + (fOppositeLengthU * 2.0f), mNearSize.height + (fOppositeLengthV * 2.0f) );

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
void cgProjectorLightObject::setInnerRange( cgFloat fRange )
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
//  Name : setFOV()
/// <summary>
/// Set the two "field of view" angles (U&V) that describe the slope of
/// the projector light's frustum planes.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setFOV( cgFloat fUFoV, cgFloat fVFoV )
{
    setFOV( cgSizeF( fUFoV, fVFoV ) );
}
void cgProjectorLightObject::setFOV( const cgSizeF & FoV )
{
    // Is this a no-op?
    if ( mFOV == FoV )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateFOV.bindParameter( 1, FoV.width );
        mUpdateFOV.bindParameter( 2, FoV.height );
        mUpdateFOV.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateFOV.step( true ) == false )
        {
            cgString strError;
            mUpdateFOV.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update FoV angles for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store new angles
    mFOV = FoV;
    
    // Compute the length of the opposite edge of the triangle
    // formed by the rectangle corner point projected onto the far plane
    // and the unknown point describing the far corner of the frustum
    // given the outer spot angle PHI (Tan(a) = Opposite/Adjacent)
    cgFloat fOppositeLengthU = tanf( CGEToRadian(mFOV.width) / 2.0f ) * mOuterRange;
    cgFloat fOppositeLengthV = tanf( CGEToRadian(mFOV.height) / 2.0f ) * mOuterRange;

    // Compute the size of the far plane
    mFarSize = cgSizeF( mNearSize.width + (fOppositeLengthU * 2.0f), mNearSize.height + (fOppositeLengthV * 2.0f) );

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("FoV");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setTiling()
/// <summary>
/// Set the amount to tile the projected light texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setTiling( cgFloat fTileU, cgFloat fTileV )
{
    setTiling( cgSizeF(fTileU, fTileV) );
}
void cgProjectorLightObject::setTiling( const cgSizeF & Tile )
{
    // Is this a no-op?
    if ( mTiling == Tile )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateTiling.bindParameter( 1, Tile.width );
        mUpdateTiling.bindParameter( 2, Tile.height );
        mUpdateTiling.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateTiling.step( true ) == false )
        {
            cgString strError;
            mUpdateTiling.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update tiling factors for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store values
    mTiling = Tile;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("Tiling");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setPlaybackRate ()
/// <summary>
/// Set the rate at which the projector color map animation should play 
/// back (if at all).
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setPlaybackRate( cgUInt32 nRate )
{
    // Is this a no-op?
    if ( mProjectionPlaybackRate == nRate )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdatePlaybackRate.bindParameter( 1, nRate );
        mUpdatePlaybackRate.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdatePlaybackRate.step( true ) == false )
        {
            cgString strError;
            mUpdatePlaybackRate.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update playback rate for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Store values
    mProjectionPlaybackRate = nRate;

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("PlaybackRate");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setShadowUpdateRate ()
/// <summary>
/// Set the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setShadowUpdateRate( cgUInt32 nRate )
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
void cgProjectorLightObject::SetShadowFrustumConfig( const cgShadowGeneratorConfig & Config )
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
//  Name : setLightColorSampler()
/// <summary>
/// Set the sampler details for any projected lighting texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::setLightColorSampler( const cgTextureHandle & hTexture, const cgSamplerStateDesc & States )
{
    cgAssert( mLightColorSampler != CG_NULL );

    // Is this a no-op?
    if ( mLightColorSampler->getStates() == States && 
        mLightColorSampler->getTexture() == hTexture )
        return;
    
    // Update sampler states
    mLightColorSampler->setStates( States );
    mLightColorSampler->setTexture( hTexture );

    // Retrieve the total number of frames (of animation) in the light color texture.
    cgTexture * pTexture = mLightColorSampler->getTexture().getResource(false);
    if ( pTexture != CG_NULL )
        mProjectionTotalFrames = std::max<cgUInt32>( 1, pTexture->getInfo().depth );
    else
        mProjectionTotalFrames = 1;

    // Notify any listeners of this change.
    static const cgString strContext = _T("LightColorSampler");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getOuterRange()
/// <summary>
/// Get the outer range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgProjectorLightObject::getOuterRange( ) const
{
    return mOuterRange;
}

//-----------------------------------------------------------------------------
//  Name : getInnerRange()
/// <summary>
/// Get the inner range of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgProjectorLightObject::getInnerRange( ) const
{
    return mInnerRange;
}

//-----------------------------------------------------------------------------
//  Name : getSize()
/// <summary>
/// Get the size of the projector light source at the near plane.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF & cgProjectorLightObject::getSize( ) const
{
    return mNearSize;
}

//-----------------------------------------------------------------------------
//  Name : getFarSize()
/// <summary>
/// Get the size of the projector light source at the far plane.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF & cgProjectorLightObject::getFarSize( ) const
{
    return mFarSize;
}

//-----------------------------------------------------------------------------
//  Name : getFOV()
/// <summary>
/// Get the two "field of view" angles (U&V) that describe the slope of
/// the projector light's frustum planes.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF & cgProjectorLightObject::getFOV( ) const
{
    return mFOV;
}

//-----------------------------------------------------------------------------
//  Name : getTiling()
/// <summary>
/// Get the amount to tile the projected light texture.
/// </summary>
//-----------------------------------------------------------------------------
const cgSizeF & cgProjectorLightObject::getTiling( ) const
{
    return mTiling;
}

//-----------------------------------------------------------------------------
//  Name : getPlaybackRate ()
/// <summary>
/// Get the rate at which the projector color map animation should play 
/// back (if at all).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgProjectorLightObject::getPlaybackRate( ) const
{
    return mProjectionPlaybackRate;
}

//-----------------------------------------------------------------------------
//  Name : getPlaybackFrames ()
/// <summary>
/// Get the total number of frames in the projector color map animation.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgProjectorLightObject::getPlaybackFrames( ) const
{
    return mProjectionTotalFrames;
}

//-----------------------------------------------------------------------------
//  Name : getShadowUpdateRate ()
/// <summary>
/// Get the maximum rate (in fps) at which the shadow map can be updated.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgProjectorLightObject::getShadowUpdateRate( ) const
{
    return mShadowUpdateRate;
}

/*//-----------------------------------------------------------------------------
//  Name : GetShadowFrustumConfig ()
/// <summary>
/// Get the base shadow frustum configuration parameters.
/// </summary>
//-----------------------------------------------------------------------------
const cgShadowGeneratorConfig & cgProjectorLightObject::GetShadowFrustumConfig( ) const
{
    return m_ShadowConfig;
}*/

//-----------------------------------------------------------------------------
//  Name : getLightColorSampler()
/// <summary>
/// Get the sampler that defines any projected lighting texture.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgProjectorLightObject::getLightColorSampler( ) const
{
    return mLightColorSampler;
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the bounding box of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgProjectorLightObject::getLocalBoundingBox( )
{
    cgBoundingBox Bounds;

    // Generate local bounding box from projector rectangles (light aligned to Z axis).
    cgSizeF MaxSize( max(mNearSize.width, mFarSize.width ), max( mNearSize.height, mFarSize.height ) );
    Bounds.min = cgVector3( -MaxSize.width * 0.5f, -MaxSize.height * 0.5f, 0.0f );
    Bounds.max = cgVector3(  MaxSize.width * 0.5f,  MaxSize.height * 0.5f, mOuterRange );
	return Bounds;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ProjectorLightObject )
        return true;

    // Supported by base?
    return cgLightObject::queryReferenceType( type );
}

/*//-----------------------------------------------------------------------------
//  Name : SetLightingTerms () 
/// <summary>
/// Populate the supplied base lighting terms structure as well as optionally
/// setting up any custom light parameters as required by the effect lib.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::SetLightingTerms( cgBaseLightingTerms & BaseTerms )
{
    cgToDoAssert( "Effect Overhaul", "Implement this." );
    
    // Call base class implementation first to fill out common details.
    cgLightObject::SetLightingTerms( BaseTerms );

    // Setup the light source attenuation data.
    BaseTerms.attenuation.w  = 1.0f / mOuterRange;
    BaseTerms.clipDistance.x = 0.0f;
    BaseTerms.clipDistance.y = mOuterRange;

    // Set other custom effect parameters.
    sgEffectFile * pEffect = m_hEffect;
    if ( pEffect != CG_NULL && pEffect->IsLoaded() == true )
    {
        // Set the cone adjustment values (i.e. widths and heights at the near and far planes)
        // used for the custom projection process applicable to projector lights.
        pEffect->SetValue( _T("TextureTiling"), &mTiling, sizeof(cgVector2) );
	    pEffect->SetVector( _T("coneAdjust"), cgVector4( mNearSize.width, mFarSize.width, mNearSize.height, mFarSize.height ) );
    
    } // End if loaded
    
    // Apply the "light" texture that this light source will project onto the scene
    if ( mLightColorSampler->isTextureValid() == true )
    {
        mLightColorSampler->apply();
    
    } // End if tex valid
    else
    {
        // Use default white texture.
        cgResourceManager * pResources = mWorld->getResourceManager();
        mLightColorSampler->apply( pResources->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->getTexture() );
    
    } // End if tex ! valid

    // Success!
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : beginLighting () 
/// <summary>
/// Assuming the light would like to opt in to the lighting process
/// this method is called to allow the light to set-up ready for that process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::beginLighting( )
{
    cgAssert( mLightColorSampler != CG_NULL );

    // Call base class implementation first.
    if ( !cgLightObject::beginLighting() )
        return false;

    // Apply the "light" texture that this light source will project onto the scene
    if ( mLightColorSampler->isTextureValid() )
    {
        mLightColorSampler->apply();
    
    } // End if tex valid
    else
    {
        // Use default white texture.
        cgResourceManager * pResources = mWorld->getResourceManager();
        mLightColorSampler->apply( pResources->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->getTexture() );
    
    } // End if tex ! valid

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
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
    cgUInt32  Indices[12];
    Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
    Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
    Indices[6]  = 0; Indices[7]  = 3; Indices[8]  = 4;
    Indices[9]  = 0; Indices[10] = 4; Indices[11] = 1;

    // First check to see if the ray intersects the arrow tip pyramid
    cgFloat t, tMin = FLT_MAX;
    bool    bIntersect = false;
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
        if ( fabsf( vIntersect.x ) < (mNearSize.width * 0.5f) + fTolerance && fabsf( vIntersect.y ) < (mNearSize.height * 0.5f) + fTolerance )
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
void cgProjectorLightObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    cgShadedVertex      Points[20];
    cgUInt32            Indices[72], i;

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
    pDriver->setWorldTransform( ObjectTransform );

    // Retrieve useful values
    const cgViewport & Viewport = pDriver->getViewport();
    bool       bOrtho      = (pCamera->getProjectionMode() == cgProjectionMode::Orthographic);
    bool       bSelected   = pIssuer->isSelected();
    cgFloat    fZoomFactor = pCamera->estimateZoomFactor( Viewport.size, pIssuer->getPosition( false ), 2.5f );
    cgUInt32   nColor      = (bSelected) ? 0xFFFFFFFF : 0xFFFFF600;

    // Set the color of each of the points first of all. This saves
    // us from having to set them during object construction.
    for ( i = 0; i < 20; ++i )
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
    
    // Begin rendering
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
            cgUInt32 nCount    = 0;
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
                bCull = (cgCollision::pointClassifyPlane( vCameraPos, cgVector3( 1,0,0 ), -fSize ) != cgPlaneQuery::Front);
            
            // Build indices
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

            // Always add the base plate / plane
            Points[12].position = cgVector3( mNearSize.width * -0.5f, mNearSize.height * -0.5f, 0.0f );
            Points[13].position = cgVector3( mNearSize.width * -0.5f, mNearSize.height *  0.5f, 0.0f );
            Points[14].position = cgVector3( mNearSize.width *  0.5f, mNearSize.height *  0.5f, 0.0f );
            Points[15].position = cgVector3( mNearSize.width *  0.5f, mNearSize.height * -0.5f, 0.0f );
            
            Indices[nCount++] = 12; Indices[nCount++] = 13;
            Indices[nCount++] = 13; Indices[nCount++] = 14;
            Indices[nCount++] = 14; Indices[nCount++] = 15;
            Indices[nCount++] = 15; Indices[nCount++] = 12;

            // Always add the far plate / plane
            Points[16].position = cgVector3( mFarSize.width * -0.5f, mFarSize.height * -0.5f, mOuterRange );
            Points[17].position = cgVector3( mFarSize.width * -0.5f, mFarSize.height *  0.5f, mOuterRange );
            Points[18].position = cgVector3( mFarSize.width *  0.5f, mFarSize.height *  0.5f, mOuterRange );
            Points[19].position = cgVector3( mFarSize.width *  0.5f, mFarSize.height * -0.5f, mOuterRange );
            
            Indices[nCount++] = 16; Indices[nCount++] = 17;
            Indices[nCount++] = 17; Indices[nCount++] = 18;
            Indices[nCount++] = 18; Indices[nCount++] = 19;
            Indices[nCount++] = 19; Indices[nCount++] = 16;

            // Draw lines between near and far plates
            Indices[nCount++] = 12; Indices[nCount++] = 16;
            Indices[nCount++] = 13; Indices[nCount++] = 17;
            Indices[nCount++] = 14; Indices[nCount++] = 18;
            Indices[nCount++] = 15; Indices[nCount++] = 19;

            // Draw the rest of the light source representation
            pDriver->drawIndexedPrimitiveUP( cgPrimitiveType::LineList, 0, 20, nCount / 2, Indices, cgBufferFormat::Index32, Points );
        
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
void cgProjectorLightObject::applyObjectRescale( cgFloat fScale )
{
    // Apply the scale to object-space data
    cgFloat fNewOuterRange  = mOuterRange * fScale;
    cgFloat fNewInnerRange  = mInnerRange * fScale;
    cgFloat fNewNearSizeX   = mNearSize.width * fScale;
    cgFloat fNewNearSizeY   = mNearSize.height * fScale;
    
    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Begin a transaction in case we need to roll back.
        mWorld->beginTransaction( _T("applyObjectRescaleInner") );
        
        // Update ranges
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

        // Update sizes
        mUpdateSize.bindParameter( 1, fNewNearSizeX );
        mUpdateSize.bindParameter( 2, fNewNearSizeY );
        mUpdateSize.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( mUpdateSize.step( true ) == false )
        {
            cgString strError;
            mUpdateSize.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update size for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

            // Roll back range modifications also.
            mWorld->rollbackTransaction( _T("applyObjectRescaleInner") );
            return;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("applyObjectRescaleInner") );
    
    } // End if serialize

    // Update local values.
    mInnerRange     = fNewInnerRange;
    mOuterRange     = fNewOuterRange;
    mNearSize.width  = fNewNearSizeX;
    mNearSize.height = fNewNearSizeY;

    // Compute the length of the opposite edge of the triangle
    // formed by the rectangle corner point projected onto the far plane
    // and the unknown point describing the far corner of the frustum
    // given the outer spot angle PHI (Tan(a) = Opposite/Adjacent)
    cgFloat fOppositeLengthU = tanf( CGEToRadian(mFOV.width) / 2.0f ) * mOuterRange;
    cgFloat fOppositeLengthV = tanf( CGEToRadian(mFOV.height) / 2.0f ) * mOuterRange;

    // Compute the size of the far plane
    mFarSize = cgSizeF( mNearSize.width + (fOppositeLengthU * 2.0f), mNearSize.height + (fOppositeLengthV * 2.0f) );

    // Notify listeners that property was altered
    static const cgString strContext = _T("ApplyRescale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Call base class implementation.
    cgLightObject::applyObjectRescale( fScale );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData() )
        return false;

    // Base class implementation loads required shader(s).
    if ( !cgLightObject::onComponentCreated( e ) )
        return false;

    // Create light color sampler (must come after shader load).
    return createLightColorSampler( true );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ProjectorLightObject::insertComponentData") );

        // Write frustum configuration source to the database first of all.
        /*if ( !m_nShadowFrustumId )
            m_nShadowFrustumId = InsertShadowFrustum( m_ShadowConfig );
        if ( !m_nShadowFrustumId )
        {
            mWorld->rollbackTransaction( _T("ProjectorLightObject::insertComponentData") );
            return false;
        
        } // End if failed*/

        // Insert new spot light object
        prepareQueries();
        mInsertProjectorLight.bindParameter( 1, mReferenceId );
        mInsertProjectorLight.bindParameter( 2, mOuterRange );
        mInsertProjectorLight.bindParameter( 3, mInnerRange );
        mInsertProjectorLight.bindParameter( 4, mNearSize.width );
        mInsertProjectorLight.bindParameter( 5, mNearSize.height );
        mInsertProjectorLight.bindParameter( 6, mFOV.width );
        mInsertProjectorLight.bindParameter( 7, mFOV.height );
        mInsertProjectorLight.bindParameter( 8, mTiling.width );
        mInsertProjectorLight.bindParameter( 9, mTiling.height );
        mInsertProjectorLight.bindParameter( 10, (cgUInt32)0 ); // ToDo: DistanceAttenuationSplineId
        mInsertProjectorLight.bindParameter( 11, (cgUInt32)0 ); // ToDo: AttenuationMaskSamplerId
        if ( mLightColorSampler )
            mInsertProjectorLight.bindParameter( 12, mLightColorSampler->getReferenceId() );
        else
            mInsertProjectorLight.bindParameter( 12, 0 ); // LightSamplerId (defaults to 0 -- updated later in 'createLightColorSampler')
        // ToDo: mInsertProjectorLight.bindParameter( 13, m_nShadowFrustumId );
        mInsertProjectorLight.bindParameter( 14, mShadowUpdateRate );
        mInsertProjectorLight.bindParameter( 15, mProjectionPlaybackRate );

        // Database ref count (just in case it has already been adjusted)
        mInsertProjectorLight.bindParameter( 16, mSoftRefCount );
        
        // Execute
        if ( mInsertProjectorLight.step( true ) == false )
        {
            cgString strError;
            mInsertProjectorLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for projector light object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("ProjectorLightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("ProjectorLightObject::insertComponentData") );

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
bool cgProjectorLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    cgUInt32 nLightSamplerId = 0;

    // Load the light data.
    prepareQueries();
    mLoadProjectorLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadProjectorLight.step( ) || !mLoadProjectorLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadProjectorLight.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for projector light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for projector light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadProjectorLight.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadProjectorLight;

    // Update our local members
    mLoadProjectorLight.getColumn( _T("OuterRange"), mOuterRange );
    mLoadProjectorLight.getColumn( _T("InnerRange"), mInnerRange );
    mLoadProjectorLight.getColumn( _T("SizeU"), mNearSize.width );
    mLoadProjectorLight.getColumn( _T("SizeV"), mNearSize.height );
    mLoadProjectorLight.getColumn( _T("FoVU"), mFOV.width );
    mLoadProjectorLight.getColumn( _T("FoVV"), mFOV.height );
    mLoadProjectorLight.getColumn( _T("TilingU"), mTiling.width );
    mLoadProjectorLight.getColumn( _T("TilingV"), mTiling.height );
    // ToDo: 9999 - DistanceAttenuationSplineId
    // ToDo: 9999 - AttenuationMaskSamplerId
    mLoadProjectorLight.getColumn( _T("LightSamplerId"), nLightSamplerId );
    // ToDo: mLoadProjectorLight.getColumn( _T("ShadowFrustumId"), m_nShadowFrustumId );
    mLoadProjectorLight.getColumn( _T("ShadowUpdateRate"), mShadowUpdateRate );
    mLoadProjectorLight.getColumn( _T("ProjectionPlaybackRate"), mProjectionPlaybackRate );

    // Load frustum configuration source from the database.
    /*if ( m_nShadowFrustumId )
    {
        if ( !LoadShadowFrustum( m_nShadowFrustumId, m_ShadowConfig ) )
        {
            // Release any pending read operation.
            mLoadProjectorLight.reset();
            return false;
        
        } // End if failed
    
    } // End if has frustum*/

    // Compute the size of the far plane based on loaded data.
    cgFloat fOppositeLengthU = tanf( CGEToRadian(mFOV.width) / 2.0f ) * mOuterRange;
    cgFloat fOppositeLengthV = tanf( CGEToRadian(mFOV.height) / 2.0f ) * mOuterRange;
    mFarSize = cgSizeF( mNearSize.width + (fOppositeLengthU * 2.0f), mNearSize.height + (fOppositeLengthV * 2.0f) );

    // Base class implementation reads remaining data and loads required shader(s).
    if ( !cgLightObject::onComponentLoading( e ) )
        return false;

    // Load referenced sampler if one was supplied (must come after shader load).
    cgResourceManager * pResources = mWorld->getResourceManager();
    if ( nLightSamplerId )
    {
        // If we're cloning, we potentially need a new copy of the sampler data,
        // otherwise we can load it as a straight forward wrapped resource.
        cgUInt32 nFlags = 0;
        if ( mReferenceId != e->sourceRefId )
            nFlags = cgResourceFlags::ForceNew;
        mLightColorSampler = pResources->loadSampler( mWorld, mShader, nLightSamplerId, isInternalReference(), nFlags, 0, cgDebugSource() );

        // We have reconnected to this sampler.
        mLightColorSampler->addReference( this, true );

    } // End if load sampler

    // If no sampler was loaded, create one.
    if ( !mLightColorSampler )
    {
        if ( !createLightColorSampler( ( mReferenceId == e->sourceRefId ) ) )
            return false;
    
    } // End no sampler
    
    // Retrieve the total number of frames (of animation) in the light color texture.
    cgTexture * pTexture = mLightColorSampler->getTexture().getResource(false);
    if ( pTexture != CG_NULL )
        mProjectionTotalFrames = std::max<cgUInt32>( 1, pTexture->getInfo().depth );
    else
        mProjectionTotalFrames = 1;

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
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::onComponentDeleted( )
{
    // Remove our physical references to any child samplers. Full database update 
    // and potentially removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case.
    if ( mLightColorSampler )
        mLightColorSampler->removeReference( this, isInternalReference() );
    mLightColorSampler = CG_NULL;
    
    // Call base class implementation last.
    cgLightObject::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertProjectorLight.isPrepared() == false )
            mInsertProjectorLight.prepare( mWorld, _T("INSERT INTO 'Objects::ProjectorLight' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16)"), true );
        if ( mUpdateRanges.isPrepared() == false )
            mUpdateRanges.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET OuterRange=?1, InnerRange=?2 WHERE RefId=?3"), true );
        if ( mUpdateSize.isPrepared() == false )
            mUpdateSize.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET SizeU=?1, SizeV=?2 WHERE RefId=?3"), true );
        if ( mUpdateFOV.isPrepared() == false )
            mUpdateFOV.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET FoVU=?1, FoVV=?2 WHERE RefId=?3"), true );
        if ( mUpdateTiling.isPrepared() == false )
            mUpdateTiling.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET TilingU=?1, TilingV=?2 WHERE RefId=?3"), true );
        if ( mUpdateShadowRate.isPrepared() == false )
            mUpdateShadowRate.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET ShadowUpdateRate=?1 WHERE RefId=?2"), true );
        if ( mUpdatePlaybackRate.isPrepared() == false )
            mUpdatePlaybackRate.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET ProjectionPlaybackRate=?1 WHERE RefId=?2"), true );
        if ( mUpdateColorSamplerId.isPrepared() == false )
            mUpdateColorSamplerId.prepare( mWorld, _T("UPDATE 'Objects::ProjectorLight' SET LightSamplerId=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadProjectorLight.isPrepared() == false )
        mLoadProjectorLight.prepare( mWorld, _T("SELECT * FROM 'Objects::ProjectorLight' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : createLightColorSampler() (Protected)
/// <summary>
/// Create a new light color sampler -- serialized or internal as necessary.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::createLightColorSampler( bool bSerialize )
{
    // No-op?
    if ( mLightColorSampler )
        return true;

    // Create sampler object
    cgResourceManager * pResources = mWorld->getResourceManager();
    if ( isInternalReference() )
        mLightColorSampler = pResources->createSampler( _T("LightColor"), mShader );
    else
        mLightColorSampler = pResources->createSampler( mWorld, _T("LightColor"), mShader );

    // We now hold a reference to this sampler. Full reference for new sampler.
    mLightColorSampler->addReference( this, isInternalReference() );

    // Serialize sampler identifier as necessary.
    if ( shouldSerialize() && bSerialize )
    {
        prepareQueries();
        mUpdateColorSamplerId.bindParameter( 1, mLightColorSampler->getReferenceId() );
        mUpdateColorSamplerId.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateColorSamplerId.step( true ) == false )
        {
            cgString strError;
            mUpdateColorSamplerId.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update light color sampler identifier for projector light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return false;
        
        } // End if failed

    } // End if serialize

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getLightType () (Virtual)
/// <summary>
/// Determine the type of this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::LightType cgProjectorLightObject::getLightType( ) const
{
    return Light_Projector;
}

// ToDo: Remove when complete
/*//-----------------------------------------------------------------------------
//  Name : Deserialize ()
/// <summary>
/// Initialize the object based on the XML data pulled from the 
/// environment / scene definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightObject::Deserialize( const gpXMLNode & InitData, gpSceneLoader * pLoader )
{
    gpXMLNode xChild, xProjection, xAttenShadow;

    // Allow base class to process data first
    if ( gpLightObject::Deserialize( InitData, pLoader ) == false )
        return false;

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
            if ( xAttenShadow.isEmpty() == false )
                m_DistanceAttenCurve.ParseXML( xAttenShadow );

            // Retrieve the attenuation mask data (if any)
            xAttenShadow     = xChild.GetChildNode( _T("Mask") );
            m_xAttenMaskData = xAttenShadow.GetChildNode( _T("Sampler") );
            
        } // End if Attenuation
        else if ( xChild.IsOfType( _T("Projection") ) )
        {
            for ( cgUInt32 j = 0; j < xChild.GetChildNodeCount(); ++j )
            {
                xProjection = xChild.GetChildNode( j );

                if ( xProjection.IsOfType( _T("Sampler") ) )
                {
                    m_xLightColorData = xProjection;

                } // End if Sampler
                else if ( xProjection.IsOfType( _T("UFoV") ) )
                {
                    cgFloat fValue;
                    cgStringParser( xProjection.GetText() ) >> fValue;
                    setFOV( fValue, m_fVFoV );

                } // End if UFoV
                else if ( xProjection.IsOfType( _T("VFoV") ) )
                {
                    cgFloat fValue;
                    cgStringParser( xProjection.GetText() ) >> fValue;
                    setFOV( m_fUFoV, fValue );

                } // End if VFoV
                else if ( xProjection.IsOfType( _T("Size") ) )
                {   
                    // Extract the size.
                    cgVector2 vSize;
                    if ( cgStringUtility::TryParse( xChild.GetText(), vSize ) == true )
                        setSize( vSize.x, vSize.y );

                } // End if Size
                else if ( xProjection.IsOfType( _T("Tiling") ) )
                {
                    // Extract the size.
                    cgVector2 vTiling;
                    if ( cgStringUtility::TryParse( xChild.GetText(), vTiling ) == true )
                        setTiling( vTiling.x, vTiling.y );

                } // End if Tiling
                else if ( xProjection.IsOfType( _T("PlaybackRate") ) )
                {
                    cgUInt32 nValue;
                    cgStringParser( xProjection.GetText() ) >> nValue;
                    setPlaybackRate( nValue );

                } // End if PlaybackRate

            } // Next Child
        
        } // End if Projection
        else if ( xChild.IsOfType( _T("Shadowing") ) )
        {
            // Iterate through all child nodes to get shadowing settings
            for ( cgUInt32 j = 0; j < xChild.GetChildNodeCount(); ++j )
            {
                xAttenShadow = xChild.GetChildNode( j );

                // What type of node is this?
                if ( xAttenShadow.IsOfType( _T("Frustum") ) )
                {
                    if ( mShadowFrustum->Deserialize( xAttenShadow ) == false )
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
bool cgProjectorLightObject::InitObject( bool bAutoAddObject = true )
{
    // Allow base class to process
    if ( gpLightObject::InitObject( bAutoAddObject ) == false )
        return false;

    // Get access to required systems.
    gpResourceManager * pResources = mParentScene->getResourceManager();
    gpRenderDriver    * pDriver    = mParentScene->getRenderDriver();

    // Create a custom box (if it doesn't already exist) that will allow us
    // to represent the shape of the light source. We use a custom vertex shader
    // to perform the transformation into the correct shape and thus the following
    // mesh definition simply creates a box shape used as input.
    if ( pResources->getMesh( &mLightShape, _T("Core::LightShapes::Projector") ) == false )
    {
        cgUInt32    Indices[6];
        cgVector3 Vertices[8];
	    gpMesh    * pMesh = new gpMesh();

        // Set the mesh data format
        const gpVertexFormat * pFormat = gpVertexFormat::formatFromFVF( D3DFVF_XYZ );
        pMesh->prepareMesh( pFormat, false, pResources );

	    // Box vertices
        Vertices[0] = cgVector3( -0.5f,  0.5f, 0.0f );
        Vertices[1] = cgVector3( -0.5f,  0.5f, 1.0f );
        Vertices[2] = cgVector3(  0.5f,  0.5f, 1.0f );
        Vertices[3] = cgVector3(  0.5f,  0.5f, 0.0f );
        Vertices[4] = cgVector3( -0.5f, -0.5f, 0.0f );
        Vertices[5] = cgVector3( -0.5f, -0.5f, 1.0f );
        Vertices[6] = cgVector3(  0.5f, -0.5f, 1.0f );
        Vertices[7] = cgVector3(  0.5f, -0.5f, 0.0f );
	    pMesh->setVertexSource( Vertices, 8, pFormat );

        // +X indices
        Indices[0]  = 3; Indices[1]  = 2; Indices[2]  = 6;
        Indices[3]  = 3; Indices[4]  = 6; Indices[5]  = 7;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

        // -X indices
        Indices[0]  = 1; Indices[1]  = 0; Indices[2]  = 4;
        Indices[3]  = 1; Indices[4]  = 4; Indices[5]  = 5;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

        // +Y indices
        Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
        Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

        // -Y indices
        Indices[0]  = 4; Indices[1]  = 6; Indices[2]  = 5;
        Indices[3]  = 4; Indices[4]  = 7; Indices[5]  = 6;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

        // +Z indices
        Indices[0]  = 1; Indices[1]  = 6; Indices[2]  = 2;
        Indices[3]  = 1; Indices[4]  = 5; Indices[5]  = 6;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

        // -Z indices
        Indices[0]  = 0; Indices[1]  = 3; Indices[2]  = 7;
        Indices[3]  = 0; Indices[4]  = 7; Indices[5]  = 4;
	    pMesh->addPrimitives( gpPrimitiveType::TriangleList, Indices, 0, 6, ResourceHandle(), 0 );

	    // Build the mesh
        pMesh->endPrepare( );
        
        // Add to the resource manager
        pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::Projector"), gpDebugSource() );

    } // End if no mesh

    // Initialize the frustums as required
    const gpShadowMapPool::Config & Settings = mParentScene->GetShadowMaps()->GetConfig();

    // Compute the correct shadow frustum camera details.
    gpCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
    pFrustumCamera->SetObjectMatrix( GetObjectMatrix() );
    pFrustumCamera->setFOV( 90.0f );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 0.0f );
    pFrustumCamera->setFarClip( mOuterRange );
    
    // Set frustum properties
    mShadowFrustum->SetResolutionRanges( Settings.nMinResolution, Settings.nMaxResolution );

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    if ( mShadowUpdateRate > 0 )
        mShadowTimeSinceLast = -gpMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)mShadowUpdateRate );

    // Import the attenuation mask texture if supplied.
    if ( m_xAttenMaskData.isEmpty() == false )
    {
        if ( (m_pAttenMaskSampler = pDriver->createSampler( _T("LightAtten"), m_hEffect ) ) != CG_NULL )
        {
            // Parse sampler.
            if ( m_pAttenMaskSampler->ParseXML( m_xAttenMaskData ) == false )
            {
                delete m_pAttenMaskSampler;
                m_pAttenMaskSampler = CG_NULL;
            
            } // End if failed
            else
            {
                // Attenuation mask sampler MUST use border mode.
                gpSamplerStates & States = m_pAttenMaskSampler->getStates();
                States.AddressU = D3DTADDRESS_BORDER;
                States.AddressV = D3DTADDRESS_BORDER;
                States.AddressW = D3DTADDRESS_BORDER;

            } // End if success

        } // End if valid

        // Free up unnecessary memory.
        m_xAttenMaskData = gpXMLNode();

    } // End if Sampler

    // Create a 1D distance attenuation texture if the attenuation curve does
    // not describe standard linear attenuation.
    if ( m_DistanceAttenCurve.IsLinear() == false || m_pAttenMaskSampler != CG_NULL )
    {
        // Generate a texture for this curve unless one has already been generated
        cgString strCurveName = _T("Core::Curves::256::") + m_DistanceAttenCurve.ComputeHash( 2 );
        pResources->CreateTexture( &m_hDistanceAttenTex, m_DistanceAttenCurve, 256, 1, true, 0, strCurveName, gpDebugSource() );

        // Create a sampler for distance attenuation and bind the above texture
        if ( m_hDistanceAttenTex.IsValid() == true )
        {
            if ( (m_pDistanceAttenSampler = pDriver->createSampler( _T("LightDistanceAtten"), m_hEffect ) ) != CG_NULL )
                m_pDistanceAttenSampler->setTexture( m_hDistanceAttenTex );

        } // End if valid

    } // End if not linear

    // Projector lights by definition project a texture, as a form of light, into the scene.
    // If one has been provided, load it here.
    if ( (mLightColorSampler = pDriver->createSampler( _T("LightColor"), m_hEffect ) ) != CG_NULL )
    {
        // Custom data supplied?
        if ( m_xLightColorData.isEmpty() == false )
        {
            // Parse sampler.
            if ( mLightColorSampler->ParseXML( m_xLightColorData ) == false )
            {
                delete mLightColorSampler;
                mLightColorSampler = CG_NULL;
            
            } // End if failed

            // Free up unnecessary memory
            m_xLightColorData = gpXMLNode();

        } // End if Supplied
        else
        {
            ResourceHandle hTexture;

            // Otherwise, use a default white texture.
            if ( pResources->LoadTexture( &hTexture, _T("sys://Textures/DefaultDiffuse.dds"), 0, gpDebugSource() ) == false ||
                 mLightColorSampler->setTexture( hTexture ) == false )
            {
                delete mLightColorSampler;
                mLightColorSampler = CG_NULL;
            
            } // End if failed
            
        } // End if use default
        
    } // End if valid

    // Projector lights /must/ have a light color sampler!
    if ( mLightColorSampler == CG_NULL )
    {
        gpAppLog::write( gpAppLog::Error, _T("Failed to load 'light' texture for projector light source '%s'.\n"), GetInstanceName().c_str() );
        return false;

    } // End if failed

    // Retrieve the total number of frames (of animation) in the light color texture.
    gpTexture * pTexture = mLightColorSampler->getTexture();
    mProjectionTotalFrames = std::max<cgUInt32>( 1, pTexture->getInfo().depth );
    
    // Success
    return true;
}*/

///////////////////////////////////////////////////////////////////////////////
// cgProjectorLightNode Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgProjectorLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightNode::cgProjectorLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgLightNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults
    mShadowFrustum            = new cgShadowGenerator( this, 0 );
    mIndirectFrustum          = new cgReflectanceGenerator( this, 0 );
    mProjectionCurrentFrame   = 0;
    mShadowTimeSinceLast      = 0.0f;
    mProjectionTimeLast       = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : cgProjectorLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightNode::cgProjectorLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    mShadowFrustum            = new cgShadowGenerator( this, 0 );
    mIndirectFrustum          = new cgReflectanceGenerator( this, 0 );
    mProjectionCurrentFrame   = 0;
    mShadowTimeSinceLast      = 0.0f;
    mProjectionTimeLast       = 0.0f;
}

//-----------------------------------------------------------------------------
//  Name : ~cgProjectorLightNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgProjectorLightNode::~cgProjectorLightNode()
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
void cgProjectorLightNode::dispose( bool bDisposeBase )
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
    mProjectorLightConstants.close();
    
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
bool cgProjectorLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ProjectorLightNode )
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
cgObjectNode * cgProjectorLightNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgProjectorLightNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgProjectorLightNode::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgProjectorLightNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::postCreate( )
{
    // Call base class implementation.
    if ( !cgLightNode::postCreate( ) )
        return false;

    // Retrieve the required properties from the referenced light object.
    cgFloat fRange = getOuterRange();
    
    // Always force at least a small range to prevent invalid
    // light shape matrix from being produced.
    if ( fRange < CGE_EPSILON_1MM )
        fRange = CGE_EPSILON_1MM;

    // Initialize the frustums as required
    if ( !mShadowFrustum->initialize( ) )
        return false;
    // ToDo: 6767 - Reintroduce?
    //if ( !mIndirectFrustum->initialize( ) )
        //return false;

    // Compute the correct shadow frustum camera details.
    cgCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
    pFrustumCamera->setWorldTransform( getWorldTransform( false ) );
    pFrustumCamera->setFOV( 90.0f );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 0.0f );
    pFrustumCamera->setFarClip( fRange );

    // Compute the correct indirect frustum camera details.
    // ToDo: 6767 - Reintroduce?
    /*pFrustumCamera = mIndirectFrustum->getCamera();
    pFrustumCamera->setWorldTransform( getWorldTransform( false ) );
    pFrustumCamera->setFOV( 90.0f );
    pFrustumCamera->setAspectRatio( 1.0f, true );
    pFrustumCamera->setNearClip( 0.0f );
    pFrustumCamera->setFarClip( fRange );*/

    // If shadow update rate is being limited, we compute a random offset here
    // to attempt to stagger shadow updates between different light sources in 
    // the scene (simple but effective).
    if ( getShadowUpdateRate() > 0 )
        mShadowTimeSinceLast = -cgMathUtility::randomFloat( 0.0f, 1.0f / (cgFloat)getShadowUpdateRate() );

    // Generate the projection frustum.
    updateFrustum();

    // Create constant buffers
    cgResourceManager * pResources = mParentScene->getResourceManager();
    if ( !pResources->createConstantBuffer( &mProjectorLightConstants, mShader, _T("_cbProjectorLight"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required projector light source constant buffers for object node 0x%x in scene '%s'.\n"), mReferenceId, mParentScene->getName().c_str() );
        return false;
    
    } // End if failed
    cgAssert( mProjectorLightConstants->getDesc().length == sizeof(_cbProjectorLight) );

    // Create the required light shape.
    return createLightShape();
}

//-----------------------------------------------------------------------------
// Name : createLightShape () (Protected)
/// <summary>
/// Create the light shape mesh that is applicable to this spot light.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::createLightShape( )
{
    // Get access to required systems.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    cgRenderDriver    * pDriver    = mParentScene->getRenderDriver();

    // Create a custom box (if it doesn't already exist) that will allow us
    // to represent the shape of the light source. We use a custom vertex shader
    // to perform the transformation into the correct shape and thus the following
    // mesh definition simply creates a box shape used as input.
    if ( pResources->getMesh( &mLightShape, _T("Core::LightShapes::Projector") ) == false )
    {
        cgUInt32  Indices[6];
        cgVector3 Vertices[8];
        cgMesh  * pMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), CG_NULL );

        // Set the mesh data format
        cgVertexFormat * pFormat = cgVertexFormat::formatFromFVF( D3DFVF_XYZ );
        pMesh->prepareMesh( pFormat, false, pResources );

        // Box vertices
        Vertices[0] = cgVector3( -0.5f,  0.5f, 0.0f );
        Vertices[1] = cgVector3( -0.5f,  0.5f, 1.0f );
        Vertices[2] = cgVector3(  0.5f,  0.5f, 1.0f );
        Vertices[3] = cgVector3(  0.5f,  0.5f, 0.0f );
        Vertices[4] = cgVector3( -0.5f, -0.5f, 0.0f );
        Vertices[5] = cgVector3( -0.5f, -0.5f, 1.0f );
        Vertices[6] = cgVector3(  0.5f, -0.5f, 1.0f );
        Vertices[7] = cgVector3(  0.5f, -0.5f, 0.0f );
	    pMesh->setVertexSource( Vertices, 8, pFormat );

        // +X indices
        Indices[0]  = 3; Indices[1]  = 2; Indices[2]  = 6;
        Indices[3]  = 3; Indices[4]  = 6; Indices[5]  = 7;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

        // -X indices
        Indices[0]  = 1; Indices[1]  = 0; Indices[2]  = 4;
        Indices[3]  = 1; Indices[4]  = 4; Indices[5]  = 5;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

        // +Y indices
        Indices[0]  = 0; Indices[1]  = 1; Indices[2]  = 2;
        Indices[3]  = 0; Indices[4]  = 2; Indices[5]  = 3;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

        // -Y indices
        Indices[0]  = 4; Indices[1]  = 6; Indices[2]  = 5;
        Indices[3]  = 4; Indices[4]  = 7; Indices[5]  = 6;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

        // +Z indices
        Indices[0]  = 1; Indices[1]  = 6; Indices[2]  = 2;
        Indices[3]  = 1; Indices[4]  = 5; Indices[5]  = 6;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

        // -Z indices
        Indices[0]  = 0; Indices[1]  = 3; Indices[2]  = 7;
        Indices[3]  = 0; Indices[4]  = 7; Indices[5]  = 4;
	    pMesh->addPrimitives( cgPrimitiveType::TriangleList, Indices, 0, 6, cgMaterialHandle::Null, 0 );

	    // Build the mesh
        pMesh->endPrepare( );
        
        // Add to the resource manager
        pResources->addMesh( &mLightShape, pMesh, 0, _T("Core::LightShapes::Projector"), cgDebugSource() );

    } // End if no mesh

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getFrustum ()
/// <summary>
/// Retrieve the frustum for this light source.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgProjectorLightNode::getFrustum( ) const
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
bool cgProjectorLightNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgLightNode::setCellTransform( Transform, Source ) )
        return false;

    // If this node has a target node associated with it, update 
    // the projector light's range to match the distance between the two.
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
void cgProjectorLightNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What property was modified?
    if ( e->context == _T("OuterRange") || e->context == _T("ApplyRescale") )
    {
        // Retrieve the required properties from the referenced light object.
        cgFloat fRange = getOuterRange();

        // Recompute the projection frustum.
        updateFrustum();
        
        // Update shadow frustum camera's far clip plane to match range of light source.
        cgCameraNode * pFrustumCamera = mShadowFrustum->getCamera();
        pFrustumCamera->setFarClip( std::min<cgFloat>(CGE_EPSILON_1MM, fRange) );

        // And the same for the indirect frustum camera.
        pFrustumCamera = mIndirectFrustum->getCamera();
        pFrustumCamera->setFarClip( std::min<cgFloat>(CGE_EPSILON_1MM, fRange) );
        
        // Light source node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

        // If in target mode (and we're not already updating in response
        // to a target node update), push target to correct distance based
        // on the spot light's altered range.
        if ( mTargetNode && !mTargetNode->isUpdating() )
            mTargetNode->setPosition( getPosition() + (mTargetNode->getZAxis( ) * fRange) );
    
    } // End if OuterRange | ApplyRescale
    else if ( e->context == _T("FoV") || e->context == _T("Size") )
    {
        // Recompute the projection frustum.
        updateFrustum();
        
        // Light source node is now 'dirty' and should always 
        // re-trigger a shadow map fill during the next render
        // process if necessary.
        nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );
    
    } // End if FoV | Size

    // Call base class implementation last
    cgLightNode::onComponentModified( e );
    
}

//-----------------------------------------------------------------------------
//  Name : updateFrustum() (Protected)
/// <summary>
/// Update the frustum based on current projector light settings such as
/// FoV angles, range etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightNode::updateFrustum()
{
    cgPlane   Planes[6];
    cgVector3 vecPoints[8], vecCenter;
    cgVector2 vecSizeN, vecSizeF;

    // Extract new light properties
    const cgVector3 & vecRight = getXAxis(false), & vecUp = getYAxis(false), & vecLook = getZAxis(false);
    const cgVector3 & vecPosition = getPosition(false);

    // Generate the 8 points of the frustum
    vecSizeN = (cgVector2&)getSize() * 0.5f;
    vecSizeF = (cgVector2&)getFarSize() * 0.5f;

	// Near Plane (TL,TR,BR,BL)
    vecCenter    = vecPosition;
	vecPoints[0] = (vecCenter - (vecRight * vecSizeN.x)) + (vecUp * vecSizeN.y);
	vecPoints[1] = (vecCenter + (vecRight * vecSizeN.x)) + (vecUp * vecSizeN.y);
	vecPoints[2] = (vecCenter + (vecRight * vecSizeN.x)) - (vecUp * vecSizeN.y);
	vecPoints[3] = (vecCenter - (vecRight * vecSizeN.x)) - (vecUp * vecSizeN.y);

	// Far Plane (TL,TR,BR,BL)
	vecCenter   += vecLook * getOuterRange();
	vecPoints[4] = (vecCenter - (vecRight * vecSizeF.x)) + (vecUp * vecSizeF.y);
	vecPoints[5] = (vecCenter + (vecRight * vecSizeF.x)) + (vecUp * vecSizeF.y);
	vecPoints[6] = (vecCenter + (vecRight * vecSizeF.x)) - (vecUp * vecSizeF.y);
	vecPoints[7] = (vecCenter - (vecRight * vecSizeF.x)) - (vecUp * vecSizeF.y);

    // Build the 6 planes of the projector light source
    const cgSizeF & FoV = getFOV();
	cgFloat fCosU = cosf( CGEToRadian( FoV.width ) * 0.5f );
	cgFloat fSinU = sinf( CGEToRadian( FoV.width ) * 0.5f );
    cgFloat fCosV = cosf( CGEToRadian( FoV.height ) * 0.5f );
	cgFloat fSinV = sinf( CGEToRadian( FoV.height ) * 0.5f );
    cgPlane::fromPointNormal( Planes[cgVolumePlane::Left]  , vecPoints[3], ((-vecRight * fCosU) - (vecLook * fSinU)) );
	cgPlane::fromPointNormal( Planes[cgVolumePlane::Right] , vecPoints[1], ((vecRight * fCosU) - (vecLook * fSinU)) );
	cgPlane::fromPointNormal( Planes[cgVolumePlane::Bottom], vecPoints[2], ((-vecUp * fCosV) - (vecLook * fSinV)) );
	cgPlane::fromPointNormal( Planes[cgVolumePlane::Top]   , vecPoints[0], ((vecUp * fCosV) - (vecLook * fSinV)) );
    cgPlane::fromPointNormal( Planes[cgVolumePlane::Near]  , vecPoints[0], -vecLook );    
	cgPlane::fromPointNormal( Planes[cgVolumePlane::Far]   , vecPoints[4], vecLook );

	// Assign the planes to our frustum
	mFrustum.setPlanes( Planes );
    mFrustum.position = vecPosition;
}

// ToDo: The following volume tests are frustum based tests, which could pose a problem
//       under certain cone geometry rendering conditions. We need to investigate this 
//       further when we update our intersection testing functions to determine what we want to do.

//-----------------------------------------------------------------------------
//  Name : boundsInVolume () (Virtual)
/// <summary>
/// Does the specified bounding box fall within the volume of this light 
/// source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::boundsInVolume( const cgBoundingBox & AABB )
{
    return mFrustum.testAABB( AABB );
}

//-----------------------------------------------------------------------------
//  Name : pointInVolume () (Virtual)
/// <summary>
/// Does the specified point fall within the volume of this light source?
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::pointInVolume( const cgVector3 & Point, cgFloat fErrRadius /* = 0.0f */ )
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
bool cgProjectorLightNode::frustumInVolume( const cgFrustum & Frustum )
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
bool cgProjectorLightNode::testObjectShadowVolume( cgObjectNode * pObject, const cgFrustum & ViewFrustum )
{
    cgBoundingBox ObjectAABB = pObject->getBoundingBox();

    // If the light source falls inside the bounding box of the object, 
    // this can automatically cast shadows.
    if ( ObjectAABB.containsPoint( getPosition( false ) ) )
        return true;

    // Compute an extruded bounding box for the object. This extrudes the box edges
    // away from the light source position (just like extruding an actual mesh shadow
    // volume when implementing stencil shadows).
    // ToDo: 9999 - Does this work properly for projector lights?
    cgExtrudedBoundingBox ExtrudedAABB( ObjectAABB, getPosition( false ), getOuterRange()  * 1.73205f /* sqrt((1*1)+(1*1)+(1*1)) */ );

    // Test to see if the box intersects the frustum.
    if ( ViewFrustum.testExtrudedAABB( ExtrudedAABB ) )
        return true;

    // Cannot cast a shadow into the view frustum
    return false;
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Virtual)
/// <summary>
/// Compute the visibility set from the point of view of this light.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightNode::computeVisibility( )
{
    // If it was determined that we should calculate shadows,
    // attempt to refine this further to see if we /really/
    // need to based on the state(s) of our shadow frustums.
    if ( isShadowSource() )
    {
        // Allow the frustum to compute its own local shadow set information.
        // This will return 'true' if it is deemed necessary to compute shadows
        // or at least render from a pool shadow map.
        // ToDo: 6767 - Reintroduce!
        /*if ( !mShadowFrustum->ComputeVisibilitySet( pCamera ) )*/
            mComputeShadows = false;

    } // End if mComputeShadows

    // Call base class implementation last
    cgLightNode::computeVisibility();
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail () (Virtual)
/// <summary>
/// Computes level of detail settings for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
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
            for ( size_t i = 0; i < aDescriptions.size(); ++i )
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
void cgProjectorLightNode::updateIndirectSettings( )
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
        for ( size_t i = 0; i < aDescriptions.size(); ++i )
        {
            aDescriptions[ i ].bufferDesc.width  = nResolution;
            aDescriptions[ i ].bufferDesc.height = nResolution;
        
        } // Next resource

        // If we are downsampling the RSM, update the resolutions of the recipient targets based on the tap count
        if ( mIndirectSystemSettings.boxFilter && aDescriptions.size() > 4 )
        {
            cgAssert( mIndirectSystemSettings.primarySamples > 0 );
            for ( size_t i = 4; i < aDescriptions.size(); ++i )
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
void cgProjectorLightNode::setClipPlanes( )
{
    // Get access to required systems / objects
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();

	// Set the user-defined clip planes for the frustum
    pDriver->setUserClipPlanes( mFrustum.planes, 6 );

    // Update state
	mSetClipPlanes = true;
}

/*//-----------------------------------------------------------------------------
//  Name : ComputeShadowSets( ) (Virtual)
/// <summary>
/// Creates the shadow mapping visibility sets that are also found within
/// the specified visibility set (i.e. main player camera). This 
/// essentially narrows down the list of visible objects from the
/// point of view of the light source to only those that can also be
/// seen by the camera.
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightNode::ComputeShadowSets( cgCameraNode * pCamera )
{
    // Allow base class to process first.
    cgLightNode::ComputeShadowSets( pCamera );

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
            bCalcShadows |= mShadowFrustum->ComputeShadowSets( pCamera );

        } // Next Frustum

        // Is there /still/ any need to assume that this is a shadow source?
        if ( !bCalcShadows )
            mComputeShadows = false;

    } // End if mComputeShadows
}*/

// ToDo: 6767 - For all light types, roll ComputeIndirectSets into compute visibility like we did for ComputeShadowSets
/*//-----------------------------------------------------------------------------
//  Name : ComputeIndirectSets( ) (Virtual)
/// <summary>
/// Creates the rsm visibility sets
/// </summary>
//-----------------------------------------------------------------------------
void cgProjectorLightNode::ComputeIndirectSets( cgCameraNode * pCamera )
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
        if ( !mIndirectFrustum->ComputeVisibilitySet( pCamera ) )
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
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::reassignShadowMaps( cgTexturePool * pPool )
{
    // Ignore the frustum if we are not a shadow source, are disabled, or have
    // no visible objects to process.
    if ( !isShadowSource() || !mShadowFrustum->containsRenderableObjects() )
    {
        // Release shadow resources back to the pool.
        mShadowFrustum->releaseResources();
        return true;

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
bool cgProjectorLightNode::reassignIndirectMaps( cgTexturePool * pPool )
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
cgInt32 cgProjectorLightNode::beginShadowFill( cgTexturePool * pPool )
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

    // If there was literally nothing to do, return -1 (don't even process).
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

    // Bind the custom projector lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mProjectorLightConstants );

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
bool cgProjectorLightNode::beginShadowFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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

    // Did not fill.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFillPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::endShadowFillPass( )
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
bool cgProjectorLightNode::endShadowFill( )
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
bool cgProjectorLightNode::updateLightConstants()
{
    // Determine whether or not we should advance the current frame of the
    // projector light's animated color map (if any).
    cgFloat   fFrameOffset = 0.0f;
    cgTimer * pTimer       = cgTimer::getInstance();
    if ( getPlaybackRate() > 0 )
    {
        cgDouble fElapsedTime = (pTimer->getTime() - mProjectionTimeLast);
        cgDouble fFrameTime   = (1.0 / (cgDouble)getPlaybackRate());
        
        // Has enough time elapsed since the last update?
        if ( fElapsedTime >= fFrameTime )
        {
            mProjectionCurrentFrame = (mProjectionCurrentFrame + (cgUInt32)(fElapsedTime / fFrameTime) ) % getPlaybackFrames();
            mProjectionTimeLast = pTimer->getTime();
        
        } // End if enough time elapsed

        // Compute offset for smooth frame interpolation.
        fFrameOffset = (cgFloat)((pTimer->getTime() - mProjectionTimeLast) / fFrameTime);

    } // End if animation update

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
    pLightData->attenuation.z  = fOuterRange / (fOuterRange - fInnerRange);
    pLightData->attenuation.w  = -(fInnerRange / (fOuterRange - fInnerRange));
    pLightData->clipDistance.x = 0.0f;
    pLightData->clipDistance.y = fOuterRange;
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pLightBuffer->unlock();

    // Populate custom projector light terms
    pLightBuffer = mProjectorLightConstants.getResource( true );
    cgAssert( pLightBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbProjectorLight * pProjectorLightData = CG_NULL;
    if ( !(pProjectorLightData = (_cbProjectorLight*)pLightBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock projector light data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Set U/V tiling factors
    pProjectorLightData->textureData.x = getTiling().width;
    pProjectorLightData->textureData.y = getTiling().height;

    // ...and the current frame.
    cgFloat fFrame = ((cgFloat)mProjectionCurrentFrame + fFrameOffset) / (cgFloat)getPlaybackFrames();
    pProjectorLightData->textureData.z = fFrame;

    // Set the cone adjustment scalars.
    const cgSizeF & NearSize = getSize();
    const cgSizeF & FarSize  = getFarSize();
    pProjectorLightData->coneAdjust = cgVector4( NearSize.width, FarSize.width, NearSize.height, FarSize.height );
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pLightBuffer->unlock();

    // Call base class implementation LAST
    return cgLightNode::updateLightConstants();
}

//-----------------------------------------------------------------------------
//  Name : updateSystemConstants() (Virtual)
/// <summary>
/// Update the lighting system's constant buffer as necessary whenever any of the
/// appropriate matrices change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::updateSystemConstants()
{
    // Call base class implementation first.
    if ( !cgLightNode::updateSystemConstants() )
        return false;

    // Override the current direction
    cgVector3 direction = getZAxis(false);
    static const cgString strConstantLightDirection = _T("_lightDirection");
    mParentScene->getLightingManager()->setConstant( strConstantLightDirection, &direction );

    // Override the texture projection matrix.
    cgMatrix mtxTexRemap;
    cgMatrix::identity( mtxTexRemap );
    mtxTexRemap._22 = -1.0f; mtxTexRemap._33 = 1.0f / getOuterRange();
    static const cgString strConstantLightTexProjMatrix = _T("_lightTexProjMatrix");
    mParentScene->getLightingManager()->setConstant( strConstantLightTexProjMatrix, &(getViewMatrix() * mtxTexRemap) );

    // Is this a 3D color texture?
    cgSampler * pSampler = getLightColorSampler();
    if ( pSampler && pSampler->isTextureValid() )
    {
        cgTexture * pTexture = pSampler->getTexture().getResourceSilent();
        if ( pTexture->getInfo().type == cgBufferType::Texture3D && pTexture->getInfo().depth > 1 )
            mParentScene->getRenderDriver()->setSystemState( cgSystemState::ColorTexture3D, true );
    
    } // End if valid texture

    // Success!
    return true;
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
cgInt32 cgProjectorLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
{
    // Override shadow application settings just in case.
    if ( !isShadowSource() )
        bApplyShadows = false;

    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginLighting( pPool, bApplyShadows, bDeferred ) < 0 )
        return -1;

    // Bind the custom projector lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mProjectorLightConstants );

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
cgLightNode::LightingOp cgProjectorLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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

         // Use custom texture projection matrix for projector light.
        cgMatrix mtxTexRemap;
        cgMatrix::identity( mtxTexRemap );
        mtxTexRemap._22 = -1.0f; mtxTexRemap._33 = 1.0f / getOuterRange();

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
            renderDeferred( mLightShapeTransform * getWorldTransform( false ), VolumeStatus, (mLightingApplyShadows) ? mShadowFrustum : CG_NULL,
                            &mtxTexRemap );

        } // End if deferred
        else
        {
            // Begin the forward rendering process
            if ( beginRenderForward( mLightShapeTransform * getWorldTransform( false ), (mLightingApplyShadows) ? mShadowFrustum : CG_NULL, 
                                     &mFrustum, &mtxTexRemap ) )
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
bool cgProjectorLightNode::endLightingPass( )
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
bool cgProjectorLightNode::endLighting( )
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
cgInt32 cgProjectorLightNode::beginIndirectFill( cgTexturePool * pPool )
{
    // Call base class implementation (performs much of the pass tracking logic)
    if ( cgLightNode::beginIndirectFill( pPool ) < 0 )
        return -1;

    // ToDo: 6767 - Can use the specialized method for this?
    // Do nothing if frustum has nothing to draw
    if ( mIndirectFrustum->getVisibilitySet()->isEmpty() )
        return 0;

    // ToDo: Determine if it is an appropriate time for us to update based on
    // our shadow map/rsm update period limiting property.

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

    // Bind the custom projector lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mProjectorLightConstants );

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
//  Name : beginShadowFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::beginIndirectFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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
bool cgProjectorLightNode::endIndirectFillPass( )
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
bool cgProjectorLightNode::endIndirectFill( )
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
cgInt32 cgProjectorLightNode::beginIndirectLighting( cgTexturePool * pPool )
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
    pDriver->setConstantBufferAuto( mProjectorLightConstants );

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
cgLightNode::LightingOp cgProjectorLightNode::beginIndirectLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
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

    } // End if FillReflectiveShadowMap
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
bool cgProjectorLightNode::endIndirectLightingPass( )
{
    cgInt32 nCurrentPass = mCurrentLightingPass;

    // Valid end operation?
    if ( !cgLightNode::endIndirectLightingPass() )
        return false;

    // What are we doing?
    LightingOpPass & Pass = mLightingPasses[nCurrentPass];
    if ( Pass.op == Lighting_FillShadowMap )
    {
        // Complete any shadow map fill for this frustum.
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
bool cgProjectorLightNode::endIndirectLighting( )
{
    // Valid end operation?
    if ( !cgLightNode::endIndirectLighting() )
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
void cgProjectorLightNode::setIndirectLightingMethod( cgUInt32 nMethod )
{
    mIndirectFrustum->setMethod( nMethod );
}

/*//-----------------------------------------------------------------------------
//  Name : SetLightingTerms () 
/// <summary>
/// Populate the supplied base lighting terms structure as well as optionally
/// setting up any custom light parameters as required by the effect lib.
/// </summary>
//-----------------------------------------------------------------------------
bool cgProjectorLightNode::SetLightingTerms( cgBaseLightingTerms & BaseTerms )
{
    cgToDoAssert( "Effect Overhaul", "Implement this." );
    
    // Call base class implementation first to fill out common details.
    if ( cgLightNode::SetLightingTerms( BaseTerms ) == false )
        return false;

    // Set custom effect parameters.
    sgEffectFile* pEffect = m_hEffect;
    if ( pEffect != CG_NULL && pEffect->IsLoaded() == true )
    {
        // Override the texture projection matrix (initially set by base class)
        // used to project the "light" texture onto the scene
        cgMatrix mtx;
        cgMatrix::identity( &mtx );
	    mtx._22 = -1.0f; mtx._33 = 1.0f / getOuterRange();
        pEffect->SetMatrix( _T("LightTexProjMatrix"), m_mtxView * mtx );

        // Determine whether or not we should advance the current frame of the
        // projector light's animated color map (if any).
        cgFloat   fFrameOffset = 0.0f;
        cgTimer * pTimer       = cgTimer::getInstance();
        if ( getPlaybackRate() > 0 )
        {
            cgDouble fElapsedTime = (pTimer->getTime() - mProjectionTimeLast);
            cgDouble fFrameTime   = (1.0 / (cgDouble)getPlaybackRate());
            
            // Has enough time elapsed since the last update?
            if ( fElapsedTime >= fFrameTime )
            {
                mProjectionCurrentFrame = (mProjectionCurrentFrame + (cgUInt32)(fElapsedTime / fFrameTime) ) % getPlaybackFrames();
                mProjectionTimeLast = pTimer->getTime();
            
            } // End if enough time elapsed

            // Compute offset for smooth frame interpolation.
            fFrameOffset = (cgFloat)((pTimer->getTime() - mProjectionTimeLast) / fFrameTime);

        } // End if animation update

        // ToDo: The following code choices are for explicit and smooth frame selection. Any preference? or perhaps user option?
        // Inform the system of the current frame scalar (0 - 1 range).
        //pEffect->SetFloat( _T("ProjectorFrame"), (cgFloat)(mProjectionCurrentFrame + 0.5f) / (cgFloat)getPlaybackFrames() );
        pEffect->SetFloat( _T("ProjectorFrame"), ((cgFloat)mProjectionCurrentFrame + fFrameOffset) / (cgFloat)getPlaybackFrames() );

    } // End if valid effect

    // Success!
    return true;
}*/