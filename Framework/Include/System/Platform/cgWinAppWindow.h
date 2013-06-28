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
// Name : cgWinAppWindow.h                                                   //
//                                                                           //
// Desc : Custom derived window class specific to the Windows(tm) platform.  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGWINAPPWINDOW_H_ )
#define _CGE_CGWINAPPWINDOW_H_

//-----------------------------------------------------------------------------
// cgWinAppWindow Header Includes
//-----------------------------------------------------------------------------
#include <System/cgAppWindow.h>

// Windows platform includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {79987E3F-0FB8-4335-A4E0-B187161D73C0}
const cgUID RTID_WinAppWindow = {0x79987E3F, 0xFB8, 0x4335, {0xA4, 0xE0, 0xB1, 0x87, 0x16, 0x1D, 0x73, 0xC0}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgWinAppWindow (Class)
/// <summary>
/// Custom derived window class specific to the Windows(tm) platform.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgWinAppWindow : public cgAppWindow
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	         cgWinAppWindow();
             cgWinAppWindow( void * nativeWindow );
	virtual ~cgWinAppWindow();

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    HWND                    getWindowHandle     ( ) const;

    //-------------------------------------------------------------------------
	// Public Virtual Methods (Overrides cgAppWindow)
	//-------------------------------------------------------------------------
    virtual bool            create              ( bool fullScreen, cgUInt32 width, cgUInt32 height, const cgString & title = _T("Render Output"), cgInt32 iconResource = -1 );
    virtual cgSize          getSize             ( ) const;
    virtual cgRect          getRect             ( ) const;
    virtual cgSize          getClientSize       ( ) const;
    virtual cgRect          getClientRect       ( ) const;
    virtual void            show                ( );
    virtual cgPoint         screenToClient      ( const cgPoint & screenPoint );
    virtual cgPoint         clientToScreen      ( const cgPoint & clientPoint );
    virtual void            setTitle            ( const cgString & title );
    virtual void            setSize             ( const cgSize & size );
    virtual void            setClientSize       ( const cgSize & size );
    virtual void            setFullScreenMode   ( bool fullScreen );
    virtual void            setCursor           ( cgCursor * cursor );
    virtual void            showCursor          ( bool show );
    virtual void            constrainCursor     ( bool enable );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_WinAppWindow; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    LRESULT WndProc ( HWND wnd, UINT message, WPARAM wparam, LPARAM lparam );

    //-------------------------------------------------------------------------
	// Protected Variables;
	//-------------------------------------------------------------------------
    HWND        mWnd;               // OS window being wrapped.
    bool        mOwnsWindow;        // Were we responsible for creating the above window handle?
    bool        mFullScreen;        // Is the window currently in fullscreen mode?
    WNDCLASS	mWndClass;
    ATOM        mClassAtom;         // ATOM of the window class generated for this window.
    bool        mCursorConstrained; // Is the cursor currently constrained?

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static LRESULT CALLBACK staticWndProc   ( HWND wnd, UINT message, WPARAM wparam, LPARAM lparam );
};

#endif // !_CGE_CGWINAPPWINDOW_H_