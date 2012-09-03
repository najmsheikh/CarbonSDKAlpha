#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgObjectRenderQueue.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace ObjectRenderQueue
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.ObjectRenderQueue" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Enumerations
            BINDSUCCESS( engine->registerEnum( "QueueProcessHandler" ) );
            BINDSUCCESS( engine->registerEnum( "QueueMaterialHandler" ) );
            BINDSUCCESS( engine->registerEnum( "QueueLightingHandler" ) );

            // Reference types
            BINDSUCCESS( engine->registerObjectType( "ObjectRenderQueue", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgQueueProcessHandler (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "QueueProcessHandler", "Default"            , cgQueueProcessHandler::Default ) );
            BINDSUCCESS( engine->registerEnumValue( "QueueProcessHandler", "DepthSortedBlending", cgQueueProcessHandler::DepthSortedBlending ) );

            ///////////////////////////////////////////////////////////////////////
            // cgQueueMaterialHandler (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "QueueMaterialHandler", "Default" , cgQueueMaterialHandler::Default ) );
            BINDSUCCESS( engine->registerEnumValue( "QueueMaterialHandler", "Collapse", cgQueueMaterialHandler::Collapse ) );

            ///////////////////////////////////////////////////////////////////////
            // cgQueueLightingHandler (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "QueueLightingHandler", "None"   , cgQueueLightingHandler::None ) );
            BINDSUCCESS( engine->registerEnumValue( "QueueLightingHandler", "Default", cgQueueLightingHandler::Default ) );

            ///////////////////////////////////////////////////////////////////////
            // cgObjectRenderQueue (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgObjectRenderQueue>( engine );

            // Register the object factories
            BINDSUCCESS( engine->registerObjectBehavior( "ObjectRenderQueue", asBEHAVE_FACTORY, "ObjectRenderQueue@ f( Scene@+ )", asFUNCTIONPR(objectRenderQueueFactory, ( cgScene* ), cgObjectRenderQueue*), asCALL_CDECL) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void begin( VisibilitySet@+ )", asMETHODPR(cgObjectRenderQueue, begin, ( cgVisibilitySet*), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void begin( VisibilitySet@+, QueueProcessHandler )", asMETHODPR(cgObjectRenderQueue, begin, ( cgVisibilitySet*, cgQueueProcessHandler::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void end( )", asMETHODPR(cgObjectRenderQueue, end, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void end( bool )", asMETHODPR(cgObjectRenderQueue, end, ( bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void clear( )", asMETHODPR(cgObjectRenderQueue, clear, ( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in, const String &in )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString&, const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in, QueueMaterialHandler )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString&, cgQueueMaterialHandler::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in, QueueMaterialHandler, const String &in )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString&, cgQueueMaterialHandler::Base, const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in, QueueMaterialHandler, QueueLightingHandler )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString&, cgQueueMaterialHandler::Base, cgQueueLightingHandler::Base ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void renderClass( const String &in, QueueMaterialHandler, QueueLightingHandler, const String &in )", asMETHODPR(cgObjectRenderQueue, renderClass, ( const cgString&, cgQueueMaterialHandler::Base, cgQueueLightingHandler::Base, const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void setCollapseMaterial( const MaterialHandle &in )", asMETHODPR(cgObjectRenderQueue, setCollapseMaterial, ( const cgMaterialHandle& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "void setMaterialFilter( FilterExpression@+ )", asMETHODPR(cgObjectRenderQueue, setMaterialFilter, ( cgFilterExpression* ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ObjectRenderQueue", "bool setMaterialFilter( const String &in )", asMETHODPR(cgObjectRenderQueue, setMaterialFilter, ( const cgString& ), bool), asCALL_THISCALL) );

        }

        //---------------------------------------------------------------------
        //  Name : objectRenderQueueFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgObjectRenderQueue class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgObjectRenderQueue * objectRenderQueueFactory( cgScene * pScene )
        {
            return new cgObjectRenderQueue( pScene );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::ObjectRenderQueue