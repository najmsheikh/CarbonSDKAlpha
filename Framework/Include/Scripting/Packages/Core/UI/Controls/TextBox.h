#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgTextBoxControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace TextBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.TextBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "TextBox", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgTextBoxControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgTextBoxControl>( engine, "TextBox" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void setSelectionRange( int, int )", asMETHODPR(cgTextBoxControl, setSelectionRange, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "String getSelectedText( ) const", asMETHODPR(cgTextBoxControl, getSelectedText, ( ) const, cgString ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void setMultiline( bool )", asMETHODPR(cgTextBoxControl, setMultiline, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool getMultiline( ) const", asMETHODPR(cgTextBoxControl, getMultiline, ( ) const, bool ), asCALL_THISCALL) );            
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void setReadOnly( bool )", asMETHODPR(cgTextBoxControl, setReadOnly, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool getReadOnly( ) const", asMETHODPR(cgTextBoxControl, getReadOnly, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void setAllowFormatCode( bool )", asMETHODPR(cgTextBoxControl, setAllowFormatCode, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool getAllowFormatCode( ) const", asMETHODPR(cgTextBoxControl, getAllowFormatCode, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void insertString( int, const String&in )", asMETHODPR(cgTextBoxControl, insertString, ( cgInt32, const cgString&),void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void scrollToCaret( )", asMETHODPR(cgTextBoxControl, scrollToCaret, ( ),void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void scrollToEnd( )", asMETHODPR(cgTextBoxControl, scrollToEnd, ( ),void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void scrollToStart( )", asMETHODPR(cgTextBoxControl, scrollToStart, ( ),void ), asCALL_THISCALL) );
            
            // Register object property methods
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void set_multiline( bool )", asMETHODPR(cgTextBoxControl, setMultiline, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool get_multiline( ) const", asMETHODPR(cgTextBoxControl, getMultiline, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void set_readOnly( bool )", asMETHODPR(cgTextBoxControl, setReadOnly, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool get_readOnly( ) const", asMETHODPR(cgTextBoxControl, getReadOnly, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "void set_allowFormatCode( bool )", asMETHODPR(cgTextBoxControl, setAllowFormatCode, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "TextBox", "bool get_allowFormatCode( ) const", asMETHODPR(cgTextBoxControl, getAllowFormatCode, ( ) const, bool ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "TextBox@+ createTextBox( const String&in )", asFUNCTIONPR(createTextBox, (const cgString&), cgTextBoxControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "TextBox@+ createTextBox( const String&in, UIControl @+ )", asFUNCTIONPR(createTextBox, (const cgString&, cgUIControl*), cgTextBoxControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createTextBox() (Static)
        /// <summary>
        /// Static method which simply creates a text box control. This is mostly to
        /// provide scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgTextBoxControl * createTextBox( const cgString & controlName )
        {
            cgTextBoxControl * textBox = new cgTextBoxControl();
            
            // Set the control details.
            textBox->setName( controlName );

            // Return the new control
            return textBox;
        }

        //-----------------------------------------------------------------------------
        // Name : createTextBox() (Static)
        /// <summary>
        /// Static method which creates a text box control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgTextBoxControl * createTextBox( const cgString & controlName, cgUIControl * parentControl )
        {
            cgTextBoxControl * textBox = new cgTextBoxControl();

            // Set the control details.
            textBox->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( textBox );

            // Return the new control
            return textBox;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::TextBox
