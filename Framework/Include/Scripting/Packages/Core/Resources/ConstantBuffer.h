#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgConstantBuffer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace ConstantBuffer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.ConstantBuffer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ConstantBuffer", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "ConstantBufferHandle", sizeof(cgConstantBufferHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgConstantBuffer>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgConstantBuffer>( engine, "ConstantBuffer" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "const ConstantBufferDesc & getDesc( ) const", asMETHODPR(cgConstantBuffer,getDesc, () const, const cgConstantBufferDesc& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool getConstantDesc( const String &in, ConstantDesc &inout ) const", asMETHODPR(cgConstantBuffer,getConstantDesc, (const cgString&, cgConstantDesc&) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool getConstantDesc( const String &in, ConstantDesc &inout, uint &inout ) const", asMETHODPR(cgConstantBuffer,getConstantDesc, (const cgString&, cgConstantDesc&, cgUInt32&) const, bool ), asCALL_THISCALL) );
            // ToDo: const cgConstantTypeDesc::Array   & getTypes                ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setFloat( const String &in, float )", asMETHODPR(cgConstantBuffer,setFloat, (const cgString&, float), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setVector( const String &in, const Vector2 &in )", asMETHODPR(cgConstantBuffer,setVector, (const cgString&, const cgVector2&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setVector( const String &in, const Vector3 &in )", asMETHODPR(cgConstantBuffer,setVector, (const cgString&, const cgVector3&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setVector( const String &in, const Vector4 &in )", asMETHODPR(cgConstantBuffer,setVector, (const cgString&, const cgVector4&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setVector( const String &in, const ColorValue &in )", asMETHODPR(cgConstantBuffer,setVector, (const cgString&, const cgVector4&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ConstantBuffer", "bool setMatrix( const String &in, const Matrix &in )", asMETHODPR(cgConstantBuffer,setMatrix, (const cgString&, const cgMatrix&), bool ), asCALL_THISCALL) );
            // ToDo: Note: lock / unlock not currently supported in script (pointer access).

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgConstantBufferHandle>( engine, "ConstantBufferHandle", "ConstantBuffer" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::ConstantBuffer