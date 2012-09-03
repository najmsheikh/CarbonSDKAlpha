#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "UI/Types.h"
#include "UI/UIManager.h"
#include "UI/Layers.h"
#include "UI/Controls.h"
#include "UI/Form.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace UI
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( UIManager )
            DECLARE_PACKAGE_CHILD( Layers )
            DECLARE_PACKAGE_CHILD( Controls )
            DECLARE_PACKAGE_CHILD( Form )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::UI