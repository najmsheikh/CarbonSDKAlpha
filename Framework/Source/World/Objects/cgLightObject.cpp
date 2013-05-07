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
// Name : cgLightObject.cpp                                                  //
//                                                                           //
// Desc : Our light class implemented as a scene object that can be          //
//        managed and controlled in the same way as any other object.        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLightObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/cgVisibilitySet.h>
#include <World/cgScene.h>
#include <World/Lighting/cgLightingManager.h>
#include <World/Lighting/cgShadowGenerator.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgMesh.h>
#include <Resources/cgScript.h>             // ToDo: Remove if we get rid of 'RenderDriver::getShaderInterface()' calls.
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>            // ToDo: Make sure this is still required.
#include <System/cgStringUtility.h>
#include <System/cgXML.h>                   // ToDo: Remove when no longer necessary
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgLightObject::mInsertBaseLight;
cgWorldQuery cgLightObject::mInsertShadowFrustum;
cgWorldQuery cgLightObject::mInsertShadowFrustumLOD;
cgWorldQuery cgLightObject::mDeleteShadowFrustumLODs;
cgWorldQuery cgLightObject::mUpdateShadowFrustum;
cgWorldQuery cgLightObject::mUpdateDiffuse;
cgWorldQuery cgLightObject::mUpdateSpecular;
cgWorldQuery cgLightObject::mUpdateAmbient;
cgWorldQuery cgLightObject::mUpdateAmbientFar;
cgWorldQuery cgLightObject::mUpdateRim;
cgWorldQuery cgLightObject::mUpdateHDRScalars;
cgWorldQuery cgLightObject::mUpdateProcessStages;
cgWorldQuery cgLightObject::mUpdateSpecularAttenuation;
cgWorldQuery cgLightObject::mUpdateShadowAttenuation;
cgWorldQuery cgLightObject::mUpdateShadowLODRanges;
cgWorldQuery cgLightObject::mLoadBaseLight;
cgWorldQuery cgLightObject::mLoadShadowFrustum;
cgWorldQuery cgLightObject::mLoadShadowFrustumLODs;

///////////////////////////////////////////////////////////////////////////////
// cgLightObject Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLightObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::cgLightObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize variables to sensible defaults
    mDiffuseColor              = cgColorValue(1,1,1,1);                    
	mSpecularColor             = cgColorValue(0,0,0,1);
	mAmbientColor              = cgColorValue(0,0,0,1);
    mAmbientFarColor           = cgColorValue(0,0,0,1);
	mRimColor                  = cgColorValue(0,0,0,1);
    mLightingStage             = cgSceneProcessStage::Runtime;
    mShadowStage               = cgSceneProcessStage::None;
    mIsDiffuseSource            = false;
	mIsSpecularSource           = false;
    mAttenDistSampler         = CG_NULL;
    mAttenMaskSampler         = CG_NULL;
	
    // HDR settings.
    mDiffuseHDRScale          = 1.0f;
    mAmbientHDRScale          = 1.0f;
    mAmbientFarHDRScale       = 1.0f;
    mSpecularHDRScale         = 1.0f;
    mRimHDRScale              = 1.0f;

    // Lighting LOD settings.
    mSpecularMinDistance      = 0.0f;
	mSpecularMaxDistance      = 0.0f;
    
    // Shadow LOD settings.
    mShadowMinDistance        = 0.0f;  
	mShadowMaxDistance        = 0.0f; 
	mShadowLODMinDistance     = 0.0f;
    mShadowLODMaxDistance     = 0.0f;    
}

//-----------------------------------------------------------------------------
//  Name : cgLightObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::cgLightObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgLightObject * pObject = (cgLightObject*)pInit;
    mDiffuseColor              = pObject->mDiffuseColor;
	mSpecularColor             = pObject->mSpecularColor;
	mAmbientColor              = pObject->mAmbientColor;
    mAmbientFarColor           = pObject->mAmbientFarColor;
	mRimColor                  = pObject->mRimColor;
    mLightingStage             = pObject->mLightingStage;
    mShadowStage               = pObject->mShadowStage;
    mIsDiffuseSource            = pObject->mIsDiffuseSource;
	mIsSpecularSource           = pObject->mIsSpecularSource;
	
    // HDR settings.
    mDiffuseHDRScale          = pObject->mDiffuseHDRScale;
    mAmbientHDRScale          = pObject->mAmbientHDRScale;
    mAmbientFarHDRScale       = pObject->mAmbientFarHDRScale;
    mSpecularHDRScale         = pObject->mSpecularHDRScale;
    mRimHDRScale              = pObject->mRimHDRScale;

    // Lighting LOD settings.
    mSpecularMinDistance      = pObject->mSpecularMinDistance;
	mSpecularMaxDistance      = pObject->mSpecularMaxDistance;
    
    // Shadow LOD settings.
    mShadowMinDistance        = pObject->mShadowMinDistance;
	mShadowMaxDistance        = pObject->mShadowMaxDistance; 
	mShadowLODMinDistance     = pObject->mShadowLODMinDistance;
    mShadowLODMaxDistance     = pObject->mShadowLODMaxDistance;

    // Duplicate attenuation data.
    mAttenDistSampler         = CG_NULL;
    mAttenMaskSampler         = CG_NULL;
    mDistanceAttenCurve        = pObject->mDistanceAttenCurve;

    // Duplicate samplers.
    if ( pObject->mAttenDistSampler )
    {
        cgResourceManager * pResources = pWorld->getResourceManager();
        mAttenDistSampler = pResources->cloneSampler( pObject->mAttenDistSampler );
    
    } // End if has distance sampler
    if ( pObject->mAttenMaskSampler )
    {
        cgResourceManager * pResources = pWorld->getResourceManager();
        mAttenMaskSampler = pResources->cloneSampler( pObject->mAttenMaskSampler );
    
    } // End if has mask sampler
}

//-----------------------------------------------------------------------------
//  Name : ~cgLightObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightObject::~cgLightObject()
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
void cgLightObject::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated memory
    if ( mAttenDistSampler )
        mAttenDistSampler->scriptSafeDispose();
    if ( mAttenMaskSampler )
        mAttenMaskSampler->scriptSafeDispose();

    // Release resources
    mShader.close();

    // Clear out unnecessary data to save memory.
    mDistanceAttenCurve.clear();

    // Clear variables
    mAttenDistSampler = CG_NULL;
    mAttenMaskSampler = CG_NULL;

    // Dispose base.
    if ( bDisposeBase )
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
bool cgLightObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_LightObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

/* ToDo: Remove on completion.
//-----------------------------------------------------------------------------
//  Name : Deserialize ()
/// <summary>
/// Initialize the object based on the XML data pulled from the 
/// environment definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::Deserialize( const cgXMLNode & InitData, cgSceneLoader * pLoader )
{
    cgXMLNode xChild;

    // Iterate through all child nodes
    for ( cgUInt32 i = 0; i < InitData.GetChildNodeCount(); ++i )
    {
        xChild = InitData.GetChildNode( i );

        // What type of node is this?
        if ( xChild.IsOfType( _T("Diffuse") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mDiffuseColor ) )
                continue;
            if ( mDiffuseColor.r > 1e-3f || mDiffuseColor.g > 1e-3f || mDiffuseColor.b > 1e-3f )
                mIsDiffuseSource = true;

        } // End if Diffuse
        else if ( xChild.IsOfType( _T("Ambient") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mAmbientColor ) )
                continue;

        } // End if Ambient
        else if ( xChild.IsOfType( _T("Specular") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mSpecularColor ) )
                continue;
            if ( mSpecularColor.r > 1e-3f || mSpecularColor.g > 1e-3f || mSpecularColor.b > 1e-3f )
            {
                mComputeSpecular   = true;
                mIsSpecularSource = true;
            
            } // End if has specular

        } // End if Specular
        else if ( xChild.IsOfType( _T("DiffuseHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mDiffuseHDRScale;

        } // End if DiffuseHDRScale
        else if ( xChild.IsOfType( _T("AmbientHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mAmbientHDRScale;

        } // End if AmbientHDRScale
        else if ( xChild.IsOfType( _T("SpecularHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularHDRScale;

        } // End if SpecularHDRScale
        else if ( xChild.IsOfType( _T("LightingStage") ) )
        {
            if ( cgStringUtility::Compare( xChild.GetText(), _T("Precomputed") ) == 0 )
                mLightingStage = cgSceneProcessStage::Precomputed;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Runtime") ) == 0 )
                mLightingStage = cgSceneProcessStage::Runtime;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Both") ) == 0 )
                mLightingStage = cgSceneProcessStage::Both;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("None") ) == 0 )
                mLightingStage = cgSceneProcessStage::None;

        } // End if LightingStage
        else if ( xChild.IsOfType( _T("CastShadows") ) )
        {
            if ( cgStringUtility::Compare( xChild.GetText(), _T("Precomputed") ) == 0 )
                mShadowStage = cgSceneProcessStage::Precomputed;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Runtime") ) == 0 )
                mShadowStage = cgSceneProcessStage::Runtime;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Both") ) == 0 )
                mShadowStage = cgSceneProcessStage::Both;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("None") ) == 0 )
                mShadowStage = cgSceneProcessStage::None;

            // Default correct shadowing LOD setting.
            if ( mShadowStage == cgSceneProcessStage::Runtime || mShadowStage == cgSceneProcessStage::Both )
                mComputeShadows = true;

        } // End if CastShadows
        else if ( xChild.IsOfType( _T("SpecularMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularMinDistance;

        } // End if SpecularMinDistance
        else if ( xChild.IsOfType( _T("SpecularMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularMaxDistance;

        } // End if SpecularMaxDistance
        else if ( xChild.IsOfType( _T("ShadowMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowMinDistance;

        } // End if ShadowMinDistance
        else if ( xChild.IsOfType( _T("ShadowMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowMaxDistance;

        } // End if ShadowMaxDistance
        else if ( xChild.IsOfType( _T("ShadowLODMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowLODMinDistance;

        } // End if ShadowLODMinDistance
        else if ( xChild.IsOfType( _T("ShadowLODMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowLODMaxDistance;

        } // End if ShadowLODMaxDistance
        else if ( xChild.IsOfType( _T("CosineAdjust") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), m_CosineAdjust ) )
                continue;

        } // End if CosineBias

	} // Next Child Node

    // Call base class last.
    return cgSceneObject::Deserialize( InitData, pLoader );
}*/

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this object is currently renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::isRenderable() const
{
	// It may seem strange that a light source would be classed as
    // 'renderable', but this flag applies to anything which needs to
    // exist in the visibility set for rendering purposes (although
    // the lights will not themselves be physically rendered).
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isDiffuseSource () 
/// <summary>
/// Determine if this light is a source of diffuse light.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::isDiffuseSource( ) const
{
    return mIsDiffuseSource && (mLightingStage == cgSceneProcessStage::Runtime || mLightingStage == cgSceneProcessStage::Both);
}

//-----------------------------------------------------------------------------
//  Name : isSpecularSource () 
/// <summary>
/// Determine if this light is a source of specular light (before LOD).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::isSpecularSource( ) const
{
    return mIsSpecularSource && (mLightingStage == cgSceneProcessStage::Runtime || mLightingStage == cgSceneProcessStage::Both);
}

//-----------------------------------------------------------------------------
//  Name : isShadowSource () 
/// <summary>
/// Determine if this light is a source of shadow data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::isShadowSource( ) const
{
    return ( mShadowStage == cgSceneProcessStage::Runtime || mShadowStage == cgSceneProcessStage::Both );
}

//-----------------------------------------------------------------------------
//  Name : getDistanceAttenSampler()
/// <summary>
/// Get the sampler that defines any applied distance attenuation texture.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLightObject::getDistanceAttenSampler( ) const
{
    return mAttenDistSampler;
}

//-----------------------------------------------------------------------------
//  Name : getAttenMaskSampler()
/// <summary>
/// Get the sampler that defines any applied attenuation mask texture.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLightObject::getAttenMaskSampler( ) const
{
    return mAttenMaskSampler;
}

//-----------------------------------------------------------------------------
//  Name : getDistanceAttenCurve ()
/// <summary>
/// Get the curve that describes the attenuation to apply over distance.
/// </summary>
//-----------------------------------------------------------------------------
const cgBezierSpline2 & cgLightObject::getDistanceAttenCurve( ) const
{
    return mDistanceAttenCurve;
}

//-----------------------------------------------------------------------------
//  Name : getLightingStage() 
/// <summary>
/// Returns the current lighting stage for this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneProcessStage::Base cgLightObject::getLightingStage( ) const
{
    return mLightingStage;
}

//-----------------------------------------------------------------------------
//  Name : getShadowStage() 
/// <summary>
/// Returns the current shadow stage for this light source.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneProcessStage::Base cgLightObject::getShadowStage( ) const
{
    return mShadowStage;
}

//-----------------------------------------------------------------------------
//  Name : getDiffuseHDRScale() 
/// <summary>
/// Returns the HDR scale to apply to the diffuse light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getDiffuseHDRScale( ) const
{
    return mDiffuseHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularHDRScale() 
/// <summary>
/// Returns the HDR scale to apply to the specular light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getSpecularHDRScale( ) const
{
    return mSpecularHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getAmbientHDRScale() 
/// <summary>
/// Returns the HDR scale to apply to the ambient light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getAmbientHDRScale( ) const
{
    return mAmbientHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getAmbientFarHDRScale() 
/// <summary>
/// Returns the HDR scale to apply to the far ambient light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getAmbientFarHDRScale( ) const
{
    return mAmbientFarHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getRimHDRScale() 
/// <summary>
/// Returns the HDR scale to apply to the rim light component.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getRimHDRScale( ) const
{
    return mRimHDRScale;
}

//-----------------------------------------------------------------------------
//  Name : getDiffuseColor() 
/// <summary>
/// Retrieve the light's diffuse color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLightObject::getDiffuseColor( ) const
{
    return mDiffuseColor;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularColor() 
/// <summary>
/// Retrieve the light's specular color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLightObject::getSpecularColor( ) const
{
    return mSpecularColor;
}

//-----------------------------------------------------------------------------
//  Name : getAmbientColor() 
/// <summary>
/// Retrieve the light's ambient color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLightObject::getAmbientColor( ) const
{
    return mAmbientColor;
}

//-----------------------------------------------------------------------------
//  Name : getAmbientFarColor() 
/// <summary>
/// Retrieve the light's far ambient color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLightObject::getAmbientFarColor( ) const
{
    return mAmbientFarColor;
}

//-----------------------------------------------------------------------------
//  Name : getRimColor() 
/// <summary>
/// Retrieve the light's rim color component.
/// </summary>
//-----------------------------------------------------------------------------
const cgColorValue & cgLightObject::getRimColor( ) const
{
    return mRimColor;
}

//-----------------------------------------------------------------------------
//  Name : setLightingStage() 
/// <summary>
/// Set the current lighting stage for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setLightingStage( cgSceneProcessStage::Base Stage )
{
    // No-op?
    if ( Stage == mLightingStage )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProcessStages.bindParameter( 1, (cgInt32)Stage );
        mUpdateProcessStages.bindParameter( 2, (cgInt32)mShadowStage );
        mUpdateProcessStages.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateProcessStages.step( true ) )
        {
            cgString strError;
            mUpdateProcessStages.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update process stages for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mLightingStage = Stage;

    // Notify any listeners of this change.
    static const cgString strContext = _T("LightingStage");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Force shadow stage to recompute its value based on the
    // new lighting stage in case it needs to be updated.
    setShadowStage( mShadowStage );
}

//-----------------------------------------------------------------------------
//  Name : setShadowStage() 
/// <summary>
/// Set the current shadow stage for this light source.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setShadowStage( cgSceneProcessStage::Base Stage )
{
    // Cannot cast pre-computed shadows if the lighting is not
    // being performed at the pre-computed stage.
    if ( (Stage == cgSceneProcessStage::Precomputed) || (Stage == cgSceneProcessStage::Both) )
    {
        if ( (mLightingStage != cgSceneProcessStage::Precomputed) && (mLightingStage != cgSceneProcessStage::Both) )
        {
            // Select next best choice
            if ( mLightingStage != cgSceneProcessStage::None )
                Stage = cgSceneProcessStage::Runtime;
            
        } // End if no pre-computed lighting

    } // End if pre-computed shadow

    // No-op?
    if ( Stage == mShadowStage )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateProcessStages.bindParameter( 1, (cgInt32)mLightingStage );
        mUpdateProcessStages.bindParameter( 2, (cgInt32)Stage );
        mUpdateProcessStages.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateProcessStages.step( true ) )
        {
            cgString strError;
            mUpdateProcessStages.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update process stages for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mShadowStage = Stage;

    // Notify any listeners of this change.
    static const cgString strContext = _T("ShadowStage");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setDiffuseHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the diffuse light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setDiffuseHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mDiffuseHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateHDRScalars.bindParameter( 1, fScale );
        mUpdateHDRScalars.bindParameter( 2, mSpecularHDRScale );
        mUpdateHDRScalars.bindParameter( 3, mAmbientHDRScale );
        mUpdateHDRScalars.bindParameter( 4, mAmbientFarHDRScale );
        mUpdateHDRScalars.bindParameter( 5, mRimHDRScale );
        mUpdateHDRScalars.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( !mUpdateHDRScalars.step( true ) )
        {
            cgString strError;
            mUpdateHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars of light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mDiffuseHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("DiffuseHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the specular light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setSpecularHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mSpecularHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateHDRScalars.bindParameter( 1, mDiffuseHDRScale );
        mUpdateHDRScalars.bindParameter( 2, fScale );
        mUpdateHDRScalars.bindParameter( 3, mAmbientHDRScale );
        mUpdateHDRScalars.bindParameter( 4, mAmbientFarHDRScale );
        mUpdateHDRScalars.bindParameter( 5, mRimHDRScale );
        mUpdateHDRScalars.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( !mUpdateHDRScalars.step( true ) )
        {
            cgString strError;
            mUpdateHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars of light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mSpecularHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setAmbientHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the ambient light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setAmbientHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mAmbientHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateHDRScalars.bindParameter( 1, mDiffuseHDRScale );
        mUpdateHDRScalars.bindParameter( 2, mSpecularHDRScale );
        mUpdateHDRScalars.bindParameter( 3, fScale );
        mUpdateHDRScalars.bindParameter( 4, mAmbientFarHDRScale );
        mUpdateHDRScalars.bindParameter( 5, mRimHDRScale );
        mUpdateHDRScalars.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( !mUpdateHDRScalars.step( true ) )
        {
            cgString strError;
            mUpdateHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mAmbientHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("AmbientHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setAmbientFarHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the far ambient light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setAmbientFarHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mAmbientFarHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateHDRScalars.bindParameter( 1, mDiffuseHDRScale );
        mUpdateHDRScalars.bindParameter( 2, mSpecularHDRScale );
        mUpdateHDRScalars.bindParameter( 3, mAmbientHDRScale );
        mUpdateHDRScalars.bindParameter( 4, fScale );
        mUpdateHDRScalars.bindParameter( 5, mRimHDRScale );
        mUpdateHDRScalars.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( !mUpdateHDRScalars.step( true ) )
        {
            cgString strError;
            mUpdateHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mAmbientFarHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("AmbientFarHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setRimHDRScale() 
/// <summary>
/// Set the HDR scale to apply to the rim light component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setRimHDRScale( cgFloat fScale )
{
    // Is this a no-op?
    if ( mRimHDRScale == fScale )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateHDRScalars.bindParameter( 1, mDiffuseHDRScale );
        mUpdateHDRScalars.bindParameter( 2, mSpecularHDRScale );
        mUpdateHDRScalars.bindParameter( 3, mAmbientHDRScale );
        mUpdateHDRScalars.bindParameter( 4, mAmbientFarHDRScale );
        mUpdateHDRScalars.bindParameter( 5, fScale );
        mUpdateHDRScalars.bindParameter( 6, mReferenceId );
        
        // Execute
        if ( !mUpdateHDRScalars.step( true ) )
        {
            cgString strError;
            mUpdateHDRScalars.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update HDR scalars for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mRimHDRScale = fScale;

    // Notify any listeners of this change.
    static const cgString strContext = _T("RimHDRScale");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setDiffuseColor() 
/// <summary>
/// Set the light's diffuse color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setDiffuseColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mDiffuseColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateDiffuse.bindParameter( 1, Color.r );
        mUpdateDiffuse.bindParameter( 2, Color.g );
        mUpdateDiffuse.bindParameter( 3, Color.b );
        mUpdateDiffuse.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateDiffuse.step( true ) )
        {
            cgString strError;
            mUpdateDiffuse.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update diffuse color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mDiffuseColor = Color;

    // Should we enable diffuse?
    mIsDiffuseSource = ( mDiffuseColor.r > CGE_EPSILON || mDiffuseColor.g > CGE_EPSILON || mDiffuseColor.b > CGE_EPSILON );

    // Notify any listeners of this change.
    static const cgString strContext = _T("DiffuseColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularColor() 
/// <summary>
/// Set the light's specular color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setSpecularColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mSpecularColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSpecular.bindParameter( 1, Color.r );
        mUpdateSpecular.bindParameter( 2, Color.g );
        mUpdateSpecular.bindParameter( 3, Color.b );
        mUpdateSpecular.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateSpecular.step( true ) )
        {
            cgString strError;
            mUpdateSpecular.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update specular color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mSpecularColor = Color;

    // Should we enable specular?
    mIsSpecularSource = ( mSpecularColor.r > CGE_EPSILON || mSpecularColor.g > CGE_EPSILON || mSpecularColor.b > CGE_EPSILON );

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setAmbientColor() 
/// <summary>
/// Set the light's ambient color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setAmbientColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mAmbientColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAmbient.bindParameter( 1, Color.r );
        mUpdateAmbient.bindParameter( 2, Color.g );
        mUpdateAmbient.bindParameter( 3, Color.b );
        mUpdateAmbient.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateAmbient.step( true ) )
        {
            cgString strError;
            mUpdateAmbient.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update ambient color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mAmbientColor = Color;

    // Notify any listeners of this change.
    static const cgString strContext = _T("AmbientColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setAmbientFarColor() 
/// <summary>
/// Set the light's far ambient color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setAmbientFarColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mAmbientFarColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAmbientFar.bindParameter( 1, Color.r );
        mUpdateAmbientFar.bindParameter( 2, Color.g );
        mUpdateAmbientFar.bindParameter( 3, Color.b );
        mUpdateAmbientFar.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateAmbientFar.step( true ) )
        {
            cgString strError;
            mUpdateAmbientFar.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update far ambient color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mAmbientFarColor = Color;

    // Notify any listeners of this change.
    static const cgString strContext = _T("AmbientFarColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setRimColor() 
/// <summary>
/// Set the light's rim color component.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setRimColor( const cgColorValue & Color )
{
    // Is this a no-op?
    if ( mRimColor == Color )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateRim.bindParameter( 1, Color.r );
        mUpdateRim.bindParameter( 2, Color.g );
        mUpdateRim.bindParameter( 3, Color.b );
        mUpdateRim.bindParameter( 4, mReferenceId );
        
        // Execute
        if ( !mUpdateRim.step( true ) )
        {
            cgString strError;
            mUpdateRim.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update rim color for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value.
	mRimColor = Color;

    // Notify any listeners of this change.
    static const cgString strContext = _T("RimColor");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}


//-----------------------------------------------------------------------------
//  Name : setAttenMaskSampler()
/// <summary>
/// Set the sampler that defines any applied attenuation mask texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setAttenMaskSampler( cgSampler * pSampler )
{
    // Is this a no-op?
    if ( mAttenMaskSampler == pSampler )
        return;

    // Determine if there was a prior valid mask sampler set
    // to this light source. If we did not, and we are /about/ to
    // apply a valid one, we must /also/ always have a distance 
    // attenuation texture as required by the shader library.
    bool bPrevSamplerValid = (mAttenMaskSampler && mAttenMaskSampler->isTextureValid());
    bool bNewSamplerValid  = (pSampler && pSampler->isTextureValid());

    // Destroy the previous sampler.
    if ( mAttenMaskSampler )
        mAttenMaskSampler->scriptSafeDispose();

    // If user specified a NULL sampler, don't just blindly apply it.
    // We will simply generate a new empty sampler.
    if ( !pSampler )
    {
        cgResourceManager * pResources = mWorld->getResourceManager();
        pSampler = pResources->createSampler( _T("sLightAttenuation"), mShader );
    
    } // End if NULL sampler

    // Update internal value.
	mAttenMaskSampler = pSampler;

    // Attenuation mask sampler MUST use border mode.
    cgSamplerStateDesc States = pSampler->getStates();
    States.addressU = cgAddressingMode::Border;
    States.addressV = cgAddressingMode::Border;
    States.addressW = cgAddressingMode::Border;
    pSampler->setStates( States );

    // Notify any listeners of this change.
    static const cgString strContext = _T("AttenMaskSampler");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : setDistanceAttenCurve ()
/// <summary>
/// Set the curve that describes the attenuation to apply over distance.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setDistanceAttenCurve( const cgBezierSpline2 & Spline )
{
    // Update internal value.
    mDistanceAttenCurve = Spline;

    // Create a 1D distance attenuation texture if the attenuation curve does
    // not describe standard linear attenuation.
    if ( mDistanceAttenCurve.getDescription() != cgBezierSpline2::LinearDecay )
    {
        cgResourceManager * pResources = mWorld->getResourceManager();
        cgRenderDriver    * pDriver    = mWorld->getRenderDriver();

        // Generate a texture for this curve unless one has already been generated
        cgTextureHandle hTexture;
        cgString strCurveName = _T("Core::Curves::256::") + mDistanceAttenCurve.computeHash( 2 );
        pResources->createTexture( &hTexture, mDistanceAttenCurve, 256, 1, true, 0, strCurveName, cgDebugSource() );
        
        // Bind this texture to the distance attenuation sampler.
        mAttenDistSampler->setTexture( hTexture );

    } // End if not linear decay
    else
    {
        // Unload any existing texture.
        mAttenDistSampler->setTexture( cgTextureHandle::Null );
        
    } // End if linear decay

    // Notify listeners that object data has changed.
    static const cgString strContext = _T("DistanceAttenCurve");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : beginLighting () 
/// <summary>
/// Assuming the light would like to opt in to the lighting process
/// this method is called to allow the light to set-up ready for that process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::beginLighting( )
{
    // Set the light type.
    cgRenderDriver * pDriver = mWorld->getRenderDriver();
    pDriver->setSystemState( cgSystemState::LightType, getLightType() );
    
    // Determine what attenuation textures are valid
    bool bDistValid = (mAttenDistSampler && mAttenDistSampler->isTextureValid());
    bool bMaskValid = (mAttenMaskSampler && mAttenMaskSampler->isTextureValid());
    
    // Apply the samplers
    if ( bDistValid )
        mAttenDistSampler->apply();
    if ( bMaskValid )
        mAttenMaskSampler->apply();

    // Update system state
    pDriver->setSystemState( cgSystemState::SampleAttenuation,         bMaskValid );
    pDriver->setSystemState( cgSystemState::SampleDistanceAttenuation, bDistValid );


    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setShadowMinDistance() 
/// <summary>
/// Set the minimum distance at which the camera can be positioned with
/// respect the light source before shadows will start to become attenuated.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setShadowMinDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mShadowMinDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateShadowAttenuation.bindParameter( 1, value );
        mUpdateShadowAttenuation.bindParameter( 2, mShadowMaxDistance );
        mUpdateShadowAttenuation.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateShadowAttenuation.step( true ) )
        {
            cgString strError;
            mUpdateShadowAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mShadowMinDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("ShadowMinDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure max distance is at least equal to the min distance.
    if ( mShadowMinDistance > mShadowMaxDistance )
        setShadowMaxDistance( mShadowMinDistance );
}

//-----------------------------------------------------------------------------
//  Name : setShadowMaxDistance() 
/// <summary>
/// Set the maximum distance at which the camera can be positioned with
/// respect the light source before shadows will be disabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setShadowMaxDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mShadowMaxDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateShadowAttenuation.bindParameter( 1, mShadowMinDistance );
        mUpdateShadowAttenuation.bindParameter( 2, value );
        mUpdateShadowAttenuation.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateShadowAttenuation.step( true ) )
        {
            cgString strError;
            mUpdateShadowAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mShadowMaxDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("ShadowMaxDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure min distance is equal to or smaller than the max distance.
    if ( mShadowMinDistance > mShadowMaxDistance )
        setShadowMinDistance( mShadowMaxDistance );
}

//-----------------------------------------------------------------------------
//  Name : setShadowLODMinDistance() 
/// <summary>
/// Set the minimum distance at which the camera can be positioned with
/// respect the light source before smaller shadowmaps are selected for LOD
/// purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setShadowLODMinDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mShadowLODMinDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateShadowLODRanges.bindParameter( 1, value );
        mUpdateShadowLODRanges.bindParameter( 2, mShadowLODMaxDistance );
        mUpdateShadowLODRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateShadowLODRanges.step( true ) )
        {
            cgString strError;
            mUpdateShadowLODRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow LOD ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mShadowLODMinDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("ShadowLODMinDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure max distance is at least equal to the min distance.
    if ( mShadowLODMinDistance > mShadowLODMaxDistance )
        setShadowLODMaxDistance( mShadowLODMinDistance );
}

//-----------------------------------------------------------------------------
//  Name : setShadowLODMaxDistance() 
/// <summary>
/// Set the maximum distance at which the camera can be positioned with
/// respect the light source before the smallest shadowmap possible will be
/// selected for LOD purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setShadowLODMaxDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mShadowLODMaxDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateShadowLODRanges.bindParameter( 1, mShadowLODMinDistance );
        mUpdateShadowLODRanges.bindParameter( 2, value );
        mUpdateShadowLODRanges.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateShadowLODRanges.step( true ) )
        {
            cgString strError;
            mUpdateShadowLODRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow LOD ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mShadowLODMaxDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("ShadowLODMaxDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure min distance is equal to or smaller than the max distance.
    if ( mShadowLODMinDistance > mShadowLODMaxDistance )
        setShadowLODMinDistance( mShadowLODMaxDistance );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularMinDistance() 
/// <summary>
/// Set the minimum distance at which the camera can be positioned with
/// respect the light source before specular will start to become attenuated.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setSpecularMinDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mSpecularMinDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSpecularAttenuation.bindParameter( 1, value );
        mUpdateSpecularAttenuation.bindParameter( 2, mSpecularMaxDistance );
        mUpdateSpecularAttenuation.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateSpecularAttenuation.step( true ) )
        {
            cgString strError;
            mUpdateSpecularAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update specular attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mSpecularMinDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularMinDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure max distance is at least equal to the min distance.
    if ( mSpecularMinDistance > mSpecularMaxDistance )
        setSpecularMaxDistance( mSpecularMinDistance );
}

//-----------------------------------------------------------------------------
//  Name : setSpecularMaxDistance() 
/// <summary>
/// Set the maximum distance at which the camera can be positioned with
/// respect the light source before specular will be disabled.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::setSpecularMaxDistance( cgFloat value )
{
    // Is this a no-op?
    if ( mSpecularMaxDistance == value )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateSpecularAttenuation.bindParameter( 1, mSpecularMinDistance );
        mUpdateSpecularAttenuation.bindParameter( 2, value );
        mUpdateSpecularAttenuation.bindParameter( 3, mReferenceId );
        
        // Execute
        if ( !mUpdateSpecularAttenuation.step( true ) )
        {
            cgString strError;
            mUpdateSpecularAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update specular attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update internal value
    mSpecularMaxDistance = value;

    // Notify any listeners of this change.
    static const cgString strContext = _T("SpecularMaxDistance");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Make sure min distance is equal to or smaller than the max distance.
    if ( mSpecularMinDistance > mSpecularMaxDistance )
        setSpecularMinDistance( mSpecularMaxDistance );
}

//-----------------------------------------------------------------------------
//  Name : getShadowMinDistance() 
/// <summary>
/// Retrieve the minimum distance at which the camera can be positioned with
/// respect the light source before shadows will start to become attenuated.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getShadowMinDistance( ) const
{
    return mShadowMinDistance;
}

//-----------------------------------------------------------------------------
//  Name : getShadowMaxDistance() 
/// <summary>
/// Retrieve the maximum distance at which the camera can be positioned with
/// respect the light source before shadows will be disabled.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getShadowMaxDistance( ) const
{
    return mShadowMaxDistance;
}

//-----------------------------------------------------------------------------
//  Name : getShadowLODMinDistance() 
/// <summary>
/// Retrieve the minimum distance at which the camera can be positioned with
/// respect the light source before smaller shadowmaps are selected for LOD
/// purposes.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getShadowLODMinDistance( ) const
{
    return mShadowLODMinDistance;
}

//-----------------------------------------------------------------------------
//  Name : getShadowLODMaxDistance() 
/// <summary>
/// Retrieve the maximum distance at which the camera can be positioned with
/// respect the light source before the smallest shadowmap possible will be
/// selected for LOD purposes.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getShadowLODMaxDistance( ) const
{
    return mShadowLODMaxDistance;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularMinDistance() 
/// <summary>
/// Retrieve the minimum distance at which the camera can be positioned with
/// respect the light source before specular will start to become attenuated.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getSpecularMinDistance( ) const
{
    return mSpecularMinDistance;
}

//-----------------------------------------------------------------------------
//  Name : getSpecularMaxDistance() 
/// <summary>
/// Retrieve the maximum distance at which the camera can be positioned with
/// respect the light source before specular will be disabled.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLightObject::getSpecularMaxDistance( ) const
{
    return mSpecularMaxDistance;
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
        return false;
    
    // Load the core integration shader.
    cgResourceManager * pResources = mWorld->getResourceManager();
    if ( !pResources->createSurfaceShader( &mShader, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load/compile light source shader while creating object 0x%x.\n"), mReferenceId );
        return false;
    
    } // End if failed

    // Create attenuation samplers if they are not already allocated.
    if ( !mAttenDistSampler )
        mAttenDistSampler = pResources->createSampler( _T("LightDistanceAttenuation"), mShader );
    if ( !mAttenMaskSampler )
        mAttenMaskSampler = pResources->createSampler( _T("LightAttenuation"), mShader );

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("LightObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertBaseLight.bindParameter( 1, mReferenceId );
        mInsertBaseLight.bindParameter( 2, mDiffuseColor.r );
        mInsertBaseLight.bindParameter( 3, mDiffuseColor.g );
        mInsertBaseLight.bindParameter( 4, mDiffuseColor.b );
        mInsertBaseLight.bindParameter( 5, mDiffuseHDRScale );
        mInsertBaseLight.bindParameter( 6, mSpecularColor.r );
        mInsertBaseLight.bindParameter( 7, mSpecularColor.g );
        mInsertBaseLight.bindParameter( 8, mSpecularColor.b );
        mInsertBaseLight.bindParameter( 9, mSpecularHDRScale );
        mInsertBaseLight.bindParameter( 10, mAmbientColor.r );
        mInsertBaseLight.bindParameter( 11, mAmbientColor.g );
        mInsertBaseLight.bindParameter( 12, mAmbientColor.b );
        mInsertBaseLight.bindParameter( 13, mAmbientHDRScale );
        mInsertBaseLight.bindParameter( 14, mAmbientFarColor.r );
        mInsertBaseLight.bindParameter( 15, mAmbientFarColor.g );
        mInsertBaseLight.bindParameter( 16, mAmbientFarColor.b );
        mInsertBaseLight.bindParameter( 17, mAmbientFarHDRScale );
        mInsertBaseLight.bindParameter( 18, mRimColor.r );
        mInsertBaseLight.bindParameter( 19, mRimColor.g );
        mInsertBaseLight.bindParameter( 20, mRimColor.b );
        mInsertBaseLight.bindParameter( 21, mRimHDRScale );
        mInsertBaseLight.bindParameter( 22, (cgInt32)mLightingStage );
        mInsertBaseLight.bindParameter( 23, (cgInt32)mShadowStage );
        mInsertBaseLight.bindParameter( 24, mSpecularMinDistance );
        mInsertBaseLight.bindParameter( 25, mSpecularMaxDistance );
        mInsertBaseLight.bindParameter( 26, mShadowMinDistance );
        mInsertBaseLight.bindParameter( 27, mShadowMaxDistance );
        mInsertBaseLight.bindParameter( 28, mShadowLODMinDistance );
        mInsertBaseLight.bindParameter( 29, mShadowLODMaxDistance );
        
        // Execute
        if ( !mInsertBaseLight.step( true ) )
        {
            cgString strError;
            mInsertBaseLight.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert light object '0x%x' base data into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("LightObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("LightObject::insertComponentData") );

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
bool cgLightObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the base light data.
    prepareQueries();
    mLoadBaseLight.bindParameter( 1, e->sourceRefId );
    if ( !mLoadBaseLight.step( ) || !mLoadBaseLight.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadBaseLight.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for light object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve base data for light object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadBaseLight.reset();
        return false;
    
    } // End if failed
    
    // Update our local members
    mLoadBaseLight.getColumn( _T("DiffuseR"), mDiffuseColor.r );
    mLoadBaseLight.getColumn( _T("DiffuseG"), mDiffuseColor.g );
    mLoadBaseLight.getColumn( _T("DiffuseB"), mDiffuseColor.b );
    mLoadBaseLight.getColumn( _T("DiffuseHDRScalar"), mDiffuseHDRScale );
    mLoadBaseLight.getColumn( _T("SpecularR"), mSpecularColor.r );
    mLoadBaseLight.getColumn( _T("SpecularG"), mSpecularColor.g );
    mLoadBaseLight.getColumn( _T("SpecularB"), mSpecularColor.b );
    mLoadBaseLight.getColumn( _T("SpecularHDRScalar"), mSpecularHDRScale );
    mLoadBaseLight.getColumn( _T("AmbientR"), mAmbientColor.r );
    mLoadBaseLight.getColumn( _T("AmbientG"), mAmbientColor.g );
    mLoadBaseLight.getColumn( _T("AmbientB"), mAmbientColor.b );
    mLoadBaseLight.getColumn( _T("AmbientHDRScalar"), mAmbientHDRScale );
    mLoadBaseLight.getColumn( _T("AmbientFarR"), mAmbientFarColor.r );
    mLoadBaseLight.getColumn( _T("AmbientFarG"), mAmbientFarColor.g );
    mLoadBaseLight.getColumn( _T("AmbientFarB"), mAmbientFarColor.b );
    mLoadBaseLight.getColumn( _T("AmbientFarHDRScalar"), mAmbientFarHDRScale );
    mLoadBaseLight.getColumn( _T("RimR"), mRimColor.r );
    mLoadBaseLight.getColumn( _T("RimG"), mRimColor.g );
    mLoadBaseLight.getColumn( _T("RimB"), mRimColor.b );
    mLoadBaseLight.getColumn( _T("RimHDRScalar"), mRimHDRScale );
    mLoadBaseLight.getColumn( _T("LightStage"), (cgInt32&)mLightingStage );
    mLoadBaseLight.getColumn( _T("ShadowCastStage"), (cgInt32&)mShadowStage );
    mLoadBaseLight.getColumn( _T("SpecularAttenuateBegin"), mSpecularMinDistance );
    mLoadBaseLight.getColumn( _T("SpecularAttenuateEnd"), mSpecularMaxDistance );
    mLoadBaseLight.getColumn( _T("ShadowAttenuateBegin"), mShadowMinDistance );
    mLoadBaseLight.getColumn( _T("ShadowAttenuateEnd"), mShadowMaxDistance );
    mLoadBaseLight.getColumn( _T("ShadowLODBegin"), mShadowLODMinDistance );
    mLoadBaseLight.getColumn( _T("ShadowLODEnd"), mShadowLODMaxDistance );
    mLoadBaseLight.reset();

    //mShadowMinDistance = 0;
    //mShadowMaxDistance = 0;
    //mShadowStage = cgSceneProcessStage::None;

    // Load the core integration shader.
    cgResourceManager * pResources = mWorld->getResourceManager();
    if ( !pResources->createSurfaceShader( &mShader, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load/compile light source shader while creating object 0x%x.\n"), mReferenceId );
        return false;
    
    } // End if failed

    // Create attenuation samplers if they are not already allocated.
    if ( !mAttenDistSampler )
        mAttenDistSampler = pResources->createSampler( _T("LightDistanceAttenuation"), mShader );
    if ( !mAttenMaskSampler )
        mAttenMaskSampler = pResources->createSampler( _T("LightAttenuation"), mShader );

    // Should we enable diffuse / specular?
    mIsDiffuseSource  = ( mDiffuseColor.r > CGE_EPSILON || mDiffuseColor.g > CGE_EPSILON || mDiffuseColor.b > CGE_EPSILON );
    mIsSpecularSource = ( mSpecularColor.r > CGE_EPSILON || mSpecularColor.g > CGE_EPSILON || mSpecularColor.b > CGE_EPSILON );

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
bool cgLightObject::createTypeTables( const cgUID & TypeIdentifier )
{
    // Ensure this base class table is created first.
    if ( !cgWorldObject::createTypeTables( RTID_LightObject ) )
        return false;

    // Then allow derived type to create as normal.
    return cgWorldObject::createTypeTables( TypeIdentifier );
}

/*//-----------------------------------------------------------------------------
//  Name : InsertShadowFrustum() 
/// <summary>
/// Insert the specified shadow frustum configuration into the database. 
/// Returns the new identifier of the inserted top level frustum entry.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgLightObject::InsertShadowFrustum( const cgShadowGeneratorConfig & Config )
{
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Wrap in transaction so that we can roll back on failure.
        mWorld->beginTransaction( _T("insertShadowFrustum") );

        // Bind parameters
        mInsertShadowFrustum.bindParameter( 1, Config.IsEnabled() );
        mInsertShadowFrustum.bindParameter( 2, Config.GetMinResolutionScale() );
        mInsertShadowFrustum.bindParameter( 3, Config.GetMaxResolutionScale() );
        mInsertShadowFrustum.bindParameter( 4, Config.GetSize() );
        
        // Execute
        if ( !mInsertShadowFrustum.step( true ) )
        {
            cgString strError;
            mInsertShadowFrustum.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new shadow frustum configuration entry for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("insertShadowFrustum") );
            return 0;
        
        } // End if failed

        // Insert succeeded, retrieve the new frustum identifier. This will be used 
        // to link any inserted LOD data (below) to this frustum.
        cgUInt32 nFrustumId = mInsertShadowFrustum.getLastInsertId();

        // Insert the associated frustum LOD data.
        cgShadowGeneratorConfig::LODTable::const_iterator itLOD;
        const cgShadowGeneratorConfig::LODTable & LODs = Config.GetLevelOfDetailTable();
        for ( itLOD = LODs.begin(); itLOD != LODs.end(); ++itLOD )
        {
            const cgShadowGeneratorConfig::LODSettings & LOD = itLOD->second;

            // Setup insert parameters
            mInsertShadowFrustumLOD.bindParameter( 1, nFrustumId );
            mInsertShadowFrustumLOD.bindParameter( 2, LOD.nResolution );
            mInsertShadowFrustumLOD.bindParameter( 3, LOD.strShadowMapType );
            mInsertShadowFrustumLOD.bindParameter( 4, (cgUInt32)LOD.cullMode );
            mInsertShadowFrustumLOD.bindParameter( 5, LOD.bFilterMaps );
            mInsertShadowFrustumLOD.bindParameter( 6, LOD.fZBias );
            mInsertShadowFrustumLOD.bindParameter( 7, LOD.fSlopeScale );
            mInsertShadowFrustumLOD.bindParameter( 8, LOD.nFilterPasses );
            mInsertShadowFrustumLOD.bindParameter( 9, LOD.nFilterKernelSize );
            mInsertShadowFrustumLOD.bindParameter( 10, LOD.fMinVariance );
            mInsertShadowFrustumLOD.bindParameter( 11, LOD.fBleedReduction );

            // Execute
            if ( !mInsertShadowFrustumLOD.step( true ) )
            {
                cgString strError;
                mInsertShadowFrustumLOD.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert new shadow frustum configuration LOD entry for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                mWorld->rollbackTransaction( _T("insertShadowFrustum") );
                return 0;
            
            } // End if failed

        } // Next LOD

        // All done. Commit!
        mWorld->commitTransaction( _T("insertShadowFrustum") );
        return nFrustumId;
        
    } // End if serialize

    // Not serializing
    return 0;
}*/

/*//-----------------------------------------------------------------------------
//  Name : UpdateShadowFrustum() 
/// <summary>
/// Update the specified shadow frustum configuration database entry with the
/// newly supplied settings.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::UpdateShadowFrustum( cgUInt32 nFrustumId, const cgShadowGeneratorConfig & Config )
{
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Wrap in transaction so that we can roll back on failure.
        mWorld->beginTransaction( _T("updateShadowFrustum") );

        // Bind parameters
        mUpdateShadowFrustum.bindParameter( 1, Config.IsEnabled() );
        mUpdateShadowFrustum.bindParameter( 2, Config.GetMinResolutionScale() );
        mUpdateShadowFrustum.bindParameter( 3, Config.GetMaxResolutionScale() );
        mUpdateShadowFrustum.bindParameter( 4, Config.GetSize() );
        mUpdateShadowFrustum.bindParameter( 5, nFrustumId );
        
        // Execute
        if ( !mUpdateShadowFrustum.step( true ) )
        {
            cgString strError;
            mUpdateShadowFrustum.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow frustum configuration entry '%i' for light '0x%x'. Error: %s\n"), nFrustumId, mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("updateShadowFrustum") );
            return false;
        
        } // End if failed

        // Update succeeded. Now we need to update LOD data. First delete the existing
        // LOD data if any.
        mDeleteShadowFrustumLODs.bindParameter( 1, nFrustumId );

        // Execute
        if ( !mDeleteShadowFrustumLODs.step( true ) )
        {
            cgString strError;
            mDeleteShadowFrustumLODs.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to remove prior shadow frustum configuration LOD entries for frustum '%i', light '0x%x'. Error: %s\n"), nFrustumId, mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("updateShadowFrustum") );
            return false;
        
        } // End if failed
        
        // Insert the new frustum LOD data.
        cgShadowGeneratorConfig::LODTable::const_iterator itLOD;
        const cgShadowGeneratorConfig::LODTable & LODs = Config.GetLevelOfDetailTable();
        for ( itLOD = LODs.begin(); itLOD != LODs.end(); ++itLOD )
        {
            const cgShadowGeneratorConfig::LODSettings & LOD = itLOD->second;

            // Setup insert parameters
            mInsertShadowFrustumLOD.bindParameter( 1, nFrustumId );
            mInsertShadowFrustumLOD.bindParameter( 2, LOD.nResolution );
            mInsertShadowFrustumLOD.bindParameter( 3, LOD.strShadowMapType );
            mInsertShadowFrustumLOD.bindParameter( 4, (cgUInt32)LOD.cullMode );
            mInsertShadowFrustumLOD.bindParameter( 5, LOD.bFilterMaps );
            mInsertShadowFrustumLOD.bindParameter( 6, LOD.fZBias );
            mInsertShadowFrustumLOD.bindParameter( 7, LOD.fSlopeScale );
            mInsertShadowFrustumLOD.bindParameter( 8, LOD.nFilterPasses );
            mInsertShadowFrustumLOD.bindParameter( 9, LOD.nFilterKernelSize );
            mInsertShadowFrustumLOD.bindParameter( 10, LOD.fMinVariance );
            mInsertShadowFrustumLOD.bindParameter( 11, LOD.fBleedReduction );

            // Execute
            if ( !mInsertShadowFrustumLOD.step( true ) )
            {
                cgString strError;
                mInsertShadowFrustumLOD.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert shadow frustum configuration LOD entry for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
                mWorld->rollbackTransaction( _T("updateShadowFrustum") );
                return false;
            
            } // End if failed

        } // Next LOD

        // All done. Commit!
        mWorld->commitTransaction( _T("updateShadowFrustum") );
        return true;
        
    } // End if serialize

    // Not serializing (success)
    return true;
}*/

/*//-----------------------------------------------------------------------------
//  Name : LoadShadowFrustum() 
/// <summary>
/// Load the specified shadow frustum configuration.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightObject::LoadShadowFrustum( cgUInt32 nFrustumId, cgShadowGeneratorConfig & Config )
{
    prepareQueries();

    // Select the shadow frustum config entry.
    mLoadShadowFrustum.bindParameter( 1, nFrustumId );
    if ( !mLoadShadowFrustum.step() || !mLoadShadowFrustum.nextRow() )
    {
        cgString strError;
        mLoadShadowFrustum.getLastError( strError );
        cgAppLog::write( cgAppLog::Error, _T("Failed to load shadow frustum configuration entry '%i' for light '0x%x'. Error: %s\n"), nFrustumId, mReferenceId, strError.c_str() );
        mLoadShadowFrustum.reset();
        return false;
    
    } // End if failed

    // Get the data.
    bool bEnabled;
    if ( mLoadShadowFrustum.getColumn( _T("Enabled"), bEnabled ) )
        Config.SetEnabled( bEnabled );
    cgFloat fValue;
    if ( mLoadShadowFrustum.getColumn( _T("MinResolutionScale"), fValue ) )
        Config.SetMinResolutionScale( fValue );
    if ( mLoadShadowFrustum.getColumn( _T("MaxResolutionScale"), fValue ) )
        Config.SetMaxResolutionScale( fValue );
    if ( mLoadShadowFrustum.getColumn( _T("Size"), fValue ) )
        Config.SetSize( fValue );

    // All done with the frustum data.
    mLoadShadowFrustum.reset();

    // Now load the associated LOD data.
    mLoadShadowFrustumLODs.bindParameter( 1, nFrustumId );
    if ( !mLoadShadowFrustumLODs.step() )
    {
        cgString strError;
        mLoadShadowFrustumLODs.getLastError( strError );
        cgAppLog::write( cgAppLog::Error, _T("Failed to load shadow frustum configuration LOD entries for frustum '%i', light '0x%x'. Error: %s\n"), nFrustumId, mReferenceId, strError.c_str() );
        mLoadShadowFrustumLODs.reset();
        return false;
    
    } // End if failed
    for ( ; mLoadShadowFrustumLODs.nextRow(); )
    {
        cgShadowGeneratorConfig::LODSettings LOD;
        mLoadShadowFrustumLODs.getColumn( _T("Resolution"), LOD.nResolution );
        mLoadShadowFrustumLODs.getColumn( _T("ShadowMapType"), LOD.strShadowMapType );
        mLoadShadowFrustumLODs.getColumn( _T("CullMode"), (cgUInt32&)LOD.cullMode );
        mLoadShadowFrustumLODs.getColumn( _T("FilterMaps"), LOD.bFilterMaps );
        mLoadShadowFrustumLODs.getColumn( _T("ZBias"), LOD.fZBias );
        mLoadShadowFrustumLODs.getColumn( _T("SlopeScaleBias"), LOD.fSlopeScale );
        mLoadShadowFrustumLODs.getColumn( _T("FilterPasses"), LOD.nFilterPasses );
        mLoadShadowFrustumLODs.getColumn( _T("FilterKernelSize"), LOD.nFilterKernelSize );
        mLoadShadowFrustumLODs.getColumn( _T("MinVariance"), LOD.fMinVariance );
        mLoadShadowFrustumLODs.getColumn( _T("BleedReduction"), LOD.fBleedReduction );
        Config.SetLevelOfDetail( LOD.nResolution, LOD );

    } // Next LOD entry
    mLoadShadowFrustumLODs.reset();

    // All done.
    return true;
}*/

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertBaseLight.isPrepared() )
            mInsertBaseLight.prepare( mWorld, _T("INSERT INTO 'Objects::Base::Light' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23,?24,?25,?26,?27,?28,?29)"), true );
        if ( !mInsertShadowFrustum.isPrepared() )
            mInsertShadowFrustum.prepare( mWorld, _T("INSERT INTO 'ShadowFrustums' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( !mInsertShadowFrustumLOD.isPrepared() )
            mInsertShadowFrustumLOD.prepare( mWorld, _T("INSERT INTO 'ShadowFrustums::LOD' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11)"), true );
        if ( !mDeleteShadowFrustumLODs.isPrepared() )
            mDeleteShadowFrustumLODs.prepare( mWorld, _T("DELETE FROM 'ShadowFrustums::LOD' WHERE FrustumId=?1"), true );
        if ( !mUpdateShadowFrustum.isPrepared() )
            mUpdateShadowFrustum.prepare( mWorld, _T("UPDATE 'ShadowFrustums' SET Enabled=?1, MinResolutionScale=?2, MaxResolutionScale=?3, Size=?4 WHERE FrustumId=?5"), true );
        if ( !mUpdateDiffuse.isPrepared() )
            mUpdateDiffuse.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET DiffuseR=?1, DiffuseG=?2, DiffuseB=?3 WHERE RefId=?4"), true );
        if ( !mUpdateSpecular.isPrepared() )
            mUpdateSpecular.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET SpecularR=?1, SpecularG=?2, SpecularB=?3 WHERE RefId=?4"), true );
        if ( !mUpdateAmbient.isPrepared() )
            mUpdateAmbient.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET AmbientR=?1, AmbientG=?2, AmbientB=?3 WHERE RefId=?4"), true );
        if ( !mUpdateAmbientFar.isPrepared() )
            mUpdateAmbientFar.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET AmbientFarR=?1, AmbientFarG=?2, AmbientFarB=?3 WHERE RefId=?4"), true );
        if ( !mUpdateRim.isPrepared() )
            mUpdateRim.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET RimR=?1, RimG=?2, RimB=?3 WHERE RefId=?4"), true );
        if ( !mUpdateHDRScalars.isPrepared() )
            mUpdateHDRScalars.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET DiffuseHDRScalar=?1, SpecularHDRScalar=?2, AmbientHDRScalar=?3, AmbientFarHDRScalar=?4, RimHDRScalar=?5 WHERE RefId=?6"), true );
        if ( !mUpdateProcessStages.isPrepared() )
            mUpdateProcessStages.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET LightStage=?1, ShadowCastStage=?2 WHERE RefId=?3"), true );
        if ( !mUpdateSpecularAttenuation.isPrepared() )
            mUpdateSpecularAttenuation.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET SpecularAttenuateBegin=?1, SpecularAttenuateEnd=?2 WHERE RefId=?3"), true );
        if ( !mUpdateShadowAttenuation.isPrepared() )
            mUpdateShadowAttenuation.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET ShadowAttenuateBegin=?1, ShadowAttenuateEnd=?2 WHERE RefId=?3"), true );
        if ( !mUpdateShadowLODRanges.isPrepared() )
            mUpdateShadowLODRanges.prepare( mWorld, _T("UPDATE 'Objects::Base::Light' SET ShadowLODBegin=?1, ShadowLODEnd=?2 WHERE RefId=?3"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadBaseLight.isPrepared() )
        mLoadBaseLight.prepare( mWorld, _T("SELECT * FROM 'Objects::Base::Light' WHERE RefId=?1"), true );
    if ( !mLoadShadowFrustum.isPrepared() )
        mLoadShadowFrustum.prepare( mWorld, _T("SELECT * FROM 'ShadowFrustums' WHERE FrustumId=?1"), true );
    if ( !mLoadShadowFrustumLODs.isPrepared() )
        mLoadShadowFrustumLODs.prepare( mWorld, _T("SELECT * FROM 'ShadowFrustums::LOD' WHERE FrustumId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightObject::sandboxRender( cgUInt32 flags, cgCameraNode * pCamera, cgVisibilitySet * pVisData, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // No post-clear operation.
    if ( flags & cgSandboxRenderFlags::PostDepthClear )
        return;

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
void cgLightObject::applyObjectRescale( cgFloat fScale )
{
    cgToDoAssert( "Carbon General", "cgLightObject::applyObjectRescale()" );

    // Compute new distances
    cgFloat fNewShadowLODMinDistance = mShadowLODMinDistance * fScale;
    cgFloat fNewShadowLODMaxDistance = mShadowLODMaxDistance * fScale;
    cgFloat fNewShadowMinDistance    = mShadowMinDistance * fScale;
    cgFloat fNewShadowMaxDistance    = mShadowMaxDistance * fScale;
    cgFloat fNewSpecularMinDistance  = mSpecularMinDistance * fScale;
    cgFloat fNewSpecularMaxDistance  = mSpecularMaxDistance * fScale;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Shadow LOD
        mUpdateShadowLODRanges.bindParameter( 1, fNewShadowLODMinDistance );
        mUpdateShadowLODRanges.bindParameter( 2, fNewShadowLODMaxDistance );
        mUpdateShadowLODRanges.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateShadowLODRanges.step( true ) )
        {
            cgString strError;
            mUpdateShadowLODRanges.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow LOD ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

        // Shadow attenuation
        mUpdateShadowAttenuation.bindParameter( 1, fNewShadowMinDistance );
        mUpdateShadowAttenuation.bindParameter( 2, fNewShadowMaxDistance );
        mUpdateShadowAttenuation.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateShadowAttenuation.step( true ) )
        {
            cgString strError;
            mUpdateShadowAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update shadow attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

        // Specular attenuation
        mUpdateSpecularAttenuation.bindParameter( 1, fNewSpecularMinDistance );
        mUpdateSpecularAttenuation.bindParameter( 2, fNewSpecularMaxDistance );
        mUpdateSpecularAttenuation.bindParameter( 3, mReferenceId );

        // Execute
        if ( !mUpdateShadowLODRanges.step( true ) )
        {
            cgString strError;
            mUpdateSpecularAttenuation.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update specular attenuation ranges for light '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if serialize

    // Update local values.
    mShadowLODMinDistance = fNewShadowLODMinDistance;
    mShadowLODMaxDistance = fNewShadowLODMaxDistance;
    mShadowMinDistance    = fNewShadowMinDistance;
    mShadowMaxDistance    = fNewShadowMaxDistance;
    mSpecularMinDistance  = fNewSpecularMinDistance;
    mSpecularMaxDistance  = fNewSpecularMaxDistance;
    
    // ToDo: derived class called -- avoid calling twice?
    // Notify listeners that property was altered
    //static const cgString strContext = _T("ApplyRescale");
    //onComponentModified( &cgComponentModifiedEventArgs( strContext ) );

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

///////////////////////////////////////////////////////////////////////////////
// cgLightNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::cgLightNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults
	mSetClipPlanes            = false;
	mUseLightClipPlanes       = false;
	mUseLightScreenRect       = false;
    mComputeSpecular             = false;
    mSpecularAttenuation      = 1.0f;
    mComputeShadows              = false;
    mComputeIndirect             = false;
    mShadowAttenuation        = 1.0f;
    mShadowLODScale           = 1.0f;
    mCurrentShadowPass        = -2;
    mCurrentLightingPass      = -2;
    mLightingPassCount        = 0;
    mShadowPassCount          = 0;
    mShadowTimedUpdate        = false;
    mLightingDeferred         = false;
    mLightingApplyShadows     = false;
    mTexturePool              = CG_NULL;
    mIndirectMethod           = 0;
    mIndirectResolution       = 0;
    mIndirectFlags            = 0;
    mIndirectMethod           = 0;
    mIndirectResolution       = 0;
    
    // Clear structures
    memset( &mScissorRectangle, 0, sizeof(RECT) );
    cgMatrix::identity( mProjectionMatrix );
    cgMatrix::identity( mViewMatrix );

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Light%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgLightNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::cgLightNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    // Initialize variables to sensible defaults
    cgLightNode * pNode = (cgLightNode*)pInit;
 	mSetClipPlanes          = false;
 	mUseLightClipPlanes     = pNode->mUseLightClipPlanes;
	mUseLightScreenRect     = pNode->mUseLightScreenRect;
    mComputeSpecular        = pNode->mComputeSpecular;
    mSpecularAttenuation    = pNode->mSpecularAttenuation;
    mComputeShadows         = pNode->mComputeShadows;
    mComputeIndirect        = pNode->mComputeIndirect;
    mShadowAttenuation      = pNode->mShadowAttenuation;
    mShadowLODScale         = pNode->mShadowLODScale;
    mCurrentShadowPass      = -2;
    mCurrentLightingPass    = -2;
    mLightingPassCount      = 0;
    mShadowPassCount        = 0;
    mShadowTimedUpdate      = false;
    mLightingDeferred       = false;
    mLightingApplyShadows   = false;
    mTexturePool            = CG_NULL;
    mIndirectMethod         = 0;
    mIndirectResolution     = 0;
    mIndirectDetailLevels   = pNode->mIndirectDetailLevels;
    mIndirectSettings       = pNode->mIndirectSettings;
    mIndirectSystemSettings = pNode->mIndirectSystemSettings;
    mIndirectFlags          = pNode->mIndirectFlags;
    mIndirectMethod         = pNode->mIndirectMethod;
    mIndirectResolution     = pNode->mIndirectResolution;
    
    // Clear structures
    memset( &mScissorRectangle, 0, sizeof(RECT) );
    mProjectionMatrix       = pNode->mProjectionMatrix;
    mLightShapeTransform    = pNode->mLightShapeTransform;
    cgMatrix::identity( mViewMatrix );
}

//-----------------------------------------------------------------------------
//  Name : ~cgLightNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::~cgLightNode()
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
void cgLightNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear variables
    mCurrentShadowPass        = -2;
    mCurrentLightingPass      = -2;
    mLightingPassCount        = 0;
    mShadowPassCount          = 0;
    mShadowTimedUpdate        = false;
    mLightingDeferred         = false;
    mLightingApplyShadows     = false;
    mTexturePool              = CG_NULL;

    // Release resources
    mInLightStates.Close();
    mOutStencilFillStates.Close();
    mOutLightStates.Close();
    mOutStencilClearStates.Close();
    mSpanStencilFillBStates.Close();
    mSpanStencilFillFStates.Close();
    mSpanLightStates.Close();
    mSpanStencilClearStates.Close();
    mLightConstants.close();
    mShader.close();
    mLightShape.close();
    
    // Dispose base.
    if ( bDisposeBase )
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
bool cgLightNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_LightNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

/* ToDo: Remove on completion.
//-----------------------------------------------------------------------------
//  Name : Deserialize ()
/// <summary>
/// Initialize the object based on the XML data pulled from the 
/// environment definition file.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::Deserialize( const cgXMLNode & InitData, cgSceneLoader * pLoader )
{
    cgXMLNode xChild;

    // Iterate through all child nodes
    for ( cgUInt32 i = 0; i < InitData.GetChildNodeCount(); ++i )
    {
        xChild = InitData.GetChildNode( i );

        // What type of node is this?
        if ( xChild.IsOfType( _T("Diffuse") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mDiffuseColor ) )
                continue;
            if ( mDiffuseColor.r > 1e-3f || mDiffuseColor.g > 1e-3f || mDiffuseColor.b > 1e-3f )
                mIsDiffuseSource = true;

        } // End if Diffuse
        else if ( xChild.IsOfType( _T("Ambient") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mAmbientColor ) )
                continue;

        } // End if Ambient
        else if ( xChild.IsOfType( _T("Specular") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), mSpecularColor ) )
                continue;
            if ( mSpecularColor.r > 1e-3f || mSpecularColor.g > 1e-3f || mSpecularColor.b > 1e-3f )
            {
                mComputeSpecular   = true;
                mIsSpecularSource = true;
            
            } // End if has specular

        } // End if Specular
        else if ( xChild.IsOfType( _T("DiffuseHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mDiffuseHDRScale;

        } // End if DiffuseHDRScale
        else if ( xChild.IsOfType( _T("AmbientHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mAmbientHDRScale;

        } // End if AmbientHDRScale
        else if ( xChild.IsOfType( _T("SpecularHDRScale") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularHDRScale;

        } // End if SpecularHDRScale
        else if ( xChild.IsOfType( _T("LightingStage") ) )
        {
            if ( cgStringUtility::Compare( xChild.GetText(), _T("Precomputed") ) == 0 )
                mLightingStage = cgSceneProcessStage::Precomputed;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Runtime") ) == 0 )
                mLightingStage = cgSceneProcessStage::Runtime;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Both") ) == 0 )
                mLightingStage = cgSceneProcessStage::Both;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("None") ) == 0 )
                mLightingStage = cgSceneProcessStage::None;

        } // End if LightingStage
        else if ( xChild.IsOfType( _T("CastShadows") ) )
        {
            if ( cgStringUtility::Compare( xChild.GetText(), _T("Precomputed") ) == 0 )
                mShadowStage = cgSceneProcessStage::Precomputed;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Runtime") ) == 0 )
                mShadowStage = cgSceneProcessStage::Runtime;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("Both") ) == 0 )
                mShadowStage = cgSceneProcessStage::Both;
            else if ( cgStringUtility::Compare( xChild.GetText(), _T("None") ) == 0 )
                mShadowStage = cgSceneProcessStage::None;

            // Default correct shadowing LOD setting.
            if ( mShadowStage == cgSceneProcessStage::Runtime || mShadowStage == cgSceneProcessStage::Both )
                mComputeShadows = true;

        } // End if CastShadows
        else if ( xChild.IsOfType( _T("SpecularMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularMinDistance;

        } // End if SpecularMinDistance
        else if ( xChild.IsOfType( _T("SpecularMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mSpecularMaxDistance;

        } // End if SpecularMaxDistance
        else if ( xChild.IsOfType( _T("ShadowMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowMinDistance;

        } // End if ShadowMinDistance
        else if ( xChild.IsOfType( _T("ShadowMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowMaxDistance;

        } // End if ShadowMaxDistance
        else if ( xChild.IsOfType( _T("ShadowLODMinDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowLODMinDistance;

        } // End if ShadowLODMinDistance
        else if ( xChild.IsOfType( _T("ShadowLODMaxDistance") ) )
        {
            cgStringParser( xChild.GetText() ) >> mShadowLODMaxDistance;

        } // End if ShadowLODMaxDistance
        else if ( xChild.IsOfType( _T("CosineAdjust") ) )
        {
            if ( !cgStringUtility::TryParse( xChild.GetText(), m_CosineAdjust ) )
                continue;

        } // End if CosineBias

	} // Next Child Node

    // Call base class last.
    return cgSceneObject::Deserialize( InitData, pLoader );
}*/

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::onNodeCreated( const cgUID & ObjectType, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation.
    if ( !cgObjectNode::onNodeCreated( ObjectType, CloneMethod ) )
        return false;

    // Run post creation process.
    return postCreate();
}

//-----------------------------------------------------------------------------
// Name : onNodeLoading ( ) (Virtual)
/// <summary>
/// Virtual method called when the node is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::onNodeLoading( const cgUID & ObjectType, cgWorldQuery * pNodeData, cgSceneCell * pParentCell, cgCloneMethod::Base CloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( ObjectType, pNodeData, pParentCell, CloneMethod ) )
        return false;

    // Run post creation process.
    return postCreate();
}

//-----------------------------------------------------------------------------
// Name : postCreate ( ) (Protected Virtual)
/// <summary>
/// Performs post creation tasks that are required after both first time
/// creation and loading step.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::postCreate( )
{
    // Load the core lighting integration shader.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    if ( !pResources->createSurfaceShader( &mShader, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load/compile light source shader for object node 0x%x in scene '%s'.\n"), mReferenceId, mParentScene->getName().c_str() );
        return false;
    
    } // End if failed

    // Create constant buffers
    if ( !pResources->createConstantBuffer( &mLightConstants, mShader, _T("_cbLight"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required light source constant buffers for object node 0x%x in scene '%s'.\n"), mReferenceId, mParentScene->getName().c_str() );
        return false;
    
    } // End if failed
    cgAssert( mLightConstants->getDesc().length == sizeof(_cbLight ) );
    
    cgToDo( "Carbon General", "It's possible that the view matrix should be cell relative?" );

    // Set view matrix values
    const cgVector3 & vecRight    = getXAxis( false ), & vecUp = getYAxis( false ), & vecLook = getZAxis( false );
    const cgVector3 & vecPosition = getPosition( false );
    mViewMatrix._11 = vecRight.x; mViewMatrix._12 = vecUp.x; mViewMatrix._13 = vecLook.x;
    mViewMatrix._21 = vecRight.y; mViewMatrix._22 = vecUp.y; mViewMatrix._23 = vecLook.y;
    mViewMatrix._31 = vecRight.z; mViewMatrix._32 = vecUp.z; mViewMatrix._33 = vecLook.z;
    mViewMatrix._41 = -cgVector3::dot( vecPosition, vecRight );
    mViewMatrix._42 = -cgVector3::dot( vecPosition, vecUp    );
    mViewMatrix._43 = -cgVector3::dot( vecPosition, vecLook  );

    // Build vertex/pixel shader permutation arguments to save generating
    // them dynamically each time we process a light source / frustum.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    cgScriptObject * pSystemExports = pDriver->getShaderInterface();
    
    // Vertex shader permutations for stencil passes.
    static const bool bStencilPass   = true;
    mStencilVSArgs.clear();
    mStencilVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), pSystemExports->getAddressOfMember( _T("lightType") ) ) );
    mStencilVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bStencilPass ) );

    // Vertex shader permutations for lighting passes.
    static const bool bNoStencilPass = false;
    mLightingVSArgs.clear();
    mLightingVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), pSystemExports->getAddressOfMember( _T("lightType") ) ) );
    mLightingVSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bNoStencilPass ) );

    // Pixel shader permutations for lighting passes.
    static const bool bUseAttenuationBuffer = false;
    mLightingPSArgs.clear();
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("lightType") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("lightFlags") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("shadowMethod") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("primaryTaps") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("secondaryTaps") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("renderFlags") ) ) );
    mLightingPSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("shadingQuality") ) ) );
    
    // We're done with the shader interface
    pSystemExports->release();

    // Load shadow settings
    loadShadowSettings();
    
    // Build state blocks necessary for lighting process.
    return buildStateBlocks();
}

// ToDo: 6767 - TEMPORARY!
//-----------------------------------------------------------------------------
//  Name : loadShadowSettings () (Protected Virtual)
/// <summary>
/// Temporary substitute for shadow settings loading from database
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::loadShadowSettings( )
{
    cgUInt32 i;
    cgString strFileName = cgFileSystem::resolveFileLocation( _T("sys://Config/ShadowConfig.ini") );

    // How many settings entries are in the database?
    cgUInt32 nNumEntries = GetPrivateProfileInt( _T("Lights"), _T("NumEntries"), 0, strFileName.c_str() );

    // Load in the entries
    for ( i = 0; i < nNumEntries; i++ )
    {
        cgString strEntry = cgString::format( _T("LightEntry%i"), i );

        cgTChar lpszEntryName[128];
        GetPrivateProfileString( _T("LightEntries"), strEntry.c_str(), _T(""), lpszEntryName, 128, strFileName.c_str() );

        cgTChar lpszTechniqueName[128];
        GetPrivateProfileString( lpszEntryName, _T("Technique"), _T(""), lpszTechniqueName, 128, strFileName.c_str() );

        cgShadowSettingsLOD Entry;
        Entry.name = lpszTechniqueName;
        Entry.level  = GetPrivateProfileInt( lpszEntryName, _T("LOD"), 0, strFileName.c_str() );
        mShadowDetailLevels.push_back( Entry );

        cgShadowSettingsLight Settings;
        Settings.method                = GetPrivateProfileInt( lpszEntryName, _T("Method"), 0, strFileName.c_str() );
        Settings.resolutionAdjust      = GetPrivateProfileInt( lpszEntryName, _T("Resolution"), 0, strFileName.c_str() );
        Settings.cullMode              = (cgCullMode::Base)GetPrivateProfileInt( lpszEntryName, _T("CullMode"), 0, strFileName.c_str() );
        Settings.filterBlurFactor      = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterBlurFactor"), 0, strFileName.c_str() );
        Settings.filterDistanceNear    = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterDistanceNear"), 0, strFileName.c_str() );
        Settings.filterDistanceFar     = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterDistanceFar"), 0, strFileName.c_str() );
        Settings.depthBiasSW           = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("ZBiasSW"), 0, strFileName.c_str() );
        Settings.depthBiasHW		   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("ZBiasHW"), 0, strFileName.c_str() );
        Settings.slopeScaleBias        = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("SlopeScaleBias"), 0, strFileName.c_str() );
        Settings.normalBiasSurface     = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("NormalBiasSurface"), 0, strFileName.c_str() );
        Settings.normalBiasLight       = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("NormalBiasLight"), 0, strFileName.c_str() );
        Settings.minimumVariance	   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MinVariance"), 0, strFileName.c_str() );
        Settings.exponent			   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("Exponent"), 0, strFileName.c_str() );
        Settings.minimumCutoff		   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MinCutoff"), 0, strFileName.c_str() );
        Settings.maskThreshold         = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MaskThreshold"), 0, strFileName.c_str() );
        Settings.translucency          = (GetPrivateProfileInt( lpszEntryName, _T("Translucency"), 0, strFileName.c_str() ) > 0);           
        mShadowSettings.push_back( Settings );

    } // Next entry


    // How many settings entries are in the database?
    nNumEntries = GetPrivateProfileInt( _T("Indirect"), _T("NumEntries"), 0, strFileName.c_str() );

    // Load in the entries
    for ( i = 0; i < nNumEntries; i++ )
    {
        cgString strEntry = cgString::format( _T("IndirectEntry%i"), i );

        cgTChar lpszEntryName[128];
        GetPrivateProfileString( _T("IndirectEntries"), strEntry.c_str(), _T(""), lpszEntryName, 128, strFileName.c_str() );

        cgTChar lpszTechniqueName[128];
        GetPrivateProfileString( lpszEntryName, _T("Technique"), _T(""), lpszTechniqueName, 128, strFileName.c_str() );

        cgShadowSettingsLOD Entry;
        Entry.name = lpszTechniqueName;
        Entry.level  = GetPrivateProfileInt( lpszEntryName, _T("LOD"), 0, strFileName.c_str() );
        mIndirectDetailLevels.push_back( Entry );

        cgShadowSettingsLight Settings;
        Settings.method                = GetPrivateProfileInt( lpszEntryName, _T("Method"), 0, strFileName.c_str() );
        Settings.resolutionAdjust      = GetPrivateProfileInt( lpszEntryName, _T("Resolution"), 0, strFileName.c_str() );
        Settings.cullMode              = (cgCullMode::Base)GetPrivateProfileInt( lpszEntryName, _T("cullMode"), 0, strFileName.c_str() );
        Settings.filterBlurFactor      = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterBlurFactor"), 0, strFileName.c_str() );
        Settings.filterDistanceNear    = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterDistanceNear"), 0, strFileName.c_str() );
        Settings.filterDistanceFar     = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterDistanceFar"), 0, strFileName.c_str() );
        Settings.depthBiasSW           = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("ZBiasSW"), 0, strFileName.c_str() );
        Settings.depthBiasHW		   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("ZBiasHW"), 0, strFileName.c_str() );
        Settings.slopeScaleBias        = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("SlopeScaleBias"), 0, strFileName.c_str() );
        Settings.normalBiasSurface     = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("NormalBiasSurface"), 0, strFileName.c_str() );
        Settings.normalBiasLight       = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("NormalBiasLight"), 0, strFileName.c_str() );
        Settings.minimumVariance	   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MinVariance"), 0, strFileName.c_str() );
        Settings.exponent			   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("Exponent"), 0, strFileName.c_str() );
        Settings.minimumCutoff		   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MinCutoff"), 0, strFileName.c_str() );
        Settings.maskThreshold         = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("MaskThreshold"), 0, strFileName.c_str() );
        Settings.translucency          = (GetPrivateProfileInt( lpszEntryName, _T("Translucency"), 0, strFileName.c_str() ) > 0);           
        Settings.projectCell           = (GetPrivateProfileInt( lpszEntryName, _T("ProjectCell"), 0, strFileName.c_str() ) > 0);      
        Settings.intensity	    	   = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("Intensity"), 1.0f, strFileName.c_str() );

        mIndirectSettings.push_back( Settings );

    } // Next entry

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : buildStateBlocks () (Protected Virtual)
/// <summary>
/// Build state blocks necessary for lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::buildStateBlocks( )
{
    cgResourceManager * pResources = mParentScene->getResourceManager();

    ///////////////////////////////////////////////////////////////////////////
    // Camera Inside Case
    ///////////////////////////////////////////////////////////////////////////
    {
        //*********************************************************************
        // Process standard lighting case first.
        //*********************************************************************
        StateGroup * pStates = &mInLightStates;

        // Depth stencil state
        cgDepthStencilStateDesc dsState;
        dsState.depthEnable      = true;
        dsState.depthWriteEnable = false;
        dsState.depthFunction    = cgComparisonFunction::Greater;
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;
        
        // Rasterizer state
        cgRasterizerStateDesc rsState;
        rsState.cullMode = cgCullMode::Front;
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

    ///////////////////////////////////////////////////////////////////////////
    // Camera Outside Case
    ///////////////////////////////////////////////////////////////////////////
    {
        //*********************************************************************
        // Process stencil fill pass first.
        //*********************************************************************
        StateGroup * pStates = &mOutStencilFillStates;

        // Depth stencil state - stencil buffer fill
        cgDepthStencilStateDesc dsState;
        dsState.depthEnable                         = true;
        dsState.depthWriteEnable                    = false;
        dsState.depthFunction                       = cgComparisonFunction::LessEqual;
        dsState.stencilEnable                       = true;
        dsState.stencilReadMask                     = 1;
        dsState.stencilWriteMask                    = 1;
        dsState.frontFace.stencilFunction           = cgComparisonFunction::Always;
        dsState.frontFace.stencilPassOperation      = cgStencilOperation::Replace;
        dsState.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.backFace                            = dsState.frontFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;

        // Rasterizer state
        cgRasterizerStateDesc rsState;
        rsState.cullMode = cgCullMode::Back;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;

        // Blend state
        cgBlendStateDesc blState;
        blState.renderTarget[0].renderTargetWriteMask = 0;
        if ( !pResources->createBlendState( &pStates->blend, blState, 0, cgDebugSource() ) )
            return false;

        //*********************************************************************
        // Now build states for stencil clear case. This is basically the same
        // as the stencil fill case except for tweaked depth stencil pass and
        // fail rules.
        //*********************************************************************
        pStates = &mOutStencilClearStates;
        pStates->rasterizer = mOutStencilFillStates.rasterizer;
        pStates->blend      = mOutStencilFillStates.blend;

        // Depth stencil state - stencil buffer clear
        dsState.frontFace.stencilFunction           = cgComparisonFunction::Equal;
        dsState.frontFace.stencilPassOperation      = cgStencilOperation::Decr;
        dsState.frontFace.stencilFailOperation      = cgStencilOperation::Keep;
        dsState.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.backFace                            = dsState.frontFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;

        //*********************************************************************
        // Next, build states for standard lighting pass.
        //*********************************************************************
        pStates = &mOutLightStates;

        // Depth stencil state
        dsState.depthFunction                      = cgComparisonFunction::Greater;
        dsState.stencilWriteMask                   = 0;
        dsState.stencilReadMask                    = 1;
        dsState.backFace.stencilFunction           = cgComparisonFunction::Equal;
        dsState.backFace.stencilPassOperation      = cgStencilOperation::Keep;
        dsState.backFace.stencilFailOperation      = cgStencilOperation::Keep;
        dsState.backFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.frontFace                   = dsState.backFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;
        
        // Rasterizer state
        rsState.cullMode = cgCullMode::Front;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;
        
        // Blend state
        blState.renderTarget[0].blendEnable           = true;
        blState.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
        blState.renderTarget[0].sourceBlend           = cgBlendMode::One;
        blState.renderTarget[0].destinationBlend      = cgBlendMode::One;	    
        if ( !pResources->createBlendState( &pStates->blend, blState, 0, cgDebugSource() ) )
            return false;

    } // End Outside Case

    ///////////////////////////////////////////////////////////////////////////
    // Camera Intersecting Case
    ///////////////////////////////////////////////////////////////////////////
    {
        //*********************************************************************
        // Process stencil back fill pass first.
        //*********************************************************************
        StateGroup * pStates = &mSpanStencilFillBStates;

        // Depth stencil state - back face stencil buffer fill
        cgDepthStencilStateDesc dsState;
        dsState.depthEnable                        = true;
        dsState.depthWriteEnable                   = false;
        dsState.depthFunction                      = cgComparisonFunction::Greater;
        dsState.stencilEnable                      = true;
        dsState.stencilReadMask                    = 1;
        dsState.stencilWriteMask                   = 1;
        dsState.backFace.stencilFunction           = cgComparisonFunction::Always;
        dsState.backFace.stencilPassOperation      = cgStencilOperation::Replace;
        dsState.backFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.frontFace                          = dsState.backFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;

        // Rasterizer state
        cgRasterizerStateDesc rsState;
        rsState.cullMode = cgCullMode::Front;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;

        // Blend state
        cgBlendStateDesc blState;
        blState.renderTarget[0].renderTargetWriteMask = 0;
        if ( !pResources->createBlendState( &pStates->blend, blState, 0, cgDebugSource() ) )
            return false;

        //*********************************************************************
        // Process stencil front fill pass.
        //*********************************************************************
        pStates = &mSpanStencilFillFStates;
        pStates->blend = mSpanStencilFillBStates.blend;

        // Depth stencil state - front face stencil buffer fill
        dsState.depthFunction                       = cgComparisonFunction::Less;
        dsState.frontFace.stencilFunction           = cgComparisonFunction::Equal;
        dsState.frontFace.stencilPassOperation      = cgStencilOperation::Keep;
        dsState.frontFace.stencilDepthFailOperation = cgStencilOperation::Decr;
        dsState.backFace                            = dsState.frontFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;

        // Rasterizer state
        rsState.cullMode = cgCullMode::Back;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;

        //*********************************************************************
        // Now build states for stencil clear case. 
        //*********************************************************************
        pStates = &mSpanStencilClearStates;
        pStates->rasterizer = mSpanStencilFillBStates.rasterizer;
        pStates->blend      = mSpanStencilFillBStates.blend;

        // Depth stencil state - stencil buffer clear
        dsState.depthFunction                      = cgComparisonFunction::GreaterEqual;
        dsState.backFace.stencilFunction           = cgComparisonFunction::Equal;
        dsState.backFace.stencilPassOperation      = cgStencilOperation::Decr;
        dsState.backFace.stencilFailOperation      = cgStencilOperation::Keep;
        dsState.backFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.frontFace                          = dsState.backFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;

        //*********************************************************************
        // Next, build states for standard lighting pass.
        //*********************************************************************
        pStates = &mSpanLightStates;

        // Depth stencil state
        dsState.depthFunction                      = cgComparisonFunction::GreaterEqual;
        dsState.stencilWriteMask                   = 0;
        dsState.stencilReadMask                    = 1;
        dsState.backFace.stencilFunction           = cgComparisonFunction::Equal;
        dsState.backFace.stencilPassOperation      = cgStencilOperation::Keep;
        dsState.backFace.stencilFailOperation      = cgStencilOperation::Keep;
        dsState.backFace.stencilDepthFailOperation = cgStencilOperation::Keep;
        dsState.frontFace                          = dsState.backFace; // Duplicate to improve batching.
        if ( !pResources->createDepthStencilState( &pStates->depthStencil, dsState, 0, cgDebugSource() ) )
            return false;
        
        // Rasterizer state
        rsState.cullMode = cgCullMode::Front;
        if ( !pResources->createRasterizerState( &pStates->rasterizer, rsState, 0, cgDebugSource() ) )
            return false;
        
        // Blend state
        blState.renderTarget[0].blendEnable           = true;
        blState.renderTarget[0].renderTargetWriteMask = cgColorChannel::All;
        blState.renderTarget[0].sourceBlend           = cgBlendMode::One;
        blState.renderTarget[0].destinationBlend      = cgBlendMode::One;	    
        if ( !pResources->createBlendState( &pStates->blend, blState, 0, cgDebugSource() ) )
            return false;

    } // End Intersecting Case

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
bool cgLightNode::setCellTransform( const cgTransform & Transform, cgTransformSource::Base Source /* = cgTransformSource::Standard */ )
{
    // Call base class implementation
    if ( !cgObjectNode::setCellTransform( Transform, Source ) )
        return false;

    cgToDo( "Carbon General", "It's possible that the view matrix should be cell relative?" );

    // Extract new light properties
    const cgVector3 & vecRight    = getXAxis( false ), & vecUp = getYAxis( false ), & vecLook = getZAxis( false );
    const cgVector3 & vecPosition = getPosition( false );

    // Set view matrix values
    mViewMatrix._11 = vecRight.x; mViewMatrix._12 = vecUp.x; mViewMatrix._13 = vecLook.x;
    mViewMatrix._21 = vecRight.y; mViewMatrix._22 = vecUp.y; mViewMatrix._23 = vecLook.y;
    mViewMatrix._31 = vecRight.z; mViewMatrix._32 = vecUp.z; mViewMatrix._33 = vecLook.z;
    mViewMatrix._41 = -cgVector3::dot( vecPosition, vecRight );
    mViewMatrix._42 = -cgVector3::dot( vecPosition, vecUp    );
    mViewMatrix._43 = -cgVector3::dot( vecPosition, vecLook  );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : updateSystemConstants() (Virtual)
/// <summary>
/// Update the lighting system constants.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::updateSystemConstants()
{
    // Get access to the lighting manager
    cgLightingManager * pLightingManager = mParentScene->getLightingManager();

    // Set the light's position
    cgVector3 vPosition = getPosition( false );
    static const cgString strConstantLightPosition = _T("_lightPosition");
    pLightingManager->setConstant( strConstantLightPosition, &vPosition );

    // Give the lighting manager the current direction
    cgVector3 vDirection = getZAxis( false );
    static const cgString strConstantLightDirection = _T("_lightDirection");
    pLightingManager->setConstant( strConstantLightDirection, &vDirection );

    // Set the texture projection matrix
    cgMatrix mtxTexRemap, mtxViewProj = mViewMatrix * mProjectionMatrix;
    memset( &mtxTexRemap, 0, sizeof(cgMatrix) );
	mtxTexRemap._11 = 0.5f; mtxTexRemap._22 = -0.5f;
	mtxTexRemap._41 = 0.5f; mtxTexRemap._42 =  0.5f;
    mtxTexRemap._43 = 1.0f; mtxTexRemap._44 =  1.0f;
    static const cgString strConstantLightTexProjMatrix = _T("_lightTexProjMatrix");
    pLightingManager->setConstant( strConstantLightTexProjMatrix, &(mtxViewProj * mtxTexRemap) );

    // ToDo: 6767 - Not supporting attenuation buffer!
    /*// Set the attenuation buffer mask
    static const cgString strAttenuationBufferMask = _T("_lightAttenuationBufferMask");
    switch ( m_nAttenuationBufferChannel )
    {
        case cgLightObject::ChannelMask_Red: 
            pLightingManager->setConstant( strAttenuationBufferMask, &cgVector4(1,0,0,0) );
        break;
        case cgLightObject::ChannelMask_Green: 
            pLightingManager->setConstant( strAttenuationBufferMask, &cgVector4(0,1,0,0) );
        break;
        case cgLightObject::ChannelMask_Blue: 
            pLightingManager->setConstant( strAttenuationBufferMask, &cgVector4(0,0,1,0) );
        break;
        case cgLightObject::ChannelMask_Alpha: 
            pLightingManager->setConstant( strAttenuationBufferMask, &cgVector4(0,0,0,1) );
        break;
        default:
            pLightingManager->setConstant( strAttenuationBufferMask, &cgVector4(1,1,1,1) );
        break;
    }*/

    // Set default system states for texture based features
    cgRenderDriver * pRenderDriver = mParentScene->getRenderDriver();
    pRenderDriver->setSystemState( cgSystemState::ColorTexture3D, false );
    pRenderDriver->setSystemState( cgSystemState::DiffuseIBL,     false );
    pRenderDriver->setSystemState( cgSystemState::SpecularIBL,    false );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : updateLightConstants() (Virtual)
/// <summary>
/// Update the lighting base terms constant buffer as necessary whenever any
/// of the appropriate properties change.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::updateLightConstants()
{
    // Update 'lighting terms' constant buffer.
    cgConstantBuffer * pLightBuffer = mLightConstants.getResource( true );
    cgAssert( pLightBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbLight * pLightData = CG_NULL;
    if ( !(pLightData = (_cbLight*)pLightBuffer->lock( 0, 0, cgLockFlags::WriteOnly ) ) ) // Don't discard, derived class may have already updated.
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock light data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Retrieve necessary values from light object.
    const cgColorValue & cSpecular = getSpecularColor();

    // Update buffer.
    pLightData->position        = getPosition(false);
    pLightData->diffuse         = getDiffuseColor();
    pLightData->specular        = cgColorValue( cSpecular.r, cSpecular.g, cSpecular.b, 1.0f ) * mSpecularAttenuation;
    pLightData->ambient         = getAmbientColor();
    pLightData->ambient2        = getAmbientFarColor();
    pLightData->rim             = getRimColor();
    pLightData->clipDistance.z  = 1.0f / (pLightData->clipDistance.y - pLightData->clipDistance.x);
    pLightData->clipDistance.w  = -pLightData->clipDistance.x / (pLightData->clipDistance.y - pLightData->clipDistance.x);
    cgToDo( "Carbon General", "Populate MipLevel.... x = atten/color tex, y = diffuse IBL max, z = specular IBL max" )
    pLightData->textureMipLevel = cgVector3( 0, 0, 0 );
    cgToDo( "Carbon General", "The default eventually should be 0,0,1,0" )
    pLightData->ambientOcclusionAmount = cgVector4(0,0,0,0);
	
    // ToDo: 6767 - WARNING! Specifies WRITEONLY flag, but reads here!

    // When linear lighting is enabled, we may need to linearize non-linear color values 
    cgRenderDriver * pRenderDriver = mParentScene->getRenderDriver();
    bool m_bManualColorLinearize = pRenderDriver->getSystemState( cgSystemState::HDRLighting ) > 0; //ToDo 9999: Make this a light property and expose in SG UI? 
    if ( m_bManualColorLinearize )
	{
		pLightData->diffuse.r  = powf( pLightData->diffuse.r, 2.2f );
		pLightData->diffuse.g  = powf( pLightData->diffuse.g, 2.2f );
		pLightData->diffuse.b  = powf( pLightData->diffuse.b, 2.2f );
		
		pLightData->ambient.r  = powf( pLightData->ambient.r, 2.2f );
		pLightData->ambient.g  = powf( pLightData->ambient.g, 2.2f );
		pLightData->ambient.b  = powf( pLightData->ambient.b, 2.2f );

		pLightData->ambient2.r = powf( pLightData->ambient2.r, 2.2f );
		pLightData->ambient2.g = powf( pLightData->ambient2.g, 2.2f );
		pLightData->ambient2.b = powf( pLightData->ambient2.b, 2.2f );

		pLightData->specular.r = powf( pLightData->specular.r, 2.2f );
		pLightData->specular.g = powf( pLightData->specular.g, 2.2f );
		pLightData->specular.b = powf( pLightData->specular.b, 2.2f );
	}

    // If HDR lighting is enabled, apply intensity scalars
    if ( pRenderDriver->getSystemState( cgSystemState::HDRLighting ) > 0 )
    {
        float fHDRIntensityDiffuse    = getDiffuseHDRScale();
        float fHDRIntensitySpecular   = getSpecularHDRScale();
        float fHDRIntensityAmbient    = getAmbientHDRScale();
        float fHDRIntensityAmbientFar = getAmbientFarHDRScale();

        pLightData->diffuse.r  *= fHDRIntensityDiffuse;
        pLightData->diffuse.g  *= fHDRIntensityDiffuse;
        pLightData->diffuse.b  *= fHDRIntensityDiffuse;

        pLightData->ambient.r  *= fHDRIntensityAmbient;
        pLightData->ambient.g  *= fHDRIntensityAmbient;
        pLightData->ambient.b  *= fHDRIntensityAmbient;

        pLightData->ambient2.r *= fHDRIntensityAmbientFar;
        pLightData->ambient2.g *= fHDRIntensityAmbientFar;
        pLightData->ambient2.b *= fHDRIntensityAmbientFar;

        pLightData->specular.r *= fHDRIntensitySpecular;
        pLightData->specular.g *= fHDRIntensitySpecular;
        pLightData->specular.b *= fHDRIntensitySpecular;
    
    } // End if HDR

    // Compute active color component permutations
    // ToDo: 6767 - Precompute these (just like bDiffuseSource and bSpecularSource which already exists!!!!!)
    bool bAmbientSource = (fabsf(pLightData->ambient.r) > CGE_EPSILON) || 
                          (fabsf(pLightData->ambient.g) > CGE_EPSILON) ||
                          (fabsf(pLightData->ambient.b) > CGE_EPSILON);
    bool bDiffuseSource = (fabsf(pLightData->diffuse.r) > CGE_EPSILON) || 
                          (fabsf(pLightData->diffuse.g) > CGE_EPSILON) ||
                          (fabsf(pLightData->diffuse.b) > CGE_EPSILON);
    bool bSpecularSource = mComputeSpecular && ((fabsf(pLightData->specular.r) > CGE_EPSILON) || 
                           (fabsf(pLightData->specular.g) > CGE_EPSILON) ||
                           (fabsf(pLightData->specular.b) > CGE_EPSILON));
    bool bTrilightingSource = (fabsf(pLightData->ambient2.r) > CGE_EPSILON) || 
                              (fabsf(pLightData->ambient2.g) > CGE_EPSILON) ||
                              (fabsf(pLightData->ambient2.b) > CGE_EPSILON);
    bool bUseSSAO = (fabsf(pLightData->ambientOcclusionAmount.x) > CGE_EPSILON) || 
                    (fabsf(pLightData->ambientOcclusionAmount.y) > CGE_EPSILON) ||
                    (fabsf(pLightData->ambientOcclusionAmount.z) > CGE_EPSILON);

    // Set the matching system states
    pRenderDriver->setSystemState( cgSystemState::Trilighting, true );
    pRenderDriver->setSystemState( cgSystemState::UseSSAO, bUseSSAO );
    pRenderDriver->setSystemState( cgSystemState::ComputeAmbient, bAmbientSource | bTrilightingSource );
    pRenderDriver->setSystemState( cgSystemState::ComputeDiffuse, bDiffuseSource );
    pRenderDriver->setSystemState( cgSystemState::ComputeSpecular, bSpecularSource );

    // The following constants are set by derived classes:
    // Direction
    // Attenuation
    // Diffuse2
    // Specular2
    // clipDistance.x (Near)
    // clipDistance.y (Far)
    
    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pLightBuffer->unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgLightNode::canScale( ) const
{
    // Light sources cannot be scaled.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : renderShape () 
/// <summary>
/// Draws the light's mesh representation. Light shape transform matrix is 
/// already set via cgRenderDriver::setWorldTransform().
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::renderShape( cgCameraNode * pCamera, cgUInt32 nSubsetId /* = 0 */, cgMesh * pMeshOverride /* = CG_NULL */ )
{
    // Draw the standard light shape.
    cgMesh * pMesh = (pMeshOverride) ? pMeshOverride : mLightShape.getResource(true);
    if ( pMesh && pMesh->isLoaded() )
        pMesh->drawSubset( nSubsetId, cgMeshDrawMode::Simple );
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
void cgLightNode::computeShadowSets( cgCameraNode * pCamera )
{
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
bool cgLightNode::registerVisibility( cgVisibilitySet * pSet )
{
    cgUInt32 nFlags = pSet->getSearchFlags();

    // First test filters.
    if ( (nFlags & cgVisibilitySearchFlags::MustCastShadows) )
        return false;
    if ( (nFlags & cgVisibilitySearchFlags::MustRender) && !isRenderable() )
        return false;

    // Retrieve the frustum used to compute the visibility set.
    const cgFrustum & Frustum = pSet->getVolume( );

    // Determine if the source position of the frustum falls inside 
    // the light volume and perform a frustum visibility test if it 
    // does not (narrow phase).
    //if ( pointInVolume( Frustum.position, 2.0f ) || frustumInVolume( Frustum ) )
        return pSet->addVisibleLight( this );
    
    // We did not modify the visibility set.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : unregisterVisibility () (Virtual)
/// <summary>
/// Unregisters this oject from the specified visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::unregisterVisibility( cgVisibilitySet * visibilityData )
{
    visibilityData->removeVisibleLight( this );
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail() (Virtual)
/// <summary>
/// Called just prior to the scene update process to allow the object to 
/// compute its required detail level for subsequent rendering / processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::computeLevelOfDetail( cgCameraNode * pCamera )
{
    // Call base class implementation.
    cgObjectNode::computeLevelOfDetail( pCamera );

	// Reset LOD values to their initial state.
    cgLightObject * pData = (cgLightObject*)mReferencedObject;
    mComputeShadows  = pData->isShadowSource();
	mComputeSpecular = pData->isSpecularSource();

	// ToDo: Compute a scissor rectangle for this light source.
	//if ( !ComputeScissorRectangle( ) ) return false;

	// Compute distance from the camera to the center of light source
	cgFloat fDistance = cgVector3::length( pCamera->getPosition( false ) - getPosition( false ) );
	
	// Set specular falloff (if applicable)
    mSpecularAttenuation = 1.0f;
    cgFloat fMinDistance = pData->getSpecularMinDistance();
    cgFloat fMaxDistance = pData->getSpecularMaxDistance();
	if ( mComputeSpecular && (fMinDistance > CGE_EPSILON_1MM || fMaxDistance > CGE_EPSILON_1MM) )
	{
		if ( fDistance > fMaxDistance )
		{
			mComputeSpecular     = false;
			mSpecularAttenuation = 0.0f;
		
        } // End if outside range
		else
		{
            // Compute required attenuation value
			if ( fDistance > fMinDistance )
				mSpecularAttenuation = max( 0.0f, min( 1.0f, ( fMaxDistance - fDistance ) / (fMaxDistance - fMinDistance) ) );
			else
				mSpecularAttenuation = 1.0f;
		
        } // End if inside range
	
    } // End if CalcSpecular

	// Set shadow falloff (if applicable)
    mShadowAttenuation = 1.0f;
    fMinDistance = pData->getShadowMinDistance();
    fMaxDistance = pData->getShadowMaxDistance();
	if ( mComputeShadows && (fMinDistance > CGE_EPSILON_1MM || fMaxDistance > CGE_EPSILON_1MM) )
	{
		// Compute shadow attenuation
		if ( fDistance > fMaxDistance )
		{
			mComputeShadows    = false;
			mShadowAttenuation = 0.0f;
		
        } // End if outside range
		else
		{
            // Compute required attenuation value
			if ( fDistance > fMinDistance )
				mShadowAttenuation = max( 0.0f, min( 1.0f, ( fMaxDistance - fDistance ) / (fMaxDistance - fMinDistance) ) );
			else
				mShadowAttenuation = 1.0f;
		
        } // End if inside range
	
    } // End if CalcShadows

    // If we are still computing shadows, select the correct shadow LOD / resolution settings
    mShadowLODScale = 1.0f;
    fMinDistance = pData->getShadowLODMinDistance();
    fMaxDistance = pData->getShadowLODMaxDistance();
	if ( mComputeShadows && (fMinDistance > CGE_EPSILON_1MM || fMaxDistance > CGE_EPSILON_1MM) )
	{
		// Compute shadow LOD scalar
		if ( fDistance > fMaxDistance )
		{
            // Minimum LOD
			mShadowLODScale = 0.0f;
		
        } // End if outside range
		else
		{
            // Compute required LOD value
			if ( fDistance > fMinDistance )
				mShadowLODScale = max( 0.0f, min( 1.0f, ( fMaxDistance - fDistance ) / (fMaxDistance - fMinDistance) ) );
			else
				mShadowLODScale = 1.0f;
		
        } // End if inside range
	
    } // End if CalcShadows

    /*
	// TODO: This is temporary. Move this to light types.
	// Get the currently selected shadow algorithm (given by the artist)
	//nMethod = cgShadowMethod::PCF;
	//nMethod = cgShadowMethod::PCSS;
	//nMethod = cgShadowMethod::Variance;
	//nMethod = cgShadowMethod::Exponential;
	//nMethod = cgShadowMethod::Variance | cgShadowMethod::Exponential;
	cgUInt32 nMethod = cgShadowMethod::PCF;

	// Get the current system level shadow detail settings for this method type
    cgRenderDriver * pDriver = GetScene()->getResourceManager()->getRenderDriver();
	const cgShadowSettings * pSystemLOD = pDriver->GetShadowDetailLevel( nMethod );

	// Ensure the method is allowed given the current system level LOD 
	if ( pSystemLOD->method != nMethod )
		nMethod = pSystemLOD->method;

	m_ShadowLOD.method              = nMethod;
	m_ShadowLOD.Resolution          = min( pSystemLOD->Resolution,       512  );
	m_ShadowLOD.PrimarySamples      = min( pSystemLOD->PrimarySamples,    16  );
	m_ShadowLOD.SecondarySamples    = min( pSystemLOD->SecondarySamples,   0  );
	m_ShadowLOD.FilterRadius        = min( pSystemLOD->FilterRadius,     3.0f );
	m_ShadowLOD.FilterPasses        = min( pSystemLOD->FilterPasses,       1  );
	m_ShadowLOD.filterBlurFactor    = min( pSystemLOD->FilterPasses,     1.0f );

	m_ShadowLOD.filterDistanceNear  = 0.35f;
	m_ShadowLOD.filterDistanceFar   = 4.0f;
	m_ShadowLOD.FilterRadiusNear    = min( pSystemLOD->FilterRadius, 2.0f );
	m_ShadowLOD.FilterRadiusFar     = min( pSystemLOD->FilterRadius, 5.0f );

	m_ShadowLOD.cullMode            = cgCullMode::Front;

	m_ShadowLOD.depthBiasSW             = min( 0, pSystemLOD->depthBiasHW );
    m_ShadowLOD.depthBiasHW             = 0;
    m_ShadowLOD.slopeScaleBias      = 0;
	m_ShadowLOD.normalBiasSurface   = 1.0f;
	m_ShadowLOD.normalBiasLight    = 0.0f;

	m_ShadowLOD.minimumVariance         = 0.001f;
    m_ShadowLOD.minimumCutoff           = 0.4f;
    m_ShadowLOD.exponent            = 64.0f;

	m_ShadowLOD.MaskType            = 0;
	m_ShadowLOD.maskThreshold       = 0.0f;
	m_ShadowLOD.MaskPrecision       = 0;
	*/
}

//-----------------------------------------------------------------------------
//  Name : setScissorRectangle () 
/// <summary>
/// Sets up the screen scissor rectangle for this light source.
/// </summary>
// ToDo : Need to fix this feature.
//-----------------------------------------------------------------------------
void cgLightNode::setScissorRectangle( )
{
    // ToDo: Remove?
    //cgRenderDriver * pDriver = mParentScene->getRenderDriver();
	//pDriver->setScissorRectangle( &mScissorRectangle );
	mSetScissorRect = true;
}

//-----------------------------------------------------------------------------
//  Name : setRenderOptimizations () 
/// <summary>
/// Turns on various optimizations to speed things up during scene rendering/lighting
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::setRenderOptimizations( bool bStencilMask, bool bUserClipPlanes, bool bScissorRect )
{
	mUseLightClipPlanes  = bUserClipPlanes;
	mUseLightScreenRect  = bScissorRect;
}

//-----------------------------------------------------------------------------
//  Name : enableRenderOptimizations () 
/// <summary>
/// Turns on various optimizations to speed things up during scene rendering/lighting
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::enableRenderOptimizations( )
{
	// Clip rendering to projected scissor rectangle
	if ( mUseLightScreenRect )
		setScissorRectangle();

	// Set user-defined clip planes for the light source
    cgToDo( "Carbon General", "May want to move to a push / pop model to avoid overwriting any prior data." )
	if ( mUseLightClipPlanes )
		setClipPlanes();
}

//-----------------------------------------------------------------------------
//  Name : disableRenderOptimizations () 
/// <summary>
/// Turns off the optimizations set by the above function
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::disableRenderOptimizations( )
{
	// Get access to required systems / objects
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();

    // Clear clip planes
    cgToDo( "Carbon General", "May want to move to a push / pop model to avoid overwriting any prior data." )
	if ( mSetClipPlanes ) 
        pDriver->setUserClipPlanes( CG_NULL, 0 );

	// Disable the scissor test
	if ( mSetScissorRect )
        pDriver->setScissorRect( CG_NULL ); 

	// Reset to defaults
	mSetClipPlanes   = false;
	mSetScissorRect  = false;
}

//-----------------------------------------------------------------------------
//  Name : getViewMatrix () 
/// <summary>
/// Retrieve this light source' view matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgLightNode::getViewMatrix( ) const
{
    return mViewMatrix;
}

//-----------------------------------------------------------------------------
//  Name : getProjectionMatrix () 
/// <summary>
/// Retrieve this light source' projection matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgLightNode::getProjectionMatrix( ) const
{
    return mProjectionMatrix;
}

//-----------------------------------------------------------------------------
//  Name : isDiffuseSource () 
/// <summary>
/// Determine if this light is a source of diffuse light.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::isDiffuseSource( ) const
{
    return ((cgLightObject*)mReferencedObject)->isDiffuseSource();
}

//-----------------------------------------------------------------------------
//  Name : isSpecularSource () 
/// <summary>
/// Determine if this light is a source of specular light.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::isSpecularSource( ) const
{
    // We return our own local 'mComputeSpecular' value which is 
    // set during LOD computations (computeLevelOfDetail())
    return mComputeSpecular;
}

//-----------------------------------------------------------------------------
//  Name : isShadowSource () 
/// <summary>
/// Determine if this light is a source of shadow data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::isShadowSource( ) const
{
    // We return our own local 'mComputeShadows' value which is
    // set during LOD computations (computeLevelOfDetail())
    return mComputeShadows;
}


//-----------------------------------------------------------------------------
//  Name : isIndirectSource () 
/// <summary>
/// Determine if this light is a source of reflective shadow map data for 
/// dynamic lighting purposes.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::isIndirectSource( ) const
{
    // ToDo: 6767 - This should be based SOLELY on 'mComputeIndirect'. The OBJECT'S
    // isIndirectSource() method should be the one doing this.
    // If we have a non-zero indirect lighting intensity, this is an indirect source
    for ( size_t i = 0; i < mIndirectSettings.size(); ++i )
    {
        if ( mIndirectSettings[ i ].intensity > 0.0f )
            return true;
    
    } // Next LOD
    return false;
}

//-----------------------------------------------------------------------------
//  Name : reassignShadowMaps() (Virtual)
/// <summary>
/// Allows the light source to determine if it is necessary to partake in
/// the assignment of new shadow maps at this stage, or if it can re-use
/// shadow maps that it may have populated earlier.
/// Note : Returning true indicates that the light does not require further
/// shadow map assignment processing (i.e. it reused pooled maps).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::reassignShadowMaps( cgTexturePool * pPool )
{
    // By default, the light source does not re-use shadow maps, and
    // will later be asked to select any maps that it needs (assuming 
    // that the light requires shadow maps at all).
    return false;
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
cgInt32 cgLightNode::beginShadowFill( cgTexturePool * pPool )
{
    // If we are not ready to begin, or there is nothing in the illumination set, fail.
    if ( !isShadowSource() || mCurrentShadowPass > -2 )
        return -1;

    // Set the light type.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setSystemState( cgSystemState::LightType, getLightType() );

    cgToDo( "Carbon General", "This update process only needs to be performed when a property changes (OnNodeUpdated())" )
    updateLightConstants();

    // Bind the base lighting terms constant buffer.
    pDriver->setConstantBufferAuto( mLightConstants );

    // Ready for first pass.
    mShadowPasses.clear();
    mShadowPassCount   = 0;
    mCurrentShadowPass = -1;
    mTexturePool       = pPool;
    mShadowTimedUpdate = true;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : beginShadowFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::beginShadowFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Be polite and clear output variable.
    pRenderSetOut = CG_NULL;

    // Valid begin operation?
    if ( mCurrentShadowPass != -1 || nPass >= mShadowPassCount )
        return false;

    // Nothing in base implementation.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFillPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endShadowFillPass( )
{
    // Valid end operation?
    if ( mCurrentShadowPass < 0 || mCurrentShadowPass >= mShadowPassCount )
        return false;

    // Ready for next pass
    mCurrentShadowPass = -1;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endShadowFill() (Virtual)
/// <summary>
/// Called in order to signify that the shadow fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endShadowFill( )
{
    if ( mCurrentShadowPass == -2 )
        return false;
    if ( mCurrentShadowPass != -1 )
        endShadowFillPass();

    // We're done.
    mCurrentShadowPass  = -2;
    mShadowPassCount    = 0;
    mShadowPasses.clear();
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
cgInt32 cgLightNode::beginLighting( cgTexturePool * pPool, bool bApplyShadows, bool bDeferred )
{
    cgToDo( "Queue", "COMMENTED OUT ILLUMINATION SET TEST" );
    // If we are not ready to begin, or there is nothing in the illumination set, fail.
    if ( /*( !bDeferred && m_pIlluminationSet->IsEmpty() ) ||*/ mCurrentLightingPass > -2 )
        return -1;

    // Allow light source object to set itself up.
    if ( mReferencedObject && !((cgLightObject*)mReferencedObject)->beginLighting() )
        return -1;

    cgToDo( "Carbon General", "This update process only needs to be performed when a property changes (OnNodeUpdated())" )
    updateLightConstants();
    updateSystemConstants();

    // Bind the base lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mLightConstants );

    // If this is /not/ a shadow casting light source, upload the actual lighting
    // system data. If it is a shadow casting light however, this data will
    // be uploaded by the shadow frustum.
    if ( !bApplyShadows || !isShadowSource() )
        mParentScene->getLightingManager()->applyLightingConstants();
    
    // Ready for first pass.
    mLightingPasses.clear();
    mLightingPassCount    = 0;
    mCurrentLightingPass  = -1;
    mLightingApplyShadows = bApplyShadows;
    mLightingDeferred     = bDeferred;
    mTexturePool          = pPool;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : beginLightingPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the lighting process.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::LightingOp cgLightNode::beginLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Be polite and clear output variable.
    pRenderSetOut = CG_NULL;

    // Valid begin operation?
    if ( mCurrentLightingPass != -1 || nPass >= mLightingPassCount )
        return Lighting_Abort;

    // Nothing in base implementation.
    return Lighting_None;
}

//-----------------------------------------------------------------------------
//  Name : endLightingPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endLightingPass( )
{
    // Valid end operation?
    if ( mCurrentLightingPass < 0 || mCurrentLightingPass >= mLightingPassCount )
        return false;

    // Ready for next pass
    mCurrentLightingPass = -1;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endLighting() (Virtual)
/// <summary>
/// Called in order to signify that the lighting process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endLighting( )
{
    if ( mCurrentLightingPass == -2 )
        return false;
    if ( mCurrentLightingPass != -1 )
        endLightingPass();

    // We're done.
    mCurrentLightingPass = -2;
    mLightingPassCount   = 0;
    mLightingPasses.clear();
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
cgInt32 cgLightNode::beginIndirectLighting( cgTexturePool * pPool )
{
    // If we are not ready to begin, fail.
    if ( mCurrentLightingPass > -2 )
        return -1;

    // Allow light source object to set itself up.
    if ( mReferencedObject && !((cgLightObject*)mReferencedObject)->beginLighting() )
        return -1;

    cgToDo( "Carbon General", "This update process only needs to be performed when a property changes (OnNodeUpdated())" )
    updateLightConstants();
    updateSystemConstants();

    // Bind the base lighting terms constant buffer.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setConstantBufferAuto( mLightConstants );

    // Ready for first pass.
    mLightingPasses.clear();
    mLightingPassCount    = 0;
    mCurrentLightingPass  = -1;
    mTexturePool          = pPool;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectLightingPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the indirect lighting process.
/// </summary>
//-----------------------------------------------------------------------------
cgLightNode::LightingOp cgLightNode::beginIndirectLightingPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Be polite and clear output variable.
    pRenderSetOut = CG_NULL;

    // Valid begin operation?
    if ( mCurrentLightingPass != -1 || nPass >= mLightingPassCount )
        return Lighting_Abort;

    // Nothing in base implementation.
    return Lighting_None;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectLightingPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the indirect lighting process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endIndirectLightingPass( )
{
    // Valid end operation?
    if ( mCurrentLightingPass < 0 || mCurrentLightingPass >= mLightingPassCount )
        return false;

    // Ready for next pass
    mCurrentLightingPass = -1;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectLighting() (Virtual)
/// <summary>
/// Called in order to signify that the indirect lighting process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endIndirectLighting( )
{
    if ( mCurrentLightingPass == -2 )
        return false;
    if ( mCurrentLightingPass != -1 )
        endLightingPass();

    // We're done.
    mCurrentLightingPass = -2;
    mLightingPassCount   = 0;
    mLightingPasses.clear();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : renderDeferred () (Protected)
/// <summary>
/// Renders deferred lighting with optional shadows.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::renderDeferred( const cgTransform & LightShapeTransform, cgVolumeQuery::Class CameraVolumeStatus, cgShadowGenerator * pShadowFrustum /* = CG_NULL */, const cgMatrix * pLightTexProj /* = CG_NULL */, cgMesh * pMeshOverride /* = CG_NULL */, cgUInt32 nSubsetId /* = 0 */ )
{
    // Get access to required systems.
    cgRenderDriver* pDriver = mParentScene->getRenderDriver();
    cgCameraNode  * pCamera = pDriver->getCamera();
    
    // Force rendering approach for orthographic camera.
    if ( pCamera->getProjectionMode() == cgProjectionMode::Orthographic )
    {
        // Note: Could use 'Outside' in most cases, but intersect is safest.
        CameraVolumeStatus = cgVolumeQuery::Intersect;
    
    } // End if Orthographic
    
    // Render!
    cgSurfaceShader * pShader = mShader.getResource(true);
    if ( pShader && pShader->isLoaded() )
    {
        cgToDo( "Carbon General", "Cache some of these shaders?" )

        // Supply shadow map settings and sampler data as necessary (if shadowing)
        if ( pShadowFrustum )
            pShadowFrustum->beginRead( mShadowAttenuation, 10000, 10001, pLightTexProj );   
        
        // Is the main camera outside, inside or intersecting the light source volume?
        bool bCanDraw = true;
        if ( CameraVolumeStatus == cgVolumeQuery::Inside )
        {
            // Select common states
            pDriver->setWorldTransform( LightShapeTransform );
            bCanDraw &= pShader->selectVertexShader( _T("transform"), mLightingVSArgs );

            // Apply states
            pDriver->setDepthStencilState( mInLightStates.depthStencil );
            pDriver->setRasterizerState( mInLightStates.rasterizer );
            pDriver->setBlendState( mInLightStates.blend );

            // Select vertex / pixel shaders
            bCanDraw &= pShader->selectPixelShader( _T("drawDirectLighting"), mLightingPSArgs );

            // Render light shape.
            if ( bCanDraw )
                renderShape( pCamera, nSubsetId, pMeshOverride );

        } // End if Inside
        else if ( CameraVolumeStatus == cgVolumeQuery::Outside )
        {
            // Pass 1 - Apply states for stencil fill (stencil ref = 1).
            pDriver->setDepthStencilState( mOutStencilFillStates.depthStencil, 1 );
            pDriver->setRasterizerState( mOutStencilFillStates.rasterizer );
            pDriver->setBlendState( mOutStencilFillStates.blend );
            pDriver->setWorldTransform( LightShapeTransform );

            // Select vertex / pixel shaders
            bCanDraw &= pShader->selectVertexShader( _T("transform"), mStencilVSArgs );
            bCanDraw &= pShader->selectPixelShader( _T("nullOutput") );

            // Render light shape.
            if ( bCanDraw )
            {
	            renderShape( pCamera, nSubsetId, pMeshOverride );

                // Pass 2 - Lighting / shadowing
                // Apply states for lighting.
                pDriver->setDepthStencilState( mOutLightStates.depthStencil, 1 );
                pDriver->setRasterizerState( mOutLightStates.rasterizer );
                pDriver->setBlendState( mOutLightStates.blend );

                // Select vertex / pixel shaders
                bCanDraw &= pShader->selectVertexShader( _T("transform"), mLightingVSArgs );
                bCanDraw &= pShader->selectPixelShader( _T("drawDirectLighting"), mLightingPSArgs );
                
                // Render light shape.
                if ( bCanDraw )
                {
	                renderShape( pCamera, nSubsetId, pMeshOverride );

                    // Pass 3 - Apply states for stencil clear (stencil ref = 1).
                    pDriver->setDepthStencilState( mOutStencilClearStates.depthStencil, 1 );
                    pDriver->setRasterizerState( mOutStencilClearStates.rasterizer );
                    pDriver->setBlendState( mOutStencilClearStates.blend );

                    // Select vertex / pixel shaders
                    bCanDraw &= pShader->selectVertexShader( _T("transform"), mStencilVSArgs );
                    bCanDraw &= pShader->selectPixelShader( _T("nullOutput") );

                    // Render light shape.
                    if ( bCanDraw )
	                    renderShape( pCamera, nSubsetId, pMeshOverride );

                } // End if can draw pass 2

            } // End if can draw pass 1

        } // End if Outside
        else
        {
            // Pass 1 - Apply states for stencil back fill (stencil ref = 1).
            pDriver->setDepthStencilState( mSpanStencilFillBStates.depthStencil, 1 );
            pDriver->setRasterizerState( mSpanStencilFillBStates.rasterizer );
            pDriver->setBlendState( mSpanStencilFillBStates.blend );
            pDriver->setWorldTransform( LightShapeTransform );

            // Select vertex / pixel shaders
            bCanDraw &= pShader->selectVertexShader( _T("transform"), mStencilVSArgs );
            bCanDraw &= pShader->selectPixelShader( _T("nullOutput") );

            // Render light shape.
            if ( bCanDraw )
            {
	            renderShape( pCamera, nSubsetId, pMeshOverride );

                // Pass 2 - Apply states for stencil front fill (stencil ref = 1).
                pDriver->setDepthStencilState( mSpanStencilFillFStates.depthStencil, 1 );
                pDriver->setRasterizerState( mSpanStencilFillFStates.rasterizer );
                pDriver->setBlendState( mSpanStencilFillFStates.blend );

                // Render light shape (same shaders).
	            renderShape( pCamera, nSubsetId, pMeshOverride );

                // Pass 3 - Lighting / shadowing
                // Apply states for lighting.
                pDriver->setDepthStencilState( mSpanLightStates.depthStencil, 1 );
                pDriver->setRasterizerState( mSpanLightStates.rasterizer );
                pDriver->setBlendState( mSpanLightStates.blend );

                // Select vertex / pixel shaders
                bCanDraw &= pShader->selectVertexShader( _T("transform"), mLightingVSArgs );
                bCanDraw &= pShader->selectPixelShader( _T("drawDirectLighting"), mLightingPSArgs );
                
                // Render light shape.
                if ( bCanDraw )
                {
	                renderShape( pCamera, nSubsetId, pMeshOverride );

                    // Pass 4 - Apply states for stencil clear (stencil ref = 1).
                    pDriver->setDepthStencilState( mSpanStencilClearStates.depthStencil, 1 );
                    pDriver->setRasterizerState( mSpanStencilClearStates.rasterizer );
                    pDriver->setBlendState( mSpanStencilClearStates.blend );

                    // Select vertex / pixel shaders
                    bCanDraw &= pShader->selectVertexShader( _T("transform"), mStencilVSArgs );
                    bCanDraw &= pShader->selectPixelShader( _T("nullOutput") );

                    // Render light shape.
                    if ( bCanDraw )
	                    renderShape( pCamera, nSubsetId, pMeshOverride );

                } // End if can draw pass 3

            } // End if can draw pass 1 & 2

        } // End if Intersecting

        // Finish up
        if ( pShadowFrustum )
            pShadowFrustum->endRead( );

    } // End if surface shader loaded
}

//-----------------------------------------------------------------------------
//  Name : beginRenderForward () (Protected)
/// <summary>
/// Begins forward lighting process (with optional shadows).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::beginRenderForward( const cgTransform & LightShapeTransform, cgShadowGenerator * pShadowFrustum, const cgFrustum * pFrustumOverride, const cgMatrix * pLightTexProj )
{
    // Cache information necessary for endRenderForward()
    mShapeMatrixCache = LightShapeTransform;
    mGeneratorCache = pShadowFrustum;

    // ToDo: Rewrite these comments. Frustum is provided if we want frustum drawing, not just if we want shadows.
	// If we are not computing shadows, draw the full illumination set (with optional optimizations)
	if ( !pShadowFrustum )
	{
        // Turn on light optimizations
	    enableRenderOptimizations();

	} // End if no shadows
	else
	{
        cgToDo( "Carbon General", "Support forward shadow casting again." );
        /*cgRenderDriver * pDriver = mParentScene->getRenderDriver();
        cgEffectFile   * pEffect = m_hEffect;
        cgMesh         * pMesh   = mLightShape;
        cgFrustum        Frustum;

        // Select correct frustum for interior testing.
		if ( !pFrustumOverride )
			Frustum = pShadowFrustum->getCamera()->GetFrustum();
		else
			Frustum = *pFrustumOverride;

        // ToDo: This isn't necessary in all cases (especially single frustum light sources).
		// Cull the objects in the input list to build a local visibility set
        m_pReceiversSet->Clear();
		m_pIlluminationSet->Intersect( Frustum, m_pReceiversSet );
		cgObjectNodeSet & Objects = m_pReceiversSet->GetVisibleObjects();
		if ( Objects.empty() )
            return false;

		// Clip planes can also be used to optimize pixel processing 
		if ( mUseLightClipPlanes )
			pDriver->setUserClipPlanes( Frustum.Planes, 6, true );

		// Supply shadow map settings and sampler data
        pShadowFrustum->beginRead( pDriver, m_hEffect, mShadowAttenuation, pLightTexProj );*/

	} // End if shadow caster

    // Proceed with render.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endRenderForward () (Protected)
/// <summary>
/// Completes forward lighting process (with optional shadows).
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::endRenderForward(  )
{
    if ( !mGeneratorCache )
	{
		// Turn off optimizations
		disableRenderOptimizations();
	
    } // End if no shadows
	else
	{
        cgToDo( "Carbon General", "Support forward shadow casting again." );
        /*cgRenderDriver * pDriver = mParentScene->getRenderDriver();
        cgEffectFile   * pEffect = m_hEffect;
        cgMesh         * pMesh   = mLightShape;

        // Release allocated memory.
        m_pReceiversSet->Clear();

		// Restore state.
        m_pGeneratorCache->endRead( pDriver );

		// Clear the clip planes if we used any.
		if ( mUseLightClipPlanes )
			pDriver->ClearUserClipPlanes();*/

	} // End if shadow caster
}

// ToDo: 6767 - For all light types, roll ComputeIndirectSets into compute visibility like we did for computeShadowSets
/*//-----------------------------------------------------------------------------
//  Name : ComputeIndirectSets( ) (Virtual)
/// <summary>
/// Creates the rsm visibility sets. 
/// </summary>
//-----------------------------------------------------------------------------
void cgLightNode::ComputeIndirectSets( cgCameraNode * pCamera )
{
    // Compute the visibility set for the purposes of indirect lighting.
    m_pIndirectSet->Clear();

    //if ( isIndirectSource() && !m_pVisibility->IsEmpty() )
    if ( !m_pVisibility->IsEmpty() )
    {
        cgObjectNodeSet::iterator itObject;
        cgObjectNode * pObject;

        // ToDo: Process spatial tree leaves.

        // Process all objects visible to the light source.
        cgObjectNodeSet & VisibleObjects = m_pVisibility->GetVisibleObjects( );
        for ( itObject = VisibleObjects.begin(); itObject != VisibleObjects.end(); ++itObject )
        {
            pObject = (*itObject);

            // ToDo: Skip spatial tree objects?

            // If this object cannot participate in the indirect lighting process, skip it.
            // If this object cannot participate in the shadowing process (i.e. it's 
            // not a mesh of some kind), then we can skip it.
            if ( !pObject->IsShadowCaster() )
                continue;

            // if ( !pObject->IsIndirectLightingObject() )
            //    continue;

            // Add object to set
            m_pIndirectSet->AddVisibleObject( pObject );

        } // Next Visible Object

    } // End if isIndirectSource

    // It is not necessary to bounce light if the set is empty.
    mComputeIndirect = !m_pIndirectSet->IsEmpty();
}*/


//-----------------------------------------------------------------------------
//  Name : reassignIndirectMaps() (Virtual)
/// <summary>
/// Allows the light source to determine if it is necessary to partake in
/// the assignment of new shadow resources at this stage, or if it can re-use
/// maps that it may have populated earlier.
/// Note : Returning true indicates that the light does not require further
/// resource assignment processing (i.e. it reused pooled maps).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::reassignIndirectMaps( cgTexturePool * pPool )
{
    // By default, the light source does not re-use resources, and
    // will later be asked to select any maps that it needs.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectFill() (Virtual)
/// <summary>
/// Assuming the light would like to opt in to the population of any
/// assigned reflective shadow maps, this method is called to start that 
/// process and should return the total number of fill passes that will be
/// required.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgLightNode::beginIndirectFill( cgTexturePool * pPool )
{
    // If we are not ready to begin, or there is nothing in the illumination set, fail.
    if ( !isIndirectSource() || mCurrentShadowPass > -2 )
        return -1;

    // Set the light type.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setSystemState( cgSystemState::LightType, getLightType() );
    
    cgToDo( "Carbon General", "This update process only needs to be performed when a property changes (OnNodeUpdated())" )
    updateLightConstants();

    // Bind the base lighting terms constant buffer.
    pDriver->setConstantBufferAuto( mLightConstants );

    // Ready for first pass.
    mShadowPasses.clear();
    mShadowPassCount   = 0;
    mCurrentShadowPass = -1;
    mTexturePool       = pPool;
    mShadowTimedUpdate = true;
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : beginIndirectFillPass() (Virtual)
/// <summary>
/// Called in order to begin an individual pass of the rsm fill process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::beginIndirectFillPass( cgInt32 nPass, cgVisibilitySet *& pRenderSetOut )
{
    // Be polite and clear output variable.
    pRenderSetOut = CG_NULL;

    // Valid begin operation?
    if ( mCurrentShadowPass != -1 || nPass >= mShadowPassCount )
        return false;

    // Nothing in base implementation.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectFillPass() (Virtual)
/// <summary>
/// Called in order to end an individual pass of the reflective shadow fill
/// process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endIndirectFillPass( )
{
    // Valid end operation?
    if ( mCurrentShadowPass < 0 || mCurrentShadowPass >= mShadowPassCount )
        return false;

    // Ready for next pass
    mCurrentShadowPass = -1;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : endIndirectFill() (Virtual)
/// <summary>
/// Called in order to signify that the reflective shadow fill process is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::endIndirectFill( )
{
    if ( mCurrentShadowPass == -2 )
        return false;
    if ( mCurrentShadowPass != -1 )
        endIndirectFillPass( );

    // We're done.
    mCurrentShadowPass  = -2;
    mShadowPassCount    = 0;
    mShadowPasses.clear();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightNode::getSandboxIconInfo( cgCameraNode * pCamera, const cgSize & ViewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin )
{
    strAtlas  = _T("sys://Textures/ObjectBillboardSheet.xml");
    strFrame  = _T("Light");
    
    // Position icon
    vIconOrigin = getPosition(false);
    cgFloat fZoomFactor = pCamera->estimateZoomFactor( ViewportSize, vIconOrigin, 2.5f );
    vIconOrigin += (pCamera->getXAxis() * 0.0f * fZoomFactor) + (pCamera->getYAxis() * 17.0f * fZoomFactor);
    return true;
}
