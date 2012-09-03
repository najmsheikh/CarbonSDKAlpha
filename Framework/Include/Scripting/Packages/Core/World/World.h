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
            // ToDo
	        //BINDSUCCESS( engine->RegisterObjectMethod("World", "Scene@+ loadScene( const string& )", asMETHOD(cgSceneManager,loadScene), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "void unloadScene( const string& )", asMETHODPR(cgSceneManager,unloadScene,(const CString&),void), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "void unloadScene( Scene@+ )", asMETHODPR(cgSceneManager,unloadScene,(cgScene*),void), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "uint getSceneCount( ) const", asMETHOD(cgSceneManager,getSceneCount), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "SceneDescriptor@+ getSceneDescriptor( uint nIndex )", asMETHOD(cgSceneManager,getSceneDescriptor), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "SceneDescriptor@+ getSceneDescriptorByName( const string& )", asMETHOD(cgSceneManager,getSceneDescriptorByName), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "bool addScene( SceneDescriptor@+ )", asMETHOD(cgSceneManager,addScene), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "bool removeScene( const string& )", asMETHOD(cgSceneManager,removeScene), asCALL_THISCALL) );
            //BINDSUCCESS( engine->RegisterObjectMethod("World", "Scene@+ getLoadedScene( const string& )", asMETHOD(cgSceneManager,getLoadedScene), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::World
