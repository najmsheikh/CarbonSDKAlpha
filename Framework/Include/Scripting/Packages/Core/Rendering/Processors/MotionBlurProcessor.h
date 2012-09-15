#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgMotionBlurProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace MotionBlurProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.MotionBlurProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "MotionBlurProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgMotionBlurProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "MotionBlurProcessor", asBEHAVE_FACTORY, "MotionBlurProcessor@ f()", asFUNCTIONPR(motionBlurProcessorFactory, (), cgMotionBlurProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgMotionBlurProcessor>( engine, "MotionBlurProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setTargetRate( float )", asMETHODPR(cgMotionBlurProcessor, setTargetRate, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setAttenuationRates( float, float )", asMETHODPR(cgMotionBlurProcessor, setAttenuationRates, (cgFloat, cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setAttenuationRates( const RangeF& )", asMETHODPR(cgMotionBlurProcessor, setAttenuationRates, (const cgRangeF&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setTranslationBlurAmount( float )", asMETHODPR(cgMotionBlurProcessor, setTranslationBlurAmount, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setRotationBlurAmount( float )", asMETHODPR(cgMotionBlurProcessor, setRotationBlurAmount, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setBlurAmount( float )", asMETHODPR(cgMotionBlurProcessor, setBlurAmount, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setMaxSpeed( float )", asMETHODPR(cgMotionBlurProcessor, setMaxSpeed, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "void setCompositeSpeedScale( float, float, float )", asMETHODPR(cgMotionBlurProcessor, setCompositeSpeedScale, (cgFloat, cgFloat, cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "bool computePixelVelocity( CameraNode@+, float, const TextureHandle &in, DepthType, const RenderTargetHandle & in )", asMETHODPR(cgMotionBlurProcessor, computePixelVelocity, (cgCameraNode*,cgFloat, const cgTextureHandle&, cgDepthType::Base, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "MotionBlurProcessor", "bool execute( int, const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, const RenderTargetHandle &in, const RenderTargetHandle &in )", asMETHODPR(cgMotionBlurProcessor, execute, (cgInt32, const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
		}

        //---------------------------------------------------------------------
        //  Name : motionBlurProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgMotionBlurProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgMotionBlurProcessor * motionBlurProcessorFactory( )
        {
            return new cgMotionBlurProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::MotionBlurProcessor