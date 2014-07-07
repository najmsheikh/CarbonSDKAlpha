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
// Name : cgProceduralTreeObject.h                                           //
//                                                                           //
// Desc : Provides support for procedurally generated tree that can be       //
//        placed into a scene as an standalone object.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPROCEDURALTREEOBJECT_H_ )
#define _CGE_CGPROCEDURALTREEOBJECT_H_ 

//-----------------------------------------------------------------------------
// cgProceduralTreeObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgProceduralTreeGenerator;
class cgProceduralTreeGrowthProperties;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {6DA7E28D-7270-48C7-A98E-67FC66023324}
const cgUID RTID_ProceduralTreeNode   = { 0x6da7e28d, 0x7270, 0x48c7, { 0xa9, 0x8e, 0x67, 0xfc, 0x66, 0x2, 0x33, 0x24 } };
// {61F28388-5F12-4262-84C9-AAD2EB84627B}
const cgUID RTID_ProceduralTreeObject = { 0x61f28388, 0x5f12, 0x4262, { 0x84, 0xc9, 0xaa, 0xd2, 0xeb, 0x84, 0x62, 0x7b } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeObject (Class)
/// <summary>
/// Core object type that provides management of an individual procedural tree
/// can be designed and placed into the world.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgProceduralTreeObject, cgWorldObject, "ProceduralTreeObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProceduralTreeObject( cgUInt32 referenceId, cgWorld * world );
             cgProceduralTreeObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgProceduralTreeObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgProceduralTreeGrowthProperties& getGrowthProperties ( ) const;
    bool                        setGrowthProperties     ( const cgProceduralTreeGrowthProperties & properties, bool regenerate );
    bool                        regenerate              ( );
    cgMeshHandle                getMesh                 ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual bool                render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & material );
    virtual void                applyObjectRescale      ( cgFloat scale );
    virtual bool                isRenderable            ( ) const;
    virtual bool                isShadowCaster          ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                onComponentDeleted      ( );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ProceduralTreeObject; }
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
    cgProceduralTreeGenerator * mGenerator;
    cgSceneProcessStage::Base   mShadowStage;   // Describes the lighting stage(s) in which this object is allowed to cast shadows.
    cgMeshHandle                mTreeMesh;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertTree;
    //static cgWorldQuery     mUpdateProcessStages;
    static cgWorldQuery     mLoadTree;
};

//-----------------------------------------------------------------------------
//  Name : cgProceduralTreeNode (Class)
/// <summary>
/// Custom node type for the procedural tree object. Manages additional
/// properties that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProceduralTreeNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgProceduralTreeNode, cgObjectNode, "ProceduralTreeNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProceduralTreeNode( cgUInt32 referenceId, cgScene * scene );
             cgProceduralTreeNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgProceduralTreeNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setGrowthProperties( const cgProceduralTreeGrowthProperties & properties, bool regenerate )
    {
        ((cgProceduralTreeObject*)mReferencedObject)->setGrowthProperties( properties, regenerate );
    }

    // Object Property 'Get' Routing
    inline cgMeshHandle getMesh( ) const
    {
        return ((cgProceduralTreeObject*)mReferencedObject)->getMesh( );
    }
    inline const cgProceduralTreeGrowthProperties & getGrowthProperties( ) const
    {
        return ((cgProceduralTreeObject*)mReferencedObject)->getGrowthProperties( );
    }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual void                onComponentModified     ( cgReference * sender, cgComponentModifiedEventArgs * e );
    virtual bool                registerVisibility      ( cgVisibilitySet * visibilityData );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ProceduralTreeNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
};

#endif // !_CGE_CGPROCEDURALTREEOBJECT_H_ 