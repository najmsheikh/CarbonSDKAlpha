#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Vector3
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Vector3" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Vector3", sizeof(cgVector3), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgVector3>( engine, "Vector3" );
            BINDSUCCESS( engine->registerObjectBehavior("Vector3", asBEHAVE_CONSTRUCT,  "void f(float,float,float)", asFUNCTIONPR(constructVector3,(cgFloat,cgFloat,cgFloat,cgVector3*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHODPR(cgVector3, operator+=, (const cgVector3&), cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 &opSubAssign(const Vector3 &in)", asMETHODPR(cgVector3, operator-=, (const cgVector3&), cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(cgVector3, operator/=, (cgFloat), cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(cgVector3, operator*=, (cgFloat), cgVector3&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asMETHODPR(cgVector3, operator+, (const cgVector3 &) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asMETHODPR(cgVector3, operator-, (const cgVector3 &) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opMul(float) const", asMETHODPR(cgVector3, operator*, (cgFloat) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgVector3&), cgVector3), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opDiv(float) const", asMETHODPR(cgVector3, operator/, (cgFloat) const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asMETHODPR(cgVector3, operator==, (const cgVector3 &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Vector3", "Vector3 opNeg() const", asMETHODPR(cgVector3, operator-, () const, cgVector3), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Vector3", "float x", offsetof(cgVector3,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector3", "float y", offsetof(cgVector3,y) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Vector3", "float z", offsetof(cgVector3,z) ) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "float vec3Length(const Vector3 &in)", asFUNCTIONPR(cgVector3::length, (const cgVector3&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec3LengthSq(const Vector3 &in)", asFUNCTIONPR(cgVector3::lengthSq, (const cgVector3&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float vec3Dot(const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::dot, (const cgVector3&,const cgVector3&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Cross(Vector3 &inout, const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::cross, (cgVector3&, const cgVector3&,const cgVector3&), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Add(Vector3 &inout, const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::add, (cgVector3&, const cgVector3&,const cgVector3&), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Subtract(Vector3 &inout, const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::subtract, (cgVector3&, const cgVector3&,const cgVector3&), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Minimize(Vector3 &inout, const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::minimize, (cgVector3&, const cgVector3&,const cgVector3&), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Maximize(Vector3 &inout, const Vector3 &in, const Vector3 &in)", asFUNCTIONPR(cgVector3::maximize, (cgVector3&, const cgVector3&,const cgVector3&), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Scale(Vector3 &inout, const Vector3 &in, float)", asFUNCTIONPR(cgVector3::scale, (cgVector3&, const cgVector3&,cgFloat), cgVector3*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Lerp(Vector3 &inout, const Vector3 &in, const Vector3 &in, float)", asFUNCTIONPR(cgVector3::lerp, (cgVector3&, const cgVector3&,const cgVector3&, cgFloat), cgVector3*), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Normalize(Vector3 &inout, const Vector3 &in)", asFUNCTION(cgVector3::normalize), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Hermite(Vector3 &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in, const Vector3 &in, float)", asFUNCTION(cgVector3::hermite), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3CatmullRom(Vector3 &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in, const Vector3 &in, float)", asFUNCTION(cgVector3::catmullRom), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3BaryCentric(Vector3 &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in, float, float)", asFUNCTION(cgVector3::baryCentric), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "Vector4& vec3Transform(Vector4 &inout, const Vector3 &in, const Matrix &in)", asFUNCTION(cgVector3::transform), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3TransformCoord(Vector3 &inout, const Vector3 &in, const Matrix &in)", asFUNCTION(cgVector3::transformCoord), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3TransformNormal(Vector3 &inout, const Vector3 &in, const Matrix &in)", asFUNCTION(cgVector3::transformNormal), asCALL_CDECL ) );
            //ToDo: BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Project(Vector3 &inout, const Vector3 &in, const Viewport &in, const Matrix &in, const Matrix &in, const Matrix &in)", asFUNCTION(cgVector3::project), asCALL_CDECL ) );
            //ToDo: BINDSUCCESS( engine->registerGlobalFunction( "Vector3& vec3Unproject(Vector3 &inout, const Vector3 &in, const Viewport &in, const Matrix &in, const Matrix &in, const Matrix &in)", asFUNCTION(cgVector3::unproject), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVector3 ()
        /// <summary>
        /// This is a wrapper for the default cgVector3 constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVector3( cgFloat x, cgFloat y, cgFloat z, cgVector3 *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVector3( x, y, z );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Vector3