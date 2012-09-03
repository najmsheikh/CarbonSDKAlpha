#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgReferenceManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace References {

// Package declaration
namespace Management
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.References.Management" )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgReferenceManager (Static Class as Global Functions)
            ///////////////////////////////////////////////////////////////////////

            // Static methods implemented as global functions
            BINDSUCCESS( engine->registerGlobalFunction("Reference @+ getReference( uint )", asFUNCTION(cgReferenceManager::getReference), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("void registerReference( Reference@+ )", asFUNCTION(cgReferenceManager::registerReference), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("void unregisterReference( uint, bool )", asFUNCTION(cgReferenceManager::unregisterReference), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("uint generateInternalRefId( )", asFUNCTION(cgReferenceManager::generateInternalRefId), asCALL_CDECL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::References::Management