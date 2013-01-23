#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Animation/cgAnimationTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Animation {

// Package declaration
namespace Types
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Animation.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "AnimationTrackDesc" , sizeof(cgAnimationTrackDesc), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "AnimationPlaybackMode" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgAnimationPlaybackMode (Enum)
            ///////////////////////////////////////////////////////////////////////
            const cgChar * typeName = "AnimationPlaybackMode";

            // Register values
            BINDSUCCESS( engine->registerEnumValue( typeName, "Loop"    , cgAnimationPlaybackMode::Loop ) );
            BINDSUCCESS( engine->registerEnumValue( typeName, "PlayOnce", cgAnimationPlaybackMode::PlayOnce ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgAnimationTrackDesc (Struct)
            ///////////////////////////////////////////////////////////////////////
            typeName = "AnimationTrackDesc";

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgAnimationTrackDesc>( engine, typeName );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( typeName, "AnimationPlaybackMode playbackMode", offsetof(cgAnimationTrackDesc,playbackMode) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "double position"                   , offsetof(cgAnimationTrackDesc,position) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "double length"                     , offsetof(cgAnimationTrackDesc,length) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float weight"                      , offsetof(cgAnimationTrackDesc,weight) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "float speed"                       , offsetof(cgAnimationTrackDesc,speed) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int firstFrame"                    , offsetof(cgAnimationTrackDesc,firstFrame) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "int lastFrame"                     , offsetof(cgAnimationTrackDesc,lastFrame) ) );
            BINDSUCCESS( engine->registerObjectProperty( typeName, "bool enabled"                      , offsetof(cgAnimationTrackDesc,enabled) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Animation::Types