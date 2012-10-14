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
// Name : cgInputDriver.h                                                    //
//                                                                           //
// Desc : Input class wraps the properties, initialization and management of //
//        our input device. This includes the enumeration, creation and      //
//        destruction of our DirectInput devices and associated resources.   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGINPUTDRIVER_H_ )
#define _CGE_CGINPUTDRIVER_H_

//-----------------------------------------------------------------------------
// cgInputDriver Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Input/cgInputTypes.h>
#include <System/cgReference.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgAppWindow;

#if defined(UNICODE) || defined(_UNICODE)
struct IDirectInput8W;
struct IDirectInputDevice8W;
struct IDirectInputDevice8W;
#else // UNICODE
struct IDirectInput8A;
struct IDirectInputDevice8A;
struct IDirectInputDevice8A;
#endif // !UNICODE

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {75DEEBF1-E0B8-46B2-B901-C3EBE7A86EFE}
const cgUID RTID_InputDriver = {0x75DEEBF1, 0xE0B8, 0x46B2, {0xB9, 0x1, 0xC3, 0xEB, 0xE7, 0xA8, 0x6E, 0xFE}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgInputListener (Class)
/// <summary>
/// Abstract interface class from which other classes can derive in order 
/// to recieve messages whenever input events occur.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgInputListener : public cgEventListener
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgInputListener, cgEventListener, "InputListener" )

public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void    onMouseMove         ( const cgPoint & position, const cgPointF & offset ) {};
    virtual void    onMouseButtonDown   ( cgInt32 buttons, const cgPoint & position ) {};
    virtual void    onMouseButtonUp     ( cgInt32 buttons, const cgPoint & position ) {};
    virtual void    onMouseWheelScroll  ( cgInt32 delta, const cgPoint & position ) {};
    virtual void    onKeyDown           ( cgInt32 keyCode, cgUInt32 modifiers ) {};
    virtual void    onKeyUp             ( cgInt32 keyCode, cgUInt32 modifiers ) {};
    virtual void    onKeyPressed        ( cgInt32 keyCode, cgUInt32 modifiers ) {};
};

//-----------------------------------------------------------------------------
//  Name : cgInputDriver (Class)
/// <summary>
/// Class that contains our core input handling / initialization logic.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgInputDriver : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgInputDriver, cgReference, "InputDriver" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations 
    //-------------------------------------------------------------------------
    // The selected input driver configuration options
    struct InitConfig
    {
        bool     ignoreWindowsKey;      // Should the 'Windows' key be ignored?
        cgFloat  mouseSensitivity;      // Scalar describing the mouse sensitivity
        cgUInt32 keyRepeatDelay;        // The amount of time, in milliseconds, to wait before re-sending key pressed messages
        cgUInt32 keyRepeatRate;         // Once key repeat delay has passed, how quickly should the key pressed messages occur.
        cgUInt32 keyboardBufferSize;    // The size of the keyboard buffer in bytes.
        bool     hideSystemCursor;      // Should we hide the OS cursor during initialization?
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgInputDriver( );
    virtual ~cgInputDriver( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgInputDriver      * getInstance             ( );
    static void                 createSingleton         ( );
    static void                 destroySingleton        ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgConfigResult::Base        loadConfig              ( const cgString & fileName );
    cgConfigResult::Base        loadDefaultConfig       ( );
    bool                        saveConfig              ( const cgString & fileName );
    InitConfig                  getConfig               ( ) const;
    bool                        initialize              ( cgAppWindow * focusWindow, bool windowed = true );
    void                        poll                    ( );
    bool                        wasKeyPressed           ( cgInt32 keyCode ) const;
    bool                        isKeyPressed            ( cgInt32 keyCode ) const;
    bool                        isKeyPressed            ( cgInt32 keyCode, bool wasNotPressed ) const;
    bool                        wasMouseButtonPressed   ( cgMouseButtons::Base button ) const;
    bool                        isMouseButtonPressed    ( cgMouseButtons::Base button ) const;
    bool                        isMouseButtonPressed    ( cgMouseButtons::Base button, bool wasNotPressed ) const;
    void                        setMouseMode            ( cgMouseHandlerMode::Base mode );
    cgMouseHandlerMode::Base    getMouseMode            ( ) const;
    const cgPoint             & getCursorPosition       ( ) const;

    // Object query functions
    bool                        isInitialized           ( ) const { return mInitialized; }

    // Utility methods
    cgString                    keyCodeToCharacter      ( cgInt32 keyCode );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType        ( ) const { return RTID_InputDriver; }
    virtual bool                queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                Dispose                 ( bool bDisposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures & Enumerations 
    //-------------------------------------------------------------------------
    // Structure containing information about to send repeats for
    struct KeyRepeatInfo
    {
        cgUInt32 timePressed;           // Time in milliseconds at which the key was pressed
        cgUInt32 timeLastSignalled;     // Time in milliseconds at which the onKeyPressed event was last sent for this key
    };
    CGE_UNORDEREDMAP_DECLARE(cgInt32, KeyRepeatInfo, KeyRepeatMap)

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void                onMouseMove             ( const cgPoint & position, const cgPointF & offset );
    void                onMouseButtonDown       ( cgInt32 buttons, const cgPoint & position );
    void                onMouseButtonUp         ( cgInt32 buttons, const cgPoint & position );
    void                onMouseWheelScroll      ( cgInt32 delta, const cgPoint & position );
    void                onKeyDown               ( cgInt32 keyCode, cgUInt32 modifiers );
    void                onKeyUp                 ( cgInt32 keyCode, cgUInt32 modifiers );
    void                onKeyPressed            ( cgInt32 keyCode, cgUInt32 modifiers );

    //-------------------------------------------------------------------------
    // Private Static Variables.
    //-------------------------------------------------------------------------
    static cgMouseButtons::Base mouseButtonIdxToEnum( cgInt32 button );
    static cgInt32              mouseButtonEnumToIdx( cgMouseButtons::Base button );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    InitConfig                  mConfig;                    // Input driver configuration settings
    bool                        mConfigLoaded;              // Has the configuration been loaded yet?
    bool                        mInitialized;               // Has the driver been fully initialized?
    cgAppWindow               * mFocusWnd;                  // The main focus window for the input device

#if defined(UNICODE) || defined(_UNICODE)
    IDirectInput8W            * mDI;                        // The main DirectX Direct Input object.
    IDirectInputDevice8W      * mDIKeyboard;                // Keyboard device.
    IDirectInputDevice8W      * mDIMouse;                   // Mouse device.
#else // UNICODE
    IDirectInput8A            * mDI;                        // The main DirectX Direct Input object.
    IDirectInputDevice8A      * mDIKeyboard;                // Keyboard device.
    IDirectInputDevice8A      * mDIMouse;                   // Mouse device.
#endif // !UNICODE

    cgUInt8                     mPrevKeyStates[256];        // Buffer storing all previous states of the keyboard keys (on last poll)
    cgUInt8                     mKeyStates[256];            // Buffer storing all current states of the keyboard keys
    cgUInt8                     mPrevMouseButtonStates[5];  // Previous state of the mouse buttons.
    cgUInt8                     mMouseButtonStates[5];      // Current state of the mouse buttons.

    cgPoint                     mMousePosition;             // The recorded position of the mouse (floating point so we accumulate small motions).
    cgMouseHandlerMode::Base    mMouseMode;                 // How mouse motion should be interpreted or adjusted.
    bool                        mWindowed;                  // The application is running in windowed mode?

    KeyRepeatMap                mKeyRepeatData;             // Information about repeated key pressed messages to be sent
    
    //-------------------------------------------------------------------------
    // Private Static Variables.
    //-------------------------------------------------------------------------
    static cgInputDriver      * mSingleton;                 // Static singleton object instance.

};

#endif // !_CGE_CGINPUTDRIVER_H_