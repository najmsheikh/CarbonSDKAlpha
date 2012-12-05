#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgWorld.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace World
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.World" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "World", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgWorld>( engine );

            // Register base class methods
            Core::System::References::Reference::registerReferenceMethods<cgWorld>( engine, "World" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod("World", "bool open( const InputStream &in )", asMETHODPR(cgWorld,open,(const cgInputStream&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "bool open( const String &in )", asFUNCTIONPR(worldOpen, (const cgString&,cgWorld*), bool ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void close( )", asMETHODPR(cgWorld,close,(), void), asCALL_THISCALL) );
            // ToDo: create
            BINDSUCCESS( engine->registerObjectMethod("World", "bool save( const String &in )", asMETHODPR(cgWorld,save,(const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "Scene@+ loadScene( uint )", asMETHODPR(cgWorld,loadScene,(cgUInt32), cgScene*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void unloadScene( uint )", asMETHODPR(cgWorld,unloadScene,(cgUInt32), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void unloadScene( Scene@ )", asMETHODPR(cgWorld,unloadScene,(cgScene*), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void update( )", asMETHODPR(cgWorld,update,(), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "uint generateRefId( bool )", asMETHODPR(cgWorld,generateRefId,( bool ), cgUInt32), asCALL_THISCALL) );
            // ToDo: getConfiguration
            BINDSUCCESS( engine->registerObjectMethod("World", "uint getSceneCount( ) const", asMETHODPR(cgWorld,getSceneCount,( ) const, cgUInt32), asCALL_THISCALL) );
            // ToDo: getSceneDescriptor
            // ToDo: getSceneDescriptorById
            // ToDo: getSceneDescriptorByName
            // ToDo: updateSceneDescriptorById
            BINDSUCCESS( engine->registerObjectMethod("World", "uint getLoadedSceneCount( ) const", asMETHODPR(cgWorld,getLoadedSceneCount,( ) const, cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "Scene@+ getLoadedScene( uint ) const", asMETHODPR(cgWorld,getLoadedScene,( cgUInt32 ) const, cgScene*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "Scene@+ getLoadedSceneByName( const String &in ) const", asMETHODPR(cgWorld,getLoadedSceneByName,( const cgString& ) const, cgScene*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "Scene@+ getLoadedSceneById( uint ) const", asMETHODPR(cgWorld,getLoadedSceneById,( cgUInt32 ) const, cgScene*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "RenderDriver@+ getRenderDriver( ) const", asMETHODPR(cgWorld,getRenderDriver,( ) const, cgRenderDriver*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "ResourceManager@+ getResourceManager( ) const", asMETHODPR(cgWorld,getResourceManager,( ) const, cgResourceManager*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "bool isSceneLoaded( uint ) const", asMETHODPR(cgWorld,isSceneLoaded,( cgUInt32 ) const, bool), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "World@+ getAppWorld( )", asFUNCTIONPR(cgWorld::getInstance, ( ), cgWorld*), asCALL_CDECL) );
        }

        //---------------------------------------------------------------------
        //  Name : worldOpen () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// World::open() method that allows the script to pass a string type
        /// directly (no implicit cast is supported to the required InputStream
        /// type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool worldOpen( const cgString & stream, cgWorld * thisPointer )
        {
            return thisPointer->open( stream );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::World
