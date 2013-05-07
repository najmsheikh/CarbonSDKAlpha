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
// Name : cgKinematicControllerJoint.h                                       //
//                                                                           //
// Desc : Class implementing a simple kinematic controller joint. This joint //
//        can be used to anchor a body to a specific point in space. This    //
//        anchor point can then be manipulated dynamically at runtime        //
//        resulting in the body following.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGKINEMATICCONTROLLERJOINT_H_ )
#define _CGE_CGKINEMATICCONTROLLERJOINT_H_

//-----------------------------------------------------------------------------
// cgKinematicControllerJoint Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsJoint.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsBody;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {2F54B928-AD29-4431-A418-8A92FB98F06F}
const cgUID RTID_KinematicControllerJoint = {0x2F54B928, 0xAD29, 0x4431, {0xA4, 0x18, 0x8A, 0x92, 0xFB, 0x98, 0xF0, 0x6F}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgKinematicControllerJoint (Class)
/// <summary>
/// Class implementing a simple kinematic controller joint. This joint can be 
/// used to anchor a body to a specific point in space. This anchor point can 
/// then be manipulated dynamically at runtime resulting in the body following.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgKinematicControllerJoint : public cgPhysicsJoint
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgKinematicControllerJoint, cgPhysicsJoint, "KinematicControllerJoint" )

public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum ConstraintMode
    {
        SinglePoint = 0,
        PreserveOrientation
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgKinematicControllerJoint( cgPhysicsWorld * world, cgPhysicsBody * body, cgVector3 worldSpaceHandle );
    virtual ~cgKinematicControllerJoint( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setConstraintMode       ( ConstraintMode mode );
    void                        setMaxAngularFriction   ( cgFloat friction );
    void                        setMaxLinearFriction    ( cgFloat friction );
    void                        setPosition             ( const cgVector3 & pos );
    void                        setOrientation          ( const cgQuaternion & rotation );
    void                        setTransform            ( const cgTransform & transform );

    ConstraintMode              getConstraintMode       ( ) const;
    cgFloat                     getMaxAngularFriction   ( ) const;
    cgFloat                     getMaxLinearFriction    ( ) const;
    cgVector3                   getPosition             ( ) const;
    const cgQuaternion        & getOrientation          ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_KinematicControllerJoint; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgPhysicsJoint)
    //-------------------------------------------------------------------------
    virtual void                getInfo                 ( NewtonJointRecord * info );
    virtual void                submitConstraints       ( cgFloat timeStep, cgInt threadIndex );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFloat         mMaxLinearFriction;     // Maximum linear friction to be considered.
    cgFloat         mMaxAngularFriction;    // Maximum angular friction to be considered.
    ConstraintMode  mConstraintMode;        // The mode in which the constraint is to be applied.
    cgVector3       mLocalHandleOffset;     // Local space offset from the body to the 'handle' to which the body is constrained.
    cgVector3       mPosition;              // World space position of the joint.
    cgQuaternion    mOrientation;           // World space orientation of the joint.
    bool            mOldBodyAutoSleep;      // Previous auto-sleep state of the body to which we are attached.
};

#endif // !_CGE_CGKINEMATICCONTROLLERJOINT_H_