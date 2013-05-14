//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : cgMessageTypes.h                                                   //
//                                                                           //
// Desc : Defines all of the standard engine message identifiers for use by  //
//        both the host application and the engine itself. Pay close         //
//        attention to the identifiers which you select in your host         //
//        application to ensure that no collision occurs.                    //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMESSAGETYPES_H_ )
#define _CGE_CGMESSAGETYPES_H_

//********************************* NOTE *************************************
// Values between 1 and 99,999 are reserved for use by the engine
// subsystems. You should only use values above this threshold for application
// defined message identifiers.
//********************************* NOTE *************************************
namespace cgSystemMessages
{
    enum
    {
        // Messages issued by cgRenderDriver
        RenderDriver_DeviceLost         = 1,
        RenderDriver_DeviceRestored     = 2,
        RenderDriver_ScreenLayoutChange = 3,

        // Messages processed by cgResourceManager
        Resources_CollectGarbage        = 1001,
        Resources_ReloadShaders         = 1002,
        Resources_ReloadScripts         = 1003,
        Resources_ResourceAdded         = 1004,
        Resources_ResourceRemoved       = 1005,

        // Messages issued by cgInputDriver
        InputDriver_MouseMoved          = 2001,
        InputDriver_MouseButtonDown     = 2002,
        InputDriver_MouseButtonUp       = 2003,
        InputDriver_MouseWheelScroll    = 2004,
        InputDriver_KeyDown             = 2010,
        InputDriver_KeyUp               = 2011,
        InputDriver_KeyPressed          = 2012,

        // Messages issued by cgAudioDriver
        AudioDriver_Apply3DSettings     = 3001,

        // Messages issued by user interface system
        UI_CollectGarbage               = 4000,
        UI_OnInitControl                = 4001,
        UI_OnSize                       = 4002,
        UI_OnClosing                    = 4003,
        UI_OnClose                      = 4004,
        UI_OnScreenLayoutChange         = 4005,
        UI_OnMouseButtonDown            = 4006,
        UI_OnMouseButtonUp              = 4007,
        UI_OnMouseMove                  = 4008,
        UI_OnMouseWheelScroll           = 4009,
        UI_OnKeyDown                    = 4010,
        UI_OnKeyUp                      = 4011,
        UI_OnKeyPressed                 = 4012,
        UI_OnLostFocus                  = 4013,
        UI_OnGainFocus                  = 4014,
        
        UI_Button_OnClick                   = 4100,
        UI_ScrollBar_OnValueChange          = 4120,
        UI_ListBox_OnSelectedIndexChange    = 4140,
        UI_ComboBox_OnSelectedIndexChange   = 4160,
        UI_CheckBox_OnClick                 = 4180,
        UI_CheckBox_OnCheckedStateChange    = 4181,

        // Messages issued by cgSceneLoader
        SceneLoader_ProgressUpdated     = 5001,

        // Messages issued by cgAppWindow
        AppWindow_OnCreate              = 6001,
        AppWindow_OnClose               = 6002,
        AppWindow_OnSize                = 6003,
        AppWindow_OnUpdateCursor        = 6004,

        // Messages issued by cgWorld
        World_Disposing                 = 7001,
        World_SceneAdded                = 7002,
        World_SceneLoading              = 7003,
        World_SceneLoadFailed           = 7004,
        World_SceneLoaded               = 7005,
        World_SceneUnloading            = 7006,

        // Messages issued by cgWorldComponent
        WorldComponent_Created          = 8001,
        WorldComponent_Loading          = 8002,
        WorldComponent_Deleted          = 8003,
        WorldComponent_Modified         = 8004,

        // Messages issued by cgWorldObject
        WorldObject_PropertyChange      = 9001,

    };

} // cgSystemMessages

namespace cgSystemMessageGroups
{
    // {D2C75758-EBBF-45E4-B463-E0B5854D1F1F}
    const cgUID MGID_RenderDriver    = { 0xD2C75758, 0xEBBF, 0x45E4, { 0xB4, 0x63, 0xE0, 0xB5, 0x85, 0x4D, 0x1F, 0x1F } };
    // {379C5DE3-9026-4A95-9797-AE6C964D4349}
    const cgUID MGID_ResourceManager = { 0x379C5DE3, 0x9026, 0x4A95, { 0x97, 0x97, 0xAE, 0x6C, 0x96, 0x4D, 0x43, 0x49 } };
    // {8C9BC07F-6349-43f2-AF0C-13C0AE315222}
    const cgUID MGID_MouseInput      = { 0x8c9bc07f, 0x6349, 0x43f2, { 0xaf, 0x0c, 0x13, 0xc0, 0xae, 0x31, 0x52, 0x22 } };
    // {7026E4D4-2946-44fe-B4DD-41DCD785F6C2}
    const cgUID MGID_KeyboardInput   = { 0x7026e4d4, 0x2946, 0x44fe, { 0xb4, 0xdd, 0x41, 0xdc, 0xd7, 0x85, 0xf6, 0xc2 } };
    // {8D14BD26-A712-4086-A1C5-36EA9C9B679B}
    const cgUID MGID_SceneLoader     = { 0x8d14bd26, 0xa712, 0x4086, { 0xa1, 0xc5, 0x36, 0xea, 0x9c, 0x9b, 0x67, 0x9b } };
    // {089FB0DF-6862-4E0D-96D8-2018DDD546A7}
    const cgUID MGID_AppWindow       = { 0x089FB0DF, 0x6862, 0x4E0D, { 0x96, 0xD8, 0x20, 0x18, 0xDD, 0xD5, 0x46, 0xA7 } };
    // {E47EB347-E611-4411-AD1C-4AA9E1294CDD}
    const cgUID MGID_World           = { 0xE47EB347, 0xE611, 0x4411, { 0xAD, 0x1C, 0x4A, 0xA9, 0xE1, 0x29, 0x4C, 0xDD } };
    // {11340C0F-D5B3-4922-A873-FB46CE1A3A54}
    const cgUID MGID_WorldComponent  = { 0x11340C0F, 0xD5B3, 0x4922, { 0xA8, 0x73, 0xFB, 0x46, 0xCE, 0x1A, 0x3A, 0x54 } };
    
}; // cgSystemMessageGroups

#endif // !_CGE_CGMESSAGETYPES_H_