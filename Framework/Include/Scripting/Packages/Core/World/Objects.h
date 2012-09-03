#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Objects/WorldObject.h"
#include "Objects/ObjectNode.h"
#include "Objects/Camera.h"
#include "Objects/Lights.h"
#include "Objects/Group.h"
#include "Objects/Actor.h"
#include "Objects/ObjectBehavior.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Objects
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects" )
            DECLARE_PACKAGE_CHILD( WorldObject )
            DECLARE_PACKAGE_CHILD( ObjectNode )
            DECLARE_PACKAGE_CHILD( ObjectBehavior )
            DECLARE_PACKAGE_CHILD( Camera )
            DECLARE_PACKAGE_CHILD( Lights )
            DECLARE_PACKAGE_CHILD( Group )
            DECLARE_PACKAGE_CHILD( Actor )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Objects