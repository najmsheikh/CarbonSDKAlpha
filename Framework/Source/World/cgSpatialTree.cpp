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
// Name : cgSpatialTree.cpp                                                  //
//                                                                           //
// Desc : Base interface designed to allow the application to interact with  //
//        any type or object that organizes data using a spatial tree        //
//        structure.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgSpatialTree Module Includes
//-----------------------------------------------------------------------------
#include <World/cgSpatialTree.h>
#include <World/cgVisibilitySet.h>
#include <World/cgObjectNode.h>
#include <Math/cgFrustum.h>

//-----------------------------------------------------------------------------
//  Name : cgSpatialTree() (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTree::cgSpatialTree( )
{
    // Initialize variables to sensible defaults
    mRootNode = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTree () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTree::cgSpatialTree( cgSpatialTree * pInit )
{
    // Initialize variables to sensible defaults initially.
    mRootNode = CG_NULL;

    // Clone nodes
    if ( pInit->mRootNode != CG_NULL )
        mRootNode = allocateNode( pInit->mRootNode );

    // Clone leaves
    mLeaves.resize( pInit->mLeaves.size() );
    for ( size_t i = 0; i < pInit->mLeaves.size(); ++i )
        mLeaves[i] = allocateLeaf( pInit->mLeaves[i] );
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTree () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTree::~cgSpatialTree()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSpatialTree::dispose( bool bDisposeBase )
{
    // Release the node hierarchy we're managing (if any).
    delete mRootNode;

    // Destroy any referenced leaves.
    for ( size_t i = 0; i < mLeaves.size(); ++i )
        delete mLeaves[i];
    
    // Clear variables
    mRootNode = CG_NULL;
    mLeaves.clear();
}

//-----------------------------------------------------------------------------
//  Name : allocateNode () (Virtual)
/// <summary>
/// Default behavior for allocating new nodes. Can be overriden to return
/// an alternate node type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode * cgSpatialTree::allocateNode( cgSpatialTreeSubNode * pInit /* = CG_NULL */ )
{
    if ( pInit == CG_NULL )
        return new cgSpatialTreeSubNode();
    else
        return new cgSpatialTreeSubNode( pInit, this );
}

//-----------------------------------------------------------------------------
//  Name : allocateLeaf () (Virtual)
/// <summary>
/// Default behavior for allocating new leaves. Can be overriden to
/// return an alternate leaf type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf * cgSpatialTree::allocateLeaf( cgSpatialTreeLeaf * pInit /* = CG_NULL */ )
{
    if ( pInit == CG_NULL )
        return new cgSpatialTreeLeaf();
    else
        return new cgSpatialTreeLeaf( pInit, this );
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility ()
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets based on the specified object space
/// frustum.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTree::computeVisibility( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer )
{
    /*// Recurse through the hierarchy.
    if ( mRootNode != CG_NULL )
        mRootNode->computeVisibility( issuer, this, frustum, visibilityData, flags );*/

    // Start the traversal.
    if ( mRootNode )
    {
        const cgBoundingBox & nodeBounds = mRootNode->getBoundingBox();
        cgVolumeQuery::Class classification = frustum.classifyAABB( nodeBounds );
        if ( classification != cgVolumeQuery::Outside )
            mRootNode->computeVisibility( issuer, this, frustum, visibilityData, flags );
    
    } // End if valid root
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility ()
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets based on the specified object space
/// bounding box.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTree::computeVisibility( const cgBoundingBox & Bounds, cgVisibilitySet * pVisData, cgUInt32 nFlags, cgSpatialTreeInstance * pIssuer )
{
    // ToDo: REMOVE THIS METHOD AND ITS DEPENDENCIES! Only used for lights and we need 
    // more optimal ways to retrieve light sets anyway.
    return;

    // Recurse through the hierarchy.
    if ( mRootNode != CG_NULL )
        mRootNode->computeVisibility( pIssuer, this, Bounds, pVisData, nFlags );
}

//-----------------------------------------------------------------------------
//  Name : updateObjectOwnership () (Virtual)
/// <summary>
/// Allows the spatial tree to determine if it can / wants to own the
/// specified object and optionally inserts it into the relevant leaf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTree::updateObjectOwnership( cgObjectNode * pObject, const cgBoundingBox & ObjectSpaceBounds, cgSpatialTreeInstance * pIssuer, bool bTestOnly )
{
    if ( !mRootNode )
        return false;

    // Recurse through the hierarchy.
    bool rootContained = false;
    if ( ObjectSpaceBounds.intersect( mRootNode->getBoundingBox(), rootContained ) )
    {
        size_t leafCount = 0;
        return mRootNode->updateObjectOwnership( pIssuer, this, pObject, ObjectSpaceBounds, bTestOnly, rootContained, leafCount );
    
    } // End if intersects

    // No ownership
    return false;
}

//-----------------------------------------------------------------------------
//  Name : collectLeaves ()
/// <summary>
/// This default implementation can called by the application to retrieve 
/// a list of all of the leaves that were intersected by the specified 
/// object space bounding box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTree::collectLeaves( cgSceneLeafArray & Leaves, const cgBoundingBox & Bounds, cgSpatialTreeInstance * pIssuer )
{
    if ( mRootNode == CG_NULL )
        return false;

    // Recurse through the hierarchy.
    return mRootNode->collectLeaves( pIssuer, this, Leaves, Bounds );
}

//-----------------------------------------------------------------------------
//  Name : getLeaves () (const overload)
/// <summary>
/// Retrieve a list of all defined spatial tree leaves.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneLeafArray & cgSpatialTree::getLeaves( ) const
{
    return mLeaves;
}

//-----------------------------------------------------------------------------
//  Name : getRootNode ()
/// <summary>
/// Retrieve the root spatial tree sub node.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode * cgSpatialTree::getRootNode( )
{
    return mRootNode;
}

///////////////////////////////////////////////////////////////////////////////
// cgSpatialTreeInstance Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeInstance () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeInstance::cgSpatialTreeInstance( )
{
    // Initialize variables to sensible defaults.
    mTree = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeInstance () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeInstance::cgSpatialTreeInstance( cgSpatialTree * tree )
{
    // Initialize variables to sensible defaults.
    mTree = tree;
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeInstance () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeInstance::cgSpatialTreeInstance( cgSpatialTreeInstance * pInit )
{
    // Initialize variables 
    mTree = pInit->mTree;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeInstance () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeInstance::~cgSpatialTreeInstance()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::dispose( bool bDisposeBase )
{
    // Destroy all allocated leaves.
    clearTreeData();

    // Clear variables
    mTree = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : clearTreeData () (Virtual)
/// <summary>
/// Destroy any tree data that has been allocated and stored within this
/// spatial tree instance. This allows for the tree data to be repopulated as 
/// necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::clearTreeData( )
{
    mObjectOwnership.clear();
}

//-----------------------------------------------------------------------------
//  Name : updateObjectOwnership () (Virtual)
/// <summary>
/// Allows the spatial tree to determine if it can / wants to own the
/// specified object and optionally inserts it into the relevant leaf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeInstance::updateObjectOwnership( cgObjectNode * pObject, bool bTestOnly /* = false */ )
{
    // ToDo: 9999 - Process child spatial trees first

    // Anything to do?
    if ( !mTree )
        return false;

    // Get the bounding box of the object in local space.
    cgMatrix mtxInv;
    getInstanceTransform( mtxInv );
    cgMatrix::inverse( mtxInv, mtxInv );
    cgBoundingBox ObjectSpaceBounds = cgBoundingBox::transform( pObject->getBoundingBox(), mtxInv );

    // Pass message on to our tree last.
    return mTree->updateObjectOwnership( pObject, ObjectSpaceBounds, this, bTestOnly );
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Protected, Virtual)
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::computeVisibility( const cgFrustum & Frustum, cgVisibilitySet * pVisData, cgUInt32 nFlags )
{
    // Anything to do?
    if ( !mTree )
        return;

    // Transform frustum into spatial tree 'object space' for testing.
    cgMatrix mtxInv;
    getInstanceTransform( mtxInv );
    cgMatrix::inverse( mtxInv, mtxInv );
    cgFrustum ObjectSpaceFrustum = cgFrustum::transform( Frustum, mtxInv );

    // Pass to referenced tree object.
    mTree->computeVisibility( ObjectSpaceFrustum, pVisData, nFlags, this );
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility () (Protected, Virtual)
/// <summary>
/// Called by the application to allow us to retrieve leaf visibility
/// information and rendering subsets.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::computeVisibility( const cgBoundingBox & Bounds, cgVisibilitySet * pVisData, cgUInt32 nFlags )
{
    // Anything to do?
    if ( !mTree )
        return;

    // Transform bounding box into spatial tree 'object space' for testing.
    cgMatrix mtxInv;
    getInstanceTransform( mtxInv );
    cgMatrix::inverse( mtxInv, mtxInv );
    cgBoundingBox ObjectSpaceBounds = cgBoundingBox::transform( Bounds, mtxInv );

    // Pass to referenced tree object.
    mTree->computeVisibility( ObjectSpaceBounds, pVisData, nFlags, this );
}

//-----------------------------------------------------------------------------
//  Name : collectLeaves ()
/// <summary>
/// This default implementation can called by the application to retrieve 
/// a list of all of the leaves that were intersected by the specified 
/// world space bounding box.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeInstance::collectLeaves( cgSceneLeafArray & Leaves, const cgBoundingBox & Bounds )
{
    // Anything to do?
    if ( !mTree )
        return false;

    // Transform bounding box into spatial tree 'object space' for testing.
    cgMatrix mtxInv;
    getInstanceTransform( mtxInv );
    cgMatrix::inverse( mtxInv, mtxInv );
    cgBoundingBox ObjectSpaceBounds = cgBoundingBox::transform( Bounds, mtxInv );

    // Pass to referenced tree object.
    return mTree->collectLeaves( Leaves, ObjectSpaceBounds, this );
}


//-----------------------------------------------------------------------------
//  Name : addObjectOwnership ()
/// <summary>
/// Mark the specified object as being owned by the specified leaf within this
/// instance of the spatial tree. Returns 'true' only if the object was
/// actually inserted (duplicates are rejected).
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeInstance::addObjectOwnership( cgUInt32 nLeafIndex, cgObjectNode * pObject )
{
    // Valid spatial tree?
    if ( !mTree )
        return false;

    // Ownership array is valid?
    size_t nNumLeaves = mTree->getLeaves().size();
    if ( mObjectOwnership.size() != nNumLeaves )
    {
        mObjectOwnership.clear();
        mObjectOwnership.resize( nNumLeaves );
    
    } // End if no ownership array

    // Out of bounds?
    if ( nLeafIndex >= nNumLeaves )
        return false;

    // Store in the list (will automatically overwrite if already exists).
    std::pair<cgObjectNodeSet::iterator,bool> ret = mObjectOwnership[nLeafIndex].insert( pObject );

    // Was it actually inserted?
    return ret.second;
}

//-----------------------------------------------------------------------------
//  Name : removeObjectOwnership ()
/// <summary>
/// Remove the ownership information for specified object from the specified 
/// leaf.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::removeObjectOwnership( cgUInt32 nLeafIndex, cgObjectNode * pObject )
{
    // Out of bounds?
    if ( nLeafIndex >= mObjectOwnership.size() )
        return;

    // Remove in the list.
    mObjectOwnership[nLeafIndex].erase( pObject );
}

//-----------------------------------------------------------------------------
//  Name : getOwnedObjects ()
/// <summary>
/// Retrieve the list of objects currently marked as owned by the specified 
/// leaf.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeSet & cgSpatialTreeInstance::getOwnedObjects( cgUInt32 nLeafIndex ) const
{
    static cgObjectNodeSet EmptySet;

    // Valid spatial tree?
    if ( !mTree )
        return EmptySet;

    // Out of bounds?
    if ( nLeafIndex >= mObjectOwnership.size() )
        return EmptySet;
    
    // Retrieve the set.
    return mObjectOwnership[nLeafIndex];
}

//-----------------------------------------------------------------------------
//  Name : getInstanceTransform ()
/// <summary>
/// Retrieve the transform that describes the position, orientation and scale
/// of this specific instance of the spatial tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeInstance::getInstanceTransform( cgMatrix & transform )
{
    transform = cgMatrix::Identity;
}

///////////////////////////////////////////////////////////////////////////////
// cgSpatialTreeSubNode Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeSubNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode::cgSpatialTreeSubNode()
{
    // Initialize variables to sensible defaults
    mDepth            = 0;
    mChildNodes      = CG_NULL;
    mChildNodeCount   = 0;
    mLeaf             = 0xFFFFFFFF;
    mLastFrustumPlane = -1;
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeSubNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode::cgSpatialTreeSubNode( cgSpatialTreeSubNode * pInit, cgSpatialTree * pParent )
{
    // Initialize variables to sensible defaults.
    mLastFrustumPlane = -1;
    mChildNodes      = CG_NULL;
    mChildNodeCount   = 0;
    
    // Clone Values
    mDepth            = pInit->mDepth;
    mNodeBounds        = pInit->mNodeBounds;
    mLeaf             = pInit->mLeaf;
    
    // Deep copy child nodes if applicable
    if ( pInit->mChildNodeCount > 0 && pInit->mChildNodes != CG_NULL )
    {
        setChildNodeCount( pInit->mChildNodeCount );
        for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
        {
            if ( pInit->mChildNodes[i] != CG_NULL )
                mChildNodes[i] = pParent->allocateNode( pInit->mChildNodes[i] );
            else
                mChildNodes[i] = CG_NULL;

        } // Next Child Node

    } // End if allocated.
}


//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeSubNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode::~cgSpatialTreeSubNode()
{
    // Release all child nodes stored here
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        if ( mChildNodes[i] != CG_NULL )
            delete mChildNodes[i];
    
    } // Next Child Node
    delete []mChildNodes;

    // Clear variables
    mLeaf             = 0xFFFFFFFF;
    mDepth            = 0;
    mChildNodes      = CG_NULL;
    mChildNodeCount   = 0;
}

//-----------------------------------------------------------------------------
//  Name : setBoundingBox ()
/// <summary>
/// Set the bounding area of this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::setBoundingBox( const cgBoundingBox & NodeBounds )
{
    mNodeBounds = NodeBounds;
}

//-----------------------------------------------------------------------------
//  Name : setNodeDepth ()
/// <summary>
/// Set the depth value of this node (its position in the tree hierarchy)
/// to the value specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::setNodeDepth( cgUInt32 nDepth )
{
    mDepth = nDepth;
}

//-----------------------------------------------------------------------------
//  Name : setChildNode ()
/// <summary>
/// Store the specified child node in the relevant position of this 
/// child node list. Optionally delete any existing node if one has 
/// already previously been stored.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeSubNode::setChildNode( cgUInt32 nChildIndex, cgSpatialTreeSubNode * pChildNode, bool bDeleteExisting /* = true */ )
{
    // Validate parameters
    if ( nChildIndex >= mChildNodeCount ) 
        return false;

    // Delete the node already stored at this index if any
    if ( bDeleteExisting == true && mChildNodes[ nChildIndex ] != CG_NULL )
        delete mChildNodes[ nChildIndex ];

    // Store the child node
    mChildNodes[ nChildIndex ] = pChildNode;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setChildNodeCount ()
/// <summary>
/// Should be called by the derived class in order to allocate space for
/// child nodes. i.e. derived KD tree node type would specify '2' nodes
/// in its constructor.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::setChildNodeCount( cgUInt32 nChildCount )
{
    cgSpatialTreeSubNode ** ppNewNodes = CG_NULL;

    // Is this a no-op?
    if ( nChildCount == mChildNodeCount )
        return;

    if ( mChildNodes != CG_NULL )
    {
        // Delete any nodes that already exist if the node count is shrinking
        for ( cgUInt32 i = nChildCount; i < mChildNodeCount; ++i )
        {
            if ( mChildNodes[i] != CG_NULL )
                delete mChildNodes[i];

        } // Next Child

    } // End if child nodes already exist

    if ( nChildCount > 0 )
    {
        // Allocate a new array if requested
        ppNewNodes = new cgSpatialTreeSubNode*[nChildCount];
        memset( ppNewNodes, 0, nChildCount * sizeof(cgSpatialTreeSubNode*) );

        // Copy over old data if available
        if ( mChildNodes != CG_NULL )
        {
            memcpy( ppNewNodes, mChildNodes, min( mChildNodeCount, nChildCount ) * sizeof(cgSpatialTreeSubNode*) );
            delete []mChildNodes;
            mChildNodes = CG_NULL;

        } // End if old data available

    } // End if new buffer required

    // Store updated values
    mChildNodes    = ppNewNodes;
    mChildNodeCount = nChildCount;
}

//-----------------------------------------------------------------------------
//  Name : setLeaf ()
/// <summary>
/// Store the index to the required leaf for this node.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::setLeaf( cgUInt32 nLeafIndex )
{
    // Store the new leaf
    mLeaf = nLeafIndex;
}

//-----------------------------------------------------------------------------
//  Name : updateObjectOwnership () (Recursive)
/// <summary>
/// The default recursive function which traverses the tree nodes in
/// order to update the issuer's object ownership list.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeSubNode::updateObjectOwnership( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgObjectNode * object, const cgBoundingBox & objectBounds, bool testOnly, bool autoAdd, size_t & leafCount )
{
    // Is there a leaf here, add it to the list
    if ( isLeafNode() )
    {
        ++leafCount;
        if ( !testOnly )
        {
            // ToDo:
        
        } // End if !testOnly
        return true;
    
    } // End if leaf node

    // Test all children for intersection
    bool result = false;
    for ( size_t i = 0; i < mChildNodeCount; ++i )
    {
        cgSpatialTreeSubNode * childNode = mChildNodes[i];
        if ( childNode )
        {
            if ( !autoAdd )
            {
                // Does the specified box intersect this node?
                bool childContained = false;
                if ( objectBounds.intersect( childNode->getBoundingBox(), childContained ) )
                    result |= childNode->updateObjectOwnership( issuer, tree, object, objectBounds, testOnly, childContained, leafCount );

            } // End if test
            else
            {
                result |= childNode->updateObjectOwnership( issuer, tree, object, objectBounds, testOnly, true, leafCount );

            } // End if !test

        } // End if valid child

    } // Next child

    // No ownership
    return result;
}

//-----------------------------------------------------------------------------
//  Name : collectLeaves () (Recursive)
/// <summary>
/// The default recursive function which traverses the tree nodes in
/// order to build a list of intersecting leaves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSpatialTreeSubNode::collectLeaves( cgSpatialTreeInstance * pIssuer, cgSpatialTree * pTree, cgSceneLeafArray & Leaves, const cgBoundingBox & Bounds, bool bAutoCollect /* = false */ )
{
    bool bResult = false;

    // Does the specified box intersect this node?
    if ( bAutoCollect == false && Bounds.intersect( mNodeBounds, bAutoCollect ) == false )
        return false;
    
    // Is there a leaf here, add it to the list
    if ( isLeafNode() == true )
    {
        Leaves.push_back( pTree->getLeaves()[mLeaf] );
        return true;
    
    } // End if leaf node

    // Test all children for intersection
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        // Recurse into the child node.
        if ( mChildNodes[i] != CG_NULL )
        {
            if ( mChildNodes[i]->collectLeaves( pIssuer, pTree, Leaves, Bounds, bAutoCollect ) == true )
                bResult = true;

        } // End if child node available

    } // Next child node
    
    // Return the 'was anything added' result.
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility() (Virtual, Recursive)
/// <summary>
/// The DEFAULT recursive function (for frustum testing) which traverses 
/// the node hierarchy and updates the visibility set as appropriate.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::computeVisibility( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgUInt8 frustumBits /* = 0x0 */, bool autoVisible /* = false */ )
{
    // Process the leaf if there is one stored here
    if ( isLeafNode() )
    {
        // Are there any objects stored in this leaf?
        if ( issuer )
        {
            const cgObjectNodeSet & leafObjects = issuer->getOwnedObjects( getLeaf() );
            if ( !leafObjects.empty() )
            {
                // Iterate through each object in the leaf and request that they register
                // their visibility (assuming they pass the final frustum test).
                cgObjectNodeSet::const_iterator itObject;
                for ( itObject = leafObjects.begin(); itObject != leafObjects.end(); ++itObject )
                {
                    cgObjectNode * objectNode = *itObject;
                    if ( frustum.testAABB( objectNode->getBoundingBox() ) )
                        objectNode->registerVisibility( visibilityData );

                } // Next object

            } // End if any objects to cull

        } // End if valid issuer

        // No further recursion required
        return;

    } // End if process leaf

    // Recurse into visible children.
    for ( cgUInt i = 0; i < mChildNodeCount; ++i )
    {
        cgSpatialTreeSubNode * childNode = mChildNodes[i];
        if ( childNode )
        {
            // Classify the node bounding box against the frustum if required
            if ( !autoVisible )
            {
                const cgBoundingBox & nodeBounds = childNode->getBoundingBox();
                cgVolumeQuery::Class classification = frustum.classifyAABB( nodeBounds, frustumBits, mLastFrustumPlane );
                if ( classification != cgVolumeQuery::Outside )
                    childNode->computeVisibility( issuer, tree, frustum, visibilityData, flags, frustumBits, (classification == cgVolumeQuery::Inside) );
            
            } // End if test
            else
            {
                childNode->computeVisibility( issuer, tree, frustum, visibilityData, flags, frustumBits, true );

            } // End if !test
        
        } // End if valid

    } // Next child

    /*bool bNodeVisible = false;

    // Perform full test?
    if ( bAutoVisible == false )
    {
        cgVolumeQuery::Class Result = Frustum.classifyAABB( getBoundingBox(), FrustumBits, mLastFrustumPlane );

        // Test result of frustum collide
        switch ( Result )
        {
            case cgVolumeQuery::Outside:
                // Node is not visible
                return;

            case cgVolumeQuery::Inside:
                // Node is fully visible
                bAutoVisible = true;
                
                // Pass through to next case

            case cgVolumeQuery::Intersect:
                // It is partially visible, but we need to test further.
                break;

        } // End Switch

    } // End if auto

    // If this is a leaf node, mark it as visible.
    if ( isLeafNode() == true )
    {
        cgSpatialTreeLeaf * pLeaf = pTree->getLeaves()[mLeaf];
        pVisData->addVisibleLeaf( pIssuer, pLeaf );

        // Also add any visible objects contained in the leaf.
        if ( pIssuer )
        {
            const cgObjectNodeSet & LeafObjects = pIssuer->getOwnedObjects( mLeaf );
            //for ( cgObjectNodeSet::const_iterator itObject = LeafObjects.begin(); itObject != LeafObjects.end(); ++itObject )
                //(*itObject)->registerVisibility( pVisData, nFlags );
        
        } // End if issuer
        return;
    
    } // End if leaf node

    // The remaining cases (intersecting or contained) means we need to recurse further.
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        // Recurse into the child node.
        if ( mChildNodes[i] != CG_NULL )
            mChildNodes[i]->computeVisibility( pIssuer, pTree,Frustum, pVisData, nFlags, FrustumBits, bAutoVisible );
        
    } // Next child node*/
}

//-----------------------------------------------------------------------------
//  Name : computeVisibility() (Virtual, Recursive)
/// <summary>
/// The DEFAULT recursive function (for AABB testing) which traverses 
/// the node hierarchy and updates the visibility set as appropriate.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeSubNode::computeVisibility( cgSpatialTreeInstance * pIssuer, cgSpatialTree * pTree, const cgBoundingBox & TestBounds, cgVisibilitySet * pVisData, cgUInt32 nFlags, bool bAutoVisible /* = false */ )
{
	bool bContained = false;

    // Check for intersection if required
    if ( bAutoVisible == false && TestBounds.intersect( getBoundingBox(), bAutoVisible ) == false )
        return;

    // If this is a leaf node, mark it as visible.
    if ( isLeafNode() == true )
    {
        cgSpatialTreeLeaf * pLeaf = pTree->getLeaves()[mLeaf];
        pVisData->addVisibleLeaf( pIssuer, pLeaf );

        // Also add any visible objects contained in the leaf.
        if ( pIssuer )
        {
            const cgObjectNodeSet & LeafObjects = pIssuer->getOwnedObjects( mLeaf );
            for ( cgObjectNodeSet::const_iterator itObject = LeafObjects.begin(); itObject != LeafObjects.end(); ++itObject )
                (*itObject)->registerVisibility( pVisData );
        
        } // End if issuer
        return;
    
    } // End if leaf node

    // The remaining cases (Intersecting or contained) means we need to recurse further.
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        // Recurse into the child node.
        if ( mChildNodes[i] != CG_NULL )
            mChildNodes[i]->computeVisibility( pIssuer, pTree, TestBounds, pVisData, nFlags, bAutoVisible );
        
    } // Next child node
}

///////////////////////////////////////////////////////////////////////////////
// cgSpatialTreeLeaf Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeLeaf () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf::cgSpatialTreeLeaf()
{
    // Initialize variables to sensible defaults
    mDataGroupId = -1;
    mLeafIndex   = 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeLeaf () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf::cgSpatialTreeLeaf( cgSpatialTreeLeaf * pInit, cgSpatialTree * pParent )
{
    // Clone values.
    mLeafIndex    = pInit->mLeafIndex;
    mDataGroupId  = pInit->mDataGroupId;
    mLeafBounds    = pInit->mLeafBounds;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeLeaf () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf::~cgSpatialTreeLeaf()
{
}

//-----------------------------------------------------------------------------
//  Name : setBoundingBox ()
/// <summary>
/// Set the object space (that of the parent tree) bounding area of this leaf.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeLeaf::setBoundingBox( const cgBoundingBox & LeafBounds )
{
    mLeafBounds = LeafBounds;
}

//-----------------------------------------------------------------------------
//  Name : setDataGroupId ()
/// <summary>
/// Set the data group id, used to link the leaf to subsets found within
/// the tree geometry mesh, for this leaf.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeLeaf::setDataGroupId( cgInt32 nDataGroupId )
{
    mDataGroupId = nDataGroupId;
}

//-----------------------------------------------------------------------------
//  Name : setDataGroupId ()
/// <summary>
/// Set the index of this leaf as it exists in the parent object's leaf array.
/// </summary>
//-----------------------------------------------------------------------------
void cgSpatialTreeLeaf::setLeafIndex( cgUInt32 nLeafIndex )
{
    mLeafIndex = nLeafIndex;
}