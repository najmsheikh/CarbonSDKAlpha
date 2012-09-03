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
// Name : cgFixedAxisJointObject.h                                           //
//                                                                           //
// Desc : Class implementing a simple axis pinning joint as a scene object.  //
//        This joint can be used to 'fix' the orientation of a body to a     //
//        specific axis while allowing complete freedom to translate in any  //
//        way required.                                                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGFIXEDAXISJOINTOBJECT_H_ )
#define _CGE_CGFIXEDAXISJOINTOBJECT_H_

//-----------------------------------------------------------------------------
// cgFixedAxisJointObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldQuery.h>
#include <World/Objects/cgJointObject.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgFixedAxisJoint;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {D4B658BF-21CE-4D8D-8CB9-3315DF4D7D27}
const cgUID RTID_FixedAxisJointNode   = {0xD4B658BF, 0x21CE, 0x4D8D, {0x8C, 0xB9, 0x33, 0x15, 0xDF, 0x4D, 0x7D, 0x27}};
// {7CE576EC-177A-45F6-A952-2E2F1CD4FF33}
const cgUID RTID_FixedAxisJointObject = {0x7CE576EC, 0x177A, 0x45F6, {0xA9, 0x52, 0x2E, 0x2F, 0x1C, 0xD4, 0xFF, 0x33}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFixedAxisJointObject (Class)
/// <summary>
/// Class implementing a simple axis pinning joint as a scene object. This 
/// joint can be used to 'fix' the orientation of a body to a specific axis 
/// while allowing complete freedom to translate in any way required.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFixedAxisJointObject : public cgJointObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgFixedAxisJointObject, cgJointObject, "FixedAxisJointObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFixedAxisJointObject( cgUInt32 referenceId, cgWorld * world );
             cgFixedAxisJointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgFixedAxisJointObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject      * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject      * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Public Virtual Methods (cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual void                sandboxRender           ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, const cgVector3 & wireTolerance, cgFloat & distanceOut );
    virtual void                applyObjectRescale      ( cgFloat scale );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString            getDatabaseTable        ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_FixedAxisJointObject; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        insertComponentData     ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertJoint;
    static cgWorldQuery     mLoadJoint;
};

//-----------------------------------------------------------------------------
//  Name : cgFixedAxisJointNode (Class)
/// <summary>
/// Custom node type for the fixed axis joint object. Manages additional 
/// properties that may need to be overriden by this type.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFixedAxisJointNode : public cgJointNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgFixedAxisJointNode, cgJointNode, "FixedAxisJointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFixedAxisJointNode( cgUInt32 referenceId, cgScene * scene );
             cgFixedAxisJointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgFixedAxisJointNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode       * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode       * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_FixedAxisJointNode; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                getSandboxIconInfo      ( cgCameraNode * camera, const cgSize & viewportSize, cgString & strAtlas, cgString & strFrame, cgVector3 & vIconOrigin );
    virtual cgBoundingBox       getLocalBoundingBox     ( );
    virtual bool                setParent               ( cgObjectNode * parent, bool constructing );
    virtual bool                setCellTransform        ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFixedAxisJoint  * mJoint;   // The actual physics joint.
};

#endif // !_CGE_CGFIXEDAXISJOINTOBJECT_H_