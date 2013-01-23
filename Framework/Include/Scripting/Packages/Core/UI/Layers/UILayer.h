#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUILayers.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI { namespace Layers {

// Package declaration
namespace UILayer
{
    //-------------------------------------------------------------------------
    // Name : registerLayerMethods ()
    // Desc : Register the base cgUILayer class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerLayerMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register the object methods
        BINDSUCCESS( engine->registerObjectMethod( typeName, "UILayerType getLayerType( ) const", asMETHODPR(type, getLayerType, () const, cgUILayerType::Base ), asCALL_THISCALL) );
        // ToDo: cgBillboardBuffer * GetLayerBuffer      ( ) { return m_pBillboards; }
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool prepareLayer( const InputStream &in )", asMETHODPR(type, prepareLayer, ( const cgInputStream & ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool endPrepareLayer( )", asMETHODPR(type, endPrepareLayer, ( ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "UIManager@+ getUIManager( )", asMETHODPR(type, getUIManager, ( ), cgUIManager* ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void bringToFront( )", asMETHODPR(type, bringToFront, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void sendToBack( )", asMETHODPR(type, sendToBack, ( ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void render( )", asMETHODPR(type, render, ( ), void ), asCALL_THISCALL) );

        // Events
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseMove( const Point&, const PointF& )", asMETHODPR(type, onMouseMove, ( const cgPoint&, const cgPointF& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseButtonDown( int, const Point& )", asMETHODPR(type, onMouseButtonDown, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseButtonUp( int, const Point& )", asMETHODPR(type, onMouseButtonUp, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onMouseWheelScroll( int, const Point& )", asMETHODPR(type, onMouseWheelScroll, ( cgInt32, const cgPoint& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyDown( int, uint )", asMETHODPR(type, onKeyDown, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyUp( int, uint )", asMETHODPR(type, onKeyUp, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onKeyPressed( int, uint )", asMETHODPR(type, onKeyPressed, ( cgInt32, cgUInt32 ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool onScreenLayoutChange( )", asMETHODPR(type, onScreenLayoutChange, ( ), bool ), asCALL_THISCALL) );
        
    } // End Method registerLayerMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.Layers.UILayer" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "UILayer", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgUILayer (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUILayer>( engine );

            // Register base / system behaviors.
            registerLayerMethods<cgUILayer>( engine, "UILayer" );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::UI::Layers::UILayer
