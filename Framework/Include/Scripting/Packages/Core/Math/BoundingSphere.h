#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Math/cgBoundingSphere.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace BoundingSphere
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.BoundingSphere" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "BoundingSphere", sizeof(cgBoundingSphere), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgBoundingSphere>( engine, "BoundingSphere" );
            BINDSUCCESS( engine->registerObjectBehavior( "BoundingSphere", asBEHAVE_CONSTRUCT,  "void f(const Vector3&, float)", asFUNCTIONPR(constructBoundingSphere,(const cgVector3&, cgFloat, cgBoundingSphere*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "BoundingSphere", asBEHAVE_CONSTRUCT,  "void f(float, float, float, float)", asFUNCTIONPR(constructBoundingSphere,(cgFloat, cgFloat, cgFloat, cgFloat, cgBoundingSphere*),void), asCALL_CDECL_OBJLAST) );
            
            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "BoundingSphere", "BoundingSphere &opAddAssign(const Vector3 &in)", asMETHODPR(cgBoundingSphere, operator+=, (const cgVector3&), cgBoundingSphere&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingSphere", "BoundingSphere &opSubAssign(const Vector3 &in)", asMETHODPR(cgBoundingSphere, operator-=, (const cgVector3&), cgBoundingSphere&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "BoundingSphere", "bool opEquals(const BoundingSphere &in) const", asMETHODPR(cgBoundingSphere, operator==, (const cgBoundingSphere &) const, bool), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "BoundingSphere", "Vector3 position", offsetof(cgBoundingSphere,position) ) );
            BINDSUCCESS( engine->registerObjectProperty( "BoundingSphere", "float radius", offsetof(cgBoundingSphere,radius) ) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "BoundingSphere", "bool containsPoint( const Vector3 &in ) const", asMETHODPR(cgBoundingSphere,containsPoint,( const cgVector3& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingSphere", "bool containsPoint( const Vector3 &in, float ) const", asMETHODPR(cgBoundingSphere,containsPoint,( const cgVector3&, cgFloat ) const, bool ), asCALL_THISCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructBoundingSphere()
        /// <summary>
        /// This is a wrapper for the default cgBoundingSphere constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructBoundingSphere( cgFloat x, cgFloat y, cgFloat z, cgFloat radius, cgBoundingSphere *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgBoundingSphere( x, y, z, radius );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructBoundingSphere ()
        /// <summary>
        /// This is a wrapper for the default cgBoundingSphere constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructBoundingSphere( const cgVector3 & position, cgFloat radius, cgBoundingSphere *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgBoundingSphere( position, radius );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::BoundingSphere