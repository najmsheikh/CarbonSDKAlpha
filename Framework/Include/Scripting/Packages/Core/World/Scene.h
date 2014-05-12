#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <World/cgScene.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace World {

// Package declaration
namespace Scene
{
    // Namespace promotion (within parent)
    using namespace cgScriptInterop::Types;

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
            
            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "const String & getName( ) const", asMETHODPR(cgScene,getName,() const, const cgString &), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void update( )", asMETHODPR(cgScene,update,(),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void render( )", asMETHODPR(cgScene,render,(),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void unload( )", asMETHODPR(cgScene,unload,(),void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool clear( )", asMETHODPR(cgScene,clear,(),bool), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "Scene", "void suppressEvents( bool )", asMETHODPR(cgScene,suppressEvents,(bool),void), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool isEventSuppressionEnabled( ) const", asMETHODPR(cgScene,isEventSuppressionEnabled,() const,bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool isUpdating( ) const", asMETHODPR(cgScene,isUpdating,() const,bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool isUpdatingEnabled( ) const", asMETHODPR(cgScene,isUpdatingEnabled,() const,bool), asCALL_THISCALL) );
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
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ pickClosestNode( const Size &in, const Vector3 &in, const Vector3 &in, uint32, float, Vector3 &inout )", asMETHODPR(cgScene,pickClosestNode,( const cgSize&, const cgVector3&, const cgVector3&, cgUInt32, cgFloat, cgVector3& ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ pickClosestNode( const Size &in, const Vector3 &in, const Vector3 &in, Vector3 &inout )", asMETHODPR(cgScene,pickClosestNode,( const cgSize&, const cgVector3&, const cgVector3&, cgVector3& ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "PhysicsWorld@+ getPhysicsWorld( ) const", asMETHODPR(cgScene,getPhysicsWorld,( ) const, cgPhysicsWorld*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ loadObjectNode( uint, CloneMethod, bool )", asMETHODPR(cgScene,loadObjectNode,( cgUInt32, cgCloneMethod::Base, bool ), cgObjectNode*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getSceneElementsByType( const UID &in, array<SceneElement@>@+ ) const", asFUNCTIONPR(getSceneElementsByType,( const cgUID&, ScriptArray*, cgScene* ), void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ createObjectNode( bool, const UID &in, bool )", asMETHODPR(cgScene,createObjectNode,( bool, const cgUID&, bool ), cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ createObjectNode( bool, const UID &in, bool, CloneMethod, ObjectNode@+, const Transform &in )", asMETHODPR(cgScene,createObjectNode,( bool, const cgUID&, bool, cgCloneMethod::Base, cgObjectNode*, const cgTransform& ), cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getObjectNodesByType( const UID &in, array<ObjectNode@>@+ ) const", asFUNCTIONPR(getObjectNodesByType,( const cgUID&, ScriptArray*, cgScene* ), void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "ObjectNode@+ getObjectNodeById( uint ) const", asMETHODPR(cgScene,getObjectNodeById,( cgUInt32 ) const, cgObjectNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getObjectNodesInBounds( const Vector3 &in, float, array<ObjectNode@>@+ ) const", asFUNCTIONPR(getObjectNodesInBounds,( const cgVector3 &, cgFloat, ScriptArray*, cgScene* ), void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "void getObjectNodesInBounds( const BoundingBox &in, array<ObjectNode@>@+ ) const", asFUNCTIONPR(getObjectNodesInBounds,( const cgBoundingBox &, ScriptArray*, cgScene* ), void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "Scene", "bool rayCastClosest( const Vector3 &in, const Vector3 &in, SceneRayCastContact &inout )", asMETHODPR(cgScene, rayCastClosest, ( const cgVector3&, const cgVector3&, cgSceneRayCastContact& ), bool), asCALL_THISCALL) );
            // ToDo: bool                rayCast             ( const cgVector3 & from, const cgVector3 & to, bool sortContacts, cgSceneRayCastContact::Array & contacts );
        }

        //---------------------------------------------------------------------
        //  Name : getObjectNodesByType ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Scene::getObjectNodesByType() method that allows the script to
        /// use a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void getObjectNodesByType( const cgUID & type, ScriptArray * nodes, cgScene *thisPointer )
        {
            const cgObjectNodeArray & sceneNodes = thisPointer->getObjectNodesByType( type );
            nodes->resize( sceneNodes.size() );
            for ( size_t i = 0; i < sceneNodes.size(); ++i )
                nodes->setValue( i, (void**)&sceneNodes[i] );
        }

        //---------------------------------------------------------------------
        //  Name : getSceneElementsByType ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Scene::getSceneElementsByType() method that allows the script to
        /// use a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void getSceneElementsByType( const cgUID & type, ScriptArray * elements, cgScene *thisPointer )
        {
            const cgSceneElementArray & sceneElements = thisPointer->getSceneElementsByType( type );
            elements->resize( sceneElements.size() );
            for ( size_t i = 0; i < sceneElements.size(); ++i )
                elements->setValue( i, (void**)&sceneElements[i] );
        }

        //---------------------------------------------------------------------
        //  Name : getObjectNodesInBounds ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Scene::getObjectNodesInBounds() method that allows the script to
        /// use a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void getObjectNodesInBounds( const cgVector3 & center, cgFloat radius, ScriptArray * nodes, cgScene *thisPointer )
        {
            cgObjectNodeArray sceneNodes;
            thisPointer->getObjectNodesInBounds( center, radius, sceneNodes );
            nodes->resize( sceneNodes.size() );
            for ( size_t i = 0; i < sceneNodes.size(); ++i )
                nodes->setValue( i, &sceneNodes[i] );
        }

        //---------------------------------------------------------------------
        //  Name : getObjectNodesInBounds ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// Scene::getObjectNodesInBounds() method that allows the script to
        /// use a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void getObjectNodesInBounds( const cgBoundingBox & bounds, ScriptArray * nodes, cgScene *thisPointer )
        {
            cgObjectNodeArray sceneNodes;
            thisPointer->getObjectNodesInBounds( bounds, sceneNodes );
            nodes->resize( sceneNodes.size() );
            for ( size_t i = 0; i < sceneNodes.size(); ++i )
                nodes->setValue( i, &sceneNodes[i] );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::World::Scene
