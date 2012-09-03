#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgWorldComponent.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace WorldComponent
{
    //-------------------------------------------------------------------------
    // Name : registerComponentMethods ()
    // Desc : Register the base cgWorldComponents's class methods. Can be
    //        called by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerComponentMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );

        // Register object methods.
        BINDSUCCESS( engine->registerObjectMethod(typeName, "World @+ getParentWorld( ) const", asMETHODPR(type,getParentWorld,() const,cgWorld* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "uint getLocalTypeId( ) const", asMETHODPR(type,getLocalTypeId,() const,cgUInt32 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "String getDatabaseTable( ) const", asMETHODPR(type,getDatabaseTable,() const,cgString ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool createTypeTables( const UID &in )", asMETHODPR(type,createTypeTables,( const cgUID& ),bool ), asCALL_THISCALL) );
        // ToDo: virtual void                        onComponentModified ( cgComponentModifiedEventArgs * e );
        // ToDo: virtual void                        onComponentCreated  ( cgComponentCreatedEventArgs * e );
        // ToDo: virtual void                        onComponentLoading  ( cgComponentLoadingEventArgs * e );
        // ToDo: virtual void                        onComponentDeleted  ( );

    } // End Method registerComponentMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.WorldComponent" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "WorldComponent", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgWorldComponent>( engine );

            // Register the object methods
            registerComponentMethods<cgWorldComponent>( engine, "WorldComponent" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::WorldComponent