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
// Name : cgHingeJoint.cpp                                                   //
//                                                                           //
// Desc : Class implementing a single axis hinge joint. This joint can be    //
//        used to connect two bodies around a central axis and allow them    //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgHingeJoint Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Joints/cgHingeJoint.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgHingeJoint Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgHingeJoint () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJoint::cgHingeJoint( cgPhysicsWorld * world, cgPhysicsBody * parent, cgPhysicsBody * child, const cgTransform & pivotTransform ) : cgPhysicsJoint( world, 6, child, parent )
{
    // Setup defaults.
    mUseLimits      = false;
    mJointAngle     = 0;
    mJointOmega     = 0;
    mMinimumAngle   = CGEToRadian( -45.0f );
    mMaximumAngle   = CGEToRadian( 45.0f );

    // Set the pivot frame for the joint.
    setPivotTransform( pivotTransform );

    // Listen for adjustments to the body transformation. We'll need to update
    // the pivot frames due to transforms NOT related to dynamics updates.
    if ( child )
        child->registerEventListener( this );
    if ( parent )
        parent->registerEventListener( this );
}

//-----------------------------------------------------------------------------
//  Name : ~cgHingeJoint () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgHingeJoint::~cgHingeJoint()
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
void cgHingeJoint::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Remove event listeners.
    if ( mBody0 )
        mBody0->unregisterEventListener( this );
    if ( mBody1 )
        mBody1->unregisterEventListener( this );

    // Dispose base class if requested.
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
bool cgHingeJoint::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_HingeJoint )
        return true;

    // Supported by base?
    return cgPhysicsJoint::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onPhysicsBodyTransformed ( )
/// <summary>
/// Triggered whenever the physics body designed represent this object during
/// scene dynamics processing is transformed as a result of its simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e )
{
    // Ignore updates that were due to dynamics.
    if ( e->dynamicsUpdate )
        return;

    // Convert pivot frame into the space of the two connected bodies.
    getLocalTransforms( mGlobalPivotFrame, mLocalPivotFrame0, mLocalPivotFrame1 );
}

//-----------------------------------------------------------------------------
// Name : setPivotTransform ( )
/// <summary>
/// Set a new transform for the pivot frame (position and orientation). Hinge
/// will operate about the X axis of the specified transform.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::setPivotTransform( const cgTransform & pivotTransform )
{
    // Convert pivot transform into the same scale as the physics world.
    mGlobalPivotFrame = pivotTransform;
    mGlobalPivotFrame.position() *= mWorld->toPhysicsScale();

    // Convert pivot frame into the space of the two connected bodies.
    getLocalTransforms( mGlobalPivotFrame, mLocalPivotFrame0, mLocalPivotFrame1 );
}

//-----------------------------------------------------------------------------
//  Name : enableLimits ()
/// <summary>
/// Enable angle of rotation limiting for this hinge joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::enableLimits( bool enabled )
{
    mUseLimits = enabled;
}

//-----------------------------------------------------------------------------
//  Name : setLimits ()
/// <summary>
/// Set the allowable range of rotation for this joint when limiting is enabled
/// via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::setLimits( cgFloat minDegrees, cgFloat maxDegrees )
{
    mMinimumAngle = CGEToRadian( minDegrees );
    mMaximumAngle = CGEToRadian( maxDegrees );
}

//-----------------------------------------------------------------------------
//  Name : getAngle ()
/// <summary>
/// Get the current angle of rotation for the joint in degrees.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHingeJoint::getAngle( ) const
{
    return CGEToDegree(mJointAngle);
}

//-----------------------------------------------------------------------------
//  Name : getRelativeAngularVelocity ()
/// <summary>
/// Get the current relative angular velocity of the two connected bodies.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHingeJoint::getRelativeAngularVelocity( ) const
{
    return mJointOmega;
}

//-----------------------------------------------------------------------------
//  Name : getAxis ()
/// <summary>
/// Get the axis about which the bodies connected to the hinge joint are 
/// currently constrained.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgHingeJoint::getAxis( ) const
{
    cgVector3 axisOut;
    const cgTransform & t0 = mBody0->getTransform();
    return t0.transformNormal( axisOut, mLocalPivotFrame0.xAxis() );
}

//-----------------------------------------------------------------------------
//  Name : getInfo () (Protected, Virtual)
/// <summary>
/// Retrieve information about the joint for serialization. Internal newton
/// requirement.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::getInfo( NewtonJointRecord * info )
{
    strcpy( info->m_descriptionType, "hinge" );

	info->m_attachBody_0 = mBody0->getInternalBody();
	info->m_attachBody_1 = mBody1->getInternalBody();

    info->m_minLinearDof[0] = 0.0f;
    info->m_maxLinearDof[0] = 0.0f;

    info->m_minLinearDof[1] = 0.0f;
    info->m_maxLinearDof[1] = 0.0f;;

    info->m_minLinearDof[2] = 0.0f;
    info->m_maxLinearDof[2] = 0.0f;

    // The joint angle can be determined by getting the angle between 
    // any two non parallel vectors
    if ( mUseLimits )
    {
        // Calculate the position of the pivot point and the Jacobian 
        // direction vectors, in global space. 
        cgVector3 yyCross;
        cgTransform t0, t1;
        getGlobalTransforms( mLocalPivotFrame0, mLocalPivotFrame1, t0, t1 );
        cgVector3::cross(yyCross, t0.yAxis(), t1.yAxis() );
        const cgFloat sinAngle = cgVector3::dot( yyCross, t0.xAxis() );
        const cgFloat cosAngle = cgVector3::dot( t0.yAxis(), t1.yAxis() );
        const cgFloat angle = atan2f( sinAngle, cosAngle );
        info->m_minAngularDof[0] = CGEToDegree(mMinimumAngle - angle);
        info->m_maxAngularDof[0] = CGEToDegree(mMaximumAngle - angle);
    
    } // End if limited
    else
    {
        info->m_minAngularDof[0] = -FLT_MAX ;
        info->m_maxAngularDof[0] = FLT_MAX ;
    
    } // End if ! limited

    info->m_minAngularDof[1] = 0.0f;
    info->m_maxAngularDof[1] = 0.0f;

    info->m_minAngularDof[2] = 0.0f;
    info->m_maxAngularDof[2] = 0.0f;

    info->m_bodiesCollisionOn = (mBodyCollision) ? 1 : 0;

    memcpy (info->m_attachmenMatrix_0, &((cgMatrix)mLocalPivotFrame0), sizeof(cgMatrix));
    memcpy (info->m_attachmenMatrix_1, &((cgMatrix)mLocalPivotFrame1), sizeof(cgMatrix));	
}

//-----------------------------------------------------------------------------
//  Name : submitConstraints() (Protected, Virtual)
/// <summary>
/// Callback triggered when we need to submit constraints to the simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgHingeJoint::submitConstraints( cgFloat fTimeStep, cgInt nThreadIndex )
{
    static const cgFloat MinJointPinLength = 50.0f;

    // Calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
    cgTransform t0, t1;
    getGlobalTransforms( mLocalPivotFrame0, mLocalPivotFrame1, t0, t1 );

    // Restrict the movement on the pivot point along all three orthonormal directions.
    NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.xAxis() );
    NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.yAxis() );
    NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.zAxis() );

    // Get a point along the pin axis at some reasonably large distance from the pivot.
    cgVector3 q0 ( t0.position() + t0.xAxis() * MinJointPinLength );
    cgVector3 q1 ( t1.position() + t1.xAxis() * MinJointPinLength );

    // Two constraint rows perpendicular to the pin vector
    NewtonUserJointAddLinearRow( mJoint, q0, q1, t0.yAxis() );
    NewtonUserJointAddLinearRow( mJoint, q0, q1, t0.zAxis() );

    // The joint angle can be determined by getting the angle between any two non parallel vectors.
    cgVector3 yyCross;
    cgVector3::cross( yyCross, t0.yAxis(), t1.yAxis() );
    const cgFloat sinAngle = cgVector3::dot( yyCross, t0.xAxis() );
    const cgFloat cosAngle = cgVector3::dot( t0.yAxis(), t1.yAxis() );
    const cgFloat angle    = computeNewJointAngle( cosAngle, sinAngle );

    // Restrict rotation if limits are enabled.
    if ( mUseLimits )
    {
        if ( angle < mMinimumAngle )
        {
            const cgFloat relativeAngle = angle - mMinimumAngle;
            
            // The angle was clipped. Save the new clip limit.
            mJointAngle = mMinimumAngle;

            // Minimize the exceeded angle error.
            NewtonUserJointAddAngularRow( mJoint, relativeAngle, t0.xAxis() );
            NewtonUserJointSetRowStiffness( mJoint, 1.0f );

            // Allow the joint to move back freely 
            NewtonUserJointSetRowMaximumFriction( mJoint, 0.0f );

        } // End if < minimum
        else if ( angle > mMaximumAngle )
        {
            const cgFloat relativeAngle = angle - mMaximumAngle;

            // The angle was clipped save the new clip limit
            mJointAngle = mMaximumAngle;

            // Minimize the exceeded angle error.
            NewtonUserJointAddAngularRow( mJoint, relativeAngle, t0.xAxis() );
            NewtonUserJointSetRowStiffness( mJoint, 1.0f );

            // Allow the joint to move back freely
            NewtonUserJointSetRowMinimumFriction( mJoint, 0.0f );

        } // End if > maximum
    
    } // End if limited

    // save the current joint Omega
    cgVector3 omega0(0.0f, 0.0f, 0.0f);
    cgVector3 omega1(0.0f, 0.0f, 0.0f);
    NewtonBodyGetOmega( mBody0->getInternalBody(), omega0 );
    if ( mBody1 )
        NewtonBodyGetOmega( mBody1->getInternalBody(), omega1 );
    mJointOmega = cgVector3::dot( (omega0 - omega1), t0.xAxis() );
}

//-----------------------------------------------------------------------------
//  Name : computeNewJointAngle () (Protected)
/// <summary>
/// Compute the new angle of rotation for the joint.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgHingeJoint::computeNewJointAngle( cgFloat newAngleCos, cgFloat newAngleSin )
{
    const cgFloat sinJointAngle = sinf(mJointAngle);
    const cgFloat cosJointAngle = cosf(mJointAngle);
    const cgFloat sinDelta = newAngleSin * cosJointAngle - newAngleCos * sinJointAngle; 
    const cgFloat cosDelta = newAngleCos * cosJointAngle + newAngleSin * sinJointAngle; 
    mJointAngle += atan2f( sinDelta, cosDelta );
    return mJointAngle;
}