#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgVariant.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace Variant
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Variant" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value types
            BINDSUCCESS( engine->registerObjectType("Variant", sizeof(cgVariant), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "VariantType" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            ///////////////////////////////////////////////////////////////////////
            // cgVariant::VariantType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values.
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "None", cgVariant::Type_none ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Bool", cgVariant::Type_bool ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Int8", cgVariant::Type_int8 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "UInt8", cgVariant::Type_uint8 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Int32", cgVariant::Type_int32 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "UInt32", cgVariant::Type_uint32 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Int", cgVariant::Type_int32 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "UInt", cgVariant::Type_uint32 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Int64", cgVariant::Type_int64 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "UInt64", cgVariant::Type_uint64 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Float", cgVariant::Type_float ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Vector2", cgVariant::Type_vector2 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Vector3", cgVariant::Type_vector3 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Vector4", cgVariant::Type_vector4 ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "Double", cgVariant::Type_double ) );
            BINDSUCCESS( engine->registerEnumValue( "VariantType", "String", cgVariant::Type_string ) );

            ///////////////////////////////////////////////////////////////////////
            // cgVariant (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the object constructors / destructors
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f()", asFUNCTIONPR(constructVariant,(cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(int8)", asFUNCTIONPR(constructVariant,(cgInt8,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(uint8)", asFUNCTIONPR(constructVariant,(cgUInt8,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(bool)", asFUNCTIONPR(constructVariant,(bool,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(int)", asFUNCTIONPR(constructVariant,(cgInt32,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(uint)", asFUNCTIONPR(constructVariant,(cgUInt32,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(int64)", asFUNCTIONPR(constructVariant,(cgInt64,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(uint64)", asFUNCTIONPR(constructVariant,(cgUInt64,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(float)", asFUNCTIONPR(constructVariant,(cgFloat,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(const Vector2 &in)", asFUNCTIONPR(constructVariant,(const cgVector2&,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(const Vector3 &in)", asFUNCTIONPR(constructVariant,(const cgVector3&,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(const Vector4 &in)", asFUNCTIONPR(constructVariant,(const cgVector4&,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(const String &in)", asFUNCTIONPR(constructVariant,(const cgString&,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_CONSTRUCT,  "void f(double)", asFUNCTIONPR(constructVariant,(cgDouble,cgVariant*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_DESTRUCT,   "void f()", asFUNCTION(destructVariant), asCALL_CDECL_OBJLAST) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(const Variant &in)", asMETHODPR(cgVariant, operator=, (const cgVariant&), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(int8)", asMETHODPR(cgVariant, operator=, (cgInt8), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(uint8)", asMETHODPR(cgVariant, operator=, (cgUInt8), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(bool)", asMETHODPR(cgVariant, operator=, (bool), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(int)", asMETHODPR(cgVariant, operator=, (cgInt32), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(uint)", asMETHODPR(cgVariant, operator=, (cgUInt32), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(int64)", asMETHODPR(cgVariant, operator=, (cgInt64), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(uint64)", asMETHODPR(cgVariant, operator=, (cgUInt64), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(float)", asMETHODPR(cgVariant, operator=, (cgFloat), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(const Vector2 &in)", asMETHODPR(cgVariant, operator=, (const cgVector2&), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(const Vector3 &in)", asMETHODPR(cgVariant, operator=, (const cgVector3&), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(const Vector4 &in)", asMETHODPR(cgVariant, operator=, (const cgVector4&), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(double)", asMETHODPR(cgVariant, operator=, (cgDouble), cgVariant&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "Variant &opAssign(const String &in)", asMETHODPR(cgVariant, operator=, (const cgString&), cgVariant&), asCALL_THISCALL) );
	        BINDSUCCESS( engine->registerObjectMethod( "Variant", "bool opEquals(const Variant &in) const", asMETHODPR(cgVariant, operator==, (const cgVariant &) const, bool), asCALL_THISCALL) );

            // Register cast operators
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "int8 f() const", asMETHODPR(cgVariant,operator cgInt8,() const,cgInt8), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "uint8 f() const", asMETHODPR(cgVariant,operator cgUInt8,() const,cgUInt8), asCALL_THISCALL) );
            // ToDo: asERR_NOTSUPPORTED - BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "bool f() const", asMETHODPR(cgVariant,operator bool,() const,bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "int f() const", asMETHODPR(cgVariant,operator cgInt32,() const,cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "uint f() const", asMETHODPR(cgVariant,operator cgUInt32,() const,cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "int64 f() const", asMETHODPR(cgVariant,operator cgInt64,() const,cgInt64), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "uint64 f() const", asMETHODPR(cgVariant,operator cgUInt64,() const,cgUInt64), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "float f() const", asMETHODPR(cgVariant,operator cgFloat,() const,cgFloat), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "Vector2 f() const", asMETHODPR(cgVariant,operator cgVector2,() const,cgVector2), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "Vector3 f() const", asMETHODPR(cgVariant,operator cgVector3,() const,cgVector3), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "Vector4 f() const", asMETHODPR(cgVariant,operator cgVector4,() const,cgVector4), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "double f() const", asMETHODPR(cgVariant,operator cgDouble,() const,cgDouble), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectBehavior( "Variant", asBEHAVE_IMPLICIT_VALUE_CAST, "String f() const", asMETHODPR(cgVariant,operator cgString,() const,cgString), asCALL_THISCALL) );
            
            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "VariantType getType( ) const", asMETHODPR(cgVariant, getType, () const, cgVariant::VariantType), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Variant", "bool convert( VariantType )", asMETHODPR(cgVariant, convert, ( cgVariant::VariantType ), bool), asCALL_THISCALL) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the default cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant();
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgInt8 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgUInt8 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( bool value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgInt32 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgUInt32 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgInt64 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgUInt64 value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgFloat value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( const cgVector2 & value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( const cgVector3 & value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( const cgVector4 & value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( cgDouble value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructVariant ()
        /// <summary>
        /// This is a wrapper for the alternate cgVariant constructor, since
        /// it is not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructVariant( const cgString & value, cgVariant *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgVariant( value );
        }

        //-----------------------------------------------------------------------------
        //  Name : destructVariant ()
        /// <summary>
        /// A wrapper for the cgVariant destructor. This will be triggered
        /// whenever it goes out of scope inside the script.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void destructVariant( cgVariant * thisPointer )
        {
            thisPointer->~cgVariant();
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::Variant