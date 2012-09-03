#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace DebugSource
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.DebugSource" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "DebugSource", sizeof(cgDebugSourceInfo), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "DebugSource", asBEHAVE_CONSTRUCT,  "void f()", asFUNCTIONPR(constructDebugSource,(cgDebugSourceInfo*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "DebugSource", asBEHAVE_CONSTRUCT,  "void f(const String &in, uint)", asFUNCTIONPR(constructDebugSource,(const cgString&, cgUInt32, cgDebugSourceInfo*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "DebugSource", asBEHAVE_DESTRUCT,  "void f()", asFUNCTIONPR(destructDebugSource,(cgDebugSourceInfo*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "DebugSource", "DebugSource &opAssign(const DebugSource &in)", asMETHODPR(cgDebugSourceInfo, operator=, (const cgDebugSourceInfo&), cgDebugSourceInfo&), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "DebugSource", "String source", offsetof(cgDebugSourceInfo,source) ) );
            BINDSUCCESS( engine->registerObjectProperty( "DebugSource", "uint line", offsetof(cgDebugSourceInfo,line) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructDebugSource ()
        /// <summary>
        /// This is a wrapper for the default cgDebugSourceInfo constructor
        /// since it is not possible to take the address of the constructor
        /// directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructDebugSource( cgDebugSourceInfo *thisPointer )
        {
            cgString executingFile;
            cgScript * script = (cgScript*)asGetActiveContext()->GetUserData();
            if ( script )
                executingFile = script->getResourceName();

            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgDebugSourceInfo( executingFile, asGetActiveContext()->GetLineNumber() );
        }

        //---------------------------------------------------------------------
        //  Name : constructDebugSource ()
        /// <summary>
        /// This is a wrapper for the alternative cgDebugSourceInfo constructor
        /// since it is not possible to take the address of the constructor
        /// directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructDebugSource( const cgString & source, cgUInt32 line, cgDebugSourceInfo *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgDebugSourceInfo( source, line );
        }

        //---------------------------------------------------------------------
        //  Name : destructDebugSource ()
        /// <summary>
        /// A wrapper for the cgDebugSourceInfo destructor. This will be
        /// triggered whenever it goes out of scope inside the script.
        /// </summary>
        //---------------------------------------------------------------------
        static void destructDebugSource( cgDebugSourceInfo * thisPointer )
        {
            thisPointer->~cgDebugSourceInfo();
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::DebugSource