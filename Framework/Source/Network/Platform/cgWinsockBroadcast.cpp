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
// File : cgWinsockBroadcast.cpp                                             //
//                                                                           //
// Desc : Rudimentary communication library for the transmission of          //
//        application data, commands and information between a client and /  //
//        or server. This file houses the derived platform specific          //
//        implementation of the top level network management layer using     //
//        Winsock as the network API.                                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinsockBroadcast Module Includes
//-----------------------------------------------------------------------------
#include <Network/Platform/cgWinsockBroadcast.h>
#include <Network/Platform/cgWinsockBroadcastClient.h>
#include <System/cgThreading.h>

//-----------------------------------------------------------------------------
// Name : cgWinsockBroadcast () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgWinsockBroadcast::cgWinsockBroadcast(  )
{
    // Initialize variables to sensible defaults
    mListenSocket     = CG_NULL;
    mListenEvent      = CG_NULL;
    mTermEvent        = CG_NULL;
    mEventThread      = cgThread::createInstance();
}

//-----------------------------------------------------------------------------
// Name : ~cgWinsockBroadcast () (Destructor)
// Desc : Destructor for this class.
//-----------------------------------------------------------------------------
cgWinsockBroadcast::~cgWinsockBroadcast()
{
    // Dispose of internal objects
    dispose( false );

    // Destroy event thread. Should auto-terminate if it is somehow still running.
    delete mEventThread;
    mEventThread = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgWinsockBroadcast::dispose( bool disposeBase )
{
    // Call base class implementation as required
    if ( disposeBase )
        cgBroadcast::dispose( true );
}

//-----------------------------------------------------------------------------
// Name : listenForConnections () (Virtual)
/// <summary>
/// Initialize the system to listen for incoming connections.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinsockBroadcast::listenForConnections( cgUInt16 port, const cgString & connectionKey, cgUInt16 minVersion, cgUInt16 maxVersion )
{
    // Shutdown any previously open connections
    disconnect();

    // Mark this as a server session
    mIsServer = true;

    // Store connection criteria
    mConnectionKey      = connectionKey;
    mMinConnectVersion  = minVersion;
    mMaxConnectVersion  = maxVersion;

    // Initialize winsock (v2.2)
    WSADATA winsockData;
    cgInt error = WSAStartup( MAKEWORD(2, 2), &winsockData );
    if ( error )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize winsock v2.2. Error = 0x%x\n"), error );
        return false;
    } // End if failed

    // Invalid version?
    if ( winsockData.wVersion != MAKEWORD(2, 2) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T("Invalid winsock version detected. v2.2 is required for this application but API reported v%i.%i\n"), LOBYTE( winsockData.wVersion ), HIBYTE( winsockData.wVersion ) );
        disconnect();
        return false;
    
    } // End if invalid version

    // Create the listening socket (use TCP)
    mListenSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if ( mListenSocket == SOCKET_ERROR )
    { 
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create TCP socket when initializing broadcast system in server mode. Error = 0x%x\n"), error );
        disconnect();
        return false;
    
    } // End if invalid socket

    // Setup the information about our local listening socket
    // Here we specify an address family of 'Internet', on the
    // specified port for ANY incoming address
    sockaddr_in socketAddress;
    socketAddress.sin_family        = AF_INET;
    socketAddress.sin_port          = htons( port );
    socketAddress.sin_addr.s_addr   = htonl( INADDR_ANY );
    
    // Bind the socket the specified port
    if ( bind( mListenSocket, (LPSOCKADDR)&socketAddress, sizeof(socketAddress)) == SOCKET_ERROR )
    {
        // Clean up and bail
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind TCP socket to port %i when initializing broadcast system in server mode.\n"), port );
        disconnect();
        return false;
    
    } // End if error binding

    // Listen for a connection
    if ( listen( mListenSocket, 5 ) == SOCKET_ERROR )
    {
        // Clean up and bail
        cgAppLog::write( cgAppLog::Error, _T("Failed to open TCP socket for listening on port %i when initializing broadcast system in server mode.\n"), port );
        disconnect();
        return false;

    } // End if error listening

    // Create an event for retrieving data from the listening socket
    mListenEvent = WSACreateEvent();
    if ( mListenEvent == WSA_INVALID_EVENT )
    { 
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create socket event handler when initializing broadcast system in server mode. Error = 0x%x\n"), error );
        disconnect();
        return false;
    
    } // End if failed

    // Create an event that we can signal to prevent winsock from blocking
    // when the system is shutting down.
    mTermEvent = WSACreateEvent();
    if ( mTermEvent == WSA_INVALID_EVENT )
    { 
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create termination event handler when initializing broadcast system in server mode. Error = 0x%x\n"), error );
        disconnect();
        return false;
    
    } // End if failed

    // Set this socket to spawn events
    if ( WSAEventSelect( mListenSocket, mListenEvent, FD_ACCEPT ) == SOCKET_ERROR )
    {
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to select event handler when initializing broadcast system in server mode. Error = 0x%x\n"), error );
        disconnect();
        return false;
    
    } // End if event select failed

    // The socket should be asynchronous, spin off a worker thread to listen for events.
    if ( !mEventThread->start( serverEventThread, this ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to start event worker thread when initializing broadcast system in server mode.\n") );
        disconnect();
        return false;

    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : connect () (Virtual)
/// <summary>
/// Connect to an external server.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinsockBroadcast::connect( const cgString & address, cgUInt16 port, const cgString & connectionKey, cgUInt16 connectionVersion, bool logFailedConnection /* = true */ )
{
    STRING_CONVERT;

    // Shutdown any previously open connections
    disconnect();

    // Mark this as a client session
    mIsServer = false;

    // Store connection criteria
    mConnectionKey      = connectionKey;
    mConnectionAddress  = address;
    mConnectionPort     = port;
    mMinConnectVersion  = connectionVersion;
    mMaxConnectVersion  = connectionVersion;

    // Initialize winsock (v2.2)
    WSADATA winsockData;
    cgInt error = WSAStartup( MAKEWORD(2, 2), &winsockData );
    if ( error )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to initialize winsock v2.2. Error = 0x%x\n"), error );
        return false;
    } // End if failed

    // Invalid version?
    if ( winsockData.wVersion != MAKEWORD(2, 2) ) 
    {
        cgAppLog::write( cgAppLog::Error, _T("Invalid winsock version detected. v2.2 is required for this application but API reported v%i.%i\n"), LOBYTE( winsockData.wVersion ), HIBYTE( winsockData.wVersion ) );
        disconnect();
        return false;

    } // End if invalid version

    // Build local address descriptor (specify 0 for random local port)
    sockaddr_in localAddress;
    localAddress.sin_family         = AF_INET;
    localAddress.sin_port           = 0;
    localAddress.sin_addr.s_addr    = inet_addr("127.0.0.1");

    // Build remote address descriptor
    sockaddr_in remoteAddress;
    remoteAddress.sin_family        = AF_INET;
    remoteAddress.sin_port          = htons( port );
    remoteAddress.sin_addr.s_addr   = inet_addr( stringConvertT2CA(address.c_str()) );
    if ( remoteAddress.sin_addr.S_un.S_addr == INADDR_NONE )
    {
        cgAppLog::write( cgAppLog::Error, _T("Invalid IP address '%s' specified when attempting to connect to remote system.\n"), address.c_str() );
        disconnect();
        return false;
    
    } // End if invalid address

    // Create a new client object designed to represent 'self' in the local environment.
    mSelf = cgBroadcastClient::createInstance( this, 0xFFFFFFFF );
    if ( !mSelf )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create valid broadcast client when connecting to remote system at address '%s'.\n"), address.c_str() );
        disconnect();
        return false;
    
    } // End if failed

    // Register the broadcast event router as a listener to receive
    // events from the local client. This will be used to pass new data packet 
    // messages up to any subscribers (see cgBroadcast::onNewDataPacket())
    mSelf->registerEventListener( mClientEvents );

    // Create the socket (use TCP)
    SOCKET socket = ::socket( PF_INET, SOCK_STREAM, 0 );
    if ( socket == SOCKET_ERROR )
    { 
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create TCP socket when when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;
    
    } // End if SOCKET_ERROR

    // Set this socket to the new client
    ((cgWinsockBroadcastClient*)mSelf)->setSocket( socket );

    // Bind the socket to the specified port
    if ( bind( socket, (LPSOCKADDR)&localAddress, sizeof(localAddress)) == SOCKET_ERROR )
    {
        // Clean up and bail
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind TCP socket to port when when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;

    } // End if error binding

    // Attempt to connect
    if ( ::connect( socket, (sockaddr*)&remoteAddress, sizeof(remoteAddress) ) == SOCKET_ERROR )
    {
        // Clean up and bail
        error = WSAGetLastError();
        if ( logFailedConnection == true )
            cgAppLog::write( cgAppLog::Error, _T("Failed to connect to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;

    } // End if error listening

    // Create an event for retrieving data from the listening socket
    WSAEVENT dataEvent = WSACreateEvent();
    if ( dataEvent == WSA_INVALID_EVENT )
    {
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create socket event handler when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;
    
    } // End if failed

    // Set this to the client
    ((cgWinsockBroadcastClient*)mSelf)->setEventHandle( dataEvent );

    // Set this socket to spawn events
    if ( WSAEventSelect( socket, dataEvent, (FD_READ | FD_WRITE | FD_CLOSE) ) == SOCKET_ERROR )
    {
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to select event handler when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;

    } // End if event select failed

    // Create an event that we can signal to prevent winsock from blocking
    // when the system is shutting down.
    mTermEvent = WSACreateEvent();
    if ( mTermEvent == WSA_INVALID_EVENT )
    { 
        error = WSAGetLastError();
        cgAppLog::write( cgAppLog::Error, _T("Failed to create termination event handler when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str(), error );
        disconnect();
        return false;
    
    } // End if failed

    // The socket should be asynchronous, spin off a worker thread to listen for events.
    if ( !mEventThread->start( clientEventThread, this ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to start event worker thread when connecting to remote system at address '%s'. Error = 0x%x\n"), address.c_str() );
        disconnect();
        return false;
    
    } // End if failed
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : disconnect () (Virtual)
/// <summary>
/// Shut down the broadcast system and clean up.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinsockBroadcast::disconnect( )
{
    // Shut down the event thread.
    mEventThread->signalTerminate();

    // Pulse the terminate event just in case WSAWaitForMultipleEvents() 
    // is blocking within the event thread(s).
    if ( mTermEvent )
        PulseEvent( mTermEvent );

    // Wait for completion and then clean up.
    mEventThread->terminate();
    
    // Close the listen event /  socket
    if ( mListenEvent )
        WSACloseEvent( mListenEvent );
    if ( mListenSocket )
    {
        shutdown( mListenSocket, SD_BOTH );
        closesocket( mListenSocket );

    } // End if listen socket exists

    // Close other utility events.
    if ( mTermEvent )
        WSACloseEvent( mTermEvent );

    // Shut down winsock
    WSACleanup();

    // Clear variables
    mListenSocket = CG_NULL;
    mListenEvent  = CG_NULL;
    mTermEvent    = CG_NULL;

    // Call base class implementation last
    return cgBroadcast::disconnect( );
}

//-----------------------------------------------------------------------------
// Name : serverEventThread () (Static, Worker Thread Function)
// Desc : This is the primary function for the socket event listening thread.
//-----------------------------------------------------------------------------
cgUInt32 cgWinsockBroadcast::serverEventThread( cgThread * parentThread, void * context )
{
    cgWinsockBroadcast * thisPointer = (cgWinsockBroadcast*)context;

    // Loop until we're signalled to close
    while ( !parentThread->terminateRequested() )
    {
        // Allocate room for enough events (user connections + listen event + term event)
        WSAEVENT * events = new WSAEVENT[ thisPointer->mConnectionCount + 2 ];

        // Populate events
        cgUInt32 eventCount = 0;
        thisPointer->mClientSection->enter();
        for ( ClientMap::iterator it = thisPointer->mClients.begin(); it != thisPointer->mClients.end(); ++it )
        {
            cgWinsockBroadcastClient * client = (cgWinsockBroadcastClient*)it->second;
            if (!client)
                continue;

            // Add to event handle list
            WSAEVENT event = client->getEventHandle();
            if ( event )
                events[ eventCount++ ] = event;
        
        } // Next client
        thisPointer->mClientSection->exit();

        // Add the listen socket event handler and the termination handler.
        events[ eventCount++ ] = thisPointer->mListenEvent;
        events[ eventCount++ ] = thisPointer->mTermEvent;

        // Wait for an event to happen (including terminate which will pulse the listen event)
        WSAWaitForMultipleEvents( eventCount, events, FALSE, INFINITE, FALSE );

        // ToDo: Iterating ALL clients is not necessarily a good idea.

        // Enumerate the events
        WSANETWORKEVENTS eventInfo;
        thisPointer->mClientSection->enter();
        for ( ClientMap::iterator it = thisPointer->mClients.begin(); it != thisPointer->mClients.end(); )
        {
            cgWinsockBroadcastClient * client = (cgWinsockBroadcastClient*)it->second;
            if (!client)
            {
                ++it; 
                continue;
            
            } // End if skip

            // Enumerate any events that occured for this socket
            WSAEnumNetworkEvents( client->getSocket(), client->getEventHandle(), &eventInfo );

            // What event happened?
            if ( (eventInfo.lNetworkEvents & FD_READ) && (eventInfo.iErrorCode[FD_READ_BIT] == 0) )
            {
                // Some data arrived from this client
                cgUInt32 result = client->readData( );
                if ( result == cgBroadcastResult::Disconnected )
                {
                    // Disconnect the client
                    ++it;
                    thisPointer->disconnectClient( client->getClientId() );
                    continue;
                
                } // End if disconnected

            } // End if read operation
            
            if ( (eventInfo.lNetworkEvents & FD_WRITE) && (eventInfo.iErrorCode[FD_WRITE_BIT] == 0) )
            {
                // It is once again safe to send any remaining data
                cgUInt32 result = client->sendBufferedData( );
                if ( result == cgBroadcastResult::Disconnected )
                {
                    // Disconnect the client
                    ++it;
                    thisPointer->disconnectClient( client->getClientId() );
                    continue;

                } // End if disconnected

            } // End if read operation
            
            if ( (eventInfo.lNetworkEvents & FD_CLOSE) && (eventInfo.iErrorCode[FD_CLOSE_BIT] == 0) )
            {
                // Disconnect the client
                ++it;
                thisPointer->disconnectClient( client->getClientId() );
                continue;

            } // End if close operation

            // Move on to next client
            ++it;

        } // Next client
        thisPointer->mClientSection->exit();

        // Enumerate any events that occured for the listen socket
        WSAEnumNetworkEvents( thisPointer->mListenSocket, thisPointer->mListenEvent, &eventInfo );
        if ( (eventInfo.lNetworkEvents & FD_ACCEPT) && (eventInfo.iErrorCode[FD_ACCEPT_BIT] == 0) )
        {
            // Create a new client object
            cgBroadcastClient * newClient = cgBroadcastClient::createInstance( thisPointer, thisPointer->mNextClientId );
            
            // Was it allocated?
            if ( newClient )
            {
                // Attempt to accept the connection
                if ( ((cgWinsockBroadcastClient*)newClient)->accept( thisPointer->mListenSocket ) )
                {
                    // Add this to the client map
                    thisPointer->mClientSection->enter();
                    thisPointer->mClients[ newClient->getClientId() ] = newClient;
                    
                    // Increment unique identifier field, and connection count.
                    thisPointer->mNextClientId++;
                    thisPointer->mConnectionCount++;
                    thisPointer->mClientSection->exit();

                    // Register the broadcast event router as a listener to recieve
                    // events from the new client. This will be used to pass new data packet 
                    // messages up to any subscribers (see cgBroadcast::OnNewDataPacket())
                    newClient->registerEventListener( thisPointer->mClientEvents );

                } // End if connection was accepted

            } // End if Valid allocation
        
        } // End if this is a connection request event
        
        // Destroy the event handles array
        delete []events;

    } // Next iteration

    // Thread exit with no error.
    return 0;
}

//-----------------------------------------------------------------------------
// Name : clientEventThread () (Static, Worker Thread Function)
// Desc : This is the primary function for the socket event listening thread.
//-----------------------------------------------------------------------------
cgUInt32 cgWinsockBroadcast::clientEventThread( cgThread * parentThread, void * context )
{
    cgWinsockBroadcast * thisPointer = (cgWinsockBroadcast*)context;
    
    // Loop until we're signalled to close
    while ( !parentThread->terminateRequested() )
    {
        cgWinsockBroadcastClient * client = (cgWinsockBroadcastClient*)thisPointer->mSelf;
        if ( !client ) continue;

        // Wait for an event to happen
        WSAEVENT events[] = { client->getEventHandle(), thisPointer->mTermEvent }; 
        WSAWaitForMultipleEvents( 2, events, FALSE, INFINITE, FALSE );

        // Enumerate any events that occured for this socket
        WSANETWORKEVENTS eventInfo;
        WSAEnumNetworkEvents( client->getSocket(), client->getEventHandle(), &eventInfo );

        // What event happened?
        if ( (eventInfo.lNetworkEvents & FD_READ) && (eventInfo.iErrorCode[FD_READ_BIT] == 0) )
        {
            // Some data arrived from this client
            cgUInt32 result = client->readData( );
            if ( result == cgBroadcastResult::Disconnected )
            {
                client->close();
                break;
            
            } // End if client disconnect

        } // End if read operation
        
        if ( (eventInfo.lNetworkEvents & FD_WRITE) && (eventInfo.iErrorCode[FD_WRITE_BIT] == 0) )
        {
            // We are clear to send any buffered data again.
            cgUInt32 result = client->sendBufferedData();
            if ( result == cgBroadcastResult::Disconnected )
            {
                client->close();
                break;

            } // End if client disconnect

        } // End if close operation
        
        if ( (eventInfo.lNetworkEvents & FD_CLOSE) && (eventInfo.iErrorCode[FD_CLOSE_BIT] == 0) )
        {
            // Close our connection
            client->close();

            // Exit listen thread
            break;

        } // End if close operation

    } // Next iteration

    // Thread exit with no error.
    return 0;
}