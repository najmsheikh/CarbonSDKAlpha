#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Navigation/cgNavigationTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Navigation {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Navigation.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "NavigationMeshCreateParams" , sizeof(cgNavigationMeshCreateParams), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "NavigationAgentCreateParams", sizeof(cgNavigationAgentCreateParams), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "NavigationRegionType" ) );
            BINDSUCCESS( engine->registerEnum( "NavigationPolyFlags" ) );
            BINDSUCCESS( engine->registerEnum( "NavigationAvoidanceQuality" ) );
            BINDSUCCESS( engine->registerEnum( "NavigationAgentState" ) );
            BINDSUCCESS( engine->registerEnum( "NavigationTargetState" ) );
            
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationRegionType (Enum)
            ///////////////////////////////////////////////////////////////////////
            const cgChar * typeName = "NavigationRegionType";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Ground", cgNavigationRegionType::Ground ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Water" , cgNavigationRegionType::Water ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Road"  , cgNavigationRegionType::Road ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Door"  , cgNavigationRegionType::Door) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Grass" , cgNavigationRegionType::Grass ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Jump"  , cgNavigationRegionType::Jump ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationPolyFlags (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationPolyFlags";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Walk"    , cgNavigationPolyFlags::Walk ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Swim"    , cgNavigationPolyFlags::Swim ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Door"    , cgNavigationPolyFlags::Door ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Jump"    , cgNavigationPolyFlags::Jump ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Disabled", cgNavigationPolyFlags::Disabled ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "All"     , cgNavigationPolyFlags::All ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationAvoidanceQuality (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationAvoidanceQuality";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Low"   , cgNavigationAvoidanceQuality::Low ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Medium", cgNavigationAvoidanceQuality::Medium ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "High"  , cgNavigationAvoidanceQuality::High ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Best"  , cgNavigationAvoidanceQuality::Best ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationAgentState (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationAgentState";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Invalid"       , cgNavigationAgentState::Invalid ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Ready"         , cgNavigationAgentState::Ready ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "TraversingLink", cgNavigationAgentState::TraversingLink ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationTargetState (Enum)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationTargetState";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "None"           , cgNavigationTargetState::None ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Failed"         , cgNavigationTargetState::Failed ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Valid"          , cgNavigationTargetState::Valid ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Requesting"     , cgNavigationTargetState::Requesting ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WaitingForQueue", cgNavigationTargetState::WaitingForQueue ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "WaitingForPath" , cgNavigationTargetState::WaitingForPath ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Velocity"       , cgNavigationTargetState::Velocity ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Arrived"        , cgNavigationTargetState::Arrived ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgNavigationMeshCreateParams (Struct)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationMeshCreateParams";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgNavigationMeshCreateParams>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float cellSize"                , offsetof(cgNavigationMeshCreateParams,cellSize) ) );
	        BINDSUCCESS( engine->registerObjectProperty( typeName, "float cellHeight"              , offsetof(cgNavigationMeshCreateParams,cellHeight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int tileCells"                 , offsetof(cgNavigationMeshCreateParams,tileCells) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentRadius"             , offsetof(cgNavigationMeshCreateParams,agentRadius) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentHeight"             , offsetof(cgNavigationMeshCreateParams,agentHeight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentMaximumSlope"       , offsetof(cgNavigationMeshCreateParams,agentMaximumSlope) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentMaximumStepHeight"  , offsetof(cgNavigationMeshCreateParams,agentMaximumStepHeight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float edgeMaximumLength"       , offsetof(cgNavigationMeshCreateParams,edgeMaximumLength) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float edgeMaximumError"        , offsetof(cgNavigationMeshCreateParams,edgeMaximumError) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float regionMinimumSize"       , offsetof(cgNavigationMeshCreateParams,regionMinimumSize) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float regionMergedSize"        , offsetof(cgNavigationMeshCreateParams,regionMergedSize) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int verticesPerPoly"           , offsetof(cgNavigationMeshCreateParams,verticesPerPoly) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float detailSampleDistance"    , offsetof(cgNavigationMeshCreateParams,detailSampleDistance) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float detailSampleMaximumError", offsetof(cgNavigationMeshCreateParams,detailSampleMaximumError) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgNavigationAgentCreateParams (Struct)
            ///////////////////////////////////////////////////////////////////////
            typeName = "NavigationAgentCreateParams";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgNavigationAgentCreateParams>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentRadius"                          , offsetof(cgNavigationAgentCreateParams,agentRadius) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float agentHeight"                          , offsetof(cgNavigationAgentCreateParams,agentHeight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float slowDownRadius"                       , offsetof(cgNavigationAgentCreateParams,slowDownRadius) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool anticipateTurns"                       , offsetof(cgNavigationAgentCreateParams,anticipateTurns) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool optimizeVisibility"                    , offsetof(cgNavigationAgentCreateParams,optimizeVisibility) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool optimizeTopology"                      , offsetof(cgNavigationAgentCreateParams,optimizeTopology) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool obstacleAvoidance"                     , offsetof(cgNavigationAgentCreateParams,obstacleAvoidance) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool separation"                            , offsetof(cgNavigationAgentCreateParams,separation) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float separationWeight"                     , offsetof(cgNavigationAgentCreateParams,separationWeight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float maximumAcceleration"                  , offsetof(cgNavigationAgentCreateParams,maximumAcceleration) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float maximumSpeed"                         , offsetof(cgNavigationAgentCreateParams,maximumSpeed) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "NavigationAvoidanceQuality avoidanceQuality", offsetof(cgNavigationAgentCreateParams,avoidanceQuality) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Navigation::Types