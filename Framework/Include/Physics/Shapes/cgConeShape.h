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
// Name : cgConeShape.h                                                      //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        cone shape.                                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCONESHAPE_H_ )
#define _CGE_CGCONESHAPE_H_

//-----------------------------------------------------------------------------
// cgConeShape Header Includes
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
// {054A90A3-B9CB-4C46-ADCE-1B23C5BFA069}
const cgUID RTID_ConeShape = {0x54A90A3, 0xB9CB, 0x4C46, {0xAD, 0xCE, 0x1B, 0x23, 0xC5, 0xBF, 0xA0, 0x69}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgConeShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for a simple cone
/// shape.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgConeShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgConeShape, cgConvexShape, "ConeShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConeShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height );
             cgConeShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height, const cgTransform & offset );
             cgConeShape( cgPhysicsWorld * world, const cgVector3 & dimensions );
             cgConeShape( cgPhysicsWorld * world, const cgVector3 & dimensions, const cgTransform & offset );
             cgConeShape( cgPhysicsWorld * world, const cgBoundingBox & bounds );
             cgConeShape( cgPhysicsWorld * world, const cgBoundingBox & bounds, const cgTransform & offset );
    virtual ~cgConeShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ConeShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGCONESHAPE_H_