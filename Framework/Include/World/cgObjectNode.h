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
// Name : cgObjectNode.h                                                     //
//                                                                           //
// Desc : Class responsible for managing a tangible representation of any    //
//        object as it exists in the scene. This includes the provision of   //
//        information such as position, scale and orientation as well as     //
//        other common properties.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGOBJECTNODE_H_ )
#define _CGE_CGOBJECTNODE_H_

//-----------------------------------------------------------------------------
// cgObjectNode Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Animation/cgAnimationTarget.h>
#include <World/cgWorldTypes.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgWorldComponent.h>
#include <Physics/cgPhysicsBody.h>
#include <Navigation/cgNavigationAgent.h>
#include <System/cgPropertyContainer.h>
#include <Math/cgBoundingBox.h>
#include <Math/cgBoundingSphere.h>
#include <Math/cgTransform.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSceneCell;
class cgWorldObject;
class cgSphereTreeSubNode;
class cgCameraNode;
class cgGroupNode;
class cgObjectBehavior;
class cgNavigationAgent;
class cgPhysicsController; // ToDo: Remove if controllers go away :)
class cgVisibilitySet;
class cgTargetNode;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {708DFF17-2613-4F7D-BAA3-9BC4E4A718CB}
const cgUID RTID_ObjectNode = {0x708DFF17, 0x2613, 0x4F7D, {0xBA, 0xA3, 0x9B, 0xC4, 0xE4, 0xA7, 0x18, 0xCB}};

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgObjectNodeEventArgs
{
    cgObjectNodeEventArgs( cgObjectNode * _node ) :
        node( _node ) {}
    cgObjectNode * node;

}; // End Struct cgObjectNodeEventArgs

struct CGE_API cgObjectNodeNameChangeEventArgs : public cgObjectNodeEventArgs
{
    cgObjectNodeNameChangeEventArgs( cgObjectNode * _node, const cgString & _oldName ) :
        cgObjectNodeEventArgs( _node ), oldName( _oldName ) {}
    const cgString & oldName;

}; // End Struct cgObjectNodeNameChangeEventArgs

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgObjectNodeEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever object node events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectNodeEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgObjectNodeEventListener, cgEventListener, "ObjectNodeEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onInstanceIdentifierChange  ( cgObjectNodeNameChangeEventArgs * e ) {};
    virtual void    onTransformChange           ( cgObjectNodeEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgObjectNode (Class)
/// <summary>
/// Base class for the various types of object nodes that may exist in 
/// a scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgObjectNode : public cgAnimationTarget, public cgWorldComponentEventListener, public cgPhysicsBodyEventListener, public cgNavigationAgentEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgObjectNode, cgAnimationTarget, "ObjectNode" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgScene;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgObjectNode( cgUInt32 referenceId, cgScene * scene );
             cgObjectNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgObjectNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgInt16                  registerInputChannel    ( const cgString & channelName );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Management
    void                            unload                  ( );
    void                            unload                  ( bool unloadChildren );
    bool                            clone                   ( cgCloneMethod::Base method, cgScene * scene, bool internalNode, cgObjectNode *& nodeOut, const cgTransform & initTransform );
    
    // Properties
    bool                            setName                 ( const cgString & name );
    const cgString                & getObjectClass          ( ) const;
    const cgString                & getRenderClass          ( ) const;
    cgUInt32                        getRenderClassId        ( ) const;
    cgWorldObject                 * getReferencedObject     ( ) const;
    cgPropertyContainer           & getCustomProperties     ( );
    const cgPropertyContainer     & getCustomProperties     ( ) const;
    void                            setCustomProperties     ( const cgPropertyContainer & properties );

    // Type Information
    const cgUID                   & getObjectType           ( ) const;
    bool                            queryObjectType         ( const cgUID & type ) const;

    // Transformations
    const cgConstantBufferHandle  & getRenderTransformBuffer( );
    const cgTransform             & getCellTransform        ( );
    const cgTransform             & getLocalTransform       ( ) const;
    const cgTransform             & getOffsetTransform      ( ) const;
    const cgTransform             & getWorldTransform       ( );
    const cgTransform             & getWorldTransform       ( bool atPivot );
    const cgVector3               & getPosition             ( bool atPivot );
    cgVector3                       getXAxis                ( bool atPivot );
    cgVector3                       getYAxis                ( bool atPivot );
    cgVector3                       getZAxis                ( bool atPivot );
    const cgVector3               & getPosition             ( );
    cgVector3                       getXAxis                ( );
    cgVector3                       getYAxis                ( );
    cgVector3                       getZAxis                ( );
    cgVector3                       getScale                ( );
    const cgBoundingBox           & getBoundingBox          ( );
    void                            move                    ( cgFloat x, cgFloat y, cgFloat z );
    void                            moveLocal               ( cgFloat x, cgFloat y, cgFloat z );
    void                            scale                   ( cgFloat x, cgFloat y, cgFloat z );
    void                            scale                   ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center );
    void                            setPosition             ( cgFloat x, cgFloat y, cgFloat z );
    void                            setWorldTransform       ( const cgTransform & transform );
    void                            setTransformMethod      ( cgTransformMethod::Base method );
    cgTransformMethod::Base         getTransformMethod      ( ) const;
    void                            lookAt                  ( cgFloat x, cgFloat y, cgFloat z );
    void                            lookAt                  ( const cgVector3 & point );
    
    // Update Process
    void                            resolvePendingUpdates   ( cgUInt32 updateMask );
    cgUpdateRate::Base              getUpdateRate           ( ) const;
    bool                            hasPendingUpdates       ( ) const;
    cgUInt32                        getPendingUpdates       ( ) const;
    cgUInt32                        getLastDirtyFrame       ( ) const;

    // Relationship Management
    cgObjectNode                  * getParent               ( ) const;
    cgScene                       * getScene                ( ) const;
    cgSceneCell                   * getCell                 ( ) const;
    cgGroupNode                   * getOwnerGroup           ( ) const;
    cgObjectNode                  * getParentOfType         ( const cgUID & typeIdentifier ) const;
    cgObjectNodeList              & getChildren             ( );
    cgNodeTargetMethod::Base        getTargetMethod         ( ) const;
    cgTargetNode                  * getTargetNode           ( ) const;
    cgSphereTreeSubNode           * getSceneTreeNode        ( ) const;
    void                            setSceneTreeNode        ( cgSphereTreeSubNode * sceneTreeNode );
    bool                            isMergedAsGroup         ( ) const;
    bool                            setParent               ( cgObjectNode * parent );
    bool                            setCell                 ( cgSceneCell * cell );
    cgObjectNode                  * findChild               ( const cgString & instanceId );
    cgObjectNode                  * findChild               ( const cgString & instanceId, bool recursive );
    cgObjectNode                  * findChildOfType         ( const cgUID & objectType );
    cgObjectNode                  * findChildOfType         ( const cgUID & objectType, bool recursive );

    // System Integration (Physics, Collision, Input, etc.)
    void                            setInputChannelState    ( const cgString & channel, cgFloat state );
	void                            setInputChannelState    ( cgInt16 handle, cgFloat state );
    cgFloat                         getInputChannelState    ( const cgString & channel, cgFloat default ) const;
	cgFloat                         getInputChannelState    ( cgInt16 handle, cgFloat default ) const;
    cgPhysicsBody                 * getPhysicsBody          ( ) const;
    cgNavigationAgent             * getNavigationAgent      ( ) const;

    // Behaviors
    cgInt32                         addBehavior             ( cgObjectBehavior * behavior );
    bool                            removeBehavior          ( cgObjectBehavior * behavior, bool destroy = true );
    bool                            removeBehavior          ( cgInt32 index, bool destroy = true );
    cgObjectBehavior              * getBehavior             ( cgInt32 index );
    cgInt32                         getBehaviorCount        ( ) const;    

    // Physics
    void                            setPhysicsController    ( cgPhysicsController * controller );
    void                            setPhysicsModel         ( cgPhysicsModel::Base model );
    cgPhysicsModel::Base            getPhysicsModel         ( ) const;
    void                            setSimulationQuality    ( cgSimulationQuality::Base quality );
    cgSimulationQuality::Base       getSimulationQuality    ( ) const;
    void                            applyForce              ( const cgVector3 & force );
    void                            applyForce              ( const cgVector3 & force, const cgVector3 & at );
    void                            applyImpulse            ( const cgVector3 & impulse );
    void                            applyImpulse            ( const cgVector3 & impulse, const cgVector3 & at );
    void                            applyTorque             ( const cgVector3 & torque );
    void                            applyTorqueImpulse      ( const cgVector3 & torqueImpulse );
    void                            setVelocity             ( const cgVector3 & velocity );
    cgVector3                       getVelocity             ( ) const;
    void                            setAngularVelocity      ( const cgVector3 & velocity );
    cgVector3                       getAngularVelocity      ( ) const;

    // Navigation
    bool                            enableNavigation        ( const cgNavigationAgentCreateParams * params );
    bool                            navigateTo              ( const cgVector3 & position );
    bool                            isNavigationAgent       ( ) const;
    cgNavigationAgentState::Base    getNavigationAgentState ( ) const;
    cgNavigationTargetState::Base   getNavigationTargetState( ) const;

    // Sandbox Integration
    bool                            isParentSelected        ( bool immediateOnly = false ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // Properties
    virtual cgString                getName                 ( ) const;
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    virtual cgFloat                 getObjectSize           ( );

    // Transformations
    virtual void                    setPosition             ( const cgVector3 & position );
    virtual void                    move                    ( const cgVector3 & amount );
    virtual void                    moveLocal               ( const cgVector3 & amount );
    virtual void                    rotate                  ( cgFloat x, cgFloat y, cgFloat z );
    virtual void                    rotate                  ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center );
    virtual void                    rotateLocal             ( cgFloat x, cgFloat y, cgFloat z );
    virtual void                    rotateAxis              ( cgFloat degrees, const cgVector3 & axis );
    virtual void                    rotateAxis              ( cgFloat degrees, const cgVector3 & axis, const cgVector3 & center );
    virtual void                    scale                   ( cgFloat x, cgFloat y, cgFloat z, bool positionOnly );
    virtual void                    scale                   ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & center, bool positionOnly );
    virtual void                    scaleLocal              ( cgFloat x, cgFloat y, cgFloat z );
    virtual void                    scaleLocal              ( cgFloat x, cgFloat y, cgFloat z, const cgVector3 & localCenter );
    virtual void                    setOrientation          ( const cgVector3 & x, const cgVector3 & y, const cgVector3 & z );
    virtual void                    setOrientation          ( const cgQuaternion & rotation );
    virtual void                    resetOrientation        ( );
    virtual void                    resetScale              ( );
    virtual void                    resetPivot              ( );
    virtual void                    setWorldTransform       ( const cgTransform & transform, cgTransformSource::Base source );
    virtual bool                    setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual void                    lookAt                  ( const cgVector3 & eye, const cgVector3 & at );
    virtual void                    lookAt                  ( const cgVector3 & eye, const cgVector3 & at, const cgVector3 & up );
    virtual bool                    reloadTransforms        ( bool reloadChildren );
    virtual cgBoundingSphere        getBoundingSphere       ( );

    // Visibility
    virtual void                    showNode                ( bool visible = true, bool updateChildren = false );
    virtual bool                    isRenderable            ( ) const;
    virtual bool                    isShadowCaster          ( ) const;
    virtual bool                    registerVisibility      ( cgVisibilitySet * visibilityData );
    virtual void                    unregisterVisibility    ( cgVisibilitySet * visibilityData );
    
    // Update Process
    virtual void                    computeLevelOfDetail    ( cgCameraNode * camera );
    virtual void                    nodeUpdated             ( cgUInt32 deferredUpdates, cgUInt32 childDeferredUpdates );
    virtual bool                    isNodeDirty             ( ) const;
    virtual bool                    isNodeDirtySince        ( cgUInt32 frame ) const;
    virtual void                    setUpdateRate           ( cgUpdateRate::Base rate );
    virtual void                    update                  ( cgFloat timeDelta );

    // Relationship Management
    virtual bool                    setParent               ( cgObjectNode * parent, bool constructing );
    virtual bool                    setCell                 ( cgSceneCell * cell, bool constructing );
    virtual bool                    setOwnerGroup           ( cgGroupNode * group );
    virtual void                    setTargetMethod         ( cgNodeTargetMethod::Base mode );

    // System Integration (Physics, Collision, Input, etc.)
    virtual cgPhysicsController   * setPhysicsController    ( cgPhysicsController * controller, bool destroyOld );
    virtual cgPhysicsController   * getPhysicsController    ( );
    virtual bool                    supportsInputChannels	( ) const;
    virtual void                    hitByObject             ( cgObjectNode * node, const cgVector3 & hitPoint, const cgVector3 & surfaceNormal );
    virtual void                    objectHit               ( cgObjectNode * node, const cgVector3 & hitPoint, const cgVector3 & surfaceNormal );

    // Rendering
    virtual bool                    render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData );
    virtual bool                    renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgMaterialHandle & material );
    
    // Event Handlers
    virtual bool                    onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                    onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual bool                    onNodeInit              ( const cgUInt32IndexMap & nodeReferenceRemap );
    virtual bool                    onNodeDeleted           ( );
    virtual void                    onParentCellChanged     ( );
    virtual void                    onParentLevelChanged    ( );
    virtual void                    onResolvePendingUpdates ( cgUInt32 updates );

    // Permissions
    virtual bool                    canSetName              ( ) const;
    virtual bool                    canAdjustPivot          ( ) const;
    virtual bool                    canScale                ( ) const;
    virtual bool                    canRotate               ( ) const;
    virtual bool                    canDelete               ( ) const;
    virtual bool                    canClone                ( ) const;

    // Sandbox Integration
    virtual bool                    pick                    ( cgCameraNode * camera, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut, cgObjectNode *& closestNodeOut );
    virtual bool                    sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane );
    virtual bool                    isSelected              ( ) const;
    virtual void                    setSelected             ( bool selected, bool updateDependents = true, bool sendNotifications = true, cgObjectNodeMap & alteredNodes = cgObjectNodeMap() );
    virtual cgUInt32                getNodeColor            ( ) const;
    virtual bool                    setNodeColor            ( cgUInt32 color );
    virtual bool                    showSelectionAABB       ( ) const;
    virtual bool                    validateAttachment      ( cgObjectNode * node, bool nodeAsChild );
    virtual bool                    getSubElementCategories ( cgObjectSubElementCategory::Map & categoriesOut ) const;
    virtual bool                    supportsSubElement      ( const cgUID & Category, const cgUID & Identifier ) const;
    virtual bool                    getSandboxIconInfo      ( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasName, cgString & frameName, cgVector3 & iconOrigin );
    virtual bool                    allowSandboxUpdate      ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onComponentModified         ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsBodyEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onPhysicsBodyTransformed    ( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgNavigationAgentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onNavigationAgentReposition ( cgNavigationAgent * sender, cgNavigationAgentRepositionEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_ObjectNode; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;
    virtual bool                    processMessage              ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgAnimationTarget)
    //-------------------------------------------------------------------------
    virtual void                    getAnimationTransform       ( cgTransform & transform ) const;
    virtual void                    onAnimationTransformUpdated ( const cgTransform & transform );
    virtual void                    setInstanceIdentifier       ( const cgString & identifier );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Access Methods
    inline void setPendingUpdateEntry( cgObjectNode ** entry )
    {
        mPendingUpdateFIFO = entry;
    }
    inline cgObjectNode ** getPendingUpdateEntry( ) const
    {
        return mPendingUpdateFIFO;
    }

    // Object Method Routing
    inline cgObjectSubElement * createSubElement( const cgUID & category, const cgUID & identifier )
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->createSubElement( category, identifier );
    }
    inline void deleteSubElement( cgObjectSubElement * element )
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->deleteSubElement( element );
    }
    inline void deleteSubElements( const cgObjectSubElementArray & elements )
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->deleteSubElements( elements );
    }
    inline cgObjectSubElement * cloneSubElement( cgObjectSubElement * element )
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->cloneSubElement( element );
    }

    // Object Property 'Get' Routing
    inline const cgObjectSubElementArray & getSubElements( const cgUID & category ) const
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->getSubElements( category );
    }
    inline cgFloat getBaseMass( ) const
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->getBaseMass();
    }
    inline cgFloat getMassTransformAmount( ) const
    {
        cgAssert(mReferencedObject);
        return mReferencedObject->getMassTransformAmount();
    }

    // Object Property 'Set' Routing
    inline void setBaseMass( cgFloat mass )
    {
        cgAssert(mReferencedObject);
        mReferencedObject->setBaseMass( mass );
    }
    inline void setMassTransformAmount( cgFloat amount )
    {
        cgAssert(mReferencedObject);
        mReferencedObject->setMassTransformAmount( amount );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE      (cgObjectBehavior*, BehaviorArray)
    CGE_UNORDEREDMAP_DECLARE(cgInt16, cgFloat, InputChannelMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries                  ( );
    bool                        shouldSerialize                 ( ) const;
    bool                        serializeUpdateRate             ( );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                buildPhysicsBody                ( );
    virtual void                attachChild                     ( cgObjectNode * child );
    virtual void                removeChild                     ( cgObjectNode * child );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Properties
    cgWorldObject             * mReferencedObject;          // Physical object being referenced by this node.
    cgString                    mName;                      // The name of this node (must be unique).
    cgString                    mObjectClass;               // A custom user specified string that allows object nodes to be categorized.
    cgPropertyContainer       * mCustomProperties;          // List of custom, application defined properties for this node.
    cgUInt32                    mColor;                     // Base node color (if no material is assigned).
    cgUInt32                    mRenderClassId;             // Custom user specified identifier used to identify the object's rendering category / class.
    cgUInt16                    mFlags;                     // Various flags that describe the state of this node.

    // Transformations
    cgTransform                 mOffsetTransform;           // Transformation describing the offset from the pivot (origin) of the node, to the *actual* referenced object.
    cgTransform                 mCellTransform;             // Node's parent cell relative transformation information.
    cgTransform                 mLocalTransform;            // Describes the relative transformation between any associated parent node and this child.
    cgTransformMethod::Base     mTransformMethod;           // The currently selected approach to use when applying transformations (i.e. Standard, Adjust only the pivot, etc.)
    
    // Update Process
    cgUpdateRate::Base          mUpdateRate;                // The rate at which the node is currently set to update
    cgUInt32                    mLastDirtyFrame;            // The most recent frame on which this node was updated (moved, animated etc.)
    cgUInt32                    mPendingUpdates;            // Describes pending updates such as child hierarchy transformation adjustments.
    cgObjectNode             ** mPendingUpdateFIFO;         // This node's position in the main scene's pending update FIFO buffer.
    
    // Relationship Management
    cgObjectNodeList            mChildren;                  // List of attached child nodes in the relationship hierarchy.
    cgObjectNode              * mParentNode;                // Parent hierarchy node if any.
    cgScene                   * mParentScene;               // The parent scene in which this node exists.
    cgSceneCell               * mParentCell;                // The scene cell in which this node exists. Transformation data is relative to the center of this cell.
    cgGroupNode               * mOwnerGroup;                // This node belongs to a group?
    cgSphereTreeSubNode       * mSceneTreeNode;             // The sub-node to which this object belongs within the main scene's broadphase / scene tree.
    cgTargetNode              * mTargetNode;                // The special 'target' node that allows the user to easily manipulate this node's orientation using a positionable target.
    cgUInt16                    mNodeLevel;                 // Our level in the node hierarchy (0=root).

    // System Integration (Physics, Collision, Input, etc.)
    InputChannelMap             mInputChannels;		        // The state of the object node's input channels (if any).
    cgPhysicsController       * mPhysicsController;         // The physics controller (if any) assigned to this node.
    
    // Behaviors
    BehaviorArray               mBehaviors;                 // Array containing all behaviors assigned to this node.

    // Physics
    cgPhysicsModel::Base        mPhysicsModel;              // The type of physics integration to apply for this node.
    cgPhysicsBody             * mPhysicsBody;               // Physics body handler for this node (if any).
    cgSimulationQuality::Base   mSimulationQuality;         // Quality of the physics simulation (i.e. continuous?)

    // Navigation
    cgNavigationAgent         * mNavigationAgent;           // Underlying navigation agent, used for pathfinding when a navigation mode is selected.

    // Cached responses
    cgBoundingBox               mWorldBounds;               // Cached version of the underlying object's bounding box, transformed to world space.
    cgTransform                 mWorldPivotTransform;       // Cached world transform at pivot point.
    cgConstantBufferHandle      mWorldTransformBuffer;      // Cached world transform constant buffer.
    
    // Sandbox Related
    cgInt32                     mSelectionId;               // An identifier, issued in consecutive order, that allows us to retrieve selected nodes in the order in which they were selected.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    /// <summary>Node insert.</summary>
    static cgWorldQuery mNodeInsert;
    /// <summary>Node delete.</summary>
    static cgWorldQuery mNodeDelete;
    /// <summary>Node update cellId.</summary>
    static cgWorldQuery mNodeUpdateCell;
    /// <summary>Node update position, orientation and scale.</summary>
    static cgWorldQuery mNodeUpdateTransform;
    /// <summary>Node update pivot position, orientation and scale.</summary>
    static cgWorldQuery mNodeUpdateOffsetTransform;
    /// <summary>Node update parent relative position, orientation and scale.</summary>
    static cgWorldQuery mNodeUpdateLocalTransform;
    /// <summary>Node update color.</summary>
    static cgWorldQuery mNodeUpdateColor;
    /// <summary>Node update editor name.</summary>
    static cgWorldQuery mNodeUpdateName;
    /// <summary>Node update instance identifier.</summary>
    static cgWorldQuery mNodeUpdateInstanceIdentifier;
    /// <summary>Node update parent node.</summary>
    static cgWorldQuery mNodeUpdateParent;
    /// <summary>Node update level.</summary>
    static cgWorldQuery mNodeUpdateLevel;
    /// <summary>Node update owner group.</summary>
    static cgWorldQuery mNodeUpdateGroup;
    /// <summary>Node update physics properties.</summary>
    static cgWorldQuery mNodeUpdatePhysicsProperties;
    /// <summary>Node update processing interval (update rate).</summary>
    static cgWorldQuery mNodeUpdateUpdateRate;
    /// <summary>Update the Id of the node being targeted by this one.</summary>
    static cgWorldQuery mNodeUpdateTargetReference;
    /// <summary>Remove all custom properties associated with this node.</summary>
    static cgWorldQuery mNodeClearCustomProperties;
    /// <summary>Insert a new custom property associated with this node.</summary>
    static cgWorldQuery mNodeInsertCustomProperty;
    /// <summary>Remove specific behavior data associated with this node from the database.</summary>
    static cgWorldQuery mNodeDeleteBehavior;
    /// <summary>Insert a new behavior to be associated with this node.</summary>
    static cgWorldQuery mNodeInsertBehavior;
    /// <summary>Reload transformations from the database.</summary>
    static cgWorldQuery mNodeLoadTransforms;
    /// <summary>Load any custom properties associated with this node from the database.</summary>
    static cgWorldQuery mNodeLoadCustomProperties;
    /// <summary>Load any behaviors to be associated with this node.</summary>
    static cgWorldQuery mNodeLoadBehaviors;

private:
    //-------------------------------------------------------------------------
    // Private Structures, Typedefs & Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgInt16, InputChannelLUT)

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static InputChannelLUT  mRegisteredInputChannels;  // Look up table that allows us to retrieve the handle for a registered input channel.                  
};

#endif // !_CGE_CGOBJECTNODE_H_