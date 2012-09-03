#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgRenderTarget.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace RenderTarget
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.RenderTarget" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RenderTarget", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "RenderTargetHandle", sizeof(cgRenderTargetHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgRenderTarget>( engine );

            // Register base / system behaviors.
            Core::Resources::Texture::registerTextureMethods<cgRenderTarget>( engine, "RenderTarget" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethodsEx<cgRenderTargetHandle, cgTextureHandle>( engine, "RenderTargetHandle", "RenderTarget", "TextureHandle" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::RenderTarget