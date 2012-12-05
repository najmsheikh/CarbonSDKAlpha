#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgAtmosphericsProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace AtmosphericsProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.AtmosphericsProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AtmosphericsProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAtmosphericsProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "AtmosphericsProcessor", asBEHAVE_FACTORY, "AtmosphericsProcessor@ f()", asFUNCTIONPR(atmosphericsProcessorFactory, (), cgAtmosphericsProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgAtmosphericsProcessor>( engine, "AtmosphericsProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSky( SkyElement@+, bool )", asMETHODPR(cgAtmosphericsProcessor, drawSky, ( cgSkyElement*, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSky( SkyElement@+, bool, const RenderTargetHandle &in )", asMETHODPR(cgAtmosphericsProcessor, drawSky, ( cgSkyElement*, bool, const cgRenderTargetHandle&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSkyColor( ColorValue, bool )", asMETHODPR(cgAtmosphericsProcessor, drawSkyColor, ( cgColorValue, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSkyColor( ColorValue, bool, const RenderTargetHandle &in )", asMETHODPR(cgAtmosphericsProcessor, drawSkyColor, ( cgColorValue, bool, const cgRenderTargetHandle&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSkyBox( const TextureHandle &in, bool )", asMETHODPR(cgAtmosphericsProcessor, drawSkyBox, (const cgTextureHandle&, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawSkyBox( const TextureHandle &in, bool, const RenderTargetHandle &in )", asMETHODPR(cgAtmosphericsProcessor, drawSkyBox, (const cgTextureHandle&, bool, const cgRenderTargetHandle&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "AtmosphericsProcessor", "void drawFog( FogModel, const TextureHandle &in, DepthType, const RenderTargetHandle &in )", asMETHODPR(cgAtmosphericsProcessor, drawFog, (cgFogModel::Base, const cgTextureHandle&, cgDepthType::Base, const cgRenderTargetHandle&), void), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : atmosphericsProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgAtmosphericsProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgAtmosphericsProcessor * atmosphericsProcessorFactory( )
        {
            return new cgAtmosphericsProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::AtmosphericsProcessor