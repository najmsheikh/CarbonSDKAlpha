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
// Name : cgPhysicsJoint.cpp                                                 //
//                                                                           //
// Desc : Base class from which specific physics joints should derive.       //
//        Joints provide the means by which a body can be constrained to     //
//        a particular location, orientation, bound to another body and      //
//        more. A simple example joint might be a hinge for a door.          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsJoint Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsJoint.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsBody.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsJoint Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsJoint () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsJoint::cgPhysicsJoint( cgPhysicsWorld * pWorld, cgUInt32 nMaxDoF, cgPhysicsBody * pBody0, cgPhysicsBody * pBody1 ) : cgReference( cgReferenceManager::generateInternalRefId() )
{
    // Initialize variables to sensible defaults
    mWorld            = pWorld;
    mJoint            = CG_NULL;
    mBody0            = pBody0;
    mBody1            = pBody1;
    mMaxDoF           = nMaxDoF;
    mBodyCollision    = false;

    // Create the newton user joint
    mJoint = NewtonConstraintCreateUserJoint( pWorld->getInternalWorld(), nMaxDoF, submitConstraints, getInfo,
                                                (pBody0) ? pBody0->getInternalBody() : CG_NULL, 
                                                (pBody1) ? pBody1->getInternalBody() : CG_NULL );

    // Apply user data and destructor so that we get notified
    // whenever newton destroys its representation of the joint.
    NewtonJointSetUserData( mJoint, this );
    NewtonJointSetDestructor( mJoint, autoDestructor );

    // Enable body collision as requested
    NewtonJointSetCollisionState( mJoint, (mBodyCollision) ? 1 : 0 );

    // Hold references to the supplied bodies.
    if ( mBody0 )
        mBody0->addReference( this );
    if ( mBody1 )
        mBody1->addReference( this );
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsJoint () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsJoint::~cgPhysicsJoint()
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
void cgPhysicsJoint::dispose( bool bDisposeBase )
{
    // Release wrapped newton shape.
    if ( mJoint )
        NewtonDestroyJoint( mWorld->getInternalWorld(), mJoint );
    
    // Release our references to the supplied bodies.
    if ( mBody0 )
        mBody0->removeReference( this );
    if ( mBody1 )
        mBody1->removeReference( this );

    // Clear variables
    mJoint = CG_NULL;
    mBody0 = CG_NULL;
    mBody1 = CG_NULL;

    // Call base if requested.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsJoint::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PhysicsJoint )
        return true;

    // Unsupported
    return false;
}

//-----------------------------------------------------------------------------
//  Name : enableBodyCollision ()
/// <summary>
/// Enable or disable the ability for the bodies connected via this joint to
/// collide with one another. Default values if 'false'.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::enableBodyCollision( bool bEnable )
{
    if ( mJoint )
        NewtonJointSetCollisionState( mJoint, (bEnable) ? 1 : 0 );
    mBodyCollision = bEnable;
}

//-----------------------------------------------------------------------------
//  Name : isBodyCollisionEnabled ()
/// <summary>
/// Retrieve the state that describes whether or not the bodies connected via 
/// this joint are able to collide with one another.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsJoint::isBodyCollisionEnabled( ) const
{
    return mBodyCollision;
}

//-----------------------------------------------------------------------------
//  Name : getBody0()
/// <summary>
/// Retrieve the first of two bodies to which this joint is attached.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsBody * cgPhysicsJoint::getBody0( )
{
    return mBody0;
}

//-----------------------------------------------------------------------------
//  Name : getBody1()
/// <summary>
/// Retrieve the second of two bodies to which this joint is attached.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsBody * cgPhysicsJoint::getBody1( )
{
    return mBody1;
}

//-----------------------------------------------------------------------------
//  Name : autoDestructor() (Private, Static)
/// <summary>
/// Callback triggered when newton destroys its local representation of the
/// joint.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::autoDestructor( const NewtonJoint * pJoint )
{
    cgPhysicsJoint * pThis = (cgPhysicsJoint*)NewtonJointGetUserData( pJoint );
    pThis->mJoint = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getInfo() (Private, Static)
/// <summary>
/// Callback triggered when newton wants to retrieve information about this
/// joint and its properties.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::getInfo( const NewtonJoint * pJoint, NewtonJointRecord * pInfo )
{
    cgPhysicsJoint * pThis = (cgPhysicsJoint*)NewtonJointGetUserData( pJoint );
    pThis->getInfo( pInfo );
}

//-----------------------------------------------------------------------------
//  Name : submitConstraints() (Private, Static)
/// <summary>
/// Callback triggered when newton wants us to submit constraints to the 
/// simulation.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::submitConstraints( const NewtonJoint * pJoint, cgFloat fTimeStep, cgInt nThreadIndex )
{
    cgPhysicsJoint * pThis = (cgPhysicsJoint*)NewtonJointGetUserData( pJoint );
    pThis->submitConstraints( fTimeStep, nThreadIndex );
}

//-----------------------------------------------------------------------------
//  Name : grammSchmidtBasis() (Protected, Static)
/// <summary>
/// Generate a transformation whose X axis vector is aligned to the specified
/// direction using Gramm-Schmidt.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::grammSchmidtBasis( const cgVector3 & vDir, cgTransform & t )
{
    cgVector3 basisX, basisY, basisZ;
    cgVector3::normalize( basisX, vDir );
    if ( fabsf(basisX.z) > 0.577f)
        cgVector3::cross( basisZ, basisX, cgVector3(-basisX.y, basisX.z, 0.0f) );
    else
        cgVector3::cross( basisZ, basisX, cgVector3(-basisX.y, basisX.x, 0.0f) );
    cgVector3::normalize( basisZ, basisZ );
    cgVector3::cross( basisY, basisZ, basisX );
    t.setOrientation( basisX, basisY, basisZ );
}

//-----------------------------------------------------------------------------
//  Name : getLocalTransforms() (Protected)
/// <summary>
/// Compute local transformations within the space of each connected body 
/// for the specified pin and pivot frame.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::getLocalTransforms( const cgTransform & tPinsAndPivotFrame, cgTransform & tLocal0, cgTransform & tLocal1 ) const
{
    cgMatrix m0, m1;
    cgMatrix::identity( m1 );

    // Get the global matrices of each rigid body.
    NewtonBodyGetMatrix( mBody0->getInternalBody(), m0 );
    if ( mBody1 )
        NewtonBodyGetMatrix( mBody1->getInternalBody(), m1 );
    
    // calculate the relative matrix of the pin and pivot on each body
    cgTransform::inverse( tLocal0, m0 );
    cgTransform::inverse( tLocal1, m1 );
    tLocal0 = tPinsAndPivotFrame * tLocal0;
    tLocal1 = tPinsAndPivotFrame * tLocal1;
}

//-----------------------------------------------------------------------------
//  Name : getGlobalTransforms() (Protected)
/// <summary>
/// Convert the local transformations into global space with respect each 
/// connected body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsJoint::getGlobalTransforms( const cgTransform & tLocal0, const cgTransform & tLocal1, cgTransform & tGlobal0, cgTransform & tGlobal1 ) const
{
    cgMatrix m0, m1;
    cgMatrix::identity( m1 );

    // Get the global matrices of each rigid body.
    NewtonBodyGetMatrix( mBody0->getInternalBody(), m0 );
    if ( mBody1 )
        NewtonBodyGetMatrix( mBody1->getInternalBody(), m1 );

    // Transform local to global
    tGlobal0 = tLocal0 * m0;
    tGlobal1 = tLocal1 * m1;
}