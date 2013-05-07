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
// File: cgOctree.h                                                          //
//                                                                           //
// Desc: Specialized classes that provide the mechanism through which a      //
//       scene can partitioned using the octree spatial partitioning scheme. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGOCTREE_H_ )
#define _CGE_CGOCTREE_H_

//-----------------------------------------------------------------------------
// cgOctree Header Includes
//-----------------------------------------------------------------------------
#include <World/cgSpatialTree.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgOctreeSubNode;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgOctree (Class)
/// <summary>
/// Represents specialized octree spatial partitioning hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgOctree : public cgSpatialTree
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgOctree, cgSpatialTree, "Octree" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgOctree( );
             cgOctree( cgSpatialTree * init );
    virtual ~cgOctree( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                            buildTree               ( const cgBoundingBox & region );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTree)
    //-------------------------------------------------------------------------
    virtual cgSpatialTreeSubNode  * allocateNode            ( cgSpatialTreeSubNode * init = CG_NULL );
    virtual cgSpatialTreeLeaf     * allocateLeaf            ( cgSpatialTreeLeaf * init = CG_NULL );
    virtual void                    computeVisibility       ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                            buildTree               ( cgUInt32 level, cgOctreeSubNode * node, const cgBoundingBox & nodeBounds );
    void                            computeVisibility       ( cgOctreeSubNode * node, bool testChildren, const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgInt32     mMaxTreeDepth;

}; // End Class cgOctree

//---------------------------------------------------------------------------------
// Name : cgOctreeSubNode (Class)
/// <summary>
/// The node class specific to the octree spatial tree type.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgOctreeSubNode : public cgSpatialTreeSubNode
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
     cgOctreeSubNode( );
     cgOctreeSubNode( cgSpatialTreeSubNode * init, cgSpatialTree * parent );
    ~cgOctreeSubNode();

    //-----------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTreeSubNode)
    //---------------------------------------------------------------------------
    virtual void    nodeConstructed     ( );
    virtual bool    collectLeaves       ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgSceneLeafArray & leaves, const cgBoundingBox & bounds, bool autoCollect = false );
};

//---------------------------------------------------------------------------------
// Name : cgLandscapeCell (Class)
/// <summary>
/// The leaf class specific to the octree spatial tree type.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgOctreeLeaf : public cgSpatialTreeLeaf
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
    cgOctreeLeaf();
    cgOctreeLeaf( cgSpatialTreeLeaf * init, cgSpatialTree * parent );

    //-----------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTreeLeaf)
    //---------------------------------------------------------------------------
    
    //-----------------------------------------------------------------------------
    // Public Variables
    //-----------------------------------------------------------------------------
};

#endif // !_CGE_CGOCTREE_H_