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
// Name : cgRigidBody.cpp                                                    //
//                                                                           //
// Desc : Class responsible for providing collision and/or dynamics          //
//        properties for all types of /rigid/ physics bodies. The rigid body //
//        class itself can be used for most physics entities, but can also   //
//        serve as a base class from which other types of rigid bodies       //
//        derive.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRigidBody Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Bodies/cgRigidBody.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/Shapes/cgConvexShape.h>
#include <Physics/Shapes/cgNullShape.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgRigidBody Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRigidBody () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRigidBody::cgRigidBody( cgPhysicsWorld * pWorld, cgPhysicsShape * pShape, const cgRigidBodyCreateParams & Params ) : cgPhysicsBody( pWorld, pShape )
{
    // Initialize variables to sensible defaults
    mCollisionModifier = CG_NULL;

    // Build the body
    initialize( pWorld, Params, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : cgRigidBody () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRigidBody::cgRigidBody( cgPhysicsWorld * pWorld, cgPhysicsShape * pShape, const cgRigidBodyCreateParams & Params, cgPhysicsBody * pInitBody ) : cgPhysicsBody( pWorld, pShape )
{
    // Initialize variables to sensible defaults
    mCollisionModifier = CG_NULL;

    // Build the body
    initialize( pWorld, Params, pInitBody );
}

//-----------------------------------------------------------------------------
//  Name : ~cgRigidBody () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRigidBody::~cgRigidBody()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRigidBody::dispose( bool bDisposeBase )
{
    // We are in the process of disposing
    mDisposing = true;

    // Destroy custom shape data.
    if ( mCollisionModifier )
        NewtonReleaseCollision( mWorld->getInternalWorld(), mCollisionModifier );
    
    // Clear variables
    mCollisionModifier    = CG_NULL;

    // Call base class implementation if required.
    if ( bDisposeBase == true )
        cgPhysicsBody::dispose( true );
    else
        mDisposed = true;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRigidBody::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_RigidBody )
        return true;

    // Supported by base?
    return cgPhysicsBody::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Protected)
/// <summary>
/// Initialize the rigid body based on the supplied constructiondata.
/// </summary>
//-----------------------------------------------------------------------------
void cgRigidBody::initialize( cgPhysicsWorld * pWorld, const cgRigidBodyCreateParams & Params, cgPhysicsBody * pInitBody )
{
    // Decompose the input transform into its core components.
    // We'll need to 'sanitize' it by removing shear scale.
    cgVector3 vScale, vShear;
    Params.initialTransform.decompose( vScale, vShear, mEntityRotation, mEntityPosition );
    mPrevEntityRotation = mEntityRotation;
    mPrevEntityPosition = mEntityPosition;

    // Contained shear/scale?
    bool bNonUniform = ( fabsf(1.0f - vScale.x) > CGE_EPSILON || fabsf(1.0f - vScale.y) > CGE_EPSILON || fabsf(1.0f - vScale.z) > CGE_EPSILON ||
                         fabsf(vShear.x) >= CGE_EPSILON || fabsf(vShear.y) >= CGE_EPSILON || fabsf(vShear.z) >= CGE_EPSILON );

    // If the transform contained scale or shear, we need to construct a
    // convex hull 'modifier' to allow it to be applied separately. This is *only*
    // possible if the shape is guaranteed to be convex.
    NewtonCollision * pShape = mShape->getInternalShape();
    if ( mShape->queryReferenceType( RTID_ConvexShape ) && !mShape->queryReferenceType( RTID_NullShape )  )
    {
        if ( bNonUniform )
        {
            // A collision modifier is required.
            mCollisionModifier = NewtonCreateConvexHullModifier( mWorld->getInternalWorld(), mShape->getInternalShape(), getReferenceId() );
            pShape = mCollisionModifier;

            // Build and apply the scale / shear transform to the modifier.
            mModifierTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
            NewtonConvexHullModifierSetMatrix( mCollisionModifier, (cgMatrix)mModifierTransform );

        } // End if needs modifier

    } // End if convex
    else
    {
        // Make sure that the scale and shear components are re-applied
        // when the transform is requested via getTransform(), even though
        // they will not be considered by the physics engine.
        if ( bNonUniform )
             mModifierTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
    
    } // End if 
    
    // Create the rigid body with the requested initial transform (minus shear and scale).
    cgTransform InitTransform;
    InitTransform.compose( mEntityRotation, mWorld->toPhysicsScale( mEntityPosition ) );
    mBody = NewtonCreateBody( mWorld->getInternalWorld(), pShape, (cgMatrix)InitTransform ); 

    // Set the default material
    NewtonBodySetMaterialGroupID( mBody, mWorld->getDefaultMaterialGroupId( cgDefaultPhysicsMaterialGroup::Standard ) );

    // Save this rigid body object as the user data for this body.
	NewtonBodySetUserData( mBody, this );

    // Enable body motion if requested.
    cgVector3 vOrigin;
    if ( Params.model == cgPhysicsModel::RigidDynamic )
    {
        // We need to set the proper center of mass and inertia matrix for this body.
        // It can be computed from the shape itself so do so now. The inertia is
        // stored so that the mass matrix can be recomputed on a subsequent call
        // to cgPhysicsBody::SetMass().
        NewtonConvexCollisionCalculateInertialMatrix( pShape, mInertia, vOrigin );	

	    // Set the body mass matrix.
        // Note: The inertia value calculated by the above function does not include the mass.
	    // Therefore it needs to be multiplied by the mass of the body before it is used.
	    NewtonBodySetMassMatrix( mBody, Params.mass, Params.mass * mInertia.x, Params.mass * mInertia.y, Params.mass * mInertia.z );

        // Set the body origin
	    NewtonBodySetCentreOfMass( mBody, vOrigin );

        // Set the function callback to set the transformation state of the 
        // physics entity associated with this body each time the body changes
        // position and orientation in the physics world.
	    NewtonBodySetTransformCallback( mBody, setTransformCallback );

        // Set the function callback to apply the external forces and torque to 
        // the body the most common force being gravity.
	    NewtonBodySetForceAndTorqueCallback( mBody, applyForceAndTorqueCallback );

        // If an initializing body was specified, copy its current motion details.
        if ( pInitBody )
        {
            setVelocity( pInitBody->getVelocity() );
            setAngularVelocity( pInitBody->getAngularVelocity() );
        
        } // End if init from body

        // Enable CCD if requested.
        if ( Params.quality == cgSimulationQuality::CCD )
            enableContinuousCollision( true );

    } // End if !Static motion
}

//-----------------------------------------------------------------------------
//  Name : getTransform () (Virtual)
/// <summary>
/// Retrieve the current transformation of the body within the physics 
/// world.
/// </summary>
//-----------------------------------------------------------------------------
cgTransform cgRigidBody::getTransform( cgDouble fInterp /* = 1.0 */ ) const
{
    // Call base class implementation.
    cgTransform m = cgPhysicsBody::getTransform( fInterp );

    // Re-apply any scale and skew recorded when initialized.
    return mModifierTransform * m;
}

//-----------------------------------------------------------------------------
//  Name : setTransform () (Virtual)
/// <summary>
/// Update the current transformation of the body within the physics 
/// world.
/// </summary>
//-----------------------------------------------------------------------------
void cgRigidBody::setTransform( const cgTransform & Transform )
{
    // Decompose the matrix into its core components. We also
    // update the 'cgPhysicsEntity' members directly rather than
    // calling the base class implementation in order to avoid two
    // separate decompose operations.
    cgVector3 vScale, vShear;
    Transform.decompose( vScale, vShear, mEntityRotation, mEntityPosition );
    mPrevEntityRotation = mEntityRotation;
    mPrevEntityPosition = mEntityPosition;

    // Any body yet?
    if ( !mBody )
        return;

    // Contained shear/scale?
    bool bNonUniform = ( fabsf(1.0f - vScale.x) > CGE_EPSILON || fabsf(1.0f - vScale.y) > CGE_EPSILON || fabsf(1.0f - vScale.z) > CGE_EPSILON ||
                         fabsf(vShear.x) >= CGE_EPSILON || fabsf(vShear.y) >= CGE_EPSILON || fabsf(vShear.z) >= CGE_EPSILON );

    // Set new body transform minus shear and scale.
    cgTransform NewTransform;
    NewTransform.compose( mEntityRotation, mWorld->toPhysicsScale( mEntityPosition ) );
    NewtonBodySetMatrix( mBody, (cgMatrix)NewTransform );

    // If the new transform contained scale or shear, we may need to adjust
    // the convex hull 'modifier' to allow to be applied separately assuming
    // one previously existed. If it did not, we'll need to create one. This
    // is only valid however if the shape is convex.
    if ( mShape->queryReferenceType( RTID_ConvexShape ) && !mShape->queryReferenceType( RTID_NullShape ) )
    {
        // Collision modifier already exists?
        if ( mCollisionModifier )
        {
            // Contains scale/shear?
            if ( bNonUniform )
            {
                // Update the modifier
                mModifierTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
                NewtonConvexHullModifierSetMatrix( mCollisionModifier, (cgMatrix)mModifierTransform );

                // Recompute the inertia matrix for the body based on the modifier shape
                cgVector3 vOrigin;
                cgFloat mass = getMass();
                NewtonBodyGetCentreOfMass( mBody, vOrigin );
                NewtonConvexCollisionCalculateInertialMatrix( mCollisionModifier, mInertia, vOrigin );	
                NewtonBodySetMassMatrix( mBody, mass, mass * mInertia.x, mass * mInertia.y, mass * mInertia.z );

            } // End if has scale / shear
            else
            {
                // The hull modifier can now be removed.
                NewtonBodySetCollision( mBody, mShape->getInternalShape() );
                NewtonReleaseCollision( mWorld->getInternalWorld(), mCollisionModifier );
                mCollisionModifier = CG_NULL;
                
                // Reset the internal modifier transform so that we don't
                // re-scale / shear the transform in response to a 'getTransform()'
                mModifierTransform.identity();

                // Recompute the inertia matrix for the body based on the original shape
                cgVector3 vOrigin;
                cgFloat mass = getMass();
                NewtonBodyGetCentreOfMass( mBody, vOrigin );
                NewtonConvexCollisionCalculateInertialMatrix( mShape->getInternalShape(), mInertia, vOrigin );	
                NewtonBodySetMassMatrix( mBody, mass, mass * mInertia.x, mass * mInertia.y, mass * mInertia.z );
                
            } // End if no scale / shear

        } // End if already exists
        else
        {
            // Contains scale / shear?
            if ( bNonUniform )
            {
                // A new hull modifier is required because scale / shear was detected.
                mCollisionModifier = NewtonCreateConvexHullModifier( mWorld->getInternalWorld(), mShape->getInternalShape(), getReferenceId() );

                // Build and apply the scale / shear transform to the modifier.
                mModifierTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
                NewtonConvexHullModifierSetMatrix( mCollisionModifier, (cgMatrix)mModifierTransform );

                // Attach the modifier to the body.
                NewtonBodySetCollision( mBody, mCollisionModifier );

                // Recompute the inertia matrix for the body based on the modifier shape
                cgVector3 vOrigin;
                cgFloat mass = getMass();
                NewtonBodyGetCentreOfMass( mBody, vOrigin );
                NewtonConvexCollisionCalculateInertialMatrix( mCollisionModifier, mInertia, vOrigin );	
                NewtonBodySetMassMatrix( mBody, mass, mass * mInertia.x, mass * mInertia.y, mass * mInertia.z );

            } // End if has scale / shear
            else
            {
                // No processing is required.
                mModifierTransform.identity();
                
            } // End if no scale / shear

        } // End if no modifier

    } // End if convex
    else
    {
        // Make sure that the scale and shear components are re-applied
        // when the transform is requested via getTransform(), even though
        // they will not be considered by the physics engine.
        if ( bNonUniform )
            mModifierTransform.compose( vScale, vShear, cgQuaternion(0,0,0,1), cgVector3(0,0,0) );
        else
            mModifierTransform.identity();

    } // End if !convex

    // Notify listeners of new transform NOT due to dynamics update.
    if ( !mEventListeners.empty() )
        onPhysicsBodyTransformed( &cgPhysicsBodyTransformedEventArgs( Transform, false ) );
}

//-----------------------------------------------------------------------------
//  Name : setTransformCallback () (Private, Static)
/// <summary>
/// Callback function, called by Newton whenever the transformation of a rigid
/// body is updated.
/// </summary>
//-----------------------------------------------------------------------------
void cgRigidBody::setTransformCallback( const NewtonBody * pBody, const cgFloat * pMatrix, cgInt nThreadIndex )
{
    // Retrieve the Carbon side rigid body to which this callback applies.
    cgRigidBody * pThis = (cgRigidBody*)NewtonBodyGetUserData( pBody );

    // Retrieve the position.
    cgVector3 vPosition = pThis->mWorld->fromPhysicsScale( (cgVector3&)((cgMatrix*)pMatrix)->_41 );

    // And the orientation (ignore the value in the matrix)
    cgFloat fRotation[4];
	NewtonBodyGetRotation( pBody, fRotation );
    cgQuaternion qRotation(fRotation[1], fRotation[2], fRotation[3], fRotation[0] );
    
    // In case the physics system is running at a different rate
    // to the renderer, cache the previous position and orientation
    // so that we can interpolate between the two when retrieving
    // the current transform for rendering.
    pThis->mPrevEntityPosition = pThis->mEntityPosition;
    pThis->mPrevEntityRotation = pThis->mEntityRotation;

    // Ensure that the rotation is aligned.
    if ( cgQuaternion::dot( pThis->mEntityRotation, qRotation ) < 0.0f )
        pThis->mPrevEntityRotation *= -1.0f;

    // Set the new position and orientation of this body.
    pThis->mEntityPosition = vPosition;
    pThis->mEntityRotation = qRotation;

    // Send notifications?
    if ( !pThis->mEventListeners.empty() )
    {
        // Generate the new transform object.
        cgTransform NewTransform;
        NewTransform.compose( qRotation, vPosition );

        // Re-apply any scale and skew recorded when initialized and 
        // then notify any listeners.
        pThis->onPhysicsBodyTransformed( &cgPhysicsBodyTransformedEventArgs( pThis->mModifierTransform * NewTransform, true ) );

    } // End if has listeners
}

//-----------------------------------------------------------------------------
//  Name : applyForceAndTorqueCallback () (Private, Static)
/// <summary>
/// Callback function, called by Newton whenever it needs to query for alist
/// of forces which are acting upon the specified body.
/// </summary>
//-----------------------------------------------------------------------------
void cgRigidBody::applyForceAndTorqueCallback( const NewtonBody * pBody, cgFloat fTimeStep, cgInt nThreadIndex )
{
    // Retrieve the Carbon side rigid body to which this callback applies.
    cgRigidBody * pThis = (cgRigidBody*)NewtonBodyGetUserData( pBody );

    // Compute the force of gravity.
    cgVector3 vGravityForce;
    cgFloat fIxx, fIyy, fIzz, fMass;
	NewtonBodyGetMassMatrix( pBody, &fMass, &fIxx, &fIyy, &fIzz);
    if ( pThis->mUseCustomGravity )
        vGravityForce = pThis->mCustomGravity * fMass;
    else
	    vGravityForce = pThis->mWorld->getDefaultGravity() * fMass;

    // Apply forces.
	NewtonBodySetForce( pBody, pThis->mWorld->toPhysicsScale( (pThis->mTotalForce + vGravityForce) ) );
    NewtonBodySetTorque( pBody, pThis->mTotalTorque );
}