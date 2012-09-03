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
// Name : cgJointObject.h                                                    //
//                                                                           //
// Desc : Our physics joint class implemented as a scene object that can be  //
//        managed and controlled in the same way as any other object.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGJOINTOBJECT_H_ )
#define _CGE_CGJOINTOBJECT_H_

//-----------------------------------------------------------------------------
// cgJointObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgWorldQuery.h>
#include <World/cgObjectNode.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {BE287E70-917C-40D1-92DE-AE2AB5B453D8}
const cgUID RTID_JointObject = {0xBE287E70, 0x917C, 0x40D1, {0x92, 0xDE, 0xAE, 0x2A, 0xB5, 0xB4, 0x53, 0xD8}};
// {433F52B7-F066-45B4-9801-FE388799D5E8}
const cgUID RTID_JointNode = {0x433F52B7, 0xF066, 0x45B4, {0x98, 0x1, 0xFE, 0x38, 0x87, 0x99, 0xD5, 0xE8}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgJointObject (Base Class)
/// <summary>
/// An individual physics joint object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgJointObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgJointObject, cgWorldObject, "JointObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgJointObject( cgUInt32 referenceId, cgWorld * world );
             cgJointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgJointObject( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual bool                getSubElementCategories     ( cgObjectSubElementCategory::Map & categories ) const;
    virtual void                applyObjectRescale          ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_JointObject; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );
};

//-----------------------------------------------------------------------------
// Name : cgJointNode (Base Class)
/// <summary>
/// An individual physics joint object node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgJointNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgJointNode, cgObjectNode, "JointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgJointNode( cgUInt32 referenceId, cgScene * scene );
             cgJointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgJointNode( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    canScale                    ( ) const;
    virtual bool                    validateAttachment          ( cgObjectNode * node, bool nodeAsChild );
    virtual bool                    getSandboxIconInfo          ( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasFile, cgString & frameName, cgVector3 & iconOrigin );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_JointNode; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;
};

#endif // !_CGE_CGJOINTOBJECT_H_