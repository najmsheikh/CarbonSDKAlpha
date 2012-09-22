#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgRenderingCapabilities.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

	// Package declaration
	namespace RenderingCapabilities
	{
		// Package descriptor
		class Package : public cgScriptPackage
		{
			BEGIN_SCRIPT_PACKAGE( "Core.Rendering.RenderingCapabilities" )
			END_SCRIPT_PACKAGE( )

			// Type declarations
			void declare( cgScriptEngine * engine )
			{
				BINDSUCCESS( engine->registerObjectType( "RenderingCapabilities", 0, asOBJ_REF ) );
			}

			// Member bindings
			void bind( cgScriptEngine * engine )
			{
				using namespace cgScriptInterop::Utils;

				///////////////////////////////////////////////////////////////////////
				// cgRenderingCapabilities (Class)
				///////////////////////////////////////////////////////////////////////

				// Register the reference/object handle support for the objects
				registerHandleBehaviors<cgRenderingCapabilities>( engine );

				// Register the object methods
				BINDSUCCESS( engine->registerObjectMethod( "RenderingCapabilities", "int getMaxBlendTransforms( ) const", asMETHODPR(cgRenderingCapabilities, getMaxBlendTransforms, () const, cgUInt32), asCALL_THISCALL) );
				BINDSUCCESS( engine->registerObjectMethod( "RenderingCapabilities", "int getMaxAnisotropySamples( ) const", asMETHODPR(cgRenderingCapabilities, getMaxAnisotropySamples, () const, cgUInt32), asCALL_THISCALL) );
				BINDSUCCESS( engine->registerObjectMethod( "RenderingCapabilities", "bool supportsDepthStencilReading( ) const", asMETHODPR(cgRenderingCapabilities, supportsDepthStencilReading, () const, bool), asCALL_THISCALL) );
				BINDSUCCESS( engine->registerObjectMethod( "RenderingCapabilities", "bool supportsFastStencilFill( ) const", asMETHODPR(cgRenderingCapabilities, supportsFastStencilFill, () const, bool), asCALL_THISCALL) );
				BINDSUCCESS( engine->registerObjectMethod( "RenderingCapabilities", "bool supportsNonPow2Textures( ) const", asMETHODPR(cgRenderingCapabilities, supportsNonPow2Textures, () const, bool), asCALL_THISCALL) );
			}

		}; // End Class : Package

	} } } } // End Namespace : cgScriptPackages::Core::Rendering::RenderingCapabilities