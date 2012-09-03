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
// Name : cgNullShape.cpp                                                    //
//                                                                           //
// Desc : Class implementing a valid shape that has no collidable geometry.  //
//        This allows a dynamics body to be simulated and constrained with   //
//        joints for example, without taking part in the collision detection //
//        phase.                                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNullShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgNullShape.h>
#include <Physics/cgPhysicsWorld.h>

// Newton Game Dynamics
#include <Newton.h>

///////////////////////////////////////////////////////////////////////////////
// cgNullShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNullShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNullShape::cgNullShape( cgPhysicsWorld * pWorld ) : cgConvexShape( pWorld )
{
    // Construct the wrapped Newton shape.
    mShape = NewtonCreateNull( pWorld->getInternalWorld() );
}

//-----------------------------------------------------------------------------
//  Name : ~cgNullShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNullShape::~cgNullShape()
{

}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNullShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NullShape )
        return true;

    // Supported by base?
    return cgConvexShape::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : compare () (Virtual)
/// <summary>
/// Compare the physics shapes to see if they describe the same data.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::Dispose()" />
//-----------------------------------------------------------------------------
cgInt cgNullShape::compare( cgPhysicsShape * pShape ) const
{
    // Compare base properties first.
    cgInt nResult = cgPhysicsShape::compare( pShape );
    if ( nResult != 0 )
        return nResult;

    // Now compare custom properties.
    cgNullShape * pNullShape = (cgNullShape*)pShape;
    
    // Equivalent
    return 0;
}