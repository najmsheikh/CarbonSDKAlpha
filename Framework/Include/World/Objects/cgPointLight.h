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
// Name : cgPointLight.h                                                     //
//                                                                           //
// Desc : Point / omni-directional light source classes.                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPOINTLIGHT_H_ )
#define _CGE_CGPOINTLIGHT_H_

//-----------------------------------------------------------------------------
// cgPointLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Lighting/cgShadowGenerator.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {633DA59A-FB2E-447E-B1D2-5EA1E910BE1F}
const cgUID RTID_PointLightNode   = { 0x633DA59A, 0xFB2E, 0x447E, { 0xB1, 0xD2, 0x5E, 0xA1, 0xE9, 0x10, 0xBE, 0x1F } };
// {766C9C18-B552-4F6D-84C5-9A1C5C67A32E}
const cgUID RTID_PointLightObject = { 0x766C9C18, 0xB552, 0x4F6D, { 0x84, 0xC5, 0x9A, 0x1C, 0x5C, 0x67, 0xA3, 0x2E } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgPointLightObject (Class)
// Desc : Point light source object.
//-----------------------------------------------------------------------------
class CGE_API cgPointLightObject : public cgLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPointLightObject, cgLightObject, "PointLightObject" )

public:
    //-------------------------------------------------------------------------
    // Public Structures, Typedefs and Enumerations
    //-------------------------------------------------------------------------
    enum ShadowMapProcess
    {
        NoProcessing    = 0,   // Make no attempt to repair shadow map borders
        CaptureBorder   = 1,   // Capture additional information around the border of the map to improve bilinear filtering.
        FullRepair      = 2    // Perform additional border duplication process to completely eliminate seams.
    
    }; // End Enum ShadowMapProcess

    enum ShadowCubeFace
    {
        ShadowFace_PositiveX = 0,
        ShadowFace_NegativeX = 1,
        ShadowFace_PositiveY = 2,
        ShadowFace_NegativeY = 3,
        ShadowFace_PositiveZ = 4,
        ShadowFace_NegativeZ = 5

    }; // End Enum ShadowCubeFace

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPointLightObject( cgUInt32 referenceId, cgWorld * world );
             cgPointLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgPointLightObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                            setOuterRange           ( cgFloat range );
    void                            setInnerRange           ( cgFloat range );
    void                            setRangeAdjust          ( cgFloat rangeAdjust );
    void                            setShadowUpdateRate     ( cgUInt32 rate );
    void                            setShadowMapEdgeProcess ( ShadowMapProcess process );
    cgFloat                         getOuterRange           ( ) const;
    cgFloat                         getInnerRange           ( ) const;
    cgFloat                         getRangeAdjust          ( ) const;
    cgUInt32                        getShadowUpdateRate     ( ) const;
    ShadowMapProcess                getShadowMapEdgeProcess ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual LightType               getLightType            ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    virtual void                    sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual void                    applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString                getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_PointLightObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat                 mOuterRange;              // Range of this light source outside of which no light is cast.
    cgFloat                 mInnerRange;              // Inner range of this light source inside of which light is full-bright.
    cgFloat                 mRangeAdjust;             // Range error adjustment for use when drawing low-res geometric light shape.
    ShadowMapProcess        mShadowMapEdgeProcess;     // process to undertake in order to "repair" shadow frustum / map edges.
    cgUInt32                mShadowUpdateRate;        // The minimum rate at which child shadow maps will update in frames per second.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertPointLight;
    static cgWorldQuery     mUpdateRanges;
    static cgWorldQuery     mUpdateShadowConfigSources;
    static cgWorldQuery     mUpdateShadowRate;
    static cgWorldQuery     mLoadPointLight;
};

//-----------------------------------------------------------------------------
// Name : cgPointLightNode (Class)
// Desc : Point light source object node.
//-----------------------------------------------------------------------------
class CGE_API cgPointLightNode : public cgLightNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPointLightNode, cgLightNode, "PointLightNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPointLightNode( cgUInt32 referenceId, cgScene * scene );
             cgPointLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgPointLightNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightNode)
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume          ( const cgBoundingBox & bounds );
    virtual bool                    pointInVolume           ( const cgVector3 & point, cgFloat errorRadius = 0.0f );
    virtual bool                    frustumInVolume         ( const cgFrustum & frustum );
    virtual bool                    testObjectShadowVolume  ( cgObjectNode * object, const cgFrustum & viewFrustum );
    
    virtual void                    computeShadowSets       ( cgCameraNode * camera );
    virtual void                    computeLevelOfDetail    ( cgCameraNode * camera );

    // Direct lighting
    virtual bool                    reassignShadowMaps      ( cgTexturePool * pool );
    virtual cgInt32                 beginShadowFill         ( cgTexturePool * pool );
    virtual bool                    beginShadowFillPass     ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endShadowFillPass       ( );
    virtual bool                    endShadowFill           ( );

    virtual cgInt32                 beginLighting           ( cgTexturePool * pool, bool applyShadows, bool deferred );
    virtual LightingOp              beginLightingPass       ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endLightingPass         ( );
    virtual bool                    endLighting             ( );

    // Indirect lighting
    virtual void					updateIndirectSettings       ( );
    virtual bool                    reassignIndirectMaps		 ( cgTexturePool * pool );
    virtual cgInt32                 beginIndirectFill			 ( cgTexturePool * pool );
    virtual bool                    beginIndirectFillPass		 ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectFillPass			 ( );
    virtual bool                    endIndirectFill				 ( );

	virtual cgInt32                 beginIndirectLighting        ( cgTexturePool * pool );
    virtual LightingOp              beginIndirectLightingPass    ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectLightingPass      ( );
    virtual bool                    endIndirectLighting          ( );

	virtual	void                    setIndirectLightingMethod    ( cgUInt32 method );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual bool                    canScale                ( ) const;
    virtual bool                    canRotate               ( ) const;
    virtual cgBoundingSphere        getBoundingSphere       ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onComponentModified     ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_PointLightNode; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setOuterRange( cgFloat range )
    {
        ((cgPointLightObject*)mReferencedObject)->setOuterRange( range );
    }
    inline void setInnerRange( cgFloat range )
    {
        ((cgPointLightObject*)mReferencedObject)->setInnerRange( range );
    }
    inline void setRangeAdjust( cgFloat adjust )
    {
        ((cgPointLightObject*)mReferencedObject)->setRangeAdjust( adjust );
    }
    inline void setShadowMapEdgeProcess( cgPointLightObject::ShadowMapProcess process )
    {
        ((cgPointLightObject*)mReferencedObject)->setShadowMapEdgeProcess( process );
    }
    inline void setShadowUpdateRate( cgUInt32 value )
    {
        ((cgPointLightObject*)mReferencedObject)->setShadowUpdateRate( value );
    }
    
    // Object Property 'Get' Routing
    inline cgFloat getOuterRange( ) const
    {
        return ((cgPointLightObject*)mReferencedObject)->getOuterRange( );
    }
    inline cgFloat getInnerRange( ) const
    {
        return ((cgPointLightObject*)mReferencedObject)->getInnerRange( );
    }
    inline cgFloat getRangeAdjust( ) const
    {
        return ((cgPointLightObject*)mReferencedObject)->getRangeAdjust( );
    }
    inline cgPointLightObject::ShadowMapProcess getShadowMapEdgeProcess( ) const
    {
        return ((cgPointLightObject*)mReferencedObject)->getShadowMapEdgeProcess( );
    }
    inline cgUInt32 getShadowUpdateRate( ) const
    {
        return ((cgPointLightObject*)mReferencedObject)->getShadowUpdateRate( );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual void                setClipPlanes           ( );
    virtual bool                updateLightConstants    ( );
    virtual bool                postCreate              ( );
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        renderFrustum           ( cgCameraNode * renderCamera, cgInt32 frustumIndex, cgShadowGenerator * shadowGenerator, bool deferred );
    void                        repairShadowBorders     ( bool frustumInvalidated[] );
    void                        getShadowFaceNeighbors  ( cgInt32 face, cgInt32 neighbours[] );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // Shadow maps.
    cgShadowGenerator     * mFrustums[6];               // The 6 shadow frustums we need for an omni-directional shadowing implementation.
    cgMeshHandle            mFrustumMesh;               // Custom shape mesh to use when using per-frustum rendering.
    cgFloat                 mShadowTimeSinceLast;       // Amount of time that has elapsed since the last shadow update.
    
    // Shadow map fill process tracking.
    bool                    mShadowFrustumUpdated[6];   // Records which shadow frustums were updated during the shadow map fill process.
    bool                    mShadowMapsUpdated;         // Were /any/ shadow maps updated?

    // Reflective shadow maps.
    cgReflectanceGenerator* mIndirectFrustums[6];       // The N reflective shadow frustums we need for an omni-directional shadowing implementation.
};

#endif // !_CGE_CGPOINTLIGHT_H_