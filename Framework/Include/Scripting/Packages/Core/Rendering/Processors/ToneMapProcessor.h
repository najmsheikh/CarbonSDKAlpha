#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgToneMapProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace ToneMapProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.ToneMapProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerEnum( "ToneMapMethod" ) );
            BINDSUCCESS( engine->registerObjectType( "ToneMapProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgToneMapProcessor::ToneMapMethod (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "ToneMapMethod", "Photographic", cgToneMapProcessor::Photographic ) );
            BINDSUCCESS( engine->registerEnumValue( "ToneMapMethod", "Filmic", cgToneMapProcessor::Filmic ) );
            BINDSUCCESS( engine->registerEnumValue( "ToneMapMethod", "ExponentialTM", cgToneMapProcessor::ExponentialTM ) );

            ///////////////////////////////////////////////////////////////////////
            // cgToneMapProcessor (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgToneMapProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "ToneMapProcessor", asBEHAVE_FACTORY, "ToneMapProcessor@ f()", asFUNCTIONPR(toneMapProcessorFactory, (), cgToneMapProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgToneMapProcessor>( engine, "ToneMapProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void enableLuminanceCache( bool )", asMETHODPR(cgToneMapProcessor, enableLuminanceCache, (bool), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setToneMapMethod( ToneMapMethod )", asMETHODPR(cgToneMapProcessor, setToneMapMethod, ( cgToneMapProcessor::ToneMapMethod ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setLuminanceSampleRate( float )", asMETHODPR(cgToneMapProcessor, setLuminanceSampleRate, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setLuminanceRange( float, float )", asMETHODPR(cgToneMapProcessor, setLuminanceRange, ( cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setLuminanceRange( const RangeF &in )", asMETHODPR(cgToneMapProcessor, setLuminanceRange, ( const cgRangeF& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setWhitePointAdjust( float, float )", asMETHODPR(cgToneMapProcessor, setWhitePointAdjust, ( cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setKeyAdjust( float, float )", asMETHODPR(cgToneMapProcessor, setKeyAdjust, ( cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "void setLuminanceAdaptation( float, float, float )", asMETHODPR(cgToneMapProcessor, setLuminanceAdaptation, ( cgFloat, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ToneMapProcessor", "bool execute( RenderView@+, float, const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(cgToneMapProcessor, execute, ( cgRenderView*, cgFloat, const cgTextureHandle&, const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : toneMapProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgToneMapProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgToneMapProcessor * toneMapProcessorFactory( )
        {
            return new cgToneMapProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::ToneMapProcessor