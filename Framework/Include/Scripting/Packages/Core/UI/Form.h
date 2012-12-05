#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUIForm.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI {

// Package declaration
namespace Form
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Form" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Object Types
            BINDSUCCESS( engine->registerObjectType( "Form", 0, asOBJ_REF ) );

            // Interface Types
            BINDSUCCESS( engine->registerInterface("IScriptedForm" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgUIForm (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUIForm>( engine );

            // Register base / system behaviors.
            Core::UI::Controls::UIControl::registerControlMethods<cgUIForm>( engine, "Form" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Form", "bool loadForm( InputStream, const String&in )", asMETHODPR(cgUIForm, loadForm, ( cgInputStream, const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "bool createForm( const String&in )", asMETHODPR(cgUIForm, createForm, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "UIManager@+ getUIManager( ) const", asMETHODPR(cgUIForm,getUIManager,() const,cgUIManager*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "ControlLayer@+ getControlLayer( ) const", asMETHODPR(cgUIForm,getControlLayer,() const,cgUIControlLayer*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "void setAcceptButton( Button@+ )", asMETHODPR(cgUIForm,setAcceptButton,( cgButtonControl* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "Button@+ getAcceptButton( ) const", asMETHODPR(cgUIForm,getAcceptButton,( ) const,cgButtonControl* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "void setCancelButton( Button@+ )", asMETHODPR(cgUIForm,setCancelButton,( cgButtonControl* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "Button@+ getCancelButton( ) const", asMETHODPR(cgUIForm,getCancelButton,( ) const,cgButtonControl* ), asCALL_THISCALL) );
            // ToDo: cgScript                  * GetFormScript           ( ) const { return m_pFormScript; }
            BINDSUCCESS( engine->registerObjectMethod( "Form", "IScriptedForm @ getScriptObject( )", asFUNCTIONPR(getScriptObject,( cgUIForm* ), asIScriptObject*), asCALL_CDECL_OBJLAST ) );

            // Register the property methods
            BINDSUCCESS( engine->registerObjectMethod( "Form", "void set_acceptButton( Button@+ )", asMETHODPR(cgUIForm,setAcceptButton,( cgButtonControl* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "Button@+ get_acceptButton( ) const", asMETHODPR(cgUIForm,getAcceptButton,( ) const,cgButtonControl* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "void set_cancelButton( Button@+ )", asMETHODPR(cgUIForm,setCancelButton,( cgButtonControl* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "Button@+ get_cancelButton( ) const", asMETHODPR(cgUIForm,getCancelButton,( ) const,cgButtonControl* ), asCALL_THISCALL) );
            
            // Custom events
            BINDSUCCESS( engine->registerObjectMethod( "Form", "bool onPreCreateForm( FormProperties &inout )", asMETHODPR(cgUIForm,onPreCreateForm,(cgUIFormProperties&),bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Form", "bool onCreateForm( )", asMETHODPR(cgUIForm,onCreateForm,(),bool), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // IScriptedForm (Interface)
            ///////////////////////////////////////////////////////////////////////

            // Register the /required/ interface methods
            // ToDo: Are any actually required?

        }

        //-------------------------------------------------------------------------
        // Name : getScriptObject ()
        // Desc : Wrapper function that returns a reference to the script based
        //        'IScriptedForm' interface rather than the C++ side 
        //        'cgScriptObject' that is returned by the 'cgUIForm' 
        //        native method of the same name.
        //-------------------------------------------------------------------------
        static asIScriptObject * getScriptObject( cgUIForm * thisPointer )
        {
            cgScriptObject * object = thisPointer->getScriptObject();
            if ( object && object->getInternalObject() )
            {
                object->getInternalObject()->AddRef();
                return object->getInternalObject();
            
            } // End if valid
            return CG_NULL;
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::UI::Form
