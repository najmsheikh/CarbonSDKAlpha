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
// Name : cgNavigationPatrolPoint.h                                          //
//                                                                           //
// Desc : Specialized navigation waypoint designed to provide points of      //
//        interest for different types of navigation agent during regular    //
//        patrol pathing.                                                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONPATROLPOINT_H_ )
#define _CGE_CGNAVIGATIONPATROLPOINT_H_

//-----------------------------------------------------------------------------
// cgNavigationPatrolPoint Header Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgNavigationWaypoint.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {EF636224-36E2-4B2A-B057-EF28ABA62877}
const cgUID RTID_NavigationPatrolPointNode   = { 0xef636224, 0x36e2, 0x4b2a, { 0xb0, 0x57, 0xef, 0x28, 0xab, 0xa6, 0x28, 0x77 } };
// {76E18ADA-AB37-42C0-91BC-E0AE2A5C9D49}
const cgUID RTID_NavigationPatrolPointObject = { 0x76e18ada, 0xab37, 0x42c0, { 0x91, 0xbc, 0xe0, 0xae, 0x2a, 0x5c, 0x9d, 0x49 } };

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgNavigationPatrolPointObject (Class)
/// <summary>
/// Specialized navigation waypoint designed to provide points of interest for 
/// different types of navigation agent during regular patrol pathing.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationPatrolPointObject : public cgNavigationWaypointObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationPatrolPointObject, cgNavigationWaypointObject, "NavigationPatrolPointObject" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationPatrolPointObject( cgUInt32 referenceId, cgWorld * world );
             cgNavigationPatrolPointObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgNavigationPatrolPointObject( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgWorldObject          * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgWorld * world );
    static cgWorldObject          * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    virtual cgString                getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_NavigationPatrolPointObject; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                            prepareQueries          ( );
    bool                            insertComponentData     ( );

	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    
    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertPatrolPoint;
    static cgWorldQuery     mLoadPatrolPoint;
};

//-----------------------------------------------------------------------------
// Name : cgNavigationPatrolPointNode (Base Class)
/// <summary>
/// An individual navigation patrol point object node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationPatrolPointNode : public cgNavigationWaypointNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationPatrolPointNode, cgNavigationWaypointNode, "NavigationPatrolPointNode" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationPatrolPointNode( cgUInt32 referenceId, cgScene * scene );
             cgNavigationPatrolPointNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgNavigationPatrolPointNode( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgObjectNode           * allocateNew             ( const cgUID & type, cgUInt32 referenceId, cgScene * scene );
    static cgObjectNode           * allocateClone           ( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_NavigationPatrolPointNode; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );
};

#endif // !_CGE_CGNAVIGATIONPATROLPOINT_H_