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
// File: cgSphereTree.cpp                                                    //
//                                                                           //
// Desc: Highly optimized dynamic hierarchical sphere tree, ideal for        //
//       managing and querying scene dynamics.                               //
//       Portions Copyright (C) John W. Ratcliff, 2001                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSphereTree Module Includes
//-----------------------------------------------------------------------------
#include <World/cgSphereTree.h>
#include <World/cgBSPVisTree.h>
#include <Math/cgFrustum.h>

///////////////////////////////////////////////////////////////////////////////
// cgSphereTree Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgSphereTree() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgSphereTree::cgSphereTree( cgUInt32 maxSpheres, cgFloat leafSize, cgFloat padding, cgBSPTree * staticVisTree ) :
    mNodePool( maxSpheres * 2 ), mIntegrationFIFO( maxSpheres * 2 ), mRecomputeFIFO( maxSpheres * 2 )
{
    // Note: Pool and FIFO buffers allocated with storage for 'maxSpheres * 2' in order to
    // include room for both the tree and the superspheres.

    // Record tree details.
    mStaticVisTree      = staticVisTree;
    mMaximumLeafSize    = leafSize;
    mPadding            = padding;

    // Initialize the root entry of the node tree.
    mRoot = mNodePool.getFreeElement();
    mRoot->initialize( this, cgVector3(0,0,0), 65536, CG_NULL );
    mRoot->setFlag( NodeFlags( SuperSphere | RootNode ) );
}

//-----------------------------------------------------------------------------
// Name : ~cgSphereTree() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgSphereTree::~cgSphereTree()
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
void cgSphereTree::dispose( bool disposeBase )
{
    // Clear out containers.
    mNodePool.clear();
    mIntegrationFIFO.clear();
    mRecomputeFIFO.clear();

    // Clear variables.
    mRoot   = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : addSphere ()
/// <summary>
/// Insert a new element into the sphere tree representing the specified
/// bounding sphere.
/// </summary>
//-----------------------------------------------------------------------------
cgSphereTreeSubNode * cgSphereTree::addSphere( const cgBoundingSphere & bounds, void * userData, NodeFlags flags /* = Terminal */ )
{
    cgSphereTreeSubNode * newNode = mNodePool.getFreeElement();
    cgAssert( newNode );

    // Insert into the sphere tree
    if ( newNode )
    {
        newNode->initialize( this, bounds.position, bounds.radius, userData );
        newNode->setFlag( NodeFlags((flags & Terminal) | UpdateLeaves) );
        addIntegrate( newNode );

    } // End if valid

    // Return the node representing this sphere.
    return newNode;
}

//-----------------------------------------------------------------------------
//  Name : removeSphere ()
/// <summary>
/// Remove the specified element from the tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::removeSphere( cgSphereTreeSubNode * node )
{
    // Can never remove the root node.
    if ( node->isFlagSet( RootNode ) )
        return;

    // If this is a terminal node, remove its referenced object from 
    // all registered visibility sets.
    if ( node->isFlagSet( Terminal ) )
    {
        cgObjectNode * object = (cgObjectNode*)node->getUserData();
        if ( object )
        {
            for ( size_t i = 0; i < mSets.size(); ++i )
            {
                if ( node->isVisFlagSet( i, Partial ) || node->isVisFlagSet( i, Inside ) )
                    object->unregisterVisibility( mSets[i] );
            
            } // Next set
        
        } // End if valid
    
    } // End if terminal

    // Unlink the node from its lists.
    node->unlink();

    // Return the node back to the pool.
    node->getVisibilityFlags().clear();
    mNodePool.releaseElement( node );
}

//-----------------------------------------------------------------------------
//  Name : addIntegrate () (Protected)
/// <summary>
/// Add the specified node to the integration queue.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::addIntegrate( cgSphereTreeSubNode * node )
{
    // Attach at the root
    mRoot->addChild( node );

    // Add to the integration stack.
    node->setFlag( Integrate );
    node->setFIFO2(mIntegrationFIFO.push(node));
}

//-----------------------------------------------------------------------------
//  Name : addRecompute() (Protected)
/// <summary>
/// Add the specified node to the bounds recomputation queue.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::addRecompute( cgSphereTreeSubNode * node )
{
    if ( !node->isFlagSet(Recompute) )
    {
        if ( node->getChildCount() )
        {
            node->setFlag(Recompute);
            node->setFIFO1(mRecomputeFIFO.push(node));
        
        } // End if has children
        else
        {
            removeSphere(node);
        
        } // End if no children
    
    } // End if !no-op
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility()
/// <summary>
/// Populate the visibility set based on the objects that fall within the
/// supplied frustum.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::computeVisibility( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags )
{
    // Find the index of the set in the list.
    size_t setIndex;
    for ( setIndex = 0; setIndex < mSets.size(); ++setIndex )
    {
        if ( mSets[setIndex] == visibilityData )
            break;
    
    } // Next set

    // Traverse the hierarchy if it was a set we manage.
    if ( setIndex != mSets.size() )
    {
        static std::map<const cgFrustum*,cgVector3> lastFrustumPosition;
        lastFrustumPosition[&frustum] = frustum.position;

        // If a static PVS tree is available, find the source leaf in which the
        // frustum is currently positioned (for the purposes of visibility flow).
        cgUInt32 sourceLeaf = cgBSPTree::InvalidLeaf;
        if ( mStaticVisTree )
            sourceLeaf = mStaticVisTree->findLeaf( frustum.position );

        const cgByte * sourceLeafVis = CG_NULL;
        if ( sourceLeaf != cgBSPTree::InvalidLeaf && sourceLeaf != cgBSPTree::SolidLeaf )
            sourceLeafVis = &mStaticVisTree->getPVSData()[mStaticVisTree->getLeaves()[sourceLeaf].visibilityOffset];


        // Traverse!
        mRoot->computeVisibility( frustum, visibilityData, setIndex, flags, cgVolumeQuery::Intersect,
                                  mStaticVisTree, sourceLeaf, sourceLeafVis );
    } // End if we manage
}

//-----------------------------------------------------------------------------
//  Name : process()
/// <summary>
/// Process any updates that are pending for the sphere tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::process( )
{
    // When leaf node spheres exit their parent sphere, then the parent sphere needs to 
    // be rebalanced.  In fact, it may now be empty and need to be removed. This is the 
    // location where (n) number of spheres in the recomputation FIFO are allowed to be 
    // rebalanced in the tree.
    size_t maximumRecompute = mRecomputeFIFO.getEntryCount();
    for ( size_t i = 0; i < maximumRecompute; ++i )
    {
        cgSphereTreeSubNode * node = mRecomputeFIFO.pop();
        if ( !node )
            break;
        node->setFIFO1( CG_NULL );
        if ( node->recompute( mPadding ) )
            removeSphere( node );
    
    } // Next test

    // Now, process the integration step.
    size_t maximumIntegrate = mIntegrationFIFO.getEntryCount();
    for ( size_t i = 0; i < maximumIntegrate; ++i )
    {
        cgSphereTreeSubNode * node = mIntegrationFIFO.pop();
        if ( !node )
            break;
        node->setFIFO2( CG_NULL );
        integrate( node, mRoot, mMaximumLeafSize );
    
    } // Next test
}

//-----------------------------------------------------------------------------
//  Name : integrate() (Protected)
/// <summary>
/// Integrate the specified sphere into the tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::integrate( cgSphereTreeSubNode * node, cgSphereTreeSubNode * superSphere, cgFloat nodeSize )
{
    // First find which supersphere we are closest to (center point)
    cgSphereTreeSubNode * search = superSphere->getChildren();
    cgSphereTreeSubNode * nearest1 = 0; // Closest completely enclosing supersphere
    cgSphereTreeSubNode * nearest2 = 0; // Supersphere that grows the least when we add to it.
    cgFloat neardist1 = 1e9, neardist2 = 1e9;

    while ( search )
    {
        if ( search->isFlagSet(SuperSphere) && !search->isFlagSet(RootNode) && search->getChildCount() )
        {
            cgFloat distanceSq = cgVector3::lengthSq(node->position - search->position);
            if ( nearest1 )
            {
                if ( distanceSq < neardist1 )
                {
                    cgFloat d = sqrtf(distanceSq) + node->radius;
                    if ( d <= search->radius )
                    {
                        neardist1 = distanceSq;
                        nearest1  = search;
                    }
                }
            }
            else
            {
                cgFloat d = (sqrtf(distanceSq) + node->radius) - search->radius;
                if ( d < neardist2 )
                {
                    if ( d < 0 )
                    {
                        neardist1 = distanceSq;
                        nearest1  = search;
                    }
                    else
                    {
                        neardist2 = d;
                        nearest2  = search;
                    }
                }
            }
        
        } // End if SuperSphere with children

        search = search->getNextSibling();
    
    } // Next child

    // Integrate appropriately.
    if ( nearest1 )
    {
        // If we are inside an existing supersphere, we are all good.
        // We need to detach item from wherever it is, and then add it to
        // this supersphere as a child.
        node->unlink();
        nearest1->addChild(node);
        node->computeBindingDistance(nearest1);
        nearest1->recompute(mPadding);

    } // End if found enclosing
    else
    {
        bool createNewSphere = true;
        if ( nearest2 )
        {
            cgFloat newSize = neardist2 + nearest2->radius + mPadding;
            if ( newSize <= nodeSize )
            {
                node->unlink();
                nearest2->radius = newSize;
                nearest2->addChild(node);
                nearest2->recompute(mPadding);
                node->computeBindingDistance(nearest2);
                createNewSphere = false;
            }
        }

        // Creating a new supersphere around this?
        if ( createNewSphere )
        {
            cgAssert( superSphere->isFlagSet( RootNode ) );
            node->unlink();

            cgSphereTreeSubNode * parent = mNodePool.getFreeElement();
            cgAssert( parent );
            parent->initialize( this, node->position, node->radius + mPadding, CG_NULL );
            parent->setFlag( SuperSphere );
            parent->addChild(node);
            superSphere->addChild(parent);

            parent->recompute(mPadding);
            node->computeBindingDistance(parent);
        
        } // End if new super sphere
    
    } // End if no enclosing

    // Integration complete.
    node->clearFlag( Integrate );
}

//-----------------------------------------------------------------------------
//  Name : addVisibilitySet()
/// <summary>
/// Register the specified visibility set with the sphere tree. Frame coherent
/// visibility updates will be automatically handled during scene processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::addVisibilitySet( cgVisibilitySet * set )
{
    mSets.push_back( set );

    // Add a spare visibility flags entry to all active nodes.
    cgSphereTreeSubNode * node;
    for ( node = mNodePool.begin(); node; node = mNodePool.next() )
        node->getVisibilityFlags().push_back( 0 );
}

//-----------------------------------------------------------------------------
//  Name : removeVisibilitySet()
/// <summary>
/// Unregister the specified visibility set from the sphere tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTree::removeVisibilitySet( cgVisibilitySet * set )
{
    // Find the index of the set in the list.
    size_t setIndex;
    for ( setIndex = 0; setIndex < mSets.size(); ++setIndex )
    {
        if ( mSets[setIndex] == set )
            break;
    
    } // Next set

    // Found?
    if ( setIndex == mSets.size() )
        return;

    // Remove the visibility set from the main list.
    mSets.erase( mSets.begin() + setIndex );

    // Remove from all active nodes.
    cgSphereTreeSubNode * node;
    for ( node = mNodePool.begin(); node; node = mNodePool.next() )
        node->getVisibilityFlags().erase( node->getVisibilityFlags().begin() + setIndex );
}

///////////////////////////////////////////////////////////////////////////////
// cgSphereTreeSubNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : recompute()
/// <summary>
/// TODO. Returns true if the node should be removed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSphereTreeSubNode::recompute( cgFloat padding )
{
    // If there are no remaining children, return true
    // to indicate that it should be removed from the tree.
    if ( !mChildren )
        return true;

    // Root nodes should not be recomputed.
    if ( isFlagSet(cgSphereTree::RootNode) )
        return false;

    // Sum the center points of all children. This will be used to
    // compute an average center point later on.
    size_t count = 0;
    cgVector3 newCenter(0,0,0);
    cgSphereTreeSubNode * node = mChildren;
    while ( node )
    {
        newCenter += node->position;
        node = node->getNextSibling();
        ++count;
    
    } // Next node

    // If any children were found...
    if ( count )
    {
        // Update position of this node to match the average
        // of the positions of all of its children.
        cgVector3 oldPosition = this->position;
        this->position = newCenter / (cgFloat)count;
        
        // Find the largest radius
        node = mChildren;
        cgFloat maximumRadius = 0.0f;
        while ( node )
        {
            cgFloat distance = cgVector3::length( this->position - node->position );
            cgFloat enclosingRadius = distance + node->radius;
            if ( enclosingRadius > maximumRadius )
            {
                maximumRadius = enclosingRadius;
                if ( (maximumRadius+padding) >= this->radius )
                {
                    this->position = oldPosition;
                    clearFlag( cgSphereTree::Recompute );
                    return false;
                }
            }
            node = node->getNextSibling();
        
        } // Next child

        // Update our final enclosing radius
        maximumRadius += padding;
        this->radius = maximumRadius;

        // All children now have to recompute their binding distance.
        node = mChildren;
        while ( node )
        {
            node->computeBindingDistance(this);
            node = node->getNextSibling();
        
        } // Next child

    } // End if any children

    // Node has been recomputed
    clearFlag( cgSphereTree::Recompute );

    // Do not remove.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : invalidateVisibility()
/// <summary>
/// Invalidate the visibility data for this node. This should be used when
/// the visibility state for this sphere should be 'reconsidered' (i.e. it
/// was omitted from the set because it was manually set as invisible, but is
/// now ready for display, or vice-versa).
/// </summary>
//-----------------------------------------------------------------------------
void cgSphereTreeSubNode::invalidateVisibility( )
{
    if ( mVisFlags.empty() )
        return;

    // If this is a terminal node, remove from any visibility sets it
    // currently exists in.
    if ( isFlagSet( cgSphereTree::Terminal ) )
    {
        cgObjectNode * object = (cgObjectNode*)mUserData;
        if ( object )
        {
            const cgSphereTree::VisibilitySetArray & sets = mTree->getVisibilitySets();
            for ( size_t i = 0; i < sets.size(); ++i )
            {
                if ( isVisFlagSet( i, cgSphereTree::Partial ) || isVisFlagSet( i, cgSphereTree::Inside ) )
                    object->unregisterVisibility( sets[i] );
            
            } // Next set
        
        } // End if valid
    
    } // End if terminal

    // Clear all visibility flags entirely.
    memset( &mVisFlags[0], 0, mVisFlags.size() * sizeof(cgUInt32) );

    // Reflect up the hierarchy.
    if ( mParent )
        mParent->invalidateVisibility();
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility()
/// <summary>
/// Populate the visibility set based on the objects that fall within the
/// supplied frustum.
/// </summary>
//-----------------------------------------------------------------------------
#include <World/Objects/cgPointLight.h>
void cgSphereTreeSubNode::computeVisibility( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 setIndex, cgUInt32 flags, cgVolumeQuery::Class state, cgBSPTree * staticVisTree, cgUInt32 sourceLeaf, const cgByte * sourceLeafVis )
{
    // Further refine frustum based visibility state as necessary.
    if ( state == cgVolumeQuery::Intersect )
        state = frustum.classifySphere( this->position, this->radius );

    // If the sphere passes the frustum check, test against the PVS if one is available.
    if ( state != cgVolumeQuery::Outside && sourceLeafVis && isFlagSet( cgSphereTree::Terminal ) )
    {
        // Update the list of visibility leaves into which this sphere falls if necessary.
        if ( isFlagSet( cgSphereTree::UpdateLeaves ) )
        {
            clearFlag( cgSphereTree::UpdateLeaves );
            cgUInt32 leafVisRestrict = cgBSPTree::InvalidLeaf;
            //if ( isFlagSet( cgSphereTree::Terminal ) )
            {
                cgObjectNode * node = (cgObjectNode*)mUserData;
                if ( node->getObjectType() == RTID_PointLightObject )
                    leafVisRestrict = staticVisTree->findLeaf( node->getPosition(false) );
            }
            staticVisTree->findLeaves( mLeaves, mLeafCount, *this, leafVisRestrict );
        }

        // If this sphere exists in empty space, test to see if it is visible
        // from the perspective of the source.
        if ( mLeafCount > 0 )
        {
            cgUInt32 i;
            for ( i = 0; i < mLeafCount; ++i )
            {
                if ( cgBSPTree::getPVSBit( sourceLeafVis, mLeaves[i] ) )
                    break;
            
            } // Next leaf
            if ( i == mLeafCount )
                state = cgVolumeQuery::Outside;
        
        } // End if in leaves

    } // End if test PVS

    if ( isFlagSet( cgSphereTree::SuperSphere ) )
    {
        if ( state == cgVolumeQuery::Outside )
        {
            // Frame coherence (Note: Disabled due to introduction of PVS).
            //if ( isVisFlagSet( setIndex, cgSphereTree::Hidden ) )
                //return; // no state change
            if ( !isVisFlagSet( setIndex, cgSphereTree::Hidden ) )
            {
                clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Inside | cgSphereTree::Partial ) );
                setVisFlag( setIndex, cgSphereTree::Hidden );
            
            } // End if !outside
        
        } // End if outside
        else
        {
            if ( state == cgVolumeQuery::Inside )
            {
                // Frame coherence (Note: Disabled due to introduction of PVS).
                //if ( isVisFlagSet( setIndex, cgSphereTree::Inside ) )
                    //return; // no state change
                if ( !isVisFlagSet( setIndex, cgSphereTree::Inside ) )
                {
                    clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Partial | cgSphereTree::Hidden ) );
                    setVisFlag( setIndex, cgSphereTree::Inside );
                
                } // End if !inside

            } // End if inside
            else
            {
                clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Hidden | cgSphereTree::Inside ) );
                setVisFlag( setIndex, cgSphereTree::Partial );

            } // End if partial
        
        } // End if !outside

        // Process children
        cgSphereTreeSubNode * child = mChildren;
        while ( child )
        {
            child->computeVisibility( frustum, visibilityData, setIndex, flags, state, staticVisTree, sourceLeaf, sourceLeafVis );
            child = child->getNextSibling();
        
        } // Next child

    } // End if supersphere
    else
    {
        switch ( state )
        {
            case cgVolumeQuery::Inside:
                if ( !isVisFlagSet( setIndex, cgSphereTree::Inside ) )
                {
                    if ( isFlagSet( cgSphereTree::Terminal ) && isVisFlagSet( setIndex, cgSphereTree::Partial ) )
                    {
                        clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Hidden | cgSphereTree::Partial ) );
                        setVisFlag( setIndex, cgSphereTree::Inside );

                    } // End if partial -> inside
                    else
                    {
                        clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Hidden | cgSphereTree::Partial ) );
                        setVisFlag( setIndex, cgSphereTree::Inside );
                        
                        if ( isFlagSet( cgSphereTree::Terminal ) )
                        {
                            // Add to visibility set.
                            ((cgObjectNode*)mUserData)->registerVisibility( visibilityData );
                        
                        } // End if leaf

                    } // End if outside -> inside
                
                } // End if !inside
                break;

            case cgVolumeQuery::Outside:
                if ( !isVisFlagSet( setIndex, cgSphereTree::Hidden ) )
                {
                    clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Inside | cgSphereTree::Partial ) );
                    setVisFlag( setIndex, cgSphereTree::Hidden );
                    
                    if ( isFlagSet( cgSphereTree::Terminal ) )
                    {
                        // Remove from visibility set.
                        ((cgObjectNode*)mUserData)->unregisterVisibility( visibilityData );
                    
                    } // End if leaf
                
                } // End if !hidden
                break;

            case cgVolumeQuery::Intersect:
                if ( !isVisFlagSet( setIndex, cgSphereTree::Partial ) )
                {
                    if ( isFlagSet( cgSphereTree::Terminal ) && isVisFlagSet( setIndex, cgSphereTree::Inside ) )
                    {
                        clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Inside | cgSphereTree::Hidden ) );
                        setVisFlag( setIndex, cgSphereTree::Partial );
                    
                    } // End if inside -> partial
                    else
                    {
                        clearVisFlag( setIndex, cgSphereTree::NodeFlags( cgSphereTree::Inside | cgSphereTree::Hidden ) );
                        setVisFlag( setIndex, cgSphereTree::Partial );

                        if ( isFlagSet( cgSphereTree::Terminal ) )
                        {
                            // Add to visibility set.
                            ((cgObjectNode*)mUserData)->registerVisibility( visibilityData );
                        
                        } // End if leaf

                    } // End if outside -> partial
                
                } // End if !partial
                break;
        
        } // End switch state

    } // End if !supersphere
}