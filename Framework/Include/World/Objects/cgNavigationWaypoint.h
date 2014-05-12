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
// Name : cgNavigationWaypoint.h                                             //
//                                                                           //
// Desc : Base navigation waypoint object from which different types of      //
//        waypoint can be derived (i.e. patrol, cover, etc.)                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONWAYPOINT_H_ )
#define _CGE_CGNAVIGATIONWAYPOINT_H_

//-----------------------------------------------------------------------------
// cgNavigationWaypoint Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {E116371A-13B3-41C7-A04B-2A5CB7C29312}
const cgUID RTID_NavigationWaypointObject = { 0xE116371A, 0x13B3, 0x41C7, { 0xA0, 0x4B, 0x2A, 0x5C, 0xB7, 0xC2, 0x93, 0x12 } };
// {CED87243-6765-405A-8638-006E7B0EB909}
const cgUID RTID_NavigationWaypointNode = { 0xCED87243, 0x6765, 0x405A, { 0x86, 0x38, 0x00, 0x6E, 0x7B, 0x0E, 0xB9, 0x09 } };;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgNavigationWaypointObject (Base Class)
/// <summary>
/// Base navigation waypoint object from which different types of waypoint 
/// can be derived (i.e. patrol, cover, etc.)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationWaypointObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationWaypointObject, cgWorldObject, "NavigationWaypointObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationWaypointObject( cgUInt32 referenceId, cgWorld * world );
             cgNavigationWaypointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgNavigationWaypointObject( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual cgBoundingBox       getLocalBoundingBox         ( );
    virtual void                sandboxRender               ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual bool                pick                        ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgUInt32 flags, cgFloat wireTolerance, cgFloat & distanceOut );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated          ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading          ( cgComponentLoadingEventArgs * e );
    virtual bool                createTypeTables            ( const cgUID & typeIdentifier );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_NavigationWaypointObject; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    bool                        insertComponentData         ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32                    mAvailability;  // Access rights for this waypoint (0 = available to all).
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBaseWaypoint;
    static cgWorldQuery     mLoadBaseWaypoint;
};

//-----------------------------------------------------------------------------
// Name : cgNavigationWaypointNode (Base Class)
/// <summary>
/// Base navigation waypoint node from which various types of required
/// navigation waypoint can be derived.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationWaypointNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationWaypointNode, cgObjectNode, "NavigationWaypointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationWaypointNode( cgUInt32 referenceId, cgScene * scene );
             cgNavigationWaypointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgNavigationWaypointNode( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    canScale                    ( ) const;
    virtual bool                    onNodeCreated               ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                    onNodeLoading               ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_NavigationWaypointNode; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                     ( bool disposeBase );
    
};

#endif // !_CGE_CGNAVIGATIONWAYPOINT_H_