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
// Name : cgCylinderShape.h                                                  //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        cylinder shape.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCYLINDERSHAPE_H_ )
#define _CGE_CGCYLINDERSHAPE_H_

//-----------------------------------------------------------------------------
// cgCylinderShape Header Includes
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
// {81CCC842-3C10-4480-94E9-F1F14A879AD9}
const cgUID RTID_CylinderShape = {0x81CCC842, 0x3C10, 0x4480, {0x94, 0xE9, 0xF1, 0xF1, 0x4A, 0x87, 0x9A, 0xD9}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCylinderShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for a simple cylinder
/// shape.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCylinderShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCylinderShape, cgConvexShape, "CylinderShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCylinderShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height );
             cgCylinderShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height, const cgTransform & offset );
             cgCylinderShape( cgPhysicsWorld * world, const cgVector3 & dimensions );
             cgCylinderShape( cgPhysicsWorld * world, const cgVector3 & dimensions, const cgTransform & offset );
             cgCylinderShape( cgPhysicsWorld * world, const cgBoundingBox & bounds );
             cgCylinderShape( cgPhysicsWorld * world, const cgBoundingBox & bounds, const cgTransform & offset );
    virtual ~cgCylinderShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CylinderShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGCYLINDERSHAPE_H_