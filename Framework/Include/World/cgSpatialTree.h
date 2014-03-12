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
// Name : cgSpatialTree.h                                                    //
//                                                                           //
// Desc : Base interface designed to allow the application to interact with  //
//        any type or object that organizes data using a spatial tree        //
//        structure.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPATIALTREE_H_ )
#define _CGE_CGSPATIALTREE_H_

//-----------------------------------------------------------------------------
// cgSpatialTree Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgBoundingBox.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgFrustum;
class cgBoundingBox;
class cgVisibilitySet;
class cgCameraNode;
class cgSpatialTreeSubNode;
class cgSpatialTreeLeaf;
class cgSpatialTreeInstance;
class cgObjectNode;
class cgMatrix;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSpatialTree (Base Class)
/// <summary>
/// Base spatial tree object type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTree : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgSpatialTree, "SpatialTree" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTree( );
             cgSpatialTree( cgSpatialTree * init );
    virtual ~cgSpatialTree( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgSceneLeafArray        & getLeaves               ( ) const;
    cgSpatialTreeSubNode          * getRootNode             ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgSpatialTreeSubNode  * allocateNode            ( cgSpatialTreeSubNode * init = CG_NULL );
    virtual cgSpatialTreeLeaf     * allocateLeaf            ( cgSpatialTreeLeaf * init = CG_NULL );
    virtual void                    computeVisibility       ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer );
    virtual void                    computeVisibility       ( const cgBoundingBox & bounds, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer );
    virtual bool                    collectLeaves           ( cgSceneLeafArray & leaves, const cgBoundingBox & bounds, cgSpatialTreeInstance * issuer );
    virtual bool                    updateObjectOwnership   ( cgObjectNode * object, const cgBoundingBox & objectBounds, cgSpatialTreeInstance * issuer, bool testOnly );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSpatialTreeSubNode  * mRootNode;  // The root spatial tree node.
    cgSceneLeafArray        mLeaves;    // List of all leaves belonging to this spatial tree.
};

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeInstance (Class)
/// <summary>
/// Provides additional data specific to an individual /instance/ of a spatial
/// tree if necessary. An 'instance' describes an additional transformation as
/// well as being capable of storing the lists of objects contained within that
/// instance.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTreeInstance : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgSpatialTreeInstance, "SpatialTreeInstance" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTreeInstance( );
             cgSpatialTreeInstance( cgSpatialTree * tree );
             cgSpatialTreeInstance( cgSpatialTreeInstance * init );
    virtual ~cgSpatialTreeInstance( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        addObjectOwnership      ( cgUInt32 leafIndex, cgObjectNode * object );
    void                        removeObjectOwnership   ( cgUInt32 leafIndex, cgObjectNode * object );
    const cgObjectNodeSet     & getOwnedObjects         ( cgUInt32 leafIndex ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                computeVisibility       ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags );
    virtual void                computeVisibility       ( const cgBoundingBox & bounds, cgVisibilitySet * visibilityData, cgUInt32 flags );
    virtual bool                collectLeaves           ( cgSceneLeafArray & leaves, const cgBoundingBox & bounds );
    virtual bool                updateObjectOwnership   ( cgObjectNode * node, bool testOnly = false );
    virtual void                clearTreeData           ( );
    virtual void                getInstanceTransform    ( cgMatrix & transform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE( cgObjectNodeSet, LeafObjectSet )

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSpatialTree     * mTree;              // The specific spatial tree being referenced.
    LeafObjectSet       mObjectOwnership;   // Container that describes a list of objects owned by each leaf in this spatial tree instance.
};

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeLeaf (Base Class)
/// <summary>
/// The leaf class, which can be derived / implemented by each tree
/// type, provides access for the application to various pieces of leaf
/// information such as visibility, and face data.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTreeLeaf
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTreeLeaf( );
             cgSpatialTreeLeaf( cgSpatialTreeLeaf * init, cgSpatialTree * parent );
    virtual ~cgSpatialTreeLeaf();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void    setBoundingBox      ( const cgBoundingBox & leafBounds );
    void    setDataGroupId      ( cgInt32 dataGroupId );
    void    setLeafIndex        ( cgUInt32 leafIndex );
    
    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : getBoundingBox ()
    /// <summary>
    /// Retrieve the bounding area of this leaf in /object/ space (the space
    /// of the parent spatial tree).
    /// </summary>
    //-------------------------------------------------------------------------
    inline const cgBoundingBox & getBoundingBox( ) const
    {
        return mLeafBounds;
    }

    //-------------------------------------------------------------------------
    //  Name : getDataGroupId ()
    /// <summary>
    /// If any data from the internal geometry is associated with this leaf
    /// this method will return the data group identifier. Otherwise will
    /// return -1.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt32 getDataGroupId( ) const
    {
        return mDataGroupId;
    }

    //-------------------------------------------------------------------------
    //  Name : getLeafIndex ()
    /// <summary>
    /// Retrieve the index of this leaf as it exists in the parent object's
    /// leaf array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getLeafIndex( ) const
    {
        return mLeafIndex;
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32        mLeafIndex;     // Index of this leaf as it exists in the parent object's leaf array.
    cgBoundingBox   mLeafBounds;    // The spatial tree local axis aligned bounding box describing the leaf size.
    cgInt32         mDataGroupId;   // The data group 'context' identifier (mesh, landscape) that contains data associated with this leaf.
};

//-----------------------------------------------------------------------------
//  Name : cgSpatialTreeSubNode (Base Class)
/// <summary>
/// The node class, which should be derived / implemented by each tree type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSpatialTreeSubNode
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSpatialTreeSubNode();
             cgSpatialTreeSubNode( cgSpatialTreeSubNode * init, cgSpatialTree * parent );
    virtual ~cgSpatialTreeSubNode();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    setBoundingBox          ( const cgBoundingBox & nodeBounds );
    void                    setNodeDepth            ( cgUInt32 depth );
    bool                    setChildNode            ( cgUInt32 childIndex, cgSpatialTreeSubNode * childNode, bool deleteExisting = true );
    void                    setChildNodeCount       ( cgUInt32 childCount );
    void                    setLeaf                 ( cgUInt32 leafIndex );

    // Default implementations
    bool                    collectLeaves           ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgSceneLeafArray & leaves, const cgBoundingBox & bounds, bool autoCollect = false );
    void                    computeVisibility       ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgUInt8 frustumBits = 0x0, bool autoVisible = false );
    void                    computeVisibility       ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, const cgBoundingBox & testBounds, cgVisibilitySet * visibilityData, cgUInt32 flags, bool autoVisible = false );
    bool                    updateObjectOwnership   ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgObjectNode * object, const cgBoundingBox & objectBounds, bool testOnly, bool autoAdd, size_t & leafCount );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            nodeConstructed         ( ) {};

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : getChildNodeCount ()
    /// <summary>
    /// Retrieve the total number of child nodes stored here.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getChildNodeCount( ) const
    {
        return mChildNodeCount;
    }

    //-------------------------------------------------------------------------
    //  Name : getNodeDepth ()
    /// <summary>
    /// Retrieve the depth of this node as it exists within the spatial tree
    /// node hierarchy.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getNodeDepth( ) const
    {
        return mDepth;
    }

    //-------------------------------------------------------------------------
    //  Name : getLeaf ()
    /// <summary>
    /// Retrieve the index of any leaf stored as a child of this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getLeaf( ) const
    {
        return mLeaf;
    }

    //-------------------------------------------------------------------------
    //  Name : getBoundingBox ()
    /// <summary>
    /// Retrieve the bounding area of this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const cgBoundingBox & getBoundingBox( ) const
    {
        return mNodeBounds;
    }

    //-------------------------------------------------------------------------
    //  Name : getChildNode ()
    /// <summary>
    /// Retrieve the specified child node by index.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgSpatialTreeSubNode * getChildNode( cgUInt32 childIndex )
    {
        return mChildNodes[childIndex];
    }

    //-------------------------------------------------------------------------
    //  Name : isLeafNode ()
    /// <summary>
    /// Call this method to quickly determine if this node contains a leaf.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isLeafNode( ) const
    {
        return (mLeaf != 0xFFFFFFFF);
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBoundingBox           mNodeBounds;            // The spatial tree local axis aligned bounding box describing the node size.
    cgUInt32                mDepth;                 // The depth of the node within the spatial tree.
    cgSpatialTreeSubNode ** mChildNodes;            // An array containing all defined child nodes.
    cgUInt32                mChildNodeCount;        // Number of child nodes defined in the above array.
    cgUInt32                mLeaf;                  // If this is a leaf node, a leaf index will be stored here. Otherwise 0xFFFFFFFF.
    cgInt8                  mLastFrustumPlane;      // The frame-to-frame coherence 'last plane' index.
};

#endif // !_CGE_CGSPATIALTREE_H_