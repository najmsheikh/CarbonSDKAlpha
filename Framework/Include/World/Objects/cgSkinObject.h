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
// Name : cgSkinObject.h                                                     //
//                                                                           //
// Desc : Contains managed mesh and associated classes used to maintain      //
//        skinned mesh data.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSKINOBJECT_H_ )
#define _CGE_CGSKINOBJECT_H_

//-----------------------------------------------------------------------------
// cgSkinObject Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgMeshObject.h>
#include <World/cgWorldQuery.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;
class cgBoneNode;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5688AE45-A1A9-462C-9376-023FFF1E396E}
const cgUID RTID_SkinNode   = {0x5688AE45, 0xA1A9, 0x462C, {0x93, 0x76, 0x2, 0x3F, 0xFF, 0x1E, 0x39, 0x6E}};
// {EF8CDC29-D61B-4AC6-A10A-DEAAB4983518}
const cgUID RTID_SkinObject = {0xEF8CDC29, 0xD61B, 0x4AC6, {0xA1, 0xA, 0xDE, 0xAA, 0xB4, 0x98, 0x35, 0x18}};

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgSkinObject (Class)
/// <summary>
/// Maintains information about any skinned mesh that has been created / loaded 
/// and inserted into the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSkinObject : public cgMeshObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSkinObject, cgMeshObject, "SkinObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSkinObject( cgUInt32 referenceId, cgWorld * world );
             cgSkinObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgSkinObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    void                        setShadowStage          ( cgSceneProcessStage::Base stage );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual bool                render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & material );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                getSubElementCategories ( cgObjectSubElementCategory::Map & categories ) const;
    virtual bool                supportsSubElement      ( const cgUID & category, const cgUID & identifier ) const;
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual void                onComponentDeleted      ( );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SkinObject; }
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
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertSkin;
    static cgWorldQuery mUpdateMeshData;
    static cgWorldQuery mUpdateProcessStages;
    static cgWorldQuery mLoadSkin;
};

//-----------------------------------------------------------------------------
//  Name : cgSkinNode (Class)
/// <summary>
/// Custom node type for the skin object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSkinNode : public cgMeshNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSkinNode, cgMeshNode, "SkinNode" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgSkinObject;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSkinNode( cgUInt32 referenceId, cgScene * scene );
             cgSkinNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgSkinNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        attachBone              ( cgBoneNode * pBone );
    void                        generateBoneShapes      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeInit              ( const cgUInt32IndexMap & NodeReferenceRemap );
    virtual bool                onNodeDeleted           ( );
    virtual cgBoundingBox       getLocalBoundingBox     ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SkinNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setShadowStage( cgSceneProcessStage::Base stage ) const
    {
        ((cgSkinObject*)mReferencedObject)->setShadowStage( stage );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE( cgString, cgUInt32, BoneInstanceMap )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual void                buildPhysicsBody        ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>List of bones that influence this skin.</summary>
    cgUInt32Set         mBoneReferences;
    /// <summary>Dictionary containing bones that influence this skin with their instance identifier as key.</summary>
    BoneInstanceMap     mBoneInstanceIdentifiers;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertAttachedBone;
    static cgWorldQuery mDeleteAttachedBones;
    static cgWorldQuery mLoadAttachedBones;
};

#endif // !_CGE_CGSKINOBJECT_H_