#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgRenderDriver.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace RenderView
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.RenderView" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RenderView", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgRenderView>( engine );

            // Register base class object methods
            Core::System::References::Reference::registerReferenceMethods<cgRenderView>( engine, "RenderView" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool initialize( )", asMETHODPR(cgRenderView, initialize, (), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool initialize( const Rect& )", asMETHODPR(cgRenderView, initialize, ( const cgRect&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool initialize( ScaleMode, const RectF& )", asMETHODPR(cgRenderView, initialize, ( cgScaleMode::Base, const cgRectF&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool isViewLost( ) const", asMETHODPR(cgRenderView, isViewLost, ( ) const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool setLayout( const Rect &in )", asMETHODPR(cgRenderView, setLayout, ( const cgRect&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool setLayout( ScaleMode, const RectF &in )", asMETHODPR(cgRenderView, setLayout, ( cgScaleMode::Base, const cgRectF&), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "void getLayout( ScaleMode &inout, RectF &inout ) const", asMETHODPR(cgRenderView, getLayout, ( cgScaleMode::Base &, cgRectF&) const, void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "Rect getRectangle() const", asMETHODPR(cgRenderView, getRectangle, ( ) const, cgRect), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "Point getPosition() const", asMETHODPR(cgRenderView, getPosition, ( ) const, cgPoint), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "Size getSize() const", asMETHODPR(cgRenderView, getSize, ( ) const, cgSize), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "int getRenderCount() const", asMETHODPR(cgRenderView, getRenderCount, ( ) const, cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool begin()", asMETHODPR(cgRenderView, begin, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool end()", asMETHODPR(cgRenderView, end, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool end( bool )", asMETHODPR(cgRenderView, end, ( bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "bool present( )", asMETHODPR(cgRenderView, present, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getViewBuffer( ) const", asMETHODPR(cgRenderView, getViewBuffer, ( ) const, const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilBuffer( ) const", asMETHODPR(cgRenderView, getDepthStencilBuffer, ( ) const, const cgDepthStencilTargetHandle&), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, const String &in )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, const cgString& ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, float, float )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgFloat, cgFloat ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, float, float, const String &in )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, const cgString& ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, MultiSampleType, uint )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgMultiSampleType::Base, cgUInt32 ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, MultiSampleType, uint, const String &in )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgMultiSampleType::Base, cgUInt32, const cgString& ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, float, float, MultiSampleType, uint )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, cgMultiSampleType::Base, cgUInt32 ), const cgRenderTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const RenderTargetHandle & getRenderSurface( BufferFormat, float, float, MultiSampleType, uint, const String &in )", asMETHODPR(cgRenderView, getRenderSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, cgMultiSampleType::Base, cgUInt32, const cgString& ), const cgRenderTargetHandle&), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, const String &in )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, const cgString& ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, float, float )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgFloat, cgFloat ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, float, float, const String &in )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, const cgString& ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, MultiSampleType, uint )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgMultiSampleType::Base, cgUInt32 ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, MultiSampleType, uint, const String &in )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgMultiSampleType::Base, cgUInt32, const cgString& ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, float, float, MultiSampleType, uint )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, cgMultiSampleType::Base, cgUInt32 ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderView", "const DepthStencilTargetHandle & getDepthStencilSurface( BufferFormat, float, float, MultiSampleType, uint, const String &in )", asMETHODPR(cgRenderView, getDepthStencilSurface, ( cgBufferFormat::Base, cgFloat, cgFloat, cgMultiSampleType::Base, cgUInt32, const cgString& ), const cgDepthStencilTargetHandle&), asCALL_THISCALL) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::RenderView