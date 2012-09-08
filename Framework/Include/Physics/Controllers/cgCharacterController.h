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
// Name : cgCharacterController.h                                            //
//                                                                           //
// Desc : Contains classes which provide physics managenent for character    //
//        class objects such as the player or other NPCs.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCHARACTERCONTROLLER_H_ )
#define _CGE_CGCHARACTERCONTROLLER_H_

//-----------------------------------------------------------------------------
// cgCharacterController Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsController.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRigidBody;
class cgPhysicsBody;
class cgPhysicsShape;
class cgPhysicsJoint;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCharacterController (Class)
/// <summary>
/// A physics controller class that provides physics simulation and 
/// collision detection / response for any character class object to 
/// which it is attached.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCharacterController : public cgPhysicsController
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCharacterController, cgPhysicsController, "CharacterController" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum CharacterState
    {
        Airborne,
        OnFloor,
        OnRamp
    };

    enum StandingMode
    {
        Standing,
        Crouching,
        Prone
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCharacterController( cgPhysicsWorld * world );
    virtual ~cgCharacterController( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgPhysicsController * allocate( const cgString & typeName, cgPhysicsWorld * world );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    CharacterState      getCharacterState       ( ) const;
    void                setKinematicCushion     ( cgFloat value );
    void                setCharacterMass        ( cgFloat value );
    void                setCharacterHeight      ( cgFloat value );
    void                setCharacterRadius      ( cgFloat value );
    void                setCharacterOffset      ( cgFloat value );
    void                setMaximumSlope         ( cgFloat degrees );
    void                setMaximumStepHeight    ( cgFloat value );
    void                setMaximumWalkSpeed     ( cgFloat value );
    void                setWalkAcceleration     ( cgFloat value );
    void                setGravity              ( const cgVector3 & gravity );
    void                setJumpImpulse          ( cgFloat value );
    void                setAirborneWalkDamping  ( cgFloat value );
    void                setRampWalkDamping      ( cgFloat value );
    bool                requestStandingMode     ( StandingMode mode );
    cgFloat             getKinematicCushion     ( ) const;
    cgFloat             getCharacterMass        ( ) const;
    cgFloat             getCharacterHeight      ( ) const;
    cgFloat             getCharacterHeight      ( bool adjustByStandingMode ) const;
    cgFloat             getCharacterRadius      ( ) const;
    cgFloat             getCharacterOffset      ( ) const;
    cgFloat             getMaximumSlope         ( ) const;
    cgFloat             getMaximumStepHeight    ( ) const;
    cgFloat             getMaximumWalkSpeed     ( ) const;
    cgFloat             getWalkAcceleration     ( ) const;
    const cgVector3 & getGravity              ( ) const;
    const cgVector3 & getVelocity             ( ) const;
    cgFloat             getJumpImpulse          ( ) const;
    cgFloat             getAirborneWalkDamping  ( ) const;
    cgFloat             getRampWalkDamping      ( ) const;
    StandingMode        getRequestedStandingMode( ) const;
    StandingMode        getActualStandingMode   ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsController)
    //-------------------------------------------------------------------------
    virtual void        preStep                 ( cgFloat timeDelta );
    virtual void        postStep                ( cgFloat timeDelta );
    virtual bool        initialize              ( );
    virtual bool        supportsInputChannels   ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                 ( bool disposeBase );
	
protected:
    //-------------------------------------------------------------------------
    // Protected Typdefs & Structures
    //-------------------------------------------------------------------------
    struct ContactData
    {
        cgPhysicsBody * body;       // The body we made contact with
        cgVector3       point;      // World space point of contact
        cgVector3       normal;     // Contact normal.
        cgVector3       impulse;    // Impulse to apply to the body based on the impact.
    };
    CGE_VECTOR_DECLARE( ContactData, ContactDataArray )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            buildSensorShapes       ( );
    cgVector3       stepForward             ( cgVector3 & position, const cgVector3 & velocity, const cgVector3 & upAxis, cgFloat timeDelta );
    bool            findSupportingSurface   ( const cgVector3 & source, const cgVector3 & destination, const cgVector3 & upAxis, cgPhysicsShape * shape, cgFloat & distance, cgVector3 & surfaceNormal, bool rejectPenetratingDynamics = false ) const;
    bool            attemptStandingChange   ( );
    
    void            preProcessAirborne      ( cgVector3 & currentPosition, cgFloat timeDelta );
    void            preProcessOnRamp        ( cgVector3 & currentPosition, cgFloat timeDelta );
    void            preProcessOnFloor       ( cgVector3 & currentPosition, cgFloat timeDelta );
    void            postProcessAirborne     ( cgVector3 & currentPosition, cgFloat timeDelta );
    void            postProcessOnRamp       ( cgVector3 & currentPosition, cgFloat timeDelta );
    void            postProcessOnFloor      ( cgVector3 & currentPosition, cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgRigidBody       * mBody;                // The rigid body that ties us to the physics simulation.
    cgPhysicsJoint    * mUpJoint;             // Joint that fixes the rigid body to a given up axis.
    cgPhysicsShape    * mDynamicsSensorShape; // Shape used to search for stairs.
    cgPhysicsShape    * mFloorSensorShape;    // Shape used to search for suitable floor.
    cgPhysicsShape    * mBodySensorShape;     // Shape used to search for collisions in forward motion.
    
    // Character Properties
    cgFloat             mKinematicCushion;      // Cushion around the object used during kinematic motion processing.
    cgFloat             mCharacterMass;         // The mass of the character.
    cgFloat             mCharacterHeight;       // Height of the character.
    cgFloat             mCharacterRadius;       // Radius of the character.
    cgFloat             mCharacterOffset;       // Amount to offset the character's collision shape along Y from its transform origin.
    cgFloat             mMaxStepHeight;         // Maximum height that we can automatically step up.
    cgFloat             mMaxSlope;              // Maximum allowed slope angle that can support the player.
    cgFloat             mWalkSpeed;             // Requested walking speed of the character
    cgVector3           mGravity;               // Gravity force to apply to the character.
    cgFloat             mMaximumAcceleration;   // The rate at which the character can accelerate / decelerate.
    cgFloat             mJumpImpulse;           // Impulse to apply when jumping.
    cgFloat             mAirborneWalkDamping;   // Amount to damp walk acceleration when airborne.
    cgFloat             mRampWalkDamping;       // Amount to damp walk acceleration when on a ramp.
    StandingMode        mRequestedStandMode;    // The stand mode that was most recently requested.
    StandingMode        mActualStandMode;       // The standing mode that the character is currently using.
    cgFloat             mCrouchHeightScale;     // Amount to scale the character's height when crouching.
    cgFloat             mProneHeightScale;      // Amount to scale the character's height when prone.

    // Character State
    cgVector3           mUpAxis;                // Axis to which this character is aligned.
    cgVector3           mVelocity;              // Current velocity of the charater
    cgVector3           mFloorNormal;           // Normal of the surface on which the character is standing (ramp or floor)
    CharacterState      mState;                 // The current state of the character (falling, on floor, etc.)
    cgFloat             mAirborneTime;          // The amount of time that the has elapsed since the character left the floor.

    // Collision Reporting
    ContactDataArray    mContacts;              // List of contacts made during kinematic motion phase

    cgFloat             mElevation;
    cgVector3           mOriginalPosition;
};

#endif // !_CGE_CGCHARACTERCONTROLLER_H_