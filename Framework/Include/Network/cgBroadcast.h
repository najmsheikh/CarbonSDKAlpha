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
// File : cgBroadcast.h                                                      //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the top level network management layer //
//        classes.                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBROADCAST_H_ )
#define _CGE_CGBROADCAST_H_

//-----------------------------------------------------------------------------
// cgBroadcast Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgEventDispatcher.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgBroadcast;
class cgBroadcastClient;
class cgCriticalSection;

//-----------------------------------------------------------------------------
// Macros and Defines
//-----------------------------------------------------------------------------
/// <summary>16 bit packet signature (CG)</summary>
#define CGEBROADCAST_PACKET_SIGNATURE (((cgUInt16)'C') | (((cgUInt16)'G') << 8))

/// <summary>Broadcast system command codes</summary>
namespace cgBroadcastCommand
{
    enum System
    {
        /// <summary>Handshake sent from the server to the client.</summary>
        ServerHandshake = 0x0001,
        /// <summary>Handshake sent from the client to the server.</summary>
        ClientHandshake = 0x0002,
        /// <summary>Sent from server to client to acknowledge a successful connection.</summary>
        HandshakeAccept = 0x0003,

        /// <summary>Origin value for any user defined commands (i.e. UserCommands::MyCommand = (System::User + 10))
        User            = 0x0100
    };

}; // End Namespace : cgBroadcastCommand

/// <summary>Broadcast system internal result codes</summary>
namespace cgBroadcastResult
{
    enum Success
    {
        OK                  = 0x00000000
    };

    enum Error
    {
        Disconnected        = 0x00010000,
        Timeout             = 0x00010001,
        UnknownPacket       = 0x00010002,
        ClientNotVerified   = 0x00010003
    };

}; // End Namespace : cgBroadcastResult

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
#pragma pack(push, networkData, 1)
struct cgCommandPacket
{
private:
    // Private Packet Header
    /// <summary>The signature of the packet (should be equal to CGEBROADCAST_PACKET_SIGNATURE)</summary>
    cgUInt16    _signature;    
    /// <summary>The length of the packet overall (including this 6 byte header).</summary>
    cgUInt32    _packetLength;

public:
    // Public Variables
    /// <summary>The command being sent in the packet (its identifier).</summary>
    cgUInt16    command;

    // Public Inline Methods
    inline cgUInt32 getDataLength   ( ) const { return _packetLength - sizeof(cgCommandPacket); }
    inline cgByte * getData         ( ) const { return (cgByte*)(this + 1); }
    inline cgUInt32 getPacketLength ( ) const { return _packetLength; }
    inline void     setPacketData   ( void * data, cgUInt32 length )
    {
        // Setup packet headers
        _signature = CGEBROADCAST_PACKET_SIGNATURE;
        _packetLength = sizeof(cgCommandPacket) + length;
        
        // Copy any data where specified
        if ( length ) 
            memcpy( getData(), data, length );

    } // End Method
};
#pragma pack(pop, networkData)

//-----------------------------------------------------------------------------
// Event Argument Definitions
//-----------------------------------------------------------------------------
struct CGE_API cgBroadcastDataEventArgs
{
    cgBroadcastDataEventArgs( cgUInt32 _sourceClientId, cgCommandPacket * _data ) :
        sourceClientId ( _sourceClientId ),
        data           ( _data ) {}
    cgUInt32          sourceClientId;
    cgCommandPacket * data;

}; // End Struct cgBroadcastDataEventArgs

struct CGE_API cgBroadcastConnectionEventArgs
{
    cgBroadcastConnectionEventArgs( cgUInt32 _clientId ) :
        clientId ( _clientId )
        {}
    cgUInt32 clientId;

}; // End Struct cgBroadcastConnectionEventArgs

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBroadcastListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever broadcast events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBroadcastListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgBroadcastListener, cgEventListener, "BroadcastListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onNewDataPacket         ( cgBroadcastDataEventArgs * e ) {};
    virtual void    onConnectionEstablished ( cgBroadcastConnectionEventArgs * e ) {}
    virtual void    onConnectionClosed      ( cgBroadcastConnectionEventArgs * e ) {}
};

//-----------------------------------------------------------------------------
//  Name : cgBroadcastEventRouter (Class)
/// <summary>
/// Routes events from a broadcast client to the broadcast object so that they
/// can be distributed.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBroadcastEventRouter : public cgBroadcastListener
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgBroadcastEventRouter( cgBroadcast * broadcast );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onNewDataPacket         ( cgBroadcastDataEventArgs * e );
    virtual void    onConnectionEstablished ( cgBroadcastConnectionEventArgs * e );
    virtual void    onConnectionClosed      ( cgBroadcastConnectionEventArgs * e );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBroadcast   * mParent;
};

//-----------------------------------------------------------------------------
// Name : cgBroadcast (Base Class)
/// <summary>
/// Provides an interface for the application's network functionality. Specific
/// platforms / network layer implementations will be derived from this.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBroadcast : public cgEventDispatcher
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE( cgUInt32, cgBroadcastClient*, ClientMap )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBroadcast( );
    virtual ~cgBroadcast( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgBroadcast    * createInstance          ( );
    static cgCommandPacket* allocatePacket          ( cgUInt32 dataLength = 0 );
    static void             releasePacket           ( cgCommandPacket * packet );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    isServer                ( ) const;
    cgUInt32                getMaximumPacketLength  ( ) const;
    void                    setMaximumPacketLength  ( cgUInt32 length );
    ClientMap             & lockClientList          ( );
    const ClientMap       & lockClientList          ( ) const;
    void                    unlockClientList        ( );
    void                    getConnectionKey        ( cgString & connectionKeyOut ) const;
    void                    getConnectionVersions   ( cgUInt16 & minVersionOut, cgUInt16 & maxVersionOut ) const;
    cgString                getHostAddress          ( ) const;
    cgUInt16                getHostPort             ( ) const;
    cgUInt32                sendToAll               ( cgUInt16 command, void * data, cgUInt32 length );
    cgUInt32                sendToServer            ( cgUInt16 command, void * data, cgUInt32 length );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            listenForConnections    ( cgUInt16 port, const cgString & connectionKey, cgUInt16 minVersion, cgUInt16 maxVersion ) = 0;
    virtual bool            connect                 ( const cgString & address, cgUInt16 port, const cgString & connectionKey, cgUInt16 connectionVersion, bool logFailedConnection = true ) = 0;
    virtual bool            disconnect              ( );
    virtual bool            disconnectClient        ( cgUInt32 clientId );

    // Event Dispatchers
    virtual void            onNewDataPacket         ( cgBroadcastDataEventArgs * e );
    virtual void            onConnectionEstablished ( cgBroadcastConnectionEventArgs * e );
    virtual void            onConnectionClosed      ( cgBroadcastConnectionEventArgs * e );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    bool                mIsServer;              // Is this a server or client connection
    cgString            mConnectionKey;         // Client must provide this key during handshake in order to consider the connection valid.
    cgUInt16            mMinConnectVersion;     // Client must provide a version number larger or equal to this in order to be verified.
    cgUInt16            mMaxConnectVersion;     // Client must provide a version number smaller or equal to this in order to be verified.
    cgString            mConnectionAddress;     // (Client Only) IP address of the server to which we have connected.
    cgUInt16            mConnectionPort;        // (Client Only) Port of the server to which we have connected.
    cgUInt32            mMaxPacketLength;       // Maximum size (in bytes) that a packet can reach before it is discarded.
    cgUInt32            mConnectionCount;       // Number of clients currently connected to the system.
    cgUInt32            mNextClientId;          // Next Id to assign to any clients that establish a connection.
    ClientMap           mClients;               // Map of all connected clients
    cgBroadcastClient * mSelf;                  // The client object used when we are connecting rather than listening.
    
    cgBroadcastEventRouter* mClientEvents;      // Routes event calls made from child clients to this broadcast object for further distribution.
    cgCriticalSection     * mClientSection;     // Critical section protecting the contents of the client list (use 'lockClientList()' and 'unlockClientList()').
};

#endif // !_CGE_CGBROADCAST_H_