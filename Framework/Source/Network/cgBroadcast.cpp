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
// File : cgBroadcast.cpp                                                    //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the top level network management layer //
//        classes.                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBroadcast Module Includes
//-----------------------------------------------------------------------------
#include <Network/cgBroadcast.h>
#include <Network/cgBroadcastClient.h>
#include <System/cgThreading.h>

// Platform APIs
#include <Network/Platform/cgWinsockBroadcast.h>

// ToDo: There is a small memory leak somewhere in this system (seems to be per-client and packet data related).

///////////////////////////////////////////////////////////////////////////////
// cgBroadcast Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgBroadcast () (Constructor)
/// <summary>Constructor for this class.</summary>
//-----------------------------------------------------------------------------
cgBroadcast::cgBroadcast(  )
{
    // Initialize variables to sensible defaults
    mSelf                 = CG_NULL;
    mNextClientId         = 0;
    mConnectionCount      = 0;
    mIsServer             = false;
    mClientSection        = cgCriticalSection::createInstance();
    mClientEvents         = new cgBroadcastEventRouter( this );
    mMinConnectVersion    = 0;
    mMaxConnectVersion    = 0;
    mConnectionAddress    = _T("0.0.0.0");
    mConnectionPort       = 0;

    // Default maximum packet size to 3mb
    mMaxPacketLength = 3145728;
}

//-----------------------------------------------------------------------------
// Name : ~cgBroadcast () (Destructor)
/// <summary>Destructor for this class.</summary>
//-----------------------------------------------------------------------------
cgBroadcast::~cgBroadcast()
{
    dispose( false );

    // Release lifetime members
    delete mClientSection;
    mClientSection = CG_NULL;
    delete mClientEvents;
    mClientEvents = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgBroadcast::dispose( bool disposeBase )
{
    // Shutdown the broadcast system
    disconnect();

    // Call base class implementation as required
    if ( disposeBase )
        cgEventDispatcher::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBroadcast * cgBroadcast::createInstance()
{
    // Determine which driver we should create.
    const CGEConfig & config = cgGetEngineConfig();
    if ( config.platform == cgPlatform::Windows )
    {
        if ( config.networkAPI == cgNetworkAPI::Winsock )
            return new cgWinsockBroadcast();

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : allocatePacket () (Static)
/// <summary>
/// Helper function to allocate an esCommandPacket which potentially has 
/// additional packet data attached to the end.
/// </summary>
//-----------------------------------------------------------------------------
cgCommandPacket * cgBroadcast::allocatePacket( cgUInt32 dataLength /* = 0 */ )
{
    return (cgCommandPacket*) new cgByte[ sizeof(cgCommandPacket) + dataLength ];
}

//-----------------------------------------------------------------------------
// Name : releasePacket () (Static)
/// <summary>
/// Release a packet that has been allocated via 'allocatePacket'.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::releasePacket( cgCommandPacket * packet )
{
    delete [](cgByte*)packet;
}

//-----------------------------------------------------------------------------
// Name : lockClientList ()
/// <summary>
/// Lock the client list in order to prevent changes from being made to it 
/// while the caller is reading (this includes removing clients on disconnect).
/// </summary>
//-----------------------------------------------------------------------------
cgBroadcast::ClientMap & cgBroadcast::lockClientList( )
{
    // Lock the client list critical section.
    mClientSection->enter();

    // Return client list
    return mClients;
}

//-----------------------------------------------------------------------------
// Name : lockClientList () (const overload)
/// <summary>
/// Lock the client list in order to prevent changes from being made to it 
/// while the caller is reading (this includes removing clients on disconnect).
/// </summary>
//-----------------------------------------------------------------------------
const cgBroadcast::ClientMap & cgBroadcast::lockClientList( ) const
{
    // Lock the client list critical section.
    mClientSection->enter();

    // Return client list
    return mClients;
}

//-----------------------------------------------------------------------------
// Name : unlockClientList ()
/// <summary>
/// Unlock the client list to allow changes to be made.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::unlockClientList( )
{
    // Release lock.
    mClientSection->exit();
}

//-----------------------------------------------------------------------------
// Name : disconnectClient () (Virtual)
/// <summary>
/// Disconnect the client with the specified client identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBroadcast::disconnectClient( cgUInt32 clientId )
{
    // Lock the client list.
    mClientSection->enter();
    
    // Erase the client from the list, and then delete it (this order is important
    // to prevent close, read, close, read recursions.
    ClientMap::iterator it = mClients.find( clientId );
    if ( it != mClients.end() )
    {
        // Erase client.
        cgBroadcastClient * client = it->second;
        mClients.erase( it );
        if ( client )
            client->scriptSafeDispose();
    
    } // End if found

    // Unlock.
    mClientSection->exit();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : disconnect () (Virtual)
/// <summary>
/// Shut down the broadcast system and clean up.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBroadcast::disconnect( )
{
    // Lock the client list.
    mClientSection->enter();

    // Remove all active clients.
    for ( ClientMap::iterator it = mClients.begin(); it != mClients.end(); ++it )
    {
        cgBroadcastClient * client = it->second;
        if ( client )
            client->scriptSafeDispose();

    } // Next client
    if ( mSelf )
        mSelf->scriptSafeDispose();
    mSelf = CG_NULL;
    mClients.clear();

    // Unlock
    mClientSection->exit();

    // Clear variables
    mNextClientId         = 0;
    mConnectionCount      = 0;
    mIsServer             = false;
    mConnectionAddress    = _T("0.0.0.0");
    mConnectionPort       = 0;
    
    // Successful shutdown
    return true;
}

//-----------------------------------------------------------------------------
// Name : getMaximumPacketLength ()
/// <summary>
/// Retrieve the maximum size (in bytes) that a packet can reach before it is 
/// discarded.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcast::getMaximumPacketLength( ) const
{
    return mMaxPacketLength;
}

//-----------------------------------------------------------------------------
// Name : setMaximumPacketLength ()
/// <summary>
/// Set the maximum size (in bytes) that a packet can reach before it is 
/// discarded.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::setMaximumPacketLength( cgUInt32 length )
{
    mMaxPacketLength = length;
}

//-----------------------------------------------------------------------------
// Name : isServer ()
/// <summary>
/// Is this a client or a server broadcast session.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBroadcast::isServer( ) const
{
    return mIsServer;
}

//-----------------------------------------------------------------------------
// Name : getConnectionKey()
/// <summary>
/// Retrieve the key part of the credentials required for creating a valid
/// connection. If this is a server session, this will be the key that the
/// server expects. If this is a client session, this will be the key that was
/// specified by the connecting client through its call to 'Connect()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::getConnectionKey( cgString & connectionKeyOut ) const
{
    connectionKeyOut = mConnectionKey;
}

//-----------------------------------------------------------------------------
// Name : getConnectionVersions()
/// <summary>
/// Retrieve the versioning part of the credentials required for creating a 
/// valid connection. If this is a server session, this will be the minimum
/// and maximum version numbers that will be accepted when the client sends its
/// version through during handshaking. If this is a client session, both 
/// returned values will be identical and will be set to the single version
/// number provided by the connecting client through its call to 'Connect()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::getConnectionVersions( cgUInt16 & minVersionOut, cgUInt16 & maxVersionOut ) const
{
    minVersionOut = mMinConnectVersion;
    maxVersionOut = mMaxConnectVersion;
}

//-----------------------------------------------------------------------------
// Name : getHostAddress()
/// <summary>
/// Retrieve the address of the host server to which we have connected as a
/// client.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgBroadcast::getHostAddress( ) const
{
    return mConnectionAddress;
}

//-----------------------------------------------------------------------------
// Name : getHostPort()
/// <summary>
/// Retrieve the remote port of the host server to which we have connected as a
/// client.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt16 cgBroadcast::getHostPort( ) const
{
    return mConnectionPort;
}

//-----------------------------------------------------------------------------
// Name : onNewDataPacket () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::onNewDataPacket( cgBroadcastDataEventArgs * e )
{
    // We just pass this message on to any broadcast object listeners.
    // Trigger 'onNewDataPacket' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgBroadcastListener*)(*itListener))->onNewDataPacket( e );
}

//-----------------------------------------------------------------------------
// Name : onConnectionEstablished () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::onConnectionEstablished( cgBroadcastConnectionEventArgs * e )
{
    // We just pass this message on to any broadcast object listeners.
    // Trigger 'onConnectionEstablished' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgBroadcastListener*)(*itListener))->onConnectionEstablished( e );
}

//-----------------------------------------------------------------------------
// Name : onConnectionClosed () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcast::onConnectionClosed( cgBroadcastConnectionEventArgs * e )
{
    // We just pass this message on to any broadcast object listeners.
    // Trigger 'onConnectionClosed' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgBroadcastListener*)(*itListener))->onConnectionClosed( e );
}

//-----------------------------------------------------------------------------
// Name : sendToServer () (Virtual)
/// <summary>
/// Send untargeted data packet to the server (client mode only).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcast::sendToServer( cgUInt16 command, void * data, cgUInt32 length )
{
    if ( mSelf && mSelf->isVerified() )
        return mSelf->sendData( command, data, length );
    return cgBroadcastResult::Disconnected;
}

//-----------------------------------------------------------------------------
// Name : sendToAll () (Virtual)
/// <summary>
/// Send data to every connected client (server mode only).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcast::sendToAll( cgUInt16 command, void * data, cgUInt32 length )
{
    if ( isServer() )
    {
        // Lock client list to prevent modification.
        ClientMap::iterator itClient;
        ClientMap & clients = lockClientList();

        // Send data to all connected clients.
        for ( itClient = clients.begin(); itClient != clients.end(); ++itClient )
            itClient->second->sendData( command, data, length );

        // We're done with the client list. Finish up.
        unlockClientList();
        return cgBroadcastResult::OK;
    
    } // End if server
    return cgBroadcastResult::Disconnected;
}

///////////////////////////////////////////////////////////////////////////////
// cgBroadcastEventRouter Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgBroadcastEventRouter () (Constructor)
/// <summary>Constructor for this class.</summary>
//-----------------------------------------------------------------------------
cgBroadcastEventRouter::cgBroadcastEventRouter( cgBroadcast * broadcast ) : cgBroadcastListener()
{
    // Initialize variables to sensible defaults
    mParent = broadcast;
}

//-----------------------------------------------------------------------------
// Name : onNewDataPacket () (Virtual)
/// <summary>
/// Triggered in response to events raised in a specific client. This message
/// will be routed to the main owner broadcast object for further dispatch.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcastEventRouter::onNewDataPacket( cgBroadcastDataEventArgs * e )
{
    mParent->onNewDataPacket( e );
}

//-----------------------------------------------------------------------------
// Name : onConnectionEstablished () (Virtual)
/// <summary>
/// Triggered in response to events raised in a specific client. This message
/// will be routed to the main owner broadcast object for further dispatch.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcastEventRouter::onConnectionEstablished( cgBroadcastConnectionEventArgs * e )
{
    mParent->onConnectionEstablished( e );
}

//-----------------------------------------------------------------------------
// Name : onConnectionClosed () (Virtual)
/// <summary>
/// Triggered in response to events raised in a specific client. This message
/// will be routed to the main owner broadcast object for further dispatch.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcastEventRouter::onConnectionClosed( cgBroadcastConnectionEventArgs * e )
{
    mParent->onConnectionClosed( e );
}