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
// Name : cgCapsuleShape.h                                                   //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for a simple    //
//        capsule shape.                                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCAPSULESHAPE_H_ )
#define _CGE_CGCAPSULESHAPE_H_

//-----------------------------------------------------------------------------
// cgCapsuleShape Header Includes
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
// {11A8755F-06E1-4F64-BA0F-EAC8D130D185}
const cgUID RTID_CapsuleShape = {0x11A8755F, 0x6E1, 0x4F64, {0xBA, 0xF, 0xEA, 0xC8, 0xD1, 0x30, 0xD1, 0x85}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgCapsuleShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for a simple capsule
/// shape.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgCapsuleShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgCapsuleShape, cgConvexShape, "CapsuleShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgCapsuleShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height );
             cgCapsuleShape( cgPhysicsWorld * world, cgFloat radius, cgFloat height, const cgTransform & offset );
             cgCapsuleShape( cgPhysicsWorld * world, const cgVector3 & dimensions );
             cgCapsuleShape( cgPhysicsWorld * world, const cgVector3 & dimensions, const cgTransform & offset );
             cgCapsuleShape( cgPhysicsWorld * world, const cgBoundingBox & bounds );
             cgCapsuleShape( cgPhysicsWorld * world, const cgBoundingBox & bounds, const cgTransform & offset );
    virtual ~cgCapsuleShape( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_CapsuleShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGCAPSULESHAPE_H_