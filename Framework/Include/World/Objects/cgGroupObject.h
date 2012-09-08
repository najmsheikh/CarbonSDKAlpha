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
// Name : cgGroupObject.h                                                    //
//                                                                           //
// Desc : Contains classes responsible for providing a scene level "group"   //
//        object that can be used to combine objects into higher level       //
//        collections that can be manipulated as one.                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGGROUPOBJECT_H_ )
#define _CGE_CGGROUPOBJECT_H_

//-----------------------------------------------------------------------------
// cgGroupObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {0A17C615-C88B-4DF3-87DF-11AFB985E84B}
const cgUID RTID_GroupNode   = { 0x0A17C615, 0xC88B, 0x4DF3, { 0x87, 0xDF, 0x11, 0xAF, 0xB9, 0x85, 0xE8, 0x4B } };
// {ABCA2953-715A-42DB-85B6-C2FC000F1863}
const cgUID RTID_GroupObject = { 0xABCA2953, 0x715A, 0x42DB, { 0x85, 0xB6, 0xC2, 0xFC, 0x00, 0x0F, 0x18, 0x63 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgGroupObject (Class)
/// <summary>
/// Maintains a collection of scene nodes as a single parent level group.
/// Almost all of the functionality is supplied by the custom cgGroupNode
/// type, with this object type largely managing database access.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgGroupObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgGroupObject, cgWorldObject, "GroupObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgGroupObject( cgUInt32 referenceId, cgWorld * world );
             cgGroupObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgGroupObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        isOpen                  ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                setOpen                 ( bool open );

    //---------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //---------------------------------------------------------------------
    virtual void                sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_GroupObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool mOpen;   // True if the group is currently open and its child objects are available for editing.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertGroup;
    static cgWorldQuery     mUpdateOpen;
    static cgWorldQuery     mLoadGroup;
};

//-----------------------------------------------------------------------------
//  Name : cgGroupNode (Class)
/// <summary>
/// Custom node type for the group object. Manages additional processes
/// necessary when changes to the group object take place and to correctly
/// interpret child node arrangements.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgGroupNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgGroupNode, cgObjectNode, "GroupNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgGroupNode( cgUInt32 referenceId, cgScene * scene );
             cgGroupNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgGroupNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        detachNode              ( cgObjectNode * node );
    void                        getGroupedNodes         ( cgObjectNodeMap & nodesOut, bool explodeGroups );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgUInt32            onGroupRefAdded         ( cgObjectNode * node );
    virtual cgUInt32            onGroupRefRemoved       ( cgObjectNode * node );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual bool                pick                    ( cgCameraNode * camera, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut, cgObjectNode *& closestNode );
    virtual void                setSelected             ( bool selected, bool updateDependents = true, bool sendNotifications = true );
    virtual bool                onNodeDeleted           ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setOpen( bool open )
    {
        ((cgGroupObject*)mReferencedObject)->setOpen( open );
    }
    
    // Object Property 'Get' Routing
    inline bool isOpen( ) const
    {
        return ((cgGroupObject*)mReferencedObject)->isOpen( );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_GroupNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void         alterSelectionState( bool state, cgObjectNode * node );
    void         alterOpenState     ( bool state, cgObjectNode * node );
    void         computeGroupAABB   ( cgObjectNode * node, cgBoundingBox & bounds, cgTransform & inverseGroupTransform );
    void         getGroupedNodes    ( cgObjectNodeMap & nodesOut, cgGroupNode * parentNode, cgObjectNode * node, bool explodeGroups );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool                mAutoSelfDestruct;      // Automatically destroy the group when all child nodes are detached / deleted.
    cgBoundingBox       mBounds;                // Object space axis aligned bounding box values.
    cgUInt32            mGroupRefCount;         // The number of nodes in the scene that reference this group as their owner.
};

#endif // !_CGE_CGGROUPOBJECT_H_