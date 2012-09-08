#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgRenderingTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "Viewport", sizeof(cgViewport), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
            BINDSUCCESS( engine->registerObjectType( "SamplerStateDesc", sizeof(cgSamplerStateDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "TargetBlendStateDesc", sizeof(cgTargetBlendStateDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "BlendStateDesc", sizeof(cgBlendStateDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "RasterizerStateDesc", sizeof(cgRasterizerStateDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "DepthStencilOpDesc", sizeof(cgDepthStencilOpDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "DepthStencilStateDesc", sizeof(cgDepthStencilStateDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "BlurOpDesc", sizeof(cgBlurOpDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "ShadowSettings", sizeof(cgShadowSettings), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "TechniqueResult" ) );
            BINDSUCCESS( engine->registerEnum( "HardwareType" ) );
            BINDSUCCESS( engine->registerEnum( "ColorChannel" ) );
            BINDSUCCESS( engine->registerEnum( "FilterMethod" ) );
            BINDSUCCESS( engine->registerEnum( "AddressingMode" ) );
            BINDSUCCESS( engine->registerEnum( "BlendMode" ) );
            BINDSUCCESS( engine->registerEnum( "BlendOperation" ) );
            BINDSUCCESS( engine->registerEnum( "ComparisonFunction" ) );
            BINDSUCCESS( engine->registerEnum( "CullMode" ) );
            BINDSUCCESS( engine->registerEnum( "FillMode" ) );
            BINDSUCCESS( engine->registerEnum( "StencilOperation" ) );
            BINDSUCCESS( engine->registerEnum( "PrimitiveType" ) );
            BINDSUCCESS( engine->registerEnum( "ClearFlags" ) );
            BINDSUCCESS( engine->registerEnum( "MaterialFilter" ) );
            BINDSUCCESS( engine->registerEnum( "SystemState" ) );
            BINDSUCCESS( engine->registerEnum( "ReflectionMode" ) );
            BINDSUCCESS( engine->registerEnum( "ImageOperation" ) );
            BINDSUCCESS( engine->registerEnum( "DepthType" ) );
            BINDSUCCESS( engine->registerEnum( "AlphaWeightMethod" ) );
            BINDSUCCESS( engine->registerEnum( "DiscontinuityTestMethod" ) );
            BINDSUCCESS( engine->registerEnum( "ShadowMethod" ) );
            BINDSUCCESS( engine->registerEnum( "FogModel" ) );
            BINDSUCCESS( engine->registerEnum( "AntialiasingMethod" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgHardwareType (Enum)
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "HardwareType";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Generic", cgHardwareType::Generic ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NVIDIA" , cgHardwareType::NVIDIA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AMD"    , cgHardwareType::AMD ) );

            ///////////////////////////////////////////////////////////////////////
            // cgColorChannel (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "ColorChannel";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"          , cgColorChannel::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Red"           , cgColorChannel::Red ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Green"         , cgColorChannel::Green ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Blue"          , cgColorChannel::Blue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Alpha"         , cgColorChannel::Alpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RedGreen"      , cgColorChannel::RedGreen ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RedBlue"       , cgColorChannel::RedBlue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RedAlpha"      , cgColorChannel::RedAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "GreenBlue"     , cgColorChannel::GreenBlue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "GreenAlpha"    , cgColorChannel::GreenAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BlueAlpha"     , cgColorChannel::BlueAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RedGreenBlue"  , cgColorChannel::RedGreenBlue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RedGreenAlpha" , cgColorChannel::RedGreenAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "GreenBlueAlpha", cgColorChannel::GreenBlueAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "All"			  , cgColorChannel::All ) );

            ///////////////////////////////////////////////////////////////////////
            // cgFilterMethod (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "FilterMethod";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"       , cgFilterMethod::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Point"      , cgFilterMethod::Point ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Linear"     , cgFilterMethod::Linear ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Anisotropic", cgFilterMethod::Anisotropic ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAddressingMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "AddressingMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Wrap"       , cgAddressingMode::Wrap ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Mirror"     , cgAddressingMode::Mirror ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Clamp"      , cgAddressingMode::Clamp ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Border"     , cgAddressingMode::Border ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MirrorOnce" , cgAddressingMode::MirrorOnce ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBlendMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "BlendMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Zero"           , cgBlendMode::Zero ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "One"            , cgBlendMode::One ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SrcColor"       , cgBlendMode::SrcColor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvSrcColor"    , cgBlendMode::InvSrcColor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SrcAlpha"       , cgBlendMode::SrcAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvSrcAlpha"    , cgBlendMode::InvSrcAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DestAlpha"      , cgBlendMode::DestAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvDestAlpha"   , cgBlendMode::InvDestAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DestColor"      , cgBlendMode::DestColor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvDestColor"   , cgBlendMode::InvDestColor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SrcAlphaSat"    , cgBlendMode::SrcAlphaSat ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BothSrcAlpha"   , cgBlendMode::BothSrcAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BothInvSrcAlpha", cgBlendMode::BothInvSrcAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BlendFactor"    , cgBlendMode::BlendFactor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvBlendFactor" , cgBlendMode::InvBlendFactor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Src1Color"      , cgBlendMode::Src1Color ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvSrc1Color"   , cgBlendMode::InvSrc1Color ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Src1Alpha"      , cgBlendMode::Src1Alpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InvSrc1Alpha"   , cgBlendMode::InvSrc1Alpha ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgBlendOperation (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "BlendOperation";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Add"        , cgBlendOperation::Add ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Subtract"   , cgBlendOperation::Subtract ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RevSubtract", cgBlendOperation::RevSubtract ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Min"        , cgBlendOperation::Min ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Max"        , cgBlendOperation::Max ) );

            ///////////////////////////////////////////////////////////////////////
            // cgComparisonFunction (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "ComparisonFunction";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Never"       , cgComparisonFunction::Never ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Less"        , cgComparisonFunction::Less ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Equal"       , cgComparisonFunction::Equal ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LessEqual"   , cgComparisonFunction::LessEqual ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Greater"     , cgComparisonFunction::Greater ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NotEqual"    , cgComparisonFunction::NotEqual ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "GreaterEqual", cgComparisonFunction::GreaterEqual ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Always"      , cgComparisonFunction::Always ) );

            ///////////////////////////////////////////////////////////////////////
            // cgCullMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "CullMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None" , cgCullMode::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Front", cgCullMode::Front ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Back" , cgCullMode::Back ) );

            ///////////////////////////////////////////////////////////////////////
            // cgFillMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "FillMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Point"    , cgFillMode::Point ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Wireframe", cgFillMode::Wireframe ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Solid"    , cgFillMode::Solid ) );

            ///////////////////////////////////////////////////////////////////////
            // cgStencilOperation (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "StencilOperation";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Keep"   , cgStencilOperation::Keep ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Zero"   , cgStencilOperation::Zero ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Replace", cgStencilOperation::Replace ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "IncrSat", cgStencilOperation::IncrSat ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DecrSat", cgStencilOperation::DecrSat ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Invert" , cgStencilOperation::Invert ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Incr"   , cgStencilOperation::Incr ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Decr"   , cgStencilOperation::Decr ) );

            ///////////////////////////////////////////////////////////////////////
            // cgPrimitiveType (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "PrimitiveType";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "PointList"    , cgPrimitiveType::PointList ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LineList"     , cgPrimitiveType::LineList ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LineStrip"    , cgPrimitiveType::LineStrip ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TriangleList" , cgPrimitiveType::TriangleList ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TriangleStrip", cgPrimitiveType::TriangleStrip ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TriangleFan"  , cgPrimitiveType::TriangleFan ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgClearFlags (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "ClearFlags";
            
            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Target" , cgClearFlags::Target ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Depth"  , cgClearFlags::Depth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Stencil", cgClearFlags::Stencil ) );

            ///////////////////////////////////////////////////////////////////////
            // cgTechniqueResult (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "TechniqueResult";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Continue", cgTechniqueResult::Continue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Complete", cgTechniqueResult::Complete ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Abort"   , cgTechniqueResult::Abort ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSystemState (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "SystemState";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "ShadingQuality"             , cgSystemState::ShadingQuality ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "OrthographicCamera"         , cgSystemState::OrthographicCamera ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "OutputEncodingType"         , cgSystemState::OutputEncodingType ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "FogModel"                   , cgSystemState::FogModel ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "HDRLighting"                , cgSystemState::HDRLighting ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ViewSpaceLighting"          , cgSystemState::ViewSpaceLighting ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DeferredRendering"          , cgSystemState::DeferredRendering ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DeferredLighting"           , cgSystemState::DeferredLighting ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SpecularColorOutput"        , cgSystemState::SpecularColorOutput ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "GBufferSRGB"                , cgSystemState::GBufferSRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NonLinearZ"                 , cgSystemState::NonLinearZ ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NormalizedDistance"         , cgSystemState::NormalizedDistance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SurfaceNormals"             , cgSystemState::SurfaceNormals ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PackedDepth"                , cgSystemState::PackedDepth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "MaximumBlendIndex"          , cgSystemState::MaximumBlendIndex ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthType"                  , cgSystemState::DepthType ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SurfaceNormalType"          , cgSystemState::SurfaceNormalType ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorWrites"                , cgSystemState::ColorWrites ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CullMode"                   , cgSystemState::CullMode ) );

            BINDSUCCESS( engine->registerEnumValue( typeName, "LightType"                  , cgSystemState::LightType ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ShadowMethod"               , cgSystemState::ShadowMethod ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PrimaryTaps"                , cgSystemState::PrimaryTaps ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SecondaryTaps"              , cgSystemState::SecondaryTaps ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleAttenuation"          , cgSystemState::SampleAttenuation ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleDistanceAttenuation"  , cgSystemState::SampleDistanceAttenuation ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorTexture3D"             , cgSystemState::ColorTexture3D ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DiffuseIBL"                 , cgSystemState::DiffuseIBL ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SpecularIBL"                , cgSystemState::SpecularIBL ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ComputeAmbient"             , cgSystemState::ComputeAmbient ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ComputeDiffuse"             , cgSystemState::ComputeDiffuse ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ComputeSpecular"            , cgSystemState::ComputeSpecular ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "UseSSAO"                    , cgSystemState::UseSSAO ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Trilighting"                , cgSystemState::Trilighting ) );

            BINDSUCCESS( engine->registerEnumValue( typeName, "NormalSource"               , cgSystemState::NormalSource ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ReflectionMode"             , cgSystemState::ReflectionMode ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LightTextureType"           , cgSystemState::LightTextureType ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleDiffuseTexture"       , cgSystemState::SampleDiffuseTexture ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleSpecularColor"        , cgSystemState::SampleSpecularColor ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleSpecularMask"         , cgSystemState::SampleSpecularMask ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleGlossTexture"         , cgSystemState::SampleGlossTexture ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleEmissiveTexture"      , cgSystemState::SampleEmissiveTexture ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleOpacityTexture"       , cgSystemState::SampleOpacityTexture ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DecodeSRGB"                 , cgSystemState::DecodeSRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ComputeToksvig"             , cgSystemState::ComputeToksvig ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CorrectNormals"             , cgSystemState::CorrectNormals ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "OpacityInDiffuse"           , cgSystemState::OpacityInDiffuse ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SurfaceFresnel"             , cgSystemState::SurfaceFresnel ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Metal"                      , cgSystemState::Metal ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Transmissive"               , cgSystemState::Transmissive ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Translucent"                , cgSystemState::Translucent ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AlphaTest"                  , cgSystemState::AlphaTest ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Emissive"                   , cgSystemState::Emissive ) );

            ///////////////////////////////////////////////////////////////////////
            // cgReflectionMode (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "ReflectionMode";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"          , cgReflectionMode::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Planar"        , cgReflectionMode::Planar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "EnvironmentMap", cgReflectionMode::EnvironmentMap ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgDepthType (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "DepthType";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"                  , cgDepthType::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearDistance"        , cgDepthType::LinearDistance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearZ"               , cgDepthType::LinearZ ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NonLinearZ"            , cgDepthType::NonLinearZ ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearDistance_Packed" , cgDepthType::LinearDistance_Packed ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearZ_Packed"        , cgDepthType::LinearZ_Packed ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NonLinearZ_Packed"     , cgDepthType::NonLinearZ_Packed ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearZ_Normal_Packed" , cgDepthType::LinearZ_Normal_Packed ) );

            ///////////////////////////////////////////////////////////////////////
            // cgImageOperation (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "ImageOperation";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"                    , cgImageOperation::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "All"                     , cgImageOperation::All ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SetColorRGB"             , cgImageOperation::SetColorRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SetColorRGBA"            , cgImageOperation::SetColorRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SetColorA"               , cgImageOperation::SetColorA ) );
			BINDSUCCESS( engine->registerEnumValue( typeName, "ScaleUserColorRGBA"      , cgImageOperation::ScaleUserColorRGBA ) );
			BINDSUCCESS( engine->registerEnumValue( typeName, "ScaleUserColorRGB"       , cgImageOperation::ScaleUserColorRGB ) );
			BINDSUCCESS( engine->registerEnumValue( typeName, "ScaleUserColorA"         , cgImageOperation::ScaleUserColorA ) );
			BINDSUCCESS( engine->registerEnumValue( typeName, "CopyAlpha"               , cgImageOperation::CopyAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CopyRGBA"                , cgImageOperation::CopyRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CopyRGB"                 , cgImageOperation::CopyRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CopyRGBSetAlpha"         , cgImageOperation::CopyRGBSetAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorScaleRGBA"          , cgImageOperation::ColorScaleRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorScaleRGB"           , cgImageOperation::ColorScaleRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AddRGBA"                 , cgImageOperation::AddRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AddRGB"                  , cgImageOperation::AddRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AddRGBSetAlpha"          , cgImageOperation::AddRGBSetAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorScaleAddRGBA"       , cgImageOperation::ColorScaleAddRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorScaleAddRGB"        , cgImageOperation::ColorScaleAddRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorScaleAddRGBSetAlpha", cgImageOperation::ColorScaleAddRGBSetAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TextureScaleRGBA"        , cgImageOperation::TextureScaleRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TextureScaleRGB"         , cgImageOperation::TextureScaleRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TextureScaleA"           , cgImageOperation::TextureScaleA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InverseRGBA"             , cgImageOperation::InverseRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "InverseRGB"              , cgImageOperation::InverseRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "UnsignedToSignedRGBA"    , cgImageOperation::UnsignedToSignedRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "UnsignedToSignedRGB"     , cgImageOperation::UnsignedToSignedRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SignedToUnsignedRGBA"    , cgImageOperation::SignedToUnsignedRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SignedToUnsignedRGB"     , cgImageOperation::SignedToUnsignedRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Grayscale"               , cgImageOperation::Grayscale ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Luminance"               , cgImageOperation::Luminance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Sepia"                   , cgImageOperation::Sepia ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RGBEtoRGB"               , cgImageOperation::RGBEtoRGB ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RGBAtoRGBL"              , cgImageOperation::RGBAtoRGBL ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TestAlpha"               , cgImageOperation::TestAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TestAlphaDS"             , cgImageOperation::TestAlphaDS ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AlphaToRGBA"             , cgImageOperation::AlphaToRGBA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BilateralResample"       , cgImageOperation::BilateralResample ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BilateralBlur"           , cgImageOperation::BilateralBlur ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Blur"                    , cgImageOperation::Blur ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Exposure"                , cgImageOperation::Exposure ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Saturation"              , cgImageOperation::Saturation ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LevelsIn"                , cgImageOperation::LevelsIn ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LevelsOut"               , cgImageOperation::LevelsOut ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Gamma"                   , cgImageOperation::Gamma ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ColorRemap"              , cgImageOperation::ColorRemap ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TintAndBrightness"       , cgImageOperation::TintAndBrightness ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Grain"                   , cgImageOperation::Grain ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Vignette"                , cgImageOperation::Vignette ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BlueShift"               , cgImageOperation::BlueShift ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearZToDistance"       , cgImageOperation::LinearZToDistance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DistanceToLinearZ"       , cgImageOperation::DistanceToLinearZ ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LinearZToDepth"          , cgImageOperation::LinearZToDepth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DistanceToDepth"         , cgImageOperation::DistanceToDepth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DownSampleAverage"       , cgImageOperation::DownSampleAverage ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DownSampleMinimum"       , cgImageOperation::DownSampleMinimum ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DownSampleMaximum"       , cgImageOperation::DownSampleMaximum ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DownSampleMinMax"        , cgImageOperation::DownSampleMinMax ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAlphaWeightMethod (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "AlphaWeightMethod";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"              , cgAlphaWeightMethod::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SetZero"           , cgAlphaWeightMethod::SetZero ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SetOne"            , cgAlphaWeightMethod::SetOne ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Sample"            , cgAlphaWeightMethod::Sample ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Center"            , cgAlphaWeightMethod::Center ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SampleCenter"      , cgAlphaWeightMethod::SampleCenter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BinarySample"      , cgAlphaWeightMethod::BinarySample ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BinaryCenter"      , cgAlphaWeightMethod::BinaryCenter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "BinarySampleCenter", cgAlphaWeightMethod::BinarySampleCenter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SamplePow2"        , cgAlphaWeightMethod::SamplePow2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SamplePow3"        , cgAlphaWeightMethod::SamplePow3 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SamplePow4"        , cgAlphaWeightMethod::SamplePow4 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AvgSample"         , cgAlphaWeightMethod::AvgSample ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "AvgSampleCenter"   , cgAlphaWeightMethod::AvgSampleCenter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpGreater"        , cgAlphaWeightMethod::CmpGreater ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpGreaterEqual"   , cgAlphaWeightMethod::CmpGreaterEqual ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpLess"           , cgAlphaWeightMethod::CmpLess ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpLessEqual"      , cgAlphaWeightMethod::CmpLessEqual ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpMin"            , cgAlphaWeightMethod::CmpMin ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "CmpMax"            , cgAlphaWeightMethod::CmpMax ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bilateral"         , cgAlphaWeightMethod::Bilateral ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bilateral_Pow2"    , cgAlphaWeightMethod::Bilateral_Pow2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bilateral_Pow3"    , cgAlphaWeightMethod::Bilateral_Pow3 ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDiscontinuityTestMethod (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "DiscontinuityTestMethod";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Depth"             , cgDiscontinuityTestMethod::Depth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Normal"            , cgDiscontinuityTestMethod::Normal ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthAndNormal"    , cgDiscontinuityTestMethod::DepthAndNormal ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Plane"             , cgDiscontinuityTestMethod::Plane ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PlaneAndNormal"    , cgDiscontinuityTestMethod::PlaneAndNormal ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthFast"         , cgDiscontinuityTestMethod::DepthFast ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NormalFast"        , cgDiscontinuityTestMethod::NormalFast ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthAndNormalFast", cgDiscontinuityTestMethod::DepthAndNormalFast ) );           

            ///////////////////////////////////////////////////////////////////////
            // cgShadowMethod (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "ShadowMethod";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Depth"			, cgShadowMethod::Depth ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Variance"		, cgShadowMethod::Variance ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Exponential"		, cgShadowMethod::Exponential ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Reflective"		, cgShadowMethod::Reflective ) );

            BINDSUCCESS( engine->registerEnumValue( typeName, "Bits16"			, cgShadowMethod::Bits16 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bits24"			, cgShadowMethod::Bits24 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bits32"			, cgShadowMethod::Bits32 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ExtentsBits16"	, cgShadowMethod::ExtentsBits16 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Precomputed"		, cgShadowMethod::Precomputed ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PrecomputedAlpha", cgShadowMethod::PrecomputedAlpha ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Hardware"		, cgShadowMethod::Hardware ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Compare"			, cgShadowMethod::Compare ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Gather"			, cgShadowMethod::Gather ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RAWZ"			, cgShadowMethod::RAWZ ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthReads"		, cgShadowMethod::DepthReads ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "SoftShadows"		, cgShadowMethod::SoftShadows ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ContactHardening", cgShadowMethod::ContactHardening ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "EdgeMask"		, cgShadowMethod::EdgeMask ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "DepthExtentsMask", cgShadowMethod::DepthExtentsMask ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Translucency"	, cgShadowMethod::Translucency ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Jitter"			, cgShadowMethod::Jitter ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Rotate"			, cgShadowMethod::Rotate ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Manual2x2"		, cgShadowMethod::Manual2x2 ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "NormalOffset"	, cgShadowMethod::NormalOffset ) );

            ///////////////////////////////////////////////////////////////////////
            // cgFogModel (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "FogModel";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"              , cgFogModel::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Exponential"       , cgFogModel::Exponential ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "ExponentialSquared", cgFogModel::ExponentialSquared ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Linear"            , cgFogModel::Linear ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAntialiasingMethod (Enum)
            ///////////////////////////////////////////////////////////////////////

            typeName = "AntialiasingMethod";

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"        , cgAntialiasingMethod::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "FXAA"        , cgAntialiasingMethod::FXAA ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "FXAA_T2x"    , cgAntialiasingMethod::FXAA_T2x ) );

            ///////////////////////////////////////////////////////////////////////
            // cgViewport (Struct)
            ///////////////////////////////////////////////////////////////////////
            
            typeName = "Viewport";

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint x"        , offsetof(cgViewport,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint y"        , offsetof(cgViewport,y) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint width"    , offsetof(cgViewport,width) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint height"   , offsetof(cgViewport,height) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float minimumZ", offsetof(cgViewport,minimumZ) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float maximumZ", offsetof(cgViewport,maximumZ) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSamplerStateDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "SamplerStateDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgSamplerStateDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int magnificationFilter", offsetof(cgSamplerStateDesc,magnificationFilter) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int minificationFilter" , offsetof(cgSamplerStateDesc,minificationFilter) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int mipmapFilter"       , offsetof(cgSamplerStateDesc,mipmapFilter) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int addressU"           , offsetof(cgSamplerStateDesc,addressU) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int addressV"           , offsetof(cgSamplerStateDesc,addressV) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int addressW"           , offsetof(cgSamplerStateDesc,addressW) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float mipmapLODBias"    , offsetof(cgSamplerStateDesc,mipmapLODBias) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint maximumAnisotropy" , offsetof(cgSamplerStateDesc,maximumAnisotropy) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int comparisonFunction" , offsetof(cgSamplerStateDesc,comparisonFunction) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "ColorValue borderColor" , offsetof(cgSamplerStateDesc,borderColor) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float minimumMipmapLOD" , offsetof(cgSamplerStateDesc,minimumMipmapLOD) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float maximumMipmapLOD" , offsetof(cgSamplerStateDesc,maximumMipmapLOD) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgTargetBlendStateDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "TargetBlendStateDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgTargetBlendStateDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool blendEnable"             , offsetof(cgTargetBlendStateDesc,blendEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int sourceBlend"              , offsetof(cgTargetBlendStateDesc,sourceBlend) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int destinationBlend"         , offsetof(cgTargetBlendStateDesc,destinationBlend) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int blendOperation"           , offsetof(cgTargetBlendStateDesc,blendOperation) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool separateAlphaBlendEnable", offsetof(cgTargetBlendStateDesc,separateAlphaBlendEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int sourceBlendAlpha"         , offsetof(cgTargetBlendStateDesc,sourceBlendAlpha) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int destinationBlendAlpha"    , offsetof(cgTargetBlendStateDesc,destinationBlendAlpha) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int blendOperationAlpha"      , offsetof(cgTargetBlendStateDesc,blendOperationAlpha) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int renderTargetWriteMask"    , offsetof(cgTargetBlendStateDesc,renderTargetWriteMask) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBlendStateDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "BlendStateDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgBlendStateDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool alphaToCoverageEnable"        , offsetof(cgBlendStateDesc,alphaToCoverageEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool independentBlendEnable"       , offsetof(cgBlendStateDesc,independentBlendEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget0" , offsetof(cgBlendStateDesc,renderTarget[0]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget1" , offsetof(cgBlendStateDesc,renderTarget[1]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget2" , offsetof(cgBlendStateDesc,renderTarget[2]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget3" , offsetof(cgBlendStateDesc,renderTarget[3]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget4" , offsetof(cgBlendStateDesc,renderTarget[4]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget5" , offsetof(cgBlendStateDesc,renderTarget[5]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget6" , offsetof(cgBlendStateDesc,renderTarget[6]) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "TargetBlendStateDesc renderTarget7" , offsetof(cgBlendStateDesc,renderTarget[7]) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgRasterizerStateDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "RasterizerStateDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgRasterizerStateDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int fillMode"              , offsetof(cgRasterizerStateDesc,fillMode) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int cullMode"              , offsetof(cgRasterizerStateDesc,cullMode) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool frontCounterClockwise", offsetof(cgRasterizerStateDesc,frontCounterClockwise) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int depthBias"             , offsetof(cgRasterizerStateDesc,depthBias) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float depthBiasClamp"      , offsetof(cgRasterizerStateDesc,depthBiasClamp) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float slopeScaledDepthBias", offsetof(cgRasterizerStateDesc,slopeScaledDepthBias) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool depthClipEnable"      , offsetof(cgRasterizerStateDesc,depthClipEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool scissorTestEnable"    , offsetof(cgRasterizerStateDesc,scissorTestEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool multisampleEnable"    , offsetof(cgRasterizerStateDesc,multisampleEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool antialiasedLineEnable", offsetof(cgRasterizerStateDesc,antialiasedLineEnable) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDepthStencilOpDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "DepthStencilOpDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgDepthStencilOpDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int stencilFailOperation"     , offsetof(cgDepthStencilOpDesc,stencilFailOperation) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int stencilDepthFailOperation", offsetof(cgDepthStencilOpDesc,stencilDepthFailOperation) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int stencilPassOperation"     , offsetof(cgDepthStencilOpDesc,stencilPassOperation) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int stencilFunction"          , offsetof(cgDepthStencilOpDesc,stencilFunction) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgDepthStencilStateDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "DepthStencilStateDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgDepthStencilStateDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool depthEnable"              , offsetof(cgDepthStencilStateDesc,depthEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool depthWriteEnable"         , offsetof(cgDepthStencilStateDesc,depthWriteEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int depthFunction"             , offsetof(cgDepthStencilStateDesc,depthFunction) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool stencilEnable"            , offsetof(cgDepthStencilStateDesc,stencilEnable) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint8 stencilReadMask"         , offsetof(cgDepthStencilStateDesc,stencilReadMask) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "uint8 stencilWriteMask"        , offsetof(cgDepthStencilStateDesc,stencilWriteMask) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "DepthStencilOpDesc frontFace"  , offsetof(cgDepthStencilStateDesc,frontFace) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "DepthStencilOpDesc backFace"   , offsetof(cgDepthStencilStateDesc,backFace) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgShadowSettings (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "ShadowSettings";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgShadowSettings>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int method"				 , offsetof(cgShadowSettings,method) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int resolutionAdjust"	 , offsetof(cgShadowSettings,resolutionAdjust) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int precision"			 , offsetof(cgShadowSettings,precision) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int cullMode"			 , offsetof(cgShadowSettings,cullMode) ) );

            BINDSUCCESS( engine->registerObjectProperty( typeName, "int primarySamples"      , offsetof(cgShadowSettings,primarySamples) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int secondarySamples"    , offsetof(cgShadowSettings,secondarySamples) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int anisotropy"			 , offsetof(cgShadowSettings,anisotropy) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int msaaSamples"		 , offsetof(cgShadowSettings,msaaSamples) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool jitter"			 , offsetof(cgShadowSettings,jitter) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool rotate"			 , offsetof(cgShadowSettings,rotate) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool boxFilter"			 , offsetof(cgShadowSettings,boxFilter) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool bilinearFiltering"  , offsetof(cgShadowSettings,bilinearFiltering) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool trilinearFiltering" , offsetof(cgShadowSettings,trilinearFiltering) ) );

            BINDSUCCESS( engine->registerObjectProperty( typeName, "int filterPasses"		 , offsetof(cgShadowSettings,filterPasses) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterRadius"      , offsetof(cgShadowSettings,filterRadius) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterBlurFactor"  , offsetof(cgShadowSettings,filterBlurFactor) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterRadiusNear"  , offsetof(cgShadowSettings,filterRadiusNear) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterRadiusFar"   , offsetof(cgShadowSettings,filterRadiusFar) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterDistanceNear", offsetof(cgShadowSettings,filterDistanceNear) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float filterDistanceFar" , offsetof(cgShadowSettings,filterDistanceFar) ) );

            BINDSUCCESS( engine->registerObjectProperty( typeName, "float depthBiasSW"		 , offsetof(cgShadowSettings,depthBiasSW) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float depthBiasHW"		 , offsetof(cgShadowSettings,depthBiasHW) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float slopeScaleBias"    , offsetof(cgShadowSettings,slopeScaleBias) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float NormalBiasSurface" , offsetof(cgShadowSettings,normalBiasSurface) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float NormalBiasLight"   , offsetof(cgShadowSettings,normalBiasLight) ) );

            BINDSUCCESS( engine->registerObjectProperty( typeName, "float minimumVariance"   , offsetof(cgShadowSettings,minimumVariance) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float exponent"			 , offsetof(cgShadowSettings,exponent) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float minimumCutoff"	 , offsetof(cgShadowSettings,minimumCutoff) ) );

            BINDSUCCESS( engine->registerObjectProperty( typeName, "int maskType"			 , offsetof(cgShadowSettings,maskType) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int maskPrecision"		 , offsetof(cgShadowSettings,maskPrecision) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float maskThreshold"     , offsetof(cgShadowSettings,maskThreshold) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBlurOpDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            typeName = "BlurOpDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgBlurOpDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int passCount"                , offsetof(cgBlurOpDesc,passCount) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int pixelRadiusV"             , offsetof(cgBlurOpDesc,pixelRadiusV) ) );
			BINDSUCCESS( engine->registerObjectProperty( typeName, "int pixelRadiusH"             , offsetof(cgBlurOpDesc,pixelRadiusH) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float distanceFactorV"        , offsetof(cgBlurOpDesc,distanceFactorV) ) );
			BINDSUCCESS( engine->registerObjectProperty( typeName, "float distanceFactorH"        , offsetof(cgBlurOpDesc,distanceFactorH) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float worldRadius"            , offsetof(cgBlurOpDesc,worldRadius) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "AlphaWeightMethod inputAlpha" , offsetof(cgBlurOpDesc,inputAlpha) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "AlphaWeightMethod outputAlpha", offsetof(cgBlurOpDesc,outputAlpha) ) );

            // Requires array type for several methods in the image processing interface.
            BINDSUCCESS( engine->registerObjectType( "BlurOpDesc[]", sizeof(std::vector<cgBlurOpDesc>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            STDVectorHelper<cgBlurOpDesc>::registerMethods( engine, "BlurOpDesc[]", "BlurOpDesc" );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::Types