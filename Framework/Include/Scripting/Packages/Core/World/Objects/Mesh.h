#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/Objects/cgMeshObject.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World { namespace Objects {

// Package declaration
namespace Mesh
{
    //-------------------------------------------------------------------------
    // Name : registerNodeMethods ()
    // Desc : Register the base cgGroupNode's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerNodeMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::ObjectNode::registerNodeMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMesh( const MeshHandle &in )", asMETHODPR(type,setMesh, (const cgMeshHandle&), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "MeshHandle getMesh( ) const", asMETHODPR(type,getMesh, () const, cgMeshHandle ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool setMeshMaterial( const MaterialHandle &in )", asMETHODPR(type, setMeshMaterial, ( const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool setFaceMaterial( uint, const MaterialHandle &in )", asMETHODPR(type, setFaceMaterial, ( cgUInt32, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool replaceMaterial( const MaterialHandle &in, const MaterialHandle &in )", asMETHODPR(type, replaceMaterial, ( const cgMaterialHandle&, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowStage( SceneProcessStage )", asMETHODPR(type, setShadowStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getShadowStage( ) const", asMETHODPR(type, getShadowStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createBox( float, float, float, uint, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createBox, ( cgFloat, cgFloat, cgFloat, cgUInt32, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createSphere( float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createSphere, ( cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCylinder( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCylinder, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCapsule( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCapsule, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCone( float, float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCone, ( cgFloat, cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createTorus( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createTorus, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );

    } // End Method registerNodeMethods<>

    //-------------------------------------------------------------------------
    // Name : registerObjectMethods ()
    // Desc : Register the base cgGroupObject's class methods. Can be called by
    //        derived classes to re-register the behaviors
    //-------------------------------------------------------------------------
    template <class type>
    void registerObjectMethods( cgScriptEngine * engine, const cgChar * typeName )
    {
        // Register base class object methods
        Core::World::Objects::WorldObject::registerObjectMethods<type>( engine, typeName );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setMesh( const MeshHandle &in )", asMETHODPR(type,setMesh, (const cgMeshHandle&), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "MeshHandle getMesh( ) const", asMETHODPR(type,getMesh, () const, cgMeshHandle ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createBox( float, float, float, uint, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createBox, ( cgFloat, cgFloat, cgFloat, cgUInt32, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createSphere( float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createSphere, ( cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCylinder( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCylinder, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCapsule( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCapsule, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createCone( float, float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createCone, ( cgFloat, cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool createTorus( float, float, uint, uint, bool, MeshCreateOrigin )", asMETHODPR(type,createTorus, ( cgFloat, cgFloat, cgUInt32, cgUInt32, bool, cgMeshCreateOrigin::Base ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool pickFace( const Vector3 &in, const Vector3 &in, Vector3 &inout, uint &inout, MaterialHandle &inout )", asMETHODPR(type, pickFace, ( const cgVector3&, const cgVector3&, cgVector3&, cgUInt32&, cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool setMeshMaterial( const MaterialHandle &in )", asMETHODPR(type, setMeshMaterial, ( const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool setFaceMaterial( uint, const MaterialHandle &in )", asMETHODPR(type, setFaceMaterial, ( cgUInt32, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool replaceMaterial( const MaterialHandle &in, const MaterialHandle &in )", asMETHODPR(type, replaceMaterial, ( const cgMaterialHandle&, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setShadowStage( SceneProcessStage )", asMETHODPR(type, setShadowStage, ( cgSceneProcessStage::Base ), void ), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "SceneProcessStage getShadowStage( ) const", asMETHODPR(type, getShadowStage, ( ) const, cgSceneProcessStage::Base ), asCALL_THISCALL) );

    } // End Method registerObjectMethods<>

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Objects.Mesh" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "MeshObject", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "MeshNode", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgMeshObject>( engine );
            registerHandleBehaviors<cgMeshNode>( engine );

            ///////////////////////////////////////////////////////////////////////
            // Type Identifiers
            ///////////////////////////////////////////////////////////////////////
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_MeshObject", (void*)&RTID_MeshObject ) );
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_MeshNode", (void*)&RTID_MeshNode ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMeshObject (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register object methods.
            registerObjectMethods<cgMeshObject>( engine, "MeshObject" );
            
            ///////////////////////////////////////////////////////////////////////
            // cgMeshNode (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register node methods.
            registerNodeMethods<cgMeshNode>( engine, "MeshNode" );            
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::World::Objects::Mesh