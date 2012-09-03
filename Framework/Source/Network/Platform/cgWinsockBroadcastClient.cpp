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
// File : cgWinsockBroadcastClient.cpp                                       //
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

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBroadcast Module Includes
//-----------------------------------------------------------------------------
#include <Network/Platform/cgWinsockBroadcastClient.h>
#include <Network/cgBroadcast.h>
#include <System/cgThreading.h>

//-----------------------------------------------------------------------------
// Name : cgWinsockBroadcastClient () (Constructor)
/// <summary>Constructor for this class.</summary>
//-----------------------------------------------------------------------------
cgWinsockBroadcastClient::cgWinsockBroadcastClient( cgBroadcast * parent, cgUInt32 clientId ) : cgBroadcastClient( parent, clientId )
{
    // Initialize variables to sensible defaults
    mSocket               = CG_NULL;
    mDataEvent            = CG_NULL;
    mExpectedPacketSize   = -1;
    
    // Clear structures
    memset( &mSocketAddress, 0, sizeof(sockaddr_in) );
}

//-----------------------------------------------------------------------------
// Name : ~cgWinsockBroadcastClient () (Destructor)
/// <summary>Destructor for this class.</summary>
//-----------------------------------------------------------------------------
cgWinsockBroadcastClient::~cgWinsockBroadcastClient()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgWinsockBroadcastClient::dispose( bool disposeBase )
{
    // Call base class implementation as required
    if ( disposeBase )
        cgBroadcastClient::dispose( true );
}

//-----------------------------------------------------------------------------
// Name : accept ()
/// <summary>
/// Accept the client connection pending on the specified socket.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinsockBroadcastClient::accept( SOCKET listenSocket )
{
    // Close any already open connection
    close();

    // Accept the new connection dataEvent
    cgInt addressLength = sizeof(mSocketAddress);
    mSocket = ::accept( listenSocket, (sockaddr*)&mSocketAddress, &addressLength );
    if ( mSocket == SOCKET_ERROR )
    {
        cgInt nError = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create TCP socket on acceptance of incoming connection. Error = 0x%x\n"), nError );
        return false;
    
    } // End if SOCKET_ERROR

    // Create the dataEvent that will fire on new data arrival.
    mDataEvent = WSACreateEvent();
    if ( mDataEvent == WSA_INVALID_EVENT )
    {
        cgInt error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create socket data dataEvent handler on acceptance of incoming connection. Error = 0x%x\n"), error );
        close();
        return false;

    } // End if WSA_INVALID_EVENT

    // Set new socket as asynchronous dataEvent driven
    if ( WSAEventSelect( mSocket, mDataEvent, (FD_READ | FD_WRITE | FD_CLOSE) ) == SOCKET_ERROR )
    {
        // Close the socket and bail
        cgInt error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to select dataEvent handler on acceptance of incoming connection. Error = 0x%x\n"), error );
        close();
        return false;
        
    } // End if failed

    // Handshake with the client and request that it sends us its credentials.
    return (sendData( cgBroadcastCommand::ServerHandshake, CG_NULL, 0 ) == cgBroadcastResult::OK);
}

//-----------------------------------------------------------------------------
// Name : sendBufferedData () (Virtual)
/// <summary>
/// Send any data that was buffered and is pending. Note: This should be called 
/// in response to an FD_WRITE socket dataEvent to ensure that cases in which only 
/// fragments of packets were issued can be completed succesfully.
/// </summary>
//-----------------------------------------------------------------------------
ULONG cgWinsockBroadcastClient::sendBufferedData( )
{
    // Wait for output buffer to become unlocked
    mOutputSection->enter();
    
    // Send the output buffer
    cgInt  sendLength = (cgInt)mOutputSize, bytesSent = 0;
    char * sendBuffer = (char*)mOutputBuffer;
    for ( cgInt fragmentCount = 0; sendLength > 0; )
    {
        // Attempt to send the buffer
        bytesSent = send( mSocket, sendBuffer, sendLength, 0 );
        ++fragmentCount;

        // What was the result of the send operation
        if ( bytesSent == 0 )
        {
            // The socket was closed, flush the entire buffer
            mOutputSize = 0;
            return cgBroadcastResult::Disconnected;

        } // End if socket closed
        else if ( bytesSent == SOCKET_ERROR )
        {
            // An error occured, was it something we can handle
            cgInt error = WSAGetLastError();
            if ( error == WSAEWOULDBLOCK )
            {
                // The stack's send buffer was most likely full.
                // We should receive an FD_WRITE dataEvent when it is safe
                // to send again, so we can simply commit our changes
                // to the output buffer and wait for that to happen.
                break;

            } // End if 'Try Again Later'
            else
            {
                // We'll just abort the send in a similar fashion to
                // blocking. We can only hope that the connection
                // will be re-established. If not, the socket will
                // be closed later by the broadcast system.
                break;

            } // End if some other error occurred

        } // End if socket error

        // Move our buffer forwards in case it didn't send the whole packet
        sendBuffer += bytesSent;
        sendLength -= bytesSent;

    } // Keep going until we've sent the entire buffer

    // Do we need to commit our changes to the output buffer?
    if ( sendLength > 0 && sendLength != mOutputSize )
    {
        // Shuffle the remaining data back to the start of the buffer
        memmove( mOutputBuffer, sendBuffer, sendLength );

    } // End if data remains and we sent at least something

    // Store the new length of the remaining output buffer
    mOutputSize = sendLength;

    // We're done with the output buffer
    mOutputSection->exit();
        
    // Success!
    return cgBroadcastResult::OK;
}

//-----------------------------------------------------------------------------
// Name : close ()
/// <summary>
/// Close the socket and clean up the client
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinsockBroadcastClient::close( )
{
    // Close the socket
    if ( mSocket )
    {
        // Send any remaining data
        sendBufferedData();

        // Shutdown the socket for both sending and receiving
        shutdown( mSocket, SD_SEND );

        // Clear the receive buffer
        readData();

        // Close the socket
        closesocket( mSocket );  
    
    } // End if socket open

    // Remove the data dataEvent
    if ( mDataEvent )
        WSACloseEvent( mDataEvent );

    // Clear variables
    mSocket               = CG_NULL;
    mDataEvent            = CG_NULL;
    mExpectedPacketSize   = -1;

    // Call base class implementation last
    return cgBroadcastClient::close();
}

//-----------------------------------------------------------------------------
// Name : readData ()
/// <summary>
/// Read any data pending in the client's socket.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWinsockBroadcastClient::readData( )
{
    static cgInt growAmount = 8192, readAmount = 2048;
    
    // Determine maximum allow packet length before it is discarded
    cgUInt32 maxPacketLength = mBroadcast->getMaximumPacketLength();
    if ( maxPacketLength == 0 )
        maxPacketLength = INT_MAX;
    
    // Keep reading until we're done
    for ( ;; )
    {
        // Resize packet buffer in large increments
        if ( mInputSize + readAmount > mInputCapacity )
        {
            // Resize capacity until we have enough space
            for ( ; mInputSize + readAmount > mInputCapacity; )
                mInputCapacity += growAmount;

            // Allocate a new buffer for the packet
            cgByte * buffer = new cgByte[ mInputCapacity ];
            
            // Copy over any previously allocated data
            if ( mInputBuffer && mInputSize )
                memcpy( buffer, mInputBuffer, mInputSize );

            // Release previous buffer and overwrite with newly sized one
            delete []mInputBuffer;
            mInputBuffer = buffer;

        } // End if packet buffer too small

        // Attempt to receive data
        cgInt bytesReceived = recv( mSocket, (char*)&mInputBuffer[mInputSize], readAmount, 0 );
        if ( ((bytesReceived == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK)) || bytesReceived == 0 )
        {
            // If there was an error (and it wasn't that we would block because there is no data available) or
            // the client closed the connection gracefully then we should cancel the packet buildup and just return
            mInputSize = 0;
            return cgBroadcastResult::Disconnected;
        
        } // End if an error occured

        // Increment packet size by that which we have read (unless it indicated WSAWOULDBLOCK error)
        if ( bytesReceived >= 0 )
            mInputSize += bytesReceived;

        // If this is larger than our allowed packet size, just clear the packet data we have so far
        // and loop round to the next piece of available data. Note: This prevents a malicious application
        // from sending huge amounts of data and potentially crashing the entire system due to low resources.
        if ( mInputSize > maxPacketLength )
        {
            // Simply clear packet entries
            mInputSize          = 0;
            mExpectedPacketSize = -1;
            continue;
        
        } // End if exceeded max allowed packet length

        // If we still need the packet length and we have read enough for the packet signature and size so far, test it
        if ( mExpectedPacketSize < 0 && mInputSize >= sizeof(cgCommandPacket) )
        {
            // TODO: Optimize this to search only that data which we received in this iteration
            //       rather than the entire packet each time

            // Find the packet start (this should always be at the beginning, but pays to be safe).
            cgInt packetBegin = -1;
            for ( cgInt i = 0; i < (cgInt)mInputSize - 1; ++i )
            {
                // Does this match the packet signature
                if ( *(cgUInt16*)(mInputBuffer + i) == CGEBROADCAST_PACKET_SIGNATURE )
                {
                    packetBegin = i;
                    break;
                
                } // End if matches packet signature
            
            } // Next byte

            // If we couldn't find the beginning of the packet, try the next receive call
            if ( packetBegin == -1 )
            {
                mInputSize = 0;
                continue;
            
            } // End if not found

            // If this is not at the beginning of the array, this is clearly garbage data
            if ( packetBegin > 0 )
            {
                // Remove the garbage from the buffer
                memmove( mInputBuffer, &mInputBuffer[packetBegin], mInputSize - packetBegin );
                mInputSize -= packetBegin;

                // If we are not verified yet, we MUST get a valid packet as the first thing we ever receive
                // otherwise we might be sitting here getting data from a completely invalid application forever
                if ( !mVerified )
                {
                    mBroadcast->disconnectClient( mClientId );
                    return cgBroadcastResult::Disconnected;
                
                } // End if !verified
            
            } // End if garbage data exists
            
            // Otherwise attempt to get the length if we have all of it (i.e. garbage has been stripped)
            if ( mInputSize < sizeof(cgCommandPacket) )
                continue;
            mExpectedPacketSize = ((cgCommandPacket*)mInputBuffer)->getPacketLength();
        
        } // End if require packet info

        // If we received less than we requested, we're done reading the available data.
        if ( bytesReceived < readAmount )
        {
            // Have we read the entire packet yet
            if ( mExpectedPacketSize >= 0 && (signed)mInputSize >= mExpectedPacketSize )
            {
                cgCommandPacket * packet = (cgCommandPacket*)mInputBuffer;

                // Parse the packet
                cgUInt32 result = parsePacket( packet );

                // Remove our packet data from the buffer if there was any additional data 
                // remaining past the end of what we just read (i.e. two or more packets combined) (rare)
                if ( (signed)mInputSize > mExpectedPacketSize )
                {
                    // Remove the packet from the buffer leaving remaining data in tact
                    memmove( mInputBuffer, &mInputBuffer[mExpectedPacketSize], mInputSize - mExpectedPacketSize );
                    mInputSize         -= mExpectedPacketSize;
                    mExpectedPacketSize = -1;

                    // Go round again and parse other data in the packet buffer
                    continue;

                } // End if more data than we needed for this packet
                else
                {
                    // Simply clear packet entries
                    mInputSize          = 0;
                    mExpectedPacketSize = -1;

                } // End if exactly the right amount of data

                // Did the packet parser request that the connection be closed?
                if ( result == cgBroadcastResult::Disconnected )
                    return result;
                
            } // End if entire packet read

            // There is nothing more for us to do
            break;

        } // End if read all data in buffer.
            
    } // Next read iteration
    
    // Success
    return cgBroadcastResult::OK;
}

//-----------------------------------------------------------------------------
// Name : getEventHandle ()
/// <summary>
/// Retrieve the winsock dataEvent handle that triggers whenever new data is
/// available for parsing.
/// </summary>
//-----------------------------------------------------------------------------
WSAEVENT cgWinsockBroadcastClient::getEventHandle( ) const
{
    return mDataEvent;
}

//-----------------------------------------------------------------------------
// Name : getSocket ()
/// <summary>
/// Retrieve the winsock socket object managing the connection for this client.
/// </summary>
//-----------------------------------------------------------------------------
SOCKET cgWinsockBroadcastClient::getSocket( ) const
{
    return mSocket;
}

//-----------------------------------------------------------------------------
// Name : setEventHandle ()
/// <summary>
/// Set the winsock dataEvent handle that triggers whenever new data is
/// available for parsing.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinsockBroadcastClient::setEventHandle( WSAEVENT dataEvent )
{
    mDataEvent = dataEvent;
}

//-----------------------------------------------------------------------------
// Name : setSocket ()
/// <summary>
/// Set the winsock socket object managing the connection for this client.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinsockBroadcastClient::setSocket( SOCKET socket )
{
    mSocket = socket;
}

//-----------------------------------------------------------------------------
// Name : getClientAddress ()
/// <summary>
/// Retrieve the address of the connecting client.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgWinsockBroadcastClient::getClientAddress( ) const
{
    STRING_CONVERT;
    char * address = inet_ntoa(mSocketAddress.sin_addr);
    if ( !address )
        return cgString::Empty;
    else
        return stringConvertA2CT( address );
}

//-----------------------------------------------------------------------------
// Name : getClientPort()
/// <summary>
/// Retrieve the port of the connecting client.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt16 cgWinsockBroadcastClient::getClientPort( ) const
{
    return mSocketAddress.sin_port;
}