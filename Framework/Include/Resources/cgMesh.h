//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : cgMesh.h                                                           //
//                                                                           //
// Desc : Contains classes that provide support for loading, creating and    //
//        rendering geometry. Support is included for loading directly from  //
//        XML data structure, or building a mesh procedurally.               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGMESH_H_ )
#define _CGE_CGMESH_H_

//-----------------------------------------------------------------------------
// cgMesh Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Rendering/cgRenderingTypes.h>
#include <Resources/cgResourceHandles.h>
#include <World/cgWorldResourceComponent.h>
#include <World/cgWorldQuery.h>
#include <Math/cgBoundingBox.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgObjectNode;
class cgRenderDriver;
class cgResourceManager;
class cgVertexFormat;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {BD9FFCCF-F907-4D3F-9DB5-B5E52036C9FA}
const cgUID RTID_MeshResource = {0xBD9FFCCF, 0xF907, 0x4D3F, {0x9D, 0xB5, 0xB5, 0xE5, 0x20, 0x36, 0xC9, 0xFA}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgSkinBindData (Class)
/// <summary>
/// Structure describing how a skinned mesh should be bound to any bones
/// that influence its vertices.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSkinBindData
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // Describes how a bone influences a specific vertex.
    struct VertexInfluence
    {
        cgUInt32            vertexIndex;
        cgFloat             weight;

        // Constructors
        VertexInfluence( ) {}
        VertexInfluence( cgUInt32 _index, cgFloat _weight ) :
            vertexIndex(_index), weight(_weight) {}
    };
    CGE_VECTOR_DECLARE(VertexInfluence, VertexInfluenceArray)

    // Describes the vertices that are connected to the referenced bone, and how much influence it has on them.
    struct BoneInfluence
    {
        cgString                boneIdentifier;     // Unique identifier of the bone from which transformation information should be taken.
        cgTransform             bindPoseTransform;  // The "bind pose" or "offset" transform that describes how the bone was positioned in relation to the original vertex data.
        VertexInfluenceArray    influences;         // List of vertices influenced by the referenced bone.
    };
    CGE_VECTOR_DECLARE(BoneInfluence*, BoneArray)
    
    // Contains per-vertex influence and weight information.
    struct VertexData
    {
        cgInt32Array        influences;         // List of bones that influence this vertex.
        cgFloatArray        weights;            // List of weights that describe how this vertex is influenced.
        cgInt32             palette;            // Index of the palette to which this vertex has been assigned.
        cgUInt32            originalVertex;     // The index of the original vertex stored in the mesh.
    };
    CGE_VECTOR_DECLARE(VertexData*, BindVertexArray)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgSkinBindData( );
     cgSkinBindData( const cgSkinBindData & init );
    ~cgSkinBindData( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                addBone                 ( BoneInfluence * bone );
    void                removeEmptyBones        ( );
    void                buildVertexTable        ( cgUInt32 vertexCount, const cgUInt32Array & vertexRemap, BindVertexArray & table );
    void                remapVertices           ( const cgUInt32Array & remap );
    // ToDo: 9999 - Remove
    //bool                Deserialize     ( cgXMLNode & BindData, cgSceneLoader * pLoader, const cgString & strSource );
    const BoneArray   & getBones                ( ) const;
    BoneArray         & getBones                ( );
    void                clearVertexInfluences   ( );

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    BoneArray   mBones; // List of bones that influence the skin mesh vertices.
    
}; // End Class cgSkinBindData

//-----------------------------------------------------------------------------
//  Name : cgBonePalette (Class)
/// <summary>
/// Outlines a collection of bones that influence a given set of faces / 
/// vertices in the mesh.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBonePalette
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgUInt32, BoneIndexMap)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgBonePalette( cgUInt32 paletteSize );
     cgBonePalette( const cgBonePalette & init );
    ~cgBonePalette( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    apply                   ( cgObjectNodeArray & boneObjects, cgSkinBindData * bindData, bool computeInverseTranspose, cgRenderDriver * driver );
    void                    computePaletteFit       ( BoneIndexMap & input, cgInt32 & currentSpace, cgInt32 & commonBones, cgInt32 & additionalBones );
    void                    assignBones             ( BoneIndexMap & bones, cgUInt32Array & faces );
    void                    assignBones             ( const cgUInt32Array & bones );
    cgUInt32                translateBoneToPalette  ( cgUInt32 boneIndex );

    // Accessor methods
    cgInt32                 getMaximumBlendIndex    ( ) const;
    cgUInt32                getMaximumSize          ( ) const;
    cgMaterialHandle      & getMaterial             ( );
    cgUInt32                getDataGroup            ( ) const;
    cgUInt32Array         & getInfluencedFaces      ( );
    const cgUInt32Array   & getBones                ( ) const;
    void                    setMaximumBlendIndex    ( int index );
    void                    setMaterial             ( const cgMaterialHandle & material );
    void                    setDataGroup            ( cgUInt32 group );
    void                    clearInfluencedFaces    ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    BoneIndexMap        mBoneLUT;           // Sorted list of bones in this palette. References the elements in the standard list.
    cgUInt32Array       mBones;             // Main palette of indices that reference the bones outlined in the main skin binding data.
    cgUInt32Array       mFaces;             // List of faces assigned to this palette.
    cgMaterialHandle    mMaterial;          // The material assigned to this subset of the mesh.
    cgUInt32            mDataGroupId;       // The data group identifier used to separate the mesh data into subsets relevant to this bone palette.
    cgUInt32            mMaximumSize;       // The maximum size of this palette.
    cgInt32             mMaximumBlendIndex; // The maximum vertex blend index for this palette (i.e. if every vertex was only influenced by one bone, this variable would contain a value of 0).

}; // End Class cgBonePalette

//-----------------------------------------------------------------------------
//  Name : cgMesh (Class)
/// <summary>
/// Core mesh class. Responsible for loading, managing and rendering
/// mesh data. Used primarily for separate objects in the world (i.e.
/// outside of the static spatial tree), but also utilized by various
/// systems internally.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMesh : public cgWorldResourceComponent
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgMesh, cgWorldResourceComponent, "Mesh" )

public:
    //-------------------------------------------------------------------------
	// Public Structures, Typedefs and Enumerations
	//-------------------------------------------------------------------------
    // Structure describing an individual "piece" of the mesh, often grouped
    // by material , but can be any arbitrary collection of triangles.
    struct MeshSubset
    {
        cgMaterialHandle    material;       // The material assigned to this subset
        cgUInt32            dataGroupId;    // The unique user assigned "data group" that can be used to separate subsets.
        cgInt32             vertexStart;    // The beginning vertex for this batch.
        cgInt32             vertexCount;    // Number of vertices included in this batch.
        cgInt32             faceStart;      // The initial face, from the index buffer, to render in this batch
        cgInt32             faceCount;      // Number of faces to render in this batch.

        // Constructor
        MeshSubset()
        {
            dataGroupId    = 0;
            vertexStart    = -1;
            vertexCount    = 0;
            faceStart      = -1;
            faceCount      = 0;
        
        } // End Constructor

    }; // End Struct MeshSubset

    // Structure describing data for a single triangle in the mesh.
    struct Triangle
    {
        cgUInt32            dataGroupId;
        cgMaterialHandle    material;
        cgUInt32            indices[3];

    }; // End Struct Triangle

    // Typedefs
    CGE_VECTOR_DECLARE(Triangle, TriangleArray)
    CGE_VECTOR_DECLARE(MeshSubset*, SubsetArray)
    CGE_VECTOR_DECLARE(cgBonePalette*, BonePaletteArray)

    //-------------------------------------------------------------------------
	// Constructors & Destructors
	//-------------------------------------------------------------------------
             cgMesh( cgUInt32 referenceId, cgWorld * world );
             cgMesh( cgUInt32 referenceId, cgWorld * world, cgMesh * init );
             cgMesh( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId, bool finalizeMesh );
	virtual ~cgMesh( );

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    // Creation and management methods
    bool                    loadMesh            ( cgUInt32 sourceRefId, cgResourceManager * resourceManager = CG_NULL, bool finalizeMesh = true );
    bool                    bindSkin            ( const cgSkinBindData & bindData, bool finalizeMesh = true );

    // Rendering methods
    void                    draw                ( cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );
    void                    draw                ( cgUInt32 faceCount );
    void                    drawSubset          ( const cgMaterialHandle & material, cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );
    void                    drawSubset          ( const cgMaterialHandle & material, cgUInt32 dataGroupId, cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );
    void                    drawSubset          ( cgUInt32 dataGroupId, cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );
    void                    drawSubset          ( const cgMaterialHandle & material, cgUInt32Array & dataGroups, cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );
    void                    drawSubset          ( cgUInt32Array & dataGroups, cgMeshDrawMode::Base mode = cgMeshDrawMode::Automatic );

    // Mesh creation methods
    void                    prepareMesh         ( cgVertexFormat * vertexFormat, bool rollBackPrepare = false, cgResourceManager * resourceManager = CG_NULL );
    bool                    prepareMesh         ( cgVertexFormat * vertexFormat, void * vertices, cgUInt32 vertexCount, const TriangleArray & faces, bool hardwareCopy = true, bool weld = true, bool optimize = true, cgResourceManager * resourceManager = CG_NULL );
    bool                    setVertexSource     ( void * source, cgUInt32 vertexCount, cgVertexFormat * sourceFormat );
    bool                    addPrimitives       ( cgPrimitiveType::Base dataOrder, cgUInt32 indexBuffer[], cgUInt32 indexStart, cgUInt32 indexCount, cgMaterialHandle material, cgUInt32 dataGroupId = 0 );
    bool                    addPrimitives       ( const TriangleArray & triangles );
    bool                    createBox           ( cgVertexFormat * format, cgFloat width, cgFloat height, cgFloat depth, cgUInt32 widthSegments, cgUInt32 heightSegments, cgUInt32 depthSegments, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createSphere        ( cgVertexFormat * format, cgFloat radius, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createCylinder      ( cgVertexFormat * format, cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createCapsule       ( cgVertexFormat * format, cgFloat radius, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createCone          ( cgVertexFormat * format, cgFloat radius, cgFloat radiusTip, cgFloat height, cgUInt32 stacks, cgUInt32 slices, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createTorus         ( cgVertexFormat * format, cgFloat outerRadius, cgFloat innerRadius, cgUInt32 bands, cgUInt32 sides, bool inverted, cgMeshCreateOrigin::Base origin, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    createTeapot        ( cgVertexFormat * format, bool hardwareCopy = true, cgResourceManager * resourceManager = NULL );
    bool                    endPrepare          ( bool hardwareCopy = true, bool weld = true, bool optimize = true );

    // Mesh queries
    bool                    pick                ( const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgFloat & distanceOut );
    bool                    pick                ( cgCameraNode * pCamera, const cgSize & ViewportSize, const cgTransform & ObjectTransform, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance );
    bool                    pickFace            ( const cgVector3 & rayOrigin, const cgVector3 & rayDirection, cgVector3 & intersectionOut, cgUInt32 & intersectedFaceOut, cgMaterialHandle & intersectedMaterialOut );

    // Material management methods
    void                    setDefaultColor     ( cgUInt32 color );
    bool                    setMeshMaterial     ( const cgMaterialHandle & material );
    bool                    setFaceMaterial     ( cgUInt32 faceIndex, const cgMaterialHandle & material );

    // Utility functions
    bool                    scaleMeshData       ( cgFloat scale );
    
    // Object access methods
    cgRenderDriver        * getRenderDriver     ( );
    cgUInt32                getFaceCount        ( ) const;
    cgUInt32                getVertexCount      ( ) const;
    cgByte                * getSystemVB         ( );
    cgUInt32              * getSystemIB         ( );
    cgVertexFormat        * getVertexFormat     ( );
    cgMaterialHandleArray & getMaterials        ( );
    cgSkinBindData        * getSkinBindData     ( );
    const BonePaletteArray& getBonePalettes     ( ) const;
    const MeshSubset      * getSubset           ( const cgMaterialHandle & material, cgUInt32 dataGroupId = 0 ) const;

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline const cgBoundingBox   & getBoundingBox      ( ) const { return mBoundingBox; }
    inline cgMeshStatus::Base      getPrepareStatus    ( ) const { return mPrepareStatus; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool            loadResource        ( );
    virtual bool            unloadResource      ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_MeshResource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldResourceComponent)
    //-------------------------------------------------------------------------
    virtual cgString        getDatabaseTable    ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );
    
protected:
    //-------------------------------------------------------------------------
	// Protected Structures, Typedefs and Enumerations
	//-------------------------------------------------------------------------
    CGE_MAP_DECLARE(cgUInt32, SubsetArray, DataGroupSubsetMap)
    CGE_MAP_DECLARE(cgMaterialHandle, SubsetArray, MaterialSubsetMap)
    CGE_MAP_DECLARE(cgMaterialHandle, cgUInt32, MaterialUsageMap)
    
    // Mesh Construction Structures
    struct PreparationData
    {
        enum Flags
        {
            SourceContainsNormal   = 0x1,
            SourceContainsBinormal = 0x2,
            SourceContainsTangent  = 0x4
        };

        cgByte        * vertexSource;       // The source vertex data currently being used to prepare the mesh.
        bool            ownsSource;         // Do we own the source data?
        cgVertexFormat* sourceFormat;       // The format of the vertex data currently being used to prepare the mesh.
        cgUInt32Array   vertexRecords;      // Records the location in the vertex buffer that each vertex has been placed during data insertion.
        cgByteArray     vertexData;         // Final vertex buffer currently being prepared.
        cgByteArray     vertexFlags;        // Provides additional descriptive information about the vertices in the above buffer (i.e. source provided a normal, etc.)
        TriangleArray   triangleData;       // This is used to store the current face / triangle data.
        cgUInt32        triangleCount;      // Total number of triangles currently stored here.
        cgUInt32        vertexCount;        // Total number of vertices currently stored here.
        bool            computeNormals;     // Vertex normals should be computed (at least for any vertices where none were supplied).
        bool            computeBinormals;   // Vertex binormals should be computed (at least for any vertices where none were supplied).
        bool            computeTangents;    // Vertex binormals should be computed (at least for any vertices where none were supplied).
    
    }; // End Struct PreparationData
    
    struct FaceInfluences
    {
        cgBonePalette::BoneIndexMap bones;          // List of unique bones that influence a given number of faces.
        cgMaterialHandle            material;       // The material assigned to this list of faces.
    
    }; // End Struct FaceInfluences
    
    // Mesh Sorting / Optimization structures
    struct OptimizerVertexInfo
    {
        cgInt32       cachePosition;                // The position of the vertex in the pseudo-cache
        cgFloat       vertexScore;                  // The score associated with this vertex
        cgUInt32      unusedTriangleReferences;     // Total number of triangles that reference this vertex that have not yet been added
        cgUInt32Array triangleReferences;           // List of all of the triangles referencing this vertex

        // Constructor
        OptimizerVertexInfo() :
            cachePosition(-1),
            vertexScore(0.0f),
            unusedTriangleReferences(0) {}

    }; // End Struct OptimizerVertexInfo

    struct OptimizerTriangleInfo
    {
        cgFloat     triangleScore;              // The sum of all three child vertex scores.
        bool        added;                      // Has the triangle been added to the draw list already?

        // Constructor
        OptimizerTriangleInfo() :
            triangleScore(0.0f),
            added(false) {}

    }; // End Struct OptimizerTriangleInfo

    struct AdjacentEdgeKey
    {
        const cgVector3 * vertex1;            // Pointer to the first vertex in the edge
        const cgVector3 * vertex2;            // Pointer to the second vertex in the edge

    }; // End Struct AdjacentEdgeKey

    struct MeshSubsetKey
    {
        cgMaterialHandle    material;           // Reference to the subset's material.
        cgUInt32            dataGroupId;        // The data group identifier for this subset.

        // Constructors
        MeshSubsetKey() :
            dataGroupId( 0 ) {}
        MeshSubsetKey( cgMaterialHandle _material, cgUInt32 _dataGroupId ) :
            material( _material ), dataGroupId( _dataGroupId ) {}

    }; // End Struct MeshSubsetKey
    CGE_MAP_DECLARE   (MeshSubsetKey, MeshSubset*, SubsetKeyMap)
    CGE_VECTOR_DECLARE(MeshSubsetKey, SubsetKeyArray)

    // Simple structure to allow us to leverage the hierarchical properties of a map
    // to accelerate the weld operation.
    struct WeldKey
    {
        cgByte        * vertex;         // Pointer to the vertex for easy access.
        cgVertexFormat* format;         // Format of the above vertex for easy access.
        cgFloat         tolerance;      // The tolerance we're using to weld (transport only, these should be the same for every key).
    };

    // Simple structure to allow us to leverage the hierarchical properties of a map
    // to accelerate the bone index combination process.
    struct BoneCombinationKey
    {
        FaceInfluences * influences;

        // Constructor
        BoneCombinationKey( FaceInfluences * _influences ) :
            influences( _influences ) {}
    
    }; // End Struct BoneCombinationKey
    CGE_MAP_DECLARE(BoneCombinationKey, cgUInt32Array*, BoneCombinationMap)

    //-------------------------------------------------------------------------
	// Friend List
	//-------------------------------------------------------------------------
    friend bool CGE_API operator < (const AdjacentEdgeKey & key1, const AdjacentEdgeKey & key2);
    friend bool CGE_API operator < (const MeshSubsetKey & key1, const MeshSubsetKey & key2);
    friend bool CGE_API operator < (const WeldKey & key1, const WeldKey & key2);
    friend bool CGE_API operator < (const BoneCombinationKey & key1, const BoneCombinationKey & key2);

    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    void                    prepareQueries              ( );
    bool                    serializeMesh               ( );
    void                    generateDefaultMaterial     ( );
    bool                    generateVertexComponents    ( bool weld );
    bool                    generateVertexNormals       ( cgUInt32 * adjacency, cgUInt32Array * remapArray = CG_NULL );
    bool                    generateVertexTangents      ( );
    cgUInt32              * generateAdjacency           ( );
    bool                    weldVertices                ( cgUInt32Array * vertexRemap = CG_NULL );
    void                    renderMeshData              ( cgRenderDriver * driver, cgMeshDrawMode::Base mode, const cgMaterialHandle * material, cgUInt32 faceStart, cgUInt32 faceCount, cgUInt32 vertexStart, cgUInt32 vertexCount );
    bool                    restoreBuffers              ( );
    bool                    pickMeshSubset              ( cgUInt32 dataGroupId, cgCameraNode * pCamera, const cgSize & ViewportSize, const cgTransform & ObjectTransform, const cgVector3 & rayOrigin, const cgVector3 & rayDirection, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut, cgUInt32 & intersectedFaceOut, cgMaterialHandle & intersectedMaterialOut );
    bool                    sortMeshData                ( bool optimize, bool buildHardwareBuffers );
    
    //-------------------------------------------------------------------------
	// Protected Static Functions
	//-------------------------------------------------------------------------
    static void             buildOptimizedIndexBuffer   ( const MeshSubset * subset, cgUInt32 * sourceBuffer, cgUInt32 * destinationBuffer, cgUInt32 minimumVertex, cgUInt32 maximumVertex );
    static cgFloat          findVertexOptimizerScore    ( const OptimizerVertexInfo * vertexInfo );
    static bool             subsetSortPredicate         ( const MeshSubset * lhs, const MeshSubset * rhs );

    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    // Resource loading properties.
    cgUInt32                mSourceRefId;           // Reference identifier of the mesh source data from which we should load (if any).
    bool                    mFinalizeMesh;          // Should the mesh be finalized after import?
    bool                    mForceTangentGen;       // Should we force the re-generation of tangent space vectors?
    bool                    mForceNormalGen;        // Should we force the re-generation of vertex normals?
    bool                    mDisableFinalSort;      // Allows derived classes to disable / enable the automatic re-sort operation that happens during several operations such as setFaceMaterial(), etc.

    // Serialization
    bool                    mMeshSerialized;        // Has data for this mesh been serialized yet? (i.e. the initial insert).
    cgUInt32                mDBStreamId;            // Identifier of the vertex stream database record associated with this mesh (if any).
    bool                    mDBIndicesDirty;        // The index buffer has been updated and needs to be re-serialized.
    bool                    mDBFormatDirty;         // The vertex format has been updated and needs to be re-serialized.
    bool                    mDBVertexStreamDirty;   // The vertex buffer has been updated and needs to be re-serialized.
    bool                    mDBSubsetsDirty;        // Subset data has been updated and needs to be re-serialized.
    bool                    mDBSkinDataDirty;       // Skin binding data has been updated and needs to be re-serialized.
    bool                    mDBBonePalettesDirty;   // Bone palette data has been updated and needs to be re-serialized.
    
    // Mesh data
    cgByte                * mSystemVB;              // The vertex data as it exists during data insertion (prior to the actual build) and also used as the system memory copy.
    cgVertexFormat        * mVertexFormat;          // Vertex format used for the mesh internal vertex data.
    cgUInt32              * mSystemIB;              // The final system memory copy of the index buffer.
    SubsetKeyArray          mTriangleData;          // Material and data group information for each triangle.
    cgVertexBufferHandle    mHardwareVB;            // After constructing the mesh, this will contain the actual hardware vertex buffer resource
    cgIndexBufferHandle     mHardwareIB;            // After constructing the mesh, this will contain the actual hardware index buffer resource
    cgMaterialHandle        mDefaultMaterial;       // Default material to use if data was added with no valid material.
    cgUInt32                mDefaultColor;          // Color to assign to the default material when drawing.
    
    // Mesh data look up tables
    SubsetArray             mMeshSubsets;           // The actual list of subsets maintained by this mesh.
    DataGroupSubsetMap      mDataGroups;            // A map containing lookup information which maps data groups to subsets batched by material.
    MaterialSubsetMap       mMaterials;             // A map containing lookup information that maps materials to subsets.
    SubsetKeyMap            mSubsetLookup;          // Quick binary tree lookup of existing subsets based on material AND data group id.
    
    // Mesh properties
    bool                    mHardwareMesh;          // Does the mesh use a hardware vertex/index buffer?
    bool                    mOptimizedMesh;         // Was the mesh optimized when it was prepared?
    cgBoundingBox           mBoundingBox;           // Axis aligned bounding box describing object dimensions (in object space)
    cgUInt32                mFaceCount;             // Total number of faces in the prepared mesh.
    cgUInt32                mVertexCount;           // Total number of vertices in the prepared mesh.
    cgMaterialHandleArray   mObjectMaterials;       // Cached array of materials used / retrieved only by the render control batching mechanism (by reference).

    // Mesh data preparation
    cgMeshStatus::Base      mPrepareStatus;         // Preparation status of the mesh (i.e. has it been constructed yet).
    PreparationData         mPrepareData;           // Input data used for constructing the final mesh.

    // Skin binding information
    cgSkinBindData        * mSkinBindData;          // Data that describes how the mesh should be bound as a skin with supplied bone matrices.
    BonePaletteArray        mBonePalettes;          // List of each of the unique combinations of bones to use during rendering.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMesh;
    static cgWorldQuery mDeleteSubsets;
    static cgWorldQuery mInsertSubset;
    static cgWorldQuery mUpdateIndices;
    static cgWorldQuery mUpdateVertexFormat;
    static cgWorldQuery mDeleteStreams;
    static cgWorldQuery mInsertStream;
    static cgWorldQuery mUpdateStream;
    static cgWorldQuery mDeleteSkinBindData;
    static cgWorldQuery mInsertSkinInfluence;
    static cgWorldQuery mDeleteBonePalettes;
    static cgWorldQuery mInsertBonePalette;
    static cgWorldQuery mLoadMesh;
    static cgWorldQuery mLoadStreams;
    static cgWorldQuery mLoadSubsets;
    static cgWorldQuery mLoadSkinBindData;
    static cgWorldQuery mLoadBonePalettes;
};

//-----------------------------------------------------------------------------
// Global Operators
//-----------------------------------------------------------------------------
inline bool CGE_API operator < (const cgMesh::AdjacentEdgeKey & key1, const cgMesh::AdjacentEdgeKey & key2);
inline bool CGE_API operator < (const cgMesh::MeshSubsetKey & key1, const cgMesh::MeshSubsetKey & key2);
inline bool CGE_API operator < (const cgMesh::WeldKey & key1, const cgMesh::WeldKey & key2);
inline bool CGE_API operator < (const cgMesh::BoneCombinationKey & key1, const cgMesh::BoneCombinationKey & key2);

#endif // !_CGE_CGMESH_H_