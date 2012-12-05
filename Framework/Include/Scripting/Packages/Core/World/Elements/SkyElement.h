#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Elements/cgSkyElement.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Elements {

// Package declaration
namespace SkyElement
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Elements.SkyElement" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SkyElement", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSkyElement>( engine );

            // Register type identifiers
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_SkyElement", (void*)&RTID_SkyElement ) );

            // Register base class object methods
            Core::World::Elements::SceneElement::registerElementMethods<cgSkyElement>( engine, "SkyElement" );

            // Register methods.
            BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "bool setBaseSampler( Sampler@+ )", asMETHODPR(cgSkyElement, setBaseSampler, ( cgSampler* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "bool setBaseSampler( InputStream, const SamplerStateDesc &in )", asMETHODPR(cgSkyElement, setBaseSampler, ( cgInputStream, const cgSamplerStateDesc& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "void setBaseHDRScale( float )", asMETHODPR(cgSkyElement, setBaseHDRScale, ( cgFloat ), void), asCALL_THISCALL) );
            // TODO: BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "void setSkyType( SkyElementType )", asMETHODPR(cgSkyElement, setSkyType, ( cgSkyElementType::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "Sampler@+ getBaseSampler( )", asMETHODPR(cgSkyElement, getBaseSampler, ( ), cgSampler*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "float getBaseHDRScale( ) const", asMETHODPR(cgSkyElement, getBaseHDRScale, ( ) const, cgFloat), asCALL_THISCALL) );
            // TODO: BINDSUCCESS( engine->registerObjectMethod( "SkyElement", "SkyElementTyoe getSkyType( ) const", asMETHODPR(cgSkyElement, getSkyType, ( ) const, cgSkyElementType::Base ), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Elements::SkyElement