#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "IO/Types.h"
#include "IO/InputStream.h"
#include "IO/Logging.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace IO
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.IO" )
            DECLARE_PACKAGE_CHILD( Types )
            DECLARE_PACKAGE_CHILD( InputStream )
            DECLARE_PACKAGE_CHILD( Logging )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            engine->registerGlobalFunction( "String loadStringFromStream( const InputStream &in )", asFUNCTIONPR(cgFileSystem::loadStringFromStream,(const cgInputStream&),cgString), asCALL_CDECL );
            engine->registerGlobalFunction( "String loadStringFromStream( const String &in )", asFUNCTIONPR(loadStringFromStream,(const cgString&),cgString), asCALL_CDECL );
        }

        //---------------------------------------------------------------------
        //  Name : loadStringFromStream ()
        /// <summary>
        /// Wrapper around cgFileSystem::loadStringFromStream() that accepts
        /// a string based filename rather than an input stream.
        /// </summary>
        //---------------------------------------------------------------------
        static cgString loadStringFromStream( const cgString & stream )
        {
            return cgFileSystem::loadStringFromStream( stream );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::IO