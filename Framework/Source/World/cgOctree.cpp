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
// File: cgOctree.cpp                                                        //
//                                                                           //
// Desc: Specialized classes that provide the mechanism through which a      //
//       scene can partitioned using the octree spatial partitioning scheme. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgOctree Module Includes
//-----------------------------------------------------------------------------
#include <World/cgOctree.h>
#include <Math/cgCollision.h>
#include <Math/cgFrustum.h>

///////////////////////////////////////////////////////////////////////////////
// cgOctree Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgOctree() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgOctree::cgOctree( ) : cgSpatialTree( )
{
    // Initialize variables to sensible defaults
    mMaxTreeDepth = 3;
}

//-----------------------------------------------------------------------------
// Name : cgOctree() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgOctree::cgOctree( cgSpatialTree * init ) : cgSpatialTree( init )
{
    // Initialize variables to sensible defaults
    cgOctree * octree = (cgOctree*)init;
    mMaxTreeDepth = octree->mMaxTreeDepth;
}

//-----------------------------------------------------------------------------
// Name : ~cgOctree() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgOctree::~cgOctree()
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
void cgOctree::dispose( bool disposeBase )
{
    // Dispose of base resources.
    if ( disposeBase )
        cgSpatialTree::dispose( true );
}

//-----------------------------------------------------------------------------
// Name : buildTree ( )
/// <summary>
/// Compile the spatial tree based on the specified configuration and data
/// added to this tree object (geometry, construction data etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgOctree::buildTree( const cgBoundingBox & region )
{
    // Verify that the bounding box has any size at all
    if ( region.isDegenerate() == true )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Unable to compile octree because the root bounding box was found to be degenerate.\r\n") );
        return false;
    
    } // End if degenerate root node
    
    // Allocate the root node
    mRootNode = allocateNode();

    // Build the spatial tree.
    if ( !buildTree( 0, (cgOctreeSubNode*)mRootNode, region ) )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildTree () (Protected, Recursive)
/// <summary>
/// Performs the recursive build behavior for this tree type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgOctree::buildTree( cgUInt32 level, cgOctreeSubNode * node, const cgBoundingBox & nodeBounds )
{
    // Store the initial position / size properties of the node.
    node->setBoundingBox( nodeBounds );
    node->setNodeDepth( level );

    // Determine if we should stop building here
    bool stopCode = false;
    if ( mMaxTreeDepth >= 0 )
        stopCode |= (level == mMaxTreeDepth);

    // If we reached our stop limit, build a leaf here
    if ( stopCode )
    {
        // Construct a leaf.
        cgSpatialTreeLeaf * leaf = allocateLeaf();
        leaf->setLeafIndex( (cgUInt32)mLeaves.size() );
        leaf->setBoundingBox( nodeBounds );
        
        // Insert into the leaf list
        mLeaves.push_back( leaf );

        // Set the new leaf index to the node.
        node->setLeaf( leaf->getLeafIndex() );

        // Notify the node that it has been fully constructed
        node->nodeConstructed();
        
        // We have reached a leaf, so we can stop compiling this tree branch
        return true;

    } // End if reached stop code

    // Build each of the 8 children here
    cgVector3 midPoint = nodeBounds.getCenter();
    for ( cgInt i = 0; i < 8; ++i )
    {
        cgBoundingBox childBounds = nodeBounds;

        // Calculate quater size child bounding box values
        switch( i )
        {
            case 0: // Top Behind left
                childBounds.max.x = midPoint.x;
                childBounds.max.z = midPoint.z;
                childBounds.min.y = midPoint.y;
                break;

            case 1: // Top InFront left
                childBounds.min.z = midPoint.z;
                childBounds.max.x = midPoint.x;
                childBounds.min.y = midPoint.y;
                break;

            case 2: // Top Behind Right
                childBounds.min.x = midPoint.x;
                childBounds.max.z = midPoint.z;
                childBounds.min.y = midPoint.y;
                break;

            case 3: // Top InFront Right
                childBounds.min.x = midPoint.x;
                childBounds.min.z = midPoint.z;
                childBounds.min.y = midPoint.y;
                break;

            case 4: // Bottom Behind left
                childBounds.max.x = midPoint.x;
                childBounds.max.z = midPoint.z;
                childBounds.max.y = midPoint.y;
                break;

            case 5: // Bottom InFront left
                childBounds.min.z = midPoint.z;
                childBounds.max.x = midPoint.x;
                childBounds.max.y = midPoint.y;
                break;

            case 6: // Bottom Behind Right
                childBounds.min.x = midPoint.x;
                childBounds.max.z = midPoint.z;
                childBounds.max.y = midPoint.y;
                break;

            case 7: // Bottom InFront Right
                childBounds.min.x = midPoint.x;
                childBounds.min.z = midPoint.z;
                childBounds.max.y = midPoint.y;
                break;

        } // End switch child type

        // Allocate child node
        node->setChildNode(i, allocateNode() );
        
        // Recurse into this new node
        if ( !buildTree( level + 1, (cgOctreeSubNode*)node->getChildNode(i), childBounds ) )
            return false;

    } // Next Child

    // Notify the node that it has been fully constructed
    node->nodeConstructed();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : allocateNode () (Virtual)
/// <summary>
/// Default behavior for allocating new nodes. Can be overriden to return an 
/// alternate node type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode * cgOctree::allocateNode( cgSpatialTreeSubNode * init /* = CG_NULL */ )
{
    if ( !init )
        return new cgOctreeSubNode( );
    else
        return new cgOctreeSubNode( init, this );
}

//-----------------------------------------------------------------------------
// Name : allocateLeaf () (Virtual)
/// <summary>
/// Default behavior for allocating new leaves. Can be overriden to return an 
/// alternate leaf type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf * cgOctree::allocateLeaf( cgSpatialTreeLeaf * init /* = CG_NULL */ )
{
    if ( !init )
        return new cgOctreeLeaf( );
    else
        return new cgOctreeLeaf( init, this );
}

//-----------------------------------------------------------------------------
// Name : computeVisibility () (Virtual)
/// <summary>
/// Called in order to request that leaves and owned objects are registered 
/// with the specified visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgOctree::computeVisibility( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer )
{
    // Use default implementation.
    cgSpatialTree::computeVisibility( frustum, visibilityData, flags, issuer );

    /*// Start the traversal.
    if ( mRootNode )
    {
        const cgBoundingBox & nodeBounds = mRootNode->getBoundingBox();
        cgVolumeQuery::Class classification = frustum.classifyAABB( nodeBounds );
        if ( classification != cgVolumeQuery::Outside )
            computeVisibility( (cgOctreeSubNode*)mRootNode, (classification == cgVolumeQuery::Intersect), frustum, visibilityData, flags, issuer );
    
    } // End if valid root*/
}

//-----------------------------------------------------------------------------
// Name : computeVisibility () (Protected)
/// <summary>
/// The actual recursive function which traverses the tree and updates the 
/// visible status of the tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgOctree::computeVisibility( cgOctreeSubNode * node, bool testChildren, const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer )
{
    // Process the leaf if there is one stored here
    if ( node->isLeafNode() )
    {
        // Are there any objects stored in this leaf?
        if ( issuer )
        {
            const cgObjectNodeSet & leafObjects = issuer->getOwnedObjects( node->getLeaf() );
            if ( !leafObjects.empty() )
            {
                // Iterate through each object in the leaf and request that they register
                // their visibility (assuming they pass the final frustum test).
                cgObjectNodeSet::const_iterator itObject;
                for ( itObject = leafObjects.begin(); itObject != leafObjects.end(); ++itObject )
                {
                    cgObjectNode * objectNode = *itObject;
                    /*if ( frustum.testAABB( node->getBoundingBox() ) )
                        objectNode->registerVisibility( visibilityData, flags );*/

                } // Next object

            } // End if any objects to cull

        } // End if valid issuer

        // No further recursion required
        return;

    } // End if process leaf

    // Recurse into visible children.
    for ( cgInt i = 0; i < 8; ++i )
    {
        cgOctreeSubNode * childNode = (cgOctreeSubNode*)node->getChildNode(i);
        if ( childNode )
        {
            // Classify the node bounding box against the frustum if required
            if ( testChildren )
            {
                const cgBoundingBox & nodeBounds = childNode->getBoundingBox();
                cgVolumeQuery::Class classification = frustum.classifyAABB( nodeBounds );
                if ( classification != cgVolumeQuery::Outside )
                    computeVisibility( childNode, (classification == cgVolumeQuery::Intersect), frustum, visibilityData, flags, issuer );
            
            } // End if test
            else
            {
                computeVisibility( childNode, false, frustum, visibilityData, flags, issuer );

            } // End if !test
        
        } // End if valid

    } // Next child
}

///////////////////////////////////////////////////////////////////////////////
// cgOctreeSubNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgOctreeSubNode () (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgOctreeSubNode::cgOctreeSubNode( ) : cgSpatialTreeSubNode()
{
    // Initialize variables to sensible defaults.
    setChildNodeCount( 8 );
}

//-----------------------------------------------------------------------------
// Name : cgOctreeSubNode () (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgOctreeSubNode::cgOctreeSubNode( cgSpatialTreeSubNode * init, cgSpatialTree * parent ) : cgSpatialTreeSubNode( init, parent )
{
    // Initialize variables to sensible defaults
    cgOctreeSubNode * node = (cgOctreeSubNode*)init;
}

//-----------------------------------------------------------------------------
// Name : ~cgOctreeSubNode () (Destructor)
/// <summary>Class destructor.</summary>
//-----------------------------------------------------------------------------
cgOctreeSubNode::~cgOctreeSubNode( )
{    
    // Destroy resources
}

//-----------------------------------------------------------------------------
// Name : collectLeaves () (Virtual, Recursive)
/// <summary>
/// The default recursive function which traverses the tree nodes in order to 
/// build a list of intersecting leaves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgOctreeSubNode::collectLeaves( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgSceneLeafArray & leaves, const cgBoundingBox & bounds, bool autoCollect /* = false */ )
{
    // Does the specified box intersect this node (IGNORE Y AXIS!)?
    if ( !autoCollect && !cgCollision::AABBIntersectAABB( autoCollect, bounds, mNodeBounds, false, true, false ) )
        return false;
    
    // Is there a leaf here, add it to the list
    if ( isLeafNode() )
    {
        // ToDo: Previously used push_front... Any specific reason?
        leaves.push_back( tree->getLeaves()[mLeaf] );
        return true;
    
    } // End if leaf node

    // Test all children for intersection
    bool result = false;
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        // Recurse into the child node.
        if ( mChildNodes[i] != CG_NULL )
            result |= mChildNodes[i]->collectLeaves( issuer, tree, leaves, bounds, autoCollect );

    } // Next child node
    
    // Return the 'was anything added' result.
    return result;
}

//-----------------------------------------------------------------------------
// Name : nodeConstructed () (Virtual)
/// <summary>
/// This function will be called whenever a node has been fully constructed by 
/// whichever build function is executing. This allows any derived class to 
/// perform any post build processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgOctreeSubNode::nodeConstructed( )
{
}

///////////////////////////////////////////////////////////////////////////////
// cgOctreeCell Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgOctreeLeaf () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgOctreeLeaf::cgOctreeLeaf()
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
// Name : cgOctreeLeaf () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgOctreeLeaf::cgOctreeLeaf( cgSpatialTreeLeaf * init, cgSpatialTree * parent ) : cgSpatialTreeLeaf( init, parent )
{
    // Initialize variables to sensible defaults
    cgOctreeLeaf * leaf = (cgOctreeLeaf*)init;

    // Nothing in current implementation
}