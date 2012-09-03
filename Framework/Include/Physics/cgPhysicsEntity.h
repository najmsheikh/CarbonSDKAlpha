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
// Name : cgPhysicsEntity.h                                                  //
//                                                                           //
// Desc : Base class from which all types of physics objects that have a     //
//        physical presence within the physics world should derive. This     //
//        most commonly includes bodies and joints for instance.             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSENTITY_H_ )
#define _CGE_CGPHYSICSENTITY_H_

//-----------------------------------------------------------------------------
// cgPhysicsEntity Header Includes
//-----------------------------------------------------------------------------
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5BCB3E05-BFBE-40EB-B5D2-5031662B6FAC}
const cgUID RTID_PhysicsEntity = {0x5BCB3E05, 0xBFBE, 0x40EB, {0xB5, 0xD2, 0x50, 0x31, 0x66, 0x2B, 0x6F, 0xAC}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsEntity (Class)
/// <summary>
/// Base class from which all types of physics objects that have a
/// physical presence within the physics world should derive. This
/// most commonly includes bodies and joints for instance.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsEntity: public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsEntity, cgReference, "PhysicsEntity" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsEntity( cgPhysicsWorld * world );
    virtual ~cgPhysicsEntity( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void               clearForces              ( );
    virtual cgTransform        getTransform             ( cgDouble interp = 1.0 ) const;
    virtual void               setTransform             ( const cgTransform & transform );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_PhysicsEntity; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgPhysicsWorld* mWorld;                 // The parent physics world in which this body exists (if any).
    cgVector3       mEntityPosition;        // Current position of the rigid body.
    cgQuaternion    mEntityRotation;        // Current orientation of the rigid body.
    cgVector3       mPrevEntityPosition;    // Previous position of the rigid body (for interpolation).
    cgQuaternion    mPrevEntityRotation;    // Previous orientation of the rigid body (for interpolation).
    
};

#endif // !_CGE_CGPHYSICSENTITY_H_