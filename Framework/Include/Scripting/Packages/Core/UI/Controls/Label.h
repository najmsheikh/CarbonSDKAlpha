#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgLabelControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace Label
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.Label" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Label", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgLabelControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgLabelControl>( engine, "Label" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setMultiline( bool )", asMETHODPR(cgLabelControl, setMultiline, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "bool getMultiline( ) const", asMETHODPR(cgLabelControl, getMultiline, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setTruncationMode( TextTruncationMode )", asMETHODPR(cgLabelControl, setTruncationMode, ( cgTextTruncationMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "TextTruncationMode getTruncationMode( ) const", asMETHODPR(cgLabelControl, getTruncationMode, ( ) const, cgTextTruncationMode::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setHorizontalAlignment( HorizontalAlignment )", asMETHODPR(cgLabelControl, setHorizontalAlignment, ( cgHorizontalAlignment::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "HorizontalAlignment getHorizontalAlignment( ) const", asMETHODPR(cgLabelControl, getHorizontalAlignment, ( ) const, cgHorizontalAlignment::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setVerticalAlignment( VerticalAlignment )", asMETHODPR(cgLabelControl, setVerticalAlignment, ( cgVerticalAlignment::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "VerticalAlignment getVerticalAlignment( ) const", asMETHODPR(cgLabelControl, getVerticalAlignment, ( ) const, cgVerticalAlignment::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setTextColor( const ColorValue &in )", asMETHODPR(cgLabelControl, setTextColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "const ColorValue& getTextColor( ) const", asMETHODPR(cgLabelControl, getTextColor, ( ) const, const cgColorValue & ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void setAllowFormatCode( bool )", asMETHODPR(cgLabelControl, setAllowFormatCode, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "bool getAllowFormatCode( ) const", asMETHODPR(cgLabelControl, getAllowFormatCode, ( ) const, bool ), asCALL_THISCALL) );

            // Register object property methods
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_multiline( bool )", asMETHODPR(cgLabelControl, setMultiline, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "bool get_multiline( ) const", asMETHODPR(cgLabelControl, getMultiline, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_truncationMethod( TextTruncationMode )", asMETHODPR(cgLabelControl, setTruncationMode, ( cgTextTruncationMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "TextTruncationMode get_truncationMethod( ) const", asMETHODPR(cgLabelControl, getTruncationMode, ( ) const, cgTextTruncationMode::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_horizontalAlignment( HorizontalAlignment )", asMETHODPR(cgLabelControl, setHorizontalAlignment, ( cgHorizontalAlignment::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "HorizontalAlignment get_horizontalAlignment( ) const", asMETHODPR(cgLabelControl, getHorizontalAlignment, ( ) const, cgHorizontalAlignment::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_verticalAlignment( VerticalAlignment )", asMETHODPR(cgLabelControl, setVerticalAlignment, ( cgVerticalAlignment::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "VerticalAlignment get_verticalAlignment( ) const", asMETHODPR(cgLabelControl, getVerticalAlignment, ( ) const, cgVerticalAlignment::Base ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_textColor( const ColorValue &in )", asMETHODPR(cgLabelControl, setTextColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "const ColorValue& get_textColor( ) const", asMETHODPR(cgLabelControl, getTextColor, ( ) const, const cgColorValue & ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "void set_allowFormatCode( bool )", asMETHODPR(cgLabelControl, setAllowFormatCode, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Label", "bool get_allowFormatCode( ) const", asMETHODPR(cgLabelControl, getAllowFormatCode, ( ) const, bool ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "Label@+ createLabel( const String&in )", asFUNCTIONPR(createLabel, (const cgString&), cgLabelControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Label@+ createLabel( const String&in, UIControl @+ )", asFUNCTIONPR(createLabel, (const cgString&, cgUIControl*), cgLabelControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createLabel() (Static)
        /// <summary>
        /// Static method which simply creates a label control. This is mostly to
        /// provide scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgLabelControl * createLabel( const cgString & controlName )
        {
            cgLabelControl * label = new cgLabelControl();
            
            // Set the control details.
            label->setName( controlName );

            // Return the new control
            return label;
        }

        //-----------------------------------------------------------------------------
        // Name : createLabel() (Static)
        /// <summary>
        /// Static method which creates a label control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgLabelControl * createLabel( const cgString & controlName, cgUIControl * parentControl )
        {
            cgLabelControl * label = new cgLabelControl();

            // Set the control details.
            label->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( label );

            // Return the new control
            return label;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::Label
