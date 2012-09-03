#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgResampleChain.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace ResampleChain
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.ResampleChain" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "ResampleChain", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgResampleChain>( engine );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "ResampleChain", asBEHAVE_FACTORY, "ResampleChain@ f()", asFUNCTIONPR(resampleChainFactory, (), cgResampleChain*), asCALL_CDECL) );

            // Register the object methods
            // Configuration and Capabilities
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( const RenderTargetHandle &in )", asMETHODPR(cgResampleChain, setSource, (const cgRenderTargetHandle &), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( const RenderTargetHandle &in, uint )", asMETHODPR(cgResampleChain, setSource, (const cgRenderTargetHandle &, cgUInt32), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( RenderView @+, const RenderTargetHandle &in, int, const String &in )", asMETHODPR(cgResampleChain, setSource, (cgRenderView *, const cgRenderTargetHandle &, cgUInt32, const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( const RenderTargetHandle &in, const String &in )", asMETHODPR(cgResampleChain, setSource, (const cgRenderTargetHandle &, const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( RenderView @+, const RenderTargetHandle &in, const String &in )", asMETHODPR(cgResampleChain, setSource, (cgRenderView *, const cgRenderTargetHandle &, const cgString&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "uint getLevelCount( ) const", asMETHODPR(cgResampleChain, getLevelCount, ( ) const, cgUInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "const RenderTargetHandle & getLevel( uint ) const", asMETHODPR(cgResampleChain, getLevel, ( cgUInt32 ) const, const cgRenderTargetHandle &), asCALL_THISCALL) );
            
            // Script 'null' handle support
            BINDSUCCESS( engine->registerObjectMethod( "ResampleChain", "bool setSource( NullHandle@ )", asFUNCTIONPR(setSource,( void*, cgResampleChain* ), bool), asCALL_CDECL_OBJLAST ) );
        }

        //---------------------------------------------------------------------
        //  Name : resampleChainFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgResampleChain class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgResampleChain * resampleChainFactory( )
        {
            return new cgResampleChain();
        }

        //---------------------------------------------------------------------
        //  Name : setSource ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// ResampleChain::setSource() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setSource( void*nullHandle, cgResampleChain *thisPointer )
        {
            return thisPointer->setSource( cgRenderTargetHandle::Null );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::ResampleChain