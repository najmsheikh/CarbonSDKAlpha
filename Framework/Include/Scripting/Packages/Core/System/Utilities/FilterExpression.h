#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgFilterExpression.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System { namespace Utilities {

// Package declaration
namespace FilterExpression
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.Utilities.FilterExpression" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "FilterExpression"          , 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "FilterExpressionIdentifier", sizeof(cgFilterExpression::Identifier), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgFilterExpression::Identifier (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgFilterExpression::Identifier>( engine, "FilterExpressionIdentifier" );

            // Register Values
            BINDSUCCESS( engine->registerObjectProperty( "FilterExpressionIdentifier", "String name" , offsetof(cgFilterExpression::Identifier,name) ) );
            BINDSUCCESS( engine->registerObjectProperty( "FilterExpressionIdentifier", "uint64 value", offsetof(cgFilterExpression::Identifier,value) ) );
            
            // Requires array type for the value array methods.
            ArrayBindHelper<cgFilterExpression::Identifier>::registerType( engine, "FilterExpressionIdentifier[]", "FilterExpressionIdentifier" );
            ArrayBindHelper<cgFilterExpression::Identifier>::registerMethods( engine, "FilterExpressionIdentifier[]", "FilterExpressionIdentifier" );

            ///////////////////////////////////////////////////////////////////////
            // cgFilterExpression (Class)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgFilterExpression>( engine );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "FilterExpression", asBEHAVE_FACTORY, "FilterExpression@ f()", asFUNCTIONPR(filterExpressionFactory, (), cgFilterExpression*), asCALL_CDECL) );
            BINDSUCCESS( engine->registerObjectBehavior( "FilterExpression", asBEHAVE_FACTORY, "FilterExpression@ f( const String &in )", asFUNCTIONPR(filterExpressionFactory, ( const cgString& ), cgFilterExpression*), asCALL_CDECL) );
            BINDSUCCESS( engine->registerObjectBehavior( "FilterExpression", asBEHAVE_FACTORY, "FilterExpression@ f( const String &in, const FilterExpressionIdentifier[] &in )", asFUNCTIONPR(filterExpressionFactory, ( const cgString&, const cgFilterExpression::IdentifierArray& ), cgFilterExpression*), asCALL_CDECL) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "FilterExpression", "bool isCompiled() const", asMETHODPR(cgFilterExpression, isCompiled, () const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "FilterExpression", "bool compileExpression( const String &in )", asMETHODPR(cgFilterExpression, compileExpression, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "FilterExpression", "bool compileExpression( const String &in, const FilterExpressionIdentifier[] &in )", asMETHODPR(cgFilterExpression, compileExpression, ( const cgString&, const cgFilterExpression::IdentifierArray& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "FilterExpression", "bool evalute( uint64 )", asMETHODPR(cgFilterExpression, evaluate, ( cgUInt64 ), bool), asCALL_THISCALL) );            
        }

        //---------------------------------------------------------------------
        //  Name : filterExpressionFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgFilterExpression class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgFilterExpression * filterExpressionFactory( )
        {
            return new cgFilterExpression();
        }

        //---------------------------------------------------------------------
        //  Name : filterExpressionFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgFilterExpression class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgFilterExpression * filterExpressionFactory( const cgString & expression )
        {
            return new cgFilterExpression( expression );
        }

        //---------------------------------------------------------------------
        //  Name : filterExpressionFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgFilterExpression class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgFilterExpression * filterExpressionFactory( const cgString & expression, const cgFilterExpression::IdentifierArray & defines )
        {
            return new cgFilterExpression( expression, defines );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::System::Utilities::FilterExpression