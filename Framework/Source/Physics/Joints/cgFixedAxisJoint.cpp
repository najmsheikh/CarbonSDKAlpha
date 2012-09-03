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
// Name : cgFixedAxisJoint.cpp                                               //
//                                                                           //
// Desc : Class implementing a simple axis pinning joint. This joint can be  //
//        used to 'fix' the orientation of a body to a specific axis while   //
//        allowing complete freedom to translate in any way required.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgFixedAxisJoint Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Joints/cgFixedAxisJoint.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgFixedAxisJoint Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgFixedAxisJoint () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFixedAxisJoint::cgFixedAxisJoint( cgPhysicsWorld * pWorld, cgPhysicsBody * pBody, const cgVector3 & vWorldSpaceAxis ) : cgPhysicsJoint( pWorld, 2, pBody, CG_NULL )
{
    // Initialize variables to sensible defaults.
    mAxis = vWorldSpaceAxis;

    // Get the initial position and orientation of the body.
    cgMatrix m;
    NewtonBodyGetMatrix( pBody->getInternalBody(), m );
    cgTransform tBody = m;

    // Generate a local orientation pin/pivot frame for this direction.
    cgTransform tPivotFrame;
    grammSchmidtBasis( vWorldSpaceAxis, tPivotFrame );
    tPivotFrame.setPosition( tBody.position() );

    // Convert pivot frame into the space of the connected body.
    // Since there is no second body, when this function returns,
    // mLocalPivotFrame will contain the pivot frame with respect
    // the constrained rigid body, and mGlobalPivotFrame will contain
    // the original.
    getLocalTransforms( tPivotFrame, mLocalPivotFrame, mGlobalPivotFrame );
}

//-----------------------------------------------------------------------------
//  Name : ~cgFixedAxisJoint () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgFixedAxisJoint::~cgFixedAxisJoint()
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
void cgFixedAxisJoint::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

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
bool cgFixedAxisJoint::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_FixedAxisJoint )
        return true;

    // Supported by base?
    return cgPhysicsJoint::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : setAxis ()
/// <summary>
/// Update the world space axis to which the connected body will be constrained.
/// Altering this axis after creation will cause the body to rotate such that
/// the *original* axis of constraint is now aligned to this new axis unless
/// a value of 'true' is specified to the 'bReset' parameter.
/// </summary>
//-----------------------------------------------------------------------------
void cgFixedAxisJoint::setAxis( const cgVector3 & vAxis, bool bReset )
{
    mAxis = vAxis;
    if ( !bReset )
    {
        // We're not resetting a new axis, we're simply altering
        // the existing one (i.e. rotating the body).
        grammSchmidtBasis( vAxis, mGlobalPivotFrame );

    } // End if !reset
    else
    {
        // Generate a brand new local pivot frame. The body
        // will not be rotated. First we need to get the new
        // *current* position and orientation of the body.
        cgMatrix m;
        NewtonBodyGetMatrix( mBody0->getInternalBody(), m );
        cgTransform tBody = m;

        // Generate a local orientation pin/pivot frame for this direction.
        cgTransform tPivotFrame;
        grammSchmidtBasis( mAxis, tPivotFrame );
        tPivotFrame.setPosition( tBody.position() );

        // Convert pivot frame into the space of the connected body.
        // Since there is no second body, when this function returns,
        // mLocalPivotFrame will contain the pivot frame with respect
        // the constrained rigid body, and mGlobalPivotFrame will contain
        // the original.
        getLocalTransforms( tPivotFrame, mLocalPivotFrame, mGlobalPivotFrame );

    } // End if reset
}

//-----------------------------------------------------------------------------
//  Name : getAxis ()
/// <summary>
/// Get the world space axis to which the child body will be constrained.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgFixedAxisJoint::getAxis( ) const
{
    return mAxis;
}

//-----------------------------------------------------------------------------
//  Name : getInfo () (Protected, Virtual)
/// <summary>
/// Retrieve information about the joint for serialization. Internal newton
/// requirement.
/// </summary>
//-----------------------------------------------------------------------------
void cgFixedAxisJoint::getInfo( NewtonJointRecord * pInfo )
{
    strcpy( pInfo->m_descriptionType, "fixedAxis" );

	pInfo->m_attachBody_0 = mBody0->getInternalBody();
	pInfo->m_attachBody_1 = CG_NULL;

	pInfo->m_minLinearDof[0] = -FLT_MAX;
	pInfo->m_maxLinearDof[0] = FLT_MAX;

	pInfo->m_minLinearDof[1] = -FLT_MAX;
	pInfo->m_maxLinearDof[1] = FLT_MAX;

	pInfo->m_minLinearDof[2] = -FLT_MAX;
	pInfo->m_maxLinearDof[2] = FLT_MAX;

	pInfo->m_minAngularDof[0] = -FLT_MAX;
	pInfo->m_maxAngularDof[0] = FLT_MAX;

	pInfo->m_minAngularDof[1] = 0.0f;
	pInfo->m_maxAngularDof[1] = 0.0f;

	pInfo->m_minAngularDof[2] = 0.0f;
	pInfo->m_maxAngularDof[2] = 0.0f;

    pInfo->m_bodiesCollisionOn = (mBodyCollision) ? 1 : 0;

	memcpy (pInfo->m_attachmenMatrix_0, &((cgMatrix)mLocalPivotFrame), sizeof(cgMatrix));
	memcpy (pInfo->m_attachmenMatrix_1, &((cgMatrix)mLocalPivotFrame), sizeof(cgMatrix));
}

//-----------------------------------------------------------------------------
//  Name : submitConstraints() (Protected, Virtual)
/// <summary>
/// Callback triggered when we need to submit constraints to the simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgFixedAxisJoint::submitConstraints( cgFloat fTimeStep, cgInt nThreadIndex )
{
    // Calculate the position of the pivot point and the Jacobian direction vectors, in global space. 
    cgTransform t0, t1;
    getGlobalTransforms( mLocalPivotFrame, mGlobalPivotFrame, t0, t1 );

    // If the body has rotated by some amount, there will be a plane of rotation.
    cgVector3 vLateralDir;
    cgVector3::cross( vLateralDir, t0.xAxis(), t1.xAxis() );
    cgFloat fMag = cgVector3::lengthSq( vLateralDir );
    if ( fMag > 1.0e-6f )
    {
        // If the side vector is not zero, it means the body has rotated.
        fMag = sqrtf( fMag );
        vLateralDir *= (1.0f / fMag);
        cgFloat fAngle = asinf( fMag );

        // Add an angular constraint to correct the error angle
        NewtonUserJointAddAngularRow( mJoint, fAngle, vLateralDir );

        // In theory only one correction is needed, but this produces instability 
        // as the body may move sideways. A lateral correction prevents this from happening.
        cgVector3::cross( vLateralDir, vLateralDir, t0.xAxis() );
        NewtonUserJointAddAngularRow( mJoint, 0.0f, vLateralDir );
    
    } // End if > 0
    else
    {
        // If the angle error is very small then two angular correction along the 
        // plane axis should do the trick.
        NewtonUserJointAddAngularRow( mJoint, 0.0f, t0.yAxis() );
        NewtonUserJointAddAngularRow( mJoint, 0.0f, t0.zAxis() );
    
    } // End if == 0
}