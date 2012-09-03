#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgDepthStencilTarget.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace DepthStencilTarget
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.DepthStencilTarget" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DepthStencilTarget", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "DepthStencilTargetHandle", sizeof(cgDepthStencilTargetHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgDepthStencilTarget>( engine );

            // Register base / system behaviors.
            Core::Resources::Texture::registerTextureMethods<cgDepthStencilTarget>( engine, "DepthStencilTarget" );

            // Register the object methods
            // ToDo

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethodsEx<cgDepthStencilTargetHandle, cgTextureHandle>( engine, "DepthStencilTargetHandle", "DepthStencilTarget", "TextureHandle" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::DepthStencilTarget