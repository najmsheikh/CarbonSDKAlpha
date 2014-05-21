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
// Name : cgClutterMaterial.h                                                //
//                                                                           //
// Desc : Provides classes which outline properties for scene clutter        //
//        (foliage, rocks, trees, etc.) that can be applied to landscapes    //
//        and other scene elements.                                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGCLUTTERMATERIAL_H_ )
#define _CGE_CGCLUTTERMATERIAL_H_

//-----------------------------------------------------------------------------
// cgClutterMaterial Header Includes
//-----------------------------------------------------------------------------
#include <Resources/cgMaterial.h>
#include <Rendering/cgRenderingTypes.h>
#include <World/cgWorldQuery.h>
#include <Math/cgRandom.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgClutterMaterial;
class cgSampler;
class cgVertexFormat;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {B45809CC-B7ED-4A0B-A3B4-FA88308E1D0C}
const cgUID RTID_ClutterMaterial = { 0xb45809cc, 0xb7ed, 0x4a0b, { 0xa3, 0xb4, 0xfa, 0x88, 0x30, 0x8e, 0x1d, 0xc } };

//-----------------------------------------------------------------------------
// Global Enumerations
//-----------------------------------------------------------------------------
namespace cgClutterType
{
    enum Base
    {
        Foliage  = 0

    }; // End Enum : Base

} // End Namespace : cgClutterType

namespace cgFoliageRenderMethod
{
    enum Base
    {
        Billboard   = 0,
        ClusterMesh

    }; // End Enum : Base

} // End Namespace : cgFoliageRenderMethod

namespace cgFoliageAnimationMethod
{
    enum Base
    {
        None    = 0

    }; // End Enum : Base

} // End Namespace : cgFoliageAnimationMethod

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFoliageVertex (Class)
/// <summary>
/// Vertex class used to construct & store vertex components for foliage
/// geometry.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFoliageVertex
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgFoliageVertex( ) {}
    cgFoliageVertex( cgFloat _x, cgFloat _y, cgFloat _z, const cgVector3 & _normal, const cgVector2 & _coords, cgUInt32 _color = 0 ) :
        position(_x,_y,_z), normal(_normal), color(_color), coords(_coords) {};
    cgFoliageVertex( const cgVector3 & _position, const cgVector3 & _normal, const cgVector2 & _coords, cgUInt32 _color = 0 ) :
        position(_position), normal(_normal), color(_color), coords(_coords) {};

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
	cgVector3 position;     // Vertex position
    cgVector3 normal;       // Vertex normal vector
    cgUInt32  color;        // Vertex color
    cgVector2 coords;       // Single texture coordinate.

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
    const static D3DVERTEXELEMENT9 declarator[5];
};

//-----------------------------------------------------------------------------
//  Name : cgClutterLayer (Base Class)
/// <summary>
/// Base class from which the different types of clutter layer definition
/// classes are derived for specific types of clutter (i.e. foliage).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgClutterLayer : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgClutterLayer, "ClutterLayer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgClutterLayer( cgClutterMaterial * parent );
             cgClutterLayer( cgClutterMaterial * parent, cgClutterLayer * initLayer );
    virtual ~cgClutterLayer( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual cgClutterType::Base getType             ( ) const = 0;
    virtual void                setName             ( const cgString & name );
    virtual bool                serializeLayer      ( ) { return true; };
    virtual void                deleteLayer         ( ) {};
    virtual bool                loadLayer           ( cgUInt32 sourceLayerId, bool cloning ) { return true; };
    virtual bool                begin               ( cgRenderDriver * driver, bool depthFill ) { return false; }
    virtual void                end                 ( cgRenderDriver * driver, bool depthFill ) { }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgClutterMaterial         * getMaterial         ( );
    const cgString            & getName             ( ) const;
    cgRandom::NoiseGenerator  & getPerlinGenerator  ( );
    bool                        isDirtySince        ( cgUInt32 frame ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Static Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 NameDirty       = 0x1;
    static const cgUInt32 AllDirty        = 0xFFFFFFFF;

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>The parent material that owns this layer.</summary>
    cgClutterMaterial         * mMaterial;
    /// <summary>The name of this layer -- just for identification purposes.</summary>
    cgString                    mName;
    /// <summary>The unique identifier of this layer in the layer type database.</summary>
    cgUInt32                    mDatabaseId;
    /// <summary>Has data for this layer been serialized yet? (i.e. the initial insert).</summary>
    bool                        mLayerSerialized;
    /// <summary>Which components of the material are dirty?</summary>
    cgUInt32                    mDBDirtyFlags;
    /// <summary>Perlin noise generator configured for this layer</summary>
    cgRandom::NoiseGenerator    mPerlin;
    /// <summary>Frame on which this layer's properties were last modified.</summary>
    cgUInt32                    mLastModifiedFrame;
};

//-----------------------------------------------------------------------------
//  Name : cgClutterCell (Base Class)
/// <summary>
/// Base class from which the different types of clutter layer management 
/// classes are derived (i.e. foliage). This is where the actual data for 
/// the clutter is managed.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgClutterCell : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgClutterCell, "ClutterCell" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgClutterCell( cgClutterLayer * layer );
    virtual ~cgClutterCell( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                shouldRender        ( cgCameraNode * camera ) const { return false; }
    virtual void                render              ( cgRenderDriver * driver, bool depthFill ) {};
    virtual void                invalidate          ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgClutterLayer            * getLayer            ( );
    void                        setCellLayout       ( const cgPointF & cellOffset, const cgSizeF & cellSize, const cgSize & cellGridLayout );
    void                        setGridDisplacement ( cgInt32 x, cgInt32 y, const cgInt16 * displacement, const cgSize & mapPitch, const cgRect & sourceRegion, cgFloat displacementScale );
    void                        setGridGrowthMask   ( cgInt32 x, cgInt32 y, const cgByte * mapData, const cgSize & mapPitch, const cgRect & sourceRegion );
    cgFloat                     noise2D             ( cgUInt32 seed, cgFloat x, cgFloat y );
    const cgSize              & getCellGridLayout   ( ) const;
    cgFloat                     getDisplacement     ( cgFloat x, cgFloat y );
    cgFloat                     getGrowthMask       ( cgFloat x, cgFloat y );
    cgVector3                   getNormal           ( cgFloat x, cgFloat y );

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct GridData
    {
        const cgInt16 * displacement;
        cgSize          displacementPitch;
        cgRect          displacementRegion;
        cgFloat         displacementScale;
        const cgByte  * growthMask;
        cgSize          growthMaskPitch;
        cgRect          growthMaskRegion;    

        // Constructor
        GridData() : 
            displacement(CG_NULL), displacementPitch(0,0), displacementScale(1),
            displacementRegion(0,0,0,0), growthMask(CG_NULL), growthMaskPitch(0,0), 
            growthMaskRegion(0,0,0,0) {}
    };

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>The description for this clutter cell.</summary>
    cgClutterLayer    * mLayer;
    /// <summary>Offset of this cell in world space units along the X & Z axes.</summary>
    cgPointF            mCellOffset;
    /// <summary>Size of this cell in world space units along the X & Z axes.</summary>
    cgSizeF             mCellSize;
    /// <summary>Layout of the displacement grid for this clutter cell (cells can be made up of multiple displacement/blend maps).</summary>
    cgSize              mGridLayout;
    /// <summary>Grid of displacement data.</summary>
    cgArray<GridData>   mGrid;
    /// <summary>The frame on which this cell was most recently refreshed.</summary>
    cgUInt32            mLastRefreshFrame;
};

//-----------------------------------------------------------------------------
//  Name : cgFoliageClutterLayer (Class)
/// <summary>
/// Class that defines the properties used to procedurally generate foliage
/// type clutter (i.e. grasses, bushes, shrubs, etc.)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFoliageClutterLayer : public cgClutterLayer
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgFoliageClutterLayer, cgClutterLayer, "FoliageClutterLayer" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFoliageClutterLayer( cgClutterMaterial * parent );
             cgFoliageClutterLayer( cgClutterMaterial * parent, cgFoliageClutterLayer * initLayer );
    virtual ~cgFoliageClutterLayer( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgClutterLayer)
    //-------------------------------------------------------------------------
    virtual cgClutterType::Base getType             ( ) const { return cgClutterType::Foliage; }
    virtual void                setName             ( const cgString & name );
    virtual bool                serializeLayer      ( );
    virtual void                deleteLayer         ( );
    virtual bool                loadLayer           ( cgUInt32 sourceLayerId, bool cloning );
    virtual bool                begin               ( cgRenderDriver * driver, bool depthFill );
    virtual void                end                 ( cgRenderDriver * driver, bool depthFill );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                        setRenderMethod     ( cgFoliageRenderMethod::Base method );
    void                        setAnimationMethod  ( cgFoliageAnimationMethod::Base method );
    void                        setElementSize      ( const cgRangeF & width, const cgRangeF & height );
    void                        setElementSize      ( cgFloat minWidth, cgFloat maxWidth, cgFloat minHeight, cgFloat maxHeight );
    void                        setGrowthChance     ( cgFloat chance );
    void                        setClusterElements  ( cgUInt32 elements );
    void                        setClusterRadius    ( cgFloat radius );
    void                        setClusterSeparation( cgFloat separation );
    void                        setClusterEmbed     ( cgFloat embed );
    void                        setSeed             ( cgUInt32 seed );
    void                        setColorSampler     ( cgSampler * sampler );
    void                        setNormalSampler    ( cgSampler * sampler );
    void                        setFadeDistance     ( const cgRangeF & distance );
    void                        setFadeDistance     ( cgFloat minimum, cgFloat maximum );
    bool                        loadColorSampler    ( cgInputStream stream );
    bool                        loadNormalSampler   ( cgInputStream stream );
    
    void                        getElementSize      ( cgRangeF & width, cgRangeF & height ) const;
    cgFloat                     getGrowthChance     ( ) const;
    cgUInt32                    getClusterElements  ( ) const;
    cgFloat                     getClusterRadius    ( ) const;
    cgFloat                     getClusterEmbed     ( ) const;
    cgFloat                     getClusterSeparation( ) const;
    cgUInt32                    getSeed             ( ) const;
    const cgRangeF            & getFadeDistance     ( ) const;
    cgSampler                 * getColorSampler     ( ) const;
    cgSampler                 * getNormalSampler    ( ) const;
    cgFoliageRenderMethod::Base      getRenderMethod     ( ) const;
    cgFoliageAnimationMethod::Base   getAnimationMethod  ( ) const;

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    enum SerializeDirtyFlags
    {
        SamplersDirty           =   0x8,
        RenderPropertiesDirty   =   0x10,
        ElementPropertiesDirty  =   0x20,
        GrowthPropertiesDirty   =   0x40,
        MaterialPropertiesDirty =   0x80,
        LODPropertiesDirty      =   0x100
    };

    // Constant buffers
    struct _cbFoliageData
    {
        cgVector4   fadeDistance;   // Begin, End, -Begin, 1.0f / (End-Begin)
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                prepareQueries      ( );
    bool                buildResources      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgFoliageRenderMethod::Base         mRenderMethod;
    cgFoliageAnimationMethod::Base      mAnimationMethod;
    cgRangeF                            mWidth, mHeight;
    cgFloat                             mGrowthChance;
    cgUInt32                            mClusterElements;
    cgFloat                             mClusterSeparation;
    cgFloat                             mClusterEmbed;
    cgFloat                             mClusterRadius;
    cgRangeF                            mFadeDistances;
    cgUInt32                            mSeed;
    /// <summary>The color map to apply to this layer (i.e. grass, shrub, etc.)</summary>
    cgSampler                         * mColorSampler;
    /// <summary>The normal map to associated with this layer.</summary>
    cgSampler                         * mNormalSampler;
    /// <summary>The screen door noise texture.</summary>
    cgSampler                         * mScreenDoorSampler;

    /// <summary>The internal surface shader file used to handle rendering of foliage.</summary>
    cgSurfaceShaderHandle               mShader;
    /// <summary>Rasterizer states necessary for rendering this foliage layer.</summary>
    cgRasterizerStateHandle             mRasterizerState;
    /// <summary>Depth stencil states necessary for populating depth buffer for this foliage layer.</summary>
    cgDepthStencilStateHandle           mDepthFillDepthState;
    /// <summary>Depth stencil states necessary for populating geometry buffers for this foliage layer.</summary>
    cgDepthStencilStateHandle           mGeometryFillDepthState;
    /// <summary>Constant buffer necessary to supply constant data to the shader during rendering of this foliage layer.</summary>
    cgConstantBufferHandle              mFoliageConstants;
    /// <summary>Cached reference to the diffuse sampler register.</summary>
    cgInt32                             mColorSamplerRegister;
    /// <summary>Cached reference to the normal sampler register.</summary>
    cgInt32                             mNormalSamplerRegister;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertLayer;
    static cgWorldQuery mUpdateName;
    static cgWorldQuery mUpdateSamplers;
    static cgWorldQuery mUpdateRenderProperties;
    static cgWorldQuery mUpdateElementProperties;
    static cgWorldQuery mUpdateGrowthProperties;
    static cgWorldQuery mUpdateMaterialProperties;
    static cgWorldQuery mUpdateLODProperties;
    static cgWorldQuery mLoadLayer;
    static cgWorldQuery mDeleteLayer;
};

//-----------------------------------------------------------------------------
//  Name : cgFoliageClutterCell (Class)
/// <summary>
/// Class that manages procedural generation of foliage type clutter 
/// (i.e. grasses, bushes, shrubs, etc.)
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFoliageClutterCell : public cgClutterCell
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgFoliageClutterCell, cgClutterCell, "FoliageClutterCell" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgFoliageClutterCell( cgClutterLayer * layer );
    virtual ~cgFoliageClutterCell( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void        dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgClutterCell)
    //-------------------------------------------------------------------------
    virtual bool        shouldRender        ( cgCameraNode * camera ) const;
    virtual void        render              ( cgRenderDriver * driver, bool depthFill );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                refresh             ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    void                updateClusterOffsets( );
    bool                updateVertices      ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Houses the actual vertex data used to render the foliage for this cell.</summary>
    cgVertexBufferHandle    mVertices;
    /// <summary>Houses the index data used to render the foliage for this cell.</summary>
    cgIndexBufferHandle     mIndices;
    /// <summary>Generated positions for each foliage cluster.</summary>
    cgArray<cgVector3>      mClusterOffsets;
    /// <summary>Format of the vertices used to represent foliage clusters.</summary>
    cgVertexFormat        * mVertexFormat;
};

//-----------------------------------------------------------------------------
//  Name : cgClutterMaterial (Class)
/// <summary>
/// Describes properties for procedurally defining scene clutter (foliage, 
/// rocks, trees, etc.) that can be applied to landscapes and other scene
/// elements.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgClutterMaterial : public cgMaterial
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgClutterMaterial, cgMaterial, "ClutterMaterial" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgClutterMaterial( cgUInt32 referenceId, cgWorld * world );
             cgClutterMaterial( cgUInt32 referenceId, cgWorld * world, cgClutterMaterial * init );
             cgClutterMaterial( cgUInt32 referenceId, cgWorld * world, cgUInt32 sourceRefId );
    virtual ~cgClutterMaterial( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgResource)
    //-------------------------------------------------------------------------
    virtual bool                unloadResource              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_ClutterMaterial; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldResourceComponent)
    //-------------------------------------------------------------------------
    virtual void                onComponentDeleted          ( );
    virtual cgString            getDatabaseTable            ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgMaterial)
    //-------------------------------------------------------------------------
    virtual cgInt               compare                     ( const cgMaterial & material ) const;
    virtual bool                loadMaterial                ( cgUInt32 sourceRefId, cgResourceManager * manager = CG_NULL );
    virtual bool                apply                       ( cgRenderDriver * driver );
    virtual void                reset                       ( cgRenderDriver * driver );
    virtual bool                setSurfaceShader            ( const cgSurfaceShaderHandle & shader );
    virtual bool                buildPreviewImage           ( cgScene * scene, cgRenderView * view );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    size_t                      createLayer                 ( cgClutterType::Base type );
    cgClutterLayer            * getLayer                    ( size_t index ) const;
    size_t                      getLayerCount               ( ) const;
    void                        removeLayer                 ( size_t index );
    void                        removeLayer                 ( cgClutterLayer * layer );
    void                        layerUpdated                ( );

protected:
    //-------------------------------------------------------------------------
    // Protected Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    enum SerializeDirtyFlags
    {
        LayersDirty     = 0x8
    };

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgMaterial)
    //-------------------------------------------------------------------------
    virtual bool                serializeMaterial           ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    /// <summary>Collection of clutter layers assigned to this material.</summary>
    cgArray<cgClutterLayer*>    mLayers;

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery mInsertMaterial;
    static cgWorldQuery mUpdateName;
    static cgWorldQuery mUpdatePreview;
    static cgWorldQuery mLoadMaterial;
    static cgWorldQuery mLoadFoliageLayers;
};

#endif // !_CGE_CGCLUTTERMATERIAL_H_