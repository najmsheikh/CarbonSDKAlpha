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
// Name : cgParticleEmitterObject.h                                          //
//                                                                           //
// Desc : Classes responsible for creating, managing and rendering particles //
//        used for scene special effects.                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPARTICLEEMITTEROBJECT_H_ )
#define _CGE_CGPARTICLEEMITTEROBJECT_H_

//-----------------------------------------------------------------------------
// cgParticleEmitterObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>
#include <Rendering/cgParticleEmitter.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {89D6C65E-9C1E-4028-A9DA-22F2DA5D303F}
const cgUID RTID_ParticleEmitterObject = {0x89D6C65E, 0x9C1E, 0x4028, {0xA9, 0xDA, 0x22, 0xF2, 0xDA, 0x5D, 0x30, 0x3F}};
// {A8137D6E-AD08-4A74-801D-63F073627813}
const cgUID RTID_ParticleEmitterNode = {0xA8137D6E, 0xAD08, 0x4A74, {0x80, 0x1D, 0x63, 0xF0, 0x73, 0x62, 0x78, 0x13}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgParticleEmitterObject (Class)
/// <summary>
/// Main particle manager object from which particles are emitted.
/// </summary>
//-----------------------------------------------------------------------------
class cgParticleEmitterObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgParticleEmitterObject, cgWorldObject, "ParticleEmitterObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgParticleEmitterObject( cgUInt32 referenceId, cgWorld * world );
             cgParticleEmitterObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgParticleEmitterObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject              * allocateNew                 ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject              * allocateClone               ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUInt32                            getLayerCount               ( ) const;
    const cgParticleEmitterProperties & getLayerProperties          ( cgUInt32 layerIndex ) const;
    
    // Emitter properties
    cgFloat                             getInnerCone                ( cgUInt32 layerIndex ) const;
    cgFloat                             getOuterCone                ( cgUInt32 layerIndex ) const;
    cgFloat                             getEmissionRadius           ( cgUInt32 layerIndex ) const;
    cgFloat                             getDeadZoneRadius           ( cgUInt32 layerIndex ) const;
    cgUInt32                            getMaxSimultaneousParticles ( cgUInt32 layerIndex ) const;
    cgUInt32                            getMaximumParticles         ( cgUInt32 layerIndex ) const;
    cgFloat                             getBirthFrequency           ( cgUInt32 layerIndex ) const;
    cgFloat                             getHDRScale                 ( cgUInt32 layerIndex ) const;
    bool                                getSortedRender             ( cgUInt32 layerIndex ) const;
    bool                                getRandomizedRotation       ( cgUInt32 layerIndex ) const;
    bool                                getInitialEnabledState      ( cgUInt32 layerIndex ) const;
    bool                                getVelocityAlignment        ( cgUInt32 layerIndex ) const;
    cgFloat                             getVelocityScaleStrength    ( cgUInt32 layerIndex ) const;
    bool                                isGravityEnabled            ( cgUInt32 layerIndex ) const;
    void                                setInnerCone                ( cgUInt32 layerIndex, cgFloat degrees );
    void                                setOuterCone                ( cgUInt32 layerIndex, cgFloat degrees );
    void                                setEmissionRadius           ( cgUInt32 layerIndex, cgFloat radius );
    void                                setDeadZoneRadius           ( cgUInt32 layerIndex, cgFloat radius );
    void                                setMaxSimultaneousParticles ( cgUInt32 layerIndex, cgUInt32 amount );
    void                                setMaximumParticles         ( cgUInt32 layerIndex, cgUInt32 amount );
    void                                setBirthFrequency           ( cgUInt32 layerIndex, cgFloat amount );
    void                                setHDRScale                 ( cgUInt32 layerIndex, cgFloat scale );
    void                                enableSortedRender          ( cgUInt32 layerIndex, bool enabled );
    void                                enableVelocityAlignment     ( cgUInt32 layerIndex, bool enabled );
    void                                enableRandomizedRotation    ( cgUInt32 layerIndex, bool enabled );
    void                                enableGravity               ( cgUInt32 layerIndex, bool enabled );
    void                                setVelocityScaleStrength    ( cgUInt32 layerIndex, cgFloat strength );
    void                                setInitialEnabledState      ( cgUInt32 layerIndex, bool enabled );
    void                                setParticleTexture          ( cgUInt32 layerIndex, const cgString & textureFile );

    // Particle properties
    void                                setParticleSpeed            ( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum );
    void                                setParticleSpeed            ( cgUInt32 layerIndex, const cgRangeF & range );
    void                                setParticleMass             ( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum );
    void                                setParticleMass             ( cgUInt32 layerIndex, const cgRangeF & range );
    void                                setParticleAngularSpeed     ( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum );
    void                                setParticleAngularSpeed     ( cgUInt32 layerIndex, const cgRangeF & range );
    void                                setParticleBaseScale        ( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum );
    void                                setParticleBaseScale        ( cgUInt32 layerIndex, const cgRangeF & range );
    void                                setParticleLifetime         ( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum );
    void                                setParticleLifetime         ( cgUInt32 layerIndex, const cgRangeF & range );
    void                                setParticleSize             ( cgUInt32 layerIndex, cgFloat width, cgFloat height );
    void                                setParticleSize             ( cgUInt32 layerIndex, const cgSizeF & size );
    void                                setParticleAirResistance    ( cgUInt32 layerIndex, cgFloat value );
    void                                setParticleBlendMethod      ( cgUInt32 layerIndex, cgParticleBlendMethod::Base method );
    const cgRangeF                    & getParticleSpeed            ( cgUInt32 layerIndex ) const;
    const cgRangeF                    & getParticleMass             ( cgUInt32 layerIndex ) const;
    const cgRangeF                    & getParticleAngularSpeed     ( cgUInt32 layerIndex ) const;
    const cgRangeF                    & getParticleBaseScale        ( cgUInt32 layerIndex ) const;
    const cgRangeF                    & getParticleLifetime         ( cgUInt32 layerIndex ) const;
    const cgSizeF                     & getParticleSize             ( cgUInt32 layerIndex ) const;
    cgFloat                             getParticleAirResistance    ( cgUInt32 layerIndex ) const;
    cgParticleBlendMethod::Base         getParticleBlendMethod      ( cgUInt32 layerIndex ) const;
    const cgString                    & getParticleTexture          ( cgUInt32 layerIndex ) const;

    // Animation keys
    void                                setColorCurves              ( cgUInt32 layerIndex, const cgBezierSpline2 & r, const cgBezierSpline2 & g, const cgBezierSpline2 & b, const cgBezierSpline2 & a );
    void                                setScaleCurves              ( cgUInt32 layerIndex, const cgBezierSpline2 & x, const cgBezierSpline2 & y );
    void                                setColorRCurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    void                                setColorGCurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    void                                setColorBCurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    void                                setColorACurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    void                                setScaleXCurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    void                                setScaleYCurve              ( cgUInt32 layerIndex, const cgBezierSpline2 & curve );
    const cgBezierSpline2             & getColorRCurve              ( cgUInt32 layerIndex ) const;
    const cgBezierSpline2             & getColorGCurve              ( cgUInt32 layerIndex ) const;
    const cgBezierSpline2             & getColorBCurve              ( cgUInt32 layerIndex ) const;
    const cgBezierSpline2             & getColorACurve              ( cgUInt32 layerIndex ) const;
    const cgBezierSpline2             & getScaleXCurve              ( cgUInt32 layerIndex ) const;
    const cgBezierSpline2             & getScaleYCurve              ( cgUInt32 layerIndex ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    virtual bool                render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    virtual bool                renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const cgMaterialHandle & hMaterial );
    virtual void                applyObjectRescale      ( cgFloat scale );
    virtual bool                isRenderable            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ParticleEmitterObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct Layer
    {
        cgUInt32                    databaseId;
        cgParticleEmitterProperties properties;
        bool                        initialEmission;    // Initial enabled state.
        bool                        applyGravity;       // Apply scene gravity?
    };
    CGE_VECTOR_DECLARE( Layer, LayerArray );

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    LayerArray                  mLayers;        // Configured list of emitter layers.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertEmitter;
    static cgWorldQuery     mInsertEmitterLayer;
    static cgWorldQuery     mDeleteEmitterLayer;
    static cgWorldQuery     mUpdateConeAngles;
    static cgWorldQuery     mUpdateEmissionRadii;
    static cgWorldQuery     mUpdateParticleCounts;
    static cgWorldQuery     mUpdateReleaseProperties;
    static cgWorldQuery     mUpdateRenderingProperties;
    static cgWorldQuery     mUpdateParticleProperties;
    static cgWorldQuery     mUpdateColorKeys;
    static cgWorldQuery     mUpdateScaleKeys;
    static cgWorldQuery     mLoadEmitter;
    static cgWorldQuery     mLoadEmitterLayers;
};

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitterNode (Class)
/// <summary>
/// Custom node type for the particle emitter object. Manages additional 
/// properties that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgParticleEmitterNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgParticleEmitterNode, cgObjectNode, "ParticleEmitterNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgParticleEmitterNode( cgUInt32 referenceId, cgScene * scene );
             cgParticleEmitterNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgParticleEmitterNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgParticleEmitter         * getLayerEmitter         ( cgUInt32 layerIndex ) const;
    void                        enableLayerEmission     ( cgUInt32 layerIndex, bool enable );
    bool                        isLayerEmissionEnabled  ( cgUInt32 layerIndex ) const;
    bool                        particlesSpent          ( bool includeAlive ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                registerVisibility      ( cgVisibilitySet * visibilityData );
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual void                update                  ( cgFloat timeDelta );
    virtual bool                allowSandboxUpdate      ( ) const;

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setInitialEnabledState( cgUInt32 layerIndex, bool value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setInitialEnabledState( layerIndex, value );
    }
    inline void setInnerCone( cgUInt32 layerIndex, cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setInnerCone( layerIndex, value );
    }
    inline void setOuterCone( cgUInt32 layerIndex, cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setOuterCone( layerIndex, value );
    }
    inline void setEmissionRadius( cgUInt32 layerIndex, cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setEmissionRadius( layerIndex, value );
    }
    inline void setDeadZoneRadius( cgUInt32 layerIndex, cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setDeadZoneRadius( layerIndex, value );
    }
    inline void setMaxSimultaneousParticles( cgUInt32 layerIndex, cgUInt32 amount ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setMaxSimultaneousParticles( layerIndex, amount );
    }
    inline void setBirthFrequency( cgUInt32 layerIndex, cgFloat value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setBirthFrequency( layerIndex, value );
    }
    inline void setHDRScale( cgUInt32 layerIndex, cgFloat value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setHDRScale( layerIndex, value );
    }
    inline void enableRandomizedRotation( cgUInt32 layerIndex, bool value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableRandomizedRotation( layerIndex, value );
    }
    inline void enableSortedRender( cgUInt32 layerIndex, bool value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableSortedRender( layerIndex, value );
    }
    inline void setParticleSpeed( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSpeed( layerIndex, minimum, maximum );
    }
    inline void setParticleSpeed( cgUInt32 layerIndex, const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSpeed( layerIndex, range );
    }
    inline void setParticleMass( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleMass( layerIndex, minimum, maximum );
    }
    inline void setParticleMass( cgUInt32 layerIndex, const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleMass( layerIndex, range );
    }
    inline void setParticleAngularSpeed( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAngularSpeed( layerIndex, minimum, maximum );
    }
    inline void setParticleAngularSpeed( cgUInt32 layerIndex, const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAngularSpeed( layerIndex, range );
    }
    inline void setParticleBaseScale( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleBaseScale( layerIndex, minimum, maximum );
    }
    inline void setParticleBaseScale( cgUInt32 layerIndex, const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleBaseScale( layerIndex, range );
    }
    inline void setParticleLifetime( cgUInt32 layerIndex, cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleLifetime( layerIndex, minimum, maximum );
    }
    inline void setParticleLifetime( cgUInt32 layerIndex, const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleLifetime( layerIndex, range );
    }
    inline void setParticleSize( cgUInt32 layerIndex, cgFloat fWidth, cgFloat fHeight )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSize( layerIndex, fWidth, fHeight );
    }
    inline void setParticleSize( cgUInt32 layerIndex, const cgSizeF & size )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSize( layerIndex, size );
    }
    inline void setParticleAirResistance( cgUInt32 layerIndex, cgFloat value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAirResistance( layerIndex, value );
    }
    inline void setParticleBlendMethod( cgUInt32 layerIndex, cgParticleBlendMethod::Base value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleBlendMethod( layerIndex, value );
    }
    inline void setParticleTexture( cgUInt32 layerIndex, const cgString & textureFile )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleTexture( layerIndex, textureFile );
    }
    inline void setMaximumParticles( cgUInt32 layerIndex, cgUInt32 amount )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setMaximumParticles( layerIndex, amount );
    }
    inline void enableVelocityAlignment( cgUInt32 layerIndex, bool enable )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableVelocityAlignment( layerIndex, enable );
    }
    inline void enableGravity( cgUInt32 layerIndex, bool enable )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableGravity( layerIndex, enable );
    }
    inline void setVelocityScaleStrength( cgUInt32 layerIndex, cgFloat strength )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setVelocityScaleStrength( layerIndex, strength );
    }
    inline void setColorCurves( cgUInt32 layerIndex, const cgBezierSpline2 & r, const cgBezierSpline2 & g, const cgBezierSpline2 & b, const cgBezierSpline2 & a )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setColorCurves( layerIndex, r, g, b, a );
    }
    inline void setScaleCurves( cgUInt32 layerIndex, const cgBezierSpline2 & x, const cgBezierSpline2 & y )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setScaleCurves( layerIndex, x, y );
    }
    inline void setColorRCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setColorRCurve( layerIndex, curve );
    }
    inline void setColorGCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setColorGCurve( layerIndex, curve );
    }
    inline void setColorBCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setColorBCurve( layerIndex, curve );
    }
    inline void setColorACurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setColorACurve( layerIndex, curve );
    }
    inline void setScaleXCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setScaleXCurve( layerIndex, curve );
    }
    inline void setScaleYCurve( cgUInt32 layerIndex, const cgBezierSpline2 & curve )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setScaleYCurve( layerIndex, curve );
    }
    
    // Object Property 'Get' Routing
    inline cgUInt32 getLayerCount( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getLayerCount();
    }
    inline const cgParticleEmitterProperties & getLayerProperties( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getLayerProperties( layerIndex );
    }
    inline bool getInitialEnabledState( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getInitialEnabledState( layerIndex );
    }
    inline cgFloat getInnerCone( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getInnerCone( layerIndex );
    }
    inline cgFloat getOuterCone( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getOuterCone( layerIndex );
    }
    inline cgFloat getEmissionRadius( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getEmissionRadius( layerIndex );
    }
    inline cgFloat getDeadZoneRadius( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getDeadZoneRadius( layerIndex );
    }
    inline cgUInt32 getMaxSimultaneousParticles( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getMaxSimultaneousParticles( layerIndex );
    }
    inline cgFloat getBirthFrequency( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getBirthFrequency( layerIndex );
    }
    inline cgFloat getHDRScale( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getHDRScale( layerIndex );
    }
    inline bool getRandomizedRotation( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getRandomizedRotation( layerIndex );
    }
    inline bool getSortedRender( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getSortedRender( layerIndex );
    }
    inline const cgRangeF & getParticleSpeed( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleSpeed( layerIndex );
    }
    inline const cgRangeF & getParticleMass( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleMass( layerIndex );
    }
    inline const cgRangeF & getParticleAngularSpeed( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleAngularSpeed( layerIndex );
    }
    inline const cgRangeF & getParticleBaseScale( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleBaseScale( layerIndex );
    }
    inline const cgRangeF & getParticleLifetime( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleLifetime( layerIndex );
    }
    inline const cgSizeF & getParticleSize( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleSize( layerIndex );
    }
    inline cgFloat getParticleAirResistance( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleAirResistance( layerIndex );
    }
    inline cgParticleBlendMethod::Base getParticleBlendMethod( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleBlendMethod( layerIndex );
    }
    inline const cgString & getParticleTexture( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleTexture( layerIndex );
    }
    inline cgUInt32 getMaximumParticles( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getMaximumParticles( layerIndex );
    }
    inline bool getVelocityAlignment( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getVelocityAlignment( layerIndex );
    }
    inline bool isGravityEnabled( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->isGravityEnabled( layerIndex );
    }
    inline cgFloat getVelocityScaleStrength( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getVelocityScaleStrength( layerIndex );
    }
    inline const cgBezierSpline2 & getColorRCurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getColorRCurve( layerIndex );
    }
    inline const cgBezierSpline2 & getColorGCurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getColorGCurve( layerIndex );
    }
    inline const cgBezierSpline2 & getColorBCurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getColorBCurve( layerIndex );
    }
    inline const cgBezierSpline2 & getColorACurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getColorACurve( layerIndex );
    }
    inline const cgBezierSpline2 & getScaleXCurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getScaleXCurve( layerIndex );
    }
    inline const cgBezierSpline2 & getScaleYCurve( cgUInt32 layerIndex ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getScaleYCurve( layerIndex );
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ParticleEmitterNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( cgParticleEmitter*, EmitterArray );
    EmitterArray mEmitters;   // The array of underlying particle emitters / managers for this node (one per layer).
};

#endif // !_CGE_CGPARTICLEEMITTEROBJECT_H_