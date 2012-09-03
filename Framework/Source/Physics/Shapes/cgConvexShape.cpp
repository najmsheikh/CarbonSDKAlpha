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
// Name : cgConvexShape.cpp                                                  //
//                                                                           //
// Desc : Base class from which all physics shape that can easily be         //
//        classified as convex should derive. Such shapes might include      //
//        boxes, spheres, cylinders, capsules and cones for example.         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgConvexShape Module Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexShape.h>

///////////////////////////////////////////////////////////////////////////////
// cgConvexShape Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgConvexShape () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexShape::cgConvexShape( cgPhysicsWorld * pWorld ) : cgPhysicsShape( pWorld )
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : ~cgConvexShape () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConvexShape::~cgConvexShape()
{
    // Clear variables
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConvexShape::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ConvexShape )
        return true;

    // Supported by base?
    return cgPhysicsShape::queryReferenceType( type );
}