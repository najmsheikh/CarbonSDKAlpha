#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Lighting/cgLightingManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Lighting {

// Package declaration
namespace LightingManager
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Lighting.LightingManager" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "LightingManager", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgLightingManager>( engine );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "TexturePool@+ getShadowMaps( ) const", asMETHODPR(cgLightingManager,getShadowMaps,( ) const, cgTexturePool*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void processShadowMaps( CameraNode@+, bool, const String &in )", asMETHODPR(cgLightingManager,processShadowMaps,( cgCameraNode*, bool, const cgString&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void processLights( CameraNode@+, bool, bool, bool, const String &in, const String &in )", asMETHODPR(cgLightingManager,processLights,( cgCameraNode*, bool, bool, bool, const cgString&, const cgString&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void processLights( const RenderTargetHandle &in, CameraNode@+, bool, bool, bool, const String &in, const String &in )", asMETHODPR(cgLightingManager,processLights,( const cgRenderTargetHandle&, cgCameraNode*, bool, bool, bool, const cgString&, const cgString&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void processIndirectLights( const RenderTargetHandle &in, const RenderTargetHandle &in, const RenderTargetHandle &in, const DepthStencilTargetHandle &in, const DepthStencilTargetHandle &in, CameraNode@+, const String &in, const String &in )", asMETHODPR(cgLightingManager,processIndirectLights,( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgDepthStencilTargetHandle&, const cgDepthStencilTargetHandle&, cgCameraNode*, const cgString&, const cgString&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "bool addRadianceGrid( float, const Vector3 &in, uint, uint, float, float )", asMETHODPR(cgLightingManager,addRadianceGrid,( cgFloat, const cgVector3&, cgUInt32, cgUInt32, cgFloat, cgFloat ), bool), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "bool beginShadowConfigure( uint, uint, uint )", asMETHODPR(cgLightingManager,beginShadowConfigure,( cgUInt32, cgUInt32, cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void addDefaultMaps( uint )", asMETHODPR(cgLightingManager,addDefaultMaps,( cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void addCacheMaps( uint, uint, uint )", asMETHODPR(cgLightingManager,addCachedMaps,( cgUInt32, cgUInt32, cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "void addCacheMaps( BufferFormat, uint, uint )", asMETHODPR(cgLightingManager,addCachedMaps,( cgBufferFormat::Base, cgUInt32, cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "LightingManager", "bool endShadowConfigure( )", asMETHODPR(cgLightingManager,endShadowConfigure,( ), bool), asCALL_THISCALL) );

        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Lighting::LightingManager
