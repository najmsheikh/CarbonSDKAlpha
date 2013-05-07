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
// File: cgLandscape.h                                                       //
//                                                                           //
// Desc: Contains classes responsible for managing and rendering scene       //
//       landscape / terrain data.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLANDSCAPE_H_ )
#define _CGE_CGLANDSCAPE_H_

//-----------------------------------------------------------------------------
// cgLandscape Header Includes
//-----------------------------------------------------------------------------
#include <World/cgLandscapeTypes.h>
#include <World/cgWorldQuery.h>
#include <World/cgSpatialTree.h>
#include <Resources/cgResourceHandles.h>
#include <Math/cgLeastSquares.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScene;
class cgSampler;
class cgRenderDriver;
class cgCameraNode;
class cgLandscapeTextureData;
class cgTerrainBlock;
class cgHeightMap;
class cgTerrainLOD;
class cgLandscapeSubNode;
class cgVertexFormat;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgTerrainVertex (Class)
/// <summary>
/// Vertex class used to construct & store vertex components for terrain
/// geometry.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTerrainVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors For This Class.
    //-------------------------------------------------------------------------
    cgTerrainVertex( ) {}
    cgTerrainVertex( cgFloat _x, cgFloat _y, cgFloat _z, const cgVector3 & _normal, cgUInt32 _color = 0 ) :
        position(_x,_y,_z), normal(_normal), color(_color) {};
    cgTerrainVertex( const cgVector3 & _position, const cgVector3 & _normal, cgUInt32 _color = 0 ) :
        position(_position), normal(_normal), color(_color) {};

    //-------------------------------------------------------------------------
    // Public Variables For This Class
    //-------------------------------------------------------------------------
	cgVector3 position;           // Vertex position
    cgVector3 normal;             // Vertex normal vector
	cgUInt32  color;              // Vertex color

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 declarator[4];
};

//-----------------------------------------------------------------------------
// Name : cgLandscape (Class)
/// <summary>
/// Represents scene landscape data. Derived from spatial tree in order to 
/// supply hierarchical horizon occlusion culling.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLandscape : public cgSpatialTree
{
    // ToDo: 9999 - Eventually this will be cgSpatialTree?
    DECLARE_SCRIPTOBJECT( cgLandscape, "Landscape" )

    // Friend List
    friend class cgLandscapeTextureData;

public:
    //-------------------------------------------------------------------------
    // Constants
    //-------------------------------------------------------------------------
    static const cgInt MaxLandscapeLOD = 7;

    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum DrawResult
    {
        /// <summary>Specified horizon was visible on the screen but entirely occluded.</summary>
        HorizonOccluded = 0,
        /// <summary>Horizon was visible above the current horizon line.</summary>
        HorizonVisible  = 1,
        /// <summary>The horizon geometry was off the screen and could not be tested.</summary>
        HorizonClipped  = 2,
        /// <summary>The horizon edge was invalid (perhaps it ran in the incorrect direction).</summary>
        HorizonInvalid  = 3
    
    }; // End Enum DrawResult

    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    // Local structure used to pass data to effect
    struct EffectLayerData
    {
        cgVector2 scale;
        cgVector2 baseScale;
        cgVector4 rotation;
        cgVector2 offset;
        cgInt32   tilingReduction;    // 0 or 1

    }; // End Struct EffectLayerData

    struct EffectProceduralData
    {
        cgVector3 heightParams;       // X = Center, Y = -(1/FadeRange), Z = ((Max-Center)+FadeRange)/FadeRange
        cgVector3 slopeAxis;          // Direction vector.
        cgVector2 slopeParams;        // X = SlopeScale, Y = SlopeBias
        cgFloat   weight;

    }; // End Struct EffectProceduralData

    struct ProceduralLayerRef
    {
        /// <summary>Identifier for the layer as it exists in the database (0 if it does not).</summary>
        cgUInt32            layerId;
        /// <summary>Name of this procedural layer.</summary>
        cgString            name;
        /// <summary>The actual layer type being referenced.</summary>
        cgMaterialHandle    layerType;
        /// <summary>Minimum height at which this layer can occur.</summary>
        cgFloat             minimumHeight;
        /// <summary>Maximum height at which this layer can occur.</summary>
        cgFloat             maximumHeight;
        /// <summary>Distance over which the layer will fade in / out based on height.</summary>
        cgFloat             heightAttenuationBand;
        /// <summary>A vector describing the direction of "slope" we are most interested in.</summary>
        cgVector3           slopeAxis;
        /// <summary>Amount to scale the slope factor (Normal.Y^2) used to generate the slope weight.</summary>
        cgFloat             slopeScale;
        /// <summary>Amount to bias the slope factor (Normal.Y^2) used to generate the slope weight.</summary>
        cgFloat             slopeBias;
        /// <summary>Overall weight to apply to this layer.</summary>
        cgFloat             weight;
        /// <summary>Slope parameters should be assumed inverted.</summary>
        bool                invertSlope;
        /// <summary>Height parameters should be ignored?</summary>
        bool                enableHeight;

    }; // End Struct ProceduralLayerRef

    struct ProceduralDrawBatch
    {
        /// <summary>The list of layers being referenced.</summary>
        std::vector<cgInt>              layers;
        /// <summary>The list of blocks to draw.</summary>
        std::vector<cgTerrainBlock*>    blocks;

    }; // End Struct ProceduralDrawBatch

    struct CullDescriptor
    {
        /// <summary>Maximum size of the objects that will be allowed to draw up to this distance.</summary>
        cgFloat maximumSize;
        /// <summary>Squared version of the above value (optimization).</summary>
        cgFloat maximumSizeSquared;
        /// <summary>Distance after which these objects will be culled.</summary>
        cgFloat distance;
        /// <summary>Squared version of the above value (optimization).</summary>
        cgFloat distanceSquared;

    }; // End Struct CullDescriptor

    //-------------------------------------------------------------------------
    // Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( ProceduralLayerRef, ProceduralLayerArray )
    CGE_VECTOR_DECLARE( cgTerrainBlock*, TerrainBlockArray )
    CGE_VECTOR_DECLARE( cgTerrainLOD*, TerrainLODArray )

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLandscape( cgScene * scene );
    virtual ~cgLandscape( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                            load                    ( cgUInt32 landscapeId );
    bool                            import                  ( cgHeightMap * heightMap, const cgVector3 & dimensions, cgUInt32 initFlags );
    bool                            import                  ( const cgLandscapeImportParams & params );
    bool                            heightMapUpdated        ( const cgRect & bounds );
    bool                            setDimensions           ( const cgVector3 & dimensions );
    bool                            setDimensions           ( const cgVector3 & dimensions, bool rebuildTerrain );
    cgScene                       * getScene                ( ) const;
    bool                            shouldSerialize         ( ) const;
    cgUInt32                        getDatabaseId           ( ) const;
    
    // Terrain Queries
    cgFloat                         getTerrainHeight        ( cgFloat x, cgFloat z ) const;
    cgFloat                         getTerrainHeight        ( cgFloat x, cgFloat z, bool accountForLOD ) const;
    cgVector3                       getHeightMapNormal      ( cgInt32 x, cgInt32 z ) const;
    bool                            getRayIntersect         ( const cgVector3 & origin, const cgVector3 & velocity, cgFloat & t ) const;
    bool                            getRayIntersect         ( const cgVector3 & origin, const cgVector3 & velocity, cgFloat & t, cgFloat accuracy ) const;

    // Procedural layer management
    void                            updateProceduralLayers  ( const ProceduralLayerArray & layers );
    const ProceduralLayerRef      & getProceduralLayer      ( cgUInt32 layerId ) const;
    const ProceduralLayerArray    & getProceduralLayers     ( ) const;
    bool                            setProceduralLayer      ( cgUInt32 layerId, const ProceduralLayerRef & layer );

    // Terrain painting
    void                            beginPaint              ( const cgMaterialHandle & material, const cgLandscapePaintParams & params );
    void                            endPaint                ( );
    bool                            paint                   ( cgFloat fromX, cgFloat fromZ, cgFloat toX, cgFloat toZ );
    void                            updatePaintPreview      ( );
    bool                            getBlendMapPaintData    ( const cgRect & sourceBounds, cgByteArray & data ) const;

    // Properties
    void                            setTerrainDetail        ( cgFloat value );
    const cgInt32Array            & getMipLookUp            ( ) const;
    cgUInt32                        getFlags                ( ) const;
    const cgVector3               & getTerrainScale         ( ) const;
    const cgVector3               & getTerrainOffset        ( ) const;
    cgFloat                         getTerrainDetail        ( ) const;
    cgSize                          getHeightMapSize        ( ) const;
    const cgSize                  & getBlockSize            ( ) const;
    const cgSize                  & getBlockBlendMapSize    ( ) const;
    const cgSize                  & getBlockLayout          ( ) const;
    cgHeightMap                   * getHeightMap            ( ) const;
    cgUInt32Array                 & getColorMap             ( );
    const TerrainLODArray         & getLODData              ( ) const;
    const TerrainBlockArray       & getTerrainBlocks        ( ) const;
    cgTextureHandle                 getNormalTexture        ( ) const;
    bool                            isYVariant              ( ) const;

    // Rendering Data
    cgSampler                     * getSharedColorSampler   ( cgUInt32 index ) const;
    cgSampler                     * getSharedNormalSampler  ( cgUInt32 index ) const;
    cgSampler                     * getBlendMapSampler      ( ) const;

    // Rendering
    bool                            renderPass              ( cgLandscapeRenderMethod::Base pass, cgCameraNode * camera, cgVisibilitySet * visibilityData );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    // ToDo: 9999 - Remove
    //virtual bool                    onComponentCreated      ( cgComponentCreatedEventArgs * e );
    //virtual bool                    onComponentLoading      ( cgComponentLoadingEventArgs * e );
    //virtual void                    onComponentModified     ( cgComponentModifiedEventArgs * e );
    //virtual void                    onComponentDeleted      ( );
    //virtual cgString                getDatabaseTable        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    // ToDo: 9999 - Remove
    //virtual void                    sandboxRender           ( cgUInt32 flags, cgCameraNode * camera, cgVisibilitySet * visibilityData, const cgPlane & gridPlane, cgObjectNode * issuer );
    //virtual bool                    render                  ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer );
    //virtual bool                    renderSubset            ( cgCameraNode * camera, cgVisibilitySet * visibilityData, cgObjectNode * issuer, const ResourceHandle & material );
    //virtual bool                    pick                    ( cgCameraNode * camera, cgObjectNode * issuer, const cgSize & viewportSize, const cgVector3 & origin, const cgVector3 & direction, bool wireframe, cgFloat wireTolerance, cgFloat & distanceOut );
    //virtual bool                    isRenderable            ( ) const;
    //virtual bool                    isShadowCaster          ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTree)
    //-------------------------------------------------------------------------
    virtual cgSpatialTreeSubNode  * allocateNode            ( cgSpatialTreeSubNode * init = CG_NULL );
    virtual cgSpatialTreeLeaf     * allocateLeaf            ( cgSpatialTreeLeaf * init = CG_NULL );
    virtual void                    computeVisibility       ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags, cgSpatialTreeInstance * issuer );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    // ToDo: 9999 - Remove
    //virtual const cgUID           & getReferenceType        ( ) const { return RTID_LandscapeObject; }
    //virtual bool                    queryReferenceType      ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------

    // Constant buffer 'type' structures (referenced by constant buffer members)
    struct ctLayerData
    {
        cgVector4 textureSize;
        cgVector2 scale;
        cgVector2 baseScale;
        cgVector4 rotation;
        cgVector2 offset;
    
    }; // End Struct : ctLayerData

    struct ctProceduralData
    {
        cgVector3 heightParams;     // X = Center, Y = -(1/FadeRange), Z = ((Max-Center)+FadeRange)/FadeRange
        cgVector3 slopeAxis;        // Direction Vector
        cgVector2 slopeParams;      // X = SlopeScale, Y = SlopeBias
        float     weight;
    
    }; // End Struct : ctProceduralData

    // Constant buffer data structures (mapped to constants in the landscape shader).
    struct cbTerrainBaseData
    {
        cgColorValue    color;
        cgVector4       terrainSize;
        cgVector3       terrainOffset;
        float           noiseStrength;

    }; // End Struct : cbTerrainBaseData

    struct cbTerrainLayerData
    {
        ctLayerData     layers[4];

    }; // End Struct : cbTerrainLayerData

    struct cbTerrainProcData
    {
        ctProceduralData    procedural[3];
        
    }; // End Struct : cbTerrainProcData
    
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( ProceduralDrawBatch, ProceduralDrawBatchArray )
    CGE_VECTOR_DECLARE( CullDescriptor, CullDescriptorArray )
    
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries          ( );
    bool                        postInit                ( );
    bool                        buildMipLookUp          ( );
    bool                        buildLODLevel           ( cgUInt32 mipLevel );
    bool                        updateNormalTexture     ( );
    bool                        updateNormalTexture     ( cgRect bounds );
    void                        batchProceduralDraws    ( );
    bool                        initializeTextureData   ( const cgInt32Array & init );

    // ToDo: Temp
    //void        debugDraw           ( cgLandscapeSubNode * node, cgCameraNode * camera, cgUInt32 & visitOrder );

    // Horizon Occlusion Culling
    bool                        compileSpatialTree      ( );
    bool                        buildTree               ( cgUInt32 level, cgLandscapeSubNode * node, const cgBoundingBox & nodeBounds );
    void                        updateTreeVisibility    ( cgSpatialTreeInstance * issuer, cgLandscapeSubNode * node, const cgFrustum & viewFrustum, const cgVector3 & sortOrigin, const cgMatrix & visibilityMatrix, bool occlusionCull, cgVisibilitySet * visibilitySet, cgUInt32 flags );
    bool                        testNodeOcclusion       ( cgLandscapeSubNode * node, const cgVector3 & sortOrigin, const cgMatrix & visibilityMatrix );
    void                        drawHorizonEdge         ( cgVector4 a, cgVector4 b );
    DrawResult                  testHorizonEdge         ( cgVector4 a, cgVector4 b );
    void                        rasterHorizonLine       ( cgFloat x1, cgFloat y1, cgFloat x2, cgFloat y2 );
    DrawResult                  testHorizonLine         ( cgFloat x1, cgFloat y1, cgFloat x2, cgFloat y2 );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>The parent scene to which the landscape belongs.</summary>
    cgScene                       * mParentScene;
    /// <summary>Identifier of the landscape as it exists within the database.</summary>
    cgUInt32                        mLandscapeId;
    /// <summary>Array of individual terrain block objects.</summary>
    TerrainBlockArray               mTerrainBlocks;
    /// <summary>Total number of terrain blocks for entire terrain.</summary>
    cgSize                          mBlockLayout;
    /// <summary>Number of vertices in an individual block.</summary>
    cgSize                          mBlockSize;
    /// <summary>Overall dimensions of the landscape in world space.</summary>
    cgVector3                       mDimensions;
    /// <summary>Offset value for the landscape to position it at the required location.</summary>
    cgVector3                       mOffset;
    /// <summary>The scale values for the terrain (spacing between vertices).</summary>
    cgVector3                       mScale;
    /// <summary>Locally stored copy of the heightmap (if applicable).</summary>
    cgHeightMap                   * mHeightMap;
    /// <summary>Locally stored copy of the color map data (if applicable).</summary>
    cgUInt32Array                   mColorMap;
    /// <summary>Flags from the LandscapeFlags enum specified during load.</summary>
    cgUInt32                        mLandscapeFlags;
    /// <summary>The internal surface shader file used to handle default and special case rendering of the terrain element.</summary>
    cgSurfaceShaderHandle           mLandscapeShader;
    /// <summary>Noise sampler used for reducing tiling artefacts during texturing..</summary>
    cgSampler                     * mNoiseSampler;
    /// <summary>Normal map texture used by shaders.</summary>
    cgTextureHandle                 mNormalTexture;
    /// <summary>Sampler used to grant the shader access to the terrain normal data.</summary>
    cgSampler                     * mNormalSampler;
    /// <summary>The global size of the blend map texture data maintained at each block.</summary>
    cgSize                          mBlendMapSize;
    /// <summary>World space axis aligned bounding box of the entire landscape.</summary>
    cgBoundingBox                   mBounds;
    /// <summary>Format used for rendering.</summary>
    cgVertexFormat                * mVertexFormat;
    /// <summary>Common samplers, used to bind layer color maps to the rendering effect.</summary>
    cgSampler                     * mLayerColorSamplers[4];
    /// <summary>Common samplers, used to bind layer normal maps to the rendering effect.</summary>
    cgSampler                     * mLayerNormalSamplers[4];
    /// <summary>Common sampler, used to bind the blend map textures to the rendering effect.</summary>
    cgSampler                     * mBlendMapSampler;

    // Texture mapping
    /// <summary>We are currently painting?</summary>
    bool                            mIsPainting;
    /// <summary>Painting parameters.</summary>
    cgLandscapePaintParams          mPaintParams;
    /// <summary>The layer type we're painting with.</summary>
    cgMaterialHandle                mPaintLayerType;
    /// <summary>List of layers that should be applied to the terrain procedurally (in order).</summary>
    ProceduralLayerArray            mProceduralLayers;
    /// <summary>List of batches to optimize procedural layer drawing.</summary>
    ProceduralDrawBatchArray        mProceduralBatches;

    // Level of Detail
    /// <summary>Global terrain detail scalar influences the detail level of the terrain.</summary>
    cgFloat                         mTerrainDetail;
    /// <summary>The data for level of detail processing (highest to lowest).</summary>
    TerrainLODArray                 mLODData;
    /// <summary>Geo-Mip optimization VB lookup table.</summary>
    cgInt32Array                    mMipLookUp;
    /// <summary>Describes distances at which certain objects will be culled based on their size.</summary>
    CullDescriptorArray             mCullDistances;
    /// <summary>Distance after which all objects will simply be culled, irrespective of their size.</summary>
    cgFloat                         mObjectCullDistance;
    /// <summary>Squared version of the above distance (optimization).</summary>
    cgFloat                         mObjectCullDistanceSq;

    // Horizon Occlusion Culling
    /// <summary>The horizon buffer which records the height of the horizon at each location on the screen</summary>
    cgFloatArray                    mHorizonBuffer;
    /// <summary>Variable containing the vertical plane, in front of which the occlusion data is valid.</summary>
    cgPlane                         mHorizonNearPlane;
    /// <summary>The currently active viewport size.</summary>
    cgSize                          mViewportSize;
    /// <summary>Cached value describing half the width of the viewport (optimization)</summary>
    cgFloat                         mViewHalfWidth;
    /// <summary>Cached value describing half the height of the viewport (optimization)</summary>
    cgFloat                         mViewHalfHeight;  
    /// <summary>The maximum height currently recorded at any point on the horizon</summary>
    cgFloat                         mMaxHorizonHeight;
    /// <summary>The maximum depth to subdivide the landscape until cells are created.</summary>
    cgUInt32                        mMaxTreeDepth;
    /// <summary>Generate Y-Variant style node bounding boxes.</summary>
    bool                            mYVTree;
    /// <summary>Number of successful leaf occlusions that occured in the prior frame.</summary>
    cgUInt32                        mOcclusionSuccess;
    /// <summary>Number of failed leaf occlusions that occured in the prior frame.</summary>
    cgUInt32                        mOcclusionFailure;
    /// <summary>Next time at which occlusion should be test again, or 0 to sample every frame.</summary>
    cgUInt32                        mNextOcclusionTest;
    /// <summary>Is occlusion culling currently enabled?</summary>
    bool                            mOcclusionCull;

    // States
    /// <summary>Depth stencil states for terrain depth fill pass.</summary>
    cgDepthStencilStateHandle       mDepthFillDepthState;
    /// <summary>Depth stencil states for terrain procedural layer rendering pass.</summary>
    cgDepthStencilStateHandle       mProceduralDepthState;
    /// <summary>Depth stencil states for terrain painted layer rendering pass.</summary>
    cgDepthStencilStateHandle       mPaintedDepthState;
    /// <summary>Depth stencil states for G-buffer post-processing pass.</summary>
    cgDepthStencilStateHandle       mPostProcessDepthState;
    /// <summary>Blend states for terrain procedural layer rendering pass.</summary>
    cgBlendStateHandle              mProceduralBlendState;
    /// <summary>Blend states for terrain painted layer rendering pass.</summary>
    cgBlendStateHandle              mPaintedBlendState;
    /// <summary>Blend states for G-buffer post-processing pass.</summary>
    cgBlendStateHandle              mPostProcessBlendState;

    // Constants
    /// <summary>Base terrain rendering shader constant buffer.</summary>
    cgConstantBufferHandle          mTerrainBaseDataBuffer;
    /// <summary>Constant buffer describing the collection of (up to 4) layer types being processed in the current rendering pass.</summary>
    cgConstantBufferHandle          mTerrainLayerDataBuffer;
    /// <summary>Constant buffer describing the collection of (up to 3) procedural layer generation sets being processed in the current rendering pass.</summary>
    cgConstantBufferHandle          mTerrainProcDataBuffer;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertLandscape;
    static cgWorldQuery     mInsertProceduralLayer;
    static cgWorldQuery     mUpdateLandscapeLayout;
    static cgWorldQuery     mUpdateTextureConfig;
    static cgWorldQuery     mDeleteProceduralLayers;
    static cgWorldQuery     mUpdateProceduralLayer;
    static cgWorldQuery     mLoadLandscape;
    static cgWorldQuery     mLoadTerrainBlocks;
    static cgWorldQuery     mLoadProceduralLayers;

}; // End Class cgLandscape

//-----------------------------------------------------------------------------
// Name : cgTerrainLOD (Class)
/// <summary>
/// Level of detail information for rendering a terrain block.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTerrainLOD
{
public:
    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    /// <summary>Contains information for the skirt which allows a block to connect to a lower LOD.</summary>
    struct BlockSkirt
    {
        /// <summary>Stores reference information for the main LOD index buffer for this skirt.</summary>
        struct LODLevel
        {
            /// <summary>The starting index for this skirt item.</summary>
            cgUInt32    indexStart;
            /// <summary>The number of primitives to render for this skirt.</summary>
            cgUInt32    primitiveCount;
        
        }; // End Struct LODLevel

        /// <summary>Information for connecting the block to a neighbor at this or any lower LOD.</summary>
        std::vector<LODLevel> levels;
    
    }; // End Struct BlockSkirt

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgTerrainLOD( );
    ~cgTerrainLOD( );

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    /// <summary>Main index buffer for this level of detail.</summary>
    cgIndexBufferHandle indexBuffer;
    /// <summary>The four skirts for this piece of terrain.</summary>
    BlockSkirt          skirts[4];
    /// <summary>The primitive count for just the interior portion of the block.</summary>
    cgUInt32            interiorPrimitiveCount;

}; // End Class cgTerrainLOD

//-----------------------------------------------------------------------------
// Name : cgTerrainBlock (Class)
/// <summary>
/// A portion of the overall terrain. Manages vertex buffer creation, 
/// various levels of detail features, and maintains the loaded terrain 
/// information.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgTerrainBlock
{
public:
    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    /// <summary>Enumeration items for refering to specific edges / sides of the block.</summary>
    enum EdgeSide
    {
        /// <summary>North edge of the block.</summary>
        North = 0,
        /// <summary>East edge of the block.</summary>
        East  = 1,
        /// <summary>South edge of the block.</summary>
        South = 2,
        /// <summary>West edge of the block.</summary>
        West  = 3
    
    }; // End Enum EdgeSide

    /// <summary>The rendering approach that should be employed when drawing the block.</summary>
    enum RenderMode
    {
        Wire        = 0,
        Simple      = 1,
        Procedural  = 2,
        Painted     = 3
    
    }; // End Enum RenderMode

    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    /// <summary>More comprehensive rectangle structure stores additional information.</summary>
    struct CGE_API TerrainRect : public cgRect
    {
        /// <summary>Size of the item inside which the rectangle exists.</summary>
        cgInt pitchX, pitchY;
    };

    /// <summary>Stores information about the piece of terrain to load / manage.</summary>
    // ToDo: 9999 - Even necessary any more now we're using the database?
    struct CGE_API TerrainSection
    {
        /// <summary>Name of the height map file to load from.</summary>
        cgInputStream   heightMapStream;
        /// <summary>Existing heightmap buffer to use instead of a file.</summary>
        cgInt16       * heightMapBuffer;
        /// <summary>The rectangle of the terrain we're managing (in the space of the entire terrain).</summary>
        TerrainRect     blockBounds;
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgTerrainBlock( cgLandscape * parent, cgUInt32 blockIndex, const TerrainSection & info );
    ~cgTerrainBlock( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    importBlock             ( );
    bool                    loadBlock               ( cgWorldQuery & blockQuery );
    bool                    dataUpdated             ( );
    void                    calculateLOD            ( cgCameraNode * camera );
    void                    calculateLOD            ( cgCameraNode * camera, cgFloat terrainDetail );
    void                    calculateLOD            ( cgCameraNode * camera, cgFloat terrainDetail, cgInt32 detailLevelAdjust );
    cgVector3               getHeightMapNormal      ( cgInt32 x, cgInt32 z ) const;
    short                   getHeightMapHeight      ( cgInt32 x, cgInt32 z ) const;
    cgFloat                 getTerrainHeight        ( cgFloat x, cgFloat z ) const;
    cgFloat                 getTerrainHeight        ( cgFloat x, cgFloat z, bool accountForLOD ) const;
    const cgBoundingBox   & getBoundingBox          ( ) const;
    void                    draw                    ( cgRenderDriver * driver, RenderMode mode );

    // Properties
    cgInt32                 getCurrentLOD           ( ) const;
    cgUInt32                getBlockIndex           ( ) const;
    cgUInt32                getDatabaseId           ( ) const;
    bool                    isVisible               ( ) const;
    cgTerrainBlock*         getNeighbor             ( EdgeSide side ) const;
    cgLandscapeTextureData* getTextureData          ( ) const;
    void                    setCurrentLOD           ( cgInt32 level );
    void                    setNeighbor             ( EdgeSide side, cgTerrainBlock * block );
    void                    setVisible              ( bool visible );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void            prepareQueries          ( );
    bool            buildVertexBuffer       ( const cgInt32Array & mipLookUp );
    bool            updateVertexBuffer      ( const cgInt32Array & mipLookUp );
    void            calculateLODVariance    ( );
    cgUInt32        GetHeightMapIndex       ( cgInt32 x, cgInt32 z ) const;

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Identifier of the block as it exists in the database (if any).</summary>
    cgUInt32                mBlockId;
    /// <summary>Index of this block in parent's block array.</summary>
    cgUInt32                mBlockIndex;
    /// <summary>Parent landscape element.</summary>
    cgLandscape           * mParent;
    /// <summary>Reference to the heightmap for this portion of the terrain.</summary>
    cgInt16               * mHeightMap;
    /// <summary>Do we own the heightmap memory?</summary>
    bool                    mOwnsHeightMap; // ToDo: 9999 - Still need this?
    /// <summary>Vertex color map for this portion of the terrain.</summary>
    cgUInt32              * mColorMap;
    /// <summary>Information about the terrain section.</summary>
    TerrainSection          mSection;
    /// <summary>The rectangle of the terrain that we are physically managing (may contain additional border pixel for instance).</summary>
    TerrainRect             mManagedTerrainBounds; // ToDo: 9999 - Still need this?
    /// <summary>The rectangle into the heightmap buffer that describe the active area mapped to vertices (no border pixels).</summary>
    TerrainRect             mActiveHeightMapBounds; // ToDo: 9999 - Still need this?
    /// <summary>Axis aligned bounding box describing the extents of the terrain block in the space of its parent (cgLandscape).</summary>
    cgBoundingBox           mBounds;
    /// <summary>The 4 neighboring terrain blocks.</summary>
    cgTerrainBlock        * mNeighbors[4];
    /// <summary>Individual vertex buffer for the entire terrain block.</summary>
    cgVertexBufferHandle    mVertexBuffer;
    /// <summary>The error metric values for this block of terrain used to determine best time to switch LOD.</summary>
    cgFloat                 mLODVariance[cgLandscape::MaxLandscapeLOD];
    /// <summary>Number of active variance descriptors in the above array.</summary>
    cgInt32                 mVarianceCount;
    /// <summary>The current level of detail.</summary>
    cgInt32                 mCurrentLOD;
    /// <summary>Object responsible for managing how this block is textured.</summary>
    cgLandscapeTextureData* mTextureData;
    /// <summary>Is the block visible?</summary>
    bool                    mVisible;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBlock;
    static cgWorldQuery     mInsertBlockLOD;
    static cgWorldQuery     mUpdateBlockHeights;
    static cgWorldQuery     mDeleteBlockLODs;
    static cgWorldQuery     mLoadBlockLODs;

}; // End Class cgTerrainBlock

//---------------------------------------------------------------------------------
// Name : cgLandscapeSubNode (Class)
/// <summary>
/// The node class specific to the spatial tree type implemented by the
/// landscape system.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgLandscapeSubNode : public cgSpatialTreeSubNode
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
     cgLandscapeSubNode( cgLandscape * parent );
     cgLandscapeSubNode( cgSpatialTreeSubNode * init, cgSpatialTree * parent );
    ~cgLandscapeSubNode();

    //-----------------------------------------------------------------------------
    // Public Methods
    //---------------------------------------------------------------------------
    void            updateOcclusionData ( );

    //-----------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTreeSubNode)
    //---------------------------------------------------------------------------
    virtual void    nodeConstructed     ( );
    virtual bool    collectLeaves       ( cgSpatialTreeInstance * issuer, cgSpatialTree * tree, cgSceneLeafArray & leaves, const cgBoundingBox & bounds, bool autoCollect = false );

    //-----------------------------------------------------------------------------
    // Public Variables
    //-----------------------------------------------------------------------------
    cgPlane             minimumHorizonPlane;    // The minimum horizon plane
    cgPlane             maximumHorizonPlane;    // The maximum horizon plane
    cgVector3         * minimumHorizonPoints;   // Minimum horizon geometry used for building the horizon buffer
    cgVector3         * maximumHorizonPoints;   // Maximum horizon geometry used for testing for occlusion
    
protected:
    //-----------------------------------------------------------------------------
    // Protected Variables
    //-----------------------------------------------------------------------------
    cgLeastSquaresSums  mLSSums;        // The least squares sum data for this node
    cgLandscape       * mLandscape;     // Parent landscape object
};

//---------------------------------------------------------------------------------
// Name : cgLandscapeCell (Class)
/// <summary>
/// The leaf class specific to the spatial tree type implemented by the
/// landscape system which also acts as a specific "cell" of the landscape for
/// management of additional landscape data.
/// </summary>
//---------------------------------------------------------------------------------
class CGE_API cgLandscapeCell : public cgSpatialTreeLeaf
{
public:
    //-----------------------------------------------------------------------------
    // Constructors & Destructors
    //-----------------------------------------------------------------------------
     cgLandscapeCell();
     cgLandscapeCell( cgSpatialTreeLeaf * init, cgSpatialTree * parent );

    //-----------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgSpatialTreeLeaf)
    //---------------------------------------------------------------------------
    
    //-----------------------------------------------------------------------------
    // Public Variables
    //-----------------------------------------------------------------------------
};

//-----------------------------------------------------------------------------
// Name : cgLandscapeTextureData (Class)
/// <summary>
/// Assigned to each individual block, this class is responsible for
/// managing the blend map, layer list and setting up the device ready for
/// rendering the block.
/// </summary>
//-----------------------------------------------------------------------------
class cgLandscapeTextureData
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgLandscapeTextureData( cgLandscape * landscape, cgTerrainBlock * parentBlock );
    ~cgLandscapeTextureData( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                loadLayers          ( );
    bool                getPaintRectangle   ( cgRectF bounds, cgInt32 & startX, cgInt32 & startY, cgInt32 & endX, cgInt32 & endY, cgVector2 & paintCenter, cgVector2 & paintOrigin, cgVector2 & paintDelta ) const;
    bool                beginPaint          ( const cgMaterialHandle & type, const cgLandscapePaintParams & params );
    void                endPaint            ( );
    bool                paint               ( cgFloat x, cgFloat z );
    bool                paintLine           ( cgFloat fromX, cgFloat fromZ, cgFloat toX, cgFloat toZ );
    bool                setPixel            ( cgInt32 x, cgInt32 y );
    void                updatePaintPreview  ( );
    
    bool                beginDraw           ( cgRenderDriver * driver );
    bool                beginDrawPass       ( cgRenderDriver * driver );
    bool                endDrawPass         ( cgRenderDriver * driver );

    // Notifications
    // ToDo: 9999 - With material
    //void                layerTypeRemoved    ( cgLandscapeLayerType * type, cgInt32 typeId );
    //void                layerTypeUpdated    ( cgLandscapeLayerType * type, cgLandscapeLayerType * newType, cgInt32 typeId );

    // Properties
    bool                isPainting          ( ) const;
    bool                isEmpty             ( ) const;
    const cgByteArray & getPaintData        ( ) const;
    cgRectF             getBlendMapWorldArea( ) const;
    cgRect              getBlendMapArea     ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct LayerReference
    {
        /// <summary>The actual layer type being referenced.</summary>
        cgMaterialHandle        layerType;
        /// <summary>Number of times that the above layer is referenced with a weight > 0 (pixel count).</summary>
        cgInt32                 referenceCount;
        /// <summary>Blend map data for this layer within the confines of this block.</summary>
        cgByteArray             blendMap;
        /// <summary>Temporary buffer used to store painted data.</summary>
        cgByteArray             blendMapPaint;
        /// <summary>The index of the combined blend map in which the layer data should be placed.</summary>
        cgUInt32                combinedBlendMap;
        /// <summary>The channel of the 32 bit A8R8G8B8 blend map in which this layer should be placed.</summary>
        cgByte                  channel;
        /// <summary>Has this layer been updated in such a way that will require the texture to update?</summary>
        bool                    dirty;
        /// <summary>The rectangle that describes the area of the blend map that needs to be updated.</summary>
        cgRect                  dirtyRectangle;
        /// <summary>The final blend map will be dirty on completion (separate from whether or not the texture currently needs updating.</summary>
        bool                    finalDirty;
        /// <summary>The area of the final blend map that needs to be updated.</summary>
        cgRect                  finalDirtyRectangle;
        /// <summary>Unique identifier for the layer as it exists within the database (if at all).</summary>
        cgUInt32                layerId;
        /// <summary>When true, this layer describes temporary 'erasing' data used by the erase painting tools.</summary>
        bool                    eraseLayer;

    }; // End Struct LayerReference

    struct BatchedMap
    {
        /// <summary>The underlying texture resource used to maintain the layer blend map data for rendering.</summary>
        cgTextureHandle texture;
        /// <summary>List of channels that are currently in use (bit-pattern, 0x1 = Channel 0, 0x2 = Channel 1, 0x4 = Channel 2, 0x8 = Channel 3).</summary>
        cgUInt32        channelUsage;

    }; // End Struct BatchedMap

    struct RenderBatch
    {
        /// <summary>List of layers to render in the batch.</summary>
        cgUInt32                layers[4];
        /// <summary>Number of layers to render in the batch.</summary>
        cgInt32                 layerCount;
        /// <summary>Maximum number of layers that can be packed into this batch (i.e. because of swizzling).</summary>
        cgInt32                 maximumLayers;
        /// <summary>The index of the combined blend map in which the layer data should be placed.</summary>
        cgInt32                 combinedBlendMap;
        /// <summary>Should we swizzle the layer data (i.e. these 2 layers in the last two channels of the above texture?).</summary>
        bool                    swizzle;
        /// <summary>Constant buffer describing the collection of (up to 4) layer types being processed in the current rendering pass.</summary>
        cgConstantBufferHandle  layerDataBuffer;
    
    }; // End Struct RenderBatch

    // Constant buffer data structures (mapped to constants in the landscape shader).
    struct cbTerrainPaintData
    {
        cgVector2       blendMapScale;
        cgVector2       blendMapOffset;
        cgVector2       passCount;      // x = count, y = 1/count
        float           noiseStrength;

    }; // End Struct : cbTerrainPaintData

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE( LayerReference, LayerReferenceArray )
    CGE_VECTOR_DECLARE( BatchedMap, BatchedMapArray )
    CGE_VECTOR_DECLARE( RenderBatch, RenderBatchArray )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void    updateLayerTexture  ( LayerReference & layer, const cgRect & bounds );
    void    filterPaintLayer    ( cgByteArray & data );
    void    optimizeLayers      ( );
    void    prepareQueries      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Parent terrain block to which this data belongs.</summary>
    cgTerrainBlock            * mParentBlock;
    /// <summary>Top level landscape object to which this data belongs.</summary>
    cgLandscape               * mParentLandscape;
    /// <summary>List of the terrain layers used within this block.</summary>
    LayerReferenceArray         mLayers;
    /// <summary>List of combined 4 channel blend map textures.</summary>
    BatchedMapArray             mCombinedBlendMaps;
    /// <summary>List of render passes (and associated batch data).</summary>
    RenderBatchArray            mRenderBatches;
    /// <summary>Active interior region of the blend map that is actually mapped to the parent block.</summary>
    cgRect                      mActiveBlendMapBounds;
    /// <summary>The size of the blend map texture data INCLUDING its border.</summary>
    cgSize                      mBlendMapSize;
    /// <summary>Describes the number of texels per terrain quad within our active paint area.</summary>
    cgSizeF                     mBlendMapResolution;
    /// <summary>The current rendering pass.</summary>
    cgInt32                     mCurrentPass;
    /// <summary>We are currently painting to this block.</summary>
    bool                        mIsPainting;
    /// <summary>The layer into which we're painting.</summary>
    cgInt32                     mPaintLayer;
    /// <summary>Painting parameters.</summary>
    cgLandscapePaintParams      mParams;
    /// <summary>Configuration constants for painted layer rendering pass.</summary>
    cgConstantBufferHandle      mTerrainPaintDataBuffer;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries
    static cgWorldQuery mInsertLayer;
    static cgWorldQuery mDeleteLayer;
    static cgWorldQuery mUpdateLayerPaintData;
    static cgWorldQuery mLoadLayers;
    
}; // End Class cgLandscapeTextureData

#endif // !_CGE_CGLANDSCAPE_H_