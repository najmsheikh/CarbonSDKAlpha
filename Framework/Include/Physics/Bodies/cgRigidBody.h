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
// Name : cgRigidBody.h                                                      //
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

#pragma once
#if !defined( _CGE_CGRIGIDBODY_H_ )
#define _CGE_CGRIGIDBODY_H_

//-----------------------------------------------------------------------------
// cgRigidBody Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsBody.h>
#include <Physics/cgPhysicsTypes.h>
#include <Math/cgMathTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;
class cgPhysicsShape;
struct NewtonCollision;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {20030D08-7A90-4E87-8F1C-0843C0B6D904}
const cgUID RTID_RigidBody = {0x20030D08, 0x7A90, 0x4E87, {0x8F, 0x1C, 0x8, 0x43, 0xC0, 0xB6, 0xD9, 0x4}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRigidBody (Class)
/// <summary>
/// Class responsible for providing collision and/or dynamics
/// properties for all types of /rigid/ physics bodies. The rigid body
/// class itself can be used for most physics entities, but can also
/// serve as a base class from which other types of rigid bodies
/// derive.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRigidBody : public cgPhysicsBody
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRigidBody, cgPhysicsBody, "RigidBody" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRigidBody( cgPhysicsWorld * world, cgPhysicsShape * shape, const cgRigidBodyCreateParams & params );
             cgRigidBody( cgPhysicsWorld * world, cgPhysicsShape * shape, const cgRigidBodyCreateParams & params, cgPhysicsBody * initBody );
    virtual ~cgRigidBody( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsEntity)
    //-------------------------------------------------------------------------
    virtual cgTransform     getTransform                ( cgDouble interp = 1.0 ) const;
    virtual void            setTransform                ( const cgTransform & transform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType            ( ) const { return RTID_RigidBody; }
    virtual bool            queryReferenceType          ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    initialize                  ( cgPhysicsWorld * world, const cgRigidBodyCreateParams & params, cgPhysicsBody * initBody );

    //-------------------------------------------------------------------------
    // Protected Static Methods
    //-------------------------------------------------------------------------
    static void             setTransformCallback        ( const NewtonBody * body, const cgFloat * matrix, cgInt threadIndex );
    static void             applyForceAndTorqueCallback ( const NewtonBody * body, cgFloat timeStep, cgInt threadIndex );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    NewtonCollision   * mCollisionModifier;     // Used to provide support for non-uniform scaling and shears.
    cgTransform         mModifierTransform;     // Local scale and shear of the body as it existed when initializing (internally stripped).
};

#endif // !_CGE_CGRIGIDBODY_H_