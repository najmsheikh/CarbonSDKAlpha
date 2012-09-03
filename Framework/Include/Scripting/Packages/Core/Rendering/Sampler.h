#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgSampler.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace Sampler
{
    // Namespace promotion (within parent)
    using namespace cgScriptInterop::Types;

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Sampler" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Sampler", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgSampler (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgSampler>( engine );

            // Register base class object methods
            Core::World::WorldComponent::registerComponentMethods<cgSampler>( engine, "Sampler" );
            
            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool apply( )", asMETHODPR(cgSampler, apply, (), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool apply( const TextureHandle &in )", asMETHODPR(cgSampler, apply, (const cgTextureHandle & ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool apply( uint )", asMETHODPR(cgSampler, apply, ( cgUInt32 ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool apply( uint, const TextureHandle &in )", asMETHODPR(cgSampler, apply, ( cgUInt32, const cgTextureHandle & ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool setTexture( const TextureHandle &in )", asMETHODPR(cgSampler, setTexture, (const cgTextureHandle &), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool loadTexture( const InputStream &in, uint )", asMETHODPR(cgSampler, loadTexture, (const cgInputStream&, cgUInt32), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool loadTexture( const InputStream &in, uint, const DebugSource &in )", asMETHODPR(cgSampler, loadTexture, (const cgInputStream&, cgUInt32, const cgDebugSourceInfo&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool loadTexture( const String &in, uint )", asFUNCTIONPR(samplerLoadTexture, (const cgString&, cgUInt32,cgSampler*), bool ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool loadTexture( const String &in, uint, const DebugSource &in )", asFUNCTIONPR(samplerLoadTexture, (const cgString&, cgUInt32, const cgDebugSourceInfo&, cgSampler*), bool ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "TextureHandle getTexture( )", asMETHODPR(cgSampler, getTexture, (), cgTextureHandle ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "const TextureHandle & getTexture( ) const", asMETHODPR(cgSampler, getTexture, () const, const cgTextureHandle & ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "bool isSystemSampler( ) const", asMETHODPR(cgSampler, isSystemSampler, () const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "const SamplerStateDesc & getStates( ) const", asMETHODPR(cgSampler, getStates, () const, const cgSamplerStateDesc& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setStates( const SamplerStateDesc &in )", asMETHODPR(cgSampler, setStates, (const cgSamplerStateDesc&), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setStrength( float )", asMETHODPR(cgSampler, setStrength, (cgFloat), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "float getStrength( ) const", asMETHODPR(cgSampler, getStrength, () const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "const String & getName( ) const", asMETHODPR(cgSampler, getName, () const, const cgString& ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setAddressU( AddressingMode )", asMETHODPR(cgSampler, setAddressU, ( cgAddressingMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setAddressV( AddressingMode )", asMETHODPR(cgSampler, setAddressV, ( cgAddressingMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setAddressW( AddressingMode )", asMETHODPR(cgSampler, setAddressW, ( cgAddressingMode::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setBorderColor( const ColorValue& )", asMETHODPR(cgSampler, setBorderColor, ( const cgColorValue& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMagnificationFilter( FilterMethod )", asMETHODPR(cgSampler, setMagnificationFilter, ( cgFilterMethod::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMinificationFilter( FilterMethod )", asMETHODPR(cgSampler, setMinificationFilter, ( cgFilterMethod::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMipmapFilter( FilterMethod )", asMETHODPR(cgSampler, setMipmapFilter, ( cgFilterMethod::Base ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMipmapLODBias( float )", asMETHODPR(cgSampler, setMipmapLODBias, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMaximumMipmapLOD( float )", asMETHODPR(cgSampler, setMaximumMipmapLOD, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMinimumMipmapLOD( float )", asMETHODPR(cgSampler, setMinimumMipmapLOD, ( cgFloat ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Sampler", "void setMaximumAnisotropy( int )", asMETHODPR(cgSampler, setMaximumAnisotropy, ( cgInt32 ), void ), asCALL_THISCALL) );
        }

        //---------------------------------------------------------------------
        //  Name : samplerLoadTexture () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Sampler::loadTexture() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool samplerLoadTexture( const cgString & stream, cgUInt32 flags, cgSampler * thisPointer )
        {
            return thisPointer->loadTexture( stream, flags );
        }

        //---------------------------------------------------------------------
        //  Name : samplerLoadTexture () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Sampler::loadTexture() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required 
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool samplerLoadTexture( const cgString & stream, cgUInt32 flags, const cgDebugSourceInfo & _debugSource, cgSampler * thisPointer )
        {
            return thisPointer->loadTexture( stream, flags, _debugSource );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::RenderDriver