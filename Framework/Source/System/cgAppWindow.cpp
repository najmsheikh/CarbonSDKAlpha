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
// Name : cgAppWindow.cpp                                                    //
//                                                                           //
// Desc : Base window / form class that provides an interface between the    //
//        engine and any platform specific window handling logic.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAppWindow Module Includes
//-----------------------------------------------------------------------------
#include <System/cgAppWindow.h>
#include <System/cgMessageTypes.h>
#include <System/cgCursor.h>
#include <System/Platform/cgWinAppWindow.h>

///////////////////////////////////////////////////////////////////////////////
// cgAppWindow Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAppWindow () (Constructor)
/// <summary>
/// cgAppWindow Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow::cgAppWindow( ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Clear variables
    mCursor         = CG_NULL;
    mCursorVisCount = 1;
}

//-----------------------------------------------------------------------------
//  Name : ~cgAppWindow () (Destructor)
/// <summary>
/// cgAppWindow Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow::~cgAppWindow()
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
void cgAppWindow::dispose( bool bDisposeBase )
{
    // Release references to shared resources.
    if ( mCursor )
        mCursor->removeReference( this );

    // Reset variables
    mCursorVisCount = 1;
    mCursor         = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow * cgAppWindow::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinAppWindow();
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object, wrapping the specified native window.
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow * cgAppWindow::createInstance( void * pNativeWindow )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinAppWindow( pNativeWindow );
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : screenToClient ()
/// <summary>
/// Convert a rectangle currently expressed in screen space coordinates
/// into the space of this window's client area.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgAppWindow::screenToClient( const cgRect & rcScreen )
{
    cgPoint pt = screenToClient( cgPoint( rcScreen.left, rcScreen.top ) );
    return cgRect( rcScreen.left + pt.x, rcScreen.top + pt.y,
                   rcScreen.right + pt.x, rcScreen.bottom + pt.y );
}

//-----------------------------------------------------------------------------
//  Name : clientToScreen ()
/// <summary>
/// Convert a rectangle currently expressed in the space of this window's
/// client area into screen space.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgAppWindow::clientToScreen( const cgRect & rcClient )
{
    cgPoint pt = clientToScreen( cgPoint( rcClient.left, rcClient.top ) );
    return cgRect( rcClient.left + pt.x, rcClient.top + pt.y,
                   rcClient.right + pt.x, rcClient.bottom + pt.y );
}

//-----------------------------------------------------------------------------
//  Name : setCursor () (Virtual)
/// <summary>
/// Update the cursor image to be displayed when the user's cursor falls within
/// this application window.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppWindow::setCursor( cgCursor * pCursor )
{
    // Add reference to new cursor first (in case it's the same reference)
    if ( pCursor )
        pCursor->addReference( this );

    // Remove any prior cursor reference.
    if ( mCursor )
        mCursor->removeReference( this );

    // Store.
    mCursor = pCursor;
}

//-----------------------------------------------------------------------------
//  Name : setCursor ()
/// <summary>
/// Show or hide the platform cursor for this window. This is implemented
/// as a counter whereby multiple calls to show the cursor are counted and
/// thus will require the same number of matching calls to hide the cursor 
/// before it will be hidden.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppWindow::showCursor( bool show )
{
    if ( show )
        ++mCursorVisCount;
    else
        --mCursorVisCount;
}

//-----------------------------------------------------------------------------
//  Name : onSize () (Virtual)
/// <summary>
/// Triggered whenever the size of the window changes.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppWindow::onSize( const cgSize & Size, bool bMinimized )
{
    // Build message data.
    cgWindowSizeEventArgs Args( Size, bMinimized );
    
    // Construct and send message to any interested party.
    cgMessage Msg;
    Msg.messageData = &Args;
    Msg.messageId   = cgSystemMessages::AppWindow_OnSize;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onClose () (Virtual)
/// <summary>
/// Triggered when the window is closed.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppWindow::onClose( )
{
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::AppWindow_OnClose;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onCreate () (Virtual)
/// <summary>
/// Triggered when the window is first created.
/// </summary>
//-----------------------------------------------------------------------------
void cgAppWindow::onCreate( )
{
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::AppWindow_OnCreate;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onUpdateCursor () (Virtual)
/// <summary>
/// Triggered when the cursor associated with the window is about to be updated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppWindow::onUpdateCursor( )
{
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::AppWindow_OnUpdateCursor;
    return cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAppWindow::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AppWindow )
        return true;

    // Unsupported.
    return false;
}