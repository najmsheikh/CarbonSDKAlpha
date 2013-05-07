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
// File : cgBroadcastClient.cpp                                              //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the data and communication             //
//        functionality for an individual client connection. This is where   //
//        all incoming data packets will be routed.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBroadcastClient Module Includes
//-----------------------------------------------------------------------------
#include <Network/cgBroadcastClient.h>
#include <Network/cgBroadcast.h>
#include <System/cgThreading.h>

// Platform APIs
#include <Network/Platform/cgWinsockBroadcastClient.h>

//-----------------------------------------------------------------------------
// Name : cgBroadcastClient () (Constructor)
/// <summary>Constructor for this class.</summary>
//-----------------------------------------------------------------------------
cgBroadcastClient::cgBroadcastClient( cgBroadcast * parent, cgUInt32 clientId )
{
    // Initialize variables to sensible defaults
    mBroadcast        = parent;
    mClientId         = clientId;
    mInputBuffer      = CG_NULL;
    mInputSize        = 0;
    mInputCapacity    = 0;
    mOutputBuffer     = CG_NULL;
    mOutputSize       = 0;
    mOutputCapacity   = 0;
    mOutputSection    = cgCriticalSection::createInstance();
    mVerified         = false;
}

//-----------------------------------------------------------------------------
// Name : ~cgBroadcastClient () (Destructor)
/// <summary>Destructor for this class.</summary>
//-----------------------------------------------------------------------------
cgBroadcastClient::~cgBroadcastClient()
{
    // Clean up.
    dispose( false );

    // Release lifetime members
    delete mOutputSection;
    mOutputSection = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgBroadcastClient::dispose( bool disposeBase )
{
    // close the client socket
    close();
    
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
cgBroadcastClient * cgBroadcastClient::createInstance( cgBroadcast * parent, cgUInt32 clientId )
{
    // Determine which driver we should create.
    const CGEConfig & config = cgGetEngineConfig();
    if ( config.platform == cgPlatform::Windows )
    {
        if ( config.networkAPI == cgNetworkAPI::Winsock )
            return new cgWinsockBroadcastClient( parent, clientId );

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : sendClientHandshake () (Protected)
/// <summary>
/// Send the client handshake data back to the server.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcastClient::sendClientHandshake( )
{
    // Send handshake
    if ( !mBroadcast->isServer() )
    {
        STRING_CONVERT;

        // Retrieve the connection credentials (key, version) specified by the user.
        cgString connectionKey;
        cgUInt16 minVersion, maxVersion;
        mBroadcast->getConnectionKey( connectionKey );
        mBroadcast->getConnectionVersions( minVersion, maxVersion );

        // Client sends connection data to server which will be verified as soon
        // as the transfer is completed. If the connection details do not match
        // the values expected by the server then the connection will be dropped.
        // First allocate they packet data buffer.
        cgByte   keyLength  = (cgByte)min( 255, connectionKey.size() );
        cgUInt32 dataLength = 2 + 1 + keyLength; // (version, key length, key data)
        cgByte * buffer     = new cgByte[ dataLength ];

        // Populate buffer with required data.
        *((cgUInt16*)buffer) = minVersion;
        *(buffer + 2) = keyLength;
        memcpy( buffer + 3, stringConvertT2CA(connectionKey.c_str()), keyLength );

        // Send it
        cgUInt32 result = sendData( cgBroadcastCommand::ClientHandshake, buffer, dataLength );

        // Clean up and return the result.
        delete []buffer;
        return result;

    } // End if client
    
    // This outgoing packet is only relevant to clients.
    return cgBroadcastResult::UnknownPacket;
}

//-----------------------------------------------------------------------------
// Name : sendData ()
/// <summary>
/// Send a specific piece of data to the client.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcastClient::sendData( cgUInt16 command, void * data, cgUInt32 length )
{
    // Cannot send anything other than system commands to 
    // this client until it has been verified.
    if ( mVerified == false && command >= cgBroadcastCommand::User )
        return cgBroadcastResult::ClientNotVerified;

    // Compute the amount of data we are about to send
    cgUInt32 sendAmount = sizeof(cgCommandPacket) + length;
    cgUInt32 growAmount = 1024;

    // Wait for output buffer to become unlocked
    mOutputSection->enter();
    
    // Resize output buffer in large increments
    if ( mOutputSize + sendAmount > mOutputCapacity )
    {
        // Resize capacity until we have enough space
        for ( ; mOutputSize + sendAmount > mOutputCapacity; )
            mOutputCapacity += growAmount;

        // Allocate a new buffer for the packet
        cgByte * buffer = new cgByte[ mOutputCapacity ];
        
        // Copy over any previously allocated data
        if ( mOutputBuffer && mOutputSize ) 
            memcpy( buffer, mOutputBuffer, mOutputSize );

        // Release previous buffer and overwrite with newly sized one
        delete []mOutputBuffer;
        mOutputBuffer = buffer;

    } // End if packet buffer too small

    // Overlay packet structure over the correct output buffer area
    cgCommandPacket * packet = (cgCommandPacket*)&mOutputBuffer[mOutputSize];

    // Set packet data
    packet->setPacketData( data, length );
    packet->command = command;

    // Increase size of output buffer
    mOutputSize += packet->getPacketLength();

    // Release the output buffer lock
    mOutputSection->exit();
    
    // Send any data that exists in the send buffer (from this and potentially previous calls)
    return sendBufferedData();
}

//-----------------------------------------------------------------------------
// Name : close () (Virtual)
/// <summary>
/// Close the socket and clean up the client
/// </summary>
//-----------------------------------------------------------------------------
bool cgBroadcastClient::close( )
{
    // Were we connected in the first place? We will only send
    // notifications of a closed connection if this was the case.
    bool wasVerified = mVerified;

    // Release memory
    delete []mInputBuffer;
    delete []mOutputBuffer;

    // Clear variables
    mInputBuffer      = CG_NULL;
    mInputSize        = 0;
    mInputCapacity    = 0;
    mOutputBuffer     = CG_NULL;
    mOutputSize       = 0;
    mOutputCapacity   = 0;
    mVerified         = false;

    // Notify listeners
    if ( wasVerified == true )
        onConnectionClosed( &cgBroadcastConnectionEventArgs( mClientId ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : parsePacket () (Protected, Virtual)
/// <summary>
/// Parse the specified command packet and perform any operations
/// dictated by it.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcastClient::parsePacket( cgCommandPacket * packet )
{
    // If the client has not yet been verified, the first command to
    // come in should be the handshake / handshake response
    if ( !mVerified )
    {
        // Which packet command was sent.
        switch ( packet->command )
        {
            //////////////////////////////////////////////////////////
            // Client-side messages.
            //////////////////////////////////////////////////////////
            case cgBroadcastCommand::ServerHandshake:
                // Received handshake packet from server, respond appropriately
                // (Only applicable to client applications)
                if ( mBroadcast->isServer() )
                    return cgBroadcastResult::Disconnected;
                
                // Send response
                return sendClientHandshake();

            case cgBroadcastCommand::HandshakeAccept:
                // Server accepted our handshake and we are ready to roll.
                mVerified = true;
                onConnectionEstablished( &cgBroadcastConnectionEventArgs( 0xFFFFFFFF ) );
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Established connection with broadcast server at remote address %s:%i.\n"), mBroadcast->getHostAddress().c_str(), mBroadcast->getHostPort() );
                return cgBroadcastResult::OK;

            //////////////////////////////////////////////////////////
            // Server-side messages.
            //////////////////////////////////////////////////////////
            case cgBroadcastCommand::ClientHandshake:
            {
                STRING_CONVERT;

                // Received handshake response from client, respond appropriately
                // (Only applicable to server applications)
                if ( !mBroadcast->isServer() )
                    return cgBroadcastResult::Disconnected;

                // Verify minimum packet length
                if ( packet->getDataLength() < 3 )
                    return cgBroadcastResult::Disconnected;

                // Extract supplied packet data.
                cgByte * data    = packet->getData();
                cgUInt16 version = *((cgUInt16*)data);
                data += 2;
                cgByte   keyLength = *data++;

                // Enough data left for key?
                if ( packet->getDataLength() < 3 + (cgUInt32)keyLength )
                    return cgBroadcastResult::Disconnected;

                // Retrieve the connection credentials (key, version) required by the server.
                cgString connectionKey;
                cgUInt16 minVersion, maxVersion;
                mBroadcast->getConnectionKey( connectionKey );
                mBroadcast->getConnectionVersions( minVersion, maxVersion );

                // Version falls within required criteria?
                if ( version < minVersion || version > maxVersion )
                    return cgBroadcastResult::Disconnected;

                // Does the key length match that expected?
                if ( (size_t)keyLength != connectionKey.size() )
                    return cgBroadcastResult::Disconnected;

                // Do the keys themselves match?
                if ( memcmp( stringConvertT2CA(connectionKey.c_str()), data, keyLength ) != 0 )
                    return cgBroadcastResult::Disconnected;

                // Client was verified!
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Info, _T("Established connection with client for broadcast at remote address %s:%i.\n"), getClientAddress().c_str(), getClientPort() );
                mVerified = true;

                // Send acceptance message to client.
                if ( sendData( cgBroadcastCommand::HandshakeAccept, CG_NULL, 0 ) != cgBroadcastResult::OK )
                {
                    mVerified = false;
                    return cgBroadcastResult::Disconnected;
                
                } // End if failed

                // Connection with new client was established.
                onConnectionEstablished( &cgBroadcastConnectionEventArgs( this->getClientId() ) );
                
                return cgBroadcastResult::OK;
            
            } // End case ClientHandshake
            default:
                // Not a packet that we recognize, skip it (probably interfacing with an invalid app)
                return cgBroadcastResult::Disconnected;

        } // End command switch
    
    } // End if not yet verified
    else
    {
        // Validated, no system packets to process yet.
        // Send packet data to listeners
        onNewDataPacket( &cgBroadcastDataEventArgs( getClientId(), packet ) );
        return cgBroadcastResult::OK;

    } // End if client is verified
}

//-----------------------------------------------------------------------------
// Name : getClientId ()
/// <summary>
/// Retrieve the unique identifier that is used to identify this specific 
/// client.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBroadcastClient::getClientId() const
{
    return mClientId;
}

//-----------------------------------------------------------------------------
// Name : isVerified ()
/// <summary>
/// Has the client been verified yet (connection accepted)?
/// </summary>
//-----------------------------------------------------------------------------
bool cgBroadcastClient::isVerified( ) const
{
    return mVerified;
}

//-----------------------------------------------------------------------------
// Name : onNewDataPacket () (Virtual)
/// <summary>
/// Can be overriden or called by derived class in order to trigger the event
/// with matching name. All listeners will subsequently be notified.
/// </summary>
//-----------------------------------------------------------------------------
void cgBroadcastClient::onNewDataPacket( cgBroadcastDataEventArgs * e )
{
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
void cgBroadcastClient::onConnectionEstablished( cgBroadcastConnectionEventArgs * e )
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
void cgBroadcastClient::onConnectionClosed( cgBroadcastConnectionEventArgs * e )
{
    // We just pass this message on to any broadcast object listeners.
    // Trigger 'onConnectionClosed' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList::iterator itListener;
    EventListenerList listeners = mEventListeners;
    for ( itListener = listeners.begin(); itListener != listeners.end(); ++itListener )
        ((cgBroadcastListener*)(*itListener))->onConnectionClosed( e );
}