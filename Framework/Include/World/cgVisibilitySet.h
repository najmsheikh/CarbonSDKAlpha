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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
    CGE_MAP_DECLARE(void*, size_t, EntryLUT)
    struct MaterialBatch
    {
        cgBoundingBox       combinedBounds;
        EntryLUT            objectNodeLUT;
        cgObjectNodeArray   objectNodes;
    };
    CGE_MAP_DECLARE(cgMaterialHandle, MaterialBatch, MaterialBatchMap )
    struct RenderClass
    {
        cgBoundingBox       combinedBounds;
        cgObjectNodeArray   objectNodes;
        MaterialBatchMap    materials;
    };
    CGE_MAP_DECLARE(cgUInt32, RenderClass, RenderClassMap)
    
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgVisibilitySet( );
    virtual ~cgVisibilitySet( );
    
    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    void                        compute                 ( cgScene * scene, const cgFrustum & frustum, cgUInt32 flags = 0, bool autoApply = false );
    void                        compute                 ( cgScene * scene, const cgBoundingBox & bounds, cgUInt32 flags = 0, bool autoApply = false );
    void                        apply                   ( );
    void                        clear                   ( );
    bool                        isEmpty                 ( ) const;
    cgUInt32                    getResultId             ( ) const;
    void                        addVisibleObject        ( cgObjectNode * object );
    void                        addVisibleLight         ( cgObjectNode * light );
    void                        addVisibleMaterial      ( const cgMaterialHandle & material, cgObjectNode * object );
    void                        addVisibleLeaf          ( cgSpatialTreeInstance * tree, cgSpatialTreeLeaf * leaf );
    void                        addVisibleGroup         ( void * context, cgInt32 groupId );
    void                        enableLightOcclusion    ( bool enable );
    void                        intersect               ( cgVisibilitySet * operatorSet, cgVisibilitySet * outputSet );
    void                        intersect               ( const cgFrustum & operatorFrustum, cgVisibilitySet * outputSet );
    void                        intersect               ( const cgFrustum & operatorFrustum, const cgVector3 & sweepDir, cgVisibilitySet * outputSet, cgBoundingBox & boundsOut );
    bool                        query                   ( cgObjectNode * node ) const;
    cgObjectNodeArray         & getVisibleObjects       ( );
    const cgObjectNodeArray   & getVisibleObjects       ( ) const;
    cgObjectNodeArray         & getVisibleLights        ( );
    const cgObjectNodeArray   & getVisibleLights        ( ) const;
    cgSceneLeafSet            & getVisibleLeaves        ( cgSpatialTreeInstance * tree );
    const cgSceneLeafSet      & getVisibleLeaves        ( cgSpatialTreeInstance * tree ) const;
    cgInt32Set                & getVisibleGroups        ( void * context );
    const cgInt32Set          & getVisibleGroups        ( void * context ) const;
    RenderClassMap            & getVisibleRenderClasses ( );
    const RenderClassMap      & getVisibleRenderClasses ( ) const;
    cgFrustum                 & getVolume               ( );
    const cgFrustum           & getVolume               ( ) const;

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
    CGE_UNORDEREDMAP_DECLARE(void*, cgObjectNodeArray, AssociatedLightMap)

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    cgScene               * mScene;                 // The scene on which the query is to run.
    TreeLeafMap             mTreeLeaves;            // List of visible spatial tree leaves, grouped by their parent tree.
    AssociatedGroupMap      mAssociatedGroups;      // List of visible data groups associated with a given context (i.e. blocks in a terrain).
    
    EntryLUT                mObjectNodeLUT;         // Look up table which maps a specific object node pointer to the entry in the main array.
    cgObjectNodeArray       mObjectNodes;           // Array containing a list of all objects visible to this visibility set's parent camera / frustum / bounds.
    EntryLUT                mLightLUT;              // Look up table which maps a specific light node pointer to the entry in the main array.
    cgObjectNodeArray       mLights;                // Array containing a list of all objects visible to this visibility set's parent camera / frustum / bounds.
    RenderClassMap          mRenderClasses;         // List of visible materials & objects categorized by render class.
    AssociatedLightMap      mIlluminationSet;       // Keeps track of all lights which influence a given object.

    cgUInt32                mResultId;              // The unique result identifier used to distinguish between multiple visibility sets.
    bool                    mLightOcclusion;        // Perform light occlusion testing.
    cgFrustum               mFrustum;               // Frustum representing the visibility set's volume.
    
    // Recomputation avoidance
    cgUInt32                mLastComputedFrame;     // The frame on which this visibility set was last computed
    cgFrustum               mLastFrustum;           // The last frustum used to compute visibility
    cgUInt32                mLastFlags;             // The last search flags used to computed visibility
    bool                    mLastLightOcclusion;    // The last light occlusion status used to compute visibility.

    //-------------------------------------------------------------------------
	// Private Static Variables
	//-------------------------------------------------------------------------
    static cgUInt32         mNextResultId;          // The next 'result' identifier used to differentiate between multiple visibility sets.
    static cgUInt32         mAppliedSet;            // The visibility set that was most recently "applied" to the scene.
};

#endif // !_CGE_CGVISIBILITYSET_H_