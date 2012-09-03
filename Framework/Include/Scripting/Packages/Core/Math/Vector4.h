#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Vector4
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Vector4" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Vector4", sizeof(cgVector4), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgVector4>( engine, "Vector4" );
            BINDSUCCESS( engine->registerObjectBehavior("Vector4", asBEHAVE_CONSTRUCT,  "void f(float,float,float,float)", asFUNCTIONPR(constructVector4,(cgFloat,cgFloat,cgFloat,cgFloat,cgVector4*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 &opAddAssign(const Vector4 &in)", asMETHODPR(cgVector4, operator+=, (const cgVector4&), cgVector4&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 &opSubAssign(const Vector4 &in)", asMETHODPR(cgVector4, operator-=, (const cgVector4&), cgVector4&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 &opDivAssign(float)", asMETHODPR(cgVector4, operator/=, (cgFloat), cgVector4&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 &opMulAssign(float)", asMETHODPR(cgVector4, operator*=, (cgFloat), cgVector4&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opAdd(const Vector4 &in) const", asMETHODPR(cgVector4, operator+, (const cgVector4 &) const, cgVector4), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opSub(const Vector4 &in) const", asMETHODPR(cgVector4, operator-, (const cgVector4 &) const, cgVector4), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opMul(float) const", asMETHODPR(cgVector4, operator*, (cgFloat) const, cgVector4), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgVector4&), cgVector4), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opDiv(float) const", asMETHODPR(cgVector4, operator/, (cgFloat) const, cgVector4), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "bool opEquals(const Vector4 &in) const", asMETHODPR(cgVector4, operator==, (const cgVector4 &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector4", "Vector4 opNeg() const", asMETHODPR(cgVector4, operator-, () const, cgVector4), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Vector4", "float x", offsetof(cgVector4,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector4", "float y", offsetof(cgVector4,y) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector4", "float z", offsetof(cgVector4,z) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector4", "float w", offsetof(cgVector4,w) ) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "float vec4Length(const Vector4 &in)", asFUNCTIONPR(cgVector4::length, (const cgVector4&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec4LengthSq(const Vector4 &in)", asFUNCTIONPR(cgVector4::lengthSq, (const cgVector4&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec4Dot(const Vector4 &in, const Vector4 &in)", asFUNCTIONPR(cgVector4::dot, (const cgVector4&,const cgVector4&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Add(Vector4 &inout, const Vector4 &in, const Vector4 &in)", asFUNCTIONPR(cgVector4::add, (cgVector4&, const cgVector4&,const cgVector4&), cgVector4*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Subtract(Vector4 &inout, const Vector4 &in, const Vector4 &in)", asFUNCTIONPR(cgVector4::subtract, (cgVector4&, const cgVector4&,const cgVector4&), cgVector4*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Minimize(Vector4 &inout, const Vector4 &in, const Vector4 &in)", asFUNCTIONPR(cgVector4::minimize, (cgVector4&, const cgVector4&,const cgVector4&), cgVector4*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Maximize(Vector4 &inout, const Vector4 &in, const Vector4 &in)", asFUNCTIONPR(cgVector4::maximize, (cgVector4&, const cgVector4&,const cgVector4&), cgVector4*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Scale(Vector4 &inout, const Vector4 &in, float)", asFUNCTIONPR(cgVector4::scale, (cgVector4&, const cgVector4&,cgFloat), cgVector4*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Lerp(Vector4 &inout, const Vector4 &in, const Vector4 &in, float)", asFUNCTIONPR(cgVector4::lerp, (cgVector4&, const cgVector4&,const cgVector4&, cgFloat), cgVector4*), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Cross(Vector4 &inout, const Vector4 &in, const Vector4 &in, const Vector4 &in)", asFUNCTION(cgVector4::cross), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Normalize(Vector4 &inout, const Vector4 &in)", asFUNCTION(cgVector4::normalize), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Hermite(Vector4 &inout, const Vector4 &in, const Vector4 &in, const Vector4 &in, const Vector4 &in, float)", asFUNCTION(cgVector4::hermite), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4CatmullRom(Vector4 &inout, const Vector4 &in, const Vector4 &in, const Vector4 &in, const Vector4 &in, float)", asFUNCTION(cgVector4::catmullRom), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4BaryCentric(Vector4 &inout, const Vector4 &in, const Vector4 &in, const Vector4 &in, float, float)", asFUNCTION(cgVector4::baryCentric), asCALL_STDCALL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec4Transform(Vector4 &inout, const Vector4 &in, const Matrix &in)", asFUNCTION(cgVector4::transform), asCALL_STDCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVector4 ()
        /// <summary>
        /// This is a wrapper for the default cgVector4 constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVector4( cgFloat x, cgFloat y, cgFloat z, cgFloat w, cgVector4 *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVector4( x, y, z, w );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Vector4