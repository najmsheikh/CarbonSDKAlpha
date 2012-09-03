#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgGroupBoxControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace GroupBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.GroupBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "GroupBox", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgGroupBoxControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgGroupBoxControl>( engine, "GroupBox" );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "GroupBox@+ createGroupBox( const String&in )", asFUNCTIONPR(createGroupBox, (const cgString&), cgGroupBoxControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "GroupBox@+ createGroupBox( const String&in, UIControl @+ )", asFUNCTIONPR(createGroupBox, (const cgString&, cgUIControl*), cgGroupBoxControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createGroupBox() (Static)
        /// <summary>
        /// Static method which simply creates a button control. This is mostly to
        /// provide scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgGroupBoxControl * createGroupBox( const cgString & controlName )
        {
            cgGroupBoxControl * groupBox = new cgGroupBoxControl();
            
            // Set the control details.
            groupBox->setName( controlName );

            // Return the new control
            return groupBox;
        }

        //-----------------------------------------------------------------------------
        // Name : createGroupBox() (Static)
        /// <summary>
        /// Static method which creates a group box control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgGroupBoxControl * createGroupBox( const cgString & controlName, cgUIControl * parentControl )
        {
            cgGroupBoxControl * groupBox = new cgGroupBoxControl();

            // Set the control details.
            groupBox->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( groupBox );

            // Return the new control
            return groupBox;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::GroupBox
