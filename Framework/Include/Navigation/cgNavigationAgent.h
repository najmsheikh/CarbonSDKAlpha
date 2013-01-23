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
// Name : cgNavigationAgent.h                                                //
//                                                                           //
// Desc : A navigation agent represents an entity that can be issued with    //
//        instructions for navigating along / through a provided navigation  //
//        mesh. This object will ultimately provide transformation details   //
//        to the owner outlining where the agent is in the world at any      //
//        point in time as it navigates.                                     //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGNAVIGATIONAGENT_H_ )
#define _CGE_CGNAVIGATIONAGENT_H_

//-----------------------------------------------------------------------------
// cgNavigationAgent Header Includes
//-----------------------------------------------------------------------------
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>
#include <Navigation/cgNavigationTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgNavigationHandler;
class  cgNavigationAgent;
struct cgNavigationAgentCreateParams;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {5517B6E1-170B-48D2-80A0-26016592A0E2}
const cgUID RTID_NavigationAgent = { 0x5517B6E1, 0x170B, 0x48D2, { 0x80, 0xA0, 0x26, 0x01, 0x65, 0x92, 0xA0, 0xE2 } };

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgNavigationAgentRepositionEventArgs
{
    cgNavigationAgentRepositionEventArgs( const cgVector3 & _newPosition, bool _navigationUpdate ) :
        newPosition( _newPosition ), navigationUpdate( _navigationUpdate ) {}
    cgVector3 newPosition;
    bool navigationUpdate;
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgNavigationAgentEventListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever navigation agent events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationAgentEventListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationAgentEventListener, cgEventListener, "NavigationAgentEventListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onNavigationAgentReposition ( cgNavigationAgent * sender, cgNavigationAgentRepositionEventArgs * e ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgNavigationAgent (Class)
/// <summary>
/// A navigation agent represents an entity that can be issued with 
/// instructions for navigating along / through a provided navigation mesh.
/// This object will ultimately provide transformation details to the owner 
/// outlining where the agent is in the world at any point in time as it 
/// navigates.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgNavigationAgent : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgNavigationAgent, cgReference, "NavigationAgent" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgNavigationAgent( cgNavigationHandler * handler );
    virtual ~cgNavigationAgent( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                            initialize          ( const cgNavigationAgentCreateParams & params, const cgVector3 & position );
    bool                            setMoveTarget       ( const cgVector3 & position, bool adjust );
    bool                            setMoveVelocity     ( const cgVector3 & velocity );
    cgVector3                       getPosition         ( ) const;
    cgVector3                       getDesiredVelocity  ( ) const;
    cgVector3                       getActualVelocity   ( ) const;
    cgNavigationAgentState::Base    getAgentState       ( ) const;
    cgNavigationTargetState::Base   getTargetState      ( ) const;
    cgNavigationHandler           * getHandler          ( ) const;
    void                            updateParameters    ( const cgNavigationAgentCreateParams & params );
    const cgNavigationAgentCreateParams & getParameters ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void                    onNavigationAgentReposition ( cgNavigationAgentRepositionEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_NavigationAgent; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgNavigationHandler           * mHandler;           // The handler that owns this agent.
    cgNavigationAgentCreateParams   mParams;            // Parameters specified during agent creation.
    cgInt                           mDetourIndex;       // Detour reference to this agent.
    cgUInt                          mTargetRef;         // Reference to poly closest to requested target position.
    cgVector3                       mTargetPosition;    // Actual target position, closest to that requested.
};

#endif // !_CGE_CGNAVIGATIONAGENT_H_