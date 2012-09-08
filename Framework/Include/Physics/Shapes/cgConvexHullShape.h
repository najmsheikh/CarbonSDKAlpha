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
// Name : cgConvexHullShape.h                                                //
//                                                                           //
// Desc : Class implementing collision / dynamics properties for an          //
//        automatically generated convex hull shape constructed from the     //
//        supplied vertex data.                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//
#pragma once
#if !defined( _CGE_CGCONVEXHULLSHAPE_H_ )
#define _CGE_CGCONVEXHULLSHAPE_H_

//-----------------------------------------------------------------------------
// cgConvexHullShape Header Includes
//-----------------------------------------------------------------------------
#include <Physics/Shapes/cgConvexShape.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgResourceHandles.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgTransform;
class cgResourceManager;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {F47BF6C0-FEBE-4995-BD31-F2754C3747E6}
const cgUID RTID_ConvexHullShape = {0xF47BF6C0, 0xFEBE, 0x4995, {0xBD, 0x31, 0xF2, 0x75, 0x4C, 0x37, 0x47, 0xE6}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgConvexHullShape (Class)
/// <summary>
/// Class implementing collision / dynamics properties for an automatically 
/// generated convex hull shape constructed from the supplied vertex data.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgConvexHullShape : public cgConvexShape
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgConvexHullShape, cgConvexShape, "ConvexHullShape" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgConvexHullShape( cgPhysicsWorld * world, void * serializedBuffer, cgUInt32 dataSize );
             cgConvexHullShape( cgPhysicsWorld * world, const cgByteArray & serializedBuffer );
             cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride );
             cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, const cgTransform & offset );
             cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, cgFloat collapseTolerance );
             cgConvexHullShape( cgPhysicsWorld * world, void * vertexData, cgUInt32 vertexCount, cgUInt32 stride, cgFloat collapseTolerance, const cgTransform & offset );
    virtual ~cgConvexHullShape( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        serializeShape          ( cgByteArray & serializedBuffer );
    cgMeshHandle                buildRenderMesh         ( cgResourceManager * resourceManager );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgPhysicsShape)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                 ( cgPhysicsShape * shape ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_ConvexHullShape; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Protected Static Functions
    //-------------------------------------------------------------------------
    static void                 deserialize             ( void * serializeHandle, void * buffer, cgInt size );
    static void                 serialize               ( void * serializeHandle, const void * buffer, cgInt size );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CGE_CGCONVEXHULLSHAPE_H_