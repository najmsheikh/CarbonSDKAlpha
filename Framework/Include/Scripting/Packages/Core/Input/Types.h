#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Input/cgInputTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Input {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Input.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "Keys" ) );
            BINDSUCCESS( engine->registerEnum( "MouseButtons" ) );
            BINDSUCCESS( engine->registerEnum( "MouseHandlerMode" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgKeys (Enum).
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "Keys";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "Escape"       , cgKeys::Escape ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D1"           , cgKeys::D1 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D2"           , cgKeys::D2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D3"           , cgKeys::D3 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D4"           , cgKeys::D4 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D5"           , cgKeys::D5 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D6"           , cgKeys::D6 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D7"           , cgKeys::D7 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D8"           , cgKeys::D8 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D9"           , cgKeys::D9 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D0"           , cgKeys::D0 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Minus"        , cgKeys::Minus ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Equals"       , cgKeys::Equals ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Backspace"    , cgKeys::Backspace ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Tab"          , cgKeys::Tab ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Q"            , cgKeys::Q ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "W"            , cgKeys::W ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "E"            , cgKeys::E ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "R"            , cgKeys::R ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "T"            , cgKeys::T ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Y"            , cgKeys::Y ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "U"            , cgKeys::U ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "I"            , cgKeys::I ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "O"            , cgKeys::O ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "P"            , cgKeys::P ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LBracket"     , cgKeys::LBracket ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RBracket"     , cgKeys::RBracket ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Return"       , cgKeys::Return ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LControl"     , cgKeys::LControl ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "A"            , cgKeys::A ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "S"            , cgKeys::S ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "D"            , cgKeys::D ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F"            , cgKeys::F ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "G"            , cgKeys::G ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "H"            , cgKeys::H ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "J"            , cgKeys::J ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "K"            , cgKeys::K ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "L"            , cgKeys::L ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Semicolon"    , cgKeys::Semicolon ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Apostrophe"   , cgKeys::Apostrophe ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Grave"        , cgKeys::Grave ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LShift"       , cgKeys::LShift ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Backslash"    , cgKeys::Backslash ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Z"            , cgKeys::Z ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "X"            , cgKeys::X ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "C"            , cgKeys::C ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "V"            , cgKeys::V ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "B"            , cgKeys::B ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "N"            , cgKeys::N ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "M"            , cgKeys::M ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Comma"        , cgKeys::Comma ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Period"       , cgKeys::Period ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Slash"        , cgKeys::Slash ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RShift"       , cgKeys::RShift ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadStar"   , cgKeys::NumpadStar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LAlt"         , cgKeys::LAlt ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Space"        , cgKeys::Space ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CapsLock"     , cgKeys::CapsLock ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F1"           , cgKeys::F1 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F2"           , cgKeys::F2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F3"           , cgKeys::F3 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F4"           , cgKeys::F4 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F5"           , cgKeys::F5 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F6"           , cgKeys::F6 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F7"           , cgKeys::F7 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F8"           , cgKeys::F8 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F9"           , cgKeys::F9 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F10"          , cgKeys::F10 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumLock"      , cgKeys::NumLock ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ScrollLock"   , cgKeys::ScrollLock ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad7"      , cgKeys::Numpad7 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad8"      , cgKeys::Numpad8 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad9"      , cgKeys::Numpad9 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadMinus"  , cgKeys::NumpadMinus ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad4"      , cgKeys::Numpad4 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad5"      , cgKeys::Numpad5 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad6"      , cgKeys::Numpad6 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadPlus"   , cgKeys::NumpadPlus ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad1"      , cgKeys::Numpad1 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad2"      , cgKeys::Numpad2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad3"      , cgKeys::Numpad3 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Numpad0"      , cgKeys::Numpad0 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadPeriod" , cgKeys::NumpadPeriod ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Oem102"       , cgKeys::Oem102 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F11"          , cgKeys::F11 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F12"          , cgKeys::F12 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F13"          , cgKeys::F13 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F14"          , cgKeys::F14 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "F15"          , cgKeys::F15 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "KanaMode"     , cgKeys::KanaMode ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "IMEConvert"   , cgKeys::IMEConvert ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "IMENoConvert" , cgKeys::IMENoConvert ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Yen"          , cgKeys::Yen ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadEquals" , cgKeys::NumpadEquals ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PreviousTrack", cgKeys::PreviousTrack ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "At"           , cgKeys::At ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Colon"        , cgKeys::Colon ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Underline"    , cgKeys::Underline ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "KanjiMode"    , cgKeys::KanjiMode ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Stop"         , cgKeys::Stop ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AX"           , cgKeys::AX ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NextTrack"    , cgKeys::NextTrack ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadEnter"  , cgKeys::NumpadEnter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RControl"     , cgKeys::RControl ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Mute"         , cgKeys::Mute ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Calculator"   , cgKeys::Calculator ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PlayPause"    , cgKeys::PlayPause ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MediaStop"    , cgKeys::MediaStop ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "VolumeDown"   , cgKeys::VolumeDown ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "VolumeUp"     , cgKeys::VolumeUp ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebHome"      , cgKeys::WebHome ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadComma"  , cgKeys::NumpadComma ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NumpadSlash"  , cgKeys::NumpadSlash ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SysRequest"   , cgKeys::SysRequest ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RAlt"         , cgKeys::RAlt ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Pause"        , cgKeys::Pause ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Home"         , cgKeys::Home ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Up"           , cgKeys::Up ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PageUp"       , cgKeys::PageUp ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Left"         , cgKeys::Left ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Right"        , cgKeys::Right ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "End"          , cgKeys::End ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Down"         , cgKeys::Down ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PageDown"     , cgKeys::PageDown ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Insert"       , cgKeys::Insert ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Delete"       , cgKeys::Delete ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LWin"         , cgKeys::LWin ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RWin"         , cgKeys::RWin ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Apps"         , cgKeys::Apps ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Power"        , cgKeys::Power ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Sleep"        , cgKeys::Sleep ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Wake"         , cgKeys::Wake ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebSearch"    , cgKeys::WebSearch ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebFavorites" , cgKeys::WebFavorites ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebRefresh"   , cgKeys::WebRefresh ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebStop"      , cgKeys::WebStop ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebForward"   , cgKeys::WebForward ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WebBack"      , cgKeys::WebBack ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MyComputer"   , cgKeys::MyComputer ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Mail"         , cgKeys::Mail ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MediaSelect"  , cgKeys::MediaSelect ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMouseHandlerMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "MouseHandlerMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Direct", cgMouseHandlerMode::Direct ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Cursor", cgMouseHandlerMode::Cursor ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMouseButtons (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "MouseButtons";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Left"    , cgMouseButtons::Left ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Right"   , cgMouseButtons::Right ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Middle"  , cgMouseButtons::Middle ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "XButton1", cgMouseButtons::XButton1 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "XButton2", cgMouseButtons::XButton2 ) );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Input::Types