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
// Name : cgResourceManager.h                                                //
//                                                                           //
// Desc : This file houses all of the classes responsible for the creation   //
//        and management of resources to be used within the application.     //
//        This includes items such as vertex/index buffers, textures, meshes //
//        scripts and surface material information.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRESOURCEMANAGER_H_ )
#define _CGE_CGRESOURCEMANAGER_H_

//-----------------------------------------------------------------------------
// cgResourceManager Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <Resources/cgResourceHandles.h>
#include <Resources/cgResource.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class  cgInputStream;
class  cgVertexFormat;
class  cgSceneLoader; // ToDo: Remove
class  cgWorld;
class  cgXMLNode; // ToDo: Remove
class  cgSampler;
class  cgResource;
class  cgBezierSpline2;
class  cgBufferFormatEnum;
class  cgVertexShader;
class  cgPixelShader;
class  cgMaterial;
class  cgTexture;
class  cgVertexBuffer;
class  cgIndexBuffer;
class  cgConstantBuffer;
class  cgMesh;
class  cgRenderTarget;
class  cgSurface;
class  cgAudioBuffer;
class  cgAnimationSet;
class  cgScript;
class  cgSurfaceShader;
class  cgShaderIdentifier;
struct cgSamplerStateDesc;
struct cgDepthStencilStateDesc;
struct cgRasterizerStateDesc;
struct cgBlendStateDesc;
struct cgConstantBufferDesc;

// ToDo: 9999 - We're getting rid of the material map for batching materials.

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {68A37B4A-A149-44FA-8A11-F14E6DEBE2FE}
const cgUID RTID_ResourceManager = {0x68A37B4A, 0xA149, 0x44FA, {0x8A, 0x11, 0xF1, 0x4E, 0x6D, 0xEB, 0xE2, 0xFE}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgResourceManager (Class)
/// <summary>
/// Class for the management of application resources.
/// Note : cgResourceManager is designed to be a singleton object, only one of
/// which will exist throughout the application's lifetime. However
/// it can be instantiated should the need arise (such as managing
/// resources for multiple render devices).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgResourceManager : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgResourceManager, cgReference, "ResourceManager" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgRenderDriver;
    friend class cgAudioDriver;
    friend class cgResource;

public:
    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    struct InitConfig                       // The selected resource configuration options
    {
        cgInt32     textureMipLevels;       // Number of levels to generate for texture mip mapping (0 = full chain, -1 = whatever is in the file)
        bool        compressTextures;       // Should attempt to compress textures if supported
    };

    struct MaterialKey
    {
        // Constructor
        MaterialKey( const cgMaterial * compareMaterial )
        {
            material = compareMaterial;
        }

        // Public Variables
        const cgMaterial * material;   // The material we're comparing against
    
    }; // End Struct MaterialKey
    friend bool CGE_API operator < ( const MaterialKey&, const MaterialKey& );

    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    CGE_MAP_DECLARE         (MaterialKey, cgMaterial*, MaterialMap)
    CGE_UNORDEREDMAP_DECLARE(cgString, cgUInt32, NameUsageMap)

    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum DefaultSamplerTypes
    {
        DefaultDiffuseSampler = 0,
        DefaultNormalSampler  = 1
    
    }; // End Enum DefaultSamplerTypes

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgResourceManager( );
    virtual ~cgResourceManager( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgResourceManager  * getInstance             ( );
    static void                 createSingleton         ( );
    static void                 destroySingleton        ( );
    static void                 setDefaultDestroyDelay  ( cgFloat delay );
    static cgFloat              getDefaultDestroyDelay  ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                        loadConfig                  ( const cgString & fileName );
    bool                        saveConfig                  ( const cgString & fileName );
    InitConfig                  getConfig                   ( ) const;
    bool                        initialize                  ( );
    cgRenderDriver            * getRenderDriver             ( );
    cgAudioDriver             * getAudioDriver              ( );
    const cgBufferFormatEnum  & getBufferFormats            ( ) const;
    void                        emptyGarbage                ( );
    void                        releaseOwnedResources       ( );
    void                        enableDestruction           ( bool enable );
    bool                        isDestructionEnabled        ( ) const;

    // Textures
    bool                        addTexture                  ( cgTextureHandle * resourceOut, cgTexture * texture, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getTexture                  ( cgTextureHandle * resourceOut, const cgString & resourceName );
    bool                        loadTexture                 ( cgTextureHandle * resourceOut, const cgInputStream & stream, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createTexture               ( cgTextureHandle * resourceOut, const cgImageInfo & description, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createTexture               ( cgTextureHandle * resourceOut, cgBufferType::Base type, cgUInt32 width, cgUInt32 height, cgUInt32 depth, cgUInt32 mipLevels, cgBufferFormat::Base format, cgMemoryPool::Base pool, bool autoGenMips = false, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createTexture               ( cgTextureHandle * resourceOut, cgBezierSpline2 & spline, cgUInt32 width, cgUInt32 mipLevels, bool rightToLeft = false, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        cloneTexture                ( cgTextureHandle * resourceOut, cgTextureHandle sourceTexture, cgBufferFormat::Base newFormat = cgBufferFormat::Unknown, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        cloneTexture                ( cgTextureHandle * resourceOut, cgTextureHandle sourceTexture, const cgRect & sourceRectangle, cgBufferFormat::Base newFormat = cgBufferFormat::Unknown, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        cloneTexture                ( cgTextureHandle * resourceOut, cgTextureHandle sourceTexture, cgRect sourceRectangle, cgSize TextureSize, bool stretch, cgBufferFormat::Base newFormat = cgBufferFormat::Unknown, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    
    // Vertex / Index buffers
    bool                        createVertexBuffer          ( cgVertexBufferHandle * resourceOut, cgUInt32 length, cgUInt32 usage, cgVertexFormat * format, cgMemoryPool::Base pool, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createIndexBuffer           ( cgIndexBufferHandle * resourceOut, cgUInt32 length, cgUInt32 usage, cgBufferFormat::Base format, cgMemoryPool::Base pool, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    void                      * lockVertexBuffer            ( cgVertexBufferHandle & resource, cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags );
    void                        unlockVertexBuffer          ( cgVertexBufferHandle & resource );
    void                      * lockIndexBuffer             ( cgIndexBufferHandle & resource, cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags );
    void                        unlockIndexBuffer           ( cgIndexBufferHandle & resource );

    // Constant buffers
    bool                        createConstantBuffer        ( cgConstantBufferHandle * resourceOut, const cgSurfaceShaderHandle & shader, const cgString & bufferName, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createConstantBuffer        ( cgConstantBufferHandle * resourceOut, const cgConstantBufferDesc & description, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createConstantBuffer        ( cgConstantBufferHandle * resourceOut, const cgConstantBufferDesc & description, const cgConstantTypeDesc::Array & types, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    void                      * lockConstantBuffer          ( cgConstantBufferHandle & resource, cgUInt32 offsetToLock, cgUInt32 sizeToLock, cgUInt32 flags );
    void                        unlockConstantBuffer        ( cgConstantBufferHandle & resource );
    
    // Meshes
    bool                        addMesh                     ( cgMeshHandle * resourceOut, cgMesh * mesh, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getMesh                     ( cgMeshHandle * resourceOut, const cgString & resourceName );
    bool                        getMesh                     ( cgMeshHandle * resourceOut, cgUInt32 referenceId );
    bool                        loadMesh                    ( cgMeshHandle * resourceOut, cgWorld * world, cgUInt32 sourceRefId, bool internalResource, bool finalizeMesh = true, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    
    // Materials
    bool                        addMaterial                 ( cgMaterialHandle * resourceOut, cgMaterial * material, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createMaterial              ( cgMaterialHandle * resourceOut, bool internalResource, cgWorld * world, cgMaterialType::Base type, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createMaterial              ( cgMaterialHandle * resourceOut, cgMaterialType::Base type, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        cloneMaterial               ( cgMaterialHandle * resourceOut, cgWorld * world, bool internalResource, cgMaterial * init, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadMaterial                ( cgMaterialHandle * resourceOut, cgWorld * world, cgMaterialType::Base type, cgUInt32 sourceRefId, bool internalResource, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getMaterialFromId           ( cgMaterialHandle * resourceOut, cgUInt32 referenceId );
    bool                        buildMaterialBatchTable     ( MaterialMap & table );
    cgString                    makeUniqueMaterialName      ( const cgString & name );
    cgString                    makeUniqueMaterialName      ( const cgString & name, cgUInt32 suffixNumber );
    NameUsageMap              & getMaterialNameUsage        ( );
    cgMaterialHandle            getDefaultMaterial          ( ) const;

    // Render Targets
    bool                        createRenderTarget          ( cgRenderTargetHandle * resourceOut, const cgImageInfo & description, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getRenderTarget             ( cgRenderTargetHandle * resourceOut, const cgString & resourceName );
    
    // Depth Stencil Targets
    bool                        createDepthStencilTarget    ( cgDepthStencilTargetHandle * resourceOut, const cgImageInfo & description, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getDepthStencilTarget       ( cgDepthStencilTargetHandle * resourceOut, const cgString & resourceName );

    // Animation Sets
    bool                        addAnimationSet             ( cgAnimationSetHandle * resourceOut, cgAnimationSet * animationSet, cgUInt32 flags = 0, const cgString & resourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadAnimationSet            ( cgAnimationSetHandle * resourceOut, cgWorld * world, cgUInt32 sourceRefId, bool internalResource, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getAnimationSet             ( cgAnimationSetHandle * resourceOut, const cgString & resourceName );
    bool                        getAnimationSet             ( cgAnimationSetHandle * resourceOut, cgUInt32 referenceId );

    // Sound Effects
    bool                        loadAudioBuffer             ( cgAudioBufferHandle * resourceOut, const cgInputStream & stream, cgUInt32 soundFlags = 0, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getAudioBuffer              ( cgAudioBufferHandle * resourceOut, const cgString & resourceName, cgUInt32 soundFlags = 0 );

    // Scripts
    bool                        loadScript                  ( cgScriptHandle * resourceOut, const cgInputStream & stream, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadScript                  ( cgScriptHandle * resourceOut, const cgInputStream & stream, const cgString & thisType, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadScript                  ( cgScriptHandle * resourceOut, const cgInputStream & stream, const cgString & thisType, const cgString & instanceId, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        reloadScripts               ( cgFloat delay = 0.0f );

    // Surface Shaders
    bool                        createSurfaceShader         ( cgSurfaceShaderHandle * resourceOut, const cgInputStream & stream, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createDefaultSurfaceShader  ( cgSurfaceShaderHandle * resourceOut, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadSurfaceShaderScript     ( cgScriptHandle * resourceOut, const cgInputStream & stream, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        reloadShaders               ( cgFloat delay = 0.0f );

    // Hardware Shaders
    bool                        createVertexShader          ( cgVertexShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, const cgString & shaderCode, const cgString & entryPoint, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadVertexShader            ( cgVertexShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadCompiledVertexShader    ( cgVertexShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, const cgInputStream & compiledStream,  cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createPixelShader           ( cgPixelShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, const cgString & shaderCode, const cgString & entryPoint, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadPixelShader             ( cgPixelShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        loadCompiledPixelShader     ( cgPixelShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier, const cgInputStream & compiledStream,  cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        getVertexShader             ( cgVertexShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier );
    bool                        getPixelShader              ( cgPixelShaderHandle * resourceOut, const cgShaderIdentifier * cacheIdentifier );

    // State Block Objects
    bool                        createSamplerState          ( cgSamplerStateHandle * resourceOut, const cgSamplerStateDesc & values, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createDepthStencilState     ( cgDepthStencilStateHandle * resourceOut, const cgDepthStencilStateDesc & values, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createRasterizerState       ( cgRasterizerStateHandle * resourceOut, const cgRasterizerStateDesc & values, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    bool                        createBlendState            ( cgBlendStateHandle * resourceOut, const cgBlendStateDesc & values, cgUInt32 flags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    
    // Samplers (Not a resource type).
    cgSampler                 * createSampler               ( const cgString & name );
    cgSampler                 * createSampler               ( const cgString & name, const cgSurfaceShaderHandle & customShader );
    cgSampler                 * createSampler               ( cgWorld * world, const cgString & name );
    cgSampler                 * createSampler               ( cgWorld * world, const cgString & name, const cgSurfaceShaderHandle & customShader );
    cgSampler                 * cloneSampler                ( cgSampler * init );
    cgSampler                 * cloneSampler                ( cgWorld * world, bool internal, cgSampler * init );
    cgSampler                 * loadSampler                 ( cgWorld * world, cgUInt32 sourceRefId, bool internal, cgUInt32 resourceFlags = 0, cgUInt32 childResourceFlags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    cgSampler                 * loadSampler                 ( cgWorld * world, const cgSurfaceShaderHandle & customShader, cgUInt32 sourceRefId, bool internal, cgUInt32 resourceFlags = 0, cgUInt32 childResourceFlags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) );
    cgSampler                 * getDefaultSampler           ( DefaultSamplerTypes type );

    // Bezier Splines (Not a resource type).
    cgBezierSpline2           * createBezierSpline          ( );
    cgBezierSpline2           * createBezierSpline          ( cgWorld * world );
    cgBezierSpline2           * cloneBezierSpline           ( cgBezierSpline2 * init );
    cgBezierSpline2           * cloneBezierSpline           ( cgWorld * world, cgBezierSpline2 * init );
    cgBezierSpline2           * loadBezierSpline            ( cgWorld * world, cgUInt32 sourceRefId );

    // Debugging
    void                        debugResources              ( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID       & getReferenceType            ( ) const { return RTID_ResourceManager; }
    virtual bool                queryReferenceType          ( const cgUID & type ) const;
    virtual bool                processMessage              ( cgMessage * message );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void                dispose                     ( bool disposeBase );

private:
    //-------------------------------------------------------------------------
    // Private Structures
    //-------------------------------------------------------------------------
    struct ShaderKey
    {
        // Constructor
        ShaderKey( const cgShaderIdentifier * compareIdentifier ) :
            identifier( compareIdentifier ) {}
        
        // Public Variables
        const cgShaderIdentifier * identifier;   // The identifier we're comparing against
    
    }; // End Struct ShaderKey
    friend bool CGE_API operator < ( const ShaderKey&, const ShaderKey& );

    struct SamplerStateKey
    {
        // Constructor
        SamplerStateKey( const cgSamplerStateDesc * compareDesc ) :
            desc( compareDesc ) {}
        
        // Public Variables
        const cgSamplerStateDesc * desc;   // The description we're comparing against
    
    }; // End Struct SamplerStateKey
    friend bool CGE_API operator < ( const SamplerStateKey&, const SamplerStateKey& );

    struct DepthStencilStateKey
    {
        // Constructor
        DepthStencilStateKey( const cgDepthStencilStateDesc * compareDesc ) :
            desc( compareDesc ) {}
        
        // Public Variables
        const cgDepthStencilStateDesc * desc;   // The description we're comparing against
    
    }; // End Struct DepthStencilStateKey
    friend bool CGE_API operator < ( const DepthStencilStateKey&, const DepthStencilStateKey& );

    struct RasterizerStateKey
    {
        // Constructor
        RasterizerStateKey( const cgRasterizerStateDesc * compareDesc ) :
            desc( compareDesc ) {}
        
        // Public Variables
        const cgRasterizerStateDesc * desc;   // The description we're comparing against
    
    }; // End Struct RasterizerStateKey
    friend bool CGE_API operator < ( const RasterizerStateKey&, const RasterizerStateKey& );

    struct BlendStateKey
    {
        // Constructor
        BlendStateKey( const cgBlendStateDesc * compareDesc ) :
            desc( compareDesc ) {}
        
        // Public Variables
        const cgBlendStateDesc * desc;   // The description we're comparing against
    
    }; // End Struct BlendStateKey
    friend bool CGE_API operator < ( const BlendStateKey&, const BlendStateKey& );
    
    //-------------------------------------------------------------------------
    // Private Typedefs
    //-------------------------------------------------------------------------
    CGE_LIST_DECLARE(cgResource*, ResourceItemList)
    CGE_SET_DECLARE (cgResource*, ResourceItemSet)
    CGE_MAP_DECLARE (ShaderKey, cgResource*, ShaderMap)
    CGE_MAP_DECLARE (SamplerStateKey, cgResource*, SamplerStateMap)
    CGE_MAP_DECLARE (DepthStencilStateKey, cgResource*, DepthStencilStateMap)
    CGE_MAP_DECLARE (RasterizerStateKey, cgResource*, RasterizerStateMap)
    CGE_MAP_DECLARE (BlendStateKey, cgResource*, BlendStateMap)
    CGE_MAP_DECLARE (cgString, cgResource*, NamedResourceMap)
    
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                    postInit                    ( );
    cgString                listActiveResources         ( ResourceItemList & resourceList );
    void                    notifyDeviceLost            ( ResourceItemList & resourceList );
    void                    notifyDeviceRestored        ( ResourceItemList & resourceList );
    bool                    setRenderDriver             ( cgRenderDriver * driver );
    bool                    setAudioDriver              ( cgAudioDriver * driver );

    template <class _HandleType, class _ResourceType>
    bool                    processNewResource          ( _HandleType * resourceOut, _ResourceType * newResource, ResourceItemList & resourceList, const cgString & resourceName, cgStreamType::Base streamType, cgUInt32 flags, const cgDebugSourceInfo & _debugSource );
    template <class _HandleType, class _ResourceType>
    bool                    processExistingResource     ( _HandleType * resourceOut, _ResourceType * existingResource, cgUInt32 flags );
    
    // Resource Search Methods
    cgResource            * findTexture                 ( const cgString & resourceName ) const;
    cgResource            * findMesh                    ( const cgString & resourceName ) const;
    cgResource            * findMesh                    ( cgUInt32 referenceId ) const;
    cgResource            * findMaterial                ( cgUInt32 referenceId ) const;
    cgResource            * findRenderTarget            ( const cgString & resourceName ) const;
    cgResource            * findDepthStencilTarget      ( const cgString & resourceName ) const;
    cgResource            * findAnimationSet            ( const cgString & resourceName ) const;
    cgResource            * findAnimationSet            ( cgUInt32 referenceId ) const;
    cgResource            * findAudioBuffer             ( const cgString & resourceName, cgUInt32 creationFlags ) const;
    cgResource            * findScript                  ( const cgString & resourceName, bool shaderScript = false ) const;
    cgResource            * findVertexShader            ( const cgShaderIdentifier * cacheIdentifier );
    cgResource            * findPixelShader             ( const cgShaderIdentifier * cacheIdentifier );
    cgResource            * findSamplerState            ( const cgSamplerStateDesc * description );
    cgResource            * findDepthStencilState       ( const cgDepthStencilStateDesc * description );
    cgResource            * findRasterizerState         ( const cgRasterizerStateDesc * description );
    cgResource            * findBlendState              ( const cgBlendStateDesc * description );

    // cgResource Management Methods
    void                    removeResource              ( cgResource * resource );
    void                    releaseResourceList         ( ResourceItemList & resourceList, bool ownedOnly = true );
    void                    addGarbageResource          ( cgResource * resource );
    void                    removeGarbageResource       ( cgResource * resource );
    void                    checkGarbage                ( bool bEmpty = false );
    void                    sendGarbageMessage          ( );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    InitConfig                  mConfig;                        // Configuration for the resource manager.
    bool                        mDestructionEnabled;            // When destruction is disabled, resources are forced into the garbage queue on unload until it is re-enabled.

    ResourceItemList            mTextures;                      // List of texture resources.
    ResourceItemList            mVertexBuffers;                 // List of vertex buffer resources.
    ResourceItemList            mIndexBuffers;                  // List of index buffer resources.
    ResourceItemList            mConstantBuffers;               // List of constant buffer resources.
    ResourceItemList            mSurfaceShaders;                // List of surface shader resources.
    ResourceItemList            mVertexShaders;                 // List of vertex shader resources.
    ResourceItemList            mPixelShaders;                  // List of pixel shader resources.
    ResourceItemList            mMeshes;                        // List of mesh resources.
    ResourceItemList            mRenderTargets;                 // List of render target resources.
    ResourceItemList            mDepthStencilTargets;           // List of depth stencil target resources.
    ResourceItemList            mAnimationSets;                 // List of animation set resources.
    ResourceItemList            mAudioBuffers;                  // List of audio buffer resources.
    ResourceItemList            mScripts;                       // List of script resources.
    ResourceItemList            mSamplerStates;                 // List of sampler state resources.
    ResourceItemList            mDepthStencilStates;            // List of depth stencil state resources.
    ResourceItemList            mRasterizerStates;              // List of rasterizer state resources.
    ResourceItemList            mBlendStates;                   // List of blend state resources.
    ResourceItemSet             mResourceGarbage;               // Set of resources which are due to be released at some point in the future

    // Lookup Tables
    ShaderMap                   mVertexShaderLUT;               // Vertex shader dictionary for fast lookup by identifier.
    ShaderMap                   mPixelShaderLUT;                // Pixel shader dictionary for fast lookup by identifier.
    SamplerStateMap             mSamplerStateLUT;               // Sampler state dictionary for fast lookup by state values.
    DepthStencilStateMap        mDepthStencilStateLUT;          // Depth stencil state dictionary for fast lookup by state values.
    RasterizerStateMap          mRasterizerStateLUT;            // Rasterizer state dictionary for fast lookup by state values.
    BlendStateMap               mBlendStateLUT;                 // Blend state dictionary for fast lookup by state values.
    NamedResourceMap            mRenderTargetLUT;               // Render target dictionary for fast lookup by resource name.
    
    // Material Data
    ResourceItemList            mMaterials;                     // List of material resources.
    NameUsageMap                mMaterialNames;                 // A list of names currently in use by resident materials.
    
    cgRenderDriver            * mRenderDriver;                  // The render driver to which we are tied.
    cgAudioDriver             * mAudioDriver;                   // The audio driver to which we are tied.
    cgSampler                 * mDefaultSamplers[2];            // The default samplers to apply if one is not explicitly available.
    cgScriptHandle              mDefaultShader;                 // Default surface shader script to use if none is supplied within a material.
    cgString                    mDefaultShaderFile;             // The name of the default surface shader file to associate if unavailable.
    cgMaterialHandle            mDefaultMaterial;               // Default material used for the 'CollapseMaterials' batching method.

    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static cgFloat              mDefaultDestroyDelay;         // The default destruction delay for all resources
    static cgResourceManager  * mSingleton;                   // Static singleton object instance.
};

//-----------------------------------------------------------------------------
// Global Operators
//-----------------------------------------------------------------------------
inline bool CGE_API operator < (const cgResourceManager::MaterialKey & key1, const cgResourceManager::MaterialKey& key2);
inline bool CGE_API operator < (const cgResourceManager::ShaderKey & key1, const cgResourceManager::ShaderKey& key2);
inline bool CGE_API operator < (const cgResourceManager::SamplerStateKey & key1, const cgResourceManager::SamplerStateKey& key2);
inline bool CGE_API operator < (const cgResourceManager::DepthStencilStateKey & key1, const cgResourceManager::DepthStencilStateKey& key2);
inline bool CGE_API operator < (const cgResourceManager::RasterizerStateKey & key1, const cgResourceManager::RasterizerStateKey& key2);
inline bool CGE_API operator < (const cgResourceManager::BlendStateKey & key1, const cgResourceManager::BlendStateKey& key2);

#endif // !_CGE_CGRESOURCEMANAGER_H_