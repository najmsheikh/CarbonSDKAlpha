#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgImageProcessor.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace ImageProcessor
{
    //-------------------------------------------------------------------------
    // Name : registerProcessorMethods ()
    // Desc : Register the base cgImageProcessor class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerProcessorMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool initialize( RenderDriver@+ )", asMETHODPR(type, initialize, (cgRenderDriver*), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDepthStencilTarget( const DepthStencilTargetHandle &in )", asMETHODPR(type, setDepthStencilTarget, (const cgDepthStencilTargetHandle&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDefaultUserColor( const ColorValue &in )", asMETHODPR(type, setDefaultUserColor, (const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDepthExtents( float, float )", asMETHODPR(type, setDepthExtents, (cgFloat,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setNormalExtents( float, float )", asMETHODPR(type, setNormalExtents, (cgFloat,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setNormalDistances( float, float )", asMETHODPR(type, setNormalDistances, (cgFloat,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setWhiteLevel( const Vector2 &in )", asMETHODPR(type, setWhiteLevel, (const cgVector2&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setBlackLevel( const Vector2 &in )", asMETHODPR(type, setBlackLevel, (const cgVector2&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setGamma( float )", asMETHODPR(type, setGamma, (cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setTint( const ColorValue &in )", asMETHODPR(type, setTint, (const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setBrightness( float )", asMETHODPR(type, setBrightness, (cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setExposure( float, float )", asMETHODPR(type, setExposure, (cgFloat,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSaturation( float )", asMETHODPR(type, setSaturation, (cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setGrain( const TextureHandle &in, float )", asMETHODPR(type, setGrain, (const cgTextureHandle&,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setVignette( const TextureHandle &in, float, float, float )", asMETHODPR(type, setVignette, (const cgTextureHandle&,cgFloat,cgFloat,cgFloat), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRemap( const TextureHandle &in )", asMETHODPR(type, setRemap, (const cgTextureHandle&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void forceLinearSampling( bool )", asMETHODPR(type, forceLinearSampling, (bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void forcePointSampling( bool )", asMETHODPR(type, forcePointSampling, (bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void testStencilBuffer( bool, uint )", asMETHODPR(type, testStencilBuffer, (bool, cgUInt32), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyDepthStencilStates( bool )", asMETHODPR(type, applyDepthStencilStates, (bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyRasterizerStates( bool )", asMETHODPR(type, applyRasterizerStates, (bool), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyBlendStates( bool )", asMETHODPR(type, applyBlendStates, (bool), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImage( const RenderTargetHandle &in, ImageOperation )", asMETHODPR(type, processColorImage, (const cgRenderTargetHandle&, cgImageOperation::Base), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImage( const RenderTargetHandle &in, ImageOperation, const ColorValue &in )", asMETHODPR(type, processColorImage, (const cgRenderTargetHandle&, cgImageOperation::Base, const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImage( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation )", asMETHODPR(type, processColorImage, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImage( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation, const ColorValue &in )", asMETHODPR(type, processColorImage, (const cgTextureHandle&, const cgRenderTargetHandle &, cgImageOperation::Base, const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImageMulti( const RenderTargetHandle &in, const array<ImageOperation> &in )", asMETHODPR(type, processColorImageMulti, (const cgRenderTargetHandle&, const cgImageOperation::Array&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImageMulti( const RenderTargetHandle &in, const array<ImageOperation> &in, const ColorValue &in )", asMETHODPR(type, processColorImageMulti, (const cgRenderTargetHandle&, const cgImageOperation::Array&, const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImageMulti( const TextureHandle &in, const RenderTargetHandle &in, const array<ImageOperation> &in )", asMETHODPR(type, processColorImageMulti, (const cgTextureHandle&, const cgRenderTargetHandle&, const cgImageOperation::Array&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processColorImageMulti( const TextureHandle &in, const RenderTargetHandle &in, const array<ImageOperation> &in, const ColorValue &in )", asMETHODPR(type, processColorImageMulti, (const cgTextureHandle&, const cgRenderTargetHandle &, const cgImageOperation::Array&, const cgColorValue&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processDepthImage( const TextureHandle &in, const RenderTargetHandle &in, DepthType, DepthType )", asMETHODPR(type, processDepthImage, (const cgTextureHandle&, const cgRenderTargetHandle&, cgDepthType::Base, cgDepthType::Base), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void processDepthImage( const TextureHandle &in, const RenderTargetHandle &in, DepthType, DepthType, const DepthStencilTargetHandle &in )", asMETHODPR(type, processDepthImage, (const cgTextureHandle&, const cgRenderTargetHandle&, cgDepthType::Base, cgDepthType::Base, const cgDepthStencilTargetHandle&), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation, bool, bool, bool )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base, bool, bool, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( ResampleChain@+ )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( ResampleChain@+, ImageOperation )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSample( ResampleChain@+, ImageOperation, bool, bool, bool )", asMETHODPR(type, downSample, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base, bool, bool, bool ), void), asCALL_THISCALL) );
        
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSampleDepth( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation )", asMETHODPR(type, downSampleDepth, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSampleDepth( const TextureHandle &in, const RenderTargetHandle &in, ImageOperation, bool )", asMETHODPR(type, downSampleDepth, (const cgTextureHandle&, const cgRenderTargetHandle&, cgImageOperation::Base, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSampleDepth( const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, const RenderTargetHandle &in, DepthType, DepthType, ImageOperation, bool )", asMETHODPR(type, downSampleDepth, (const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, const cgRenderTargetHandle&, cgDepthType::Base, cgDepthType::Base, cgImageOperation::Base, bool ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, int, float, int )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, cgInt32, cgFloat, cgInt32 ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, int, float, int, AlphaWeightMethod, AlphaWeightMethod )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, cgInt32, cgFloat, cgInt32, cgAlphaWeightMethod::Base , cgAlphaWeightMethod::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, int, float, float, int )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgFloat, cgInt32 ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, int, float, float, int, AlphaWeightMethod, AlphaWeightMethod )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgFloat, cgInt32, cgAlphaWeightMethod::Base , cgAlphaWeightMethod::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, const BlurOpDesc &in )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgBlurOpDesc& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, const BlurOpDesc[] &in )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgBlurOpDesc::Array& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void blur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const BlurOpDesc[] &in )", asMETHODPR(type, blur, (const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgBlurOpDesc::Array& ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bilateralBlur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, int, float, int, bool, bool, DepthType )", asMETHODPR(type, bilateralBlur, ( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgInt32, bool, bool, cgDepthType::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bilateralBlur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, int, float, int, bool, bool, DepthType, bool )", asMETHODPR(type, bilateralBlur, ( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgInt32, bool, bool, cgDepthType::Base, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bilateralBlur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, int, float, float, int, bool, bool, DepthType )", asMETHODPR(type, bilateralBlur, ( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgFloat, cgInt32, bool, bool, cgDepthType::Base ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bilateralBlur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, int, float, float, int, bool, bool, DepthType, bool )", asMETHODPR(type, bilateralBlur, ( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgFloat, cgInt32, bool, bool, cgDepthType::Base, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bilateralBlur( const RenderTargetHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, int, float, float, int, bool, bool, DepthType, bool, bool, float, float )", asMETHODPR(type, bilateralBlur, ( const cgRenderTargetHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, cgInt32, cgFloat, cgFloat, cgInt32, bool, bool, cgDepthType::Base, bool, bool, cgFloat, cgFloat ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resample( const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(type, resample, ( const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resample( const TextureHandle &in, const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, const TextureHandle &in, const TextureHandle &in, bool, bool, DepthType, DepthType, bool, bool, bool )", asMETHODPR(type, resample, ( const cgTextureHandle&, const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, const cgTextureHandle&, const cgTextureHandle&, bool, bool, cgDepthType::Base, cgDepthType::Base, bool, bool, bool ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void packDepthNormal( const TextureHandle &in, const RenderTargetHandle &in, bool )", asMETHODPR(type, packDepthNormal, (const cgTextureHandle&, const cgRenderTargetHandle&, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void packDepthNormal( const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, bool )", asMETHODPR(type, packDepthNormal, (const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, bool ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void discontinuityDetect( const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, DiscontinuityTestMethod, bool, DepthType )", asMETHODPR(type, discontinuityDetect, (const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, cgDiscontinuityTestMethod::Base, bool, cgDepthType::Base ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void computeScreenMask( const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(type, computeScreenMask, (const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void downSampleScreenMask( const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(type, downSampleScreenMask, (const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resolveScreenMask( const TextureHandle &in )", asMETHODPR(type, resolveScreenMask, (const cgTextureHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resolveScreenMask( const TextureHandle &in, bool )", asMETHODPR(type, resolveScreenMask, (const cgTextureHandle&, bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resolveScreenMask( const TextureHandle &in, const RenderTargetHandle &in )", asMETHODPR(type, resolveScreenMask, (const cgTextureHandle&, const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resolveScreenMask( const TextureHandle &in, float )", asMETHODPR(type, resolveScreenMask, (const cgTextureHandle&, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void resolveScreenMask( const TextureHandle &in, const RenderTargetHandle &in, float, bool )", asMETHODPR(type, resolveScreenMask, (const cgTextureHandle&, const cgRenderTargetHandle&, cgFloat, bool ), void), asCALL_THISCALL) );

		BINDSUCCESS( engine->registerObjectMethod( typeName, "void compositeLighting( const TextureHandle &in, const TextureHandle &in, const TextureHandle &in, const TextureHandle &in, const TextureHandle &in, const RenderTargetHandle &in, bool, bool, bool, bool )", asMETHODPR(type, compositeLighting, (const cgTextureHandle&, const cgTextureHandle&, const cgTextureHandle&, const cgTextureHandle&, const cgTextureHandle&, const cgRenderTargetHandle&, bool, bool, bool, bool ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void drawClipQuad( )", asMETHODPR(type, drawClipQuad, ( ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void drawClipQuad( float )", asMETHODPR(type, drawClipQuad, ( cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void drawClipQuad( const RenderTargetHandle &in )", asMETHODPR(type, drawClipQuad, ( const cgRenderTargetHandle& ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void drawClipQuad( const RenderTargetHandle &in, float )", asMETHODPR(type, drawClipQuad, ( const cgRenderTargetHandle&, cgFloat ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void drawQuad( const RenderTargetHandle &in, bool, float )", asMETHODPR(type, drawQuad, ( const cgRenderTargetHandle&, bool, cgFloat ), void), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool selectClipQuadVertexShader( )", asMETHODPR(type, selectClipQuadVertexShader, ( ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool selectClipQuadVertexShader( bool )", asMETHODPR(type, selectClipQuadVertexShader, ( bool ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setConstants( const TextureHandle &in, int, const ColorValue& )", asMETHODPR(type, setConstants, ( const cgTextureHandle&, cgInt32, const cgColorValue&), void), asCALL_THISCALL) );
        
        /*
        void            FXAA                    ( const cgTextureHandle & hSrc, const cgRenderTargetHandle & hDest, cgInt32 nVersion );

        void            BlendFrames             ( const cgTextureHandle & hCurrentColor, const cgTextureHandle & hPrevColor,
            const cgTextureHandle & hCurrentDepth, const cgTextureHandle & hPrevDepth, 
            const cgRenderTargetHandle & hDest, const cgMatrix & mtxReprojection, cgFloat fDepthTolerance, cgFloat fBlendAmount );*/

    } // End Method registerProcessorMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.ImageProcessor" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ImageProcessor", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgImageProcessor>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "ImageProcessor", asBEHAVE_FACTORY, "ImageProcessor@ f()", asFUNCTIONPR(imageProcessorFactory, (), cgImageProcessor*), asCALL_CDECL) );

            // Register the object methods
            registerProcessorMethods<cgImageProcessor>( engine, "ImageProcessor" );
        }

        //---------------------------------------------------------------------
        //  Name : imageProcessorFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgImageProcessor class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgImageProcessor * imageProcessorFactory( )
        {
            return new cgImageProcessor();
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::ImageProcessor