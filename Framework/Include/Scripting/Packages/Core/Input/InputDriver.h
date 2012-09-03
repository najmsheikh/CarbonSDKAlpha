#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Input/cgInputDriver.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Input {

// Package declaration
namespace InputDriver
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Input.InputDriver" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "InputDriverConfig", sizeof(cgInputDriver::InitConfig), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
            BINDSUCCESS( engine->registerObjectType( "InputDriver", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgInputDriver::InitConfig (Struct)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "bool ignoreWindowsKey"  , offsetof(cgInputDriver::InitConfig,ignoreWindowsKey) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "float mouseSensitivity" , offsetof(cgInputDriver::InitConfig,mouseSensitivity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyRepeatDelay"    , offsetof(cgInputDriver::InitConfig,keyRepeatDelay) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyRepeatRate"     , offsetof(cgInputDriver::InitConfig,keyRepeatRate) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "uint keyboardBufferSize", offsetof(cgInputDriver::InitConfig,keyboardBufferSize) ) );
            BINDSUCCESS( engine->registerObjectProperty( "InputDriverConfig", "bool hideSystemCursor"  , offsetof(cgInputDriver::InitConfig,hideSystemCursor) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgInputDriver (Class)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgInputDriver>( engine );
            
            // Register base class object methods
            Core::System::References::Reference::registerReferenceMethods<cgInputDriver>( engine, "InputDriver" );
            
            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "InputDriverConfig getConfig() const", asMETHODPR(cgInputDriver, getConfig, () const, cgInputDriver::InitConfig), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "void poll()", asMETHODPR(cgInputDriver, poll, (), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "bool wasKeyPressed( int ) const", asMETHODPR(cgInputDriver, wasKeyPressed, ( cgInt32 ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "bool isKeyPressed( int ) const", asMETHODPR(cgInputDriver, isKeyPressed, ( cgInt32 ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "bool wasMouseButtonPressed( MouseButtons ) const", asMETHODPR(cgInputDriver, wasMouseButtonPressed, ( cgMouseButtons::Base ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "bool isMouseButtonPressed( MouseButtons ) const", asMETHODPR(cgInputDriver, isMouseButtonPressed, ( cgMouseButtons::Base ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "const Point & getCursorPosition( ) const", asMETHODPR(cgInputDriver, getCursorPosition, ( ) const, const cgPoint&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "void setMouseMode( MouseHandlerMode )", asMETHODPR(cgInputDriver, setMouseMode, ( cgMouseHandlerMode::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "MouseHandlerMode getMouseMode( ) const", asMETHODPR(cgInputDriver, getMouseMode, ( ) const, cgMouseHandlerMode::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "bool isInitialized( ) const", asMETHODPR(cgInputDriver, isInitialized, ( ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "InputDriver", "String keyCodeToCharacter( int )", asMETHODPR(cgInputDriver, keyCodeToCharacter, ( cgInt32 ), cgString), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "InputDriver@+ getAppInputDriver( )", asFUNCTIONPR(cgInputDriver::getInstance, ( ), cgInputDriver*), asCALL_CDECL) );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Input::InputDriver