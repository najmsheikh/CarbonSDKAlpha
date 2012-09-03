#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Vector2
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Vector2" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Vector2", sizeof(cgVector2), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgVector2>( engine, "Vector2" );
            BINDSUCCESS( engine->registerObjectBehavior("Vector2", asBEHAVE_CONSTRUCT,  "void f(float,float)", asFUNCTIONPR(constructVector2,(cgFloat,cgFloat,cgVector2*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)", asMETHODPR(cgVector2, operator+=, (const cgVector2&), cgVector2&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 &opSubAssign(const Vector2 &in)", asMETHODPR(cgVector2, operator-=, (const cgVector2&), cgVector2&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 &opDivAssign(float)", asMETHODPR(cgVector2, operator/=, (cgFloat), cgVector2&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 &opMulAssign(float)", asMETHODPR(cgVector2, operator*=, (cgFloat), cgVector2&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in) const", asMETHODPR(cgVector2, operator+, (const cgVector2 &) const, cgVector2), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in) const", asMETHODPR(cgVector2, operator-, (const cgVector2 &) const, cgVector2), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opMul(float) const", asMETHODPR(cgVector2, operator*, (cgFloat) const, cgVector2), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgVector2&), cgVector2), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opDiv(float) const", asMETHODPR(cgVector2, operator/, (cgFloat) const, cgVector2), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "bool opEquals(const Vector2 &in) const", asMETHODPR(cgVector2, operator==, (const cgVector2 &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector2", "Vector2 opNeg() const", asMETHODPR(cgVector2, operator-, () const, cgVector2), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Vector2", "float x", offsetof(cgVector2,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector2", "float y", offsetof(cgVector2,y) ) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "float vec2Length(const Vector2 &in)", asFUNCTIONPR(cgVector2::length, (const cgVector2&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec2LengthSq(const Vector2 &in)", asFUNCTIONPR(cgVector2::lengthSq, (const cgVector2&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec2Dot(const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::dot, (const cgVector2&,const cgVector2&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec2CCW(const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::ccw, (const cgVector2&,const cgVector2&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Add(Vector2 &inout, const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::add, (cgVector2&, const cgVector2&,const cgVector2&), cgVector2*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Subtract(Vector2 &inout, const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::subtract, (cgVector2&, const cgVector2&,const cgVector2&), cgVector2*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Minimize(Vector2 &inout, const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::minimize, (cgVector2&, const cgVector2&,const cgVector2&), cgVector2*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Maximize(Vector2 &inout, const Vector2 &in, const Vector2 &in)", asFUNCTIONPR(cgVector2::maximize, (cgVector2&, const cgVector2&,const cgVector2&), cgVector2*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Scale(Vector2 &inout, const Vector2 &in, float)", asFUNCTIONPR(cgVector2::scale, (cgVector2&, const cgVector2&,cgFloat), cgVector2*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Lerp(Vector2 &inout, const Vector2 &in, const Vector2 &in, float)", asFUNCTIONPR(cgVector2::lerp, (cgVector2&, const cgVector2&,const cgVector2&, cgFloat), cgVector2*), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Normalize(Vector2 &inout, const Vector2 &in)", asFUNCTION(cgVector2::normalize), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2Hermite(Vector2 &inout, const Vector2 &in, const Vector2 &in, const Vector2 &in, const Vector2 &in, float)", asFUNCTION(cgVector2::hermite), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2CatmullRom(Vector2 &inout, const Vector2 &in, const Vector2 &in, const Vector2 &in, const Vector2 &in, float)", asFUNCTION(cgVector2::catmullRom), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2BaryCentric(Vector2 &inout, const Vector2 &in, const Vector2 &in, const Vector2 &in, float, float)", asFUNCTION(cgVector2::baryCentric), asCALL_STDCALL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec2Transform(Vector4 &inout, const Vector2 &in, const Matrix &in)", asFUNCTION(cgVector2::transform), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2TransformCoord(Vector2 &inout, const Vector2 &in, const Matrix &in)", asFUNCTION(cgVector2::transformCoord), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector2& vec2TransformNormal(Vector2 &inout, const Vector2 &in, const Matrix &in)", asFUNCTION(cgVector2::transformNormal), asCALL_STDCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVector2 ()
        /// <summary>
        /// This is a wrapper for the default cgVector2 constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVector2( cgFloat x, cgFloat y, cgVector2 *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVector2( x, y );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Vector2