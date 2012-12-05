#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "World/Types.h"
#include "World/WorldComponent.h"
#include "World/World.h"
#include "World/Scene.h"
#include "World/Elements.h"
#include "World/Objects.h"
#include "World/Lighting.h"
#include "World/Visibility.h"
#include "World/Landscape.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace World
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( WorldComponent )
            DECLARE_PACKAGE_CHILD( World )
            DECLARE_PACKAGE_CHILD( Scene )
            DECLARE_PACKAGE_CHILD( Elements )
            DECLARE_PACKAGE_CHILD( Objects )
            DECLARE_PACKAGE_CHILD( Lighting )
            DECLARE_PACKAGE_CHILD( Visibility )
            DECLARE_PACKAGE_CHILD( Landscape )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::World