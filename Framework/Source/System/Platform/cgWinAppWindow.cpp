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
// Name : cgWinAppWindow.cpp                                                 //
//                                                                           //
// Desc : Custom derived window class specific to the Windows(tm) platform.  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinAppWindow Module Includes
//-----------------------------------------------------------------------------
#include <System/Platform/cgWinAppWindow.h>
#include <System/Platform/cgWinCursor.h>

///////////////////////////////////////////////////////////////////////////////
// cgWinAppWindow Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgWinAppWindow () (Constructor)
/// <summary>
/// cgWinAppWindow Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinAppWindow::cgWinAppWindow()
{
    // Clear variables
    mWnd                = CG_NULL;
    mClassAtom          = CG_NULL;
    mOwnsWindow         = false;
    mFullScreen         = false;
    mCursorConstrained  = false;
}

//-----------------------------------------------------------------------------
//  Name : cgWinAppWindow () (Constructor)
/// <summary>
/// cgWinAppWindow Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinAppWindow::cgWinAppWindow( void * pNativeWindow ) : cgAppWindow( )
{
    // Clear variables
    mWnd                = (HWND)pNativeWindow;
    mClassAtom          = CG_NULL;
    mOwnsWindow         = false;
    mFullScreen         = false;
    mCursorConstrained  = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinAppWindow () (Destructor)
/// <summary>
/// cgWinAppWindow Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinAppWindow::~cgWinAppWindow()
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
void cgWinAppWindow::dispose( bool bDisposeBase )
{
    // Release cursor constraint
    constrainCursor( false );

    // Clean up window data.
    if ( mWnd && mOwnsWindow )
    {
        DestroyWindow( mWnd );
        UnregisterClass( (LPCTSTR)mClassAtom, mWndClass.hInstance );

    } // End if created

    // Clear variables
    mWnd                = CG_NULL;
    mClassAtom          = CG_NULL;
    mOwnsWindow         = false;
    mCursorConstrained  = false;

    // Call base class implementation
    if ( bDisposeBase )
        cgAppWindow::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : getWindowHandle ()
/// <summary>
/// Retrieve the OS specific window handle being wrapped by this window
/// object instance.
/// </summary>
//-----------------------------------------------------------------------------
HWND cgWinAppWindow::getWindowHandle( ) const
{
    return mWnd;
}

//-----------------------------------------------------------------------------
//  Name : create () (Virtual)
/// <summary>
/// Create the application window ready for display.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinAppWindow::create( bool bFullscreen, cgUInt32 Width, cgUInt32 Height, const cgString & strTitle /* = _T("Render Output") */, cgInt32 IconResource /* = -1 */ )
{
    cgUInt32 Left  = CW_USEDEFAULT, Top = CW_USEDEFAULT;
    cgUInt32 Style = WS_OVERLAPPEDWINDOW;
    cgUInt32 StyleEx = 0;

    // Store details
    mFullScreen = bFullscreen;

    // Register the new window's window class.
    memset( &mWndClass, 0, sizeof(WNDCLASS));
    mWndClass.style            = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
    mWndClass.lpfnWndProc      = staticWndProc;
    mWndClass.cbClsExtra       = 0;
    mWndClass.cbWndExtra       = 0;
    mWndClass.hInstance        = (HINSTANCE)GetModuleHandle(NULL);
    mWndClass.hIcon            = NULL;
    mWndClass.hCursor          = LoadCursor(NULL, IDC_ARROW);
    mWndClass.hbrBackground    = (HBRUSH )GetStockObject(BLACK_BRUSH);
    mWndClass.lpszMenuName     = NULL;
    mWndClass.lpszClassName    = strTitle.c_str();
    if ( IconResource >= 0 )
        mWndClass.hIcon = LoadIcon( mWndClass.hInstance, MAKEINTRESOURCE( IconResource ) );
    mClassAtom = RegisterClass(&mWndClass);

    // Select the window layout
    if ( bFullscreen == true )
    {
        Left  = 0; Top = 0;
        Style = WS_VISIBLE | WS_POPUP;
        StyleEx = WS_EX_TOPMOST;
        
    } // End if Fullscreen
    else
    {
        // Adjust width / height to take into account window decoration
        Width  += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
        Height += (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );
    
    } // End if windowed

    // Create the window
    mWnd = CreateWindowEx( StyleEx, (LPCTSTR)mClassAtom, strTitle.c_str(), Style, Left,
                           Top, Width, Height, NULL, NULL, mWndClass.hInstance, this );

    // Success?
    mOwnsWindow = true;
    return (mWnd != CG_NULL);
    
}

//-----------------------------------------------------------------------------
//  Name : setSize ()
/// <summary>
/// Alter the size of the window such that its outer dimensions (including 
/// non-client regions such as caption and border) match those supplied.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::setSize( const cgSize & size )
{
    SetWindowPos( mWnd, NULL, 0, 0, size.width, size.height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
}

//-----------------------------------------------------------------------------
//  Name : setClientSize ()
/// <summary>
/// Alter the size of the window such that its inner client area dimensions 
/// (excluding non-client regions such as caption and border) match those 
/// supplied.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::setClientSize( const cgSize & size )
{
    cgSize outerSize = size;

    // Adjust the size to include the border / caption, etc. if this
    // window is not in fullscreen mode.
    if ( !mFullScreen )
    {
        outerSize.width  += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
        outerSize.height += (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );
    
    } // End if popup window
    SetWindowPos( mWnd, NULL, 0, 0, outerSize.width, outerSize.height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
}

//-----------------------------------------------------------------------------
//  Name : setFullScreenMode ()
/// <summary>
/// Alter the styling of the window to match the requested full screen mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::setFullScreenMode( bool fullScreen )
{
    // Is this a no-op?
    if ( mFullScreen == fullScreen )
        return;

    // Switching to windowed or full screen?
    if ( fullScreen )
    {
        // Switch to borderless style and set always on top.
        cgUInt32 Style = WS_VISIBLE | WS_POPUP;
        cgUInt32 StyleEx = GetWindowLong( mWnd, GWL_EXSTYLE ) | WS_EX_TOPMOST;
        SetWindowLong( mWnd, GWL_STYLE, Style );
        SetWindowLong( mWnd, GWL_EXSTYLE, StyleEx );
        SetWindowPos( mWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );

    } // End if full screen
    else
    {
        // Re-add borders and remove top most style
        cgUInt32 Style = WS_OVERLAPPEDWINDOW;
        cgUInt32 StyleEx = GetWindowLong( mWnd, GWL_EXSTYLE ) & ~WS_EX_TOPMOST;
        SetWindowLong( mWnd, GWL_STYLE, Style );
        SetWindowLong( mWnd, GWL_EXSTYLE, StyleEx );
        SetWindowPos( mWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );

    } // End if windowed

    // Store current state
    mFullScreen = fullScreen;
}

//-----------------------------------------------------------------------------
//  Name : staticWndProc () (Static Callback)
/// <summary>
/// This is the main messge pump for ALL display devices, it captures
/// the appropriate messages, and routes them through to the application
/// class for which it was intended, therefore giving full class access.
/// Note : It is VITALLY important that you should pass your 'this' pointer to
/// the lpParam parameter of the CreateWindow function if you wish to be
/// able to pass messages back to that app object.
/// </summary>
//-----------------------------------------------------------------------------
LRESULT CALLBACK cgWinAppWindow::staticWndProc(HWND hWnd, cgUInt Message, WPARAM wParam, LPARAM lParam)
{
    // NOTE: Added 64bit compatibility casts using 'LONG_PTR' to prevent compatibility warnings

    //    The call to SetWindowLongPtr here may generate a warning when
    //    the compiler has been instructed to inform us of 64-bit
    //    portability issues.
    //
    //        warning C4244: 'argument' :
    //          conversion from 'LONG_PTR' to 'LONG', possible loss of data
    //
    //    It is safe to ignore this warning because it is bogus per MSDN Magazine.
    //
    //        http://msdn.microsoft.com/msdnmag/issues/01/08/bugslayer/ 

    // If this is a create message, trap the 'this' pointer passed in and store it within the window.
    #pragma warning (push)
    #pragma warning (disable : 4244) // inhibit ignorable warning
    if ( Message == WM_CREATE ) SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT FAR *)lParam)->lpCreateParams);
    #pragma warning (pop) 

    // Obtain the correct destination for this message
    cgWinAppWindow *Destination = (cgWinAppWindow*)((LONG_PTR)GetWindowLongPtr( hWnd, GWL_USERDATA ));
    
    // If the hWnd has a related class, pass it through
    if (Destination) return Destination->WndProc( hWnd, Message, wParam, lParam );

    // No destination found, defer to system...
    return DefWindowProc( hWnd, Message, wParam, lParam );
}

//-----------------------------------------------------------------------------
//  Name : WndProc ()
/// <summary>
/// The window's internal WndProc function. All messages being passed to 
/// this method are specific to this application window instance.
/// </summary>
//-----------------------------------------------------------------------------
LRESULT cgWinAppWindow::WndProc( HWND hWnd, cgUInt Message, WPARAM wParam, LPARAM lParam )
{
    RECT rc;

    // Determine message type
    switch (Message)
    {
        case WM_ACTIVATEAPP:
            if ( wParam )
            {
                // Activating. Re-activate cursor constraint if necessary.
                if ( mCursorConstrained )
                {
                    cgRect clipRect = cgAppWindow::clientToScreen(getClientRect());
                    ClipCursor( (RECT*)&clipRect );
                
                } // End if constrained

            } // End if activating
            else
            {
                // De-activating, release the constraint
                if ( mCursorConstrained )
                    ClipCursor( CG_NULL );
                
            } // End if deactivating
            break;

        case WM_CREATE:
            
            // Trigger creation message
            onCreate();
            break;

        case WM_CLOSE:

            // Trigger close message
            onClose();
            break;

        case WM_SETCURSOR:

            // Cursor hidden?
            if ( mCursorVisCount <= 0 )
            {
                SetCursor( NULL );
                return TRUE;
            
            } // End if cursor hidden

            // Trigger cursor update message
            if ( onUpdateCursor() )
                return TRUE;

            // If nothing overrides the cursor, attempt to set any
            // cursor that was supplied via 'cgAppWindow::setCursor()'.
            if ( mCursor )
            {
                SetCursor( (HCURSOR)static_cast<cgWinCursor*>(mCursor)->getCursorHandle() );
                return TRUE;
            
            } // End if cursor supplied
            else
            {
                SetCursor( NULL );
                return TRUE;
            
            } // End if NULL cursor supplied
            break;

        case WM_SIZE:

            // Retrieve new client rectangle for window                
            ::GetClientRect( hWnd, &rc );

            // Trigger the window resize event
            onSize( cgSize(rc.right - rc.left, rc.bottom - rc.top), ( wParam == SIZE_MINIMIZED ) );
            break;

    } // End Message Switch

    // Perform default processing
    return DefWindowProc(hWnd, Message, wParam, lParam);
}

//-----------------------------------------------------------------------------
//  Name : showCursor ()
/// <summary>
/// Show or hide the platform cursor for this window. This is implemented
/// as a counter whereby multiple calls to show the cursor are counted and
/// thus will require the same number of matching calls to hide the cursor 
/// before it will be hidden.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::showCursor( bool show )
{
    // Call base class implementation first
    cgAppWindow::showCursor( show );

    // If first time hidden, force window cursor update.
    // Otherwise, force it to be shown.
    if ( !mCursorVisCount )
        SetCursor( NULL ); // Win API
    else if ( mCursorVisCount == 1 )
        setCursor( mCursor ); // Internal method
}

//-----------------------------------------------------------------------------
//  Name : setCursor () (Virtual)
/// <summary>
/// Update the cursor image to be displayed when the user's cursor falls within
/// this application window.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::setCursor( cgCursor * pCursor )
{
    // Call base class implementation first.
    cgAppWindow::setCursor( pCursor );

    // Attempt to set any cursor that was supplied (unless hidden)
    if ( mCursorVisCount > 0 )
    {
        if ( mCursor )
            SetCursor( static_cast<cgWinCursor*>(mCursor)->getCursorHandle() );
        else
            SetCursor( CG_NULL );
    
    } // End if show cursor
}

//-----------------------------------------------------------------------------
//  Name : getSize () (Virtual)
/// <summary>
/// Retrieve the size of the window including its non client area.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgWinAppWindow::getSize( ) const
{
    RECT rcWnd;
    ::GetWindowRect( mWnd, &rcWnd );
    return cgSize( rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top );
}

//-----------------------------------------------------------------------------
//  Name : getRect () (Virtual)
/// <summary>
/// Retrieve the size and position of the window including its non client
/// area.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgWinAppWindow::getRect( ) const
{
    cgRect rcWnd;
    ::GetWindowRect( mWnd, (RECT*)&rcWnd );
    return rcWnd;
}

//-----------------------------------------------------------------------------
//  Name : getClientSize () (Virtual)
/// <summary>
/// Retrieve the size of the window excluding its non client area.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgWinAppWindow::getClientSize( ) const
{
    RECT rcWnd;
    ::GetClientRect( mWnd, &rcWnd );
    return cgSize( rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top );
}

//-----------------------------------------------------------------------------
//  Name : getClientRect () (Virtual)
/// <summary>
/// Retrieve the size and position of the window excluding its non client
/// area.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgWinAppWindow::getClientRect( ) const
{
    cgRect rcOut;
    ::GetClientRect( mWnd, (RECT*)&rcOut );
    return rcOut;
}

//-----------------------------------------------------------------------------
//  Name : show () (Virtual)
/// <summary>
/// Display the window if it is currently hidden.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::show( )
{
    ::ShowWindow( mWnd, SW_SHOW );
}

//-----------------------------------------------------------------------------
//  Name : setTitle () (Virtual)
/// <summary>
/// Set a new title for the window caption.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::setTitle( const cgString & strTitle )
{
    ::SetWindowText( mWnd, strTitle.c_str() );
}

//-----------------------------------------------------------------------------
//  Name : screenToClient () (Virtual)
/// <summary>
/// Convert a point currently expressed in screen space coordinates into
/// the space of this window's client area.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgWinAppWindow::screenToClient( const cgPoint & ptScreen )
{
    cgPoint ptOut = ptScreen;
    ::ScreenToClient( mWnd, (POINT*)&ptOut);
    return ptOut;
}

//-----------------------------------------------------------------------------
//  Name : clientToScreen () (Virtual)
/// <summary>
/// Convert a point currently expressed in the space of this window's
/// client area into screen space.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgWinAppWindow::clientToScreen( const cgPoint & ptClient )
{
    cgPoint ptOut = ptClient;
    ::ClientToScreen( mWnd, (POINT*)&ptOut );
    return ptOut;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWinAppWindow::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_WinAppWindow )
        return true;

    // Supported by base?
    return cgAppWindow::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : constrainCursor ()
/// <summary>
/// Enable / disable constraint that prevents cursor from leaving the client
/// area of the window while it has focus. Only one window may constrain at
/// a time.
/// </summary>
//-----------------------------------------------------------------------------
void cgWinAppWindow::constrainCursor( bool enable )
{
    // No-op?
    if ( mCursorConstrained == enable )
        return;

    // Capture cursor?
    if ( enable )
    {
        cgRect clipRect = cgAppWindow::clientToScreen(getClientRect());
        ClipCursor( (RECT*)&clipRect );
    
    } // End if enabled
    else
    {
        ClipCursor( CG_NULL );

    } // End if disabled

    // Store local variable
    mCursorConstrained = enable;
}