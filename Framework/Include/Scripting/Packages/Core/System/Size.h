#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Size
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Size" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Size", sizeof(cgSize), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * pEngine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgSize>( pEngine, "Size" );

            // Register custom behaviors
            BINDSUCCESS( pEngine->registerObjectBehavior( "Size", asBEHAVE_CONSTRUCT,  "void f(int, int)", asFUNCTIONPR(constructSize,(cgInt32, cgInt32, cgSize*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( pEngine->registerObjectProperty( "Size", "int width", offsetof(cgSize,width) ) );
            BINDSUCCESS( pEngine->registerObjectProperty( "Size", "int height", offsetof(cgSize,height) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructSize ()
        /// <summary>
        /// This is a wrapper for the alternative cgSize constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructSize( cgInt32 x, cgInt32 y, cgSize *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgSize( x, y );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Size