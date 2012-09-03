#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgMaterial.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources { namespace Materials {

// Package declaration
namespace Material
{
    //-------------------------------------------------------------------------
    // Name : registerMaterialMethods ()
    // Desc : Register the base cgMaterial class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerMaterialMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Resources::Resource::registerResourceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SurfaceShaderHandle getSurfaceShader( )", asMETHODPR(type, getSurfaceShader, (), cgSurfaceShaderHandle ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SurfaceShaderHandle getSurfaceShader( bool )", asMETHODPR(type, getSurfaceShader, (bool), cgSurfaceShaderHandle ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool setSurfaceShader( const SurfaceShaderHandle &in )", asMETHODPR(type, setSurfaceShader, ( const cgSurfaceShaderHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMaterialProperties( uint64 )", asMETHODPR(type, setMaterialProperties, ( cgUInt64 ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "uint64 getMaterialProperties( ) const", asMETHODPR(type, getMaterialProperties, ( ) const, cgUInt64 ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool addMaterialProperty( const String &in )", asMETHODPR(type, addMaterialProperty, ( const cgString&), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool removeMaterialProperty( const String &in )", asMETHODPR(type, removeMaterialProperty, ( const cgString&), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setName( const String &in )", asMETHODPR(type, setName, ( const cgString & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String & getName( ) const", asMETHODPR(type, getName, ( ) const, const cgString & ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool apply( RenderDriver@+ )", asMETHODPR(type, apply, ( cgRenderDriver* ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void beginPopulate( )", asMETHODPR(type, beginPopulate, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void beginPopulate( ResourceManager@+ )", asMETHODPR(type, beginPopulate, ( cgResourceManager* ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void endPopulate( )", asMETHODPR(type, endPopulate, ( ), void ), asCALL_THISCALL) );
    
    } // End Method registerMaterialMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Materials.Material" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Material", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "MaterialHandle", sizeof(cgMaterialHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgMaterial>( engine );

            // Register the object methods
            registerMaterialMethods<cgMaterial>( engine, "Material" );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgMaterialHandle>( engine, "MaterialHandle", "Material" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Resources::Materials::Material