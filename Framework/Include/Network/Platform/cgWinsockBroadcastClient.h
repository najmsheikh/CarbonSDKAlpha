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
// File : cgWinsockBroadcastClient.h                                         //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the data and communication             //
//        functionality for an individual client connection using the        //
//        Winsock API. This is where all incoming data packets will be       //
//        routed.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINSOCKBROADCASTCLIENT_H_ )
#define _CGE_CGWINSOCKBROADCASTCLIENT_H_

//-----------------------------------------------------------------------------
// cgWinsockBroadcastClient Header Includes
//-----------------------------------------------------------------------------
#include <Network/cgBroadcastClient.h>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBroadcast;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgWinsockBroadcastClient (Class)
/// <summary>
/// An individual client that is connected to the broadcast server. This
/// derived version provides network functionality using the Winsock API.
/// </summary>
//-----------------------------------------------------------------------------
class cgWinsockBroadcastClient : public cgBroadcastClient
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgWinsockBroadcastClient( cgBroadcast * parent, cgUInt32 clientId );
    virtual ~cgWinsockBroadcastClient( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                accept          ( SOCKET listenSocket );
    cgUInt32            readData        ( );
    WSAEVENT            getEventHandle  ( ) const;
    SOCKET              getSocket       ( ) const;
    void                setEventHandle  ( WSAEVENT dataEvent );
    void                setSocket       ( SOCKET socket );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgBroadcastClient)
    //-------------------------------------------------------------------------
    virtual cgUInt32    sendBufferedData( );
    virtual bool        close           ( );
    virtual cgString    getClientAddress( ) const;
    virtual cgUInt16    getClientPort   ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose         ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    SOCKET          mSocket;                // Client socket
    WSAEVENT        mDataEvent;             // Event handle
    sockaddr_in     mSocketAddress;         // The winsock socket address for this client
    cgInt           mExpectedPacketSize;    // The size of the packet we are expecting to receive.
};

#endif // !_CGE_CGWINSOCKBROADCASTCLIENT_H_