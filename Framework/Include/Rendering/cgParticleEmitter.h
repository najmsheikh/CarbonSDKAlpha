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
// Name : cgParticleEmitter.h                                                //
//                                                                           //
// Desc : Classes responsible for world space particle billboard emission,   //
//        management and basic particle physics simulation.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPARTICLEEMITTER_H_ )
#define _CGE_CGPARTICLEEMITTER_H_

//-----------------------------------------------------------------------------
// cgParticleEmitter Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Rendering/cgBillboardBuffer.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgBezierSpline.h>

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgParticleBlendMethod
{
    enum Base
    {
        Additive    = 0,
        Linear      = 1,
        Screen      = 2
    };

} // End Namespace : cgParticleBlendMethod

namespace cgParticleEmitterType
{
    enum Base
    {
        Billboards      = 0,
        LocalBillboards = 1,
    };

} // End Namespace : cgParticleEmitterType

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
// Contains setup information for the emitter
struct cgParticleEmitterProperties
{
    // Emitter Properties
    cgParticleEmitterType::Base emitterType;
    cgFloat             innerCone;                  // The inner angle (in degrees) of the cone that will be used to emit particles.
    cgFloat             outerCone;                  // The outer angle (in degrees) of the cone that will be used to emit particles.
    cgFloat             emissionRadius;             // The radius around the emitter position from which particles can be released
    cgFloat             deadZoneRadius;             // The radius around the emitter position within which no particles will be released (must be smaller than fBaseRadius)
    cgUInt32            maxSimultaneousParticles;   // Maximum number of particles alive at any point in time. 0 = auto.
    cgUInt32            initialParticles;           // Number of particles to create before the emitter starts
    cgUInt32            maxFiredParticles;          // Maximum number of allowed particles over the emitter lifetime.
    cgFloat             birthFrequency;             // Number of particles to create every second
    cgString            particleTexture;            // The base texture (or atlas) file applied to billboards.
    cgString            particleShader;             // The surface shader source file to use for rendering billboards for this emitter.
    cgParticleBlendMethod::Base blendMethod;        // Method to use when blending particles to the frame buffer.

    bool                sortedRender;               // The particles should be rendered in a back to front order? (Warning: Performance Penalty)        
    cgVector3           emitterDirection;           // A fixed emitter direction (world space, not based on emitter Y axis) can be specified.
    bool                randomizeRotation;          // When enabled, each particle's rotation angle will be randomized upon its birth. Otherwise, all particles will default to a rotation angle of 0 degrees.
    cgUInt32            fireAmount;                 // Amount of particles to fire after delay... Replaces m_fFireFrequency
    cgFloat             fireDelay;                  // Time delay between particle firings
    cgFloat             fireDelayOffset;            // Amount to offset delay between particle firings
    
    // Particle Properties
    cgRangeF            speed;
    cgRangeF            angularSpeed;
    cgRangeF            mass;
    cgRangeF            baseScale;
    cgRangeF            lifetime;
    cgFloat             airResistance;
    cgSizeF             baseSize;
    cgFloat             hdrScale;
    cgFloat             velocityScaleStrength;
    bool                velocityAligned;            // Align the particles to their velocity vector.

    // Property Adjustment Keyframes
    cgBezierSpline2     scaleXCurve;
    cgBezierSpline2     scaleYCurve;
    cgBezierSpline2     colorRCurve;
    cgBezierSpline2     colorGCurve;
    cgBezierSpline2     colorBCurve;
    cgBezierSpline2     colorACurve;
};

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgParticle (Class)
/// <summary>
/// Class for the management of an individual particle.
/// Note : Derived from cgBillboard which provides our rendering functionality
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgParticle : public cgBillboard3D
{
public:
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgParticleEmitter;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgParticle( );
    virtual ~cgParticle( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void        setVelocity         ( const cgVector3 & velocity );
    void        setAngularVelocity  ( cgFloat angularVelocity );
    void        setMaximumAge       ( cgFloat maximumAge );
    void        setMass             ( cgFloat mass );
    cgFloat     getMass             ( ) const { return mMass; }
    bool        isAlive             ( ) const { return (mAge <= mMaximumAge); }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgFloat   mMass;              // The mass of this particle.
    cgVector3 mVelocity;          // Particle's current velocity
    cgFloat   mAngularVelocity;   // Velocity of particle rotation (degrees/sec)
    cgFloat   mAge;               // Age of the particle
    cgFloat   mMaximumAge;        // Maximum age of the particle before it dies
    // ToDo: 6767 -- Are these necessary any more?
    cgUInt16  mLastColorKeyIndex; // Cached index for color keyframes
    cgUInt16  mLastScaleKeyIndex; // Cached index for scale keyframes
};

//-----------------------------------------------------------------------------
//  Name : cgParticleEmitter (Class)
/// <summary>
/// A high level object responsible for building billboards and updating
/// their properties based on a configurable particle simulation.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgParticleEmitter : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgParticleEmitter, "ParticleEmitter" )

public:
    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgParticleEmitter( );
	virtual ~cgParticleEmitter( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool                                initialize              ( cgInputStream scriptStream, const cgParticleEmitterProperties & properties, cgRenderDriver * driver );
    bool                                setEmitterProperties    ( const cgParticleEmitterProperties & properties );
    const cgParticleEmitterProperties & getEmitterProperties    ( ) const;
    void                                update                  ( cgFloat timeDelta, const cgVector3 & worldVelocity, bool velocityScale );
    void                                render                  ( cgCameraNode * camera );
    void                                setEmitterMatrix        ( const cgMatrix & matrix );
    const cgMatrix                    & getEmitterMatrix        ( ) const;
    const cgVector3                   & getEmitterPosition      ( ) const;
    const cgVector3                   & getEmitterDirection     ( ) const;
    cgVector3                           generateConeDirection   ( cgFloat innerAngle, cgFloat outerAngle ) const;
    cgVector3                           generateOrigin          ( cgFloat deadZoneRadius, cgFloat baseRadius ) const;
    bool                                particlesSpent          ( bool includeAlive = true );
    void                                setGravity              ( const cgVector3 & gravity );
    void                                setGlobalForce          ( const cgVector3 & force );
    void                                enableEmission          ( bool enable );
    bool                                isEmissionEnabled       ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                 ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
	// Private Methods
	//-------------------------------------------------------------------------
    bool                onParticleBirth         ( cgParticle * particle );

    //-------------------------------------------------------------------------
	// Private Variables
	//-------------------------------------------------------------------------
    bool                        mEnabled;               // Is emission currently enabled.
    cgString                    mScriptFile;            // The name of the particle script file.
    cgRenderDriver            * mRenderDriver;          // render used for creating billboard buffers and rendering.
    cgParticleEmitterProperties mProperties;            // Setup information for the emitter (birth rate etc.)
    cgParticle               ** mParticles;             // Array containing all managed particles.
    cgParticle               ** mActiveParticles;       // Array containing currently active particles.
    cgParticle               ** mDeadParticles;         // Array containing inactive particles.
    cgUInt32                    mMaxParticles;          // Final computed maximum number of simultaneous particles that can be active.
    cgUInt32                    mActiveParticleCount;   // Number of currently active particles
    cgFloat                     mReleaseCount;          // Keeps track of the amount of particles to release over time
    cgUInt32                    mTotalReleased;         // Total number of particles released so far
    cgFloat                     mTimeElapsed;           // Time elapsed in fire delay mode.
    cgBillboardBuffer         * mBillboardBuffer;       // The billboard buffer controlling management, construction and rendering of particle billboards.
    cgMatrix                    mTransform;             // The emitter matrix, describes the current position and orientation of the emitter.
    cgVector3                   mGravity;               // Gravity to apply to the particles as required.
    cgVector3                   mGlobalForce;           // Any external/global forces applied to the particles (wind etc).
};

#endif // !_CGE_CGPARTICLEEMITTER_H_