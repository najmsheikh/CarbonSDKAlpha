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
// Name : cgObjectRenderQueue.cpp                                            //
//                                                                           //
// Desc : Utility class used for the automatic rendering of object nodes     //
//        in a defined, or sorted order including the ability to batch by    //
//        material and control the rendering behavior through scripted       //
//        render control "techniques".                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgObjectRenderQueue Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgObjectRenderQueue.h>
#include <Rendering/cgObjectRenderContext.h>
#include <Resources/cgMaterial.h>
#include <Resources/cgResourceManager.h>
#include <World/cgScene.h>
#include <World/cgVisibilitySet.h>
#include <World/cgWorldConfiguration.h>
#include <World/cgObjectNode.h>
#include <World/Objects/cgLightObject.h>
#include <System/cgFilterExpression.h>

///////////////////////////////////////////////////////////////////////////////
// cgObjectRenderQueue Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgObjectRenderQueue () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectRenderQueue::cgObjectRenderQueue( cgScene * scene )
{
    // Initialize variables to sensible defaults
    mScene                    = scene;
    mPopulating               = false;
    mCurrentProcessHandler    = cgQueueProcessHandler::Default;
    mCurrentVisibilitySet     = CG_NULL;
    mCurrentMaterialFilter    = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgObjectRenderQueue () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectRenderQueue::~cgObjectRenderQueue( )
{
    // Clean up
    dispose( false );

    // Clear variables
    mScene = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::dispose( bool disposeBase )
{
    // Clear queue to free up memory.
    clear();
}

//-----------------------------------------------------------------------------
//  Name : begin()
/// <summary>
/// Begin queue population from the specified visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::begin( cgVisibilitySet * visibilityData )
{
    // Clear any existing queue contents.
    clear();

    // Setup queue population tracking
    mCurrentVisibilitySet = visibilityData;
    mCurrentProcessHandler = cgQueueProcessHandler::Default;

    // Hold on to a reference to the visibility set
    if ( mCurrentVisibilitySet )
        mCurrentVisibilitySet->scriptAddRef();

    // Use the system default material for the collapse material
    // if one was not explicitly provided.
    if ( !mCollapseMaterial.isValid() )
        mCollapseMaterial = mScene->getResourceManager()->getDefaultMaterial();

    // Population is started.
    mPopulating = true;
}

//-----------------------------------------------------------------------------
//  Name : begin()
/// <summary>
/// Begin queue population from the specified visibility set. The queue will
/// be constructed in-line with the requested process handling description.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::begin( cgVisibilitySet * visibilityData, cgQueueProcessHandler::Base process )
{
    // Clear any existing queue contents.
    clear();

    // Setup queue population tracking
    mCurrentVisibilitySet = visibilityData;
    mCurrentProcessHandler = process;

    // Hold on to a reference to the visibility set
    if ( mCurrentVisibilitySet )
        mCurrentVisibilitySet->scriptAddRef();

    // Use the system default material for the collapse material
    // if one was not explicitly provided.
    if ( !mCollapseMaterial.isValid() )
        mCollapseMaterial = mScene->getResourceManager()->getDefaultMaterial();

    // Population is started.
    mPopulating = true;
}

//-----------------------------------------------------------------------------
//  Name : end()
/// <summary>
/// Queue population is complete and can be executed with a call to 'flush()'. 
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::end( )
{
    end( false );
}

//-----------------------------------------------------------------------------
//  Name : end()
/// <summary>
/// Queue population is complete. The queue can optionally be immediately
/// flushed.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::end( bool flushQueue )
{
    if ( !mPopulating )
        return;

    // Release references
    if ( mCurrentVisibilitySet )
        mCurrentVisibilitySet->scriptReleaseRef();

    // Reset population parameters
    setMaterialFilter( CG_NULL );
    mCurrentVisibilitySet = CG_NULL;
    mCollapseMaterial.close();

    // Sort the contexts if required.
    if ( mCurrentProcessHandler == cgQueueProcessHandler::DepthSortedBlending )
    {
        // Sort the objects.
        struct BackToFrontSort
        {
            bool operator()( cgObjectRenderContext * p1, cgObjectRenderContext * p2 ) const
            {
                return p1->mSortKey > p2->mSortKey;
            }
        };
        mContextList.sort( BackToFrontSort() );

    } // End if requires distance sort

    // Population is now complete.
    mPopulating = false;

    // Flush, if requested.
    if ( flushQueue )
        flush();
}

//-----------------------------------------------------------------------------
//  Name : clear()
/// <summary>
/// Release all internal data that represents the constructed render queue.
/// This method is automatically triggered during calls to 'begin()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::clear( )
{
    // Complete population (just in case)
    end( false );

    // Destroy active contexts.
    ContextList::iterator itContext;
    for ( itContext = mContextList.begin(); itContext != mContextList.end(); ++itContext )
        (*itContext)->scriptReleaseRef();
    mContextList.clear();
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className )
{
    renderClass( className, cgQueueMaterialHandler::Default, cgQueueLightingHandler::None, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class. Call
/// back into the script when one or more elements of this type are encountered.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className, const cgString & callback )
{
    renderClass( className, cgQueueMaterialHandler::Default, cgQueueLightingHandler::None, callback );
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class. 
/// Process materials in-line with the requested material handling description.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className, cgQueueMaterialHandler::Base materialHandler )
{
    renderClass( className, materialHandler, cgQueueLightingHandler::None, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class. 
/// Process materials in-line with the requested material handling description.
/// Call back into the script when one or more elements of this type are 
/// encountered.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className, cgQueueMaterialHandler::Base materialHandler, const cgString & callback )
{
    renderClass( className, materialHandler, cgQueueLightingHandler::None, callback );
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class. 
/// Process materials in-line with the requested material handling description.
/// Process lights in-line with the requested light handling description.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler )
{
    renderClass( className, materialHandler, lightingHandler, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : renderClass()
/// <summary>
/// Render the objects that are assigned to the specified rendering class. 
/// Process materials in-line with the requested material handling description.
/// Process lights in-line with the requested light handling description.
/// Call back into the script when one or more elements of this type are 
/// encountered.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClass( const cgString & className, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString & callback )
{
    // Bail if we didn't 'begin'.
    if ( !mPopulating )
        return;

    // Retrieve the integer identifier of the specified class by name.
    cgWorld * world = mScene->getParentWorld();
    cgWorldConfiguration * config = world->getConfiguration();
    cgUInt32 classId = config->getRenderClassId( className );

    // Bail if this is not a valid render class.
    if ( !classId )
        return;

    // Queue is built differently depending on the specified process handler.
    switch ( mCurrentProcessHandler )
    {
        case cgQueueProcessHandler::Default:
            renderClassDefault( classId, materialHandler, lightingHandler, callback );
            break;

        case cgQueueProcessHandler::DepthSortedBlending:
            renderClassDepthSortedBlending( classId, materialHandler, lightingHandler, callback );
            break;
    
    } // End switch process
}

//-----------------------------------------------------------------------------
//  Name : renderClassDefault() (Protected)
/// <summary>
/// Render the objects that are assigned to the specified rendering class. 
/// This is the internal method that is used to construct the queue when
/// the 'Default' process handler is specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClassDefault( cgUInt32 classId, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString& callback )
{
    // Find the referenced render class from the main visibility set.
    cgVisibilitySet::RenderClassMap & renderClasses = mCurrentVisibilitySet->getVisibleRenderClasses();
    cgVisibilitySet::RenderClassMap::iterator itClass = renderClasses.find( classId );
    if ( itClass == renderClasses.end() )
        return;

    // Get the most recent queue render context (if any).
    cgObjectRenderContext * context = CG_NULL;
    if ( !mContextList.empty() )
        context = mContextList.back();
    
    // Join with existing context if we are able.
    if ( !context ||
         context->mMaterialHandler != materialHandler ||
         context->mLightingHandler != lightingHandler ||
         context->mScriptCallback  != callback ||
         (materialHandler == cgQueueMaterialHandler::Collapse && context->mCollapseMaterial != mCollapseMaterial) )
    {
        // Create a new rendering context
        context = new cgObjectRenderContext( this, materialHandler, lightingHandler, callback );
        
        // Set the collapse handler material if required.
        if ( materialHandler == cgQueueMaterialHandler::Collapse )
            context->mCollapseMaterial = mCollapseMaterial;

        // Queue up.
        mContextList.push_back( context );

    } // End if cannot join

    // Applying lighting?
    if ( lightingHandler == cgQueueLightingHandler::None )
    {
        // Process the visible materials in this render class.
        cgVisibilitySet::MaterialBatchMap & materials = itClass->second.materials;
        cgVisibilitySet::MaterialBatchMap::iterator itMaterial = materials.begin();
        for ( ; itMaterial != materials.end(); ++itMaterial )
        {
            const cgMaterialHandle & material = itMaterial->first;

            // Skip materials that don't match any currently defined material filter.
            // *Always* include the 'null' material type.
            if ( mCurrentMaterialFilter )
            {   
                if ( material.isValid() && !mCurrentMaterialFilter->evaluate( material->getMaterialProperties() ) )
                    continue;

            } // End if filtering

            // We've found the objects that belong to the referenced class. Add these 
            // objects to the appropriate context.
            context->insertObjectsByMaterial( itMaterial->first, itMaterial->second.objectNodes );
            
        } // Next material

    } // End if no lighting
    else if ( lightingHandler == cgQueueLightingHandler::Default )
    {
        // Process the lights in the visibility set.
        cgObjectNodeArray & visibleLights = mCurrentVisibilitySet->getVisibleLights();
        context->mLights.reserve( context->mLights.size() + visibleLights.size() );
        for ( size_t i = 0; i < visibleLights.size(); ++i )
        {
            cgLightNode * light = (cgLightNode*)visibleLights[i];

            // Immediately reject all visible nodes with this render class if their
            // combined bounding box does not intersect the light's volume.
            if ( !light->boundsInVolume( itClass->second.combinedBounds ) )
                continue;

            // Process the visible objects in this render class.
            cgVisibilitySet::MaterialBatchMap & materials = itClass->second.materials;
            cgVisibilitySet::MaterialBatchMap::iterator itMaterial = materials.begin();
            for ( ; itMaterial != materials.end(); ++itMaterial )
            {
                const cgMaterialHandle & material = itMaterial->first;

                // Immediately reject all visible nodes with this material if their
                // combined bounding box does not intersect the light's volume.
                if ( !light->boundsInVolume( itMaterial->second.combinedBounds ) )
                    continue;

                // Skip materials that don't match any currently defined material filter.
                // *Always* include the 'null' material type.
                if ( mCurrentMaterialFilter )
                {   
                    if ( material.isValid() && !mCurrentMaterialFilter->evaluate( material->getMaterialProperties() ) )
                        continue;

                } // End if filtering

                // Build a list of all nodes which intersect the light source volume.
                cgObjectNodeArray outputNodes;
                const cgObjectNodeArray & inputNodes = itMaterial->second.objectNodes;
                for ( size_t i = 0; i < inputNodes.size(); ++i )
                {
                    if ( light->boundsInVolume( inputNodes[i]->getBoundingBox() ) )
                        outputNodes.push_back( inputNodes[i] );

                } // Next input node

                // We've found the objects that belong to the referenced class. Add these 
                // objects to the appropriate context.
                if ( !outputNodes.empty() )
                    context->insertObjectsByLightAndMaterial( light, itMaterial->first, outputNodes );
                
            } // Next material

        } // Next light

    } // End if lighting
}

//-----------------------------------------------------------------------------
//  Name : renderClassDepthSortedBlending() (Protected)
/// <summary>
/// Render the objects that are assigned to the specified rendering class in
/// distance sorted order. This is the internal method that is used to 
/// construct the queue when the 'DepthSortedBlending' process handler is 
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::renderClassDepthSortedBlending( cgUInt32 classId, cgQueueMaterialHandler::Base materialHandler, cgQueueLightingHandler::Base lightingHandler, const cgString& callback )
{
    // Get list of visible lights in case we need it.
    cgObjectNodeArray & visibleLights = mCurrentVisibilitySet->getVisibleLights();

    // Find the referenced render class from the main visibility set.
    cgVisibilitySet::RenderClassMap & renderClasses = mCurrentVisibilitySet->getVisibleRenderClasses();
    cgVisibilitySet::RenderClassMap::iterator itClass = renderClasses.find( classId );
    if ( itClass == renderClasses.end() )
        return;

    // Process the visible objects in this render class.
    cgVector3 sortOrigin = mCurrentVisibilitySet->getVolume().position;
    cgObjectNodeArray & objectNodes = itClass->second.objectNodes;
    for ( size_t i = 0; i < objectNodes.size(); ++i )
    {
        // Create a new rendering context
        cgObjectRenderContext * context = new cgObjectRenderContext( this, materialHandler, lightingHandler, callback );

        // Render with this object 
        cgObjectNode * objectNode = objectNodes[i];
        context->insertObject( objectNode );

        // Compute distance for sorting based on the closest point on its bounding box
        const cgBoundingBox & objectBounds = objectNode->getBoundingBox();
        context->mSortKey = cgVector3::lengthSq( objectBounds.closestPoint( sortOrigin ) - sortOrigin );

        // Should light?
        if ( lightingHandler == cgQueueLightingHandler::Default )
        {
            // Reserve enough space for all light candidates
            context->mLights.reserve( context->mLights.size() + visibleLights.size() );
            
            // Process for each light.
            for ( size_t j = 0; j < visibleLights.size(); ++j )
            {
                cgLightNode * light = (cgLightNode*)visibleLights[j];

                // Immediately reject this light source if it doesn't even intersect the 
                // combined bounding box of the render class.
                if ( !light->boundsInVolume( itClass->second.combinedBounds ) )
                    continue;

                // Also reject the light source if it doesn't intersect the node's bounds.
                if ( !light->boundsInVolume( objectBounds ) )
                    continue;

                // Use this light source.
                context->insertLight( light );

            } // Next light

        } // End if lighting

        // Use specified material filter during rendering.
        context->mMaterialFilter = mCurrentMaterialFilter;
        if ( mCurrentMaterialFilter )
            mCurrentMaterialFilter->scriptAddRef();

        // Queue up.
        mContextList.push_back( context );

    } // Next object
}

//-----------------------------------------------------------------------------
//  Name : setCollapseMaterial()
/// <summary>
/// Set the material that will replace the object's materials when the 
/// 'Collapse' material handling  description is supplied via a call to one of 
/// the 'Render' methods. This material will be automatically cleared when
/// queue population is complete.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::setCollapseMaterial( const cgMaterialHandle & material )
{
    mCollapseMaterial = material;
}

//-----------------------------------------------------------------------------
//  Name : setMaterialFilter()
/// <summary>
/// Set the filter expression that describes the types of materials that will
/// be considered when objects are being added to the queue. The filter 
/// expression will be automatically cleared when queue population is complete.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::setMaterialFilter( cgFilterExpression * filter )
{
    if ( mCurrentMaterialFilter )
        mCurrentMaterialFilter->scriptReleaseRef();
    mCurrentMaterialFilter = filter;
    if ( mCurrentMaterialFilter )
        mCurrentMaterialFilter->scriptAddRef();
}

//-----------------------------------------------------------------------------
//  Name : setMaterialFilter()
/// <summary>
/// Set the filter expression that describes the types of materials that will
/// be considered when objects are being added to the queue. The filter 
/// expression will be automatically cleared when queue population is complete.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectRenderQueue::setMaterialFilter( const cgString & expression )
{
    cgToDo( "Carbon General", "Optimize with some kind of expression lookup to save compiling each time?" );

    // Release prior filter expression.
    if ( mCurrentMaterialFilter )
        mCurrentMaterialFilter->scriptReleaseRef();

    // Compile the new expression.
    mCurrentMaterialFilter = new cgFilterExpression( expression, mScene->getMaterialPropertyIdentifiers() );

    // Did it fail to compile?
    if ( !mCurrentMaterialFilter->isCompiled() )
    {
        delete mCurrentMaterialFilter;
        mCurrentMaterialFilter = CG_NULL;
        return false;

    } // end if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : flush()
/// <summary>
/// Flush / execute the queue.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectRenderQueue::flush( )
{
    // End population if necessary
    if ( mPopulating )
        end( false );

    // Process the queue.
    ContextList::iterator itContext;
    for ( itContext = mContextList.begin(); itContext != mContextList.end(); ++itContext )
    {
        cgObjectRenderContext * context = (*itContext);
        
        // Prepare the context ready for processing (i.e. prepare for 'step' calls).
        context->prepare( );

        // Execute any callback if supplied, otherwise perform default handling.
        if ( !context->mScriptCallback.empty() )
        {
        } // End if has callback
        else
        {
            // Queue is processed differently depending on the lighting handler.
            switch ( context->mLightingHandler )
            {
                case cgQueueLightingHandler::None:
                    while ( context->step() )
                        context->render();
                    break;

                case cgQueueLightingHandler::Default:
                    while ( context->step() )
                        context->light();
                    break;

            } // End switch lighting handler

        } // End if default

    } // Next context
}