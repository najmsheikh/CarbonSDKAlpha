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
// Name : cgLightObject.h                                                    //
//                                                                           //
// Desc : Our light class implemented as a scene object that can be          //
//        managed and controlled in the same way as any other object.        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGLIGHTOBJECT_H_ )
#define _CGE_CGLIGHTOBJECT_H_

//-----------------------------------------------------------------------------
// cgLightObject Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <World/Lighting/cgLightingTypes.h>
#include <World/cgWorldTypes.h>
#include <World/cgWorldObject.h>
#include <World/cgObjectNode.h>
#include <Scripting/cgScriptInterop.h>
#include <Math/cgMathTypes.h>
#include <Math/cgBezierSpline.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgTexturePool;
class cgShadowGenerator;
class cgVisibilitySet;
class cgFrustum;
class cgSampler;
class cgBillboardBuffer;
struct cgBaseLightingTerms;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {9A98C26E-9FE9-4b55-9C8F-E72F4D42326F}
const cgUID RTID_LightObject = {0x9a98c26e, 0x9fe9, 0x4b55, {0x9c, 0x8f, 0xe7, 0x2f, 0x4d, 0x42, 0x32, 0x6f}};
// {56D54180-4F3B-408B-B46D-A8E9AAD53B91}
const cgUID RTID_LightNode = {0x56D54180, 0x4F3B, 0x408B, {0xB4, 0x6D, 0xA8, 0xE9, 0xAA, 0xD5, 0x3B, 0x91}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cgLightObject (Base Class)
/// <summary>
/// An individual light source object.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLightObject : public cgWorldObject
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgLightObject, cgWorldObject, "LightObject" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    // ToDo: Move shader identifier selection into the actual light object types?
    /// Shader light types 
    enum LightType
    {
        Light_None        = 0,
        Light_Point       = 1,
        Light_Spot        = 2,
        Light_Directional = 3,
        Light_Projector   = 4,
		Light_Hemisphere  = 5,
		Light_IBL         = 6,
    };

    /// Allows us to specify which texture channel to update with lighting/shadowing information
    enum OutputChannelMask
    {
        ChannelMask_Red     = 0,
        ChannelMask_Green   = 1,
        ChannelMask_Blue    = 2,
        ChannelMask_Alpha   = 3,
        ChannelMask_RGB     = 4,
        ChannelMask_RGBA    = 5,
    
    }; // End Enum OutputChannelMask

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLightObject( cgUInt32 referenceId, cgWorld * world );
             cgLightObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod );
    virtual ~cgLightObject( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        isDiffuseSource             ( ) const;
    bool                        isSpecularSource            ( ) const;
    bool                        isShadowSource              ( ) const;

    cgSceneProcessStage::Base   getLightingStage            ( ) const;
	cgSceneProcessStage::Base   getShadowStage              ( ) const;
    cgFloat                     getSpecularMinDistance      ( ) const;
    cgFloat                     getSpecularMaxDistance      ( ) const;
    cgFloat                     getShadowMinDistance        ( ) const;
    cgFloat                     getShadowMaxDistance        ( ) const;
    cgFloat                     getShadowLODMinDistance     ( ) const;
    cgFloat                     getShadowLODMaxDistance     ( ) const;
    void                        setLightingStage            ( cgSceneProcessStage::Base stage );
    void                        setShadowStage              ( cgSceneProcessStage::Base stage );
    void                        setSpecularMinDistance      ( cgFloat distance );
    void                        setSpecularMaxDistance      ( cgFloat distance );
    void                        setShadowMinDistance        ( cgFloat distance );
    void                        setShadowMaxDistance        ( cgFloat distance );
    void                        setShadowLODMinDistance     ( cgFloat distance );
    void                        setShadowLODMaxDistance     ( cgFloat distance );

    cgFloat                     getDiffuseHDRScale          ( ) const;
    cgFloat                     getSpecularHDRScale         ( ) const;
    cgFloat                     getAmbientHDRScale          ( ) const;
    cgFloat                     getAmbientFarHDRScale       ( ) const;
    cgFloat                     getRimHDRScale              ( ) const;
    const cgColorValue        & getDiffuseColor             ( ) const;
    const cgColorValue        & getSpecularColor            ( ) const;
    const cgColorValue        & getAmbientColor             ( ) const;
    const cgColorValue        & getAmbientFarColor          ( ) const;
    const cgColorValue        & getRimColor                 ( ) const;
    
    cgSampler                 * getDistanceAttenSampler     ( ) const;
    const cgBezierSpline2     & getDistanceAttenCurve       ( ) const;
    cgSampler                 * getAttenMaskSampler         ( ) const;
    void                        setDistanceAttenCurve       ( const cgBezierSpline2 & spline );
    void                        setAttenMaskSampler         ( cgSampler * sampler );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual LightType           getLightType                ( ) const = 0;
    virtual bool                beginLighting               ( );

    virtual void                setDiffuseHDRScale          ( cgFloat scale );
    virtual void                setSpecularHDRScale         ( cgFloat scale );
    virtual void                setAmbientHDRScale          ( cgFloat scale );
    virtual void                setAmbientFarHDRScale       ( cgFloat scale );
    virtual void                setRimHDRScale              ( cgFloat scale );
    virtual void                setDiffuseColor             ( const cgColorValue & color );
    virtual void                setSpecularColor            ( const cgColorValue & color );
    virtual void                setAmbientColor             ( const cgColorValue & color );
    virtual void                setAmbientFarColor          ( const cgColorValue & color );
    virtual void                setRimColor                 ( const cgColorValue & color );


    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldObject)
    //-------------------------------------------------------------------------
    virtual bool                isRenderable                ( ) const;
    virtual void                sandboxRender               ( cgCameraNode * camera, cgVisibilitySet * visibilityData, bool wireframe, const cgPlane & gridPlane, cgObjectNode * issuer );
    virtual void                applyObjectRescale          ( cgFloat scale );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgWorldComponent)
    //-------------------------------------------------------------------------
    virtual bool                onComponentCreated          ( cgComponentCreatedEventArgs * e );
    virtual bool                onComponentLoading          ( cgComponentLoadingEventArgs * e );
    virtual bool                createTypeTables            ( const cgUID & typeIdentifier );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_LightObject; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        prepareQueries              ( );
    bool                        insertComponentData         ( );
    
	//-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgSurfaceShaderHandle       mShader;                    //< Internal surface shader file used to correctly handle light source rendering features.
    cgColorValue                mDiffuseColor;              //< Diffuse color to be emitted by this light source type.
    cgColorValue                mAmbientColor;              //< Ambient color to be emitted by this light source type.
    cgColorValue                mAmbientFarColor;           //< Far ambient color to be emitted by this light source type (color for the far side of the lit object).
    cgColorValue                mSpecularColor;             //< Specular color to be emitted by this light source type.   
    cgColorValue                mRimColor;                  //< rim lighting color to be used by this light source type.   
    cgSceneProcessStage::Base   mLightingStage;             //< The stage at which this light source computes lighting (precomputed, runtime etc.)
    cgSceneProcessStage::Base   mShadowStage;               //< The stage at which this light source casts shadows (precomputed, runtime etc.)
    bool                        mIsDiffuseSource;           //< Is this light a source of diffuse light?
	bool                        mIsSpecularSource;          //< Is this light a source of specular light?
    cgBezierSpline2             mDistanceAttenCurve;        //< Curve that describes the attenuation to apply over distance.

    // HDR settings.
    cgFloat                     mDiffuseHDRScale;           //< Defines the intensity of the diffuse lighting component when HDR is in use.
    cgFloat                     mAmbientHDRScale;           //< Defines the intensity of the ambient lighting component when HDR is in use.
    cgFloat                     mAmbientFarHDRScale;        //< Defines the intensity of the far ambient lighting component when HDR is in use.
    cgFloat                     mSpecularHDRScale;          //< Defines the intensity of the specular lighting component when HDR is in use.;
    cgFloat                     mRimHDRScale;               //< Defines the intensity of the rim lighting component when HDR is in use.;

    // Lighting LOD settings.
    cgFloat                     mSpecularMinDistance;       //< Distance beyond which specular starts to fall off.
    cgFloat                     mSpecularMaxDistance;       //< Distance beyond which specular is disabled.

    // Standard shadow LOD settings.
    cgFloat                     mShadowMinDistance;         //< Distance beyond which shadows start to attenuate.
    cgFloat                     mShadowMaxDistance;         //< Distance beyond which shadows are disabled.
    cgFloat                     mShadowLODMinDistance;      //< Distance beyond which maximum shadow resolution starts to decrease.
    cgFloat                     mShadowLODMaxDistance;      //< Distance at which minimum shadow resolution is reached.

    // Rendering
    cgSampler                 * mAttenDistSampler;          //< Sampler used to supply distance attenuation as required.
    cgSampler                 * mAttenMaskSampler;          //< Sampler used to supply an attenuation mask that will be projected onto the environment.

    //-------------------------------------------------------------------------
    // Protected Static Variables
    //-------------------------------------------------------------------------
    // Cached database queries.
    static cgWorldQuery     mInsertBaseLight;
    static cgWorldQuery     mInsertShadowFrustum;
    static cgWorldQuery     mInsertShadowFrustumLOD;
    static cgWorldQuery     mDeleteShadowFrustumLODs;
    static cgWorldQuery     mUpdateShadowFrustum;
    static cgWorldQuery     mUpdateDiffuse;
    static cgWorldQuery     mUpdateSpecular;
    static cgWorldQuery     mUpdateAmbient;
    static cgWorldQuery     mUpdateAmbientFar;
    static cgWorldQuery     mUpdateRim;
    static cgWorldQuery     mUpdateHDRScalars;
    static cgWorldQuery     mUpdateProcessStages;
    static cgWorldQuery     mUpdateSpecularAttenuation;
    static cgWorldQuery     mUpdateShadowAttenuation;
    static cgWorldQuery     mUpdateShadowLODRanges;
    static cgWorldQuery     mLoadBaseLight;
    static cgWorldQuery     mLoadShadowFrustum;
    static cgWorldQuery     mLoadShadowFrustumLODs;
};

//-----------------------------------------------------------------------------
// Name : cgLightNode (Base Class)
/// <summary>
/// An individual light source object node.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgLightNode : public cgObjectNode
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgLightNode, cgObjectNode, "LightNode" )

public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures & Enumerations
    //-------------------------------------------------------------------------
    /// Informs the caller of the type of lighting operation it is expected to perform
    /// after a call to "BeginXXLightingPass()" returns.
    enum LightingOp
    {
        Lighting_Abort          = 0,
        Lighting_None           = 1,
        Lighting_ProcessLight   = 2,
        Lighting_FillShadowMap  = 3,

    }; // End enum LightingOp

    /// Structure used to build a list of operations to perform during lighting passes.
    struct LightingOpPass
    {
        LightingOp  op;         //< Which operation are we performing (default shadow fill, lighting process)?
        cgInt32     frustum;    //< Which frustum are we operating on?
        cgInt32     subPass;    //< Which sub-pass are we processing?

        /// Constructor
        LightingOpPass() : op(Lighting_None), frustum(0), subPass(0) {}
        LightingOpPass( LightingOp _op, cgInt32 _frustum, cgInt32 _subPass ) :
            op( _op ),
            frustum( _frustum ),
            subPass( _subPass ) {}
    
    }; // End Struct LightingOpPass

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgLightNode( cgUInt32 referenceId, cgScene * scene );
             cgLightNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform );
    virtual ~cgLightNode( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                            isDiffuseSource             ( ) const;
    bool                            isSpecularSource            ( ) const;
    bool                            isShadowSource              ( ) const;
    bool							isIndirectSource            ( ) const;
    const cgMatrix                & getViewMatrix               ( ) const;
    const cgMatrix                & getProjectionMatrix         ( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    boundsInVolume              ( const cgBoundingBox & bounds ) = 0;
    virtual bool                    pointInVolume               ( const cgVector3 & point, cgFloat errorRadius = 0.0f ) = 0;
    virtual bool                    frustumInVolume             ( const cgFrustum & frustum ) = 0;
    virtual bool                    testObjectShadowVolume      ( cgObjectNode * object, const cgFrustum & viewFrustum ) = 0;

    virtual void                    renderShape                 ( cgCameraNode * camera, cgUInt32 subsetId = 0, cgMesh * meshOverride = CG_NULL ); 

    virtual void                    computeShadowSets           ( cgCameraNode * camera );
	// ToDo: 6767 - Remove
    virtual void                    computeVisibility           ( );
    virtual void                    setRenderOptimizations      ( bool stencilMask = true, bool userClipPlanes = true, bool scissorRect = true );

    // Direct lighting
    virtual bool                    reassignShadowMaps          ( cgTexturePool * pool );
    virtual cgInt32                 beginShadowFill             ( cgTexturePool * pool );
    virtual bool                    beginShadowFillPass         ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endShadowFillPass           ( );
    virtual bool                    endShadowFill               ( );

    virtual cgInt32                 beginLighting               ( cgTexturePool * pool, bool applyShadows, bool deferred );
    virtual LightingOp              beginLightingPass           ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endLightingPass             ( );
    virtual bool                    endLighting                 ( );

	// Indirect lighting
	virtual void					updateIndirectSettings       ( ){};
    virtual bool                    reassignIndirectMaps         ( cgTexturePool * pool );
    virtual cgInt32                 beginIndirectFill            ( cgTexturePool * pool );
    virtual bool                    beginIndirectFillPass        ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectFillPass          ( );
    virtual bool                    endIndirectFill              ( );

    virtual cgInt32                 beginIndirectLighting        ( cgTexturePool * pool );
    virtual LightingOp              beginIndirectLightingPass    ( cgInt32 pass, cgVisibilitySet *& renderSetOut );
    virtual bool                    endIndirectLightingPass      ( );
    virtual bool                    endIndirectLighting          ( );

    // ToDo: 6767 - Move into module and adjust to use actual ENUM?
	virtual	void                    setIndirectLightingMethod    ( cgUInt32 method ){ mIndirectMethod = method; }

    // ToDo: 6767 - Should probably not be virtual.
	virtual cgUInt32                getIndirectLightingMethod    ( ) const { return mIndirectMethod; }

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgObjectNode)
    //-------------------------------------------------------------------------
    virtual bool                    registerVisibility          ( cgVisibilitySet * visibilityData, cgUInt32 flags );
    virtual void                    computeLevelOfDetail        ( cgCameraNode * camera );
    virtual bool                    canScale                    ( ) const;
    virtual bool                    setCellTransform            ( const cgTransform & transform, cgTransformSource::Base source = cgTransformSource::Standard );
    virtual bool                    onNodeCreated               ( const cgUID & objectType, cgCloneMethod::Base cloneMethod );
    virtual bool                    onNodeLoading               ( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod );
    virtual bool                    getSandboxIconInfo          ( cgCameraNode * camera, const cgSize & viewportSize, cgString & atlasFile, cgString & frameName, cgVector3 & iconOrigin );
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType            ( ) const { return RTID_LightNode; }
    virtual bool                    queryReferenceType          ( const cgUID & type ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                     ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Object Property 'Set' Routing
    inline void setDiffuseHDRScale( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setDiffuseHDRScale( value );
    }
    inline void setSpecularHDRScale( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setSpecularHDRScale( value );
    }
    inline void setAmbientHDRScale( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setAmbientHDRScale( value );
    }
    inline void setAmbientFarHDRScale( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setAmbientFarHDRScale( value );
    }
    inline void setRimHDRScale( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setRimHDRScale( value );
    }
    
    inline void setDiffuseColor( const cgColorValue & value )
    {
        ((cgLightObject*)mReferencedObject)->setDiffuseColor( value );
    }
    inline void setSpecularColor( const cgColorValue & value )
    {
        ((cgLightObject*)mReferencedObject)->setSpecularColor( value );
    }
    inline void setAmbientColor( const cgColorValue & value )
    {
        ((cgLightObject*)mReferencedObject)->setAmbientColor( value );
    }
    inline void setAmbientFarColor( const cgColorValue & value )
    {
        ((cgLightObject*)mReferencedObject)->setAmbientFarColor( value );
    }
    inline void setRimColor( const cgColorValue & value )
    {
        ((cgLightObject*)mReferencedObject)->setRimColor( value );
    }

    inline void setLightingStage( cgSceneProcessStage::Base value )
    {
        ((cgLightObject*)mReferencedObject)->setLightingStage( value );
    }
	inline void setShadowStage( cgSceneProcessStage::Base value )
    {
        ((cgLightObject*)mReferencedObject)->setShadowStage( value );
    }
    inline void setSpecularMinDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setSpecularMinDistance( value );
    }
    inline void setSpecularMaxDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setSpecularMaxDistance( value );
    }
    inline void setShadowMinDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setShadowMinDistance( value );
    }
    inline void setShadowMaxDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setShadowMaxDistance( value );
    }
    inline void setShadowLODMinDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setShadowLODMinDistance( value );
    }
    inline void setShadowLODMaxDistance( cgFloat value )
    {
        ((cgLightObject*)mReferencedObject)->setShadowLODMaxDistance( value );
    }
    inline void setDistanceAttenCurve( const cgBezierSpline2 & spline )
    {
        ((cgLightObject*)mReferencedObject)->setDistanceAttenCurve( spline );
    }
    inline void setAttenMaskSampler( cgSampler * sampler )
    {
        ((cgLightObject*)mReferencedObject)->setAttenMaskSampler( sampler );
    }
    
    // Object Property 'Get' Routing
    cgLightObject::LightType getLightType( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getLightType( );
    }
    inline cgSceneProcessStage::Base getLightingStage( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getLightingStage( );
    }
	inline cgSceneProcessStage::Base getShadowStage( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getShadowStage( );
    }
    inline cgSampler * getAttenMaskSampler( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getAttenMaskSampler();
    }
    inline const cgBezierSpline2 & getDistanceAttenCurve( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getDistanceAttenCurve();
    }
    inline cgSampler * getDistanceAttenSampler( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getDistanceAttenSampler();
    }
    
    inline const cgColorValue & getDiffuseColor( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getDiffuseColor();
    }
    inline const cgColorValue & getSpecularColor( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getSpecularColor();
    }
    inline const cgColorValue & getAmbientColor( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getAmbientColor();
    }
    inline const cgColorValue & getAmbientFarColor( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getAmbientFarColor();
    }
    inline const cgColorValue & getRimColor( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getRimColor();
    }
    
    inline cgFloat getDiffuseHDRScale( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getDiffuseHDRScale();
    }
    inline cgFloat getSpecularHDRScale( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getSpecularHDRScale();
    }
    inline cgFloat getAmbientHDRScale( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getAmbientHDRScale();
    }
    inline cgFloat getAmbientFarHDRScale( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getAmbientFarHDRScale();
    }
    inline cgFloat getRimHDRScale( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getRimHDRScale();
    }

    inline cgFloat getSpecularMinDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getSpecularMinDistance();
    }
    inline cgFloat getSpecularMaxDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getSpecularMaxDistance();
    }
    inline cgFloat getShadowMinDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getShadowMinDistance();
    }
    inline cgFloat getShadowMaxDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getShadowMaxDistance();
    }
    inline cgFloat getShadowLODMinDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getShadowLODMinDistance();
    }
    inline cgFloat getShadowLODMaxDistance( ) const
    {
        return ((cgLightObject*)mReferencedObject)->getShadowLODMaxDistance();
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    struct CGE_API StateGroup
    {
        cgDepthStencilStateHandle   depthStencil;
        cgRasterizerStateHandle     rasterizer;
        cgBlendStateHandle          blend;

        // Public methods
        void Close( )
        {
            depthStencil.close();
            rasterizer.close();
            blend.close();
        }
    };

    // Constant buffer mapped structures
    struct _cbLight
    {
        cgVector3       position;               // The world space light position
        cgVector3       direction;              // The world space light direction
        cgVector4       attenuation;            // xyz = AttenTerms,  w = 1/Range
        cgColorValue    diffuse;
        cgColorValue    specular;
        cgColorValue    ambient;
        cgColorValue    diffuse2;
        cgColorValue    specular2;
        cgColorValue    ambient2;
        cgColorValue    rim;                    // rgb = rim, a = unused
        cgVector4       clipDistance;           // x = Near clip, y = Far clip
        cgVector3       textureMipLevel;
        cgVector4       ambientOcclusionAmount; // x = Diffuse, y = Specular, z = Ambient, w = direction

    }; // End Struct : _cbLight

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(LightingOpPass, LightingOpPassArray)
    CGE_VECTOR_DECLARE(cgShadowSettingsLight, ShadowSettingsArray)

	//-------------------------------------------------------------------------
    // Protected Virtual Functions
    //-------------------------------------------------------------------------
	virtual void                enableRenderOptimizations   ( );
	virtual void                disableRenderOptimizations  ( );
	virtual void                setClipPlanes               ( ){ }
	virtual void                setScissorRectangle         ( );
    virtual void                renderDeferred              ( const cgTransform & lightShapeTransform, cgVolumeQuery::Class cameraVolumeStatus, cgShadowGenerator * shadowGenerator = CG_NULL, const cgMatrix * textureProjection = CG_NULL, cgMesh *meshOverride = CG_NULL, cgUInt32 subsetId = 0 );
    virtual	bool                beginRenderForward          ( const cgTransform & lightShapeTransform, cgShadowGenerator * shadowGenerator = CG_NULL, const cgFrustum * frustumOverride = CG_NULL, const cgMatrix * textureProjection = CG_NULL );
    virtual void                endRenderForward            ( );
    virtual bool                updateSystemConstants       ( );
    virtual bool                updateLightConstants        ( );
    virtual bool                postCreate                  ( );
    virtual bool                buildStateBlocks            ( );

    // ToDo: 6767 - TEMPORARY FUNCTION (PENDING CONFIG REINTRODUCTION)
	bool loadShadowSettings( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgRect                      mScissorRectangle;          //< Screen space scissor rectangle for pixel clipping.
    cgConstantBufferHandle      mLightConstants;            //< Constants that provide base lighting terms.
    cgSurfaceShaderHandle       mShader;                    //< Internal surface shader used to correctly handle light source rendering features.
    
    // Current lighting LOD settings.
    bool                        mComputeSpecular;           //< SHOULD we currently calculate specular based on required level of detail?
    cgFloat                     mSpecularAttenuation;       //< Controls current specular lighting contribution.
    
    // Standard shadow LOD settings.
    bool                        mComputeShadows;            //< SHOULD we currently calculate shadows based on required level of detail?
    cgFloat                     mShadowAttenuation;         //< Controls current shadow contribution.
    cgFloat                     mShadowLODScale;            //< The current level of detail scale for shadowing (always 0-1).
    // ToDo
    cgShadowSettings            mCurrentShadowSettings;     //< The current level of details settings for shadowing.
	cgShadowSettingsLOD::Array  mShadowDetailLevels;        //< The list of shadow detail levels.
	ShadowSettingsArray         mShadowSettings;            //< An array of light settings (per detail level).
    // ToDo: 6767 - Need to be stored here, or can get from scene/lighting manager?
	cgShadowSettingsSystem      mShadowSystemSettings;      //< The current system settings for shadowing. 
    cgInt32                     mShadowFlags;               //< The current shadow flags.

    // Standard indirect settings.
	bool                        mComputeIndirect;           //< Should we currently calculate indirect lighting based on RSMs?
    cgShadowSettingsLOD::Array  mIndirectDetailLevels;      //< The list of reflective shadow detail levels.
	ShadowSettingsArray         mIndirectSettings;          //< An array of light settings (per detail level).
    // ToDo: 6767 - Need to be stored here, or can get from scene/lighting manager?
	cgShadowSettingsSystem      mIndirectSystemSettings;    //< The current system settings for indirect lighting.
    cgInt32                     mIndirectFlags;             //< The current indirect lighting flags.
	cgUInt32                    mIndirectMethod;            //< What indirect lighting method are we using?
	cgUInt32                    mIndirectResolution;        //< What is the resolution of the current RSM being processed?
    
    // Rendering
    cgMeshHandle                mLightShape;                //< Rendering geometry (mesh) for this light source
    
	// ToDo: Even need the view/proj matrices any more?
    cgMatrix                    mViewMatrix;                //< View transform for this light source.
    cgMatrix                    mProjectionMatrix;          //< Projection matrix for this light source.
    cgTransform                 mLightShapeTransform;       //< Base transformation to use when rendering the light shape (will be combined with object transform).

	// Light optimization features
	bool                        mUseLightClipPlanes;        //< Should we use light clip planes?
	bool                        mUseLightScreenRect;        //< Should we use scissor rectangles for our lights?
	bool                        mSetClipPlanes;             //< Did we set user defined clip planes for this light source?
	bool                        mSetScissorRect;            //< Did we set a scissor rectangle?

    /*// Visibility
    cgVisibilitySet           * mVisibility;                //< Items that CAN be lit by this light source.
    cgCameraNode              * mLastShadowCamera;          //< The camera that was last used to compute the shadow casting visibility set.
    cgCameraNode              * mLastIllumCamera;           //< The camera that was last used to compute the illumination visibility set.
    cgVisibilitySet           * mIlluminationSet;           //< Items that SHOULD be lit by this light source.
    cgVisibilitySet           * mReceiversSet;              //< Final list of culled receivers for an individual rendered frustum.
    cgVisibilitySet           * mShadowSet;                 ///< Items that should cast shadows from this light source.
    cgInt32                     mQueryId;                   ///< Render driver cclusion query identifier*/

    // Render pass tracking
    cgInt32                     mCurrentShadowPass;         //< The current shadow fill pass we are in (-2 if not begun at all, -1 if waiting to begin next pass).
    cgInt32                     mCurrentLightingPass;       //< The current lighting pass we are in (-2 if not begun at all, -1 if waiting to begin next pass).
    cgInt32                     mLightingPassCount;         //< Total number of passes required during the lighting process.
    cgInt32                     mShadowPassCount;           //< Total number of passes required during the shadow map fill process.
    bool                        mShadowTimedUpdate;         //< Have we hit our shadow update time limit and should update in this frame?
    bool                        mLightingDeferred;          //< Are we using deferred lighting during the lighting process.
    bool                        mLightingApplyShadows;      //< Are we to apply shadows during the lighting process.
    cgTexturePool             * mTexturePool;               //< Pool specified during beginShadowFill() / beginLighting(). Warning: this is a "temporary" pointer that is guaranteed only to be valid during the fill.
    cgMatrix                    mShapeMatrixCache;          //< This is cached during a beginRenderForward() call so that endRenderForward() can reuse.
    cgShadowGenerator         * mGeneratorCache;            //< This is cached during a beginRenderForward() call so that endRenderForward() can reuse.
    LightingOpPassArray         mLightingPasses;            //< List of lighting passes that need to be performed.
    LightingOpPassArray         mShadowPasses;              //< List of shadow passes that need to be performed.

    // State blocks
    StateGroup                  mInLightStates;             //< Render states to apply for camera inside case.
    StateGroup                  mOutStencilFillStates;      //< Render states to apply for camera outside case during stencil fill pass.
    StateGroup                  mOutLightStates;            //< Render states to apply for camera outside case.
    StateGroup                  mOutStencilClearStates;     //< Render states to apply for camera outside case during stencil clear pass.
    StateGroup                  mSpanStencilFillBStates;    //< Render states to apply for camera intersecting case during back stencil fill pass.
    StateGroup                  mSpanStencilFillFStates;    //< Render states to apply for camera intersecting case during front stencil fill pass.
    StateGroup                  mSpanLightStates;           //< Render states to apply for camera intersecting case.
    StateGroup                  mSpanStencilClearStates;    //< Render states to apply for camera intersecting case during stencil clear pass.

    // Shader permutation arguments
    cgScriptArgument::Array     mStencilVSArgs;
    cgScriptArgument::Array     mLightingVSArgs;
    cgScriptArgument::Array     mLightingPSArgs;
};

#endif // !_CGE_CGLIGHTOBJECT_H_