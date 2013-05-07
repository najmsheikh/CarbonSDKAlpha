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
// Name : cgAppWindow.h                                                      //
//                                                                           //
// Desc : Base window / form class that provides an interface between the    //
//        engine and any platform specific window handling logic.            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAPPWINDOW_H_ )
#define _CGE_CGAPPWINDOW_H_

//-----------------------------------------------------------------------------
// cgAppWindow Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {989C5A6B-CC6B-49AD-B17B-B0E44C437955}
const cgUID RTID_AppWindow = {0x989C5A6B, 0xCC6B, 0x49AD, {0xB1, 0x7B, 0xB0, 0xE4, 0x4C, 0x43, 0x79, 0x55}};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgCursor;

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
// AppWindow_OnSize Message Data
struct CGE_API cgWindowSizeEventArgs
{
    cgSize  size;
    bool    minimized;
    
    // Constructor
    cgWindowSizeEventArgs( const cgSize & _size, bool _minimized ) : 
        size(_size), minimized(_minimized) {}
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgAppWindow (Class)
/// <summary>
/// Base window / form class that provides an interface between the
/// engine and any platform specific window handling logic.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgAppWindow : public cgReference
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
	         cgAppWindow();
	virtual ~cgAppWindow();

    //-------------------------------------------------------------------------
	// Public Static Functions
	//-------------------------------------------------------------------------
    static cgAppWindow    * createInstance          ( );
    static cgAppWindow    * createInstance          ( void * nativeWindow );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    cgRect                  screenToClient          ( const cgRect & screenRectangle );
    cgRect                  clientToScreen          ( const cgRect & clientRectangle );
    void                    showCursor              ( bool show );

    //-------------------------------------------------------------------------
	// Public Virtual Methods
	//-------------------------------------------------------------------------
    virtual bool            create                  ( bool fullScreen, cgUInt32 width, cgUInt32 height, const cgString & title = _T("Render Output"), cgInt32 iconResource = -1 ) = 0;
    virtual cgSize          getSize                 ( ) const = 0;
    virtual cgRect          getRect                 ( ) const = 0;
    virtual cgSize          getClientSize           ( ) const = 0;
    virtual cgRect          getClientRect           ( ) const = 0;
    virtual void            show                    ( ) = 0;
    virtual cgPoint         screenToClient          ( const cgPoint & screenPoint ) = 0;
    virtual cgPoint         clientToScreen          ( const cgPoint & clientPoint ) = 0;
    virtual void            setTitle                ( const cgString & title ) = 0;
    virtual void            setSize                 ( const cgSize & size ) = 0;
    virtual void            setClientSize           ( const cgSize & size ) = 0;
    virtual void            setFullScreenMode       ( bool fullScreen ) = 0;
    virtual void            setCursor               ( cgCursor * cursor );

    // Events
    virtual void            onSize                  ( const cgSize & size, bool minimized );
    virtual void            onCreate                ( );
    virtual void            onClose                 ( );
    virtual bool            onUpdateCursor          ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType        ( ) const { return RTID_AppWindow; }
    virtual bool            queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    cgCursor  * mCursor;            // Cursor icon to be applied automatically when the user's cursor is over this window.
    cgInt       mCursorVisCount;    // Hidden <= 0 < Visible

};

#endif // !_CGE_CGAPPWINDOW_H_