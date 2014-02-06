#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgComboBoxControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace ComboBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.ComboBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ComboBox", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgComboBoxControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgComboBoxControl>( engine, "ComboBox" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "void onSelectedIndexChange( int, int )", asMETHODPR(cgComboBoxControl, onSelectedIndexChange, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "int addItem( const String &in )", asMETHODPR(cgComboBoxControl, addItem, ( const cgString& ), cgInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "void clear()", asMETHODPR(cgComboBoxControl, clear, ( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "void setSelectedIndex( int )", asMETHODPR(cgComboBoxControl, setSelectedIndex, ( cgInt32 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "int getSelectedIndex( ) const", asMETHODPR(cgComboBoxControl, getSelectedIndex, ( ) const, cgInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "void showDropDown( bool )", asMETHODPR(cgComboBoxControl, showDropDown, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "bool isDropDownVisible( ) const", asMETHODPR(cgComboBoxControl, isDropDownVisible, ( ) const, bool ), asCALL_THISCALL) );
            
            // Register object property methods
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "void set_selectedIndex( int )", asMETHODPR(cgComboBoxControl, setSelectedIndex, ( cgInt32 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ComboBox", "int get_selectedIndex( ) const", asMETHODPR(cgComboBoxControl, getSelectedIndex, ( ) const, cgInt32 ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "ComboBox@+ createComboBox( const String&in )", asFUNCTIONPR(createComboBox, (const cgString&), cgComboBoxControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "ComboBox@+ createComboBox( const String&in, UIControl @+ )", asFUNCTIONPR(createComboBox, (const cgString&, cgUIControl*), cgComboBoxControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createComboBoxl() (Static)
        /// <summary>
        /// Static method which simply creates a combo box control. This is mostly to
        /// provide scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgComboBoxControl * createComboBox( const cgString & controlName )
        {
            cgComboBoxControl * listBox = new cgComboBoxControl();
            
            // Set the control details.
            listBox->setName( controlName );

            // Return the new control
            return listBox;
        }

        //-----------------------------------------------------------------------------
        // Name : createComboBox() (Static)
        /// <summary>
        /// Static method which creates a combo box control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgComboBoxControl * createComboBox( const cgString & controlName, cgUIControl * parentControl )
        {
            cgComboBoxControl * listBox = new cgComboBoxControl();

            // Set the control details.
            listBox->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( listBox );

            // Return the new control
            return listBox;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::ComboBox