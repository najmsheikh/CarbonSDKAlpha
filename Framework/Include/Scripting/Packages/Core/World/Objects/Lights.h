#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Lights/Light.h"
#include "Lights/SpotLight.h"
#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/ProjectorLight.h"
#include "Lights/HemisphereLight.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Lights
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights" )
            DECLARE_PACKAGE_CHILD( Light )
            DECLARE_PACKAGE_CHILD( SpotLight )
            DECLARE_PACKAGE_CHILD( PointLight )
            DECLARE_PACKAGE_CHILD( ProjectorLight )
            DECLARE_PACKAGE_CHILD( DirectionalLight )
            DECLARE_PACKAGE_CHILD( HemisphereLight )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Lights