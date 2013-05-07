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
// Name : cgPhysicsJoint.h                                                   //
//                                                                           //
// Desc : Base class from which specific physics joints should derive.       //
//        Joints provide the means by which a body can be constrained to     //
//        a particular location, orientation, bound to another body and      //
//        more. A simple example joint might be a hinge for a door.          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSJOINT_H_ )
#define _CGE_CGPHYSICSJOINT_H_

//-----------------------------------------------------------------------------
// cgPhysicsJoint Header Includes
//-----------------------------------------------------------------------------
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;
class cgPhysicsBody;
class cgTransform;
class cgVector3;
struct NewtonJoint;
struct NewtonJointRecord;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {A958D02A-46A4-4988-8295-747EC484A1BB}
const cgUID RTID_PhysicsJoint = {0xA958D02A, 0x46A4, 0x4988, {0x82, 0x95, 0x74, 0x7E, 0xC4, 0x84, 0xA1, 0xBB}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsJoint (Class)
/// <summary>
/// Base class from which specific physics joints should derive. Joints provide
/// the means by which a body can be constrained to a particular location, 
/// orientation, bound to another body and more. A simple example joint might 
/// be a hinge for a door.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsJoint : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsJoint, cgReference, "PhysicsJoint" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgRigidBody;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsJoint( cgPhysicsWorld * world, cgUInt32 maxDoF, cgPhysicsBody * body0, cgPhysicsBody * body1 );
    virtual ~cgPhysicsJoint( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        enableBodyCollision     ( bool enable );
    bool                        isBodyCollisionEnabled  ( ) const;
    cgPhysicsBody             * getBody0                ( );
    cgPhysicsBody             * getBody1                ( );


    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_PhysicsJoint; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    static void                 autoDestructor          ( const NewtonJoint * joint );
    static void                 getInfo                 ( const NewtonJoint * joint, NewtonJointRecord * info );
    static void                 submitConstraints       ( const NewtonJoint * joint, cgFloat timeStep, cgInt threadIndex );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        getLocalTransforms      ( const cgTransform & pinsAndPivotFrame, cgTransform & local0, cgTransform & local1 ) const;
    void                        getGlobalTransforms     ( const cgTransform & local0, const cgTransform & local1, cgTransform & global0, cgTransform & global1 ) const;

    //-------------------------------------------------------------------------
    // Protected Static Methods
    //-------------------------------------------------------------------------
    static void                 grammSchmidtBasis       ( const cgVector3 & dir, cgTransform & t );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                getInfo                 ( NewtonJointRecord * info ) = 0;
    virtual void                submitConstraints       ( cgFloat timeStep, cgInt threadIndex ) = 0;
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgPhysicsWorld    * mWorld;             // The parent physics world in which this joint has been created (if any).
    NewtonJoint       * mJoint;             // The Newton joint being wrapped
    cgPhysicsBody     * mBody0;             // The first body to which the joint is attached.
    cgPhysicsBody     * mBody1;             // The second body to which the joint is attached (if any).
    cgUInt32            mMaxDoF;            // Maximum number of degrees of freedom for this joint.
    bool                mBodyCollision;     // State that describes whether or not the bodies connected via this joint are able to collide with one another.
};

#endif // !_CGE_CGPHYSICSJOINT_H_