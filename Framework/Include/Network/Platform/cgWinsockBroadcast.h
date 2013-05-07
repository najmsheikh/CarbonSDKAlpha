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
// File : cgWinsockBroadcast.h                                               //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the derived platform specific          //
//        implementation of the top level network management layer using     //
//        Winsock as the network API.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINSOCKBROADCAST_H_ )
#define _CGE_CGWINSOCKBROADCAST_H_

//-----------------------------------------------------------------------------
// cgWinsockBroadcast Header Includes
//-----------------------------------------------------------------------------
#include <Network/cgBroadcast.h>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgThread;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgWinsockBroadcast (Class)
/// <summary>
/// Provides an interface for the application's network functionality. This
/// derived version provides network functionality using the Winsock API.
/// </summary>
//-----------------------------------------------------------------------------
class cgWinsockBroadcast : public cgBroadcast
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWinsockBroadcast( );
    virtual ~cgWinsockBroadcast( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgBroadcast)
    //-------------------------------------------------------------------------
    virtual bool            listenForConnections    ( cgUInt16 port, const cgString & connectionKey, cgUInt16 minVersion, cgUInt16 maxVersion );
    virtual bool            connect                 ( const cgString & address, cgUInt16 port, const cgString & connectionKey, cgUInt16 connectionVersion, bool logFailedConnection = true );
    virtual bool            disconnect              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );
    
private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static cgUInt32         serverEventThread       ( cgThread * parentThread, void * context );
    static cgUInt32         clientEventThread       ( cgThread * parentThread, void * context );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    SOCKET              mListenSocket;      // The socket used for listening for incoming connections
    WSAEVENT            mListenEvent;       // Event handler for receiving the listen socket events
    WSAEVENT            mTermEvent;         // WSA thread termination event handler.
    cgThread          * mEventThread;       // Socket event processing thread (client or server).
};

#endif // !_CGE_CGWINSOCKBROADCAST_H_