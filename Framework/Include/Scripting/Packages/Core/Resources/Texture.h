#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgTexture.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Texture
{
    //-------------------------------------------------------------------------
    // Name : registerTextureMethods () (Static)
    // Desc : Register the base cgTexture class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerTextureMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::Resources::Resource::registerResourceMethods<type>( engine, typeName );
        
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ImageInfo & getInfo( ) const", asMETHODPR(type,getInfo, () const, const cgImageInfo & ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "Size getSize( ) const", asMETHODPR(type, getSize, ( ) const, cgSize ), asCALL_THISCALL) );
        // ToDo: Note: lock / unlock not currently supported in script (pointer access).
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void update( )", asMETHODPR(type,update, (), void), asCALL_THISCALL) );
        //ToDo: BINDSUCCESS( engine->registerObjectMethod( lpszType, "void ConfigureMedia( const MediaConfig &in )", asMETHODPR(cgTexture, configureMedia, ( const cgTexture::MediaConfig& ), void), asCALL_THISCALL) );
    
    } // End Method registerTextureMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Texture" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Texture", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "TextureHandle", sizeof(cgTextureHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgTexture>( engine );

            // Register the object methods.
            registerTextureMethods<cgTexture>( engine, "Texture" );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgTextureHandle>( engine, "TextureHandle", "Texture" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Texture