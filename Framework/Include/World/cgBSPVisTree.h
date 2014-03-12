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
// File: cgBSPVisTree.h                                                      //
//                                                                           //
// Desc: BSP tree / PVS combination used to provide static visibility and    //
//       occlusion data for the scene based on any static scene objects.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBSPVISTREE_H_ )
#define _CGE_CGBSPVISTREE_H_

//-----------------------------------------------------------------------------
// cgBSPVisTree Header Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptInterop.h>
#include <Math/cgBoundingBox.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBSPTreeLeaf;
class cgBSPTreeSubNode;
class cgBoundingSphere;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgBSPTree (Class)
/// <summary>
/// Represents specialized BSP tree spatial partitioning hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBSPTree : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgBSPTree, "BSPTree" )

public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 SolidLeaf     = 0xFFFFFFFF;
    static const cgUInt32 InvalidLeaf   = 0xFFFFFFFE;
    
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    struct Winding
    {
        // Constructor
        Winding() : 
            firstVertex(0), vertexCount(0), plane( cgUInt32(-1) ),
            used(false), next( CG_NULL ) {}

        // Members
        cgUInt32    firstVertex;
        cgUInt32    vertexCount;
        cgUInt32    plane;
        bool        used;
        Winding   * next;
    };
    struct Portal : public Winding
    {
        // Constructor
        Portal()
        {
            leafOwner[0] = cgUInt32(-1);
            leafOwner[1] = cgUInt32(-1);
        }

        // Members
        cgUInt32    node;
        cgUInt32    leafOwner[2];
    };
    CGE_ARRAY_DECLARE( cgPlane, PlaneArray );
    CGE_ARRAY_DECLARE( cgBSPTreeLeaf, LeafArray );
    CGE_ARRAY_DECLARE( cgBSPTreeSubNode, NodeArray );
    CGE_ARRAY_DECLARE( cgVector3, PointArray );
    CGE_ARRAY_DECLARE( Portal, PortalArray );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBSPTree( );
    virtual ~cgBSPTree( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                buildTree               ( cgUInt32 meshCount, const cgMeshHandle meshes[], const cgTransform transforms[] );
    bool                compilePVS              ( );
    cgUInt32            findLeaf                ( const cgVector3 & point );
    void                findLeaves              ( cgUInt32 * leaves, cgUInt32 & leafCount, const cgBoundingSphere & sphere, cgUInt32 leafSourceVisRestrict = InvalidLeaf );
    bool                isVolumeVisible         ( cgUInt32 sourceLeaf, const cgBoundingSphere & sphere );
    const cgByteArray & getPVSData              ( ) const;
    const LeafArray   & getLeaves               ( ) const;
    const NodeArray   & getNodes                ( ) const;
    const PlaneArray  & getNodePlanes           ( ) const;
    const PortalArray & getPortals              ( ) const;
    const PointArray  & getPortalVertices       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Static Inline Functions
    //-------------------------------------------------------------------------
    static inline bool getPVSBit( const cgByte visArray[], cgUInt32 leaf )
    {
        return (visArray[ leaf >> 3 ] & (1 << (leaf & 7))) != 0;
    }
    static inline void setPVSBit( cgByte visArray[], cgUInt32 leaf, bool value = true )
    {
        if ( value )
            visArray[ leaf >> 3 ] |=  (1 << ( leaf & 7 ));
        else
            visArray[ leaf >> 3 ] &= ~(1 << ( leaf & 7 ));
    }
    
protected:
    //-------------------------------------------------------------------------
    // Protected Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 LeafNodeBit   = 0x80000000;
    static const cgUInt32 LeafIndexMask = 0x7FFFFFFF;

    //-------------------------------------------------------------------------
    // Protected Enumerations, Structures & Typedefs
    //-------------------------------------------------------------------------
    enum LeafOwner
    {
        FrontOwner  = 0,
        BackOwner   = 1,
        NoOwner     = 2
    };
    enum PortalStatus
    {
        Unprocessed,
        Processing,
        Processed
    };
    struct PVSPortal;
    struct PVSPortalPoints
    {
        // Constructor
        PVSPortalPoints() :
            vertices(CG_NULL), ownerPortal(CG_NULL), vertexCount(0), ownsVertices(false) {}

        // Destructor
        ~PVSPortalPoints()
        {
            if ( ownsVertices )
                delete vertices;
        }

        // Members
        bool            ownsVertices;
        cgVector3     * vertices;
        cgUInt32        vertexCount;
        PVSPortal     * ownerPortal;
    };
    struct PVSPortal
    {
        // Constructor
        PVSPortal() :
            status(Unprocessed), leaf(cgUInt32(-1)), possibleVisCount(0),
            points(CG_NULL), ownsPoints(false) {}
        
        // Destructor
        ~PVSPortal()
        {
            if ( ownsPoints )
                delete points;
        }
        
        // Members
        bool                ownsPoints;
        PVSPortalPoints   * points;
        PortalStatus        status;
        cgUInt32            plane;
        LeafOwner           leafSide;           // Side on which the leaf falls with respect the original portal.
        cgUInt32            leaf;               // Leaf into which this portal points
        cgUInt32            possibleVisCount;
        cgByteArray         possibleVis;
        cgByteArray         actualVis;
    };
    struct PVSData
    {
        // Constructor
        PVSData() :
            sourcePoints(CG_NULL), targetPoints(CG_NULL) {}

        // Members
        PVSPortalPoints   * sourcePoints;
        PVSPortalPoints   * targetPoints;
        cgByteArray         visBits;
        cgPlane             targetPlane;
    };
    CGE_ARRAY_DECLARE( PVSPortal, PVSPortalArray );
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                mergeMeshes             ( cgUInt32 meshCount, const cgMeshHandle meshes[], const cgTransform transforms[] );
    void                buildPlaneSet           ( );
    Winding           * selectBestSplitter      ( Winding * windingList, cgUInt32 splitterSample, cgFloat splitHeuristic );
    void                buildTree               ( cgUInt32 level, cgUInt32 node, Winding * windingList );
    void                releaseWindings         ( Winding * windingList );
    cgUInt32            countSplitters          ( Winding * windingList );
    void                splitWinding            ( Winding * winding, const cgPlane & plane, Winding * frontSplit, Winding * backSplit );
    Portal            * generatePortal          ( cgUInt32 nodeIndex, const cgBoundingBox & bounds );
    Portal            * clipPortal              ( cgUInt32 nodeIndex, Portal * portal, LeafOwner ownerNodeSide );
    void                generatePVSPortals      ( PVSPortalArray & portals );
    void                initialPortalVis        ( PVSPortalArray & portals );
    void                portalFlood             ( PVSPortalArray & portals, PVSPortal & sourcePortal, cgByte * portalVis, cgUInt32 leafIndex );
    void                calculatePortalVis      ( PVSPortalArray & portals );
    cgUInt32            getNextPVSPortal        ( PVSPortalArray & portals );
    void                recursePVS              ( PVSPortalArray & portals, cgUInt32 leafIndex, PVSPortal & sourcePortal, PVSData & prevData );
    PVSPortalPoints   * clipPVSPortalPoints     ( PVSPortalPoints * points, const cgPlane & plane, bool keepOnPlane );
    PVSPortalPoints   * clipToAntiPenumbra      ( PVSPortalPoints * source, PVSPortalPoints * target, PVSPortalPoints * generator, bool reverseClip );
    cgUInt32            compressLeafSet         ( cgByte masterPVS[], const cgByte visArray[], cgUInt32 writePos );
    cgUInt32            findLeaf                ( cgUInt32 nodeIndex, const cgVector3 & point );
    void                findLeaves              ( cgUInt32 nodeIndex, cgUInt32 * leaves, cgUInt32 & leafCount, const cgBoundingSphere & sphere, cgByte * sourceVis );
    bool                isVolumeVisible         ( cgUInt32 nodeIndex, cgByte * leafVisData, const cgBoundingSphere & sphere );

    //-------------------------------------------------------------------------
    // Protected Inline Methods
    //-------------------------------------------------------------------------
    inline cgPlane getPVSPortalPlane( const PVSPortal & portal )
    {
        return ( portal.leafSide == BackOwner ) ? -mNodePlanes[portal.plane] : mNodePlanes[portal.plane];
    }

    //-------------------------------------------------------------------------
    // Protected Static Functions
    //-------------------------------------------------------------------------
    void                releasePVSPortalPoints  ( PVSPortalPoints * points );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Input data
    PointArray          mInputVertices;
    Winding           * mInputWindings;
    
    // Tree data
    cgBoundingBox       mBounds;
    PlaneArray          mNodePlanes;
    LeafArray           mLeaves;
    NodeArray           mNodes;
    PortalArray         mPortals;
    PointArray          mPortalVertices;
    cgUInt32            mPVSBytesPerSet;
    cgByteArray         mPVSData;

    // Configuration
    cgUInt32            mSplitterSample;
    cgFloat             mSplitHeuristic;

}; // End Class cgBSPTree

//---------------------------------------------------------------------------------
// Name : cgBSPTreeSubNode (Class)
/// <summary>
/// The node class specific to the BSP spatial tree type.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgBSPTreeSubNode
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
    cgBSPTreeSubNode() : 
      front( 0x7FFFFFFF ), back( 0x7FFFFFFF ), plane( -1 ) {}

    //-----------------------------------------------------------------------------
    // Public Variables
    //-----------------------------------------------------------------------------
    cgUInt32 front;
    cgUInt32 back;
    cgUInt32 plane;
};

//---------------------------------------------------------------------------------
// Name : cgBSPTreeLeaf (Class)
/// <summary>
/// The leaf class specific to the BSP spatial tree type.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgBSPTreeLeaf
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
    cgBSPTreeLeaf() :
      visibilityOffset(0) {}

    //-----------------------------------------------------------------------------
    // Public Variables
    //-----------------------------------------------------------------------------
    cgUInt32Array   portals;
    cgUInt32        visibilityOffset;
};

#endif // !_CGE_CGBSPVISTREE_H_