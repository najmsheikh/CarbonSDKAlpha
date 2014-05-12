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
            BINDSUCCESS( engine->registerObjectType( "WorldQuery", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgWorld (Class)
            ///////////////////////////////////////////////////////////////////////

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
			BINDSUCCESS( engine->registerObjectMethod("World", "Scene@+ loadScene( uint, bool )", asMETHODPR(cgWorld,loadScene,(cgUInt32, bool), cgScene*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void unloadScene( uint )", asMETHODPR(cgWorld,unloadScene,(cgUInt32), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void unloadScene( Scene@ )", asMETHODPR(cgWorld,unloadScene,(cgScene*), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod("World", "bool deleteScene( uint )", asMETHODPR(cgWorld,deleteScene,(cgUInt32), bool), asCALL_THISCALL) );
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

            BINDSUCCESS( engine->registerObjectMethod("World", "bool tableExists( const String &in )", asMETHODPR(cgWorld,tableExists,( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "bool componentTablesExist( const UID &in ) const", asMETHODPR(cgWorld,componentTablesExist,( const cgUID& ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "bool executeQuery( const String &in, bool )", asMETHODPR(cgWorld,executeQuery,( const cgString&, bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void beginTransaction( )", asMETHODPR(cgWorld,beginTransaction,( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void beginTransaction( const String &in )", asMETHODPR(cgWorld,beginTransaction,( const cgString& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void commitTransaction( )", asMETHODPR(cgWorld,commitTransaction,( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void commitTransaction( const String &in )", asMETHODPR(cgWorld,commitTransaction,( const cgString& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void rollbackTransaction( )", asMETHODPR(cgWorld,rollbackTransaction,( ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void rollbackTransaction( const String &in )", asMETHODPR(cgWorld,rollbackTransaction,( const cgString& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("World", "void rollbackTransaction( const String &in, bool )", asMETHODPR(cgWorld,rollbackTransaction,( const cgString&, bool ), void ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod("World", "WorldObject@+ loadObject( const UID &in, uint, CloneMethod )", asMETHODPR(cgWorld,loadObject,( const cgUID&, cgUInt32, cgCloneMethod::Base ), cgWorldObject* ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // cgWorldQuery (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgWorldQuery>( engine );

			// Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "WorldQuery", asBEHAVE_FACTORY, "WorldQuery@ f()", asFUNCTIONPR(worldQueryFactory, (), cgWorldQuery*), asCALL_CDECL) );
            BINDSUCCESS( engine->registerObjectBehavior( "WorldQuery", asBEHAVE_FACTORY, "WorldQuery@ f( World@+, const String &in )", asFUNCTIONPR(worldQueryFactory, ( cgWorld *, const cgString& ), cgWorldQuery*), asCALL_CDECL) );

			// Register the object methods
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool prepare( World@+, const String &in, bool )", asMETHODPR(cgWorldQuery,prepare,(cgWorld*,const cgString&, bool), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "void unprepare( )", asMETHODPR(cgWorldQuery,unprepare,(), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool step( )", asMETHODPR(cgWorldQuery,step,(), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool step( bool )", asMETHODPR(cgWorldQuery,step,( bool ), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "void stepAll( )", asMETHODPR(cgWorldQuery,stepAll,(), void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool reset( )", asMETHODPR(cgWorldQuery,reset,(), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool reset( bool )", asMETHODPR(cgWorldQuery,reset,(bool), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool resetCurrent( )", asMETHODPR(cgWorldQuery,resetCurrent,(), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool nextRow( )", asMETHODPR(cgWorldQuery,nextRow,(), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, bool &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, bool&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, bool &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, bool&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, uint32 &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgUInt32&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, uint32 &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgUInt32&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, int32 &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgInt32&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, int32 &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgInt32&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, uint16 &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgUInt16&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, uint16 &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgUInt16&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, int16 &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgInt16&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, int16 &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgInt16&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, float &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgFloat&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, float &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgFloat&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, double &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgDouble&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, double &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgDouble&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( const String &in, String &inout )", asMETHODPR(cgWorldQuery,getColumn,( const cgString&, cgString&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getColumn( int16, String &inout )", asMETHODPR(cgWorldQuery,getColumn,( cgInt16, cgString&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, const String &in )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, const cgString&), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, bool )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, bool), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, float )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgFloat), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, double )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgDouble), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, uint32 )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgUInt32), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, int32 )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgInt32), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, uint16 )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgUInt16), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool bindParameter( int16, int16 )", asMETHODPR(cgWorldQuery,bindParameter,( cgInt16, cgInt16), bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool hasResults( ) const", asMETHODPR(cgWorldQuery,hasResults,( ) const, bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "int32 getLastInsertId( ) const", asMETHODPR(cgWorldQuery,getLastInsertId,( ) const, cgInt32), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getLastError( ) const", asMETHODPR(cgWorldQuery,getLastError,( ) const, bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool getLastError( String &inout ) const", asMETHODPR(cgWorldQuery,getLastError,( cgString& ) const, bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "bool isPrepared( ) const", asMETHODPR(cgWorldQuery,isPrepared,( ) const, bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "WorldQuery", "World@+ getWorld( ) const", asMETHODPR(cgWorldQuery,getWorld,( ) const, cgWorld*), asCALL_THISCALL) );

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

		//---------------------------------------------------------------------
        //  Name : worldQueryFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgWorldQuery class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgWorldQuery * worldQueryFactory( )
        {
            return new cgWorldQuery();
        }

        //---------------------------------------------------------------------
        //  Name : worldQueryFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgWorldQuery class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgWorldQuery * worldQueryFactory( cgWorld * world, const cgString & statements )
        {
            return new cgWorldQuery( world, statements );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::World
