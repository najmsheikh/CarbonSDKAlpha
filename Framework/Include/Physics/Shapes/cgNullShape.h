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
// Name : cgNullShape.h                                                      //
//                                                                           //
// Desc : Class implementing a valid shape that has no collidable geometry.  //
//        This allows a dynamics body to be simulated and constrained with   //
//        joints for example, without taking part in the collision detection //
//        phase.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNULLSHAPE_H_ )
#define _CGE_CGNULLSHAPE_H_

//-----------------------------------------------------------------------------
// cgNullShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexShape.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {207BABE5-2656-4FFE-A6B0-5DDC265B513C}
const cgUID RTID_NullShape = {0x207BABE5, 0x2656, 0x4FFE, {0xA6, 0xB0, 0x5D, 0xDC, 0x26, 0x5B, 0x51, 0x3C}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNullShape (Class)
/// <summary>
/// Class implementing a valid shape that has no collidable geometry. This 
/// allows a dynamics body to be simulated and constrained with joints for 
/// example, without taking part in the collision detection phase.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNullShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNullShape, cgConvexShape, "NullShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNullShape( cgPhysicsWorld * world );
    virtual ~cgNullShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_NullShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGNULLSHAPE_H_