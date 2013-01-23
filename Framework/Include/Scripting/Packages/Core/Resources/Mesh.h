#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgMesh.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Mesh
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Mesh" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "Mesh", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "MeshHandle", sizeof(cgMeshHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgMesh>( engine );

            // Type Identifiers
            BINDSUCCESS( engine->registerGlobalProperty( "const UID RTID_MeshResource", (void*)&RTID_MeshResource ) );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceComponentMethods<cgMesh>( engine, "Mesh" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool loadMesh( uint, ResourceManager@+, bool )", asMETHODPR(cgMesh,loadMesh, (cgUInt32, cgResourceManager*, bool), bool ), asCALL_THISCALL) );
            // ToDo: bool bindSkin( const cgSkinBindData & bindData, bool finalizeMesh = true );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void draw( MeshDrawMode )", asMETHODPR(cgMesh,draw, (cgMeshDrawMode::Base), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void draw( uint )", asMETHODPR(cgMesh,draw, (cgUInt32), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void drawSubset( const MaterialHandle &in, MeshDrawMode )", asMETHODPR(cgMesh, drawSubset, ( const cgMaterialHandle&, cgMeshDrawMode::Base), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void drawSubset( const MaterialHandle &in, uint, MeshDrawMode )", asMETHODPR(cgMesh, drawSubset, ( const cgMaterialHandle&, cgUInt32, cgMeshDrawMode::Base), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void drawSubset( uint, MeshDrawMode )", asMETHODPR(cgMesh, drawSubset, ( cgUInt32, cgMeshDrawMode::Base), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void drawSubset( const MaterialHandle &in, array<uint> &in, MeshDrawMode )", asMETHODPR(cgMesh, drawSubset, ( const cgMaterialHandle&, cgUInt32Array&, cgMeshDrawMode::Base), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void drawSubset( array<uint> &in, MeshDrawMode )", asMETHODPR(cgMesh, drawSubset, ( cgUInt32Array &, cgMeshDrawMode::Base), void ), asCALL_THISCALL) );

            // ToDo: void prepareMesh         ( cgVertexFormat * vertexFormat, bool rollBackPrepare = false, cgResourceManager * resourceManager = CG_NULL );
            // ToDo: bool prepareMesh         ( cgVertexFormat * vertexFormat, void * vertices, cgUInt32 vertexCount, const TriangleArray & faces, bool hardwareCopy = true, bool weld = true, bool optimize = true, cgResourceManager * resourceManager = CG_NULL );
            // ToDo: bool setVertexSource     ( void * source, cgUInt32 vertexCount, cgVertexFormat * sourceFormat );
            // ToDo: bool addPrimitives       ( cgPrimitiveType::Base dataOrder, cgUInt32 indexBuffer[], cgUInt32 indexStart, cgUInt32 indexCount, cgMaterialHandle material, cgUInt32 dataGroupId = 0 );
            // ToDo: bool addPrimitives       ( const TriangleArray & triangles );
            // ToDo: bool createBox           ( cgVertexFormat * format, cgFloat width, cgFloat height, cgFloat depth, cgUInt32 widthSegments, cgUInt32 heightSegments, cgUInt32 depthSegments, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDO: bool createSphere        ( cgVertexFormat * format, cgFloat radius, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool createCylinder      ( cgVertexFormat * format, cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool createCapsule       ( cgVertexFormat * format, cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool createCone          ( cgVertexFormat * format, cgFloat radius, cgFloat radiusTip, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool createTorus         ( cgVertexFormat * format, cgFloat outerRadius, cgFloat innerRadius, cgUInt32 bands, cgUInt32 sides, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool createTeapot        ( cgVertexFormat * format, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
            // ToDo: bool endPrepare          ( bool hardwareCopy = true, bool weld = true, bool optimize = true );

            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool pick( const Vector3&in, const Vector3&in, float &inout )", asMETHODPR(cgMesh, pick, ( const cgVector3&, const cgVector3&, cgFloat& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool pick( CameraNode@+, const Size &in, const Transform &in, const Vector3 &in, const Vector3 &in, bool, float, float &inout )", asMETHODPR(cgMesh, pick, ( cgCameraNode*, const cgSize&, const cgTransform&, const cgVector3&, const cgVector3&, bool, cgFloat, cgFloat & ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool pickFace( const Vector3 &in, const Vector3 &in, Vector3 &inout, uint &inout, MaterialHandle &inout )", asMETHODPR(cgMesh, pickFace, ( const cgVector3&, const cgVector3&, cgVector3&, cgUInt32&, cgMaterialHandle& ), bool ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "void setDefaultColor( uint )", asMETHODPR(cgMesh, setDefaultColor, ( cgUInt32 ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool setMeshMaterial( const MaterialHandle &in )", asMETHODPR(cgMesh, setMeshMaterial, ( const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool setFaceMaterial( uint, const MaterialHandle &in )", asMETHODPR(cgMesh, setFaceMaterial, ( cgUInt32, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool replaceMaterial( const MaterialHandle &in, const MaterialHandle &in )", asMETHODPR(cgMesh, replaceMaterial, ( const cgMaterialHandle&, const cgMaterialHandle& ), bool ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool scaleMeshData( float )", asMETHODPR(cgMesh, scaleMeshData, ( cgFloat ), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "bool generateAdjacency( array<uint> &inout )", asMETHODPR(cgMesh, generateAdjacency, ( cgUInt32Array& ), bool ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "RenderDriver @+ getRenderDriver( )", asMETHODPR(cgMesh, getRenderDriver, ( ), cgRenderDriver* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "uint getFaceCount( ) const", asMETHODPR(cgMesh, getFaceCount, ( ) const, cgUInt32 ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "uint getVertexCount( ) const", asMETHODPR(cgMesh, getFaceCount, ( ) const, cgUInt32 ), asCALL_THISCALL) );
    
            // ToDo: cgByte                * getSystemVB         ( );
            // ToDo: cgUInt32              * getSystemIB         ( );
            // ToDo: cgVertexFormat        * getVertexFormat     ( );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "const array<MaterialHandle> & getMaterials( ) const", asMETHODPR(cgMesh, getMaterials, ( ) const, const cgMaterialHandleArray& ), asCALL_THISCALL) );
            // ToDo: cgSkinBindData        * getSkinBindData     ( );
            // ToDo: const BonePaletteArray& getBonePalettes     ( ) const;
            // ToDo: const MeshSubset      * getSubset           ( const cgMaterialHandle & material, cgUInt32 dataGroupId = 0 ) const;

            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "const BoundingBox & getBoundingBox( ) const", asMETHODPR(cgMesh, getBoundingBox, ( ) const, const cgBoundingBox& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "Mesh", "MeshStatus getPrepareStatus( ) const", asMETHODPR(cgMesh, getPrepareStatus, ( ) const, cgMeshStatus::Base ), asCALL_THISCALL) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgMeshHandle>( engine, "MeshHandle", "Mesh" );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Mesh