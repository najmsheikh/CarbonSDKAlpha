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
// Name : cgScene.h                                                          //
//                                                                           //
// Desc : Provides classes responsible for the loading, management, update   //
//        and rendering of an individual scene.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCENE_H_ )
#define _CGE_CGSCENE_H_

//-----------------------------------------------------------------------------
// cgScene Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldTypes.h>
#include <World/cgWorldQuery.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>
#include <Physics/cgPhysicsWorld.h>
#include <System/cgReferenceManager.h>
#include <System/cgFilterExpression.h>
#include <Math/cgMathTypes.h>
#include <Math/cgBoundingBox.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgResourceManager;
class cgRenderDriver;
class cgSceneController;
class cgLightingManager;
class cgCameraNode;
class cgLightNode;
class cgGroupNode;
class cgVisibilitySet;
class cgScriptObject;
class cgSpatialTreeNode;
class cgMaterial;
class cgScene;
class cgSceneCell;
class cgSelectionSet;
class cgWorldQuery;
class cgFrustum;
class cgLandscape;
struct cgLandscapeImportParams;
class cgSphereTree;
class cgBSPTree;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {F33C4A2E-AF7A-439F-AB66-F96D2026DF5F}
const cgUID RTID_Scene = {0xF33C4A2E, 0xAF7A, 0x439F, {0xAB, 0x66, 0xF9, 0x6D, 0x20, 0x26, 0xDF, 0x5F}};

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgSceneEventArgs
{
    cgSceneEventArgs( cgScene * _scene ) :
        scene( _scene ) {}
    cgScene * scene;

}; // End Struct cgSceneEventArgs

struct CGE_API cgSelectionUpdatedEventArgs : public cgSceneEventArgs
{
    cgSelectionUpdatedEventArgs( cgScene * _scene, cgObjectNodeMap * _alteredSelection ) : 
        cgSceneEventArgs( _scene ),
        alteredSelection( _alteredSelection ) {}
    cgObjectNodeMap * alteredSelection;

};  // End Struct cgSelectionUpdatedEventArgs

struct CGE_API cgNodesUpdatedEventArgs : public cgSceneEventArgs
{
    cgNodesUpdatedEventArgs( cgScene * _scene, cgObjectNodeMap * _nodes ) :
        cgSceneEventArgs( _scene ),
        nodes           ( _nodes ) {}
    cgObjectNodeMap * nodes;

}; // End Struct cgNodesUpdatedEventArgs

struct CGE_API cgNodeUpdatedEventArgs : public cgSceneEventArgs
{
    cgNodeUpdatedEventArgs( cgScene * _scene, cgObjectNode * _node ) :
        cgSceneEventArgs( _scene ),
        node            ( _node ) {}
    cgObjectNode * node;

}; // End Struct cgNodeUpdatedEventArgs

struct CGE_API cgNodeParentChangeEventArgs : public cgSceneEventArgs
{
    cgNodeParentChangeEventArgs( cgScene * _scene, cgObjectNode * _node, cgObjectNode * _oldParent, cgObjectNode * _newParent ) :
        cgSceneEventArgs( _scene ),
        node            ( _node ),
        oldParent       ( _oldParent ),
        newParent       ( _newParent ) {}
    cgObjectNode * node;
    cgObjectNode * oldParent;
    cgObjectNode * newParent;

}; // End Struct cgNodeParentChangeEventArgs

struct CGE_API cgSceneElementEventArgs : public cgSceneEventArgs
{
    cgSceneElementEventArgs( cgScene * _scene, cgSceneElement * _element ) :
        cgSceneEventArgs( _scene ),
        element ( _element ) {}
    cgSceneElement * element;

}; // End Struct cgSceneElementEventArgs

struct CGE_API cgSelectionSetEventArgs : public cgSceneEventArgs
{
    cgSelectionSetEventArgs( cgScene * _scene, cgSelectionSet * _set ) :
        cgSceneEventArgs( _scene ),
        set             ( _set ) {}
    cgSelectionSet * set;

}; // End Struct cgSelectionSetEventArgs

struct CGE_API cgSceneMaterialEventArgs : public cgSceneEventArgs
{
public:
    cgSceneMaterialEventArgs( cgScene * _scene, cgMaterial * _material ) :
        cgSceneEventArgs( _scene ),
        material        ( _material ) {}
    cgMaterial * material;

}; // End Struct cgSceneMaterialEventArgs

struct CGE_API cgSceneLoadProgressEventArgs : public cgSceneEventArgs
{
    cgSceneLoadProgressEventArgs( cgScene * _scene ) :
        cgSceneEventArgs( _scene ) {}

}; // End Struct cgSceneLoadProgressEventArgs

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
// Key for quick lookup of scene cells based on their 3D grid reference.
struct cgSceneCellKey
{
    cgInt16 cellX, cellY, cellZ;
    cgSceneCellKey( ) :
        cellX(0), cellY(0), cellZ(0) {}
    cgSceneCellKey( cgInt16 _x, cgInt16 _y, cgInt16 _z ) :
        cellX(_x), cellY(_y), cellZ(_z) {}
};

//-----------------------------------------------------------------------------
// Global Typedefs
//-----------------------------------------------------------------------------
CGE_MAP_DECLARE(cgSceneCellKey, cgSceneCell*, cgSceneCellMap)

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSceneEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever scene events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSceneEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSceneEventListener, cgEventListener, "SceneEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onSceneLoadProgress     ( cgSceneLoadProgressEventArgs * e ) {};
    virtual void    onSceneDirtyChange      ( cgSceneEventArgs * e ) {};
    virtual void    onNodeAdded             ( cgNodeUpdatedEventArgs * e ) {};
    virtual void    onNodeDeleted           ( cgNodeUpdatedEventArgs * e ) {};
    virtual void    onNodeNameChange        ( cgNodeUpdatedEventArgs * e ) {};
    virtual void    onNodeParentChange      ( cgNodeParentChangeEventArgs * e ) {};
    virtual void    onNodesDeleted          ( cgNodesUpdatedEventArgs * e ) {};
    virtual void    onSceneElementAdded     ( cgSceneElementEventArgs * e ) {};
    virtual void    onSceneElementDeleted   ( cgSceneElementEventArgs * e ) {};
    virtual void    onSelectionUpdated      ( cgSelectionUpdatedEventArgs * e ) {};
    virtual void    onSelectionCleared      ( cgSceneEventArgs * e ) {};
    virtual void    onModifySelection       ( cgSceneEventArgs * e ) {};
    virtual void    onDeleteSelection       ( cgSceneEventArgs * e ) {};
    virtual void    onSelectionSetAdded     ( cgSelectionSetEventArgs * e ) {};
    virtual void    onSelectionSetRemoved   ( cgSelectionSetEventArgs * e ) {};
    virtual void    onMaterialAdded         ( cgSceneMaterialEventArgs * e ) {};
    virtual void    onMaterialRemoved       ( cgSceneMaterialEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgSceneDescriptor (Class)
/// <summary>
/// Contains all of the information that describes how a scene is
/// constructed, and maintains this information such that it can be
/// retrieved when it is necessary to actually load the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSceneDescriptor
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgSceneDescriptor();

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    cgUInt32                    sceneId;                            // Unique identifier of the scene as described within the database.
    cgSceneType::Base           type;                               // type of this scene (interior, exterior, etc.)
    cgString                    name;                               // The 'Editor' name of the scene (i.e. WorldExterior)
    cgString                    friendlyName;                       // Friendly name of the scene used for display to the user in-game.
    cgString                    description;                        // A more verbose description of the scene, potentially for display during loading
    cgString                    renderControl;                      // The custom render control script to use when rendering this scene (empty if use default).
    cgSceneRenderContext::Base  renderContext;                      // Context to supply to the render control script that defines its behavior.
    cgUInt32                    flags;                              // Flags describing how the scene is to be interpreted.
    cgUnitType::Base            distanceDisplayUnits;               // SI unit type in which the user wants to display distance values.
    cgBoundingBox               sceneBounds;                        // Overall bounding box of the scene.
    cgVector3                   cellDimensions;                     // Dimensions of each cell in the scene (default 64x64x64)
    cgUInt32                    landscapeId;                        // Identifier for any landscape associated with this scene (0 if none).
};

//-----------------------------------------------------------------------------
// Name : cgSelectionSet (Class)
/// <summary>
/// Stores a list of nodes that form a pre-defined selection set.
/// </summary>
//-----------------------------------------------------------------------------
class cgSelectionSet
{
public:
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgUInt32, RefIdIndexMap)
    
    /// <summary>Public name of the selection set.</summary>
    cgString                name;
    /// <summary>Is this an internal selection set, not visible to the user?</summary>
    bool                    internalSet;
    /// <summary>Lookup table to find the referenced nodes in the ordered node map.</summary>
    RefIdIndexMap           nodeLUT;
    /// <summary>Map of selected nodes in the order in which they were selected (key=order).</summary>
    cgObjectNodeMap         orderedNodes;
};

//-----------------------------------------------------------------------------
// Name : cgSceneUpdateFIFO (Class)
/// <summary>
/// Stores a list of nodes that need are pending further (deferred) updates 
/// after modification by the engine or application during the scene's update 
/// process.
/// </summary>
//-----------------------------------------------------------------------------
class cgSceneUpdateFIFO
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgSceneUpdateFIFO( size_t size )
    {
        mCount      = 0;
        mCurrent    = 0;
        mBottom     = 0;
        mSize       = size;
        mBuffer     = new cgObjectNode*[size];
    }

    ~cgSceneUpdateFIFO( )
    {
        clear();
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgObjectNode ** push( cgObjectNode * node )
    {
        ++mCount;
        cgObjectNode ** result = &mBuffer[mCurrent];
        mBuffer[mCurrent] = node;
        ++mCurrent;
        if ( mCurrent == mSize )
            mCurrent = 0;
        return result;
    };

    cgObjectNode * pop()
    {
        while ( mCurrent != mBottom )
        {
            --mCount;
            cgObjectNode * result = mBuffer[mBottom];
            ++mBottom;
            if ( mBottom == mSize )
                mBottom = 0;
            if ( result )
                return result;
        }
        return CG_NULL;
    }

    bool flush( cgObjectNode * node )
    {
        if ( mCurrent == mBottom )
            return false;

        for ( size_t i = mBottom; i != mCurrent; )
        {
            if ( mBuffer[i] == node )
            {
                mBuffer[i] = CG_NULL;
                return true;
            }
            if ( ++i == mSize )
                i = 0;
        
        } // Next
        return false;
    }

    //-------------------------------------------------------------------------
    // Name : clear ()
    /// <summary>
    /// Clear the buffer and deallocate internal resources.
    /// </summary>
    //-------------------------------------------------------------------------
    void clear( )
    {
        delete []mBuffer;
        mCount      = 0;
        mCurrent    = 0;
        mBottom     = 0;
        mSize       = 0;
        mBuffer     = CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : getEntryCount ()
    /// <summary>
    /// Determine the number of elements that are pushed onto the FIFO buffer.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_t getEntryCount( ) const
    {
        return mCount;
    }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    size_t          mCount;
    size_t          mCurrent;
    size_t          mBottom;
    size_t          mSize;
    cgObjectNode ** mBuffer;
};

//-----------------------------------------------------------------------------
//  Name : cgScene (Class)
/// <summary>
/// The main class responsible for loading and managing an individual scene. 
/// A scene is essentially a collection of objects which might for instance 
/// make up the exterior world (containing a full blown landscape etc.) or the 
/// interior of a building.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScene : public cgReference, public cgPhysicsWorldEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgScene, cgReference, "Scene" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgWorld;

public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE         (cgString, cgSelectionSet*, SelectionSetMap)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgUInt32, NameUsageMap)
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgMaterial*, SceneMaterialMap)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgScene( cgWorld * world, const cgSceneDescriptor * descriptor );
    virtual ~cgScene( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString            & getName                     ( ) const;
    cgUInt32                    getSceneId                  ( ) const;
    cgWorld                   * getParentWorld              ( ) const;
    bool                        load                        ( );
    bool                        reload                      ( );
    void                        unload                      ( );
    bool                        isLoading                   ( ) const;
    cgUInt32                    getRenderClassId            ( const cgString & className ) const;
    const cgFilterExpression::IdentifierArray & getMaterialPropertyIdentifiers( ) const;

    // Update Process
    void                        enableUpdates               ( bool enabled );
    bool                        isUpdatingEnabled           ( ) const;
    bool                        isUpdating                  ( ) const;
    void                        update                      ( );
    void                        queueNodeUpdates            ( cgObjectNode * node );
    void                        resolvedNodeUpdates         ( cgObjectNode * node );
    void                        resolvePendingUpdates       ( );

    // Scene Dynamics
    void                        enableDynamics              ( bool enabled );
    bool                        isDynamicsEnabled           ( ) const;
    
    // Scene Database
    cgObjectNode              * createObjectNode            ( bool internalNode, const cgUID & objectTypeIdentifier, bool autoAssignName );
    cgObjectNode              * createObjectNode            ( bool internalNode, const cgUID & objectTypeIdentifier, bool autoAssignName, cgCloneMethod::Base cloneMethod, cgObjectNode * nodeInit, const cgTransform & initTransform );
    cgObjectNode              * loadObjectNode              ( cgUInt32 referenceId, cgCloneMethod::Base cloneMethod, bool loadChildren );
    void                        unloadObjectNode            ( cgObjectNode * node );
    void                        unloadObjectNode            ( cgObjectNode * node, bool unloadChildren );
    void                        unloadObjectNodes           ( cgObjectNodeMap & nodes );
    cgSceneElement            * createSceneElement          ( bool internalElement, const cgUID & elementTypeIdentifier );
    void                        unloadSceneElement          ( cgSceneElement * element );
    void                        addRootNode                 ( cgObjectNode * node );
    void                        removeRootNode              ( cgObjectNode * node );
    cgObjectNode              * getObjectNodeById           ( cgUInt32 referenceId ) const;
    cgObjectNodeMap           & getObjectNodes              ( );
    const cgObjectNodeMap     & getObjectNodes              ( ) const;
    cgObjectNodeMap           & getRootObjectNodes          ( );
    const cgObjectNodeMap     & getRootObjectNodes          ( ) const;
    const cgSceneCellMap      & getSceneCells               ( ) const;
    const cgSceneElementArray & getSceneElements            ( ) const;
    const cgSceneElementArray & getSceneElementsByType      ( const cgUID & type ) const;
    const cgObjectNodeArray   & getObjectNodesByType        ( const cgUID & type ) const;
    void                        getObjectNodesInBounds      ( const cgVector3 & center, cgFloat radius, cgObjectNodeArray & nodesOut ) const;
    void                        getObjectNodesInBounds      ( const cgBoundingBox&, cgObjectNodeArray & nodesOut ) const;
    void                        setObjectUpdateRate         ( cgObjectNode * node, cgUpdateRate::Base rate );
    void                        addController               ( cgSceneController * controller );
    bool                        setActiveCamera             ( cgCameraNode * camera );
    cgCameraNode              * getActiveCamera             ( ) const;
    cgLandscape               * importLandscape             ( const cgLandscapeImportParams & params );
    bool                        rayCastClosest              ( const cgVector3 & from, const cgVector3 & to, cgSceneRayCastContact & closestContact );
    bool                        rayCastAll                  ( const cgVector3 & from, const cgVector3 & to, bool sortContacts, cgSceneRayCastContact::Array & contacts );
    
    // Cell Management
    const cgVector3           & getCellSize                 ( ) const;
    void                        updateObjectOwnership       ( cgObjectNode * node );
    void                        computeVisibility           ( const cgFrustum & frustum, cgVisibilitySet * visibilityData );
    
    // Scene Components
    cgPhysicsWorld            * getPhysicsWorld             ( ) const;
    const cgSceneDescriptor   * getSceneDescriptor          ( ) const;
    cgRenderDriver            * getRenderDriver             ( ) const;
    cgResourceManager         * getResourceManager          ( ) const;
    cgLandscape               * getLandscape                ( ) const;
    cgLightingManager         * getLightingManager          ( ) const; 
    cgSphereTree              * getSceneTree                ( ) const;

    // Scene Rendering
    void                        render                      ( );
    void                        sandboxRender               ( cgUInt32 flags, const cgPlane & gridPlane );
    bool                        beginRenderPass             ( const cgString & passName );
    void                        endRenderPass               ( );
    
    // Sandbox: General
    bool                        resetScene                  ( );
    void                        enableSceneWrites           ( bool enable );
    bool                        isSceneWritingEnabled       ( ) const;
    void                        setDirty                    ( bool dirty );
    bool                        isDirty                     ( ) const;
    cgString                    makeUniqueName              ( const cgString & name );
    cgString                    makeUniqueName              ( const cgString & name, cgUInt32 suffixNumber );
    NameUsageMap              & getNameUsage                ( );
    cgUnitType::Base            getDistanceDisplayUnits     ( ) const;
    void                        applySceneRescale           ( cgFloat scale );

    // Sandbox: Materials
    void                        addSceneMaterial            ( cgMaterial * material );
    void                        removeSceneMaterial         ( cgMaterial * material );
    const SceneMaterialMap    & getSceneMaterials           ( ) const;
    bool                        isActiveMaterial            ( cgMaterial * material ) const;

    // Sandbox: Selection management.
    void                        setActiveObjectElementType  ( const cgUID & typeIdentifier );
    const cgUID               & getActiveObjectElementType  ( ) const;
    const SelectionSetMap     & getSelectionSets            ( ) const;
    bool                        getSelectedAABB             ( cgBoundingBox & boundsOut, cgFloat growAmount );
    bool                        getSelectedPivot            ( cgVector3 & pivotOut ) const;
    bool                        getObjectNodesPivot         ( const cgObjectNodeMap & nodes, cgVector3 & pivotOut ) const;
    void                        selectAllNodes              ( );
    void                        selectSimilarNodes          ( cgObjectNodeMap & nodes, bool replaceSelection );
    bool                        selectNodes                 ( cgObjectNodeMap & nodes, bool replaceSelection );
    void                        clearSelection              ( );
    cgObjectNodeMap           & getSelectedNodes            ( );
    cgObjectNodeMap           & getSelectedNodesOrdered     ( );
    cgInt32                     getNextSelectionId          ( );
    void                        deleteSelected              ( );
    bool                        deleteObjectNode            ( cgObjectNode * node );
    bool                        deleteObjectNodes           ( cgObjectNodeMap & nodes );
    bool                        deleteSceneElement          ( cgSceneElement * element );
    cgObjectNode              * pickClosestNode             ( const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgVector3 & intersectionOut );
    cgObjectNode              * pickClosestNode             ( const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgVector3 & intersectionOut );
    void                        groupSelected               ( bool asActor );
    cgGroupNode               * groupObjectNodes            ( cgObjectNodeMap & nodes, bool asActor );
    void                        ungroupSelected             ( );
    void                        openSelectedGroups          ( );
    void                        closeSelectedGroups         ( );
    bool                        canGroupSelected            ( ) const;
    bool                        canGroupObjectNodes         ( const cgObjectNodeMap & nodes ) const;
    bool                        canUngroupSelected          ( ) const;
    bool                        canOpenSelectedGroups       ( ) const;
    bool                        canCloseSelectedGroups      ( ) const;
    bool                        detachSelected              ( );
    bool                        createSelectionSet          ( const cgString & name, bool internalSet, bool overwrite );
    bool                        removeSelectionSet          ( const cgString & name );
    bool                        applySelectionSet           ( const cgString & name, bool clearCurrent );
    bool                        cloneSelected               ( cgCloneMethod::Base method, cgObjectNodeMap & nodes, bool internalNode );
    bool                        cloneSelected               ( cgCloneMethod::Base method, cgObjectNodeMap & nodes, bool internalNode, cgUInt32 cloneCount, const cgTransform & transformDelta, cgOperationSpace::Base transformSpace );
    void                        applyMaterialToSelected     ( cgMaterial * material );
    void                        resetSelectedScale          ( );
    void                        resetSelectedOrientation    ( );
    void                        resetSelectedPivot          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                onSceneLoadProgress         ( cgSceneLoadProgressEventArgs * e );
    virtual void                onSceneDirtyChange          ( cgSceneEventArgs * e );
    virtual void                onNodeAdded                 ( cgNodeUpdatedEventArgs * e );
    virtual void                onNodeNameChange            ( cgNodeUpdatedEventArgs * e );
    virtual void                onNodeParentChange          ( cgNodeParentChangeEventArgs * e );
    virtual void                onNodeDeleted               ( cgNodeUpdatedEventArgs * e );
    virtual void                onNodesDeleted              ( cgNodesUpdatedEventArgs * e );
    virtual void                onSceneElementAdded         ( cgSceneElementEventArgs * e );
    virtual void                onSceneElementDeleted       ( cgSceneElementEventArgs * e );
    virtual void                onModifySelection           ( cgSceneEventArgs * e );
    virtual void                onSelectionUpdated          ( cgSelectionUpdatedEventArgs * e );
    virtual void                onSelectionCleared          ( cgSceneEventArgs * e );
    virtual void                onDeleteSelection           ( cgSceneEventArgs * e );
    virtual void                onSelectionSetAdded         ( cgSelectionSetEventArgs * e );
    virtual void                onSelectionSetRemoved       ( cgSelectionSetEventArgs * e );
    virtual void                onMaterialAdded             ( cgSceneMaterialEventArgs * e );
    virtual void                onMaterialRemoved           ( cgSceneMaterialEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsWorldEventListener)
    //-------------------------------------------------------------------------
    virtual void                onPhysicsStep               ( cgPhysicsWorld * sender, cgPhysicsWorldStepEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_Scene; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;
    virtual bool                processMessage              ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    // Struct containing a list of nodes to be updated and at what time.
    struct UpdateBucket
    {
        bool             locked;            // This bucket is currently being processed and should not be modified.
        cgObjectNodeList nodes;             // List of nodes to be updated at this interval
        cgDouble         lastUpdateTime;    // The last time at which these objects were updated
        cgDouble         nextUpdateTime;    // The next time at which these objects are scheduled to be updated

    }; // End Struct UpdateBucket

    struct RayCastFilterData
    {
        cgObjectNode  * node;
        void * userData;
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE       (cgSceneController*, ControllerArray)
    CGE_ARRAY_DECLARE       (cgVisibilitySet*, VisibilitySetArray)
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgObjectNodeArray, ObjectNodeTypeMap)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgObjectNode*, ObjectNodeNamedMap)
    CGE_UNORDEREDMAP_DECLARE(cgUID, cgSceneElementArray, SceneElementTypeMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    bool                        reloadRenderControl         ( bool reloadScript );
    cgObjectNode              * loadObjectNode              ( cgUInt32 rootReferenceId, cgUInt32 referenceId, cgWorldQuery * nodeData, cgCloneMethod::Base cloneMethod, cgSceneCell * parentCell, cgObjectNode * parentNode, cgObjectNodeMap & loadedNodes, bool loadChildren );
    bool                        loadSceneElements           ( );

    // Cell Management
    bool                        loadAllCells                ( );

    //-------------------------------------------------------------------------
    // Protected Static Functions
    //-------------------------------------------------------------------------
    static bool                 rayCastPreFilter            ( cgPhysicsBody * body, cgPhysicsShape * shape, void * userData );
    static cgFloat              rayCastClosestFilter        ( cgPhysicsBody * body, const cgVector3 & hitNormal, cgInt collisionId, void * userData, cgFloat intersectParam );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgWorld               * mWorld;                     // Cached pointer to the parent world object.
    cgSceneDescriptor       mSceneDescriptor;           // The descriptor structure for this scene
    cgScriptHandle          mRenderScript;              // Script based render control logic.
    cgScriptObject        * mScriptObject;              // Reference to the scripted render control object (owned exclusively by the script).
    bool                    mIsLoading;                 // Is the scene in the process of loading?
    
    // Additional Scene Components
    cgPhysicsWorld        * mPhysicsWorld;              // The physics world responsible for handling the dynamics portion of the scene update.
    cgLandscape           * mLandscape;                 // Each 'exterior world' can have an expansive terrain/landscape to represent the primary game environment.
    cgLightingManager     * mLightingManager;           // Support class to manage the lighting system
    cgSceneElementArray     mElements;                  // List of scene elements currently loaded and resident in memory.
    SceneElementTypeMap     mElementTypes;              // List of scene elements categorized by type.

    // Object nodes
    cgObjectNodeMap         mObjectNodes;               // Physical scene nodes being managed (in the order in which they were added).
    ObjectNodeTypeMap       mObjectNodeTypes;           // List of scene nodes categorized by type.
    cgObjectNodeMap         mRootNodes;                 // List of nodes that exist at the root level of the hierarchy.
    cgCameraNode          * mActiveCamera;              // The currently active camera node

    // Object Update Processing
    UpdateBucket            mUpdateBuckets[cgUpdateRate::Count];
    cgSceneUpdateFIFO       mPendingUpdateFIFO;
    bool                    mUpdatingEnabled;
    bool                    mIsUpdating;

    // Controllers
    ControllerArray         mSceneControllers;          // List of applied scene controllers that may manipulate scene data.

    // Script callback cache
    cgScriptArgument::Array mOnSceneRenderArgs;         // Cached argument list for script 'onSceneRender()' method.
    cgScriptFunctionHandle  mOnSceneRenderMethod;       // Cached handle for script 'onSceneRender()' method.

    // Rendering Related
    bool                    mPassBegun;                 // Was a rendering pass begun?
    
    // Cell Management
    cgSceneCellMap          mCells;                     // All defined cells, organized by 3D grid reference.
    cgSphereTree          * mSceneTree;                 // Primary scene broadphase tree.
    cgBSPTree             * mStaticVisTree;             // Static visibility tree.
    
    // Dynamics Related
    bool                    mDynamicsEnabled;           // Is dynamics processing (physics) enabled?

    // Miscellaneous
    VisibilitySetArray      mOrphanVisSets;             // A list of orphan visibility sets after scene has been disposed.

    // Sandbox: General
    bool                    mIsDirty;                   // Has the scene been modified since it was last serialized?
    bool                    mSceneWritesEnabled;        // Nodes are allowed to update their transformation states and owner cells?
    NameUsageMap            mNameUsage;                 // A list of node names currently in use within the scene (used for optimizing name selection).

    // Sandbox: Materials
    SceneMaterialMap        mActiveMaterials;           // List of materials that are active within this scene and will be visible in the material editor.

    // Sandbox: Selection Management
    cgObjectNodeMap         mSelectedNodes;             // Collection of currently selected nodes indexed by reference identifier.
    cgObjectNodeMap         mSelectedNodesOrdered;      // Collection of currently selected nodes in the order in which they were selected.
    cgInt32                 mNextSelectionId;           // The next identifier that will be issued when a node is selected.
    SelectionSetMap         mSelectionSets;             // Collection of named selection sets (lists of nodes to be selected as a group).
    cgUID                   mActiveObjectElementType;   // The sandbox environment is manipulating the specified element type (i.e. object sub-element category).

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    static cgWorldQuery     mInsertMaterialUsage;
    static cgWorldQuery     mDeleteMaterialUsage;
    static cgWorldQuery     mInsertElementUsage;
    static cgWorldQuery     mDeleteElementUsage;
    static cgWorldQuery     mLoadObjectNodes;
    static cgWorldQuery     mLoadChildObjectNodes;
};

//-----------------------------------------------------------------------------
// Global Operators
//-----------------------------------------------------------------------------
inline bool CGE_API operator < (const cgSceneCellKey& key1, const cgSceneCellKey& key2);

#endif // !_CGE_CGSCENE_H_