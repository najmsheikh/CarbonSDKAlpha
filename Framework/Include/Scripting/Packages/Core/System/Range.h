#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Range
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Range" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Range", sizeof(cgRange), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgRange>( engine, "Range" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "Range", asBEHAVE_CONSTRUCT,  "void f(int, int)", asFUNCTIONPR(constructRange,(cgInt32, cgInt32, cgRange*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Range", "int min", offsetof(cgRange,min) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Range", "int max", offsetof(cgRange,max) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructRange ()
        /// <summary>
        /// This is a wrapper for the alternative cgRange constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructRange( cgInt32 minimum, cgInt32 maximum, cgRange *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgRange( minimum, maximum );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Range