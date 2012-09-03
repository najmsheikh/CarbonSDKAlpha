#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgLightObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Lights {

// Package declaration
namespace Light
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights.Light" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "LightObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "LightNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgLightObject>( engine );
            registerHandleBehaviors<cgLightNode>( engine );

            // ToDo: Finish these types

            ///////////////////////////////////////////////////////////////////////
            // cgLightObject (Class)
            ///////////////////////////////////////////////////////////////////////
            
            ///////////////////////////////////////////////////////////////////////
            // cgLightNode (Class)
            ///////////////////////////////////////////////////////////////////////

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Lights::Light