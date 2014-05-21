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
// Name : cgVisibilitySet.cpp                                                //
//                                                                           //
// Desc : Classes responsible for collecting, storing and managing the       //
//        visibility information for parts of the scene including            //
//        objects and spatial tree leaves.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

// TODO: Object needs to remove itself from the visibility set if its shadow caster / renderable status changes
// TODO: cgLightNode::registerVisibility did a narrow phase test that is now disabled (will not function with frame coherence -- think about this further).
// TODO: cgScene::sandboxRender will not currently collect ALL objects as it did before.
// TODO: Render class combined bounding box needs to be recomputed when removing.
// TODO: Material batch combined bounding box needs to be recomputed when removing.

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgVisibilitySet Module Includes
//-----------------------------------------------------------------------------
#include <World/cgVisibilitySet.h>
#include <World/cgScene.h>
#include <World/cgSphereTree.h>
#include <World/Objects/cgLightObject.h>
#include <System/cgTimer.h>

//-----------------------------------------------------------------------------
// Static Member Variable Definitions
//-----------------------------------------------------------------------------
cgUInt32 cgVisibilitySet::mNextResultId = 1;

///////////////////////////////////////////////////////////////////////////////
// cgVisibilitySet Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgVisibilitySet () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::cgVisibilitySet( cgScene * scene )
{
    // Initialize variables to sensible defaults.
    mResultId             = 0;
    mScene                = scene;
    mLastComputedFrame    = 0;
    mLastModifiedFrame    = 0;
    mSearchFlags          = cgVisibilitySearchFlags::MustRender;

    // Register with the scene tree.
    if ( scene && scene->getSceneTree() )
        scene->getSceneTree()->addVisibilitySet( this );
}

//-----------------------------------------------------------------------------
//  Name : ~cgVisibilitySet () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::~cgVisibilitySet( )
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
void cgVisibilitySet::dispose( bool bDisposeBase )
{
    // Detach from the scene tree.
    if ( mScene && mScene->getSceneTree() )
        mScene->getSceneTree()->removeVisibilitySet( this );

    // Clear the set.
    clear();
}

//-----------------------------------------------------------------------------
//  Name : compute ()
/// <summary>
/// Compute the visibility for all scene objects and scenery based on the
/// frustum specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::compute( const cgFrustum & Frustum )
{
    // Is it really necessary for us to recompute visibility in this frame?
    cgTimer * pTimer = cgTimer::getInstance();
    if ( mLastComputedFrame == pTimer->getFrameCounter() && mLastFrustum == Frustum )
         return;

    // Store the frustum used for visibility computation so that other
    // parts of the system can query for the visibility volume later.
    mFrustum = Frustum;

    // Ask the scene to register all visible objects based on its broadphase data.
    mScene->computeVisibility( Frustum, this );
   
    // Record information about this computation in order to prevent
    // it from being accidentally recomputed a second time in any given frame.
    mLastComputedFrame    = pTimer->getFrameCounter();
    mLastFrustum          = mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : query ()
/// <summary>
/// Determine if the specified object is contained within the visibility
/// set or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::query( cgObjectNode * pObject ) const
{
    if ( pObject == CG_NULL )
        return false;

    // Test the correct set.
    if ( pObject->queryReferenceType( RTID_LightNode ) )
        return (mLightLUT.find( pObject ) != mLightLUT.end());
    else 
        return (mObjectNodeLUT.find( pObject ) != mObjectNodeLUT.end());
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Clear out all of the currently computed visibility information.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::clear( )
{
    // Clear out tree visibility data
    mTreeLeaves.clear();
    mAssociatedGroups.clear();
    mObjectNodes.clear();
    mObjectNodeLUT.clear();
    mLights.clear();
    mLightLUT.clear();
    mRenderClasses.clear();

    // Select the next unique visibility result identifier
    mResultId = mNextResultId++;

    // If the result identifier variable overflowed and wrapped around,
    // ensure that the reserved code '0' is skipped.
    if ( mNextResultId == 0 )
        mNextResultId = 1;

    // Visibility set is now dirty.
    mLastComputedFrame    = 0;
}

//-----------------------------------------------------------------------------
//  Name : getResultId ()
/// <summary>
/// Retrieve the unique visibility result identifier for the call to 'compute'
/// that generated this visibility set information.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVisibilitySet::getResultId( ) const
{
    return mResultId;
}

//-----------------------------------------------------------------------------
//  Name : isSetModifiedSince () (Virtual)
/// <summary>
/// Determine if the set has been modified at some point after the specified
/// frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isSetModifiedSince( cgUInt32 frame ) const
{
    return (mLastModifiedFrame >= frame);
}

//-----------------------------------------------------------------------------
//  Name : addVisibleObject ()
/// <summary>
/// If an object is deemed to be visible, this method should be called
/// in order for the object to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::addVisibleObject( cgObjectNode * pObject )
{
    // Objects must have a valid reference identifier in order to be considered.
    /*if ( !pObject || !pObject->getReferenceId() )
        return;*/
    
    // Skip object if it already exists (user lower bound for existence test, which will also 
    // act as the insert point if no precise match was found).
    EntryLUT::iterator itLB = mObjectNodeLUT.lower_bound( pObject );
    if ( itLB == mObjectNodeLUT.end() || mObjectNodeLUT.key_comp()(pObject,itLB->first) )
    {
        // Store in the list.
        mObjectNodes.push_back( pObject );
        mObjectNodeLUT.insert( itLB, EntryLUT::value_type( pObject, --mObjectNodes.end() ) );

        // Also store in render class list.
        RenderClass & Class = mRenderClasses[pObject->getRenderClassId()];
        Class.objectNodes.push_back( pObject );
        Class.objectNodeLUT[ pObject ] = --Class.objectNodes.end();

        // Add to the combined bounding box for the entire render class.
        const cgBoundingBox & Bounds = pObject->getBoundingBox();
        Class.combinedBounds.addPoint( Bounds.min );
        Class.combinedBounds.addPoint( Bounds.max );

        // Update last modified frame timer
        mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();

        // Added
        return true;

    } // End if does not exist.

    // Nothing added
    return false;
}

//-----------------------------------------------------------------------------
//  Name : removeVisibleObject ()
/// <summary>
/// Remove the specified object from this visibility set (including material
/// and render class lists).
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::removeVisibleObject( cgObjectNode * pObject )
{
    EntryLUT::iterator itObject = mObjectNodeLUT.find( pObject );
    if ( itObject != mObjectNodeLUT.end() )
    {
        // Remove from the main object node list.
        mObjectNodes.erase( itObject->second );
        mObjectNodeLUT.erase( itObject );

        // Remove from the render class list.
        RenderClassMap::iterator itClass = mRenderClasses.find( pObject->getRenderClassId() );
        if ( itClass != mRenderClasses.end() )
        {
            RenderClass & renderClass = itClass->second;
            EntryLUT::iterator itClassObject = renderClass.objectNodeLUT.find( pObject );
            if ( itClassObject != renderClass.objectNodeLUT.end() )
            {
                renderClass.objectNodes.erase( itClassObject->second );
                renderClass.objectNodeLUT.erase( itClassObject );

                // Remove object from all materials. TODO: This is temporary.
                MaterialBatchMap::iterator itMaterial;
                for ( itMaterial = renderClass.materials.begin(); itMaterial != renderClass.materials.end();  )
                {
                    MaterialBatch & batch = itMaterial->second;
                    EntryLUT::iterator itMaterialObject = batch.objectNodeLUT.find( pObject );
                    if ( itMaterialObject != batch.objectNodeLUT.end() )
                    {
                        batch.objectNodes.erase( itMaterialObject->second );
                        batch.objectNodeLUT.erase( itMaterialObject );

                        // Remove material batch if it is now empty.
                        if ( batch.objectNodes.empty() )
                            itMaterial = renderClass.materials.erase( itMaterial );
                        else
                            ++itMaterial;

                    } // End if found object
                    else
                        ++itMaterial;

                } // Next material

                // Remove render class if it is now empty.
                if ( renderClass.objectNodes.empty() && renderClass.materials.empty() )
                    mRenderClasses.erase( itClass );
            
            } // End if found object

        } // End if found class

        // Update last modified frame timer
        mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();

    } // End if found object
}

//-----------------------------------------------------------------------------
//  Name : addVisibleLight ()
/// <summary>
/// If a light is deemed to be visible, this method should be called
/// in order for the object to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::addVisibleLight( cgObjectNode * pLight )
{
    /*// Objects must have a valid reference identifier in order to be considered.
    if ( !pLight || !pLight->GetReferenceId() )
        return;*/
    
    // Skip object if it already exists (user lower bound for existence test, which will also 
    // act as the insert point if no precise match was found).
    EntryLUT::iterator itLB = mLightLUT.lower_bound( pLight );
    if ( itLB == mLightLUT.end() || mLightLUT.key_comp()(pLight,itLB->first) )
    {
        // Store in the list (will automatically overwrite if already exists).
        mLights.push_back( pLight );
        mLightLUT.insert( itLB, EntryLUT::value_type( pLight, --mLights.end() ) );

        // Update last modified frame timer
        mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();

        // Added
        return true;

    } // End if does not exist.

    // Nothing added
    return false;
}

//-----------------------------------------------------------------------------
//  Name : removeVisibleLight ()
/// <summary>
/// Remove the specified light from this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::removeVisibleLight( cgObjectNode * pLight )
{
    EntryLUT::iterator itLight = mLightLUT.find( pLight );
    if ( itLight != mLightLUT.end() )
    {
        // Remove from the main light list.
        mLights.erase( itLight->second );
        mLightLUT.erase( itLight );

        // Update last modified frame timer
        mLastModifiedFrame = cgTimer::getInstance()->getFrameCounter();
   
    } // End if found light
}

//-----------------------------------------------------------------------------
//  Name : addVisibleMaterial ()
/// <summary>
/// If an object is deemed to be visible, this method should be called
/// for each material that it utilizes in order for us to record the need to
/// potentially render that object in conjunction with the specified material.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::addVisibleMaterial( const cgMaterialHandle & hMaterial, cgObjectNode * pObject )
{
    MaterialBatch & Material = mRenderClasses[pObject->getRenderClassId()].materials[ hMaterial ];

    // Skip material if it already exists (user lower bound for existence test, which will also 
    // act as the insert point if no precise match was found).
    EntryLUT::iterator itLB = Material.objectNodeLUT.lower_bound( pObject );
    if ( itLB == Material.objectNodeLUT.end() || Material.objectNodeLUT.key_comp()(pObject,itLB->first) )
    {
        Material.objectNodes.push_back( pObject );
        Material.objectNodeLUT.insert( itLB, EntryLUT::value_type( pObject, --Material.objectNodes.end() ) );

        // Add to the combined bounding box for the entire material match.
        const cgBoundingBox & Bounds = pObject->getBoundingBox();
        Material.combinedBounds.addPoint( Bounds.min );
        Material.combinedBounds.addPoint( Bounds.max );

        // Added
        return true;
    
    } // End if not existing

    // Nothing added
    return false;

    /*// Objects must have a valid reference identifier in order to be considered.
    if ( pObject == CG_NULL || pObject->GetReferenceId() == 0 || hMaterial.IsValid() == false )
        return;
    
    // Store in the list (will automatically overwrite if already exists).
    mRenderClasses[pObject->getRenderClassId()].materials[hMaterial].insert( pObject );*/
}

//-----------------------------------------------------------------------------
//  Name : addVisibleLeaf ()
/// <summary>
/// If a spatial tree leaf is deemed to be visible, this method should
/// be called in order for it to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleLeaf( cgSpatialTreeInstance * pTree, cgSpatialTreeLeaf * pLeaf )
{
    // Store in the list (will automatically overwrite if already exists)
    (mTreeLeaves[pTree]).insert( pLeaf );
}

//-----------------------------------------------------------------------------
//  Name : addVisibleGroup ()
/// <summary>
/// If a data group (associated with a given context object) is deemed to be 
/// visible, this method should be called in order for it to be marked 
/// as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleGroup( void * pContext, cgInt32 nDataGroupId )
{
    // Store in the list (will automatically overwrite if already exists)
    (mAssociatedGroups[pContext]).insert( nDataGroupId );
}

//-----------------------------------------------------------------------------
//  Name : clearVisibleGroups ()
/// <summary>
/// Clear out the list of visible groups associated with a given context object
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::clearVisibleGroups( void * pContext )
{
    mAssociatedGroups.erase( pContext );
}

//-----------------------------------------------------------------------------
//  Name : getVisibleObjects ()
/// <summary>
/// Retrieve the list of visible objects.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeList & cgVisibilitySet::getVisibleObjects()
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleObjects () (const overload)
/// <summary>
/// Retrieve the list of visible objects.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeList & cgVisibilitySet::getVisibleObjects() const
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleRenderClasses ()
/// <summary>
/// Retrieve the list of visible object render classes (and their associated 
/// objects batched by material).
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::RenderClassMap & cgVisibilitySet::getVisibleRenderClasses()
{
    return mRenderClasses;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleRenderClasses () (const overload)
/// <summary>
/// Retrieve the list of visible object render classes (and their associated 
/// objects batched by material).
/// </summary>
//-----------------------------------------------------------------------------
const cgVisibilitySet::RenderClassMap & cgVisibilitySet::getVisibleRenderClasses() const
{
    return mRenderClasses;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLights ()
/// <summary>
/// Retrieve the list of visible lights.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeList & cgVisibilitySet::getVisibleLights()
{
    return mLights;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLights () (const overload)
/// <summary>
/// Retrieve the list of visible lights.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeList & cgVisibilitySet::getVisibleLights() const
{
    return mLights;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLeaves ()
/// <summary>
/// Retrieve the list of visible leaves associated with the specified
/// spatial tree.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneLeafSet & cgVisibilitySet::getVisibleLeaves( cgSpatialTreeInstance * pTree )
{
    return mTreeLeaves[pTree];
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLeaves () (const overload)
/// <summary>
/// Retrieve the list of visible leaves associated with the specified
/// spatial tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneLeafSet & cgVisibilitySet::getVisibleLeaves( cgSpatialTreeInstance * pTree ) const
{
    static cgSceneLeafSet emptySet;
    TreeLeafMap::const_iterator itLeaf = mTreeLeaves.find( pTree );
    if ( itLeaf == mTreeLeaves.end() )
        return emptySet;
    return itLeaf->second;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleGroups ()
/// <summary>
/// Retrieve the list of visible data groups associated with the specified
/// context.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32Set & cgVisibilitySet::getVisibleGroups( void * pContext )
{
    return mAssociatedGroups[pContext];
}

//-----------------------------------------------------------------------------
//  Name : getVisibleGroups () (const overload)
/// <summary>
/// Retrieve the list of visible data groups associated with the specified
/// context.
/// </summary>
//-----------------------------------------------------------------------------
const cgInt32Set & cgVisibilitySet::getVisibleGroups( void * pContext ) const
{
    static cgInt32Set emptySet;
    AssociatedGroupMap::const_iterator itGroupList = mAssociatedGroups.find( pContext );
    if ( itGroupList == mAssociatedGroups.end() )
        return emptySet;
    return itGroupList->second;
}

//-----------------------------------------------------------------------------
//  Name : getVolume ()
/// <summary>
/// Retrieve the visibility volume / frustum used to construct this set.
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum & cgVisibilitySet::getVolume( )
{
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : getVolume ()
/// <summary>
/// Retrieve the visibility volume / frustum used to construct this set.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgVisibilitySet::getVolume( ) const
{
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if anything is contained in the visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isEmpty( ) const
{
    return ( mObjectNodes.empty() && mLights.empty() 
             && mTreeLeaves.empty() && mAssociatedGroups.empty() 
             && mRenderClasses.empty() );
}

//-----------------------------------------------------------------------------
//  Name : isObjectVisible ()
/// <summary>
/// Determine if the specified object is contained in this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isObjectVisible( cgObjectNode * object ) const
{
    return (mObjectNodeLUT.find( object ) != mObjectNodeLUT.end());
}

//-----------------------------------------------------------------------------
//  Name : isLightVisible ()
/// <summary>
/// Determine if the specified light is contained in this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isLightVisible( cgObjectNode * light ) const
{
    return (mLightLUT.find( light ) != mLightLUT.end());
}

//-----------------------------------------------------------------------------
//  Name : setSearchFlags ()
/// <summary>
/// Update the filtering flags that describe the types of objects that are
/// important to this visibility set. This method should be called before
/// visibility collection begins in order to avoid frame coherence problems.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::setSearchFlags( cgUInt32 flags )
{
    mSearchFlags = flags;
}

//-----------------------------------------------------------------------------
//  Name : getSearchFlags ()
/// <summary>
/// Retrieve the filtering flags that describe the types of objects that are
/// important to this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVisibilitySet::getSearchFlags() const
{
    return mSearchFlags;
}