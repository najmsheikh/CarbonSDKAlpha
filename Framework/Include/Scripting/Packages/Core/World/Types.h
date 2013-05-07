#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgWorldTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "SceneCollisionContact", sizeof(cgSceneCollisionContact), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "WorldComponentType" ) );
            BINDSUCCESS( engine->registerEnum( "VisibilitySearchFlags" ) );
            BINDSUCCESS( engine->registerEnum( "SceneProcessStage" ) );
            BINDSUCCESS( engine->registerEnum( "SceneRenderContext" ) );
            BINDSUCCESS( engine->registerEnum( "UpdateRate" ) );
            BINDSUCCESS( engine->registerEnum( "WorldType" ) );
            BINDSUCCESS( engine->registerEnum( "SceneType" ) );
            BINDSUCCESS( engine->registerEnum( "CloneMethod" ) );
            BINDSUCCESS( engine->registerEnum( "ProjectionMode" ) );
            BINDSUCCESS( engine->registerEnum( "TransformSource" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgVisibilitySearchFlags (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "VisibilitySearchFlags", "MustRender"      , cgVisibilitySearchFlags::MustRender ) );
            BINDSUCCESS( engine->registerEnumValue( "VisibilitySearchFlags", "MustCastShadows" , cgVisibilitySearchFlags::MustCastShadows ) );
            BINDSUCCESS( engine->registerEnumValue( "VisibilitySearchFlags", "CollectMaterials", cgVisibilitySearchFlags::CollectMaterials ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSceneProcessStage (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "SceneProcessStage", "None"       , cgSceneProcessStage::None ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneProcessStage", "Precomputed", cgSceneProcessStage::Precomputed ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneProcessStage", "Runtime"    , cgSceneProcessStage::Runtime ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneProcessStage", "Both"       , cgSceneProcessStage::Both ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSceneRenderContext (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "SceneRenderContext", "Runtime"        , cgSceneRenderContext::Runtime ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneRenderContext", "SandboxRender"  , cgSceneRenderContext::SandboxRender ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneRenderContext", "SandboxMaterial", cgSceneRenderContext::SandboxMaterial ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgUpdateRate (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "Never" , cgUpdateRate::Never ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "Always", cgUpdateRate::Always ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS1"  , cgUpdateRate::FPS1 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS2"  , cgUpdateRate::FPS2 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS5"  , cgUpdateRate::FPS5 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS10" , cgUpdateRate::FPS10 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS15" , cgUpdateRate::FPS15 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS20" , cgUpdateRate::FPS20 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS30" , cgUpdateRate::FPS30 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS40" , cgUpdateRate::FPS40 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS50" , cgUpdateRate::FPS50 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS60" , cgUpdateRate::FPS60 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS100", cgUpdateRate::FPS100 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "FPS120", cgUpdateRate::FPS120 ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "NTSC"  , cgUpdateRate::NTSC ) );
            BINDSUCCESS( engine->registerEnumValue( "UpdateRate", "PAL"   , cgUpdateRate::PAL ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgWorldType (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "WorldType", "Master", cgWorldType::Master ) );
            BINDSUCCESS( engine->registerEnumValue( "WorldType", "Merge" , cgWorldType::Merge ) );
            BINDSUCCESS( engine->registerEnumValue( "WorldType", "Data"  , cgWorldType::Data ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSceneType (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "SceneType", "Standard", cgSceneType::Standard ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneType", "Exterior", cgSceneType::Exterior ) );
            BINDSUCCESS( engine->registerEnumValue( "SceneType", "Construct", cgSceneType::Construct ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgCloneMethod (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "CloneMethod", "Copy"          , cgCloneMethod::Copy ) );
            BINDSUCCESS( engine->registerEnumValue( "CloneMethod", "ObjectInstance", cgCloneMethod::ObjectInstance ) );
            BINDSUCCESS( engine->registerEnumValue( "CloneMethod", "DataInstance"  , cgCloneMethod::DataInstance ) );

            ///////////////////////////////////////////////////////////////////////
            // cgProjectionMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "ProjectionMode", "Perspective" , cgProjectionMode::Perspective ) );
            BINDSUCCESS( engine->registerEnumValue( "ProjectionMode", "Orthographic", cgProjectionMode::Orthographic ) );

            ///////////////////////////////////////////////////////////////////////
            // cgTransformSource (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "TransformSource", "Standard", cgTransformSource::Standard ) );
            BINDSUCCESS( engine->registerEnumValue( "TransformSource", "Dynamics", cgTransformSource::Dynamics ) );
            BINDSUCCESS( engine->registerEnumValue( "TransformSource", "Navigation", cgTransformSource::Navigation ) );

            ///////////////////////////////////////////////////////////////////////
            // cgSceneCollisionContact (Struct)
            ///////////////////////////////////////////////////////////////////////
            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgSceneCollisionContact>( engine, "SceneCollisionContact" );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "SceneCollisionContact", "ObjectNode @ node"    , offsetof(cgSceneCollisionContact,node) ) );
            BINDSUCCESS( engine->registerObjectProperty( "SceneCollisionContact", "PhysicsBody @ body"   , offsetof(cgSceneCollisionContact,body) ) );
            BINDSUCCESS( engine->registerObjectProperty( "SceneCollisionContact", "float intersectParam" , offsetof(cgSceneCollisionContact,intersectParam) ) );
            BINDSUCCESS( engine->registerObjectProperty( "SceneCollisionContact", "Vector3 contactNormal", offsetof(cgSceneCollisionContact,contactNormal) ) );
            BINDSUCCESS( engine->registerObjectProperty( "SceneCollisionContact", "int collisionId"      , offsetof(cgSceneCollisionContact,collisionId) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Types