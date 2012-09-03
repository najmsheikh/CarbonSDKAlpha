#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUIControl.h>
#include <Interface/cgUIForm.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Controls {

// Package declaration
namespace UIControl
{
    //-------------------------------------------------------------------------
    // Name : registerControlMethods ()
    // Desc : Register the base cgUIControl class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerControlMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Form@+ getRootForm( ) const", asMETHODPR(type, getRootForm, () const, cgUIForm* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String& getName( ) const", asMETHODPR(type, getName, () const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setName( const String &in )", asMETHODPR(type, setName, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool addChildControl( UIControl@+ )", asMETHODPR(type, addChildControl, ( cgUIControl* ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect getControlArea( ) const", asMETHODPR(type, getControlArea, ( ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point getControlOrigin( ) const", asMETHODPR(type, getControlOrigin, ( ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect getClientArea( ) const", asMETHODPR(type, getClientArea, ( ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point getClientOrigin( ) const", asMETHODPR(type, getClientOrigin, ( ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect getControlArea( ControlCoordinateSpace ) const", asMETHODPR(type, getControlArea, ( cgControlCoordinateSpace::Base ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point getControlOrigin( ControlCoordinateSpace ) const", asMETHODPR(type, getControlOrigin, ( cgControlCoordinateSpace::Base ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect getClientArea( ControlCoordinateSpace ) const", asMETHODPR(type, getClientArea, ( cgControlCoordinateSpace::Base ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point getClientOrigin( ControlCoordinateSpace ) const", asMETHODPR(type, getClientOrigin, ( cgControlCoordinateSpace::Base ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point clientToScreen( const Point &in ) const", asMETHODPR(type, clientToScreen, ( const cgPoint& ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point controlToScreen( const Point &in ) const", asMETHODPR(type, controlToScreen, ( const cgPoint& ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point screenToClient( const Point &in ) const", asMETHODPR(type, screenToClient, ( const cgPoint& ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point screenToControl( const Point &in ) const", asMETHODPR(type, screenToControl, ( const cgPoint& ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool pointInControl( const Point &in ) const", asMETHODPR(type, pointInControl, ( const cgPoint& ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool loadControlText( InputStream )", asMETHODPR(type, loadControlText, ( cgInputStream ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String& getControlText( ) const", asMETHODPR(type, getControlText, ( ) const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& getSize( ) const", asMETHODPR(type, getSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Size getClientSize( ) const", asMETHODPR(type, getClientSize, ( ) const, cgSize ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& getMinimumSize( ) const", asMETHODPR(type, getMinimumSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& getMaximumSize( ) const", asMETHODPR(type, getMaximumSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Point& getPosition( ) const", asMETHODPR(type, getPosition, ( ) const, const cgPoint& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Rect& getPadding( ) const", asMETHODPR(type, getPadding, ( ) const, const cgRect& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isVisible( ) const", asMETHODPR(type, isVisible, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isVisible( bool ) const", asMETHODPR(type, isVisible, ( bool ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setVisible( bool )", asMETHODPR(type, setVisible, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isEnabled( ) const", asMETHODPR(type, isEnabled, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isEnabled( bool ) const", asMETHODPR(type, isEnabled, ( bool ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setEnabled( bool )", asMETHODPR(type, setEnabled, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "String getFont( ) const", asMETHODPR(type, getFont, ( ) const, cgString ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setControlText( const String&in )", asMETHODPR(type, setControlText, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setFont( const String&in )", asMETHODPR(type, setFont, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMinimumSize( const Size &in )", asMETHODPR(type, setMinimumSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMinimumSize( int, int )", asMETHODPR(type, setMinimumSize, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumSize( const Size &in )", asMETHODPR(type, setMaximumSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaximumSize( int, int )", asMETHODPR(type, setMaximumSize, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPadding( const Rect &in )", asMETHODPR(type, setPadding, ( const cgRect& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPadding( int, int, int, int )", asMETHODPR(type, setPadding, ( cgInt32, cgInt32, cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( const Size &in )", asMETHODPR(type, setSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSize( int, int )", asMETHODPR(type, setSize, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPosition( const Point &in )", asMETHODPR(type, setPosition, ( const cgPoint& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setPosition( int, int )", asMETHODPR(type, setPosition, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void move( int, int )", asMETHODPR(type, move, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void renderSecondary( )", asMETHODPR(type, renderSecondary, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setBackgroundOpacity( float )", asMETHODPR(type, setBackgroundOpacity, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getBackgroundOpacity( ) const", asMETHODPR(type, getBackgroundOpacity, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDockMode( DockMode )", asMETHODPR(type, setDockMode, ( cgDockMode::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "DockMode getDockMode( ) const", asMETHODPR(type, getDockMode, ( ) const, cgDockMode::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void focus( )", asMETHODPR(type, focus, ( ), void ), asCALL_THISCALL) );

        // Register object properties
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Form@+ get_rootForm( ) const", asMETHODPR(type, getRootForm, () const, cgUIForm* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String& get_name( ) const", asMETHODPR(type, getName, () const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_name( const String &in )", asMETHODPR(type, setName, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect get_controlArea( ) const", asMETHODPR(type, getControlArea, ( ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point get_controlOrigin( ) const", asMETHODPR(type, getControlOrigin, ( ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Rect get_clientArea( ) const", asMETHODPR(type, getClientArea, ( ) const, cgRect ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Point get_clientOrigin( ) const", asMETHODPR(type, getClientOrigin, ( ) const, cgPoint ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String& get_controlText( ) const", asMETHODPR(type, getControlText, ( ) const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& get_size( ) const", asMETHODPR(type, getSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Size get_clientSize( ) const", asMETHODPR(type, getClientSize, ( ) const, cgSize ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& get_minimumSize( ) const", asMETHODPR(type, getMinimumSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Size& get_maximumSize( ) const", asMETHODPR(type, getMaximumSize, ( ) const, const cgSize& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Point& get_position( ) const", asMETHODPR(type, getPosition, ( ) const, const cgPoint& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const Rect& get_padding( ) const", asMETHODPR(type, getPadding, ( ) const, const cgRect& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool get_visible( ) const", asMETHODPR(type, isVisible, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_visible( bool )", asMETHODPR(type, setVisible, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool get_enabled( ) const", asMETHODPR(type, isEnabled, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_enabled( bool )", asMETHODPR(type, setEnabled, ( bool ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "String get_font( ) const", asMETHODPR(type, getFont, ( ) const, cgString ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_controlText( const String&in )", asMETHODPR(type, setControlText, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_font( const String&in )", asMETHODPR(type, setFont, ( const cgString& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_minimumSize( const Size &in )", asMETHODPR(type, setMinimumSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_maximumSize( const Size &in )", asMETHODPR(type, setMaximumSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_padding( const Rect &in )", asMETHODPR(type, setPadding, ( const cgRect& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_size( const Size &in )", asMETHODPR(type, setSize, ( const cgSize& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_position( const Point &in )", asMETHODPR(type, setPosition, ( const cgPoint& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_backgroundOpacity( float )", asMETHODPR(type, setBackgroundOpacity, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float get_backgroundOpacity( ) const", asMETHODPR(type, getBackgroundOpacity, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_dockMode( DockMode )", asMETHODPR(type, setDockMode, ( cgDockMode::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "DockMode get_dockMode( ) const", asMETHODPR(type, getDockMode, ( ) const, cgDockMode::Base ), asCALL_THISCALL) );
        
        // Events
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void onInitControl( )", asMETHODPR(type, onInitControl, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseMove( const Point&, const PointF& )", asMETHODPR(type, onMouseMove, ( const cgPoint&, const cgPointF& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseButtonDown( int, const Point& )", asMETHODPR(type, onMouseButtonDown, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseButtonUp( int, const Point& )", asMETHODPR(type, onMouseButtonUp, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseWheelScroll( int, const Point& )", asMETHODPR(type, onMouseWheelScroll, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyDown( int, uint )", asMETHODPR(type, onKeyDown, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyUp( int, uint )", asMETHODPR(type, onKeyUp, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyPressed( int, uint )", asMETHODPR(type, onKeyPressed, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void onSize( int, int )", asMETHODPR(type, onSize, ( cgInt32, cgInt32 ), void ), asCALL_THISCALL) );

        // Message registration.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void registerEventHandler( uint, const String &in, IScriptedForm@+ )", asFUNCTIONPR(registerEventHandler, ( cgUInt32, const cgString&, asIScriptObject*, cgUIControl* ), void ), asCALL_CDECL_OBJLAST) );
        // ToDo: ??? void                raiseEvent          ( cgUInt32 nUIMessage, UIEventArgs * pData );
        
    } // End Method registerControlMethods<>

    //-----------------------------------------------------------------------------
    //  Name : registerEventHandler ()
    /// <summary>
    /// Translates the system supplied 'asIScriptObject' into an 'cgScriptObject'
    /// for use within the cgUIControl class.
    /// </summary>
    //-----------------------------------------------------------------------------
    void registerEventHandler( cgUInt32 messageId, const cgString & handler, asIScriptObject * nativeScriptObject, cgUIControl* thisPointer )
    {
        cgScriptObject * scriptObject = new cgScriptObject( thisPointer->getRootForm()->getFormScript(), nativeScriptObject );
        thisPointer->registerEventHandler( messageId, handler, scriptObject );
        scriptObject->release();
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls.UIControl" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "UIControl", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgUIControl (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUIControl>( engine );

            // Register base / system behaviors.
            registerControlMethods<cgUIControl>( engine, "UIControl" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Controls::UIControl
