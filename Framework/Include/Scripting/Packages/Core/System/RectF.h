#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace RectF
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.RectF" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RectF", sizeof(cgRectF), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgRectF>( engine, "RectF" );

            // Register custom behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "RectF", asBEHAVE_CONSTRUCT,  "void f(float, float, float, float)", asFUNCTIONPR(constructRectF,(cgFloat, cgFloat, cgFloat, cgFloat, cgRectF*),void), asCALL_CDECL_OBJLAST) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "RectF", "float left", offsetof(cgRectF,left) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RectF", "float top", offsetof(cgRectF,top) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RectF", "float right", offsetof(cgRectF,right) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RectF", "float bottom", offsetof(cgRectF,bottom) ) );

            // Register property methods
            BINDSUCCESS( engine->registerObjectMethod( "RectF", "float get_width() const", asMETHODPR( cgRectF, width, () const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "RectF", "float get_height() const", asMETHODPR( cgRectF, height, () const, cgFloat ), asCALL_THISCALL ) );

            // Register methods
            BINDSUCCESS( engine->registerObjectMethod( "RectF", "bool containsPoint( const PointF&in ) const", asMETHODPR( cgRectF, containsPoint, ( const cgPointF&) const, bool ), asCALL_THISCALL ) );
        }

        //---------------------------------------------------------------------
        //  Name : constructRectF ()
        /// <summary>
        /// This is a wrapper for the alternative cgRectF constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void constructRectF( cgFloat left, cgFloat top, cgFloat right, cgFloat bottom, cgRectF *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgRectF( left, top, right, bottom );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::RectF