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
#include "Objects/Dummy.h"
#include "Objects/Mesh.h"
#include "Objects/Bone.h"
#include "Objects/ParticleEmitter.h"
#include "Objects/ObjectBehavior.h"
#include "Objects/Navigation.h"
#include "Objects/Billboard.h"

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
            DECLARE_PACKAGE_CHILD( Navigation )
            DECLARE_PACKAGE_CHILD( Group )
            DECLARE_PACKAGE_CHILD( Actor )
            DECLARE_PACKAGE_CHILD( Bone )
            DECLARE_PACKAGE_CHILD( Dummy )
            DECLARE_PACKAGE_CHILD( Mesh )
            DECLARE_PACKAGE_CHILD( ParticleEmitter )
            DECLARE_PACKAGE_CHILD( Billboard )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Objects