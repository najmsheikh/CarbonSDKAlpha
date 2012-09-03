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
// Name : cgCharacterController.cpp                                          //
//                                                                           //
// Desc : Contains classes which provide physics managenent for character    //
//        class objects such as the player or other NPCs.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCharacterController Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Controllers/cgCharacterController.h>
#include <Physics/cgPhysicsWorld.h>
#include <World/cgObjectNode.h>
#include <Math/cgMathTypes.h>
#include <Physics/Bodies/cgRigidBody.h>
#include <Physics/Joints/cgFixedAxisJoint.h>
#include <Physics/Shapes/cgCylinderShape.h>
#include <Physics/Shapes/cgConvexHullShape.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
namespace
{    
    //-----------------------------------------------------------------------------
    //  Name : preProcessContacts () (Static, Protected)
    /// <summary>
    /// Flatten all contacts onto the plane described by the up axis vector and
    /// also reject duplicates.
    /// </summary>
    //-----------------------------------------------------------------------------
    cgInt preProcessContacts( NewtonWorldConvexCastReturnInfo * const contacts, cgInt contactCount, const cgVector3 & upDirection )
    {
	    // Flatten contacts onto the plane described by the up axis.
        // This ensures that no vertical "slide" is introduced.
	    for ( cgInt i = 0; i < contactCount; ++i )
        {
		    cgVector3 normal( contacts[i].m_normal );
		    normal -= upDirection * cgVector3::dot( upDirection, normal );
            cgVector3::normalize( normal, normal );
		    contacts[i].m_normal[0] = normal.x;
		    contacts[i].m_normal[1] = normal.y;
		    contacts[i].m_normal[2] = normal.z;
    	
        } // Next Contact

	    // Remove contacts with the same normal
	    for ( cgInt i = 0; i < (contactCount - 1); ++i )
        {
		    const cgVector3 & normal0 = *((cgVector3*)(&contacts[i].m_normal[0])); 
		    for ( cgInt j = i + 1; j < contactCount; ++j )
            {
			    const cgVector3 & normal1 = *((cgVector3*)(&contacts[j].m_normal[0])); 
			    cgFloat fDot = cgVector3::dot( normal0, normal1 );
			    if ( fDot > 0.9995f )
                {
				    --contactCount;
				    contacts[j] = contacts[contactCount];
				    --j;
    			
                } // End if flat
    		
            } // Next Contact
    	
        } // Next Contact
    	
        return contactCount;
    }

    //-----------------------------------------------------------------------------
    //  Name : convexCastPreFilter () (Static, Protected)
    /// <summary>
    /// Callback used to reject any bodies in the supplied set during a convex
    /// cast procedure.
    /// </summary>
    //-----------------------------------------------------------------------------
    cgUInt convexCastPreFilter( const NewtonBody * body, const NewtonCollision * collision, void * userData )
    {
        // Skip any bodies that occur in the set.
        std::set<const NewtonBody*> & skipSet = *(std::set<const NewtonBody*>*)userData;
        return ( skipSet.find( body ) == skipSet.end() );
    }

    //-----------------------------------------------------------------------------
    //  Name : convexCastPreFilterStaticOnly () (Static, Protected)
    /// <summary>
    /// Callback used to reject any bodies in the supplied set in addition to
    /// rejecting any dynamic bodies during a convex cast procedure.
    /// </summary>
    //-----------------------------------------------------------------------------
    cgUInt convexCastPreFilterStaticOnly( const NewtonBody * body, const NewtonCollision * collision, void * userData )
    {
        // If the body has mass, then it's dynamic and should be rejected.
        cgFloat mass, ixx, iyy, izz;
        NewtonBodyGetMassMatrix( body, &mass, &ixx, &iyy, &izz );
        if ( mass > 0 )
            return 0;
        
        // Skip any bodies that occur in the set.
        std::set<const NewtonBody*> & skipSet = *(std::set<const NewtonBody*>*)userData;
        return ( skipSet.find( body ) == skipSet.end() );
    }

    //-----------------------------------------------------------------------------
    //  Name : convexCastPreFilterDynamicOnly () (Static, Protected)
    /// <summary>
    /// Callback used to reject any bodies in the supplied set in addition to
    /// rejecting any static bodies during a convex cast procedure.
    /// </summary>
    //-----------------------------------------------------------------------------
    cgUInt convexCastPreFilterDynamicOnly( const NewtonBody * body, const NewtonCollision * collision, void * userData )
    {
        // If the body has no mass, then it's static and should be rejected.
        cgFloat mass, ixx, iyy, izz;
        NewtonBodyGetMassMatrix( body, &mass, &ixx, &iyy, &izz );
        if ( mass <= 0 )
            return 0;
        
        // Skip any bodies that occur in the set.
        std::set<const NewtonBody*> & skipSet = *(std::set<const NewtonBody*>*)userData;
        return ( skipSet.find( body ) == skipSet.end() );
    }

} // End Unnamed Namespace


///////////////////////////////////////////////////////////////////////////////
// cgCharacterController Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCharacterController () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCharacterController::cgCharacterController( cgPhysicsWorld * world ) : cgPhysicsController( world )
{
    // Initialize variables to sensible defaults
    mBody                 = CG_NULL;
    mUpJoint              = CG_NULL;
    mDynamicsSensorShape  = CG_NULL;
    mFloorSensorShape     = CG_NULL;
    mBodySensorShape      = CG_NULL;
    
    // Properties
    mKinematicCushion     = 3.0f/64.0f;    
    mCharacterHeight      = 1.8f;          // 1.8 meters
    mCharacterRadius      = 0.3f;          // 30cm
    mCharacterMass        = 80.0f;         // 80kg (roughly average for male)
    mCharacterOffset      = 0.0f;
    mMaxStepHeight        = 0.4f;          // 40cm
    mMaxSlope             = 45.0f;         // 45 degrees
    mWalkSpeed            = 1.34112f * 4;  // Meters per second (average 3mph walking, 15mph (5 times that) for sprinting)
    mGravity              = cgVector3(0, -9.81f, 0);
    mMaximumAcceleration  = 80.0f;         // 80 m/s/s
    mJumpImpulse          = 4.0f;          // 4.0m/s
    mAirborneWalkDamping  = 0.3f;
    mRampWalkDamping      = 0.2f;
    mCrouchHeightScale    = 0.5f;
    mProneHeightScale     = 0.2f;
    
    // States
    mUpAxis               = cgVector3(0,1,0);
    mAirborneTime         = 1.0f;         // Airborne for > the switching threshold
    mVelocity             = cgVector3(0,0,0);
    mFloorNormal          = cgVector3(0,0,0);
    mState                = Airborne;
    mRequestedStandMode   = Standing;
    mActualStandMode      = Standing;
}

//-----------------------------------------------------------------------------
//  Name : ~cgCharacterController () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgCharacterController::~cgCharacterController()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgCharacterController::dispose( bool disposeBase )
{
    // Release internal references
    if ( mBody )
        mBody->removeReference( CG_NULL );
    if ( mUpJoint )
        mUpJoint->removeReference( CG_NULL );
    if ( mBodySensorShape ) 
        mBodySensorShape->removeReference( CG_NULL );
    if ( mFloorSensorShape ) 
        mFloorSensorShape->removeReference( CG_NULL );
    if ( mDynamicsSensorShape ) 
        mDynamicsSensorShape->removeReference( CG_NULL );
    
    // Clear variables
    mBody                 = CG_NULL;
    mUpJoint              = CG_NULL;
    mBodySensorShape      = CG_NULL;
    mFloorSensorShape     = CG_NULL;
    mDynamicsSensorShape  = CG_NULL;

    // Dispose base
    if ( disposeBase )
        cgPhysicsController::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : allocate() (Static)
/// <summary>
/// Allocate a physics controller of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgPhysicsController * cgCharacterController::allocate( const cgString & typeName, cgPhysicsWorld * world )
{
    return new cgCharacterController( world );
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Allow the controller to initialize (called after setting up).
/// </summary>
//-----------------------------------------------------------------------------
bool cgCharacterController::initialize(  )
{
    // Validate requirements
    if ( mParentObject == CG_NULL )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Attempting to initialize a 'Character' physics controller without first attaching it to a parent object is not allowed.\n") );
        return false;
    
    } // End if no parent

    // Release prior data. This allows this function to be called more than 
    // once internally when properties are modified.
    if ( mBody )
        mBody->removeReference( CG_NULL );
    if ( mUpJoint )
        mUpJoint->removeReference( CG_NULL );
    if ( mBodySensorShape ) 
        mBodySensorShape->removeReference( CG_NULL );
    if ( mFloorSensorShape ) 
        mFloorSensorShape->removeReference( CG_NULL );
    if ( mDynamicsSensorShape ) 
        mDynamicsSensorShape->removeReference( CG_NULL );
    
    // Clear variables
    mBody                 = CG_NULL;
    mUpJoint              = CG_NULL;
    mBodySensorShape      = CG_NULL;
    mFloorSensorShape     = CG_NULL;
    mDynamicsSensorShape  = CG_NULL;

    // Generate the various sensor shapes that are required for
    // the kinematic and dynamics motion phases.
    buildSensorShapes();

    // Compute the intial transformation of the object.
    cgTransform objectTransform = mParentObject->getWorldTransform();

    // Design a simple *dynamic* rigid body that maintains the
    // details of this object as it exists in the physics world.
    // This allows other dynamics rigid bodies to collide with us.
    cgRigidBody::ConstructData Construct;
    Construct.model             = cgPhysicsModel::RigidDynamic;
    Construct.quality           = cgSimulationQuality::Default;
    Construct.initialTransform  = objectTransform;
    Construct.mass              = mCharacterMass;

    // Create the body.
    mBody = new cgRigidBody( mWorld, mDynamicsSensorShape, Construct );
    mBody->addReference( CG_NULL );

    // Never allow the body to sleep
    mBody->enableAutoSleep( false );

    // Enable continuous collision mode for the character body.
    // This helps to improve (but does not prevent) penetration.
    mBody->enableContinuousCollision( true );

    // Disable the application of gravity for the dynamics body.
    mBody->setCustomGravity( cgVector3(0,0,0) );
    mBody->enableCustomGravity( true );
    
    // Generate a fixed axis joint and attach it to the dynamics body.
    // This constraints the body to the initial axis and prevents it from "falling" over.
    mUpJoint = new cgFixedAxisJoint( mWorld, mBody, cgVector3(0,1,0) );
    mUpJoint->addReference( CG_NULL );

    //////////////////////////////////////
    cgToDo( "Physics", "When we add the concept of physics materials, move into physics world." );
    
    // Set material properties
    NewtonWorld * nativeWorld = mWorld->getInternalWorld();
    cgInt characterMaterial = NewtonMaterialCreateGroupID(nativeWorld);
    cgInt32 defaultMaterialId = NewtonMaterialGetDefaultGroupID( nativeWorld );
    NewtonMaterialSetDefaultElasticity( nativeWorld, characterMaterial, defaultMaterialId, -1000 );
    NewtonBodySetMaterialGroupID( mBody->getInternalBody(), characterMaterial );

    //////////////////////////////////////

    // Call base class implementation.
    return cgPhysicsController::initialize( );
}

//-----------------------------------------------------------------------------
//  Name : buildSensorShapes () (Protected)
/// <summary>
/// Generate the various sensor shapes that are required for the kinematic 
/// motion phase.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::buildSensorShapes( )
{
    static const cgInt SensorShapeSegments = 32;
    cgVector3 bodyShapePoints[ SensorShapeSegments * 2 ];
    cgVector3 floorShapePoints[ SensorShapeSegments * 2 ];
    cgVector3 dynamicsShapePoints[ SensorShapeSegments * 2 ];

    // Compute the final height of the character in its current standing mode.
    cgFloat height = mCharacterHeight;
    if ( mActualStandMode == Crouching )
        height *= mCrouchHeightScale;
    else if ( mActualStandMode == Prone )
        height *= mProneHeightScale;

    // Compute the height for the top and bottom caps of the cylinders
    // we're about to construct.
    cgFloat top    =  height * 0.5f;
    cgFloat bottom = -height * 0.5f;
    top    += mCharacterOffset;
    bottom += mCharacterOffset;

    // Construct convex hull geometry for each of the three sensor 
    // shapes that we require (floor, body and dynamics)
    for ( cgInt i = 0; i < SensorShapeSegments; ++i )
    {
        // Compute the normalized X & Z 'offset' for this cap vertex.
		cgFloat x = cosf(CGE_TWO_PI * (cgFloat)i / (cgFloat)SensorShapeSegments );
		cgFloat z = sinf(CGE_TWO_PI * (cgFloat)i / (cgFloat)SensorShapeSegments );

        // Compute final X & Z location for the floor sensor vertex
		cgFloat floorX = mCharacterRadius * x;
		cgFloat floorZ = mCharacterRadius * z;

        // Compute the final X & Z location for the dynamics sensor vertex
        cgFloat dynamicsX = (mCharacterRadius + (mKinematicCushion * 0.75f) ) * x;
		cgFloat dynamicsZ = (mCharacterRadius + (mKinematicCushion * 0.75f) ) * z;

        // Compute final X & Z location for the body sensor vertex (includes kinematic cushion).
		x = (mCharacterRadius + mKinematicCushion) * x;
		z = (mCharacterRadius + mKinematicCushion) * z;

        // Add point for the top and bottom caps of the body sensor
		bodyShapePoints[i].x = x;
        bodyShapePoints[i].y = bottom;
        bodyShapePoints[i].z = z;
        bodyShapePoints[i+SensorShapeSegments].x = x;
        bodyShapePoints[i+SensorShapeSegments].y = top;
        bodyShapePoints[i+SensorShapeSegments].z = z;

        // Same for the floor sensor shape using the alternative (smaller) radius
        // that does not include the kinematic cushion. The top and bottom cap
        // are also offset by the kinematic cushion size to prevent the floor search
        // from causing the shape to be embedded in the surface.
        floorShapePoints[i].x = floorX;
        floorShapePoints[i].y = bottom + mKinematicCushion;
        floorShapePoints[i].z = floorZ;
        floorShapePoints[i+SensorShapeSegments].x = floorX;
        floorShapePoints[i+SensorShapeSegments].y = top - mKinematicCushion;
        floorShapePoints[i+SensorShapeSegments].z = floorZ;

        // Finally we add the dynamics sensor shape points using its custom radius also.
        dynamicsShapePoints[i].x = dynamicsX;
        dynamicsShapePoints[i].y = bottom;
        dynamicsShapePoints[i].z = dynamicsZ;
        dynamicsShapePoints[i+SensorShapeSegments].x = dynamicsX;
        dynamicsShapePoints[i+SensorShapeSegments].y = top;
        dynamicsShapePoints[i+SensorShapeSegments].z = dynamicsZ;

	} // Next Segment

    // Build the shapes
    mDynamicsSensorShape = new cgConvexHullShape( mWorld, dynamicsShapePoints, SensorShapeSegments * 2, sizeof(cgVector3) );
    mFloorSensorShape    = new cgConvexHullShape( mWorld, floorShapePoints, SensorShapeSegments * 2, sizeof(cgVector3) );
    mBodySensorShape     = new cgConvexHullShape( mWorld, bodyShapePoints, SensorShapeSegments * 2, sizeof(cgVector3) );

    // Maintain references.
    mDynamicsSensorShape->addReference( CG_NULL );
    mFloorSensorShape->addReference( CG_NULL );
    mBodySensorShape->addReference( CG_NULL );    
}

//-----------------------------------------------------------------------------
//  Name : preStep ()
/// <summary>
/// Called just prior to the upcoming physics simulation step.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::preStep( cgFloat timeDelta )
{
    // First attempt to make any requested standing mode change
    // if one is still pending.
    if ( mRequestedStandMode != mActualStandMode )
        attemptStandingChange();

    // Separate out current velocity into its vertical and horizontal components.
	cgVector3 verticalVelocity   = mUpAxis * cgVector3::dot( mVelocity, mUpAxis );
    cgVector3 horizontalVelocity = mVelocity - verticalVelocity;

    // Compute requested walkDirection direction.
    cgVector3 forwardDirection  = mParentObject->getZAxis();
    cgVector3 rightDirection    = mParentObject->getXAxis();
    cgVector3 walkDirection     = forwardDirection * mParentObject->getInputChannelState( _T("ForwardBackward"), 0.0f ) +
                                  rightDirection * mParentObject->getInputChannelState( _T("LeftRight"), 0.0f );

    // Diminish the amount of applied motion if character is not currently standing 
    // on 'flat' ground. If it is currently standing on a supporting surface however, 
    // apply any jumping impulse that has been requested
    CharacterState state = getCharacterState();
    switch ( state )
    {
        case OnFloor:
        {
            // Was a jumpStrength requested?
            cgFloat jumpStrength = mParentObject->getInputChannelState( _T("Jump"), 0.0f );
            if ( jumpStrength > 0 )
            {
                // Apply impulse to vertical velocity
                verticalVelocity += mUpAxis * jumpStrength * mJumpImpulse;

                // Mark as airborne.
                mState = Airborne;
                mAirborneTime = 1.0f;

                // Apply airborne damping.
                walkDirection *= mAirborneWalkDamping;
            
            } // End if jumping
            break;
        
        } // End case OnFloor
        
        case OnRamp:
            // Only a portion of the requested motion will be applied when standing on a ramp?
            walkDirection *= mRampWalkDamping;
            break;

        case Airborne:
            // Only a portion of the requested motion will be applied when airborne?
            walkDirection *= mAirborneWalkDamping;
            break;
    
    } // End case state
    
    // Compute new horizontal velocity depending on whether the
    // character is accelerating or decelerating.
    if ( cgVector3::length( walkDirection ) > CGE_EPSILON )
    {
        horizontalVelocity += (walkDirection * mMaximumAcceleration) * timeDelta;
    
    } // End if accelerating
    else if ( state == OnFloor )
    {
        cgFloat horizontalSpeed = cgVector3::length( horizontalVelocity );
        cgFloat newSpeed = horizontalSpeed - (mMaximumAcceleration * timeDelta);
        if ( newSpeed < 0 )
            horizontalVelocity = cgVector3(0,0,0);
        else
            horizontalVelocity *= (newSpeed / horizontalSpeed);
    
    } // End if decelerating

    // Clamp horizontal velocity to max value.
    cgFloat horizontalSpeed = cgVector3::length( horizontalVelocity );
    if ( horizontalSpeed > mWalkSpeed )
        horizontalVelocity *= (mWalkSpeed / horizontalSpeed);

    // Reconstruct final velocity vector.
    mVelocity = horizontalVelocity + verticalVelocity;

    // Apply acceleration due to gravity ((Gravity * Mass) / Mass) / Time.
    mVelocity += mGravity * timeDelta;

    // Record the most recent position of the parent object. This will serve 
    // as both our starting point for the upcoming kinematic motion step. It
    // must also be backed up so that we can compare how the object moved
    // during the actual physics step.
    cgVector3 currentPosition = mParentObject->getPosition();
    mOriginalPosition = currentPosition;

    // Different collision handling is used depending on the current state of the
    // character. Let's apply these now. Note: We use the /actual/ current state
    // rather than that returned via 'getCharacterState()'. We need to ensure that
    // we ignore the modifications that may be made by that method for the purposes
    // of reporting (such as delaying the status change report for OnLand->Airborne).
    switch( mState )
    {
        case Airborne:
            preProcessAirborne( currentPosition, timeDelta );
            break;
        
        case OnFloor:
            preProcessOnFloor( currentPosition, timeDelta );
            break;
        
        case OnRamp:
            preProcessOnRamp( currentPosition, timeDelta );
            break;
    
    } // End switch state

    // Retrieve the final horizontal velocity of the character.
    // This will be the velocity we pass to the physics engine to
    // have it simulate dynamics.
    verticalVelocity   = mUpAxis * cgVector3::dot( mVelocity, mUpAxis );
    horizontalVelocity = mVelocity - verticalVelocity;

    // Position and set the body properties and then "let it fly" :)
    cgTransform bodyTransform( currentPosition );
    mBody->setTransform( bodyTransform );
    mBody->setAngularVelocity( cgVector3(0,0,0) );
    mBody->setVelocity( horizontalVelocity );

    // Call base class implementation.
    cgPhysicsController::preStep( timeDelta );
}

//-----------------------------------------------------------------------------
//  Name : postStep ()
/// <summary>
/// Called just after the most recent physics simulation step.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::postStep( cgFloat timeDelta )
{
    // Retrieve the final position and velocity of the physics body
    // as it exists after the dynamics simulation has run.
    cgTransform bodyTransform = mBody->getTransform();
    mVelocity.x = mBody->getVelocity().x;
    mVelocity.z = mBody->getVelocity().z;
    cgVector3 currentPosition = bodyTransform.position();

    // Different collision handling is used depending on the current state of the
    // character. Let's apply these now. Note: We use the /actual/ current state
    // rather than that returned via 'getCharacterState()'. We need to ensure that
    // we ignore the modifications that may be made by that method for the purposes
    // of reporting (such as delaying the status change report for OnLand->Airborne).
    switch( mState )
    {
        case Airborne:
            postProcessAirborne( currentPosition, timeDelta );
            break;
        
        case OnFloor:
            postProcessOnFloor( currentPosition, timeDelta );
            break;
        
        case OnRamp:
            postProcessOnRamp( currentPosition, timeDelta );
            break;
    
    } // End switch state

    // We're done. Update the parent' objects final position.
    mParentObject->setPosition( currentPosition );

    // Call base class implementation.
    cgPhysicsController::postStep( timeDelta );
}

//-----------------------------------------------------------------------------
//  Name : preProcessAirborne() (Protected)
/// <summary>
/// Run pre-simulation character control process for when the body is airborne.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::preProcessAirborne( cgVector3 & currentPosition, cgFloat timeDelta )
{
    // If the vertical velocity is directed upwards, we first need to determine
    // if we have enough room to move up by the indicated amount.
    mElevation = 0.0f;
    cgFloat verticalSpeed = cgVector3::dot( mVelocity * timeDelta, mUpAxis );
    if ( verticalSpeed > 0 )
    {
        // Generate source and destination positions for the convex cast.
        // The sensor shape top and bottom positions were adjusted during construction
        // to subtract the kinematic cushion distance from either side in order to
        // ensure the shape can never become embedded in a surface. As a result, we cast
        // out slightly further by the cushion amount and then adjust the final position back
        // by the same amount to bring it back off the surface.
        float distance;
        cgVector3 surfaceNormal, source = currentPosition;
        cgVector3 destination = source + mUpAxis * (verticalSpeed + mKinematicCushion);
        if ( findSupportingSurface( source, destination, -mUpAxis, mFloorSensorShape, distance, surfaceNormal ) )
        {
            // The character hit their "head". Snap the character position to the surface.
            currentPosition = source + ((destination - source) * distance);
            currentPosition -= mUpAxis * mKinematicCushion;
            mElevation = cgVector3::dot( mUpAxis, currentPosition - mOriginalPosition );

            // Diminish the vertical component of the velocity.
            mVelocity -= mUpAxis * cgVector3::dot( mUpAxis, mVelocity );

        } // End if found
        else
        {
            // Move into the clear space.
            currentPosition += mUpAxis * verticalSpeed;
        
        } // End if !found

    } // End if upward velocity

    // Now step forwardDirection along the horizontal velocity in order to ensure
    // that the body does not penetrate any other objects.
    mVelocity = stepForward( currentPosition, mVelocity, mUpAxis, timeDelta );
}

//-----------------------------------------------------------------------------
//  Name : preProcessOnRamp() (Protected)
/// <summary>
/// Run pre-simulation character control process for when the body is on a 
/// ramp.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::preProcessOnRamp( cgVector3 & currentPosition, cgFloat timeDelta )
{
    // No stepping on ramps.
    mElevation = 0.0f;

    // Adjust velocity to slide vector *only* if the velocity
    // describes motion up / into the ramp.
    //mVelocity += mGravity * timeDelta;
    cgFloat dot = cgVector3::dot( mVelocity, mFloorNormal );
    if ( dot < 0 )
        mVelocity -= mFloorNormal * dot;

    // Now step forwardDirection along the horizontal velocity in order to ensure
    // that the body does not penetrate any other objects.
    mVelocity = stepForward( currentPosition, mVelocity, mUpAxis, timeDelta );
}

//-----------------------------------------------------------------------------
//  Name : preProcessOnFloor() (Protected)
/// <summary>
/// Run pre-simulation character control process for when the body is on the 
/// floor.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::preProcessOnFloor( cgVector3 & currentPosition, cgFloat timeDelta )
{
    // Strip any vertical velocity component.
    mVelocity -= mUpAxis * cgVector3::dot( mUpAxis, mVelocity );

    // In order to walkDirection over steps we first have to lift the body off the ground
    // by the maximum step height just as if the character was lifting their foot
    // or jumping over steps. We must perform an upward cast to make sure that there
    // is enough room for us to do so. Begin by generating source and destination 
    // positions for the convex cast. The sensor shape top and bottom positions were 
    // adjusted during construction to subtract the kinematic cushion distance from 
    // either side in order to ensure the shape can never become embedded in a surface. 
    // As a result, we cast out slightly further by the cushion amount and then adjust 
    // the final position back by the same amount to bring it back off the surface.
    float distance;
    cgVector3 surfaceNormal, source = currentPosition;
    cgVector3 destination = source + mUpAxis * (mMaxStepHeight + mKinematicCushion);
    if ( findSupportingSurface( source, destination, -mUpAxis, mFloorSensorShape, distance, surfaceNormal, true ) )
    {
        // The character hit their "head". Snap the character position to the surface.
        currentPosition = source + ((destination - source) * distance);
        currentPosition -= mUpAxis * mKinematicCushion;
        mElevation = cgVector3::dot( mUpAxis, currentPosition - mOriginalPosition );

    } // End if found
    else
    {
        // Move into the clear space.
        currentPosition += mUpAxis * mMaxStepHeight;
        mElevation = mMaxStepHeight;
    
    } // End if !found

    // Now step forwardDirection along the horizontal velocity in order to ensure
    // that the body does not penetrate any other objects.
    mVelocity = stepForward( currentPosition, mVelocity, mUpAxis, timeDelta );
}

//-----------------------------------------------------------------------------
//  Name : postProcessAirborne() (Protected)
/// <summary>
/// Run post-simulation character control process for when the body is 
/// airborne.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::postProcessAirborne( cgVector3 & currentPosition, cgFloat timeDelta )
{
    // Now we need to search for any floor geometry below the player based
    // on any downward vertical velocity.
    cgFloat verticalSpeed = cgVector3::dot( mVelocity * timeDelta, mUpAxis );
    if ( verticalSpeed < CGE_EPSILON )
    {
        // Generate source and destination positions for the convex cast.
        // The same cushion rules are used as in the upward cast above.
        cgFloat distance;
        cgVector3 surfaceNormal, source = currentPosition;
        cgVector3 destination = source + mUpAxis * (verticalSpeed - mKinematicCushion);
        if ( findSupportingSurface( source, destination, mUpAxis, mFloorSensorShape, distance, surfaceNormal ) )
        {
            // Character is landing on floor geometry. Position directly on the surface.
            currentPosition = source + ((destination - source) * distance);
            currentPosition += mUpAxis * mKinematicCushion;

            // Did the character land on flat ground, or a ramp?
            if ( 90.0f - CGEToDegree(asinf(min(1.0f,surfaceNormal.y))) < mMaxSlope )
            {
                // Supportable ground. 
                mState = OnFloor;
                
                // Diminish the vertical component of the velocity.
                mVelocity -= mUpAxis * cgVector3::dot( mUpAxis, mVelocity );

            } // End if floor
            else
            {
                // Landed on an illegal ramp.
                mState = OnRamp;

            } // End if ramp 

            // Record the current floor surface normal
            mFloorNormal = surfaceNormal;

        } // End if found
        else
        {
            // Move into the clear space.
            currentPosition += mUpAxis * verticalSpeed;

            // We are still airborne.
            mAirborneTime += timeDelta;
        
        } // End if !found

    } // End if downward velocity

}

//-----------------------------------------------------------------------------
//  Name : postProcessOnRamp() (Protected)
/// <summary>
/// Run post-simulation character control process for when the body is on a 
/// ramp.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::postProcessOnRamp( cgVector3 & currentPosition, cgFloat timeDelta )
{
    // Now we need to search for any floor geometry below the player based
    // on any remaining downward vertical velocity (plus a tolerance).
    cgFloat verticalSpeed = cgVector3::dot( mVelocity * timeDelta, mUpAxis );
    if ( verticalSpeed < CGE_EPSILON )
    {
        // Generate source and destination positions for the convex cast.
        // The same cushion rules are used as in the upward cast above.
        cgFloat distance;
        cgVector3 surfaceNormal, source = currentPosition;
        cgVector3 destination = source + mUpAxis * (verticalSpeed - (mKinematicCushion * 1.5f));
        if ( findSupportingSurface( source, destination, mUpAxis, mFloorSensorShape, distance, surfaceNormal ) )
        {
            // Character is landing on floor geometry. Position directly on the surface.
            currentPosition = source + ((destination - source) * distance);
            currentPosition += mUpAxis * mKinematicCushion;

            // Did the character land on flat ground, or a ramp?
            if ( 90.0f - CGEToDegree(asinf(min(1.0f,surfaceNormal.y))) < mMaxSlope )
            {
                // "Flat" ground. 
                mState        = OnFloor;
                mFloorNormal = surfaceNormal;
                
                // Diminish the vertical component of the velocity.
                mVelocity -= mUpAxis * cgVector3::dot( mUpAxis, mVelocity );

            } // End if floor

            // Record the current floor surface normal
            mFloorNormal = surfaceNormal;
            
        } // End if found
        else
        {
            // No floor was found below the player, enter free-fall.
            mState         = Airborne;
            mAirborneTime = 0.0f;
            mFloorNormal  = cgVector3(0,0,0);

            // Perform full move.
            currentPosition += mUpAxis * verticalSpeed;

        } // End if !found

    } // End if downward velocity
}

//-----------------------------------------------------------------------------
//  Name : postProcessOnFloor() (Protected)
/// <summary>
/// Run post-simulation character control process for when the body is on the 
/// floor.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::postProcessOnFloor( cgVector3 & currentPosition, cgFloat elapsedTime )
{
    // Now we need to search for any floor geometry below the character as they
    // step back down by an amount equal to their actual *recorded* upward step 
    // height plus a downward step. Generate source and destination positions for 
    // the convex cast. The same cushion rules are used as in the upward cast above.
    cgFloat distance;
    cgVector3 surfaceNormal, source = currentPosition;
    cgVector3 destination = source - mUpAxis * (mElevation + mMaxStepHeight + mKinematicCushion);
    if ( findSupportingSurface( source, destination, mUpAxis, mFloorSensorShape, distance, surfaceNormal, true ) )
    {
        // Are we transitioning to a ramp?
        bool newPositionIsRamp = ( 90.0f - CGEToDegree(asinf(min(1.0f,surfaceNormal.y))) >= mMaxSlope );
        if ( newPositionIsRamp )
        {
            // Character has walked onto a ramp. Position it at the surface initially.
            currentPosition = source + ((destination - source) * distance);
            currentPosition += mUpAxis * mKinematicCushion;

            // Is it higher or lower than our previous location? If we are stepping up, this is
            // simply disallowed. Otherwise, we slide down.
            if ( cgVector3::dot( mUpAxis, currentPosition ) < cgVector3::dot( mUpAxis, mOriginalPosition ) )
            {
                // New surface is lower. Character is now on a ramp (sliding down).
                mState = OnRamp;

            } // End if down step
            else
            {
                // This move is disallowed, but step as close to the surface as we can.
                currentPosition = mOriginalPosition;
                mVelocity = stepForward( currentPosition, mVelocity, mUpAxis, elapsedTime );

                // Apply final slide.
                currentPosition += mVelocity * elapsedTime;

            } // End if up step

        } // End if ramp
        else
        {
            // Character is landing on floor geometry. Position directly on the surface.
            currentPosition = source + ((destination - source) * distance);
            currentPosition += mUpAxis * mKinematicCushion;
        
        } // End if !ramp

        // Record the normal of the surface currently supporting the character.
        mFloorNormal = surfaceNormal;

    } // End if found
    else
    {
        // Undo the upward step, but do NOT step down by maximum step height.
        currentPosition -= mUpAxis * mElevation;

        // No floor was found below the player, enter free-fall.
        mState         = Airborne;
        mAirborneTime = 0.0f;
        mFloorNormal  = cgVector3(0,0,0);
    
    } // End if !found
}

//-----------------------------------------------------------------------------
//  Name : findSupportingSurface() (Protected)
/// <summary>
/// Perform a convex cast in the specified direction to find a valid supporting
/// surface (i.e. floor), or alternatively to find overhead obstructions when
/// jumping / stepping for example.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCharacterController::findSupportingSurface( const cgVector3 & source, const cgVector3 & destination, const cgVector3 & upAxis, cgPhysicsShape * shape, cgFloat & distance, cgVector3 & surfaceNormal, bool rejectPenetratingDynamics /* = false */ ) const
{
    static const cgInt MaxIterations = 8;
    static const cgInt MaxContacts   = 16;
    
    // Convert required values to the physics system scale
    cgTransform sourceTransform( mWorld->toPhysicsScale( source ) );
    cgVector3 finalDestination( mWorld->toPhysicsScale( destination ) );

    // Always ignore the character body during convex cast.
    std::set<const NewtonBody*> skipSet;
    skipSet.insert( mBody->getInternalBody() );

    // Keep searching until we find a "clean" detection (will only
    // iterate more than once if the caller wants to reject any
    // penetrating intersections).
    NewtonWorldConvexCastReturnInfo contactInfo[MaxContacts];
    for ( cgInt iterations = 0; iterations < MaxIterations; ++iterations )
    {
        // Cast to find the floor;
	    cgInt contactCount = NewtonWorldConvexCast( mWorld->getInternalWorld(), (cgMatrix)sourceTransform, finalDestination, shape->getInternalShape(),
                                                    &distance, &skipSet, convexCastPreFilter, contactInfo,
                                                    sizeof(contactInfo) / sizeof(contactInfo[0]), 0 );
    	
        // Any contacts?
        if ( contactCount )
        {
            // Find the best contact (most aligned to UP axis).
            cgInt     bestContact = -1;
            cgVector3 normal, bestNormal;
            cgFloat   bestValue = -FLT_MAX;
            for ( cgInt i = 0; i < contactCount; ++i )
            {
                // Reject entire convex cast if penetrating contacts are to be rejected.
                if ( rejectPenetratingDynamics && fabsf(contactInfo[i].m_penetration) > CGE_EPSILON )
                {
                    //cgAppLog::write( _T("Rejecting penetrating support surface contact.\n") );
                    cgFloat mass, ixx, iyy, izz;
                    NewtonBodyGetMassMatrix( contactInfo[i].m_hitBody, &mass, &ixx, &iyy, &izz );
                    if ( mass > 0 )
                    {
                        // Revert test result.
                        bestContact = -1;

                        // Don't test the penetrating body again.
                        skipSet.insert( contactInfo[i].m_hitBody );
                        break;
                    
                    } // End if dymamic
                
                } // End if penetrating

                // Workaround: With box collision shapes, sometimes 'm_normalOnHitPoint'
                // contains an unitialized normal (<0,0,0>) when the hit point was on the
                // extreme outer edges of the box during convex casting (due to the internal
                // raycast failing). In this case, we'll reluctantly use the contact normal.
                if ( cgVector3::lengthSq( contactInfo[i].m_normalOnHitPoint ) < 0.5f )
                    normal = cgVector3(contactInfo[i].m_normal);
                else
                    normal = cgVector3(contactInfo[i].m_normalOnHitPoint);
                
                // Is this most aligned to the up axis so far?
                cgFloat value = cgVector3::dot( upAxis, normal );
                if ( value > bestValue )
                {
                    bestContact = i;
                    bestNormal  = normal;
                    bestValue   = value;
                
                } // End if better

            } // Next Contact

            // Return the normal of the best contact (if any).
            if ( bestContact >= 0 )
            {
                surfaceNormal = bestNormal;
                return true;
            
            } // End if none valid

        } // End if hit
        else
        {
            // No hit, nothing found.
            break;
        
        } // End if !hit

    } // Next iteration
    
    // No contacts
    return false;
}

//-----------------------------------------------------------------------------
//  Name : stepForward() (Protected)
/// <summary>
/// Apply the requested forwardDirection motion to the body. Returns the final velocity 
/// vector after intersection testing has been applied and the body placed into 
/// its final appropriate location prior to the physics step.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgCharacterController::stepForward( cgVector3 & position, const cgVector3 & velocity, const cgVector3 & upAxis, cgFloat timeDelta )
{
    static const cgInt MaxIterations = 8;
    static const cgInt MaxContacts = 16;
    static const cgFloat Restitution           = 0.0f;
    static const cgFloat MinResidualVelocitySq = 0.1f * 0.1f;
    
    // Cache newton objects.
    NewtonCollision * shape = mBodySensorShape->getInternalShape();
    NewtonWorld * nativeWorld = mWorld->getInternalWorld();

    // Select the appropriate current velocity for the computation. Either use
    // the velocity vector as-is, or separate it out into its vertical and horizontal 
    // components.
    cgVector3 verticalVelocity = upAxis * cgVector3::dot( velocity, upAxis );
    cgVector3 currentVelocity = velocity - verticalVelocity;

    // Convert required values into physics system scale.
    position *= mWorld->toPhysicsScale();
    currentVelocity *= mWorld->toPhysicsScale();
    cgFloat cushion = mWorld->toPhysicsScale(mKinematicCushion);

    // Attempt horizontal motion first
    std::set<const NewtonBody*> skipSet;
    cgFloat currentSpeedSq = cgVector3::lengthSq( currentVelocity );
	if ( currentSpeedSq > 0.0f )
    {
        cgFloat   hitParam;
        cgFloat   speeds[MaxContacts * 2];
		cgFloat   bounceSpeeds[MaxContacts * 2];
		cgVector3 bounceNormals[MaxContacts * 2];

        // Do not test character body for (self) intersection.
        skipSet.insert( mBody->getInternalBody() );

        // Compute the initial source and destination for the first horizontal cast.
        cgTransform sourceTransform;
        sourceTransform.setPosition( position );
        cgVector3 translationStep = currentVelocity * timeDelta;
		cgVector3 paddingStep = currentVelocity * (cushion / sqrtf(currentSpeedSq));

//#define USEPADDING 1
#if USEPADDING
		cgVector3 destination = position + translationStep + paddingStep;
#else
        cgVector3 destination = position + translationStep;
#endif
        cgFloat   paddingTimeStep = cgVector3::dot( paddingStep, currentVelocity ) / currentSpeedSq;

        // Search for contacts until all horizontal motion is diminished
        // or we reach the maximum number of collision iterations.
        bool continueIteration = true;
        cgInt contactCount = 0, prevContactCount = 0;
        NewtonWorldConvexCastReturnInfo contactInfo[MaxContacts];
        NewtonWorldConvexCastReturnInfo prevContactInfo[MaxContacts];
        for ( cgInt iterations = 0; iterations < MaxIterations && continueIteration; ++iterations )
        {
            // Exit after this iteration unless we need to continue.
            continueIteration = false;

		    // Cast out and collect contacts. Dynamic bodies are ignored! These
            // will be handled separately during the actual physics simulation.
		    contactCount = NewtonWorldConvexCast( nativeWorld, (cgMatrix)sourceTransform, destination, shape, &hitParam, 
                                                  &skipSet, convexCastPreFilterStaticOnly, contactInfo,
                                                  sizeof(contactInfo) / sizeof(contactInfo[0]), 0 );
		    contactCount = preProcessContacts( contactInfo, contactCount, upAxis );

            // Any collision detected?
            if ( contactCount )
            {
                // Calculate the travel time to the point of intersection.
                #if USEPADDING
			    cgFloat intersectTime = hitParam * cgVector3::dot( (translationStep + paddingStep), currentVelocity) / currentSpeedSq - paddingTimeStep;
                #else
                cgFloat intersectTime = hitParam * cgVector3::dot( translationStep, currentVelocity) / currentSpeedSq;
                #endif

                // Move the body to the point of intersection.
                sourceTransform.translate( currentVelocity * intersectTime );
                position = sourceTransform.position();

                // Compute direction and speed of "bounce" or "slide" for each of the
                // current contacts, and any contacts from the previous iteration
                // which need to be processed further. Current contacts first.
                cgInt count = 0;
			    for ( cgInt i = 0; i < contactCount; ++i )
                {
                    speeds[count] = 0.0f;
				    bounceNormals[count] = cgVector3(contactInfo[i].m_normal);
				    cgFloat reboundVelocity = -cgVector3::dot( currentVelocity, bounceNormals[count] ) * (1.0f + Restitution);
				    bounceSpeeds[count] = (reboundVelocity > 0.0f) ? reboundVelocity : 0.0f;      
                    ++count;
			
                } // Next Contact

                // Contacts from previous iterations next.
			    for ( cgInt i = 0; i < prevContactCount; ++i )
                {
                    speeds[count] = 0.0f;
				    bounceNormals[count] = cgVector3(prevContactInfo[i].m_normal);
				    cgFloat reboundVelocity = -cgVector3::dot( currentVelocity, bounceNormals[count] ) * (1.0f + Restitution);
				    bounceSpeeds[count] = (reboundVelocity > 0.0f) ? reboundVelocity : 0.0f; 
                    ++count;

			    } // Next Contact

                // Compute final speed adjustments for each surface up to a maximum
                // number (for this iteration).
                cgFloat residual = 10.0f;
                cgVector3 auxiliaryBounceVelocity( 0.0f, 0.0f, 0.0f );
                for ( cgInt i = 0; i < MaxContacts && (residual > CGE_EPSILON_1MM); ++i )
                {
                    residual = 0.0f;
                    for ( cgInt k = 0; k < count; ++k )
                    {
                        cgFloat v = bounceSpeeds[k] - cgVector3::dot( bounceNormals[k], auxiliaryBounceVelocity );
                        cgFloat x = speeds[k] + v;
    					
                        // Do not allow to bounce back
					    if ( x < 0.0f )
                        {
						    v = 0.0f;
						    x = 0.0f;
    					
                        } // End if < 0

					    if ( fabsf(v) > residual )
						    residual = fabsf(v);

					    auxiliaryBounceVelocity += bounceNormals[k] * (x - speeds[k]);
					    speeds[k] = x;
    				
                    } // Next bounce contact
    			
                } // Next iteration

                // Adjust velocity
                for ( cgInt i = 0; i < count; ++i )
                {
				    continueIteration = (speeds[i] > 0.0f);
                    currentVelocity += bounceNormals[i] * speeds[i];
    			
                } // Next bounce contact

                // Continue iteration?
                if ( continueIteration )
                {
                    prevContactCount = contactCount;
				    memcpy( prevContactInfo, contactInfo, prevContactCount * sizeof(NewtonWorldConvexCastReturnInfo) );
				    
                    // If we're certain that velocity remains, continue iteration.
                    continueIteration = false;
                    currentSpeedSq = cgVector3::lengthSq( currentVelocity );
				    if ( currentSpeedSq > CGE_EPSILON )
                    {
                        // Compute next iteration values
					    translationStep = currentVelocity * timeDelta;
					    paddingStep     = currentVelocity * (cushion / sqrtf(currentSpeedSq));
                        #if USEPADDING
					    destination     = sourceTransform.position() + translationStep + paddingStep;
                        #else
                        destination     = sourceTransform.position() + translationStep;
                        #endif
                        paddingTimeStep = cgVector3::dot( paddingStep, currentVelocity ) / currentSpeedSq;
                        continueIteration = true;
				
                    } // End if remaining velocity 

                } // End if continue
		
            } // End if hit
        
        } // Next iteration
		
        // Prevent the body from drifting due to residual velocity when it is in a corner.
		currentSpeedSq = cgVector3::lengthSq( currentVelocity );
		if ( currentSpeedSq < MinResidualVelocitySq )
            currentVelocity = cgVector3( 0, 0, 0 );
	
    } // End if moving

    // Note: We don't actually move the object for the final iteration
    // when the object is in "clear" space. This last step will be handled
    // by the physics engine. At this stage we simply exit the iteration
    // loop (continueIteration is false at this point).

    // Scale values back out from physics scale.
    position *= mWorld->fromPhysicsScale();
    currentVelocity *= mWorld->fromPhysicsScale();

    // Return final velocity
    return currentVelocity + verticalVelocity;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterState ()
/// <summary>
/// Retrieve the current state of the character.
/// </summary>
//-----------------------------------------------------------------------------
cgCharacterController::CharacterState cgCharacterController::getCharacterState( ) const
{
    if ( mState == Airborne && mAirborneTime < 0.100f )
        return OnFloor;
    else
        return mState;
}

//-----------------------------------------------------------------------------
//  Name : supportsInputChannels() (Virtual)
/// <summary>
/// Determine if this controller supports the reading of input channel
/// states from its parent object in order to apply motion (rather than
/// through direct force or velocity manipulation).
/// </summary>
//-----------------------------------------------------------------------------
bool cgCharacterController::supportsInputChannels( ) const
{
    // We use input channels in order to directly control the object.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setKinematicCushion ()
/// <summary>
/// Set the size of the 'cushion' that surrounds the character in order to
/// help prevent penetrations with dynamics objects.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setKinematicCushion( cgFloat value )
{
    mKinematicCushion = value;

    // Sensors and body must be rebuilt if they have already  been constructed.
    if ( mBody )
        initialize();
}

//-----------------------------------------------------------------------------
//  Name : setCharacterMass ()
/// <summary>
/// Set the mass of the character in kilograms.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setCharacterMass( cgFloat value )
{
    mCharacterMass = value;

    // Adjust mass of dynamics body if it has already been constructed.
    if ( mBody )
        mBody->setMass( value );
}

//-----------------------------------------------------------------------------
//  Name : setCharacterHeight ()
/// <summary>
/// Set the overall height of the character.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setCharacterHeight( cgFloat value )
{
    mCharacterHeight = value;

    // Sensors and body must be rebuilt if they have already  been constructed.
    if ( mBody )
        initialize();
}

//-----------------------------------------------------------------------------
//  Name : setCharacterRadius ()
/// <summary>
/// Set the radius of the volume that defines the boundary of the character.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setCharacterRadius( cgFloat value )
{
    mCharacterRadius = value;

    // Sensors and body must be rebuilt if they have already  been constructed.
    if ( mBody )
        initialize();
}

//-----------------------------------------------------------------------------
//  Name : setCharacterOffset ()
/// <summary>
/// Set the amount to offset the collision shape for the character from the
/// origin of its transform along Y.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setCharacterOffset( cgFloat value )
{
    mCharacterOffset = value;

    // Sensors and body must be rebuilt if they have already  been constructed.
    if ( mBody )
        initialize();
}

//-----------------------------------------------------------------------------
//  Name : setMaximumSlope ()
/// <summary>
/// Set the angle in degrees that should be considered the absolute maximum
/// ramp angle at which the character can be supported without sliding where
/// 90 degrees represents a vertical wall, and 0 degrees is flat.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setMaximumSlope( cgFloat degrees )
{
    mMaxSlope = degrees;
}

//-----------------------------------------------------------------------------
//  Name : setMaximumStepHeight ()
/// <summary>
/// Set the maximum height of obstacles that the character is allowed to 
/// automatically step onto / over without jumping.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setMaximumStepHeight( cgFloat value )
{
    mMaxStepHeight = value;
}

//-----------------------------------------------------------------------------
//  Name : setMaximumWalkSpeed ()
/// <summary>
/// Set the maximum speed at which the character is able to walkDirection in any
/// horizontal direction (does not limit vertical velocity).
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setMaximumWalkSpeed( cgFloat value )
{
    mWalkSpeed = value;
}

//-----------------------------------------------------------------------------
//  Name : setWalkAcceleration ()
/// <summary>
/// Set the rate at which the character accelerates when input is received, or
/// decelerates when no input is received.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setWalkAcceleration( cgFloat value )
{
    mMaximumAcceleration = value;
}

//-----------------------------------------------------------------------------
//  Name : setGravity ()
/// <summary>
/// Set the force of gravity that is applicable to this character.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setGravity( const cgVector3 & gravity )
{
    mGravity = gravity;
}

//-----------------------------------------------------------------------------
//  Name : setJumpImpulse ()
/// <summary>
/// Set the magnitude of the impulse that will be applied when jumpStrength input
/// is received.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setJumpImpulse( cgFloat value )
{
    mJumpImpulse = value;
}

//-----------------------------------------------------------------------------
//  Name : setAirborneWalkDamping ()
/// <summary>
/// Set the scaling factor that will be applied to the walking forces when the
/// character is airborne. This allows the application to configure the
/// percentage of force that will be allowed whilst in the air for example.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setAirborneWalkDamping( cgFloat value )
{
    mAirborneWalkDamping = value;
}

//-----------------------------------------------------------------------------
//  Name : setRampWalkDamping ()
/// <summary>
/// Set the scaling factor that will be applied to the walking forces when the
/// character is on an 'illegal' ramp. This allows the application to configure
/// the percentage of force that will be allowed whilst sliding for example.
/// </summary>
//-----------------------------------------------------------------------------
void cgCharacterController::setRampWalkDamping( cgFloat value )
{
    mRampWalkDamping = value;
}

//-----------------------------------------------------------------------------
//  Name : getKinematicCushion ()
/// <summary>
/// Get the size of the 'cushion' that surrounds the character in order to
/// help prevent penetrations with dynamics objects.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getKinematicCushion( ) const
{
    return mKinematicCushion;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterMass ()
/// <summary>
/// Get the mass of the character in kilograms.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getCharacterMass( ) const
{
    return mCharacterMass;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterHeight ()
/// <summary>
/// Get the overall height of the character.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getCharacterHeight( ) const
{
    return mCharacterHeight;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterHeight ()
/// <summary>
/// Get the overall height of the character, optionally scaled by an amount
/// appropriate to the current standing mode (crouched height, prone height, 
/// etc.)
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getCharacterHeight( bool adjustByStandingMode ) const
{
    if ( !adjustByStandingMode || mActualStandMode == Standing )
        return mCharacterHeight;
    else if ( mActualStandMode == Crouching )
        return mCharacterHeight * mCrouchHeightScale;
    else 
        return mCharacterHeight * mProneHeightScale;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterRadius ()
/// <summary>
/// Get the radius of the volume that defines the boundary of the character.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getCharacterRadius( ) const
{
    return mCharacterRadius;
}

//-----------------------------------------------------------------------------
//  Name : getCharacterOffset ()
/// <summary>
/// Get the amount to offset the collision shape for the character from the
/// origin of its transform along Y.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getCharacterOffset( ) const
{
    return mCharacterOffset;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumSlope ()
/// <summary>
/// Get the angle in degrees that should be considered the absolute maximum
/// ramp angle at which the character can be supported without sliding where
/// 90 degrees represents a vertical wall, and 0 degrees is flat.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getMaximumSlope( ) const
{
    return mMaxSlope;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumStepHeight ()
/// <summary>
/// Get the maximum height of obstacles that the character is allowed to 
/// automatically step onto / over without jumping.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getMaximumStepHeight( ) const
{
    return mMaxStepHeight;
}

//-----------------------------------------------------------------------------
//  Name : getMaximumWalkSpeed ()
/// <summary>
/// Get the maximum speed at which the character is able to walkDirection in any
/// horizontal direction (does not limit vertical velocity).
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getMaximumWalkSpeed( ) const
{
    return mWalkSpeed;
}

//-----------------------------------------------------------------------------
//  Name : getWalkAcceleration ()
/// <summary>
/// Get the rate at which the character accelerates when input is received, or
/// decelerates when no input is received.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getWalkAcceleration( ) const
{
    return mMaximumAcceleration;
}

//-----------------------------------------------------------------------------
//  Name : getGravity ()
/// <summary>
/// Get the force of gravity that is applicable to this character.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgCharacterController::getGravity( ) const
{
    return mGravity;
}

//-----------------------------------------------------------------------------
//  Name : getVelocity ()
/// <summary>
/// Get the current velocity of the character.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgCharacterController::getVelocity( ) const
{
    return mVelocity;
}

//-----------------------------------------------------------------------------
//  Name : getJumpImpulse ()
/// <summary>
/// Get the magnitude of the impulse that will be applied when jumpStrength input
/// is received.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getJumpImpulse( ) const
{
    return mJumpImpulse;
}

//-----------------------------------------------------------------------------
//  Name : getAirborneWalkDamping ()
/// <summary>
/// Get the scaling factor that will be applied to the walking forces when the
/// character is airborne. This allows the application to configure the
/// percentage of force that will be allowed whilst in the air for example.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getAirborneWalkDamping( ) const
{
    return mAirborneWalkDamping;
}

//-----------------------------------------------------------------------------
//  Name : getRampWalkDamping ()
/// <summary>
/// Get the scaling factor that will be applied to the walking forces when the
/// character is on an 'illegal' ramp. This allows the application to configure
/// the percentage of force that will be allowed whilst sliding for example.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgCharacterController::getRampWalkDamping( ) const
{
    return mRampWalkDamping;
}

//-----------------------------------------------------------------------------
//  Name : requestStandingMode ()
/// <summary>
/// Request that the character move into the specified standing mode; either
/// standing, crouching or prone are supported. Returns true if the character
/// was immediately able to switch to this mode, or false if it was not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCharacterController::requestStandingMode( StandingMode mode )
{
    // Skip if this is a no-op.
    if ( mRequestedStandMode == mode )
        return (mActualStandMode == mode);

    // Record requested mode.
    mRequestedStandMode = mode;

    // Make the attempt.
    return attemptStandingChange();
}

//-----------------------------------------------------------------------------
//  Name : attemptStandingChange () (Protected)
/// <summary>
/// Attempt to change from the current standing mode to that requested
/// by the application. There is no guarantee of success if, for instance,
/// there is not enough room above the character.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCharacterController::attemptStandingChange( )
{
    // Anything to do?
    if ( mRequestedStandMode == mActualStandMode )
        return true;

    // Compute the new requested height of the character.
    cgFloat currentHeight = getCharacterHeight( true );
    cgFloat newHeight = mCharacterHeight;
    if ( mRequestedStandMode == Crouching )
        newHeight *= mCrouchHeightScale;
    else if ( mRequestedStandMode == Prone )
        newHeight *= mProneHeightScale;
    cgFloat heightChange = newHeight - currentHeight;

    // We can always switch from a higher standing to a lower one
    // (i.e. standing to crouching or crouching to prone, etc.)
    if ( heightChange <= 0 )
    {
        // Offset the parent by an appropriate amount.
        mParentObject->move( 0, heightChange * 0.5f, 0 );

        // Switch to the requested mode.
        mActualStandMode = mRequestedStandMode;

        // Immediately rebuild the shapes and body.
        initialize();

        // Success!
        return true;

    } // End if higher -> lower

    // We're about to stand up. Check to see if there is enough
    // room above us to do so.
    cgFloat   distance;
    cgVector3 surfaceNormal;
    cgVector3 source = mParentObject->getPosition();
    cgVector3 destination = source + mUpAxis * heightChange;
    if ( !findSupportingSurface( source, destination, -mUpAxis, mBodySensorShape, distance, surfaceNormal ) )
    {
        // It was all clear above us to make the transition.
        // Offset the parent by an appropriate amount.
        mParentObject->move( 0, heightChange * 0.5f, 0 );

        // Switch to the requested mode.
        mActualStandMode = mRequestedStandMode;

        // Immediately rebuild the shapes and body.
        initialize();

        // Success!
        return true;

    } // End if

    // We were not able to make the switch.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getRequestedStandingMode ()
/// <summary>
/// Retrieve the most recently requested standing mode. Note that the character
/// may not actually be able to switch to this mode at this time and will 
/// switch at the earliest opportunity. Use 'getActualStandingMode()' to
/// determine the currently active mode.
/// </summary>
//-----------------------------------------------------------------------------
cgCharacterController::StandingMode cgCharacterController::getRequestedStandingMode( ) const
{
    return mRequestedStandMode;
}

//-----------------------------------------------------------------------------
//  Name : getActualStandingMode ()
/// <summary>
/// Retrieve the current standing mode of the character.
/// </summary>
//-----------------------------------------------------------------------------
cgCharacterController::StandingMode cgCharacterController::getActualStandingMode( ) const
{
    return mActualStandMode;
}