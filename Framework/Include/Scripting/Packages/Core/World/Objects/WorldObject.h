#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgWorldObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace WorldObject
{
    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgWorldObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::WorldComponent::registerComponentMethods<type>( engine, typeName );

        // Register object methods.
        // Properties
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isRenderable( ) const", asMETHODPR(type,isRenderable,() const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isShadowCaster( ) const", asMETHODPR(type,isShadowCaster,() const,bool  ), asCALL_THISCALL) );
        // ToDo: virtual cgBoundingBox       GetLocalBoundingBox     ( );

        // Rendering
        // ToDo: virtual bool                Render                  ( cgCameraNode * pCamera );
        // ToDo: virtual bool                RenderSubset            ( cgCameraNode * pCamera, const ResourceHandle & hMaterial );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.WorldObject" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "WorldObject", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgWorldObject>( engine );

            // Register the object methods for the base class (template method in header)
            registerObjectMethods<cgWorldObject>( engine, "WorldObject" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::WorldObject