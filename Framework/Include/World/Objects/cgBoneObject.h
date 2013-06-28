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
// Name : cgBoneObject.h                                                     //
//                                                                           //
// Desc : Contains classes that allow for rigging of skinned meshes with     //
//        bone information. These bones can then be animated independantly   //
//        using the animation controller, in order to affect the skinned     //
//        mesh to which they are assigned.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBONEOBJECT_H_ )
#define _CGE_CGBONEOBJECT_H_

//-----------------------------------------------------------------------------
// cgBoneObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {13BB0956-1CF7-43B9-8772-02905C79AA9D}
const cgUID RTID_BoneNode   = {0x13BB0956, 0x1CF7, 0x43B9, {0x87, 0x72, 0x02, 0x90, 0x5C, 0x79, 0xAA, 0x9D}};
// {D4FAA572-A4A4-4F14-8E74-75842E63D4CB}
const cgUID RTID_BoneObject = { 0xD4FAA572, 0xA4A4, 0x4F14, { 0x8E, 0x74, 0x75, 0x84, 0x2E, 0x63, 0xD4, 0xCB } };

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgMesh;
class cgSkinBindData;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgBoneObject (Class)
/// <summary>
/// A simple bone object. Used most often to provide control and influences for
/// skinned meshes.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoneObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBoneObject, cgWorldObject, "BoneObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBoneObject( cgUInt32 referenceId, cgWorld * world );
             cgBoneObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgBoneObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setInitialCollisionState( bool enable );
    bool                        getInitialCollisionState( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual cgBoundingBox       getLocalBoundingBox     ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BoneObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );
    bool                        createSandboxMeshes     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool            mInitialCollisionState;     // Initial state of whether or not bone should be included in collision testing (i.e., for ray tests).
    cgMeshHandle    mSandboxMesh;               // Representation of this bone for sandbox rendering.
    cgMeshHandle    mSandboxLinkMesh;           // Representation of this bone for sandbox rendering.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBone;
    static cgWorldQuery     mUpdateProperties;
    static cgWorldQuery     mLoadBone;
};

//-----------------------------------------------------------------------------
//  Name : cgBoneNode (Class)
/// <summary>
/// Custom node type for the bone object. Manages additional properties
/// that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoneNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBoneNode, cgObjectNode, "BoneNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBoneNode( cgUInt32 referenceId, cgScene * scene );
             cgBoneNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgBoneNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgVector3                   getDirection            ( );
    void                        setBoneOrientation      ( const cgVector3 & source, const cgVector3 & destination, const cgVector3 & up );
    bool                        generateCollisionShape  ( cgMesh * mesh, cgUInt32 boneIndex = cgUInt32(-1) );
    bool                        isCollisionEnabled      ( ) const;
    void                        enableCollision         ( bool enable );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual bool                onNodeInit              ( const cgUInt32IndexMap & nodeReferenceRemap );
    virtual void                move                    ( const cgVector3 & amount );
    virtual void                moveLocal               ( const cgVector3 & amount );
    virtual bool                getSubElementCategories ( cgObjectSubElementCategory::Map & categoriesOut ) const;
    virtual void                buildPhysicsBody        ( );

    // Promote remaining base class method overloads.
    using cgObjectNode::move;
    using cgObjectNode::moveLocal;

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setInitialCollisionState( bool enable )
    {
        ((cgBoneObject*)mReferencedObject)->setInitialCollisionState( enable );
    }

    // Object Property 'Get' Routing
    inline bool getInitialCollisionState( ) const
    {
        return ((cgBoneObject*)mReferencedObject)->getInitialCollisionState();
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BoneNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool    mCollisionEnabled;    // Current state of whether or not collision volume should be used.
};

#endif // !_CGE_CGBONEOBJECT_H_