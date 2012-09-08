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
// Name : cgFixedAxisJoint.h                                                 //
//                                                                           //
// Desc : Class implementing a simple axis pinning joint. This joint can be  //
//        used to 'fix' the orientation of a body to a specific axis while   //
//        allowing complete freedom to translate in any way required.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGFIXEDAXISJOINT_H_ )
#define _CGE_CGFIXEDAXISJOINT_H_

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
// {37A76A16-6F4F-46B2-A9DD-CA990214505B}
const cgUID RTID_FixedAxisJoint = {0x37A76A16, 0x6F4F, 0x46B2, {0xA9, 0xDD, 0xCA, 0x99, 0x2, 0x14, 0x50, 0x5B}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFixedAxisJoint (Class)
/// <summary>
/// Class implementing a simple axis pinning joint. This joint can be used to 
/// 'fix' the orientation of a body to a specific axis while allowing complete 
/// freedom to translate in any way required.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFixedAxisJoint : public cgPhysicsJoint
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgFixedAxisJoint, cgPhysicsJoint, "FixedAxisJoint" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFixedAxisJoint( cgPhysicsWorld * world, cgPhysicsBody * body, const cgVector3 & worldSpaceAxis );
    virtual ~cgFixedAxisJoint( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setAxis                 ( const cgVector3 & axis, bool reset );
    const cgVector3           & getAxis                 ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_FixedAxisJoint; }
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
    cgVector3       mAxis;              // Axis to which the body is being constrained
    cgTransform     mLocalPivotFrame;
    cgTransform     mGlobalPivotFrame;
};

#endif // !_CGE_CGFIXEDAXISJOINT_H_