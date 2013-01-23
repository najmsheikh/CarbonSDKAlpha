#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgCheckBoxControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace CheckBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.CheckBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "CheckBox", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgCheckBoxControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgCheckBoxControl>( engine, "CheckBox" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "CheckBox", "void onClick( )", asMETHODPR(cgCheckBoxControl, onClick, ( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CheckBox", "void setChecked( bool )", asMETHODPR(cgCheckBoxControl, setChecked, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CheckBox", "bool isChecked( ) const", asMETHODPR(cgCheckBoxControl, isChecked, ( ) const, bool ), asCALL_THISCALL) );

            // Register object property methods
            BINDSUCCESS( engine->registerObjectMethod( "CheckBox", "void set_checked( bool )", asMETHODPR(cgCheckBoxControl, setChecked, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "CheckBox", "bool get_checked( ) const", asMETHODPR(cgCheckBoxControl, isChecked, ( ) const, bool ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "CheckBox@+ createCheckBox( const String&in )", asFUNCTIONPR(createCheckBox, (const cgString&), cgCheckBoxControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "CheckBox@+ createCheckBox( const String&in, UIControl @+ )", asFUNCTIONPR(createCheckBox, (const cgString&, cgUIControl*), cgCheckBoxControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createCheckBox() (Static)
        /// <summary>
        /// Static method which simply allocates a check box. This is mostly to provide 
        /// scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgCheckBoxControl * createCheckBox( const cgString & controlName )
        {
            cgCheckBoxControl * checkBox = new cgCheckBoxControl();
            
            // Set the control details.
            checkBox->setName( controlName );

            // Return the new control
            return checkBox;
        }

        //-----------------------------------------------------------------------------
        // Name : createCheckBox() (Static)
        /// <summary>
        /// Static method which creates a check box control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgCheckBoxControl * createCheckBox( const cgString & controlName, cgUIControl * parentControl )
        {
            cgCheckBoxControl * checkBox = new cgCheckBoxControl();

            // Set the control details.
            checkBox->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( checkBox );

            // Return the new control
            return checkBox;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::CheckBox
