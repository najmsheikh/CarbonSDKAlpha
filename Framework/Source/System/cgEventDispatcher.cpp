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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgEventDispatcher Module Includes
//-----------------------------------------------------------------------------
#include <System/cgEventDispatcher.h>

//-----------------------------------------------------------------------------
//  Name : ~cgEventDispatcher () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgEventDispatcher::~cgEventDispatcher()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgEventDispatcher::dispose( bool bDisposeBase )
{
    // Clear memory.
    mEventListeners.clear();
}

//-----------------------------------------------------------------------------
//  Name : registerEventListener ()
/// <summary>
/// Call this method in order to add any object derived from 'cgEventListener' 
/// to which you would like the associated event methods called.
/// </summary>
//-----------------------------------------------------------------------------
bool cgEventDispatcher::registerEventListener( cgEventListener * pListener )
{
    // Already in the list?
    EventListenerList::iterator itListener;
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
    {
        // Matches?
        if ( *itListener == pListener )
            return false;

    } // Next Listener

    // Add to the list
    mEventListeners.push_back( pListener );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unregisterEventListener ()
/// <summary>
/// Remove the specified object from the event listener list.
/// </summary>
//-----------------------------------------------------------------------------
void cgEventDispatcher::unregisterEventListener( cgEventListener * pListener )
{
    // Find the item in the list
    EventListenerList::iterator itListener;
    for ( itListener = mEventListeners.begin(); itListener != mEventListeners.end(); ++itListener )
    {
        // Matches?
        if ( *itListener == pListener )
        {
            // Remove from the list
            mEventListeners.erase( itListener );
            return;

        } // End if match
    
    } // Next Listener
}