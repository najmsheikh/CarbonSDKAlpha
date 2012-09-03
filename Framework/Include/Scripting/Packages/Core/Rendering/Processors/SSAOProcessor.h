#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgSSAOProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace SSAOProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.SSAOProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerEnum( "SSAOMethod" ) );
            BINDSUCCESS( engine->registerObjectType( "SSAOProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgSSAOProcessor::SSAOMethod (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "SSAOMethod", "HemisphereAO", cgSSAOProcessor::HemisphereAO ) );
            BINDSUCCESS( engine->registerEnumValue( "SSAOMethod", "VolumetricAO", cgSSAOProcessor::VolumetricAO ) );
            BINDSUCCESS( engine->registerEnumValue( "SSAOMethod", "ObscuranceAO", cgSSAOProcessor::ObscuranceAO ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSSAOProcessor (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSSAOProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "SSAOProcessor", asBEHAVE_FACTORY, "SSAOProcessor@ f()", asFUNCTIONPR(ssaoProcessorFactory, (), cgSSAOProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgSSAOProcessor>( engine, "SSAOProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "void setSamplingMethod( SSAOMethod )", asMETHODPR(cgSSAOProcessor, setSamplingMethod, (cgSSAOProcessor::SSAOMethod), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "void setSampleCount( int )", asMETHODPR(cgSSAOProcessor, setSampleCount, (cgInt32), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "void setSampleRadii( int, const Vector4&in, const Vector4&in, const Vector4&in )", asMETHODPR(cgSSAOProcessor, setSampleRadii, (cgInt32, const cgVector4&, const cgVector4&, const cgVector4&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "void setBiasFactor( float )", asMETHODPR(cgSSAOProcessor, setBiasFactor, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "void setDepthFalloff( float )", asMETHODPR(cgSSAOProcessor, setDepthFalloff, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SSAOProcessor", "bool execute( CameraNode@+, const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, const DepthStencilTargetHandle &in )", asMETHODPR(cgSSAOProcessor, execute, (cgCameraNode*, const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, const cgDepthStencilTargetHandle&), bool), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : ssaoProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgSSAOProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgSSAOProcessor * ssaoProcessorFactory( )
        {
            return new cgSSAOProcessor();
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::SSAOProcessor