#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgReferenceManager.h>
#include <System/cgMessageTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace References {

// Package declaration
namespace Messaging
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.References.Messaging" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value types
            BINDSUCCESS( engine->registerObjectType( "Message", sizeof(cgMessage), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "SystemMessages" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgSystemMessages (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Messages issued by cgRenderDriver
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "RenderDriver_DeviceLost", cgSystemMessages::RenderDriver_DeviceLost ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "RenderDriver_DeviceRestored", cgSystemMessages::RenderDriver_DeviceRestored ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "RenderDriver_ScreenLayoutChange", cgSystemMessages::RenderDriver_ScreenLayoutChange ) );

            // Messages processed by cgResourceManager
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "Resources_CollectGarbage", cgSystemMessages::Resources_CollectGarbage ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "Resources_ReloadShaders", cgSystemMessages::Resources_ReloadShaders ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "Resources_ReloadScripts", cgSystemMessages::Resources_ReloadScripts ) );

            // Messages issued by cgInputDriver
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_MouseMoved", cgSystemMessages::InputDriver_MouseMoved ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_MouseButtonDown", cgSystemMessages::InputDriver_MouseButtonDown ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_MouseButtonUp", cgSystemMessages::InputDriver_MouseButtonUp ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_MouseWheelScroll", cgSystemMessages::InputDriver_MouseWheelScroll ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_KeyDown", cgSystemMessages::InputDriver_KeyDown ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_KeyUp", cgSystemMessages::InputDriver_KeyUp ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "InputDriver_KeyPressed", cgSystemMessages::InputDriver_KeyPressed ) );

            // Messages issued by cgAudioDriver
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "AudioDriver_Apply3DSettings", cgSystemMessages::AudioDriver_Apply3DSettings ) );

            // Messages issued by user interface system
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnInitControl", cgSystemMessages::UI_OnInitControl ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnSize", cgSystemMessages::UI_OnSize ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnClosing", cgSystemMessages::UI_OnClosing ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnClose", cgSystemMessages::UI_OnClose ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnScreenLayoutChange", cgSystemMessages::UI_OnScreenLayoutChange ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnMouseButtonDown", cgSystemMessages::UI_OnMouseButtonDown ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnMouseButtonUp", cgSystemMessages::UI_OnMouseButtonUp ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnMouseMove", cgSystemMessages::UI_OnMouseMove ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnMouseWheelScroll", cgSystemMessages::UI_OnMouseWheelScroll ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnKeyDown", cgSystemMessages::UI_OnKeyDown ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnKeyUp", cgSystemMessages::UI_OnKeyUp ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnKeyPressed", cgSystemMessages::UI_OnKeyPressed ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnLostFocus", cgSystemMessages::UI_OnLostFocus ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_OnGainFocus", cgSystemMessages::UI_OnGainFocus ) );
            
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_Button_OnClick", cgSystemMessages::UI_Button_OnClick ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_ScrollBar_OnValueChange", cgSystemMessages::UI_ScrollBar_OnValueChange ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_ListBox_OnSelectedIndexChange", cgSystemMessages::UI_ListBox_OnSelectedIndexChange ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_ComboBox_OnSelectedIndexChange", cgSystemMessages::UI_ComboBox_OnSelectedIndexChange ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_CheckBox_OnClick", cgSystemMessages::UI_CheckBox_OnClick ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "UI_CheckBox_OnCheckedStateChange", cgSystemMessages::UI_CheckBox_OnCheckedStateChange ) );

            // Messages issued by cgSceneLoader
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "SceneLoader_ProgressUpdated", cgSystemMessages::SceneLoader_ProgressUpdated ) );

            // Messages issued by cgAppWindow
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "AppWindow_OnCreate", cgSystemMessages::AppWindow_OnCreate ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "AppWindow_OnClose", cgSystemMessages::AppWindow_OnClose ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "AppWindow_OnSize", cgSystemMessages::AppWindow_OnSize ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "AppWindow_OnUpdateCursor", cgSystemMessages::AppWindow_OnUpdateCursor ) );

            // Messages issued by cgWorld
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_Disposing", cgSystemMessages::World_Disposing ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_SceneAdded", cgSystemMessages::World_SceneAdded ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_SceneLoading", cgSystemMessages::World_SceneLoading ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_SceneLoadFailed", cgSystemMessages::World_SceneLoadFailed ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_SceneLoaded", cgSystemMessages::World_SceneLoaded ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "World_SceneUnloading", cgSystemMessages::World_SceneUnloading ) );

            // Messages issued by cgWorldComponent
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "WorldComponent_Created", cgSystemMessages::WorldComponent_Created ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "WorldComponent_Loading", cgSystemMessages::WorldComponent_Loading ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "WorldComponent_Deleted", cgSystemMessages::WorldComponent_Deleted ) );
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "WorldComponent_Modified", cgSystemMessages::WorldComponent_Modified ) );

            // Messages issued by cgWorldObject
            BINDSUCCESS( engine->registerEnumValue( "SystemMessages", "WorldObject_PropertyChange", cgSystemMessages::WorldObject_PropertyChange ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMessage (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgMessage>( engine, "Message" );
            
            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Message", "uint messageId", offsetof(cgMessage,messageId) ) );
            // Note: MessageContext & MessageData (as pointers) are not currently supported.
            BINDSUCCESS( engine->registerObjectProperty( "Message", "bool guaranteedData", offsetof(cgMessage,guaranteedData) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "uint dataSize", offsetof(cgMessage,dataSize) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "uint fromId", offsetof(cgMessage,fromId) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "uint toId", offsetof(cgMessage,toId) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "UID groupToId", offsetof(cgMessage,groupToId) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "double sendTime", offsetof(cgMessage,sendTime) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "double deliveryTime", offsetof(cgMessage,deliveryTime) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "bool delayed", offsetof(cgMessage,delayed) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Message", "bool sourceUnregistered", offsetof(cgMessage,sourceUnregistered) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgReferenceManager (Static Class as Global Functions)
            ///////////////////////////////////////////////////////////////////////

            // Static methods implemented as global functions
            BINDSUCCESS( engine->registerGlobalFunction("bool sendMessageTo( uint, uint, const Message &in, float )", asFUNCTION(cgReferenceManager::sendMessageTo), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool sendMessageToAll( uint, const Message &in, float )", asFUNCTION(cgReferenceManager::sendMessageToAll), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool sendMessageToGroup( uint, const UID &in, const Message &in, float )", asFUNCTION(cgReferenceManager::sendMessageToGroup), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool sendMessageToSubscribers( uint, const Message &in, float )", asFUNCTION(cgReferenceManager::sendMessageToSubscribers), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool subscribeToGroup( uint, const UID &in )", asFUNCTION(cgReferenceManager::subscribeToGroup), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool unsubscribeFromGroup( uint, const UID &in )", asFUNCTION(cgReferenceManager::unsubscribeFromGroup), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool subscribeToReference( uint, uint )", asFUNCTION(cgReferenceManager::subscribeToReference), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool unsubscribeFromReferemce( uint, uint )", asFUNCTION(cgReferenceManager::unsubscribeFromReference), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("bool processMessages( uint, bool, bool )", asFUNCTION(cgReferenceManager::processMessages), asCALL_CDECL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::References::Messaging