#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace PointF
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.PointF" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "PointF", sizeof(cgPointF), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * pEngine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgPointF>( pEngine, "PointF" );

            // Register custom behaviors
            BINDSUCCESS( pEngine->registerObjectBehavior( "PointF", asBEHAVE_CONSTRUCT,  "void f(float, float)", asFUNCTIONPR(constructPointF,(cgFloat, cgFloat, cgPointF*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( pEngine->registerObjectProperty( "PointF", "float x", offsetof(cgPointF,x) ) );
            BINDSUCCESS( pEngine->registerObjectProperty( "PointF", "float y", offsetof(cgPointF,y) ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructPointF ()
        /// <summary>
        /// This is a wrapper for the alternative cgPointF constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructPointF( cgFloat x, cgFloat y, cgPointF *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgPointF( x, y );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::PointF