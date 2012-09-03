#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Processors/AntialiasProcessor.h"
#include "Processors/AtmosphericsProcessor.h"
#include "Processors/DepthOfFieldProcessor.h"
#include "Processors/GlareProcessor.h"
#include "Processors/MotionBlurProcessor.h"
#include "Processors/SSAOProcessor.h"
#include "Processors/ToneMapProcessor.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace Processors
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.Processors" )
            DECLARE_PACKAGE_CHILD( AntialiasProcessor )
            DECLARE_PACKAGE_CHILD( AtmosphericsProcessor )
            DECLARE_PACKAGE_CHILD( DepthOfFieldProcessor )
            DECLARE_PACKAGE_CHILD( GlareProcessor )
            DECLARE_PACKAGE_CHILD( MotionBlurProcessor )
            DECLARE_PACKAGE_CHILD( SSAOProcessor )
            DECLARE_PACKAGE_CHILD( ToneMapProcessor )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::Processors