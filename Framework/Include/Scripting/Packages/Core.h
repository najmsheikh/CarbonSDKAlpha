#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Core/System.h"
#include "Core/Math.h"
#include "Core/Animation.h"
#include "Core/World.h"
#include "Core/Rendering.h"
#include "Core/Audio.h"
#include "Core/Navigation.h"
#include "Core/Physics.h"
#include "Core/Input.h"
#include "Core/Resources.h"
#include "Core/UI.h"
#include "Core/States.h"

// Parent hierarchy
namespace cgScriptPackages {

// Package declaration
namespace Core {

    //-------------------------------------------------------------------------
    // Module Local Types
    //-------------------------------------------------------------------------
    // Provides an alternative overload for certain script accessible methods 
    // that allow the script to pass 'null'.
    class NullHandle : public cgScriptInterop::DisposableScriptObject
    {
        DECLARE_SCRIPTOBJECT( NullHandle, "NullHandle" )
    };

    // Package descriptor
    class Package : public cgScriptPackage
    {
        // Package description
        BEGIN_SCRIPT_PACKAGE( "Core" )
            DECLARE_PACKAGE_CHILD( System )
            DECLARE_PACKAGE_CHILD( Math )
            DECLARE_PACKAGE_CHILD( Animation )
            DECLARE_PACKAGE_CHILD( Resources )
            DECLARE_PACKAGE_CHILD( Rendering )
            DECLARE_PACKAGE_CHILD( Input )
            DECLARE_PACKAGE_CHILD( Audio )
            DECLARE_PACKAGE_CHILD( Physics )
            DECLARE_PACKAGE_CHILD( Navigation )
            DECLARE_PACKAGE_CHILD( World )
            DECLARE_PACKAGE_CHILD( UI )
            DECLARE_PACKAGE_CHILD( States )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Object Types
            BINDSUCCESS( engine->registerObjectType( "NullHandle", 0, asOBJ_REF ) );
            
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "ConfigResult" ) );

            // Register standard templated array types for *all* angelscript
            // supported primitive data types. This is necessary for the surface
            // shader permutation selection logic in which arrays will be expanded.
            // No valid primitive array types should ever use the default array template.
            ArrayBindHelper<bool, true>::registerType( engine, "bool[]", "bool" );
            ArrayBindHelper<cgByte>::registerType( engine, "uint8[]", "uint8" );
            ArrayBindHelper<cgInt8>::registerType( engine, "int8[]", "int8" );
            ArrayBindHelper<cgUInt16>::registerType( engine, "uint16[]", "uint16" );
            ArrayBindHelper<cgInt16>::registerType( engine, "int16[]", "int16" );
            ArrayBindHelper<cgUInt32>::registerType( engine, "uint[]", "uint" );
            ArrayBindHelper<cgInt32>::registerType( engine, "int[]", "int" );
            ArrayBindHelper<cgUInt64>::registerType( engine, "uint64[]", "uint64" );
            ArrayBindHelper<cgInt64>::registerType( engine, "int64[]", "int64" );
            ArrayBindHelper<cgFloat>::registerType( engine, "float[]", "float" );
            ArrayBindHelper<cgDouble>::registerType( engine, "double[]", "double" ); 
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // NullHandle (Class)
            ///////////////////////////////////////////////////////////////////////
            registerHandleBehaviors<NullHandle>( engine );

            ///////////////////////////////////////////////////////////////////////
            // cgConfigResult (Enum)
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerEnumValue( "ConfigResult", "Valid"   , cgConfigResult::Valid ) );
            BINDSUCCESS( engine->registerEnumValue( "ConfigResult", "Mismatch", cgConfigResult::Mismatch ) );
            BINDSUCCESS( engine->registerEnumValue( "ConfigResult", "Error"   , cgConfigResult::Error ) );
            
            // Register standard templated array types for *all* angelscript
            // supported primitive data types. This is necessary for the surface
            // shader permutation selection logic in which arrays will be expanded.
            // No valid primitive array types should ever use the default array template.
            ArrayBindHelper<bool>::registerMethods( engine, "bool[]", "bool" );
            ArrayBindHelper<cgByte>::registerMethods( engine, "uint8[]", "uint8" );
            ArrayBindHelper<cgInt8>::registerMethods( engine, "int8[]", "int8" );
            ArrayBindHelper<cgUInt16>::registerMethods( engine, "uint16[]", "uint16" );
            ArrayBindHelper<cgInt16>::registerMethods( engine, "int16[]", "int16" );
            ArrayBindHelper<cgUInt32>::registerMethods( engine, "uint[]", "uint" );
            ArrayBindHelper<cgInt32>::registerMethods( engine, "int[]", "int" );
            ArrayBindHelper<cgUInt64>::registerMethods( engine, "uint64[]", "uint64" );
            ArrayBindHelper<cgInt64>::registerMethods( engine, "int64[]", "int64" );
            ArrayBindHelper<cgFloat>::registerMethods( engine, "float[]", "float" );
            ArrayBindHelper<cgDouble>::registerMethods( engine, "double[]", "double" );  
        }

    }; // End Class : Package

} } // End Namespace : cgScriptPackages::Core