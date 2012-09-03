#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Resources/Types.h"
#include "Resources/Resource.h"
#include "Resources/ResourceManager.h"
#include "Resources/Texture.h"
#include "Resources/AnimationSet.h"
#include "Resources/AudioBuffer.h"
#include "Resources/ConstantBuffer.h"
#include "Resources/DepthStencilTarget.h"
#include "Resources/IndexBuffer.h"
#include "Resources/Materials.h"
#include "Resources/Mesh.h"
#include "Resources/PixelShader.h"
#include "Resources/RenderTarget.h"
#include "Resources/Script.h"
#include "Resources/SurfaceShader.h"
#include "Resources/VertexBuffer.h"
#include "Resources/VertexShader.h"
#include "Resources/StateBlocks.h"
#include "Resources/TexturePool.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Resources
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( Resource )
            DECLARE_PACKAGE_CHILD( ResourceManager )
            DECLARE_PACKAGE_CHILD( Texture )
            DECLARE_PACKAGE_CHILD( AnimationSet )
            DECLARE_PACKAGE_CHILD( AudioBuffer )
            DECLARE_PACKAGE_CHILD( ConstantBuffer )
            DECLARE_PACKAGE_CHILD( DepthStencilTarget )
            DECLARE_PACKAGE_CHILD( IndexBuffer )
            DECLARE_PACKAGE_CHILD( Materials )
            DECLARE_PACKAGE_CHILD( Mesh )
            DECLARE_PACKAGE_CHILD( PixelShader )
            DECLARE_PACKAGE_CHILD( RenderTarget )
            DECLARE_PACKAGE_CHILD( Script )
            DECLARE_PACKAGE_CHILD( SurfaceShader )
            DECLARE_PACKAGE_CHILD( VertexBuffer )
            DECLARE_PACKAGE_CHILD( VertexShader )
            DECLARE_PACKAGE_CHILD( StateBlocks )
            DECLARE_PACKAGE_CHILD( TexturePool )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Resources