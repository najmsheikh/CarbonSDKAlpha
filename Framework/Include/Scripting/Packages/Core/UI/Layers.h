#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Layers/UILayer.h"
#include "Layers/ControlLayer.h"
#include "Layers/CursorLayer.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI {

// Package declaration
namespace Layers
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Layers" )
            DECLARE_PACKAGE_CHILD( UILayer )
            DECLARE_PACKAGE_CHILD( ControlLayer )
            DECLARE_PACKAGE_CHILD( CursorLayer )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::UI::Layers