#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgScene.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Scene
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.World.Scene" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Scene", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgScene (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgScene>( engine );
            
            // Register base class methods.
            Core::System::References::Reference::registerReferenceMethods<cgScene>( engine, "Scene" );

            // ToDo: 9999 - Completely redo this.

            // Requires scene element array type for several methods in this interface
            BINDSUCCESS( engine->registerObjectType( "SceneElement@[]", sizeof(std::vector<cgSceneElement*>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            STDVectorHelper<cgSceneElement*>::registerMethods( engine, "SceneElement@[]", "SceneElement@" );

            // Requires object node array type for several methods in this interface
            BINDSUCCESS( engine->registerObjectType( "ObjectNode@[]", sizeof(std::vector<cgObjectNode*>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            STDVectorHelper<cgObjectNode*>::registerMethods( engine, "ObjectNode@[]", "ObjectNode@" );
            
            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "const String & getName( ) const", asMETHODPR(cgScene,getName,() const, const cgString &), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void update( )", asMETHODPR(cgScene,update,(),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void render( )", asMETHODPR(cgScene,render,(),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void unload( )", asMETHODPR(cgScene,unload,(),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void addRootNode( ObjectNode@+ )", asMETHODPR(cgScene,addRootNode,( cgObjectNode* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void removeRootNode( ObjectNode@+ )", asMETHODPR(cgScene,removeRootNode,( cgObjectNode* ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void setObjectUpdateRate( ObjectNode@+, UpdateRate )", asMETHODPR(cgScene,setObjectUpdateRate,( cgObjectNode*, cgUpdateRate::Base ),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool setActiveCamera( CameraNode@+ )", asMETHODPR(cgScene,setActiveCamera,( cgCameraNode* ), bool), asCALL_THISCALL) );        
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "CameraNode@+ getActiveCamera( ) const", asMETHODPR(cgScene,getActiveCamera,() const, cgCameraNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "RenderDriver@+ getRenderDriver( ) const", asMETHODPR(cgScene,getRenderDriver,( ) const, cgRenderDriver*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ResourceManager@+ getResourceManager( ) const", asMETHODPR(cgScene,getResourceManager,( ) const,cgResourceManager*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "World@+ getParentWorld( ) const", asMETHODPR(cgScene,getParentWorld,() const, cgWorld*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "Landscape@+ getLandscape( ) const", asMETHODPR(cgScene,getLandscape,( ) const, cgLandscape*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "LightingManager@+ getLightingManager( ) const", asMETHODPR(cgScene,getLightingManager,( ) const, cgLightingManager*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "const FilterExpressionIdentifier[] & getMaterialPropertyIdentifiers( ) const", asMETHODPR(cgScene,getMaterialPropertyIdentifiers,( ) const, const cgFilterExpression::IdentifierArray & ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "uint getRenderClassId( const String&in ) const", asMETHODPR(cgScene,getRenderClassId,( const cgString&) const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool beginRenderPass( const String &in )", asMETHODPR(cgScene,beginRenderPass,( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void endRenderPass( )", asMETHODPR(cgScene,endRenderPass,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ pickClosestNode( const Size &in, const Vector3 &in, const Vector3 &in, bool, float, Vector3 &inout )", asMETHODPR(cgScene,pickClosestNode,( const cgSize&, const cgVector3&, const cgVector3&, bool, cgFloat, cgVector3& ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ pickClosestNode( const Size &in, const Vector3 &in, const Vector3 &in, Vector3 &inout )", asMETHODPR(cgScene,pickClosestNode,( const cgSize&, const cgVector3&, const cgVector3&, cgVector3& ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "PhysicsWorld@+ getPhysicsWorld( ) const", asMETHODPR(cgScene,getPhysicsWorld,( ) const, cgPhysicsWorld*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ loadObjectNode( uint, CloneMethod, bool )", asMETHODPR(cgScene,loadObjectNode,( cgUInt32, cgCloneMethod::Base, bool ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "const array<SceneElement@> & getSceneElementsByType( const UID &in ) const", asMETHODPR(cgScene,getSceneElementsByType,( const cgUID& ) const, const cgSceneElementArray& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ createObjectNode( bool, const UID &in, bool )", asMETHODPR(cgScene,createObjectNode,( bool, const cgUID&, bool ), cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ createObjectNode( bool, const UID &in, bool, CloneMethod, ObjectNode@+, const Transform &in )", asMETHODPR(cgScene,createObjectNode,( bool, const cgUID&, bool, cgCloneMethod::Base, cgObjectNode*, const cgTransform& ), cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "const array<ObjectNode@> & getObjectNodesByType( const UID &in ) const", asMETHODPR(cgScene,getObjectNodesByType,( const cgUID& ) const, const cgObjectNodeArray& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ getObjectNodeById( uint ) const", asMETHODPR(cgScene,getObjectNodeById,( cgUInt32 ) const, cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getObjectNodesInBounds( const Vector3 &in, float, array<ObjectNode@> &inout ) const", asMETHODPR(cgScene,getObjectNodesInBounds,( const cgVector3&, cgFloat, cgObjectNodeArray& ) const, void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getObjectNodesInBounds( const BoundingBox &in, array<ObjectNode@> &inout ) const", asMETHODPR(cgScene,getObjectNodesInBounds,( const cgBoundingBox&, cgObjectNodeArray& ) const, void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool rayCastClosest( const Vector3 &in, const Vector3 &in, SceneCollisionContact &inout )", asMETHODPR(cgScene, rayCastClosest, ( const cgVector3&, const cgVector3&, cgSceneCollisionContact& ), bool), asCALL_THISCALL) );
            // ToDo: bool                rayCast             ( const cgVector3 & from, const cgVector3 & to, bool sortContacts, cgSceneCollisionContact::Array & contacts );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Scene
