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
// Name : cgConvexShape.h                                                    //
//                                                                           //
// Desc : Base class from which all physics shape that can easily be         //
//        classified as convex should derive. Such shapes might include      //
//        boxes, spheres, cylinders, capsules and cones for example.         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCONVEXSHAPE_H_ )
#define _CGE_CGCONVEXSHAPE_H_

//-----------------------------------------------------------------------------
// cgConvexShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/cgPhysicsShape.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {43A73A7B-88C2-4A03-A9E4-C4E6BF114E6B}
const cgUID RTID_ConvexShape = {0x43A73A7B, 0x88C2, 0x4A03, {0xA9, 0xE4, 0xC4, 0xE6, 0xBF, 0x11, 0x4E, 0x6B}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgConvexShape (Class)
/// <summary>
/// Base class from which all physics shape that can easily be classified
/// as convex should derive. Such shapes might include boxes, spheres, 
/// cylinders, capsules and cones for example.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgConvexShape : public cgPhysicsShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgConvexShape, cgPhysicsShape, "ConvexShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConvexShape( cgPhysicsWorld * world );
    virtual ~cgConvexShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ConvexShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    
};

#endif // !_CGE_CGCONVEXSHAPE_H_