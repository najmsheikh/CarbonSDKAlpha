#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgProfiler.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace Utilities {

// Package declaration
namespace Profiler
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Utilities.Profiler" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types
            BINDSUCCESS( engine->registerObjectType( "ProfilerConfig", sizeof(cgProfiler::InitConfig), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );

            // Object Types
            BINDSUCCESS( engine->registerObjectType( "Profiler"  , 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * pEngine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgProfiler::InitConfig (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register properties
            BINDSUCCESS( pEngine->registerObjectProperty( "ProfilerConfig", "bool broadcastData"     , offsetof(cgProfiler::InitConfig,broadcastData) ) );
            BINDSUCCESS( pEngine->registerObjectProperty( "ProfilerConfig", "uint16 broadcastPort"   , offsetof(cgProfiler::InitConfig,broadcastPort) ) );
            BINDSUCCESS( pEngine->registerObjectProperty( "ProfilerConfig", "float broadcastInterval", offsetof(cgProfiler::InitConfig,broadcastInterval) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgProfiler (Class)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgProfiler>( pEngine );

            // Register the object methods
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "const ProfilerConfig & getConfig() const", asMETHODPR(cgProfiler, getConfig, () const, const cgProfiler::InitConfig&), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "bool initialize()", asMETHODPR(cgProfiler, initialize, (), bool), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void beginFrame()", asMETHODPR(cgProfiler, beginFrame, (), void), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void beginProcess( const String &in )", asMETHODPR(cgProfiler, beginProcess, ( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void beginProcess( const String &in, bool )", asMETHODPR(cgProfiler, beginProcess, ( const cgString&, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void endProcess()", asMETHODPR(cgProfiler, endProcess, (), void), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void endFrame()", asMETHODPR(cgProfiler, endFrame, (), void), asCALL_THISCALL) );
            BINDSUCCESS( pEngine->registerObjectMethod( "Profiler", "void primitivesDrawn( uint )", asMETHODPR(cgProfiler, primitivesDrawn, (cgUInt32), void), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( pEngine->registerGlobalFunction( "Profiler@+ getAppProfiler( )", asFUNCTIONPR(cgProfiler::getInstance, ( ), cgProfiler*), asCALL_CDECL) );
            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Utilities::Profiler