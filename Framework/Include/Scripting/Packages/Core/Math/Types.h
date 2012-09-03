#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Math/cgMathTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "VolumeQuery" ) );
            BINDSUCCESS( engine->registerEnum( "PlaneQuery" ) );
            BINDSUCCESS( engine->registerEnum( "VolumePlane" ) );
            BINDSUCCESS( engine->registerEnum( "VolumeGeometry" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgVolumeQuery (Enum).
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "VolumeQuery";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "Inside"   , cgVolumeQuery::Inside ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Outside"  , cgVolumeQuery::Outside ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Intersect", cgVolumeQuery::Intersect ) );

            ///////////////////////////////////////////////////////////////////////
            // cgPlaneQuery (Enum).
            ///////////////////////////////////////////////////////////////////////

            typeName = "PlaneQuery";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "Front"   , cgPlaneQuery::Front ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Back"    , cgPlaneQuery::Back ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "On"      , cgPlaneQuery::On ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Spanning", cgPlaneQuery::Spanning ) );

            ///////////////////////////////////////////////////////////////////////
            // cgVolumePlane (Enum).
            ///////////////////////////////////////////////////////////////////////

            typeName = "VolumePlane";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "Left"  , cgVolumePlane::Left ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Right" , cgVolumePlane::Right ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Top"   , cgVolumePlane::Top ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Bottom", cgVolumePlane::Bottom ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Near"  , cgVolumePlane::Near ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "Far"   , cgVolumePlane::Far ) );

            ///////////////////////////////////////////////////////////////////////
            // cgVolumeGeometry (Enum).
            ///////////////////////////////////////////////////////////////////////

            typeName = "VolumeGeometry";

            // Register values.
            BINDSUCCESS( engine->registerEnumValue( typeName, "RightBottomFar" , cgVolumeGeometry::RightBottomFar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RightBottomNear", cgVolumeGeometry::RightBottomNear ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RightTopFar"    , cgVolumeGeometry::RightTopFar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "RightTopNear"   , cgVolumeGeometry::RightTopNear ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LeftBottomFar"  , cgVolumeGeometry::LeftBottomFar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LeftBottomNear" , cgVolumeGeometry::LeftBottomNear ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LeftTopFar"     , cgVolumeGeometry::LeftTopFar ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "LeftTopNear"    , cgVolumeGeometry::LeftTopNear ) );

        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::Types