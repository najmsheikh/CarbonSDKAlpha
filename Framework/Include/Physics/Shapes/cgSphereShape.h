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
// Name : cgSphereShape.h                                                    //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        sphere shape.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPHERESHAPE_H_ )
#define _CGE_CGSPHERESHAPE_H_

//-----------------------------------------------------------------------------
// cgSphereShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexShape.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgVector3;
class cgBoundingBox;
class cgTransform;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {FA383F3D-6C24-488B-B894-A175063E8CF8}
const cgUID RTID_SphereShape = {0xFA383F3D, 0x6C24, 0x488B, {0xB8, 0x94, 0xA1, 0x75, 0x6, 0x3E, 0x8C, 0xF8}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSphereShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for a simple sphere
/// shape.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSphereShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgSphereShape, cgConvexShape, "SphereShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSphereShape( cgPhysicsWorld * world, cgFloat radiusX, cgFloat radiusY, cgFloat radiusZ );
             cgSphereShape( cgPhysicsWorld * world, cgFloat radiusX, cgFloat radiusY, cgFloat radiusZ, const cgTransform & offset );
             cgSphereShape( cgPhysicsWorld * world, const cgVector3 & dimensions );
             cgSphereShape( cgPhysicsWorld * world, const cgVector3 & dimensions, const cgTransform & offset );
             cgSphereShape( cgPhysicsWorld * world, const cgBoundingBox & bounds );
             cgSphereShape( cgPhysicsWorld * world, const cgBoundingBox & bounds, const cgTransform & offset );
    virtual ~cgSphereShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_SphereShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGSPHERESHAPE_H_