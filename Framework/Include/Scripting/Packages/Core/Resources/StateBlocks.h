#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "StateBlocks/SamplerState.h"
#include "StateBlocks/RasterizerState.h"
#include "StateBlocks/DepthStencilState.h"
#include "StateBlocks/BlendState.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace StateBlocks
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.StateBlocks" )
            DECLARE_PACKAGE_CHILD( SamplerState )
            DECLARE_PACKAGE_CHILD( RasterizerState )
            DECLARE_PACKAGE_CHILD( DepthStencilState )
            DECLARE_PACKAGE_CHILD( BlendState )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::StateBlocks