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
// Name : cgMeshObject.h                                                     //
//                                                                           //
// Desc : Simple cgWorldObject type designed for storage and rendering of    //
//        straight forward cgMesh objects.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMESHOBJECT_H_ )
#define _CGE_CGMESHOBJECT_H_

//-----------------------------------------------------------------------------
// cgMeshObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>
#include <Resources/cgResourceTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {4986DD9C-1AF7-4AD8-A922-569A207DD394}
const cgUID RTID_MeshNode   = { 0x4986DD9C, 0x1AF7, 0x4AD8, { 0xA9, 0x22, 0x56, 0x9A, 0x20, 0x7D, 0xD3, 0x94 } };
// {68CA65E0-8768-489D-9257-2DD101975A93}
const cgUID RTID_MeshObject = { 0x68CA65E0, 0x8768, 0x489D, { 0x92, 0x57, 0x2D, 0xD1, 0x01, 0x97, 0x5A, 0x93 } };

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgMeshObject (Class)
/// <summary>
/// Maintains information about any mesh that has been created / loaded and
/// inserted into the scene.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMeshObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMeshObject, cgWorldObject, "MeshObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgMeshObject( cgUInt32 referenceId, cgWorld * world );
             cgMeshObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgMeshObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setMesh                 ( const cgMeshHandle & mesh );
    cgMeshHandle                getMesh                 ( ) const;
    bool                        createBox               ( cgFloat width, cgFloat height, cgFloat depth, cgUInt32 widthSegments, cgUInt32 heightSegments, cgUInt32 depthSegments, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createSphere            ( cgFloat radius, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createCylinder          ( cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createCapsule           ( cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createCone              ( cgFloat radius, cgFloat radiusTip, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createTorus             ( cgFloat outerRadius, cgFloat innerRadius, cgUInt32 bands, cgUInt32 sides, bool inverted, cgMeshCreateOrigin::Base origin );
    bool                        createTeapot            ( );
    bool                        pickFace                ( const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgVector3 & intersectionOut, cgUInt32 & faceOut, cgMaterialHandle & materialOut );
    bool                        setMeshMaterial         ( const cgMaterialHandle & material );
    bool                        setFaceMaterial         ( cgUInt32 faceIndex, const cgMaterialHandle & material );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual bool                getSubElementCategories ( cgObjectSubElementCategory::Map & categories ) const;
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
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
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual void                onComponentDeleted      ( );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_MeshObject; }
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
    // Protected Variables
    //-------------------------------------------------------------------------
    cgMeshHandle    mMesh;    // Handle to mesh resource
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMesh;
    static cgWorldQuery mUpdateMeshData;
    static cgWorldQuery mUpdateProcessStages;
    static cgWorldQuery mLoadMesh;
};

//-----------------------------------------------------------------------------
//  Name : cgMeshNode (Class)
/// <summary>
/// Custom node type for the mesh object. Manages additional properties
/// that may need to be override by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMeshNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMeshNode, cgObjectNode, "MeshNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgMeshNode( cgUInt32 referenceId, cgScene * scene );
             cgMeshNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgMeshNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    // Object Property 'Get' Routing
    inline cgMeshHandle getMesh( ) const
    {
        return ((cgMeshObject*)mReferencedObject)->getMesh( );
    }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                registerVisibility      ( cgVisibilitySet * visibilityData, cgUInt32 flags );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual void                onResolvePendingUpdates ( cgUInt32 updates );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual cgFloat             getObjectSize           ( );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual bool                getSubElementCategories ( cgObjectSubElementCategory::Map & categoriesOut ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_MeshNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        computeWorldSize        ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual void                buildPhysicsBody        ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Records the approximate world space size of this object for the purposes of visibility culling and LOD.</summary>
    cgFloat mWorldSize;
};

#endif // !_CGE_CGMESHOBJECT_H_