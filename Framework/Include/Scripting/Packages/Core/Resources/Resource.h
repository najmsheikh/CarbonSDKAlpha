#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgResource.h>
#include <World/cgWorldResourceComponent.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Resource
{
    //-------------------------------------------------------------------------
    // Name : registerResourceMethods ()
    // Desc : Register the base cgResource class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerResourceMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ResourceManager@+ getManager( ) const", asMETHODPR(type,getManager,() const,cgResourceManager*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "int removeReference( Reference@+, bool, bool )", asMETHODPR(type, removeReference,( cgReference*, bool, bool), cgInt32 ), asCALL_THISCALL) );        
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const String & getResourceName( ) const", asMETHODPR(type,getResourceName,() const,const cgString&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "const String & getResourceSource( ) const", asMETHODPR(type,getResourceSource,() const,const cgString&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setResourceName( const String &in )", asMETHODPR(type,setResourceName,(const cgString &),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setDestroyDelay( float,bool )", asMETHODPR(type,setDestroyDelay,(cgFloat,bool  ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "ResourceType getResourceType( ) const", asMETHODPR(type,getResourceType,() const,cgResourceType::Base), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool canEvict( ) const", asMETHODPR(type,canEvict,() const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isLoaded( ) const", asMETHODPR(type,isLoaded,( ) const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool isResourceLost( ) const", asMETHODPR(type,isResourceLost,() const,bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void setResourceLost( bool )", asMETHODPR(type,setResourceLost,( bool   ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void resourceTouched( )", asMETHODPR(type,resourceTouched,( ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool loadResource( )", asMETHODPR(type,loadResource,( ),bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool unloadResource( )", asMETHODPR(type,loadResource,( ),bool  ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void deviceLost( )", asMETHODPR(type,deviceLost,( ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "void deviceRestored( )", asMETHODPR(type,deviceRestored,( ),void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool exchangeResource( Resource@+ )", asMETHODPR(type,exchangeResource,( cgResource* ),bool  ), asCALL_THISCALL) );
    
    } // End Method registerResourceMethods<>

    //-------------------------------------------------------------------------
    // Name : registerResourceComponentMethods () (Static)
    // Desc : Register the base cgWorldResourceComponents's class methods. Can
    //        be called by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerResourceComponentMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        registerResourceMethods<type>( engine, typeName );

        // Register object methods.
        BINDSUCCESS( engine->registerObjectMethod(typeName, "World @+ getParentWorld( ) const", asMETHODPR(type,getParentWorld,() const,cgWorld* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "uint getLocalTypeId( ) const", asMETHODPR(type,getLocalTypeId,() const,cgUInt32 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "String getDatabaseTable( ) const", asMETHODPR(type,getDatabaseTable,() const,cgString ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod(typeName, "bool createTypeTables( const UID &in )", asMETHODPR(type,createTypeTables,( const cgUID& ),bool ), asCALL_THISCALL) );
        // ToDo: virtual void                        onComponentModified ( cgComponentModifiedEventArgs * e );
        // ToDo: virtual void                        onComponentCreated  ( cgComponentCreatedEventArgs * e );
        // ToDo: virtual void                        onComponentLoading  ( cgComponentLoadingEventArgs * e );
        // ToDo: virtual void                        onComponentDeleted  ( );

    } // End Method registerResourceComponentMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Resource" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Resource", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "WorldResourceComponent", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgResource>( engine );
            registerHandleBehaviors<cgWorldResourceComponent>( engine );

            // Register the object methods.
            registerResourceMethods<cgResource>( engine, "Resource" );
            registerResourceComponentMethods<cgWorldResourceComponent>( engine, "WorldResourceComponent" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Resource