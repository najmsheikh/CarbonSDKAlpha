#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgVisibilitySet.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Visibility {

// Package declaration
namespace VisibilitySet
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Visibility.VisibilitySet" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "VisibilitySet", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgVisibilitySet>( engine );

            // ToDo: Need factory to instantiate custom visibility sets?

            // Register the object methods
            // BINDSUCCESS( engine->RegisterObjectMethod( "VisibilitySet", "void update( )", asMETHODPR(cgVisibilitySet,update,(),void), asCALL_THISCALL) );

            // ToDo:
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Visibility::VisibilitySet
