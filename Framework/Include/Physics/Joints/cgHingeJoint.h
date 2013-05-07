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
// Name : cgHingeJoint.h                                                     //
//                                                                           //
// Desc : Class implementing a single axis hinge joint. This joint can be    //
//        used to connect two bodies around a central axis and allow them    //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGHINGEJOINT_H_ )
#define _CGE_CGHINGEJOINT_H_

//-----------------------------------------------------------------------------
// cgHingeJoint Header Includes
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
// {180C83F9-941F-4AE2-82EC-3F7DB3B3A150}
const cgUID RTID_HingeJoint = {0x180C83F9, 0x941F, 0x4AE2, {0x82, 0xEC, 0x3F, 0x7D, 0xB3, 0xB3, 0xA1, 0x50}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgHingeJoint (Class)
/// <summary>
/// Class implementing a single axis hinge joint. This joint can be used to 
/// connect two bodies around a central axis and allow them to rotate freely 
/// with optional angle constraints.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgHingeJoint : public cgPhysicsJoint, public cgPhysicsBodyEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgHingeJoint, cgPhysicsJoint, "HingeJoint" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgHingeJoint( cgPhysicsWorld * world, cgPhysicsBody * parent, cgPhysicsBody * child, const cgTransform & pivotTransform );
    virtual ~cgHingeJoint( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        enableLimits                ( bool enable );
    void                        setLimits                   ( cgFloat minDegrees, cgFloat maxDegrees );
    void                        setPivotTransform           ( const cgTransform & pivotTransform );
    cgTransform                 getPivotTransform           ( bool original ) const;
    cgFloat                     getAngle                    ( ) const;
    cgFloat                     getRelativeAngularVelocity  ( ) const;
    cgVector3                   getAxis                     ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsBodyEventListener)
    //-------------------------------------------------------------------------
    virtual void                onPhysicsBodyTransformed    ( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_HingeJoint; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgFloat                     computeNewJointAngle        ( cgFloat newAngleCos, cgFloat newAngleSin );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgPhysicsJoint)
    //-------------------------------------------------------------------------
    virtual void                getInfo                     ( NewtonJointRecord * info );
    virtual void                submitConstraints           ( cgFloat timeStep, cgInt threadIndex );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool            mUseLimits;
    cgFloat         mMinimumAngle;      // Radians
    cgFloat         mMaximumAngle;      // Radians
    cgFloat         mJointAngle;        // Radians
    cgFloat         mJointOmega;
    cgTransform     mGlobalPivotFrame;
    cgTransform     mLocalPivotFrame0;
    cgTransform     mLocalPivotFrame1;
};

#endif // !_CGE_CGHINGEJOINT_H_