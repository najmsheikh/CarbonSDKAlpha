#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgFileSystem.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace IO {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.IO.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "SeekOrigin" ) );
            BINDSUCCESS( engine->registerEnum( "StreamType" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgStreamType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register enum values
            BINDSUCCESS( engine->registerEnumValue( "StreamType", "None"      , cgStreamType::None ) );
            BINDSUCCESS( engine->registerEnumValue( "StreamType", "File"      , cgStreamType::File ) );
            BINDSUCCESS( engine->registerEnumValue( "StreamType", "MappedFile", cgStreamType::MappedFile ) );
            BINDSUCCESS( engine->registerEnumValue( "StreamType", "Memory"    , cgStreamType::Memory ) );

            ///////////////////////////////////////////////////////////////////////
            // cgInputStream::SeekOrigin (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register enum values
            BINDSUCCESS( engine->registerEnumValue( "SeekOrigin", "Current", cgInputStream::Current ) );
            BINDSUCCESS( engine->registerEnumValue( "SeekOrigin", "Begin"  , cgInputStream::Begin ) );
            BINDSUCCESS( engine->registerEnumValue( "SeekOrigin", "End"    , cgInputStream::End ) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::IO::Types