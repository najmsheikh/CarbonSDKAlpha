#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Point
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Point" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Point", sizeof(cgPoint), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgPoint>( engine, "Point" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "Point", asBEHAVE_CONSTRUCT,  "void f(int, int)", asFUNCTIONPR(constructPoint,(cgInt32, cgInt32, cgPoint*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Point", "int x", offsetof(cgPoint,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Point", "int y", offsetof(cgPoint,y) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructPoint ()
        /// <summary>
        /// This is a wrapper for the alternative cgPoint constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructPoint( cgInt32 x, cgInt32 y, cgPoint *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgPoint( x, y );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Point