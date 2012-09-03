#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Child packages
#include "Core/System.h"
#include "Core/Math.h"
#include "Core/Animation.h"
#include "Core/World.h"
#include "Core/Rendering.h"
#include "Core/Physics.h"
#include "Core/Input.h"
#include "Core/Resources.h"
#include "Core/UI.h"

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
            DECLARE_PACKAGE_CHILD( Physics )
            DECLARE_PACKAGE_CHILD( World )
            DECLARE_PACKAGE_CHILD( UI )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "NullHandle", 0, asOBJ_REF ) );

            // Register standard std::vector array types for *all* angelscript
            // supported primitive data types. This is necessary for the surface
            // shader permutation selection logic in which arrays will be expanded.
            // No valid primitive array types should ever use the default array template.
            BINDSUCCESS( engine->registerObjectType( "bool[]"  , sizeof(std::vector<bool>)    , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "uint8[]" , sizeof(std::vector<cgByte>)  , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "int8[]"  , sizeof(std::vector<cgInt8>)  , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "uint16[]", sizeof(std::vector<cgUInt16>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "int16[]" , sizeof(std::vector<cgInt16>) , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "uint[]"  , sizeof(std::vector<cgUInt32>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "int[]"   , sizeof(std::vector<cgInt32>) , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "uint64[]", sizeof(std::vector<cgUInt64>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "int64[]" , sizeof(std::vector<cgInt64>) , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "float[]" , sizeof(std::vector<cgFloat>) , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "double[]", sizeof(std::vector<cgDouble>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // NullHandle (Class)
            ///////////////////////////////////////////////////////////////////////
            registerHandleBehaviors<NullHandle>( engine );

            // Register standard std::vector array types for *all* angelscript
            // supported primitive data types. This is necessary for the surface
            // shader permutation selection logic in which arrays will be expanded.
            // No valid primitive array types should ever use the default array template.
            STDVectorHelper<bool, true>::registerMethods( engine, "bool[]", "bool" );
            STDVectorHelper<cgByte>::registerMethods( engine, "uint8[]", "uint8" );
            STDVectorHelper<cgInt8>::registerMethods( engine, "int8[]", "int8" );
            STDVectorHelper<cgUInt16>::registerMethods( engine, "uint16[]", "uint16" );
            STDVectorHelper<cgInt16>::registerMethods( engine, "int16[]", "int16" );
            STDVectorHelper<cgUInt32>::registerMethods( engine, "uint[]", "uint" );
            STDVectorHelper<cgInt32>::registerMethods( engine, "int[]", "int" );
            STDVectorHelper<cgUInt64>::registerMethods( engine, "uint64[]", "uint64" );
            STDVectorHelper<cgInt64>::registerMethods( engine, "int64[]", "int64" );
            STDVectorHelper<cgFloat>::registerMethods( engine, "float[]", "float" );
            STDVectorHelper<cgDouble>::registerMethods( engine, "double[]", "double" );  
        }

    }; // End Class : Package

} } // End Namespace : cgScriptPackages::Core