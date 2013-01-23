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
// Name : cgResourceManager.cpp                                              //
//                                                                           //
// Desc : This file houses all of the classes responsible for the creation   //
//        and management of resources to be used within the application.     //
//        This includes items such as vertex/index buffers, textures, meshes //
//        scripts and surface material information.                          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgResourceManager Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgResourceManager.h>
#include <Resources/cgAnimationSet.h>
#include <Resources/cgStandardMaterial.h>
#include <Resources/cgLandscapeLayerMaterial.h>
#include <Resources/cgSurfaceShaderScript.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgHardwareShaders.h>
#include <Resources/cgAudioBuffer.h>
#include <Resources/cgIndexBuffer.h>
#include <Resources/cgMesh.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgDepthStencilTarget.h>
#include <Resources/cgScript.h>
#include <Resources/cgTexture.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgStateBlocks.h>
#include <Rendering/cgRenderingCapabilities.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgSampler.h>
#include <Scripting/cgScriptEngine.h>
#include <Audio/cgAudioDriver.h>
#include <System/cgMessageTypes.h>
#include <System/cgStringUtility.h>
#include <System/cgXML.h> // ToDo: Remove
#include <Math/cgBezierSpline.h>

//-----------------------------------------------------------------------------
// Static member definitions.
//-----------------------------------------------------------------------------
cgResourceManager * cgResourceManager::mSingleton           = CG_NULL;
cgFloat             cgResourceManager::mDefaultDestroyDelay = 0.0f;

//-----------------------------------------------------------------------------
//  Name : cgResourceManager () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager::cgResourceManager( ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mRenderDriver             = CG_NULL;
    mAudioDriver              = CG_NULL;
    mDestructionEnabled       = true;

    // Clear structures
    memset( mDefaultSamplers, 0, sizeof(mDefaultSamplers) );
    memset( &mConfig, 0, sizeof(InitConfig) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgResourceManager () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager::~cgResourceManager()
{
    // Release allocated memory
    dispose( false );

    // Clear variables
    mRenderDriver  = CG_NULL;
    mAudioDriver   = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : setDefaultDestroyDelay () (Static)
/// <summary>
/// Set the default destruction delay for all resources allocated after
/// this call is made. This is an easy way to set the delay for a batch
/// of resources without having to call 'cgResource::SetDestroyDelay'
/// explicitly for each resource.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::setDefaultDestroyDelay( cgFloat fDelay )
{
    mDefaultDestroyDelay = fDelay;
}

//-----------------------------------------------------------------------------
//  Name : getDefaultDestroyDelay () (Static)
/// <summary>
/// Get the default destruction delay for all resources allocated.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgResourceManager::getDefaultDestroyDelay( )
{
    return mDefaultDestroyDelay;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgResourceManager::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Re-enable resource destruction if it was disabled.
    mDestructionEnabled = true;

    // Destroy default samplers.
    if ( mDefaultSamplers[DefaultDiffuseSampler] )
        mDefaultSamplers[DefaultDiffuseSampler]->scriptSafeDispose();
    if ( mDefaultSamplers[DefaultNormalSampler] )
        mDefaultSamplers[DefaultNormalSampler]->scriptSafeDispose();
    memset( mDefaultSamplers, 0, sizeof(mDefaultSamplers) );

    // Release defalut material.
    mDefaultMaterial.close();

    // Release all references to all resources
    releaseResourceList( mMeshes );
    releaseResourceList( mMaterials );
    releaseResourceList( mRenderTargets );
    releaseResourceList( mDepthStencilTargets );
    releaseResourceList( mVertexBuffers );
    releaseResourceList( mIndexBuffers );
    releaseResourceList( mConstantBuffers );
    releaseResourceList( mPixelShaders );
    releaseResourceList( mVertexShaders );
    releaseResourceList( mSurfaceShaders );
    releaseResourceList( mTextures );
    releaseResourceList( mAnimationSets );
    releaseResourceList( mAudioBuffers );
    releaseResourceList( mScripts );
    releaseResourceList( mSamplerStates );
    releaseResourceList( mDepthStencilStates );
    releaseResourceList( mRasterizerStates );
    releaseResourceList( mBlendStates );

    // Clear the lookup tables.
    mMaterialNames.clear();
    mVertexShaderLUT.clear();
    mPixelShaderLUT.clear();
    mSamplerStateLUT.clear();
    mDepthStencilStateLUT.clear();
    mRasterizerStateLUT.clear();
    mBlendStateLUT.clear();
    mRenderTargetLUT.clear();

    // Clear garbage list
    mResourceGarbage.clear();

    // Dispose base class if required.
    if ( bDisposeBase == true )
    {
        cgReference::dispose( true );
    
    } // End if dispose base
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager * cgResourceManager::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createSingleton () (Static)
/// <summary>
/// Creates the singleton. You would usually allocate the singleton in
/// the static member definition, however sometimes it's necessary to
/// call for allocation to allow for correct allocation ordering
/// and destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::createSingleton( )
{
    // Allocate!
    if ( mSingleton == CG_NULL )
        mSingleton = new cgResourceManager();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton != CG_NULL )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getBufferFormats ()
/// <summary>
/// Retrieve the enumerated buffer format information.
/// </summary>
//-----------------------------------------------------------------------------
const cgBufferFormatEnum & cgResourceManager::getBufferFormats( ) const
{
    return mRenderDriver->getCapabilities()->getBufferFormats();
}

//-----------------------------------------------------------------------------
//  Name : getRenderDriver ()
/// <summary>
/// Retrieve the render driver associated with this resource manager.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgResourceManager::getRenderDriver( )
{
    return mRenderDriver;
}

//-----------------------------------------------------------------------------
//  Name : getAudioDriver ()
/// <summary>
/// Retrieve the audio driver associated with this resource manager.
/// </summary>
//-----------------------------------------------------------------------------
cgAudioDriver * cgResourceManager::getAudioDriver( )
{
    return mAudioDriver;
}

//-----------------------------------------------------------------------------
//  Name : loadConfig ()
/// <summary>
/// Load the resource configuration from the file specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadConfig( const cgString & strFileName )
{
    cgTChar strValue[ MAX_PATH ];

    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Retrieve configuration options
    mConfig.compressTextures  = GetPrivateProfileInt( _T("Resources"), _T("CompressTextures"), 1, strFileName.c_str() ) > 0;
    mConfig.textureMipLevels  = GetPrivateProfileInt( _T("Resources"), _T("TextureMipLevels"), 0, strFileName.c_str() );

    // Retrieve the default surface shader details
    GetPrivateProfileString( _T("Resources"), _T("DefaultShader"), _T(""), strValue, MAX_PATH, strFileName.c_str() );
    mDefaultShaderFile = strValue;
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : saveConfig ()
/// <summary>
/// Save the configuration to the file specified.
/// Note : When specifying the save filename, it's important to either use a
/// full path ("C:\\{Path}\\Config.ini") or a path relative to the
/// current directory INCLUDING the first period (".\\Config.ini"). If
/// not, windows will place the ini in the windows directory rather than
/// the application dir.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::saveConfig( const cgString & strFileName )
{
    cgString strBuffer;

    // Validate requirements
    if ( strFileName.empty() == true )
        return false;

    // Save configuration options
    strBuffer = cgString::format( _T("%i"), mConfig.compressTextures );
    WritePrivateProfileString( _T("Resources"), _T("CompressTextures"), strBuffer.c_str(), strFileName.c_str() );
    strBuffer = cgString::format( _T("%i"), mConfig.textureMipLevels );
    WritePrivateProfileString( _T("Resources"), _T("TextureMipLevels"), strBuffer.c_str(), strFileName.c_str() );
    WritePrivateProfileString( _T("Resources"), _T("DefaultShader"), mDefaultShaderFile.c_str(), strFileName.c_str() );
    
    // Success!!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager::InitConfig cgResourceManager::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the resource manager with required properties.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::initialize( )
{
    // Release any previous resources
    dispose( false );

    // Make sure that required directories exist.
    cgString requiredPath = cgFileSystem::getAbsolutePath( cgFileSystem::resolveFileLocation( _T("sys://Cache/Shaders/") ) );
    if ( !cgFileSystem::directoryExists( requiredPath ) )
        cgFileSystem::createDirectory( requiredPath );
    
    // Begin the garbage collection scheme
    sendGarbageMessage();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setRenderDriver () (Private)
/// <summary>
/// Set by the friend 'cgRenderDriver::initialize' function, to pass back
/// the render driver pointer through which the resource manager can
/// access the driver's data / functionality.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::setRenderDriver( cgRenderDriver * pDriver )
{
    // Cannot alter initial driver.
    if ( mRenderDriver != CG_NULL )
        return false;

    // Store render driver pointer
    mRenderDriver = pDriver;

    // Register us with the render driver group so that we
    // can response to device events.
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : postInit () (Private)
/// <summary>
/// Called by the friend 'cgRenderDriver::postInit' function to allow the
/// resource manager to fully initialize based on existing render driver
/// features / resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::postInit( )
{
    // Load the default surface shader script if any supplied. New instance
    // of the surface shader class defined within this script will be created
    // for any material that does not define its own shader file.
    if ( !mDefaultShaderFile.empty() )
        loadSurfaceShaderScript( &mDefaultShader, mDefaultShaderFile, cgResourceFlags::AlwaysResident, cgDebugSource() );

    // Create the default material used primarily for the 'CollapseMaterials' batching method.
    createMaterial( &mDefaultMaterial, cgMaterialType::Standard, 0, cgDebugSource() );
    cgMaterial * pDefaultMaterial = mDefaultMaterial.getResource(true);
    pDefaultMaterial->beginPopulate( this );
    pDefaultMaterial->setName( _T("Core::Resources::DefaultMaterial") );
    pDefaultMaterial->endPopulate( );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setAudioDriver () (Private)
/// <summary>
/// Set by the friend 'cgRenderDriver::initialize' function, to pass back
/// the render driver pointer through which the resource manager can
/// access the drivers data / functionality.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::setAudioDriver( cgAudioDriver * pDriver )
{
    // Cannot alter initial driver.
    if ( mAudioDriver != CG_NULL )
        return false;

    // Store audio driver pointer
    mAudioDriver = pDriver;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : releaseOwnedResources ()
/// <summary>
/// Release any resources that are OWNED by the resource manager
/// (i.e. AlwaysResident flag set).
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::releaseOwnedResources()
{
    // Release our reference to the default surface shader script.
    mDefaultShader.close( true );

    // Release our reference to the default material.
    mDefaultMaterial.close( true );

    // Release our references to the default samplers
    if ( mDefaultSamplers[DefaultDiffuseSampler] )
        mDefaultSamplers[DefaultDiffuseSampler]->scriptSafeDispose();
    if ( mDefaultSamplers[DefaultNormalSampler] != CG_NULL )
        mDefaultSamplers[DefaultNormalSampler]->scriptSafeDispose();
    memset( mDefaultSamplers, 0, sizeof(mDefaultSamplers) );

    // Release our own references to resources created with the AlwaysResident flag
    releaseResourceList( mMeshes, true );
    releaseResourceList( mMaterials, true );
    releaseResourceList( mRenderTargets, true );
    releaseResourceList( mDepthStencilTargets, true );
    releaseResourceList( mVertexBuffers, true );
    releaseResourceList( mIndexBuffers, true );
    releaseResourceList( mConstantBuffers, true );
    releaseResourceList( mPixelShaders, true );
    releaseResourceList( mVertexShaders, true );
    releaseResourceList( mSurfaceShaders, true );
    releaseResourceList( mTextures, true );
    releaseResourceList( mAnimationSets, true );
    releaseResourceList( mAudioBuffers, true );
    releaseResourceList( mScripts, true );
    releaseResourceList( mSamplerStates, true );
    releaseResourceList( mRasterizerStates, true );
    releaseResourceList( mDepthStencilStates, true );
    releaseResourceList( mBlendStates, true );
}

//-----------------------------------------------------------------------------
//  Name : loadTexture ()
/// <summary>
/// Given the specified input stream, load and return a handle to a
/// texture resource object.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadTexture( cgTextureHandle * hResOut, const cgInputStream & Stream, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the texture if it already exists and user is not forcing new.
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgTexture * pExistingTexture = (cgTexture*)findTexture( Stream.getName() );
        if ( pExistingTexture )
            return processExistingResource( hResOut, pExistingTexture, nFlags );
    
    } // End if !ForceNew

    // Allow textures with no source file to remain resident only if we're in sandbox mode
    // (preview or full). This ensures that even if a texture is not found, its information 
    // will not be lost.
    if ( (cgGetSandboxMode() != cgSandboxMode::Disabled) && (Stream.getType() == cgStreamType::None) )
        return false;

    // No texture found, so lets build a new texture resource.
    cgTexture * pNewTexture = cgTexture::createInstance( cgReferenceManager::generateInternalRefId(), Stream, getRenderDriver(), mConfig.textureMipLevels );

    // Process the new resource
    return processNewResource( hResOut, pNewTexture, mTextures, Stream.getName(), Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createTexture ()
/// <summary>
/// Create a new 1D texture based on the curve data specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createTexture( cgTextureHandle * hResOut, cgBezierSpline2 & Spline, cgUInt32 Width, cgUInt32 MipLevels, bool bRightToLeft /* = false */, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // If this is a complex spline, we must fail.
    if ( Spline.isComplex() == true )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Unable to generate texture from spline data because that data is classified as complex.\n") );
        return false;

    } // End if complex

    // Attempt to find the texture if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgTexture * pExistingTexture = (cgTexture*)findTexture( strResourceName );
        if ( pExistingTexture )
            return processExistingResource( hResOut, pExistingTexture, nFlags );

    } // End if resource name provided

    // Select a valid single channel format. Hopefully A8 is available to use, otherwise 
    // we allow padding out to a multi-channel texture format.
    const cgBufferFormatEnum & Formats = getBufferFormats();
    cgUInt32 nSearchFlags = cgFormatSearchFlags::OneChannel | cgFormatSearchFlags::RequireAlpha | cgFormatSearchFlags::AllowPaddingChannels;
    cgBufferFormat::Base SelectedFormat = Formats.getBestFormat( cgBufferType::Texture1D, nSearchFlags );

    // No texture found, so lets build a new texture resource.
    cgImageInfo Info;
    Info.type      = cgBufferType::Texture1D;
    Info.width     = Width;
    Info.height    = 1;
    Info.mipLevels = MipLevels;
    Info.format    = SelectedFormat;
    Info.pool      = cgMemoryPool::Managed;
    cgTexture * pNewTexture = cgTexture::createInstance( cgReferenceManager::generateInternalRefId(), Info );

    // Process the new resource
    if ( processNewResource( hResOut, pNewTexture, mTextures, strResourceName, cgStreamType::Memory, nFlags, _debugSource ) == false )
        return false;

    // Lock the texture and populate it based on the evaluated curve.
    cgUInt32 nPitch, nStride = cgBufferFormatEnum::formatBitsPerPixel( SelectedFormat ) / 8;
    cgByte * pData = (cgByte*)pNewTexture->lock( nPitch, cgLockFlags::WriteOnly | cgLockFlags::Discard );
    if ( pData == CG_NULL )
    {
        // Destroy and remove the resource immediately.
        (*hResOut).close( true );
        return false;

    } // End if failed

    // Step over the surface and populate
    for ( cgUInt32 i = 0; i < Width; ++i )
    {
        cgFloat fDelta = (cgFloat)i / (cgFloat)(Width-1);
        cgFloat fValue = Spline.evaluateForX( (bRightToLeft == true) ? 1.0f - fDelta : fDelta, false, 3 );
        
        // Clamp the curve value to the 0 - 1 range.
        if ( fValue < 0.0f ) fValue = 0.0f;
        if ( fValue > 1.0f ) fValue = 1.0f;
        
        // Write the value
        *pData = (cgByte)(fValue * 255.0f);
        pData += nStride;

    } // Next pixel

    // Finish up (unlock and generate mip chain)
    pNewTexture->unlock( true );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createTexture ()
/// <summary>
/// Create a new texture using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createTexture( cgTextureHandle * hResOut, const cgImageInfo & Description, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the texture if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgTexture * pExistingTexture = (cgTexture*)findTexture( strResourceName );
        if ( pExistingTexture )
            return processExistingResource( hResOut, pExistingTexture, nFlags );

    } // End if resource name provided

    // No texture found, so lets build a new texture resource.
    cgTexture * pNewTexture = cgTexture::createInstance( cgReferenceManager::generateInternalRefId(), Description );

    // Process the new resource
    return processNewResource( hResOut, pNewTexture, mTextures, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createTexture ()
/// <summary>
/// Create a new texture using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createTexture( cgTextureHandle * hResOut, cgBufferType::Base Type, cgUInt32 Width, cgUInt32 Height, cgUInt32 Depth, cgUInt32 MipLevels, cgBufferFormat::Base Format, cgMemoryPool::Base Pool, bool bAutoGenMips /* = false */, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the texture if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgTexture * pExistingTexture = (cgTexture*)findTexture( strResourceName );
        if ( pExistingTexture )
            return processExistingResource( hResOut, pExistingTexture, nFlags );

    } // End if resource name provided

    // No texture found, so lets build a new texture resource.
    cgImageInfo Description;
    Description.type        = Type;
    Description.width       = Width;
    Description.height      = Height;
    Description.depth       = Depth;
    Description.mipLevels   = MipLevels;
    Description.format      = Format;
    Description.pool        = Pool;
    Description.autoGenerateMipmaps = bAutoGenMips;
    cgTexture * pNewTexture = cgTexture::createInstance( cgReferenceManager::generateInternalRefId(), Description );

    // Process the new resource
    return processNewResource( hResOut, pNewTexture, mTextures, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : cloneTexture ()
/// <summary>
/// Clone an existing texture using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::cloneTexture( cgTextureHandle * hResOut, cgTextureHandle hSourceTexture, cgBufferFormat::Base NewFormat /* = cgBufferFormat::Unknown */, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    cgToDoAssert( "DX11", "Re-implement in an API independant fashion!" );
    
    /*// Validate requirements
    if ( strResourceName.empty() == false && 
        (nFlags & cgResourceFlags::ForceNew) == 0 &&
        (nFlags & cgResourceFlags::ForceUnique) == 0 )
    {
        // Attempt to find the texture if it already exists
        cgTexture * pExistingTexture = (cgTexture*)findTexture( strResourceName );
        if ( pExistingTexture != CG_NULL && pExistingTexture->IsUnique() == false ) 
            return processExistingResource( hResOut, pExistingTexture, nFlags );

    } // End if resource name provided

    // Access to the source texture and ensure that this is a valid clone.
    cgTexture * pSourceTexture = hSourceTexture.getResource(true);
    if ( pSourceTexture == CG_NULL )
        return false;

    // Auto-detect cloned format if none supplied.
    cgImageInfo Description = pSourceTexture->GetInfo();
    if ( NewFormat != cgBufferFormat::Unknown )
        Description.format = NewFormat;

    // Create a new texture into which data can be cloned.
    cgTexture * pNewTexture = cgTexture::createInstance( cgReferenceManager::generateInternalRefId(), Description );

    // Process the new resource
    if ( processNewResource( hResOut, pNewTexture, mTextures, strResourceName, cgStreamType::Memory, nFlags, _debugSource ) == false )
        return false;

    // Force new texture to generate itself in case it deferred loading.
    if ( pNewTexture->GuaranteedLoad() == false )
    {
        if ( hResOut )
            hResOut->close( true );
        return false;
    
    } // End if failed

    cgToDoAssert( "Carbon General", "DepthStencil cannot always be cloned and this will fail in such cases. This should also ideally move into a platform specific texture method." );

    // Perform the clone.
    switch ( Description.Type )
    {
        case cgBufferType::Texture:
        case cgBufferType::RenderTarget:
        case cgBufferType::DepthStencil:
        case cgBufferType::ShadowMap:
        {
            LPDIRECT3DTEXTURE9 pSrcTexture = CG_NULL, pDstTexture = CG_NULL;
            LPDIRECT3DSURFACE9 pSrcSurface = CG_NULL, pDstSurface = CG_NULL;
            
            // Retrieve both textures.
            pSrcTexture = (LPDIRECT3DTEXTURE9)pSourceTexture->GetManagedData();
            pDstTexture = (LPDIRECT3DTEXTURE9)pNewTexture->GetManagedData();

            // Clone all mip levels
            for ( cgUInt32 i = 0; i < pSrcTexture->GetLevelCount(); ++i )
            {
                pSrcTexture->GetSurfaceLevel( i, &pSrcSurface );
                pDstTexture->GetSurfaceLevel( i, &pDstSurface );

                // Perform the clone.
                D3DXLoadSurfaceFromSurface( pDstSurface, CG_NULL, CG_NULL, pSrcSurface, CG_NULL, CG_NULL, D3DX_DEFAULT, 0 );

                // Clean up
                pSrcSurface->Release();
                pDstSurface->Release();

            } // Next mip level

            // Clean up
            pSrcTexture->Release();
            pDstTexture->Release();
            break;

        } // End Case Standard | RenderTarget | DepthStencil
        case cgBufferType::CubeMap:
        case cgBufferType::CubeRenderTarget:
        {
            LPDIRECT3DCUBETEXTURE9 pSrcTexture = CG_NULL, pDstTexture = CG_NULL;
            LPDIRECT3DSURFACE9     pSrcSurface = CG_NULL, pDstSurface = CG_NULL;
            
            // Retrieve both textures.
            pSrcTexture = (LPDIRECT3DCUBETEXTURE9)pSourceTexture->GetManagedData();
            pDstTexture = (LPDIRECT3DCUBETEXTURE9)pNewTexture->GetManagedData();

            // Clone all faces at all mip levels
            for ( cgUInt32 i = 0; i < 6; ++i )
            {
                for ( cgUInt32 j = 0; j < pSrcTexture->GetLevelCount(); ++j )
                {

                    pSrcTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, j, &pSrcSurface );
                    pDstTexture->GetCubeMapSurface( (D3DCUBEMAP_FACES)i, j, &pDstSurface );

                    // Perform the clone.
                    D3DXLoadSurfaceFromSurface( pDstSurface, CG_NULL, CG_NULL, pSrcSurface, CG_NULL, CG_NULL, D3DX_DEFAULT, 0 );

                    // Clean up
                    pSrcSurface->Release();
                    pDstSurface->Release();

                } // Next mip level

            } // Next cube face

            // Clean up
            pSrcTexture->Release();
            pDstTexture->Release();
            break;

        } // End Case CubeMap | CubeRenderTarget
        case cgBufferType::Volume:
        {
            LPDIRECT3DVOLUMETEXTURE9 pSrcTexture = CG_NULL, pDstTexture = CG_NULL;
            LPDIRECT3DVOLUME9        pSrcVolume  = CG_NULL, pDstVolume  = CG_NULL;
            
            // Retrieve both textures.
            pSrcTexture = (LPDIRECT3DVOLUMETEXTURE9)pSourceTexture->GetManagedData();
            pDstTexture = (LPDIRECT3DVOLUMETEXTURE9)pNewTexture->GetManagedData();

            // Clone all mip levels
            for ( cgUInt32 i = 0; i < pSrcTexture->GetLevelCount(); ++i )
            {
                pSrcTexture->GetVolumeLevel( i, &pSrcVolume );
                pDstTexture->GetVolumeLevel( i, &pDstVolume );

                // Perform the clone.
                D3DXLoadVolumeFromVolume( pDstVolume, CG_NULL, CG_NULL, pSrcVolume, CG_NULL, CG_NULL, D3DX_DEFAULT, 0 );

                // Clean up
                pSrcVolume->Release();
                pDstVolume->Release();

            } // Next mip level

            // Clean up
            pSrcTexture->Release();
            pDstTexture->Release();
            break;
        
        } // End Case Volume
        default:
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to clone texture '%s' into new texture '%s'. Unknown type detected.\n"), pSourceTexture->getResourceName().c_str(), strResourceName.c_str() );
            return false;
    
    } // End Switch Type

    // Success!
    return true;*/

    return false;
}

//-----------------------------------------------------------------------------
//  Name : addTexture( )
/// <summary>
/// Simply add a texture to the resource database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::addTexture( cgTextureHandle * hResOut, cgTexture * pTexture, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Validate requirements
    if ( !pTexture )
        return false;

    // Attempt to find the texture if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgTexture * pExistingTexture = (cgTexture*)findTexture( strResourceName );
        if ( pExistingTexture ) 
            return processExistingResource( hResOut, pExistingTexture, nFlags );

    } // End if resource name provided

    // Process the new resource
    return processNewResource( hResOut, pTexture, mTextures, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : getTexture ()
/// <summary>
/// Retrieve a resource handle to an underlying texture based on the name
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getTexture( cgTextureHandle * hResOut, const cgString & strResourceName )
{
    cgResource * pTexture = findTexture( strResourceName );
    if ( pTexture == CG_NULL ) return false;

    // Return the relevant texture
    if ( hResOut != CG_NULL )
        *hResOut = cgTextureHandle( (cgTexture*)pTexture, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Texture found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findTexture ()
/// <summary>
/// Given the specified resource name, find the resource object for any
/// matching texture found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findTexture( const cgString & strResourceName ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Loop through and see if this texture already exists.
    for ( itItem = mTextures.begin(); itItem != mTextures.end(); ++itItem )
    {
        cgTexture * pResource = (cgTexture*)(*itItem);
        if ( pResource == CG_NULL ) continue;

        // Found one with matching details?
        if ( strResourceName == pResource->getResourceName() ) 
            return pResource;

    } // Next Texture

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : loadAudioBuffer ()
/// <summary>
/// Given the specified input stream, load and return a handle to a
/// audio buffer resource object.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadAudioBuffer( cgAudioBufferHandle * hResOut, const cgInputStream & Stream, cgUInt32 nSoundFlags /* = 0 */, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // First, attempt to find the audio buffer if it already exists
    cgAudioBuffer * pExistingBuffer = (cgAudioBuffer*)findAudioBuffer( Stream.getName(), nSoundFlags );

    // Did we find an existing resource we can duplicate from?
    cgAudioBuffer * pNewBuffer = CG_NULL;
    if ( pExistingBuffer != CG_NULL && pExistingBuffer->supportsMode( cgAudioBufferFlags::Streaming ) == false )
    {
        // Duplicate the buffer.
        pNewBuffer = new cgAudioBuffer( cgReferenceManager::generateInternalRefId(), mAudioDriver, pExistingBuffer );
    
    } // End if duplicate
    else
    {
        // Must be valid stream.
        if ( Stream.getType() == cgStreamType::None )
            return false;
        
        // Load new sound from stream
        pNewBuffer = new cgAudioBuffer( cgReferenceManager::generateInternalRefId(), mAudioDriver, Stream, nSoundFlags );

    } // End if load new

    // Process the new resource
    return processNewResource( hResOut, pNewBuffer, mAudioBuffers, Stream.getName(), Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : getAudioBuffer ()
/// <summary>
/// Retrieve a resource handle to an underlying audio buffer based on the
/// name specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getAudioBuffer( cgAudioBufferHandle * hResOut, const cgString & strResourceName, cgUInt32 nCreationFlags /* = 0 */ )
{
    cgResource * pSound = findAudioBuffer( strResourceName, nCreationFlags );
    if ( pSound == CG_NULL ) return false;

    // Return the relevant sound
    if ( hResOut != CG_NULL )
        *hResOut = cgAudioBufferHandle( (cgAudioBuffer*)pSound, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Resource found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findAudioBuffer ()
/// <summary>
/// Given the specified resource name and creation flags, find the 
/// resource object for any matching audio buffer found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findAudioBuffer( const cgString & strResourceName, cgUInt32 nCreationFlags ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Loop through and see if this sound effect already exists.
    for ( itItem = mAudioBuffers.begin(); itItem != mAudioBuffers.end(); ++itItem )
    {
        cgAudioBuffer * pResource = (cgAudioBuffer*)(*itItem);
        if ( pResource == CG_NULL ) continue;

        // Found one with matching details?
        if ( (strResourceName == pResource->getResourceName()) && (nCreationFlags = pResource->getCreationFlags()) ) 
            return pResource;

    } // Next Sound

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createSamplerState ()
/// <summary>
/// Create a new sampler state block using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createSamplerState( cgSamplerStateHandle * hResOut, const cgSamplerStateDesc & Values, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find any existing resource
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgSamplerState * pExistingState = (cgSamplerState*)findSamplerState( &Values );
        if ( pExistingState )
            return processExistingResource( hResOut, pExistingState, nFlags );

    } // End if !ForceNew

    // No existing states found, so lets build a new resource.
    cgSamplerState * pNewState = cgSamplerState::createInstance( cgReferenceManager::generateInternalRefId(), Values );

    // Process the new resource
    cgString strName = cgString::format( _T("SamplerState(0x%x)"), pNewState->getReferenceId() );
    if ( !processNewResource( hResOut, pNewState, mSamplerStates, strName, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;
    
    // Add to the lookup table
    mSamplerStateLUT[ SamplerStateKey(&pNewState->getValues()) ] = pNewState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findSamplerState () (Private)
/// <summary>
/// Looks for a matching sampler state in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findSamplerState( const cgSamplerStateDesc * pDesc )
{
    SamplerStateMap::iterator itState = mSamplerStateLUT.find( SamplerStateKey( pDesc ) );
    if ( itState != mSamplerStateLUT.end() )
        return itState->second;
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createDepthStencilState ()
/// <summary>
/// Create a new depth stencil state block using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createDepthStencilState( cgDepthStencilStateHandle * hResOut, const cgDepthStencilStateDesc & Values, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find any existing resource
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgDepthStencilState * pExistingState = (cgDepthStencilState*)findDepthStencilState( &Values );
        if ( pExistingState )
            return processExistingResource( hResOut, pExistingState, nFlags );

    } // End if !ForceNew

    // No existing states found, so lets build a new resource.
    cgDepthStencilState * pNewState = cgDepthStencilState::createInstance( cgReferenceManager::generateInternalRefId(), Values );

    // Process the new resource
    cgString strName = cgString::format( _T("DepthStencilState(0x%x)"), pNewState->getReferenceId() );
    if ( !processNewResource( hResOut, pNewState, mDepthStencilStates, strName, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;
    
    // Add to the lookup table
    mDepthStencilStateLUT[ DepthStencilStateKey(&pNewState->getValues()) ] = pNewState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findDepthStencilState () (Private)
/// <summary>
/// Looks for a matching depth stencil state in the list and returns object 
/// pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findDepthStencilState( const cgDepthStencilStateDesc * pDesc )
{
    DepthStencilStateMap::iterator itState = mDepthStencilStateLUT.find( DepthStencilStateKey( pDesc ) );
    if ( itState != mDepthStencilStateLUT.end() )
        return itState->second;
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createRasterizerState ()
/// <summary>
/// Create a new rasterizer state block using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createRasterizerState( cgRasterizerStateHandle * hResOut, const cgRasterizerStateDesc & Values, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find any existing resource
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgRasterizerState * pExistingState = (cgRasterizerState*)findRasterizerState( &Values );
        if ( pExistingState )
            return processExistingResource( hResOut, pExistingState, nFlags );

    } // End if !ForceNew

    // No existing states found, so lets build a new resource.
    cgRasterizerState * pNewState = cgRasterizerState::createInstance( cgReferenceManager::generateInternalRefId(), Values );

    // Process the new resource
    cgString strName = cgString::format( _T("RasterizerState(0x%x)"), pNewState->getReferenceId() );
    if ( !processNewResource( hResOut, pNewState, mRasterizerStates, strName, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;
    
    // Add to the lookup table
    mRasterizerStateLUT[ RasterizerStateKey(&pNewState->getValues()) ] = pNewState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findRasterizerState () (Private)
/// <summary>
/// Looks for a matching sampler state in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findRasterizerState( const cgRasterizerStateDesc * pDesc )
{
    RasterizerStateMap::iterator itState = mRasterizerStateLUT.find( RasterizerStateKey( pDesc ) );
    if ( itState != mRasterizerStateLUT.end() )
        return itState->second;
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createBlendState ()
/// <summary>
/// Create a new blend state block using the specified details.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createBlendState( cgBlendStateHandle * hResOut, const cgBlendStateDesc & Values, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find any existing resource
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgBlendState * pExistingState = (cgBlendState*)findBlendState( &Values );
        if ( pExistingState )
            return processExistingResource( hResOut, pExistingState, nFlags );

    } // End if !ForceNew

    // No existing states found, so lets build a new resource.
    cgBlendState * pNewState = cgBlendState::createInstance( cgReferenceManager::generateInternalRefId(), Values );

    // Process the new resource
    cgString strName = cgString::format( _T("BlendState(0x%x)"), pNewState->getReferenceId() );
    if ( !processNewResource( hResOut, pNewState, mBlendStates, strName, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the lookup table
    mBlendStateLUT[ BlendStateKey(&pNewState->getValues()) ] = pNewState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findBlendState () (Private)
/// <summary>
/// Looks for a matching sampler state in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findBlendState( const cgBlendStateDesc * pDesc )
{
    BlendStateMap::iterator itState = mBlendStateLUT.find( BlendStateKey( pDesc ) );
    if ( itState != mBlendStateLUT.end() )
        return itState->second;
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : loadScript ()
/// <summary>
/// Given the specified stream, load and return a handle to a script
/// file resource handle.
/// Note : Overload of main script loading function that does not require a
/// "this" type, or a script instance identifier (assumes '.default').
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadScript( cgScriptHandle * hResOut, const cgInputStream & Stream, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    return loadScript( hResOut, Stream, _T(""), _T(".default"), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : loadScript ()
/// <summary>
/// Given the specified stream, load and return a handle to a script
/// file resource handle.
/// Note : Overload of main script loading function that does not require a
/// a script instance identifier (assumes '.default').
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadScript( cgScriptHandle * hResOut, const cgInputStream & Stream, const cgString & strThisType, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    return loadScript( hResOut, Stream, strThisType, _T(".default"), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : loadScript ()
/// <summary>
/// Given the specified filename, load and return a handle to a script
/// file resource.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadScript( cgScriptHandle * hResOut, const cgInputStream & Stream, const cgString & strThisType, const cgString & strInstanceId, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Build the script UID string used to identify unique scripts
    cgString strScriptUID = Stream.getName() + _T("::") + ((strInstanceId.empty() == true ) ? _T(".default") : strInstanceId);

    // Attempt to find the script if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgScript * pExistingScript = (cgScript*)findScript( strScriptUID, false );
        if ( pExistingScript ) 
            return processExistingResource( hResOut, pExistingScript, nFlags );
    
    } // End if !ForceNew

    // Validate requirements
    if ( Stream.getType() == cgStreamType::None )
        return false;

    // No script found, so lets allocate a new one.
    cgScript * pNewScript = new cgScript( cgReferenceManager::generateInternalRefId(), Stream, strThisType, cgScriptEngine::getInstance() );
    
    // Process the new resource
    return processNewResource( hResOut, pNewScript, mScripts, strScriptUID, Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : reloadScripts ()
/// <summary>
/// Reload all scripts (except surface shaders).
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::reloadScripts( cgFloat fDelay /* = 0.0f */ )
{
    if ( fDelay > 0 )
    {
        cgMessage Msg;
        Msg.messageId = cgSystemMessages::Resources_ReloadScripts;
        cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, fDelay );
    
    } // End if delay
    else
    {
        // Unload non surface shader scripts.
        ResourceItemList::iterator itResource;
        for ( itResource = mScripts.begin(); itResource != mScripts.end(); ++itResource )
        {
            if ( !(*itResource)->queryReferenceType(RTID_SurfaceShaderScriptResource) )
                (*itResource)->unloadResource();
        
        } // Next surface shader script

        // Reload scripts.
        for ( itResource = mScripts.begin(); itResource != mScripts.end(); ++itResource )
            (*itResource)->loadResource();

        // Send a message out to anyone interested to let
        // them know that scripts have been reloaded.
        cgMessage Msg;
        Msg.messageId = cgSystemMessages::Resources_ReloadScripts;
        cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_ResourceManager, &Msg, 0 );

    } // End if no delay

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findScript ()
/// <summary>
/// Looks for a matching script in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findScript( const cgString & strResourceName, bool bShaderScript /* = false */ ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() )
        return CG_NULL;

    // Loop through and see if this script file already exists.
    for ( itItem = mScripts.begin(); itItem != mScripts.end(); ++itItem )
    {
        cgScript * pResource = (cgScript*)(*itItem);
        if ( !pResource )
            continue;

        // Found one with matching details?
        if ( strResourceName == pResource->getResourceName() && bShaderScript == pResource->queryReferenceType(RTID_SurfaceShaderScriptResource) ) 
            return pResource;

    } // Next Script

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : loadSurfaceShaderScript ()
/// <summary>
/// Identical to the 'loadScript()' methods, with the exception that this 
/// version enables the surface shader syntax extension when compiling the
/// script.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadSurfaceShaderScript( cgScriptHandle * hResOut, const cgInputStream & Stream, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Build the script UID string used to identify unique scripts
    cgString strScriptUID = Stream.getName() + _T("::.default");

    // Attempt to find the script if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgScript * pExistingScript = (cgScript*)findScript( strScriptUID, true );
        if ( pExistingScript ) 
            return processExistingResource( hResOut, pExistingScript, nFlags );
    
    } // End if !ForceNew

    // Validate requirements
    if ( Stream.getType() == cgStreamType::None )
        return false;

    // No script found, so lets allocate a new one.
    cgScript * pNewScript = new cgSurfaceShaderScript( cgReferenceManager::generateInternalRefId(), Stream, cgString::Empty, cgScriptEngine::getInstance() );

    // ToDo: Needs to be defined by the engine at some point
    // Determine which system macros should be in use
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.renderAPI )
    {
        case cgRenderAPI::Null:
            break;

#if defined( CGE_DX9_RENDER_SUPPORT )

        case cgRenderAPI::DirectX9:
            pNewScript->defineMacro( _T("DX9C"), cgString::Empty );
            break;

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

        case cgRenderAPI::DirectX11:
            pNewScript->defineMacro( _T("DX11"), cgString::Empty );
            break;

#endif // CGE_DX11_RENDER_SUPPORT

#if defined( CGE_GL_RENDER_SUPPORT )

        case cgRenderAPI::OpenGL:
            pNewScript->defineMacro( _T("GLSL"), cgString::Empty );
            break;

#endif // CGE_GL_RENDER_SUPPORT

    } // End switch renderAPI
    
    // Process the new resource
    return processNewResource( hResOut, pNewScript, mScripts, strScriptUID, Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createSurfaceShader ()
/// <summary>
/// Given the specified input stream, create a surface shader instance based
/// on the surface shader /script/ in that stream. Surface shaders are always
/// unique.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createSurfaceShader( cgSurfaceShaderHandle * hResOut, const cgInputStream & Stream, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Surface shader instances are always unique.
    nFlags &= ~cgResourceFlags::ForceNew;

    // Valid file?
    if ( Stream.getType() == cgStreamType::None )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Unable to create surface shader from stream '%s' because the file or resource was not found.\n"), Stream.getName().c_str() );
        return false;
    
    } // End if not found

    // Build the new shader resource.
    cgSurfaceShader * pNewShader = new cgSurfaceShader( cgReferenceManager::generateInternalRefId(), Stream );

    // Process the new resource
    return processNewResource( hResOut, pNewShader, mSurfaceShaders, Stream.getName(), Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createDefaultSurfaceShader ()
/// <summary>
/// Create a surface shader instance based on the default surface shader 
/// /script/. Surface shaders are always unique resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createDefaultSurfaceShader( cgSurfaceShaderHandle * hResOut, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    return createSurfaceShader( hResOut, mDefaultShaderFile, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : reloadShaders ()
/// <summary>
/// Reload all surface shaders and the hardware shader permutations (vertex &
/// pixel shader, etc.) that they have generated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::reloadShaders( cgFloat fDelay /* = 0.0f */ )
{
    if ( fDelay > 0 )
    {
        cgMessage Msg;
        Msg.messageId = cgSystemMessages::Resources_ReloadShaders;
        cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, fDelay );
    
    } // End if delay
    else
    {
        // Unload all vertex shaders.
        ResourceItemList::iterator itResource;
        //for ( itResource = mVertexShaders.begin(); itResource != mVertexShaders.end(); ++itResource )
            //(*itResource)->unloadResource();
        releaseResourceList( mVertexShaders, false );

        // Unload all pixel shaders.
        //for ( itResource = mPixelShaders.begin(); itResource != mPixelShaders.end(); ++itResource )
            //(*itResource)->unloadResource();
        releaseResourceList( mPixelShaders, false );

        // Unload surface shaders.
        for ( itResource = mSurfaceShaders.begin(); itResource != mSurfaceShaders.end(); ++itResource )
            (*itResource)->unloadResource();

        // Unload surface shader scripts.
        for ( itResource = mScripts.begin(); itResource != mScripts.end(); ++itResource )
        {
            if ( (*itResource)->queryReferenceType(RTID_SurfaceShaderScriptResource) )
                (*itResource)->unloadResource();
        
        } // Next surface shader script

        // Reload surface shaders.
        for ( itResource = mSurfaceShaders.begin(); itResource != mSurfaceShaders.end(); ++itResource )
            (*itResource)->loadResource();

        // The rest (scripts and hardware shader permutations) should load up
        // automatically on first time use.

        // Send a message out to anyone interested to let
        // them know that shaders have been reloaded.
        cgMessage Msg;
        Msg.messageId = cgSystemMessages::Resources_ReloadShaders;
        cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_ResourceManager, &Msg, 0 );

    } // End if no delay

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadAnimationSet ( )
/// <summary>
/// Load the animation set with matching source reference identifier from the
/// specified world database. If the 'ForceNew' flag is specified and the
/// destination reference identifier is zero, then a new internal animation set 
/// with a unique identifier will be automatically generated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadAnimationSet( cgAnimationSetHandle * hResOut, cgWorld * pWorld, cgUInt32 nSourceRefId, bool bInternalResource, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // ToDo: 9999 - Add transaction for rollback on failure (in the cloning case)?

    // Validate requirements
    if ( !nSourceRefId )
        return false;

    // Force internal if no world was supplied.
    if ( !pWorld )
        bInternalResource = true;

    // Attempt to find an existing resident set with the source reference identifier.
    cgAnimationSet * pExistingSet = (cgAnimationSet*)findAnimationSet( nSourceRefId );
    if ( pExistingSet )
    {
        // If a source set was found we can potentially just return the existing resource 
        // depending on the supplied options. Specifically, if the user did not want a new 
        // resource, and both resources are internal *or* both are serialized, we can avoid
        // a clone operation.
        if ( !(nFlags & cgResourceFlags::ForceNew) &&
             bInternalResource == pExistingSet->isInternalReference() )
        {
            // No clone was required, we can just return the existing set.
            return processExistingResource( hResOut, pExistingSet, nFlags );

        } // End if can instance

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview modes.
        if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
            bInternalResource = true;

        // A new resource is required. Generate a new destination reference identifier.
        cgUInt32 nDestRefId = 0;
        if ( !pWorld )
            nDestRefId = cgReferenceManager::generateInternalRefId();
        else
            nDestRefId = pWorld->generateRefId( bInternalResource );

        // Clone the animation set out to the new reference identifier.
        cgAnimationSet * pNewSet = new cgAnimationSet( nDestRefId, pWorld, pExistingSet );
        cgString strResourceName = pNewSet->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        cgStreamType::Base StreamType = (pExistingSet->isInternalReference()) ? cgStreamType::Memory : cgStreamType::File;
        return processNewResource( hResOut, pNewSet, mAnimationSets, strResourceName, StreamType, nFlags, _debugSource );

    } // End if existing set
    else
    {
        // Must have access to a valid world database.
        if ( !pWorld )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load animation set resource '0x%x' because no valid world database was supplied.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // If the source reference identifier indicates that the source was internal,
        // but no set was found, this is an error situation.
        if ( nSourceRefId >= cgReferenceManager::InternalRefThreshold )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load animation set resource '0x%x' because the requested source reference identifier indicated an internal resource that was not resident for cloning.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // When cloning, we must always generate an internal resource in runtime
        // or sandbox preview modes.
        if ( (cgGetSandboxMode() != cgSandboxMode::Enabled) && (nFlags & cgResourceFlags::ForceNew) )
            bInternalResource = true;

        // If a new resource is required, make sure that we generate a new destination reference identifier.
        cgUInt32 nDestRefId = nSourceRefId;
        if ( nFlags & cgResourceFlags::ForceNew || bInternalResource )
            pWorld->generateRefId( bInternalResource );

        // Create the new set resource, instructing it to load from the specified
        // database data source when loadResource() is triggered.
        cgAnimationSet * pNewSet = new cgAnimationSet( nDestRefId, pWorld, nSourceRefId );
        cgString strResourceName = pNewSet->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        return processNewResource( hResOut, pNewSet, mAnimationSets, strResourceName, cgStreamType::File, nFlags, _debugSource );

    } // End if no existing set
}

//-----------------------------------------------------------------------------
//  Name : addAnimationSet ( )
/// <summary>
/// Simply add an animation set to the resource database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::addAnimationSet( cgAnimationSetHandle * hResOut, cgAnimationSet * pSet, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Validate requirements
    if ( pSet == CG_NULL )
        return false;

    // Attempt to find the animation set if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgAnimationSet * pExistingSet = (cgAnimationSet*)findAnimationSet( strResourceName );
        if ( pExistingSet )
            return processExistingResource( hResOut, pExistingSet, nFlags );

    } // End if resource name provided

    // Process the new resource
    return processNewResource( hResOut, pSet, mAnimationSets, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSet ()
/// <summary>
/// Retrieve a resource handle to an underlying animation set based on
/// the name specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getAnimationSet( cgAnimationSetHandle * hResOut, const cgString & strResourceName )
{
    cgResource * pSet = findAnimationSet( strResourceName );
    if ( pSet == CG_NULL ) return false;

    // Return the relevant set
    if ( hResOut != CG_NULL )
        *hResOut = cgAnimationSetHandle( (cgAnimationSet*)pSet, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMesh ()
/// <summary>
/// Retrieve a resource handle to an underlying animation set based on the
/// reference identifier specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getAnimationSet( cgAnimationSetHandle * hResOut, cgUInt32 nReferenceId )
{
    cgResource * pSet = findAnimationSet( nReferenceId );
    if ( pSet == CG_NULL ) return false;

    // Return the relevant set
    if ( hResOut != CG_NULL )
        *hResOut = cgAnimationSetHandle( (cgAnimationSet*)pSet, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findAnimationSet ()
/// <summary>
/// Given the specified resource name, find the resource object for any
/// matching animation set found.
/// Note : A value of CG_NULL is returned if the resource could not be found or an 
/// error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findAnimationSet( const cgString & strResourceName ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Loop through and see if this animation set already exists.
    for ( itItem = mAnimationSets.begin(); itItem != mAnimationSets.end(); ++itItem )
    {
        cgAnimationSet * pResource = (cgAnimationSet*)(*itItem);
        if ( pResource == CG_NULL ) continue;

        // Found one with matching details?
        if ( strResourceName == pResource->getResourceName() ) 
            return pResource;

    } // Next Animation Set

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findAnimationSet ()
/// <summary>
/// Given the specified reference identifier, find the resource object for any
/// matching animation set found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occurred.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findAnimationSet( cgUInt32 nReferenceId ) const
{
    // Valid reference identifier?
    if ( nReferenceId == 0 )
        return CG_NULL;

    // Use reference manager to rapidly retrieve the associated reference target.
    cgReference * pReference = cgReferenceManager::getReference( nReferenceId );
    if ( pReference == CG_NULL || pReference->queryReferenceType( RTID_AnimationSetResource ) == false )
        return CG_NULL;

    // Must be associated with this resource manager.
    cgResource * pResource = (cgResource*)pReference;
    if ( pResource->getManager() != this )
        return CG_NULL;

    // Resource found
    return pResource;
}

//-----------------------------------------------------------------------------
//  Name : createRenderTarget ()
/// <summary>
/// Create a new managed render target based on the description
/// provided.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createRenderTarget( cgRenderTargetHandle * hResOut, const cgImageInfo & Description, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the target if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgRenderTarget * pExistingTarget = (cgRenderTarget*)findRenderTarget( strResourceName );
        if ( pExistingTarget )
            return processExistingResource( hResOut, pExistingTarget, nFlags );

    } // End if resource name provided

    // No existing target found, so lets build a new resource.
    cgRenderTarget * pNewTarget = cgRenderTarget::createInstance( cgReferenceManager::generateInternalRefId(), Description );

    // Process the new resource
    if ( !processNewResource( hResOut, pNewTarget, mRenderTargets, strResourceName, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the lookup table if necessary
    if ( !strResourceName.empty() )
        mRenderTargetLUT[ strResourceName ] = pNewTarget;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getRenderTarget ()
/// <summary>
/// Retrieve a resource handle to an underlying render target based on the
/// name specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getRenderTarget( cgRenderTargetHandle * hResOut, const cgString & strResourceName )
{
    cgResource * pTarget = findRenderTarget( strResourceName );
    if ( pTarget == CG_NULL ) return false;

    // Return the relevant target
    if ( hResOut != CG_NULL )
        *hResOut = cgRenderTargetHandle( (cgRenderTarget*)pTarget, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findRenderTarget ()
/// <summary>
/// Given the specified resource name, find the resource object for any
/// matching render target found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findRenderTarget( const cgString & strResourceName ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Use the lookup table to find a matching target with this name.
    NamedResourceMap::const_iterator itResource = mRenderTargetLUT.find( strResourceName );
    if ( itResource != mRenderTargetLUT.end() )
        return itResource->second;

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createDepthStencilTarget ()
/// <summary>
/// Create a new managed depth stencil target based on the description
/// provided.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createDepthStencilTarget( cgDepthStencilTargetHandle * hResOut, const cgImageInfo & Description, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the target if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgDepthStencilTarget * pExistingTarget = (cgDepthStencilTarget*)findDepthStencilTarget( strResourceName );
        if ( pExistingTarget )
            return processExistingResource( hResOut, pExistingTarget, nFlags );

    } // End if resource name provided

    // No existing target found, so lets build a new resource.
    cgDepthStencilTarget * pNewTarget = cgDepthStencilTarget::CreateInstance( cgReferenceManager::generateInternalRefId(), Description );

    // Process the new resource
    return processNewResource( hResOut, pNewTarget, mDepthStencilTargets, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilTarget ()
/// <summary>
/// Retrieve a resource handle to an underlying depth stencil target based on
/// the name specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getDepthStencilTarget( cgDepthStencilTargetHandle * hResOut, const cgString & strResourceName )
{
    cgResource * pTarget = findDepthStencilTarget( strResourceName );
    if ( pTarget == CG_NULL ) return false;

    // Return the relevant target
    if ( hResOut != CG_NULL )
        *hResOut = cgDepthStencilTargetHandle( (cgDepthStencilTarget*)pTarget, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findDepthStencilTarget ()
/// <summary>
/// Given the specified resource name, find the resource object for any
/// matching depth stencil target found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findDepthStencilTarget( const cgString & strResourceName ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Loop through and see if this target already exists.
    for ( itItem = mDepthStencilTargets.begin(); itItem != mDepthStencilTargets.end(); ++itItem )
    {
        cgDepthStencilTarget * pResource = (cgDepthStencilTarget*)(*itItem);
        if ( pResource == CG_NULL ) continue;

        // Found one with matching details?
        if ( strResourceName == pResource->getResourceName() ) 
            return pResource;

    } // Next DepthStencil Target

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : makeUniqueMaterialName ( )
/// <summary>
/// Given the specified suggested name, determine if it is unique. If it is
/// not unique, append a number to the end such that it will become unique.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgResourceManager::makeUniqueMaterialName( const cgString & strName )
{
    // Pass through to main overload with no postfix initially
    return makeUniqueMaterialName( strName, 0xFFFFFFFF );
}

//-----------------------------------------------------------------------------
// Name : makeUniqueMaterialName ( )
/// <summary>
/// Given the specified suggested name, determine if it is unique. If it is
/// not unique, append a number to the end such that it will become unique.
/// Note: Specify 0xFFFFFFFF to nPostFix to initially search for name with no
/// appended number.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgResourceManager::makeUniqueMaterialName( const cgString & strName, cgUInt32 nPostFix )
{
    // ToDo: Should this be based on data in the database rather than active materials?
    // Materials may not even be resident at this stage.

    cgUInt32 nCounter       = 1;
    cgString strUniqueName  = strName;
    cgString strUniqueNameL;

    // Was a postfix value supplied?
    if ( nPostFix < 0xFFFFFFFF )
    {
         strUniqueName = cgString::format( _T("%s%02i"), strUniqueName.c_str(), nPostFix );
         nCounter      = nPostFix;
    
    } // End if 
    strUniqueNameL = cgString::toLower( strUniqueName );
    
    // Keep searching until we find a unique name
    NameUsageMap::iterator itName;
    while( (itName = mMaterialNames.find(strUniqueNameL)) != mMaterialNames.end() )
    {
        // Name matches. Try a new name and search again
        strUniqueName  = cgString::format( _T("%s%02i"), strName.c_str(), nCounter );
        strUniqueNameL = cgString::toLower( strUniqueName );
        nCounter++;

    } // Next name

    // Return the unique version of the name
    return strUniqueName;
}

//-----------------------------------------------------------------------------
// Name : getMaterialNameUsage ( )
/// <summary>
/// Retrieve the map that contains a list of all the material names currently
/// in use at this time.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager::NameUsageMap & cgResourceManager::getMaterialNameUsage( )
{
    return mMaterialNames;
}

//-----------------------------------------------------------------------------
//  Name : addMaterial ()
/// <summary>
/// Add an existing material to the manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::addMaterial( cgMaterialHandle * hResOut, cgMaterial * pMaterial, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Validate requirements
    if ( pMaterial == CG_NULL )
        return false;

    // Process the new material resource
    return processNewResource( hResOut, pMaterial, mMaterials, pMaterial->getName(), cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createMaterial ()
/// <summary>
/// Create a material of the specified type and add it to the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createMaterial( cgMaterialHandle * hResOut, cgMaterialType::Base Type, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    return createMaterial( hResOut, true, CG_NULL, Type, nFlags, _debugSource );
}
bool cgResourceManager::createMaterial( cgMaterialHandle * hResOut, bool bInternalResource, cgWorld * pWorld, cgMaterialType::Base Type, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Validate requirements
    if ( bInternalResource == false && pWorld == CG_NULL )
        return false;

    // All created materials are internal if sandbox mode is not enabled.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        bInternalResource = true;

    // Generate a new reference identifier for this resource.
    cgUInt32 nResourceRefId = 0;
    if ( bInternalResource == false )
        nResourceRefId = pWorld->generateRefId( false );
    else
        nResourceRefId = cgReferenceManager::generateInternalRefId();
    
    // Create the new material resource, instructing it to load from the specified
    // database data source when loadResource() is triggered.
    cgMaterial * pMaterial = CG_NULL;
    switch ( Type )
    {
        case cgMaterialType::Standard:
            pMaterial = new cgStandardMaterial( nResourceRefId, pWorld );
            break;

        case cgMaterialType::LandscapeLayer:
            pMaterial = new cgLandscapeLayerMaterial( nResourceRefId, pWorld );
            break;

    } // End switch type

    // Valid material?
    if ( pMaterial == CG_NULL )
        return false;

    // Process the new material resource
    return processNewResource( hResOut, pMaterial, mMaterials, cgString::Empty, cgStreamType::Memory, nFlags, _debugSource );

}

//-----------------------------------------------------------------------------
//  Name : buildMaterialBatchTable ()
/// <summary>
/// Builds a lookup table that can be used to determine matches against any
/// existing material based on its data alone.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::buildMaterialBatchTable( MaterialMap & Table )
{
    // ToDo: Arguably, this should be based on data in the database rather than
    // simply the active materials?

    Table.clear();
    ResourceItemList::const_iterator itMaterial;
    const ResourceItemList & Materials = mMaterials;
    for ( itMaterial = Materials.begin(); itMaterial != Materials.end(); ++itMaterial )
    {
        // Add the material for batcing.
        cgMaterial * pMaterial = (cgMaterial*)(*itMaterial);
        Table[ MaterialKey( pMaterial ) ] = pMaterial;

    } // Next Material

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadMaterial ( )
/// <summary>
/// Load the material with matching source reference identifier from the
/// specified world database. If the 'ForceNew' flag is specified and the
/// destination reference identifier is zero, then a new internal material with 
/// a unique identifier will be automatically generated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadMaterial( cgMaterialHandle * hResOut, cgWorld * pWorld, cgMaterialType::Base Type, cgUInt32 nSourceRefId, bool bInternalResource, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // ToDo: 9999 - Add transaction for rollback on failure (in the cloning case)?

    // Validate requirements
    if ( !nSourceRefId )
        return false;

    // Force internal if no world was supplied.
    if ( !pWorld )
        bInternalResource = true;

    // Attempt to find an existing resident material with the source reference identifier.
    cgMaterial * pExistingMaterial = (cgMaterial*)findMaterial( nSourceRefId );
    if ( pExistingMaterial )
    {
        // If a source material was found we can potentially just return the existing resource 
        // depending on the supplied options. Specifically, if the user did not want a new 
        // resource, and both resources are internal *or* both are serialized, we can avoid
        // a clone operation.
        if ( !(nFlags & cgResourceFlags::ForceNew) &&
             bInternalResource == pExistingMaterial->isInternalReference() )
        {
            // No clone was required, we can just return the existing material.
            return processExistingResource( hResOut, pExistingMaterial, nFlags );

        } // End if can instance

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview modes.
        if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
            bInternalResource = true;

        // A new resource is required. Generate a new destination reference identifier.
        cgUInt32 nDestRefId = 0;
        if ( !pWorld )
            nDestRefId = cgReferenceManager::generateInternalRefId();
        else
            nDestRefId = pWorld->generateRefId( bInternalResource );

        // Clone the material out to the new reference identifier.
        cgMaterial * pNewMaterial = CG_NULL;
        if ( pExistingMaterial->queryReferenceType( RTID_StandardMaterial ) )
            pNewMaterial = new cgStandardMaterial( nDestRefId, pWorld, (cgStandardMaterial*)pExistingMaterial );
        else if ( pExistingMaterial->queryReferenceType( RTID_LandscapeLayerMaterial ) )
            pNewMaterial = new cgLandscapeLayerMaterial( nDestRefId, pWorld, (cgLandscapeLayerMaterial*)pExistingMaterial );
        cgString strResourceName = pNewMaterial->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        cgStreamType::Base StreamType = (pExistingMaterial->isInternalReference()) ? cgStreamType::Memory : cgStreamType::File;
        return processNewResource( hResOut, pNewMaterial, mMaterials, strResourceName, StreamType, nFlags, _debugSource );

    } // End if existing material
    else
    {
        // Must have access to a valid world database.
        if ( !pWorld )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load material resource '0x%x' because no valid world database was supplied.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // If the source reference identifier indicates that the source was internal,
        // but no material was found, this is an error situation.
        if ( nSourceRefId >= cgReferenceManager::InternalRefThreshold )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load material resource '0x%x' because the requested source reference identifier indicated an internal resource that was not resident for cloning.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview modes.
        if ( (cgGetSandboxMode() != cgSandboxMode::Enabled) && (nFlags & cgResourceFlags::ForceNew) )
            bInternalResource = true;

        // If a new resource is required, make sure that we generate a new destination reference identifier.
        cgUInt32 nDestRefId = nSourceRefId;
        if ( nFlags & cgResourceFlags::ForceNew || bInternalResource )
            pWorld->generateRefId( bInternalResource );

        // Create the material mesh resource, instructing it to load from the specified
        // database data source when loadResource() is triggered.
        cgMaterial * pNewMaterial = CG_NULL;
        switch ( Type )
        {
            case cgMaterialType::Standard:
                pNewMaterial = new cgStandardMaterial( nDestRefId, pWorld, nSourceRefId );
                break;

            case cgMaterialType::LandscapeLayer:
                pNewMaterial = new cgLandscapeLayerMaterial( nDestRefId, pWorld, nSourceRefId );
                break;

        } // End switch type
        cgString strResourceName = pNewMaterial->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        return processNewResource( hResOut, pNewMaterial, mMaterials, strResourceName, cgStreamType::File, nFlags, _debugSource );

    } // End if no existing material
}

//-----------------------------------------------------------------------------
//  Name : cloneMaterial ()
/// <summary>
/// Create a new material, cloned from that specified, and add it to the 
/// resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::cloneMaterial( cgMaterialHandle * hResOut, cgWorld * pWorld, bool bInternalResource, cgMaterial * pInit, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Validate requirements
    if ( !pInit || (!bInternalResource && !pWorld) )
        return false;

    // All created materials are internal if sandbox mode is not enabled.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        bInternalResource = true;

    // Generate a new reference identifier for this resource.
    cgUInt32 nDestRefId = 0;
    if ( bInternalResource)
        nDestRefId = cgReferenceManager::generateInternalRefId();
    else
        nDestRefId = pWorld->generateRefId( false );
        
    // Create the new material resource, instructing it to clone from the specified source.
    cgMaterial * pMaterial = CG_NULL;
    if ( pInit->queryReferenceType( RTID_StandardMaterial ) )
        pMaterial = new cgStandardMaterial( nDestRefId, pWorld, (cgStandardMaterial*)pInit );
    else if ( pInit->queryReferenceType( RTID_LandscapeLayerMaterial ) )
        pMaterial = new cgLandscapeLayerMaterial( nDestRefId, pWorld, (cgLandscapeLayerMaterial*)pInit );

    // Valid material?
    if ( !pMaterial )
        return false;

    // Process the new material resource
    return processNewResource( hResOut, pMaterial, mMaterials, cgString::Empty, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : findMaterial ()
/// <summary>
/// Given the specified reference identifier, find the resource object for any
/// matching material found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occurred.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findMaterial( cgUInt32 nReferenceId ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid reference identifier?
    if ( nReferenceId == 0 )
        return CG_NULL;

    // Use reference manager to rapidly retrieve the associated reference target.
    cgReference * pReference = cgReferenceManager::getReference( nReferenceId );
    if ( pReference == CG_NULL || pReference->queryReferenceType( RTID_MaterialResource ) == false )
        return CG_NULL;

    // Must be associated with this resource manager.
    cgResource * pResource = (cgResource*)pReference;
    if ( pResource->getManager() != this )
        return CG_NULL;

    // Resource found
    return pResource;
}

//-----------------------------------------------------------------------------
//  Name : getMaterialFromId ()
/// <summary>
/// Retrieve a resource handle to an underlying material based on the
/// unique numeric identifier specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getMaterialFromId( cgMaterialHandle * hResOut, cgUInt32 referenceId )
{
    // Find the material.
    cgResource * pResource = findMaterial( referenceId );
    if ( pResource == CG_NULL ) return false;
    
    // Return the relevant material
    if ( hResOut != CG_NULL )
        *hResOut = cgMaterialHandle( (cgMaterial*)pResource, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Material found
    return true;
}

/*//-----------------------------------------------------------------------------
//  Name : loadMesh ()
/// <summary>
/// Given the specified mesh details, load and return a handle to a
/// mesh resource.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadMesh( ResourceHandle * hResOut, const cgInputStream & Stream, bool bFinalizeMesh = true, cgUInt32 nFlags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) )
{
    // Pass through to primary method.
    return loadMesh( hResOut, Stream, _T(""), bFinalizeMesh, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : loadMesh ()
/// <summary>
/// Given the specified mesh details, load and return a handle to a
/// mesh resource.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadMesh( ResourceHandle * hResOut, const cgInputStream & Stream, const cgString & strMeshReference, bool bFinalizeMesh = true, cgUInt32 nFlags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) )
{
    cgMesh     * pNewMesh  = CG_NULL;
    cgResource * pResource = CG_NULL;

    // Compute name for mesh based on source.
    cgString strMeshName = Stream.getName();
    if ( strMeshReference.empty() == false )
        strMeshName += _T("::") + strMeshReference;

    // Attempt to find the mesh if it already exists and is not unique.
    if ( (nFlags & cgResourceFlags::ForceNew) == 0 || (nFlags & cgResourceFlags::ForceUnique) == 0 )
    {
        pResource = findMesh( strMeshName );
        if ( pResource != CG_NULL && pResource->IsUnique() == false )
            return processExistingResource( hResOut, pResource, nFlags );
    
    } // End if !ForceUnique

    // Validate requirements
    if ( Stream.getType() == cgStreamType::None )
        return false;

    // No mesh found, so lets build a new mesh resource.
    pNewMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), Stream, strMeshReference, bFinalizeMesh );

    // Process the new resource
    return processNewResource( hResOut, pNewMesh, mMeshes, strMeshName, Stream.getType(), nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : loadMesh ()
/// <summary>
/// Given the specified mesh details, load and return a handle to a
/// mesh resource.
/// Note : A value of false is returned if the resource could not be
/// found, or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadMesh( ResourceHandle * hResOut, cgSceneLoader * pLoader, const cgXMLNode & MeshData, bool bFinalizeMesh = true, cgUInt32 nFlags = 0, const cgString & strResourceName = _T(""), const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0) )
{
    cgMesh     * pNewMesh  = CG_NULL;
    cgResource * pResource = CG_NULL;

    if ( strResourceName.empty() == false && 
        (nFlags & cgResourceFlags::ForceNew) == 0 && 
        (nFlags & cgResourceFlags::ForceUnique) == 0 )
    {
        // Attempt to find the mesh if it already exists
        pResource = findMesh( strResourceName );
        if ( pResource != CG_NULL && pResource->IsUnique() == false ) 
            return processExistingResource( hResOut, pResource, nFlags );

    } // End if resource name provided

    // No mesh found, so lets build a new mesh resource.
    pNewMesh = new cgMesh( cgReferenceManager::generateInternalRefId(), pLoader, MeshData, bFinalizeMesh );

    // Process the new resource
    return processNewResource( hResOut, pNewMesh, mMeshes, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}*/

//-----------------------------------------------------------------------------
//  Name : addMesh ( )
/// <summary>
/// Simply add a mesh to the resource database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::addMesh( cgMeshHandle * hResOut, cgMesh * pMesh, cgUInt32 nFlags /* = 0 */, const cgString & strResourceName /* = _T("") */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // ToDo: 9999 - Add transaction for rollback on failure.

    // Validate requirements
    if ( !pMesh )
        return false;

    // Attempt to find the mesh if it already exists
    if ( !strResourceName.empty() && !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgMesh * pExistingMesh = (cgMesh*)findMesh( strResourceName );
        if ( pExistingMesh ) 
            return processExistingResource( hResOut, pExistingMesh, nFlags );

    } // End if resource name provided

    // Process the new resource
    return processNewResource( hResOut, pMesh, mMeshes, strResourceName, cgStreamType::Memory, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : loadMesh ( )
/// <summary>
/// Load the mesh with matching source reference identifier from the specified
/// world database. If the 'ForceNew' flag is specified then a new copy of the
/// mesh with a new unique reference identifier will be automatically generated.
/// Otherwise the resulting mesh will automatically wrap or clone the serialized
/// data depending on the state of the 'bInternalResource' parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadMesh( cgMeshHandle * hResOut, cgWorld * pWorld, cgUInt32 nSourceRefId, bool bInternalResource, bool bFinalizeMesh /* = true */, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // ToDo: 9999 - Add transaction for rollback on failure (in the cloning case)?

    // Validate requirements
    if ( !nSourceRefId )
        return false;

    // Force internal if no world was supplied.
    if ( !pWorld )
        bInternalResource = true;

    // Attempt to find an existing resident mesh with the source reference identifier.
    cgMesh * pExistingMesh = (cgMesh*)findMesh( nSourceRefId );
    if ( pExistingMesh )
    {
        // If a source mesh was found we can potentially just return the existing resource 
        // depending on the supplied options. Specifically, if the user did not want a new 
        // resource, and both resources are internal *or* both are serialized, we can avoid
        // a clone operation.
        if ( !(nFlags & cgResourceFlags::ForceNew) &&
             bInternalResource == pExistingMesh->isInternalReference() )
        {
            // No clone was required, we can just return the existing mesh.
            return processExistingResource( hResOut, pExistingMesh, nFlags );

        } // End if can instance

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview modes.
        if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
            bInternalResource = true;

        // A new resource is required. Generate a new destination reference identifier.
        cgUInt32 nDestRefId = 0;
        if ( !pWorld )
            nDestRefId = cgReferenceManager::generateInternalRefId();
        else
            nDestRefId = pWorld->generateRefId( bInternalResource );

        // Clone the mesh out to the new reference identifier.
        cgMesh * pNewMesh = new cgMesh( nDestRefId, pWorld, pExistingMesh );
        cgString strResourceName = pNewMesh->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        cgStreamType::Base StreamType = (pExistingMesh->isInternalReference()) ? cgStreamType::Memory : cgStreamType::File;
        return processNewResource( hResOut, pNewMesh, mMeshes, strResourceName, StreamType, nFlags, _debugSource );

    } // End if existing mesh
    else
    {
        // Must have access to a valid world database.
        if ( !pWorld )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load mesh resource '0x%x' because no valid world database was supplied.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // If the source reference identifier indicates that the source was internal,
        // but no mesh was found, this is an error situation.
        if ( nSourceRefId >= cgReferenceManager::InternalRefThreshold )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load mesh resource '0x%x' because the requested source reference identifier indicated an internal resource that was not resident for cloning.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // When cloning, we must always generate an internal resource in runtime 
        // or sand box preview modes.
        if ( (cgGetSandboxMode() != cgSandboxMode::Enabled) && (nFlags & cgResourceFlags::ForceNew) )
            bInternalResource = true;

        // If a new resource is required, make sure that we generate a new destination reference identifier.
        cgUInt32 nDestRefId = nSourceRefId;
        if ( nFlags & cgResourceFlags::ForceNew || bInternalResource )
            pWorld->generateRefId( bInternalResource );

        // Create the new mesh resource, instructing it to load from the specified
        // database data source when loadResource() is triggered.
        cgMesh * pNewMesh = new cgMesh( nDestRefId, pWorld, nSourceRefId, bFinalizeMesh );
        cgString strResourceName = pNewMesh->getDatabaseTable() + cgString::format( _T("(0x%x)"), nDestRefId );

        // Process the new resource
        return processNewResource( hResOut, pNewMesh, mMeshes, strResourceName, cgStreamType::File, nFlags, _debugSource );

    } // End if no existing mesh
}

//-----------------------------------------------------------------------------
//  Name : getMesh ()
/// <summary>
/// Retrieve a resource handle to an underlying mesh based on the name
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getMesh( cgMeshHandle * hResOut, const cgString & strResourceName )
{
    cgResource * pMesh = findMesh( strResourceName );
    if ( !pMesh )
        return false;

    // Return the relevant mesh
    if ( hResOut )
        *hResOut = cgMeshHandle( (cgMesh*)pMesh, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMesh ()
/// <summary>
/// Retrieve a resource handle to an underlying mesh based on the reference
/// identifier specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getMesh( cgMeshHandle * hResOut, cgUInt32 nReferenceId )
{
    cgResource * pMesh = findMesh( nReferenceId );
    if ( pMesh == CG_NULL ) return false;

    // Return the relevant mesh
    if ( hResOut != CG_NULL )
        *hResOut = cgMeshHandle( (cgMesh*)pMesh, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Target found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findMesh ()
/// <summary>
/// Given the specified resource name, find the resource object for any
/// matching mesh found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occured.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findMesh( const cgString & strResourceName ) const
{
    ResourceItemList::const_iterator itItem;

    // Valid resource name?
    if ( strResourceName.empty() == true )
        return CG_NULL;

    // Loop through and see if this mesh already exists.
    for ( itItem = mMeshes.begin(); itItem != mMeshes.end(); ++itItem )
    {
        cgMesh * pResource = (cgMesh*)(*itItem);
        if ( !pResource ) continue;

        // Found one with matching details?
        if ( strResourceName == pResource->getResourceName() ) 
            return pResource;

    } // Next Mesh

    // Nothing found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findMesh ()
/// <summary>
/// Given the specified reference identifier, find the resource object for any
/// matching mesh found.
/// Note : A value of CG_NULL  is returned if the resource could not be
/// found or an error occurred.
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findMesh( cgUInt32 nReferenceId ) const
{
    // Valid reference identifier?
    if ( nReferenceId == 0 )
        return CG_NULL;

    // Use reference manager to rapidly retrieve the associated reference target.
    cgReference * pReference = cgReferenceManager::getReference( nReferenceId );
    if ( pReference == CG_NULL || pReference->queryReferenceType( RTID_MeshResource ) == false )
        return CG_NULL;

    // Must be associated with this resource manager.
    cgResource * pResource = (cgResource*)pReference;
    if ( pResource->getManager() != this )
        return CG_NULL;

    // Resource found
    return pResource;
}

//-----------------------------------------------------------------------------
//  Name : createVertexShader ()
/// <summary>
/// Compile the provided shader code into a compatible system vertex shader
/// or return an existing shader with matching identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createVertexShader( cgVertexShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, const cgString & strSource, const cgString & strEntryPoint, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgVertexShader * pExistingShader = (cgVertexShader*)findVertexShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // No shader found, so lets build a new vertex shader resource.
    cgVertexShader * pNewShader = cgVertexShader::createInstance( cgReferenceManager::generateInternalRefId(), strSource, strEntryPoint, pIdentifier );

    // Process the new resource
    if ( !processNewResource( hResOut, pNewShader, mVertexShaders, pIdentifier->shaderIdentifier, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the vertex shader lookup table
    mVertexShaderLUT[ ShaderKey(&pNewShader->getIdentifier()) ] = pNewShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadCompiledVertexShader ()
/// <summary>
/// Retrieve an existing resident vertex shader permutation that has a matching
/// shader identifier to that supplied. If none exists, this method will
/// attempt to load the specified compiled vertex shader byte code.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadCompiledVertexShader( cgVertexShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, const cgInputStream & Stream,  cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgVertexShader * pExistingShader = (cgVertexShader*)findVertexShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // Must be valid stream.
    if ( Stream.getType() == cgStreamType::None )
        return false;

    // No shader found, so lets build a new vertex shader resource.
    cgVertexShader * pNewShader = cgVertexShader::createInstance( cgReferenceManager::generateInternalRefId(), Stream, true );

    // Process the new resource
    if ( !processNewResource( hResOut, pNewShader, mVertexShaders, pIdentifier->shaderIdentifier, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the vertex shader lookup table
    mVertexShaderLUT[ ShaderKey(&pNewShader->getIdentifier()) ] = pNewShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadVertexShader ()
/// <summary>
/// Retrieve an existing resident vertex shader permutation that has a matching
/// shader identifier to that supplied. If none is currently resident, this 
/// method will then attempt to load the shader from the on disk shader 
/// permutation cache. If it does not exist in the cache at this stage, this
/// method will fail and a new permutation should be requested by the caller.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadVertexShader( cgVertexShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgVertexShader * pExistingShader = (cgVertexShader*)findVertexShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // Shader cache is not available when debugging shaders.
    if ( mRenderDriver->getConfig().debugVShader )
        return false;

    // No existing /resident/ shader was found, but let's see if a version
    // exists in the on-disk shader cache. First build the unique hashed file name 
    // for this shader permutation based on the source files, the parameter data and 
    // the shader identifier / name.
    cgString strCacheFile = _T("sys://Cache/Shaders/") + pIdentifier->generateHashName();
    cgToDo( "DX11", "Clean this up to detect shader model rather than hardcoding based on DX version." );
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.renderAPI )
    {
        case cgRenderAPI::Null:
        default:
            strCacheFile.clear();
            break;

#if defined( CGE_DX11_RENDER_SUPPORT )
        case cgRenderAPI::DirectX11:
            strCacheFile.append( _T(".vs4") );
            break;

#endif // CGE_DX11_RENDER_SUPPORT

#if defined( CGE_DX9_RENDER_SUPPORT )
        case cgRenderAPI::DirectX9:
            strCacheFile.append( _T(".vs3") );
            break;

#endif // CGE_DX9_RENDER_SUPPORT

    } // End switch renderAPI

    // Attempt to load the on-disk shader permutation (will fail if not found).
    return loadCompiledVertexShader( hResOut, pIdentifier, strCacheFile, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createPixelShader ()
/// <summary>
/// Compile the provided shader code into a compatible system pixel shader
/// or return an existing shader with matching identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createPixelShader( cgPixelShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, const cgString & strSource, const cgString & strEntryPoint, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgPixelShader * pExistingShader = (cgPixelShader*)findPixelShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // No shader found, so lets build a new pixel shader resource.
    cgPixelShader * pNewShader = cgPixelShader::createInstance( cgReferenceManager::generateInternalRefId(), strSource, strEntryPoint, pIdentifier );

    // Process the new resource
    if ( !processNewResource( hResOut, pNewShader, mPixelShaders, pIdentifier->shaderIdentifier, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the pixel shader lookup table
    mPixelShaderLUT[ ShaderKey(&pNewShader->getIdentifier()) ] = pNewShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadCompiledPixelShader ()
/// <summary>
/// Retrieve an existing resident pixel shader permutation that has a matching
/// shader identifier to that supplied. If none exists, this method will
/// attempt to load the specified compiled pixel shader byte code.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadCompiledPixelShader( cgPixelShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, const cgInputStream & Stream,  cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgPixelShader * pExistingShader = (cgPixelShader*)findPixelShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // Must be valid stream.
    if ( Stream.getType() == cgStreamType::None )
        return false;

    // No shader found, so lets build a new pixel shader resource.
    cgPixelShader * pNewShader = cgPixelShader::createInstance( cgReferenceManager::generateInternalRefId(), Stream, true );

    // Process the new resource
    if ( !processNewResource( hResOut, pNewShader, mPixelShaders, pIdentifier->shaderIdentifier, cgStreamType::Memory, nFlags, _debugSource ) )
        return false;

    // Add to the pixel shader lookup table
    mPixelShaderLUT[ ShaderKey(&pNewShader->getIdentifier()) ] = pNewShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadPixelShader ()
/// <summary>
/// Retrieve an existing resident pixel shader permutation that has a matching
/// shader identifier to that supplied. If none is currently resident, this 
/// method will then attempt to load the shader from the on disk shader 
/// permutation cache. If it does not exist in the cache at this stage, this
/// method will fail and a new permutation should be requested by the caller.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::loadPixelShader( cgPixelShaderHandle * hResOut, const cgShaderIdentifier * pIdentifier, cgUInt32 nFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Attempt to find the shader if it already exists
    if ( !(nFlags & cgResourceFlags::ForceNew) )
    {
        cgPixelShader * pExistingShader = (cgPixelShader*)findPixelShader( pIdentifier );
        if ( pExistingShader )
            return processExistingResource( hResOut, pExistingShader, nFlags );
    
    } // End if !ForceNew

    // Shader cache is not available when debugging shaders.
    if ( mRenderDriver->getConfig().debugPShader )
        return false;

    // No existing /resident/ shader was found, but let's see if a version
    // exists in the on-disk shader cache. First build the unique hashed file name 
    // for this shader permutation based on the source files, the parameter data and 
    // the shader identifier / name.
    cgString strCacheFile = _T("sys://Cache/Shaders/") + pIdentifier->generateHashName();
    cgToDo( "DX11", "Clean this up to detect shader model rather than hardcoding based on DX version." );
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.renderAPI )
    {
        case cgRenderAPI::Null:
        default:
            strCacheFile.clear();
            break;

#if defined( CGE_DX11_RENDER_SUPPORT )
        case cgRenderAPI::DirectX11:
            strCacheFile.append( _T(".ps4") );
            break;

#endif // CGE_DX11_RENDER_SUPPORT

#if defined( CGE_DX9_RENDER_SUPPORT )
        case cgRenderAPI::DirectX9:
            strCacheFile.append( _T(".ps3") );
            break;

#endif // CGE_DX9_RENDER_SUPPORT

    } // End switch renderAPI
    
    // Attempt to load the on-disk shader permutation (will fail if not found).
    return loadCompiledPixelShader( hResOut, pIdentifier, strCacheFile, nFlags, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Find and return any existing vertex shader that has a matching identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getVertexShader( cgVertexShaderHandle * hResOut, const cgShaderIdentifier * pCacheIdentifier )
{
    cgResource * pShader = findVertexShader( pCacheIdentifier );
    if ( !pShader )
        return false;

    // Return the relevant shader
    if ( hResOut )
        *hResOut = cgVertexShaderHandle( (cgVertexShader*)pShader, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Shader found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Find and return any existing pixel shader that has a matching identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::getPixelShader( cgPixelShaderHandle * hResOut, const cgShaderIdentifier * pCacheIdentifier )
{
    cgResource * pShader = findPixelShader( pCacheIdentifier );
    if ( !pShader )
        return false;

    // Return the relevant shader
    if ( hResOut )
        *hResOut = cgPixelShaderHandle( (cgPixelShader*)pShader, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Shader found
    return true;
}

//-----------------------------------------------------------------------------
//  Name : findVertexShader () (Private)
/// <summary>
/// Looks for a matching vertex shader in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findVertexShader( const cgShaderIdentifier * pCacheIdentifier )
{
    ShaderMap::iterator itShader = mVertexShaderLUT.find( ShaderKey( pCacheIdentifier ) );
    if ( itShader != mVertexShaderLUT.end() )
        return itShader->second;

    // No shader
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : findPixelShader () (Private)
/// <summary>
/// Looks for a matching pixel shader in the list and returns object pointer
/// </summary>
//-----------------------------------------------------------------------------
cgResource * cgResourceManager::findPixelShader( const cgShaderIdentifier * pCacheIdentifier )
{
    ShaderMap::iterator itShader = mPixelShaderLUT.find( ShaderKey( pCacheIdentifier ) );
    if ( itShader != mPixelShaderLUT.end() )
        return itShader->second;

    // No shader
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : processExistingResource ()
/// <summary>
/// Once it has been determined that an existing resource can be selected
/// this function is called to perform all the common post selection
/// tasks on that existing resource.
/// </summary>
//-----------------------------------------------------------------------------
template <class _HandleType, class _ResourceType>
bool cgResourceManager::processExistingResource( _HandleType * hResOut, _ResourceType * pExistingResource, cgUInt32 nFlags )
{
    // If the caller requested immediate loading, but the
    // resource was not yet loaded (i.e. deferred by some earlier call for 
    // this resource to be created)... load it now.
    if ( !(nFlags & cgResourceFlags::DeferredLoad) && !pExistingResource->isLoaded() )
    {
        // Load the resource
        if ( !pExistingResource->loadResource() )
            return false;

    } // End if not yet loaded

    // If caller requested a copy of the resource handle, return it.
    if ( hResOut )
        *hResOut = _HandleType( pExistingResource, hResOut->getDatabaseUpdate(), hResOut->getOwner() );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : processNewResource ()
/// <summary>
/// Once it has been determined that a new resource must be allocated
/// this function is called to perform all the common post allocation
/// tasks on that new resource.
/// </summary>
//-----------------------------------------------------------------------------
template <class _HandleType, class _ResourceType>
bool cgResourceManager::processNewResource( _HandleType * hResOut, _ResourceType * pNewResource, ResourceItemList & ResourceList, const cgString & strResourceName, cgStreamType::Base StreamType, cgUInt32 nFlags, const cgDebugSourceInfo & _debugSource )
{
    // Add ourselves as a reference to this resource.
    pNewResource->addReference( this, true );

    // Set management information
    ((cgResource*)pNewResource)->setManagementData( this, nFlags );
    ((cgResource*)pNewResource)->setResourceName( strResourceName );
    ((cgResource*)pNewResource)->setResourceSource( StreamType, _debugSource.source, _debugSource.line );

    // If we are not deferring the load operation, call it immediately.
    if ( !(nFlags & cgResourceFlags::DeferredLoad) )
    {
        // Load the resource!
        if ( pNewResource->loadResource() == false )
        {
            pNewResource->removeReference( this, true );
            return false;

        } // End if failed to load resource

    } // End if not deferring the load operation

    // Add to our resource list
    ResourceList.push_back( pNewResource );

    // Return the new handle
    if ( hResOut != CG_NULL )
        *hResOut = _HandleType( pNewResource, hResOut->getDatabaseUpdate(), hResOut->getOwner() );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : createVertexBuffer ()
/// <summary>
/// Pass through for the creation of a vertex buffer for management
/// within the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createVertexBuffer( cgVertexBufferHandle * hResOut, cgUInt32 Length, cgUInt32 Usage, cgVertexFormat * pFormat, cgMemoryPool::Base Pool, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Create new resource item for storage
    cgVertexBuffer * pNewVB = cgVertexBuffer::createInstance( cgReferenceManager::generateInternalRefId(), Length, Usage, pFormat, Pool );

    // Process the new resource
    cgString strName = cgString::format( _T("VertexBuffer(L:%i|U:%i|P:%i)"), Length, Usage, Pool );
    return processNewResource( hResOut, pNewVB, mVertexBuffers, strName, cgStreamType::Memory, 0, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createIndexBuffer ()
/// <summary>
/// Pass through for the creation of an index buffer for management
/// within the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createIndexBuffer( cgIndexBufferHandle * hResOut, cgUInt32 Length, cgUInt32 Usage, cgBufferFormat::Base Format, cgMemoryPool::Base Pool, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Create new resource item for storage
    cgIndexBuffer * pNewIB = cgIndexBuffer::createInstance( cgReferenceManager::generateInternalRefId(), Length, Usage, Format, Pool );

    // Process the new resource
    cgString strName = cgString::format( _T("IndexBuffer(L:%i|U:%i|P:%i)"), Length, Usage, Pool );
    return processNewResource( hResOut, pNewIB, mIndexBuffers, strName, cgStreamType::Memory, 0, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : lockVertexBuffer ()
/// <summary>
/// Lock a vertex buffer currently being managed by the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
LPVOID cgResourceManager::lockVertexBuffer( cgVertexBufferHandle & hResource, cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{   
    // Retrieve the vertex buffer resource object
    cgVertexBuffer * pBuffer = (cgVertexBuffer*)hResource.getResource( true );
    if ( !pBuffer )
        return CG_NULL;

    // Perform the lock
    return pBuffer->lock( OffsetToLock, SizeToLock, Flags );
}

//-----------------------------------------------------------------------------
//  Name : lockIndexBuffer ()
/// <summary>
/// Lock an index buffer currently being managed by the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
LPVOID cgResourceManager::lockIndexBuffer( cgIndexBufferHandle & hResource, cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{
    // Retrieve the index buffer resource object
    cgIndexBuffer * pBuffer = (cgIndexBuffer*)hResource.getResource( true );
    if ( !pBuffer )
        return CG_NULL;

    // Perform the lock
    return pBuffer->lock( OffsetToLock, SizeToLock, Flags );
}

//-----------------------------------------------------------------------------
//  Name : unlockVertexBuffer ()
/// <summary>
/// Unlock the specified vertex buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::unlockVertexBuffer( cgVertexBufferHandle & hResource )
{
    // Retrieve the vertex buffer resource object
    cgVertexBuffer * pBuffer = (cgVertexBuffer*)hResource.getResource(true);
    if ( pBuffer == CG_NULL )
        return;

    // Perform the unlock
    pBuffer->unlock();
}

//-----------------------------------------------------------------------------
//  Name : unlockIndexBuffer ()
/// <summary>
/// Unlock the specified index buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::unlockIndexBuffer( cgIndexBufferHandle & hResource )
{
    // Retrieve the index buffer resource object
    cgIndexBuffer * pBuffer = (cgIndexBuffer*)hResource.getResource(true);
    if ( pBuffer == CG_NULL )
        return;

    // Perform the unlock
    pBuffer->unlock();
}

//-----------------------------------------------------------------------------
//  Name : createConstantBuffer ()
/// <summary>
/// Pass through for the creation of a constant buffer for management
/// within the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createConstantBuffer( cgConstantBufferHandle * hResOut, const cgSurfaceShaderHandle & hShader, const cgString & strBufferName, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Create new resource item for storage
    cgConstantBuffer * pNewCB = cgConstantBuffer::createInstance( cgReferenceManager::generateInternalRefId(), hShader, strBufferName );

    // Process the new resource
    return processNewResource( hResOut, pNewCB, mConstantBuffers, strBufferName, cgStreamType::Memory, 0, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createConstantBuffer ()
/// <summary>
/// Pass through for the creation of a constant buffer for management
/// within the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createConstantBuffer( cgConstantBufferHandle * hResOut, const cgConstantBufferDesc & Desc, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Create new resource item for storage
    cgConstantBuffer * pNewCB = cgConstantBuffer::createInstance( cgReferenceManager::generateInternalRefId(), Desc );

    // Process the new resource
    return processNewResource( hResOut, pNewCB, mConstantBuffers, Desc.name, cgStreamType::Memory, 0, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : createConstantBuffer ()
/// <summary>
/// Pass through for the creation of a constant buffer for management
/// within the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::createConstantBuffer( cgConstantBufferHandle * hResOut, const cgConstantBufferDesc & Desc, const cgConstantTypeDesc::Array & aTypes, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0) */ )
{
    // Create new resource item for storage
    cgConstantBuffer * pNewCB = cgConstantBuffer::createInstance( cgReferenceManager::generateInternalRefId(), Desc, aTypes );

    // Process the new resource
    return processNewResource( hResOut, pNewCB, mConstantBuffers, Desc.name, cgStreamType::Memory, 0, _debugSource );
}

//-----------------------------------------------------------------------------
//  Name : lockConstantBuffer ()
/// <summary>
/// Lock a constant buffer currently being managed by the resource manager.
/// </summary>
//-----------------------------------------------------------------------------
LPVOID cgResourceManager::lockConstantBuffer( cgConstantBufferHandle & hResource, cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{   
    // Retrieve the constant buffer resource object
    cgConstantBuffer * pBuffer = (cgConstantBuffer*)hResource.getResource( true );
    if ( !pBuffer )
        return CG_NULL;

    // Perform the lock
    return pBuffer->lock( OffsetToLock, SizeToLock, Flags );
}

//-----------------------------------------------------------------------------
//  Name : unlockConstantBuffer ()
/// <summary>
/// Unlock the specified constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::unlockConstantBuffer( cgConstantBufferHandle & hResource )
{
    // Retrieve the vertex buffer resource object
    cgConstantBuffer * pBuffer = (cgConstantBuffer*)hResource.getResource(true);
    if ( !pBuffer )
        return;

    // Perform the unlock
    pBuffer->unlock();
}

//-----------------------------------------------------------------------------
//  Name : createSampler ()
/// <summary>
/// Create a new sampler mapping object for the specified sampler.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::createSampler( const cgString & strName )
{
    return createSampler( strName, cgSurfaceShaderHandle() );
}
cgSampler * cgResourceManager::createSampler( const cgString & strName, const cgSurfaceShaderHandle & hCustomShader )
{
    // Allocate the new sampler
    cgSampler * pSampler = new cgSampler( mRenderDriver, strName, hCustomShader );

    // Allow sampler to initialize.
    if ( pSampler->onComponentCreated( &cgComponentCreatedEventArgs(0, cgCloneMethod::None) ) == false )
    {
        pSampler->deleteReference();
        return CG_NULL;
    
    } // End if failed

    // Success!
    return pSampler;
}

//-----------------------------------------------------------------------------
//  Name : createSampler ()
/// <summary>
/// Create a new sampler mapping object for the specified sampler. This
/// overload creates a sampler that will serialize its data to the specified
/// world database. If sandbox mode is not enabled, this acts simply as a
/// pass through to the non-serialized version.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::createSampler( cgWorld * pWorld, const cgString & strName )
{
    return createSampler( pWorld, strName, cgSurfaceShaderHandle() );
}
cgSampler * cgResourceManager::createSampler( cgWorld * pWorld, const cgString & strName, const cgSurfaceShaderHandle & hCustomShader )
{
    // Route through to standard unserialized version if not in sandbox mode.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        return createSampler( strName, hCustomShader );

    // Wrap in a transaction so we can roll back where necessary.
    pWorld->beginTransaction( _T("createSampler") );

    // Instantiate the new sampler
    cgSampler * pSampler = CG_NULL;
    pSampler = new cgSampler( pWorld->generateRefId(false), pWorld, mRenderDriver, strName, hCustomShader );

    // Allow sampler to initialize.
    if ( pSampler->onComponentCreated( &cgComponentCreatedEventArgs(0, cgCloneMethod::None) ) == false )
    {
        // Force immediate deletion of any created sampler. Do not pass go :)
        pSampler->deleteReference();

        // Rollback any modifications to the database.
        pWorld->rollbackTransaction( _T("createSampler") );
        return CG_NULL;
    
    } // End if failed

    // Commit changes
    pWorld->commitTransaction( _T("createSampler") );

    // Success!
    return pSampler;
}

//-----------------------------------------------------------------------------
//  Name : cloneSampler ()
/// <summary>
/// Create a new sampler mapping object for the specified sampler. 
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::cloneSampler( cgSampler * pInit )
{
    return cloneSampler( CG_NULL, true, pInit );
}

//-----------------------------------------------------------------------------
//  Name : cloneSampler ()
/// <summary>
/// Create a new sampler mapping object for the specified sampler. 
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::cloneSampler( cgWorld * pWorld, bool bInternal, cgSampler * pInit )
{
    // Validate requirements
    if ( !pInit || (!bInternal && !pWorld) )
        return CG_NULL;

    // All created samplers are internal if sandbox mode is not enabled.
    if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
        bInternal = true;

    // Generate a new reference identifier for this sampler.
    cgUInt32 nDestRefId = 0;
    if ( bInternal )
        nDestRefId = cgReferenceManager::generateInternalRefId();
    else
        nDestRefId = pWorld->generateRefId( false );
        
    // Create the new sampler, instructing it to clone from the specified source.
    cgSampler * pNewSampler = new cgSampler( nDestRefId, pWorld, mRenderDriver, pInit->getSurfaceShader(), pInit );

    // Allow sampler to initialize its data.
    if ( !pNewSampler->onComponentCreated( &cgComponentCreatedEventArgs(0, cgCloneMethod::Copy) ) )
    {
        // Force immediate deletion of any created sampler. Do not pass go :)
        pNewSampler->deleteReference();
        return CG_NULL;
    
    } // End if failed

    // Success!
    return pNewSampler;
}

//-----------------------------------------------------------------------------
//  Name : loadSampler ()
/// <summary>
/// Load a sampler mapping object from the database.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::loadSampler( cgWorld * pWorld, cgUInt32 nSourceRefId, bool bInternal, cgUInt32 nFlags /* = 0 */, cgUInt32 nChildResourceFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0)*/ )
{
    return loadSampler( pWorld, cgSurfaceShaderHandle(), nSourceRefId, bInternal, nFlags, nChildResourceFlags, _debugSource );
}
cgSampler * cgResourceManager::loadSampler( cgWorld * pWorld, const cgSurfaceShaderHandle & hCustomShader, cgUInt32 nSourceRefId, bool bInternal, cgUInt32 nFlags /* = 0 */, cgUInt32 nChildResourceFlags /* = 0 */, const cgDebugSourceInfo & _debugSource /* = cgDebugSourceInfo(_T(""),0)*/ )
{
    // ToDo: 9999 - pass child resource flags and debug source data to sampler for texture loading step.
    // ToDo: 9999 - Add transaction for rollback on failure (in the cloning case)?

    // Validate requirements
    if ( !nSourceRefId )
        return false;

    // Force internal if no world was supplied.
    if ( !pWorld )
        bInternal = true;

    // Attempt to find an existing resident sampler with the source reference identifier.
    cgReference * pRef = cgReferenceManager::getReference( nSourceRefId );
    if ( pRef && pRef->queryReferenceType( RTID_Sampler ) )
    {
        cgSampler * pExistingSampler = (cgSampler*)pRef;

        // If a source sampler was found we can potentially just return the existing one
        // depending on the supplied options. Specifically, if the user did not want a new 
        // sampler, and both resources are internal *or* both are serialized, we can avoid
        // a clone operation.
        if ( !(nFlags & cgResourceFlags::ForceNew) &&
            bInternal == pExistingSampler->isInternalReference() )
            return pExistingSampler;

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview mode.
        if ( cgGetSandboxMode() != cgSandboxMode::Enabled )
            bInternal = true;
        
        // A new sampler is required. Generate a new destination reference identifier.
        cgUInt32 nDestRefId = 0;
        if ( !pWorld )
            nDestRefId = cgReferenceManager::generateInternalRefId();
        else
            nDestRefId = pWorld->generateRefId( bInternal );

        // Clone the sampler out to the new reference identifier.
        cgSampler * pNewSampler = new cgSampler( nDestRefId, pWorld, mRenderDriver, hCustomShader, pExistingSampler );

        // Allow sampler to initialize its data.
        if ( !pNewSampler->onComponentCreated( &cgComponentCreatedEventArgs(0, cgCloneMethod::Copy) ) )
        {
            // Force immediate deletion of any created sampler. Do not pass go :)
            pNewSampler->deleteReference();
            return CG_NULL;
        
        } // End if failed

        // Success!
        return pNewSampler;

    } // End if existing sampler
    else
    {
        // Must have access to a valid world database.
        if ( !pWorld )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load sampler '0x%x' because no valid world database was supplied.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // If the source reference identifier indicates that the source was internal,
        // but no sampler was found, this is an error situation.
        if ( nSourceRefId >= cgReferenceManager::InternalRefThreshold )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to load sampler '0x%x' because the requested source reference identifier indicated an internal resource that was not resident for cloning.\n"), nSourceRefId );
            return false;
        
        } // End if failed

        // When cloning, we must always generate an internal resource in runtime 
        // or sandbox preview modes.
        if ( (cgGetSandboxMode() != cgSandboxMode::Enabled) && (nFlags & cgResourceFlags::ForceNew) )
            bInternal = true;

        // If a new sampler is required, make sure that we generate a new destination reference identifier.
        cgUInt32 nDestRefId = nSourceRefId;
        if ( nFlags & cgResourceFlags::ForceNew || bInternal )
            pWorld->generateRefId( bInternal );

        // Create the new sampler.
        cgSampler * pNewSampler = new cgSampler( nDestRefId, pWorld, mRenderDriver, cgString::Empty, hCustomShader );

        // Allow sampler to load its data.
        cgCloneMethod::Base CloneMethod = (nSourceRefId == nDestRefId) ? cgCloneMethod::None : cgCloneMethod::Copy;
        if ( !pNewSampler->onComponentLoading( &cgComponentLoadingEventArgs( nSourceRefId, 0, CloneMethod, CG_NULL) ) )
        {
            // Force immediate deletion of any created sampler. Do not pass go :)
            pNewSampler->deleteReference();
            return CG_NULL;
        
        } // End if failed
       
        // Success!
        return pNewSampler;

    } // End if no existing mesh
}

//-----------------------------------------------------------------------------
// Name : getDefaultSampler()
/// <summary>
/// Retrieve one of the default samplers created by the resource manager if
/// available.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgResourceManager::getDefaultSampler( DefaultSamplerTypes Type )
{
    // Create sampler only if we haven't already done so at an earlier time.
    if ( !mDefaultSamplers[Type] )
    {
        cgTextureHandle hTexture;
        switch ( Type )
        {
            case DefaultDiffuseSampler:
                // Load the system defined default diffuse texture.
                mDefaultSamplers[Type] = createSampler( _T("Diffuse") );
                if ( loadTexture( &hTexture, _T("sys://Textures/DefaultDiffuse.dds"), 0, cgDebugSource() ) == true )
                    mDefaultSamplers[Type]->setTexture( hTexture );
                break;

            case DefaultNormalSampler:
                // Load the system defined default normal texture.
                mDefaultSamplers[Type] = createSampler( _T("Normal") );
                if ( loadTexture( &hTexture, _T("sys://Textures/DefaultNormal.dds"), 0, cgDebugSource() ) == true )
                    mDefaultSamplers[Type]->setTexture( hTexture );
                break;

        } // End switch type

    } // End if create default

    // Return default sampler
    return mDefaultSamplers[Type];
}

//-----------------------------------------------------------------------------
// Name : getDefaultMaterial()
/// <summary>
/// Retrieve the default material created by the resource manager if available.
/// </summary>
//-----------------------------------------------------------------------------
cgMaterialHandle cgResourceManager::getDefaultMaterial( ) const
{
    return mDefaultMaterial;
}

//-----------------------------------------------------------------------------
// Name : enableDestruction()
/// <summary>
/// Use this method to enable / disable the destruction of resources. When
/// disabled, all resources that are due to be destroyed will be automatically
/// added to the garbage queue instead where they will await destruction
/// until enabled once again via this method.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::enableDestruction( bool enable )
{
    mDestructionEnabled = enable;
}

//-----------------------------------------------------------------------------
// Name : isDestructionEnabled()
/// <summary>
/// Determine if automatic resource destruction is enabled. When
/// disabled, all resources that are due to be destroyed will be automatically
/// added to the garbage queue instead where they will await destruction
/// until enabled once again via this method.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::isDestructionEnabled( ) const
{
    return mDestructionEnabled;
}

//-----------------------------------------------------------------------------
//  Name : debugResources ()
/// <summary>
/// Debug the actual resources we're currently still maintaining
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::debugResources()
{
    cgString strList;
    bool     bResourcesOutstanding = false;

    strList = listActiveResources( mTextures );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Textures\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mRenderTargets );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Render Targets\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mDepthStencilTargets );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Depth Stencil Targets\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mVertexBuffers );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Vertex Buffers\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mIndexBuffers );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Index Buffers\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mConstantBuffers );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Constant Buffers\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mMeshes );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Meshes\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mAnimationSets );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Animation Sets\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mAudioBuffers );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Audio Buffers\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mScripts );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Scripts\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mMaterials );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Materials\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mSurfaceShaders );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Surface Shaders\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mVertexShaders );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Vertex Shaders\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mPixelShaders );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Pixel Shaders\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mSamplerStates );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Sampler States\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mDepthStencilStates );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding DepthStencil States\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mRasterizerStates );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Rasterizer States\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    strList = listActiveResources( mBlendStates );
    if ( strList.empty() == false )
    {
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, _T("Outstanding Blend States\n") );
        cgAppLog::writeSeparator( cgAppLog::Debug );
        cgAppLog::write( cgAppLog::Debug, strList.c_str() );
        bResourcesOutstanding = true;
    
    } // End if found resources

    if ( !bResourcesOutstanding )
        cgAppLog::write( cgAppLog::Debug, _T("No outstanding resources detected.\n") );
}

//-----------------------------------------------------------------------------
//  Name : listActiveResources () (Private)
/// <summary>
/// Part of the debug process. Dumps out a list of all the specified
/// cgResource items (irrespective of what they store).
/// </summary>
//-----------------------------------------------------------------------------
cgString cgResourceManager::listActiveResources( ResourceItemList & ResourceList )
{
    cgString strList, strItem, strLost;
    cgUInt32 Counter = 1;
    
    // Loop through list
    ResourceItemList::iterator itItem = ResourceList.begin();
    for ( ; itItem != ResourceList.end(); ++itItem )
    {
        // Retrieve resource
        cgResource * pResource = *itItem;
        if ( !pResource ) continue;

        // If the reference count == 1 and it's in the garbage list, this is fine.
        if ( pResource->getReferenceCount( true ) == 1 )
        {
            if ( mResourceGarbage.find( pResource ) != mResourceGarbage.end() )
                continue;

        } // End if RefCount==1

        // Is item lost?
        strLost = (pResource->isResourceLost()) ? _T(" (Lost)") : _T("");

        // Build string representing the item
        strItem = cgString::format( _T("Item %i - Resource '%s'%s remains referenced in %i location(s) beginning with '%s'.\n"), Counter, pResource->getResourceName().c_str(), strLost.c_str(), pResource->getReferenceCount( true ), pResource->getResourceSource().c_str() );

        // Append to list
        strList += strItem + _T("\n");

        // Incement debug counter
        Counter++;
    
    } // Next resource

    // Return the list of outstanding items
    return strList;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ResourceManager )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgResourceManager::processMessage( cgMessage * pMessage )
{
    // What message is this?
    switch ( pMessage->messageId )
    {
        case cgSystemMessages::RenderDriver_DeviceLost:

            // Notify all resources
            notifyDeviceLost( mTextures );
            notifyDeviceLost( mVertexBuffers );
            notifyDeviceLost( mIndexBuffers );
            notifyDeviceLost( mConstantBuffers );
            notifyDeviceLost( mSurfaceShaders );
            notifyDeviceLost( mVertexShaders );
            notifyDeviceLost( mPixelShaders );
            notifyDeviceLost( mMeshes );
            notifyDeviceLost( mMaterials );
            notifyDeviceLost( mRenderTargets );
            notifyDeviceLost( mDepthStencilTargets );
            notifyDeviceLost( mAnimationSets );
            notifyDeviceLost( mAudioBuffers );
            notifyDeviceLost( mScripts );
            notifyDeviceLost( mSamplerStates );
            notifyDeviceLost( mDepthStencilStates );
            notifyDeviceLost( mRasterizerStates );
            notifyDeviceLost( mBlendStates );
            
            // Processed message
            return true;

        case cgSystemMessages::RenderDriver_DeviceRestored:
            
            // Notify all resources
            notifyDeviceRestored( mTextures );
            notifyDeviceRestored( mVertexBuffers );
            notifyDeviceRestored( mIndexBuffers );
            notifyDeviceRestored( mConstantBuffers );
            notifyDeviceRestored( mSurfaceShaders );
            notifyDeviceRestored( mVertexShaders );
            notifyDeviceRestored( mPixelShaders );
            notifyDeviceRestored( mMeshes );
            notifyDeviceRestored( mMaterials );
            notifyDeviceRestored( mRenderTargets );
            notifyDeviceRestored( mDepthStencilTargets );
            notifyDeviceRestored( mAnimationSets );
            notifyDeviceRestored( mAudioBuffers );
            notifyDeviceRestored( mScripts );
            notifyDeviceRestored( mSamplerStates );
            notifyDeviceRestored( mDepthStencilStates );
            notifyDeviceRestored( mRasterizerStates );
            notifyDeviceRestored( mBlendStates );

            // Processed message
            return true;

        case cgSystemMessages::Resources_CollectGarbage:

            // Test the garbage list
            checkGarbage();

            // Send the garbage collect message again if we aren't in the process of
            // being unregistered.
            if ( !(pMessage->fromId == getReferenceId() && pMessage->sourceUnregistered) )
                sendGarbageMessage();
            
            // Processed message
            return true;

        case cgSystemMessages::Resources_ReloadShaders:

            // Reload shaders without delay.
            if ( !(pMessage->fromId == getReferenceId() && pMessage->sourceUnregistered) )
                reloadShaders( );
            
            // Processed message
            return true;

        case cgSystemMessages::Resources_ReloadScripts:

            // Reload scripts without delay.
            if ( !(pMessage->fromId == getReferenceId() && pMessage->sourceUnregistered) )
                reloadScripts( );
            
            // Processed message
            return true;
    
    } // End message type switch
    
    // Message was not processed, pass to base.
    return cgReference::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : notifyDeviceLost () (Private)
/// <summary>
/// Notify all resources in the specified list that the device was lost
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::notifyDeviceLost( ResourceItemList & ResourceList )
{
    ResourceItemList::iterator itItem;

    // Loop through list
    for ( itItem = ResourceList.begin(); itItem != ResourceList.end(); ++itItem )
    {
        // Retrieve resource
        cgResource * pResource = *itItem;
        if ( pResource != CG_NULL ) pResource->deviceLost();
    
    } // Next resource
}

//-----------------------------------------------------------------------------
//  Name : notifyDeviceRestored () (Private)
/// <summary>
/// Notify all resources in the specified list that the device was restored
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::notifyDeviceRestored( ResourceItemList & ResourceList )
{
    ResourceItemList::iterator itItem;

    // Loop through list
    for ( itItem = ResourceList.begin(); itItem != ResourceList.end(); ++itItem )
    {
        // Retrieve resource
        cgResource * pResource = *itItem;
        if ( pResource != CG_NULL ) pResource->deviceRestored();

    } // Next resource
}

//-----------------------------------------------------------------------------
//  Name : emptyGarbage ()
/// <summary>
/// Process the garbage list and empty it irrespective of the time
/// remaining on the destruction delay.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::emptyGarbage( )
{
    // Force the garbage to be emptied
    // Note : If a top level resource (i.e. a mesh) is in the garbage list
    //        and some of it's child resources are set to use a destruction delay
    //        those child resources will also be added to the garbage list when
    //        the top level resource is released. This is acceptable because of
    //        the way the 'checkGarbage' method has been written to keep searching
    //        through the garbage list as long as any items remain. As a result,
    //        when child resources are added to the end, it will automatically 
    //        release those resources too.
    checkGarbage( true );
}

//-----------------------------------------------------------------------------
//  Name : checkGarbage () (Private)
/// <summary>
/// Check the contents of the garbage list.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::checkGarbage( bool bEmpty /* = false */ )
{
    // Skip garbage check altogether if destruction is disabled.
    if ( !mDestructionEnabled )
        return;
    
    // Keep testing while necessary
    ResourceItemSet::iterator itItem;
    ResourceItemSet::iterator itNext;
    cgFloat fCurrentTime = (cgFloat)cgTimer::getInstance()->getTime();
    bool bTestGarbage = true;
    for ( ; bTestGarbage == true; )
    {
        // Following loop must request to test again if necessary
        bTestGarbage = false;

        // Loop through the garbage list
        for ( itItem = mResourceGarbage.begin(); itItem != mResourceGarbage.end(); )
        {
            cgResource * pResource = *itItem;

            // Move on in the list. ReleaseResource may remove the item from the
            // garbage list.
            itItem++;
            
            // Ensure that this is a valid resource.
            if ( pResource == CG_NULL )
                continue;

            // Due to be cleaned up?
            if ( bEmpty || (fCurrentTime > (pResource->mLastReferenced + pResource->mDestroyDelay)) )
            {
                if ( !bEmpty && pResource->mDestroyDelay > 0 )
                    cgAppLog::write( cgAppLog::Debug, _T("Unloading trashed resource '%s' because its destruction delay of %g second(s) expired.\n"), pResource->getResourceName().c_str(), pResource->mDestroyDelay );

                // Release the final reference to the resource
                pResource->removeReference( this, true );
                
                // Test the list again because the above release call may have
                // added new items to the garbage list.
                bTestGarbage = true;
            
            } // End if destroy delay time has passed
        
        } // Next Item in garbage list

    } // Keep testing while necessary
}

//-----------------------------------------------------------------------------
//  Name : sendGarbageMessage () (Private)
/// <summary>
/// Sends the garbage collection message to ourselves.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::sendGarbageMessage( )
{
    cgMessage Msg;

    // Initialize message structure
    Msg.messageId = cgSystemMessages::Resources_CollectGarbage;

    // ToDo: Configurable garbage collection schedule?

    // Send the message to ourselves with a 1 second delay
    cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, 1.0f );
}

//-----------------------------------------------------------------------------
//  Name : removeResource () (Private)
/// <summary>
/// Called by cgResource to actually remove a resource from the internal 
/// list.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::removeResource( cgResource * pResource )
{
    cgToDo( "Optimization", "Take advantage of look up tables to remove a resource from the lists where possible." );

    // Validate requirements
    if ( pResource == CG_NULL )
        return;

    // Select the correct list.
    ResourceItemList * pList = CG_NULL;
    switch ( pResource->getResourceType() )
    {
        case cgResourceType::Texture:
            pList = &mTextures;
            break;
        case cgResourceType::SamplerState:
            pList = &mSamplerStates;
            break;
        case cgResourceType::DepthStencilState:
            pList = &mDepthStencilStates;
            break;
        case cgResourceType::RasterizerState:
            pList = &mRasterizerStates;
            break;
        case cgResourceType::BlendState:
            pList = &mBlendStates;
            break;
        case cgResourceType::VertexBuffer:
            pList = &mVertexBuffers;
            break;
        case cgResourceType::IndexBuffer:
            pList = &mIndexBuffers;
            break;
        case cgResourceType::ConstantBuffer:
            pList = &mConstantBuffers;
            break;
        case cgResourceType::SurfaceShader:
            pList = &mSurfaceShaders;
            break;
        case cgResourceType::VertexShader:
            pList = &mVertexShaders;
            break;
        case cgResourceType::PixelShader:
            pList = &mPixelShaders;
            break;
        case cgResourceType::Mesh:
            pList = &mMeshes;
            break;
        case cgResourceType::RenderTarget:
            pList = &mRenderTargets;
            break;
        case cgResourceType::DepthStencilTarget:
            pList = &mDepthStencilTargets;
            break;
        case cgResourceType::AnimationSet:
            pList = &mAnimationSets;
            break;
        case cgResourceType::AudioBuffer:
            pList = &mAudioBuffers;
            break;
        case cgResourceType::Script:
            pList = &mScripts;
            break;
        case cgResourceType::Material:
            pList = &mMaterials;
            break;
    
    } // End type switch

    // Valid resource type?
    if ( !pList )
        return;

    // Find this resource in the list and remove it if it exists
    ResourceItemList::iterator itItem;
    for ( itItem = pList->begin(); itItem != pList->end(); ++itItem )
    {
        // Matching item?
        if ( *itItem == pResource )
            break;
    
    } // Next Item
    if ( itItem != pList->end() )
        pList->erase( itItem );

    // This should also be removed from the garbage list if it's there
    mResourceGarbage.erase( pResource );
    
    // Should also be removed from any lookup table associated with the type.
    switch ( pResource->getResourceType() )
    {
        case cgResourceType::RenderTarget:
        {
            NamedResourceMap::iterator itResource = mRenderTargetLUT.find( pResource->getResourceName() );
            if ( itResource != mRenderTargetLUT.end() )
                mRenderTargetLUT.erase( itResource );
            break;

        } // End case RenderTarget
        case cgResourceType::SamplerState:
        {
            cgSamplerState * pState = (cgSamplerState*)pResource;
            SamplerStateMap::iterator itState = mSamplerStateLUT.find( SamplerStateKey( &pState->getValues() ) );
            if ( itState != mSamplerStateLUT.end() )
                mSamplerStateLUT.erase( itState );
            break;

        } // End case SamplerState
        case cgResourceType::DepthStencilState:
        {
            cgDepthStencilState * pState = (cgDepthStencilState*)pResource;
            DepthStencilStateMap::iterator itState = mDepthStencilStateLUT.find( DepthStencilStateKey( &pState->getValues() ) );
            if ( itState != mDepthStencilStateLUT.end() )
                mDepthStencilStateLUT.erase( itState );
            break;

        } // End case DepthStencilState
        case cgResourceType::RasterizerState:
        {
            cgRasterizerState * pState = (cgRasterizerState*)pResource;
            RasterizerStateMap::iterator itState = mRasterizerStateLUT.find( RasterizerStateKey( &pState->getValues() ) );
            if ( itState != mRasterizerStateLUT.end() )
                mRasterizerStateLUT.erase( itState );
            break;

        } // End case RasterizerState
        case cgResourceType::BlendState:
        {
            cgBlendState * pState = (cgBlendState*)pResource;
            BlendStateMap::iterator itState = mBlendStateLUT.find( BlendStateKey( &pState->getValues() ) );
            if ( itState != mBlendStateLUT.end() )
                mBlendStateLUT.erase( itState );
            break;

        } // End case BlendState
        case cgResourceType::VertexShader:
        {
            cgVertexShader * pShader = (cgVertexShader*)pResource;
            ShaderMap::iterator itShader = mVertexShaderLUT.find( ShaderKey( &pShader->getIdentifier() ) );
            if ( itShader != mVertexShaderLUT.end() )
                mVertexShaderLUT.erase( itShader );
            break;

        } // End case VertexShader
        case cgResourceType::PixelShader:
        {
            cgPixelShader * pShader = (cgPixelShader*)pResource;
            ShaderMap::iterator itShader = mPixelShaderLUT.find( ShaderKey( &pShader->getIdentifier() ) );
            if ( itShader != mPixelShaderLUT.end() )
                mPixelShaderLUT.erase( itShader );
            break;

        } // End case PixelShader

    } // End Switch type
}

//-----------------------------------------------------------------------------
//  Name : addGarbageResource () (Private)
/// <summary>
/// Add this resource to the garbage list awaiting cleanup after the
/// relevant amount of time has passed.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::addGarbageResource( cgResource * pResource )
{
    //cgAppLog::write( cgAppLog::Debug, _T("Adding garbage resource! %s\n"), pResource->getResourceName().c_str() );
    mResourceGarbage.insert( pResource );
}

//-----------------------------------------------------------------------------
//  Name : removeGarbageResource () (Private)
/// <summary>
/// Remove this resource from the garbage list. It has been referenced
/// again while waiting for destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::removeGarbageResource( cgResource * pResource )
{
    //cgAppLog::write( cgAppLog::Debug, _T("Removing garbage resource! %s\n"), pResource->getResourceName().c_str() );
    mResourceGarbage.erase( pResource );
}

//-----------------------------------------------------------------------------
//  Name : releaseResourceList () (Private)
/// <summary>
/// Release all of the resources in the specified list.
/// </summary>
//-----------------------------------------------------------------------------
void cgResourceManager::releaseResourceList( ResourceItemList & ResourceList, bool bOwnedOnly )
{
    ResourceItemList::iterator itItem;

    // Loop through list
    for ( itItem = ResourceList.begin(); itItem != ResourceList.end();  )
    {
        // Retrieve resource
        cgResource * pResource = *itItem;

        // Move on to next iterator (ReleaseResource may alter the resource list)
        ++itItem;

        // Ensure this resource is valid
        if ( pResource == CG_NULL )
            continue;

        if ( bOwnedOnly == true )
        {
            // Release our own personal reference to this resource.
            // It may still be owned by other systems.
            if ( pResource->mFlags & cgResourceFlags::AlwaysResident )
            {
                // This resource is no longer always resident
                pResource->setManagementData( this, pResource->mFlags & ~cgResourceFlags::AlwaysResident );
                pResource->removeReference( this, true, true );

            } // End if resident resource

        } // End if only the items we own
        else
        {
            // Ensure that no automatic destruction management occurs during this release call.
            // Just release the resource manager's reference.
            pResource->setManagementData( this, cgResourceFlags::AlwaysResident );

            // Release our reference to the resource (ignore destruction delay)
            pResource->removeReference( this, true, true );

        } // End if all items

    } // Next resource
}

//-----------------------------------------------------------------------------
//  Name : operator < () (MaterialKey&, MaterialKey&)
/// <summary>
/// Perform less than comparison on the specified materials.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::MaterialKey& Key1, const cgResourceManager::MaterialKey& Key2)
{
    // CG_NULL pattern checks
    if ( Key1.material == CG_NULL && Key2.material != CG_NULL ) return true;
    if ( Key1.material == CG_NULL || Key2.material == CG_NULL ) return false;

    // Materials must have a matching type.
    if ( Key1.material->getReferenceType() != Key2.material->getReferenceType() )
        return ( Key1.material->getReferenceType() < Key2.material->getReferenceType() );

    // Perform actual compare
    return (Key1.material->compare( *Key2.material ) < 0);
}

//-----------------------------------------------------------------------------
//  Name : operator < () (ShaderKey&, ShaderKey&)
/// <summary>
/// Perform less than comparison on the specified shader identifiers.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::ShaderKey& Key1, const cgResourceManager::ShaderKey& Key2)
{
    // CG_NULL pattern checks
    if ( !Key1.identifier && Key2.identifier ) return true;
    if ( !Key1.identifier || !Key2.identifier ) return false;

    // Perform actual compare
    return (Key1.identifier->compare( *Key2.identifier ) < 0);
}

//-----------------------------------------------------------------------------
//  Name : operator < () (SamplerStateKey&, SamplerStateKey&)
/// <summary>
/// Perform less than comparison on the specified keys.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::SamplerStateKey& Key1, const cgResourceManager::SamplerStateKey& Key2)
{
    // CG_NULL pattern checks
    if ( Key1.desc == CG_NULL && Key2.desc != CG_NULL ) return true;
    if ( Key1.desc == CG_NULL || Key2.desc == CG_NULL ) return false;

    // Perform actual compare
    return (memcmp( Key1.desc, Key2.desc, sizeof(cgSamplerStateDesc) ) < 0);
}

//-----------------------------------------------------------------------------
//  Name : operator < () (DepthStencilStateKey&, DepthStencilStateKey&)
/// <summary>
/// Perform less than comparison on the specified keys.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::DepthStencilStateKey& Key1, const cgResourceManager::DepthStencilStateKey& Key2)
{
    // CG_NULL pattern checks
    if ( Key1.desc == CG_NULL && Key2.desc != CG_NULL ) return true;
    if ( Key1.desc == CG_NULL || Key2.desc == CG_NULL ) return false;

    // Perform actual compare
    return (memcmp( Key1.desc, Key2.desc, sizeof(cgDepthStencilStateDesc) ) < 0);
}

//-----------------------------------------------------------------------------
//  Name : operator < () (RasterizerStateKey&, RasterizerStateKey&)
/// <summary>
/// Perform less than comparison on the specified keys.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::RasterizerStateKey& Key1, const cgResourceManager::RasterizerStateKey& Key2)
{
    // CG_NULL pattern checks
    if ( Key1.desc == CG_NULL && Key2.desc != CG_NULL ) return true;
    if ( Key1.desc == CG_NULL || Key2.desc == CG_NULL ) return false;

    // Perform actual compare
    return (memcmp( Key1.desc, Key2.desc, sizeof(cgRasterizerStateDesc) ) < 0);
}

//-----------------------------------------------------------------------------
//  Name : operator < () (BlendStateKey&, BlendStateKey&)
/// <summary>
/// Perform less than comparison on the specified keys.
/// </summary>
//-----------------------------------------------------------------------------
bool operator < (const cgResourceManager::BlendStateKey& Key1, const cgResourceManager::BlendStateKey& Key2)
{
    // CG_NULL pattern checks
    if ( Key1.desc == CG_NULL && Key2.desc != CG_NULL ) return true;
    if ( Key1.desc == CG_NULL || Key2.desc == CG_NULL ) return false;

    // Perform actual compare
    return (memcmp( Key1.desc, Key2.desc, sizeof(cgBlendStateDesc) ) < 0);
}