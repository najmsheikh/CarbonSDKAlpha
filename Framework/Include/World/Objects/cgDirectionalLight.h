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
// Name : cgDirectionalLight.h                                               //
//                                                                           //
// Desc : Directional light source classes.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDIRECTIONALLIGHT_H_ )
#define _CGE_CGDIRECTIONALLIGHT_H_

//-----------------------------------------------------------------------------
// cgDirectionalLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Lighting/cgShadowGenerator.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {24075FC2-04B7-41A0-BAD1-41D2190E2E40}
const cgUID RTID_DirectionalLightNode   = { 0x24075FC2, 0x04B7, 0x41A0, { 0xBA, 0xD1, 0x41, 0xD2, 0x19, 0x0E, 0x2E, 0x40 } };
// {64D51571-FAE2-4FB1-A6CA-1EBFEDFDB724}
const cgUID RTID_DirectionalLightObject = { 0x64D51571, 0xFAE2, 0x4FB1, { 0xA6, 0xCA, 0x1E, 0xBF, 0xED, 0xFD, 0xB7, 0x24 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgDirectionalLightObject (Class)
// Desc : Directional light source object.
//-----------------------------------------------------------------------------
class CGE_API cgDirectionalLightObject : public cgLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDirectionalLightObject, cgLightObject, "DirectionalLightObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDirectionalLightObject( cgUInt32 referenceId, cgWorld * world );
             cgDirectionalLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgDirectionalLightObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUInt32                        getShadowUpdateRate     ( ) const;
    cgUInt32                        getShadowSplitCount     ( ) const;
    cgFloat                         getShadowSplitOverlap   ( ) const;
    cgFloat                         getShadowSplitDistance  ( cgUInt32 frustum ) const;
    void                            setShadowUpdateRate     ( cgUInt32 rate );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual LightType               getLightType            ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    virtual void                    sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
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
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_DirectionalLightObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat                     mSplitOverlapSize;      // Region of overlap between splits (optional for PSSM)
    cgFloatArray                mSplitDistances;        // The split plane distances that extend along the camera's frustum
    cgUInt32                    mShadowUpdateRate;      // The minimum rate at which child shadow maps will update in frames per second.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertDirectionalLight;
    static cgWorldQuery     mUpdateSplitOverlap;
    static cgWorldQuery     mUpdateShadowConfigSources; // ToDo: 6767 -- Don't need this any more or at least it needs to be adjusted for new data.
    static cgWorldQuery     mUpdateShadowRate;
    static cgWorldQuery     mLoadDirectionalLight;
};

//-----------------------------------------------------------------------------
// Name : cgDirectionalLightNode (Class)
// Desc : Directional light source object node.
//-----------------------------------------------------------------------------
class CGE_API cgDirectionalLightNode : public cgLightNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgDirectionalLightNode, cgLightNode, "DirectionalLightNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDirectionalLightNode( cgUInt32 referenceId, cgScene * scene );
             cgDirectionalLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgDirectionalLightNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume          ( const cgBoundingBox & bounds );
    virtual bool                    pointInVolume           ( const cgVector3 & point, cgFloat errorRadius = 0.0f );
    virtual bool                    frustumInVolume         ( const cgFrustum & frustum );
    virtual bool                    testObjectShadowVolume  ( cgObjectNode * object, const cgFrustum & viewFrustum );

    virtual void                    computeVisibility       ( );
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

	virtual cgInt32                 beginIndirectLighting		 ( cgTexturePool * pool );
    virtual LightingOp              beginIndirectLightingPass	 ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectLightingPass		 ( );
    virtual bool                    endIndirectLighting			 ( );

	virtual	void                    setIndirectLightingMethod    ( cgUInt32 method );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    registerVisibility          ( cgVisibilitySet * visibilityData, cgUInt32 flags );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onComponentModified         ( cgComponentModifiedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_DirectionalLightNode; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setShadowUpdateRate( cgUInt32 nValue )
    {
        ((cgDirectionalLightObject*)mReferencedObject)->setShadowUpdateRate( nValue );
    }
    
    // Object Property 'Get' Routing
    inline cgFloat getShadowSplitDistance( cgUInt32 frustum ) const
    {
        return ((cgDirectionalLightObject*)mReferencedObject)->getShadowSplitDistance( frustum );
    }
    inline cgFloat getShadowSplitOverlap( ) const
    {
        return ((cgDirectionalLightObject*)mReferencedObject)->getShadowSplitOverlap( );
    }
    inline cgUInt32 getShadowSplitCount( ) const
    {
        return ((cgDirectionalLightObject*)mReferencedObject)->getShadowSplitCount( );
    }
    inline cgUInt32 getShadowUpdateRate( ) const
    {
        return ((cgDirectionalLightObject*)mReferencedObject)->getShadowUpdateRate( );
    }
    /*inline const cgShadowGeneratorConfig & getShadowFrustumConfig( cgUInt32 frustum ) const
    {
        return ((cgDirectionalLightObject*)mReferencedObject)->getShadowFrustumConfig( frustum );
    }*/

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(cgShadowGenerator*, ShadowFrustumArray)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        beginRenderFrustum          ( cgInt32 frustumIndex, cgShadowGenerator * shadowGenerator, bool deferred, bool applyShadows );
    void                        endRenderFrustum            ( cgInt32 frustumIndex, cgShadowGenerator * shadowGenerator, bool deferred, bool applyShadows );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual bool                postCreate                  ( );
	virtual void                enableRenderOptimizations   ( ){};
	virtual void                disableRenderOptimizations  ( ){};
    virtual bool                buildStateBlocks            ( );
    virtual bool                updateLightConstants        ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    ShadowFrustumArray  mFrustums;                  // List of available shadow frustums for each PSSM split.
    ShadowFrustumArray  mIndirectFrustums;          // List of available reflective shadow frustums.
    cgFloat             m_fShadowTimeSinceLast;     // Amount of time that has elapsed since the last shadow update.
};

#endif // !_CGE_CGDIRECTIONALLIGHT_H_