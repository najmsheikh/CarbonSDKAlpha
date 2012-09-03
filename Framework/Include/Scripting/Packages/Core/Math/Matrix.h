#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Matrix
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Matrix" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Matrix", sizeof(cgMatrix), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgMatrix>( engine, "Matrix" );
            BINDSUCCESS( engine->registerObjectBehavior("Matrix", asBEHAVE_CONSTRUCT,  "void f(float,float,float,float,float,float,float,float,float,float,float,float,float,float,float,float)", asFUNCTIONPR(constructMatrix,(cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgFloat,cgMatrix*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix &opAddAssign(const Matrix &in)", asMETHODPR(cgMatrix, operator+=, (const cgMatrix&), cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix &opSubAssign(const Matrix &in)", asMETHODPR(cgMatrix, operator-=, (const cgMatrix&), cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix &opMulAssign(const Matrix &in)", asMETHODPR(cgMatrix, operator*=, (const cgMatrix&), cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix &opDivAssign(float)", asMETHODPR(cgMatrix, operator/=, (cgFloat), cgMatrix&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix &opMulAssign(float)", asMETHODPR(cgMatrix, operator*=, (cgFloat), cgMatrix&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opAdd(const Matrix &in) const", asMETHODPR(cgMatrix, operator+, (const cgMatrix &) const, cgMatrix), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opSub(const Matrix &in) const", asMETHODPR(cgMatrix, operator-, (const cgMatrix &) const, cgMatrix), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opMul(const Matrix &in) const", asMETHODPR(cgMatrix, operator*, (const cgMatrix &) const, cgMatrix), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opMul(float) const", asMETHODPR(cgMatrix, operator*, (cgFloat) const, cgMatrix), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opMul_r(float) const", asFUNCTIONPR(operator*, (cgFloat, const cgMatrix&), cgMatrix), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opDiv(float) const", asMETHODPR(cgMatrix, operator/, (cgFloat) const, cgMatrix), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "bool opEquals(const Matrix &in) const", asMETHODPR(cgMatrix, operator==, (const cgMatrix &) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Matrix", "Matrix opNeg() const", asMETHODPR(cgMatrix, operator-, () const, cgMatrix), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _11", offsetof(cgMatrix,_11) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _12", offsetof(cgMatrix,_12) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _13", offsetof(cgMatrix,_13) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _14", offsetof(cgMatrix,_14) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _21", offsetof(cgMatrix,_21) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _22", offsetof(cgMatrix,_22) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _23", offsetof(cgMatrix,_23) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _24", offsetof(cgMatrix,_24) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _31", offsetof(cgMatrix,_31) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _32", offsetof(cgMatrix,_32) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _33", offsetof(cgMatrix,_33) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _34", offsetof(cgMatrix,_34) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _41", offsetof(cgMatrix,_41) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _42", offsetof(cgMatrix,_42) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _43", offsetof(cgMatrix,_43) ) );
            BINDSUCCESS( engine->registerObjectProperty( "Matrix", "float _44", offsetof(cgMatrix,_44) ) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixIdentity(Matrix &inout)", asFUNCTIONPR(cgMatrix::identity, (cgMatrix&), cgMatrix*), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "bool matrixIsIdentity(const Matrix &in)", asFUNCTIONPR(cgMatrix::isIdentity, (const cgMatrix&), bool), asCALL_CDECL ) );

            BINDSUCCESS( engine->registerGlobalFunction( "float matrixDeterminant(const Matrix &in)", asFUNCTION(cgMatrix::determinant), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "int32 matrixDecompose(Vector3 &inout, Vector3 &inout, Vector3 &inout, const Matrix &in)", asFUNCTION(cgMatrix::decompose), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixTranspose(Matrix &inout, const Matrix &in)", asFUNCTION(cgMatrix::transpose), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixMultiply(Matrix &inout, const Matrix &in, const Matrix &in)", asFUNCTION(cgMatrix::multiply), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixMultiplyTranspose(Matrix &inout, const Matrix &in, const Matrix &in)", asFUNCTION(cgMatrix::multiply), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixInverse(Matrix &inout, const Matrix &in)", asFUNCTIONPR(cgMatrix::inverse, (cgMatrix&, const cgMatrix&), cgMatrix*), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixInverse(Matrix &inout, float &inout, const Matrix &in)", asFUNCTIONPR(cgMatrix::inverse, (cgMatrix&, cgFloat&, const cgMatrix&), cgMatrix*), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixScaling(Matrix &inout, float, float, float)", asFUNCTION(cgMatrix::scaling), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixTranslation(Matrix &inout, float, float, float)", asFUNCTION(cgMatrix::translation), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationX(Matrix &inout, float)", asFUNCTION(cgMatrix::rotationX), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationY(Matrix &inout, float)", asFUNCTION(cgMatrix::rotationY), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationZ(Matrix &inout, float)", asFUNCTION(cgMatrix::rotationZ), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationAxis(Matrix &inout, const Vector3 &in, float)", asFUNCTION(cgMatrix::rotationAxis), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationQuaternion(Matrix &inout, const Quaternion &in)", asFUNCTION(cgMatrix::rotationQuaternion), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixRotationYawPitchRoll(Matrix &inout, float, float, float)", asFUNCTION(cgMatrix::rotationYawPitchRoll), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixTransformation(Matrix &inout, const Vector3 &in, const Quaternion &in, const Vector3 &in, const Vector3 &in, const Quaternion &in, const Vector3 &in)", asFUNCTION(cgMatrix::transformation), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixTransformation2D(Matrix &inout, const Vector2 &in, float, const Vector2 &in, const Vector2 &in, float, const Vector2 &in)", asFUNCTION(cgMatrix::transformation2D), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixAffineTransformation(Matrix &inout, float, const Vector3 &in, const Quaternion &in, const Vector3 &in)", asFUNCTION(cgMatrix::affineTransformation), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixAffineTransformation2D(Matrix &inout, float, const Vector2 &in, float, const Vector2 &in)", asFUNCTION(cgMatrix::affineTransformation2D), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixLookAtRH(Matrix &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in)", asFUNCTION(cgMatrix::lookAtRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixLookAtLH(Matrix &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in)", asFUNCTION(cgMatrix::lookAtLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveRH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveLH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveFovRH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveFovRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveFovLH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveFovLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveOffCenterRH(Matrix &inout, float, float, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveOffCenterRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixPerspectiveOffCenterLH(Matrix &inout, float, float, float, float, float, float)", asFUNCTION(cgMatrix::perspectiveOffCenterLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixOrthoRH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::orthoRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixOrthoLH(Matrix &inout, float, float, float, float)", asFUNCTION(cgMatrix::orthoLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixOrthoOffCenterRH(Matrix &inout, float, float, float, float, float, float)", asFUNCTION(cgMatrix::orthoOffCenterRH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixOrthoOffCenterLH(Matrix &inout, float, float, float, float, float, float)", asFUNCTION(cgMatrix::orthoOffCenterLH), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixShadow(Matrix &inout, const Vector4 &in, const Plane &in)", asFUNCTION(cgMatrix::shadow), asCALL_STDCALL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Matrix & matrixReflect(Matrix &inout, const Plane &in)", asFUNCTION(cgMatrix::reflect), asCALL_STDCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructMatrix ()
        /// <summary>
        /// This is a wrapper for the default cgMatrix constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructMatrix( cgFloat m11, cgFloat m12, cgFloat m13, cgFloat m14,
                                     cgFloat m21, cgFloat m22, cgFloat m23, cgFloat m24,
                                     cgFloat m31, cgFloat m32, cgFloat m33, cgFloat m34,
                                     cgFloat m41, cgFloat m42, cgFloat m43, cgFloat m44, cgMatrix *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgMatrix( m11, m12, m13, m14,
                                       m21, m22, m23, m24,
                                       m31, m32, m33, m34,
                                       m41, m42, m43, m44 );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Matrix