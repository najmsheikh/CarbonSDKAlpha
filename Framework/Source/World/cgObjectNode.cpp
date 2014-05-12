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
// Name : cgObjectNode.cpp                                                   //
//                                                                           //
// Desc : This file houses the main object base class from which all system  //
//        and application object types should derive. These objects are      //
//        designed to be managed within a parent cgScene container.          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgObjectNode Module Includes
//-----------------------------------------------------------------------------
#include <World/cgObjectNode.h>
#include <World/cgObjectBehavior.h>
#include <World/cgWorld.h>
#include <World/cgWorldObject.h>
#include <World/cgSceneCell.h>
#include <World/cgScene.h>
#include <World/cgVisibilitySet.h>
#include <World/cgSphereTree.h>
#include <World/Objects/cgGroupObject.h>
#include <World/Objects/cgTargetObject.h>
#include <World/Elements/cgNavigationMeshElement.h>
#include <Physics/cgPhysicsController.h> // ToDo: Remove if no longer needed
#include <Physics/Bodies/cgRigidBody.h>
#include <Physics/Shapes/cgNullShape.h>
#include <Rendering/cgRenderDriver.h> // ToDo: Remove if cached buffer goes away.
#include <Resources/cgResourceManager.h> // ToDo: Remove if cached buffer goes away.
#include <Resources/cgConstantBuffer.h> // ToDo: Remove if cached buffer goes away.
#include <Resources/cgScript.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <System/cgMessageTypes.h>
#include <Math/cgMathUtility.h>

// Auto-create initial physics shape.
#include <World/Objects/Elements/cgBoxCollisionShapeElement.h>
#include <World/Objects/Elements/cgSphereCollisionShapeElement.h>
#include <World/Objects/Elements/cgCylinderCollisionShapeElement.h>
#include <World/Objects/Elements/cgCapsuleCollisionShapeElement.h>
#include <World/Objects/Elements/cgConeCollisionShapeElement.h>
#include <World/Objects/Elements/cgHullCollisionShapeElement.h>

// ToDo: Check error paths (i.e. setCellTransform() can fail).
//       Alternatively, move back to exceptions for DB access.

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgObjectNode::InputChannelLUT   cgObjectNode::mRegisteredInputChannels;
cgWorldQuery                    cgObjectNode::mNodeInsert;
cgWorldQuery                    cgObjectNode::mNodeDelete;
cgWorldQuery                    cgObjectNode::mNodeUpdateCell;
cgWorldQuery                    cgObjectNode::mNodeUpdateTransform;
cgWorldQuery                    cgObjectNode::mNodeUpdateOffsetTransform;
cgWorldQuery                    cgObjectNode::mNodeUpdateColor;
cgWorldQuery                    cgObjectNode::mNodeUpdateName;
cgWorldQuery                    cgObjectNode::mNodeUpdateInstanceIdentifier;
cgWorldQuery                    cgObjectNode::mNodeUpdateParent;
cgWorldQuery                    cgObjectNode::mNodeUpdateLevel;
cgWorldQuery                    cgObjectNode::mNodeUpdateGroup;
cgWorldQuery                    cgObjectNode::mNodeUpdatePhysicsProperties;
cgWorldQuery                    cgObjectNode::mNodeUpdateUpdateRate;
cgWorldQuery                    cgObjectNode::mNodeUpdateVisibility;
cgWorldQuery                    cgObjectNode::mNodeUpdateTargetReference;
cgWorldQuery                    cgObjectNode::mNodeClearCustomProperties;
cgWorldQuery                    cgObjectNode::mNodeRemoveCustomProperty;
cgWorldQuery                    cgObjectNode::mNodeUpdateCustomProperty;
cgWorldQuery                    cgObjectNode::mNodeInsertCustomProperty;
cgWorldQuery                    cgObjectNode::mNodeDeleteBehavior;
cgWorldQuery                    cgObjectNode::mNodeInsertBehavior;
cgWorldQuery                    cgObjectNode::mNodeLoadTransforms;
cgWorldQuery                    cgObjectNode::mNodeLoadCustomProperties;
cgWorldQuery                    cgObjectNode::mNodeLoadBehaviors;

///////////////////////////////////////////////////////////////////////////////
// cgObjectNode Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgObjectNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode::cgObjectNode( cgUInt32 referenceId, cgScene * scene ) : cgAnimationTarget( referenceId )
{
    // Initialize variables to sensible defaults
    mReferencedObject   = CG_NULL;
    mParentScene        = scene;
    mParentNode         = CG_NULL;
    mParentCell         = CG_NULL;
    mSceneTreeNode      = CG_NULL;
    mOwnerGroup         = CG_NULL;
    mTargetNode         = CG_NULL;
    mObjectClass        = _T("Object");
    mPhysicsController  = CG_NULL;
    mUpdateRate         = cgUpdateRate::Never;
    mLastDirtyFrame     = 0;
    mPendingUpdates     = 0;
    mTransformMethod    = cgTransformMethod::Standard;
    mPhysicsModel       = cgPhysicsModel::None;
    mSimulationQuality  = cgSimulationQuality::Default;
    mSelectionId        = -1;
    mNodeLevel          = 0;
    mPhysicsBody        = CG_NULL;
    mNavigationAgent    = CG_NULL;
    mRenderClassId      = 1;        // Automatically assign to the 'default' render class.
    mCustomProperties   = new cgPropertyContainer();
    mPendingUpdateFIFO  = CG_NULL;

    // Setup default flags.
    mFlags              = cgObjectNodeFlags::Visible;

    // Assign a default color to the node
    mColor = cgMathUtility::randomColor( );

    // Subscribe to necessary messaging groups.
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_ResourceManager );
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_System );
}

//-----------------------------------------------------------------------------
//  Name : cgObjectNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode::cgObjectNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgAnimationTarget( referenceId, init )
{
    // Ensure this is a valid operation
    if ( !init || !init->queryReferenceType( this->getReferenceType() ) )
        throw cgExceptions::ResultException( _T("Unable to clone. Specified node is of an incompatible type."), cgDebugSource() );

    // Initialize variables to sensible defaults
    mReferencedObject   = CG_NULL;
    mParentScene        = scene;
    mParentNode         = CG_NULL;
    mParentCell         = CG_NULL;
    mSceneTreeNode      = CG_NULL;
    mOwnerGroup         = CG_NULL;
    mTargetNode         = CG_NULL;
    mObjectClass        = init->mObjectClass;
    mPhysicsController  = CG_NULL; // ToDo: clone?
    mUpdateRate         = init->mUpdateRate;
    mLastDirtyFrame     = 0;
    mPendingUpdates     = 0;
    mTransformMethod    = cgTransformMethod::Standard;
    mPhysicsModel       = init->mPhysicsModel;
    mSimulationQuality  = init->mSimulationQuality;
    mSelectionId        = -1;
    mNodeLevel          = 0;
    mCellTransform      = initTransform;
    mLocalTransform     = initTransform;
    mOffsetTransform    = init->mOffsetTransform;
    mColor              = init->mColor;
    mCustomProperties   = new cgPropertyContainer(*init->mCustomProperties);
    mPhysicsBody        = CG_NULL;
    mNavigationAgent    = CG_NULL;
    mRenderClassId      = init->mRenderClassId;

    // Duplicate flags that are important to us.
    mFlags              = 0;
    mFlags             |= (init->mFlags & cgObjectNodeFlags::Visible);

    // Cached data.
    mWorldPivotTransform = init->mWorldPivotTransform;
    mWorldBounds         = init->mWorldBounds;

    // What cloning method is being employed?
    if ( initMethod == cgCloneMethod::Copy || initMethod == cgCloneMethod::DataInstance )
    {
        // Create a copy of the specified object.
        cgWorld * pWorld = scene->getParentWorld();
        mReferencedObject = pWorld->createObject( isInternalReference(), init->getObjectType(), initMethod, init->getReferencedObject() );
        if ( !mReferencedObject )
            throw cgExceptions::ResultException( _T("Failed to instantiate new object of required type."), cgDebugSource() );
    
    } // End if Copy | DataInstance
    else if ( initMethod == cgCloneMethod::ObjectInstance )
    {
        // Just instance the object itself.
        mReferencedObject = init->mReferencedObject;
    
    } // End if ObjectInstance

    // Clone target.
    if ( mParentScene && init->getTargetMethod() != cgNodeTargetMethod::NoTarget )
    {
        if ( (mTargetNode = (cgTargetNode*)mParentScene->createObjectNode( isInternalReference(), RTID_TargetObject, false ) ) )
        {
            cgTransform targetTransform;
            cgTransform::inverse( targetTransform, init->getWorldTransform() );
            cgTransform::multiply( targetTransform, init->getTargetNode()->getWorldTransform(), targetTransform );
            cgTransform::multiply( targetTransform, targetTransform, initTransform );
            mTargetNode->setWorldTransform( targetTransform );
            mTargetNode->setTargetingNode( this );
            mTargetNode->setMode( init->getTargetMethod() );
        
        } // End if created
    
    } // End if has targeting method

    // Add us as a reference to this object (don't increment true DB based
    // reference count if this is an internal node).
    mReferencedObject->addReference( this, isInternalReference() );
    
    // Listen for any changes made to the object
    mReferencedObject->registerEventListener( static_cast<cgWorldComponentEventListener*>(this) );

    // Subscribe to necessary messaging groups.
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_ResourceManager );
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_System );
}

//-----------------------------------------------------------------------------
//  Name : ~cgObjectNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode::~cgObjectNode()
{
    // Clean up object resources.
    dispose( false );

    // Finally release objects allocated in the constructor.
    if ( mCustomProperties )
        mCustomProperties->scriptSafeDispose();
    mCustomProperties = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgObjectNode::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Note: The correct way to remove a node is to delete it from the
    // scene using 'cgScene::deleteObjectNode()' which in turn triggers
    // the 'cgObjectNode::onNodeDeleted()' event method. This 'dispose()'
    // method is only intended to clean up during scene shut down or while
    // a specific node is being unloaded and as a result will only 'silently' 
    // detach itself from any necessary application components.

    // Paranoia: Although all pending updates should have been resolved
    // by this point under normal conditions, should the node remain
    // in the scene's pending update list for whatever reason there will
    // be a potential catastrophic fault. Make sure we are gone from this list!
    if ( mParentScene )
        mParentScene->resolvedNodeUpdates( this );

    // Release all allocated behaviors (scripted or otherwise)
    BehaviorArray::iterator itBehavior;
    for ( itBehavior = mBehaviors.begin(); itBehavior != mBehaviors.end(); ++itBehavior )
        (*itBehavior)->scriptSafeDispose();
    mBehaviors.clear();

    // Remove from name usage map
    setName( cgString::Empty );

    // Silently remove this node from any parent cell
    if ( mParentCell )
        mParentCell->removeNode( this );

    // And from any spatial tree
    if ( mSceneTreeNode )
        mParentScene->getSceneTree()->removeSphere( mSceneTreeNode );

    // ToDo: 9999 - Detaching silently may not be a good idea. Hinge joint as an example?
    // Silently detach all children from this object.
    cgObjectNodeList::iterator itChild;
    for ( itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
        (*itChild)->mParentNode = CG_NULL;
    mChildren.clear();

    // Also, if we have a parent, make sure we get detached from it 
    // in the hierarchy too.
    if ( mParentNode )
        mParentNode->mChildren.remove( this );

    // Silently detach us from any target node.
    if ( mTargetNode )
        mTargetNode->mTargetingNode = CG_NULL;

    // Destroy any physics body reference.
    if ( mPhysicsBody )
    {
        mPhysicsBody->unregisterEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );
        mPhysicsBody->removeReference( this );
    
    } // End if valid body

    // Destroy any navigation agent reference.
    if ( mNavigationAgent )
    {
        mNavigationAgent->unregisterEventListener( static_cast<cgNavigationAgentEventListener*>(this) );
        mNavigationAgent->removeReference( this );
    
    } // End if valid agent
    
    // Disconnect from our referenced object.
    if ( mReferencedObject )
    {
        mReferencedObject->unregisterEventListener( static_cast<cgWorldComponentEventListener*>(this) );
        mReferencedObject->removeReference( this, true );
    
    } // End if valid object
    
    // Release any physics controller that is still set
    if ( mPhysicsController )
        mPhysicsController->scriptSafeDispose();
    
    // Clear out any input channel data that may have been set.
    mInputChannels.clear();

    // Clear out old custom properties
    if ( mCustomProperties )
        mCustomProperties->clear();

    // Clear variables
    mReferencedObject  = CG_NULL;
    mSceneTreeNode     = CG_NULL;
    mTargetNode        = CG_NULL;
    mParentCell        = CG_NULL;
    mParentNode        = CG_NULL;
    mParentScene       = CG_NULL;
    mPhysicsController = CG_NULL;
    mPhysicsBody       = CG_NULL;
    mNavigationAgent   = CG_NULL;

    // Dispose base class if requested.
    if ( disposeBase )
    {
        cgAnimationTarget::dispose( true );
        cgWorldComponentEventListener::dispose( true );
    
    } // End if dispose base
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : unload ()
/// <summary>
/// Unload the node and remove it from the scene without physically deleting
/// it from the database.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::unload( )
{
    unload( false );
}

//-----------------------------------------------------------------------------
//  Name : unload ()
/// <summary>
/// Unload the node and remove it from the scene without physically deleting
/// it from the database. Optionally unload all children.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::unload( bool unloadChildren )
{
    // Is unloading delayed?
    if ( mFlags & cgObjectNodeFlags::DelayUnload || (mParentScene && mParentScene->isUpdating()) )
    {
        // Mark for unloading.
        nodeUpdated( cgDeferredUpdateFlags::Unload, 0 );

        // Unloading children?
        if ( unloadChildren )
        {
            cgObjectNodeList::iterator itChild;
            cgObjectNodeList childNodes = getChildren();
            for ( itChild = childNodes.begin(); itChild != childNodes.end(); ++itChild )
                (*itChild)->unload( true );

        } // End if unload children

    } // End if delayed
    else
    {
        // Otherwise, unload immediately.
        if ( mParentScene )
            mParentScene->unloadObjectNode( this, unloadChildren );

        // Note: 'this' pointer is now unsafe. Do not use after a call
        // to 'unloadObjectNode()'.
    
    } // End if !delayed
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Base update process simply calls the behavior update functions.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::update( cgFloat timeDelta )
{
    // Has any time elapsed? Passing a delta time of 0 
    // may cause problems.
    if ( timeDelta > 0 && (!mParentScene || mParentScene->isUpdatingEnabled()) )
    {
        // We don't process anything by default, simply pass through to the behaviors
        BehaviorArray::iterator itBehavior;
        for ( itBehavior = mBehaviors.begin(); itBehavior != mBehaviors.end(); ++itBehavior )
        {
            if ( *itBehavior )
                (*itBehavior)->onUpdate( timeDelta );
        
        } // Next Behavior

    } // End if elapsed
}

//-----------------------------------------------------------------------------
//  Name : computeLevelOfDetail() (Virtual)
/// <summary>
/// Called just prior to the scene update process to allow the object to 
/// compute its required detail level for subsequent rendering / processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::computeLevelOfDetail( cgCameraNode * camera )
{
    // Nothing in base class.
}

//-----------------------------------------------------------------------------
//  Name : getRenderTransformBuffer ()
/// <summary>
/// Retrieve the constant buffer containing a cached version of this object's
/// world-space rendering transformations.
/// </summary>
//-----------------------------------------------------------------------------
const cgConstantBufferHandle & cgObjectNode::getRenderTransformBuffer( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mWorldTransformBuffer;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Base render method is responsible for managing the render queue,
/// checking visibility states etc.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::render( cgCameraNode * camera, cgVisibilitySet * visibilityData )
{
    cgAssert( mParentScene != CG_NULL );

    // If the object is hidden, or marked as invisible, signal "do not draw" by returning false.
    if ( !isRenderable() )
        return false;

    // Pass on this message to any referenced object.
    if ( mReferencedObject )
        return mReferencedObject->render( camera, visibilityData, this );

    // Derived class should draw this object
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getBoundingBox ()
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in world space.
/// </summary>
//-----------------------------------------------------------------------------
const cgBoundingBox & cgObjectNode::getBoundingBox( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::BoundingBox );
    return mWorldBounds;

    /*// Does the world space bounding box need to be recomputed?
    if ( m_bBoundsDirty )
    {
        // Retrieve local bounding box and transform it.
        mWorldBounds = getLocalBoundingBox();
        mWorldBounds.transform( getWorldTransform( false ) );
        m_bBoundsDirty = false;
    
    } // End if recompute

    // Return our cached copy.
    return mWorldBounds;*/
}

//-----------------------------------------------------------------------------
//  Name : getBoundingSphere ()
/// <summary>
/// Retrieve the bounding sphere for this node (encompassing the bounding box
/// of its referenced object by default) as it exists in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingSphere cgObjectNode::getBoundingSphere( )
{
    const cgBoundingBox & boundingBox = getBoundingBox();
    return cgBoundingSphere( boundingBox.getCenter(), cgVector3::length(boundingBox.getExtents()) );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox ()
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgObjectNode::getLocalBoundingBox( )
{
    // If no object is available, the local space bounding box
    // is degenerate but positioned at the node's 'local' origin
    // (always <0,0,0>).
    return ( mReferencedObject ) ? mReferencedObject->getLocalBoundingBox() : cgBoundingBox::Empty;
}

//-----------------------------------------------------------------------------
// Name : getObjectSize ( ) (Virtual)
/// <summary>
/// Retrieve the approximate world space size of this node, used for certain
/// LOD computations that may be applied. If the size is not / cannot be known
/// return FLT_MAX to effectively disable culling.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgObjectNode::getObjectSize( )
{
    return FLT_MAX;
}

//-----------------------------------------------------------------------------
//  Name : renderSubset ()
/// <summary>
/// Render the object one material at a time (material batching).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::renderSubset( cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgMaterialHandle & material )
{
    cgAssert( mParentScene != CG_NULL );

    // If the object is hidden, or marked as invisible, signal "do not draw" by returning false.
    if ( !isRenderable() )
        return false;
    
    // Pass on this message to any referenced object.
    if ( mReferencedObject )
        return mReferencedObject->renderSubset( camera, visibilityData, this, material );

    // Derived class should draw this object
    return true;
}

//-----------------------------------------------------------------------------
//  Name : addBehavior()
/// <summary>
/// Attach a scripted, or pre-registered behavior to this scene object.
/// Note : Returns an integer index value to the current behavior so that it 
///        the behavior can be accessed at a later time.
/// Note : The index value for the behavior may change if other behaviors
///        are removed at a later time.
/// Note : The scene object will assume ownership of the allocated behavior
///        and will release it once the object is destroyed.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgObjectNode::addBehavior( const cgString & behavior )
{
    // Load the behavior script
    cgObjectBehavior * newBehavior = new cgObjectBehavior( );
    if ( !newBehavior->initialize( mParentScene->getResourceManager(), behavior, cgString::Empty ) )
    {
        newBehavior->scriptSafeDispose();
        return -1;
    
    } // End if failed to initialize

    // Pass through to standard method.
    cgInt32 result = addBehavior( newBehavior );
    if ( result < 0 )
        newBehavior->scriptSafeDispose();

    // Success?
    return result;
}

//-----------------------------------------------------------------------------
//  Name : addBehavior()
/// <summary>
/// Attach a behavior to this scene object.
/// Note : Returns an integer index value to the current behavior so that it 
///        the behavior can be accessed at a later time.
/// Note : The index value for the behavior may change if other behaviors
///        are removed at a later time.
/// Note : The scene object will assume ownership of the allocated behavior
///        and will release it once the object is destroyed.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgObjectNode::addBehavior( cgObjectBehavior * behavior )
{
    // Validate parameters
    if ( !behavior )
        return -1;

    // Get the load order of the behavior at the end of our
    // current behavior array, and set that value + 1 to the 
    // new behavior.
    if ( !mBehaviors.empty() )
        behavior->setLoadOrder( mBehaviors.back()->getLoadOrder() + 1 );

    // Insert into the database if necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Setup common behavior data.
        mNodeInsertBehavior.bindParameter( 1, mReferenceId );
        mNodeInsertBehavior.bindParameter( 2, behavior->getLoadOrder() );
        
        // Is this a scripted behavior?
        if ( behavior->isScripted() )
        {
            cgScriptHandle scriptHandle = behavior->getScript();
            mNodeInsertBehavior.bindParameter( 3, (cgUInt8)0 );         // Type
            mNodeInsertBehavior.bindParameter( 4, scriptHandle->getInputStream().getName() );
            mNodeInsertBehavior.bindParameter( 5, cgString::Empty );    // TypeName (native only)
            mNodeInsertBehavior.bindParameter( 6, CG_NULL, 0 );         // Data
        
        } // End if scripted
        else
        {
            mNodeInsertBehavior.bindParameter( 3, (cgUInt8)1 );         // Type
            mNodeInsertBehavior.bindParameter( 4, cgString::Empty );    // Script file (scripted only)
            mNodeInsertBehavior.bindParameter( 5, cgString::Empty );    // TypeName (native only) // TODO
            mNodeInsertBehavior.bindParameter( 6, CG_NULL, 0 );         // Data

        } // End if native
            
        // Insert new behavior.
        if ( !mNodeInsertBehavior.step( true ) )
        {
            cgString error;
            mNodeInsertBehavior.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert new behavior for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return -1;
        
        } // End if failed

        // Behavior was serialized.
        behavior->setUserId( mNodeInsertBehavior.getLastInsertId() );
    
    } // End if serialize

    // Notify behavior that we're now it's owner
    behavior->setParentObject( this );

    // Add this behavior to the list
    mBehaviors.push_back( behavior );
    
    // Return the index to this new behavior
    return (cgInt32)mBehaviors.size() - 1;
}

//-----------------------------------------------------------------------------
//  Name : getBehaviorCount()
/// <summary>
/// Return the total number of behaviors attached to this object.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgObjectNode::getBehaviorCount( ) const
{
    return (cgInt32)mBehaviors.size();
}

//-----------------------------------------------------------------------------
//  Name : getBehavior()
/// <summary>
/// Retrieve the specified behavior attached to this object
/// </summary>
//-----------------------------------------------------------------------------
cgObjectBehavior * cgObjectNode::getBehavior( cgInt32 index )
{
    // Validate parameters
    if ( index < 0 || index >= (cgInt32)mBehaviors.size() )
        return CG_NULL;

    // Return the specified behavior
    return mBehaviors[ index ];
}

//-----------------------------------------------------------------------------
//  Name : removeBehavior()
/// <summary>
/// Remove a behavior from this scene object and optionally destroy it.
/// Note : This method will return false if the specified behavior was not
/// found in the attached behavior list. In this case, the specified
/// behavior will NOT be destroyed if requested.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::removeBehavior( cgObjectBehavior * behavior, bool destroy /* = true */ )
{
    // Attempt to find the behavior
    BehaviorArray::iterator itBehavior = std::find( mBehaviors.begin(), mBehaviors.end(), behavior );

    // Did we find it?
    if ( itBehavior != mBehaviors.end() )
    {
        // Remove from the database
        if ( shouldSerialize() && behavior->getUserId() )
        {
            mNodeDeleteBehavior.bindParameter( 1, behavior->getUserId() );
            if ( !mNodeDeleteBehavior.step( true ) )
            {
                cgString error;
                mNodeDeleteBehavior.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to remove behavior '0x%x' from the database for object node '0x%x'. Error: %s\n"), behavior->getUserId(), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if remove from database

        // Remove it
        mBehaviors.erase( itBehavior );

        // We are no longer this behavior's owner
        if ( behavior )
            behavior->setParentObject( CG_NULL );

        // Optionally destroy the existing behavior
        if ( behavior && destroy )
            behavior->scriptSafeDispose();

        // Success!
        return true;

    } // End if behavior found

    // Behavior not found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : removeBehavior()
/// <summary>
/// Remove a behavior from this scene object and optionally destroy it.
/// Note : This method will return false if the specified behavior was not
/// found in the attached behavior list. In this case, the specified
/// behavior will NOT be destroyed if requested.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::removeBehavior( cgInt32 index, bool destroy /* = true */ )
{
    cgObjectBehavior * behavior = getBehavior( index );
    if ( !behavior )
        return false;

    // Pass through to the standard method.
    return removeBehavior( behavior, destroy );
}

//-----------------------------------------------------------------------------
//  Name : setPhysicsController ()
/// <summary>
/// Replace the current object physics controller with the one specified. Any
/// prior assigned controller will be automatically destroyed. To preven this,
/// use the alternative overload for this method.
/// Note : Any physics controller still set to the object when it is destroyed
/// will also be released.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setPhysicsController( cgPhysicsController * controller )
{
    setPhysicsController( controller, true );
}

//-----------------------------------------------------------------------------
//  Name : setPhysicsController () (Virtual)
/// <summary>
/// Replace the current object physics controller with the one specified. The
/// prior controller can be optionally destroyed by specifying true to the
/// final parameter. If the caller opts not to destroy the prior controller,
/// this method returns its pointer so that it can be released,  or saved for 
/// later use.
/// Note : Any physics controller still set to the object when it is destroyed
/// will also be released.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController * cgObjectNode::setPhysicsController( cgPhysicsController * controller, bool destroyOld )
{
    cgPhysicsController * oldController = mPhysicsController;

    // Is this a no-op?
    if ( oldController && oldController == controller )
        return CG_NULL;

    // Set new controller, and notify it that we're now it's owner
    mPhysicsController = controller;
    if ( controller )
        controller->setParentObject( this );

    // If the old controller is to be destroyed, do so now.
    if ( oldController && destroyOld )
    {
        oldController->scriptSafeDispose();
        oldController = CG_NULL;

    } // End if destroy old controller
    else if ( oldController )
        oldController->setParentObject( CG_NULL );

    // Return the old controller
    return oldController;
}

//-----------------------------------------------------------------------------
//  Name : getPhysicsController()
/// <summary>
/// Retrieve the current physics controller assigned to the object
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController * cgObjectNode::getPhysicsController( )
{
    return mPhysicsController;
}

//-----------------------------------------------------------------------------
//  Name : getPhysicsBody()
/// <summary>
/// Retrieve the physics body used to represent this object node during scene
/// dynamics simulation.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsBody * cgObjectNode::getPhysicsBody( ) const
{
    return mPhysicsBody;
}

//-----------------------------------------------------------------------------
//  Name : getName() (Virtual)
/// <summary>
/// Retrieve the name of this instance of the specified object node.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgObjectNode::getName( ) const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : setName()
/// <summary>
/// Set the name of this instance of the specified object node.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setName( const cgString & name )
{
    // Change the name if this is allowed
    if ( canSetName() )
    {
        // Update database reference only if we are not disposing
        // or are in sandbox mode.
        if ( shouldSerialize() )
        {
            prepareQueries();
            mNodeUpdateName.bindParameter( 1, name );
            mNodeUpdateName.bindParameter( 2, mReferenceId );
            if ( !mNodeUpdateName.step( true ) )
            {
                cgString error;
                mNodeUpdateName.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update name for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if exists in DB

        // First, remove the currently existing name from the usage map (uses reference counting scheme)
        if ( mParentScene && !mName.empty() )
        {
            cgScene::NameUsageMap & nameUsage = mParentScene->getNameUsage();
            cgString nameKey = cgString::toLower( mName );
            cgScene::NameUsageMap::iterator itName = nameUsage.find( nameKey );
            if ( itName != nameUsage.end() )
            {
                // Update usage map
                itName->second = itName->second - 1;
                if ( itName->second == 0 )
                    nameUsage.erase( nameKey );
                
            } // End if exists in map

        } // End if has existing name

        // Store new name
        mName = name;

        // Add a reference to this new name to the usage map
        if ( mParentScene && !name.empty() )
        {
            cgScene::NameUsageMap & nameUsage = mParentScene->getNameUsage();
            cgString nameKey = cgString::toLower( name );
            cgScene::NameUsageMap::iterator itName = nameUsage.find( nameKey );
            if ( itName != nameUsage.end() )
                itName->second = itName->second + 1;
            else
                nameUsage[nameKey] = 1;

        } // End if has existing name

        // Notify scene listeners.
        if ( mParentScene )
        {
            // Notify that the node name has changed.
            mParentScene->onNodeNameChange( &cgNodeUpdatedEventArgs( mParentScene, this ) );

            // If we have a target node, notify that its name changed too
            // (target node's name is based on its attached node).
            if ( mTargetNode )
                mParentScene->onNodeNameChange( &cgNodeUpdatedEventArgs( mParentScene, mTargetNode ) );
        
        } // End if in scene.

    } // End if allow name change

    // No error occurred.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getObjectClass()
/// <summary>
/// Retrieve the user defined string that allows this object to be 
/// categorized with similar.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgObjectNode::getObjectClass( ) const
{
    return mObjectClass;
}

//-----------------------------------------------------------------------------
//  Name : getRenderClassId()
/// <summary>
/// Retrieve the identifier of the user defined string used to identify the
/// object's rendering category / class.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgObjectNode::getRenderClassId( ) const
{
    return mRenderClassId;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ObjectNode )
        return true;

    // Supported by base?
    return cgAnimationTarget::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getObjectType()
/// <summary>
/// Get the unique identifier that describes the type of object being
/// referenced by this node.
/// </summary>
//-----------------------------------------------------------------------------
const cgUID & cgObjectNode::getObjectType( ) const
{
    static const cgUID Empty = {0};
    if ( !mReferencedObject )
        return Empty;
    return mReferencedObject->getReferenceType();
}

//-----------------------------------------------------------------------------
//  Name : queryObjectType ()
/// <summary>
/// Allows the application to determine if the world object referenced by
/// this node supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::queryObjectType( const cgUID& type ) const
{
    if ( !mReferencedObject )
        return false;
    return mReferencedObject->queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::processMessage( cgMessage * message )
{
    // We don't process anything by default, simply pass through to the behaviors
    BehaviorArray::iterator itBehavior;
    for ( itBehavior = mBehaviors.begin(); itBehavior != mBehaviors.end(); ++itBehavior )
    {
        if ( (*itBehavior)->processMessage( message ) )
            return true;
    
    } // Next Behavior

    // Message was not processed
    return cgAnimationTarget::processMessage( message );
}

//-----------------------------------------------------------------------------
//  Name : setTransformMethod()
/// <summary>
/// Set the approach to use when applying transformations to the node (i.e.
/// adjust pivot only, don't affect children and so on).
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setTransformMethod( cgTransformMethod::Base method )
{
    // Separation of pivot and object transform allowed?
    if ( !canAdjustPivot() )
    {
        // Filter disallowed methods.
        if ( method == cgTransformMethod::PivotOnly ||
             method == cgTransformMethod::ObjectOnly )
             return;
    
    } // End if

    mTransformMethod = method;
}

//-----------------------------------------------------------------------------
//  Name : getTransformMethod()
/// <summary>
/// Determine the approach currently in use when applying transformations to
/// the node (i.e. adjust pivot only, don't affect children and so on).
/// </summary>
//-----------------------------------------------------------------------------
cgTransformMethod::Base cgObjectNode::getTransformMethod( ) const
{
    return mTransformMethod;
}

//-----------------------------------------------------------------------------
// Name : getSimulationQuality ( ) (Virtual)
/// <summary>
/// Retrieve the currently configured quality of the physics simulation that
/// applies if dynamics processing is enabled for this object node.
/// </summary>
//-----------------------------------------------------------------------------
cgSimulationQuality::Base cgObjectNode::getSimulationQuality( ) const
{
    return mSimulationQuality;
}

//-----------------------------------------------------------------------------
// Name : setSimulationQuality ( ) (Virtual)
/// <summary>
/// Set the currently configured quality of the physics simulation that
/// applies if dynamics processing is enabled for this object node.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setSimulationQuality( cgSimulationQuality::Base quality )
{
    // Update database reference only if we are not disposing
    // or are in sandbox mode.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdatePhysicsProperties.bindParameter( 1, (cgUInt32)mPhysicsModel );
        mNodeUpdatePhysicsProperties.bindParameter( 2, (cgUInt32)quality );
        mNodeUpdatePhysicsProperties.bindParameter( 3, mReferenceId );
        if ( !mNodeUpdatePhysicsProperties.step( true ) )
        {
            cgString error;
            mNodeUpdatePhysicsProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update physics properties for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed

    } // End if exists in DB

    // Update local value.
    mSimulationQuality = quality;

    // If we're switching to CCD mode, enable this state on any
    // existing rigid body.
    if ( mPhysicsBody )
        mPhysicsBody->enableContinuousCollision( (quality == cgSimulationQuality::CCD) );

}

//-----------------------------------------------------------------------------
// Name : getPhysicsModel ( ) (Virtual)
/// <summary>
/// Retrieve the model that should be used when simulating physics for this
/// object. The model may dictate the types of sub-elements that are available
/// and the properties that can be defined.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsModel::Base cgObjectNode::getPhysicsModel( ) const
{
    return mPhysicsModel;
}

//-----------------------------------------------------------------------------
// Name : setPhysicsModel ( ) (Virtual)
/// <summary>
/// Set the model that should be used when simulating physics for this
/// object. The model may dictate the types of sub-elements that are available
/// and the properties that can be defined.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setPhysicsModel( cgPhysicsModel::Base model )
{
    setPhysicsModel( model, cgDefaultPhysicsShape::Auto );
}

//-----------------------------------------------------------------------------
// Name : setPhysicsModel ( ) (Virtual)
/// <summary>
/// Set the model that should be used when simulating physics for this
/// object. The model may dictate the types of sub-elements that are available
/// and the properties that can be defined.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setPhysicsModel( cgPhysicsModel::Base model, cgDefaultPhysicsShape::Base defaultShape )
{
    // Update database reference only if we are not disposing
    // or are in sandbox mode.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdatePhysicsProperties.bindParameter( 1, (cgUInt32)model );
        mNodeUpdatePhysicsProperties.bindParameter( 2, (cgUInt32)mSimulationQuality );
        mNodeUpdatePhysicsProperties.bindParameter( 3, mReferenceId );
        if ( !mNodeUpdatePhysicsProperties.step( true ) )
        {
            cgString strError;
            mNodeUpdatePhysicsProperties.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update physics properties for object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if exists in DB

    // Update local value.
    mPhysicsModel = model;

    // If we're switching to rigid-body model, and we don't currently have
    // a collision shape, automatically assign an appropriate one.
    cgBoundingBox objectBounds = getLocalBoundingBox();
    if ( model != cgPhysicsModel::None && model != cgPhysicsModel::CollisionOnly && objectBounds.isPopulated() )
    {
        const cgObjectSubElementArray & aCollisionShapes = mReferencedObject->getSubElements( OSECID_CollisionShapes );
        if ( aCollisionShapes.empty() )
        {
            if ( defaultShape == cgDefaultPhysicsShape::Auto )
            {
                // Guess an appropriate initial shape from the name of the node.
                cgString name = cgString::toLower(getName());
                if ( name.find( _T("cylinder") ) != cgString::npos ||
                     name.find( _T("barrel") ) != cgString::npos ||
                     name.find( _T("column") ) != cgString::npos )            
                    mReferencedObject->createSubElement( OSECID_CollisionShapes, RTID_CylinderCollisionShapeElement );
                else if ( name.find( _T("capsule") ) != cgString::npos )
                    mReferencedObject->createSubElement( OSECID_CollisionShapes, RTID_CapsuleCollisionShapeElement );
                else if ( name.find( _T("cone") ) != cgString::npos )
                    mReferencedObject->createSubElement( OSECID_CollisionShapes, RTID_ConeCollisionShapeElement );
                else if ( name.find( _T("sphere") ) != cgString::npos ||
                          name.find( _T("ball") ) != cgString::npos ||
                          name.find( _T("moon") ) != cgString::npos ||
                          name.find( _T("planet") ) != cgString::npos ||
                          name.find( _T("earth") ) != cgString::npos )
                    mReferencedObject->createSubElement( OSECID_CollisionShapes, RTID_SphereCollisionShapeElement );
                else
                    mReferencedObject->createSubElement( OSECID_CollisionShapes, RTID_BoxCollisionShapeElement );

            } // End if Auto
            else
            {
                cgUID shapeType = RTID_BoxCollisionShapeElement;
                switch ( defaultShape )
                {
                    case cgDefaultPhysicsShape::Sphere:
                        shapeType = RTID_SphereCollisionShapeElement;
                        break;
                    case cgDefaultPhysicsShape::Cylinder:
                        shapeType = RTID_CylinderCollisionShapeElement;
                        break;
                    case cgDefaultPhysicsShape::Cone:
                        shapeType = RTID_ConeCollisionShapeElement;
                        break;
                    case cgDefaultPhysicsShape::Capsule:
                        shapeType = RTID_CapsuleCollisionShapeElement;
                        break;
                    case cgDefaultPhysicsShape::ConvexHull:
                        shapeType = RTID_HullCollisionShapeElement;
                        break;

                } // End Switch Shape

                // Create the sub element.
                mReferencedObject->createSubElement( OSECID_CollisionShapes, shapeType);

            } // End if other
        
        } // End if no collision shapes

    } // End if physics object
        
    cgToDo( "Physics", "Delete shapes if we're switching to 'None' or 'CollisionOnly' and the object is only referenced by this node?" );
    cgToDo( "Physics", "Consider the above more. If we always do this, it would be impossible to set up shapes but temporarily disable dynamics. This may be *necessary* for CollisionOnly however." );

    // Rebuild the physics body appropriately.
    buildPhysicsBody();
}

//-----------------------------------------------------------------------------
//  Name : enableNavigation ()
/// <summary>
/// Allow this object node to act as a navigation agent, such that it can
/// autonomously navigate through the scene using the 'navigateTo()'
/// method. Specify 'NULL' to the 'params' argument to disable navigation
/// for this node.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::enableNavigation( const cgNavigationAgentCreateParams * params )
{
    // Remove any prior navigation agent.
    if ( mNavigationAgent )
    {
        mNavigationAgent->unregisterEventListener( static_cast<cgNavigationAgentEventListener*>(this) );
        mNavigationAgent->removeReference( this );
        mNavigationAgent = CG_NULL;
    
    } // End if exists

    // If new parameters are specified, create a new agent.
    if ( params )
    {
        // Retrieve the list of navigation meshes currently defined
        // within the parent scene, and find one with properties that
        // most closely match the requested agent parameters.
        cgFloat bestDifference = FLT_MAX;
        cgNavigationMeshElement * element = CG_NULL;
        const cgSceneElementArray & elements = mParentScene->getSceneElementsByType( RTID_NavigationMeshElement );
        for ( size_t i = 0; i < elements.size(); ++i )
        {
            // Closest match to radius so far?
            cgFloat difference = fabsf( params->agentRadius - ((cgNavigationMeshElement*)elements[i])->getParameters().agentRadius );
            if ( difference < bestDifference )
            {
                bestDifference = difference;
                element = (cgNavigationMeshElement*)elements[i];
            
            } // End if better

        } // Next element

        // If no navigation mesh element was found, display a warning.
        if ( !element )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("No appropriate navigation mesh was found while attempting to enable navigation for object node '0x%x'.\n"), mReferenceId );
            return false;
        
        } // End if no element

        // Create an agent for this node.
        mNavigationAgent = element->createAgent( *params, getPosition() );
        if ( !mNavigationAgent )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to create navigation agent while attempting to enable navigation for object node '0x%x'. Check supplied parameters.\n"), mReferenceId );
            return false;

        } // End if failed

        // Take ownership of the new agent and listen for events.
        mNavigationAgent->addReference( this );
        mNavigationAgent->registerEventListener( static_cast<cgNavigationAgentEventListener*>(this) );

        // Success!
        return true;

    } // End if new agent

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : navigateTo ()
/// <summary>
/// When navigation is enabled for this node, use this method to request the
/// object to autonomously navigate to the specified world space position if
/// possible.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::navigateTo( const cgVector3 & position )
{
    if ( !mNavigationAgent )
        return false;
    return mNavigationAgent->setMoveTarget( position, false );
}

//-----------------------------------------------------------------------------
//  Name : getNavigationAgent()
/// <summary>
/// Retrieve the navigation agent used to represent this object node during
/// scene navigation / pathfinding.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgent * cgObjectNode::getNavigationAgent( ) const
{
    return mNavigationAgent;
}

//-----------------------------------------------------------------------------
// Name : getNavigationAgentState()
/// <summary>
/// Determine the current state of the agent, i.e. whether it is currently 
/// in a position to receieve navigation requests.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationAgentState::Base cgObjectNode::getNavigationAgentState ( ) const
{
    if ( !mNavigationAgent )
        return cgNavigationAgentState::Invalid;
    return mNavigationAgent->getAgentState();
}

//-----------------------------------------------------------------------------
// Name : getNavigationTargetState()
/// <summary>
/// Determine the current state of the most recent navigation request.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationTargetState::Base cgObjectNode::getNavigationTargetState( ) const
{
    if ( !mNavigationAgent )
        return cgNavigationTargetState::None;
    return mNavigationAgent->getTargetState();
}

//-----------------------------------------------------------------------------
//  Name : isNavigationAgent ()
/// <summary>
/// Returns true if navigation has been enabled for this object node, allowing
/// it to autonomously navigate through the scene using the 'navigateTo()'
/// method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isNavigationAgent( ) const
{
    return (mNavigationAgent != CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : applyForce ()
/// <summary>
/// Apply a force directly to the center of mass of the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyForce( const cgVector3 & force )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyForce( force );
}

//-----------------------------------------------------------------------------
//  Name : applyForce ()
/// <summary>
/// Apply a force to the body at the specified world space location.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyForce( const cgVector3 & force, const cgVector3 & at )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyForce( force, at );
}

//-----------------------------------------------------------------------------
//  Name : applyImpulse ()
/// <summary>
/// Apply an impulse directly to the center of mass of the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyImpulse( const cgVector3 & impulse )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyImpulse( impulse );
}

//-----------------------------------------------------------------------------
//  Name : applyImpulse ()
/// <summary>
/// Apply an impulse to the body at the specified world space location.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyImpulse( const cgVector3 & impulse, const cgVector3 & at )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyImpulse( impulse, at );
}

//-----------------------------------------------------------------------------
//  Name : applyTorque()
/// <summary>
/// Apply a torque force to the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyTorque( const cgVector3 & torque )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyTorque( torque );
}

//-----------------------------------------------------------------------------
//  Name : applyTorqueImpulse()
/// <summary>
/// Apply a torque impulse to the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::applyTorqueImpulse( const cgVector3 & torqueImpulse )
{
    if ( mPhysicsBody )
        mPhysicsBody->applyTorqueImpulse( torqueImpulse );
}

//-----------------------------------------------------------------------------
//  Name : getVelocity ()
/// <summary>
/// Retrieve the current linear velocity of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getVelocity( ) const
{
    if ( mPhysicsBody )
        return mPhysicsBody->getVelocity();
    else if ( mNavigationAgent )
        return mNavigationAgent->getActualVelocity();
    else
        return cgVector3(0,0,0);
}

//-----------------------------------------------------------------------------
//  Name : getAngularVelocity ()
/// <summary>
/// Retrieve the current angular velocity (omega) of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getAngularVelocity( ) const
{
    return ( mPhysicsBody ) ? mPhysicsBody->getAngularVelocity() : cgVector3(0,0,0);
}

//-----------------------------------------------------------------------------
//  Name : setVelocity ()
/// <summary>
/// Set the current linear velocity of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setVelocity( const cgVector3 & v )
{
    if ( mPhysicsBody )
        mPhysicsBody->setVelocity( v );
}

//-----------------------------------------------------------------------------
//  Name : setAngularVelocity ()
/// <summary>
/// Set the current angular velocity (omega) of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setAngularVelocity( const cgVector3 & v )
{
    if ( mPhysicsBody )
        mPhysicsBody->setAngularVelocity( v );
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::getSubElementCategories( cgObjectSubElementCategory::Map & categoriesOut ) const
{
    // Ask the referenced object for its list of supported sub-elements.
    if ( !mReferencedObject->getSubElementCategories( categoriesOut ) )
        return false;

    // Remove support for collision shapes if we are not set to use
    // a physics model that supports them.
    if ( mPhysicsModel == cgPhysicsModel::None )
    {
        cgObjectSubElementCategory::Map::iterator itCategory = categoriesOut.find( OSECID_CollisionShapes );
        categoriesOut.erase( itCategory );
    
    } // End if non-dynamics model
        
    // Any sub-elements remaining?
    return (!categoriesOut.empty());
}

//-----------------------------------------------------------------------------
//  Name : supportsSubElement () (Virtual)
/// <summary>
/// Determine if the specified object sub element type is supported by this
/// object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::supportsSubElement( const cgUID & Category, const cgUID & Identifier ) const
{
    // We do not support collision shapes at all if we are not set to
    // use a physics model that supports them.
    if ( mPhysicsModel == cgPhysicsModel::None && Category == OSECID_CollisionShapes )
        return false;

    // Ask the referenced object if it supports.
    return mReferencedObject->supportsSubElement( Category, Identifier );
}

//-----------------------------------------------------------------------------
//  Name : move()
/// <summary>
/// Move the current position of the node by the specified amount.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::move( const cgVector3 & amount )
{
    // Pass through to setPosition so that any derived classes need not
    // override the 'move' method in order to catch this position change.
    setPosition( getPosition() + amount );
}

//-----------------------------------------------------------------------------
//  Name : move()
/// <summary>
/// Move the current position of the node by the specified amount.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::move( cgFloat x, cgFloat y, cgFloat z )
{
    move( cgVector3( x, y, z ) );
}

//-----------------------------------------------------------------------------
//  Name : moveLocal()
/// <summary>
/// Move the current position of the node by the specified amount relative to
/// its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::moveLocal( const cgVector3 & amount )
{
    cgVector3 vNewPos = getPosition();
    vNewPos += getXAxis() * amount.x;
    vNewPos += getYAxis() * amount.y;
    vNewPos += getZAxis() * amount.z;

    // Pass through to setPosition so that any derived classes need not
    // override the 'move' method in order to catch this position change.
    setPosition( vNewPos );
}

//-----------------------------------------------------------------------------
//  Name : moveLocal()
/// <summary>
/// Move the current position of the node by the specified amount in object
/// space.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::moveLocal( cgFloat x, cgFloat y, cgFloat z )
{
    moveLocal( cgVector3( x, y, z ) );
}

//-----------------------------------------------------------------------------
//  Name : setPosition()
/// <summary>
/// Set the current world space position of the node.
/// Note : This bypasses the physics system, so should really only be used
/// for initialization purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setPosition( const cgVector3 & position )
{
    cgTransform m = getCellTransform();

    // Get the parent cell's world origin (if any)
    // so that we can adjust the specified world space
    // position, transforming it such that it is relative
    // to the cell as required by our 'sliding world' system.
    cgVector3 vCellOrigin = ( mParentCell ) ? mParentCell->getWorldOrigin() : cgVector3(0,0,0);
    
    // Set new cell relative position
    m.setPosition( position.x - vCellOrigin.x,
                   position.y - vCellOrigin.y,
                   position.z - vCellOrigin.z );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : setPosition()
/// <summary>
/// Set the current world space position of the node.
/// Note : This bypasses the physics system, so should really only be used
/// for initialization purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setPosition( cgFloat x, cgFloat y, cgFloat z )
{
    setPosition( cgVector3( x, y, z ) );
}

//-----------------------------------------------------------------------------
//  Name : getPosition()
/// <summary>
/// Retrieve the current world space position of the node.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgObjectNode::getPosition( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mWorldPivotTransform.position();
    /*// Get the parent cell's world origin (if any) so that we can adjust the 
    // cell space position extracted from our transformation as required by our 
    // 'sliding world' system, transforming it such that it is relative to world space.
    if ( !mParentCell )
        return mCellTransform.position();
    else
        return mCellTransform.position() + mParentCell->getWorldOrigin();*/
}

//-----------------------------------------------------------------------------
//  Name : getOrientation()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternion cgObjectNode::getOrientation( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform.orientation();
}

//-----------------------------------------------------------------------------
//  Name : getXAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getXAxis( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform.xUnitAxis();
}

//-----------------------------------------------------------------------------
//  Name : getYAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getYAxis( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform.yUnitAxis();
}

//-----------------------------------------------------------------------------
//  Name : getZAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getZAxis( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform.zUnitAxis();
}

//-----------------------------------------------------------------------------
//  Name : getScale()
/// <summary>
/// Retrieve the current scale of the node along its world space axes.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getScale( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform.localScale();
}

//-----------------------------------------------------------------------------
//  Name : getPosition()
/// <summary>
/// Retrieve the current world space position of the node.
/// Optionally, caller can retrieve the position at the pivot point or 
/// otherwise.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgObjectNode::getPosition( bool atPivot )
{
    return getWorldTransform(atPivot).position();
    
    /*cgToDo( "Carbon General", "Optimize all of these (right, up, look, position) to only transform the data they need rather than the full matrix." );
    if ( atPivot || mOffsetTransform.IsIdentity() )
    {
        // Get the parent cell's world origin (if any) so that we can adjust the 
        // cell space position extracted from our transformation as required by our 
        // 'sliding world' system, transforming it such that it is relative to world space.
        if ( !mParentCell )
            return mCellTransform.position();
        else
            return mCellTransform.position() + mParentCell->getWorldOrigin();

    } // End if at pivot
    else
    {
        cgTransform SourceTransform;
        cgTransform::multiply( SourceTransform, mOffsetTransform, mCellTransform );

        // Get the parent cell's world origin (if any) so that we can adjust the 
        // cell space position extracted from our transformation as required by our 
        // 'sliding world' system, transforming it such that it is relative to world space.
        if ( !mParentCell )
            return SourceTransform.position();
        else
            return SourceTransform.position() + mParentCell->getWorldOrigin();
    
    } // End if at object*/
}

//-----------------------------------------------------------------------------
//  Name : getOrientation()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// Optionally, caller can retrieve the orientation at the pivot point or 
/// otherwise.
/// </summary>
//-----------------------------------------------------------------------------
cgQuaternion cgObjectNode::getOrientation( bool atPivot )
{
    return getWorldTransform(atPivot).orientation();
}

//-----------------------------------------------------------------------------
//  Name : getXAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// Optionally, caller can retrieve the orientation at the pivot point or 
/// otherwise.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getXAxis( bool atPivot )
{
    return getWorldTransform(atPivot).xUnitAxis();

    /*cgToDo( "Carbon General", "Optimize all of these (right, up, look, position) to only transform the data they need rather than the full matrix." );
    if ( !atPivot )
    {
        cgVector3 v;
        return mCellTransform.transformNormal( v, mOffsetTransform.xUnitAxis() );
    
    } // End if !pivot
    else
        return mCellTransform.xUnitAxis();*/
}

//-----------------------------------------------------------------------------
//  Name : getYAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// Optionally, caller can retrieve the orientation at the pivot point or 
/// otherwise.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getYAxis( bool atPivot )
{
    return getWorldTransform(atPivot).yUnitAxis();

    /*cgToDo( "Carbon General", "Optimize all of these (right, up, look, position) to only transform the data they need rather than the full matrix." );
    if ( !atPivot )
    {
        cgVector3 v;
        return mCellTransform.transformNormal( v, mOffsetTransform.yUnitAxis() );
    
    } // End if !pivot
    else
        return mCellTransform.YAxis();*/
}

//-----------------------------------------------------------------------------
//  Name : getZAxis()
/// <summary>
/// Retrieve the current orientation of the node in world space.
/// Optionally, caller can retrieve the orientation at the pivot point or 
/// otherwise.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgObjectNode::getZAxis( bool atPivot )
{
    return getWorldTransform(atPivot).zUnitAxis();

    /*cgToDo( "Carbon General", "Optimize all of these (right, up, look, position) to only transform the data they need rather than the full matrix." );
    if ( !atPivot )
    {
        cgVector3 v;
        return mCellTransform.transformNormal( v, mOffsetTransform.zUnitAxis() );
    
    } // End if !pivot
    else
        return mCellTransform.ZAxis();*/
}

//-----------------------------------------------------------------------------
//  Name : getWorldTransform()
/// <summary>
/// Retrieve the node's current world transform.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgObjectNode::getWorldTransform( bool atPivot )
{
    // Resolve any outstanding transformation updates.
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );

    // At pivot, or not?
    if ( atPivot )
        return mWorldPivotTransform;
    else
    {
        const cgConstantBuffer * pBuffer = mWorldTransformBuffer.getResourceSilent();
        return *(cgTransform const * const)pBuffer->getReadOnlyBuffer();

    } // End if !pivot 

    /*if ( !mParentCell )
    {
        if ( atPivot || mOffsetTransform.IsIdentity() )
            return mCellTransform;
        else
            return (mOffsetTransform * mCellTransform);
        
    } // End if no cell
    else
    {
        // Offset transform based on cell center point.
        cgTransform worldTransform = (atPivot || mOffsetTransform.IsIdentity()) ? mCellTransform : (mOffsetTransform * mCellTransform);
        worldTransform.translate( mParentCell->getWorldOrigin() );
        return worldTransform;

    } // End if cell*/

    /*if ( !atPivot )
    {
        if ( m_bWorldTransformDirty )
        {
            if ( !mParentCell )
            {
                if ( atPivot || mOffsetTransform.IsIdentity() )
                    m_WorldTransform = mCellTransform;
                else
                    m_WorldTransform = (mOffsetTransform * mCellTransform);
                
            } // End if no cell
            else
            {
                // Offset transform based on cell center point.
                m_WorldTransform = (atPivot || mOffsetTransform.IsIdentity()) ? mCellTransform : (mOffsetTransform * mCellTransform);
                m_WorldTransform.translate( mParentCell->getWorldOrigin() );

            } // End if cell

            cgResourceManager * resources = mParentScene->getResourceManager();
            cgSurfaceShaderHandle hShader;
            resources->CreateSurfaceShader( &hShader, _T("sys://Shaders/RenderDriver.sh"), 0 );
            resources->createConstantBuffer( &mWorldTransformBuffer, hShader, _T("_cbWorld"), cgDebugSource() );
            cgConstantBuffer * pWorldBuffer = mWorldTransformBuffer.getResource( true );

            // Lock the buffer ready for population
            cgMatrix * pMatrices = (cgMatrix*)pWorldBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard );
            pMatrices[0] = m_WorldTransform;
            pMatrices[1] = m_WorldTransform;
            pMatrices[1]._41 = 0; pMatrices[1]._42 = 0; pMatrices[1]._43 = 0; pMatrices[1]._44 = 1; // Remove translation component.
            if ( cgMatrix::inverse( &pMatrices[1], CG_NULL, &pMatrices[1] ) )
                cgMatrix::transpose( &pMatrices[1], &pMatrices[1] );
            pWorldBuffer->unlock();

            m_bWorldTransformDirty = false;

        } // End if world transform dirty

        return m_WorldTransform;

    }
    else
    {
        if ( !mParentCell )
        {
            return mCellTransform;
            
        } // End if no cell
        else
        {
            // Offset transform based on cell center point.
            static cgTransform worldTransform;
            worldTransform = mCellTransform;
            worldTransform.translate( mParentCell->getWorldOrigin() );
            return worldTransform;

        } // End if cell
    }*/
}

//-----------------------------------------------------------------------------
//  Name : getWorldTransform()
/// <summary>
/// Retrieve the node's current world transform at its pivot.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgObjectNode::getWorldTransform( )
{
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mWorldPivotTransform;

     /*// Adjust the matrix relative to the center of the parent cell.
    if ( !mParentCell )
        return mCellTransform;
    
    // Offset matrix based on cell center point.
    cgTransform worldTransform = mCellTransform;
    worldTransform.translate( mParentCell->getWorldOrigin() );
    return worldTransform;*/
}

//-----------------------------------------------------------------------------
//  Name : getLocalTransform()
/// <summary>
/// Retrieve the node's parent relative 'local' transformation matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgObjectNode::getLocalTransform( ) const
{
    // Return reference to our internal matrix
    return mLocalTransform;
}

//-----------------------------------------------------------------------------
//  Name : getOffsetTransform()
/// <summary>
/// Retrieve the transform describing the offset from the pivot (origin) of the 
/// node, to the *actual* origin of the referenced object.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgObjectNode::getOffsetTransform( ) const
{
    // Return reference to our internal matrix
    return mOffsetTransform;
}

//-----------------------------------------------------------------------------
//  Name : getCellTransform()
/// <summary>
/// Retrieve the node's current cell relative transformation.
/// </summary>
//-----------------------------------------------------------------------------
const cgTransform & cgObjectNode::getCellTransform( )
{
    // Return reference to our internal matrix
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );
    return mCellTransform;
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Instruct the object to look at the specified point.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::lookAt( cgFloat x, cgFloat y, cgFloat z )
{
    cgToDo( "Carbon General", "These lookAt methods need to consider the pivot and the currently applied transform method!!!" );
    lookAt( getPosition(), cgVector3( x, y, z ) );
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Instruct the object to look at the specified point.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::lookAt( const cgVector3 & point )
{
    lookAt( getPosition(), point );
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Instruct the object to look at the specified point.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::lookAt( const cgVector3 & eye, const cgVector3 & at )
{
    cgTransform m;
    cgTransform::lookAt( m, eye, at );

    // Update the camera position / orientation through the common base method.
    setWorldTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : lookAt()
/// <summary>
/// Instruct the object to look at the specified point, aligned to a
/// prefered / specific up axis.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::lookAt( const cgVector3 & eye, const cgVector3 & at, const cgVector3 & up )
{
    cgTransform m;
    cgTransform::lookAt( m, eye, at, up );

    // Update the camera position / orientation through the common base method.
    setWorldTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : reloadTransforms()
/// <summary>
/// Reload the original object transforms from the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::reloadTransforms( bool reloadChildren )
{
    if ( !isInternalReference() )
    {
        // If we have a parent node, its transform must be fully resolved at this point
        // in order to ensure that the node's local transform can be computed accurately.
        if ( mParentNode )
            mParentNode->resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );

        // Load the transform  data.
        prepareQueries();
        mNodeLoadTransforms.bindParameter( 1, mReferenceId );
        if ( !mNodeLoadTransforms.step( ) || !mNodeLoadTransforms.nextRow() )
        {
            // Log any error.
            cgString error;
            if ( !mNodeLoadTransforms.getLastError( error ) )
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve transformation data for object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
            else
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve transformation data for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

            // Release any pending read operation.
            mNodeLoadTransforms.reset();
            return false;
        
        } // End if failed
        
        // Retrieve transforms. Cell transform first.
        cgQuaternion rotation;
        cgVector3 scale, shear, position;
        mNodeLoadTransforms.getColumn( _T("PositionX"), position.x );
        mNodeLoadTransforms.getColumn( _T("PositionY"), position.y );
        mNodeLoadTransforms.getColumn( _T("PositionZ"), position.z );
        mNodeLoadTransforms.getColumn( _T("RotationX"), rotation.x );
        mNodeLoadTransforms.getColumn( _T("RotationY"), rotation.y );
        mNodeLoadTransforms.getColumn( _T("RotationZ"), rotation.z );
        mNodeLoadTransforms.getColumn( _T("RotationW"), rotation.w );
        mNodeLoadTransforms.getColumn( _T("ShearXY"), shear.x );
        mNodeLoadTransforms.getColumn( _T("ShearXZ"), shear.y );
        mNodeLoadTransforms.getColumn( _T("ShearYZ"), shear.z );
        mNodeLoadTransforms.getColumn( _T("ScaleX"), scale.x );
        mNodeLoadTransforms.getColumn( _T("ScaleY"), scale.y );
        mNodeLoadTransforms.getColumn( _T("ScaleZ"), scale.z );

        // Retrieve the location of the /original/ cell (before we
        // potentially jumped to a new cell).
        cgInt32 locationX, locationY, locationZ;
        mNodeLoadTransforms.getColumn( _T("LocationX"), locationX );
        mNodeLoadTransforms.getColumn( _T("LocationY"), locationY );
        mNodeLoadTransforms.getColumn( _T("LocationZ"), locationZ );

        // Offset the original cell position so that it is relative to 
        // world space initially (rather than the original cell).
        const cgVector3 cellSize = mParentScene->getCellSize();
        position.x += locationX * cellSize.x;
        position.y += locationY * cellSize.y;
        position.z += locationZ * cellSize.z;

        // Now transform it into the space of the /current/ cell (if any)
        // in order to ensure that 'cgScene::updateObjectOwnership()' transfers 
        // us back to become relative to the correct original cell.
        if ( mParentCell )
            position -= mParentCell->getWorldOrigin();

        // Recompose the transform.
        mCellTransform.compose( scale, shear, rotation, position );

        // Pivot offset transform next.
        mNodeLoadTransforms.getColumn( _T("OffsetPositionX"), position.x );
        mNodeLoadTransforms.getColumn( _T("OffsetPositionY"), position.y );
        mNodeLoadTransforms.getColumn( _T("OffsetPositionZ"), position.z );
        mNodeLoadTransforms.getColumn( _T("OffsetRotationX"), rotation.x );
        mNodeLoadTransforms.getColumn( _T("OffsetRotationY"), rotation.y );
        mNodeLoadTransforms.getColumn( _T("OffsetRotationZ"), rotation.z );
        mNodeLoadTransforms.getColumn( _T("OffsetRotationW"), rotation.w );
        mNodeLoadTransforms.getColumn( _T("OffsetShearXY"), shear.x );
        mNodeLoadTransforms.getColumn( _T("OffsetShearXZ"), shear.y );
        mNodeLoadTransforms.getColumn( _T("OffsetShearYZ"), shear.z );
        mNodeLoadTransforms.getColumn( _T("OffsetScaleX"), scale.x );
        mNodeLoadTransforms.getColumn( _T("OffsetScaleY"), scale.y );
        mNodeLoadTransforms.getColumn( _T("OffsetScaleZ"), scale.z );

        // Recompose the offset transform.
        mOffsetTransform.compose( scale, shear, rotation, position );

        // We're done with the read. Reset.
        mNodeLoadTransforms.reset();

        // Update the cached world transformation at the pivot point.
        mWorldPivotTransform = mCellTransform;
        if ( mParentCell )
            mWorldPivotTransform.translate( mParentCell->getWorldOrigin() );

        // Update the world transform constant buffer.
        cgAssert( mWorldTransformBuffer.isValid() );
        cgConstantBuffer * matrixBuffer = mWorldTransformBuffer.getResource(true);
        cgMatrix * matrixData = (cgMatrix*)matrixBuffer->lock( 0, 0, cgLockFlags::Discard | cgLockFlags::WriteOnly );
        cgMatrix finalTransform = (mOffsetTransform * mWorldPivotTransform);
        matrixData[0] = finalTransform;
        cgMatrix::inverse( finalTransform, finalTransform );
        cgMatrix::transpose( finalTransform, finalTransform );
        matrixData[1] = finalTransform;
        matrixBuffer->unlock();

        // Forcibly update the physics body's current transform on request.
        if ( mPhysicsBody )
            mPhysicsBody->setTransform( getWorldTransform(false) );

        // Store newly updated parent relative transformation.
        if ( mParentNode )
        {
            cgTransform::inverse( mLocalTransform, mParentNode->getCellTransform() );
            cgTransform::multiply( mLocalTransform, mCellTransform, mLocalTransform );
        
        } // End if has parent
        else
        {
            mLocalTransform = mCellTransform;
        
        } // End if no parent

        // Node is automatically dirty if the transformation changed.
        // Also automatically re-insert the node into the relevant cell
        // and generate the correct /final/ parent cell relative transform.
        cgUInt32 deferredUpdates = cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus;
        
        // Inform any children that the node was transformed only
        // if we are not about to reload their information too.
        cgUInt32 childDeferredUpdates = 0;
        if ( !reloadChildren )
            childDeferredUpdates = deferredUpdates | cgDeferredUpdateFlags::Transforms; 

        // Queue updates
        nodeUpdated( deferredUpdates, childDeferredUpdates );

    } // End if serialized reference

    // Reload the transformation state of any children on request.
    if ( reloadChildren )
    {
        for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
        {
            cgObjectNode * childNode = *itNode;
            if ( !childNode->reloadTransforms( true ) )
                return false;

        } // Next child node
    
    } // End if !reloadChildren

    // Clear out any velocity that may have been accumulated
    // since the transforms were loaded.
    setVelocity( cgVector3(0,0,0) );
    setAngularVelocity( cgVector3(0,0,0) );

    // Success!
    return true;
    
}

//-----------------------------------------------------------------------------
//  Name : setWorldTransform()
/// <summary>
/// Update our internal cell transformation based on an adjustment of the world
/// transform provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setWorldTransform( const cgTransform & transform )
{
    setWorldTransform( transform, cgTransformSource::Standard );
}

//-----------------------------------------------------------------------------
//  Name : setWorldTransform() (Virtual)
/// <summary>
/// Update our internal cell transformation based on an adjustment of the world
/// transform provided. This overload allows the caller to describe the 
/// operating mode under which the transformation is taking place.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setWorldTransform( const cgTransform & transform, cgTransformSource::Base source )
{
    // Compute the new cell relative version if necessary.
    if ( mParentCell )
    {
        cgTransform newTransform = transform;
        newTransform.translate( -mParentCell->getWorldOrigin() );
        setCellTransform( newTransform, source );

    } // End if has parent cell
    else
    {
        setCellTransform( transform, source );

    } // End if no parent cell
}

//-----------------------------------------------------------------------------
//  Name : setCellTransform()
/// <summary>
/// Update our internal cell matrix with that specified here. Optionally, you
/// can disable the recomputation of the local (parent relative) transformation
/// matrix (defaults to true). This is generally only used internally in order
/// to reduce introducing accumulated errors into the transformation matrix but
/// can be supplied if you know that this does not need to be performed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setCellTransform( const cgTransform & transform, cgTransformSource::Base source /* = cgTransformSource::Standard */ )
{
    cgTransform newTransform, newOffset = mOffsetTransform;

    // If node cannot be rotated, only translated.
    if ( !canRotate() )
    {
        newTransform = mCellTransform;
        newTransform.setPosition( transform.position() );

        // ToDo: transfer scale unless disallowed

    } // End if cannot rotate
    else
    {
        newTransform = transform;
    
    } // End if free transform

    // If we have a target node, make sure we are always aligned 
    // accordingly.
    if ( mTargetNode && mTransformMethod != cgTransformMethod::ObjectOnly )
        mTargetNode->adjustTargetingTransform( newTransform );

    // What method are we using to apply this transformation?
    bool offsetUpdated = false;
    if ( mTransformMethod == cgTransformMethod::PivotOnly )
    {
        // Update offset transform such that the actual space of the object
        // remains in the same position and orientation, only the node transform 
        // is adjusted.
        cgTransform inverseNew;
        cgTransform::inverse( inverseNew, newTransform );
        cgTransform::multiply( newOffset, mOffsetTransform, mCellTransform );
        cgTransform::multiply( newOffset, newOffset, inverseNew );
        offsetUpdated = true;        
    
    } // End if adjust pivot
    else if ( mTransformMethod == cgTransformMethod::ObjectOnly )
    {
        // Update offset transform such that the actual space of the object
        // remains in the same position and orientation, only the node transform 
        // is adjusted.
        cgTransform inverseCell;
        cgTransform::inverse( inverseCell, mCellTransform );
        cgTransform::multiply( newOffset, mOffsetTransform, newTransform );
        cgTransform::multiply( newOffset, newOffset, inverseCell );
        newTransform = mCellTransform;
        offsetUpdated = true;
        
    } // End if adjust object

    // ToDo: 9999 - This no-op will kill the object only transform.
    // Disabled for now.
    // No-op?
    //if ( memcmp( &m_mtxCell, &mtxNew, sizeof(cgMatrix) ) == 0 )
        //return true;

    // Update database reference.
    if ( shouldSerialize() && mParentScene->isSceneWritingEnabled() )
    {
        // Decompose the transform for storage.
        cgQuaternion rotation;
        cgVector3 scale, shear, position;
        newTransform.decompose( scale, shear, rotation, position );

        // Update database.
        prepareQueries();
        mNodeUpdateTransform.bindParameter( 1, position.x );
        mNodeUpdateTransform.bindParameter( 2, position.y );
        mNodeUpdateTransform.bindParameter( 3, position.z );
        mNodeUpdateTransform.bindParameter( 4, rotation.x );
        mNodeUpdateTransform.bindParameter( 5, rotation.y );
        mNodeUpdateTransform.bindParameter( 6, rotation.z );
        mNodeUpdateTransform.bindParameter( 7, rotation.w );
        mNodeUpdateTransform.bindParameter( 8, shear.x );
        mNodeUpdateTransform.bindParameter( 9, shear.y );
        mNodeUpdateTransform.bindParameter( 10, shear.z );
        mNodeUpdateTransform.bindParameter( 11, scale.x );
        mNodeUpdateTransform.bindParameter( 12, scale.y );
        mNodeUpdateTransform.bindParameter( 13, scale.z );
        mNodeUpdateTransform.bindParameter( 14, mReferenceId );
        if ( !mNodeUpdateTransform.step( true ) )
        {
            cgString error;
            mNodeUpdateTransform.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update cell transform for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

        // Update offset transform?
        cgToDo( "Carbon General", "Note: We always update the offset transform currently because the 'cgTransform::scale()' method might have modified it in its 'PositionOnly' mode." );
        //if ( offsetUpdated )
        {
            // Decompose the transform for storage.
            newOffset.decompose( scale, shear, rotation, position );

            // Update database.
            mNodeUpdateOffsetTransform.bindParameter( 1, position.x );
            mNodeUpdateOffsetTransform.bindParameter( 2, position.y );
            mNodeUpdateOffsetTransform.bindParameter( 3, position.z );
            mNodeUpdateOffsetTransform.bindParameter( 4, rotation.x );
            mNodeUpdateOffsetTransform.bindParameter( 5, rotation.y );
            mNodeUpdateOffsetTransform.bindParameter( 6, rotation.z );
            mNodeUpdateOffsetTransform.bindParameter( 7, rotation.w );
            mNodeUpdateOffsetTransform.bindParameter( 8, shear.x );
            mNodeUpdateOffsetTransform.bindParameter( 9, shear.y );
            mNodeUpdateOffsetTransform.bindParameter( 10, shear.z );
            mNodeUpdateOffsetTransform.bindParameter( 11, scale.x );
            mNodeUpdateOffsetTransform.bindParameter( 12, scale.y );
            mNodeUpdateOffsetTransform.bindParameter( 13, scale.z );
            mNodeUpdateOffsetTransform.bindParameter( 14, mReferenceId );
            if ( !mNodeUpdateOffsetTransform.step( true ) )
            {
                cgString error;
                mNodeUpdateOffsetTransform.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update offset transform for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

            // Store new offset transform
            mOffsetTransform = newOffset;

        } // End if offset altered

    } // End if exists in DB

    // Store new cell transform
    mCellTransform = newTransform;

    // Update the cached world transformation at the pivot point.
    mWorldPivotTransform = mCellTransform;
    if ( mParentCell )
        mWorldPivotTransform.translate( mParentCell->getWorldOrigin() );

    // Update the world transform constant buffer.
    cgAssert( mWorldTransformBuffer.isValid() );
    cgConstantBuffer * matrixBuffer = mWorldTransformBuffer.getResource(true);
    cgMatrix * matrixData = (cgMatrix*)matrixBuffer->lock( 0, 0, cgLockFlags::Discard | cgLockFlags::WriteOnly );
    cgMatrix finalTransform = (mOffsetTransform * mWorldPivotTransform);
    matrixData[0] = finalTransform;
    cgMatrix::inverse( finalTransform, finalTransform );
    cgMatrix::transpose( finalTransform, finalTransform );
    matrixData[1] = finalTransform;
    matrixBuffer->unlock();

    // Forcibly update the physics body's current transform on request.
    if ( (source != cgTransformSource::Dynamics) && mPhysicsBody )
    {
        const cgTransform & worldTransform = getWorldTransform(false);

        // Set the new transform for the body.
        mPhysicsBody->setTransform( worldTransform );

        // Update the mass of any dynamic physics body based on the new scale.
        if ( mPhysicsModel == cgPhysicsModel::RigidDynamic )
        {
            // Generate a final mass that is scaled by this node's
            // scaling transform component.
            cgFloat mass = getBaseMass();
            cgFloat massTransformAmount = getMassTransformAmount();
            cgVector3 scale = worldTransform.localScale();
            mass *= 1.0f + (scale.x - 1.0f) * massTransformAmount;
            mass *= 1.0f + (scale.y - 1.0f) * massTransformAmount;
            mass *= 1.0f + (scale.z - 1.0f) * massTransformAmount;
            mPhysicsBody->setMass( mass );

        } // End if has body

    } // End if update body

    // Certain processes shouldn't be run during a transform resolve.
    if ( source != cgTransformSource::TransformResolve )
    {
        // Compute newly updated parent relative transformation.
        if ( mParentNode )
        {
            cgTransform::inverse( mLocalTransform, mParentNode->getCellTransform() );
            cgTransform::multiply( mLocalTransform, mCellTransform, mLocalTransform );
        
        } // End if has parent
        else
        {
            mLocalTransform = mCellTransform;
        
        } // End if no parent

        // With the transformation being altered, certain other pieces of information
        // may need to be computed such as world space bounding box, new child node
        // transformations and so on. Ideally however, the computation of these details
        // would only occur once the entire update process has been completed (i.e.
        // all targets have been processed by an animation controller). As a result,
        // we defer the remaining updates for resolution later. We start by deferring
        // the computation of the object's bounding box.
        cgUInt32 deferredUpdates = cgDeferredUpdateFlags::BoundingBox;

        // In addition, the ownership of the node also needs to be updated (parent cell or 
        // spatial tree) because either the bounding box or the node transformation
        // has been altered.
        deferredUpdates |= cgDeferredUpdateFlags::OwnershipStatus;

        // If the currently applied transform method dictated that the transformation should 
        // *not* be applied to this node's child hierarchy then simply update any child's 
        // local transform accordingly in order to counteract the changes in parent transformation.
        // Otherwise, add the children to the update queue for transform resolution too.
        cgUInt32 childDeferredUpdates = 0;
        if ( mTransformMethod == cgTransformMethod::NoChildUpdate || mTransformMethod == cgTransformMethod::PivotOnly )
        {
            for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
            {
                cgObjectNode * childNode = *itNode;
                
                // Just update child's (parent relative) transformation based on the
                // parent's new position, orientation and scale.
                cgTransform::inverse( childNode->mLocalTransform, mCellTransform );
                cgTransform::multiply( childNode->mLocalTransform, childNode->mCellTransform, childNode->mLocalTransform );

            } // Next child node

        } // End if no child update
        else if ( !mChildren.empty() )
        {
            // Our children need to recompute their transforms as a result of this update.
            childDeferredUpdates = cgDeferredUpdateFlags::BoundingBox |
                                   cgDeferredUpdateFlags::OwnershipStatus |
                                   cgDeferredUpdateFlags::Transforms;

        } // End if child updates
        
        // Node is automatically dirty if the transformation changed.
        nodeUpdated( deferredUpdates, childDeferredUpdates );

        /*// Trigger 'onTransformChange' of all listeners (duplicate list in case
        // it is altered in response to event).
        if ( !mEventListeners.empty() )
        {
            EventListenerList listeners = mEventListeners;
            cgObjectNodeEventArgs eventArgs( this );
            for ( EventListenerList::iterator itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
                ((cgObjectNodeEventListener*)(*itListener))->onTransformChange( &eventArgs );

        } // End if any listeners*/
    
    } // End if !TransformResolve

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getReferencedObject()
/// <summary>
/// Retrieve the object being referenced by this node.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgObjectNode::getReferencedObject( ) const
{
    return mReferencedObject;
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::onNodeCreated( const cgUID & objectType, cgCloneMethod::Base cloneMethod )
{
    // Create a new unique object for us to reference if not already
    // allocated by (for instance) clone.
    if ( !mReferencedObject && cloneMethod == cgCloneMethod::None )
    {
        cgWorld * world = mParentScene->getParentWorld();
        mReferencedObject = world->createObject( isInternalReference(), objectType );
        if ( !mReferencedObject )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to instantiate new object of required type.\n") );
            return false;
        
        } // End if failed

        // Add us as a reference to this object (don't increment true DB based
        // reference count if this is an internal node).
        mReferencedObject->addReference( this, isInternalReference() );

        // Listen for any changes made to the object
        mReferencedObject->registerEventListener( static_cast<cgWorldComponentEventListener*>(this) );
        
        // ToDo: 9999
        /*// Make a temporary copy of the event to avoid possibility of
        // a race condition if the last subscriber unsubscribes
        // immediately after the null check and before the event is raised.
        NodeCreatedEventHandler ^ handler = NodeCreated;
        if (handler != nullptr) handler(this, e);*/

    } // End if !exists

    // Update the cached world transformation at the pivot point.
    mWorldPivotTransform = mCellTransform;
    if ( mParentCell )
        mWorldPivotTransform.translate( mParentCell->getWorldOrigin() );

    // Create the cached world transform constant buffer.
    cgRenderDriver    * renderDriver = mParentScene->getRenderDriver();
    cgResourceManager * resources    = mParentScene->getResourceManager();
    if ( !resources->createConstantBuffer( &mWorldTransformBuffer, renderDriver->getSystemShader(), _T("_cbWorld"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create constant buffer for object node world transform cache.\n") );
        return false;
    
    } // End if failed

    // Pre-cache the world transform.
    cgConstantBuffer * matrixBuffer = mWorldTransformBuffer.getResource(true);
    cgMatrix * matrixData = (cgMatrix*)matrixBuffer->lock( 0, 0, cgLockFlags::Discard | cgLockFlags::WriteOnly );
    cgMatrix finalTransform = (mOffsetTransform * mWorldPivotTransform);
    matrixData[0] = finalTransform;
    cgMatrix::inverse( finalTransform, finalTransform );
    cgMatrix::transpose( finalTransform, finalTransform );
    matrixData[1] = finalTransform;
    matrixBuffer->unlock();

    // Mark object as dirty immediately upon its creation and ensure that
    // its bounding box and ownership status are computed.
    nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );
    
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
bool cgObjectNode::onNodeLoading( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod )
{
    // Attached to associated parent cell initially.
    mParentCell = parentCell;

    // Retrieve the object referenced by this node as well as the 
    // reference identifier for the /original/ source node from which
    // we are loading (may not be the same as our current reference id
    // in cases where, for instance, objects are being spawned at runtime).
    cgUInt32 objectRefId = 0, sourceNodeRefId = 0;
    nodeData->getColumn( _T("RefId"), sourceNodeRefId );
    nodeData->getColumn( _T("ObjectId"), objectRefId );

    // Load the object for us to reference if specified.
    if ( objectRefId )
    {
        cgWorld * world = mParentScene->getParentWorld();
        mReferencedObject = world->loadObject( objectType, objectRefId, cloneMethod );
        if ( !mReferencedObject )
            return false;
        
        // Reconnect our reference to this object.
        mReferencedObject->addReference( this, true );

        // Listen for any changes made to the object
        mReferencedObject->registerEventListener( static_cast<cgWorldComponentEventListener*>(this) );

    } // End if valid identifier

    // Load flags
    bool visible;
    mFlags = 0;
    nodeData->getColumn( _T("Visible"), visible );
    if ( visible )
        mFlags |= cgObjectNodeFlags::Visible;

    // Load remaining node data.
    cgString name;
    nodeData->getColumn( _T("EditorName"), name );
    nodeData->getColumn( _T("InstanceIdentifier"), mInstanceIdentifier );
    nodeData->getColumn( _T("EditorColor"), mColor );
    nodeData->getColumn( _T("Level"), mNodeLevel );
    nodeData->getColumn( _T("PhysicsModel"), (cgInt32&)mPhysicsModel );
    nodeData->getColumn( _T("SimulationQuality"), (cgInt32&)mSimulationQuality );
    nodeData->getColumn( _T("RenderClassId"), mRenderClassId );
    nodeData->getColumn( _T("UpdateRate"), (cgInt32&)mUpdateRate );

    // Update local members
    setName( name );

    // Retrieve identifier of target node to re-attach to when the node is finally
    // initialized during onNodeInit(). We'll store it in the node custom property
    // container temporarily.
    cgInt32 targetId = 0;
    nodeData->getColumn( _T("TargetId"), targetId );
    if ( targetId != 0 )
        mCustomProperties->setProperty( _T("Core::NodeInitData::TargetId"), targetId );

    // Retrieve transforms. Cell transform first.
    cgQuaternion rotation;
    cgVector3 scale, shear, position;
    nodeData->getColumn( _T("PositionX"), position.x );
    nodeData->getColumn( _T("PositionY"), position.y );
    nodeData->getColumn( _T("PositionZ"), position.z );
    nodeData->getColumn( _T("RotationX"), rotation.x );
    nodeData->getColumn( _T("RotationY"), rotation.y );
    nodeData->getColumn( _T("RotationZ"), rotation.z );
    nodeData->getColumn( _T("RotationW"), rotation.w );
    nodeData->getColumn( _T("ShearXY"), shear.x );
    nodeData->getColumn( _T("ShearXZ"), shear.y );
    nodeData->getColumn( _T("ShearYZ"), shear.z );
    nodeData->getColumn( _T("ScaleX"), scale.x );
    nodeData->getColumn( _T("ScaleY"), scale.y );
    nodeData->getColumn( _T("ScaleZ"), scale.z );

    // Recompose the transform.
    mCellTransform.compose( scale, shear, rotation, position );

    // Pivot offset transform next.
    nodeData->getColumn( _T("OffsetPositionX"), position.x );
    nodeData->getColumn( _T("OffsetPositionY"), position.y );
    nodeData->getColumn( _T("OffsetPositionZ"), position.z );
    nodeData->getColumn( _T("OffsetRotationX"), rotation.x );
    nodeData->getColumn( _T("OffsetRotationY"), rotation.y );
    nodeData->getColumn( _T("OffsetRotationZ"), rotation.z );
    nodeData->getColumn( _T("OffsetRotationW"), rotation.w );
    nodeData->getColumn( _T("OffsetShearXY"), shear.x );
    nodeData->getColumn( _T("OffsetShearXZ"), shear.y );
    nodeData->getColumn( _T("OffsetShearYZ"), shear.z );
    nodeData->getColumn( _T("OffsetScaleX"), scale.x );
    nodeData->getColumn( _T("OffsetScaleY"), scale.y );
    nodeData->getColumn( _T("OffsetScaleZ"), scale.z );

    // Recompose the offset transform.
    mOffsetTransform.compose( scale, shear, rotation, position );

    // Update the cached world transformation at the pivot point.
    mWorldPivotTransform = mCellTransform;
    if ( mParentCell )
        mWorldPivotTransform.translate( mParentCell->getWorldOrigin() );

    // Create the cached world transform constant buffer.
    cgRenderDriver    * renderDriver = mParentScene->getRenderDriver();
    cgResourceManager * resources    = mParentScene->getResourceManager();
    if ( !resources->createConstantBuffer( &mWorldTransformBuffer, renderDriver->getSystemShader(), _T("_cbWorld"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create constant buffer for object node world transform cache.\n") );
        return false;
    
    } // End if failed

    // Pre-cache the world transform.
    cgConstantBuffer * matrixBuffer = mWorldTransformBuffer.getResource(true);
    cgMatrix * matrixData = (cgMatrix*)matrixBuffer->lock( 0, 0, cgLockFlags::Discard | cgLockFlags::WriteOnly );
    cgMatrix finalTransform = (mOffsetTransform * mWorldPivotTransform);
    matrixData[0] = finalTransform;
    cgMatrix::inverse( finalTransform, finalTransform );
    cgMatrix::transpose( finalTransform, finalTransform );
    matrixData[1] = finalTransform;
    matrixBuffer->unlock();

    // Load node custom properties (if any)
    prepareQueries();
    mNodeLoadCustomProperties.bindParameter( 1, sourceNodeRefId );
    if ( !mNodeLoadCustomProperties.step() )
    {
        // Log any error.
        cgString error;
        if ( !mNodeLoadCustomProperties.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve custom property data for object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve custom property data for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mNodeLoadCustomProperties.reset();
        return false;

    } // End if failed
    for ( ; mNodeLoadCustomProperties.nextRow(); )
    {
        // Load the variant data
        cgString name;
        cgUInt32 type, dataSize;
        void * variantData;
        mNodeLoadCustomProperties.getColumn( _T("Name"), name );
        mNodeLoadCustomProperties.getColumn( _T("Type"), type );
        mNodeLoadCustomProperties.getColumn( _T("Value"), &variantData, dataSize );

        // Insert into the custom property container.
        mCustomProperties->setProperty( name, cgVariant( variantData, (cgVariant::VariantType)type ) );

    } // Next property

    // We're done reading properties
    mNodeLoadCustomProperties.reset();

    // Load node behaviors (if any)
    mNodeLoadBehaviors.bindParameter( 1, sourceNodeRefId );
    if ( !mNodeLoadBehaviors.step() )
    {
        // Log any error.
        cgString error;
        if ( !mNodeLoadBehaviors.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve behavior data for object node '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve behavior data for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mNodeLoadBehaviors.reset();
        return false;

    } // End if failed
    for ( ; mNodeLoadBehaviors.nextRow(); )
    {
        // Load the behavior data.
        cgInt32 loadOrder, type;
        cgUInt32 behaviorId;
        mNodeLoadBehaviors.getColumn( _T("BehaviorId"), behaviorId );
        mNodeLoadBehaviors.getColumn( _T("LoadOrder"), loadOrder );
        mNodeLoadBehaviors.getColumn( _T("Type"), type );

        // Scripted or native?
        if ( type == 0 )
        {
            // Scripted behavior.
            cgString scriptFile;
            cgObjectBehavior * newBehavior = new cgObjectBehavior();
            newBehavior->setUserId( behaviorId );
            newBehavior->setLoadOrder( loadOrder );
            mNodeLoadBehaviors.getColumn( _T("Script"), scriptFile );
            newBehavior->initialize( mParentScene->getResourceManager(), scriptFile, _T("") );
            mBehaviors.push_back( newBehavior );

            // Note: We don't call 'setParentObject()' on the behavior here. Instead
            // this is performed during the 'onNodeInit()' method in order to ensure
            // that all child objects have been loaded first.

        } // End if scripted
        else
        {
            // Native behavior class.
            // ToDo: Support deserialization for native classes.

        } // End if native

    } // Next behavior

    // We're done reading behaviors
    mNodeLoadBehaviors.reset();

    // Mark object as dirty immediately upon its creation and ensure
    // that its bounding box and ownership status are up to date.
    nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

    // ToDo: 9999
    /*// Make a temporary copy of the event to avoid possibility of
    // a race condition if the last subscriber unsubscribes
    // immediately after the null check and before the event is raised.
    NodeLoadingEventHandler ^ handler = NodeLoading;
    if (handler != nullptr) handler(this, e);*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeInit () (Virtual)
/// <summary>
/// Optional method called both after creation and during loading to allow the
/// node to finally initialize itself with all relevant scene data it may
/// depend upon being available. In cases where reference identifiers pointing 
/// to nodes may have been remapped during loading (i.e. in cases where 
/// performing a cloned load), information about the remapping is provided to 
/// allow the node (or its underlying object) to take appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::onNodeInit( const cgUInt32IndexMap & nodeReferenceRemap )
{
    // Was it requested for us to re-attach to an existing target node?
    cgInt32 targetId = mCustomProperties->getProperty( _T("Core::NodeInitData::TargetId"), 0 );
    if ( targetId > 0 )
    {
        // A target node was specified. We must re-attach to it.
        // First determine if the id was remapped.
        cgUInt32IndexMap::const_iterator itRef = nodeReferenceRemap.find( targetId );
        if ( itRef != nodeReferenceRemap.end() )
            targetId = itRef->second;

        // Find the node in the scene list and re-attach.
        cgReference * targetRef = cgReferenceManager::getReference( targetId );
        if ( targetRef && targetRef->queryReferenceType( RTID_TargetNode ) )
        {
            mTargetNode = ((cgTargetNode*)targetRef);
            mTargetNode->setTargetingNode( this );
        
        } // End if valid target
        
    } // End if re-attach
    mCustomProperties->removeProperty( _T("Core::NodeInitData::TargetId") );

    // Build the physics body if appropriate.
    buildPhysicsBody();

    // Trigger behavior 'onAttach'
    for ( size_t i = 0; i < mBehaviors.size(); ++i )
        mBehaviors[i]->setParentObject( this );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted () (Virtual)
/// <summary>
/// Can be overridden or called by derived class when the object is being 
/// deleted in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::onNodeDeleted( )
{
    // Stop listening for any changes made to the object
    if ( mReferencedObject )
        mReferencedObject->unregisterEventListener( static_cast<cgWorldComponentEventListener*>(this) );

    // ToDo: 9999 - The exception behavior for the following is less than ideal
    // if spatial tree removal succeeded for instance, but cell removal failed.
    // This will correctly roll back the database (in the caller) but will not 
    // re-attach the node to the cell for instance.

    // Detach all children from this node first.
    for ( ; !mChildren.empty(); )
        (*mChildren.begin())->setParent( CG_NULL );

    // Remove from any parent cell (will also automatically
    // destroy the node from the database where applicable).
    setCell( CG_NULL );

    // If we belong to a group, make sure we get removed from it.
    setOwnerGroup( CG_NULL );

    // Also detach from our parent (if any).
    setParent( CG_NULL );

    // Remove from parent scene tree
    if ( mSceneTreeNode )
        mParentScene->getSceneTree()->removeSphere(mSceneTreeNode);
    mSceneTreeNode = CG_NULL;

    // Remove target node (if any).
    setTargetMethod( cgNodeTargetMethod::NoTarget );

    // Remove us as a valid reference from the object we're referencing (don't
    // decrement true DB based reference count if this is an internal node).
    if ( mReferencedObject )
        mReferencedObject->removeReference( this, isInternalReference() );
    mReferencedObject = CG_NULL;

    // ToDo: 9999 - Notify?
    /*// Make a temporary copy of the event to avoid possibility of
    // a race condition if the last subscriber unsubscribes
    // immediately after the null check and before the event is raised.
    EventHandler ^ handler = NodeDeleted;
    if (handler != nullptr) handler(this, e);*/

    // NB: Most everything else will be handled in the node's
    // 'dispose()' method. The above are the calls we need to make in
    // order to ensure that the database is correctly updated prior to 
    // the disposal process (which blocks all DB updates).

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onParentCellChanged()
/// <summary>
/// If this node is attached to a parent node, this event will be 
/// triggered whenever the parent's cell is modified.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onParentCellChanged(  )
{
    // Update cell to match parent.
    setCell( mParentNode->getCell() );
}

//-----------------------------------------------------------------------------
//  Name : onParentLevelChanged ( )
/// <summary>
/// If this node is attached to a parent node, this event will be triggered
/// whenever the parent's node level is modified.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onParentLevelChanged( )
{
    // Update database reference only if we are not disposing or loading.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdateLevel.bindParameter( 1, (cgUInt32)(mParentNode->mNodeLevel + 1) );
        mNodeUpdateLevel.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateLevel.step( true ) )
        {
            cgString strError;
            mNodeUpdateLevel.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update hierarchy level information for object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if !loading

    // Our new level is equal to the parent level + 1.
    mNodeLevel = mParentNode->mNodeLevel + 1;

    // Inform any children that the node changed level.
    for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
    {
        cgObjectNode * childNode = *itNode;
        if ( childNode )
            childNode->onParentLevelChanged( );

    } // Next child object
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the referenced object component is modified, this method is called to
/// allow us to take any appropriate action.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What property was modified?
    if ( e->context == _T("BaseMass") || e->context == _T("MassTransformAdjust") )
    {
        // Update the mass of any available physics body.
        if ( mPhysicsModel == cgPhysicsModel::RigidDynamic && mPhysicsBody )
        {
            // Generate a final mass that is scaled by this node's
            // scaling transform component.
            cgFloat mass = getBaseMass();
            cgFloat massTransformAmount = getMassTransformAmount();
            cgVector3 scale = getWorldTransform(false).localScale();
            mass *= 1.0f + (scale.x - 1.0f) * massTransformAmount;
            mass *= 1.0f + (scale.y - 1.0f) * massTransformAmount;
            mass *= 1.0f + (scale.z - 1.0f) * massTransformAmount;

            // Update the rigid body.
            mPhysicsBody->setMass( mass );

        } // End if has body
        
    } // End if BaseMass | MassTransformAdjust
    else if ( e->context == _T("CollisionShapeAdded") || e->context == _T("CollisionShapeRemoved") ||
              e->context == _T("CollisionShapeModified") )
    {
        // A new object sub element was added or removed. We should rebuild
        // any existing rigid body so that it takes new collision shapes into
        // account.
        buildPhysicsBody();

    } // End if CollisionShapeAdded | CollisionShapeRemoved | CollisionShapeModified
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified world space ray, determine if this node is 
/// intersected (or any of its children) and also compute the world space 
/// intersection distanceOut. Note: The "distanceOut" parameter is an in/out that will
/// be used to determine if this is the closest hit so far. Pass in a value
/// of FLT_MAX initially.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::pick( cgCameraNode * camera, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut, cgObjectNode *& closestNodeOut )
{
    bool hitDetected = false;

    // Process children first.
    for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
    {
        cgObjectNode * pChild = *itNode;
        if ( pChild )
            hitDetected |= pChild->pick( camera, viewportSize, rayOrigin, rayDirection, flags, wireTolerance, distanceOut, closestNodeOut );
    
    } // Next Child

    // If we don't reference an object, return immediately
    // with results from child hit detection.
    if ( !mReferencedObject )
        return hitDetected;

    // Now we'll pass on the request to the referenced object.
    // First transform ray to object space for picking.
    cgTransform inverseObjectTransform;
    const cgTransform & objectTransform = getWorldTransform( false );
    cgTransform::inverse( inverseObjectTransform, objectTransform );
    cgVector3 objectRayOrigin, objectRayDirection;
    inverseObjectTransform.transformCoord( objectRayOrigin, rayOrigin );
    inverseObjectTransform.transformNormal( objectRayDirection, rayDirection );

    // Direction needs to be normalized in case node matrix contained scale
    cgVector3::normalize( objectRayDirection, objectRayDirection );

    // Pass through to object
    cgFloat t = FLT_MAX;
    if ( !mReferencedObject->pick( camera, this, viewportSize, objectRayOrigin, objectRayDirection, flags, wireTolerance, t ) )
        return hitDetected;

    // Compute final object space intersection point.
    cgVector3 intersectionPoint = objectRayOrigin + (objectRayDirection * t);

    // transform intersection point back into world space to compute
    // the final intersection distance.
    objectTransform.transformCoord( intersectionPoint, intersectionPoint );
    t = cgVector3::length( intersectionPoint - rayOrigin );

    // Closest intersection so far?
    if ( t < distanceOut )
    {
        closestNodeOut  = this;
        distanceOut     = t;
        hitDetected     = true;
    
    } // End if closest

    // Return final result.
    return hitDetected;
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the node to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::sandboxRender( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane )
{
    // Validate requirements
    if ( !mParentScene )
        return false;

    // If the object is hidden, or marked as invisible, signal "do not draw" by returning false
    // when in preview mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled && !(mFlags & cgObjectNodeFlags::Visible) )
        return false;

    // Pass on this message to any referenced object.
    if ( mReferencedObject )
        mReferencedObject->sandboxRender( flags, camera, visibilityData, gridPlane, this );

    // If we have a target, and that target is not itself visible, make sure it gets rendered.
    if ( mTargetNode && !visibilityData->isObjectVisible( mTargetNode ) )
        mTargetNode->sandboxRender( flags, camera, visibilityData, gridPlane );

    // Derived class should draw this object
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getScene()
/// <summary>
/// Retrieve the scene to which this node belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgObjectNode::getScene( ) const
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : getCell()
/// <summary>
/// Retrieve the cell in which this node exists.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneCell * cgObjectNode::getCell( ) const
{
    return mParentCell;
}

//-----------------------------------------------------------------------------
//  Name : getCustomProperties()
/// <summary>
/// Retrieve a reference to the interal custom property container.
/// </summary>
//-----------------------------------------------------------------------------
const cgPropertyContainer & cgObjectNode::getCustomProperties( ) const
{
    return *mCustomProperties;
}

//-----------------------------------------------------------------------------
//  Name : getCustomProperties()
/// <summary>
/// Retrieve a reference to the interal custom property container.
/// </summary>
//-----------------------------------------------------------------------------
cgPropertyContainer & cgObjectNode::getCustomProperties( )
{
    return *mCustomProperties;
}

//-----------------------------------------------------------------------------
//  Name : getCustomProperty()
/// <summary>
/// Retrieve the value of the custom property with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant & cgObjectNode::getCustomProperty( const cgString & propertyName )
{
    return mCustomProperties->getProperty( propertyName );
}

//-----------------------------------------------------------------------------
//  Name : getCustomProperty()
/// <summary>
/// Retrieve the value of the custom property with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
const cgVariant & cgObjectNode::getCustomProperty( const cgString & propertyName ) const
{
    return mCustomProperties->getProperty( propertyName );
}

//-----------------------------------------------------------------------------
//  Name : getCustomProperty()
/// <summary>
/// Retrieve the value of the custom property with the specified name. If a
/// property with the specified name is not found, the default value supplied
/// will be returned.
/// </summary>
//-----------------------------------------------------------------------------
cgVariant cgObjectNode::getCustomProperty( const cgString & propertyName, const cgVariant & defaultValue ) const
{
    return mCustomProperties->getProperty( propertyName, defaultValue );
}

//-----------------------------------------------------------------------------
//  Name : setCustomProperty()
/// <summary>
/// Insert or replace a property with the specified name and value into this
/// node's custom property container.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setCustomProperty( const cgString & propertyName, const cgVariant & value )
{
    // Update the database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();

        // If property already exists, update. Otherwise, insert.
        if ( mCustomProperties->isPropertyDefined( propertyName ) )
        {
            mNodeUpdateCustomProperty.bindParameter( 1, (cgInt)value.getType() );
            mNodeUpdateCustomProperty.bindParameter( 2, value.getData(), value.getSize() );
            mNodeUpdateCustomProperty.bindParameter( 3, propertyName );
            mNodeUpdateCustomProperty.bindParameter( 4, mReferenceId );
            if ( !mNodeUpdateCustomProperty.step( true ) )
            {
                cgString error;
                mNodeUpdateCustomProperty.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update custom property '%s' for object node '0x%x'. Error: %s\n"), propertyName.c_str(), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if UPDATE
        else
        {
            mNodeInsertCustomProperty.bindParameter( 1, mReferenceId );
            mNodeInsertCustomProperty.bindParameter( 2, propertyName );
            mNodeInsertCustomProperty.bindParameter( 3, (cgInt)value.getType() );
            mNodeInsertCustomProperty.bindParameter( 4, value.getData(), value.getSize() );
            if ( !mNodeInsertCustomProperty.step( true ) )
            {
                cgString error;
                mNodeInsertCustomProperty.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert custom property '%s' for object node '0x%x'. Error: %s\n"), propertyName.c_str(), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if INSERT

    } // End if serialize

    // Update local property container.
    mCustomProperties->setProperty( propertyName, value );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : removeCustomProperty()
/// <summary>
/// Remove the custom property with specified name
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::removeCustomProperty( const cgString & propertyName )
{
    // Update the database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeRemoveCustomProperty.bindParameter( 1, propertyName );
        mNodeRemoveCustomProperty.bindParameter( 2, mReferenceId );
        if ( !mNodeRemoveCustomProperty.step( true ) )
        {
            cgString error;
            mNodeRemoveCustomProperty.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to remove custom property '%s' for object node '0x%x'. Error: %s\n"), propertyName.c_str(), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if serialize

    // Update local property container.
    mCustomProperties->removeProperty( propertyName );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : clearCustomProperties()
/// <summary>
/// Clear and release memory allocated for the storage of custom properties
/// associated with this node.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::clearCustomProperties( )
{
    // Update the database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeClearCustomProperties.bindParameter( 1, mReferenceId );
        if ( !mNodeClearCustomProperties.step( true ) )
        {
            cgString error;
            mNodeClearCustomProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to clear custom properties for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if serialize

    // Clear local property container.
    mCustomProperties->clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setCustomProperties()
/// <summary>
/// Update the node's internal custom property container with newly specified
/// values.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setCustomProperties( const cgPropertyContainer & properties )
{
    // Update the database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Start a transaction so that we can roll back.
        cgWorld * world = mParentScene->getParentWorld();
        world->beginTransaction( _T("setCustomProperties") );

        // Clear out old properties
        mNodeClearCustomProperties.bindParameter( 1, mReferenceId );
        if ( !mNodeClearCustomProperties.step( true ) )
        {
            cgString error;
            mNodeClearCustomProperties.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to clear prior custom properties for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            world->rollbackTransaction( _T("setCustomProperties") );
            return;
        
        } // End if failed

        // Insert new properties.
        cgPropertyContainer::Collection::const_iterator itProperty;
        for ( itProperty = properties.getProperties().begin(); itProperty != properties.getProperties().end(); ++itProperty )
        {
            mNodeInsertCustomProperty.bindParameter( 1, mReferenceId );
            mNodeInsertCustomProperty.bindParameter( 2, itProperty->first );
            mNodeInsertCustomProperty.bindParameter( 3, (cgInt)itProperty->second.getType() );
            mNodeInsertCustomProperty.bindParameter( 4, itProperty->second.getData(), itProperty->second.getSize() );
            if ( !mNodeInsertCustomProperty.step( true ) )
            {
                cgString error;
                mNodeInsertCustomProperty.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert custom property '%s' for object node '0x%x'. Error: %s\n"), itProperty->first.c_str(), mReferenceId, error.c_str() );
                world->rollbackTransaction( _T("setCustomProperties") );
                return;
            
            } // End if failed

        } // Next property

        // Commit the data
        world->commitTransaction( _T("setCustomProperties") );
    
    } // End if should serialize

    // Update local member
    *mCustomProperties = properties;
}

//-----------------------------------------------------------------------------
//  Name : rotateLocal ()
/// <summary>
/// Rotate the node around its own local axes.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::rotateLocal( cgFloat x, cgFloat y, cgFloat z )
{
    // Do nothing if rotation is disallowed.
    if ( !canRotate() )
        return;

    // No-op?
    if ( !x && !y && !z )
        return;

    // Rotate a copy of the current transform.
    cgTransform m = getCellTransform();
    m.rotateLocal( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ) );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : rotateAxis ()
/// <summary>
/// Rotate the node around a specified axis
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::rotateAxis( cgFloat degrees, const cgVector3 & axis )
{
    // No - op?
    if ( !degrees )
        return;

    // If rotation is disallowed, only process position change. Otherwise
    // perform full rotation.
    if ( !canRotate() )
    {
        // Scale the position, but do not allow axes to scale.
        cgVector3 vPos = getPosition();
        cgTransform t;
        t.rotateAxis( CGEToRadian(degrees), axis );
        t.transformCoord( vPos, vPos );
        setPosition( vPos );

    } // End if !canRotate()
    else
    {
        // Rotate a copy of the current transform.
        cgTransform m = getCellTransform();
        m.rotateAxis( CGEToRadian( degrees ), axis );

        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canRotate()

}

//-----------------------------------------------------------------------------
//  Name : rotateAxis ()
/// <summary>
/// Rotate the node around a specified axis
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::rotateAxis( cgFloat degrees, const cgVector3 & axis, const cgVector3 & center )
{
    // No - op?
    if ( !degrees )
        return;

    // If rotation is disallowed, only process position change. Otherwise
    // perform full rotation.
    if ( !canRotate() )
    {
        // Scale the position, but do not allow axes to scale.
        cgTransform t;
        cgVector3 position = getPosition() - center;
        t.rotateAxis( CGEToRadian(degrees), axis );
        t.transformCoord( position, position );
        setPosition( position + center );

    } // End if !canRotate()
    else
    {
        // Rotate a copy of the current transform.
        cgTransform m = getCellTransform();
        if ( mParentCell )
            m.rotateAxis( CGEToRadian( degrees ), axis, center - mParentCell->getWorldOrigin() );
        else
            m.rotateAxis( CGEToRadian( degrees ), axis, center );

        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canRotate()
}

//-----------------------------------------------------------------------------
//  Name : rotate ()
/// <summary>
/// Rotate the node by the amounts specified in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::rotate( cgFloat x, cgFloat y, cgFloat z )
{
    // No - op?
    if ( !x && !y && !z )
        return;

    // If rotation is disallowed, only process position change. Otherwise
    // perform full rotation.
    if ( !canRotate() )
    {
        // Scale the position, but do not allow axes to scale.
        cgTransform t;
        cgVector3 position = getPosition();
        t.rotate( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ) );
        t.transformCoord( position, position );
        setPosition( position );

    } // End if !canRotate()
    else
    {
        // Rotate a copy of the current transform.
        cgTransform m = getCellTransform();
        m.rotate( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ) );

        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canRotate()

}

//-----------------------------------------------------------------------------
//  Name : rotate ()
/// <summary>
/// Rotate the node by the amounts specified in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::rotate( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center )
{
    // No - op?
    if ( !x && !y && !z )
        return;

    // If rotation is disallowed, only process position change. Otherwise
    // perform full rotation.
    if ( !canRotate() )
    {
        // Scale the position, but do not allow axes to scale.
        cgTransform t;
        cgVector3 position = getPosition() - center;
        t.rotate( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ) );
        t.transformCoord( position, position );
        setPosition( position + center );

    } // End if !canRotate()
    else
    {
        // Rotate a copy of the current transform.
        cgTransform m = getCellTransform();
        if ( mParentCell )
            m.rotate( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ), center - mParentCell->getWorldOrigin() );
        else
            m.rotate( CGEToRadian( x ), CGEToRadian( y ), CGEToRadian( z ), center );

        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canRotate()    
}

//-----------------------------------------------------------------------------
//  Name : scale()
/// <summary>
/// Scale the node by the specified amount in world space.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scale( cgFloat x, cgFloat y, cgFloat z )
{
    scale( x, y, z, false );
}

//-----------------------------------------------------------------------------
//  Name : scale()
/// <summary>
/// Scale the node by the specified amount in world space. Optionally, a scale
/// can be applied to the position /  translation of the node only without
/// affecting the scale of the object itself.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scale( cgFloat x, cgFloat y, cgFloat z, bool positionOnly )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return;

    // If scaling is disallowed, only process position change. Otherwise
    // perform full scale.
    if ( !canScale() )
        positionOnly = true;

    // Position only scale?
    if ( positionOnly )
    {
        // Scale distance between pivot and object.
        cgVector3 & offset = mOffsetTransform.position();
        offset.x *= x;
        offset.y *= y;
        offset.z *= z;

        // Scale the position, but do not allow axes to scale.
        cgVector3 position = getPosition();
        position.x *= x;
        position.y *= y;
        position.z *= z;
        setPosition( position );

    } // End if !canScale()
    else
    {
        // Perform a full scale.
        cgTransform m = getCellTransform();
        m.scale( x, y, z );
        
        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canScale()
}

///-----------------------------------------------------------------------------
//  Name : scale()
/// <summary>
/// Scale the node by the specified amount in world space. Optionally, a scale
/// can be applied to the position /  translation of the node only without
/// affecting the scale of the object itself.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scale( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center )
{
    scale( x, y, z, center, false );
}

///-----------------------------------------------------------------------------
//  Name : scale()
/// <summary>
/// Scale the node by the specified amount in world space. Optionally, a scale
/// can be applied to the position /  translation of the node only without
/// affecting the scale of the object itself.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scale( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center, bool positionOnly )
{
    // No - op?
    if ( x == 1.0f && y == 1.0f && z == 1.0f )
        return;

    // If scaling is disallowed, only process position change. Otherwise
    // perform full scale.
    if ( !canScale() )
        positionOnly = true;

    // Position only scale?
    if ( positionOnly )
    {
        // Scale distance between pivot and object.
        cgVector3 & offset = mOffsetTransform.position();
        offset.x *= x;
        offset.y *= y;
        offset.z *= z;

        // Scale the position, but do not allow axes to scale.
        cgVector3 position = getPosition() - center;
        position.x *= x;
        position.y *= y;
        position.z *= z;
        setPosition( position + center );

    } // End if !canScale()
    else
    {
        // Perform a full scale.
        cgTransform m = getCellTransform();
        if ( mParentCell )
            m.scale( x, y, z, center - mParentCell->getWorldOrigin() );
        else
            m.scale( x, y, z, center );

        // Set to the node via a common method. This way, any 
        // derived class can be easily notified of changes made.
        setCellTransform( m );

    } // End if canScale()
}

//-----------------------------------------------------------------------------
// Name : scaleLocal() (Virtual)
/// <summary>Scale the node by the specified amount in "local" space.</summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scaleLocal( cgFloat x, cgFloat y, cgFloat z )
{
    // Do nothing if scaling is disallowed.
    if ( !canScale() )
        return;

    // Scale a copy of the cell transform
    cgTransform m = getCellTransform();
    m.scaleLocal( x, y, z );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
// Name : scaleLocal() (Virtual)
/// <summary>Scale the node by the specified amount in "local" space.</summary>
//-----------------------------------------------------------------------------
void cgObjectNode::scaleLocal( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & localCenter )
{
    // Do nothing if scaling is disallowed.
    if ( !canScale() )
        return;

    // Scale a copy of the cell transform
    cgTransform m = getCellTransform();
    m.scaleLocal( x, y, z, localCenter );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : setOrientation ()
/// <summary>
/// Update the node's orientation using the axis vectors provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setOrientation( const cgVector3 & x, const cgVector3 & y, const cgVector3 & z )
{
    // Do nothing if rotation is disallowed.
    if ( !canRotate() )
        return;

    // Set orientation of new transform
    cgTransform m = getCellTransform();
    m.setOrientation( x, y, z );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
//  Name : setOrientation ()
/// <summary>
/// Update the node's orientation using the quaternion provided.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setOrientation( const cgQuaternion & rotation )
{
    // Do nothing if rotation is disallowed.
    if ( !canRotate() )
        return;

    // Set orientation of new transform
    cgTransform m = getCellTransform();
    m.setOrientation( rotation );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
// Name : resetOrientation() (Virtual)
/// <summary>
/// Simply reset the orientation of the node, maintaining the current position
/// and scale.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::resetOrientation( )
{
    // Do nothing if rotation is disallowed.
    if ( !canRotate() )
        return;

    cgQuaternion q;
    cgQuaternion::identity( q );
    cgTransform m = getCellTransform();
    m.setOrientation( q );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
// Name : resetScale() (Virtual)
/// <summary>
/// Simply reset the scale of the node, maintaining the current position
/// and orientation.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::resetScale( )
{
    // Do nothing if scaling is disallowed.
    if ( !canScale() )
        return;

    cgTransform m = getCellTransform();
    m.setLocalScale( 1, 1, 1 );

    // Set to the node via a common method. This way, any 
    // derived class can be easily notified of changes made.
    setCellTransform( m );
}

//-----------------------------------------------------------------------------
// Name : resetPivot() (Virtual)
/// <summary>
/// Reset the pivot such that it sits at the origin of the object.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::resetPivot( )
{
    // Do nothing if pivot adjustment is disallowed.
    if ( !canAdjustPivot() )
        return;

    cgTransform newCell;
    cgTransform::multiply( newCell, mOffsetTransform, getCellTransform() );
    
    // Force the reset of the pivot.
    cgTransformMethod::Base oldMethod = mTransformMethod;
    mTransformMethod = cgTransformMethod::PivotOnly;
    setCellTransform( newCell );
    mTransformMethod = oldMethod;

    // Paranoia: Offset transform should now be identity, but
    // enforce this to clear any accumulation errors.
    mOffsetTransform.identity();
}

//-----------------------------------------------------------------------------
//  Name : isRenderable ()
/// <summary>
/// Determine if this node is currently visible / renderable or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isRenderable( ) const
{
    if ( mReferencedObject )
        return ((mFlags & cgObjectNodeFlags::Visible) && mReferencedObject->isRenderable());
    else
        return (mFlags & cgObjectNodeFlags::Visible);
}

//-----------------------------------------------------------------------------
//  Name : isVisible ()
/// <summary>
/// Determine if this node is currently visible, irrespective of whether or not
/// it is a renderable object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isVisible( ) const
{
    return (mFlags & cgObjectNodeFlags::Visible);
}

//-----------------------------------------------------------------------------
//  Name : showNode ()
/// <summary>
/// Allows the application to independently control the visibility status
/// of a node irrespective of whether or not it is currently occluded. 
/// Optionally, the children of this node can also be updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::showNode( bool visible /* = true */, bool updateChildren /* = false */ )
{
    bool wasVisible = ((mFlags & cgObjectNodeFlags::Visible) != 0);

    // Set the visibility bit appropriately
    mFlags &= ~cgObjectNodeFlags::Visible;
    if ( visible )
        mFlags |= cgObjectNodeFlags::Visible;

    // Serialize new visibility state if necessary
    if ( shouldSerialize() && wasVisible != visible )
    {
        prepareQueries();
        mNodeUpdateVisibility.bindParameter( 1, visible );
        mNodeUpdateVisibility.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateVisibility.step( true ) )
        {
            cgString error;
            mNodeUpdateVisibility.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update visibility state for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
        
        } // End if failed

    } // End if exists in DB

    // Pass on to children if requested.
    if ( updateChildren )
    {
        for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
            (*itNode)->showNode( visible, updateChildren );
    
    } // End if update children

    // Invalidate the sphere tree visibility data.
    if ( mSceneTreeNode && (wasVisible != visible) )
        mSceneTreeNode->invalidateVisibility();
}

//-----------------------------------------------------------------------------
//  Name : setUpdateRate ()
/// <summary>
/// Each update list has a different rate that can be used in order
/// to reduce the amount of nodes which potentially have to be updated
/// each frame. This function simply adds the node to the relevant 
/// internal update list based on the specified rate.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setUpdateRate( cgUpdateRate::Base rate )
{
    if ( mParentScene )
        mParentScene->setObjectUpdateRate( this, rate );
}

//-----------------------------------------------------------------------------
//  Name : serializeUpdateRate () (Protected)
/// <summary>
/// Update this node's database entry with the currently specified update rate.
/// Since the update rate adjustment is performed by the parent scene directly,
/// this is called by the scene to notify the node that a change occurred.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::serializeUpdateRate( )
{
    // Update database reference only if we are not loading, disposing
    // or are in sandbox mode.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdateUpdateRate.bindParameter( 1, (cgInt32)mUpdateRate );
        mNodeUpdateUpdateRate.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateUpdateRate.step( true ) )
        {
            cgString error;
            mNodeUpdateUpdateRate.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update processing interval (update rate) for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if exists in DB

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getUpdateRate ()
/// <summary>
/// Retrieve the rate at which this node will be visited in order to allow it
/// to update and process behaviors.
/// </summary>
//-----------------------------------------------------------------------------
cgUpdateRate::Base cgObjectNode::getUpdateRate( ) const
{
    return mUpdateRate;
}

//-----------------------------------------------------------------------------
//  Name : supportsInputChannels() (Virtual)
/// <summary>
/// Determine if this object or any of its attached behaviors or physics
/// controllers support the reading of input channel states. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::supportsInputChannels( ) const
{
    // ToDo: We can cache this information when a controller / behavior
    // is attached / detached.

    // Check physics controller first.
    if ( mPhysicsController && mPhysicsController->supportsInputChannels() )
        return true;

    // Check behaviors.
    BehaviorArray::const_iterator itBehavior;
    for ( itBehavior = mBehaviors.begin(); itBehavior != mBehaviors.end(); ++itBehavior )
    {
        if ( *itBehavior && (*itBehavior)->supportsInputChannels( ) )
            return true;
            
    } // Next Behavior

    // Input channels not supported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : registerInputChannel() (Static)
/// <summary>
/// Register a unique input channel -- through which behaviors and other
/// systems can communicate -- by name, retrieving a unique handle that
/// allows more rapid access to the object's channel state storage.
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgObjectNode::registerInputChannel( const cgString & channelName )
{
    // Input channel with this name already exists in the look up table?
    cgString nameKey = cgString::toLower( channelName );
    InputChannelLUT::iterator itChannel = mRegisteredInputChannels.find( nameKey );
    
    // If so, just return the existing handle.
    if ( itChannel != mRegisteredInputChannels.end() )
        return itChannel->second;

    // Otherwise, add this channel to the map and return the new handle.
    cgInt16 channelHandle = (cgInt16)mRegisteredInputChannels.size();
    mRegisteredInputChannels[ nameKey ] = channelHandle;
    return channelHandle;
}

//-----------------------------------------------------------------------------
//  Name : setInputChannelState()
/// <summary>
/// Set the floating point state of one of the supported input channels.
/// This information can be read by any assigned object behavior or 
/// physics controller and is designed to allow for custom input mapping
/// rather than working with forces.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setInputChannelState( const cgString & channel, cgFloat state )
{
    // Retrieve the relevant handle for this channel or register 
    // it automatically if it doesn't exist.
    cgInt16 channelHandle = registerInputChannel( channel );

    // Set the relevant state.
    mInputChannels[channelHandle] = state;
}

//-----------------------------------------------------------------------------
//  Name : setInputChannelState()
/// <summary>
/// Set the floating point state of one of the supported input channels.
/// This information can be read by any assigned object behavior or 
/// physics controller and is designed to allow for custom input mapping
/// rather than working with forces.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setInputChannelState( cgInt16 handle, cgFloat state )
{
    // Set the relevant state.
    mInputChannels[handle] = state;
}

//-----------------------------------------------------------------------------
//  Name : getInputChannelState()
/// <summary>
/// Retrieve the floating point state of one of the supported input
/// channels. This information can be read by any assigned object
/// behavior or physics controller and is designed to allow for custom
/// input mapping rather than working with forces.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgObjectNode::getInputChannelState( const cgString & channel, cgFloat default ) const
{
    // Find the relevant handle for this channel.
    cgString nameKey = cgString::toLower( channel );
    InputChannelLUT::iterator itChannel = mRegisteredInputChannels.find( nameKey );
    if ( itChannel == mRegisteredInputChannels.end() )
        return default;

    // Retrieve the state associated with this channel.
    InputChannelMap::const_iterator itState = mInputChannels.find( itChannel->second );
    if ( itState == mInputChannels.end() )
        return default;

    // Return the discovered state.
    return itState->second;
}

//-----------------------------------------------------------------------------
//  Name : getInputChannelState()
/// <summary>
/// Retrieve the floating point state of one of the supported input
/// channels. This information can be read by any assigned object
/// behavior or physics controller and is designed to allow for custom
/// input mapping rather than working with forces.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgObjectNode::getInputChannelState( cgInt16 handle, cgFloat default ) const
{
    // Retrieve the state associated with this channel.
    InputChannelMap::const_iterator itState = mInputChannels.find( handle );
    if ( itState == mInputChannels.end() )
        return default;

    // Return the discovered state.
    return itState->second;
}

//-----------------------------------------------------------------------------
//  Name : setParent ()
/// <summary>
/// Attach this node to the specified parent as a child
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setParent( cgObjectNode * parent )
{
    return setParent( parent, false );
}
bool cgObjectNode::setParent( cgObjectNode * parent, bool constructing )
{
    // Skip if this is a no-op.
    if ( parent == mParentNode )
        return true;

    // If the new parent is already a child of ours then
    // this is a completely invalid attachment.
    if ( parent && parent->getParent() == this )
        return false;

    // If neither node wants to participate, fail.
    if ( parent )
    {
        if ( !validateAttachment( parent, false ) ||
             !parent->validateAttachment( this, true ) )
             return false;
    
    } // End if valid parent

    // Before we do anything, make sure that all pending transform 
    // operations are resolved (including those applied to our parent).
    resolvePendingUpdates( cgDeferredUpdateFlags::Transforms );

    // Update database reference only if we are not disposing or loading.
    if ( shouldSerialize() && mParentCell )
    {
        prepareQueries();
        mNodeUpdateParent.bindParameter( 1, (parent) ? parent->mReferenceId : 0 );
        mNodeUpdateParent.bindParameter( 2, (cgUInt32)((parent) ? parent->mNodeLevel + 1 : 0) );
        mNodeUpdateParent.bindParameter( 3, mReferenceId );
        if ( !mNodeUpdateParent.step( true ) )
        {
            cgString strError;
            mNodeUpdateParent.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update hierarchy parent reference for object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return false;
        
        } // End if failed
    
    } // End if shouldSerialize

    // If we are attached to a scene, perform necessary processing.
    if ( mParentScene )
    {
        // If we currently have a parent and we are being removed
        // then re-attach us to the scene's root node list.
        // If we do not currently have a parent, and we are being
        // attached to something then remove us from the document
        // root node list.
        if ( mParentNode && !parent )
            mParentScene->addRootNode( this );
        else if ( !mParentNode && parent )
            mParentScene->removeRootNode( this );

    } // End if scene set

    // If we previously had a parent, remove us from their child list.
    if ( mParentNode )
        mParentNode->removeChild( this );
    
    // If we are being attached to a node, add us to their child list
    if ( parent )
        parent->attachChild( this );
    
    // We're now attached / detached as required.
    cgObjectNode * oldParent = mParentNode;
    mParentNode = parent;

    // Update relative transformation matrix as required.
    if ( !mParentNode )
    {
        // Update relative transformation as required.
        mLocalTransform = getCellTransform();

        // We exist at the root level
        mNodeLevel = 0;

        // ToDo: 9999 - When an object is detached from its parent cell and it now exists at the root level, its cell needs to be recomputed.
    
    } // End if no parent
    else
    {
        // Our level is now equal to our parent's level (if any) plus one.
        mNodeLevel = mParentNode->mNodeLevel + 1;

        // ToDo: Error check 'setCell()'

        // Child objects always exist in the same cell as their parent.
        setCell( mParentNode->getCell(), constructing );

        // Update relative transformation as required. Parent's transform must be fully
        // resolved at this point in order for this to generate accurate information.
        cgTransform::inverse( mLocalTransform, mParentNode->getCellTransform() );
        cgTransform::multiply( mLocalTransform, getCellTransform(), mLocalTransform );
        
    } // End if has parent

    // Perform non-construction tasks.
    if ( !constructing )
    {
        // Notify children that the node level has changed if we're not in the process
        // of disposing.
        if ( !mDisposing )
        {
            // Inform any children that their parent node's cell was modified.
            for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
            {
                cgObjectNode * childNode = *itNode;
                if ( childNode )
                    childNode->onParentLevelChanged();

            } // Next child node

            // Send notifications to scene listeners too.
            if ( mParentScene )
                mParentScene->onNodeParentChange( &cgNodeParentChangeEventArgs( mParentScene, this, oldParent, mParentNode ) );
        
        } // End if notify.

        // If we were detached as a direct descendant of an owner group then
        // we should no longer belong to that group. As a result however, neither
        // can any of our children.
        if ( mOwnerGroup && getParentOfType( RTID_GroupObject ) != mOwnerGroup )
            mOwnerGroup->detachNode( this );

    } // End if !constructing

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : attachChild() (Protected, Virtual)
/// <summary>
/// Method called by 'setParent()' whenever a node is being attached as a
/// child of this node. Intended to allow derived classes to override or
/// provide custom behavior when this occurs.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::attachChild( cgObjectNode * child )
{
    mChildren.push_back( child );
}

//-----------------------------------------------------------------------------
//  Name : removeChild() (Protected, Virtual)
/// <summary>
/// Method called by 'setParent()' whenever a node is being detached as a
/// child of this node. Intended to allow derived classes to override or
/// provide custom behavior when this occurs.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::removeChild( cgObjectNode * child )
{
    mChildren.remove( child );
}

//-----------------------------------------------------------------------------
// Name : getSceneTreeNode ( )
/// <summary>
/// Retrieve the sub-node to which this object belongs within the main scene's 
/// broadphase / scene tree.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereTreeSubNode * cgObjectNode::getSceneTreeNode( ) const
{
    return mSceneTreeNode;
}

//-----------------------------------------------------------------------------
// Name : setSceneTreeNode ( )
/// <summary>
/// Attach this object node to the scene tree hierarchy at the relevant
/// location. This method is mostly used for the purpose of internal management 
/// and would generally not be called directly.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setSceneTreeNode( cgSphereTreeSubNode * sceneTreeNode )
{
    mSceneTreeNode = sceneTreeNode;
}

//-----------------------------------------------------------------------------
// Name : getParentOfType ( )
/// <summary>
/// Retrieve the first parent contained in the hierarchy above this node that
/// references an object that matches the specified type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::getParentOfType( const cgUID & TypeIdentifier ) const
{
    cgObjectNode * node = mParentNode;
    for ( ; node; node = node->mParentNode )
    {
        if ( node->queryObjectType( TypeIdentifier ) )
            return node;

    } // Next parent level
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : isParentSelected ( )
/// <summary>
/// Is any parent of this object node currently selected? By default, this
/// searches the entire hierarchy above this node, but can be optionally
/// constrained to the immediate parent only.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isParentSelected( bool immediateOnly /* = false */ ) const
{
    if ( immediateOnly )
    {
        // Test only the immediate parent.
        return (mParentNode && mParentNode->isSelected() );
    
    } // End if immediate parent only
    else
    {
        // Test the entire hierarchy.
        cgObjectNode * node = mParentNode;
        for ( ; node; node = node->mParentNode )
        {
            if ( node->isSelected() )
                return true;

        } // Next parent level
        return false;

    } // End if entire hierarchy
}

//-----------------------------------------------------------------------------
// Name : getOwnerGroup ( )
/// <summary>
/// Retrieve the group to which this node belongs (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgGroupNode * cgObjectNode::getOwnerGroup( ) const
{
    return mOwnerGroup;
}

//-----------------------------------------------------------------------------
// Name : setOwnerGroup ( )
/// <summary>
/// Set the group to which this node belongs (if any).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setOwnerGroup( cgGroupNode * group )
{
    // Skip if this is a no-op.
    if ( group == mOwnerGroup )
        return true;

    // Update database reference only if we are not disposing or loading.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdateGroup.bindParameter( 1, (group) ? group->getReferenceId() : 0 );
        mNodeUpdateGroup.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateGroup.step( true ) )
        {
            cgString error;
            mNodeUpdateGroup.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update owner group for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed
    
    } // End if !loading

    // Assign new parent group
    cgGroupNode * oldGroup = mOwnerGroup;
    mOwnerGroup = group;

    // Add reference to new group.
    if ( mOwnerGroup )
        mOwnerGroup->onGroupRefAdded( this );
        
    // Allow old and new groups to update.
    if ( mOwnerGroup )
        mOwnerGroup->nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );
    if ( oldGroup )
        oldGroup->nodeUpdated( cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus, 0 );

    // Remove reference from old group and allow it to destroy itself if necessary.
    if ( oldGroup )
        oldGroup->onGroupRefRemoved( this );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : isMergedAsGroup ( )
/// <summary>
/// Determine if this is currently acting as if it was part of one merged node
/// (i.e. a child of a close group).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isMergedAsGroup( ) const
{
    // If we have an owner group and it is closed we can only
    // have been selected as part of that group.
    if ( mOwnerGroup && !mOwnerGroup->isOpen() )
        return true;
    
    // Not selected as part of group.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setCell ()
/// <summary>
/// Insert this object into the specified cell.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setCell( cgSceneCell * cell )
{
    return setCell( cell, false );
}
bool cgObjectNode::setCell( cgSceneCell * cell, bool constructing )
{
    // Is this a no-op?
    if ( cell == mParentCell ) 
        return true;

    // Prepare the required queries if they aren't already.
    bool serializeData = (shouldSerialize() && !constructing &&  mParentScene->isSceneWritingEnabled());
    if ( serializeData )
        prepareQueries();

    // What action is taking place?
    if ( mParentCell && !cell )
    {
        // Remove from any prior cell.
        if ( serializeData )
        {
            mNodeDelete.bindParameter( 1, mReferenceId );
            if ( !mNodeDelete.step( true ) )
            {
                cgString error;
                mNodeDelete.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to remove object node '0x%x' from database. Error: %s\n"), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed
        
        } // End if scene item
        mParentCell->removeNode( this );

        // Reposition node relative to the world origin.
        if ( !constructing )
            mCellTransform.translate( mParentCell->getWorldOrigin() );

    } // End if detach
    else if ( !mParentCell )
    {
        cgVector3 vCellOrigin = cell->getWorldOrigin();
        if ( serializeData )
        {
            // What object?
            cgUInt32 objectTypeId = (mReferencedObject) ? mReferencedObject->getLocalTypeId() : 0;
            cgUInt32 objectRefId  = (mReferencedObject) ? mReferencedObject->getReferenceId() : 0;
            
            // Insert data.
            mNodeInsert.bindParameter( 1, mReferenceId );
            mNodeInsert.bindParameter( 2, (cgUInt32)cgNodeType::Persistent );
            mNodeInsert.bindParameter( 3, (cgUInt32)mNodeLevel );
            mNodeInsert.bindParameter( 4, cell->getCellId() );
            // ToDo: Flags not yet used.
            mNodeInsert.bindParameter( 6, objectTypeId );
            // ToDo: 9999 - ObjectClassId not yet used.
            mNodeInsert.bindParameter( 8, mRenderClassId );
            mNodeInsert.bindParameter( 9, objectRefId );
            mNodeInsert.bindParameter( 10, ( mParentNode ) ? mParentNode->mReferenceId : 0 );
            mNodeInsert.bindParameter( 11, ( mOwnerGroup ) ? mOwnerGroup->mReferenceId : 0 );
            mNodeInsert.bindParameter( 12, ( mTargetNode ) ? mTargetNode->mReferenceId : 0 );
            mNodeInsert.bindParameter( 13, mName );
            mNodeInsert.bindParameter( 14, cgString::Empty );
            mNodeInsert.bindParameter( 15, mInstanceIdentifier );

            // Decompose the cell transform for storage.
            cgQuaternion rotation;
            cgVector3 scale, shear, position;
            mCellTransform.decompose( scale, shear, rotation, position );
            
            // Store cell transform
            mNodeInsert.bindParameter( 16, position.x - vCellOrigin.x );
            mNodeInsert.bindParameter( 17, position.y - vCellOrigin.y );
            mNodeInsert.bindParameter( 18, position.z - vCellOrigin.z );
            mNodeInsert.bindParameter( 19, rotation.x );
            mNodeInsert.bindParameter( 20, rotation.y );
            mNodeInsert.bindParameter( 21, rotation.z );
            mNodeInsert.bindParameter( 22, rotation.w );
            mNodeInsert.bindParameter( 23, shear.x ); // XY
            mNodeInsert.bindParameter( 24, shear.y ); // XZ
            mNodeInsert.bindParameter( 25, shear.z ); // YZ
            mNodeInsert.bindParameter( 26, scale.x );
            mNodeInsert.bindParameter( 27, scale.y );
            mNodeInsert.bindParameter( 28, scale.z );

            // Decompose the offset transform for storage.
            mOffsetTransform.decompose( scale, shear, rotation, position );

            // Store offset transform
            mNodeInsert.bindParameter( 29, position.x );
            mNodeInsert.bindParameter( 30, position.y );
            mNodeInsert.bindParameter( 31, position.z );
            mNodeInsert.bindParameter( 32, rotation.x );
            mNodeInsert.bindParameter( 33, rotation.y );
            mNodeInsert.bindParameter( 34, rotation.z );
            mNodeInsert.bindParameter( 35, rotation.w );
            mNodeInsert.bindParameter( 36, shear.x ); // XY
            mNodeInsert.bindParameter( 37, shear.y ); // XZ
            mNodeInsert.bindParameter( 38, shear.z ); // YZ
            mNodeInsert.bindParameter( 39, scale.x );
            mNodeInsert.bindParameter( 40, scale.y );
            mNodeInsert.bindParameter( 41, scale.z );

            // Remaining properties
            mNodeInsert.bindParameter( 42, mColor );
            mNodeInsert.bindParameter( 43, (cgUInt32)mPhysicsModel );
            mNodeInsert.bindParameter( 44, (cgUInt32)mSimulationQuality );
            mNodeInsert.bindParameter( 45, (cgUInt32)mUpdateRate );
            mNodeInsert.bindParameter( 46, ((mFlags & cgObjectNodeFlags::Visible) != 0) );
            if ( !mNodeInsert.step( true ) )
            {
                cgString error;
                mNodeInsert.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if !internal

        // Reposition node relative to the new cell's world origin.
        if ( !constructing )
            mCellTransform.translate( -vCellOrigin );

        // Add to cell's list.
        cell->addNode( this );

    } // End if attach
    else
    {
        cgVector3 oldOrigin = mParentCell->getWorldOrigin();
        cgVector3 newOrigin = cell->getWorldOrigin();

        // We are just swapping cells.
        if ( serializeData )
        {
            // Update database including new cell relative position.
            mNodeUpdateCell.bindParameter( 1, cell->getCellId() );
            mNodeUpdateCell.bindParameter( 2, mCellTransform.position().x + oldOrigin.x - newOrigin.x );
            mNodeUpdateCell.bindParameter( 3, mCellTransform.position().y + oldOrigin.y - newOrigin.y );
            mNodeUpdateCell.bindParameter( 4, mCellTransform.position().z + oldOrigin.z - newOrigin.z );
            mNodeUpdateCell.bindParameter( 5, mReferenceId );
            if ( !mNodeUpdateCell.step( true ) )
            {
                cgString error;
                mNodeUpdateCell.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update parent cell reference for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                return false;
            
            } // End if failed

        } // End if scene item
        mParentCell->removeNode( this );
        cell->addNode( this );

        // Reposition node relative to the new cell's world origin.
        if ( !constructing )
            mCellTransform.translate( oldOrigin - newOrigin );
        
    } // End if swap

    // Store new cell.
    mParentCell = cell;

    // Inform any children that their parent node's cell was modified.
    for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
    {
        cgObjectNode * childNode = *itNode;
        if ( childNode )
            childNode->onParentCellChanged( );

    } // Next child node

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::prepareQueries()
{
    cgWorld * world = mParentScene->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mNodeInsert.isPrepared() )
        {
            cgString statement = _T("INSERT INTO 'Nodes' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23,?24,?25,?26,?27,?28,?29,?30,?31,?32,?33,?34,?35,?36,?37,?38,?39,?40,?41,?42,?43,?44,?45,?46)");
            mNodeInsert.prepare( world, statement, true );
        
        } // End if !prepared
        
        if ( !mNodeDelete.isPrepared() )
        {
            cgString statement = _T("DELETE FROM 'Nodes' WHERE RefId=?1");
            mNodeDelete.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateCell.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET CellId=?1,PositionX=?2,PositionY=?3,PositionZ=?4 WHERE RefId=?5");
            mNodeUpdateCell.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateTransform.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET PositionX=?1,PositionY=?2,PositionZ=?3,RotationX=?4,RotationY=?5,RotationZ=?6,RotationW=?7,ShearXY=?8,ShearXZ=?9,ShearYZ=?10,ScaleX=?11,ScaleY=?12,ScaleZ=?13 WHERE RefId=?14");
            mNodeUpdateTransform.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateOffsetTransform.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET OffsetPositionX=?1,OffsetPositionY=?2,OffsetPositionZ=?3,OffsetRotationX=?4,OffsetRotationY=?5,OffsetRotationZ=?6,OffsetRotationW=?7,OffsetShearXY=?8,OffsetShearXZ=?9,OffsetShearYZ=?10,OffsetScaleX=?11,OffsetScaleY=?12,OffsetScaleZ=?13 WHERE RefId=?14");
            mNodeUpdateOffsetTransform.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateColor.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET EditorColor=?1 WHERE RefId=?2");
            mNodeUpdateColor.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateName.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET EditorName=?1 WHERE RefId=?2");
            mNodeUpdateName.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateInstanceIdentifier.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET InstanceIdentifier=?1 WHERE RefId=?2");
            mNodeUpdateInstanceIdentifier.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateParent.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET ParentRefId=?1,Level=?2 WHERE RefId=?3");
            mNodeUpdateParent.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateLevel.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET Level=?1 WHERE RefId=?2");
            mNodeUpdateLevel.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateGroup.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET OwnerGroup=?1 WHERE RefId=?2");
            mNodeUpdateGroup.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdatePhysicsProperties.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET PhysicsModel=?1,SimulationQuality=?2 WHERE RefId=?3");
            mNodeUpdatePhysicsProperties.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateUpdateRate.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET UpdateRate=?1 WHERE RefId=?2");
            mNodeUpdateUpdateRate.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateVisibility.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET Visible=?1 WHERE RefId=?2");
            mNodeUpdateVisibility.prepare( world, statement, true );
        
        } // End if !prepared

        if ( !mNodeUpdateTargetReference.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes' SET TargetId=?1 WHERE RefId=?2");
            mNodeUpdateTargetReference.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeClearCustomProperties.isPrepared() )
        {
            cgString statement = _T("DELETE FROM 'Nodes::CustomProperties' WHERE NodeId=?1");
            mNodeClearCustomProperties.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeRemoveCustomProperty.isPrepared() )
        {
            cgString statement = _T("DELETE FROM 'Nodes::CustomProperties' WHERE Name=?1 AND NodeId=?2");
            mNodeRemoveCustomProperty.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeUpdateCustomProperty.isPrepared() )
        {
            cgString statement = _T("UPDATE 'Nodes::CustomProperties' SET Type=?1, Value=?2 WHERE Name=?3 AND NodeID=?4");
            mNodeUpdateCustomProperty.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeInsertCustomProperty.isPrepared() )
        {
            cgString statement = _T("INSERT INTO 'Nodes::CustomProperties' VALUES (NULL,?1,?2,?3,?4)");
            mNodeInsertCustomProperty.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeDeleteBehavior.isPrepared() )
        {
            cgString statement = _T("DELETE FROM 'Nodes::Behaviors' WHERE BehaviorId=?1");
            mNodeDeleteBehavior.prepare( world, statement, true );

        } // End if !prepared

        if ( !mNodeInsertBehavior.isPrepared() )
        {
            cgString statement = _T("INSERT INTO 'Nodes::Behaviors' VALUES (NULL,?1,?2,?3,?4,?5,?6)");
            mNodeInsertBehavior.prepare( world, statement, true );

        } // End if !prepared

    } // End if sandbox

    if ( !mNodeLoadTransforms.isPrepared() )
    {
        // Select the node transform data and the /original/ parent cell location
        // (as it existed prior to the update) so that we can reconstruct a valid
        // cell transform. We use 'LEFT OUTER JOIN' so that even if a cell could not
        // be found, we won't simply fail.
        cgString statement = _T("SELECT Nodes.PositionX, Nodes.PositionY, Nodes.PositionZ, Nodes.RotationX, Nodes.RotationY, ")
                          _T("Nodes.RotationZ, Nodes.RotationW, Nodes.ShearXY, Nodes.ShearXZ, Nodes.ShearYZ, Nodes.ScaleX, ")
                          _T("Nodes.ScaleY, Nodes.ScaleZ, Nodes.OffsetPositionX, Nodes.OffsetPositionY, Nodes.OffsetPositionZ, ")
                          _T("Nodes.OffsetRotationX, Nodes.OffsetRotationY, Nodes.OffsetRotationZ, Nodes.OffsetShearXY, ")
                          _T("Nodes.OffsetShearXZ, Nodes.OffsetShearYZ, Nodes.OffsetScaleX, Nodes.OffsetScaleY, Nodes.OffsetScaleZ, ")
                          _T("Cells.LocationX, Cells.LocationY, Cells.LocationZ FROM 'Nodes' LEFT OUTER JOIN 'Cells' USING (CellId) ")
                          _T("WHERE RefId=?1");
        mNodeLoadTransforms.prepare( world, statement, true );
    
    } // End if !prepared

    if ( !mNodeLoadCustomProperties.isPrepared() )
    {
        cgString statement = _T("SELECT * FROM [Nodes::CustomProperties] WHERE NodeId=?1");
        mNodeLoadCustomProperties.prepare( world, statement, true );

    } // End if !prepared

    if ( !mNodeLoadBehaviors.isPrepared() )
    {
        cgString statement = _T("SELECT * FROM [Nodes::Behaviors] WHERE NodeId=?1 ORDER BY LoadOrder ASC");
        mNodeLoadBehaviors.prepare( world, statement, true );

    } // End if !prepared
}

//-----------------------------------------------------------------------------
//  Name : getParent ()
/// <summary>
/// Return the parent node of which this is a child (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::getParent() const
{
    return mParentNode;
}

//-----------------------------------------------------------------------------
//  Name : getChildren ()
/// <summary>
/// Retrieve the list of child nodes attached to this one.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeList & cgObjectNode::getChildren( )
{
    return mChildren;
}

//-----------------------------------------------------------------------------
//  Name : findChild() (Recursive)
/// <summary>
/// Search for a child of the current node that has a matching instance
/// identifier. By default, this method will perform a recursive search through
/// the entire hierarchy below this node.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::findChild( const cgString & instanceId )
{
    for ( cgObjectNodeList::iterator itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        // Child has a matching identifier?
        if ( (*itChild)->mInstanceIdentifier == instanceId )
            return *itChild;
        
        // Ask it to search its children.
        cgObjectNode * result = (*itChild)->findChild( instanceId );
        if ( result )
            return result;

    } // Next child

    // Nothing found.
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findChild() (Recursive)
/// <summary>
/// Search for a child of the current node that has a matching instance
/// identifier. The recursive parameter allows the caller to control whether or
/// not this method will perform a recursive search through the entire 
/// hierarchy below this node, or only its immediate children.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::findChild( const cgString & instanceId, bool recursive )
{
    for ( cgObjectNodeList::iterator itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        // Child has a matching identifier?
        if ( (*itChild)->mInstanceIdentifier == instanceId )
            return *itChild;

        // Ask it to search its children.
        if ( recursive )
        {
            cgObjectNode * result = (*itChild)->findChild( instanceId );
            if ( result )
                return result;

        } // End if recursive

    } // Next child

    // Nothing found.
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findChildOfType() (Recursive)
/// <summary>
/// Search for a child of the current node that references an object with the
/// specified type identifier. By default, this method will perform a recursive
/// search through the entire hierarchy below this node.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::findChildOfType( const cgUID & objectType )
{
    for ( cgObjectNodeList::iterator itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        // Child references an object of the requested type?
        if ( (*itChild)->queryObjectType( objectType ) )
            return *itChild;

        // Ask it to search its children.
        cgObjectNode * result = (*itChild)->findChildOfType( objectType );
        if ( result )
            return result;

    } // Next child

    // Nothing found.
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findChildOfType() (Recursive)
/// <summary>
/// Search for a child of the current node that references an object with the
/// specified type identifier. The recursive parameter allows the caller to 
/// control whether or not this method will perform a recursive search through 
/// the entire hierarchy below this node, or only its immediate children.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgObjectNode::findChildOfType( const cgUID & objectType, bool recursive )
{
    for ( cgObjectNodeList::iterator itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        // Child references an object of the requested type?
        if ( (*itChild)->queryObjectType( objectType ) )
            return *itChild;

        // Ask it to search its children.
        if ( recursive )
        {
            cgObjectNode * result = (*itChild)->findChildOfType( objectType );
            if ( result )
                return result;

        } // End if recursive

    } // Next child

    // Nothing found.
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : hasPendingUpdates ()
/// <summary>
/// Determine if any node updates are pending resolution.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::hasPendingUpdates( ) const
{
    return (mPendingUpdates != 0);
}

//-----------------------------------------------------------------------------
//  Name : getPendingUpdates ()
/// <summary>
/// Retrieve the bit flags that describe which node updates are pending
/// resolution if any.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgObjectNode::getPendingUpdates( ) const
{
    return mPendingUpdates;
}

//-----------------------------------------------------------------------------
//  Name : getLastDirtyFrame ()
/// <summary>
/// Retrieve the frame on which this object node was last updated.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgObjectNode::getLastDirtyFrame( ) const
{
    return mLastDirtyFrame;
}

//-----------------------------------------------------------------------------
//  Name : resolvePendingUpdates ()
/// <summary>
/// Allow the node to resolve any information which may have been outstanding
/// such as the update of its world space transforms, bounding box or scene 
/// spatial tree ownership data subsequent to the completion a larger update 
/// process.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::resolvePendingUpdates( cgUInt32 updateMask )
{
    // Anything to do?
    cgUInt32 updates = (mPendingUpdates & updateMask);
    if ( !updates )
        return;

    // If updates to the object ownership were requested on their own,
    // but the bounding box is out of date, we must recompute that too
    // regardless of the user's selected update mask.
    if ( (updates & cgDeferredUpdateFlags::OwnershipStatus) &&
         (mPendingUpdates & cgDeferredUpdateFlags::BoundingBox) )
        updates |= cgDeferredUpdateFlags::BoundingBox;

    // If updates to the bounding box were requested on their own, but 
    // the transforms are out of date, we must recompute that too regardless
    // of the user's selected update mask.
    if ( (updates & cgDeferredUpdateFlags::BoundingBox) &&
         (mPendingUpdates & cgDeferredUpdateFlags::Transforms) )
        updates |= cgDeferredUpdateFlags::Transforms;

    // In the case of transform updates, we must resolve all nodes above us
    // that have outstanding transform updates first.
    if ( updates & cgDeferredUpdateFlags::Transforms )
    {
        cgObjectNodeArray resolveOrder;
        cgObjectNode * currentNode = this;
        while ( currentNode = currentNode->getParent() )
        {
            if ( currentNode->mPendingUpdates & cgDeferredUpdateFlags::Transforms )
                resolveOrder.push_back( currentNode );

        } // End if parent search

        // Resolve transforms in top to bottom order.
        for ( cgInt i = (cgInt)resolveOrder.size() - 1; i >= 0; --i )
        {
            currentNode = resolveOrder[i];
            currentNode->onResolvePendingUpdates( cgDeferredUpdateFlags::Transforms );

            // Transforms have been resolved for this node.
            if ( !currentNode->mPendingUpdates && mParentScene )
                mParentScene->resolvedNodeUpdates( currentNode );

        } // Next node

    } // End if parents resolved

    // Now we can resolve ourselves.
    onResolvePendingUpdates( updates );
    if ( !(mPendingUpdates &~ cgDeferredUpdateFlags::Unload) && mParentScene )
        mParentScene->resolvedNodeUpdates( this );

    // Were we due to unload?
    if ( mPendingUpdates & cgDeferredUpdateFlags::Unload )
    {
        mPendingUpdates &= ~cgDeferredUpdateFlags::Unload;
        unload();
    
    } // End if unload pending
}

//-----------------------------------------------------------------------------
//  Name : onResolvePendingUpdates () (Virtual)
/// <summary>
/// Triggered when any pending deferred updates need to be resolved. Allows 
/// derived classes to take additional action during this phase.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onResolvePendingUpdates( cgUInt32 updates )
{   
    // Generate new cell transform based on parent transform updates if required.
    if ( updates & cgDeferredUpdateFlags::Transforms )
    {
        // Note: We ignore the transform source and assume 'TransformResolve' in order
        // to ensure that the local transform of the node is not altered, nor will updates
        // be deferred for a second time during the resolution phase. In addition, irrespective
        // of whether or not the adjustment was due to a dynamics update, the physics body of 
        // any child object will be forcibly updated to match the new child transform. We 
        // acknowledge that this is not a valid  dynamics update but have no choice but to obey it.
        mPendingUpdates &= ~cgDeferredUpdateFlags::Transforms;
        if ( mParentNode )
            setCellTransform( mLocalTransform * mParentNode->getCellTransform(), cgTransformSource::TransformResolve );
        else
            setCellTransform( mLocalTransform, cgTransformSource::TransformResolve );

    } // End if update transform

    // Update the world space bounding box on request.
    if ( updates & cgDeferredUpdateFlags::BoundingBox )
    {
        mPendingUpdates &= ~cgDeferredUpdateFlags::BoundingBox;
        mWorldBounds = getLocalBoundingBox();
        mWorldBounds.transform( getWorldTransform( false ) );
    
    } // End if update bounds

    // Update the node's ownership status in order to update its scene cell 
    // where applicable. This process will also filter the object through to 
    // any child spatial trees where it may be inserted into the relevant 
    // leaves as required. The exception are those nodes that have a reference
    // identifier of 0. These are temporary internal objects that cannot / should
    // not be referenced.
    if ( updates & cgDeferredUpdateFlags::OwnershipStatus )
    {
        mPendingUpdates &= ~cgDeferredUpdateFlags::OwnershipStatus;
        if ( mParentScene && mReferenceId )
            mParentScene->updateObjectOwnership( this );

    } // End if update ownership
}

//-----------------------------------------------------------------------------
//  Name : onAnimationTransformUpdated () (Virtual)
/// <summary>
/// Called by the animation controller whenever the position, orientation or 
/// scale of this animation target needs to be updated in some way.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onAnimationTransformUpdated( const cgTransform & transform )
{
    // Replace the current local (parent relative) transform. Incoming
    // transformation is assumed to be relative to the parent's *actual*
    // transform, not its pivot frame.
    if ( mParentNode )
        mLocalTransform = transform * mParentNode->mOffsetTransform;
    else
        mLocalTransform = transform;
    
    // Defer computation of further details until process is complete.
    // These same updates need to flood through our children too.
    cgUInt32 deferredFlags = cgDeferredUpdateFlags::BoundingBox |
                             cgDeferredUpdateFlags::OwnershipStatus |
                             cgDeferredUpdateFlags::Transforms;

    // Add to update queue
    nodeUpdated( deferredFlags, deferredFlags );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationTransform () (Virtual)
/// <summary>
/// Retrieve the current animation transform for this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::getAnimationTransform( cgTransform & transform ) const
{
    if ( mParentNode )
    {
        cgTransform invParentOffset;
        cgTransform::inverse( invParentOffset, mParentNode->mOffsetTransform );
        cgTransform::multiply( transform, mLocalTransform, invParentOffset );
    }
    else
        transform = mLocalTransform;
}

//-----------------------------------------------------------------------------
//  Name : nodeUpdated ()
/// <summary>
/// Mark the node as having been updated in this frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::nodeUpdated( cgUInt32 deferredUpdates, cgUInt32 childDeferredUpdates )
{
    cgUInt32 currentFrame = cgTimer::getInstance()->getFrameCounter();

    // Mark node as having been modified in this frame.
    mLastDirtyFrame = currentFrame;

    // Update local update tracking member.
    cgUInt32 oldUpdates = mPendingUpdates;
    mPendingUpdates |= deferredUpdates;
    
    // Any updates supplied?
    if ( mPendingUpdates != oldUpdates )
    {
        // Queue any deferred updates that were requested.
        if ( mParentScene && !oldUpdates )
            mParentScene->queueNodeUpdates( this );

        // Trigger updates to bounding box of any owner group.
        if ( mOwnerGroup && (deferredUpdates & cgDeferredUpdateFlags::BoundingBox) && 
            !(mOwnerGroup->getPendingUpdates() & cgDeferredUpdateFlags::BoundingBox) )
        {
            // Group needs to have its bounding box recomputed based on the new bounding
            // box of any of its children.
            cgUInt32 groupUpdates = cgDeferredUpdateFlags::BoundingBox | cgDeferredUpdateFlags::OwnershipStatus;
            mOwnerGroup->nodeUpdated( groupUpdates, 0 );

        } // End if

    } // End if defer updates

    // Recurse into children if the caller requested that we flood changes through the 
    // child hierarchy.
    if ( childDeferredUpdates && !mChildren.empty())
    {
        for ( cgObjectNodeList::iterator itNode = mChildren.begin(); itNode != mChildren.end(); ++itNode )
            (*itNode)->nodeUpdated( childDeferredUpdates, childDeferredUpdates );

    } // End if queue children
}

//-----------------------------------------------------------------------------
//  Name : registerVisibility () (Virtual)
/// <summary>
/// This node has been deemed visible during testing, but this method gives the
/// node a final say on how it gets registered with the visibility set. The
/// default behavior is simply to insert directly into object visibility list,
/// paying close attention to its filtering rules.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::registerVisibility( cgVisibilitySet * visibilityData )
{
    cgUInt32 flags = visibilityData->getSearchFlags();

    // First test filters.
    if ( (flags & cgVisibilitySearchFlags::MustCastShadows) && !isShadowCaster() )
        return false;
    if ( (flags & cgVisibilitySearchFlags::MustRender) && !isRenderable() )
        return false;
    
    // We're visible. Add to the set.
    return visibilityData->addVisibleObject( this );
}

//-----------------------------------------------------------------------------
//  Name : unregisterVisibility () (Virtual)
/// <summary>
/// Unregisters this oject from the specified visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::unregisterVisibility( cgVisibilitySet * visibilityData )
{
    visibilityData->removeVisibleObject( this );
}

//-----------------------------------------------------------------------------
//  Name : isNodeDirty () (Virtual)
/// <summary>
/// Determine if the node has been modified in this frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isNodeDirty() const
{
    return (mLastDirtyFrame == cgTimer::getInstance()->getFrameCounter());
}

//-----------------------------------------------------------------------------
//  Name : isNodeDirtySince () (Virtual)
/// <summary>
/// Determine if the node has been modified at some point after the specified
/// frame.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isNodeDirtySince( cgUInt32 frame ) const
{
    return (mLastDirtyFrame >= frame);
}

//-----------------------------------------------------------------------------
//  Name : isShadowCaster () (Virtual)
/// <summary>
/// Is the node capable of casting shadows? (i.e. a camera may not be)
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isShadowCaster() const
{
    // Pass through to referenced object.
    if ( !mReferencedObject )
        return false;
    return mReferencedObject->isShadowCaster();
}

//-----------------------------------------------------------------------------
//  Name : isSelected () (Virtual)
/// <summary>
/// Is the node currently selected for manipulation?
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::isSelected( ) const
{
    return (mFlags & cgObjectNodeFlags::Selected) != 0;
}

//-----------------------------------------------------------------------------
//  Name : setSelected () (Virtual)
/// <summary>
/// Set the node's selection status.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setSelected( bool selected, bool updateDependents /* = true */, bool sendNotifications /* = true */, cgObjectNodeMap & alteredNodes /* = cgObjectNodeMap() */  )
{
    // Skip if this is a no-op
    if ( ((mFlags & cgObjectNodeFlags::Selected) != 0) == selected )
        return;

    // Update selected flag
    mFlags &= ~cgObjectNodeFlags::Selected;
    if ( selected )
        mFlags |= cgObjectNodeFlags::Selected;

    // We were modified.
    alteredNodes[getReferenceId()] = this;

    // Should we update the scene and other dependents, or just update our internal status?
    if ( updateDependents )
    {
        // Add / remove as appropriate from the scene's selection list.
        cgObjectNodeMap & selectedNodes = mParentScene->getSelectedNodes();
        cgObjectNodeMap & selectedNodesOrdered = mParentScene->getSelectedNodesOrdered();
        if ( !selected )
        {
            selectedNodes.erase( mReferenceId );
            selectedNodesOrdered.erase( mSelectionId );
        
        } // End if deselecting
        else
        {
            mSelectionId = mParentScene->getNextSelectionId();
            selectedNodesOrdered[mSelectionId] = this;
            selectedNodes[mReferenceId] = this;
        
        } // End if selecting

        // If we were owned by a closed group, alter the group's selection too (this
        // will automatically return if it is already selected).
        if ( isMergedAsGroup() )
        {
            mOwnerGroup->setSelected( selected, true, sendNotifications, alteredNodes );
        
        } // End if select group
        else
        {
            // Notify whoever is listening that our selection status 
            // changed if requested.
            if ( sendNotifications )
                mParentScene->onSelectionUpdated( &cgSelectionUpdatedEventArgs( mParentScene, &alteredNodes ) );

        } // End if no group
        
    } // End if update scene
}

//-----------------------------------------------------------------------------
// Name : clone ()
/// <summary>
/// Make a duplicate of this node and optionally its referenced object and 
/// data. This method will return false if the clone could not be created,
/// or cloning of nodes of this type is disallowed (see canClone()).
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::clone( cgCloneMethod::Base method, cgScene * scene, bool internalNode, cgObjectNode *& nodeOut, const cgTransform & initTransform )
{
    // Be polite and clear output variables
    nodeOut = CG_NULL;

    // Can be cloned?
    if ( !canClone() )
        return false;

    // Clones are always 'internal' when not in full sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        internalNode = true;

    // Clone the node
    nodeOut = scene->createObjectNode( internalNode, getObjectType(), false, method, this, initTransform );

    // Success?
    return (nodeOut != CG_NULL);
}

//-----------------------------------------------------------------------------
// Name : setTargetMethod ( ) (Virtual)
/// <summary>
/// Set the current target method for this node. The target method describes
/// how and when a target node will be created that the application can use to 
/// manipulate the orientation of this node by adjusting the position of that 
/// node. Useful for light sources, cameras, character's head / eyes, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setTargetMethod( cgNodeTargetMethod::Base method )
{
    // Do we already have an associated target node?
    if ( mTargetNode )
    {
        // Is this a no-op?
        if ( method == mTargetNode->getMode() )
            return;

        // Target already exists. Check to see if we are deleting
        // first of all.
        if ( method == cgNodeTargetMethod::NoTarget )
        {
            // We are deleting. Update database.
            if ( shouldSerialize() && !mTargetNode->isInternalReference() )
            {
                prepareQueries();
                mNodeUpdateTargetReference.bindParameter( 1, 0 );
                mNodeUpdateTargetReference.bindParameter( 2, mReferenceId );
                if ( !mNodeUpdateTargetReference.step( true ) )
                {
                    cgString error;
                    mNodeUpdateTargetReference.getLastError( error );
                    cgAppLog::write( cgAppLog::Error, _T("Failed to update target identifier for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                    return;

                } // End if failed

            } // End if exists in DB

            // We are deleting, instruct the target to destroy itself.
            mTargetNode->setTargetingNode( CG_NULL );
            mTargetNode = CG_NULL;
            return;
        
        } // End if

        // Now we need to reposition the target if the application switched
        // the target axis. We should first unlock the target before we reposition 
        // it in order to ensure that this node is not modified during that process.
        mTargetNode->setMode( cgNodeTargetMethod::Unlocked );
        // ToDo: 9999 - Reposition target node as necessary.
        
        // Select new targeting mode.
        mTargetNode->setMode( method );

    } // End if existing target
    else
    {
        // No target node currently exists. Check to see if this 
        // is a a no-op before we get to any serious business.
        if ( method == cgNodeTargetMethod::NoTarget )
            return;

        // All other modes assume that a target has been created
        // so do this next.
        if ( mParentScene )
            mTargetNode = (cgTargetNode*)mParentScene->createObjectNode( isInternalReference(), RTID_TargetObject, false );
        if ( !mTargetNode )
            return;

        // Set the initial position of the target appropriately 
        // just at the edge of the bounding box (default is +Z).
        cgMatrix m;
        cgMatrix::identity( m );
        cgBoundingBox Bounds = getLocalBoundingBox();
        switch ( method )
        {
            case cgNodeTargetMethod::XAxis:
                (cgVector3&)m._11 = -getZAxis();
                (cgVector3&)m._21 = getYAxis();
                (cgVector3&)m._31 = getXAxis();
                (cgVector3&)m._41 = getPosition() + (getXAxis() * -Bounds.getPlane(cgVolumePlane::Right).d);
                break;

            case cgNodeTargetMethod::YAxis:
                (cgVector3&)m._11 = getXAxis();
                (cgVector3&)m._21 = -getZAxis();
                (cgVector3&)m._31 = getYAxis();
                (cgVector3&)m._41 = getPosition() + (getYAxis() * -Bounds.getPlane(cgVolumePlane::Top).d);
                break;

            case cgNodeTargetMethod::ZAxis:
            case cgNodeTargetMethod::Relative:
            case cgNodeTargetMethod::Unlocked:
                (cgVector3&)m._11 = getXAxis();
                (cgVector3&)m._21 = getYAxis();
                (cgVector3&)m._31 = getZAxis();
                (cgVector3&)m._41 = getPosition() + (getZAxis() * -Bounds.getPlane(cgVolumePlane::Far).d);
                break;

        } // End Switch method

        // Attach!
        mTargetNode->setWorldTransform( m );
        mTargetNode->setTargetingNode( this );
        mTargetNode->setMode( method );

        // Update database.
        if ( shouldSerialize() && !mTargetNode->isInternalReference() )
        {
            prepareQueries();
            mNodeUpdateTargetReference.bindParameter( 1, mTargetNode->getReferenceId() );
            mNodeUpdateTargetReference.bindParameter( 2, mReferenceId );
            if ( !mNodeUpdateTargetReference.step( true ) )
            {
                cgString error;
                mNodeUpdateTargetReference.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to update target identifier for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                return;

            } // End if failed

        } // End if exists in DB

    } // End if no existing target
}

//-----------------------------------------------------------------------------
// Name : getTargetMethod ( )
/// <summary>
/// Retrieve the current target method for this node. The target method describes
/// how and when a target node will be created that the application can use to 
/// manipulate the orientation of this node by adjusting the position of that 
/// node. Useful for light sources, cameras, character's head / eyes, etc.
/// </summary>
//-----------------------------------------------------------------------------
cgNodeTargetMethod::Base cgObjectNode::getTargetMethod( ) const
{
    if ( !mTargetNode )
        return cgNodeTargetMethod::NoTarget;
    return mTargetNode->getMode();
}

//-----------------------------------------------------------------------------
// Name : getTargetNode ( )
/// <summary>
/// Retrieve the target associated with this node if one exists.
/// </summary>
//-----------------------------------------------------------------------------
cgTargetNode * cgObjectNode::getTargetNode( ) const
{
    return mTargetNode;
}

//-----------------------------------------------------------------------------
// Name : validateAttachment ( ) (Virtual)
/// <summary>
/// Allows nodes to describe whether or not a particular attachment of
/// another node as either a child or parent of this node is valid. This method
/// is automatically called by 'setParent()' when an attachment is due to take
/// place, and if either node does not allow the attachment the operation will
/// fail.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::validateAttachment( cgObjectNode * node, bool nodeAsChild )
{
    // Default implementation always allows attachment as child / parent.
    return true;
}

//-----------------------------------------------------------------------------
// Name : canSetName ( )
/// <summary>Determine if name setting is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canSetName( ) const
{
    // Default is to allow setting of name
    return true;
}

//-----------------------------------------------------------------------------
// Name : canDelete ( )
/// <summary>Determine if deletion is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canDelete( ) const
{
    // Default is to allow deletion
    return true;
}

//-----------------------------------------------------------------------------
// Name : canScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canScale( ) const
{
    // Default is to allow scaling.
    return true;
}

//-----------------------------------------------------------------------------
// Name : canRotate ( )
/// <summary>Determine if rotation of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canRotate( ) const
{
    // Default is to allow rotation.
    return true;
}

//-----------------------------------------------------------------------------
// Name : canAdjustPivot ( )
/// <summary>
/// Determine if separation of this node's pivot and object space is allowed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canAdjustPivot( ) const
{
    // Default is to allow pivot adjustment.
    return true;
}

//-----------------------------------------------------------------------------
// Name : canClone ( )
/// <summary>
/// Determine if node's of this type can be cloned.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::canClone( ) const
{
    // Default is to allow cloning.
    return true;
}

//-----------------------------------------------------------------------------
// Name : allowSandboxUpdate ( )
/// <summary>
/// Return true if the node should have its update method called even when
/// not running a preview in sandbox mode.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::allowSandboxUpdate( ) const
{
    // Default is to disallow updates.
    return false;
}

//-----------------------------------------------------------------------------
// Name : getNodeColor ( )
/// <summary>Get the node's current default color.</summary>
//-----------------------------------------------------------------------------
cgUInt32 cgObjectNode::getNodeColor( ) const
{
    return mColor;
}

//-----------------------------------------------------------------------------
// Name : setNodeColor ( )
/// <summary>Set the node's current default color.</summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::setNodeColor( cgUInt32 color )
{
    // Update database reference.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdateColor.bindParameter( 1, color );
        mNodeUpdateColor.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateColor.step( true ) )
        {
            cgString error;
            mNodeUpdateColor.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update base color information for object node '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if exists in DB

    // Update local member
    mColor = color;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : shouldSerialize ( )
/// <summary>
/// Determine if we should serialize our information to the currently open
/// world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::shouldSerialize( ) const
{
    return ((cgGetSandboxMode() == cgSandboxMode::Enabled) && !mDisposing && 
            !isInternalReference() && !mParentScene->isDisposing() && 
            !mParentScene->isLoading());
}

//-----------------------------------------------------------------------------
// Name : showSelectionAABB ( )
/// <summary>
/// Should the system automatically draw an AABB around the node when it is
/// in a selected state and rendered in a non-wireframe view?
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::showSelectionAABB( ) const
{
    // Default is to allow drawing only if we don't belong to a closed group.
    if ( isMergedAsGroup() )
        return false;
    
    // Allow drawing
    return true;
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyTransformed ( )
/// <summary>
/// Triggered whenever the physics body designed represent this object during
/// scene dynamics processing is transformed as a result of its simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e )
{
    // Ignore updates that were not due to dynamics, and did not 
    // originate from our local physics body.
    if ( !e->dynamicsUpdate || sender != mPhysicsBody )
        return;

    // Switch to standard transformation method.
    cgTransformMethod::Base oldMethod = getTransformMethod();
    setTransformMethod( cgTransformMethod::Standard );

    // Compute the new cell relative version if necessary.
    if ( mParentCell )
    {
        cgTransform newTransform = e->newTransform;
        newTransform.translate( -mParentCell->getWorldOrigin() );
        setCellTransform( newTransform, cgTransformSource::Dynamics );

    } // End if has parent cell
    else
    {
        setCellTransform( e->newTransform, cgTransformSource::Dynamics );

    } // End if no parent cell

    // Restore old transformation method.
    setTransformMethod( oldMethod );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyCollisionBegin ( )
/// <summary>
/// Triggered whenever the physics body first comes into contact with another
/// body in the scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onPhysicsBodyCollisionBegin( cgPhysicsBody * sender, cgPhysicsBodyCollisionEventArgs * e )
{
    // Filter out messages not intended for us.
    if ( sender != mPhysicsBody )
        return;

    // Notify behaviors.
    for ( size_t i = 0; i < mBehaviors.size(); ++i )
        mBehaviors[i]->onCollisionBegin( &cgNodeCollision(this, (cgObjectNode*)e->collision->otherBody->getUserData(), *e->collision) );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyCollisionContinue ( )
/// <summary>
/// Triggered whenever the physics body continues to be in contact with another
/// body in the scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onPhysicsBodyCollisionContinue( cgPhysicsBody * sender, cgPhysicsBodyCollisionEventArgs * e )
{
    // Filter out messages not intended for us.
    if ( sender != mPhysicsBody )
        return;

    // Notify behaviors.
    for ( size_t i = 0; i < mBehaviors.size(); ++i )
        mBehaviors[i]->onCollisionContinue( &cgNodeCollision(this, (cgObjectNode*)e->collision->otherBody->getUserData(), *e->collision) );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyCollisionEnd ( )
/// <summary>
/// Triggered whenever the physics body is no longer in contact with another
/// body in the scene.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onPhysicsBodyCollisionEnd( cgPhysicsBody * sender, cgPhysicsBodyCollisionEventArgs * e )
{
    // Filter out messages not intended for us.
    if ( sender != mPhysicsBody )
        return;

    // Notify behaviors.
    for ( size_t i = 0; i < mBehaviors.size(); ++i )
        mBehaviors[i]->onCollisionEnd( &cgNodeCollision(this, (cgObjectNode*)e->collision->otherBody->getUserData(), *e->collision) );
}

//-----------------------------------------------------------------------------
// Name : onNavigationAgentReposition ( )
/// <summary>
/// Triggered whenever the navigation agent designed represent this object
/// during scene navigation is repositioned as a result of its update process.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::onNavigationAgentReposition ( cgNavigationAgent * sender, cgNavigationAgentRepositionEventArgs * e )
{
    // Ignore updates that were not due to navigation update, and did not 
    // originate from our local physics body.
    if ( !e->navigationUpdate || sender != mNavigationAgent )
        return;

    // Switch to standard transformation method.
    cgTransformMethod::Base oldMethod = getTransformMethod();
    setTransformMethod( cgTransformMethod::Standard );

    // Generate orientation.
    cgTransform newTransform = getCellTransform();
    cgVector3 velocity = mNavigationAgent->getDesiredVelocity();
    cgFloat length = cgVector3::length( velocity );
    if ( !_isnan(length) && length > 0.001f && mNavigationAgent->getTargetState() != cgNavigationTargetState::Arrived )
    {
        // Retrieve the current orientation.
        cgQuaternion oldOrientation = newTransform.orientation();

        // Compute new orientation
        cgVector3 y( 0, 1, 0 ), x, z;
        z = velocity / length; // Normalize
        cgVector3::cross( x, y, z );
        newTransform.setOrientation( x, y, z );

        // Smoothly interpolate.
        cgQuaternion newOrientation = newTransform.orientation();
        cgQuaternion::slerp( newOrientation, oldOrientation, newOrientation, (cgFloat)(cgTimer::getInstance()->getTimeElapsed() * 4) );
        newTransform.setOrientation( newOrientation );

    } // End if has direction

    // Compute the new cell relative position
    if ( mParentCell )
        newTransform.setPosition( e->newPosition - mParentCell->getWorldOrigin() );
    else
        newTransform.setPosition( e->newPosition );

    // Set back to node.
    setCellTransform( newTransform, cgTransformSource::Navigation );

    // Restore old transformation method.
    setTransformMethod( oldMethod );
}

//-----------------------------------------------------------------------------
// Name : buildPhysicsBody ( ) (Virtual, Protected)
/// <summary>
/// Construct the internal physics body designed to represent this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::buildPhysicsBody( )
{
    if ( mPhysicsModel != cgPhysicsModel::None )
    {
        // Search through collision shapes in the referenced object if any.
        const cgObjectSubElementArray & objectShapes = getSubElements( OSECID_CollisionShapes );

        // Iterate through the shape sub elements and construct a list
        // of matching physics shape objects.
        cgArray<cgPhysicsShape*> physicsShapes;
        for ( size_t i = 0; i < objectShapes.size(); ++i )
        {
            cgPhysicsShape * shape = ((cgCollisionShapeElement*)objectShapes[i])->generatePhysicsShape( mParentScene->getPhysicsWorld() );
            if ( shape )
                physicsShapes.push_back( shape );

        } // Next sub-element

        // If there was more than one shape, we need to build a compound shape.
        // If there were *no* shapes however, assign a 'null' shape that allows 
        // the object to act like a rigid body without any collision geometry.
        cgPhysicsShape * primaryShape = CG_NULL;
        if ( physicsShapes.size() > 1 )
        {
            cgToDoAssert( "Physics", "Compound shape!" );
            if ( mPhysicsBody )
            {
                mPhysicsBody->unregisterEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );
                mPhysicsBody->removeReference( this );
            
            } // End if exists
            for ( size_t i = 0; i < physicsShapes.size(); ++i )
                physicsShapes[i]->deleteReference();
            mPhysicsBody = CG_NULL;
            return;
        
        } // End if needs compound
        else if ( physicsShapes.empty() )
        {
            // Create the 'null' shape that allows the object to act like a 
            // rigid body without any collision geometry.
            primaryShape = new cgNullShape( mParentScene->getPhysicsWorld() );
        
        } // End if no shapes
        else
        {
            primaryShape = physicsShapes[0];
        
        } // End if single shape

        // Configure new rigid body.
        cgRigidBodyCreateParams cp;
        cp.model            = mPhysicsModel;
        cp.initialTransform = getWorldTransform(false);
        cp.quality          = mSimulationQuality;
        cp.mass             = 0;

        // Compute final mass from the object-space 'base' mass that is 
        // scaled by this node's scaling transform component.
        if ( mPhysicsModel == cgPhysicsModel::RigidDynamic )
        {
            cp.mass = getBaseMass();
            cgFloat massTransformAmount = getMassTransformAmount();
            cgVector3 scale = getWorldTransform(false).localScale();
            cp.mass *= 1.0f + (scale.x - 1.0f) * massTransformAmount;
            cp.mass *= 1.0f + (scale.y - 1.0f) * massTransformAmount;
            cp.mass *= 1.0f + (scale.z - 1.0f) * massTransformAmount;
        
        } // End if dynamic

        cgToDo( "Physics", "Optionally set center of mass as pivot point." );

        // Construct a new rigid body object. Copy any necessary
        // simulation details from the existing rigid body such as
        // current velocity, torque, etc.
        cgRigidBody * rigidBody = new cgRigidBody( mParentScene->getPhysicsWorld(), primaryShape, cp, mPhysicsBody );

        // Release the previous physics body (if any).
        if ( mPhysicsBody )
        {
            mPhysicsBody->unregisterEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );
            mPhysicsBody->removeReference( this );
        
        } // End if exists

        // Take ownership of the new rigid body.
        mPhysicsBody = rigidBody;
        mPhysicsBody->addReference( this );
        mPhysicsBody->setUserData( this );

        // Listen for events
        mPhysicsBody->registerEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );

    } // End if rigid body
    else
    {
        // Release the previous physics body (if any).
        if ( mPhysicsBody )
        {
            mPhysicsBody->unregisterEventListener( static_cast<cgPhysicsBodyEventListener*>(this) );
            mPhysicsBody->removeReference( this );
        
        } // End if exists
        mPhysicsBody = CG_NULL;
    
    } // End if disable
}

//-----------------------------------------------------------------------------
//  Name : getSandboxIconInfo ( ) (Virtual)
/// <summary>
/// Retrieve information about the iconic representation of this object as it
/// is to be displayed in the sandbox rendering viewports.
/// </summary>
//-----------------------------------------------------------------------------
bool cgObjectNode::getSandboxIconInfo( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasName, cgString & frameName, cgVector3 & iconOrigin )
{
    // No representation by default
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setInstanceIdentifier () (Virtual)
/// <summary>
/// Set the string that allows the animation and other systems to identify
/// this animation target, and instances of it.
/// </summary>
//-----------------------------------------------------------------------------
void cgObjectNode::setInstanceIdentifier( const cgString & identifier )
{
    // Is this a no-op?
    if ( identifier == mInstanceIdentifier )
        return;

    // Update database reference only if we are not disposing
    // or are in sandbox mode.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mNodeUpdateInstanceIdentifier.bindParameter( 1, identifier );
        mNodeUpdateInstanceIdentifier.bindParameter( 2, mReferenceId );
        if ( !mNodeUpdateInstanceIdentifier.step( true ) )
        {
            cgString strError;
            mNodeUpdateInstanceIdentifier.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update instance identifier for object node '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if exists in DB

    // Call base class implementation last.
    cgString oldIdentifier = mInstanceIdentifier;
    cgAnimationTarget::setInstanceIdentifier( identifier );

    // Trigger 'onInstanceIdentifierChange' of all listeners (duplicate list in case
    // it is altered in response to event).
    if ( !mEventListeners.empty() )
    {
        EventListenerList listeners = mEventListeners;
        cgObjectNodeNameChangeEventArgs eventArgs( this, oldIdentifier );
        for ( EventListenerList::iterator itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
            ((cgObjectNodeEventListener*)(*itListener))->onInstanceIdentifierChange( &eventArgs );
    
    } // End if any listeners
}