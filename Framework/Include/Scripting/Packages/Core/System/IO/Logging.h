#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace IO {

// Package declaration
namespace Logging
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Logging" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "LogEvent" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * pEngine )
        {
            ///////////////////////////////////////////////////////////////////////
            // cgAppLog::EventType (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Normal"  , cgAppLog::Normal ) );
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Internal", cgAppLog::Internal ) );
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Debug"   , cgAppLog::Debug ) );
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Info"    , cgAppLog::Info ) );
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Warning" , cgAppLog::Warning ) );
            BINDSUCCESS( pEngine->registerEnumValue( "LogEvent", "Error"   , cgAppLog::Error ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAppLog (Static Class as Global Functions)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( pEngine->registerGlobalFunction("void logWrite( const String& )", asFUNCTIONPR(logWrite,(const cgString&),void), asCALL_CDECL) );
            BINDSUCCESS( pEngine->registerGlobalFunction("void logWrite( uint, const String& )", asFUNCTIONPR(logWrite,(cgUInt32, const cgString&),void), asCALL_CDECL) );
            BINDSUCCESS( pEngine->registerGlobalFunction("void logBind( const String&in, ?&in )", asFUNCTIONPR(logBind,(const cgString&, void*, int ),void), asCALL_CDECL) );
        }

        //-----------------------------------------------------------------------------
        //  Name : logWrite () (Static)
        /// <summary>
        /// Wrapper for cgAppLog::write that accepts a string reference (not
        /// available in the app log class.)
        /// </summary>
        //-----------------------------------------------------------------------------
        static void logWrite( const cgString & message )
        {
            cgAppLog::write( message.c_str() );
        }

        //-----------------------------------------------------------------------------
        //  Name : logWrite () (Static)
        /// <summary>
        /// Wrapper for cgAppLog::write that accepts a string reference (not
        /// available in the app log class.)
        /// </summary>
        //-----------------------------------------------------------------------------
        static void logWrite( cgUInt32 flags, const cgString & message )
        {
            cgAppLog::write( flags, message.c_str() );
        }

        //-----------------------------------------------------------------------------
        //  Name : logBind () (Static)
        /// <summary>
        /// Bind the specified script object as a log output stream.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void logBind( const cgString& writeMethod, void* a0, int typeId )
        {
            // Get the executing script.
            cgScript * script = (cgScript*)asGetActiveContext()->GetUserData();
            if ( script )
            {
                if ( (typeId & asTYPEID_MASK_OBJECT) )
                {
                    // Should be a reference to an object.
                    asIScriptObject * nativeScriptObject = *(asIScriptObject**)a0;

                    // Build the log output handler.
                    cgScriptObject * scriptObject = new cgScriptObject( script, nativeScriptObject );
                    cgLogOutputScript * output = new cgLogOutputScript( cgString::Empty, cgString::Empty, writeMethod, cgString::Empty, scriptObject );
                    scriptObject->release();

                    // Register it with the logging system.
                    cgAppLog::registerOutput( output );

                } // End if object

            } // End if valid script
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Logging