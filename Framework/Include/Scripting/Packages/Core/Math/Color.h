#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Color
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Color" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ColorValue", sizeof(cgColorValue), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgColorValue>( engine, "ColorValue" );
            BINDSUCCESS( engine->registerObjectBehavior( "ColorValue", asBEHAVE_CONSTRUCT,  "void f(float,float,float,float)", asFUNCTIONPR(constructColor,(cgFloat,cgFloat,cgFloat,cgFloat,cgColorValue*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "ColorValue", asBEHAVE_CONSTRUCT,  "void f(uint)", asFUNCTIONPR(constructColor,(cgUInt32,cgColorValue*),void), asCALL_CDECL_OBJLAST) );            
            
            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue &opAddAssign(const ColorValue &in)", asMETHODPR(cgColorValue, operator+=, (const cgColorValue&), cgColorValue&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue &opSubAssign(const ColorValue &in)", asMETHODPR(cgColorValue, operator-=, (const cgColorValue&), cgColorValue&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue &opDivAssign(float)", asMETHODPR(cgColorValue, operator/=, (cgFloat), cgColorValue&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue &opMulAssign(float)", asMETHODPR(cgColorValue, operator*=, (cgFloat), cgColorValue&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue opAdd(const ColorValue &in) const", asMETHODPR(cgColorValue, operator+, (const cgColorValue &) const, cgColorValue), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue opSub(const ColorValue &in) const", asMETHODPR(cgColorValue, operator-, (const cgColorValue &) const, cgColorValue), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue opMul(float) const", asMETHODPR(cgColorValue, operator*, (cgFloat) const, cgColorValue), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgColorValue&), cgColorValue), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "ColorValue opDiv(float) const", asMETHODPR(cgColorValue, operator/, (cgFloat) const, cgColorValue), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ColorValue", "bool opEquals(const ColorValue &in) const", asMETHODPR(cgColorValue, operator==, (const cgColorValue &) const, bool), asCALL_THISCALL) );

            // Cast operator overloads
            BINDSUCCESS( engine->registerObjectBehavior( "ColorValue", asBEHAVE_IMPLICIT_VALUE_CAST, "uint f()", asMETHODPR(cgColorValue, operator cgUInt32, () const, cgUInt32), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "ColorValue", "float r", offsetof(cgColorValue,r) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ColorValue", "float g", offsetof(cgColorValue,g) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ColorValue", "float b", offsetof(cgColorValue,b) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ColorValue", "float a", offsetof(cgColorValue,a) ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructColor ()
        /// <summary>
        /// This is a wrapper for the default cgColorValue constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructColor( cgFloat r, cgFloat g, cgFloat b, cgFloat a, cgColorValue *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgColorValue( r, g, b, a );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructColor ()
        /// <summary>
        /// This is a wrapper for the default cgColorValue constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructColor( cgUInt32 color, cgColorValue *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgColorValue( color );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Color