#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/Processors/cgGlareProcessor.h>
#include <Rendering/Processors/cgTonemapProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering { namespace Processors {

// Package declaration
namespace GlareProcessor
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors.GlareProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "GlareStepDesc", sizeof(cgGlareProcessor::GlareStepDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
			BINDSUCCESS( engine->registerObjectType( "ILRElement", sizeof(cgGlareProcessor::ILRElement), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "GlareProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgGlareProcessor::GlareStepDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgGlareProcessor::GlareStepDesc>( engine, "GlareStepDesc" );
			registerDefaultCDA<cgGlareProcessor::ILRElement>( engine, "ILRElement" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "GlareStepDesc", asBEHAVE_CONSTRUCT,  "void f(int, float, int, int, float, float, float)", asFUNCTIONPR(constructGlareStepDesc,(cgInt32, cgFloat, cgInt32, cgInt32, cgFloat, cgFloat, cgFloat, cgGlareProcessor::GlareStepDesc*),void), asCALL_CDECL_OBJLAST) );
			BINDSUCCESS( engine->registerObjectBehavior( "ILRElement", asBEHAVE_CONSTRUCT,  "void f(float, float, bool)", asFUNCTIONPR(constructILRElement,(cgFloat, cgFloat, bool, cgGlareProcessor::ILRElement*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "GlareStepDesc", "int levelIndex"   , offsetof(cgGlareProcessor::GlareStepDesc,levelIndex) ) );
            BINDSUCCESS( engine->registerObjectProperty( "GlareStepDesc", "float intensity"  , offsetof(cgGlareProcessor::GlareStepDesc,intensity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "GlareStepDesc", "BlurOpDesc blurOp", offsetof(cgGlareProcessor::GlareStepDesc,blurOp) ) );
			BINDSUCCESS( engine->registerObjectProperty( "GlareStepDesc", "float blendAmount", offsetof(cgGlareProcessor::GlareStepDesc,cacheBlendAmount) ) );
			BINDSUCCESS( engine->registerObjectProperty( "GlareStepDesc", "float blendRate",   offsetof(cgGlareProcessor::GlareStepDesc,cacheBlendRate) ) );

            // Requires array type for several methods in the image processing interface.
            BINDSUCCESS( engine->registerObjectType( "GlareStepDesc[]", sizeof(std::vector<cgGlareProcessor::GlareStepDesc>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            STDVectorHelper<cgGlareProcessor::GlareStepDesc>::registerMethods( engine, "GlareStepDesc[]", "GlareStepDesc" );

			BINDSUCCESS( engine->registerObjectType( "ILRElement[]", sizeof(std::vector<cgGlareProcessor::ILRElement>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
			STDVectorHelper<cgGlareProcessor::ILRElement>::registerMethods( engine, "ILRElement[]", "ILRElement" );

            ///////////////////////////////////////////////////////////////////////
            // cgGlareProcessor (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgGlareProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "GlareProcessor", asBEHAVE_FACTORY, "GlareProcessor@ f()", asFUNCTIONPR(glareProcessorFactory, (), cgGlareProcessor*), asCALL_CDECL) );

            // Register the base methods
            Core::Rendering::ImageProcessor::registerProcessorMethods<cgGlareProcessor>( engine, "GlareProcessor" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setBrightThreshold( float, float )", asMETHODPR(cgGlareProcessor, setBrightThreshold, (cgFloat,cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setBrightThreshold( const RangeF &in )", asMETHODPR(cgGlareProcessor, setBrightThreshold, (const cgRangeF&), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setCacheValues( float, float )", asMETHODPR(cgGlareProcessor, setCacheValues, (cgFloat,cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setGlareAmount( float )", asMETHODPR(cgGlareProcessor, setGlareAmount, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setGlareSteps( const array<GlareStepDesc> &in )", asMETHODPR(cgGlareProcessor, setGlareSteps, (const cgGlareProcessor::GlareStepArray&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "bool execute( const RenderTargetHandle &in, ResampleChain@+, ResampleChain@+, bool, float, bool, bool)", asMETHODPR(cgGlareProcessor, execute, (const cgRenderTargetHandle&, cgResampleChain*, cgResampleChain*, bool, cgFloat, bool, bool), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setToneMapper( ToneMapProcessor@+ )", asMETHODPR(cgGlareProcessor, setToneMapper, (cgToneMapProcessor*), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRBrightThreshold( float )", asMETHODPR(cgGlareProcessor, setILRBrightThreshold, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRContrast( float )", asMETHODPR(cgGlareProcessor, setILRContrast, (cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRLowResData( int, int, int, float )", asMETHODPR(cgGlareProcessor, setILRLowResData, (cgInt32, cgInt32, cgInt32, cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRHighResData( int, int, int, float )", asMETHODPR(cgGlareProcessor, setILRHighResData, (cgInt32, cgInt32, cgInt32, cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRElements( const array<ILRElement> &in )", asMETHODPR(cgGlareProcessor, setILRElements, (const cgGlareProcessor::ILRElementArray&), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setILRDepthRange( float, float )", asMETHODPR(cgGlareProcessor, setILRDepthRange, (cgFloat,cgFloat), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "bool executeILR( const RenderTargetHandle &in, const TextureHandle& in, ResampleChain@+, ResampleChain@+, bool, bool)", asMETHODPR(cgGlareProcessor, executeILR, (const cgRenderTargetHandle&, const cgTextureHandle&, cgResampleChain*, cgResampleChain*, bool, bool), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "GlareProcessor", "void setAnamorphicData( int, int, float, const Vector3 &, float, float )", asMETHODPR(cgGlareProcessor, setAnamorphicData, (cgInt32, cgInt32, cgFloat, const cgVector3&, cgFloat, cgFloat), void), asCALL_THISCALL) );
		}

        //---------------------------------------------------------------------
        //  Name : glareProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgGlareProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgGlareProcessor * glareProcessorFactory( )
        {
            return new cgGlareProcessor();
        }

        //---------------------------------------------------------------------
        //  Name : constructGlareStepDesc ()
        /// <summary>
        /// This is a wrapper for the alternative cgGlareProcessor::GlareStepDesc 
        /// constructor, since it is not possible to take the address of the 
        /// constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructGlareStepDesc( cgInt32 levelIndex, cgFloat levelIntensity, cgInt32 blurPassCount, cgInt32 blurPixelRadius, cgFloat blurDistanceFactor, cgFloat blendAmount, cgFloat blendRate, cgGlareProcessor::GlareStepDesc *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgGlareProcessor::GlareStepDesc( levelIndex, levelIntensity, blurPassCount, blurPixelRadius, blurDistanceFactor, blendAmount, blendRate );
        }

		//---------------------------------------------------------------------
		//  Name : constructILRElement ()
		/// <summary>
		/// This is a wrapper for the alternative cgGlareProcessor::ILRElement
		/// constructor, since it is not possible to take the address of the 
		/// constructor directly.
		/// </summary>
		//---------------------------------------------------------------------
		static void constructILRElement( cgFloat flareDelta, cgFloat flareAlpha, bool highDetail, cgGlareProcessor::ILRElement *thisPointer )
		{
			// Use placement new to allocate which will in turn call the constructor
			new(thisPointer) cgGlareProcessor::ILRElement( flareDelta, flareAlpha, highDetail );
		}


    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors::GlareProcessor