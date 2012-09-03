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
//      Copyright (c) 1997 - 2008 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWinAppWindow Module Includes
//-----------------------------------------------------------------------------
#include <System/Platform/cgWinAppWindow.h>

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
    mWnd        = CG_NULL;
    mOwnsWindow = false;
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
    mWnd        = (HWND)pNativeWindow;
    mOwnsWindow = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgWinAppWindow () (Destructor)
/// <summary>
/// cgWinAppWindow Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgWinAppWindow::~cgWinAppWindow()
{
    // Clean up window data.
    if ( mWnd != CG_NULL && mOwnsWindow == true )
    {
        DestroyWindow( mWnd );
        UnregisterClass( mWndClass.lpszClassName, mWndClass.hInstance );

    } // End if created

    // Clear variables
    mWnd        = CG_NULL;
    mOwnsWindow = false;
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

    // Store details
    mClassName = strTitle;

    // Register the new window's window class.
    mWndClass.style            = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
    mWndClass.lpfnWndProc      = staticWndProc;
    mWndClass.cbClsExtra       = 0;
    mWndClass.cbWndExtra       = 0;
    mWndClass.hInstance        = (HINSTANCE)GetModuleHandle(NULL);
    mWndClass.hIcon            = NULL;
    mWndClass.hCursor          = LoadCursor(NULL, IDC_ARROW);
    mWndClass.hbrBackground    = (HBRUSH )GetStockObject(BLACK_BRUSH);
    mWndClass.lpszMenuName     = NULL;
    mWndClass.lpszClassName    = mClassName.c_str();

    if ( IconResource >= 0 )
        mWndClass.hIcon = LoadIcon( mWndClass.hInstance, MAKEINTRESOURCE( IconResource ) );

    RegisterClass(&mWndClass);

    // Select the window layout
    if ( bFullscreen == true )
    {
        Left  = 0; Top = 0;
        Style = WS_VISIBLE | WS_POPUP;
        
    } // End if Fullscreen
    else
    {
        // Adjust width / height to take into account window decoration
        Width  += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
        Height += (GetSystemMetrics( SM_CYSIZEFRAME ) * 2) + GetSystemMetrics( SM_CYCAPTION );
    
    } // End if windowed

    // Create the window
    mWnd = CreateWindow( strTitle.c_str(), strTitle.c_str(), Style, Left,
                           Top, Width, Height, NULL, NULL, mWndClass.hInstance, this );


    // Success?
    mOwnsWindow = true;
    return (mWnd != CG_NULL);
    
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
        case WM_CREATE:
            
            // Trigger creation message
            onCreate();
            break;

        case WM_SETCURSOR:

            // ToDo: Remove?
            /*// Hide the cursor inside the client area
            if ( LOWORD(lParam) == 1 )
            {
                SetCursor( CG_NULL );
                return TRUE;
            } // End if in client area*/
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