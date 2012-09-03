#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgPropertyContainer.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace PropertyContainer
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.PropertyContainer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType("PropertyContainer", sizeof(cgPropertyContainer), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgPropertyContainer>( engine, "PropertyContainer" );

            // Register the object methods.
            BINDSUCCESS( engine->registerObjectMethod( "PropertyContainer", "bool opEquals(const PropertyContainer &in) const", asMETHODPR(cgPropertyContainer, operator==, (const cgPropertyContainer &) const, bool), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::PropertyContainer