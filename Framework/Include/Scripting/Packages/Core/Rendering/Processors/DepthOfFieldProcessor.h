#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgDepthOfFieldProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace DepthOfFieldProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.DepthOfFieldProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DepthOfFieldProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgDepthOfFieldProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "DepthOfFieldProcessor", asBEHAVE_FACTORY, "DepthOfFieldProcessor@ f()", asFUNCTIONPR(depthOfFieldProcessorFactory, (), cgDepthOfFieldProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgDepthOfFieldProcessor>( engine, "DepthOfFieldProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setBackgroundExtents( float, float )", asMETHODPR(cgDepthOfFieldProcessor, setBackgroundExtents, (cgFloat,cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setBackgroundExtents( const RangeF &in )", asMETHODPR(cgDepthOfFieldProcessor, setBackgroundExtents, (const cgRangeF&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setForegroundExtents( float, float )", asMETHODPR(cgDepthOfFieldProcessor, setForegroundExtents, (cgFloat,cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setForegroundExtents( const RangeF &in )", asMETHODPR(cgDepthOfFieldProcessor, setForegroundExtents, (const cgRangeF&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setBackgroundBlur( int, int, float, int, int, float )", asMETHODPR(cgDepthOfFieldProcessor, setBackgroundBlur, (cgInt32, cgInt32, cgFloat, cgInt32, cgInt32, cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "void setForegroundBlur( int, int, float, int, int, float )", asMETHODPR(cgDepthOfFieldProcessor, setForegroundBlur, (cgInt32, cgInt32, cgFloat, cgInt32, cgInt32, cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "DepthOfFieldProcessor", "bool execute( const RenderTargetHandle &in, const RenderTargetHandle &in, ResampleChain@+, ResampleChain@+, ResampleChain@+, DepthType )", asMETHODPR(cgDepthOfFieldProcessor, execute, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, cgResampleChain*, cgResampleChain*, cgResampleChain*, cgDepthType::Base), bool), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : depthOfFieldProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgDepthOfFieldProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgDepthOfFieldProcessor * depthOfFieldProcessorFactory( )
        {
            return new cgDepthOfFieldProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::DepthOfFieldProcessor