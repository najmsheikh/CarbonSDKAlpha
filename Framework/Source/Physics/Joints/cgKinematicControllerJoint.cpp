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
// Name : cgKinematicControllerJoint.cpp                                     //
//                                                                           //
// Desc : Class implementing a simple kinematic controller joint. This joint //
//        can be used to anchor a body to a specific point in space. This    //
//        anchor point can then be manipulated dynamically at runtime        //
//        resulting in the body following.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgKinematicControllerJoint Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Joints/cgKinematicControllerJoint.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgKinematicControllerJoint Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJoint () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJoint::cgKinematicControllerJoint( cgPhysicsWorld * pWorld, cgPhysicsBody * pBody, cgVector3 vWorldSpaceHandle ) : cgPhysicsJoint( pWorld, 6, pBody, CG_NULL )
{
    // Initialize variables to sensible defaults.
    mMaxLinearFriction    = 1.0f;
    mMaxAngularFriction   = 1.0f;
    mConstraintMode        = PreserveOrientation;

    // Convert handle location into the same scale as the physics world.
    vWorldSpaceHandle *= mWorld->toPhysicsScale();
    
    // Get the initial position and orientation of the body.
    cgMatrix m;
    NewtonBodyGetMatrix( pBody->getInternalBody(), m );
    cgTransform t = m;

    // Backup the current automatic sleep state for the object
    // and then disable it.
    mOldBodyAutoSleep = pBody->isAutoSleepEnabled();
    pBody->enableAutoSleep( false );

    // Back transform the position of the "handle" in global space such that it
    // becomes relative to the body in the correct fashion.
    t.inverseTransformCoord( mLocalHandleOffset, vWorldSpaceHandle );
    /*cgVector3 vLocalHandle = (vWorldSpaceHandle - t.Position());
    mLocalHandleOffset.x = cgVector3::dot( &vLocalHandle, &t.xAxis() );
    mLocalHandleOffset.y = cgVector3::dot( &vLocalHandle, &t.yAxis() );
    mLocalHandleOffset.z = cgVector3::dot( &vLocalHandle, &t.zAxis() );*/

    // Position the controller initially at the specified location.
    t.setPosition( vWorldSpaceHandle );

    // Store initial position and orientation
    t.decompose( mOrientation, mPosition );
}

//-----------------------------------------------------------------------------
//  Name : ~cgKinematicControllerJoint () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJoint::~cgKinematicControllerJoint()
{
    // Clean up.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Restore the auto-sleep status of the body.
    if ( mBody0 )
        mBody0->enableAutoSleep( mOldBodyAutoSleep );
    
    // dispose base class if requested.
    if ( bDisposeBase )
        cgPhysicsJoint::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgKinematicControllerJoint::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_KinematicControllerJoint )
        return true;

    // Supported by base?
    return cgPhysicsJoint::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : setConstraintMode ()
/// <summary>
/// Set the mechanism that will be used to constrain the body to which this
/// joint is attached. The body can either be anchored to a single point in
/// space with freedom to rotate about it, or the orientation can be entirely
/// fixed.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setConstraintMode( ConstraintMode Mode )
{
    mConstraintMode = Mode;
}

//-----------------------------------------------------------------------------
//  Name : setMaxAngularFriction ()
/// <summary>
/// Set the maximum amount of angular friction that should be considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setMaxAngularFriction( cgFloat fFriction )
{
    mMaxAngularFriction = fFriction;
}

//-----------------------------------------------------------------------------
//  Name : setMaxLinearFriction ()
/// <summary>
/// Set the maximum amount of linear friction that should be considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setMaxLinearFriction( cgFloat fFriction )
{
    mMaxLinearFriction = fFriction;
}

//-----------------------------------------------------------------------------
//  Name : setPosition ()
/// <summary>
/// Set the world space position of the joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setPosition( const cgVector3 & vPos )
{
    mPosition = mWorld->toPhysicsScale( vPos );
}

//-----------------------------------------------------------------------------
//  Name : setOrientation ()
/// <summary>
/// Set the world space orientation of the joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setOrientation( const cgQuaternion & qRotation )
{
    mOrientation = qRotation;
}

//-----------------------------------------------------------------------------
//  Name : setTransform ()
/// <summary>
/// Set the world space position and orientation of the joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::setTransform( const cgTransform & Transform )
{
    cgVector3 vPosition;
    cgQuaternion qRotation;
    Transform.decompose( qRotation, vPosition );
    setPosition( vPosition );
    setOrientation( qRotation );
}

//-----------------------------------------------------------------------------
//  Name : getConstraintMode ()
/// <summary>
/// Get the mechanism that is in use for constraining the body to which this
/// joint is attached. The body can either be anchored to a single point in
/// space with freedom to rotate about it, or the orientation can be entirely
/// fixed.
/// </summary>
//-----------------------------------------------------------------------------
cgKinematicControllerJoint::ConstraintMode cgKinematicControllerJoint::getConstraintMode( ) const
{
    return mConstraintMode;
}

//-----------------------------------------------------------------------------
//  Name : getMaxAngularFriction ()
/// <summary>
/// Get the maximum amount of angular friction that is being considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgKinematicControllerJoint::getMaxAngularFriction( ) const
{
    return mMaxAngularFriction;
}

//-----------------------------------------------------------------------------
//  Name : getMaxLinearFriction ()
/// <summary>
/// Get the maximum amount of linear friction that is being considered while 
/// solving the constraint between the joint and the attached body.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgKinematicControllerJoint::getMaxLinearFriction( ) const
{
    return mMaxLinearFriction;
}

//-----------------------------------------------------------------------------
//  Name : getPosition ()
/// <summary>
/// Retrieve the world space position of the joint.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgKinematicControllerJoint::getPosition( ) const
{
    return mWorld->fromPhysicsScale( mPosition );
}

//-----------------------------------------------------------------------------
//  Name : getOrientation ()
/// <summary>
/// Retrieve the world space orientation of the joint.
/// </summary>
//-----------------------------------------------------------------------------
const cgQuaternion & cgKinematicControllerJoint::getOrientation( ) const
{
    return mOrientation;
}

//-----------------------------------------------------------------------------
//  Name : getInfo () (Protected, Virtual)
/// <summary>
/// Retrieve information about the joint for serialization. Internal newton
/// requirement.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::getInfo( NewtonJointRecord * pInfo )
{
    strcpy( pInfo->m_descriptionType, "kinematicController");
    cgToDo( "Physics", "Need to add additional serialization info." );
}

//-----------------------------------------------------------------------------
//  Name : submitConstraints() (Protected, Virtual)
/// <summary>
/// Callback triggered when we need to submit constraints to the simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgKinematicControllerJoint::submitConstraints( cgFloat fTimeStep, cgInt nThreadIndex )
{
    cgVector3 v, w, cg, p0, r0;
    cgMatrix m;
    
    // Calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
    NewtonBody * pBody = mBody0->getInternalBody();
    NewtonBodyGetOmega( pBody, w );
    NewtonBodyGetVelocity( pBody, v );
    NewtonBodyGetCentreOfMass( pBody, cg );
    NewtonBodyGetMatrix( pBody, m );
    cgTransform t = m;
    
    cgFloat fInvTimeStep = 1.0f / fTimeStep;
    t.transformCoord( p0, mLocalHandleOffset );
    t.transformNormal( r0, (mLocalHandleOffset - cg) );
    cgVector3::cross( r0, w, r0 );
    cgVector3 pointVeloc = v + r0;
    cgVector3 relPosit   = mPosition - p0;
    cgVector3 relVeloc   = (relPosit * fInvTimeStep) - pointVeloc;
    cgVector3 relAccel   = (relVeloc * (fInvTimeStep * 0.3f)); 

    // Compute the /actual/ maximum frictions scaled by mass.
    cgFloat Ixx, Iyy, Izz, fMass;
    NewtonBodyGetMassMatrix( pBody, &fMass, &Ixx, &Iyy, &Izz);
	cgFloat fActualMaxLinearFriction = fabsf( mMaxLinearFriction ) * fMass;
    cgFloat fActualMaxAngularFriction = fabsf( mMaxAngularFriction ) * fMass;

    // Restrict the movement on the pivot point along all three orthonormal directions
    cgTransform restriction; // Identity initially
    if ( cgVector3::lengthSq( relPosit ) >= CGE_EPSILON )
        grammSchmidtBasis( relPosit, restriction );

    NewtonUserJointAddLinearRow( mJoint, p0, mPosition, restriction.xAxis() );
    NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAccel, restriction.xAxis() ) );
    NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxLinearFriction );
    NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxLinearFriction );

    NewtonUserJointAddLinearRow( mJoint, p0, mPosition, restriction.yAxis() );
    NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAccel, restriction.yAxis() ) );
    NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxLinearFriction);
    NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxLinearFriction);

    NewtonUserJointAddLinearRow( mJoint, p0, mPosition, restriction.zAxis() );
    NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAccel, restriction.zAxis() ) );
    NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxLinearFriction);
    NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxLinearFriction);

    if ( mConstraintMode == PreserveOrientation )
    {
        cgFloat fRotation[4];
        NewtonBodyGetRotation( pBody, fRotation );
        cgQuaternion qRotation( fRotation[1], fRotation[2], fRotation[3], fRotation[0] );
        if ( cgQuaternion::dot( mOrientation, qRotation ) < 0.0f )
        {
            qRotation.x *= -1.0f; 
            qRotation.y *= -1.0f; 
            qRotation.z *= -1.0f; 
            qRotation.w *= -1.0f; 

        } // End if inverted
    
        // Calculate average relative omega
        cgVector3 relOmega;
        cgQuaternion dq( cgQuaternion( -mOrientation.x, -mOrientation.y, -mOrientation.z, mOrientation.w ) * mOrientation );
        cgVector3 omegaDir( dq.x, dq.y, dq.z );
        cgFloat dirMag2 = cgVector3::lengthSq( omegaDir );
        if ( dirMag2 < cgFloat(cgFloat(1.0e-5f) * cgFloat(1.0e-5f)) )
        {
            relOmega = -w;
        
        } // End if degenerate
        else
        {
            cgFloat dirMagInv = 1.0f / sqrtf( dirMag2 );
            cgFloat dirMag = dirMag2 * dirMagInv;
            cgFloat omegaMag = 2.0f * atan2f( dirMag, dq.w ) / fTimeStep;
            relOmega = (omegaDir * (dirMagInv * omegaMag)) - w;

        } // End if !degenerate

        cgFloat fMag = cgVector3::lengthSq( relOmega );
        if ( fMag > 1.0e-6f )
        {
            cgTransform basis;
            grammSchmidtBasis( (relOmega * (1.0f / fMag)), basis );
	
            cgFloat relSpeed = sqrtf( fMag );
            cgFloat relAlpha = relSpeed * fInvTimeStep;

            NewtonUserJointAddAngularRow( mJoint, 0.0f, basis.xAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, relAlpha);
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);

            NewtonUserJointAddAngularRow( mJoint, 0.0f, basis.yAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, 0.0f);
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);

            NewtonUserJointAddAngularRow( mJoint, 0.0f, basis.zAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, 0.0f);
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);

        }
        else
        {
            cgVector3 relAlpha = w * (-fInvTimeStep);
            NewtonUserJointAddAngularRow( mJoint, 0.0f, t.xAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.xAxis() ) );
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);

            NewtonUserJointAddAngularRow( mJoint, 0.0f, t.yAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.yAxis() ) );
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);

            NewtonUserJointAddAngularRow( mJoint, 0.0f, t.zAxis() );
            NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.zAxis() ) );
            NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction);
            NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction);
        }

    } // End if PreserveOrientation
    else
    {
        // This is the single handle pick mode. Add some angular friction.
        cgVector3 relAlpha = w * (-fInvTimeStep);
        NewtonUserJointAddAngularRow( mJoint, 0.0f, t.xAxis() );
        NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.xAxis() ) );
        NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction * 0.025f);
        NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction * 0.025f);

        NewtonUserJointAddAngularRow( mJoint, 0.0f, t.yAxis() );
        NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.yAxis() ) );
        NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction * 0.025f);
        NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction * 0.025f);

        NewtonUserJointAddAngularRow( mJoint, 0.0f, t.zAxis() );
        NewtonUserJointSetRowAcceleration( mJoint, cgVector3::dot( relAlpha, t.zAxis() ) );
        NewtonUserJointSetRowMinimumFriction( mJoint, -fActualMaxAngularFriction * 0.025f);
        NewtonUserJointSetRowMaximumFriction( mJoint,  fActualMaxAngularFriction * 0.025f);
    
    } // End if SinglePoint
}