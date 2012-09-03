#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Controls/UIControl.h"
#include "Controls/ComboBox.h"
#include "Controls/GroupBox.h"
#include "Controls/ImageBox.h"
#include "Controls/ListBox.h"
#include "Controls/TextBox.h"
#include "Controls/Button.h"
#include "Controls/Label.h"
#include "Controls/ScrollBar.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI {

// Package declaration
namespace Controls
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Controls" )
            DECLARE_PACKAGE_CHILD( UIControl )
            //DECLARE_PACKAGE_CHILD( ComboBox )
            DECLARE_PACKAGE_CHILD( GroupBox )
            //DECLARE_PACKAGE_CHILD( ImageBox )
            DECLARE_PACKAGE_CHILD( ListBox )
            DECLARE_PACKAGE_CHILD( TextBox )
            DECLARE_PACKAGE_CHILD( Button )
            DECLARE_PACKAGE_CHILD( Label )
            //DECLARE_PACKAGE_CHILD( ScrollBar )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::UI::Controls