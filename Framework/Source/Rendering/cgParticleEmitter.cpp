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
// Name : cgParticleEmitter.cpp                                              //
//                                                                           //
// Desc : Classes responsible for world space particle billboard emission,   //
//        management and basic particle physics simulation.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgParticleEmitter Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgParticleEmitter.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgSurfaceShader.h>
#include <Math/cgMathUtility.h>
#include <System/cgStringUtility.h>

///////////////////////////////////////////////////////////////////////////////
// cgParticle Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgParticle () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticle::cgParticle( )
{
    // Initialize members with sensible defaults
    mVelocity           = cgVector3( 0.0f, 0.0f, 0.0f );
    mAngularVelocity      = 0.0f;
    mAge                  = 0.0f;
    mMaximumAge           = 0.0f;
    mLastColorKeyIndex    = 0;
    mLastScaleKeyIndex    = 0;

    // Unlike billboards, particles should not be visible by default
    setVisible( false );
}

//-----------------------------------------------------------------------------
//  Name : ~cgParticle () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgParticle::~cgParticle()
{
}

//-----------------------------------------------------------------------------
//  Name : setVelocity ()
/// <summary>
/// Set the velocity of the particle
/// </summary>
//-----------------------------------------------------------------------------
void cgParticle::setVelocity( const cgVector3 & vecVelocity )
{
    mVelocity = vecVelocity;
}

//-----------------------------------------------------------------------------
//  Name : setAngularVelocity ()
/// <summary>
/// Set the angular / rotation velocity of the particle in degrees per
/// second.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticle::setAngularVelocity( cgFloat fAngularVelocity )
{
    mAngularVelocity = fAngularVelocity;
}

//-----------------------------------------------------------------------------
//  Name : setMaximumAge ()
/// <summary>
/// Set the life span of the particle
/// </summary>
//-----------------------------------------------------------------------------
void cgParticle::setMaximumAge( cgFloat fMaximumAge )
{
    mMaximumAge = fMaximumAge;
}

//-----------------------------------------------------------------------------
//  Name : setMass ()
/// <summary>
/// Set the mass of the particle
/// </summary>
//-----------------------------------------------------------------------------
void cgParticle::setMass( cgFloat fMass )
{
    mMass = fMass;
}

///////////////////////////////////////////////////////////////////////////////
// cgParticleEmitter Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgParticleEmitter () (Constructor)
/// <summary>
/// cgParticleEmitter Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitter::cgParticleEmitter()
{
    // Set variables to sensible defaults
    mRenderDriver        = CG_NULL;
    mParticles           = CG_NULL;
    mActiveParticles     = CG_NULL;
    mDeadParticles       = CG_NULL;
    mActiveParticleCount = 0;
    mReleaseCount        = 0;
    mBillboardBuffer     = CG_NULL;
    mGravity             = cgVector3( 0.0f, 0.0f, 0.0f );
    mGlobalForce         = cgVector3( 0.0f, 0.0f, 0.0f );
    mTotalReleased       = 0;
    mEnabled             = true;
    
    // Clear necessary structure elements
    cgMatrix::identity( mTransform );
    mProperties.colorRCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorGCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorBCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorACurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleXCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleYCurve.setDescription( cgBezierSpline2::Maximum );
}

//-----------------------------------------------------------------------------
//  Name : ~cgParticleEmitter () (Destructor)
/// <summary>
/// cgParticleEmitter Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgParticleEmitter::~cgParticleEmitter()
{
    // Release allocated memory.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any resources allocated by this object.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::dispose( bool bDisposeBase )
{
    // Release allocated memory
    if ( mParticles != CG_NULL )
        delete []mParticles;
    if ( mActiveParticles != CG_NULL )
        delete []mActiveParticles;
    if ( mDeadParticles != CG_NULL )
        delete []mDeadParticles;
    if ( mBillboardBuffer )
        delete mBillboardBuffer;

    // Clear variables
    mScriptFile.clear();
    mRenderDriver        = CG_NULL;
    mParticles           = CG_NULL;
    mActiveParticles     = CG_NULL;
    mDeadParticles       = CG_NULL;
    mActiveParticleCount = 0;
    mBillboardBuffer     = CG_NULL;
    mEnabled             = true;
    
    // Clear structures
    mProperties.colorRCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorGCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorBCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorACurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleXCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleYCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.particleShader.clear();
    mProperties.particleTexture.clear();
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the particle emitter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitter::initialize( cgInputStream ScriptStream, const cgParticleEmitterProperties & Properties, cgRenderDriver * pDriver )
{
    // If we are already initialized, we must bail.
    if ( mRenderDriver != CG_NULL )
        return true;

    // Store important values
    //mScriptFile = cgFileSystem::ResolveFileLocation( strScript );
    mProperties    = Properties;
    mRenderDriver = pDriver;

    // Automatically compute the maximum number of simultaneous particles
    // if requested.
    mMaxParticles = mProperties.maxSimultaneousParticles;
    if ( mMaxParticles == 0 )
    {
        mMaxParticles = (cgUInt32)ceilf(mProperties.birthFrequency * mProperties.lifetime.max);
        if ( mProperties.maxFiredParticles > 0 )
            mMaxParticles = min( mProperties.maxFiredParticles, mMaxParticles );
        if ( mMaxParticles == 0 )
            mMaxParticles = 1;
        if ( mMaxParticles > 200000 )
            mMaxParticles = 200000;
    
    } // End if auto compute

    // Validate properties
    if ( !mMaxParticles )
    {
        // Particle script specified invalid property values
        cgAppLog::write( cgAppLog::Error, _T("Maximum number of simulatenous particles must be greater than zero. Failed to initialize particle emitter.\n") );
        dispose(false);
        return false;
    
    } // End if invalid max
    if ( mMaxParticles > 200000 )
    {
        cgAppLog::write( cgAppLog::Error, _T("Maximum number of simulatenous particles must be less than or equal to 200,000. Failed to update particle emitter properties.\n") );
        dispose(false);
        return false;
    
    } // End if invalid max

    // For non-frequency mode, initial elapsed time should be set to the required offset to allow correct timing
    mTimeElapsed = mProperties.fireDelayOffset;

    // Allocate billboard items.
    mBillboardBuffer = new cgBillboardBuffer();
    mParticles       = new cgParticle*[ mMaxParticles ];
    mActiveParticles = new cgParticle*[ mMaxParticles ];
    mDeadParticles   = new cgParticle*[ mMaxParticles ];

    // Does the billboard buffer need to support sorting?
    cgUInt32 nFlags = 0;
    if ( mProperties.sortedRender )
        nFlags |= cgBillboardBuffer::SupportSorting;
    if ( mProperties.velocityAligned )
        nFlags |= cgBillboardBuffer::OrientationAxis;

    // Prepare the billboard buffer
    cgString strShader = mProperties.particleShader;
    if ( strShader.empty() )
        strShader = _T("sys://Shaders/Particles.sh");
    if ( mProperties.particleTexture.endsWith( _T(".xml") ) )
    {
        if ( !mBillboardBuffer->prepareBufferFromAtlas( nFlags, mRenderDriver, mProperties.particleTexture, strShader ) )
        {
            dispose(false);
            return false;
        
        } // End if failed
    
    } // End if atlas
    else
    {
        if ( !mBillboardBuffer->prepareBuffer( nFlags, mRenderDriver, mProperties.particleTexture, strShader ) )
        {
            dispose(false);
            return false;
        
        } // End if failed

    } // End if !atlas

    // Set the blending method to the billboard buffer.
    cgSurfaceShaderHandle hShader = mBillboardBuffer->getSurfaceShader();
    if ( hShader.isValid() )
        hShader->setInt( _T("blendMethod"), (cgInt32)Properties.blendMethod );

    // Allocate and add all billboards to the buffer
    for ( cgUInt32 i = 0; i < mMaxParticles; ++i )
    {
        mParticles[i] = new cgParticle();
        mBillboardBuffer->addBillboard( mParticles[i] );
        mParticles[i]->update();

        // All particles are "dead" to begin with
        mDeadParticles[i] = mParticles[i];

        // Force the particle's age to be greater than its maximum age 
        // so that anyone who calls 'isAlive()' on the particle after this
        // point will know that it's dead.
        mParticles[i]->mAge = mParticles[i]->mMaximumAge + 1;
        
    } // Next particle

    // TODO: Initial Particle Release

    // Finish building
    return mBillboardBuffer->endPrepare();
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Update this particle emitter
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::update( cgFloat fTimeElapsed, const cgVector3 & vecWorldVelocity, bool bVelocityScale )
{
    cgInt32     i, nCount       = 0;
    cgParticle* pParticle       = CG_NULL;
    bool        bActiveParticle = false;
    cgFloat     fParticleAge, fSpeed, fVelocity;
    cgVector3   vecTractive, vecDrag, vecAccel, vecAxis;

    // Not initialized yet?
    if ( !mBillboardBuffer )
        return;

    // Loop through all active particles
    for ( i = 0; i < (signed)mActiveParticleCount; ++i )
    {
        // Retrieve particle
        pParticle = mActiveParticles[i];

        // Update particle age
        pParticle->mAge += fTimeElapsed;

        // Compute the age of the particle as a scalar value between 0 and 1
        fParticleAge = (pParticle->mAge / pParticle->mMaximumAge);

        // Is this particle past it's lifespan?
        if ( !pParticle->isAlive() )
        {
            // Flag as dead, and remove from active particles list
            pParticle->setVisible( false );
            if ( i < (signed)mActiveParticleCount - 1 )
                memmove( &mActiveParticles[i], &mActiveParticles[i+1], ((mActiveParticleCount - 1) - i) * sizeof(cgParticle*) );
            mActiveParticleCount--;

            // Add to the end of the dead particles list
            mDeadParticles[ (mMaxParticles - 1) - mActiveParticleCount ] = pParticle;

            // Update to allow it to be removed from consideration
            pParticle->update();

            // This particle was removed, re-test this slot in the active particle list
            i--;

            // Skip rest of calculation
            continue;

        } // End if kill particle

        // Update velocity if particle has a mass
        if ( pParticle->mMass > 0.0f )
        {
            fSpeed      = cgVector3::length( pParticle->mVelocity );
            vecTractive = (mGlobalForce + (mGravity * pParticle->getMass()));
            vecDrag     = -mProperties.airResistance * (pParticle->mVelocity * fSpeed);
            vecAccel    = (vecTractive + vecDrag) / pParticle->mMass;
            pParticle->mVelocity += vecAccel * fTimeElapsed;

        } // End if has mass
        
        // Update angles and positions
        pParticle->setPosition( pParticle->getPosition() + (pParticle->mVelocity * fTimeElapsed) );
        pParticle->mRotation += pParticle->mAngularVelocity * fTimeElapsed;

        // Update the optional direction vector that can be used to lock 
        // the billboard to a specific axis. This vector is based on the
        // specified world velocity (i.e. camera moving through the world)
        // relative to the particle's position and velocity.
        vecAxis = pParticle->mVelocity - vecWorldVelocity;
        fVelocity = cgVector3::length( vecAxis );
        if ( fVelocity > 0 )
        {
            vecAxis /= fVelocity;
            pParticle->setDirection( vecAxis );
        
        } // End if

        // ******************************************************************
        // * COLOR CURVES
        // ******************************************************************
        const bool bApproximateSample = true;
        cgColorValue NewColor;
        NewColor.r = mProperties.colorRCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.g = mProperties.colorGCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.b = mProperties.colorBCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.a = mProperties.colorACurve.evaluateForX( fParticleAge, bApproximateSample );
        pParticle->setColor( NewColor );
        pParticle->setHDRScale( mProperties.hdrScale );

        // ******************************************************************
        // * SCALE CURVES
        // ******************************************************************
        pParticle->mScale.x = mProperties.scaleXCurve.evaluateForX( fParticleAge, bApproximateSample );
        pParticle->mScale.y = mProperties.scaleYCurve.evaluateForX( fParticleAge, bApproximateSample );

        // Multiply scale by velocity if requested
        if ( bVelocityScale )
            pParticle->mScale.y *= 1.0f + (fVelocity - 1.0f) * mProperties.velocityScaleStrength;

        // Update the underlying billboard
        pParticle->update();

    } // Next Active Particle

    // Have we RELEASED all of the required particles (not including those still active)?
    if ( particlesSpent( false ) )
        return;

    // Is emission enabled?
    if ( mEnabled )
    {
        // Determine how many new particles to release in this frame
        if ( mProperties.fireAmount == 0 )
        {
            // Calculate how many new particles to fire based on frequency.
            mReleaseCount += (mProperties.birthFrequency * fTimeElapsed);
            nCount = (cgInt32)mReleaseCount;
            mReleaseCount -= (cgFloat)nCount;

        } // End if frequency mode
        else
        {
            // Calculate how many particles to fire based on fire delay
            nCount = 0;
            mTimeElapsed += fTimeElapsed;
            if ( mTimeElapsed >= mProperties.fireDelay )
            {
                nCount = mProperties.fireAmount;
                mTimeElapsed -= mProperties.fireDelay;
            
            } // End if we've reached our release delay
        
        } // End if fire delay mode

        // Fire the specified number of particles
        for ( i = 0; i < nCount; ++i )
        {
            // No spare particles?
            if ( mActiveParticleCount == mMaxParticles )
            {
                // TODO : Find the OLDEST particle

                // Pull one from the beginning of the active list (/generally/ these are the oldest particles)
                bActiveParticle = true;
                pParticle       = mActiveParticles[0];
            
            } // End if using active particle
            else
            {
                // Pull one from the end of the dead list
                bActiveParticle = false;
                pParticle       = mDeadParticles[ (mMaxParticles - 1) - mActiveParticleCount ];

            } // End if using dead particle

            // Reset this particle
            cgFloat fBaseScale = cgMathUtility::randomFloat( mProperties.baseScale.min, mProperties.baseScale.max );
            pParticle->setSize( mProperties.baseSize.width * fBaseScale, mProperties.baseSize.height * fBaseScale );
            pParticle->setPosition( 0, 0, 0 );
            pParticle->mAge               = 0.0f;
            pParticle->mRotation          = 0.0f;
            pParticle->mAngularVelocity   = 0.0f;
            pParticle->mVelocity        = cgVector3( 0, 0, 0 );
            pParticle->mLastColorKeyIndex = 0;
            pParticle->mLastScaleKeyIndex = 0;

            // Calculate initial scale
            pParticle->setScale( mProperties.scaleXCurve.evaluateForX(0,true), mProperties.scaleXCurve.evaluateForX(0,true) );
            
            // Calculate initial color
            cgColorValue NewColor;
            NewColor.r = mProperties.colorRCurve.evaluateForX( 0, true );
            NewColor.g = mProperties.colorGCurve.evaluateForX( 0, true );
            NewColor.b = mProperties.colorBCurve.evaluateForX( 0, true );
            NewColor.a = mProperties.colorACurve.evaluateForX( 0, true );
            pParticle->setColor( NewColor );
            pParticle->setHDRScale( mProperties.hdrScale );

            // Trigger the birth event
            if ( onParticleBirth( pParticle ) == false )
            {
                if ( bActiveParticle == true )
                {
                    // Flag as dead, and remove from active particles list
                    pParticle->setVisible( false );
                    if ( i < (signed)mActiveParticleCount - 1 )
                        memmove( &mActiveParticles[i], &mActiveParticles[i+1], ((mActiveParticleCount - 1) - i) * sizeof(cgParticle*) );
                    mActiveParticleCount--;

                    // Add to the end of the dead particles list
                    mDeadParticles[ (mMaxParticles - 1) - mActiveParticleCount ] = pParticle;

                    // Update to allow it to be removed from consideration
                    pParticle->update();

                } // End if particle was an active one

                // Force the particle's age to be greater than its maximum age 
                // so that anyone who calls 'isAlive()' on the particle after this
                // point will know that it's dead.
                pParticle->mAge = pParticle->mMaximumAge + 1;

                // Skip the creation
                continue;

            } // End if cancelled the creation

            // A new particle was released
            mTotalReleased++;
            
            // Update the optional direction vector that can be used to lock 
            // the billboard to a specific axis. This vector is based on the
            // specified world velocity (i.e. camera moving through the world)
            // relative to the particle's position and velocity.
            vecAxis = pParticle->mVelocity - vecWorldVelocity;
            fVelocity = cgVector3::length( vecAxis );
            if ( fVelocity > 0 )
            {
                vecAxis /= fVelocity;
                pParticle->setDirection( vecAxis );
            
            } // End if

            // Multiply scale by velocity if requested
            if ( bVelocityScale )
                pParticle->mScale.y *= 1.0f + (fVelocity - 1.0f) * mProperties.velocityScaleStrength;
            
            // Set as visible and add to active list if it wasn't already
            if ( bActiveParticle == false )
            {
                pParticle->setVisible( true );
                mActiveParticles[ mActiveParticleCount++ ] = pParticle;
            
            } // End if not from active list

            // Update the newly created particle
            pParticle->update();
        
        } // Next new particle

    } // End if emission enabled
}

//-----------------------------------------------------------------------------
//  Name : enableEmission ()
/// <summary>
/// Enable or disable the emission of new particles. Any existing particles
/// that are alive will still be updated until the end of their lifetime.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::enableEmission( bool enable )
{
    mEnabled = enable;
}

//-----------------------------------------------------------------------------
//  Name : isEmissionEnabled ()
/// <summary>
/// Determine if the emission of new particles is currently enabled.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitter::isEmissionEnabled( ) const
{
    return mEnabled;
}

//-----------------------------------------------------------------------------
//  Name : render ()
/// <summary>
/// Render all of the currently active particles for this emitter.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::render( cgCameraNode * pCamera )
{
    // Validate requirements
    if ( !mBillboardBuffer )
        return;

    // Ensure we use the correct world transform dependant
    // on the type of emitter.
    if ( mProperties.emitterType == cgParticleEmitterType::Billboards )
        mRenderDriver->setWorldTransform( CG_NULL );
    else if ( mProperties.emitterType == cgParticleEmitterType::LocalBillboards )
        mRenderDriver->setWorldTransform( &mTransform );
    
    // Render the entire billboard buffer.
    if ( !mProperties.sortedRender )
        mBillboardBuffer->render( );
    else
    {
        cgMatrix * transformMatrix = CG_NULL;
        if ( mProperties.emitterType == cgParticleEmitterType::LocalBillboards )
            transformMatrix = &mTransform;
        mBillboardBuffer->renderSorted( pCamera, transformMatrix );
    
    } // End if sorted
}

//-----------------------------------------------------------------------------
//  Name : setEmitterMatrix ()
/// <summary>
/// Update the emitter's internal world matrix that describes the current
/// position and orientation of the emitter.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::setEmitterMatrix( const cgMatrix & mtxEmitter )
{
    mTransform = mtxEmitter;
}

//-----------------------------------------------------------------------------
//  Name : getEmitterMatrix ()
/// <summary>
/// Retrieve the emitter's internal world matrix that describes the
/// current position and orientation of the emitter.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgParticleEmitter::getEmitterMatrix( ) const
{
    return mTransform;
}

//-----------------------------------------------------------------------------
//  Name : getEmitterPosition ()
/// <summary>
/// Utility function to quickly extract the position of the emitter from
/// it's matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgParticleEmitter::getEmitterPosition( ) const
{
    return (cgVector3&)mTransform._41;
}

//-----------------------------------------------------------------------------
//  Name : getEmitterDirection ()
/// <summary>
/// Utility function to quickly extract the direction of the emitter from
/// it's matrix.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgParticleEmitter::getEmitterDirection( ) const
{
    return (cgVector3&)mTransform._31;
}

//-----------------------------------------------------------------------------
//  Name : generateConeDirection ()
/// <summary>
/// Utility function for generating a random direction vector within
/// a given cone inner / outer angle in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgParticleEmitter::generateConeDirection( cgFloat fInnerAngle, cgFloat fOuterAngle ) const
{
    const bool localSpace  = (mProperties.emitterType == cgParticleEmitterType::LocalBillboards);
    cgVector3 vecUp        = (localSpace) ? cgVector3(0,1,0) : (cgVector3&)mTransform._21;
    cgVector3 vecRight     = (localSpace) ? cgVector3(1,0,0) : (cgVector3&)mTransform._11;
    cgVector3 vecLook      = (localSpace) ? cgVector3(0,0,1) : (cgVector3&)mTransform._31;
    cgVector3 vecDirection = vecLook;

    // A fixed emitter direction supplied?
    if ( cgVector3::lengthSq( mProperties.emitterDirection ) > 0.99f )
    {
        vecUp        = mProperties.emitterDirection;
        vecDirection = mProperties.emitterDirection;
        if ( fabsf(cgVector3::dot( vecLook, vecUp )) > 0.9f )
            vecLook = vecRight;
        
        // We only actually need the right vector so just compute
        // that; no need to recompute correct orthogonal look.
        cgVector3::cross( vecRight, vecUp, vecLook );
        cgVector3::normalize( vecRight, vecRight );

    } // End if fixed emitter dir

    // Angles express the entire range of the cone, but we need half that.
    fInnerAngle *= 0.5f;
    fOuterAngle *= 0.5f;

    // Generate Azimuth / Polar angles
    cgFloat fAzimuth = (cgFloat)rand() / (cgFloat)RAND_MAX;
    cgFloat fPolar   = (cgFloat)rand() / (cgFloat)RAND_MAX;
    fAzimuth = fInnerAngle + ((fOuterAngle - fInnerAngle) * fAzimuth);
    fPolar   = 360.0f * fPolar;

    // Generate 
    cgMatrix mtxRot;
    cgMatrix::rotationAxis( mtxRot, vecRight, CGEToRadian( fAzimuth ) );
    cgVector3::transformNormal( vecDirection, vecDirection, mtxRot );
    cgMatrix::rotationAxis( mtxRot, vecLook, CGEToRadian( fPolar ) );
    cgVector3::transformNormal( vecDirection, vecDirection, mtxRot );
    
    // Return the direction
    return vecDirection;
}

//-----------------------------------------------------------------------------
//  Name : generateOrigin () (Private)
/// <summary>
/// Generate a particle's origin of emission.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgParticleEmitter::generateOrigin( cgFloat fDeadZoneRadius, cgFloat fEmissionRadius ) const
{
    const bool localSpace = (mProperties.emitterType == cgParticleEmitterType::LocalBillboards);

    // If we're not using an emission radius, simply return the emitter's local / world space pos
    if ( fEmissionRadius < CGE_EPSILON_1MM )
        return (localSpace) ? cgVector3(0,0,0) : getEmitterPosition();

    // Pick a random point along the emitters radius
    cgFloat fLength = fDeadZoneRadius + (((cgFloat)rand() / (cgFloat)RAND_MAX) * (fEmissionRadius - fDeadZoneRadius));

    // A Polar angle
    cgFloat fPolar  = ((cgFloat)rand() / (cgFloat)RAND_MAX) * 360.0f;

    // Generate the initial position pushed out relative to the origin (in emitter space)
    cgVector3 vecPosition = cgVector3( 0.0f, fLength, 0.0f );

    // Rotate about the 'polar' up vector
    cgMatrix mtxTransform;
    cgMatrix::rotationZ( mtxTransform, CGEToRadian( fPolar ) );
    cgVector3::transformCoord( vecPosition, vecPosition, mtxTransform );

    // Now transform this into a world space origin if required.
    if ( !localSpace )
        cgVector3::transformCoord( vecPosition, vecPosition, getEmitterMatrix() );

    // Return the position
    return vecPosition;
}

//-----------------------------------------------------------------------------
//  Name : onParticleBirth () (Private)
/// <summary>
/// Execute the particle creation process for this particle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitter::onParticleBirth( cgParticle * pParticle )
{
    // Compute initial random properties
    cgFloat fSpeed           = cgMathUtility::randomFloat( mProperties.speed.min, mProperties.speed.max );
    cgFloat fAngularVelocity = cgMathUtility::randomFloat( mProperties.angularSpeed.min, mProperties.angularSpeed.max );
    cgFloat fInitialRotation = 0.0f;
    cgFloat fMass            = cgMathUtility::randomFloat( mProperties.mass.min, mProperties.mass.max );
    cgFloat fLifetime        = cgMathUtility::randomFloat( mProperties.lifetime.max, mProperties.lifetime.min );

    // Compute initial velocity
    cgVector3 Velocity = generateConeDirection( mProperties.innerCone, mProperties.outerCone ) * fSpeed;

    // Compute initial particle origin.
    cgVector3 Origin = generateOrigin( mProperties.deadZoneRadius, mProperties.emissionRadius );

    // Set particle properties
    pParticle->setMass( fMass );
    pParticle->setPosition( Origin );
    pParticle->setAngularVelocity( fAngularVelocity );
    pParticle->setRotation( fInitialRotation );
    pParticle->setVelocity( Velocity );
    pParticle->setMaximumAge( fLifetime );

    // Randomize initial angle?
    if ( mProperties.randomizeRotation )
        pParticle->setRotation( cgMathUtility::randomFloat( 0.0f, 360.0f ) );

    // Allow to live
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setGravity ()
/// <summary>
/// Set the internal gravity force vector used to adjust the velocities
/// of the particles over time.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::setGravity( const cgVector3 & vGravity )
{
    mGravity = vGravity;
}

//-----------------------------------------------------------------------------
//  Name : setGlobalForce ()
/// <summary>
/// Set the global force vector used to adjust the velocities of the 
/// particles over time. In this case, forces might include wind etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgParticleEmitter::setGlobalForce( const cgVector3 & vForce )
{
    mGlobalForce = vForce;
}

//-----------------------------------------------------------------------------
//  Name : particlesSpent ()
/// <summary>
/// Determine if all particles have been spent.
/// Note : If you want to ensure that all fired particles are dead, make sure
/// that you specify a value of true to the 'bIncludeAlive' parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitter::particlesSpent( bool bIncludeAlive /* = true */ )
{
    if ( bIncludeAlive && mActiveParticleCount != 0 )
        return false;

    // Fired enough particles?
    if ( mProperties.maxFiredParticles > 0 && mTotalReleased >= mProperties.maxFiredParticles )
        return true;

    // Particles not entirely spent
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setEmitterProperties ()
/// <summary>
/// Update the particle emitter's properties, altering the way it emits 
/// particles from this point forward.
/// </summary>
//-----------------------------------------------------------------------------
bool cgParticleEmitter::setEmitterProperties( const cgParticleEmitterProperties & Properties )
{
    // Automatically compute the maximum number of simultaneous particles
    // if requested.
    cgUInt32 nNewMaxParticles = Properties.maxSimultaneousParticles;
    if ( nNewMaxParticles == 0 )
    {
        nNewMaxParticles = (cgUInt32)ceilf(Properties.birthFrequency * Properties.lifetime.max);
        if ( nNewMaxParticles == 0 )
            nNewMaxParticles = 1;
        if ( nNewMaxParticles > 200000 )
            nNewMaxParticles = 200000;
    
    } // End if auto compute

    // Validate parameters
    if ( !mBillboardBuffer )
    {
        cgAppLog::write( cgAppLog::Error, _T("Cannot update a particle emitter properties via 'setEmitterProperties()' without it first having been initialized.\n") );
        return false;
    
    } // End if not initialized
    if ( !nNewMaxParticles )
    {
        cgAppLog::write( cgAppLog::Error, _T("Maximum number of simulatenous particles must be greater than zero. Failed to update particle emitter properties.\n") );
        return false;
    
    } // End if invalid max
    if ( nNewMaxParticles > 200000 )
    {
        cgAppLog::write( cgAppLog::Error, _T("Maximum number of simulatenous particles must be less than or equal to 200,000. Failed to update particle emitter properties.\n") );
        return false;
    
    } // End if invalid max

    // If the texture, shader or flags were altered, a new billboard buffer is required.
    bool bNewBuffer = false;
    if ( mProperties.particleTexture != Properties.particleTexture || mProperties.particleShader != Properties.particleShader ||
        mProperties.sortedRender != Properties.sortedRender || mProperties.velocityAligned != Properties.velocityAligned )
         bNewBuffer = true;

    // A new buffer is also required if the maximum number of simultaneous particles has been altered.
    if ( mMaxParticles != nNewMaxParticles )
        bNewBuffer = true;

    // Before we do anything, attempt to allocate a new billboard buffer with the specified
    // properties (just in case it fails, so we can easily rollback).
    // Prepare the new billboard buffer if required.
    if ( bNewBuffer )
    {
        cgBillboardBuffer * pNewBuffer = CG_NULL;

        // Does the billboard buffer need to support sorting?
        cgUInt32 nFlags = 0;
        if ( Properties.sortedRender )
            nFlags |= cgBillboardBuffer::SupportSorting;
        if ( Properties.velocityAligned )
            nFlags |= cgBillboardBuffer::OrientationAxis;

        // Select the correct shader.
        cgString strShader = Properties.particleShader;
        if ( strShader.empty() )
            strShader = _T("sys://Shaders/Particles.sh");

        // Atlas or not?
        if ( Properties.particleTexture.endsWith( _T(".xml") ) )
        {
            pNewBuffer = new cgBillboardBuffer();
            if ( !pNewBuffer->prepareBufferFromAtlas( nFlags, mRenderDriver, Properties.particleTexture, strShader ) )
            {
                delete pNewBuffer;
                return false;
            
            } // End if failed
            
        } // End if atlas
        else
        {
            pNewBuffer = new cgBillboardBuffer();
            if ( !pNewBuffer->prepareBuffer( nFlags, mRenderDriver, Properties.particleTexture, strShader ) )
            {
                delete pNewBuffer;
                return false;
            
            } // End if failed

        } // End if !atlas

        // Clear out the old buffer, but do not allow it to destroy the existing particles.
        mBillboardBuffer->clear(false);

        // Now replace the old buffer with the new one.
        delete mBillboardBuffer;
        mBillboardBuffer = pNewBuffer;

    } // End if new buffer required

    // Set the blending method to the billboard buffer.
    cgSurfaceShaderHandle hShader = mBillboardBuffer->getSurfaceShader();
    if ( hShader.isValid() )
        hShader->setInt( _T("blendMethod"), (cgInt32)Properties.blendMethod );

    // If the maximum number of simultaneous particles was altered
    // we also need to resize our internal particle containers.
    if ( mMaxParticles != nNewMaxParticles )
    {
        // Allocate new containers for the particles.
        cgParticle ** ppNewParticles       = new cgParticle*[ nNewMaxParticles ];
        cgParticle ** ppNewActiveParticles = new cgParticle*[ nNewMaxParticles ];
        cgParticle ** ppNewDeadParticles   = new cgParticle*[ nNewMaxParticles ];

        // Copy contents of old array, and delete any particles that 
        // are no longer valid or allocate the additional required.
        if ( nNewMaxParticles < mMaxParticles )
        {
            mActiveParticleCount = 0;
            
            // Copy as many particles as we need.
            for ( cgUInt32 i = 0, nDeadParticles = 0; i < nNewMaxParticles; ++i )
            {
                ppNewParticles[i] = mParticles[i];
                if ( ppNewParticles[i]->isAlive() )
                    ppNewActiveParticles[mActiveParticleCount++] = ppNewParticles[i];
                else
                    ppNewDeadParticles[ nDeadParticles++ ] = ppNewParticles[i];

            } // Next particle                    

            // Release any remaining.
            for ( cgUInt32 i = nNewMaxParticles; i < mMaxParticles; ++i )
                delete mParticles[i];

        } // End if new < old
        else
        {
            // Copy all existing particles
            memcpy( ppNewParticles, mParticles, mMaxParticles * sizeof(cgParticle*) );
            memcpy( ppNewActiveParticles, mActiveParticles, mMaxParticles * sizeof(cgParticle*) );
            memcpy( ppNewDeadParticles, mDeadParticles, mMaxParticles * sizeof(cgParticle*) );

            // Allocate any new particles that may be required.
            cgUInt32 nFirstDead = mMaxParticles - mActiveParticleCount;
            for ( cgUInt32 i = mMaxParticles; i < nNewMaxParticles; ++i )
            {
                ppNewParticles[i] = new cgParticle();

                // All new particles are "dead" to begin with
                ppNewDeadParticles[ nFirstDead + (i - mMaxParticles) ] = ppNewParticles[i];

                // Force the particle's age to be greater than its maximum age 
                // so that anyone who calls 'isAlive()' on the particle after this
                // point will know that it's dead.
                ppNewParticles[i]->mAge = ppNewParticles[i]->mMaximumAge + 1;

            } // Next new

        } // End if new > old

        // Swap containers.
        delete []mParticles;
        delete []mActiveParticles;
        delete []mDeadParticles;
        mParticles = ppNewParticles;
        mActiveParticles = ppNewActiveParticles;
        mDeadParticles = ppNewDeadParticles;
        mMaxParticles = nNewMaxParticles;

    } // End if max particles changed

    // If we got a new buffer, add all billboards back to the new one.
    if ( bNewBuffer )
    {
        for ( cgUInt32 i = 0; i < mMaxParticles; ++i )
        {
            mBillboardBuffer->addBillboard( mParticles[i] );
            mParticles[i]->update();
            
        } // Next particle
        mBillboardBuffer->endPrepare();

    } // End if had new buffer

    // Swap emission properties with new details.
    mProperties = Properties;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getEmitterProperties ()
/// <summary>
/// Retrieve the information structure that describes how this particle emitter
/// emits particles.
/// </summary>
//-----------------------------------------------------------------------------
const cgParticleEmitterProperties & cgParticleEmitter::getEmitterProperties( ) const
{
    return mProperties;
}