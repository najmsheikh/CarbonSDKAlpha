#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgLightObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects { namespace Lights {

// Package declaration
namespace Light
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgLightNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isDiffuseSource( ) const", asMETHODPR(type, isDiffuseSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isSpecularSource( ) const", asMETHODPR(type, isSpecularSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isShadowSource( ) const", asMETHODPR(type, isShadowSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getLightingStage( ) const", asMETHODPR(type, getLightingStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getShadowStage( ) const", asMETHODPR(type, getShadowStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularMinDistance( ) const", asMETHODPR(type, getSpecularMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularMaxDistance( ) const", asMETHODPR(type, getSpecularMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowMinDistance( ) const", asMETHODPR(type, getShadowMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowMaxDistance( ) const", asMETHODPR(type, getShadowMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowLODMinDistance( ) const", asMETHODPR(type, getShadowLODMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowLODMaxDistance( ) const", asMETHODPR(type, getShadowLODMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setLightingStage( SceneProcessStage )", asMETHODPR(type, setLightingStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowStage( SceneProcessStage )", asMETHODPR(type, setShadowStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularMinDistance( float )", asMETHODPR(type, setSpecularMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularMaxDistance( float )", asMETHODPR(type, setSpecularMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowMinDistance( float )", asMETHODPR(type, setShadowMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowMaxDistance( float )", asMETHODPR(type, setShadowMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowLODMinDistance( float )", asMETHODPR(type, setShadowLODMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowLODMaxDistance( float )", asMETHODPR(type, setShadowLODMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getDiffuseHDRScale( ) const", asMETHODPR(type, getDiffuseHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularHDRScale( ) const", asMETHODPR(type, getSpecularHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getAmbientHDRScale( ) const", asMETHODPR(type, getAmbientHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getAmbientFarHDRScale( ) const", asMETHODPR(type, getAmbientFarHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRimHDRScale( ) const", asMETHODPR(type, getRimHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getDiffuseColor( ) const", asMETHODPR(type, getDiffuseColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getSpecularColor( ) const", asMETHODPR(type, getSpecularColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getAmbientColor( ) const", asMETHODPR(type, getAmbientColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getAmbientFarColor( ) const", asMETHODPR(type, getAmbientFarColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getRimColor( ) const", asMETHODPR(type, getRimColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDiffuseHDRScale( float )", asMETHODPR(type, setDiffuseHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularHDRScale( float )", asMETHODPR(type, setSpecularHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientHDRScale( float )", asMETHODPR(type, setAmbientHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientFarHDRScale( float )", asMETHODPR(type, setAmbientFarHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRimHDRScale( float )", asMETHODPR(type, setRimHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDiffuseColor( const ColorValue &in )", asMETHODPR(type, setDiffuseColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularColor( const ColorValue &in )", asMETHODPR(type, setSpecularColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientColor( const ColorValue &in )", asMETHODPR(type, setAmbientColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientFarColor( const ColorValue &in )", asMETHODPR(type, setAmbientFarColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRimColor( const ColorValue &in )", asMETHODPR(type, setRimColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );

        // ToDo: cgSampler                 * getDistanceAttenSampler     ( ) const;
        // ToDo: const cgBezierSpline2     & getDistanceAttenCurve       ( ) const;
        // ToDo: cgSampler                 * getAttenMaskSampler         ( ) const;
        // ToDo: void                        setDistanceAttenCurve       ( const cgBezierSpline2 & spline );
        // ToDo: void                        setAttenMaskSampler         ( cgSampler * sampler );

        // ToDo: virtual LightType           getLightType                ( ) const = 0;
        // ToDo: virtual bool                beginLighting               ( );

    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgLightObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isDiffuseSource( ) const", asMETHODPR(type, isDiffuseSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isSpecularSource( ) const", asMETHODPR(type, isSpecularSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isShadowSource( ) const", asMETHODPR(type, isShadowSource, ( ) const, bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getLightingStage( ) const", asMETHODPR(type, getLightingStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getShadowStage( ) const", asMETHODPR(type, getShadowStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularMinDistance( ) const", asMETHODPR(type, getSpecularMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularMaxDistance( ) const", asMETHODPR(type, getSpecularMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowMinDistance( ) const", asMETHODPR(type, getShadowMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowMaxDistance( ) const", asMETHODPR(type, getShadowMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowLODMinDistance( ) const", asMETHODPR(type, getShadowLODMinDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getShadowLODMaxDistance( ) const", asMETHODPR(type, getShadowLODMaxDistance, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setLightingStage( SceneProcessStage )", asMETHODPR(type, setLightingStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowStage( SceneProcessStage )", asMETHODPR(type, setShadowStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularMinDistance( float )", asMETHODPR(type, setSpecularMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularMaxDistance( float )", asMETHODPR(type, setSpecularMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowMinDistance( float )", asMETHODPR(type, setShadowMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowMaxDistance( float )", asMETHODPR(type, setShadowMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowLODMinDistance( float )", asMETHODPR(type, setShadowLODMinDistance, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowLODMaxDistance( float )", asMETHODPR(type, setShadowLODMaxDistance, ( cgFloat ), void ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getDiffuseHDRScale( ) const", asMETHODPR(type, getDiffuseHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getSpecularHDRScale( ) const", asMETHODPR(type, getSpecularHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getAmbientHDRScale( ) const", asMETHODPR(type, getAmbientHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getAmbientFarHDRScale( ) const", asMETHODPR(type, getAmbientFarHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "float getRimHDRScale( ) const", asMETHODPR(type, getRimHDRScale, ( ) const, cgFloat ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getDiffuseColor( ) const", asMETHODPR(type, getDiffuseColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getSpecularColor( ) const", asMETHODPR(type, getSpecularColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getAmbientColor( ) const", asMETHODPR(type, getAmbientColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getAmbientFarColor( ) const", asMETHODPR(type, getAmbientFarColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "const ColorValue & getRimColor( ) const", asMETHODPR(type, getRimColor, ( ) const, const cgColorValue& ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDiffuseHDRScale( float )", asMETHODPR(type, setDiffuseHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularHDRScale( float )", asMETHODPR(type, setSpecularHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientHDRScale( float )", asMETHODPR(type, setAmbientHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientFarHDRScale( float )", asMETHODPR(type, setAmbientFarHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRimHDRScale( float )", asMETHODPR(type, setRimHDRScale, ( cgFloat ), void ), asCALL_THISCALL) );

        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setDiffuseColor( const ColorValue &in )", asMETHODPR(type, setDiffuseColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setSpecularColor( const ColorValue &in )", asMETHODPR(type, setSpecularColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientColor( const ColorValue &in )", asMETHODPR(type, setAmbientColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setAmbientFarColor( const ColorValue &in )", asMETHODPR(type, setAmbientFarColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setRimColor( const ColorValue &in )", asMETHODPR(type, setRimColor, ( const cgColorValue & ), void ), asCALL_THISCALL) );
      
        // ToDo: cgSampler                 * getDistanceAttenSampler     ( ) const;
        // ToDo: const cgBezierSpline2     & getDistanceAttenCurve       ( ) const;
        // ToDo: cgSampler                 * getAttenMaskSampler         ( ) const;
        // ToDo: void                        setDistanceAttenCurve       ( const cgBezierSpline2 & spline );
        // ToDo: void                        setAttenMaskSampler         ( cgSampler * sampler );

        // ToDo: virtual LightType           getLightType                ( ) const = 0;
        // ToDo: virtual bool                beginLighting               ( );


    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Lights.Light" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "LightObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "LightNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgLightObject>( engine );
            registerHandleBehaviors<cgLightNode>( engine );

            // ToDo: Finish these types

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_LightObject", (void*)&RTID_LightObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_LightNode", (void*)&RTID_LightNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgLightObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgLightObject>( engine, "LightObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgLightNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgLightNode>( engine, "LightNode" );

        }

    }; // End Class : Package

} } } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Lights::Light