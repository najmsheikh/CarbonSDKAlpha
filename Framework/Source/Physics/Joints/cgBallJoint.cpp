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
// Name : cgBallJoint.cpp                                                    //
//                                                                           //
// Desc : Class implementing a ball and socket joint. This joint can be      //
//        used to connect two bodies around a central point and allow them   //
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
// cgBallJoint Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Joints/cgBallJoint.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgBallJoint Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBallJoint () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBallJoint::cgBallJoint( cgPhysicsWorld * world, cgPhysicsBody * parent, cgPhysicsBody * child, const cgTransform & pivotTransform ) : cgPhysicsJoint( world, 6, child, parent )
{
    // Setup defaults.
    mUseLimits = false;
    setTwistLimits( 0, 0 );
    setConeLimit( 0 );
    
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
//  Name : ~cgBallJoint () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBallJoint::~cgBallJoint()
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
void cgBallJoint::dispose( bool bDisposeBase )
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
bool cgBallJoint::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BallJoint )
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
void cgBallJoint::onPhysicsBodyTransformed( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e )
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
void cgBallJoint::setPivotTransform( const cgTransform & pivotTransform )
{
    // Convert pivot transform into the same scale as the physics world.
    mGlobalPivotFrame = pivotTransform;
    mGlobalPivotFrame.position() *= mWorld->toPhysicsScale();

    // Convert pivot frame into the space of the two connected bodies.
    getLocalTransforms( mGlobalPivotFrame, mLocalPivotFrame0, mLocalPivotFrame1 );
}

//-----------------------------------------------------------------------------
// Name : getPivotTransform ( )
/// <summary>
/// Get either the original specified transform for the pivot frame (position 
/// and orientation), or the current transform relative to the current location
/// of the attached bodies.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgBallJoint::getPivotTransform( bool original ) const
{
    if ( original || !mBody0 )
    {
        cgTransform out = mGlobalPivotFrame;
        out.position() *= mWorld->fromPhysicsScale();
        return out;
    
    } // End if original
    else
    {
        cgMatrix m0;
        NewtonBodyGetMatrix( mBody0->getInternalBody(), m0 );
        cgTransform out = mLocalPivotFrame0 * m0;
        out.position() *= mWorld->fromPhysicsScale();
        return out;
    
    } // End if current
}

//-----------------------------------------------------------------------------
//  Name : enableLimits ()
/// <summary>
/// Enable angle of rotation limiting for this ball and socket joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgBallJoint::enableLimits( bool enabled )
{
    mUseLimits = enabled;
}

//-----------------------------------------------------------------------------
//  Name : setTwistLimits ()
/// <summary>
/// Set the allowable range of twisting rotation for this joint when limiting
/// is enabled via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgBallJoint::setTwistLimits( cgFloat minDegrees, cgFloat maxDegrees )
{
    mMinimumTwistAngle = CGEToRadian( minDegrees );
    mMaximumTwistAngle = CGEToRadian( maxDegrees );
}

//-----------------------------------------------------------------------------
//  Name : setConeLimit ()
/// <summary>
/// Set the maximum allowed rotation about the joint center when limiting
/// is enabled via 'enableLimits()'. Angles are expressed in degrees.
/// </summary>
//-----------------------------------------------------------------------------
void cgBallJoint::setConeLimit( cgFloat degrees )
{
    mMaximumConeAngle = CGEToRadian( degrees );

    // Precompute reusable values.
    mConeAngleCos = cosf( mMaximumConeAngle );
    mConeAngleSin = sinf( mMaximumConeAngle );
    mConeAngleHalfCos = cosf( mMaximumConeAngle * 0.5f );
    mConeAngleHalfSin = sinf( mMaximumConeAngle * 0.5f );
}

//-----------------------------------------------------------------------------
//  Name : getInfo () (Protected, Virtual)
/// <summary>
/// Retrieve information about the joint for serialization. Internal newton
/// requirement.
/// </summary>
//-----------------------------------------------------------------------------
void cgBallJoint::getInfo( NewtonJointRecord * info )
{
    if ( mUseLimits )
        strcpy( info->m_descriptionType, "limitballsocket" );
    else
        strcpy( info->m_descriptionType, "ballsocket" );

	info->m_attachBody_0 = mBody0->getInternalBody();
	info->m_attachBody_1 = mBody1->getInternalBody();

    info->m_minLinearDof[0] = 0.0f;
    info->m_maxLinearDof[0] = 0.0f;

    info->m_minLinearDof[1] = 0.0f;
    info->m_maxLinearDof[1] = 0.0f;;

    info->m_minLinearDof[2] = 0.0f;
    info->m_maxLinearDof[2] = 0.0f;

    info->m_minAngularDof[0] = -FLT_MAX;
    info->m_maxAngularDof[0] = FLT_MAX;
    info->m_minAngularDof[1] = -FLT_MAX;
    info->m_maxAngularDof[1] = FLT_MAX;
    info->m_minAngularDof[2] = -FLT_MAX;
    info->m_maxAngularDof[2] = FLT_MAX;

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
void cgBallJoint::submitConstraints( cgFloat fTimeStep, cgInt nThreadIndex )
{
    if ( !mUseLimits )
    {
        // Calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
        cgTransform t0, t1;
        getGlobalTransforms( mLocalPivotFrame0, mLocalPivotFrame1, t0, t1 );

        // Restrict the movement on the pivot point along all three orthonormal directions.
        NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.xAxis() );
        NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.yAxis() );
        NewtonUserJointAddLinearRow( mJoint, t0.position(), t1.position(), t0.zAxis() );

    } // End if !limited
    else
    {
        static const cgFloat MinJointPinLength = 50.0f;

        // Calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
        cgTransform t0, t1;
        getGlobalTransforms( mLocalPivotFrame0, mLocalPivotFrame1, t0, t1 );

        // Restrict the movement on the pivot point along all three orthonormal directions.
        const cgVector3 & p0 = t0.position();
        const cgVector3 & p1 = t1.position();
        NewtonUserJointAddLinearRow( mJoint, p0, p1, t0.xAxis() );
        NewtonUserJointAddLinearRow( mJoint, p0, p1, t0.yAxis() );
        NewtonUserJointAddLinearRow( mJoint, p0, p1, t0.zAxis() );

        // Fixed to a particular angle, or free to twist?
        if ((mMaximumTwistAngle - mMinimumTwistAngle) <= CGE_EPSILON)
        {
            const cgVector3 & twistDir0 = t0.zAxis();
            const cgVector3 & twistDir1 = t1.yAxis();
        
            // Construct an orthogonal coordinate system with these two vectors
            cgVector3 twistDir2, twistDir3;
            cgVector3::cross( twistDir2, twistDir1, twistDir0 );
            cgVector3::normalize( twistDir2, twistDir2 );
            cgVector3::cross( twistDir3, twistDir0, twistDir2 );

            // Get a point along the pin axis at some reasonably large distance from the pivot.
            cgVector3 q0 ( p0 + twistDir3 * MinJointPinLength );
            cgVector3 q1 ( p1 + twistDir1 * MinJointPinLength );

            // Constrain
            NewtonUserJointAddLinearRow( mJoint, q0, q1, twistDir0 );
        
        } // End if fixed
        else
        {
            cgFloat angle = 0;
            cgVector3 twistDirUp = t1.yAxis() - t0.xAxis() * cgVector3::dot( t1.yAxis(), t0.xAxis() );
            if (cgVector3::dot(twistDirUp, twistDirUp) > 0.25f )
            {
                // Up vector is good
                cgVector3 c;
                cgVector3::cross( c, t0.yAxis(), twistDirUp );
                cgFloat x = cgVector3::dot( c, t0.xAxis() );
                cgFloat y = cgVector3::dot( twistDirUp, t0.yAxis() );
			    angle = atan2f(x, y);
		    
            } // End if use up
            else
            {
                // Use alternate axis.
                cgVector3 c;
                cgVector3 twistDirRight = t1.zAxis() - t0.xAxis() * cgVector3::dot( t1.zAxis(), t0.xAxis() );
                cgVector3::cross( c, t0.zAxis(), twistDirRight );
                cgFloat x = cgVector3::dot( c, t0.xAxis() );
                cgFloat y = cgVector3::dot( twistDirRight, t0.zAxis() );
			    angle = atan2f(x, y);
		    
            } // End if use alternate

            // Clamp as necessary
	        if ( angle > mMaximumTwistAngle )
            {
			    NewtonUserJointAddAngularRow( mJoint, angle - mMaximumTwistAngle, t0.xAxis() );
			    NewtonUserJointSetRowMinimumFriction( mJoint, -0.0f );

	        } // End if > max
            else if ( angle < mMinimumTwistAngle )
            {
			    NewtonUserJointAddAngularRow( mJoint, angle - mMinimumTwistAngle, t0.xAxis() );
			    NewtonUserJointSetRowMaximumFriction( mJoint, 0.0f );
	        
            } // End if < min

        } // End if can twist

        // Now handle cone limits
        const cgVector3 & coneDir0 = t0.xAxis();
	    const cgVector3 & coneDir1 = t1.xAxis();
	    cgVector3 r0( p0 + coneDir0 * MinJointPinLength );
	    cgVector3 r1( p1 + coneDir1 * MinJointPinLength );

	    // Construct an orthogonal coordinate system with these two vectors
	    cgVector3 lateralDir;
        cgVector3::cross( lateralDir, coneDir0, coneDir1 );
        cgFloat mag2 = cgVector3::dot( lateralDir, lateralDir );
	    if ( fabsf(mag2) <= CGE_EPSILON )
        {
	        if ( mConeAngleSin <= CGE_EPSILON )
            {
			    NewtonUserJointAddLinearRow( mJoint, r0, r1, t0.yAxis() );
			    NewtonUserJointAddLinearRow( mJoint, r0, r1, t0.zAxis() );
	        
            } // End if < epsilon

	    } // End if < epsilon
        else
        {
            cgFloat cosAngle = cgVector3::dot( coneDir0, coneDir1 );
		    if ( cosAngle < mConeAngleCos )
            {
			    lateralDir /= sqrtf(mag2); // Normalize
			    cgQuaternion rot ( lateralDir.x * mConeAngleHalfSin, lateralDir.y * mConeAngleHalfSin, lateralDir.z * mConeAngleHalfSin, mConeAngleHalfCos );

                // Unrotate
                cgMatrix m;
                cgMatrix::rotationQuaternion( m, rot );
                cgVector3 c = r1 - p1;
                r1.x = p1.x + cgVector3::dot(c, (cgVector3&)m._11);
                r1.y = p1.y + cgVector3::dot(c, (cgVector3&)m._21);
                r1.z = p1.z + cgVector3::dot(c, (cgVector3&)m._31);
			    NewtonUserJointAddLinearRow( mJoint, r0, r1, lateralDir );

			    cgVector3 longitudinalDir;
                cgVector3::cross( longitudinalDir, lateralDir, t0.xAxis() );
			    NewtonUserJointAddLinearRow( mJoint, r0, r1, longitudinalDir );
			    NewtonUserJointSetRowMinimumFriction( mJoint, -0.0f );
		    
            } // End if outside tolerance
	    
        } // End if > epsilon

    } // End if limited
}