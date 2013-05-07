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
// Name : cgEventDispatcher.h                                                //
//                                                                           //
// Desc : Common system file that defines various event related types and    //
//        common enumerations.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGEVENTDISPATCHER_H_ )
#define _CGE_CGEVENTDISPATCHER_H_

//-----------------------------------------------------------------------------
// cgEventDispatcher Header Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Common Global Classes
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgEventListener (Class)
/// <summary>
/// Abstract interface class from which other listener classes can derive in 
/// order to recieve messages whenever events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgEventListener : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgEventListener, "EventListener" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    virtual ~cgEventListener() {}

}; // End Class cgEventListener

//-----------------------------------------------------------------------------
//  Name : cgEventDispatcher (Class)
/// <summary>
/// Base class from which other classes may derive if they want to be able to
/// have listeners register with them and to dispatch event notifications to 
/// those listeners.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgEventDispatcher : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgEventDispatcher, "EventDispatcher" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    virtual ~cgEventDispatcher();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Listener registration functions
    bool            registerEventListener   ( cgEventListener * listener );
    void            unregisterEventListener ( cgEventListener * listener );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE(cgEventListener*, EventListenerList)

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    EventListenerList   mEventListeners;   // All registered listener objects to receive events.

}; // End Class cgEventDispatcher

#endif // !_CGE_CGEVENTDISPATCHER_H_