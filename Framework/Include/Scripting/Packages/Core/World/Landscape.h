#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgLandscape.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Landscape
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Landscape" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Object types
            BINDSUCCESS( engine->registerObjectType( "Landscape", 0, asOBJ_REF ) );
            
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "LandscapeRenderMethod" ) );
            BINDSUCCESS( engine->registerEnum( "LandscapeFlags" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgLandscapeRenderMethod (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "TerrainDepthFill"  , cgLandscapeRenderMethod::TerrainDepthFill ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "TerrainShadowFill" , cgLandscapeRenderMethod::TerrainShadowFill ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "TerrainGBufferFill", cgLandscapeRenderMethod::TerrainGBufferFill ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "TerrainGBufferPost", cgLandscapeRenderMethod::TerrainGBufferPost ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "ClutterDepthFill"  , cgLandscapeRenderMethod::ClutterDepthFill ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeRenderMethod", "ClutterGBufferFill", cgLandscapeRenderMethod::ClutterGBufferFill ) );

            ///////////////////////////////////////////////////////////////////////
            // cgLandscapeFlags (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "LandscapeFlags", "None"             , cgLandscapeFlags::None ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeFlags", "HeightMapResident", cgLandscapeFlags::HeightMapResident ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeFlags", "Dynamic"          , cgLandscapeFlags::Dynamic ) );
            BINDSUCCESS( engine->registerEnumValue( "LandscapeFlags", "LODIgnoreY"       , cgLandscapeFlags::LODIgnoreY ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgLandscape (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgLandscape>( engine );
            
            // ToDo: 9999 - Register base class methods (cgSpatialTree).
            //Core::World::Objects::Object::RegisterObjectMethods<cgLandscape>( engine, "Landscape" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Landscape", "bool renderPass( LandscapeRenderMethod, CameraNode@+, VisibilitySet@+ )", asMETHODPR(cgLandscape,renderPass,( cgLandscapeRenderMethod::Base, cgCameraNode*, cgVisibilitySet* ), bool), asCALL_THISCALL) );
            
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Landscape
