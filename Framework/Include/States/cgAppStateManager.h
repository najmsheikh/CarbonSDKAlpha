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
// Name : cgAppStateManager.h                                                //
//                                                                           //
// Desc : This module houses classes responsible for managing the state of   //
//        the application i.e. Main menu, game play, game over, credits etc. //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAPPSTATEMANAGER_H_ )
#define _CGE_CGAPPSTATEMANAGER_H_

//-----------------------------------------------------------------------------
// cgAppStateManager Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgAppState;
class cgAppTransitionState;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {20E186CC-E1C8-4FF8-8698-46D8B337AA23}
const cgUID RTID_AppState           = {0x20E186CC, 0xE1C8, 0x4FF8, {0x86, 0x98, 0x46, 0xD8, 0xB3, 0x37, 0xAA, 0x23}};
// {88398CC0-4649-45FE-83A9-EE77EEE605F7}
const cgUID RTID_AppTransitionState = {0x88398CC0, 0x4649, 0x45FE, {0x83, 0xA9, 0xEE, 0x77, 0xEE, 0xE6, 0x5, 0xF7}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAppStateManager (Class)
/// <summary>
/// Class for the management of all states that the game can be in.
/// Includes methods for registering possible states, transitioning to
/// and from states and general state management.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAppStateManager : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgAppStateManager, "AppStateManager" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgAppState;
    friend class cgAppTransitionState;

public:
    //-------------------------------------------------------------------------
    // Public Structures, Enumerations & Typedefs
    //-------------------------------------------------------------------------
    enum StateType
    {
        StateType_Normal        = 0,
        StateType_Transition    = 1
    
    }; // End Enum StateType
    
    struct StateDesc
    {
        StateType   type;
        cgAppState* state;
    
    }; // End Struct StateDesc

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAppStateManager( );
    virtual ~cgAppStateManager( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgAppStateManager  * getInstance      ( );
    static void                 createSingleton  ( );
    static void                 destroySingleton ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool            registerState       ( cgAppState * state );
    void            unregisterState     ( const cgString & strStateId );
    cgAppState    * getActiveState      ( );
    bool            setActiveState      ( const cgString & stateId );
    bool            getStateDesc        ( const cgString & stateId, StateDesc * descriptionOut );
    cgAppState    * getState            ( const cgString & stateId );
    void            update              ( );
    void            render              ( );
    void            stop                ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Structures, Enumerations & Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgString, StateDesc, StateMap)

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool            transitionState     ( cgAppState * fromState, const cgString & toStateId, const cgString & viaStateId );
    bool            transitionState     ( cgAppState * fromState, const cgString & toStateId );
    cgAppState    * spawnChildState     ( cgAppState * parentState, const cgString & newStateId, bool suspendParent = false );
    cgAppState    * allocateState       ( const cgString & stateUID );
    void            stateEnded          ( cgAppState * state );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    StateMap        mRegisteredStates;  // All state types that have been registered with the system
    cgAppState    * mStateHistory;      // Cached pointer of the root of the active state, this is effectively the root of a list of restorable / suspended states.
    cgAppState    * mActiveState;       // The currently active state that the game is currently in (restorable states will be available via its parent)

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgAppStateManager * mSingleton;   // Static singleton object instance.
};

//-----------------------------------------------------------------------------
//  Name : cgAppState (Class)
/// <summary>
/// Designed as the base class from which an application state is
/// derived. An application state is an encapsulated mode that the app
/// can be in, i.e. menu, exterior gameplay, interior game play, credits
/// scroller etc. Supports the concept of events which can be raised in
/// order to trigger state transitions, as well as supplying additional
/// support for multiple levels of states being active at any one time.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAppState : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAppState, cgReference, "AppState" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgAppStateManager;
    friend class cgAppTransitionState;

public:
    //-------------------------------------------------------------------------
    // Public Structures, Enumerations & Typedefs
    //-------------------------------------------------------------------------
    enum EventActionType
    {
        ActionType_Transition           = 0,
        ActionType_TransitionRoot       = 1,
        ActionType_SpawnChild           = 2,
        ActionType_EndState             = 3,
        ActionType_EndRoot              = 4,
        ActionType_PassUp               = 5,
        ActionType_PassDown             = 6
    
    }; // End Enum EventActionType

    enum EventActionFlags
    {
        ActionFlag_None                 = 0,
        ActionFlag_SuspendParent        = 1,
        ActionFlag_UseTransitionState   = 2,
    
    }; // End Enum EventActionFlags

    struct EventActionDesc
    {
        EventActionType actionType;
        cgUInt32        flags;
        cgString        toStateId;
        cgString        transitionStateId;
        cgInt32         stackOffset;

        EventActionDesc() : flags(0), stackOffset(0) {}
    
    }; // End Struct EventActionDesc

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAppState( const cgString & stateId );
             cgAppState( const cgString & stateId, const cgInputStream & scriptStream, cgResourceManager * resourceManager );
    virtual ~cgAppState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    isActive                ( ) const;
    bool                    isSuspended             ( ) const;
    bool                    isScripted              ( ) const;
    cgAppTransitionState  * getOutgoingTransition   ( );
    cgAppTransitionState  * getIncomingTransition   ( );
    cgAppState            * getRootState            ( );
    cgAppState            * getTerminalState        ( );
    cgAppState            * getParentState          ( );
    const cgString        & getStateId              ( ) const;
    void                    raiseEvent              ( const cgString & eventName );
    cgAppStateManager     * getManager              ( );
    cgAppState            * spawnChildState         ( const cgString & stateId, bool suspendParent );
    const cgScriptHandle  & getScript               ( ) const;
    cgScriptObject        * getScriptObject         ( );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // State Activation
    virtual bool            initialize              ( );
    virtual bool            begin                   ( );
    virtual void            end                     ( );

    // State Frame Processing
    virtual void            update                  ( );
    virtual void            render                  ( );

    // State Event Management
    virtual void            suspend                 ( );
    virtual void            resume                  ( );
    virtual void            raiseEvent              ( const cgString & eventName, bool force );
    virtual bool            registerEventAction     ( const cgString & eventName, const EventActionDesc & description );
    virtual void            unregisterEventAction   ( const cgString & eventName );

    // Query Routines
    virtual bool            isTransitionState       ( ) const { return false; }
    virtual bool            isBegun                 ( ) const { return mBegun; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_AppState; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;
    virtual bool            processMessage          ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void                    processEvent            ( const cgString & eventName );

protected:
    
    //-------------------------------------------------------------------------
    // Protected Structures, Enumerations & Typedefs
    //-------------------------------------------------------------------------
    struct CGE_API MethodHandles
    {
        // Public methods
        cgScriptFunctionHandle  begin;
        cgScriptFunctionHandle  end;
        cgScriptFunctionHandle  initialize;
        cgScriptFunctionHandle  update;
        cgScriptFunctionHandle  render;
        cgScriptFunctionHandle  suspend;
        cgScriptFunctionHandle  resume;
        cgScriptFunctionHandle  processMessage;
        cgScriptFunctionHandle  raiseEvent;
        
        // Constructor
        MethodHandles() :
            begin(CG_NULL), end(CG_NULL), initialize(CG_NULL), update(CG_NULL),
            render(CG_NULL), suspend(CG_NULL), resume(CG_NULL), processMessage(CG_NULL),
            raiseEvent(CG_NULL) {}
    };

    CGE_MAP_DECLARE(cgString, EventActionDesc, EventActionMap)

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool                        bindToScript                ( cgScriptInterop::Utils::ObjectSerializer * serializedObject );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgAppStateManager     * mStateManager;          // Manager for this game state object
    cgString                mStateId;               // The unique identifier of this state as it has been registered with the manager.
    bool                    mStateSuspended;        // Is the state currently suspended?
    EventActionMap          mEventActions;          // The actions that should be taken when an event is raised
    cgAppState            * mChildState;            // Any currently active child state that may have been spawned.
    cgAppState            * mParentState;           // The state above us in the relationship list.
    cgAppTransitionState  * mOutgoingTransition;    // The transition state (if any) that is being used to link us to a new state.
    cgAppTransitionState  * mIncomingTransition;    // The transition state (if any) that is being used to link us to a previous state.
    bool                    mBegun;                 // Has the state already been begun?
    
    // Scripted state
    cgInputStream           mScriptStream;          // Stream from which a script will be loaded.
    cgResourceManager     * mResourceManager;       // Manager through which script is loaded.
    cgScriptHandle          mScript;                // Base state script.
    cgScriptObject        * mScriptObject;          // Reference to the scripted state object (owned exclusively by the script).
    MethodHandles           mScriptMethods;         // Cached handles to the script callback methods.

    // Hot-Reloading
    cgScriptInterop::Utils::ObjectSerializerBridge * mScriptObjectBridge;
    cgScriptInterop::Utils::ObjectSerializer         mScriptObjectSerializer;
};

//-----------------------------------------------------------------------------
//  Name : cgAppTransitionState (Class)
/// <summary>
/// A specific type of state designed to allow for a more comprehensive
/// transition process between two app states
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAppTransitionState : public cgAppState
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgAppTransitionState, cgAppState, "AppTransitionState" )

public:
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgAppStateManager;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgAppTransitionState( const cgString & stateUID );
    virtual ~cgAppTransitionState( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgAppState            * getFromState       ( );
    cgAppState            * getToState         ( );
    void                    transitionComplete ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgAppState)
    //-------------------------------------------------------------------------
    virtual void            end                 ( );
    virtual bool            isTransitionState   ( ) const { return true; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_AppTransitionState; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgAppState * mFromState;    // The state we are currently transitioning from
    cgAppState * mToState;      // The state we are currently transitioning to
};

#endif // !_CGE_CGAPPSTATEMANAGER_H_