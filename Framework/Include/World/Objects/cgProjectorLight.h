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
// Name : cgProjectorLight.h                                                 //
//                                                                           //
// Desc : Projector / area light source classes.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPROJECTORLIGHT_H_ )
#define _CGE_CGPROJECTORLIGHT_H_

//-----------------------------------------------------------------------------
// cgProjectorLight Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgLightObject.h>
#include <World/Lighting/cgShadowGenerator.h>
#include <Math/cgFrustum.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSampler;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {08E90B32-93D3-4755-AB0E-F65BAC69DC53}
const cgUID RTID_ProjectorLightNode = { 0x08e90b32, 0x93d3, 0x4755, { 0xab, 0x0e, 0xf6, 0x5b, 0xac, 0x69, 0xdc, 0x53 } };
// {D8C9F278-6183-43C9-B20B-0EA0BB1E274E}
const cgUID RTID_ProjectorLightObject = { 0xD8C9F278, 0x6183, 0x43c9, { 0xB2, 0x0B, 0x0E, 0xA0, 0xBB, 0x1E, 0x27, 0x4E } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgProjectorLightObject (Class)
/// <summary>
/// Projector light source object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProjectorLightObject : public cgLightObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgProjectorLightObject, cgLightObject, "ProjectorLightObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProjectorLightObject( cgUInt32 referenceId, cgWorld * world );
             cgProjectorLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgProjectorLightObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                            setSize                 ( cgFloat width, cgFloat height );
    void                            setSize                 ( const cgSizeF & size );
    void                            setOuterRange           ( cgFloat range );
    void                            setInnerRange           ( cgFloat range );
    void                            setFOV                  ( cgFloat fovU, cgFloat fovV );
    void                            setFOV                  ( const cgSizeF & value );
    void                            setTiling               ( cgFloat tileU, cgFloat tileV );
    void                            setTiling               ( const cgSizeF & tile );
    void                            setPlaybackRate         ( cgUInt32 rate );
    void                            setShadowUpdateRate     ( cgUInt32 rate );
    void                            setLightColorSampler    ( const cgTextureHandle & texture, const cgSamplerStateDesc & states );
    cgFloat                         getOuterRange           ( ) const;
    cgFloat                         getInnerRange           ( ) const;
    const cgSizeF                 & getSize                 ( ) const;
    const cgSizeF                 & getFarSize              ( ) const;
    const cgSizeF                 & getFOV                  ( ) const;
    const cgSizeF                 & getTiling               ( ) const;
    cgUInt32                        getPlaybackRate         ( ) const;
    cgUInt32                        getPlaybackFrames       ( ) const;
    cgUInt32                        getShadowUpdateRate     ( ) const;
    cgSampler                     * getLightColorSampler    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightObject)
    //-------------------------------------------------------------------------
    virtual LightType               getLightType            ( ) const;
    virtual bool                    beginLighting           ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox           getLocalBoundingBox     ( );
    virtual void                    sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
    virtual void                    applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual void                    onComponentDeleted      ( );
    virtual cgString                getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_ProjectorLightObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                            createLightColorSampler ( bool serialize );
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat                 mOuterRange;                // Range of this light source outside of which no light is cast.
    cgFloat                 mInnerRange;                // Inner range of this light source inside of which light is full-bright.
    cgSizeF                 mNearSize;                  // size of the near plane of the projector source (width / height).
    cgSizeF                 mFarSize;                   // size of the far plane of the projector source (width / height).
    cgSizeF                 mFOV;                       // Angle (in degrees) of the light's U & V axis value outside of which no light will be emitted.
    cgSizeF                 mTiling;                    // The amount to tile the projected light texture.
    cgUInt32                mShadowUpdateRate;          // The minimum rate at which child shadow maps will update in frames per second.
    cgUInt32                mProjectionPlaybackRate;    // The rate at which the projector color map animation should play back (if at all).
    cgUInt32                mProjectionTotalFrames;     // Total number of frames contained in the projector animated texture.
    
    // Rendering
    cgSampler             * mLightColorSampler;         // Sampler used to supply projected color texture as required.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertProjectorLight;
    static cgWorldQuery     mUpdateRanges;
    static cgWorldQuery     mUpdateSize;
    static cgWorldQuery     mUpdateTiling;
    static cgWorldQuery     mUpdateFOV;
    static cgWorldQuery     mUpdatePlaybackRate;
    static cgWorldQuery     mUpdateShadowRate;
    static cgWorldQuery     mUpdateColorSamplerId;
    static cgWorldQuery     mLoadProjectorLight;
};

//-----------------------------------------------------------------------------
// Name : cgProjectorLightNode (Class)
/// <summary>
/// An individual projector light source object node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgProjectorLightNode : public cgLightNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgProjectorLightNode, cgLightNode, "ProjectorLightNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgProjectorLightNode( cgUInt32 referenceId, cgScene * scene );
             cgProjectorLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgProjectorLightNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgFrustum               & getFrustum              ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgLightNode)
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume          ( const cgBoundingBox & bounds );
    virtual bool                    pointInVolume           ( const cgVector3 & point, cgFloat errorRadius = 0.0f );
    virtual bool                    frustumInVolume         ( const cgFrustum & frustum );
    virtual bool                    testObjectShadowVolume  ( cgObjectNode * object, const cgFrustum & viewFrustum );

    virtual void                    computeVisibility       ( );
    virtual void                    computeLevelOfDetail    ( cgCameraNode * camera );

    // Direct lighting
    virtual bool                    reassignShadowMaps      ( cgTexturePool * pool );
    virtual cgInt32                 beginShadowFill         ( cgTexturePool * pool  );
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
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponentEventListener)
    //-------------------------------------------------------------------------
    virtual void                    onComponentModified     ( cgComponentModifiedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_ProjectorLightNode; }
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
        ((cgProjectorLightObject*)mReferencedObject)->setOuterRange( range );
    }
    inline void setInnerRange( cgFloat range )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setInnerRange( range );
    }
    inline void setSize( cgFloat width, cgFloat height )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setSize( width, height );
    }
    inline void setSize( const cgSizeF & size )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setSize( size );
    }
    inline void setFOV( cgFloat fovU, cgFloat fovV )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setFOV( fovU, fovV );
    }
    inline void setFOV( const cgSizeF & value )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setFOV( value );
    }
    inline void setTiling( cgFloat tileU, cgFloat tileV )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setTiling( tileU, tileV );
    }
    inline void setTiling( const cgSizeF & value )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setTiling( value );
    }
    inline void setShadowUpdateRate( cgUInt32 rate )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setShadowUpdateRate( rate );
    }
    inline void setPlaybackRate( cgUInt32 rate )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setPlaybackRate( rate );
    }
    inline void setLightColorSampler( const cgTextureHandle & texture, const cgSamplerStateDesc & states )
    {
        ((cgProjectorLightObject*)mReferencedObject)->setLightColorSampler( texture, states );
    }
    
    // Object Property 'Get' Routing
    inline cgFloat getOuterRange( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getOuterRange( );
    }
    inline cgFloat getInnerRange( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getInnerRange( );
    }
    inline const cgSizeF & getSize( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getSize( );
    }
    inline const cgSizeF & getFarSize( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getFarSize( );
    }
    inline const cgSizeF & getFOV( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getFOV( );
    }
    inline const cgSizeF & getTiling( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getTiling( );
    }
    inline cgUInt32 getPlaybackRate( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getPlaybackRate( );
    }
    inline cgUInt32 getPlaybackFrames( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getPlaybackFrames( );
    }
    inline cgUInt32 getShadowUpdateRate( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getShadowUpdateRate( );
    }
    inline cgSampler * getLightColorSampler( ) const
    {
        return ((cgProjectorLightObject*)mReferencedObject)->getLightColorSampler( );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct _cbProjectorLight
    {
        cgVector3 textureData;    // xy = UV tiling, z = frame 
        cgVector4 coneAdjust;
    
    }; // End Struct : _cbProjectorLight

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                            createLightShape            ( );
    void                            updateFrustum               ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgLightNode)
    //-------------------------------------------------------------------------
   	virtual void                    setClipPlanes               ( );
    virtual bool                    updateSystemConstants       ( );
    virtual bool                    updateLightConstants        ( );
    virtual bool                    postCreate                  ( );
	
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgConstantBufferHandle      mProjectorLightConstants;   //< Custom projector lighting terms.
    cgShadowGenerator         * mShadowFrustum;             // The shadow frustums we need for a spot light shadowing implementation.
    cgFrustum                   mFrustum;                   // Details regarding the light frustum.
    cgFloat                     mShadowTimeSinceLast;       // Amount of time that has elapsed since the last shadow update.
    cgUInt32                    mProjectionCurrentFrame;    // The current frame of the projector animated texture to display.
    cgDouble                    mProjectionTimeLast;        // The previous time at which the projection animation was updated.
    cgReflectanceGenerator    * mIndirectFrustum;           // The reflective shadow frustum we need for dynamic ambient lighting
};

#endif // !_CGE_CGPROJECTORLIGHT_H_