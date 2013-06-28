#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgStandardMaterial.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace Materials {

// Package declaration
namespace StandardMaterial
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Materials.StandardMaterial" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "StandardMaterial", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgStandardMaterial>( engine );

            // Register base / system behaviors
            Core::Resources::Materials::Material::registerMaterialMethods<cgStandardMaterial>( engine, "StandardMaterial" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setMaterialTerms( const MaterialTerms &in )", asMETHODPR(cgStandardMaterial,setMaterialTerms, ( const cgMaterialTerms& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setDiffuse( const ColorValue &in )", asMETHODPR(cgStandardMaterial,setDiffuse, ( const cgColorValue& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setAmbient( const ColorValue &in )", asMETHODPR(cgStandardMaterial,setAmbient, ( const cgColorValue& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setSpecular( const ColorValue &in )", asMETHODPR(cgStandardMaterial,setSpecular, ( const cgColorValue& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setEmissive( const ColorValue &in )", asMETHODPR(cgStandardMaterial,setEmissive, ( const cgColorValue& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setEmissiveHDRScale( float )", asMETHODPR(cgStandardMaterial,setEmissiveHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setGloss( float )", asMETHODPR(cgStandardMaterial,setGloss, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setOpacity( float )", asMETHODPR(cgStandardMaterial,setOpacity, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setSpecularOpacity( float )", asMETHODPR(cgStandardMaterial,setSpecularOpacity, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setSpecularOpacityLinked( bool )", asMETHODPR(cgStandardMaterial,setSpecularOpacityLinked, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setMetalnessTerms( float, float, float )", asMETHODPR(cgStandardMaterial,setMetalnessTerms, ( cgFloat, cgFloat, cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setFresnelTerms( float, float, float, float, float )", asMETHODPR(cgStandardMaterial,setFresnelTerms, ( cgFloat, cgFloat, cgFloat, cgFloat, cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setRimLightTerms( float, float )", asMETHODPR(cgStandardMaterial,setRimLightTerms, ( cgFloat, cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setReflectionMode( ReflectionMode )", asMETHODPR(cgStandardMaterial,setReflectionMode, ( cgReflectionMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setReflectionTerms( float, float, float )", asMETHODPR(cgStandardMaterial,setReflectionTerms, ( cgFloat, cgFloat, cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void setOpacityMapContributions( float, float )", asMETHODPR(cgStandardMaterial,setOpacityMapContributions, ( cgFloat, cgFloat ), void ), asCALL_THISCALL) );
            //void                        setTransmissionCurve        ( const cgBezierSpline2 & curve );
            
            //const cgBezierSpline2     & getTransmissionCurve        ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "const MaterialTerms& getMaterialTerms( ) const", asMETHODPR(cgStandardMaterial,getMaterialTerms, ( ) const, const cgMaterialTerms& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "const ColorValue& getDiffuse( ) const", asMETHODPR(cgStandardMaterial,getDiffuse, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "const ColorValue& getEmissive( ) const", asMETHODPR(cgStandardMaterial,getEmissive, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "const ColorValue& getAmbient( ) const", asMETHODPR(cgStandardMaterial,getAmbient, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "const ColorValue& getSpecular( ) const", asMETHODPR(cgStandardMaterial,getSpecular, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "float getGloss( ) const", asMETHODPR(cgStandardMaterial,getGloss, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "float getOpacity( ) const", asMETHODPR(cgStandardMaterial,getOpacity, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "float getSpecularOpacity( ) const", asMETHODPR(cgStandardMaterial,getSpecularOpacity, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "float getEmissiveHDRScale( ) const", asMETHODPR(cgStandardMaterial,getEmissiveHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "bool isSpecularOpacityLinked( ) const", asMETHODPR(cgStandardMaterial,isSpecularOpacityLinked, ( ) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "ReflectionMode getReflectionMode( ) const", asMETHODPR(cgStandardMaterial,getReflectionMode, ( ) const, cgReflectionMode::Base ), asCALL_THISCALL) );
            //const SamplerArray        & getSamplers                 ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "Sampler@+ getSamplerByName( const String &in )", asMETHODPR(cgStandardMaterial,getSamplerByName, ( const cgString& ), cgSampler* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "Sampler@+ addSampler( const String &in )", asMETHODPR(cgStandardMaterial,addSampler, ( const cgString& ), cgSampler* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "Sampler@+ addSampler( const String &in, const TextureHandle&in )", asMETHODPR(cgStandardMaterial,addSampler, ( const cgString&, const cgTextureHandle& ), cgSampler* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "Sampler@+ addSampler( const String &in, InputStream, uint, const DebugSource&in )", asMETHODPR(cgStandardMaterial,addSampler, ( const cgString&, cgInputStream, cgUInt32, const cgDebugSourceInfo& ), cgSampler* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "bool removeSampler( const String &in )", asMETHODPR(cgStandardMaterial,removeSampler, ( const cgString& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "StandardMaterial", "void applySamplers( )", asMETHODPR(cgStandardMaterial,applySamplers, ( ), void ), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::Materials::StandardMaterial