#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace SizeF
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.SizeF" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SizeF", sizeof(cgSizeF), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgSizeF>( engine, "SizeF" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "SizeF", asBEHAVE_CONSTRUCT,  "void f(float, float)", asFUNCTIONPR(constructSizeF,(cgFloat, cgFloat, cgSizeF*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "SizeF", "float width", offsetof(cgSizeF,width) ) );
            BINDSUCCESS( engine->registerObjectProperty( "SizeF", "float height", offsetof(cgSizeF,height) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructSizeF ()
        /// <summary>
        /// This is a wrapper for the alternative cgSizeF constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructSizeF( cgFloat x, cgFloat y, cgSizeF *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgSizeF( x, y );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::SizeF