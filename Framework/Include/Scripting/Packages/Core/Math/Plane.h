#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Plane
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Plane" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Plane", sizeof(cgPlane), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgPlane>( engine, "Plane" );
            BINDSUCCESS( engine->registerObjectBehavior("Plane", asBEHAVE_CONSTRUCT,  "void f(float,float,float,float)", asFUNCTIONPR(constructPlane,(cgFloat,cgFloat,cgFloat,cgFloat,cgPlane*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane &opDivAssign(float)", asMETHODPR(cgPlane, operator/=, (cgFloat), cgPlane&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane &opMulAssign(float)", asMETHODPR(cgPlane, operator*=, (cgFloat), cgPlane&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane opMul(float) const", asMETHODPR(cgPlane, operator*, (cgFloat) const, cgPlane), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgPlane&), cgPlane), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane opDiv(float) const", asMETHODPR(cgPlane, operator/, (cgFloat) const, cgPlane), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Plane", "bool opEquals(const Plane &in) const", asMETHODPR(cgPlane, operator==, (const cgPlane &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Plane", "Plane opNeg() const", asMETHODPR(cgPlane, operator-, () const, cgPlane), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Plane", "float a", offsetof(cgPlane,a) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Plane", "float b", offsetof(cgPlane,b) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Plane", "float c", offsetof(cgPlane,c) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Plane", "float d", offsetof(cgPlane,d) ) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "float planeDot(const Plane &in, const Vector4 &in )", asFUNCTIONPR(cgPlane::dot, (const cgPlane&, const cgVector4&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float planeDotCoord(const Plane &in, const Vector3 &in )", asFUNCTIONPR(cgPlane::dotCoord, (const cgPlane&, const cgVector3&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float planeDotNormal(const Plane &in, const Vector3 &in )", asFUNCTIONPR(cgPlane::dotNormal, (const cgPlane&, const cgVector3&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Plane & planeScale(Plane &inout, const Plane &in, float )", asFUNCTIONPR(cgPlane::scale, (cgPlane&, const cgPlane&, cgFloat), cgPlane*), asCALL_CDECL ) );
            
            BINDSUCCESS( engine->registerGlobalFunction( "Plane & planeNormalize(Plane &inout, const Plane &in)", asFUNCTION(cgPlane::normalize), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Vector3 & planeIntersectLine(Vector3 &inout, const Plane &in, const Vector3 &in, const Vector3 &in)", asFUNCTION(cgPlane::intersectLine), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Plane & planeFromPointNormal(Plane &inout, const Vector3 &in, const Vector3 &in)", asFUNCTION(cgPlane::fromPointNormal), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Plane & planeFromPoints(Plane &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in)", asFUNCTION(cgPlane::fromPoints), asCALL_STDCALL ) );
            
            BINDSUCCESS( engine->registerGlobalFunction( "Plane & planeTransform(Plane &inout, const Plane &in, const Matrix &in)", asFUNCTION(cgPlane::transform), asCALL_STDCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructPlane ()
        /// <summary>
        /// This is a wrapper for the default cgPlane constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructPlane( cgFloat a, cgFloat b, cgFloat c, cgFloat d, cgPlane *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgPlane( a, b, c, d );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Plane