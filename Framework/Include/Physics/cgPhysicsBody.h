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
// Name : cgPhysicsBody.h                                                    //
//                                                                           //
// Desc : Base class from which specific types of physics bodys should       //
//        derive. This base does not enforce any specific type of body or    //
//        dynamics properties (such as rigid vs. soft) and is instead        //
//        responsible solely for providing access to common properties and   //
//        functionality.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSBODY_H_ )
#define _CGE_CGPHYSICSBODY_H_

//-----------------------------------------------------------------------------
// cgPhysicsBody Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsEntity.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
struct NewtonBody;
class cgPhysicsShape;
class cgPhysicsBody;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FA2FD669-A19D-4F3B-9626-9C9ACB7C0182}
const cgUID RTID_PhysicsBody = {0xFA2FD669, 0xA19D, 0x4F3B, {0x96, 0x26, 0x9C, 0x9A, 0xCB, 0x7C, 0x1, 0x82}};

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgPhysicsBodyTransformedEventArgs
{
    cgPhysicsBodyTransformedEventArgs( const cgTransform & _newTransform, bool _dynamicsUpdate ) :
        newTransform( _newTransform ), dynamicsUpdate( _dynamicsUpdate ) {}
    cgTransform newTransform;
    bool dynamicsUpdate;
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsBodyEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever physics body events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsBodyEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsBodyEventListener, cgEventListener, "PhysicsBodyEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onPhysicsBodyTransformed    ( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgPhysicsBody (Class)
/// <summary>
/// Base class from which specific types of physics bodys should derive.
/// This base does not enforce any specific type of body or dynamics
/// properties (such as rigid vs. soft) and is instead responsible solely
/// for providing access to common properties and functionality.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsBody : public cgPhysicsEntity
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsBody, cgPhysicsEntity, "PhysicsBody" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsBody( cgPhysicsWorld * world, cgPhysicsShape * shape );
    virtual ~cgPhysicsBody( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgVector3                   getVelocity                 ( ) const;
    cgVector3                   getAngularVelocity          ( ) const;
    void                        setVelocity                 ( const cgVector3 & v );
    void                        setAngularVelocity          ( const cgVector3 & v );
    void                        setMass                     ( cgFloat mass );
    cgFloat                     getMass                     ( ) const;
    cgPhysicsShape            * getShape                    ( ) const;
    void                        applyForce                  ( const cgVector3 & force );
    void                        applyForce                  ( const cgVector3 & force, const cgVector3 & at );
    void                        applyImpulse                ( const cgVector3 & impulse );
    void                        applyImpulse                ( const cgVector3 & impulse, const cgVector3 & at );
    void                        applyTorque                 ( const cgVector3 & torque );
    void                        applyTorqueImpulse          ( const cgVector3 & torqueImpulse );
    void                        setCustomGravity            ( const cgVector3 & force );
    cgVector3                   getCustomGravity            ( ) const;
    void                        enableCustomGravity         ( bool enabled );
    bool                        isCustomGravityEnabled      ( ) const;
    void                        enableAutoSleep             ( bool enabled );
    bool                        isAutoSleepEnabled          ( ) const;
    void                        enableContinuousCollision   ( bool enabled );
    bool                        isContinuousCollisionEnabled( ) const;
    void                        setMaterialGroupId          ( cgDefaultPhysicsMaterialGroup::Base defaultMaterial );
    void                        setMaterialGroupId          ( cgInt32 group );
    
    // Internal utilities
    NewtonBody                * getInternalBody         ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // Event issuers.
    virtual void                onPhysicsBodyTransformed( cgPhysicsBodyTransformedEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsEntity)
    //-------------------------------------------------------------------------
    virtual void                clearForces             ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_PhysicsBody; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    NewtonBody    * mBody;              // Wrapped newton body object.
    cgPhysicsShape* mShape;             // The shape information associated with this body.
    cgVector3       mInertia;           // Computed inertia values for the shape associated with this body.
    cgVector3       mTotalForce;        // Total linear forces which have been accumulated in this body.
    cgVector3       mTotalTorque;       // Total torque forces which have been accumulated in this body.
    cgVector3       mCustomGravity;     // Custom gravity force to apply to this body.
    bool            mUseCustomGravity;  // When enabled, the above custom gravity force will be used instead of the global force.
    
};

#endif // !_CGE_CGPHYSICSBODY_H_