#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Rendering/Types.h"
#include "Rendering/RenderDriver.h"
#include "Rendering/RenderView.h"
#include "Rendering/ResampleChain.h"
#include "Rendering/Sampler.h"
#include "Rendering/ImageProcessor.h"
#include "Rendering/RenderControl.h"
#include "Rendering/RenderingCapabilities.h"
#include "Rendering/ObjectRenderContext.h"
#include "Rendering/ObjectRenderQueue.h"
#include "Rendering/Processors.h"
#include "Rendering/BillboardBuffer.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Rendering
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( RenderDriver )
            DECLARE_PACKAGE_CHILD( RenderView )
            DECLARE_PACKAGE_CHILD( Sampler )
            DECLARE_PACKAGE_CHILD( RenderControl )
            DECLARE_PACKAGE_CHILD( ResampleChain )
            DECLARE_PACKAGE_CHILD( ImageProcessor )
            DECLARE_PACKAGE_CHILD( ObjectRenderContext )
            DECLARE_PACKAGE_CHILD( ObjectRenderQueue )
            DECLARE_PACKAGE_CHILD( Processors )
			DECLARE_PACKAGE_CHILD( RenderingCapabilities )
            DECLARE_PACKAGE_CHILD( BillboardBuffer )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Rendering