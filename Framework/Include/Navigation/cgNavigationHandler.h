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
// Name : cgNavigationHandler.h                                              //
//                                                                           //
// Desc : The top level class that owns and manages all currently defined    //
//        navigation agents.                                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONHANDLER_H_ )
#define _CGE_CGNAVIGATIONHANDLER_H_

//-----------------------------------------------------------------------------
// cgNavigationHandler Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgNavigationMesh;
class  cgNavigationAgent;
struct cgNavigationAgentCreateParams;
class  dtNavMeshQuery;
class  dtCrowd;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNavigationHandler (Class)
/// <summary>
/// The top level class that owns and manages all currently defined navigation 
/// agents.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationHandler : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgNavigationHandler, "NavigationHandler" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgNavigationAgent;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationHandler( );
    virtual ~cgNavigationHandler( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                initialize          ( cgNavigationMesh * navMesh, cgUInt32 maxAgents );
    cgNavigationAgent * createAgent         ( const cgNavigationAgentCreateParams & params, const cgVector3 & position );
    void                update              ( cgFloat timeDelta );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE( cgNavigationAgent*, AgentList )

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgNavigationMesh  * mNavMesh;   // The navigation mesh to which we are attached.
    AgentList           mAgents;    // List of allocated agents.
    dtCrowd           * mCrowd;     // The main detour crowd manager
    dtNavMeshQuery    * mQuery;     // System used to query the navigation mesh.
};

#endif // !_CGE_CGNAVIGATIONHANDLER_H_