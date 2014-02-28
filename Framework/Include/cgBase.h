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
// Name : cgBase.h                                                           //
//                                                                           //
// Desc : Base / common header file for the CGE library. Defines             //
//        common types and housekeeping routines (such as initialization /   //
//        destruction)                                                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBASE_H_ )
#define _CGE_CGBASE_H_

//-----------------------------------------------------------------------------
// Base Header Includes
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#include <cgBaseTypes.h>
#include <System/cgVariant.h>
#include <System/cgAppLog.h>
#include <System/cgFileSystem.h>

//-----------------------------------------------------------------------------
// Platform Compile Directives
//-----------------------------------------------------------------------------
#if !defined( _WIN32_WINNT )
#define _WIN32_WINNT 0x500
#endif

//-----------------------------------------------------------------------------
// Platform Configuration
//-----------------------------------------------------------------------------
namespace cgSandboxMode
{
    enum Base
    {
        Disabled,
        Preview,
        Enabled
    };

} // End Namespace : cgSandboxMode

namespace cgPlatform
{
    enum Base
    {
        Windows,
        X360,           // Not supported in this implementation.
        PS3             // Not supported in this implementation.
    };

}; // End Namespace : cgPlatform

namespace cgRenderAPI
{
    enum Base
    {
        Null,

#if defined(CGE_DX9_RENDER_SUPPORT)
        DirectX9,
#endif // CGE_DX9_RENDER_SUPPORT

#if defined(CGE_DX11_RENDER_SUPPORT )
        DirectX11
#endif // CGE_DX11_RENDER_SUPPORT

    };

}; // End Namspace : cgRenderAPI

namespace cgAudioAPI
{
    enum Base
    {
        DirectX
    };

}; // End Namespace : cgAudioAPI

namespace cgInputAPI
{
    enum Base
    {
        DirectX
    };

}; // End Namespace : cgInputAPI

namespace cgNetworkAPI
{
    enum Base
    {
        Winsock
    };

}; // End Namspace : cgNetworkAPI

struct CGEConfig
{
    cgPlatform::Base    platform;       // The selected system platform (environment, file system, etc.)
    cgRenderAPI::Base   renderAPI;      // Rendering API to be utilized.
    cgAudioAPI::Base    audioAPI;       // Audio API to be utilized.
    cgInputAPI::Base    inputAPI;       // Input API to be utilized.
    cgNetworkAPI::Base  networkAPI;     // Networking API to be utilized.
    bool                highPriority;   // Application process should run with high priority.
    bool                multiThreaded;  // Will the application require multi-threaded access to drivers such as the render driver?
    cgSandboxMode::Base sandboxMode;    // The engine is running in sandbox mode and provides editor-like functionality?

    // Constructor
    CGE_API CGEConfig() :
#       if defined(CGE_DX9_RENDER_SUPPORT)
            renderAPI( cgRenderAPI::DirectX9 ),
#       elif defined(CGE_DX11_RENDER_SUPPORT)
            renderAPI( cgRenderAPI::DirectX11 ),
#       else
            renderAPI( cgRenderAPI::Null ),
#       endif
        platform( cgPlatform::Windows ), audioAPI( cgAudioAPI::DirectX ), 
        inputAPI( cgInputAPI::DirectX ), networkAPI( cgNetworkAPI::Winsock ),
        highPriority( true ), multiThreaded(false), sandboxMode(cgSandboxMode::Disabled) {}

}; // End struct CGEConfig

// Contains the information relating to a sandbox mode change.
struct cgSandboxModeChangeEventArgs
{
    cgSandboxMode::Base oldMode;
    cgSandboxMode::Base newMode;
};

//-----------------------------------------------------------------------------
// Global Function Declarations
//-----------------------------------------------------------------------------
// cgBase.cpp
bool                CGE_API   cgEngineInit         ( const CGEConfig & config, cgLogOutput * logOutput = CG_NULL );
void                CGE_API   cgEngineCleanup      ( );
void                CGE_API   cgEngineYield        ( );
void                CGE_API   cgFPUDoublePrecision ( );
void                CGE_API   cgFPURestorePrecision( );
bool                CGE_API   cgSetSandboxMode     ( cgSandboxMode::Base mode );
cgSandboxMode::Base CGE_API   cgGetSandboxMode     ( );
const CGEConfig     CGE_API & cgGetEngineConfig    ( );

#endif // !_CGE_CGBASE_H_