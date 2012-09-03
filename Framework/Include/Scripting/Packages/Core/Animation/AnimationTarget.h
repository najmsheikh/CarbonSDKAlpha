#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Animation/cgAnimationTarget.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Animation {

// Package declaration
namespace AnimationTarget
{
    //-------------------------------------------------------------------------
    // Name : registerTargetMethods ()
    // Desc : Register the base cgAnimationTarget class methods. Can be called
    //        by derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerTargetMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::System::References::Reference::registerReferenceMethods<type>( engine, typeName );

        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void onAnimationTransformUpdated( const Transform &in )", asMETHODPR(type, onAnimationTransformUpdated, ( const cgTransform& ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const String & getInstanceIdentifier( ) const", asMETHODPR(type, getInstanceIdentifier, ( ) const, const cgString& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setInstanceIdentifier( const String &in )", asMETHODPR(type, setInstanceIdentifier, ( const cgString& ), void ), asCALL_THISCALL) );
        
    } // End Method registerTargetMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Animation.AnimationTarget" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "AnimationTarget", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgAnimationTarget>( engine );

            // Register the object methods for the base class (template method in header)
            registerTargetMethods<cgAnimationTarget>( engine, "AnimationTarget" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Animation::AnimationTarget
