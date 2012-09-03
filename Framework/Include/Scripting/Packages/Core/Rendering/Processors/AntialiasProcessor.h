#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgAntialiasProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace AntialiasProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.AntialiasProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AntialiasProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAntialiasProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "AntialiasProcessor", asBEHAVE_FACTORY, "AntialiasProcessor@ f()", asFUNCTIONPR(antialiasProcessorFactory, (), cgAntialiasProcessor*), asCALL_CDECL) );

            // Register the object methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgAntialiasProcessor>( engine, "AntialiasProcessor" );

			// Register the object methods
			BINDSUCCESS( engine->registerObjectMethod( "AntialiasProcessor", "bool executeFXAA( const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(cgAntialiasProcessor, executeFXAA, (const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "AntialiasProcessor", "bool computePixelVelocity( CameraNode@+, const TextureHandle &in, DepthType, const RenderTargetHandle & in )", asMETHODPR(cgAntialiasProcessor, computePixelVelocity, (cgCameraNode*, const cgTextureHandle&, cgDepthType::Base, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "AntialiasProcessor", "bool temporalResolve( const TextureHandle &in, const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(cgAntialiasProcessor, temporalResolve, (const cgTextureHandle&, const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
		}

        //---------------------------------------------------------------------
        //  Name : antialiasProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgAntialiasProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgAntialiasProcessor * antialiasProcessorFactory( )
        {
            return new cgAntialiasProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::AntialiasProcessor