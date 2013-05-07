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
// File : cgDX11RenderDriver.h                                               //
//                                                                           //
// Desc : Rendering class wraps the properties, initialization and           //
//        management of our rendering device. This includes the enumeration, //
//        creation and destruction of our D3D device and associated          //
//        resources (DX11 Class).                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGDX11RENDERDRIVER_H_ )
#define _CGE_CGDX11RENDERDRIVER_H_

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11RenderDriver Header Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgRenderDriver.h>
#include <Rendering/Platform/cgDX11RenderingCapabilities.h>
#include <Rendering/Platform/cgDX11Initialize.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgProfiler;
class cgDX11EnumAdapter;

// Direct3D.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11InputLayout;
struct ID3D11Query;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {3308A765-3586-4002-B422-B3BEAF6A86BF}
const cgUID RTID_DX11RenderDriver = {0x3308A765, 0x3586, 0x4002, {0xB4, 0x22, 0xB3, 0xBE, 0xAF, 0x6A, 0x86, 0xBF}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderDriver (Class)
/// <summary>
/// Class that contains our core rendering / render initialization
/// logic for the DX11 render API.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11RenderDriver : public cgRenderDriver
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgDX11RenderDriver( );
    virtual ~cgDX11RenderDriver( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    // Internal Methods
    ID3D11Device              * getD3DDevice                ( ) const;
    ID3D11DeviceContext       * getD3DDeviceContext         ( ) const;
    ID3D11Resource            * getD3DBackBuffer            ( ) const;
    cgUInt32                    registerIAInputSignature    ( cgUInt32 hash[] );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgRenderDriver)
    //-------------------------------------------------------------------------
    // Configuration and Capabilities
    virtual bool                    initialize              ( cgResourceManager * resourceManager, cgAppWindow * focusWindow, cgAppWindow * outputWindow );
    virtual bool                    initialize              ( cgResourceManager * resourceManager, const cgString & windowTitle = _T("Render Output"), cgInt32 iconResource = -1 );
    virtual cgConfigResult::Base    loadConfig              ( const cgString & fileName );
    virtual cgConfigResult::Base    loadDefaultConfig       ( bool windowed = false );
    virtual bool                    saveConfig              ( const cgString & fileName );
    virtual bool                    updateDisplayMode       ( const cgDisplayMode & mode, bool windowed, bool verticalSync );
    virtual void                    windowResized           ( cgInt32 width, cgInt32 height );
    virtual void                    releaseOwnedResources   ( );
    virtual cgSize                  getScreenSize           ( ) const;
    virtual bool                    isWindowed              ( ) const;
    virtual bool                    isVSyncEnabled          ( ) const;
    
    // Render Related
    virtual bool                    beginFrame              ( bool clearTarget, cgUInt32 targetColor );
    virtual void                    endFrame                ( cgAppWindow * overrideWindow, bool bPresent );
    virtual bool                    clear                   ( cgUInt32 flags, cgUInt32 color, cgFloat depth, cgUInt8 stencil );
    virtual bool                    clear                   ( cgUInt32 rectangleCount, cgRect rectangles[], cgUInt32 flags, cgUInt32 color, cgFloat depth, cgUInt8 stencil );
    virtual void                    drawIndexedPrimitive    ( cgPrimitiveType::Base type, cgInt32 baseVertexIndex, cgUInt32 minimumVertexIndex, cgUInt32 vertexCount, cgUInt32 startingIndex, cgUInt32 primitiveCount );
    virtual void                    drawPrimitive           ( cgPrimitiveType::Base type, cgUInt32 startingVertex, cgUInt32 primitiveCount );
    virtual void                    drawPrimitiveUP         ( cgPrimitiveType::Base type, cgUInt32 primitiveCount, const void * vertexData );
    virtual void                    drawIndexedPrimitiveUP  ( cgPrimitiveType::Base type, cgUInt32 minimumVertexIndex, cgUInt32 vertexCount, cgUInt32 primitiveCount, const void * indexData, cgBufferFormat::Base indexDataFormat, const void * vertexData );
    virtual bool                    beginTargetRender       ( cgRenderTargetHandle renderTarget, cgInt32 cubeFace, bool autoUseMultiSample, cgDepthStencilTargetHandle depthStencilTarget );
    virtual bool                    beginTargetRender       ( cgRenderTargetHandleArray renderTargets, bool autoUseMultiSample, cgDepthStencilTargetHandle depthStencilTarget );
    virtual bool                    endTargetRender         ( );
    virtual bool                    stretchRect             ( cgTextureHandle & source, const cgRect * sourceRectangle, cgTextureHandle destination, const cgRect * destinationRectangle, cgFilterMethod::Base filter );

    // Notifications
    virtual bool                    cameraUpdated           ( );

    // Occlusion Queries
    virtual cgInt32                 activateQuery           ( );
	virtual void                    deactivateQuery         ( cgInt32 queryId );
    virtual bool                    validQuery              ( cgInt32 queryId );
    virtual bool                    checkQueryResults       ( cgInt32 queryId, cgUInt32 & pixelCount, bool waitForResults );
    virtual void                    clearQueries            ( );

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
    virtual bool                    setVertexBlendData      ( const cgMatrix matrices[], const cgMatrix inverseTransposeMatrices[], cgUInt32 matrixCount, cgInt32 maximumBlendIndex = -1 );
    virtual bool                    setSamplerState         ( cgUInt32 samplerIndex, const cgSamplerStateHandle & states );
    virtual bool                    setDepthStencilState    ( const cgDepthStencilStateHandle & states, cgUInt32 stencilRef );
    virtual bool                    setRasterizerState      ( const cgRasterizerStateHandle & states );
    virtual bool                    setBlendState           ( const cgBlendStateHandle & states );
    virtual bool                    setViewport             ( const cgViewport * viewport );
    virtual bool                    setMaterialTerms        ( const cgMaterialTerms & terms );
    // ToDo: 6767 should these handles be const / copies? Usually dangerous to pass by reference directly.
    virtual bool					setVPLData              ( const cgTextureHandle & depth, const cgTextureHandle & normal );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID           & getReferenceType        ( ) const { return RTID_DX11RenderDriver; }
    virtual bool                    queryReferenceType      ( const cgUID & type ) const;
    virtual bool                    processMessage          ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                    dispose                 ( bool disposeBase );

    // Promote remaining base class method overloads.
    using cgRenderDriver::beginTargetRender;

protected:
    //-------------------------------------------------------------------------
    // Protected Structures
    //-------------------------------------------------------------------------

    // Constant buffer data structures (mapped to constants in the shader system exports file).
    struct _cbScene
    {
        cgColorValue    fogColor;
        cgVector4       fogRange;
        float           fogDensity;
        float           skyIntensity;
        float           currentTime;
        float           elapsedTime;
        
    }; // End Struct : _cbScene

    struct _cbCamera
    {
        cgMatrix        viewMatrix;
        cgMatrix        projectionMatrix;
        cgMatrix        viewProjectionMatrix;
        cgMatrix        inverseViewMatrix;
        cgMatrix        inverseProjectionMatrix;
        cgVector3       position;
        cgVector3       direction;
        float           nearClip;
        float           farClip;
        float           nearRecip;
        float           farRecip;
        float           maximumDistance;
        float           maximumDistanceRecip;
        float           rangeScale;
        float           rangeBias;
        float           inverseRangeScale;
        float           inverseRangeBias;
        cgVector2       screenToViewScale;
        cgVector2       screenToViewBias;
		cgVector2       jitterAA;
        cgVector4       viewportSize;
        cgVector2       viewportOffset;
        cgVector4       targetSize;
        cgVector2       screenUVAdjustBias;

    }; // End Struct : _cbCamera

    struct _cbMaterial
    {
        cgColorValue    diffuse;
        cgColorValue    ambient;
        cgColorValue    emissive;
        cgColorValue    specular;
        cgColorValue    emissiveTint;
        float           gloss;
        float           alphaTestValue;
        float           reflectionIntensity;
        float           reflectionBumpiness;
        float           reflectionMipLevel;
        float           fresnelExponent;
        float           fresnelDiffuse;
        float           fresnelSpecular;
        float           fresnelReflection;
        float           fresnelTransparent;
        float           metalnessAmount;
        float           metalnessSpecular;
        float           metalnessDiffuse;
        cgVector2       opacityMapStrength;
        float           parallaxHeightScale;
        float           parallaxHeightBias;
        float           rimExponent;
        float           rimIntensity;
        float           ILRStrength;
        
    }; // End Struct : _cbMaterial

    struct _cbObject
    {
        float           objectBlendFactor;
        
    }; // End Struct : _cbObject

    struct _cbWorld
    {
        cgMatrix        transform;
        cgMatrix        inverseTransposeTransform;

    }; // End Struct : _cbWorld

    struct _cbVertexBlending
    {
        cgVector3       transforms[cgDX11RenderingCapabilities::MaxVBTSlotsCB*4];

    }; // End Struct : _cbVertexBlending

    // Stores information that allows us to restore target data
    // when recursively rendering to different render targets.
    struct CGE_API TargetData
    {
        cgRenderTargetHandleArray   renderTargets;          // The list of render targets set to the device
        cgDepthStencilTargetHandle  depthStencil;           // The depth stencil target set to the device
        bool                        autoUseMultiSample;     // Multi-sample resolve was in use?
        cgInt32                     cubeFace;               // Which cube face was selected for the render target(s).
        cgUInt32Array               boundTextures;          // List of texture slots which were cleared during 'beginTargetRender()' call because assign targets were bound there.

        // Constructor
        TargetData()
            : autoUseMultiSample( true ), cubeFace( -1 ) {}
    };

    struct CGE_API UserPointerVertices
    {
        cgVertexBufferHandle    buffer;
        cgUInt32                vertexCapacity;
        cgVertexFormat        * vertexFormat;

        // Constructor
        UserPointerVertices()
            : vertexCapacity( 0 ), vertexFormat( CG_NULL ) {}
    };

    struct CGE_API UserPointerIndices
    {
        cgIndexBufferHandle     buffer;
        cgUInt32                indexCapacity;
        cgBufferFormat::Base    format;

        // Constructor
        UserPointerIndices()
            : indexCapacity( 0 ), format( cgBufferFormat::Unknown ) {}
    };

    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(void*, ID3D11InputLayout*, InputLayoutMap)
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, InputLayoutMap, InputSignatureLayoutMap)
    CGE_UNORDEREDMAP_DECLARE(cgInt32, ID3D11Query*, QueryMap)
    CGE_MAP_DECLARE         (cgUInt32Array, cgUInt32, InputSignatureHashMap)
    CGE_STACK_DECLARE       (TargetData, TargetDataStack)
    CGE_UNORDEREDMAP_DECLARE(void*,UserPointerVertices, UserPointerVerticesMap);
    CGE_UNORDEREDMAP_DECLARE(cgUInt32,UserPointerIndices, UserPointerIndicesMap);

    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    bool            selectInputLayout   ( );

    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgRenderDriver)
    //-------------------------------------------------------------------------
    virtual void        onDeviceLost            ( );
    virtual bool        postInit                ( );
    virtual void        restoreTexture          ( cgUInt32 textureIndex );
    virtual void        restoreSamplerState     ( cgUInt32 samplerIndex );
    virtual void        restoreDepthStencilState( );
    virtual void        restoreRasterizerState  ( );
    virtual void        restoreBlendState       ( );
    virtual void        restoreIndices          ( );
    virtual void        restoreStreamSource     ( cgUInt32 streamIndex );
    virtual void        restoreVertexShader     ( );
    virtual void        restorePixelShader      ( );
    virtual void        restoreVertexFormat     ( );
    
    //-------------------------------------------------------------------------
    // Protected Variables.
    //-------------------------------------------------------------------------
    ID3D11Device              * mD3DDevice;                 // Direct3D device object
    ID3D11DeviceContext       * mD3DDeviceContext;          // Default Direct3D device context
    IDXGISwapChain            * mD3DSwapChain;              // The swap chain into which we're rendering.
    ID3D11RenderTargetView    * mFrameBufferView;           // D3D render target view for the primary swap chain back buffer.
    TargetDataStack             mTargetStack;               // Allows to begin / end rendering to different render targets recursively.
    
    // Configuration and enumeration
    cgDX11Settings              mD3DSettings;               // The settings used to initialize D3D
    cgDX11Initialize          * mD3DInitialize;             // The D3D enumeration / initialization object

    // Profiling
    cgProfiler                * mProfiler;                  // Cached reference to the application profiler so we don't have to retrieve it on the fly.
    
    // Queries
    QueryMap                    mActiveQueries;             // Map containing all currently open queries.
    cgInt32                     mNextQueryId;               // Next identifier to assign to open query.

    // IA input signatures and layouts
    cgUInt32                    mNextIASignatureId;         // Next Id used to represent a unique shader input signature hash.
    InputSignatureHashMap       mIAInputSignatureLUT;       // Look up table to map input signature hash values to their associated Ids.
    InputSignatureLayoutMap     mInputSignatureFormats;     // Render driver specific shader input layout objects sorted by IA input signature Id.
    cgUInt32                    mCurrentLayoutSignature;    // The signature Id of the most recently applied IA layout. This allows us to determine if we need to re-apply when the shader changes (but the vertex format doesn't).
    bool                        mSelectNewInputLayout;      // A new input layout is required because either the vertex format or vertex shader input signature changed.

    // drawPrimitiveUP simulation
    UserPointerVerticesMap      mUserPointerVertices;       // Vertex buffers for user pointer primitive drawing simulation (drawPrimitiveUP and drawIndexedPrimitiveUP)
    UserPointerIndicesMap       mUserPointerIndices;        // Vertex buffers for user pointer primitive drawing simulation (drawIndexedPrimitiveUP)

    // Internal constant buffers
    cgConstantBufferHandle      mSceneConstants;
    cgConstantBufferHandle      mCameraConstants;
    cgConstantBufferHandle      mLightConstants;
    cgConstantBufferHandle      mShadowConstants;
    cgConstantBufferHandle      mMaterialConstants;
    cgConstantBufferHandle      mObjectConstants;
    cgConstantBufferHandle      mWorldConstants;
    cgConstantBufferHandle      mVertexBlendingConstants;

    // Internal vertex textures & samplers
    cgTextureHandle             mVertexBlendingTexture;
    cgSamplerStateHandle        mVertexBlendingSampler;
};

///////////////////////////////////////////////////////////////////////////////
// cgDX11RenderDriverInit Module Local Class
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11RenderDriverInit (Subclass)
/// <summary>
/// Initialization class, derived from cgDX11Initialize to supply any
/// validation of device settings that are deemed applicable by this
/// application.
/// </summary>
//-----------------------------------------------------------------------------
class cgDX11RenderDriverInit : public cgDX11Initialize
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgDX11RenderDriverInit   ( const cgRenderDriver::InitConfig & config, bool enforceConfig );
    
private:
    //-------------------------------------------------------------------------
    // Private Virtual Methods (Overrides cgDX9Initialize)
    //-------------------------------------------------------------------------
    virtual bool validateDisplayMode    ( const DXGI_MODE_DESC & mode );
    virtual bool validateDevice         ( cgDX11EnumAdapter * adapter, D3D_DRIVER_TYPE type, D3D_FEATURE_LEVEL maximumLevel, ID3D11Device * device );
    
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgRenderDriver::InitConfig  mConfig;    // Requested configuration options during initialization
    bool                        mEnforce;   // Should we strictly enforce the configuration options?
};

#endif // CGE_DX11_RENDER_SUPPORT

#endif // !_CGE_CGDX11RENDERDRIVER_H_