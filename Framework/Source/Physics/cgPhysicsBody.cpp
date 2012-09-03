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
// Name : cgPhysicsBody.cpp                                                  //
//                                                                           //
// Desc : Base class from which specific types of physics bodys should       //
//        derive. This base does not enforce any specific type of body or    //
//        dynamics properties (such as rigid vs. soft) and is instead        //
//        responsible solely for providing access to common properties and   //
//        functionality.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgPhysicsBody Module Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsBody.h>
#include <Physics/cgPhysicsWorld.h>
#include <Physics/cgPhysicsShape.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgPhysicsBody Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgPhysicsBody () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsBody::cgPhysicsBody( cgPhysicsWorld * pWorld, cgPhysicsShape * pShape ) : cgPhysicsEntity( pWorld )
{
    // Initialize variables to sensible defaults
    mBody             = CG_NULL;
    mShape            = pShape;
    mInertia          = cgVector3( 0, 0, 0 );
    mTotalForce       = cgVector3( 0, 0, 0 );
    mTotalTorque      = cgVector3( 0, 0, 0 );
    mCustomGravity    = cgVector3( 0, -9.8f, 0 );
    mUseCustomGravity = false;

    // We now hold a reference to the supplied shape.
    if ( mShape )
    {
        mShape->addReference( this );
        
        // Add the shape to the shape cache.
        //pWorld->AddShapeToCache( mShape );
    
    } // End if valid
}

//-----------------------------------------------------------------------------
//  Name : ~cgPhysicsBody () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsBody::~cgPhysicsBody()
{
    // Clear variables
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgPhysicsBody::dispose( bool bDisposeBase )
{
    // We are in the process of disposing
    mDisposing = true;

    // Remove our reference to the body's shape(s).
    if ( mShape )
    {
        // Remove the shape from the shape cache too if we are 
        // holding the last live reference to it.
        /*if ( mShape->GetReferenceCount( true ) == 1 )
            mWorld->RemoveShapeFromCache( mShape );*/
        mShape->removeReference( this );
    
    }  // End if valid

    // Remove our reference to the body itself.
    if ( mBody )
        NewtonDestroyBody( mWorld->getInternalWorld(), mBody );
    
    // Clear variables
    mShape = CG_NULL;
    mBody  = CG_NULL;

    // Call base class implementation if required.
    if ( bDisposeBase == true )
        cgPhysicsEntity::dispose( true );
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
bool cgPhysicsBody::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_PhysicsBody )
        return true;

    // Supported by base?
    return cgPhysicsEntity::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getVelocity ()
/// <summary>
/// Retrieve the current linear velocity of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgPhysicsBody::getVelocity( ) const
{
    cgVector3 v;
    NewtonBodyGetVelocity( mBody, v );
    return mWorld->fromPhysicsScale( v );
}

//-----------------------------------------------------------------------------
//  Name : getAngularVelocity ()
/// <summary>
/// Retrieve the current angular velocity (omega) of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgPhysicsBody::getAngularVelocity( ) const
{
    cgVector3 v;
    NewtonBodyGetOmega( mBody, v );
    return v;
}

//-----------------------------------------------------------------------------
//  Name : setVelocity ()
/// <summary>
/// Set the current linear velocity of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::setVelocity( const cgVector3 & v )
{
    NewtonBodySetVelocity( mBody, mWorld->toPhysicsScale( v ) );
}

//-----------------------------------------------------------------------------
//  Name : setAngularVelocity ()
/// <summary>
/// Set the current angular velocity (omega) of the physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::setAngularVelocity( const cgVector3 & v )
{
    NewtonBodySetOmega( mBody, v );
}

//-----------------------------------------------------------------------------
//  Name : setMass ()
/// <summary>
/// Update the overall mass of this physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::setMass( cgFloat fMass )
{
    NewtonBodySetMassMatrix( mBody, fMass, fMass * mInertia.x, fMass * mInertia.y, fMass * mInertia.z );
}

//-----------------------------------------------------------------------------
//  Name : getMass ()
/// <summary>
/// Retrieve the overall mass of this physics body.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgPhysicsBody::getMass( ) const
{
    cgFloat mass, ixx, iyy, izz;
    NewtonBodyGetMassMatrix( mBody, &mass, &ixx, &iyy, &izz );
    return mass;
}


//-----------------------------------------------------------------------------
//  Name : clearForces () (Virtual)
/// <summary>
/// Clear out any accumulated forces that have been processed.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::clearForces( )
{
    mTotalForce.x = mTotalForce.y = mTotalForce.z = 0;
    mTotalTorque.x = mTotalTorque.y = mTotalTorque.z = 0;
}

//-----------------------------------------------------------------------------
//  Name : applyForce ()
/// <summary>
/// Apply a force directly to the center of mass of the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyForce( const cgVector3 & vForce )
{
    mTotalForce += vForce;
}

//-----------------------------------------------------------------------------
//  Name : applyForce ()
/// <summary>
/// Apply a force to the body at the specified world space location.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyForce( const cgVector3 & vForce, const cgVector3 & vAt )
{
    // Accumulate force
    mTotalForce += vForce;

    // Compute center of mass for the body
    cgMatrix m;
    cgVector3 vCenter;
    NewtonBodyGetMatrix( mBody, m );
    NewtonBodyGetCentreOfMass( mBody, vCenter );
    cgVector3::transformCoord( vCenter, vCenter, m );
    vCenter *= mWorld->fromPhysicsScale();

    // Compute final torque
    cgVector3 vTorque;
    cgVector3::cross( vTorque, vAt-vCenter, vForce );

    // Accumulate torque
    mTotalTorque += vTorque;
}

//-----------------------------------------------------------------------------
//  Name : applyImpulse ()
/// <summary>
/// Apply an impulse directly to the center of mass of the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyImpulse( const cgVector3 & vImpulse )
{
    cgVector3 vLinearVelocity;
    NewtonBodyGetVelocity( mBody, vLinearVelocity );
    vLinearVelocity += mWorld->toPhysicsScale( vImpulse );
    NewtonBodySetVelocity( mBody, vLinearVelocity );
}

//-----------------------------------------------------------------------------
//  Name : applyImpulse ()
/// <summary>
/// Apply an impulse to the body at the specified world space location.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyImpulse( const cgVector3 & vImpulse, const cgVector3 & vAt )
{
    // Get current velocities
    cgVector3 vLinearVelocity, vAngularVelocity;
    NewtonBodyGetVelocity( mBody, vLinearVelocity );
    NewtonBodyGetOmega( mBody, vAngularVelocity );

    // Apply linear impulse component
    vLinearVelocity += mWorld->toPhysicsScale( vImpulse );

    // Compute center of mass for the body
    cgMatrix m;
    cgVector3 vCenter;
    NewtonBodyGetMatrix( mBody, m );
    NewtonBodyGetCentreOfMass( mBody, vCenter );
    cgVector3::transformCoord( vCenter, vCenter, m );
    vCenter *= mWorld->fromPhysicsScale();

    // Compute final torque
    cgVector3 vTorque;
    cgVector3::cross( vTorque, vAt-vCenter, vImpulse );
    
    // Apply rotational impulse component
    vAngularVelocity += vTorque;

    // Set back to the body
    NewtonBodySetVelocity( mBody, vLinearVelocity );
    NewtonBodySetOmega( mBody, vAngularVelocity );
}

//-----------------------------------------------------------------------------
//  Name : applyTorque()
/// <summary>
/// Apply a torque force to the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyTorque( const cgVector3 & vTorque )
{
    mTotalTorque += vTorque;
}

//-----------------------------------------------------------------------------
//  Name : applyTorqueImpulse()
/// <summary>
/// Apply a torque impulse to the body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::applyTorqueImpulse( const cgVector3 & vTorqueImpulse )
{
    cgVector3 vAngularVelocity;
    NewtonBodyGetOmega( mBody, vAngularVelocity );
    vAngularVelocity += vTorqueImpulse;
    NewtonBodySetOmega( mBody, vAngularVelocity );
}

//-----------------------------------------------------------------------------
//  Name : setCustomGravity ()
/// <summary>
/// Set the custom gravity force to use for this body. This custom force will
/// only be applied when enabled via the 'enableCustomGravity()' method. When
/// disabled, the global gravity force will be applied.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::setCustomGravity( const cgVector3 & vForce )
{
    mCustomGravity = vForce;
}

//-----------------------------------------------------------------------------
//  Name : getCustomGravity ()
/// <summary>
/// Retrieve the custom gravity force in use for this body. This custom force
/// will only be applied when enabled via the 'enableCustomGravity()' method.
/// When disabled, the global gravity force will be applied.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgPhysicsBody::getCustomGravity( ) const
{
    return mCustomGravity;
}

//-----------------------------------------------------------------------------
//  Name : enableCustomGravity ()
/// <summary>
/// Enable or disable the use of the custom gravity force. When disabled, the 
/// global gravity force will be applied.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::enableCustomGravity( bool bEnabled )
{
    mUseCustomGravity = bEnabled;
}

//-----------------------------------------------------------------------------
//  Name : isCustomGravityEnabled ()
/// <summary>
/// Retrieve the state of the custom gravity force enabled state.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsBody::isCustomGravityEnabled( ) const
{
    return mUseCustomGravity;
}

//-----------------------------------------------------------------------------
//  Name : enableAutoSleep ()
/// <summary>
/// Enable or disable the automatic sleep feature for this physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::enableAutoSleep( bool bEnabled )
{
    NewtonBodySetAutoSleep( mBody, (bEnabled) ? 1 : 0 );
}

//-----------------------------------------------------------------------------
//  Name : isAutoSleepEnabled ()
/// <summary>
/// Determine if the automatic sleep feature is enabled for this physics body.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsBody::isAutoSleepEnabled( ) const
{
    return (NewtonBodyGetAutoSleep( mBody ) != 0);
}

//-----------------------------------------------------------------------------
//  Name : enableContinuousCollision ()
/// <summary>
/// Enable or disable the continuous collision detection feature for this 
/// physics body.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::enableContinuousCollision( bool bEnabled )
{
    NewtonBodySetContinuousCollisionMode( mBody, (bEnabled) ? 1 : 0 );
}

//-----------------------------------------------------------------------------
//  Name : isContinuousCollisionEnabled ()
/// <summary>
/// Determine if the continuous collision detection feature is enabled for this 
/// physics body.
/// </summary>
//-----------------------------------------------------------------------------
bool cgPhysicsBody::isContinuousCollisionEnabled( ) const
{
    return (NewtonBodyGetContinuousCollisionMode( mBody ) != 0);
}

//-----------------------------------------------------------------------------
//  Name : getShape ()
/// <summary>
/// Retrieve the physics / collision shape currently in use by this body.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsShape * cgPhysicsBody::getShape( ) const
{
    return mShape;
}

//-----------------------------------------------------------------------------
//  Name : getInternalBody()
/// <summary>
/// Retrieve the internal body object specific to Newton. This is not
/// technically part of the public interface and should not be called
/// by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
NewtonBody * cgPhysicsBody::getInternalBody( ) const
{
    return mBody;
}

//-----------------------------------------------------------------------------
//  Name : onPhysicsBodyTransformed() (Virtual)
/// <summary>
/// When the body is transformed, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgPhysicsBody::onPhysicsBodyTransformed( cgPhysicsBodyTransformedEventArgs * e )
{
    // Trigger 'onPhysicsBodyTransformed' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        (static_cast<cgPhysicsBodyEventListener*>(*itListener))->onPhysicsBodyTransformed( this, e );
}