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
// Name : cgBallJoint.h                                                      //
//                                                                           //
// Desc : Class implementing a ball and socket joint. This joint can be      //
//        used to connect two bodies around a central point and allow them   //
//        to rotate freely with optional angle constraints.                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBALLJOINT_H_ )
#define _CGE_CGBALLJOINT_H_

//-----------------------------------------------------------------------------
// cgFixedAxisJoint Header Includes
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
// {23C44136-E5E0-454B-A99A-2F0A9E784165}
const cgUID RTID_BallJoint = { 0x23c44136, 0xe5e0, 0x454b, { 0xa9, 0x9a, 0x2f, 0xa, 0x9e, 0x78, 0x41, 0x65 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBallJoint (Class)
/// <summary>
/// Class implementing a ball and socket joint. This joint can be used to 
/// connect two bodies around a central point and allow them to rotate freely 
/// with optional angle constraints.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBallJoint : public cgPhysicsJoint, public cgPhysicsBodyEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBallJoint, cgPhysicsJoint, "BallJoint" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBallJoint( cgPhysicsWorld * world, cgPhysicsBody * parent, cgPhysicsBody * child, const cgTransform & pivotTransform );
    virtual ~cgBallJoint( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        enableLimits                ( bool enable );
    void                        setTwistLimits              ( cgFloat minDegrees, cgFloat maxDegrees );
    void                        setConeLimit                ( cgFloat degrees );
    void                        setPivotTransform           ( const cgTransform & pivotTransform );
    cgTransform                 getPivotTransform           ( bool original ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsBodyEventListener)
    //-------------------------------------------------------------------------
    virtual void                onPhysicsBodyTransformed    ( cgPhysicsBody * sender, cgPhysicsBodyTransformedEventArgs * e );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_BallJoint; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    //cgFloat                     computeNewJointAngle        ( cgFloat newAngleCos, cgFloat newAngleSin );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgPhysicsJoint)
    //-------------------------------------------------------------------------
    virtual void                getInfo                     ( NewtonJointRecord * info );
    virtual void                submitConstraints           ( cgFloat timeStep, cgInt threadIndex );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool            mUseLimits;
    cgFloat         mMinimumTwistAngle; // Radians
    cgFloat         mMaximumTwistAngle; // Radians
    cgFloat         mMaximumConeAngle;  // Radians
    cgTransform     mGlobalPivotFrame;
    cgTransform     mLocalPivotFrame0;
    cgTransform     mLocalPivotFrame1;

    // Cached values (performance)
    cgFloat         mConeAngleCos;
    cgFloat         mConeAngleSin;
    cgFloat         mConeAngleHalfCos;
    cgFloat         mConeAngleHalfSin;

    /*cgFloat         mJointAngle;        // Radians
    cgFloat         mJointOmega;*/
};

#endif // !_CGE_CGBALLJOINT_H_