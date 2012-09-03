#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Rect
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Rect" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Rect", sizeof(cgRect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgRect>( engine, "Rect" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "Rect", asBEHAVE_CONSTRUCT,  "void f(int, int, int, int)", asFUNCTIONPR(constructRect,(cgInt32, cgInt32, cgInt32, cgInt32, cgRect*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Rect", "int left", offsetof(cgRect,left) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Rect", "int top", offsetof(cgRect,top) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Rect", "int right", offsetof(cgRect,right) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Rect", "int bottom", offsetof(cgRect,bottom) ) );

            // Register property methods
            BINDSUCCESS( engine->registerObjectMethod( "Rect", "int get_width() const", asMETHODPR( cgRect, width, () const, cgInt32 ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "Rect", "int get_height() const", asMETHODPR( cgRect, height, () const, cgInt32 ), asCALL_THISCALL ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructRect ()
        /// <summary>
        /// This is a wrapper for the alternative cgRect constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructRect( cgInt32 left, cgInt32 top, cgInt32 right, cgInt32 bottom, cgRect *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgRect( left, top, right, bottom );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Rect