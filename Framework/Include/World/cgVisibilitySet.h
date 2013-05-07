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
// Name : cgVisibilitySet.h                                                  //
//                                                                           //
// Desc : Classes responsible for collecting, storing and managing the       //
//        visibility information for parts of the scene including            //
//        objects and spatial tree leaves.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGVISIBILITYSET_H_ )
#define _CGE_CGVISIBILITYSET_H_

//-----------------------------------------------------------------------------
// cgVisibilitySet Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>
#include <World/cgWorldTypes.h>
#include <Math/cgFrustum.h>
#include <Math/cgBoundingBox.h>
#include <System/cgPoolAllocator.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgObjectNode;
class cgSpatialTreeInstance;
class cgSpatialTreeLeaf;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgVisibilitySet (Class)
/// <summary>
/// Responsible for collecting, storing and managing the visibility 
/// information for parts of the scene including  objects and spatial 
/// tree leaves.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgVisibilitySet : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgVisibilitySet, "VisibilitySet" )

public:
    //-------------------------------------------------------------------------
	// Public Typedefs, Structures and Enumerations
	//-------------------------------------------------------------------------
    CGE_MAP_DECLARE(void*, cgObjectNodeList::iterator, EntryLUT)
    struct MaterialBatch
    {
        cgBoundingBox       combinedBounds;
        EntryLUT            objectNodeLUT;
        cgObjectNodeList    objectNodes;
    };
    CGE_MAP_DECLARE(cgMaterialHandle, MaterialBatch, MaterialBatchMap )
    struct RenderClass
    {
        cgBoundingBox       combinedBounds;
        EntryLUT            objectNodeLUT;
        cgObjectNodeList    objectNodes;
        MaterialBatchMap    materials;
    };
    CGE_MAP_DECLARE(cgUInt32, RenderClass, RenderClassMap)
    
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgVisibilitySet( cgScene * scene );
    virtual ~cgVisibilitySet( );
    
    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                        compute                 ( const cgFrustum & frustum );
    void                        clear                   ( );
    bool                        isEmpty                 ( ) const;
    bool                        isObjectVisible         ( cgObjectNode * object ) const;
    bool                        isLightVisible          ( cgObjectNode * light ) const;
    cgUInt32                    getResultId             ( ) const;
    bool                        addVisibleObject        ( cgObjectNode * object );
    void                        removeVisibleObject     ( cgObjectNode * object );
    bool                        addVisibleLight         ( cgObjectNode * light );
    void                        removeVisibleLight      ( cgObjectNode * object );
    bool                        addVisibleMaterial      ( const cgMaterialHandle & material, cgObjectNode * object );
    void                        addVisibleLeaf          ( cgSpatialTreeInstance * tree, cgSpatialTreeLeaf * leaf );
    void                        addVisibleGroup         ( void * context, cgInt32 groupId );
    bool                        query                   ( cgObjectNode * node ) const;
    void                        setSearchFlags          ( cgUInt32 flags );
    cgObjectNodeList          & getVisibleObjects       ( );
    const cgObjectNodeList    & getVisibleObjects       ( ) const;
    cgObjectNodeList          & getVisibleLights        ( );
    const cgObjectNodeList    & getVisibleLights        ( ) const;
    cgSceneLeafSet            & getVisibleLeaves        ( cgSpatialTreeInstance * tree );
    const cgSceneLeafSet      & getVisibleLeaves        ( cgSpatialTreeInstance * tree ) const;
    cgInt32Set                & getVisibleGroups        ( void * context );
    const cgInt32Set          & getVisibleGroups        ( void * context ) const;
    RenderClassMap            & getVisibleRenderClasses ( );
    const RenderClassMap      & getVisibleRenderClasses ( ) const;
    cgFrustum                 & getVolume               ( );
    const cgFrustum           & getVolume               ( ) const;
    cgUInt32                    getSearchFlags          ( ) const;
    bool                        isSetModifiedSince      ( cgUInt32 frame ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
	// Private Structures, Typedefs and Enumerations
	//-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(void*, cgSceneLeafSet, TreeLeafMap)
    CGE_UNORDEREDMAP_DECLARE(void*, cgInt32Set, AssociatedGroupMap)

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    cgScene               * mScene;                 // The scene on which the query is to run.
    TreeLeafMap             mTreeLeaves;            // List of visible spatial tree leaves, grouped by their parent tree.
    AssociatedGroupMap      mAssociatedGroups;      // List of visible data groups associated with a given context (i.e. blocks in a terrain).
    
    EntryLUT                mObjectNodeLUT;         // Look up table which maps a specific object node pointer to the entry in the main array.
    cgObjectNodeList        mObjectNodes;           // Array containing a list of all objects visible to this visibility set's parent camera / frustum / bounds.
    EntryLUT                mLightLUT;              // Look up table which maps a specific light node pointer to the entry in the main array.
    cgObjectNodeList        mLights;                // Array containing a list of all objects visible to this visibility set's parent camera / frustum / bounds.
    RenderClassMap          mRenderClasses;         // List of visible materials & objects categorized by render class.

    cgUInt32                mSearchFlags;           // Combination of cgVisibilitySearchFlags that are used to determine which objects we are interest in for this set.
    cgUInt32                mResultId;              // The unique result identifier used to distinguish between multiple visibility sets.
    cgFrustum               mFrustum;               // Frustum representing the visibility set's volume.
    
    // Recomputation avoidance
    cgUInt32                mLastModifiedFrame;     // The last frame on which data was added to, or removed from the visibility set.
    cgUInt32                mLastComputedFrame;     // The frame on which this visibility set was last computed
    cgFrustum               mLastFrustum;           // The last frustum used to compute visibility
    
    //-------------------------------------------------------------------------
	// Private Static Variables
	//-------------------------------------------------------------------------
    static cgUInt32         mNextResultId;          // The next 'result' identifier used to differentiate between multiple visibility sets.
    static cgUInt32         mAppliedSet;            // The visibility set that was most recently "applied" to the scene.
};

#endif // !_CGE_CGVISIBILITYSET_H_