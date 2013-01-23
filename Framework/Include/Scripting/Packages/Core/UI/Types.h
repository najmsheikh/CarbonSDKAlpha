#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUITypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI {

// Package declaration
namespace Types
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "FormProperties", sizeof(cgUIFormProperties), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "ImageScaleMode" ) );
            BINDSUCCESS( engine->registerEnum( "ControlCoordinateSpace" ) );
            BINDSUCCESS( engine->registerEnum( "HorizontalAlignment" ) );
            BINDSUCCESS( engine->registerEnum( "VerticalAlignment" ) );
            BINDSUCCESS( engine->registerEnum( "TextTruncationMode" ) );
            BINDSUCCESS( engine->registerEnum( "TextFlags" ) );
            BINDSUCCESS( engine->registerEnum( "FormStyle" ) );
            BINDSUCCESS( engine->registerEnum( "UILayerType" ) );
            BINDSUCCESS( engine->registerEnum( "DockMode" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgImageScaleMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "None"   , cgImageScaleMode::None ) );
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "Center" , cgImageScaleMode::Center ) );
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "Stretch", cgImageScaleMode::Stretch ) );
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "Fit"    , cgImageScaleMode::Fit ) );
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "FitU"   , cgImageScaleMode::FitU ) );
            BINDSUCCESS( engine->registerEnumValue( "ImageScaleMode", "FitV"   , cgImageScaleMode::FitV ) );

            ///////////////////////////////////////////////////////////////////////
            // cgControlCoordinateSpace (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "ControlCoordinateSpace", "ClientRelative" , cgControlCoordinateSpace::ClientRelative ) );
            BINDSUCCESS( engine->registerEnumValue( "ControlCoordinateSpace", "ControlRelative", cgControlCoordinateSpace::ControlRelative ) );
            BINDSUCCESS( engine->registerEnumValue( "ControlCoordinateSpace", "ScreenRelative" , cgControlCoordinateSpace::ScreenRelative ) );

            ///////////////////////////////////////////////////////////////////////
            // cgUIFormStyle (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "FormStyle", "Overlapped" , cgUIFormStyle::Overlapped ) );

            ///////////////////////////////////////////////////////////////////////
            // cgUILayerType (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "UILayerType", "UserLayer"  , cgUILayerType::UserLayer ) );
            BINDSUCCESS( engine->registerEnumValue( "UILayerType", "SystemLayer", cgUILayerType::SystemLayer ) );

            ///////////////////////////////////////////////////////////////////////
            // cgHorizontalAlignment (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "HorizontalAlignment", "Left"  , cgHorizontalAlignment::Left ) );
            BINDSUCCESS( engine->registerEnumValue( "HorizontalAlignment", "Center", cgHorizontalAlignment::Center ) );
            BINDSUCCESS( engine->registerEnumValue( "HorizontalAlignment", "Right" , cgHorizontalAlignment::Right ) );

            ///////////////////////////////////////////////////////////////////////
            // cgVerticalAlignment (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "VerticalAlignment", "Top"   , cgVerticalAlignment::Top ) );
            BINDSUCCESS( engine->registerEnumValue( "VerticalAlignment", "Middle", cgVerticalAlignment::Middle ) );
            BINDSUCCESS( engine->registerEnumValue( "VerticalAlignment", "Bottom", cgVerticalAlignment::Bottom ) );

            ///////////////////////////////////////////////////////////////////////
            // cgTextTruncationMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "TextTruncationMode", "None"    , cgTextTruncationMode::None ) );
            BINDSUCCESS( engine->registerEnumValue( "TextTruncationMode", "Ellipsis", cgTextTruncationMode::Ellipsis ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDockMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "None"  , cgDockMode::None ) );
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "Left"  , cgDockMode::Left ) );
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "Right" , cgDockMode::Right ) );
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "Top"   , cgDockMode::Top ) );
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "Bottom", cgDockMode::Bottom ) );
            BINDSUCCESS( engine->registerEnumValue( "DockMode", "Fill"  , cgDockMode::Fill ) );

            ///////////////////////////////////////////////////////////////////////
            // cgTextFlags (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "ClipRectangle"  , cgTextFlags::ClipRectangle ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "Multiline"      , cgTextFlags::Multiline ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "AlignCenter"    , cgTextFlags::AlignCenter ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "AlignRight"     , cgTextFlags::AlignRight ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "VAlignCenter"   , cgTextFlags::VAlignCenter ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "VAlignBottom"   , cgTextFlags::VAlignBottom ) );
            BINDSUCCESS( engine->registerEnumValue( "TextFlags", "AllowFormatCode", cgTextFlags::AllowFormatCode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgUIFormProperties (struct)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "FormStyle style" , offsetof(cgUIFormProperties,style) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "bool sizable"    , offsetof(cgUIFormProperties,sizable) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "bool movable"    , offsetof(cgUIFormProperties,movable) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "bool canMinimize", offsetof(cgUIFormProperties,canMinimize) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "bool canMaximize", offsetof(cgUIFormProperties,canMaximize) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FormProperties", "bool canClose"   , offsetof(cgUIFormProperties,canClose) ) );
            
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::UI::Types
