#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace RangeF
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.RangeF" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RangeF", sizeof(cgRangeF), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgRangeF>( engine, "RangeF" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "RangeF", asBEHAVE_CONSTRUCT,  "void f(float, float)", asFUNCTIONPR(constructRangeF,(cgFloat, cgFloat, cgRangeF*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "RangeF", "float min", offsetof(cgRangeF,min) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RangeF", "float max", offsetof(cgRangeF,max) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructRangeF ()
        /// <summary>
        /// This is a wrapper for the alternative cgRangeF constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructRangeF( cgFloat minimum, cgFloat maximum, cgRangeF *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgRangeF( minimum, maximum );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Range