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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
    
    // Clear structures
    mProperties.colorRCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorGCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorBCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.colorACurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleXCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.scaleYCurve.setDescription( cgBezierSpline2::Maximum );
    mProperties.shaderSource.clear();
    mProperties.textureFile.clear();
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
    
    /*// Load in the base emitter and particle properties from the particle definition script
    GetPrivateProfileString( _T("Emitter"), _T("innerCone"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.innerCone;
    GetPrivateProfileString( _T("Emitter"), _T("OuterCone"), _T("30"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.outerCone;
    GetPrivateProfileString( _T("Emitter"), _T("BaseRadius"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.BaseRadius;
    GetPrivateProfileString( _T("Emitter"), _T("NullRadius"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.NullRadius;
    GetPrivateProfileString( _T("Emitter"), _T("Frequency"), _T("20"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.birthFrequency;
    GetPrivateProfileString( _T("Emitter"), _T("MaxParticles"), _T("20"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.maxSimultaneousParticles;
    GetPrivateProfileString( _T("Emitter"), _T("DepthSorted"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    mProperties.sortedRender = cgStringUtility::ParseBool( Buffer );
    GetPrivateProfileString( _T("Emitter"), _T("EmitterDir"), _T("0.0,0.0,0.0"), Buffer, 511, mScriptFile.c_str() );
    cgStringUtility::TryParse( Buffer, mProperties.emitterDirection );
    GetPrivateProfileString( _T("Emitter"), _T("InternalForce"), _T("0.0,0.0,0.0"), Buffer, 511, mScriptFile.c_str() );
    cgStringUtility::TryParse( Buffer, mProperties.vecInternalForce );
    GetPrivateProfileString( _T("Emitter"), _T("InitRandomAngle"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    mProperties.InitRandomAngle = cgStringUtility::ParseBool( Buffer );
    GetPrivateProfileString( _T("Emitter"), _T("FireOnce"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    mProperties.bFireOnce = cgStringUtility::ParseBool( Buffer );
    GetPrivateProfileString( _T("Emitter"), _T("FireAmount"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.fireAmount;
    GetPrivateProfileString( _T("Emitter"), _T("FireDelay"), _T("0.0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.fireDelay;
    GetPrivateProfileString( _T("Emitter"), _T("FireDelayOffset"), _T("0.0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.fireDelayOffset;
    
    
    
    // Retrieve particle properties
    GetPrivateProfileString( _T("Particles"), _T("MinVelocity"), _T("30"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MinVelocity;
    GetPrivateProfileString( _T("Particles"), _T("MaxVelocity"), _T("30"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MaxVelocity;
    GetPrivateProfileString( _T("Particles"), _T("MinMass"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MinMass;
    GetPrivateProfileString( _T("Particles"), _T("MaxMass"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MaxMass;
    GetPrivateProfileString( _T("Particles"), _T("AirResistance"), _T("0.001"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.airResistance;
    GetPrivateProfileString( _T("Particles"), _T("MinAngularVelocity"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MinAngularVelocity;
    GetPrivateProfileString( _T("Particles"), _T("MaxAngularVelocity"), _T("0"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MaxAngularVelocity;
    GetPrivateProfileString( _T("Particles"), _T("MaxAge"), _T("5"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.fMaxAge;
    GetPrivateProfileString( _T("Particles"), _T("BaseWidth"), _T("5"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.baseSize.width;
    GetPrivateProfileString( _T("Particles"), _T("BaseHeight"), _T("5"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.baseSize.height;
    GetPrivateProfileString( _T("Particles"), _T("MinBaseScale"), _T("1"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MinBaseScale;
    GetPrivateProfileString( _T("Particles"), _T("MaxBaseScale"), _T("1"), Buffer, 511, mScriptFile.c_str() );
    cgStringParser( Buffer ) >> mProperties.MaxBaseScale;
    GetPrivateProfileString( _T("Materials"), _T("TextureFile"), _T(""), Buffer, 511, mScriptFile.c_str() );
    mProperties.textureFile = Buffer;
    GetPrivateProfileString( _T("Materials"), _T("EffectFile"), _T(""), Buffer, 511, mScriptFile.c_str() );
    mProperties.strEffectFile = Buffer;

    // Read color keyframes if any
    nKeyCount = GetPrivateProfileInt( _T("ColorKeys"), _T("KeyCount"), 0, mScriptFile.c_str() );
    for ( i = 0; i < nKeyCount; ++i )
    {
        ParticleColorKey Key;

        // Retrieve the 'time' for this particular callback (remember this is a scalar between 0 and 1 over particle lifespan)
        cgString strKeyName = cgString::Format( _T("Key[%i].Time"), i );
        GetPrivateProfileString( _T("ColorKeys"), strKeyName.c_str(), _T("0.0"), Buffer, 511, mScriptFile.c_str() );
        cgStringParser( Buffer ) >> Key.fTime;

        // Retrieve the 'color' for this particular callback
        strKeyName = cgString::Format( _T("Key[%i].Color"), i );
        GetPrivateProfileString( _T("ColorKeys"), strKeyName.c_str(), _T("1.0,1.0,1.0,1.0"), Buffer, 511, mScriptFile.c_str() );
        cgStringUtility::TryParse( Buffer, Key.Color );

        // Add to color keyframes list
        mProperties.ColorKeys.push_back( Key );

    } // Next Color Key

    // Get any scale keys
    nKeyCount = GetPrivateProfileInt( _T("ScaleKeys"), _T("KeyCount"), 0, mScriptFile.c_str() );
    for ( i = 0; i < nKeyCount; ++i )
    {
        ParticleScaleKey Key;

        // Retrieve the 'time' for this particular callback (remember this is a scalar between 0 and 1 over particle lifespan)
        cgString strKeyName = cgString::Format( _T("Key[%i].Time"), i );
        GetPrivateProfileString( _T("ScaleKeys"), strKeyName.c_str(), _T("0.0"), Buffer, 511, mScriptFile.c_str() );
        cgStringParser( Buffer ) >> Key.fTime;

        // Retrieve the 'scale' for this particular callback
        strKeyName = cgString::Format( _T("Key[%i].Scale"), i );
        GetPrivateProfileString( _T("ScaleKeys"), strKeyName.c_str(), _T("1.0,1.0"), Buffer, 511, mScriptFile.c_str() );
        cgStringUtility::TryParse( Buffer, Key.vScale );

        // Add to scale keyframes list
        mProperties.ScaleKeys.push_back( Key );

    } // Next Scale Key*/

    // Allocate billboard items.
    mBillboardBuffer = new cgBillboardBuffer();
    mParticles       = new cgParticle*[ mMaxParticles ];
    mActiveParticles = new cgParticle*[ mMaxParticles ];
    mDeadParticles   = new cgParticle*[ mMaxParticles ];

    // Does the billboard buffer need to support sorting?
    cgUInt32 nFlags = 0;
    if ( mProperties.sortedRender == true )
        nFlags |= cgBillboardBuffer::SupportSorting;

    // Prepare the billboard buffer
    if ( mProperties.textureFile.endsWith( _T(".xml") ) )
    {
        if ( !mBillboardBuffer->prepareBufferFromAtlas( nFlags, mRenderDriver, mProperties.textureFile, mProperties.shaderSource ) )
        {
            dispose(false);
            return false;
        
        } // End if failed
    
    } // End if atlas
    else
    {
        if ( !mBillboardBuffer->prepareBuffer( nFlags, mRenderDriver, mProperties.textureFile, mProperties.shaderSource ) )
        {
            dispose(false);
            return false;
        
        } // End if failed

    } // End if !atlas

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

        /*// ******************************************************************
        // * COLOR KEYS
        // ******************************************************************
        if ( mProperties.ColorKeys.empty() == false )
        {
            ParticleColorKey * pKey1 = CG_NULL, * pKey2 = CG_NULL;

            for ( j = (cgInt32)pParticle->mLastColorKeyIndex; j < (cgInt32)(mProperties.ColorKeys.size() - 1); ++j )
            {
                ParticleColorKey * pKey     = &mProperties.ColorKeys[j];
                ParticleColorKey * pNextKey = &mProperties.ColorKeys[j + 1];

                // Do these keys  bound the requested time ?
                if ( fParticleAge >= pKey->fTime && fParticleAge <= pNextKey->fTime )
                {
                    // Update last index
                    pParticle->mLastColorKeyIndex = (cgUInt16)j;

                    // Store the two bounding keys
                    pKey1 = pKey;
                    pKey2 = pNextKey;
                    break;

                } // End if found keys

            } // Next color Key

            // Make sure we found keys
            if ( pKey1 && pKey2 ) 
            {
                // Calculate interpolation
                cgFloat fInterpVal  = fParticleAge - pKey1->fTime;
                fInterpVal /= (pKey2->fTime - pKey1->fTime);

                // Interpolate!
                cgColorValue FinalColor;
                D3DXColorLerp( &FinalColor, (cgColorValue*)&pKey1->Color, (cgColorValue*)&pKey2->Color, fInterpVal );
                pParticle->mColor = FinalColor;

            } // End if keys were found
            else
            {
                // Inform cache that it should no longer search unless cache is invalidated
                pParticle->mLastColorKeyIndex = (cgUInt16)(mProperties.ColorKeys.size() - 1);

                // Color is the same as the last color key found
                pParticle->mColor = mProperties.ColorKeys[ pParticle->mLastColorKeyIndex ].Color;

            } // End if no keys found

        } // End if any color keys

        // ******************************************************************
        // * SCALE KEYS
        // ******************************************************************
        if ( mProperties.ScaleKeys.empty() == false )
        {
            ParticleScaleKey * pKey1 = CG_NULL, * pKey2 = CG_NULL;
            for ( j = (cgInt32)pParticle->mLastScaleKeyIndex; j < (cgInt32)mProperties.ScaleKeys.size() - 1; ++j )
            {
                ParticleScaleKey * pKey     = &mProperties.ScaleKeys[j];
                ParticleScaleKey * pNextKey = &mProperties.ScaleKeys[j + 1];

                // Do these keys  bound the requested time ?
                if ( fParticleAge >= pKey->fTime && fParticleAge <= pNextKey->fTime )
                {
                    // Update last index
                    pParticle->mLastScaleKeyIndex = (cgUInt16)j;

                    // Store the two bounding keys
                    pKey1 = pKey;
                    pKey2 = pNextKey;
                    break;

                } // End if found keys

            } // Next Scale Key

            // Make sure we found keys
            if ( pKey1 && pKey2 ) 
            {
                // Calculate interpolation
                cgFloat fInterpVal  = fParticleAge - pKey1->fTime;
                fInterpVal /= (pKey2->fTime - pKey1->fTime);

                // Interpolate!
                pParticle->mScale = pKey1->vScale + ((pKey2->vScale - pKey1->vScale) * fInterpVal);

            } // End if keys were found
            else
            {
                // Inform cache that it should no longer search unless cache is invalidated
                pParticle->mLastScaleKeyIndex = (cgUInt16)(mProperties.ScaleKeys.size() - 1);

                // Scale is the same as the last Scale key found
                pParticle->mScale = mProperties.ScaleKeys[ pParticle->mLastScaleKeyIndex ].vScale;

            } // End if no keys found

            // Multiply scale by velocity if requested
            if ( bVelocityScale == true )
                pParticle->mScale.y *= fVelocity;

        } // End if any Scale keys
        else
        {
            // Set scale to velocity if requested
            if ( bVelocityScale == true )
                pParticle->mScale.y = fVelocity;

        } // End if no scale keys*/

        // ******************************************************************
        // * COLOR CURVES
        // ******************************************************************
        const bool bApproximateSample = false;
        cgColorValue NewColor;
        NewColor.r = mProperties.colorRCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.g = mProperties.colorGCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.b = mProperties.colorBCurve.evaluateForX( fParticleAge, bApproximateSample );
        NewColor.a = mProperties.colorACurve.evaluateForX( fParticleAge, bApproximateSample );
        pParticle->mColor = NewColor;

        // ******************************************************************
        // * SCALE CURVES
        // ******************************************************************
        pParticle->mScale.x = mProperties.scaleXCurve.evaluateForX( fParticleAge, bApproximateSample );
        pParticle->mScale.y = mProperties.scaleYCurve.evaluateForX( fParticleAge, bApproximateSample );

        // Multiply scale by velocity if requested
        if ( bVelocityScale )
            pParticle->mScale.y *= fVelocity;

        // Update the underlying billboard
        pParticle->update();

    } // Next Active Particle

    // Have we RELEASED all of the required particles (not including those still active)?
    if ( particlesSpent( false ) )
        return;

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
        mTotalReleased = 0;
        
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
        if ( bVelocityScale == true )
            pParticle->mScale.y *= fVelocity;
        
        // Set as visible and add to active list if it wasn't already
        if ( bActiveParticle == false )
        {
            pParticle->setVisible( true );
            mActiveParticles[ mActiveParticleCount++ ] = pParticle;
        
        } // End if not from active list

        // Update the newly created particle
        pParticle->update();
    
    } // Next new particle
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
    if ( mBillboardBuffer == CG_NULL )
        return;

    // Ensure we use identity world transform
    mRenderDriver->setWorldTransform( CG_NULL );
    
    // Render the entire billboard buffer.
    if ( mProperties.sortedRender == false )
        mBillboardBuffer->render( );
    else
        mBillboardBuffer->renderSorted( pCamera );
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
    cgVector3 vecUp        = (cgVector3&)mTransform._21;
    cgVector3 vecRight     = (cgVector3&)mTransform._11;
    cgVector3 vecLook      = (cgVector3&)mTransform._31;
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
    cgMatrix mtxTransform;

    // If we're not using an emission radius, simply return the emitter's world space pos
    if ( fEmissionRadius < CGE_EPSILON_1MM )
        return getEmitterPosition();

    // Pick a random point along the emitters radius
    cgFloat fLength = fDeadZoneRadius + (((cgFloat)rand() / (cgFloat)RAND_MAX) * (fEmissionRadius - fDeadZoneRadius));

    // A Polar angle
    cgFloat fPolar  = ((cgFloat)rand() / (cgFloat)RAND_MAX) * 360.0f;

    // Generate the initial position pushed out relative to the origin (in emitter space)
    cgVector3 vecPosition = cgVector3( 0.0f, fLength, 0.0f );

    // Rotate about the 'polar' up vector
    cgMatrix::rotationZ( mtxTransform, CGEToRadian( fPolar ) );
    cgVector3::transformCoord( vecPosition, vecPosition, mtxTransform );

    // Now transform this into a world space origin
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
    if ( mProperties.textureFile != Properties.textureFile || mProperties.shaderSource != Properties.shaderSource ||
         mProperties.sortedRender != Properties.sortedRender )
         bNewBuffer = true;

    // A new buffer is also required if the number of maximum number of simultaneous particles has been altered.
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
        if ( mProperties.sortedRender == true )
            nFlags |= cgBillboardBuffer::SupportSorting;

        // Atlas or not?
        if ( Properties.textureFile.endsWith( _T(".xml") ) )
        {
            pNewBuffer = new cgBillboardBuffer();
            if ( !pNewBuffer->prepareBufferFromAtlas( nFlags, mRenderDriver, Properties.textureFile, Properties.shaderSource ) )
            {
                delete pNewBuffer;
                return false;
            
            } // End if failed
            
        } // End if atlas
        else
        {
            pNewBuffer = new cgBillboardBuffer();
            if ( !pNewBuffer->prepareBuffer( nFlags, mRenderDriver, Properties.textureFile, Properties.shaderSource ) )
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