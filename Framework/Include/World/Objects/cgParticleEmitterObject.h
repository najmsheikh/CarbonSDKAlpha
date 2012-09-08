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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
    const cgParticleEmitterProperties & getProperties               ( ) const;
    const cgString                    & getScriptFile               ( ) const;

    // Emitter properties
    cgFloat                             getInnerCone                ( ) const;
    cgFloat                             getOuterCone                ( ) const;
    cgFloat                             getEmissionRadius           ( ) const;
    cgFloat                             getDeadZoneRadius           ( ) const;
    cgUInt32                            getMaxSimultaneousParticles ( ) const;
    cgFloat                             getBirthFrequency           ( ) const;
    bool                                getSortedRender             ( ) const;
    bool                                getRandomizedRotation       ( ) const;
    void                                setInnerCone                ( cgFloat degrees );
    void                                setOuterCone                ( cgFloat degrees );
    void                                setEmissionRadius           ( cgFloat radius );
    void                                setDeadZoneRadius           ( cgFloat radius );
    void                                setMaxSimultaneousParticles ( cgUInt32 amount );
    void                                setBirthFrequency           ( cgFloat amount );
    void                                enableSortedRender          ( bool enabled );
    void                                enableRandomizedRotation    ( bool enabled );

    // Particle properties
    void                                setParticleSpeed            ( cgFloat minimum, cgFloat maximum );
    void                                setParticleSpeed            ( const cgRangeF & range );
    void                                setParticleMass             ( cgFloat minimum, cgFloat maximum );
    void                                setParticleMass             ( const cgRangeF & range );
    void                                setParticleAngularSpeed     ( cgFloat minimum, cgFloat maximum );
    void                                setParticleAngularSpeed     ( const cgRangeF & range );
    void                                setParticleBaseScale        ( cgFloat minimum, cgFloat maximum );
    void                                setParticleBaseScale        ( const cgRangeF & range );
    void                                setParticleLifetime         ( cgFloat minimum, cgFloat maximum );
    void                                setParticleLifetime         ( const cgRangeF & range );
    void                                setParticleSize             ( cgFloat fWidth, cgFloat fHeight );
    void                                setParticleSize             ( const cgSizeF & size );
    void                                setParticleAirResistance    ( cgFloat value );
    const cgRangeF                    & getParticleSpeed            ( ) const;
    const cgRangeF                    & getParticleMass             ( ) const;
    const cgRangeF                    & getParticleAngularSpeed     ( ) const;
    const cgRangeF                    & getParticleBaseScale        ( ) const;
    const cgRangeF                    & getParticleLifetime         ( ) const;
    const cgSizeF                     & getParticleSize             ( ) const;
    cgFloat                             getParticleAirResistance    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
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
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString                    mScriptFile;    // Use script file to configure emitter.
    cgParticleEmitterProperties mProperties;    // Configured emitter properties.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertEmitter;
    static cgWorldQuery     mUpdateConeAngles;
    static cgWorldQuery     mUpdateEmissionRadii;
    static cgWorldQuery     mUpdateParticleCounts;
    static cgWorldQuery     mUpdateReleaseProperties;
    static cgWorldQuery     mUpdateRenderingProperties;
    static cgWorldQuery     mUpdateParticleProperties;
    static cgWorldQuery     mLoadEmitter;
};

//-----------------------------------------------------------------------------
//  Name : cgBoneNode (Class)
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
    cgParticleEmitter         * getEmitter              ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                registerVisibility      ( cgVisibilitySet * visibilityData, cgUInt32 flags );
    virtual bool                onNodeCreated           ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                onNodeLoading           ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual void                onComponentModified     ( cgComponentModifiedEventArgs * e );
    virtual void                update                  ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setInnerCone( cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setInnerCone( value );
    }
    inline void setOuterCone( cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setOuterCone( value );
    }
    inline void setEmissionRadius( cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setEmissionRadius( value );
    }
    inline void setDeadZoneRadius( cgFloat value ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setDeadZoneRadius( value );
    }
    inline void setMaxSimultaneousParticles( cgUInt32 amount ) 
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setMaxSimultaneousParticles( amount );
    }
    inline void setBirthFrequency( cgFloat value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setBirthFrequency( value );
    }
    inline void enableRandomizedRotation( bool value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableRandomizedRotation( value );
    }
    inline void enableSortedRender( bool value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->enableSortedRender( value );
    }
    inline void setParticleSpeed( cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSpeed( minimum, maximum );
    }
    inline void setParticleSpeed( const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSpeed( range );
    }
    inline void setParticleMass( cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleMass( minimum, maximum );
    }
    inline void setParticleMass( const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleMass( range );
    }
    inline void setParticleAngularSpeed( cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAngularSpeed( minimum, maximum );
    }
    inline void setParticleAngularSpeed( const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAngularSpeed( range );
    }
    inline void setParticleBaseScale( cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleBaseScale( minimum, maximum );
    }
    inline void setParticleBaseScale( const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleBaseScale( range );
    }
    inline void setParticleLifetime( cgFloat minimum, cgFloat maximum )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleLifetime( minimum, maximum );
    }
    inline void setParticleLifetime( const cgRangeF & range )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleLifetime( range );
    }
    inline void setParticleSize( cgFloat fWidth, cgFloat fHeight )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSize( fWidth, fHeight );
    }
    inline void setParticleSize( const cgSizeF & size )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleSize( size );
    }
    inline void setParticleAirResistance( cgFloat value )
    {
        cgAssert( mReferencedObject != CG_NULL );
        ((cgParticleEmitterObject*)mReferencedObject)->setParticleAirResistance( value );
    }
    
    // Object Property 'Get' Routing
    inline const cgParticleEmitterProperties & getProperties( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getProperties();
    }
    inline const cgString & getScriptFile( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getScriptFile();
    }
    inline cgFloat getInnerCone( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getInnerCone();
    }
    inline cgFloat getOuterCone( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getOuterCone();
    }
    inline cgFloat getEmissionRadius( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getEmissionRadius();
    }
    inline cgFloat getDeadZoneRadius( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getDeadZoneRadius();
    }
    inline cgUInt32 getMaxSimultaneousParticles( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getMaxSimultaneousParticles();
    }
    inline cgFloat getBirthFrequency( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getBirthFrequency();
    }
    inline bool getRandomizedRotation( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getRandomizedRotation();
    }
    inline bool getSortedRender( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getSortedRender();
    }
    inline const cgRangeF & getParticleSpeed( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleSpeed();
    }
    inline const cgRangeF & getParticleMass( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleMass();
    }
    inline const cgRangeF & getParticleAngularSpeed( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleAngularSpeed();
    }
    inline const cgRangeF & getParticleBaseScale( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleBaseScale();
    }
    inline const cgRangeF & getParticleLifetime( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleLifetime();
    }
    inline const cgSizeF & getParticleSize( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleSize();
    }
    inline cgFloat getParticleAirResistance( ) const
    {
        cgAssert( mReferencedObject != CG_NULL );
        return ((cgParticleEmitterObject*)mReferencedObject)->getParticleAirResistance();
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
    cgParticleEmitter * mEmitter;   // The underlying particle emitter / manager for this node.
};

#endif // !_CGE_CGPARTICLEEMITTEROBJECT_H_