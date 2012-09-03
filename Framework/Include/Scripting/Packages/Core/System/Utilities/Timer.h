#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgTimer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace Utilities {

// Package declaration
namespace Timer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Utilities.Timer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Timer"  , 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;
            
            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgTimer>( engine );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "void tick()", asMETHODPR(cgTimer, tick, (), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "void tick( float )", asMETHODPR(cgTimer, tick, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "uint getFrameRate( ) const", asMETHODPR(cgTimer, getFrameRate, ( ) const, cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "void setSimulationSpeed( float )", asMETHODPR(cgTimer, setSimulationSpeed, ( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "void incrementFrameCounter( )", asMETHODPR(cgTimer, incrementFrameCounter, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "uint getFrameCounter( ) const", asMETHODPR(cgTimer, getFrameCounter, ( ) const, cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "float getTimeElapsed( ) const", asMETHODPR(cgTimer, getTimeElapsed, ( ) const, cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "double getTime( ) const", asMETHODPR(cgTimer, getTime, ( ) const, cgDouble), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Timer", "double getTime( bool ) const", asMETHODPR(cgTimer, getTime, ( bool ) const, cgDouble), asCALL_THISCALL) );
            
            ///////////////////////////////////////////////////////////////////////
            // Global Functions
            ///////////////////////////////////////////////////////////////////////
            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "Timer@+ getAppTimer( )", asFUNCTIONPR(cgTimer::getInstance, ( ), cgTimer*), asCALL_CDECL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Utilities::Timer