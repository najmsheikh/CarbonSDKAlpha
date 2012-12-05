#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgTexturePool.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace TexturePool
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.TexturePool" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "TexturePool", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgTexturePool>( engine );

            // ToDo: 6767 - Register all methods

            // Variable 32 / 64 bit registrations.
            if ( sizeof(size_t) == 4 )
            {
                BINDSUCCESS( engine->registerObjectMethod( "TexturePool", "uint getMemoryConsumption( ) const", asMETHODPR(cgTexturePool,getMemoryConsumption,( ) const, size_t), asCALL_THISCALL) );
            
            } // End if 32bit
            else
            {
                BINDSUCCESS( engine->registerObjectMethod( "TexturePool", "uint64 getMemoryConsumption( ) const", asMETHODPR(cgTexturePool,getMemoryConsumption,( ) const, size_t), asCALL_THISCALL) );

            } // End if 64bit

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::TexturePool
