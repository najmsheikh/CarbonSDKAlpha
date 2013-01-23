#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/Controls/cgButtonControl.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace Button
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.Button" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Button", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgButtonControl>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgButtonControl>( engine, "Button" );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod( "Button", "void onClick( )", asMETHODPR(cgButtonControl, onClick, ( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Button", "bool setImage( const String &in )", asMETHODPR(cgButtonControl, setImage, ( const cgString& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Button", "const String & getImage( ) const", asMETHODPR(cgButtonControl, getImage, ( ) const, const cgString& ), asCALL_THISCALL) );

            // Register object property methods
            BINDSUCCESS( engine->registerObjectMethod( "Button", "bool set_image( const String &in )", asMETHODPR(cgButtonControl, setImage, ( const cgString& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Button", "const String & get_image( ) const", asMETHODPR(cgButtonControl, getImage, ( ) const, const cgString& ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalFunction( "Button@+ createButton( const String&in )", asFUNCTIONPR(createButton, (const cgString&), cgButtonControl*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Button@+ createButton( const String&in, UIControl @+ )", asFUNCTIONPR(createButton, (const cgString&, cgUIControl*), cgButtonControl*), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        // Name : createButton() (Static)
        /// <summary>
        /// Static method which simply allocates a button. This is mostly to provide 
        /// scripts more consistent creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgButtonControl * createButton( const cgString & controlName )
        {
            cgButtonControl * button = new cgButtonControl();
            
            // Set the control details.
            button->setName( controlName );

            // Return the new control
            return button;
        }

        //-----------------------------------------------------------------------------
        // Name : createButton() (Static)
        /// <summary>
        /// Static method which creates a button control and optionally attaches it
        /// to its parent control. This is mostly to provide scripts more consistent 
        /// creation of controls.
        /// </summary>
        //-----------------------------------------------------------------------------
        static cgButtonControl * createButton( const cgString & controlName, cgUIControl * parentControl )
        {
            cgButtonControl * button = new cgButtonControl();

            // Set the control details.
            button->setName( controlName );

            // Attach to parent
            if ( parentControl )
                parentControl->addChildControl( button );

            // Return the new control
            return button;
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::Button
