#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgListBoxControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace ListBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.ListBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ListBox", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgListBoxControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgListBoxControl>( engine, "ListBox" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "ListBox", "int addItem( const String &in )", asMETHODPR(cgListBoxControl, addItem, ( const cgString& ), cgInt32 ), asCALL_THISCALL) );

            // Register object property methods

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "ListBox@+ createListBox( const String&in )", asFUNCTIONPR(createListBox, (const cgString&), cgListBoxControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "ListBox@+ createListBox( const String&in, UIControl @+ )", asFUNCTIONPR(createListBox, (const cgString&, cgUIControl*), cgListBoxControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createListBoxl() (Static)
        /// <summary>
        /// Static method which simply creates a list box control. This is mostly to
        /// provide scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgListBoxControl * createListBox( const cgString & controlName )
        {
            cgListBoxControl * listBox = new cgListBoxControl();
            
            // Set the control details.
            listBox->setName( controlName );

            // Return the new control
            return listBox;
        }

        //-----------------------------------------------------------------------------
        // Name : createListBox() (Static)
        /// <summary>
        /// Static method which creates a list box control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgListBoxControl * createListBox( const cgString & controlName, cgUIControl * parentControl )
        {
            cgListBoxControl * listBox = new cgListBoxControl();

            // Set the control details.
            listBox->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( listBox );

            // Return the new control
            return listBox;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::ListBox