#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgSceneElement.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Elements {

// Package declaration
namespace SceneElement
{
    //-------------------------------------------------------------------------
    // Name : registerElementMethods ()
    // Desc : Register the base cgSceneElement's class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerElementMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::WorldComponent::registerComponentMethods<type>( engine, typeName );

        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod(typeName, "Scene @+ getParentScene( ) const", asMETHODPR(type,getParentScene,() const,cgScene*), asCALL_THISCALL) );

    } // End Method registerElementMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Elements.SceneElement" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SceneElement", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSceneElement>( engine );

            // Register type identifiers
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_SceneElement", (void*)&RTID_SceneElement ) );

            // Register the object methods for the base class (template method in header)
            registerElementMethods<cgSceneElement>( engine, "SceneElement" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Elements::SceneElement