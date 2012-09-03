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
// Name : cgBoxShape.h                                                       //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        box shape.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBOXSHAPE_H_ )
#define _CGE_CGBOXSHAPE_H_

//-----------------------------------------------------------------------------
// cgBoxShape Header Includes
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
// {5270F2C3-2CDA-427D-B560-7AFB8AC56585}
const cgUID RTID_BoxShape = {0x5270F2C3, 0x2CDA, 0x427D, {0xB5, 0x60, 0x7A, 0xFB, 0x8A, 0xC5, 0x65, 0x85}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBoxShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for a simple box
/// shape.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBoxShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBoxShape, cgConvexShape, "BoxShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBoxShape( cgPhysicsWorld * world, cgFloat width, cgFloat height, cgFloat depth );
             cgBoxShape( cgPhysicsWorld * world, cgFloat width, cgFloat height, cgFloat depth, const cgTransform & offset );
             cgBoxShape( cgPhysicsWorld * world, const cgVector3 & dimensions );
             cgBoxShape( cgPhysicsWorld * world, const cgVector3 & dimensions, const cgTransform & offset );
             cgBoxShape( cgPhysicsWorld * world, const cgBoundingBox & bounds );
             cgBoxShape( cgPhysicsWorld * world, const cgBoundingBox & bounds, const cgTransform & offset );
    virtual ~cgBoxShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_BoxShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGBOXSHAPE_H_