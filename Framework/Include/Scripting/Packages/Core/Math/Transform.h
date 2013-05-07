#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Transform
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Transform" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Transform", sizeof(cgTransform), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgTransform>( engine, "Transform" );
            BINDSUCCESS( engine->registerObjectBehavior("Transform", asBEHAVE_CONSTRUCT,  "void f(const Matrix&in)", asFUNCTIONPR(constructTransform,(const cgMatrix&,cgTransform*),void), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform &opMulAssign(const Transform &in)", asMETHODPR(cgTransform, operator*=, (const cgTransform&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform &opMulAssign(float)", asMETHODPR(cgTransform, operator*=, (cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform &opAddAssign(const Transform &in)", asMETHODPR(cgTransform, operator+=, (const cgTransform&), cgTransform&), asCALL_THISCALL) );
            
            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform opMul(const Transform &in) const", asMETHODPR(cgTransform, operator*, (const cgTransform &) const, cgTransform), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform opMul(float) const", asMETHODPR(cgTransform, operator*, (cgFloat) const, cgTransform), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform opAdd(const Transform &in) const", asMETHODPR(cgTransform, operator+, (const cgTransform &) const, cgTransform), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "bool opEquals(const Transform &in) const", asMETHODPR(cgTransform, operator==, (const cgTransform &) const, bool), asCALL_THISCALL) );

            // Register implicit cast to matrix
            BINDSUCCESS( engine->registerObjectBehavior("Transform", asBEHAVE_IMPLICIT_VALUE_CAST, "const Matrix & f()", asFUNCTIONPR(castTransformToMatrix, (cgTransform*), const cgMatrix&), asCALL_CDECL_OBJLAST) );

            // Register object methods
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 & transformCoord( Vector3 &inout, const Vector3 &in ) const", asMETHODPR(cgTransform, transformCoord, (cgVector3&, const cgVector3&) const, cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 & inverseTransformCoord( Vector3 &inout, const Vector3 &in ) const", asMETHODPR(cgTransform, inverseTransformCoord, (cgVector3&, const cgVector3&) const, cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 & transformNormal( Vector3 &inout, const Vector3 &in ) const", asMETHODPR(cgTransform, transformNormal, (cgVector3&, const cgVector3&) const, cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 & inverseTransformNormal( Vector3 &inout, const Vector3 &in ) const", asMETHODPR(cgTransform, inverseTransformNormal, (cgVector3&, const cgVector3&) const, cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "bool decompose( Vector3 &inout, Vector3 &inout, Quaternion &inout, Vector3 &inout ) const", asMETHODPR(cgTransform, decompose, (cgVector3&, cgVector3&, cgQuaternion&, cgVector3&) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "bool decompose( Vector3 &inout, Quaternion &inout, Vector3 &inout ) const", asMETHODPR(cgTransform, decompose, (cgVector3&, cgQuaternion&, cgVector3&) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "bool decompose( Quaternion &inout, Vector3 &inout ) const", asMETHODPR(cgTransform, decompose, (cgQuaternion&, cgVector3&) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & multiply( Transform &inout, const Transform &in ) const", asMETHODPR(cgTransform, multiply, (cgTransform&, const cgTransform&) const, cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & add( Transform &inout, const Transform &in ) const", asMETHODPR(cgTransform, add, (cgTransform&, const cgTransform&) const, cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & inverse( Transform &inout ) const", asMETHODPR(cgTransform, inverse, (cgTransform&) const, cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & invert( )", asMETHODPR(cgTransform, invert, (), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "const Vector3 & position( ) const", asMETHODPR(cgTransform, position, () const, const cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Quaternion orientation( ) const", asMETHODPR(cgTransform, orientation, () const, cgQuaternion), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 localScale( ) const", asMETHODPR(cgTransform, localScale, () const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "const Vector3 & xAxis( ) const", asMETHODPR(cgTransform, xAxis, () const, const cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "const Vector3 & yAxis( ) const", asMETHODPR(cgTransform, yAxis, () const, const cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "const Vector3 & zAxis( ) const", asMETHODPR(cgTransform, zAxis, () const, const cgVector3&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 normalizedXAxis( ) const", asMETHODPR(cgTransform, xUnitAxis, () const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 normalizedYAxis( ) const", asMETHODPR(cgTransform, yUnitAxis, () const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Vector3 normalizedZAxis( ) const", asMETHODPR(cgTransform, zUnitAxis, () const, cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotate( float, float, float )", asMETHODPR(cgTransform, rotate, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotate( float, float, float, const Vector3 &in )", asMETHODPR(cgTransform, rotate, (cgFloat,cgFloat,cgFloat, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotateAxis( float, const Vector3 &in )", asMETHODPR(cgTransform, rotateAxis, (cgFloat, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotateAxis( float, const Vector3 &in, const Vector3 &in )", asMETHODPR(cgTransform, rotateAxis, (cgFloat, const cgVector3&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotateLocal( float, float, float )", asMETHODPR(cgTransform, rotateLocal, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & scale( float, float, float )", asMETHODPR(cgTransform, scale, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & scale( float, float, float, const Vector3 &in )", asMETHODPR(cgTransform, scale, (cgFloat,cgFloat,cgFloat, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & scaleLocal( float, float, float )", asMETHODPR(cgTransform, scaleLocal, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & scaleLocal( float, float, float, const Vector3 &in )", asMETHODPR(cgTransform, scaleLocal, (cgFloat,cgFloat,cgFloat, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translate( float, float, float )", asMETHODPR(cgTransform, translate, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translate( const Vector3 &in )", asMETHODPR(cgTransform, translate, (const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translateLocal( float, float, float )", asMETHODPR(cgTransform, translateLocal, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translateLocal( const Vector3 &in )", asMETHODPR(cgTransform, translateLocal, (const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setPosition( float, float, float )", asMETHODPR(cgTransform, setPosition, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setPosition( const Vector3 &in )", asMETHODPR(cgTransform, setPosition, (const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setLocalScale( float, float, float )", asMETHODPR(cgTransform, setLocalScale, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setLocalScale( const Vector3 &in )", asMETHODPR(cgTransform, setLocalScale, (const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setLocalShear( float, float, float )", asMETHODPR(cgTransform, setLocalShear, (cgFloat,cgFloat,cgFloat), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setOrientation( const Vector3 &in, const Vector3 &in, const Vector3 &in )", asMETHODPR(cgTransform, setOrientation, (const cgVector3&, const cgVector3&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & setOrientation( const Quaternion &in )", asMETHODPR(cgTransform, setOrientation, (const cgQuaternion&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & identity( )", asMETHODPR(cgTransform, identity, (), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & compose( const Vector3 &in, const Vector3 &in, const Quaternion &in, const Vector3 &in )", asMETHODPR(cgTransform, compose, ( const cgVector3&, const cgVector3&, const cgQuaternion&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & compose( const Quaternion &in, const Vector3 &in )", asMETHODPR(cgTransform, compose, ( const cgQuaternion&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & lookAt( const Vector3 &in, const Vector3 &in )", asMETHODPR(cgTransform, lookAt, ( const cgVector3&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & lookAt( const Vector3 &in, const Vector3 &in, const Vector3 &in )", asMETHODPR(cgTransform, lookAt, ( const cgVector3&, const cgVector3&, const cgVector3&), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & scaling( float, float, float )", asMETHODPR(cgTransform, scaling, ( cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotation( float, float, float )", asMETHODPR(cgTransform, rotation, ( cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & rotationAxis( float, const Vector3 &in )", asMETHODPR(cgTransform, rotationAxis, ( cgFloat, const cgVector3& ), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translation( float, float, float )", asMETHODPR(cgTransform, translation, ( cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "Transform & translation( const Vector3 &in )", asMETHODPR(cgTransform, translation, ( const cgVector3& ), cgTransform&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "int compare( const Transform &in, float ) const", asMETHODPR(cgTransform, compare, ( const cgTransform&, cgFloat ) const, cgInt), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod("Transform", "int compare( const Transform &in ) const", asMETHODPR(cgTransform, compare, ( const cgTransform& ) const, cgInt), asCALL_THISCALL) );

            // Global Functions
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformMultiply( Transform&inout, const Transform& in, const Transform &in)", asFUNCTIONPR(cgTransform::multiply, (cgTransform&, const cgTransform&, const cgTransform&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformAdd( Transform&inout, const Transform& in, const Transform &in)", asFUNCTIONPR(cgTransform::add, (cgTransform&, const cgTransform&, const cgTransform&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformInverse( Transform&inout, const Transform& in)", asFUNCTIONPR(cgTransform::inverse, (cgTransform&, const cgTransform&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformDecompose( Vector3 &inout, Vector3 &inout, Quaternion &inout, Vector3 &inout, const Transform &in )", asFUNCTIONPR(cgTransform::decompose, (cgVector3&, cgVector3&, cgQuaternion&, cgVector3&, const cgTransform&), bool), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformDecompose( Vector3 &inout, Quaternion &inout, Vector3 &inout, const Transform &in )", asFUNCTIONPR(cgTransform::decompose, (cgVector3&, cgQuaternion&, cgVector3&, const cgTransform&), bool), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformDecompose( Quaternion &inout, Vector3 &inout, const Transform &in )", asFUNCTIONPR(cgTransform::decompose, (cgQuaternion&, cgVector3&, const cgTransform&), bool), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformCompose( Transform &inout, const Vector3 &in, const Vector3 &in, const Quaternion &in, const Vector3 &in )", asFUNCTIONPR(cgTransform::compose, (cgTransform&, const cgVector3&, const cgVector3&, const cgQuaternion&, const cgVector3&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformCompose( Transform &inout, const Quaternion &in, const Vector3 &in )", asFUNCTIONPR(cgTransform::compose, (cgTransform&, const cgQuaternion&, const cgVector3&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformIdentity( Transform &inout )", asFUNCTIONPR(cgTransform::identity, (cgTransform&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformLookAt( Transform &inout, const Vector3 &in, const Vector3 &in )", asFUNCTIONPR(cgTransform::lookAt, (cgTransform&, const cgVector3&, const cgVector3&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformLookAt( Transform &inout, const Vector3 &in, const Vector3 &in, const Vector3 &in )", asFUNCTIONPR(cgTransform::lookAt, (cgTransform&, const cgVector3&, const cgVector3&, const cgVector3&), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformScaling( Transform &inout, float, float, float )", asFUNCTIONPR(cgTransform::scaling, (cgTransform&, cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformRotation( Transform &inout, float, float, float )", asFUNCTIONPR(cgTransform::rotation, (cgTransform&, cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformRotationAxis( Transform &inout, float, const Vector3 &in )", asFUNCTIONPR(cgTransform::rotationAxis, (cgTransform&, cgFloat, const cgVector3& ), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformTranslation( Transform &inout, float, float, float )", asFUNCTIONPR(cgTransform::translation, (cgTransform&, cgFloat, cgFloat, cgFloat ), cgTransform&), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "Transform & transformTranslation( Transform &inout, const Vector3 &in )", asFUNCTIONPR(cgTransform::translation, (cgTransform&, const cgVector3 & ), cgTransform&), asCALL_CDECL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructTransform ()
        /// <summary>
        /// This is a wrapper for the default cgTransform constructor, since it
        /// is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructTransform( const cgMatrix & m, cgTransform *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgTransform( m );
        }

        //-----------------------------------------------------------------------------
        //  Name : castTransformToMatrix ()
        /// <summary>
        /// Wrapper for implicit cast from transform to matrix.
        /// </summary>
        //-----------------------------------------------------------------------------
        static const cgMatrix & castTransformToMatrix( cgTransform *thisPointer )
        {
            return (const cgMatrix&)(*thisPointer);
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Transform