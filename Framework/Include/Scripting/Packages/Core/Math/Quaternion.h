#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Quaternion
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Quaternion" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Quaternion", sizeof(cgQuaternion), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgQuaternion>( engine, "Quaternion" );
            BINDSUCCESS( engine->registerObjectBehavior("Quaternion", asBEHAVE_CONSTRUCT,  "void f(float,float,float,float)", asFUNCTIONPR(constructQuaternion,(cgFloat,cgFloat,cgFloat,cgFloat,cgQuaternion*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion &opAddAssign(const Quaternion &in)", asMETHODPR(cgQuaternion, operator+=, (const cgQuaternion&), cgQuaternion&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion &opSubAssign(const Quaternion &in)", asMETHODPR(cgQuaternion, operator-=, (const cgQuaternion&), cgQuaternion&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion &opMulAssign(const Quaternion &in)", asMETHODPR(cgQuaternion, operator*=, (const cgQuaternion&), cgQuaternion&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion &opDivAssign(float)", asMETHODPR(cgQuaternion, operator/=, (cgFloat), cgQuaternion&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion &opMulAssign(float)", asMETHODPR(cgQuaternion, operator*=, (cgFloat), cgQuaternion&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opAdd(const Quaternion &in) const", asMETHODPR(cgQuaternion, operator+, (const cgQuaternion &) const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opSub(const Quaternion &in) const", asMETHODPR(cgQuaternion, operator-, (const cgQuaternion &) const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opMul(const Quaternion &in) const", asMETHODPR(cgQuaternion, operator*, (const cgQuaternion &) const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opMul(float) const", asMETHODPR(cgQuaternion, operator*, (cgFloat) const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgQuaternion&), cgQuaternion), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opDiv(float) const", asMETHODPR(cgQuaternion, operator/, (cgFloat) const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "bool opEquals(const Quaternion &in) const", asMETHODPR(cgQuaternion, operator==, (const cgQuaternion &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Quaternion", "Quaternion opNeg() const", asMETHODPR(cgQuaternion, operator-, () const, cgQuaternion), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Quaternion", "float x", offsetof(cgQuaternion,x) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Quaternion", "float y", offsetof(cgQuaternion,y) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Quaternion", "float z", offsetof(cgQuaternion,z) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Quaternion", "float w", offsetof(cgQuaternion,w) ) );

            // Global Functions (Quaternion namespace)
            BINDSUCCESS( engine->registerGlobalFunction( "float quaternionLength(const Quaternion &in)", asFUNCTIONPR(cgQuaternion::length, (const cgQuaternion&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float quaternionLengthSq(const Quaternion &in)", asFUNCTIONPR(cgQuaternion::lengthSq, (const cgQuaternion&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "float quaternionDot(const Quaternion &in, const Quaternion &in)", asFUNCTIONPR(cgQuaternion::dot, (const cgQuaternion&,const cgQuaternion&), cgFloat), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionIdentity(Quaternion &inout)", asFUNCTIONPR(cgQuaternion::identity, (cgQuaternion&), cgQuaternion*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "bool quaternionIsIdentity(const Quaternion &in)", asFUNCTIONPR(cgQuaternion::isIdentity, (const cgQuaternion&), bool), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionConjugate(Quaternion &inout, const Quaternion &in)", asFUNCTIONPR(cgQuaternion::conjugate, (cgQuaternion&, const cgQuaternion&), cgQuaternion*), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "void quaternionToAxisAngle(const Quaternion &in, Vector3 &inout, float &inout)", asFUNCTION(cgQuaternion::toAxisAngle), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionRotationMatrix(Quaternion &inout, const Matrix &in)", asFUNCTION(cgQuaternion::rotationMatrix), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionRotationAxis(Quaternion &inout, const Vector3 &in, float)", asFUNCTION(cgQuaternion::rotationAxis), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionRotationYawPitchRoll(Quaternion &inout, float, float, float)", asFUNCTION(cgQuaternion::rotationYawPitchRoll), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionMultiply(Quaternion &inout, const Quaternion &in, const Quaternion &in)", asFUNCTION(cgQuaternion::multiply), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionNormalize(Quaternion &inout, const Quaternion &in)", asFUNCTION(cgQuaternion::normalize), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionInverse(Quaternion &inout, const Quaternion &in)", asFUNCTION(cgQuaternion::inverse), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionLn(Quaternion &inout, const Quaternion &in)", asFUNCTION(cgQuaternion::ln), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionExp(Quaternion &inout, const Quaternion &in)", asFUNCTION(cgQuaternion::exp), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionSlerp(Quaternion &inout, const Quaternion &in, const Quaternion &in, float)", asFUNCTION(cgQuaternion::slerp), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionSquad(Quaternion &inout, const Quaternion &in, const Quaternion &in, const Quaternion &in, const Quaternion &in, float)", asFUNCTION(cgQuaternion::squad), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionSquadSetup(Quaternion &inout, Quaternion &inout, Quaternion &inout, const Quaternion &in, const Quaternion &in, const Quaternion &in, const Quaternion &in)", asFUNCTION(cgQuaternion::squadSetup), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Quaternion& quaternionBaryCentric(Quaternion &inout, const Quaternion &in, const Quaternion &in, const Quaternion &in, float, float)", asFUNCTION(cgQuaternion::baryCentric), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructQuaternion ()
        /// <summary>
        /// This is a wrapper for the default cgQuaternion constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructQuaternion( cgFloat x, cgFloat y, cgFloat z, cgFloat w, cgQuaternion *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgQuaternion( x, y, z, w );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Quaternion