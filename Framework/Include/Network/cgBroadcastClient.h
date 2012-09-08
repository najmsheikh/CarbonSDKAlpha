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
// File : cgBroadcastClient.h                                                //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the data and communication             //
//        functionality for an individual client connection. This is where   //
//        all incoming data packets will be routed.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBROADCASTCLIENT_H_ )
#define _CGE_CGBROADCASTCLIENT_H_

//-----------------------------------------------------------------------------
// cgBroadcastClient Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgEventDispatcher.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBroadcast;
class cgCriticalSection;
struct cgCommandPacket;
struct cgBroadcastDataEventArgs;
struct cgBroadcastConnectionEventArgs;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgBroadcastClient (Base Class)
/// <summary>
/// An individual client that is connected to the broadcast server. Specific
/// platforms / network layer implementations will be derived from this.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBroadcastClient : public cgEventDispatcher
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBroadcastClient( cgBroadcast * parent, cgUInt32 clientId );
    virtual ~cgBroadcastClient( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgBroadcastClient  * createInstance  ( cgBroadcast * parent, cgUInt32 clientId );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgUInt32            getClientId             ( ) const;
    bool                isVerified              ( ) const;
    cgUInt32            sendData                ( cgUInt16 command, void * data, cgUInt32 length );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgUInt32    sendBufferedData        ( ) = 0;
    virtual cgString    getClientAddress        ( ) const = 0;
    virtual cgUInt16    getClientPort           ( ) const = 0;
    virtual bool        close                   ( );

    // Event Dispatchers
    virtual void        onNewDataPacket         ( cgBroadcastDataEventArgs * e );
    virtual void        onConnectionEstablished ( cgBroadcastConnectionEventArgs * e );
    virtual void        onConnectionClosed      ( cgBroadcastConnectionEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    cgUInt32            sendClientHandshake ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgUInt32    parsePacket         ( cgCommandPacket * packet );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBroadcast       * mBroadcast;         // Parent broadcast object
    cgUInt32            mClientId;          // The unique Id of the client
    volatile bool       mVerified;          // Has this client been verified as valid?
    
    cgByte            * mInputBuffer;       // The buffer for any packets received
    cgUInt32            mInputSize;         // The current size of the input buffer
    cgUInt32            mInputCapacity;     // The total current capacity of the input buffer in bytes
    
    cgByte            * mOutputBuffer;      // The buffer for any packets being sent
    cgUInt32            mOutputSize;        // The current size of the output buffer
    cgUInt32            mOutputCapacity;    // The total current capacity of the output buffer in bytes
    cgCriticalSection * mOutputSection;     // Critical section used to protect the output buffer.
};

#endif // !_CGE_CGBROADCASTCLIENT_H_