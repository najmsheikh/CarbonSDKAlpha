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
// Name : cgInputTypes.h                                                     //
//                                                                           //
// Desc : Common system file that defines various input related types and    //
//        common enumerations.                                               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGINPUTTYPES_H_ )
#define _CGE_CGINPUTTYPES_H_

//-----------------------------------------------------------------------------
// cgInputTypes Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>

//-----------------------------------------------------------------------------
// Common Global Enumerations
//-----------------------------------------------------------------------------
// Interpretation mode for the mouse
namespace cgMouseHandlerMode
{
    enum Base
    {
        Direct = 0, // Mouse input should be treated as direct input (i.e. only motion detected)
        Cursor = 1  // Mouse input handling should be computed for cursor positioning.
    };

}; // End Namespace : cgMouseHandlerMode

// Modifier key definitions
namespace cgModifierKeys
{
    enum Base
    {
        LeftControl  = 0x1,
        RightControl = 0x2,
        Control      = 0x3,
        LeftShift    = 0x4,
        RightShift   = 0x8,
        Shift        = 0xC,
        LeftAlt      = 0x10,
        RightAlt     = 0x20,
        Alt          = 0x30,
    };

}; // End Namespace : cgModifierKeys

// Register enums.
namespace cgKeys
{
    enum Base
    {
        Escape          = 0x01,
        D1              = 0x02,
        D2              = 0x03,
        D3              = 0x04,
        D4              = 0x05,
        D5              = 0x06,
        D6              = 0x07,
        D7              = 0x08,
        D8              = 0x09,
        D9              = 0x0A,
        D0              = 0x0B,
        Minus           = 0x0C,
        Equals          = 0x0D,
        Backspace       = 0x0E,
        Tab             = 0x0F,
        Q               = 0x10,
        W               = 0x11,
        E               = 0x12,
        R               = 0x13,
        T               = 0x14,
        Y               = 0x15,
        U               = 0x16,
        I               = 0x17,
        O               = 0x18,
        P               = 0x19,
        LBracket        = 0x1A,
        RBracket        = 0x1B,
        Return          = 0x1C,
        LControl        = 0x1D,
        A               = 0x1E,
        S               = 0x1F,
        D               = 0x20,
        F               = 0x21,
        G               = 0x22,
        H               = 0x23,
        J               = 0x24,
        K               = 0x25,
        L               = 0x26,
        Semicolon       = 0x27,
        Apostrophe      = 0x28,
        Grave           = 0x29,
        LShift          = 0x2A,
        Backslash       = 0x2B,
        Z               = 0x2C,
        X               = 0x2D,
        C               = 0x2E,
        V               = 0x2F,
        B               = 0x30,
        N               = 0x31,
        M               = 0x32,
        Comma           = 0x33,
        Period          = 0x34,
        Slash           = 0x35,
        RShift          = 0x36,
        NumpadStar      = 0x37,
        LAlt            = 0x38,
        Space           = 0x39,
        CapsLock        = 0x3A,
        F1              = 0x3B,
        F2              = 0x3C,
        F3              = 0x3D,
        F4              = 0x3E,
        F5              = 0x3F,
        F6              = 0x40,
        F7              = 0x41,
        F8              = 0x42,
        F9              = 0x43,
        F10             = 0x44,
        NumLock         = 0x45,
        ScrollLock      = 0x46,
        Numpad7         = 0x47,
        Numpad8         = 0x48,
        Numpad9         = 0x49,
        NumpadMinus     = 0x4A,
        Numpad4         = 0x4B,
        Numpad5         = 0x4C,
        Numpad6         = 0x4D,
        NumpadPlus      = 0x4E,
        Numpad1         = 0x4F,
        Numpad2         = 0x50,
        Numpad3         = 0x51,
        Numpad0         = 0x52,
        NumpadPeriod    = 0x53,
        Oem102          = 0x56,
        F11             = 0x57,
        F12             = 0x58,
        F13             = 0x64,
        F14             = 0x65,
        F15             = 0x66,
        KanaMode        = 0x70,
        IMEConvert      = 0x79,
        IMENoConvert    = 0x7B,
        Yen             = 0x7D,
        NumpadEquals    = 0x8D,
        PreviousTrack   = 0x90,
        At              = 0x91,
        Colon           = 0x92,
        Underline       = 0x93,
        KanjiMode       = 0x94,
        Stop            = 0x95,
        AX              = 0x96,
        NextTrack       = 0x99,
        NumpadEnter     = 0x9C,
        RControl        = 0x9D,
        Mute            = 0xA0,
        Calculator      = 0xA1,
        PlayPause       = 0xA2,
        MediaStop       = 0xA4,
        VolumeDown      = 0xAE,
        VolumeUp        = 0xB0,
        WebHome         = 0xB2,
        NumpadComma     = 0xB3,
        NumpadSlash     = 0xB5,
        SysRequest      = 0xB7,
        RAlt            = 0xB8,
        Pause           = 0xC5,
        Home            = 0xC7,
        Up              = 0xC8,
        PageUp          = 0xC9,
        Left            = 0xCB,
        Right           = 0xCD,
        End             = 0xCF,
        Down            = 0xD0,
        PageDown        = 0xD1,
        Insert          = 0xD2,
        Delete          = 0xD3,
        LWin            = 0xDB,
        RWin            = 0xDC,
        Apps            = 0xDD,
        Power           = 0xDE,
        Sleep           = 0xDF,
        Wake            = 0xE3,
        WebSearch       = 0xE5,
        WebFavorites    = 0xE6,
        WebRefresh      = 0xE7,
        WebStop         = 0xE8,
        WebForward      = 0xE9,
        WebBack         = 0xEA,
        MyComputer      = 0xEB,
        Mail            = 0xEC,
        MediaSelect     = 0xED
    };

}; // End Namespace cgKeys

namespace cgMouseButtons
{
    enum Base
    {
        None        = 0,
        Left        = 0x1,
        Right       = 0x2,
        Middle      = 0x4,
        XButton1    = 0x8,
        XButton2    = 0x10
    };

}; // End Namespace cgMouseButtons

//-----------------------------------------------------------------------------
// Common Global Structures
//-----------------------------------------------------------------------------
// Contains the information relating to a mouse move event when distributed via the messaging system
struct cgMouseMoveEventArgs
{
    cgPoint     position;
    cgPointF    offset;
};

// Contains the information relating to a mouse button event when distributed via the messaging system
struct cgMouseButtonEventArgs
{
    cgInt32     buttons;
    cgPoint     position;
};

// Contains the information relating to a mouse wheel scroll event when distributed via the messaging system
struct cgMouseWheelScrollEventArgs
{
    cgInt32     delta;
    cgPoint     position;
};

// Contains the information relating to a keyboard key event when distributed via the messaging system
struct cgKeyEventArgs
{
    cgInt32     keyCode;
    cgUInt32    modifiers;
};

#endif // !_CGE_CGINPUTTYPES_H_