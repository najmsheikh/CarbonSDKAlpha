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
// Name : cgObjectRenderContext.cpp                                          //
//                                                                           //
// Desc : When rendering sections of the node render queue, render control   //
//        script side "techniques" can be supplied that customize the        //
//        necessary rendering behaviors. The context class is provided to    //
//        these 'techniques' as a means of describing the objects currently  //
//        being processed, as well as supplying functionality to operate on  //
//        them (i.e. culling, rendering, etc.)                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgObjectRenderContext Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgObjectRenderContext.h>
#include <Rendering/cgRenderDriver.h>
#include <World/cgScene.h>
#include <World/cgObjectNode.h>
#include <World/Objects/cgLightObject.h>

///////////////////////////////////////////////////////////////////////////////
// cgObjectRenderContext Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgObjectRenderContext () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectRenderContext::cgObjectRenderContext( cgObjectRenderQueue * queue, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString & callback )
{
    // Initialize variables to sensible defaults
    mQueue           = queue;
    mMaterialHandler = materialHandler;
    mLightingHandler = lightingHandler;
    mScriptCallback  = callback;
    mMaterialFilter  = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgObjectRenderContext () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectRenderContext::~cgObjectRenderContext( )
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
void cgObjectRenderContext::dispose( bool disposeBase )
{
    // Release allocated memory.
    if ( mMaterialFilter )
        mMaterialFilter->scriptReleaseRef();
    mMaterialFilter = CG_NULL;
    mRenderBatches.clear();
    mMaterials.clear();
    mLights.clear();
    mLightBatches.clear();
    mCollapseMaterial.close();
}

//-----------------------------------------------------------------------------
//  Name : insertObject () (Protected)
/// <summary>
/// Simply insert the specified object into the list of objects to process.
/// Use of this method should not be mixed with material batched processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::insertObject( cgObjectNode * node )
{
    if ( mRenderBatches.empty() )
        mRenderBatches.push_back( cgObjectNodeArray() );
    mRenderBatches[0].push_back( node );
}

//-----------------------------------------------------------------------------
//  Name : insertLight () (Protected)
/// <summary>
/// Simply insert the specified light into the light list.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::insertLight( cgLightNode * light )
{
    mLights.push_back( light );
}

//-----------------------------------------------------------------------------
//  Name : insertObjectsByMaterial () (Protected)
/// <summary>
/// Insert the specified list of objects into the relevant material category.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::insertObjectsByMaterial( const cgMaterialHandle & material, const cgObjectNodeList & objects )
{
    size_t batch = 0;

    // Does a collection for this material already exist?
    MaterialBatchLUT::iterator itMaterial = mMaterials.find( material );
    if ( itMaterial == mMaterials.end() )
    {
        // We have not encountered this material before, add it.
        batch = mRenderBatches.size();
        mMaterials[ material ] = batch;
        mRenderBatches.push_back( cgObjectNodeArray() );

    } // End if new material
    else
    {
        batch = itMaterial->second;

    } // End if existing material
        
    // Insert these objects into the node array for this material.
    cgObjectNodeArray & nodes = mRenderBatches[batch];
    nodes.reserve( nodes.size() + objects.size() );
    nodes.insert( nodes.end(), objects.begin(), objects.end() );
}

//-----------------------------------------------------------------------------
//  Name : insertObjectsByLightAndMaterial () (Protected)
/// <summary>
/// Insert the specified list of objects into the relevant light and material 
/// category.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::insertObjectsByLightAndMaterial( cgLightNode * light, const cgMaterialHandle & material, const cgObjectNodeList & objects )
{
    size_t batch = 0;

    // Does a collection for this light already exist?
    LightBatchMap::iterator itLight = mLightBatches.find( light );
    if ( itLight == mLightBatches.end() )
    {
        // We have not encountered this light before. Add it.
        mLights.push_back( light );
        itLight = mLightBatches.insert( LightBatchMap::value_type( light, MaterialBatchLUT() ) ).first;

    } // End if

    // Does a collection for this material already exist?
    MaterialBatchLUT & materials = itLight->second;
    MaterialBatchLUT::iterator itMaterial = materials.find( material );
    if ( itMaterial == materials.end() )
    {
        // We have not encountered this material before. Add it.
        batch = mRenderBatches.size();
        materials[ material ] = batch;
        mRenderBatches.push_back( cgObjectNodeArray() );

    } // End if new material
    else
    {
        batch = itMaterial->second;

    } // End if existing material
        
    // Insert these objects into the node array for this material.
    cgObjectNodeArray & nodes = mRenderBatches[batch];
    nodes.reserve( nodes.size() + objects.size() );
    nodes.insert( nodes.end(), objects.begin(), objects.end() );
}

//-----------------------------------------------------------------------------
//  Name : prepare () (Protected)
/// <summary>
/// Prepare the render context for processing (i.e. subsequent series of 'step'
/// calls).
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::prepare( )
{
    // Reset step counter
    mCurrentStep = -1;
}

//-----------------------------------------------------------------------------
//  Name : step ()
/// <summary>
/// Step the context and prepare for the next render operation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectRenderContext::step( )
{
    // Context is processed differently depending on the specified process handler.
    switch ( mQueue->mCurrentProcessHandler )
    {
        case cgQueueProcessHandler::Default:
            return stepDefault( );
            
        case cgQueueProcessHandler::DepthSortedBlending:
            return stepDepthSortedBlending( );
        
        default:
            return false;
    
    } // End switch process
}

//-----------------------------------------------------------------------------
//  Name : stepDefault () (Protected)
/// <summary>
/// Step the context and prepare for the next render operation. This version
/// is specific to the 'cgQueueProcessHandler::Default' handler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectRenderContext::stepDefault( )
{
    switch ( mLightingHandler )
    {
        case cgQueueLightingHandler::None:
            
            // What step are we on?
            switch ( mCurrentStep )
            {
                case -1:
                    // First (and only) step.
                    mCurrentStep = 0;
                    return true;

                default:
                    // No further steps.
                    return false;

            } // End switch step
            break;

        case cgQueueLightingHandler::Default:

            // Number of steps is equivalent to number of lights.
            if ( mCurrentStep >= (cgInt32)mLights.size() - 1 )
                return false;

            // Move to next step.
            mCurrentStep++;
            return true;

        default:
            // No step -- unknown handler.
            return false;

    } // End switch lighting handler
}

//-----------------------------------------------------------------------------
//  Name : stepDepthSortedBlending () (Protected)
/// <summary>
/// Step the context and prepare for the next render operation. This version
/// is specific to the 'cgQueueProcessHandler::DepthSortedBlending' handler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectRenderContext::stepDepthSortedBlending( )
{
    switch ( mLightingHandler )
    {
        case cgQueueLightingHandler::Default:    
        case cgQueueLightingHandler::None:
            
            // What step are we on?
            switch ( mCurrentStep )
            {
                case -1:
                    // First (and only) step.
                    mCurrentStep = 0;
                    return true;

                default:
                    // No further steps.
                    return false;

            } // End switch step
            break;

        default:
            // No step -- unknown handler.
            return false;

    } // End switch lighting handler
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Perform object rendering for the current context step.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::render( )
{
    // Context is processed differently depending on the specified process handler.
    switch ( mQueue->mCurrentProcessHandler )
    {
        case cgQueueProcessHandler::Default:
            renderDefault( mMaterials );
            break;
            
        case cgQueueProcessHandler::DepthSortedBlending:
            renderDepthSortedBlending( );
            break;
    
    } // End switch process
}

//-----------------------------------------------------------------------------
//  Name : renderDefault ()
/// <summary>
/// Perform object rendering for the current context step. This version
/// is specific to the 'cgQueueProcessHandler::Default' handler.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::renderDefault( const MaterialBatchLUT & Materials )
{
    // Get access to required systems / objects.
    cgRenderDriver * driver = mQueue->mScene->getRenderDriver();
    cgCameraNode  * camera = driver->getCamera();

    // ToDo: Consider constructing an ordered render batch list using MaterialKey
    // in order to ensure that materials with similar data exist consecutively
    // (i.e. same surface shader).

    // If we're collapsing the materials (i.e. not setting them) then
    // setup the specified / default material and process all batches
    // immediately. Otherwise, full material draws are required.
    if ( mMaterialHandler == cgQueueMaterialHandler::Collapse )
    {
        // Set the collapse material
        if ( !driver->setMaterial( mCollapseMaterial, true ) )
            return;
        
        // Begin rendering with the current material outside the loop if we're collapsing.
        if ( driver->beginMaterialRender() )
        {
            // Iterate through each material pass.
            size_t i;
            MaterialBatchLUT::const_iterator itMaterial;
            while ( driver->executeMaterialPass() == cgTechniqueResult::Continue )
            {
                // Render requested subsets of the specified objects.
                for ( itMaterial = Materials.begin(); itMaterial != Materials.end(); ++itMaterial )
                {
                    cgObjectNodeArray & nodes = mRenderBatches[itMaterial->second];
                    
                    // Draw this subset of each node
                    for ( i = 0; i < nodes.size(); ++i )
                        nodes[i]->renderSubset( camera, mQueue->mCurrentVisibilitySet, itMaterial->first );

                } // Next Material

            } // Next Pass
                
            // Finish up
            driver->endMaterialRender();

        } // End if begun
        
    } // End if collapsing
    else
    {          
        // Render materials in full
        size_t i;
        MaterialBatchLUT::const_iterator itMaterial;
        for ( itMaterial = Materials.begin(); itMaterial != Materials.end(); ++itMaterial )
        {
            cgObjectNodeArray & nodes = mRenderBatches[itMaterial->second];
            
            // Setup this material
            if ( !driver->setMaterial( itMaterial->first, true ) )
                continue;

            // If a 'null' material was referenced, bypass the material
            // rendering system and just directly call into each node.
            // Otherwise, process the material as-per usual.
            if ( !itMaterial->first.isValid() )
            {
                // Draw the matching subset of each node
                for ( i = 0; i < nodes.size(); ++i )
                    nodes[i]->renderSubset( camera, mQueue->mCurrentVisibilitySet, cgMaterialHandle::Null );

            } // End if 'null' material
            else
            {
                // Render with this material.
                if ( driver->beginMaterialRender() )
                {
                    while ( driver->executeMaterialPass() == cgTechniqueResult::Continue )
                    {
                        // Draw this subset of each node
                        for ( i = 0; i < nodes.size(); ++i )
                            nodes[i]->renderSubset( camera, mQueue->mCurrentVisibilitySet, itMaterial->first );

                    } // Next Render Pass
                    driver->endMaterialRender();
                
                } // End if begun

            } // End if valid material

        } // Next Material
        
    } // End if !collapsing
}

//-----------------------------------------------------------------------------
//  Name : renderDepthSortedBlending ()
/// <summary>
/// Perform object rendering for the current context step. This version
/// is specific to the 'cgQueueProcessHandler::DepthSortedBlending' handler.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::renderDepthSortedBlending( )
{
    // Anything to do?
    if ( mRenderBatches.empty() || mRenderBatches[0].empty() )
        return;

    // Get access to required systems / objects.
    cgRenderDriver * driver = mQueue->mScene->getRenderDriver();
    cgCameraNode  * camera = driver->getCamera();

    // Setup material filtering if any supplied.
    if ( mMaterialFilter )
        driver->pushMaterialFilter( mMaterialFilter );

    // Render all available objects
    cgObjectNodeArray & objectNodes = mRenderBatches[0];
    for ( size_t i = 0; i < objectNodes.size(); ++i )
    {
        objectNodes[i]->render( camera, CG_NULL );

    } // Next object node

    // We're done
    if ( mMaterialFilter )
        driver->popMaterialFilter( );
}

//-----------------------------------------------------------------------------
//  Name : light ()
/// <summary>
/// Perform object lighting for the current context step.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::light( )
{
    // Valid operation for this context?
    if ( mLightingHandler == cgQueueLightingHandler::None )
        return;

    // Context is processed differently depending on the specified process handler.
    switch ( mQueue->mCurrentProcessHandler )
    {
        case cgQueueProcessHandler::Default:
            lightDefault( );
            break;
            
        case cgQueueProcessHandler::DepthSortedBlending:
            lightDepthSortedBlending( );
            break;
    
    } // End switch process
}

//-----------------------------------------------------------------------------
//  Name : light ()
/// <summary>
/// Perform object lighting for the current context step. This version
/// is specific to the 'cgQueueProcessHandler::Default' handler.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::lightDefault( )
{
    // Get access to required systems / objects.
    cgRenderDriver * driver = mQueue->mScene->getRenderDriver();
    cgCameraNode  * camera = driver->getCamera();

    // Retrieve the current light source being processed.
    cgLightNode * light = mLights[mCurrentStep];
    const MaterialBatchLUT & materials = mLightBatches[light];

    // Begin the lighting process.
    cgInt32 pass, passCount;
    if ( (passCount = light->beginLighting( CG_NULL, false, false ) ) >= 0 )
    {
        // Process requested number of lighting passes.
        for ( pass = 0; pass < passCount; ++pass )
        {
            // Begin the pass. Note: this function returns a pointer to the final visibility set
            // we should use as the basis for rendering any appropriate objects.
            cgVisibilitySet * renderVisibility;
            cgLightNode::LightingOp op = light->beginLightingPass( pass, renderVisibility );
            if ( op == cgLightNode::Lighting_ProcessLight )
                renderDefault( materials );
            else if ( op == cgLightNode::Lighting_FillShadowMap )
            {
                cgToDoAssert( "Carbon General", "Re-add support for forward shadow casting." );
            
            } // End if fill shadow map

            // We've finished this lighting pass
            light->endLightingPass();

        } // Next pass

        // We're done with this light
        light->endLighting();

    } // End if beginLighting()
}

//-----------------------------------------------------------------------------
//  Name : lightDepthSortedBlending ()
/// <summary>
/// Perform object lighting for the current context step. This version
/// is specific to the 'cgQueueProcessHandler::DepthSortedBlending' handler.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderContext::lightDepthSortedBlending( )
{
    // Get access to required systems / objects.
    cgRenderDriver * driver = mQueue->mScene->getRenderDriver();
    cgCameraNode  * camera = driver->getCamera();

    // First we need to execute the base 'opacity' pass for this transparent object.
    cgString originalPassName = driver->getRenderPass();
    driver->pushRenderPass( originalPassName + _T("_Opacity") );
    renderDepthSortedBlending( );
    driver->popRenderPass();

    // Now we need to perform the lighting process.
    if ( !mLights.empty() )
    {
        driver->pushRenderPass( originalPassName + _T("_DirectLighting") );

        // Process each light.
        for ( size_t i = 0; i < mLights.size(); ++i )
        {
            // Retrieve the current light source being processed.
            cgLightNode * light = mLights[i];

            // Process the light.
            cgInt32 pass, passCount;
            if ( (passCount = light->beginLighting( CG_NULL, false, false ) ) >= 0 )
            {
                // Process requested number of lighting passes.
                for ( pass = 0; pass < passCount; ++pass )
                {
                    // Begin the pass. Note: this function returns a pointer to the final visibility set
                    // we should use as the basis for rendering any appropriate objects.
                    cgVisibilitySet * renderVisibility;
                    cgLightNode::LightingOp op = light->beginLightingPass( pass, renderVisibility );
                    if ( op == cgLightNode::Lighting_ProcessLight )
                        renderDepthSortedBlending( );
                    else if ( op == cgLightNode::Lighting_FillShadowMap )
                    {
                        cgToDoAssert( "Carbon General", "Re-add support for forward shadow casting." );
                    
                    } // End if fill shadow map

                    // We've finished this lighting pass
                    light->endLightingPass();

                } // Next pass

                // We're done with this light
                light->endLighting();

            } // End if beginLighting()

        } // Next Light

        // Finish up
        driver->popRenderPass();
        
    } // End if has lights

    // Now finish up with the fog pass.
    driver->pushRenderPass( originalPassName + _T("_Fog") );
    renderDepthSortedBlending( );
    driver->popRenderPass();
}