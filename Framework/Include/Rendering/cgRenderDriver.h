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
// File : cgRenderDriver.h                                                   //
//                                                                           //
// Desc : Rendering class wraps the properties, initialization and           //
//        management of our rendering device. This includes the enumeration, //
//        creation and destruction of our D3D device and associated          //
//        resources.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRENDERDRIVER_H_ )
#define _CGE_CGRENDERDRIVER_H_

//-----------------------------------------------------------------------------
// cgRenderDriver Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReference.h>
#include <System/cgPropertyContainer.h>
#include <Scripting/cgScriptInterop.h>
#include <Rendering/cgRenderingTypes.h>
#include <Resources/cgResourceHandles.h>
#include <Resources/cgResourceTypes.h>
#include <Math/cgMathTypes.h>
#include <stack>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgResourceManager;
class cgVertexFormat;
class cgCameraNode;
class cgLightNode;
class cgAppWindow;
class cgRenderingCapabilities;
class cgBoundingBox;
class cgScriptObject;
class cgResource;
class cgFilterExpression;
class cgSampler;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {33C117AF-6968-4F03-BE17-B4AAB8E007AD}
const cgUID RTID_RenderDriver = {0x33C117AF, 0x6968, 0x4F03, {0xBE, 0x17, 0xB4, 0xAA, 0xB8, 0xE0, 0x7, 0xAD}};
// {5E748E40-4062-4105-92AF-C6E59F363EAA}
const cgUID RTID_RenderView   = {0x5E748E40, 0x4062, 0x4105, {0x92, 0xAF, 0xC6, 0xE5, 0x9F, 0x36, 0x3E, 0xAA}};

//-----------------------------------------------------------------------------
// Global Structures
//-----------------------------------------------------------------------------
struct CGE_API cgRenderDriverConfig
{
    cgString            deviceName;         // Name of the device to which we will attach
    bool                windowed;           // If true, device should create in windowed mode
    cgInt32             width;              // Width of the frame buffer / window
    cgInt32             height;             // Height of the frame buffer / window
    cgInt32             refreshRate;        // Rate at which the screen is refreshed
    bool                useHardwareTnL;     // Enable the use of hardware transformation and lighting on the device
    bool                useVSync;           // Synchronize frame presentation to the vertical trace of the screen
    bool                useTripleBuffering; // Enable triple buffering.
    bool                primaryDepthBuffer; // Allocate a depth buffer for the primary view?
    bool                debugVShader;       // Enable the debugging of vertex shaders
    bool                debugPShader;       // Enable the debugging of pixel shaders
    bool                usePerfHUD;         // Enable support for NVIDIA's PerfHUD profiling tool if available.
    bool                useVTFBlending;     // Enable support for vertex-texture-fetch for vertex blending (required for instancing).
    cgInt32             shadingQuality;
    cgInt32             antiAliasingQuality;
    cgInt32             postProcessQuality;

    // Constructor
    cgRenderDriverConfig()
    {
        windowed            = false;
        width               = 0;
        height              = 0;
        refreshRate         = 0;
        useHardwareTnL      = false;
        useVSync            = false;
        useTripleBuffering  = false;
        primaryDepthBuffer  = false;
        debugVShader        = false;
        debugPShader        = false;
        usePerfHUD          = false;
        useVTFBlending      = false;
        shadingQuality      = 3; // HIGH!
        antiAliasingQuality = 3; // HIGH!
        postProcessQuality  = 3; // HIGH!
    
    } // End Constructor
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRenderDriver (Class)
/// <summary>
/// Class that contains our core rendering / render initialization
/// logic.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRenderDriver : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRenderDriver, cgReference, "RenderDriver" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgConstantBuffer;
    friend class cgRenderView;

public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgInt MaxSamplerSlots        = 16;
    static const cgInt MaxTextureSlots        = 16;
    static const cgInt MaxClipPlaneSlots      = 6;
    static const cgInt MaxStreamSlots         = 16;
    static const cgInt MaxConstantBufferSlots = 14;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRenderDriver( );
    virtual ~cgRenderDriver( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgRenderDriver         * getInstance             ( );
    static cgRenderDriver         * createInstance          ( );
    static void                     createSingleton         ( );
    static void                     destroySingleton        ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    // Configuration and Capabilities
    virtual bool                    initialize              ( cgResourceManager * resourceManager, cgAppWindow * focusWindow, cgAppWindow * outputWindow );
    virtual bool                    initialize              ( cgResourceManager * resourceManager, const cgString & windowTitle = _T("Render Output"), cgInt32 iconResource = -1 );
    virtual cgConfigResult::Base    loadConfig              ( const cgString & fileName ) = 0;
    virtual cgConfigResult::Base    loadDefaultConfig       ( bool windowed = false ) = 0;
    virtual bool                    saveConfig              ( const cgString & fileName ) = 0;
    virtual bool                    updateAdapter           ( cgInt32 adapterIndex, const cgDisplayMode & mode, bool windowed, bool verticalSync ) = 0;
    virtual bool                    updateDisplayMode       ( const cgDisplayMode & mode, bool windowed, bool verticalSync ) = 0;
    virtual void                    windowResized           ( cgInt32 width, cgInt32 height );
    virtual void                    releaseOwnedResources   ( );
    virtual cgSize                  getScreenSize           ( ) const = 0;
    virtual bool                    isWindowed              ( ) const = 0;
    virtual bool                    isVSyncEnabled          ( ) const = 0;
    
    // Render Related
    virtual bool                    beginFrame              ( bool clearTarget, cgUInt32 targetColor );
    virtual void                    endFrame                ( cgAppWindow * overrideWindow, bool present );
    virtual bool                    clear                   ( cgUInt32 flags, cgUInt32 color, cgFloat depth, cgUInt8 stencil ) = 0;
    virtual bool                    clear                   ( cgUInt32 rectangleCount, cgRect rectangles[], cgUInt32 flags, cgUInt32 color, cgFloat depth, cgUInt8 stencil ) = 0;
    virtual void                    drawIndexedPrimitive    ( cgPrimitiveType::Base type, cgInt32 baseVertexIndex, cgUInt32 minimumVertexIndex, cgUInt32 vertexCount, cgUInt32 startingIndex, cgUInt32 primitiveCount ) = 0;
    virtual void                    drawPrimitive           ( cgPrimitiveType::Base type, cgUInt32 startingVertex, cgUInt32 primitiveCount ) = 0;
    virtual void                    drawPrimitiveUP         ( cgPrimitiveType::Base type, cgUInt32 primitiveCount, const void * vertexData ) = 0;
    virtual void                    drawIndexedPrimitiveUP  ( cgPrimitiveType::Base type, cgUInt32 minimumVertexIndex, cgUInt32 vertexCount, cgUInt32 primitiveCount, const void * indexData, cgBufferFormat::Base indexDataFormat, const void * vertexData ) = 0;
    virtual bool                    beginTargetRender       ( cgRenderTargetHandle renderTarget, cgInt32 cubeFace, bool autoUseMultiSample, cgDepthStencilTargetHandle depthStencilTarget ) = 0;
    virtual bool                    beginTargetRender       ( cgRenderTargetHandleArray renderTargets, bool autoUseMultiSample, cgDepthStencilTargetHandle depthStencilTarget ) = 0;
    virtual bool                    endTargetRender         ( ) = 0;
    virtual bool                    stretchRect             ( cgTextureHandle & source, const cgRect * sourceRectangle, cgTextureHandle destination, const cgRect * destinationRectangle, cgFilterMethod::Base filter ) = 0;

    // Notifications
    virtual bool                    cameraUpdated           ( );
    
    // Occlusion Queries
    virtual cgInt32                 activateQuery           ( ) = 0;
	virtual void                    deactivateQuery         ( cgInt32 queryId ) = 0;
    virtual bool                    validQuery              ( cgInt32 queryId ) = 0;
    virtual bool                    checkQueryResults       ( cgInt32 queryId, cgUInt32 & pixelCount, bool waitForResults ) = 0;
    virtual void                    clearQueries            ( ) = 0;
	
    // States
    virtual bool                    setVertexFormat         ( cgVertexFormat * format );
    virtual bool                    setWorldTransform       ( const cgMatrix * matrix );
    virtual bool                    setUserClipPlanes       ( const cgPlane worldSpacePlanes[], cgUInt32 planeCount, bool negatePlanes = true );
    virtual bool                    setScissorRect          ( const cgRect * rect );
    virtual bool                    setTexture              ( cgUInt32 textureIndex, const cgTextureHandle & texture );
    virtual bool                    setIndices              ( const cgIndexBufferHandle & indices );
    virtual bool                    setStreamSource         ( cgUInt32 streamIndex, const cgVertexBufferHandle & vertices );
    virtual bool                    setVertexShader         ( const cgVertexShaderHandle & shader );
    virtual bool                    setPixelShader          ( const cgPixelShaderHandle & shader );
    virtual bool                    setVertexBlendData      ( const cgMatrix matrices[], const cgMatrix inverseTransposeMatrices[], cgUInt32 matrixCount, cgInt32 maximumBlendIndex = -1 ) = 0;
    virtual bool                    setSamplerState         ( cgUInt32 samplerIndex, const cgSamplerStateHandle & states );
    virtual bool                    setDepthStencilState    ( const cgDepthStencilStateHandle & states, cgUInt32 stencilRef );
    virtual bool                    setRasterizerState      ( const cgRasterizerStateHandle & states );
    virtual bool                    setBlendState           ( const cgBlendStateHandle & states );
    virtual bool                    setConstantBuffer       ( cgUInt32 bufferIndex, const cgConstantBufferHandle & buffer );
    virtual bool                    setViewport             ( const cgViewport * viewport );
    virtual bool                    setMaterialTerms        ( const cgMaterialTerms & terms ) = 0;
    virtual bool					setVPLData              ( const cgTextureHandle & depth, const cgTextureHandle & normal ) = 0;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgRenderDriverConfig            getConfig               ( ) const;
    void                            setScreenSizeOverride   ( const cgSize & size );
    
    // Object query functions
    cgResourceManager             * getResourceManager      ( ) const;
    cgAppWindow                   * getFocusWindow          ( ) const;
    cgAppWindow                   * getOutputWindow         ( ) const;
    bool                            useHardwareTnL          ( ) const;
    bool                            isWindowActive          ( ) const;
    bool                            isInitialized           ( ) const;
    cgHardwareType::Base            getHardwareType         ( ) const { return mHardwareType; }
    const cgMatrix                & getWorldTransform       ( ) const;
    cgScriptObject                * getShaderInterface      ( ) const;
    cgSurfaceShaderHandle           getSystemShader         ( ) const;
    cgRenderingCapabilities       * getCapabilities         ( ) const;

    // Render related functions
    void                            drawCircle              ( const cgVector2 & position, cgFloat radius, const cgColorValue & color, cgFloat thickness, cgUInt32 segments );
    void                            drawCircle              ( const cgVector2 & position, cgFloat radius, const cgColorValue & color, cgFloat thickness, cgUInt32 segments, cgFloat arcBeginDegrees, cgFloat arcEndDegrees );
    void                            drawRectangle           ( const cgRect & bounds, const cgColorValue & color, bool filled );
    void                            drawEllipse             ( const cgRect & bounds, const cgColorValue & color, bool filled );
    void                            drawLines               ( cgVector2 points[], cgUInt32 lineCount, const cgColorValue & color, bool stripBehavior = false );
    void                            drawOOBB                ( const cgBoundingBox & bounds, cgFloat growAmount, const cgMatrix & matrix, const cgColorValue & color, bool sealedEdges = false );
    void                            drawScreenQuad          ( );
    void                            drawScreenQuad          ( const cgSurfaceShaderHandle & shader );
    void                            drawScreenQuad          ( const cgSurfaceShaderHandle & shader, cgFloat depth );
    void                            drawScreenQuad          ( cgSurfaceShaderHandle shader, cgFloat depth, const cgVector2 textureCoordinates[] );
    void                            drawViewportClipQuad    ( );
    void                            drawViewportClipQuad    ( const cgSurfaceShaderHandle & shader );
    void                            drawViewportClipQuad    ( const cgSurfaceShaderHandle & shader, cgFloat depth );
    void                            drawClipQuad            ( );
    void                            drawClipQuad            ( cgFloat depth );
    void                            drawClipQuad            ( const cgSurfaceShaderHandle & shader );
    void                            drawClipQuad            ( const cgSurfaceShaderHandle & shader, cgFloat depth );
    void                            drawClipQuad            ( cgSurfaceShaderHandle shader, cgFloat depth, const cgVector2 textureCoordinates[] );
    void                            drawClipQuad            ( cgFloat depth, bool linearZ );
    void                            drawClipQuad            ( const cgSurfaceShaderHandle & shader, cgFloat depth, bool linearZ );
    void                            drawClipQuad            ( cgSurfaceShaderHandle shader, cgFloat depth, const cgVector2 textureCoordinates[], bool linearZ );
    bool                            beginTargetRender       ( const cgRenderTargetHandle & renderTarget );
	bool                            beginTargetRender       ( const cgRenderTargetHandle & renderTarget, const cgDepthStencilTargetHandle & depthStencilTarget );
    bool                            beginTargetRender       ( const cgRenderTargetHandle & renderTarget, cgInt32 cubeFace );
    bool                            beginTargetRender       ( const cgRenderTargetHandle & renderTarget, cgInt32 cubeFace, const cgDepthStencilTargetHandle & depthStencilTarget );
    bool                            beginTargetRender       ( const cgRenderTargetHandle & renderTarget, cgInt32 cubeFace, bool autoUseMultiSample );
    bool                            beginTargetRender       ( const cgRenderTargetHandleArray & renderTargets );
    bool                            beginTargetRender       ( const cgRenderTargetHandleArray & renderTargets, const cgDepthStencilTargetHandle & depthStencilTarget );
    bool                            beginTargetRender       ( const cgRenderTargetHandleArray & renderTargets, bool autoUseMultiSample );
    bool                            beginFrame              ( );
    bool                            beginFrame              ( bool clearTarget );
    void                            endFrame                ( );
    void                            endFrame                ( bool present );
    void                            endFrame                ( cgAppWindow * overrideWindow );
    bool                            stretchRect             ( const cgTextureHandle & source, const cgRect * sourceRectangle, const cgTextureHandle & destination, const cgRect * destinationRectangle );
    bool                            stretchRect             ( const cgTextureHandle & source, const cgTextureHandle & destination, cgFilterMethod::Base filter );
    bool                            stretchRect             ( const cgTextureHandle & source, const cgTextureHandle & destination );
    
    // Application rendering states
    bool                            setSystemState          ( cgSystemState::Base state, cgInt32 value );
    cgInt32                         getSystemState          ( cgSystemState::Base state );
    bool                            setMaterial             ( cgMaterialHandle material, bool bypassFilter = false );
    const cgMaterialHandle        & getMaterial             ( ) const;
    void                            setMaterialFilter       ( cgFilterExpression * filter );
    void                            pushMaterialFilter      ( cgFilterExpression * filter );
    void                            popMaterialFilter       ( );
    bool                            matchMaterialFilter     ( const cgMaterialHandle & material );
    bool                            pushWorldTransform      ( const cgMatrix * matrix );
    bool                            popWorldTransform       ( );
    bool                            setCamera               ( cgCameraNode * camera );
    bool                            pushCamera              ( cgCameraNode * camera );
    bool                            popCamera               ( );
    cgCameraNode                  * getCamera               ( );
    void                            setRenderPass           ( const cgString & pass );
    void                            pushRenderPass          ( const cgString & pass );
    void                            popRenderPass           ( );
    const cgString                & getRenderPass           ( ) const;
    void                            setOverrideMethod       ( const cgString & method );
    void                            pushOverrideMethod      ( const cgString & method );
    void                            popOverrideMethod       ( );
    const cgString                & getOverrideMethod       ( ) const;
    bool                            pushUserClipPlanes      ( const cgPlane worldSpacePlanes[], cgUInt32 planeCount, bool negatePlanes = true );
    bool                            popUserClipPlanes       ( );
    bool                            pushScissorRect         ( const cgRect * rect );
    bool                            popScissorRect          ( );
    bool                            pushTexture             ( cgUInt32 textureIndex, const cgTextureHandle & texture );
    bool                            popTexture              ( cgUInt32 textureIndex );
    const cgTextureHandle         & getTexture              ( cgUInt32 textureIndex ) const;
    bool                            pushSamplerState        ( cgUInt32 samplerIndex, const cgSamplerStateHandle & states );
    bool                            popSamplerState         ( cgUInt32 samplerIndex );
    const cgSamplerStateHandle    & getSamplerState         ( cgUInt32 samplerIndex ) const;
   
    void							backupTextures          ( cgUInt32 startIndex, cgUInt32 endIndex );
	void							backupSamplerStates     ( cgUInt32 startIndex, cgUInt32 endIndex );
	void							restoreTextures         ( cgUInt32 startIndex, cgUInt32 endIndex );
	void							restoreSamplerStates    ( cgUInt32 startIndex, cgUInt32 endIndex );

    bool                            setDepthStencilState    ( const cgDepthStencilStateHandle & states );
    bool                            pushDepthStencilState   ( const cgDepthStencilStateHandle & states );
    bool                            pushDepthStencilState   ( const cgDepthStencilStateHandle & states, cgUInt32 stencilRef );
    bool                            popDepthStencilState    ( );
    const cgDepthStencilStateHandle&getDepthStencilState    ( ) const;
    bool                            pushRasterizerState     ( const cgRasterizerStateHandle & states );
    bool                            popRasterizerState      ( );
    const cgRasterizerStateHandle & getRasterizerState      ( ) const;
    bool                            pushBlendState          ( const cgBlendStateHandle & states );
    bool                            popBlendState           ( );
    const cgBlendStateHandle      & getBlendState           ( ) const;
    bool                            pushConstantBuffer      ( cgUInt32 bufferIndex, const cgConstantBufferHandle & buffer );
    bool                            popConstantBuffer       ( cgUInt32 bufferIndex );
    bool                            setConstantBufferAuto   ( const cgConstantBufferHandle & buffer );
    const cgConstantBufferHandle  & getConstantBuffer       ( cgUInt32 bufferIndex ) const;
    bool                            pushIndices             ( const cgIndexBufferHandle & indices );
    bool                            popIndices              ( );
    const cgIndexBufferHandle     & getIndices              ( ) const;
    bool                            pushStreamSource        ( cgUInt32 streamIndex, const cgVertexBufferHandle & vertices );
    bool                            popStreamSource         ( cgUInt32 streamIndex );
    const cgVertexBufferHandle    & getStreamSource         ( cgUInt32 streamIndex ) const;
    bool                            pushVertexShader        ( const cgVertexShaderHandle & shader );
    bool                            popVertexShader         ( );
    const cgVertexShaderHandle    & getVertexShader         ( ) const;
    bool                            pushPixelShader         ( const cgPixelShaderHandle & shader );
    bool                            popPixelShader          ( );
    const cgPixelShaderHandle     & getPixelShader          ( ) const;
    bool                            pushVertexFormat        ( cgVertexFormat * format );
    bool                            popVertexFormat         ( );
    cgVertexFormat                * getVertexFormat         ( ) const;
    bool                            pushViewport            ( const cgViewport * viewport );
    bool                            popViewport             ( );
    const cgViewport              & getViewport             ( ) const;
    
    // Material technique processing
    bool                            beginMaterialRender     ( );
    bool                            beginMaterialRender     ( const cgString & technique );
    cgTechniqueResult::Base         executeMaterialPass     ( );
    void                            endMaterialRender       ( );
    bool                            commitChanges           ( );

    // ToDo: 6767 examine suitability with driver mode.
    // VPL 
	cgSize                          getVPLTextureDimensions ( ) const;
	const cgSize                  & getVPLBufferDimensions  ( ) const;
	cgVertexBufferHandle      	    getVPLVertexBuffer      ( cgUInt32 width, cgUInt32 height, bool autoCreate = true );
	cgVertexBufferHandle      	    getVPLVertexBuffer      ( const cgSize & dimensions, bool autoCreate = true );
	bool						    addVPLVertexBuffer      ( cgUInt32 width, cgUInt32 height );
	bool						    addVPLVertexBuffer      ( const cgSize & dimensions );

    // Occlusion Queries
    bool                            checkQueryResults       ( cgInt32 queryId, cgUInt32 & pixelCount );

    // Render Views
    cgRenderView                  * createRenderView        ( const cgString & name, cgScaleMode::Base scaleMode, const cgRectF & layout );
    cgRenderView                  * createRenderView        ( const cgString & name, const cgRect & layout );
    void                            destroyRenderView       ( const cgString & name );
    cgRenderView                  * getRenderView           ( const cgString & name );
    cgRenderView                  * getActiveRenderView     ( );

    // Sandbox
    cgSurfaceShaderHandle           getSandboxSurfaceShader ( ) const;
    cgConstantBufferHandle          getSandboxConstantBuffer( ) const;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_RenderDriver; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;
    virtual bool                    processMessage          ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Enumerations
    //-------------------------------------------------------------------------
	enum RenderFlags
	{
		// Camera
		OrthographicCamera   = 0x1,     
	    
		// Lighting System
		DeferredRendering    = 0x2,
		DeferredLighting     = 0x8,
		SpecularColorOutput  = 0x40,
		HDRLighting          = 0x80,
		ViewSpaceLighting    = 0x100,
		GBufferSRGB          = 0x200,

		// Depth/Normals	
		NonLinearZ           = 0x400,
		NormalizedDistance   = 0x800,
		SurfaceNormals       = 0x1000,
		PackedDepth          = 0x2000,
		DepthStencilReads    = 0x4000,
		
		// General
		ColorWrites          = 0x8000,
		CullMode             = 0x10000,
	};

	enum LightFlags
	{
		AttenuationTexture         = 0x1,
		DistanceAttenuationTexture = 0x2,
		ColorTexture3D             = 0x4,
		DiffuseIBL                 = 0x8,
		SpecularIBL                = 0x10,
		ComputeAmbient             = 0x20,
		ComputeDiffuse             = 0x40,
		ComputeSpecular            = 0x80,
		UseSSAO                    = 0x100,
		Trilighting                = 0x200,
	};

	enum MaterialFlags
	{
		// Textures
		SampleDiffuseTexture   = 0x1,
		SampleSpecularColor    = 0x2,
		SampleSpecularMask     = 0x4,
		SampleGlossTexture     = 0x8,
		SampleEmissiveTexture  = 0x10,
		SampleOpacityTexture   = 0x20,

		// Modifiers
		AlphaTest              = 0x1000,
		DecodeSRGB             = 0x2000,
		ComputeToksvig         = 0x4000,
		CorrectNormals         = 0x8000,
		Translucent            = 0x10000,
		OpacityInDiffuse       = 0x20000,
		SurfaceFresnel         = 0x40000,
		Metal                  = 0x80000,
		Transmissive           = 0x100000,
		Emissive               = 0x200000,
	};

    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------
    // State backup / restore structures.
    struct CGE_API ClipPlaneData
    {
        cgPlane   planes[MaxClipPlaneSlots];
        cgUInt32  planeCount;

        // Constructor
        ClipPlaneData() : planeCount(0) {}
    };

    struct CGE_API DepthStencilStateData
    {
        cgDepthStencilStateHandle   handle;
        cgUInt32                    stencilRef;

        // Constructor
        DepthStencilStateData() : stencilRef(0) {}
    };

    struct CGE_API VertexStreamData
    {
        cgVertexBufferHandle        handle;
        cgUInt32                    stride;

        // Constructor
        VertexStreamData() : stride(0) {}
    };

    // System exports class variables (permutation selection).
    struct ExportVars
    {
        cgInt32       * renderFlags;             // Flags to control shader permutations that depend on active rendering/system states.
		cgInt32       * materialFlags;           // Flags to control shader permutations that depend on active material states.
		cgInt32       * lightFlags;              // Flags to control shader permutations that depend on active light states.

        // Renderer
        cgInt32       * shadingQuality;
		cgInt32       * postProcessQuality;
		cgInt32       * antiAliasingQuality;
        cgInt32       * outputEncodingType;
        cgInt32       * fogModel;
        cgInt32       * colorWrites;
        cgInt32       * cullMode;
        bool          * orthographicCamera;
        bool          * hdrLighting;
        bool          * viewSpaceLighting;
		bool          * deferredRendering;
		bool          * deferredLighting;
		bool          * specularColorOutput;
		bool          * pGBufferSRGB; // ToDo: 6767 - Do something with this
		bool          * nonLinearZ;
		bool          * normalizedDistance;
		bool          * surfaceNormals;
		bool          * packedDepth;
		bool          * depthStencilReads;

	    // Geometry
        cgInt32       * maximumBlendIndex;
        cgInt32       * depthType;
        cgInt32       * surfaceNormalType;
        bool          * useVTFBlending;

	    // Lights
        cgInt32       * lightType;
        cgInt32       * shadowMethod;
        cgInt32       * primaryTaps;
        cgInt32       * secondaryTaps;
	    bool          * sampleAttenuation;
        bool          * sampleDistanceAttenuation;
        bool          * colorTexture3D; 
        bool          * diffuseIBL;
        bool          * specularIBL;
        bool          * computeAmbient;
        bool          * computeDiffuse;
        bool          * computeSpecular;
        bool          * useSSAO;
        bool          * trilighting;
        
	    // Materials
	    cgInt32       * normalSource;
	    cgInt32       * reflectionMode;
	    cgInt32       * lightTextureType;
        bool          * sampleDiffuseTexture;
        bool          * sampleSpecularColor;
        bool          * sampleSpecularMask;
        bool          * sampleGlossTexture;
        bool          * sampleEmissiveTexture;
	    bool          * sampleOpacityTexture;
        bool          * pDecodeSRGB;    // ToDo: 6767 -- Do something with the case
        bool          * computeToksvig;
        bool          * correctNormals;
        bool          * opacityInDiffuse;
	    bool          * surfaceFresnel;
        bool          * metal;
        bool          * transmissive;
        bool          * translucent;
        bool          * alphaTest;
        bool          * emissive;

		// Objects
		int           * objectRenderClass;
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_STACK_DECLARE           (cgString, StringStack)
    CGE_STACK_DECLARE           (cgSamplerStateHandle, SamplerStateStack)
    CGE_STACK_DECLARE           (DepthStencilStateData, DepthStencilStateStack)
    CGE_STACK_DECLARE           (cgRasterizerStateHandle, RasterizerStateStack)
    CGE_STACK_DECLARE           (cgBlendStateHandle, BlendStateStack)
    CGE_STACK_DECLARE           (cgVertexShaderHandle, VertexShaderStack)
    CGE_STACK_DECLARE           (cgPixelShaderHandle, PixelShaderStack)
    CGE_STACK_DECLARE           (ClipPlaneData, ClipPlaneStack)
    CGE_STACK_DECLARE           (cgMatrix, MatrixStack)
    CGE_STACK_DECLARE           (cgVertexFormat*, VertexFormatStack)
    CGE_DEQUE_DECLARE           (cgCameraNode*, CameraStack)
    CGE_STACK_DECLARE           (cgFilterExpression*, MaterialFilterStack)
    CGE_STACK_DECLARE           (cgRect, RectangleStack)
    CGE_STACK_DECLARE           (cgIndexBufferHandle, IndicesStack)
    CGE_STACK_DECLARE           (VertexStreamData, VertexStreamStack)
    CGE_STACK_DECLARE           (cgConstantBufferHandle, ConstantBufferStack)
    CGE_STACK_DECLARE           (cgTextureHandle, TextureStack)
    CGE_STACK_DECLARE           (cgRenderView*, RenderViewStack)
    CGE_STACK_DECLARE           (cgViewport, ViewportStack)
    CGE_UNORDEREDMAP_DECLARE    (cgString, cgRenderView*, NamedRenderViewMap )
    CGE_MAP_DECLARE             (cgSize, cgVertexBufferHandle, SizeVertexBufferMap)

    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        postInit                ( );
    virtual bool        initShaderSystem        ( );
    virtual void        onDeviceLost            ( );
    virtual void        onDeviceReset           ( );
    virtual void        restoreConstantBuffer   ( cgUInt32 bufferIndex );
    virtual void        restoreTexture          ( cgUInt32 textureIndex ) = 0;
    virtual void        restoreSamplerState     ( cgUInt32 samplerIndex ) = 0;
    virtual void        restoreDepthStencilState( ) = 0;
    virtual void        restoreRasterizerState  ( ) = 0;
    virtual void        restoreBlendState       ( ) = 0;
    virtual void        restoreIndices          ( ) = 0;
    virtual void        restoreStreamSource     ( cgUInt32 streamIndex ) = 0;
    virtual void        restoreVertexShader     ( ) = 0;
    virtual void        restorePixelShader      ( ) = 0;
    virtual void        restoreVertexFormat     ( ) = 0;

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                beginViewRender         ( cgRenderView * view );
    void                endViewRender           ( );

    //-------------------------------------------------------------------------
    // Protected Variables.
    //-------------------------------------------------------------------------
    cgRenderDriverConfig        mConfig;                                // Render driver configuration settings
    bool                        mConfigLoaded;                          // Has the configuration been loaded yet?
    bool                        mInitialized;                           // Has the driver been initialized?
    bool                        mFrameBegun;                            // Is the application within a beginFrame()/endFrame() pair?
    cgResourceManager         * mResourceManager;                       // The manager that is in charge of resources tied to this device / render driver
    cgHardwareType::Base        mHardwareType;                          // The type of hardware currently in use.
    cgRenderingCapabilities   * mCaps;                                  // Capabilities enumerator for this render driver.

    cgAppWindow               * mFocusWindow;                           // Provided window for device initialization
    cgAppWindow               * mOutputWindow;                          // Provided window for rendering output.
    bool                        mOwnsWindow;                            // Was the window created externally?
    bool                        mLostDevice;                            // Is the render device currently lost ?
    bool                        mActive;                                // Is the window active ?
    cgSize                      mTargetSize;                            // The exact size of the currently applied render target (even if this is the frame buffer).
    cgSize                      mScreenSizeOverride;                    // Current 'override' for screen size in case we want the frame buffer to be larger than the display.
	cgFloat                     mAdapterAspectRatio;                    // The aspect ratio of the adapter prior to enumeration (useful for widescreen displays)
    cgUInt32                    mConstantsDirty;                        // Records the current 'dirty' state of a maximum of 32 constant buffers currently assigned to this driver.
    cgUInt32                    mPrimitivesDrawn;                       // Number of triangles rendered in the current frame.
    bool                        mStateFilteringEnabled;                 // Duplicate device states and resources should be filtered out.
    bool                        mSuppressResizeEvent;                   // Prevent window resize events from being processed when this is true.

    // System integration
    cgScriptHandle              mSystemExportScript;                    // Primary render driver script through which system variables are exported to the surface shaders.
    cgScriptObject            * mSystemExports;                         // The actual script class interface through which system variables are exported to surface shaders.
    ExportVars                  mSystemExportVars;                      // System exports class variables (permutation selection).
    
    // State stacks
    StringStack                 mRenderPassStack;                       // Allows application to push / pop current render pass name.
    StringStack                 mOverrideMethodStack;                   // Allows application to push / pop current rendering method overrides.
    SamplerStateStack           mSamplerStateStack[MaxSamplerSlots];    // Allows application to push / pop current sampler state(s).
    DepthStencilStateStack      mDepthStencilStateStack;                // Allows application to push / pop current depth stencil state.
    RasterizerStateStack        mRasterizerStateStack;                  // Allows application to push / pop current rasterizer state.
    BlendStateStack             mBlendStateStack;                       // Allows application to push / pop current blend state.
    VertexShaderStack           mVertexShaderStack;                     // Allows application to push / pop hardware vertex shader.
    PixelShaderStack            mPixelShaderStack;                      // Allows application to push / pop hardware pixel shader.
    MatrixStack                 mWorldTransformStack;                   // Allows application to push / pop current world transform.
    CameraStack                 mCameraStack;                           // Allows application to push / pop current camera.
    ClipPlaneStack              mClipPlaneStack;                        // Allows application to push / pop clip planes.
    VertexFormatStack           mVertexFormatStack;                     // Allows application to push / pop current vertex format.
    MaterialFilterStack         mMaterialFilterStack;                   // Allows application to push / pop current material filter settings.
    RectangleStack              mScissorRectStack;                      // Allows application to push / pop current the scissor clipping rectangle.
    IndicesStack                mIndicesStack;                          // Allows application to push / pop currently assigned index buffer handle.
    VertexStreamStack           mVertexStreamStack[MaxStreamSlots];     // Allows application to push / pop currently assigned vertex stream handle.
    ConstantBufferStack         mConstantBufferStack[MaxConstantBufferSlots];     // Allows application to push / pop currently assigned constant buffers.
    TextureStack                mTextureStack[MaxTextureSlots];         // Allows application to push / pop currently assigned texture data.
    RenderViewStack             mRenderViewStack;                       // Allows application to push / pop currently active render views.
    ViewportStack               mViewportStack;                         // Allows application to push / pop current render viewport.

    // Default states
    cgSamplerStateHandle        mDefaultSamplerState;                   // Default sampler states that are assumed when no states are applied.
    cgDepthStencilStateHandle   mDefaultDepthStencilState;              // Default depth stencil states that are assumed when no states are applied.
    cgRasterizerStateHandle     mDefaultRasterizerState;                // Default rasterizer states that are assumed when no states are applied.
    cgBlendStateHandle          mDefaultBlendState;                     // Default blend states that are assumed when no states are applied.
    
    // Current States
    cgMaterialHandle            mCurrentMaterialHandle;                 // The material currently applied to the driver
    cgMaterial                * mCurrentMaterial;                       // A cached copy of the underlying resource
    cgSurfaceShaderHandle       mCurrentSurfaceShaderHandle;            // The surface shader currently applied to the driver via the above material.
    cgSurfaceShader           * mCurrentSurfaceShader;                  // A cached copy of the underlying resource
	
    // Transformation States
    cgMatrix                    mViewMatrix;                            // Local copy of view matrix
    cgMatrix                    mProjectionMatrix;                      // Local copy of projection matrix
    cgMatrix                    mViewProjectionMatrix;                  // Cached copy of view * proj matrix

    // Cached Drawing Formats & States
    cgBlendStateHandle          mElementBlendState;                     // Cached blend state for internal screen element rendering (lines, rectangles, OOBBs, etc.)
    cgDepthStencilStateHandle   mElementDepthState;                     // Cached depth stencil state for internal screen element rendering (lines, rectangles, OOBBs, etc.)
    cgVertexFormat            * mWorldSpaceFormat;                      // Vertex format for world space element rendering (drawOOBB, etc.)
    cgVertexFormat            * mScreenSpaceFormat;                     // Vertex format for screen space element rendering (drawLines, drawScreenQuad, etc.)
    cgVertexFormat            * mClipSpaceFormat;                       // Vertex format for clip space rendering (drawClipQuad, etc.)

    // Internal System Shaders
    cgSurfaceShaderHandle       mDriverShaderHandle;                    // Surface shader that simply defines a number of vertex and pixel shaders required by internal render driver features.
    cgSurfaceShader           * mDriverShader;                          // Cached pointer to above surface shader.

    // Render Views
    cgRenderView              * mPrimaryView;                           // Primary render view (representing the main frame buffer)
    NamedRenderViewMap          mRenderViews;                           // Dictionary of allocated render views ordered by name.

    // Internal resources
    cgRenderTargetHandle        mDeviceFrameBuffer;                     // Render target that represents (wraps) the main frame buffer surface.
    cgDepthStencilTargetHandle  mDeviceDepthStencilTarget;              // Depth stencil target that represents the main device depth stencil buffer (optional).

    // Virtual Point Light Rendering
	SizeVertexBufferMap         mVPLVertexBuffers;                      // A table of vertex buffers for vertex texture fetch (key = size)
	cgRenderTargetHandle        mVPLBuffer;                             // A buffer that contains depth and normal for VPL rendering
    cgSize                      mCurrentVPLSize;                        // Current active size of the VPL depth/normal buffer.
	cgDepthStencilStateHandle   mDisabledDepthState;                    // Disabled depth reading and writing state
	cgSampler *                 mSampler2D0;                            // 2D image sampler
	cgSampler *                 mSampler2D1;                            // 2D image sampler 

    // Sandbox Rendering
    cgSurfaceShaderHandle       mSandboxShader;                         // In sandbox mode, the sandbox surface shader is available for additional rendering tasks.
    cgConstantBufferHandle      mSandboxConstants;                      // In sandbox mode, these constants are available for use in conjunction with the above shader.

    //-------------------------------------------------------------------------
    // Private Static Variables.
    //-------------------------------------------------------------------------
    static cgRenderDriver     * mSingleton;                             // Static singleton object instance.
};

//-----------------------------------------------------------------------------
//  Name : cgRenderView (Class)
/// <summary>
/// A render view describes an area of the main frame buffer into which the
/// application should render. A little more comprehensive than a simple
/// 'viewport', this class essentially provides a rendering 'sandbox' with
/// unique rendering surfaces that fit precisely within the view.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRenderView : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgRenderView, cgReference, "RenderView" )
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRenderView( cgRenderDriver * driver, const cgString & name, bool primaryView );
    virtual ~cgRenderView( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                                initialize              ( );
    bool                                initialize              ( const cgRect & layout );
    bool                                initialize              ( cgScaleMode::Base scaleMode, const cgRectF & layout );
    bool                                isViewLost              ( ) const;
    cgInt32                             getRenderCount          ( ) const;

    // Layout
    bool                                setLayout               ( const cgRect & layout );
    bool                                setLayout               ( cgScaleMode::Base scaleMode, const cgRectF & layout );
    void                                getLayout               ( cgScaleMode::Base & scaleMode, cgRectF & layout ) const;
    cgRect                              getRectangle            ( ) const;
    cgPoint                             getPosition             ( ) const;
    cgSize                              getSize                 ( ) const;

    // Rendering
    bool                                begin                   ( );
    bool                                end                     ( );
    bool                                end                     ( bool presentView );
    bool                                present                 ( );
	
    // Render surfaces
    const cgRenderTargetHandle        & getViewBuffer           ( ) const;
    const cgDepthStencilTargetHandle  & getDepthStencilBuffer   ( ) const;
    
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, const cgString & instanceId );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, const cgString & instanceId );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality, const cgString & instanceId );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality );
    const cgRenderTargetHandle        & getRenderSurface        ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality, const cgString & instanceId );

    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, const cgString & instanceId );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, const cgString & instanceId );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality, const cgString & instanceId );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality );
    const cgDepthStencilTargetHandle  & getDepthStencilSurface  ( cgBufferFormat::Base format, cgFloat scalarWidth, cgFloat scalarHeight, cgMultiSampleType::Base multiSampleType, cgUInt32 multiSampleQuality, const cgString & instanceId );

	bool                                readableDepthStencilBuffer( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID               & getReferenceType        ( ) const { return RTID_RenderView; }
    virtual bool                        queryReferenceType      ( const cgUID & type ) const;
    virtual bool                        processMessage          ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                        dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Typdefs & Structures
    //-------------------------------------------------------------------------
    struct RenderSurface
    {
        cgString                    instanceId;
        cgFloat                     scalarWidth;
        cgFloat                     scalarHeight;
        cgBufferFormat::Base        format;
        cgRenderTargetHandle        handle;
        cgMultiSampleType::Base     multiSampleType;
        cgUInt32                    multiSampleQuality;
        cgDouble                    lastRequested;        // The time at which the resource was last requested

    }; // End Struct : RenderSurface
    CGE_MAP_DECLARE( cgUInt32, RenderSurface, RenderSurfaceMap )

    struct DepthStencilSurface
    {
        cgString                    instanceId;
        cgFloat                     scalarWidth;
        cgFloat                     scalarHeight;
        cgBufferFormat::Base        format;
        cgMultiSampleType::Base     multiSampleType;
        cgUInt32                    multiSampleQuality;
        cgDepthStencilTargetHandle  handle;
        cgDouble                    lastRequested;        // The time at which the resource was last requested

    }; // End Struct : DepthStencilSurface
    CGE_MAP_DECLARE( cgUInt32, DepthStencilSurface, DepthStencilSurfaceMap )

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                        releaseBuffers      ( );
    bool                        createBuffers       ( );
    void                        sendGarbageMessage  ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgRenderDriver            * mDriver;                // The render driver that owns this view.
    cgString                    mName;                  // Name of this render view.
    bool                        mViewLost;              // Is the data in the render view lost?
    bool                        mRendering;             // Currently within a begin/end pair for this view?
    bool                        mPrimaryView;           // Is this the primary render driver view (attached to the main frame buffer?)
    cgScaleMode::Base           mScaleMode;             // Absolute (pixel) or screen relative (0-1)
    cgRectF                     mLayout;                // View layout rectangle.
    cgRenderTargetHandle        mViewBuffer;            // Main 32bpp (A8R8G8B8) render view 'frame' buffer
    cgRenderTargetHandle        mDeviceFrameBuffer;     // Cached reference to the main device frame buffer target.
    cgDepthStencilTargetHandle  mDepthStencilBuffer;    // Depth stencil target associated with this view.
    RenderSurfaceMap            mRenderSurfaces;        // Map of all allocated rendering surfaces (a.k.a. G-Buffers) by reference Id
    DepthStencilSurfaceMap      mDepthStencilSurfaces;  // Map of all allocated depth stencil surfaces by reference Id
    cgInt32                     mRenderCount;           // Keeps track of the number of times this view has been used for rendering.
};

#endif // !_CGE_CGRENDERDRIVER_H_