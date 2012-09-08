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
// Name : cgLightingManager.cpp                                              //
//                                                                           //
// Desc : Provides classes responsible for the management of the various     //
//        lighting systems used by the engine.                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLightingManager Module Includes
//-----------------------------------------------------------------------------
#include <World/Lighting/cgLightingManager.h>
#include <World/Lighting/cgRadianceGrid.h>
#include <World/Lighting/cgShadowGenerator.h>       // ToDo: 6767 - May no longer be required if types move into lighting types.
#include <World/Objects/cgSpatialTreeObject.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <World/Objects/cgMeshObject.h>
#include <World/cgScene.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgScript.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgRenderingCapabilities.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgTexturePool.h>
#include <Resources/cgTexture.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <System/cgProfiler.h>
#include <Math/cgMathUtility.h>
#include <algorithm>

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgExceptions;

///////////////////////////////////////////////////////////////////////////////
// cgLightingManager Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLightingManager() (Constructor)
/// <summary>Class constructor.</summary>
/// <param name="pParent">The scene which owns this manager.</param>
//-----------------------------------------------------------------------------
cgLightingManager::cgLightingManager( cgScene * pParent )
{
    // Cache the parent scene
    mParentScene          = pParent;

    // Initialize variables to sensible defaults
    mScriptObject         = CG_NULL;
    mShadowSystemLOD      = 0;
    mIndirectMethod       = cgIndirectLightingMethod::RadianceHints;
    mIndirectSystemLOD    = 0;
    mDynamicTimeThreshold = 1.5f;
    
    // Allocate the shadow map pool. 
    mShadowMapPool        = new cgTexturePool();
}

//-----------------------------------------------------------------------------
//  Name : ~cgLightingManager() (Destructor)
/// <summary>
/// Class destructor. Falls through to the class' <see cref="dispose()" />
/// method.
/// </summary>
//-----------------------------------------------------------------------------
cgLightingManager::~cgLightingManager()
{
    // Release allocated memory
    dispose( false );

    // Destroy the shadow map pool (constructor allocated -- so one must 
    // be available even if 'dispose()' is called).
    if ( mShadowMapPool )
        mShadowMapPool->scriptSafeDispose();
    mShadowMapPool = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgLightingManager::dispose( bool bDisposeBase )
{
    // Dispose of samplers.
    if ( mStates.harmonicsRedSampler )
        mStates.harmonicsRedSampler->scriptSafeDispose();
    if ( mStates.harmonicsGreenSampler )
        mStates.harmonicsGreenSampler->scriptSafeDispose();
    if ( mStates.harmonicsBlueSampler )
        mStates.harmonicsBlueSampler->scriptSafeDispose();
    if ( mStates.harmonicsAuxiliarySampler )
        mStates.harmonicsAuxiliarySampler->scriptSafeDispose();
    if ( mStates.harmonicsRedPointSampler )
        mStates.harmonicsRedPointSampler->scriptSafeDispose();
    if ( mStates.harmonicsGreenPointSampler )
        mStates.harmonicsGreenPointSampler->scriptSafeDispose();
    if ( mStates.harmonicsBluePointSampler )
        mStates.harmonicsBluePointSampler->scriptSafeDispose();
    if ( mStates.harmonicsAuxiliaryPointSampler )
        mStates.harmonicsAuxiliaryPointSampler->scriptSafeDispose();
    if ( mStates.harmonicsRedVerticalSampler )
        mStates.harmonicsRedVerticalSampler->scriptSafeDispose();
    if ( mStates.harmonicsGreenVerticalSampler )
        mStates.harmonicsGreenVerticalSampler->scriptSafeDispose();
    if ( mStates.harmonicsBlueVerticalSampler )
        mStates.harmonicsBlueVerticalSampler->scriptSafeDispose();
    if ( mStates.harmonicsRedHorizontalSampler )
        mStates.harmonicsRedHorizontalSampler->scriptSafeDispose();
    if ( mStates.harmonicsGreenHorizontalSampler )
        mStates.harmonicsGreenHorizontalSampler->scriptSafeDispose();
    if ( mStates.harmonicsBlueHorizontalSampler )
        mStates.harmonicsBlueHorizontalSampler->scriptSafeDispose();
    if ( mStates.depthLowResSampler )
        mStates.depthLowResSampler->scriptSafeDispose();
    if ( mStates.lightingSampler )
        mStates.lightingSampler->scriptSafeDispose();

    // Close the lighting system handles
    mStates.lightingSystemConstants.close();
    mStates.lightingShader.close();
    mStates.bilateralConstants.close();
    mStates.currentLightingTarget.close();
    mStates.lightingTargetLow.close();
    mStates.depthTargetLow.close();
    mStates.depthStencilBufferLow.close(); 
    mStates.depthStencilBufferHigh.close();
    mStates.disabledDepthState.close();
    mStates.additiveBlendState.close();
    mStates.disabledBlendState.close();
    mStates.alphaBlendState.close();
    mStates.multiplyDestBlendState.close();
    mStates.testStencilDepthState.close();
    mStates.stencilClearState.close();
    mStates.lessEqualTestDepthState.close();
 
    // Release any script objects we retain.
    if ( mScriptObject )
        mScriptObject->release();

    // Release allocated objects.
    for ( size_t i = 0; i < mScratchRadianceBuffers0.size(); ++i )
        delete mScratchRadianceBuffers0[ i ];
    for ( size_t i = 0; i < mRadianceGrids.size(); ++i )
        delete mRadianceGrids[ i ];
    if ( mShadowMapPool )
        mShadowMapPool->scriptSafeDispose();
    
    // Clear members
    mStates.harmonicsRedSampler             = CG_NULL;
    mStates.harmonicsGreenSampler           = CG_NULL;
    mStates.harmonicsBlueSampler            = CG_NULL;
    mStates.harmonicsAuxiliarySampler       = CG_NULL;
    mStates.harmonicsRedPointSampler        = CG_NULL;
    mStates.harmonicsGreenPointSampler      = CG_NULL;
    mStates.harmonicsBluePointSampler       = CG_NULL;
    mStates.harmonicsAuxiliaryPointSampler  = CG_NULL;
    mStates.harmonicsRedVerticalSampler     = CG_NULL;
    mStates.harmonicsGreenVerticalSampler   = CG_NULL;
    mStates.harmonicsBlueVerticalSampler    = CG_NULL;
    mStates.harmonicsRedHorizontalSampler   = CG_NULL;
    mStates.harmonicsGreenHorizontalSampler = CG_NULL;
    mStates.harmonicsBlueHorizontalSampler  = CG_NULL;
    mStates.depthLowResSampler              = CG_NULL;
    mStates.lightingSampler                 = CG_NULL;
    mScriptObject                           = CG_NULL;
    mStaticIndirectLights.clear();
    mDynamicIndirectLights.clear();
    mScratchRadianceBuffers0.clear();
    mRadianceGrids.clear();

    // Recreate an empty shadow map pool
    mShadowMapPool = new cgTexturePool();
}

//-----------------------------------------------------------------------------
//  Name : setRenderControl ()
/// <summary>Change the active render control object class.</summary>
//-----------------------------------------------------------------------------
void cgLightingManager::setRenderControl( cgScriptObject * pScriptObject )
{
    if ( mScriptObject )
        mScriptObject->release();
    mScriptObject = pScriptObject;
    if ( mScriptObject )
        mScriptObject->addRef();
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>Initialize the lighting manager.</summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::initialize( cgScriptObject * pScriptObject )
{
    // Keep a copy of the script object
    mScriptObject = pScriptObject;
    if ( mScriptObject )
        mScriptObject->addRef();

    // Get access to required systems.
    cgResourceManager * pResources = cgResourceManager::getInstance();

    // Load the core lighting integration shader.
    if ( !pResources->createSurfaceShader( &mStates.lightingShader, _T("sys://Shaders/LightSource.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load/compile light source shader for scene '%s'.\n"), mParentScene->getName().c_str() );
        return false;
    
    } // End if failed

    // Create the lighting system constant buffer
    if ( !pResources->createConstantBuffer( &mStates.lightingSystemConstants, mStates.lightingShader, _T("_cbLightingSystem"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mStates.bilateralConstants, mStates.lightingShader, _T("cbBilateral"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required light source constant buffers for scene '%s'.\n"), mParentScene->getName().c_str() );
        return false;
    
    } // End if failed
    cgAssert( mStates.lightingSystemConstants->getDesc().length == sizeof( _cbLightingSystem ) );
    cgAssert( mStates.bilateralConstants->getDesc().length == sizeof( _cbBilateral ) );

    ///////////////////////////////////////////////////////////////
    // Depth Stencil States
    ///////////////////////////////////////////////////////////////
    bool bSuccess = true;
    cgDepthStencilStateDesc dsStates;
    dsStates.depthEnable      = false;
    dsStates.depthWriteEnable = false;
    bSuccess &= pResources->createDepthStencilState( &mStates.disabledDepthState, dsStates, 0, cgDebugSource() );
    
    dsStates.depthEnable      = true;
    dsStates.depthFunction        = cgComparisonFunction::Greater;
    bSuccess &= pResources->createDepthStencilState( &mStates.greaterDepthState, dsStates, 0, cgDebugSource() );

    dsStates.depthFunction        = cgComparisonFunction::LessEqual;
    bSuccess &= pResources->createDepthStencilState( &mStates.lessEqualTestDepthState, dsStates, 0, cgDebugSource() );
        
    dsStates.depthEnable                  = true;
    dsStates.depthFunction                    = cgComparisonFunction::Greater;
    dsStates.stencilEnable                = true;
    dsStates.stencilReadMask              = 0xff;
    dsStates.stencilWriteMask             = 0;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Equal;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Keep;
    dsStates.frontFace.stencilDepthFailOperation = cgStencilOperation::Keep;
    dsStates.backFace                     = dsStates.frontFace;
    bSuccess &= pResources->createDepthStencilState( &mStates.testStencilDepthState, dsStates, 0, cgDebugSource() );

    dsStates.stencilReadMask              = 0xff;
    dsStates.stencilWriteMask             = 0xff;
    dsStates.frontFace.stencilFunction        = cgComparisonFunction::Always;
    dsStates.frontFace.stencilPassOperation      = cgStencilOperation::Zero;
    dsStates.backFace                     = dsStates.frontFace;
    bSuccess &= pResources->createDepthStencilState( &mStates.stencilClearState, dsStates, 0, cgDebugSource() );

    // All states created successfully?
    if ( !bSuccess )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required light source depth stencil states for scene '%s'.\n"), mParentScene->getName().c_str() );
        return false;

    } // End if failed

    ///////////////////////////////////////////////////////////////
    // Blend States
    ///////////////////////////////////////////////////////////////
    cgBlendStateDesc blStates;
    for ( cgInt i = 0; i < 4; ++i )
    {
        blStates.renderTarget[i].renderTargetWriteMask    = cgColorChannel::All;
        blStates.renderTarget[i].blendEnable              = true;
        blStates.renderTarget[i].sourceBlend              = cgBlendMode::SrcAlpha;
        blStates.renderTarget[i].destinationBlend         = cgBlendMode::InvSrcAlpha;

    } // Next target
    bSuccess &= pResources->createBlendState( &mStates.alphaBlendState, blStates, 0, cgDebugSource() );

    for ( cgInt i = 0; i < 4; i++ )
    {
        blStates.renderTarget[i].renderTargetWriteMask    = cgColorChannel::All;
        blStates.renderTarget[i].blendEnable              = true;
        blStates.renderTarget[i].sourceBlend              = cgBlendMode::One;
        blStates.renderTarget[i].destinationBlend         = cgBlendMode::One;

    } // Next target
    bSuccess &= pResources->createBlendState( &mStates.additiveBlendState, blStates, 0, cgDebugSource() );

    for ( cgInt i = 0; i < 4; i++ )
    {
        blStates.renderTarget[i].renderTargetWriteMask    = cgColorChannel::None;
        blStates.renderTarget[i].blendEnable              = false;
        blStates.renderTarget[i].separateAlphaBlendEnable = false;

    } // Next target
    bSuccess &= pResources->createBlendState( &mStates.disabledBlendState, blStates, 0, cgDebugSource() );

    for ( cgInt i = 0; i < 4; i++ )
    {
        blStates.renderTarget[i].renderTargetWriteMask    = cgColorChannel::All;
        blStates.renderTarget[i].blendEnable              = true;
        blStates.renderTarget[i].sourceBlend              = cgBlendMode::Zero;
        blStates.renderTarget[i].destinationBlend         = cgBlendMode::SrcAlpha;

    } // Next target
    bSuccess &= pResources->createBlendState( &mStates.multiplyDestBlendState, blStates, 0, cgDebugSource() );

    // All states created successfully?
    if ( !bSuccess )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to generate required light source blend states for scene '%s'.\n"), mParentScene->getName().c_str() );
        return false;

    } // End if failed

    ///////////////////////////////////////////////////////////////
    // Samplers
    ///////////////////////////////////////////////////////////////
    mStates.harmonicsRedSampler             = pResources->createSampler( _T("SHR"), mStates.lightingShader );
    mStates.harmonicsGreenSampler           = pResources->createSampler( _T("SHG"), mStates.lightingShader );
    mStates.harmonicsBlueSampler            = pResources->createSampler( _T("SHB"), mStates.lightingShader );
    mStates.harmonicsAuxiliarySampler       = pResources->createSampler( _T("SHA"), mStates.lightingShader );
    mStates.harmonicsRedPointSampler        = pResources->createSampler( _T("SHRPoint"), mStates.lightingShader );
    mStates.harmonicsGreenPointSampler      = pResources->createSampler( _T("SHGPoint"), mStates.lightingShader );
    mStates.harmonicsBluePointSampler       = pResources->createSampler( _T("SHBPoint"), mStates.lightingShader );
    mStates.harmonicsAuxiliaryPointSampler  = pResources->createSampler( _T("SHAPoint"), mStates.lightingShader );
    mStates.harmonicsRedVerticalSampler     = pResources->createSampler( _T("SHRVertical"), mStates.lightingShader );
    mStates.harmonicsGreenVerticalSampler   = pResources->createSampler( _T("SHGVertical"), mStates.lightingShader );
    mStates.harmonicsBlueVerticalSampler    = pResources->createSampler( _T("SHBVertical"), mStates.lightingShader );
    mStates.harmonicsRedHorizontalSampler   = pResources->createSampler( _T("SHRHorizontal"), mStates.lightingShader );
    mStates.harmonicsGreenHorizontalSampler = pResources->createSampler( _T("SHGHorizontal"), mStates.lightingShader );
    mStates.harmonicsBlueHorizontalSampler  = pResources->createSampler( _T("SHBHorizontal"), mStates.lightingShader );
    mStates.depthLowResSampler              = pResources->createSampler( _T("DepthLowRes"), mStates.lightingShader );
    mStates.lightingSampler                 = pResources->createSampler( _T("Lighting"), mStates.lightingShader );

    ///////////////////////////////////////////////////////////////
    // Sampler States
    ///////////////////////////////////////////////////////////////

    // Linear sampling with border
    cgSamplerStateDesc smpStates;
    smpStates.minificationFilter = cgFilterMethod::Linear;
    smpStates.magnificationFilter = cgFilterMethod::Linear;		
    smpStates.mipmapFilter = cgFilterMethod::None;
    smpStates.addressU  = cgAddressingMode::Border;
    smpStates.addressV  = cgAddressingMode::Border;
    smpStates.addressW  = cgAddressingMode::Border;
    smpStates.borderColor = cgColorValue( 0, 0, 0, 0 );
    mStates.harmonicsRedSampler->setStates( smpStates );
    mStates.harmonicsGreenSampler->setStates( smpStates );
    mStates.harmonicsBlueSampler->setStates( smpStates );
    mStates.harmonicsAuxiliarySampler->setStates( smpStates );
    mStates.lightingSampler->setStates( smpStates );

    // Point sampling with border
    smpStates.minificationFilter = cgFilterMethod::Point;
    smpStates.magnificationFilter = cgFilterMethod::Point;		
    mStates.harmonicsRedPointSampler->setStates( smpStates );
    mStates.harmonicsGreenPointSampler->setStates( smpStates );
    mStates.harmonicsBluePointSampler->setStates( smpStates );
    mStates.harmonicsAuxiliaryPointSampler->setStates( smpStates );
    mStates.harmonicsRedVerticalSampler->setStates( smpStates );
    mStates.harmonicsGreenVerticalSampler->setStates( smpStates );
    mStates.harmonicsBlueVerticalSampler->setStates( smpStates );
    mStates.harmonicsRedHorizontalSampler->setStates( smpStates );
    mStates.harmonicsGreenHorizontalSampler->setStates( smpStates );
    mStates.harmonicsBlueHorizontalSampler->setStates( smpStates );

    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getShadowMaps ()
/// <summary>
/// Retrieve the shadow map pool container.
/// </summary>
//-----------------------------------------------------------------------------
cgTexturePool * cgLightingManager::getShadowMaps( ) const
{
    return mShadowMapPool;
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Simply retrieve the render driver through which items will be rendered.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgLightingManager::getRenderDriver( ) const
{
    // Scenes always currently use the main singleton render driver.
    return cgRenderDriver::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : getResourceManager ()
/// <summary>
/// Simply retrieve the resource manager that will manage resources.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager * cgLightingManager::getResourceManager( ) const
{
    // Scenes always currently use the main singleton resource manager.
    return cgResourceManager::getInstance();
}

//-----------------------------------------------------------------------------
//  Name : getParentScene ( )
/// <summary>
/// Retrieve the scene to which this manager belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgLightingManager::getParentScene( ) const
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : processShadowMaps ()
/// <summary>
/// Automatically process light sources in order to prepare their
/// relevant shadow maps for rendering. This can be performed manually,
/// but this helper function is provided to aid in script development.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::processShadowMaps( cgCameraNode * pCamera, bool bReassignMaps, const cgString & strRenderCallback )
{
    cgVisibilitySet   * pSet          = pCamera->getVisibilitySet();
    cgObjectNodeArray & VisibleLights = pSet->getVisibleLights();
    
    // Validate requirements
    if ( !mScriptObject )
        return;

    // Record profiler data.
    cgProfiler * pProfiler = cgProfiler::getInstance();
    pProfiler->beginProcess( _T("Shadow Map Population") );

    // TODO: 6767 - Confirm that this is a good place for this.
    // Update list of objects that need to cast and/or receive shadows 
    // based on both the light's and camera's visibility sets.
    for ( size_t i = 0; i < VisibleLights.size(); ++i )
    {
        cgLightNode * pLight = (cgLightNode*)VisibleLights[i];
        pLight->computeShadowSets( pCamera );
    
    } // Next visible light

    // Are we reassigning resources?
    if ( bReassignMaps )
    {
        // All pooled resources of a given type should be made available.
        mShadowMapPool->resetAvailability( cgShadowGeneratorType::ShadowMap );

        // Attempt to re-assign the same shadow maps as last frame.
        for ( size_t i = 0; i < VisibleLights.size(); ++i )
        {
            cgLightNode * pLight = (cgLightNode*)VisibleLights[i];
            if ( pLight->isShadowSource() )
                pLight->reassignShadowMaps( mShadowMapPool );

        } // Next visible light

    } // End if perform reassignment

    // If no render callback was provided, the caller does not intend to fill right now
    if ( strRenderCallback.empty() )
        return;

    // Fill shadow map resources.
    for ( size_t i = 0; i < VisibleLights.size(); ++i )
    {
        // Skip lights that are not a shadow source.
        cgLightNode * pLight = (cgLightNode*)VisibleLights[i];
        if ( !pLight->isShadowSource() )
            continue;

        // Begin the fill process.
        cgInt32 nPass, nPassCount;
        if ( (nPassCount = pLight->beginShadowFill( mShadowMapPool )) >= 0 )
        {
            // ToDo: 6767 - This can be done outside the loop
            // Precompute script callback argument list.
            cgScriptArgument::Array ScriptArgs;
            static const cgString strContext = _T("FillShadowMap");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("LightNode@+"), pLight ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("VisibilitySet@+"), CG_NULL ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord,   _T("uint"), &nPass ) );

            // Process for the required number shadow map passes.
            for ( nPass = 0; nPass < nPassCount; ++nPass )
            {
                // Begin the pass. Note: this function returns a pointer to the final visibility set
                // we should use as the basis for rendering any appropriate objects.
                cgVisibilitySet * pRenderVis;
                if ( pLight->beginShadowFillPass( nPass, pRenderVis ) )
                {
                    // Notify the script that it should render.
                    try
                    {
                        // Set any parameters that are still outstanding.
                        ScriptArgs[2].data = pRenderVis;

                        // Execute the specified script function.
                        mScriptObject->executeMethodVoid( strRenderCallback, ScriptArgs );

                    } // End try to execute
                    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                    {
                        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strRenderCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );

                    } // End catch exception

                    // We're done.
                    pLight->endShadowFillPass( );

                } // End if beginShadowFillPass()

            } // Next pass

            // Finish up shadow map fill.
            pLight->endShadowFill( );

        } // End if beginShadowFill()

    } // Next Light

    // Complete profiler data.
    pProfiler->endProcess( );
}

//-----------------------------------------------------------------------------
//  Name : updateIndirectMaps () (Protected)
/// <summary>
/// Handles reassignment and filling for reflective shadow mapping. 
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::updateIndirectMaps( const cgObjectNodeArray & Lights, cgCameraNode * pCamera, bool bReassignMaps, cgUInt32 nResetAvailabilityFilter, const cgString & strRenderCallback )
{
    // Validate requirements
    if ( !mScriptObject )
        return;

    // Record profiler data.
    cgProfiler * pProfiler = cgProfiler::getInstance();
    pProfiler->beginProcess( _T("RSM Population") );
	    
    // Are we reassigning resources?
    if ( bReassignMaps )
    {
        // All pooled resources of a given type should be made available.
		mShadowMapPool->resetAvailability( nResetAvailabilityFilter );

        // Attempt to re-assign the same shadow maps as last frame.
        for ( size_t i = 0; i < Lights.size(); ++i )
            ((cgLightNode*)Lights[ i ])->reassignIndirectMaps( mShadowMapPool );

    } // End if perform reassignment

	// If no render callback was provided, the caller does not intend to fill right now
	if ( strRenderCallback.empty() )
		return;

    // Fill shadow map resources.
    cgInt32 nPass, nPassCount;
    for ( size_t i = 0; i < Lights.size(); ++i )
    {
        cgLightNode * pLight = (cgLightNode*)Lights[i];

		// Begin the fill process.
	    if ( (nPassCount = pLight->beginIndirectFill( mShadowMapPool )) >= 0 )
        {
			// ToDo: 6767 - Most of this can be done outside of the loop
            // Prebuild script callback argument list.
            cgScriptArgument::Array ScriptArgs;
            static const cgString strContext = _T("FillReflectiveShadowMap");
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("LightNode@+"), pLight ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("VisibilitySet@+"), CG_NULL ) );
            ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord,   _T("uint"), &nPass ) );

            // Process for the required number shadow map passes.
            for ( nPass = 0; nPass < nPassCount; ++nPass )
            {
				// Begin the pass. Note: this function returns a pointer to the final visibility set
                // we should use as the basis for rendering any appropriate objects.
                cgVisibilitySet * pRenderVis = CG_NULL;
                if ( pLight->beginIndirectFillPass( nPass, pRenderVis ) )
                {
					// Notify the script that it should render.
                    try
                    {
                        // Set any parameters that are still outstanding.
                        ScriptArgs[2].data = pRenderVis;

                        // Execute the specified script function.
                        mScriptObject->executeMethodVoid( strRenderCallback, ScriptArgs );
                    
                    } // End try to execute
                    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                    {
                        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strRenderCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
                    
                    } // End catch exception

                    // We're done.
                    pLight->endIndirectFillPass( );

                } // End if beginIndirectFillPass()

            } // Next pass

            // Finish up fill.
            pLight->endIndirectFill( );

        } // End if beginIndirectFill()

    } // Next Light

    // Complete profiler data.
    pProfiler->endProcess( );
}

//-----------------------------------------------------------------------------
//  Name : processLight ()
/// <summary>
/// Runs processing for a single light source (either deferred or forward). 
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::processLight( cgLightNode * pLight, cgCameraNode * pCamera, bool bDeferred, bool bApplyShadows, const cgString & strLightCallback )
{
    // Compute an illumination set for this light source (forward rendering only)
    //if ( !bDeferred )
        //pLight->ComputeIlluminationSets( pCamera );

    // Begin the lighting process.
    cgInt32 nPass, nPassCount;
    if ( (nPassCount = pLight->beginLighting( mShadowMapPool, (bApplyShadows && pLight->isShadowSource()), bDeferred ) ) >= 0 )
    {
        // Prebuild script callback argument list.
        cgString strContext;
        cgScriptArgument::Array ScriptArgs;
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"),  &strContext ) );
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("LightNode@+"),     pLight ) );
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Object,  _T("VisibilitySet@+"), CG_NULL ) );
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord,   _T("uint"),           &nPass ) );

        // Process requested number of lighting passes.
        cgVisibilitySet * pRenderVis;
        for ( nPass = 0; nPass < nPassCount; ++nPass )
        {
            // Begin the pass. Note: this function returns a pointer to the final visibility set
            // we should use as the basis for rendering any appropriate objects.
            cgLightNode::LightingOp Op = pLight->beginLightingPass( nPass, pRenderVis );
            if ( Op == cgLightNode::Lighting_FillShadowMap )
            {
                // Notify the script that it should render.
                try
                {
                    // Set any parameters that are still outstanding.
                    strContext = _T("FillShadowMap");
                    ScriptArgs[2].data = pRenderVis;

                    // Execute the specified script function.
                    mScriptObject->executeMethodVoid( strLightCallback, ScriptArgs );
                
                } // End try to execute
                catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strLightCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
                
                } // End catch exception

            } // End if FillShadowMap
            else if ( Op == cgLightNode::Lighting_ProcessLight )
            {
                // Notify the script that it should render.
                try
                {
                    // Set any parameters that are still outstanding.
                    strContext = _T("processLight");
                    ScriptArgs[2].data = pRenderVis;

                    // Execute the specified script function.
                    mScriptObject->executeMethodVoid( strLightCallback, ScriptArgs );
                
                } // End try to execute
                catch ( cgScriptInterop::Exceptions::ExecuteException & e )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strLightCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
                
                } // End catch exception

            } // End if processLight
            else if ( Op == cgLightNode::Lighting_Abort )
            {
                continue;
            
            } // End if Abort
            
            // We've finished this lighting pass
            pLight->endLightingPass();

        } // Next pass

        // We're done
        pLight->endLighting( );

    } // End if beginLighting()

}

//-----------------------------------------------------------------------------
//  Name : processLights ()
/// <summary>
/// Automatically process light sources in order to light the scene
/// (either deferred or forward). This can be performed manually,
/// but this helper function is provided to aid in script development.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::processLights( cgCameraNode * pCamera, bool bDeferred, bool bApplyShadows, bool bViewSpace, const cgString & strLightCallback, const cgString & strProcessCallback )
{
    processLights( cgRenderTargetHandle::Null, pCamera, bDeferred, bApplyShadows, bViewSpace, strLightCallback, strProcessCallback );
}
void cgLightingManager::processLights( const cgRenderTargetHandle& hLightingBuffer, cgCameraNode * pCamera, bool bDeferred, bool bApplyShadows, bool bViewSpace, const cgString & strLightCallback, const cgString & strProcessCallback )
{
    // Validate requirements
    if ( !mScriptObject )
        return;

    // Get access to required systems.
    cgRenderDriver * pDriver = getRenderDriver();
    cgProfiler * pProfiler = cgProfiler::getInstance();

    // Record profiler data.
    pProfiler->beginProcess( _T("Lights") );

    // Update top-level system states.
    pDriver->setSystemState( cgSystemState::ViewSpaceLighting, bViewSpace ? 1 : 0 );

    // Precompute script callback argument list.
    cgString strContext;
    cgScriptArgument::Array ScriptArgs;
    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );

    // If a valid lighting buffer was not provided, we'll assume the caller already set one
    bool bUseLightingBuffer = hLightingBuffer.isValid();

    // Our first job is to categorize lights into appropriate sets such
    // as dynamic, static, etc. This allows us to process the required light
    // types in order.
    cgVisibilitySet * pSet = pCamera->getVisibilitySet();
    cgObjectNodeArray & VisibleLights = pSet->getVisibleLights();
    std::list<cgLightNode*> ShadowCastingLights, NonShadowCastingLights;
	for ( size_t i = 0; i < VisibleLights.size(); ++i )
	{
		// Check the current light's stage settings
        cgLightNode * pLight = (cgLightNode*)VisibleLights[i];
		cgSceneProcessStage::Base LightStage  = pLight->getLightingStage();
		cgSceneProcessStage::Base ShadowStage = pLight->getShadowStage();

		// Skip lights that precomputed all of their lighting, disabled lights and
        // dynamic lights that have a precomputed shadow element (not possible)
		if (  LightStage == cgSceneProcessStage::Precomputed ||
              LightStage == cgSceneProcessStage::None ||
             (LightStage == cgSceneProcessStage::Runtime && ShadowStage == cgSceneProcessStage::Precomputed) )
			continue;

        // Record which of the light sources are potential shadow sources.
	    if ( !bApplyShadows || !pLight->isShadowSource() )
        	NonShadowCastingLights.push_back( pLight );
        else
            ShadowCastingLights.push_back( pLight );

	} // Next Light

    // Call out to the script to allow it to set up any lighting inputs.
    if ( !strProcessCallback.empty() )
    {
        // Execute the script function to setup lighting buffers.   
        try
        {
            strContext = _T("SetupLightingInputs");
            mScriptObject->executeMethodVoid( strProcessCallback, ScriptArgs );
        
        } // End try to execute
        catch ( cgScriptInterop::Exceptions::ExecuteException & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strProcessCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
        
        } // End catch exception
    
    } // End if no buffer lights

    // If we are controlling lighting buffer target setting, set it now
    bool bRunLighting = false;
    if ( bUseLightingBuffer )
    {
        if ( pDriver->beginTargetRender( hLightingBuffer ) )
            bRunLighting = true; 
    
    } // End if use buffer
    else
    {
        // Assume caller successfully set the lighting target(s)
        bRunLighting = true;
    
    } // End if !use buffer

    // Should lighting proceed?
    if ( bRunLighting )
    {
        // Process shadow casting light sources.
        std::list<cgLightNode*>::iterator itLight;
        for ( itLight = ShadowCastingLights.begin(); itLight != ShadowCastingLights.end(); ++itLight )
            processLight( *itLight, pCamera, bDeferred, bApplyShadows, strLightCallback );

        // Process non-shadow casting light sources.
        for ( itLight = NonShadowCastingLights.begin(); itLight != NonShadowCastingLights.end(); ++itLight )
            processLight( *itLight, pCamera, bDeferred, false, strLightCallback );

        // If we set the lighting buffer above, reset the render target
        if ( bUseLightingBuffer )
            pDriver->endTargetRender();
    
    } // End if process lighting

    // Execute the script function to cleanup after lighting is complete.
    try
    {
        strContext = _T("CleanupLighting");
        mScriptObject->executeMethodVoid( strProcessCallback, ScriptArgs );
    
    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strProcessCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
    
    } // End catch exception

    // Complete profiler data.
    pProfiler->endProcess( );
}

//-----------------------------------------------------------------------------
// Name : setConstant() 
/// <summary>
/// Sets the value of a constant for placement in the constant buffer
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::setConstant( const cgString & strName, const void * pValue )
{
    // Cache the request for view space lighting and let the system know as well
    bool bViewSpaceLighting = mParentScene->getRenderDriver()->getSystemState( cgSystemState::ViewSpaceLighting ) > 0;

    // If we are doing view space lighting, transform the values as needed
    cgMatrix viewMatrix, invViewMatrix;
    if ( bViewSpaceLighting )
    {
        cgCameraNode * pCamera = pCamera = getRenderDriver()->getCamera();
        if ( pCamera != CG_NULL )
            viewMatrix = pCamera->getViewMatrix();
        else
            cgMatrix::identity( viewMatrix );

    } // End if view space lighting is active

    static const cgString strPosition              = _T("_lightPosition");
    static const cgString strDirection             = _T("_lightDirection");
    static const cgString strAttenuationBufferMask = _T("_lightAttenuationBufferMask");
    static const cgString strTexProjMatrix         = _T("_lightTexProjMatrix");
    if ( strName == strPosition )
    {
        mConstants.position = *((cgVector3*)pValue);
        if ( bViewSpaceLighting )
            cgVector3::transformCoord( mConstants.position, mConstants.position, viewMatrix );
    
    } // End if _lightPosition
    else if ( strName == strDirection )
    {
        mConstants.direction = *((cgVector3*)pValue);
        if ( bViewSpaceLighting )
            cgVector3::transformNormal( mConstants.direction, mConstants.direction, viewMatrix );
    
    } // End if _lightDirection
    else if ( strName == strAttenuationBufferMask )
    {
        mConstants.attenuationBufferMask = *((cgVector4*)pValue);
    
    } // End if _lightAttenuationBufferMask
    else if ( strName == strTexProjMatrix )
    {
        mConstants.textureProjMatrix = *((cgMatrix*)pValue);
        if ( bViewSpaceLighting )
        {
            cgMatrix::inverse( invViewMatrix, viewMatrix );
            mConstants.textureProjMatrix = invViewMatrix * mConstants.textureProjMatrix;
        
        } // End if view space
    
    } // End if _lightTexProjMatrix
    
    // Unknown constant
    return false;
}

//-----------------------------------------------------------------------------
// Name : applyLightingConstants () 
/// <summary>
/// Sets the constant buffer
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::applyLightingConstants( )
{ 
    // Get access to the render driver 
    cgRenderDriver * pDriver = getRenderDriver();

    // Get the constant buffer pointer.
    cgConstantBuffer * pSystemBuffer = mStates.lightingSystemConstants.getResource( true );
    cgAssert( pSystemBuffer != CG_NULL );

    // Lock the buffer ready for population
    _cbLightingSystem * pSystemData = CG_NULL;
    if ( !(pSystemData = (_cbLightingSystem*)pSystemBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock lighting system constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // If we are not doing view space lighting, just copy the buffer
    memcpy( pSystemData, &mConstants, sizeof( _cbLightingSystem ) );

    // Unlock the buffer. If it is currently bound to the device
    // then the appropriate constants will be automatically updated
    // next time 'DrawPrimitive*' is called.
    pSystemBuffer->unlock();

    // Set the constant buffer
    return pDriver->setConstantBufferAuto( mStates.lightingSystemConstants );
}

//-----------------------------------------------------------------------------
// Name : addRadianceGrid ()
// Desc : Adds a new grid to the system
//-----------------------------------------------------------------------------
// ToDo: 6767 - Dimensions is a float vector3 -- should we introduce a cgSize3 here for integer dimensions?
bool cgLightingManager::addRadianceGrid( float fCellSize, const cgVector3 & vDimensions, cgUInt32 nNumPropagations, cgUInt32 nPadding, cgFloat fLightStrength, cgFloat fBlendRegion )
{
    // Get access to required systems.
    cgRenderDriver    * pDriver    = getRenderDriver();
    cgResourceManager * pResources = getResourceManager();

    // Create a new grid
    cgRadianceGrid * pGrid = new cgRadianceGrid( this, mRadianceGrids.size() );
    if ( !pGrid->initialize( pDriver, mIndirectMethod, vDimensions, fCellSize, nPadding, fLightStrength, fBlendRegion ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize radiance grid of dimensions %ix%ix%i.\n"), (cgInt)vDimensions.x, (cgInt)vDimensions.y, (cgInt)vDimensions.z );
        return false;
    
    } // End if failed

    // If we don't already have a scratch target of these dimensions, create one
    if ( !getScratchBuffer( vDimensions ) )
    {
        // Create a new scratch buffer
        cgRadianceBuffer * pBuffer = new cgRadianceBuffer();
        pBuffer->initialize( pDriver, vDimensions );
        mScratchRadianceBuffers0.push_back( pBuffer );

        // If we are doing LPV, we need an extra buffer for propagation
        if ( pGrid->mMethod == cgIndirectLightingMethod::PropagationVolumes )
        {
            pBuffer = new cgRadianceBuffer();
            pBuffer->initialize( pDriver, vDimensions );
            mScratchRadianceBuffers1.push_back( pBuffer );
        
        } // End if LPV
    
    } // End if no buffer

    // Add the grid
    mRadianceGrids.push_back( pGrid );

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
// Name : getRadianceGrid ()
// Desc : Retrieve a previously created radiance grid.
//-----------------------------------------------------------------------------
cgRadianceGrid * cgLightingManager::getRadianceGrid( cgUInt32 nIndex ) const
{
    return mRadianceGrids[ nIndex ];
}

//-----------------------------------------------------------------------------
// Name : getScratchBuffer () (Protected)
// Desc : Returns a "scratch" radiance buffer matching the input dimensions
//-----------------------------------------------------------------------------
cgRadianceBuffer * cgLightingManager::getScratchBuffer( const cgVector3 & vDimensions )
{
    for ( size_t i = 0; i < mScratchRadianceBuffers0.size(); ++i )
    {
        if ( mScratchRadianceBuffers0[ i ]->mDimensions.x == vDimensions.x && 
             mScratchRadianceBuffers0[ i ]->mDimensions.y == vDimensions.y && 
             mScratchRadianceBuffers0[ i ]->mDimensions.z == vDimensions.z )
            return mScratchRadianceBuffers0[ i ];
    
    } // Next existing buffer
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : getScratchBuffers () (Protected)
// Desc : Returns two "scratch" radiance buffers matching the input dimensions
//        Useful for light propagation through grids.
//-----------------------------------------------------------------------------
bool cgLightingManager::getScratchBuffers( const cgVector3 & vDimensions, cgRadianceBuffer *& pBuffer0, cgRadianceBuffer *& pBuffer1 )
{
    // Be polite and clear output variables.
    pBuffer0 = CG_NULL;
    pBuffer1 = CG_NULL;

    // Check for first buffer
    for ( size_t i = 0; i < mScratchRadianceBuffers0.size(); ++i )
    {
        if ( mScratchRadianceBuffers0[ i ]->mDimensions.x == vDimensions.x && 
             mScratchRadianceBuffers0[ i ]->mDimensions.y == vDimensions.y && 
             mScratchRadianceBuffers0[ i ]->mDimensions.z == vDimensions.z )
            pBuffer0 = mScratchRadianceBuffers0[ i ];
    
    } // Next buffer

    // Check for second buffer
    for ( size_t i = 0; i < mScratchRadianceBuffers1.size(); ++i )
    {
        if ( mScratchRadianceBuffers1[ i ]->mDimensions.x == vDimensions.x && 
             mScratchRadianceBuffers1[ i ]->mDimensions.y == vDimensions.y && 
             mScratchRadianceBuffers1[ i ]->mDimensions.z == vDimensions.z )
            pBuffer1 = mScratchRadianceBuffers1[ i ];
    
    } // Next buffer

    // Return success if both were found
    return ( pBuffer0 != CG_NULL && pBuffer1 != CG_NULL );
}

//-----------------------------------------------------------------------------
// Name : replaceScratchBuffer () (Protected)
// Desc : Swaps out the old radiance buffer in favor of the new one provided
// Note : This is done for easy ping-ponging, etc.
//-----------------------------------------------------------------------------
void cgLightingManager::replaceScratchBuffer( cgRadianceBuffer * pOldBuffer, cgRadianceBuffer * pNewBuffer )
{
    // Check list 0
    for ( size_t i = 0; i < mScratchRadianceBuffers0.size(); ++i )
    {
        if ( mScratchRadianceBuffers0[ i ] == pOldBuffer )
        {
            mScratchRadianceBuffers0[ i ] = pNewBuffer;
            return;
        
        } // End if in first list
    
    } // Next buffer

    // Check list 1
    for ( size_t i = 0; i < mScratchRadianceBuffers1.size(); ++i )
    {
        if ( mScratchRadianceBuffers1[ i ] == pOldBuffer )
        {
            mScratchRadianceBuffers1[ i ] = pNewBuffer;
            break;
        
        } // End if in second list
    
    } // Next buffer
}

//-----------------------------------------------------------------------------
//  Name : processIndirectLights ()
/// <summary>
/// Computes indirect lighting using (ir)radiance grids.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::processIndirectLights( const cgRenderTargetHandle& hLightingBuffer, const cgRenderTargetHandle& hLightingBufferLow, const cgRenderTargetHandle& hDepthBufferLow, const cgDepthStencilTargetHandle& hLowResDS, const cgDepthStencilTargetHandle& hHighResDS, cgCameraNode * pCamera, const cgString & strLightCallback, const cgString & strProcessCallback )
{
    // Validate requirements
    if ( !mScriptObject || mRadianceGrids.empty() )
        return;

    // Cache the input arguments
    mIndirectLightCallback        = strLightCallback;
    mProcessCallback              = strProcessCallback;
    mStates.depthStencilBufferLow   = hLowResDS;
    mStates.depthStencilBufferHigh  = hHighResDS;
    mStates.currentLightingTarget   = hLightingBuffer;
    mStates.depthTargetLow          = hDepthBufferLow;
    mStates.lightingTargetLow       = hLightingBufferLow;

    // Process tasks for this frame
    processTasks( );

    // Compute lighting
    computeIndirectLighting( pCamera, mProcessCallback );
    //computeIndirectLightingLow( pCamera, mProcessCallback );

    // Optionally run grid debugging
    bool m_bDebugGrids = true;
    if ( m_bDebugGrids )
        debugGrids( pCamera );
}

//-----------------------------------------------------------------------------
// Name : update()
// Desc : Updates the lighting manager
//-----------------------------------------------------------------------------
bool cgLightingManager::update( cgCameraNode * pCamera )
{
    // Collect all indirect lights for this frame
    collectIndirectLights( pCamera );

    // Update global illumination grids based on camera motion
    if ( !mRadianceGrids.empty() )
        updateGrids( pCamera );

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : collectIndirectLights () (Protected)
// Desc : Builds the list of indirect lights to be injected into our radiance grids
//-----------------------------------------------------------------------------
void cgLightingManager::collectIndirectLights( cgCameraNode * pCamera )
{
    // ToDo: 6767 - This is not going to work if this scene has been created
    // long after application startup (likely). Come up with a better system!

    // Get the current frame index. We use this simply to prevent
    // the update of dynamic lights for a few frames after application startup.
    cgUInt32 nCurrentFrame = cgTimer::getInstance()->getFrameCounter();

    // Clear the lists
    mStaticIndirectLights.clear();
    mDynamicIndirectLights.clear();

    // Our first job is to identify which lights we intend to process. 
    // ToDo 9999: For now we are processing all light from the scene.
    //             Moving forward, the area of interest is probably a better bet here.
    //cgObjectNodeSet & ObjectNodes = mParentScene->GetOrphanNodes();
    cgObjectNodeMap::const_iterator itNode;
    const cgObjectNodeMap & ObjectNodes = mParentScene->getObjectNodes();
    for ( itNode = ObjectNodes.begin(); itNode != ObjectNodes.end(); ++itNode )
    {
        // Grab only light sources
        if ( !itNode->second->queryReferenceType( RTID_LightNode ) )
            continue;
        
        ////////////////////////////////////////////////
        // Note: For the moment, make sure we force a visibility set
        // so that the indirect lighting has something to work with. Moving forward
        // this can be a broad phase query for at least 1 light reflecting
        // object (i.e., an "illumination" set) and actual visibility
        // will be handled by the queued render process
        cgLightNode * pLight = (cgLightNode*)itNode->second;
        // ToDo: 6767 - Not compatible with new queue system.
        /*pLight->ComputeIndirectSets( pCamera );
        if ( pLight->GetIndirectLightingSet()->IsEmpty() )
            continue;*/
        ////////////////////////////////////////////////

        // Update any indirect lighting settings for this light
        pLight->setIndirectLightingMethod( mIndirectMethod );
        pLight->updateIndirectSettings();

        // Allow a few frames to pass before testing for dynamic lights (reduces popping at startup)
        bool bIsDynamic = false;
        if ( nCurrentFrame > 10 )
        {
            // If the last light update is within our time threshold, it is dynamic.
            cgUInt32 nElapsedFrames = nCurrentFrame - pLight->getLastDirtyFrame();
            cgUInt32 nFrameRate     = cgTimer::getInstance()->getFrameRate();
            cgFloat  fApproxSecs    = (cgFloat)nElapsedFrames / (cgFloat)nFrameRate;
            if ( fApproxSecs < mDynamicTimeThreshold )
                bIsDynamic = true;
        
        } // End if !startup

        // Add light to appropriate list
        if ( bIsDynamic )
            mDynamicIndirectLights.push_back( pLight );
        else
            mStaticIndirectLights.push_back( pLight );

    } // Next Light

}

//-----------------------------------------------------------------------------
// Name : updateGrids() (Protected)
// Desc : Computes data needed for grid updates
//-----------------------------------------------------------------------------
void cgLightingManager::updateGrids( cgCameraNode * pCamera )
{
    // Update grid spatial data (coarsest to finest) 
    cgInt nStartGrid = (cgInt)mRadianceGrids.size() - 1;
    for ( cgInt i = nStartGrid; i >= 0; --i )
    {
        // Compute the new grid world space data
        mRadianceGrids[ i ]->update( pCamera, 0, 0 );

        // Add lights to grids
        mRadianceGrids[ i ]->addLights( mStaticIndirectLights, mDynamicIndirectLights );

    } // Next grid
}

//-----------------------------------------------------------------------------
// Name : processTasks () (Protected)
// Desc : Runs all tasks needed for this frame
//-----------------------------------------------------------------------------
void cgLightingManager::processTasks( )
{
    // ToDo: 6767 - Can we not just update / fill as we go rather than building intermediate arrays of lights?
    cgObjectNodeArray ReassignLights;
    cgObjectNodeArray FillLights;
    
    // Get the current frame
    cgUInt32 nCurrentFrame = cgTimer::getInstance()->getFrameCounter();

    // Iterate the grids and process any tasks that are meant for us
    for ( size_t i = 0; i < mRadianceGrids.size(); ++i )
    {
        cgRadianceGrid::TaskList & Tasks = mRadianceGrids[ i ]->getTasks();
        cgRadianceGrid::TaskList::iterator itTask = Tasks.begin();
        for ( ; itTask != Tasks.end(); )
        {
            cgIndirectLightingTask & Task = *itTask;

            // Is this a task for this frame?
            if ( Task.frame != nCurrentFrame )
            {
                ++itTask;
                continue;
            
            } // End if !current

            // What task are we to process?
            if ( Task.type == cgIndirectLightingTaskType::ReassignRSMs )
            {
                // RSM Reassignment
                // Extract the lights and add to table
                const cgObjectNodeArray & Lights = mRadianceGrids[ i ]->getLights( Task.staticUpdate );
                ReassignLights.insert( ReassignLights.end(), Lights.begin(), Lights.end() );
                
                // Remove the task
                Tasks.erase( itTask++ );
            
            } // End if ReassignRSMs
            else if ( Task.type == cgIndirectLightingTaskType::FillRSMs )
            {
                // RSM Filling
                // Extract the lights and add to table
                const cgObjectNodeArray & Lights = mRadianceGrids[ i ]->getLights( Task.staticUpdate );
                FillLights.insert( FillLights.end(), Lights.begin(), Lights.end() );
                
                // Remove the task
                Tasks.erase( itTask++ );
            
            } // End if FillRSMs
            else
            {
                // Not a task we need to process. Leave alone.
                ++itTask;
            
            } // End if unknown

        } // Next task

    } // Next grid

    // Do we have reassignments to do?
    if ( !ReassignLights.empty() )
        updateIndirectMaps( ReassignLights, CG_NULL, true, cgShadowGeneratorType::ReflectiveShadowMap, _T("") );

    // Do we have pool fills to do?
    if ( !FillLights.empty() )
    {
        cgRenderDriver * pDriver = getRenderDriver();

        // Disable HDR and use world space lighting
        cgInt32 nPrevTransformState = pDriver->getSystemState( cgSystemState::ViewSpaceLighting );
        cgInt32 nPrevHDRState       = pDriver->getSystemState( cgSystemState::HDRLighting );
        pDriver->setSystemState( cgSystemState::ViewSpaceLighting, 0 );
        pDriver->setSystemState( cgSystemState::HDRLighting, 0 );

        // Compute shadow maps
        updateIndirectMaps( FillLights, CG_NULL, false, 0xFFFFFFFF, mIndirectLightCallback );

        // Restore prior state
        pDriver->setSystemState( cgSystemState::ViewSpaceLighting, nPrevTransformState );
        pDriver->setSystemState( cgSystemState::HDRLighting, nPrevHDRState );
    
    } // End if found fill lights

    // Run any grid level tasks 
    for ( size_t i = 0; i < mRadianceGrids.size(); ++i )
        mRadianceGrids[ i ]->processTasks( );
}

//-----------------------------------------------------------------------------
// Name : computeIndirectLighting () (Protected)
// Desc : Runs the final indirect lighting computations for scenery
//-----------------------------------------------------------------------------
void cgLightingManager::computeIndirectLighting( cgCameraNode * pCamera, const cgString & strProcessCallback )
{
    // Get access to required systems.
    cgRenderDriver * pDriver = getRenderDriver();

    // Setup script callback arguments (context in this case)
    cgScriptArgument::Array ScriptArgs;
    static const cgString strContext = _T("SetupLightingInputs");
    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );

    // Set the current lighting buffer
    if ( !pDriver->beginTargetRender( mStates.currentLightingTarget, 0, false, mStates.depthStencilBufferHigh ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set valid lighting buffer in computeIndirectLighting.\n") );
        return;

    } // End begin target

    // Set the stencil buffer to 1 
    pDriver->clear( cgClearFlags::Stencil, 0, 0, 1 );

    // Execute the script function to setup lighting buffers. Should also clear the alpha channel of the current buffer??
    try
    {
        mScriptObject->executeMethodVoid( strProcessCallback, ScriptArgs );

    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strProcessCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );

    } // End catch exception

    // Compute indirect lighting for all grids
    for ( size_t i = 0; i < mRadianceGrids.size(); ++i )
        mRadianceGrids[ i ]->composite( false );

    // Reclear the stencil buffer to 0 to prevent direct lighting issues
    pDriver->clear( cgClearFlags::Stencil, 0, 0, 0 );

    // Reset the render target
    pDriver->endTargetRender();
}

//-----------------------------------------------------------------------------
// Name : computeIndirectLightingLow () (Protected)
// Desc : Runs the final indirect lighting computations for scenery
//-----------------------------------------------------------------------------
void cgLightingManager::computeIndirectLightingLow( cgCameraNode * pCamera, const cgString & strProcessCallback )
{
    // Get access to required systems.
    cgRenderDriver  * pDriver = getRenderDriver();
    cgRenderView    * pView   = pDriver->getActiveRenderView();

    // Setup script callback arguments (context in this case)
    cgScriptArgument::Array ScriptArgs;
    static const cgString strContext = _T("SetupLowResIndirectInputs");
    ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );

    // ToDo: 6767 - Get best format??????
    // Retrieve low resolution lighting buffers from the current view
    cgRenderTargetHandleArray aTargets;
    cgRenderTargetHandle hSHR = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHR") );
    cgRenderTargetHandle hSHG = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHG") );
    cgRenderTargetHandle hSHB = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHB") );
    aTargets.push_back( hSHR );
    aTargets.push_back( hSHG );
    aTargets.push_back( hSHB );

    // Set the current low resolution lighting buffers and matching depth-stencil buffer
    if ( !pDriver->beginTargetRender( aTargets, false, mStates.depthStencilBufferLow ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set valid lighting buffer in computeIndirectLightingLow.\n") );
        return;

    } // End begin target

    // Clear targets to 0 and stencil buffer to 1 
    pDriver->clear( cgClearFlags::Target | cgClearFlags::Stencil, 0, 0, 1 );

    // Execute the script function to setup indirect lighting buffers (e.g.., low res depth and normal).
    try
    {
        mScriptObject->executeMethodVoid( strProcessCallback, ScriptArgs );

    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), strProcessCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );

    } // End catch exception

    // Compute indirect lighting for all grids
    for ( size_t i = 0; i < mRadianceGrids.size(); ++i )
        mRadianceGrids[ i ]->composite( true );

    // Fill the bilateral constants
    cgConstantBuffer * pFilterBuffer = mStates.bilateralConstants.getResource( true );
    cgAssert( pFilterBuffer != CG_NULL );
    _cbBilateral * pFilterData = (_cbBilateral*)pFilterBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    cgAssert( pFilterData != CG_NULL );

    cgTexture * pTexture = (cgTexture*)hSHR.getResource( false );
    pFilterData->textureSize.x  = (cgFloat)pTexture->getInfo().width;
    pFilterData->textureSize.y  = (cgFloat)pTexture->getInfo().height;
    pFilterData->textureSize.z  = 1.0f / pFilterData->textureSize.x;
    pFilterData->textureSize.w  = 1.0f / pFilterData->textureSize.y;
    pFilterData->depthScaleBias = cgVector2( 1.0f, 0.00001f );
    pFilterBuffer->unlock();
    pDriver->setConstantBufferAuto( mStates.bilateralConstants );

    // SSGI - Run a screen space indirect lighting pass for the remaining unprocessed pixels.
    bool bSSGI = false;
    if ( bSSGI )
    {
        // Set the low res lighting buffer for sampling
        mStates.lightingSampler->apply( mStates.lightingTargetLow );

        // Set states
        pDriver->setBlendState( cgBlendStateHandle::Null );
        pDriver->setRasterizerState( cgRasterizerStateHandle::Null );
        pDriver->setDepthStencilState( mStates.testStencilDepthState, 1 );

        // Set the "pass through" vertex shader
        cgSurfaceShader * pShader = mStates.lightingShader.getResource(true);
        if ( !pShader->selectVertexShader( _T("transformPassThrough") ) )
            return;
        if ( !pShader->selectPixelShader( _T("SSGI") ) )
            return;

        // Draw quad
        pDriver->drawClipQuad();	
    
    } // End if SSGI

    // Reset the render target
    pDriver->endTargetRender();

    // Low Resolution Blur
    bool bBlurSH = false;
    if ( bBlurSH )
        blurSphericalHarmonics( hSHR, hSHG, hSHB );

    // Upsample / Composite
    upsampleLighting( hSHR, hSHG, hSHB );
}

//-----------------------------------------------------------------------------
// Name : blurSphericalHarmonics () (Protected)
// Desc : Blurs (bilateral) the low resolution SH data
//-----------------------------------------------------------------------------
void cgLightingManager::blurSphericalHarmonics( cgRenderTargetHandle & hSHR, cgRenderTargetHandle & hSHG, cgRenderTargetHandle & hSHB )
{
    // Get access to required systems.
    cgRenderDriver  * pDriver = getRenderDriver();
    cgRenderView    * pView   = pDriver->getActiveRenderView();
    
    // Get scratch targets for blurring
    cgRenderTargetHandle hSHRScratch = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHRScratch") );
    cgRenderTargetHandle hSHGScratch = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHGScratch") );
    cgRenderTargetHandle hSHBScratch = pView->getRenderSurface( cgBufferFormat::R16G16B16A16_Float, 0.5, 0.5, _T("LowResSHBScratch") );

    // Build targets arrays
    cgRenderTargetHandleArray aTargets;
    aTargets.push_back( hSHR );
    aTargets.push_back( hSHG );
    aTargets.push_back( hSHB );

    cgRenderTargetHandleArray aScratchTargets;
    aScratchTargets.push_back( hSHRScratch );
    aScratchTargets.push_back( hSHGScratch );
    aScratchTargets.push_back( hSHBScratch );

    // Pre-bind textures to samplers
    mStates.harmonicsRedVerticalSampler->apply( hSHR );
    mStates.harmonicsGreenVerticalSampler->apply( hSHG );
    mStates.harmonicsBlueVerticalSampler->apply( hSHB );
    mStates.harmonicsRedHorizontalSampler->apply( hSHRScratch );
    mStates.harmonicsGreenHorizontalSampler->apply( hSHGScratch );
    mStates.harmonicsBlueHorizontalSampler->apply( hSHBScratch );

    // Build shader argument list
    cgScriptArgument::Array m_aBlurArgs;
    cgUInt32 nPixelRadius = 3;
    cgFloat  fFactor      = 3.0f;
    bool     bVertical    = true;
    cgUInt32 nDepthType   = cgDepthType::LinearZ_Packed;
    m_aBlurArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),   &nPixelRadius ) );
    m_aBlurArgs.push_back( cgScriptArgument( cgScriptArgumentType::Float, _T("float"), &fFactor ) );
    m_aBlurArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"),  &bVertical ) );
    m_aBlurArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),   &nDepthType ) );

    // Retrieve the two pixel shaders we will use (avoids permutation search per pass)
    cgSurfaceShader * pShader = mStates.lightingShader.getResource(true);
    bVertical = true;
    cgPixelShaderHandle hVerticalShader   = pShader->getPixelShader( _T("blurSH"), m_aBlurArgs );
    bVertical = false;
    cgPixelShaderHandle hHorizontalShader = pShader->getPixelShader( _T("blurSH"), m_aBlurArgs );

    // Set states
    pDriver->setBlendState( cgBlendStateHandle::Null );
    pDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    pDriver->setDepthStencilState( mStates.greaterDepthState, 0 );

    // Run the specified number of blur passes using ping-ponging
    cgUInt32 nNumBlurPasses = 2;
    for ( cgUInt32 i = 0; i < nNumBlurPasses; ++i )
    {
        // Run the vertical blur
        pShader->selectPixelShader( hVerticalShader );
        if ( pDriver->beginTargetRender( aScratchTargets, mStates.depthStencilBufferLow ) )
        {
            pDriver->drawClipQuad();	
            pDriver->endTargetRender();

            // Run the horizontal blur
            pShader->selectPixelShader( hHorizontalShader );
            if ( pDriver->beginTargetRender( aTargets, mStates.depthStencilBufferLow ) )
            {
                pDriver->drawClipQuad();
                pDriver->endTargetRender();
            
            } // End if begin

        } // End if begin

    } // Next pass
}

//-----------------------------------------------------------------------------
// Name : upsampleLighting () (Protected)
// Desc : Upsamples the low resolution spherical harmonics and composites
//-----------------------------------------------------------------------------
void cgLightingManager::upsampleLighting( cgRenderTargetHandle & hSHR, cgRenderTargetHandle & hSHG, cgRenderTargetHandle & hSHB )
{
    // Get access to required systems.
    cgRenderDriver  * pDriver = getRenderDriver();
    cgRenderView    * pView   = pDriver->getActiveRenderView();
    
    // Set the high resolution output buffer
    if ( !pDriver->beginTargetRender( mStates.currentLightingTarget, mStates.depthStencilBufferHigh ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set valid lighting buffer in upsampleLighting.\n") );
        return;

    } // End begin target

    // Execute the script function to setup standard lighting buffers
    try
    {
        static const cgString strContext = _T("SetupLightingInputs");
        cgScriptArgument::Array ScriptArgs;
        ScriptArgs.push_back( cgScriptArgument( cgScriptArgumentType::Address, _T("const String&"), &strContext ) );
        mScriptObject->executeMethodVoid( mProcessCallback, ScriptArgs );

    } // End try to execute
    catch ( cgScriptInterop::Exceptions::ExecuteException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to execute callback function %s() in '%s'. The engine reported the following error: %s.\n"), mProcessCallback.c_str(), e.getExceptionSource().c_str(), e.description.c_str() );
        pDriver->endTargetRender();

    } // End catch exception

    // Set blend/rasterizer states
    pDriver->setBlendState( mStates.additiveBlendState );
    pDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set the "pass through" vertex shader
    cgSurfaceShader * pShader = mStates.lightingShader.getResource(true);
    if ( !pShader->selectVertexShader( _T("transformPassThrough") ) )
    {
        pDriver->endTargetRender();
        return;
    
    } // End if failed

    // Bilinear upsample
    // Bind the radiance textures for input
    mStates.harmonicsRedSampler->apply( hSHR );
    mStates.harmonicsGreenSampler->apply( hSHG );
    mStates.harmonicsBlueSampler->apply( hSHB );

    // Set depth-stencil state
    pDriver->setDepthStencilState( mStates.testStencilDepthState, 0 );

    // Pixel shader permutations for compositing passes. 
    // Dynamic configuration parameters.
    bool bComputeIndirectSpecular = false;
    bool bBilateralUpsample = false;
    
    // Populate argument array.
    cgScriptObject * pSystemExports = pDriver->getShaderInterface();
    mCompositePSArgs.clear();
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("renderFlags") ) ) );
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"),  pSystemExports->getAddressOfMember( _T("shadingQuality") ) ) );
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bComputeIndirectSpecular ) );
    mCompositePSArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool,  _T("bool"), &bBilateralUpsample ) );

    // Set pixel shader for bilinear upsampling
    bBilateralUpsample = false;
    if ( !pShader->selectPixelShader( _T("compositeIrradiance"), mCompositePSArgs ) )
    {
        pDriver->endTargetRender();
        return;
    
    } // End if failed

    // Draw quad
    pDriver->drawClipQuad();

    // Bilateral upsample
    // Note: High res depth buffer must ALREADY have stencil buffer populated with edges
    //       to reduce upsampling processing costs
    
    // Bind the radiance textures for input
    mStates.harmonicsRedPointSampler->apply( hSHR );
    mStates.harmonicsGreenPointSampler->apply( hSHG );
    mStates.harmonicsBluePointSampler->apply( hSHB );

    // Bind low resolution depth buffer
    mStates.depthLowResSampler->apply( mStates.depthTargetLow );

    // Set depth-stencil state
    pDriver->setDepthStencilState( mStates.testStencilDepthState, 1 );

    // Set pixel shader for bilateral upsampling
    bBilateralUpsample = true;
    if ( !pShader->selectPixelShader( _T("compositeIrradiance"), mCompositePSArgs ) )
    {
        pDriver->endTargetRender();
        return;
    
    } // End if failed

    // Draw quad
    pDriver->drawClipQuad();

    // Reset the render target
    pDriver->endTargetRender();
}


//-----------------------------------------------------------------------------
//  Name : beginShadowConfigure ()
/// <summary>
/// Begins configuration of the shadow map texture pool
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::beginShadowConfigure( cgUInt32 nPoolMemLimit, cgUInt32 nMinResolution, cgUInt32 nMaxResolution )
{
    // Begin pool configuration
    return mShadowMapPool->beginConfigure( mParentScene->getResourceManager(), nPoolMemLimit, nMinResolution, nMaxResolution );
}

//-----------------------------------------------------------------------------
//  Name : addDefaultMaps ()
/// <summary>
/// Add default resources for a particular shadowing method.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::addDefaultMaps( cgUInt32 nMethod )
{
    // Generate a settings structure based on the method/flags
    cgShadowSettingsSystem Settings;
    generateShadowSettings( nMethod, Settings );

    // If these settings are valid on the current hardware, generate descriptions and update pool
    if ( validateShadowSettings( Settings ) )
    {
        cgInt32 nFlags;
        cgTexturePoolResourceDesc::Array aDescriptions;
        if ( getResourceDescriptions( Settings, aDescriptions, nFlags ) )
            mShadowMapPool->addDefaultMaps( aDescriptions );
    
    } // End if validated
}

//-----------------------------------------------------------------------------
//  Name : addCachedMaps ()
/// <summary>
/// Add the specified number of resources for this method to the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::addCachedMaps( cgUInt32 nMethod, cgUInt32 nResolution, cgUInt32 nCount )
{
    // Generate a settings structure based on the method/flags
    cgShadowSettingsSystem Settings;
    generateShadowSettings( nMethod, Settings );

    // If these settings are valid on the current hardware, generate descriptions and update pool
    if ( validateShadowSettings( Settings ) )
    {
        cgInt32 nFlags;
        cgTexturePoolResourceDesc::Array aDescriptions;
        if ( getResourceDescriptions( Settings, aDescriptions, nFlags ) )
            mShadowMapPool->addCachedMaps( aDescriptions, nResolution, nCount );
    
    } // End if validated
}

//-----------------------------------------------------------------------------
//  Name : addCachedMaps()
/// <summary>
/// Add the specified number of resources of a given size and format to the pool.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::addCachedMaps( cgBufferFormat::Base Format, cgUInt32 nResolution, cgUInt32 nCount )
{
    // Add to pool
    mShadowMapPool->addMaps( cgTexturePoolResourceType::Cached, Format, nResolution, nCount );
}

//-----------------------------------------------------------------------------
//  Name : endConfigure ()
/// <summary>
/// Ends pool configuration
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::endShadowConfigure()
{
    // End pool configuration
    mShadowMapPool->endConfigure();

    // Load all shadow settings
    return loadShadowSettings( );
}

//-----------------------------------------------------------------------------
//  Name : validateShadowSettings ()
/// <summary>
/// Determines whether the input shadow settings are supported on the current hardware
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::validateShadowSettings( const cgShadowSettingsSystem & Settings ) const
{
    // Get access to required systems
    cgRenderDriver          * pDriver       = mParentScene->getRenderDriver();
    cgTexturePool           * pPool         = mShadowMapPool;
    cgRenderingCapabilities * pDriverCaps   = pDriver->getCapabilities();

    // What type of shadow mapping was requested?
    if ( Settings.method == cgShadowType::PCF || Settings.method == cgShadowType::PCSS )
    {
        // PCF based.
        // Can we store values at the requested precision? (0 = system chooses, so always passes)
        if ( Settings.precision != 0 )
        {
            // Test for hardware and software formats
            if ( !pPool->getBestReadableDepthFormat( Settings.precision, 0 ) )
            {
                if ( !pPool->getBestRenderTargetFormat( Settings.precision, 1 ) )
                    return false;
            
            } // End if unavailable

        } // End if precision specified
    
    } // End if PCF | PCSS
    else if ( Settings.method == cgShadowType::VSM || Settings.method == cgShadowType::ESM || Settings.method == cgShadowType::EVSM )
    {
        // If statistics based.
        // Will use a custom target to store statistics
        cgUInt32 nNumChannels = 1;
        if ( Settings.method == cgShadowType::EVSM ) 
            nNumChannels = 4;
        if ( Settings.method == cgShadowType::VSM ) 
            nNumChannels = 2;

        // Can we store values at the desired precision for depth and statistics?
        cgBufferFormat::Base DepthFormat      = pPool->getBestRenderTargetFormat( Settings.precision, 1 );
        cgBufferFormat::Base StatisticsFormat = pPool->getBestRenderTargetFormat( Settings.precision, nNumChannels );
        if ( DepthFormat == cgBufferFormat::Unknown || StatisticsFormat == cgBufferFormat::Unknown )
            return false;

        // Check filtering support
        cgUInt32 nFormatCaps = pDriverCaps->getBufferFormatCaps( cgBufferType::RenderTarget, StatisticsFormat );
        bool bSupportsLinearFiltering      = ((nFormatCaps & cgBufferFormatCaps::CanLinearFilter) != 0);
        bool bSupportsTrilinearFiltering   = bSupportsLinearFiltering && Settings.trilinearFiltering;
        bool bSupportsAnisotropicFiltering = pDriverCaps->getMaxAnisotropySamples() < Settings.anisotropy;
        bool bSupportsAutoGenMips          = ((nFormatCaps & cgBufferFormatCaps::CanAutoGenMipMaps) != 0);

        // Test filtering support
        if ( Settings.bilinearFiltering && !bSupportsLinearFiltering )
            return false;
        if ( Settings.trilinearFiltering && !bSupportsTrilinearFiltering )
            return false;
        if ( Settings.anisotropy && !bSupportsAnisotropicFiltering )
            return false;
        if ( Settings.autoGenerateMipmaps && !bSupportsAutoGenMips )
            return false;
    
    } // End if VSM | ESM | EVSM

    // If edge masking is needed, test format
    if ( Settings.maskType > 0 )
    {
        cgBufferFormat::Base MaskFormat;
        if ( Settings.maskType == cgShadowMethod::DepthExtentsMask )
        {
            if ( Settings.maskPrecision == 16 )
                MaskFormat = pPool->getBestRenderTargetFormat( 16, 2 );
            else 
                MaskFormat = pPool->getBestRenderTargetFormat(  8, 4 );

        } // End if extents
        else
        {
            MaskFormat = pPool->getBestRenderTargetFormat( 8, 4 );

        } // End if edge only

        if ( MaskFormat == cgBufferFormat::Unknown )
            return false;

    } // End if edges

    // At this stage we've dealt with hardware support for fundamental features.
    // Further testing can now be done to determine whether or not other properties
    // in the structure are supported by the application given the current hardware
    // that is available (e.g., fast vs. slow cpu/gpu).

    // Success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addShadowSettings ()
/// <summary>
/// Adds new (validated) shadow settings to our table
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::addShadowSettings( const cgString & strName, const cgShadowSettingsSystem & Settings )
{
    // If we already have a matching entry, we can ignore this one 
    ShadowSettingsTable::const_iterator itTable = mShadowSettings.find( strName );
    if ( itTable != mShadowSettings.end() )
        return true;

    // Add a new entry to the table
    ShadowSettingsData Data;
    Data.settings = Settings;
    if ( getResourceDescriptions( Data.settings, Data.resourceDescriptions, Data.flags ) )
    {
        mShadowSettings[ strName ] = Data;
        return true;
    
    } // End if valid

    // Failed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getResourceDescriptions ()
/// <summary>
/// Retrieve a list of texture pool resources that are required for the
/// specified shadow system settings.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::getResourceDescriptions( const cgShadowSettingsSystem & Settings, cgTexturePoolResourceDesc::Array & aDescriptions, cgInt32 & Flags ) const
{
    // Clear the output array and flags for safety
    aDescriptions.clear();
    Flags = 0;

    // If these settings are not supported on the current hardware, do not store them
    if ( !validateShadowSettings( Settings ) )
        return false;

    // Get the buffer format enumerator
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    cgRenderingCapabilities * pDriverCaps = pDriver->getCapabilities();

    // Get access to the shared depth/stencil format
    const cgTexturePoolDepthFormatDesc * pSharedDepthStencilFormat = mShadowMapPool->getSharedDepthStencilFormat();

    // Pre-build sampling state descriptions
    cgSamplerStateDesc smpStatesPoint, smpStatesLinear, smpStatesTrilinear, smpStatesAniso;
    smpStatesPoint.addressU   = cgAddressingMode::Clamp;
    smpStatesPoint.addressV   = cgAddressingMode::Clamp;
    smpStatesPoint.addressW   = cgAddressingMode::Clamp;
    smpStatesPoint.minificationFilter  = cgFilterMethod::Point;
    smpStatesPoint.magnificationFilter  = cgFilterMethod::Point;
    smpStatesLinear           = smpStatesPoint;
    smpStatesLinear.minificationFilter = cgFilterMethod::Linear;
    smpStatesLinear.magnificationFilter = cgFilterMethod::Linear;
    smpStatesLinear.mipmapFilter = cgFilterMethod::None;
    smpStatesTrilinear        = smpStatesLinear;
    smpStatesLinear.mipmapFilter = cgFilterMethod::Linear;
    smpStatesAniso            = smpStatesPoint;
    smpStatesAniso.minificationFilter  = cgFilterMethod::Anisotropic;
    smpStatesAniso.magnificationFilter  = cgFilterMethod::Anisotropic;
    smpStatesAniso.mipmapFilter  = cgFilterMethod::Anisotropic;
    smpStatesAniso.maximumAnisotropy = min( (cgUInt)pDriverCaps->getMaxAnisotropySamples(), (cgUInt)Settings.anisotropy );

    // Assign initial high level type for flags
    Flags = Settings.method;

    // Update precision flags
    switch ( Settings.precision )
    {
        case 16:
            Flags |= cgShadowMethod::Bits16;
            break;
        case 24:
            Flags |= cgShadowMethod::Bits24;
            break;
        case 32:
            Flags |= cgShadowMethod::Bits32;
            break;
    
    } // End switch precision

    // For PCF based algorithms, we'll attempt to do hardware compares where available.
    // If this fails, we will attempt hardware gathers and reads first before finally
    // falling back to software depth reads if needed.  
    if ( Settings.method == cgShadowType::PCF || Settings.method == cgShadowType::PCSS )
    {
        cgTexturePoolResourceDesc DepthStencilDesc, DepthDesc;

        // Hardware depth compares (Note: limited to pure PCF for the moment)
        const cgTexturePoolDepthFormatDesc * pDepthFormat;
        if ( Settings.method == cgShadowType::PCF && 
             (pDepthFormat = mShadowMapPool->getBestDepthFormat( Settings.precision, false, false, true, 0 )) )
        {
            // Use a comparable hardware depth buffer format.
            DepthStencilDesc.userData              = cgShadowResourceType::DepthStencilBuffer;
            DepthStencilDesc.type                   = cgTexturePoolResourceType::Cached;
            DepthStencilDesc.bufferDesc.type        = cgBufferType::ShadowMap;
            DepthStencilDesc.bufferDesc.format      = pDepthFormat->bufferDesc.format;
            DepthStencilDesc.bufferDesc.mipLevels   = 1;
            DepthStencilDesc.samplerStates          = pDepthFormat->samplerStates;

            // Use a shared standard color buffer.
            DepthDesc.userData             = cgShadowResourceType::DepthMap;
            DepthDesc.type                  = cgTexturePoolResourceType::Shared;
            DepthDesc.bufferDesc.type       = cgBufferType::RenderTarget;
            DepthDesc.bufferDesc.format     = mShadowMapPool->getBestRenderTargetFormat( 0, 1 );
            DepthDesc.bufferDesc.mipLevels  = 1;
            DepthDesc.samplerStates         = smpStatesPoint;

            // Set hardware comparison flags.
            Flags |= (cgShadowMethod::Hardware | cgShadowMethod::Compare);

        } // End if PCF and HW compare
        else if ( (pDepthFormat = mShadowMapPool->getBestReadableDepthFormat( Settings.precision )) )
        {
            // Hardware depth gather/read
            // Use a hardware readable depth buffer format.
            DepthStencilDesc.userData              = cgShadowResourceType::DepthStencilBuffer;
            DepthStencilDesc.type                   = cgTexturePoolResourceType::Cached;
            DepthStencilDesc.bufferDesc.type        = cgBufferType::ShadowMap;
            DepthStencilDesc.bufferDesc.format      = pDepthFormat->bufferDesc.format;
            DepthStencilDesc.bufferDesc.mipLevels   = 1;
            DepthStencilDesc.samplerStates          = pDepthFormat->samplerStates;

            // Use a shared standard color buffer.
            DepthDesc.userData             = cgShadowResourceType::DepthMap;
            DepthDesc.type                  = cgTexturePoolResourceType::Shared;
            DepthDesc.bufferDesc.type       = cgBufferType::RenderTarget;
            DepthDesc.bufferDesc.format     = mShadowMapPool->getBestRenderTargetFormat( 0, 1 );
            DepthDesc.bufferDesc.mipLevels  = 1;
            DepthDesc.samplerStates         = smpStatesPoint;

            // Set hardware reads vs. gathers.
            if ( pDepthFormat->capabilities & cgBufferFormatCaps::CanGather )
                Flags |= (cgShadowMethod::Hardware | cgShadowMethod::Gather);
            else
                Flags |= (cgShadowMethod::Hardware | cgShadowMethod::DepthReads);

            // Flag RAWZ format if used
            if ( pDepthFormat->bufferDesc.format == cgBufferFormat::RAWZ )
                Flags |= cgShadowMethod::RAWZ;

            // Do manual 2x2 reads
            Flags |= cgShadowMethod::Manual2x2;

        } // End hardware reads
        else 
        {
            // 3. software read
            // Use a shared standard depth buffer
            bool bGather = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanGather) != 0);
            bool bSample = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanSample) != 0);
            DepthStencilDesc.userData              = cgShadowResourceType::DepthStencilBuffer;
            DepthStencilDesc.type                   = cgTexturePoolResourceType::Shared;
            DepthStencilDesc.bufferDesc.type        = (bGather || bSample) ? cgBufferType::ShadowMap : cgBufferType::DepthStencil;
            DepthStencilDesc.bufferDesc.format      = pSharedDepthStencilFormat->bufferDesc.format;
            DepthStencilDesc.bufferDesc.mipLevels   = 1;

            // Will need to write depth to a render target
            DepthDesc.userData                 = cgShadowResourceType::DepthMap;
            DepthDesc.type                      = cgTexturePoolResourceType::Cached;
            DepthDesc.bufferDesc.type           = cgBufferType::RenderTarget;
            DepthDesc.bufferDesc.format         = mShadowMapPool->getBestRenderTargetFormat( Settings.precision, 1 );
            DepthDesc.bufferDesc.mipLevels      = 1;
            DepthDesc.samplerStates             = smpStatesPoint;

            // Set software reading flags
            Flags |= cgShadowMethod::DepthReads;

        } // End if software reads

        // Add descriptions
        aDescriptions.push_back( DepthStencilDesc );
        aDescriptions.push_back( DepthDesc );

    } // End if PCSS or PCF
    else if ( Settings.method == cgShadowType::VSM || Settings.method == cgShadowType::ESM || Settings.method == cgShadowType::EVSM )
    {
        cgTexturePoolResourceDesc DepthStencilDesc, DepthDesc;

        // Process statistical shadow maps
        // Use a shared standard depth-stencil buffer.
        bool bGather = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanGather) != 0);
        bool bSample = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanSample) != 0);
        DepthStencilDesc.userData              = cgShadowResourceType::DepthStencilBuffer;
        DepthStencilDesc.type                   = cgTexturePoolResourceType::Shared;
        DepthStencilDesc.bufferDesc.type        = (bGather || bSample) ? cgBufferType::ShadowMap : cgBufferType::DepthStencil;
        DepthStencilDesc.bufferDesc.format      = pSharedDepthStencilFormat->bufferDesc.format;
        DepthStencilDesc.bufferDesc.mipLevels   = 1;

        // Will write linear depth to a temporary render target if we can't get a readable depth buffer
        DepthDesc.userData             = cgShadowResourceType::DepthMap;
        DepthDesc.type                  = cgTexturePoolResourceType::Shared;
        DepthDesc.bufferDesc.type       = cgBufferType::RenderTarget;
        DepthDesc.bufferDesc.format     = mShadowMapPool->getBestRenderTargetFormat( Settings.precision, 1 );
        DepthDesc.bufferDesc.mipLevels  = 1;
        DepthDesc.samplerStates         = smpStatesPoint;

        // Will use a custom target to store statistics
        cgUInt32 nNumChannels = 1;
        if ( Settings.method == cgShadowType::EVSM ) 
            nNumChannels = 4;
        if ( Settings.method == cgShadowType::VSM ) 
            nNumChannels = 2;

        // Setup basic properties
        cgTexturePoolResourceDesc StatsDesc;
        StatsDesc.userData             = cgShadowResourceType::StatisticsMap;
        StatsDesc.type                  = cgTexturePoolResourceType::Cached;
        StatsDesc.bufferDesc.type       = cgBufferType::RenderTarget;
        StatsDesc.bufferDesc.format     = mShadowMapPool->getBestRenderTargetFormat( Settings.precision, nNumChannels );
        StatsDesc.bufferDesc.mipLevels  = 1;

        // Select correct filtering types
        if ( Settings.anisotropy )
            StatsDesc.samplerStates = smpStatesAniso;
        else if ( Settings.trilinearFiltering )
            StatsDesc.samplerStates = smpStatesTrilinear;
        else if ( Settings.bilinearFiltering )
            StatsDesc.samplerStates = smpStatesLinear;
        else
        {
            StatsDesc.samplerStates = smpStatesPoint;
            Flags |= cgShadowMethod::Manual2x2;
        
        } // End if no HW

        // Add descriptions
        aDescriptions.push_back( DepthStencilDesc );
        aDescriptions.push_back( DepthDesc );
        aDescriptions.push_back( StatsDesc );

    } // End if statistical method
    else if( Settings.method == cgShadowType::RSM )
    {
        cgTexturePoolResourceDesc DepthStencilDesc, DepthDesc, NormalDesc, ColorDesc;
        
        // Reflective shadow maps
        // Use a shared standard depth buffer
        bool bGather = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanGather) != 0);
        bool bSample = ((pSharedDepthStencilFormat->capabilities & cgBufferFormatCaps::CanSample) != 0);
        DepthStencilDesc.userData              = cgShadowResourceType::DepthStencilBuffer;
        DepthStencilDesc.type                   = cgTexturePoolResourceType::Shared;
        DepthStencilDesc.bufferDesc.type        = (bGather || bSample) ? cgBufferType::ShadowMap : cgBufferType::DepthStencil;
        DepthStencilDesc.bufferDesc.format      = pSharedDepthStencilFormat->bufferDesc.format;
        DepthStencilDesc.bufferDesc.mipLevels   = 1;

        // Use an R32F for depth and standard 8-bit 4 channel buffers for normal and color
        DepthDesc.userData             = cgShadowResourceType::DepthMap;
        DepthDesc.type                  = cgTexturePoolResourceType::Cached;
        DepthDesc.bufferDesc.format     = cgBufferFormat::R32_Float;
        DepthDesc.bufferDesc.type       = cgBufferType::RenderTarget;
        DepthDesc.bufferDesc.mipLevels  = 1;
        DepthDesc.samplerStates         = smpStatesPoint;

        NormalDesc.userData            = cgShadowResourceType::NormalMap;
        NormalDesc.type                 = cgTexturePoolResourceType::Cached;
        NormalDesc.bufferDesc.format    = mShadowMapPool->getBestRenderTargetFormat( 8, 4 );
        NormalDesc.bufferDesc.type      = cgBufferType::RenderTarget;
        NormalDesc.bufferDesc.mipLevels = 1;
        NormalDesc.samplerStates        = smpStatesPoint;

        ColorDesc.userData             = cgShadowResourceType::ColorMap;
        ColorDesc.type                  = cgTexturePoolResourceType::Cached;
        ColorDesc.bufferDesc.format     = mShadowMapPool->getBestRenderTargetFormat( 8, 4 );
        ColorDesc.bufferDesc.type       = cgBufferType::RenderTarget;
        ColorDesc.bufferDesc.mipLevels  = 1;
        ColorDesc.samplerStates         = smpStatesPoint;

        // Add the descriptions
        aDescriptions.push_back( DepthStencilDesc );
        aDescriptions.push_back( DepthDesc );
        aDescriptions.push_back( NormalDesc );
        aDescriptions.push_back( ColorDesc );

        // If downsampling is required, push additional recipient target in as well
        if ( Settings.boxFilter )
        {
            aDescriptions.push_back( aDescriptions[ 1 ] );
            aDescriptions.push_back( aDescriptions[ 2 ] );
            aDescriptions.push_back( aDescriptions[ 3 ] );
        
        } // End if boxFilter

    } // End if reflective shadow map

    // If edge masking is needed, setup the requirements.
    if ( Settings.maskType > 0 )
    {
        cgTexturePoolResourceDesc MaskDesc;
        MaskDesc.userData             = cgShadowResourceType::EdgeMap;
        MaskDesc.type                 = cgTexturePoolResourceType::Cached;
        MaskDesc.bufferDesc.type      = cgBufferType::RenderTarget;
        MaskDesc.bufferDesc.mipLevels = 1;
        if ( Settings.maskType == cgShadowMethod::DepthExtentsMask )
        {
            if ( Settings.maskPrecision == 16 )
                MaskDesc.bufferDesc.format = mShadowMapPool->getBestRenderTargetFormat( 16, 2 );
            else 
                MaskDesc.bufferDesc.format = mShadowMapPool->getBestRenderTargetFormat(  8, 4 );

            MaskDesc.samplerStates  = smpStatesPoint;
            MaskDesc.channelCount  = 2;

            Flags |= cgShadowMethod::DepthExtentsMask;

        } // End if extents
        else
        {
            MaskDesc.bufferDesc.format = mShadowMapPool->getBestRenderTargetFormat( 8, 4 );
            MaskDesc.samplerStates     = smpStatesLinear;
            MaskDesc.channelCount      = 1;

            Flags |= cgShadowMethod::EdgeMask;

        } // End if edge only

        // Add description
        aDescriptions.push_back( MaskDesc );

    } // End if edges

    // Process additional settings
    if ( Settings.jitter )
        Flags |= cgShadowMethod::Jitter;
    if ( Settings.rotate )
        Flags |= cgShadowMethod::Rotate;
    if ( Settings.boxFilter )
        Flags |= cgShadowMethod::BoxFilter;
    if ( Settings.normalOffset )
        Flags |= cgShadowMethod::NormalOffset;

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : generateShadowSettings ()
/// <summary>
/// Given a set of shadow flags, populate a settings structure.
/// </summary>
//-----------------------------------------------------------------------------
void cgLightingManager::generateShadowSettings( cgUInt32 nFlags, cgShadowSettingsSystem & SettingsOut ) const
{
    // Clear the structure
    memset( &SettingsOut, 0, sizeof(cgShadowSettingsSystem) );

    // method
    if ( (nFlags & cgShadowMethod::PCSS) == cgShadowMethod::PCSS )
        SettingsOut.method = cgShadowMethod::PCSS;
    else if ( (nFlags & cgShadowMethod::PCF) == cgShadowMethod::PCF )
        SettingsOut.method = cgShadowMethod::PCF;
    else if ( (nFlags & cgShadowMethod::EVSM) == cgShadowMethod::EVSM )
        SettingsOut.method = cgShadowMethod::EVSM;
    else if ( (nFlags & cgShadowMethod::ESM) == cgShadowMethod::ESM )
        SettingsOut.method = cgShadowMethod::ESM;
    else if ( (nFlags & cgShadowMethod::VSM) == cgShadowMethod::VSM )
        SettingsOut.method = cgShadowMethod::VSM;
    else if ( (nFlags & cgShadowMethod::RSM) == cgShadowMethod::RSM )
        SettingsOut.method = cgShadowMethod::RSM;

    // precision
    if ( (nFlags & cgShadowMethod::Bits16) == cgShadowMethod::Bits16 )
        SettingsOut.precision = 16;
    else if ( (nFlags & cgShadowMethod::Bits24) == cgShadowMethod::Bits24 )
        SettingsOut.precision = 24;
    else if ( (nFlags & cgShadowMethod::Bits32) == cgShadowMethod::Bits32 )
        SettingsOut.precision = 32;

    // Masking
    if ( nFlags & cgShadowMethod::DepthExtentsMask )
    {
        SettingsOut.maskType = cgShadowMethod::DepthExtentsMask;
        if ( nFlags & cgShadowMethod::ExtentsBits16 )
            SettingsOut.maskPrecision = 16;
        else
            SettingsOut.maskPrecision = 8;
    
    } // End if extents
    else if ( nFlags & cgShadowMethod::EdgeMask )
    {
        SettingsOut.maskType      = cgShadowMethod::EdgeMask;
        SettingsOut.maskPrecision = 8;
    
    } // End if edge mask

    // General
    if ( nFlags & cgShadowMethod::Jitter )
        SettingsOut.jitter = true;
    if ( nFlags & cgShadowMethod::Rotate )
        SettingsOut.rotate = true;
    if ( nFlags & cgShadowMethod::NormalOffset )
        SettingsOut.normalOffset = true;
    if ( nFlags & cgShadowMethod::Translucency )
        SettingsOut.translucency = true;
}

//-----------------------------------------------------------------------------
//  Name : getShadowSettings ()
/// <summary>
/// Retrieves settings and resource descriptions based on current LOD and best
/// technique found in the input array
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgLightingManager::getShadowSettings( const cgShadowSettingsLOD::Array & aLODs, bool bRSM, cgShadowSettingsSystem & SettingsOut, cgTexturePoolResourceDesc::Array & aDescriptionsOut, cgInt32 & nFlagsOut ) const
{
    // Are we getting standard or reflective shadowmap settings?
    cgInt32 nCurrentLOD = bRSM ? mIndirectSystemLOD : mShadowSystemLOD;

    // Find the best entry for the current LOD
    cgInt32 nIndex    = -1;
    cgInt32 nMaxMatch = INT_MAX;
    const ShadowSettingsData * pData = CG_NULL;
    for ( size_t i = 0; i < aLODs.size(); ++i )
    {
        // Ignore all entries that exceed current LOD
        if ( aLODs[ i ].level > nCurrentLOD )
            continue;

        // Find the closest match
        cgInt32 nDelta = abs( nCurrentLOD - aLODs[ i ].level );
        if ( nDelta < nMaxMatch )
        {
            ShadowSettingsTable::const_iterator itTable = mShadowSettings.find( aLODs[ i ].name );
            if ( itTable != mShadowSettings.end() )
            {
                pData     = &(itTable->second);
                nMaxMatch = nDelta;
                nIndex    = i;

            } // End if contains entry

        } // End if level OK

    } // Next entry

    // If we did not find a match, use default settings.
    if ( !pData )
    {
        ShadowSettingsTable::const_iterator itTable = mShadowSettings.find( mDefaultShadowSettings );
        if ( itTable != mShadowSettings.end() )
        {
            pData  = &(itTable->second);
            nIndex = -2;

        } // End if contains entry

        // Failed to find a default entry
        return -1;

    } // End if use default

    // Copy values
    memcpy( &SettingsOut, &pData->settings, sizeof(cgShadowSettingsSystem) ); 
    nFlagsOut = pData->flags;
    for ( size_t i = 0; i < pData->resourceDescriptions.size(); i++ )
        aDescriptionsOut.push_back( pData->resourceDescriptions[ i ] );

    // Success
    return nIndex;
}

//-----------------------------------------------------------------------------
//  Name : getMaxShadowResolution ()
/// <summary>
/// Returns the maximum shadow resolution (as power of 2 or in pixels)
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgLightingManager::getMaxShadowResolution( bool bInPixels /* = true */ ) const
{
    cgUInt32 nMax = mShadowMapPool->getConfig().maximumResolution;
    return ( bInPixels ) ? nMax : cgMathUtility::log2( nMax );
}

//-----------------------------------------------------------------------------
//  Name : LoadShadowConfig ()
/// <summary>
/// Temporary function to simulate loading shadow settings from the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLightingManager::loadShadowSettings( )
{
    cgUInt32 i;
    cgString strFileName = cgFileSystem::resolveFileLocation( _T("sys://Config/ShadowConfig.ini") );

    // How many settings entries are in the database?
    cgUInt32 nNumEntries = GetPrivateProfileInt( _T("Global"), _T("NumEntries"), 0, strFileName.c_str() );

    // Load in the entries
    for ( i = 0; i < nNumEntries; i++ )
    {
        cgString strEntry = cgString::format( _T("Entry%i"), i );

        cgTChar lpszEntryName[128];
        GetPrivateProfileString( _T("Entries"), strEntry.c_str(), _T(""), lpszEntryName, 128, strFileName.c_str() );
        cgString strName = lpszEntryName;

        // If this is the default fallback, keep track of its name 
        bool bDefault = (GetPrivateProfileInt( lpszEntryName, _T("Default"), 0, strFileName.c_str() ) > 0);
        if ( bDefault )
            mDefaultShadowSettings = strName;

        cgShadowSettingsSystem Settings;
        Settings.method                = GetPrivateProfileInt( lpszEntryName, _T("Method"), 0, strFileName.c_str() );
        Settings.resolutionAdjust      = GetPrivateProfileInt( lpszEntryName, _T("Resolution"), 0, strFileName.c_str() );
        Settings.precision             = GetPrivateProfileInt( lpszEntryName, _T("Precision"), 0, strFileName.c_str() );
        Settings.primarySamples        = GetPrivateProfileInt( lpszEntryName, _T("PrimarySamples"), 0, strFileName.c_str() );
        Settings.secondarySamples      = GetPrivateProfileInt( lpszEntryName, _T("SecondarySamples"), 0, strFileName.c_str() );
        Settings.anisotropy            = GetPrivateProfileInt( lpszEntryName, _T("Anisotropy"), 0, strFileName.c_str() );
        Settings.MSAASamples           = GetPrivateProfileInt( lpszEntryName, _T("MSAASamples"), 0, strFileName.c_str() );
        Settings.autoGenerateMipmaps           = (GetPrivateProfileInt( lpszEntryName, _T("AutoGenMips"), 0, strFileName.c_str() ) > 0);           
        Settings.jitter                = (GetPrivateProfileInt( lpszEntryName, _T("Jitter"), 0, strFileName.c_str() ) > 0);           
        Settings.rotate                = (GetPrivateProfileInt( lpszEntryName, _T("Rotate"), 0, strFileName.c_str() ) > 0);           
        Settings.boxFilter             = (GetPrivateProfileInt( lpszEntryName, _T("BoxFilter"), 0, strFileName.c_str() ) > 0);           
        Settings.bilinearFiltering     = (GetPrivateProfileInt( lpszEntryName, _T("BilinearFiltering"), 0, strFileName.c_str() ) > 0);           
        Settings.trilinearFiltering    = (GetPrivateProfileInt( lpszEntryName, _T("TrilinearFiltering"), 0, strFileName.c_str() ) > 0);           
        Settings.normalOffset          = (GetPrivateProfileInt( lpszEntryName, _T("NormalOffset"), 0, strFileName.c_str() ) > 0);           
        Settings.filterPasses          = GetPrivateProfileInt( lpszEntryName, _T("FilterPasses"), 0, strFileName.c_str() );
        Settings.filterRadius          = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterRadius"), 0, strFileName.c_str() );
        Settings.filterRadiusNear      = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterRadiusNear"), 0, strFileName.c_str() );
        Settings.filterRadiusFar       = cgStringUtility::getPrivateProfileFloat( lpszEntryName, _T("FilterRadiusFar"), 0, strFileName.c_str() );
        Settings.maskType              = GetPrivateProfileInt( lpszEntryName, _T("MaskType"), 0, strFileName.c_str() );
        Settings.maskPrecision         = GetPrivateProfileInt( lpszEntryName, _T("MaskPrecision"), 0, strFileName.c_str() );
        Settings.translucency          = (GetPrivateProfileInt( lpszEntryName, _T("Translucency"), 0, strFileName.c_str() ) > 0);

        // Add the settings to the table (will attempt validation internally)
        addShadowSettings( strName, Settings );

    } // Next entry

    // Success
    return true;
}

//-----------------------------------------------------------------------------
// Name : debug ()
// Desc : Runs the final indirect lighting computations for scenery
//-----------------------------------------------------------------------------
void cgLightingManager::debugGrids( cgCameraNode * pCamera )
{
    cgString strContext;

    // Get access to required systems.
    cgRenderDriver * pDriver = getRenderDriver();

    // Set the current lighting buffer
    if ( !pDriver->beginTargetRender( mStates.currentLightingTarget ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to set valid lighting buffer in processIndirectLights.\n") );
        return;

    } // End begin target

    cgVector3 camPosition = pCamera->getPosition();

    // Draw grid boxes
    for ( cgUInt32 i = 0; i < mRadianceGrids.size(); ++i )
    {
        cgVector4 pc = mRadianceGrids[ i ]->mGridColor;
        mRadianceGrids[ i ]->mGridColor = cgVector4( 1, 1, 0, 0.75 );
        mRadianceGrids[ i ]->debug();
        mRadianceGrids[ i ]->mGridColor = pc;

        cgRadianceGrid * pGrid = mRadianceGrids[ i ];

        cgVector3 gridSize   = pGrid->mBounds.max - pGrid->mBounds.min;
        cgVector3 gridOrigin = pGrid->mBounds.min + cgVector3( 0, gridSize.y, 0 );
        cgVector3 cellSize;
        cellSize.x = (gridSize.x / pGrid->mGridDimensions.x);
        cellSize.y = (gridSize.y / pGrid->mGridDimensions.y);
        cellSize.z = (gridSize.z / pGrid->mGridDimensions.z);

        cgVector3 volCoord = camPosition - pGrid->mBounds.min;
        volCoord.x /= gridSize.x;
        volCoord.y /= gridSize.y;
        volCoord.z /= gridSize.z;
        volCoord.y  = 1.0f - volCoord.y; 

        cgVector3 cellCentered;
        cellCentered.x = floor( volCoord.x * pGrid->mGridDimensions.x ) + 0.5f;
        cellCentered.y = floor( volCoord.y * pGrid->mGridDimensions.y ) + 0.5f;
        cellCentered.z = floor( volCoord.z * pGrid->mGridDimensions.z ) + 0.5f;

        // Print the current cell location
        if( pGrid->mGridDirty )
            printf("[%d] = [%.2f,%.2f,%.2f]\n", pGrid->mCascadeId, cellCentered.x - 0.5f, cellCentered.y - 0.5f, cellCentered.z - 0.5f );

        cellCentered.x = gridOrigin.x + (cellCentered.x * cellSize.x);
        cellCentered.y = gridOrigin.y - (cellCentered.y * cellSize.y);
        cellCentered.z = gridOrigin.z + (cellCentered.z * cellSize.z);

        // Build a world matrix for this cell
        cgMatrix worldMatrix;
        cgMatrix::identity( worldMatrix );
        worldMatrix._11 = cellSize.x * 0.5f;
        worldMatrix._22 = cellSize.y * 0.5f;
        worldMatrix._33 = cellSize.z * 0.5f;
        worldMatrix._41 = cellCentered.x;
        worldMatrix._42 = cellCentered.y;
        worldMatrix._43 = cellCentered.z;

        cgBoundingBox aabb;
        aabb.min = cgVector3( -1, -1, -1 );
        aabb.max = cgVector3(  1,  1,  1 );
        pDriver->drawOOBB( aabb, 0.0f, worldMatrix, cgColorValue( 1, 1, 1, 1 ), true );
    }

    // Reset the render target
    pDriver->endTargetRender();
}