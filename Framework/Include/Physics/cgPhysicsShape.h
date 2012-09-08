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
// Name : cgPhysicsShape.h                                                   //
//                                                                           //
// Desc : Base class from which specific physics shapes should derive.       //
//        Shapes provide the means by which a body can describes its         //
//        interior volume(s) or geometry used for the provision of both      //
//        collision detection and dynamics response. The shape itself does   //
//        not exist within the physics world, but can be attached to a body  //
//        that does.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPHYSICSSHAPE_H_ )
#define _CGE_CGPHYSICSSHAPE_H_

//-----------------------------------------------------------------------------
// cgPhysicsShape Header Includes
//-----------------------------------------------------------------------------
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgPhysicsWorld;
struct NewtonCollision;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {2A19ADCA-0E53-456E-B571-53414F7DEC74}
const cgUID RTID_PhysicsShape = {0x2A19ADCA, 0xE53, 0x456E, {0xB5, 0x71, 0x53, 0x41, 0x4F, 0x7D, 0xEC, 0x74}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPhysicsShape (Class)
/// <summary>
/// Base class from which specific physics shapes should derive. Shapes 
/// provide the means by which a body can describes its interior 
/// volume(s) or geometry used for the provision of both collision 
/// detection and dynamics response. The shape itself does not exist 
/// within the physics world, but can be attached to a body that does.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgPhysicsShape : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgPhysicsShape, cgReference, "PhysicsShape" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgRigidBody;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgPhysicsShape( cgPhysicsWorld * world );
    virtual ~cgPhysicsShape( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    // Internal utilities
    NewtonCollision           * getInternalShape        ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_PhysicsShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgPhysicsWorld    * mWorld;         // The parent physics world in which this shape has been created (if any).
    NewtonCollision   * mShape;         // The Newton shape being wrapped
    bool                mShapeCached;   // Shape exists in the world's shape cache?
};

//-----------------------------------------------------------------------------
//  Name : cgPhysicsShapeCacheKey (Class)
/// <summary>
/// Implements comparison operator for physics shape cache.
/// </summary>
//-----------------------------------------------------------------------------
class cgPhysicsShapeCacheKey
{
public:
    cgPhysicsShapeCacheKey( cgPhysicsShape * shape );

    // Comparison operator
    virtual bool operator < ( const cgPhysicsShapeCacheKey & v ) const;

    // Cache values
    cgPhysicsShape  * mShape;
};

#endif // !_CGE_CGPHYSICSSHAPE_H_