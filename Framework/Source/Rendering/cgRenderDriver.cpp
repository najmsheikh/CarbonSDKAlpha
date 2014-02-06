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
// File : cgRenderDriver.cpp                                                 //
//                                                                           //
// Desc : Rendering class wraps the properties, initialization and           //
//        management of our rendering device. This includes the enumeration, //
//        creation and destruction of our D3D device and associated          //
//        resources.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgRenderDriver Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgRenderingCapabilities.h>
#include <World/Objects/cgCameraObject.h>
#include <World/Objects/cgLightObject.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgScript.h>
#include <Resources/cgMaterial.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgTexture.h> // ToDo: 6767 - Introduced by VPL stuff
#include <Resources/cgVertexBuffer.h> // ToDo: 6767 - Introduced by VPL stuff
#include <Rendering/cgSampler.h> // ToDo: 6767 - Introduced by VPL stuff
#include <Resources/cgDepthStencilTarget.h>
#include <System/cgMessageTypes.h>
#include <System/cgFilterExpression.h>
#include <System/cgStringUtility.h>
#include <System/cgAppWindow.h>
#include <System/cgProfiler.h>

// Platform specific implementations
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>

//-----------------------------------------------------------------------------
// Static Member Definitions.
//-----------------------------------------------------------------------------
cgRenderDriver * cgRenderDriver::mSingleton = CG_NULL;

///////////////////////////////////////////////////////////////////////////////
// cgRenderDriver Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRenderDriver () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver::cgRenderDriver() : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mConfigLoaded           = false;
    mInitialized            = false;
    mResourceManager        = CG_NULL;
    mCaps                   = CG_NULL;
    mHardwareType           = cgHardwareType::Generic;
    mFocusWindow            = CG_NULL;
    mOutputWindow           = CG_NULL;
    mOwnsWindow             = false;
    mLostDevice             = false;
    mActive                 = false;
    mSuppressResizeEvent    = false;
    mStateFilteringEnabled  = true;
    mTargetSize             = cgSize( 0, 0 );
    mAdapterAspectRatio     = 0;
    mSystemExports          = CG_NULL;
    mCurrentMaterial        = CG_NULL;
    mWorldSpaceFormat       = CG_NULL;
    mScreenSpaceFormat      = CG_NULL;
    mClipSpaceFormat        = CG_NULL;
    mDriverShader           = CG_NULL;
    mPrimaryView            = CG_NULL;
    mConstantsDirty         = 0;
    mPrimitivesDrawn        = 0;

    // ToDo: 6767 - Should these really live here? Doubt it.
    mSampler2D0             = CG_NULL;
	mSampler2D1             = CG_NULL;
    mCurrentVPLSize         = cgSize(0,0);
    
    // Clear any memory as required
    memset( &mSystemExportVars, 0, sizeof(ExportVars) );

    // Reset cached matrices
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mViewMatrix = mtxIdentity;
    mProjectionMatrix = mtxIdentity;
    mViewProjectionMatrix = mtxIdentity;

    // Initialize stacks with space for the default elements
    mRenderPassStack.push( _T("Default") );
    mOverrideMethodStack.push( cgString::Empty );
    mDepthStencilStateStack.push( DepthStencilStateData() );
    mRasterizerStateStack.push( cgRasterizerStateHandle::Null );
    mBlendStateStack.push( cgBlendStateHandle::Null );
    mVertexShaderStack.push( cgVertexShaderHandle::Null );
    mPixelShaderStack.push( cgPixelShaderHandle::Null );
    mIndicesStack.push( cgIndexBufferHandle::Null );
    mWorldTransformStack.push( mtxIdentity );
    mCameraStack.push_back( CG_NULL );
    mClipPlaneStack.push( ClipPlaneData() );
    mVertexFormatStack.push( CG_NULL );
    mMaterialFilterStack.push( CG_NULL );
    mScissorRectStack.push( cgRect() );
    for ( cgInt i = 0; i < MaxStreamSlots; ++i )
        mVertexStreamStack[i].push( VertexStreamData() );
    for ( cgInt i = 0; i < MaxSamplerSlots; ++i )
        mSamplerStateStack[i].push( cgSamplerStateHandle::Null );
    for ( cgInt i = 0; i < MaxConstantBufferSlots; ++i )
        mConstantBufferStack[i].push( cgConstantBufferHandle::Null );
    for ( cgInt i = 0; i < MaxTextureSlots; ++i )
        mTextureStack[i].push( cgTextureHandle::Null );

    // Note: We do not push a default entry onto the viewport or render
    // target data stacks. This will be handled automatically by the
    // platform render driver in its initial call to 'beginTargetRender()'
    // which will push the device buffers / viewport onto these stacks

    // Register us with the window messaging groups
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_AppWindow );
}

//-----------------------------------------------------------------------------
//  Name : ~cgRenderDriver () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver::~cgRenderDriver()
{
    // release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : releaseOwnedResources () (Virtual)
/// <summary>
/// Release any resources that the render driver may have loaded via its
/// resource manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::releaseOwnedResources()
{
    // Release script side objects.
    if ( mSystemExports )
        mSystemExports->release();

    // Destroy allocated render views.
    NamedRenderViewMap::iterator itView, itCurrent;
    for ( itView = mRenderViews.begin(); itView != mRenderViews.end(); )
    {
        // Record current iterator and increment. This entry
        // will be automatically removed from the map on disposal.
        itCurrent = itView++;
        itCurrent->second->scriptSafeDispose();

    } // Next View

    // Release any active resource data
    mSandboxShader.close( true );
    mSandboxConstants.close( true );
    mDriverShaderHandle.close( true );
    mCurrentMaterialHandle.close( true );
    mCurrentSurfaceShaderHandle.close( true );
    mSystemExportScript.close( true );
    mElementBlendState.close( true );
    mElementDepthState.close( true );
    mDefaultSamplerState.close( true );
    mDefaultDepthStencilState.close( true );
    mDefaultRasterizerState.close( true );
    mDefaultBlendState.close( true );
    mDeviceFrameBuffer.close(true);
    mDeviceDepthStencilTarget.close(true);

    // Release all stack held resources.
    while (mDepthStencilStateStack.size() > 0)
    {
        mDepthStencilStateStack.top().handle.close(true);
        mDepthStencilStateStack.pop();
    } // Next DepthStencilState
    while (mRasterizerStateStack.size() > 0)
    {
        mRasterizerStateStack.top().close(true);
        mRasterizerStateStack.pop();
    } // Next RasterizerState
    while (mBlendStateStack.size() > 0)
    {
        mBlendStateStack.top().close(true);
        mBlendStateStack.pop();
    } // Next BlendState
    while (mVertexShaderStack.size() > 0)
    {
        mVertexShaderStack.top().close(true);
        mVertexShaderStack.pop();
    } // Next VertexShader
    while (mPixelShaderStack.size() > 0)
    {
        mPixelShaderStack.top().close(true);
        mPixelShaderStack.pop();
    } // Next PixelShader
    while (mIndicesStack.size() > 0)
    {
        mIndicesStack.top().close(true);
        mIndicesStack.pop();
    } // Next IndexBuffer
    for ( cgInt i = 0; i < MaxStreamSlots; ++i )
    {
        while (mVertexStreamStack[i].size() > 0)
        {
            mVertexStreamStack[i].top().handle.close(true);
            mVertexStreamStack[i].pop();
        
        } // Next VertexBuffer

    } // Next Vertex Stream Stack
    for ( cgInt i = 0; i < MaxSamplerSlots; ++i )
    {
        while (mSamplerStateStack[i].size() > 0)
        {
            mSamplerStateStack[i].top().close(true);
            mSamplerStateStack[i].pop();
        
        } // Next SamplerState

    } // Next Sampler Stack
    for ( cgInt i = 0; i < MaxConstantBufferSlots; ++i )
    {
        while (mConstantBufferStack[i].size() > 0)
        {
            mConstantBufferStack[i].top().close(true);
            mConstantBufferStack[i].pop();
        
        } // Next ConstantBuffer

    } // Next Constant Buffer Stack
    for ( cgInt i = 0; i < MaxTextureSlots; ++i )
    {
        while (mTextureStack[i].size() > 0)
        {
            mTextureStack[i].top().close(true);
            mTextureStack[i].pop();
        
        } // Next Texture

    } // Next Texture Stack

    // Release VPL related resources 
    SizeVertexBufferMap::iterator itVPLs;
    for ( itVPLs = mVPLVertexBuffers.begin(); itVPLs != mVPLVertexBuffers.end(); ++itVPLs )
        itVPLs->second.close(true);
    mVPLVertexBuffers.clear();
	mVPLBuffer.close(true);
	mDisabledDepthState.close(true);
	if ( mSampler2D0 )
		mSampler2D0->scriptSafeDispose();
	if ( mSampler2D1 )
		mSampler2D1->scriptSafeDispose();

    // Clear Variables
    mCurrentMaterial  = CG_NULL;
    mCurrentSurfaceShader = CG_NULL;
    mSystemExports = CG_NULL;
    mDriverShader = CG_NULL;
    mPrimaryView = CG_NULL;
    mSampler2D0 = CG_NULL;
	mSampler2D1 = CG_NULL;
	mCurrentVPLSize = cgSize(0,0);
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRenderDriver::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release any active resources loaded via the resource manager.
    releaseOwnedResources();

    // Shut down any window that we created. We never create an output 
    // window, so focus window is all we need to destroy.
    if ( mOwnsWindow )
        delete mFocusWindow;

    // Release any allocated memory
    if ( mCaps )
        mCaps->scriptSafeDispose();
    
    // Reset any variables
    mCaps                   = CG_NULL;
    mConfigLoaded           = false;
    mInitialized            = false;
    mResourceManager        = CG_NULL;
    mHardwareType           = cgHardwareType::Generic;
    mFocusWindow            = CG_NULL;
    mOutputWindow           = CG_NULL;
    mOwnsWindow             = false;
    mLostDevice             = false;
    mActive                 = false;
    mTargetSize             = cgSize( 0, 0 );
    mAdapterAspectRatio     = 0;
    mSystemExports          = CG_NULL;
    mCurrentMaterial        = CG_NULL;
    mCurrentSurfaceShader   = CG_NULL;
    mWorldSpaceFormat       = CG_NULL;
    mScreenSpaceFormat      = CG_NULL;
    mClipSpaceFormat        = CG_NULL;
    mDriverShader           = CG_NULL;
    mConstantsDirty         = 0;

    // Clear any memory as required
    mConfig = cgRenderDriverConfig();
    while (mRenderPassStack.size() > 0)
        mRenderPassStack.pop();
    while (mOverrideMethodStack.size() > 0)
        mOverrideMethodStack.pop();
    while (mDepthStencilStateStack.size() > 0)
        mDepthStencilStateStack.pop();
    while (mRasterizerStateStack.size() > 0)
        mRasterizerStateStack.pop();
    while (mBlendStateStack.size() > 0)
        mBlendStateStack.pop();
    while (mVertexShaderStack.size() > 0)
        mVertexShaderStack.pop();
    while (mPixelShaderStack.size() > 0)
        mPixelShaderStack.pop();
    while (mIndicesStack.size() > 0)
        mIndicesStack.pop();
    while (mWorldTransformStack.size() > 0)
        mWorldTransformStack.pop();
    while (mCameraStack.size() > 0)
        mCameraStack.pop_back();
    while (mClipPlaneStack.size() > 0)
        mClipPlaneStack.pop();
    while (mVertexFormatStack.size() > 0)
        mVertexFormatStack.pop();
    while (mMaterialFilterStack.size() > 0)
    {
        delete mMaterialFilterStack.top();
        mMaterialFilterStack.pop();
    
    } // Next material filter
    while (mScissorRectStack.size() > 0)
        mScissorRectStack.pop();
    while (mViewportStack.size() > 0)
        mViewportStack.pop();
    while (mRenderViewStack.size() > 0)
        mRenderViewStack.pop();
    for ( cgInt i = 0; i < MaxStreamSlots; ++i )
    {
        while (mVertexStreamStack[i].size() > 0)
            mVertexStreamStack[i].pop();
    } // Next Vertex Stream Stack
    for ( cgInt i = 0; i < MaxSamplerSlots; ++i )
    {
        while (mSamplerStateStack[i].size() > 0)
            mSamplerStateStack[i].pop();
    } // Next Sampler Stack
    for ( cgInt i = 0; i < MaxConstantBufferSlots; ++i )
    {
        while (mConstantBufferStack[i].size() > 0)
            mConstantBufferStack[i].pop();
    } // Next Constant Buffer Stack
    for ( cgInt i = 0; i < MaxTextureSlots; ++i )
    {
        while (mTextureStack[i].size() > 0)
            mTextureStack[i].pop();
    } // Next Texture Stack

    // Clear structures
    memset( &mSystemExportVars, 0, sizeof(ExportVars) );

    // Reset cached matrices
    cgMatrix mtxIdentity;
    cgMatrix::identity( mtxIdentity );
    mViewMatrix = mtxIdentity;
    mProjectionMatrix = mtxIdentity;
    mViewProjectionMatrix = mtxIdentity;

    // Initialize stacks with space for the default elements
    mRenderPassStack.push( _T("Default") );
    mOverrideMethodStack.push( cgString::Empty );
    mDepthStencilStateStack.push( DepthStencilStateData() );
    mRasterizerStateStack.push( cgRasterizerStateHandle::Null );
    mBlendStateStack.push( cgBlendStateHandle::Null );
    mVertexShaderStack.push( cgVertexShaderHandle::Null );
    mPixelShaderStack.push( cgPixelShaderHandle::Null );
    mIndicesStack.push( cgIndexBufferHandle::Null );
    mWorldTransformStack.push( mtxIdentity );
    mCameraStack.push_back( CG_NULL );
    mClipPlaneStack.push( ClipPlaneData() );
    mVertexFormatStack.push( CG_NULL );
    mMaterialFilterStack.push( CG_NULL );
    mScissorRectStack.push( cgRect() );
    for ( cgInt i = 0; i < MaxStreamSlots; ++i )
        mVertexStreamStack[i].push( VertexStreamData() );
    for ( cgInt i = 0; i < MaxSamplerSlots; ++i )
        mSamplerStateStack[i].push( cgSamplerStateHandle::Null );
    for ( cgInt i = 0; i < MaxConstantBufferSlots; ++i )
        mConstantBufferStack[i].push( cgConstantBufferHandle::Null );
    for ( cgInt i = 0; i < MaxTextureSlots; ++i )
        mTextureStack[i].push( cgTextureHandle::Null );

    // Note: We do not push a default entry onto the viewport or render
    // target data stacks. This will be handled automatically by the
    // platform render driver in its initial call to 'beginTargetRender()'
    // which will push the device buffers / viewport onto these stacks
    
    // Call base class implementation if required.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : getInstance () (Static)
/// <summary>
/// Singleton instance accessor function.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgRenderDriver::getInstance( )
{
    return mSingleton;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriver * cgRenderDriver::createInstance()
{
    // Determine which driver we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9RenderDriver();

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11RenderDriver();

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
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
void cgRenderDriver::createSingleton( )
{
    // Allocate!
    if ( !mSingleton )
        mSingleton = createInstance();
}

//-----------------------------------------------------------------------------
//  Name : destroySingleton () (Static)
/// <summary>
/// Clean up the singleton memory.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::destroySingleton( )
{
    // Destroy (unless script referencing)!
    if ( mSingleton )
        mSingleton->scriptSafeDispose();
    mSingleton = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_RenderDriver )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getConfig ()
/// <summary>
/// Retrieve the configuration options for the render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderDriverConfig cgRenderDriver::getConfig( ) const
{
    return mConfig;
}

//-----------------------------------------------------------------------------
//  Name : useHardwareTnL ()
/// <summary>
/// Utility function to see if hardware TnL is available on this device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::useHardwareTnL( ) const
{
    return mConfig.useHardwareTnL;
}

//-----------------------------------------------------------------------------
//  Name : getResourceManager ()
/// <summary>
/// Retrieve the resource manager associated with this render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgResourceManager * cgRenderDriver::getResourceManager( ) const
{
    return mResourceManager;
}

//-----------------------------------------------------------------------------
//  Name : getShaderInterface ()
/// <summary>
/// Retrieve the 'shared' surface shader script object through which common
/// shared configuration values are communicated to loaded shaders.
/// Note: Reference count of returned script object is automatically 
/// incremented and any subsequent release should be correctly handled by the 
/// caller.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * cgRenderDriver::getShaderInterface( ) const
{
    if ( mSystemExports )
        mSystemExports->addRef();
    return mSystemExports;
}

//-----------------------------------------------------------------------------
//  Name : getSystemShader ()
/// <summary>
/// Retrieve the surface shader object that is the root owner of the "System"
/// surface shader script object and the shared configuration definitions.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgRenderDriver::getSystemShader( ) const
{
    return mDriverShaderHandle;
}

//-----------------------------------------------------------------------------
//  Name : getFocusWindow ()
/// <summary>
/// Retrieve the focus window object.
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow * cgRenderDriver::getFocusWindow( ) const
{
    return mFocusWindow;
}

//-----------------------------------------------------------------------------
//  Name : getOutputWindow ()
/// <summary>
/// Retrieve the output window object.
/// </summary>
//-----------------------------------------------------------------------------
cgAppWindow * cgRenderDriver::getOutputWindow( ) const
{
    return mOutputWindow;
}

//-----------------------------------------------------------------------------
//  Name : isWindowActive ()
/// <summary>
/// Determine if the device window is active and currently has focus.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::isWindowActive( ) const
{
    return mActive;
}

//-----------------------------------------------------------------------------
//  Name : isInitialized ()
/// <summary>
/// Determine if the driver has been initialized.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::isInitialized( ) const
{
    return mInitialized;
}

//-----------------------------------------------------------------------------
//  Name : initialize () (Virtual)
/// <summary>
/// Initialize the render driver using the window specified.
/// Note : In this case, you will be responsible for informing the render driver
/// about any important messages you receive in the windows' wndproc
/// (i.e. window resize events).
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::initialize( cgResourceManager * pResources, cgAppWindow * pFocusWindow, cgAppWindow * pOutputWindow )
{
    // Store the window details
    mOwnsWindow   = false;
    mFocusWindow  = pFocusWindow;
    mOutputWindow = pOutputWindow;
    if ( !mOutputWindow )
        mOutputWindow = mFocusWindow;

    // Attach to the specified resource manager.
    mResourceManager = pResources;
    if ( pResources == CG_NULL || pResources->setRenderDriver( this ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to attach to specified resource manager. The application will now exit.\n") );
        return false;
    
    } // End if no resources

    // Complete the initialization process
    return postInit();
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the render driver
/// Note : This overload will automatically create the window on your behalf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::initialize( cgResourceManager * pResources, const cgString & WindowTitle /* = _T("Render Output") */, cgInt32 IconResource /* = -1 */ )
{
    // Attach to the specified resource manager.
    mResourceManager = pResources;
    if ( pResources == CG_NULL || pResources->setRenderDriver( this ) == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to attach to specified resource manager. The application will now exit.\n") );
        return false;
    
    } // End if no resources

    // Complete the initialization process
    return postInit();
}

//-----------------------------------------------------------------------------
//  Name : postInit () (Protected Virtual)
/// <summary>
/// Complete the render driver initialization process
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::postInit()
{
    // Driver is now initialized.
    mInitialized = true;

    // Load the core system exports scripts that are required in 
    // order to bind system data to the shader system.
    if ( initShaderSystem() == false )
        return false;

    // Setup default system states.
    setSystemState( cgSystemState::ShadingQuality, mConfig.shadingQuality );
    setSystemState( cgSystemState::PostProcessQuality, mConfig.postProcessQuality );
    setSystemState( cgSystemState::AntiAliasingQuality, mConfig.antiAliasingQuality );

    // Allow resource manager to perform its post-initialization processes.
    if ( mResourceManager->postInit( ) == false )
        return false;

    // Enumerate device capabilities
    mCaps = cgRenderingCapabilities::createInstance( this );
    if ( !mCaps->enumerate( ) )
        return false;

    // Create the system required 'frame buffer' render target. In order to support multiple 
    // back buffers (i.e. triple buffering) we don't actually create a physical surface but instead 
    // rely on cgRenderTarget to perform some smart management.
    cgImageInfo Desc;
    Desc.width        = getScreenSize().width;
    Desc.height       = getScreenSize().height;
    Desc.mipLevels    = 1;
    Desc.type         = cgBufferType::DeviceBackBuffer;
    cgToDo( "DX11", "This should have a format that matches the actual frame buffer!" );
    Desc.format       = cgBufferFormat::R8G8B8A8;
    Desc.pool         = cgMemoryPool::Default;
    if ( !mResourceManager->createRenderTarget( &mDeviceFrameBuffer, Desc, 0, _T("Core::Device::FrameBuffer"), cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create internal device frame buffer render target on the selected device.\n") );
        return false;
    
    } // End if failed

    // Optionally create the system required 'depth stencil' surface. This /is/ actually a physical
    // surface that is created rather than using D3D's 'AutoDepthStencil' system.
    if ( mConfig.primaryDepthBuffer )
    {
        cgToDoAssert( "Carbon General", "Device depth stencil target needs to be re-created on reset." );
        Desc.type               = cgBufferType::DepthStencil;
        Desc.format             = cgBufferFormat::D24S8; // D24S8 is guaranteed to be supported (validated at initialization time).
        Desc.pool               = cgMemoryPool::Default;
        if ( !mResourceManager->createDepthStencilTarget( &mDeviceDepthStencilTarget, Desc, 0, _T("Core::Device::DepthStencil"), cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create internal device depth stencil target on the selected device.\n") );
            return false;

        } // End if failed
    
    } // End if request depth buffer

    // Create the primary driver render view (wraps frame buffer)
    mPrimaryView = new cgRenderView( this, _T("Core::Device::PrimaryRenderView"), true );
    if ( !mPrimaryView->initialize( ) )
        return false;

    // Create initial 'default' render view stack entry.
    mRenderViewStack.push( mPrimaryView );
    
    // Create / collect vertex formats used for rendering screen / clip space quads
    mWorldSpaceFormat  = cgVertexFormat::formatFromDeclarator( cgShadedVertex::Declarator );
    mScreenSpaceFormat = cgVertexFormat::formatFromDeclarator( cgScreenVertex::Declarator );
    mClipSpaceFormat   = cgVertexFormat::formatFromDeclarator( cgClipVertex::Declarator );

    // Setup default state objects. These state objects represent the default
    // values for the device irrespective of the API used (i.e. DX9 vs DX11).
    // Default values are selected by the default constructor for each 
    // description structure.
    cgBlendStateDesc BlendDesc;
    cgDepthStencilStateDesc DepthStencilDesc;
    cgRasterizerStateDesc RasterizerDesc;
    cgSamplerStateDesc SamplerDesc;
    if ( !(mResourceManager->createBlendState( &mDefaultBlendState, BlendDesc, 0, cgDebugSource() ) &&
           mResourceManager->createDepthStencilState( &mDefaultDepthStencilState, DepthStencilDesc, 0, cgDebugSource() ) &&
           mResourceManager->createRasterizerState( &mDefaultRasterizerState, RasterizerDesc, 0, cgDebugSource() ) &&
           mResourceManager->createSamplerState( &mDefaultSamplerState, SamplerDesc, 0, cgDebugSource() ) ) )
        return false;

    // Apply the default states in order to initialize the device 
    // with an appropriate starting configuration.
    setBlendState( mDefaultBlendState );
    setDepthStencilState( mDefaultDepthStencilState );
    setRasterizerState( mDefaultRasterizerState );
    for ( cgInt i = 0; i < MaxSamplerSlots; ++i )
        setSamplerState( i, mDefaultSamplerState );

    // Cache some rudimentary state configurations for internal rendering
    // procedures such as the drawing of screen elements (lines, rectangles, etc.)
    // Setup blending states first (basic alpha blending enabled).
    BlendDesc.renderTarget[0].blendEnable = true;
    BlendDesc.renderTarget[0].sourceBlend  = cgBlendMode::SrcAlpha;
    BlendDesc.renderTarget[0].destinationBlend = cgBlendMode::InvSrcAlpha;
    if ( !mResourceManager->createBlendState( &mElementBlendState, BlendDesc, 0, cgDebugSource() ) )
        return false;

    // Now depth stencil states (both disabled).
    DepthStencilDesc.depthEnable = false;
    DepthStencilDesc.depthWriteEnable = false;
    DepthStencilDesc.stencilEnable = false;
    if ( !mResourceManager->createDepthStencilState( &mElementDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;

    // ToDo: 6767 - Should this be a HARD failure? Should it just disable indirect lighting, or something graceful?
    //              Perhaps a hard failure only in DX11?
    // Create an initial target for virtual point light rendering (can be resized as needed)
	Desc.width     = 128;
	Desc.height    = 128;
    Desc.type      = cgBufferType::RenderTarget;
	Desc.format    = cgBufferFormat::R32G32B32A32_Float;
	Desc.mipLevels = 1;
	Desc.pool      = cgMemoryPool::Default;
	if ( !mResourceManager->createRenderTarget( &mVPLBuffer, Desc, 0, _T("VPL_Texture_Shared"), cgDebugSource() ) )
	{
        cgAppLog::write( cgAppLog::Error, _T("Failed to create internal render driver's shared VPL render target on the selected device.\n") );
        return false;
	
    } // End if failed
	
	// Create samplers for 2D image sampling
    mSampler2D0 = mResourceManager->createSampler( _T("Image2D0"), mDriverShaderHandle );
    mSampler2D1 = mResourceManager->createSampler( _T("Image2D1"), mDriverShaderHandle );
    SamplerDesc = cgSamplerStateDesc();
    SamplerDesc.minificationFilter = cgFilterMethod::Point;
    SamplerDesc.magnificationFilter = cgFilterMethod::Point;
    SamplerDesc.mipmapFilter = cgFilterMethod::None;
    SamplerDesc.addressU  = cgAddressingMode::Clamp;
    SamplerDesc.addressV  = cgAddressingMode::Clamp;
    SamplerDesc.addressW  = cgAddressingMode::Clamp;
	mSampler2D0->setStates( SamplerDesc );
	mSampler2D1->setStates( SamplerDesc );

    // Create a disabled depth state for screen rendering
    DepthStencilDesc = cgDepthStencilStateDesc();
    DepthStencilDesc.depthEnable      = false;
    DepthStencilDesc.depthWriteEnable = false;
    if ( !mResourceManager->createDepthStencilState( &mDisabledDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;

    // Setup default vertex shader (there must always be one).
    mDriverShader->selectVertexShader( _T("defaultVertexShader") );

    // Window is now active
    mActive = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : initShaderSystem () (Private, Virtual)
/// <summary>
/// Initialize the shader system by loading and enumerating the system export
/// scripts such that we can now bind render driver data / states to any
/// generated surface / vertex / pixel shader at runtime.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::initShaderSystem()
{
    // First, attempt to load the primary system export shader script 
    // in which all system provided constant buffer values and configuration
    // parameters are defined.
    if ( !mResourceManager->loadSurfaceShaderScript( &mSystemExportScript, _T("sys://Shaders/SystemDefs.shh"), 0, cgDebugSource() ) )
    {
        // Print error and return
        cgAppLog::write( cgAppLog::Error, _T("Failed to create shader communication layer on the selected device. The application will now exit.\n") );
        return false;
    
    } // End if failed

    // Instantiate a common 'SystemExports' script object through which
    // system properties can be communicated to all surface shaders.
    cgScript * pScript = mSystemExportScript.getResource( true );
    mSystemExports = pScript->createObjectInstance( _T("SystemExports") );
    if ( !mSystemExports )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to create shader communication layer on the selected device. The application will now exit.\n") );
        return false;
    
    } // End if failed

    // Extract addresses of script side system export variables.
    // These variables are mostly used to control shader permutation selection.
    if ( !(mSystemExportVars.renderFlags = (cgInt32*)mSystemExports->getAddressOfMember( _T("renderFlags") )) ||
		 !(mSystemExportVars.materialFlags = (cgInt32*)mSystemExports->getAddressOfMember( _T("materialFlags") )) ||
		 !(mSystemExportVars.lightFlags = (cgInt32*)mSystemExports->getAddressOfMember( _T("lightFlags") )) ||
         !(mSystemExportVars.shadingQuality = (cgInt32*)mSystemExports->getAddressOfMember( _T("shadingQuality") )) ||
		 !(mSystemExportVars.postProcessQuality = (cgInt32*)mSystemExports->getAddressOfMember( _T("postProcessQuality") )) ||
		 !(mSystemExportVars.antiAliasingQuality = (cgInt32*)mSystemExports->getAddressOfMember( _T("antiAliasingQuality") )) ||
		 !(mSystemExportVars.outputEncodingType = (cgInt32*)mSystemExports->getAddressOfMember( _T("outputEncodingType") )) ||
         !(mSystemExportVars.fogModel = (cgInt32*)mSystemExports->getAddressOfMember( _T("fogModel") )) ||
		 !(mSystemExportVars.orthographicCamera = (bool*)mSystemExports->getAddressOfMember( _T("orthographicCamera") )) ||
         !(mSystemExportVars.hdrLighting = (bool*)mSystemExports->getAddressOfMember( _T("hdrLighting") )) ||
         !(mSystemExportVars.viewSpaceLighting = (bool*)mSystemExports->getAddressOfMember( _T("viewSpaceLighting") )) ||
         !(mSystemExportVars.deferredRendering = (bool*)mSystemExports->getAddressOfMember( _T("deferredRendering") )) ||
         !(mSystemExportVars.deferredLighting = (bool*)mSystemExports->getAddressOfMember( _T("deferredLighting") )) ||
         !(mSystemExportVars.specularColorOutput = (bool*)mSystemExports->getAddressOfMember( _T("specularColorOutput") )) ||
         !(mSystemExportVars.pGBufferSRGB = (bool*)mSystemExports->getAddressOfMember( _T("gBufferSRGB") )) ||
         !(mSystemExportVars.colorWrites = (cgInt32*)mSystemExports->getAddressOfMember( _T("colorWrites") )) ||
         !(mSystemExportVars.cullMode = (cgInt32*)mSystemExports->getAddressOfMember( _T("cullMode") )) ||

		 !(mSystemExportVars.packedDepth = (bool*)mSystemExports->getAddressOfMember( _T("packedDepth") )) ||
         !(mSystemExportVars.nonLinearZ = (bool*)mSystemExports->getAddressOfMember( _T("nonLinearZ") )) ||
         !(mSystemExportVars.normalizedDistance = (bool*)mSystemExports->getAddressOfMember( _T("normalizedDistance") )) ||
         !(mSystemExportVars.surfaceNormals = (bool*)mSystemExports->getAddressOfMember( _T("surfaceNormals") )) ||
		 !(mSystemExportVars.depthStencilReads = (bool*)mSystemExports->getAddressOfMember( _T("depthStencilReads") )) ||

		 !(mSystemExportVars.maximumBlendIndex = (cgInt32*)mSystemExports->getAddressOfMember( _T("maxBlendIndex") )) ||
         !(mSystemExportVars.useVTFBlending = (bool*)mSystemExports->getAddressOfMember( _T("useVTFBlending") )) ||
		 !(mSystemExportVars.depthType = (cgInt32*)mSystemExports->getAddressOfMember( _T("depthType") )) ||
         !(mSystemExportVars.surfaceNormalType = (cgInt32*)mSystemExports->getAddressOfMember( _T("surfaceNormalType") )) ||

		 !(mSystemExportVars.lightType = (cgInt32*)mSystemExports->getAddressOfMember( _T("lightType") )) ||
         !(mSystemExportVars.shadowMethod = (cgInt32*)mSystemExports->getAddressOfMember( _T("shadowMethod") )) ||
         !(mSystemExportVars.primaryTaps = (cgInt32*)mSystemExports->getAddressOfMember( _T("primaryTaps") )) ||
         !(mSystemExportVars.secondaryTaps = (cgInt32*)mSystemExports->getAddressOfMember( _T("secondaryTaps") )) ||
         !(mSystemExportVars.sampleAttenuation = (bool*)mSystemExports->getAddressOfMember( _T("attenuationTexture") )) ||
         !(mSystemExportVars.sampleDistanceAttenuation = (bool*)mSystemExports->getAddressOfMember( _T("distanceAttenuationTexture") )) ||
         !(mSystemExportVars.colorTexture3D = (bool*)mSystemExports->getAddressOfMember( _T("colorTexture3D") )) ||
		 !(mSystemExportVars.diffuseIBL = (bool*)mSystemExports->getAddressOfMember( _T("diffuseIBL") )) ||
		 !(mSystemExportVars.specularIBL = (bool*)mSystemExports->getAddressOfMember( _T("specularIBL") )) ||
		 !(mSystemExportVars.computeAmbient = (bool*)mSystemExports->getAddressOfMember( _T("computeAmbient") )) ||
		 !(mSystemExportVars.computeDiffuse = (bool*)mSystemExports->getAddressOfMember( _T("computeDiffuse") )) ||
		 !(mSystemExportVars.computeSpecular = (bool*)mSystemExports->getAddressOfMember( _T("computeSpecular") )) ||
		 !(mSystemExportVars.useSSAO = (bool*)mSystemExports->getAddressOfMember( _T("useSSAO") )) ||
		 !(mSystemExportVars.trilighting = (bool*)mSystemExports->getAddressOfMember( _T("trilighting") )) ||

		 !(mSystemExportVars.normalSource = (cgInt32*)mSystemExports->getAddressOfMember( _T("normalSource") )) ||
         !(mSystemExportVars.reflectionMode = (cgInt32*)mSystemExports->getAddressOfMember( _T("reflectionMode") )) ||
         !(mSystemExportVars.lightTextureType = (cgInt32*)mSystemExports->getAddressOfMember( _T("lightTextureType") )) ||
         !(mSystemExportVars.sampleDiffuseTexture = (bool*)mSystemExports->getAddressOfMember( _T("sampleDiffuseTexture") )) ||

         !(mSystemExportVars.sampleSpecularColor = (bool*)mSystemExports->getAddressOfMember( _T("sampleSpecularColor") )) ||
         !(mSystemExportVars.sampleSpecularMask = (bool*)mSystemExports->getAddressOfMember( _T("sampleSpecularMask") )) ||
         !(mSystemExportVars.sampleGlossTexture = (bool*)mSystemExports->getAddressOfMember( _T("sampleGlossTexture") )) ||

         !(mSystemExportVars.sampleEmissiveTexture = (bool*)mSystemExports->getAddressOfMember( _T("sampleEmissiveTexture") )) ||
         !(mSystemExportVars.sampleOpacityTexture = (bool*)mSystemExports->getAddressOfMember( _T("sampleOpacityTexture") )) ||
         !(mSystemExportVars.pDecodeSRGB = (bool*)mSystemExports->getAddressOfMember( _T("decodeSRGB") )) ||
         !(mSystemExportVars.computeToksvig = (bool*)mSystemExports->getAddressOfMember( _T("computeToksvig") )) ||
         !(mSystemExportVars.correctNormals = (bool*)mSystemExports->getAddressOfMember( _T("correctNormals") )) ||
   		 !(mSystemExportVars.opacityInDiffuse = (bool*)mSystemExports->getAddressOfMember( _T("opacityInDiffuse") )) ||
   		 !(mSystemExportVars.surfaceFresnel = (bool*)mSystemExports->getAddressOfMember( _T("surfaceFresnel") )) ||
   		 !(mSystemExportVars.metal = (bool*)mSystemExports->getAddressOfMember( _T("metal") )) ||
   		 !(mSystemExportVars.transmissive = (bool*)mSystemExports->getAddressOfMember( _T("transmissive") )) ||
         !(mSystemExportVars.translucent = (bool*)mSystemExports->getAddressOfMember( _T("translucent") )) ||
         !(mSystemExportVars.emissive = (bool*)mSystemExports->getAddressOfMember( _T("emissive") )) ||
		 !(mSystemExportVars.alphaTest = (bool*)mSystemExports->getAddressOfMember( _T("alphaTest") )) ||

		 !(mSystemExportVars.objectRenderClass = (int*)mSystemExports->getAddressOfMember( _T("objectRenderClass") )) )

    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to access one or more required shader system variables.\n") );
        return false;

    } // End if failed

    // Now load the render driver internal surface rendering shader. This is used largely
    // simply to define a number of vertex and pixel shaders required by internal render 
    // driver features.
    if ( !mResourceManager->createSurfaceShader( &mDriverShaderHandle, _T("sys://Shaders/RenderDriver.sh"), 0, cgDebugSource() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to load required system render driver surface shader. See previous errors for more information. The application will now exit.\n") );
        return false;
    
    } // End if failed
    mDriverShader = mDriverShaderHandle.getResource(true);

    // In sandbox mode, the sandbox surface shader is available for additional rendering tasks.
    if ( cgGetSandboxMode() != cgSandboxMode::Disabled )
    {
        if ( !mResourceManager->createSurfaceShader( &mSandboxShader, _T("sys://Shaders/SandboxElements.sh"), 0, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to load required sandbox mode surface shader. See previous errors for more information. The application will now exit.\n") );
            return false;

        } // End if failed
        if ( !mResourceManager->createConstantBuffer( &mSandboxConstants, mSandboxShader, _T("cbSandboxData"), cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create required sandbox mode constant buffer. See previous errors for more information. The application will now exit.\n") );
            return false;

        } // End if failed

    } // End if sandbox mode

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getCapabilities ()
/// <summary>
/// Retrieve the capabilities descriptor for this render driver.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderingCapabilities * cgRenderDriver::getCapabilities( ) const
{
    return mCaps;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxSurfaceShader ()
/// <summary>
/// Get the sandbox mode rendering utility shader.
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgRenderDriver::getSandboxSurfaceShader( ) const
{
    return mSandboxShader;
}

//-----------------------------------------------------------------------------
//  Name : getSandboxConstantBuffer ()
/// <summary>
/// Get the sandbox mode rendering utility constants.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferHandle cgRenderDriver::getSandboxConstantBuffer( ) const
{
    return mSandboxConstants;
}

//-----------------------------------------------------------------------------
//  Name : createRenderView ()
/// <summary>
/// Create a new rendering view with the proposed layout.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView * cgRenderDriver::createRenderView( const cgString & strName, const cgRect & Layout )
{
    return createRenderView( strName, cgScaleMode::Absolute, cgRectF( (cgFloat)Layout.left, (cgFloat)Layout.top, (cgFloat)Layout.right, (cgFloat)Layout.bottom ) );
}

//-----------------------------------------------------------------------------
//  Name : createRenderView ()
/// <summary>
/// Create a new rendering view with the proposed layout.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView * cgRenderDriver::createRenderView( const cgString & strName, cgScaleMode::Base ScaleMode, const cgRectF & Layout )
{
    // A view with this name already exists?
    NamedRenderViewMap::const_iterator itView = mRenderViews.find( strName );
    if ( itView != mRenderViews.end() )
    {
        cgAppLog::write( cgAppLog::Error, _T("A render view with the name '%s' has already been allocated. Views must have unique names.\n"), strName.c_str() );
        return CG_NULL;
    
    } // End if duplicate

    // Create the new render view. It will be automatically added
    // to the render view map in the cgRenderView constructor.
    cgRenderView * pNewView = new cgRenderView( this, strName, false );
    if ( !pNewView->initialize( ScaleMode, Layout ) )
    {
        pNewView->deleteReference();
        return CG_NULL;
    
    } // End if failed
    return pNewView;
}

//-----------------------------------------------------------------------------
//  Name : destroyRenderView ()
/// <summary>
/// Destroy an existing rendering view.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::destroyRenderView( const cgString & strName )
{
    // A view with this name exists?
    NamedRenderViewMap::const_iterator itView = mRenderViews.find( strName );
    if ( itView == mRenderViews.end() )
        return;
    
    // Dispose of the view. Disposal method will automatically
    // remove it from the render view map.
    itView->second->scriptSafeDispose();
}

//-----------------------------------------------------------------------------
//  Name : getRenderView ()
/// <summary>
/// Retrieve an existing rendering view.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView * cgRenderDriver::getRenderView( const cgString & strName )
{
    // A view with this name exists?
    NamedRenderViewMap::const_iterator itView = mRenderViews.find( strName );
    if ( itView == mRenderViews.end() )
        return CG_NULL;
    return itView->second;
}

//-----------------------------------------------------------------------------
//  Name : getActiveRenderView ()
/// <summary>
/// Get the most recently activated render view (cgRenderView::begin()).
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView * cgRenderDriver::getActiveRenderView( )
{
    return mRenderViewStack.top();
}

/*//-----------------------------------------------------------------------------
//  Name : CacheParameterHandles () (Private)
/// <summary>
/// Search for all required shared parameter handles within the core effect.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::CacheParameterHandles()
{
     // Store the handles for any important matrix parameters
    bool bError = false;
	bError |= (m_MatrixParams.MatrixWorld           = m_pCoreEffect->GetParameterByName( _T("WorldMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixWorldInvTrans   = m_pCoreEffect->GetParameterByName( _T("WorldInverseTransposeMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixView            = m_pCoreEffect->GetParameterByName( _T("ViewMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixInvView         = m_pCoreEffect->GetParameterByName( _T("InverseViewMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixInvProj         = m_pCoreEffect->GetParameterByName( _T("InverseProjectionMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixProjection      = m_pCoreEffect->GetParameterByName( _T("ProjectionMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixWorldView       = m_pCoreEffect->GetParameterByName( _T("WorldViewMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixViewProj        = m_pCoreEffect->GetParameterByName( _T("ViewProjectionMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixInvViewProj     = m_pCoreEffect->GetParameterByName( _T("InverseViewProjectionMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixWorldViewProj   = m_pCoreEffect->GetParameterByName( _T("WorldViewProjectionMatrix") )) < 0;
    bError |= (m_MatrixParams.MatrixWorldArray      = m_pCoreEffect->GetParameterByName( _T("WorldMatrices") )) < 0;
    bError |= (m_MatrixParams.MatrixScreenTexProj   = m_pCoreEffect->GetParameterByName( _T("ScreenTexProjMatrix") )) < 0;
    bError |= (m_MatrixParams.MaximumBlendIndex         = m_pCoreEffect->GetParameterByName( _T("System_MaxBoneIndex") )) < 0;

    // Conditional handles
    if ( m_bBlendedInverseTranspose == true )
        bError |= (m_MatrixParams.MatrixWorldITArray = m_pCoreEffect->GetParameterByName( _T("WorldITMatrices") )) < 0;

    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required shared matrix parameters. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Also retrieve handles for important material parameters
    bError = false;
    bError |= (m_MaterialParams.MaterialAmbient             = m_pCoreEffect->GetParameterByName( _T("MaterialAmbient") )) < 0;
    bError |= (m_MaterialParams.MaterialDiffuse             = m_pCoreEffect->GetParameterByName( _T("MaterialDiffuse") )) < 0;
    bError |= (m_MaterialParams.MaterialSpecular            = m_pCoreEffect->GetParameterByName( _T("MaterialSpecular") )) < 0;
    bError |= (m_MaterialParams.MaterialEmissive            = m_pCoreEffect->GetParameterByName( _T("MaterialEmissive") )) < 0;
    bError |= (m_MaterialParams.MaterialPower               = m_pCoreEffect->GetParameterByName( _T("MaterialPower") )) < 0;
    bError |= (m_MaterialParams.MaterialTransmission        = m_pCoreEffect->GetParameterByName( _T("MaterialTransmission") )) < 0;
    bError |= (m_MaterialParams.MaterialMetalness           = m_pCoreEffect->GetParameterByName( _T("MaterialMetalness") )) < 0;
    bError |= (m_MaterialParams.MaterialRimLight            = m_pCoreEffect->GetParameterByName( _T("MaterialRimLight") )) < 0;
    bError |= (m_MaterialParams.MaterialReflection          = m_pCoreEffect->GetParameterByName( _T("MaterialReflection") )) < 0;
    bError |= (m_MaterialParams.MaterialReflectionType      = m_pCoreEffect->GetParameterByName( _T("System_ReflectionType") )) < 0;
    bError |= (m_MaterialParams.MaterialFresnelExponent     = m_pCoreEffect->GetParameterByName( _T("MaterialFresnelExponent") )) < 0;
    bError |= (m_MaterialParams.MaterialFresnel             = m_pCoreEffect->GetParameterByName( _T("MaterialFresnel") )) < 0;
    bError |= (m_MaterialParams.MaterialOpacityMapStrength  = m_pCoreEffect->GetParameterByName( _T("MaterialOpacityMapStrength") )) < 0;
    
    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required shared material parameters. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Retrieve camera related parameters
    bError = false;
    bError |= (m_CameraParams.CameraPosition      = m_pCoreEffect->GetParameterByName( _T("CameraPosition") )) < 0;
    bError |= (m_CameraParams.CameraDirection     = m_pCoreEffect->GetParameterByName( _T("CameraDirection") )) < 0;
    bError |= (m_CameraParams.CameraNear          = m_pCoreEffect->GetParameterByName( _T("CameraNear") )) < 0;
    bError |= (m_CameraParams.CameraFar           = m_pCoreEffect->GetParameterByName( _T("CameraFar") )) < 0;
    bError |= (m_CameraParams.CameraMaxDistance   = m_pCoreEffect->GetParameterByName( _T("CameraMaxDistance") )) < 0;
    bError |= (m_CameraParams.ViewportOffset      = m_pCoreEffect->GetParameterByName( _T("ViewportOffset") )) < 0;
    bError |= (m_CameraParams.ViewportSize        = m_pCoreEffect->GetParameterByName( _T("ViewportSize") )) < 0;
    bError |= (m_CameraParams.TargetSize          = m_pCoreEffect->GetParameterByName( _T("TargetSize") )) < 0;
    
    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required shared camera parameters. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Retrieve lighting related parameters
    bError = false;
    bError |= (m_LightingParams.NumLights              = m_pCoreEffect->GetParameterByName( _T("System_NumLights") )) < 0;
    bError |= (m_LightingParams.LightType              = m_pCoreEffect->GetParameterByName( _T("System_LightType0") )) < 0;
    bError |= (m_LightingParams.LightData              = m_pCoreEffect->GetParameterByName( _T("Lights[0]") )) < 0;
    bError |= (m_LightingParams.ShadowMapType          = m_pCoreEffect->GetParameterByName( _T("System_ShadowType") )) < 0;
    bError |= (m_LightingParams.HardwareShadows        = m_pCoreEffect->GetParameterByName( _T("System_HardwareShadows") )) < 0;
    bError |= (m_LightingParams.FilterShadows          = m_pCoreEffect->GetParameterByName( _T("System_FilterShadows") )) < 0;
    bError |= (m_LightingParams.ViewMatrix             = m_pCoreEffect->GetParameterByName( _T("LightViewMatrix") )) < 0;
    bError |= (m_LightingParams.ProjMatrix             = m_pCoreEffect->GetParameterByName( _T("LightProjMatrix") )) < 0;
    bError |= (m_LightingParams.ViewProjMatrix         = m_pCoreEffect->GetParameterByName( _T("LightViewProjectionMatrix") )) < 0;		
    bError |= (m_LightingParams.TexProjMatrix          = m_pCoreEffect->GetParameterByName( _T("LightTexProjMatrix") )) < 0;		

    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required shared lighting parameters. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Retrieve all remaining misc params
    bError = false;
    bError |= (m_SystemParams.ElapsedTime = m_pCoreEffect->GetParameterByName( _T("ElapsedTime") )) < 0;
    bError |= (m_SystemParams.CurrentTime = m_pCoreEffect->GetParameterByName( _T("CurrentTime") )) < 0;
    
    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required shared system parameters. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Retrieve system defined techniques
    bError = false;
    bError |= (m_Techniques.DrawScreenElement = m_pCoreEffect->GetTechniqueID( _T("System_DrawScreenElement") )) < 0;
    bError |= (m_Techniques.DrawWorldElement  = m_pCoreEffect->GetTechniqueID( _T("System_DrawWorldElement") )) < 0;
    
    // Test for errors at this stage
    if ( bError )
    {
        cgAppLog::write( cgAppLog::Error, _T("Unable to access all required system defined rendering techniques. This error is fatal.\n") );
        return false;
    
    } // End if failed to create core exports effect

    // Success!
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : setSystemState ()
/// <summary>
/// Set the value of one of the system states, the values of which are shared
/// between all surface shader scripts for use (most commonly) in vertex and
/// pixel shader permutation selection or to control the rendering process.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setSystemState( cgSystemState::Base State, cgInt32 Value )
{
    cgAssert( isInitialized() == true );
    switch ( State )
    {
        case cgSystemState::ShadingQuality:
            *mSystemExportVars.shadingQuality = Value;
            mConfig.shadingQuality = Value; // Update config
            break;
		case cgSystemState::PostProcessQuality:
			*mSystemExportVars.postProcessQuality = Value;
            mConfig.postProcessQuality = Value; // Update config
			break;
		case cgSystemState::AntiAliasingQuality:
			*mSystemExportVars.antiAliasingQuality = Value;
            mConfig.antiAliasingQuality = Value; // Update config
			break;
        case cgSystemState::OutputEncodingType:
            *mSystemExportVars.outputEncodingType = Value;
            break;
        case cgSystemState::FogModel:
            *mSystemExportVars.fogModel = Value;
            break;
		case cgSystemState::OrthographicCamera:
            *mSystemExportVars.orthographicCamera = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  OrthographicCamera;
			else
				*mSystemExportVars.renderFlags &= ~OrthographicCamera;
            break;
        case cgSystemState::HDRLighting:
            *mSystemExportVars.hdrLighting = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  HDRLighting;
			else
				*mSystemExportVars.renderFlags &= ~HDRLighting;
			break;
        case cgSystemState::ViewSpaceLighting:
            *mSystemExportVars.viewSpaceLighting = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  ViewSpaceLighting;
			else
				*mSystemExportVars.renderFlags &= ~ViewSpaceLighting;
            break;
		case cgSystemState::DeferredRendering:
            *mSystemExportVars.deferredRendering = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  DeferredRendering;
			else
				*mSystemExportVars.renderFlags &= ~DeferredRendering;
            break;
		case cgSystemState::DeferredLighting:
            *mSystemExportVars.deferredLighting = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  DeferredLighting;
			else
				*mSystemExportVars.renderFlags &= ~DeferredLighting;
            break;
		case cgSystemState::SpecularColorOutput:
            *mSystemExportVars.specularColorOutput = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  SpecularColorOutput;
			else
				*mSystemExportVars.renderFlags &= ~SpecularColorOutput;
            break;
		case cgSystemState::GBufferSRGB:
            *mSystemExportVars.pGBufferSRGB = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  GBufferSRGB;
			else
				*mSystemExportVars.renderFlags &= ~GBufferSRGB;
            break;
		case cgSystemState::NonLinearZ:
            *mSystemExportVars.nonLinearZ = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  NonLinearZ;
			else
				*mSystemExportVars.renderFlags &= ~NonLinearZ;
            break;
		case cgSystemState::NormalizedDistance:
            *mSystemExportVars.normalizedDistance = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  NormalizedDistance;
			else
				*mSystemExportVars.renderFlags &= ~NormalizedDistance;
            break;
		case cgSystemState::SurfaceNormals:
            *mSystemExportVars.surfaceNormals = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  SurfaceNormals;
			else
				*mSystemExportVars.renderFlags &= ~SurfaceNormals;
            break;
		case cgSystemState::PackedDepth:
            *mSystemExportVars.packedDepth = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  PackedDepth;
			else
				*mSystemExportVars.renderFlags &= ~PackedDepth;
            break;
		case cgSystemState::DepthStencilReads:
			*mSystemExportVars.depthStencilReads = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.renderFlags |=  DepthStencilReads;
			else
				*mSystemExportVars.renderFlags &= ~DepthStencilReads;
			break;
		case cgSystemState::ColorWrites:
            *mSystemExportVars.colorWrites = Value;
            break;
		case cgSystemState::CullMode:
            *mSystemExportVars.cullMode = Value;
            break;


        case cgSystemState::MaximumBlendIndex:
            *mSystemExportVars.maximumBlendIndex = Value;
            break;
		case cgSystemState::DepthType:
            *mSystemExportVars.depthType = Value;
			switch( Value )
			{
				case cgDepthType::LinearDistance:
					setSystemState( cgSystemState::NormalizedDistance, true );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, false );
				    break;
				case cgDepthType::LinearDistance_Packed:
					setSystemState( cgSystemState::NormalizedDistance, true );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, true );
				    break;
				case cgDepthType::LinearZ:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, false );
				    break;
				case cgDepthType::LinearZ_Packed:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, true );
				    break;
				case cgDepthType::NonLinearZ:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, true );
					setSystemState( cgSystemState::PackedDepth, false );
				    break;
				case cgDepthType::NonLinearZ_Packed:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, true );
					setSystemState( cgSystemState::PackedDepth, true );
				    break;
				case cgDepthType::LinearZ_Normal_Packed:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, true );
					setSystemState( cgSystemState::SurfaceNormals, true );
				    break;
				default:
					setSystemState( cgSystemState::NormalizedDistance, false );
					setSystemState( cgSystemState::NonLinearZ, false );
					setSystemState( cgSystemState::PackedDepth, false );
				    break;
			};
            break;
        case cgSystemState::SurfaceNormalType:
            *mSystemExportVars.surfaceNormalType = Value;
			if ( Value > 0 )
				setSystemState( cgSystemState::SurfaceNormals, true );
			else
				setSystemState( cgSystemState::SurfaceNormals, false );
			break;
		case cgSystemState::LightType:
            *mSystemExportVars.lightType = Value;
            break;
        case cgSystemState::ShadowMethod:
            *mSystemExportVars.shadowMethod = Value;
            break;
		case cgSystemState::PrimaryTaps:
            *mSystemExportVars.primaryTaps = Value;
            break;
		case cgSystemState::SecondaryTaps:
            *mSystemExportVars.secondaryTaps = Value;
            break;
        case cgSystemState::ColorTexture3D:
            *mSystemExportVars.colorTexture3D = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  ColorTexture3D;
			else
				*mSystemExportVars.lightFlags &= ~ColorTexture3D;
			break;

        case cgSystemState::SampleAttenuation:
            *mSystemExportVars.sampleAttenuation = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  AttenuationTexture;
			else
				*mSystemExportVars.lightFlags &= ~AttenuationTexture;
            break;
        case cgSystemState::SampleDistanceAttenuation:
            *mSystemExportVars.sampleDistanceAttenuation = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  DistanceAttenuationTexture;
			else
				*mSystemExportVars.lightFlags &= ~DistanceAttenuationTexture;
            break;
		case cgSystemState::DiffuseIBL:
            *mSystemExportVars.diffuseIBL = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  DiffuseIBL;
			else
				*mSystemExportVars.lightFlags &= ~DiffuseIBL;
			break;
		case cgSystemState::SpecularIBL:
            *mSystemExportVars.specularIBL = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  SpecularIBL;
			else
				*mSystemExportVars.lightFlags &= ~SpecularIBL;
			break;
		case cgSystemState::ComputeAmbient:
            *mSystemExportVars.computeAmbient = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  ComputeAmbient;
			else
				*mSystemExportVars.lightFlags &= ~ComputeAmbient;
			break;
		case cgSystemState::ComputeDiffuse:
            *mSystemExportVars.computeDiffuse = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  ComputeDiffuse;
			else
				*mSystemExportVars.lightFlags &= ~ComputeDiffuse;
			break;
		case cgSystemState::ComputeSpecular:
            *mSystemExportVars.computeSpecular = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  ComputeSpecular;
			else
				*mSystemExportVars.lightFlags &= ~ComputeSpecular;
			break;
		case cgSystemState::UseSSAO:
            *mSystemExportVars.useSSAO = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  UseSSAO;
			else
				*mSystemExportVars.lightFlags &= ~UseSSAO;
			break;
		case cgSystemState::Trilighting:
            *mSystemExportVars.trilighting = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.lightFlags |=  Trilighting;
			else
				*mSystemExportVars.lightFlags &= ~Trilighting;
			break;

		case cgSystemState::NormalSource:
            *mSystemExportVars.normalSource = Value;
            break;
        case cgSystemState::ReflectionMode:
            *mSystemExportVars.reflectionMode = Value;
            break;
        case cgSystemState::LightTextureType:
            *mSystemExportVars.lightTextureType = Value;
            break;
		case cgSystemState::SampleDiffuseTexture:
            *mSystemExportVars.sampleDiffuseTexture = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleDiffuseTexture;
			else
				*mSystemExportVars.materialFlags &= ~SampleDiffuseTexture;
            break;

		case cgSystemState::SampleSpecularColor:
            *mSystemExportVars.sampleSpecularColor = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleSpecularColor;
			else
				*mSystemExportVars.materialFlags &= ~SampleSpecularColor;
            break;
		case cgSystemState::SampleSpecularMask:
            *mSystemExportVars.sampleSpecularMask = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleSpecularMask;
			else
				*mSystemExportVars.materialFlags &= ~SampleSpecularMask;
            break;
		case cgSystemState::SampleGlossTexture:
            *mSystemExportVars.sampleGlossTexture = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleGlossTexture;
			else
				*mSystemExportVars.materialFlags &= ~SampleGlossTexture;
            break;

        case cgSystemState::SampleEmissiveTexture:
            *mSystemExportVars.sampleEmissiveTexture = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleEmissiveTexture;
			else
				*mSystemExportVars.materialFlags &= ~SampleEmissiveTexture;
            break;
        case cgSystemState::SampleOpacityTexture:
            *mSystemExportVars.sampleOpacityTexture = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SampleOpacityTexture;
			else
				*mSystemExportVars.materialFlags &= ~SampleOpacityTexture;
            break;
		case cgSystemState::Emissive:
            *mSystemExportVars.emissive = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  Emissive;
			else
				*mSystemExportVars.materialFlags &= ~Emissive;
            break;
		case cgSystemState::DecodeSRGB:
            *mSystemExportVars.pDecodeSRGB = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  DecodeSRGB;
			else
				*mSystemExportVars.materialFlags &= ~DecodeSRGB;
            break;
		case cgSystemState::ComputeToksvig:
            *mSystemExportVars.computeToksvig = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  ComputeToksvig;
			else
				*mSystemExportVars.materialFlags &= ~ComputeToksvig;
            break;
		case cgSystemState::CorrectNormals:
            *mSystemExportVars.correctNormals = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  CorrectNormals;
			else
				*mSystemExportVars.materialFlags &= ~CorrectNormals;
            break;
		case cgSystemState::OpacityInDiffuse:
            *mSystemExportVars.opacityInDiffuse = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  OpacityInDiffuse;
			else
				*mSystemExportVars.materialFlags &= ~OpacityInDiffuse;
            break;
        case cgSystemState::SurfaceFresnel:
            *mSystemExportVars.surfaceFresnel = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  SurfaceFresnel;
			else
				*mSystemExportVars.materialFlags &= ~SurfaceFresnel;
            break;
        case cgSystemState::Metal:
            *mSystemExportVars.metal = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  Metal;
			else
				*mSystemExportVars.materialFlags &= ~Metal;
            break;
		case cgSystemState::Transmissive:
            *mSystemExportVars.transmissive = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  Transmissive;
			else
				*mSystemExportVars.materialFlags &= ~Transmissive;
            break;
		case cgSystemState::Translucent:
            *mSystemExportVars.translucent = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  Translucent;
			else
				*mSystemExportVars.materialFlags &= ~Translucent;
            break;
		case cgSystemState::AlphaTest:
            *mSystemExportVars.alphaTest = (Value != 0);
			if ( Value > 0 )
				*mSystemExportVars.materialFlags |=  AlphaTest;
			else
				*mSystemExportVars.materialFlags &= ~AlphaTest;
            break;

		case cgSystemState::ObjectRenderClass:
			*mSystemExportVars.objectRenderClass = Value;
			break;
		
		default:
            return false;

    } // End Switch
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSystemState ()
/// <summary>
/// Gets the value of one of the system states, the values of which are shared
/// between all surface shader scripts for use (most commonly) in vertex and
/// pixel shader permutation selection or to control the rendering process.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgRenderDriver::getSystemState( cgSystemState::Base State )
{
    cgAssert( isInitialized() == true );
    cgInt32 Value = 0;
    switch ( State )
    {
        case cgSystemState::ShadingQuality:
            Value = *mSystemExportVars.shadingQuality;
            break;
		case cgSystemState::PostProcessQuality:
			Value = *mSystemExportVars.postProcessQuality;
			break;
		case cgSystemState::AntiAliasingQuality:
			Value = *mSystemExportVars.antiAliasingQuality;
			break;
		case cgSystemState::OutputEncodingType:
            Value = *mSystemExportVars.outputEncodingType;
            break;
        case cgSystemState::FogModel:
            Value = *mSystemExportVars.fogModel;
            break;
        case cgSystemState::OrthographicCamera:
            Value = (*mSystemExportVars.orthographicCamera != 0);
            break;
        case cgSystemState::HDRLighting:
            Value = (*mSystemExportVars.hdrLighting != 0);
            break;
		case cgSystemState::ViewSpaceLighting:
            Value = (*mSystemExportVars.viewSpaceLighting != 0);
            break;
		case cgSystemState::DeferredRendering:
            Value = (*mSystemExportVars.deferredRendering != 0);
            break;
		case cgSystemState::DeferredLighting:
            Value = (*mSystemExportVars.deferredLighting != 0);
            break;
		case cgSystemState::SpecularColorOutput:
            Value = (*mSystemExportVars.specularColorOutput != 0);
            break;
		case cgSystemState::GBufferSRGB:
            Value = (*mSystemExportVars.pGBufferSRGB != 0);
            break;
		case cgSystemState::NonLinearZ:
            Value = (*mSystemExportVars.nonLinearZ != 0);
            break;
		case cgSystemState::NormalizedDistance:
            Value = (*mSystemExportVars.normalizedDistance != 0);
            break;
		case cgSystemState::SurfaceNormals:
            Value = (*mSystemExportVars.surfaceNormals != 0);
            break;
		case cgSystemState::PackedDepth:
            Value = (*mSystemExportVars.packedDepth != 0);
            break;
		case cgSystemState::DepthStencilReads:
			Value = (*mSystemExportVars.depthStencilReads != 0);
			break;
		case cgSystemState::ColorWrites:
            Value = *mSystemExportVars.colorWrites;
            break;
        case cgSystemState::CullMode:
            Value = *mSystemExportVars.cullMode;
            break;

		case cgSystemState::MaximumBlendIndex:
            Value = *mSystemExportVars.maximumBlendIndex;
            break;
		case cgSystemState::DepthType:
            Value = *mSystemExportVars.depthType;
            break;
        case cgSystemState::SurfaceNormalType:
            Value = *mSystemExportVars.surfaceNormalType;
            break;

        case cgSystemState::LightType:
            Value = *mSystemExportVars.lightType;
            break;
        case cgSystemState::ShadowMethod:
            Value = *mSystemExportVars.shadowMethod;
            break;
        case cgSystemState::PrimaryTaps:
            Value = *mSystemExportVars.primaryTaps;
            break;
        case cgSystemState::SecondaryTaps:
            Value = *mSystemExportVars.secondaryTaps;
            break;

        case cgSystemState::SampleAttenuation:
            Value = (*mSystemExportVars.sampleAttenuation != 0);
            break;
        case cgSystemState::SampleDistanceAttenuation:
            Value = (*mSystemExportVars.sampleDistanceAttenuation != 0);
            break;
		case cgSystemState::ColorTexture3D:
            Value = (*mSystemExportVars.colorTexture3D != 0);
            break;
		case cgSystemState::DiffuseIBL:
            Value = (*mSystemExportVars.diffuseIBL != 0);
			break;
		case cgSystemState::SpecularIBL:
            Value = (*mSystemExportVars.specularIBL != 0);
			break;
		case cgSystemState::ComputeAmbient:
            Value = (*mSystemExportVars.computeAmbient != 0);
			break;
		case cgSystemState::ComputeDiffuse:
            Value = (*mSystemExportVars.computeDiffuse != 0);
			break;
		case cgSystemState::ComputeSpecular:
            Value = (*mSystemExportVars.computeSpecular != 0);
			break;
		case cgSystemState::UseSSAO:
            Value = (*mSystemExportVars.useSSAO != 0);
			break;
		case cgSystemState::Trilighting:
            Value = (*mSystemExportVars.trilighting != 0);
			break;

		case cgSystemState::NormalSource:
            Value = *mSystemExportVars.normalSource;
            break;
        case cgSystemState::ReflectionMode:
            Value = *mSystemExportVars.reflectionMode;
            break;
        case cgSystemState::LightTextureType:
            Value = *mSystemExportVars.lightTextureType;
            break;
        case cgSystemState::CorrectNormals:
            Value = (*mSystemExportVars.correctNormals != 0);
            break;
		case cgSystemState::SampleDiffuseTexture:
            Value = (*mSystemExportVars.sampleDiffuseTexture != 0);
            break;
        case cgSystemState::SampleSpecularColor:
            Value = (*mSystemExportVars.sampleSpecularColor != 0);
            break;
        case cgSystemState::SampleSpecularMask:
            Value = (*mSystemExportVars.sampleSpecularMask != 0);
            break;
        case cgSystemState::SampleGlossTexture:
            Value = (*mSystemExportVars.sampleGlossTexture != 0);
            break;
        case cgSystemState::SampleEmissiveTexture:
            Value = (*mSystemExportVars.sampleEmissiveTexture != 0);
            break;
		case cgSystemState::SampleOpacityTexture:
            Value = (*mSystemExportVars.sampleOpacityTexture != 0);
            break;
        case cgSystemState::OpacityInDiffuse:
            Value = (*mSystemExportVars.opacityInDiffuse != 0);
            break;
        case cgSystemState::Translucent:
            Value = (*mSystemExportVars.translucent != 0);
            break;
        case cgSystemState::ComputeToksvig:
            Value = (*mSystemExportVars.computeToksvig != 0);
            break;
		case cgSystemState::DecodeSRGB:
            Value = (*mSystemExportVars.pDecodeSRGB != 0);
            break;
		case cgSystemState::Emissive:
            Value = (*mSystemExportVars.emissive != 0);
            break;
        case cgSystemState::SurfaceFresnel:
            Value = (*mSystemExportVars.surfaceFresnel != 0);
            break;
        case cgSystemState::Metal:
            Value = (*mSystemExportVars.metal != 0);
            break;
		case cgSystemState::Transmissive:
            Value = (*mSystemExportVars.transmissive != 0);
            break;
		case cgSystemState::AlphaTest:
            Value = (*mSystemExportVars.alphaTest != 0);
            break;

		case cgSystemState::ObjectRenderClass:
			Value = *mSystemExportVars.objectRenderClass;
			break;

        default:
            return 0;
    } // End Switch

    return Value;
}

//-----------------------------------------------------------------------------
//  Name : getVPLBufferDimensions ()
/// <summary>
/// Returns the current VPL buffer's active dimensions (i.e., the valid area
/// of the render target if vtf or the VB dimensions if r2vb)
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgRenderDriver::getVPLBufferDimensions( ) const
{
	return mCurrentVPLSize;
}

//-----------------------------------------------------------------------------
//  Name : getVPLTextureDimensions ()
/// <summary>
/// Returns the dimensions of the primary VPL texture/render target
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgRenderDriver::getVPLTextureDimensions( ) const
{
	cgTexture * pOutput = (cgTexture*)mVPLBuffer.getResourceSilent();
	if ( pOutput )
        return cgSize( pOutput->getInfo().width, pOutput->getInfo().height );

    // Return empty result.
    static const cgSize Empty( 0, 0 );
	return Empty;
}

//-----------------------------------------------------------------------------
//  Name : getVPLVertexBuffer ()
/// <summary>
/// Gets a vertex buffer of given dimensions for vertex texture fetches. Can
/// automatically create a new one if needed.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexBufferHandle cgRenderDriver::getVPLVertexBuffer( cgUInt32 nWidth, cgUInt32 nHeight, bool bAutoCreate )
{
	return getVPLVertexBuffer( cgSize( nWidth, nHeight ), bAutoCreate );
}

//-----------------------------------------------------------------------------
//  Name : getVPLVertexBuffer ()
/// <summary>
/// Gets a vertex buffer of given dimensions for vertex texture fetches. Can
/// automatically create a new one if needed.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexBufferHandle cgRenderDriver::getVPLVertexBuffer( const cgSize & Dimensions, bool bAutoCreate )
{
	cgVertexBufferHandle hOut;
	SizeVertexBufferMap::iterator it = mVPLVertexBuffers.find( Dimensions );
	if ( it != mVPLVertexBuffers.end() )
	{
        // Return existing buffer
		hOut = it->second;
	
    } // End if exists
	else
	{
        // Should we create one if it doesn't exist?
		if ( bAutoCreate )
		{
			if ( addVPLVertexBuffer( Dimensions.width, Dimensions.height ) )
			{
				it = mVPLVertexBuffers.find( Dimensions );
				hOut = it->second;
			
            } // End if success
		
        } // End if create?
	
    } // End if !exists
	return hOut;
}

//-----------------------------------------------------------------------------
//  Name : addVPLVertexBuffer ()
/// <summary>
/// Adds a new vertex buffer of given dimensions for virtual point lighting
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::addVPLVertexBuffer( cgUInt32 nWidth, cgUInt32 nHeight )
{
	return addVPLVertexBuffer( cgSize( nWidth, nHeight ) );
}

//-----------------------------------------------------------------------------
//  Name : addVPLVertexBuffer ()
/// <summary>
/// Adds a new vertex buffer of given dimensions for virtual point lighting
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::addVPLVertexBuffer( const cgSize & Dimensions )
{
	// If we already have a VB matching this width/height combo, we're done
	if ( mVPLVertexBuffers.find( Dimensions ) != mVPLVertexBuffers.end() )
		return true;

    // Create the vertex formats for rendering points
	cgVertexFormat * m_pPointVertexFormat = cgVertexFormat::formatFromDeclarator( cgPointVertex::Declarator );

	// Get the vertex stride
	cgUInt32 nStride = (cgUInt32)m_pPointVertexFormat->getStride();

    // Create a new vertex buffer and fill with "positions" (i.e., 2D coordinates)
	cgVertexBufferHandle hVertexBuffer;
    cgUInt32 nLength = Dimensions.width * Dimensions.height * nStride;
	if ( !mResourceManager->createVertexBuffer( &hVertexBuffer, nLength, cgBufferUsage::WriteOnly, m_pPointVertexFormat, cgMemoryPool::Managed ) )
    {
		cgAppLog::write( cgAppLog::Error, _T("Failed to create new VPL vertex buffer of dimensions %i x %i.\n"), Dimensions.width, Dimensions.height );
		return false;
	
    } // End if failed
        
    // Create a temporary vertex array for loading
    cgVertexBuffer * pVertexBuffer = hVertexBuffer.getResource(true);
    cgPointVertex * pVertices = new cgPointVertex[ Dimensions.width * Dimensions.height ];
    for ( cgInt32 v = 0; v < Dimensions.height; v++ )
    {
        for ( cgInt32 u = 0; u < Dimensions.width; u++ )
        {
            pVertices[ v * Dimensions.width + u ].position.x = (0.5f + (cgFloat)u) / (cgFloat)Dimensions.width;
            pVertices[ v * Dimensions.width + u ].position.y = (0.5f + (cgFloat)v) / (cgFloat)Dimensions.height;
            pVertices[ v * Dimensions.width + u ].position.z = 0.0f;
        
        } // Next column
    
    } // Next row

    // Upload vertices to the buffer
    if ( !pVertexBuffer->updateBuffer( 0, nLength, pVertices ) )
    {
        delete []pVertices;
        cgAppLog::write( cgAppLog::Error, _T("Failed to populate new VPL vertex buffer of dimensions %i x %i.\n"), Dimensions.width, Dimensions.height );
        return false;
    
    } // End if failed

    // Cleanup temporary memory
    delete []pVertices;

    // Add to table
	mVPLVertexBuffers[ Dimensions ] = hVertexBuffer;

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : backupSamplerStates ()
/// <summary>
/// Backup the currently applied sampler states.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::backupSamplerStates( cgUInt32 nStartIndex, cgUInt32 nEndIndex )
{
	for ( cgUInt32 i = nStartIndex; i <= nEndIndex; ++i )
		pushSamplerState( i, cgSamplerStateHandle::Null );
}

//-----------------------------------------------------------------------------
//  Name : backupTextures ()
/// <summary>
/// Backup the currently applied textures.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::backupTextures( cgUInt32 nStartIndex, cgUInt32 nEndIndex )
{
	for ( cgUInt32 i = nStartIndex; i <= nEndIndex; ++i )
		pushTexture( i, cgTextureHandle::Null );
}

//-----------------------------------------------------------------------------
//  Name : restoreSamplerStates ()
/// <summary>
/// Backup the currently applied sampler states.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::restoreSamplerStates( cgUInt32 nStartIndex, cgUInt32 nEndIndex )
{
	for ( cgUInt32 i = nStartIndex; i <= nEndIndex; ++i )
		popSamplerState( i );
}

//-----------------------------------------------------------------------------
//  Name : restoreTextures ()
/// <summary>
/// Backup the currently applied textures.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::restoreTextures( cgUInt32 nStartIndex, cgUInt32 nEndIndex )
{
	for ( cgUInt32 i = nStartIndex; i <= nEndIndex; ++i )
		popTexture( i );
}

//-----------------------------------------------------------------------------
//  Name : setRenderPass ()
/// <summary>
/// Replace the current rendering pass name with that specified. Can be
/// retrieved with getRenderPass()
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::setRenderPass( const cgString & strPass )
{
    cgAssert( mRenderPassStack.size() > 0 );
    mRenderPassStack.top() = strPass;
}

//-----------------------------------------------------------------------------
//  Name : pushRenderPass ()
/// <summary>
/// Backup the current render pass name and apply the newly specified
/// value. The prior name can be restored with a call to 'popRenderPass()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::pushRenderPass( const cgString & strPass )
{
    mRenderPassStack.push( strPass );
}

//-----------------------------------------------------------------------------
//  Name : popRenderPass ()
/// <summary>
/// Restore the render pass name to the state in which it existed
/// prior to the most recent call to 'pushRenderPass()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::popRenderPass( )
{
    // Should always be at least 1 render pass name in the stack.
    // This last name cannot be popped.
    cgAssert( mRenderPassStack.size() > 1 );
    mRenderPassStack.pop();
}

//-----------------------------------------------------------------------------
//  Name : getRenderPass ()
/// <summary>
/// Retrieve the current render pass name. Can be pushed with 
/// pushRenderPass() and popped with popRenderPass().
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgRenderDriver::getRenderPass( ) const
{
    // Should always be at least 1 render pass name in the stack (the default)
    cgAssert( mRenderPassStack.size() > 0 );
    return mRenderPassStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setOverrideMethod ()
/// <summary>
/// Set a rendering method override name. Can be retrieved with 
/// getOverrideMethod().
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::setOverrideMethod( const cgString & strMethod )
{
    cgAssert( mOverrideMethodStack.size() > 0 );
    mOverrideMethodStack.top() = strMethod;
}

//-----------------------------------------------------------------------------
//  Name : pushOverrideMethod ()
/// <summary>
/// Backup the current override method name and apply the newly specified
/// value. The prior name can be restored with a call to 'popOverrideMethod()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::pushOverrideMethod( const cgString & strMethod )
{
    mOverrideMethodStack.push( strMethod );
}

//-----------------------------------------------------------------------------
//  Name : popOverrideMethod ()
/// <summary>
/// Restore the override method name to the state in which it existed prior to 
/// the most recent call to 'pushOverrideMethod()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::popOverrideMethod( )
{
    // Should always be at least 1 render pass name in the stack.
    // This last name cannot be popped.
    cgAssert( mOverrideMethodStack.size() > 1 );
    mOverrideMethodStack.pop();
}

//-----------------------------------------------------------------------------
//  Name : getOverrideMethod ()
/// <summary>
/// Retrieve the current rendering method override. Can be pushed with 
/// pushOverrideMethod() and popped with popOverrideMethod().
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgRenderDriver::getOverrideMethod( ) const
{
    // Should always be at least 1 override method name in the stack.
    cgAssert( mOverrideMethodStack.size() > 0 );
    return mOverrideMethodStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setMaterial ()
/// <summary>
/// Setup the render driver resources (textures, material, effect ) based
/// on the material specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setMaterial( cgMaterialHandle hMaterial, bool bBypassFilter /* = false */ )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Guarantee that the material data is loaded at this point
    cgMaterial * pMaterial = hMaterial.getResource( true );

    // Record profiler data.
    cgProfiler * pProfiler = cgProfiler::getInstance();
    static const cgString strProfilerProcessSetMaterial = _T("setMaterial");
    pProfiler->beginProcess( strProfilerProcessSetMaterial );

    // Should the material be filtered?
    if ( !bBypassFilter )
    {
        cgFilterExpression * pFilter = mMaterialFilterStack.top();
        if ( pMaterial && pFilter && !pFilter->evaluate( pMaterial->getMaterialProperties() ) )
        {
            pProfiler->endProcess();
            return false;
        
        } // End if excluded
    
    } // End if use filtering

    // Was the resource available?
    if ( pMaterial )
    {
        // Retrieve the material's active surface shader. Single parameter
        // value of 'false' ensures that the material's "default" shader is 
        // returned if no user defined shader was assigned.
        cgSurfaceShaderHandle hShader = pMaterial->getSurfaceShader(false);
        cgSurfaceShader * pShader = hShader.getResource(true);
        if ( !pShader || !pShader->isLoaded() )
        {
            // Success!
            pProfiler->endProcess();
            return false;
        
        } // End if failed

        // Automatically select the correct surface shader 
        // technique for the current rendering pass.
        if ( !pShader->selectTechnique( getRenderPass() ) )
        {
            pProfiler->endProcess();
            return false;
        
        } // End if failed
        
        // Ask the material to apply its data.
        pMaterial->apply( this );

		// Store a reference to the resources (guarantees lifetime).
        mCurrentSurfaceShaderHandle = hShader;
        mCurrentSurfaceShader       = pShader;
        mCurrentMaterialHandle      = hMaterial;
        mCurrentMaterial            = pMaterial;

    } // End if material available
    else
    {
        // Clear what we can.
        mCurrentMaterialHandle.close();
        mCurrentMaterial = CG_NULL;
        mCurrentSurfaceShaderHandle.close();
        mCurrentSurfaceShader = CG_NULL;
    
    } // End if clear

    // Success!
    pProfiler->endProcess();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getMaterial ()
/// <summary>
/// Retrieve the handle to the current material.
/// </summary>
//-----------------------------------------------------------------------------
const cgMaterialHandle & cgRenderDriver::getMaterial( ) const
{
    return mCurrentMaterialHandle;
}

//-----------------------------------------------------------------------------
//  Name : setMaterialFilter ()
/// <summary>
/// Used to filter out materials that must / must not be drawn.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::setMaterialFilter( cgFilterExpression * pFilter )
{
    // There should always be at least one filter in the stack.
    cgAssert( mMaterialFilterStack.size() > 0 );

    // Clear out previous material filter data.
    delete mMaterialFilterStack.top();
    mMaterialFilterStack.top() = CG_NULL;

    // Duplicate specified filter
    if ( pFilter )
        mMaterialFilterStack.top() = new cgFilterExpression( pFilter );
}

//-----------------------------------------------------------------------------
//  Name : pushMaterialFilter ()
/// <summary>
/// Backup the current material filter settings and apply the newly specified
/// values. The prior settings can be restored with a call to 
/// 'popMaterialFilter()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::pushMaterialFilter( cgFilterExpression * pFilter )
{
    // Grow the stack and set the new element.
    mMaterialFilterStack.push(CG_NULL);
    setMaterialFilter( pFilter );
}

//-----------------------------------------------------------------------------
//  Name : popMaterialFilter ()
/// <summary>
/// Restore the material filter settings to the state in which they existed
/// prior to the most recent call to 'pushMaterialFilter()'.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::popMaterialFilter( )
{
    // Should always be at least 1 filter entry in the stack.
    cgAssert( mMaterialFilterStack.size() > 1 );
    delete mMaterialFilterStack.top();
    mMaterialFilterStack.pop();
}

//-----------------------------------------------------------------------------
//  Name : matchMaterialFilter ()
/// <summary>
/// Determine if the specified material matches the currently applied
/// material filters (if any). Returns true if it should be rejected
/// or false if it should not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::matchMaterialFilter( const cgMaterialHandle & hMaterial )
{
    const cgMaterial * pMaterial = (const cgMaterial*)hMaterial.getResourceSilent( );

    // Should the material be filtered?
    cgFilterExpression * pFilter = mMaterialFilterStack.top();
    if ( pFilter && pMaterial && !pFilter->evaluate( pMaterial->getMaterialProperties() ) )
        return true;
    return false;
}

//-----------------------------------------------------------------------------
//  Name : setSamplerState ()
/// <summary>
/// Apply the specified sampler state to the indicated sampler register.
/// Supplying an invalid (NULL) handle will result in the default sampler 
/// states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setSamplerState( cgUInt32 nSamplerIndex, const cgSamplerStateHandle & hStates )
{
    // Out of bounds check.
    cgAssert( nSamplerIndex < MaxSamplerSlots );

    // There should always be at least one state in the stack.
    cgAssert( mSamplerStateStack[nSamplerIndex].size() > 0 );

    // Replace the top level stack entry with the new (or default) state.
    mSamplerStateStack[nSamplerIndex].top() = hStates.isValid() ? hStates : mDefaultSamplerState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushSamplerState ()
/// <summary>
/// Backup the currently applied sampler state and apply the newly specified
/// value. The prior state can be restored with a call to 'popSamplerState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushSamplerState( cgUInt32 nSamplerIndex, const cgSamplerStateHandle & hStates )
{
    // Out of bounds check.
    cgAssert( nSamplerIndex < MaxSamplerSlots );

    // Grow the stack and then apply the new element.
    mSamplerStateStack[nSamplerIndex].push( cgSamplerStateHandle() );
    if ( !setSamplerState( nSamplerIndex, hStates ) )
    {
        // Rollback.
        mSamplerStateStack[nSamplerIndex].pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popSamplerState ()
/// <summary>
/// Restore the specified sampler to the state in which it existed prior to the 
/// most recent call to 'pushSamplerState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popSamplerState( cgUInt32 nSamplerIndex )
{
    // Out of bounds check.
    cgAssert( nSamplerIndex < MaxSamplerSlots );

    // Should always be at least 1 format entry in the stack.
    // This last element cannot be removed.
    cgAssert( mSamplerStateStack[nSamplerIndex].size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mSamplerStateStack[nSamplerIndex].pop();
    restoreSamplerState( nSamplerIndex );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getSamplerState ()
/// <summary>
/// Retrieve the sampler state object currently applied to the specified
/// sampler register.
/// </summary>
//-----------------------------------------------------------------------------
const cgSamplerStateHandle & cgRenderDriver::getSamplerState( cgUInt32 nSamplerIndex ) const
{
    // Out of bounds check.
    cgAssert( nSamplerIndex < MaxSamplerSlots );

    // Should always be at least 1 format entry in the stack.
    cgAssert( mSamplerStateStack[nSamplerIndex].size() > 0 );
    return mSamplerStateStack[nSamplerIndex].top();
}

//-----------------------------------------------------------------------------
//  Name : setDepthStencilState ()
/// <summary>
/// Apply the specified depth stencil state to the device. Supplying an invalid 
/// (NULL) handle will result in the default depth stencil states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setDepthStencilState( const cgDepthStencilStateHandle & hStates )
{
    // Pass through to main function with default stencil reference.
    return setDepthStencilState( hStates, 0 );
}

//-----------------------------------------------------------------------------
//  Name : setDepthStencilState ()
/// <summary>
/// Apply the specified depth stencil state to the device. Supplying an invalid 
/// (NULL) handle will result in the default depth stencil states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setDepthStencilState( const cgDepthStencilStateHandle & hStates, cgUInt32 nStencilRef )
{
    // There should always be at least one format in the stack.
    cgAssert( mDepthStencilStateStack.size() > 0 );

    // Replace the top level stack entry with the new (or default) state.
    mDepthStencilStateStack.top().handle     = hStates.isValid() ? hStates : mDefaultDepthStencilState;
    mDepthStencilStateStack.top().stencilRef = nStencilRef;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushDepthStencilState ()
/// <summary>
/// Backup the currently applied depth stencil state and apply the newly
/// specified value. The prior state can be restored with a call to 
/// 'popDepthStencilState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushDepthStencilState( const cgDepthStencilStateHandle & hStates )
{
    // Grow the stack and then apply the new element.
    mDepthStencilStateStack.push( DepthStencilStateData() );
    if ( !setDepthStencilState( hStates ) )
    {
        // Rollback.
        mDepthStencilStateStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushDepthStencilState ()
/// <summary>
/// Backup the currently applied depth stencil state and apply the newly
/// specified value. The prior state can be restored with a call to 
/// 'popDepthStencilState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushDepthStencilState( const cgDepthStencilStateHandle & hStates, cgUInt32 nStencilRef )
{
    // Grow the stack and then apply the new element.
    mDepthStencilStateStack.push( DepthStencilStateData() );
    if ( !setDepthStencilState( hStates, nStencilRef ) )
    {
        // Rollback.
        mDepthStencilStateStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popDepthStencilState ()
/// <summary>
/// Restore the depth stencil values to the state in which they existed 
/// prior to the most recent call to 'pushDepthStencilState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popDepthStencilState( )
{
    // Should always be at least 1 format entry in the stack.
    // This last element cannot be removed.
    cgAssert( mDepthStencilStateStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mDepthStencilStateStack.pop();
    restoreDepthStencilState();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilState ()
/// <summary>
/// Retrieve the depth stencil state object currently applied to the device.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilStateHandle & cgRenderDriver::getDepthStencilState( ) const
{
    // Should always be at least 1 format entry in the stack.
    cgAssert( mDepthStencilStateStack.size() > 0 );
    return mDepthStencilStateStack.top().handle;
}

//-----------------------------------------------------------------------------
//  Name : setRasterizerState ()
/// <summary>
/// Apply the specified rasterizer state to the device. Supplying an invalid 
/// (NULL) handle will result in the default rasterizer states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setRasterizerState( const cgRasterizerStateHandle & hStates )
{
    // There should always be at least one format in the stack.
    cgAssert( mRasterizerStateStack.size() > 0 );

    // Replace the top level stack entry with the new (or default) state.
    mRasterizerStateStack.top() = hStates.isValid() ? hStates : mDefaultRasterizerState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushRasterizerState ()
/// <summary>
/// Backup the currently applied rasterizer state and apply the newly
/// specified value. The prior state can be restored with a call to 
/// 'popRasterizerState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushRasterizerState( const cgRasterizerStateHandle & hStates )
{
    // Grow the stack and then apply the new element.
    mRasterizerStateStack.push( cgRasterizerStateHandle() );
    if ( !setRasterizerState( hStates ) )
    {
        // Rollback.
        mRasterizerStateStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popRasterizerState ()
/// <summary>
/// Restore the rasterizers values to the state in which they existed 
/// prior to the most recent call to 'pushRasterizerState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popRasterizerState( )
{
    // Should always be at least 1 format entry in the stack.
    // This last element cannot be removed.
    cgAssert( mRasterizerStateStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mRasterizerStateStack.pop();
    restoreRasterizerState( );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getRasterizerState ()
/// <summary>
/// Retrieve the rasterizer state object currently applied to the device.
/// </summary>
//-----------------------------------------------------------------------------
const cgRasterizerStateHandle & cgRenderDriver::getRasterizerState( ) const
{
    // Should always be at least 1 format entry in the stack.
    cgAssert( mRasterizerStateStack.size() > 0 );
    return mRasterizerStateStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setBlendState ()
/// <summary>
/// Apply the specified blend state to the device. Supplying an invalid 
/// (NULL) handle will result in the default blend states being applied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setBlendState( const cgBlendStateHandle & hStates )
{
    // There should always be at least one format in the stack.
    cgAssert( mBlendStateStack.size() > 0 );

    // Replace the top level stack entry with the new (or default) state.
    mBlendStateStack.top() = hStates.isValid() ? hStates : mDefaultBlendState;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushBlendState ()
/// <summary>
/// Backup the currently applied blend state and apply the newly specified 
/// value. The prior state can be restored with a call to 'popBlendState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushBlendState( const cgBlendStateHandle & hStates )
{
    // Grow the stack and then apply the new element.
    mBlendStateStack.push( cgBlendStateHandle() );
    if ( !setBlendState( hStates ) )
    {
        // Rollback.
        mBlendStateStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popBlendState ()
/// <summary>
/// Restore the blending values to the state in which they existed prior to 
/// the most recent call to 'pushBlendState()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popBlendState( )
{
    // Should always be at least 1 format entry in the stack.
    // This last element cannot be removed.
    cgAssert( mBlendStateStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mBlendStateStack.pop();
    mStateFilteringEnabled = false;
    bool bResult = setBlendState( mBlendStateStack.top() );
    mStateFilteringEnabled = true;
    return bResult;
}

//-----------------------------------------------------------------------------
//  Name : getBlendState ()
/// <summary>
/// Retrieve the blend state object currently applied to the device.
/// </summary>
//-----------------------------------------------------------------------------
const cgBlendStateHandle & cgRenderDriver::getBlendState( ) const
{
    // Should always be at least 1 format entry in the stack.
    cgAssert( mBlendStateStack.size() > 0 );
    return mBlendStateStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setConstantBufferAuto ()
/// <summary>
/// Apply the specified constant buffer to the register defined within the
/// constant buffer's descriptor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setConstantBufferAuto( const cgConstantBufferHandle & hBuffer )
{
    if ( !hBuffer.isValid() )
        return false;
    const cgConstantBuffer * pBuffer = hBuffer.getResourceSilent();
    return setConstantBuffer( pBuffer->getDesc().bufferRegister, hBuffer );
}

//-----------------------------------------------------------------------------
//  Name : setConstantBuffer ()
/// <summary>
/// Apply the specified constant buffer to the indicated buffer register.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setConstantBuffer( cgUInt32 nBufferIndex, const cgConstantBufferHandle & hBuffer )
{
    // Out of bounds check.
    cgAssert( nBufferIndex < MaxConstantBufferSlots );

    // There should always be at least one buffer handle in the stack.
    cgAssert( mConstantBufferStack[nBufferIndex].size() > 0 );

    // If this buffer is the same as that currently assigned, skip.
    if ( mStateFilteringEnabled && mConstantBufferStack[nBufferIndex].top() == hBuffer )
        return true;

    // Buffer cannot be bound to multiple slots. 
    if ( hBuffer.isValid() )
    {
        const cgConstantBuffer * pBuffer = hBuffer.getResourceSilent();
        if ( pBuffer->mBoundRegister >= 0 )
        {
            cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Attempted to bind constant buffer '%s' to multiple buffer slots. Second assignment to register '%i' has been ignored.\n"), pBuffer->getResourceName().c_str(), nBufferIndex );
            return false;
        
        } // End if multiple registers
    
    } // End if valid

    // Retrieve any prior buffer and mark it as unbound.
    cgConstantBuffer * pOldBuffer = mConstantBufferStack[nBufferIndex].top().getResource(false);
    if ( pOldBuffer )
        pOldBuffer->mBoundRegister = -1;

    // Replace the top level stack entry with the new (or default) buffer.
    mConstantBufferStack[nBufferIndex].top() = hBuffer;

    // Set the 'dirty' status bit for this constant buffer and
    // mark it as bound to the device.
    cgConstantBuffer * pBuffer = mConstantBufferStack[nBufferIndex].top().getResource(false);
    if ( pBuffer )
    {
        mConstantsDirty |= (1 << nBufferIndex);
        pBuffer->mBoundRegister = nBufferIndex;

    } // End if valid
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : restoreConstantBuffer () (Protected, Virtual)
/// <summary>
/// Re-bind the current constant buffer object to the device.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::restoreConstantBuffer( cgUInt32 nBufferIndex )
{
    // Set the 'dirty' status bit for this constant buffer.
    cgConstantBuffer * pBuffer = mConstantBufferStack[nBufferIndex].top().getResource(false);
    if ( pBuffer )
        mConstantsDirty |= (1 << nBufferIndex);
}

//-----------------------------------------------------------------------------
//  Name : pushConstantBuffer ()
/// <summary>
/// Backup the currently applied constant buffer and apply the newly specified
/// value. The prior buffer can be restored with a call to 
/// 'popConstantBuffer()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushConstantBuffer( cgUInt32 nBufferIndex, const cgConstantBufferHandle & hBuffer )
{
    // Out of bounds check.
    cgAssert( nBufferIndex < MaxConstantBufferSlots );

    // Grow the stack and then apply the new element.
    mConstantBufferStack[nBufferIndex].push( cgConstantBufferHandle() );
    if ( !setConstantBuffer( nBufferIndex, hBuffer ) )
    {
        // Rollback.
        mConstantBufferStack[nBufferIndex].pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popConstantBuffer ()
/// <summary>
/// Restore the specified buffer to the state in which it existed prior to the 
/// most recent call to 'pushConstantBuffer()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popConstantBuffer( cgUInt32 nBufferIndex )
{
    // Out of bounds check.
    cgAssert( nBufferIndex < MaxConstantBufferSlots );

    // Should always be at least 1 buffer entry in the stack.
    // This last element cannot be removed.
    cgAssert( mConstantBufferStack[nBufferIndex].size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mConstantBufferStack[nBufferIndex].pop();
    restoreConstantBuffer( nBufferIndex );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getConstantBuffer ()
/// <summary>
/// Retrieve the constant buffer object currently applied to the specified
/// buffer register.
/// </summary>
//-----------------------------------------------------------------------------
const cgConstantBufferHandle & cgRenderDriver::getConstantBuffer( cgUInt32 nBufferIndex ) const
{
    // Out of bounds check.
    cgAssert( nBufferIndex < MaxConstantBufferSlots );

    // Should always be at least 1 buffer entry in the stack.
    cgAssert( mConstantBufferStack[nBufferIndex].size() > 0 );
    return mConstantBufferStack[nBufferIndex].top();
}

//-----------------------------------------------------------------------------
//  Name : setVertexFormat ()
/// <summary>
/// Set the vertex format that should be assumed by any following drawing 
/// routine.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setVertexFormat( cgVertexFormat * pFormat )
{
    // There should always be at least one format in the stack.
    cgAssert( mVertexFormatStack.size() > 0 );

    // Replace the top level stack entry with the format to use.
    mVertexFormatStack.top() = pFormat;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushVertexFormat ()
/// <summary>
/// Backup the current vertex format layout and apply the newly specified
/// value. The prior format can be restored with a call to 'popVertexFormat()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushVertexFormat( cgVertexFormat * pFormat )
{
    // Grow the stack and then set the new element.
    mVertexFormatStack.push( CG_NULL );
    if ( !setVertexFormat( pFormat ) )
    {
        // Rollback.
        mVertexFormatStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popVertexFormat ()
/// <summary>
/// Restore the vertex format to the state in which it existed prior to the 
/// most recent call to 'pushVertexFormat()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popVertexFormat( )
{
    // Should always be at least 1 format entry in the stack.
    cgAssert( mVertexFormatStack.size() > 1 );

    // Shrink the stack and then re-apply the prior element.
    mVertexFormatStack.pop();
    restoreVertexFormat( );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getVertexFormat ()
/// <summary>
/// Retrieve the vertex format that we should use to render.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgRenderDriver::getVertexFormat(  ) const
{
    cgAssert(mVertexFormatStack.size() > 0);
    return mVertexFormatStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setIndices ()
/// <summary>
/// Bind the specified index buffer to the device. This index buffer will be
/// utilized in any subsequent calls to 'DrawIndexedPrimitive()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setIndices( const cgIndexBufferHandle & hIndices )
{
    // There should always be at least one format in the stack.
    cgAssert( mIndicesStack.size() > 0 );

    // Replace the top level stack entry with the format to use.
    mIndicesStack.top() = hIndices;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushIndices ()
/// <summary>
/// Backup the current index buffer handle and apply the newly specified
/// value. The prior format can be restored with a call to 'popIndices()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushIndices( const cgIndexBufferHandle & hIndices )
{
    // Grow the stack and then set the new element.
    mIndicesStack.push( cgIndexBufferHandle() );
    if ( !setIndices( hIndices ) )
    {
        // Rollback.
        mIndicesStack.pop();
        return false;
    
    } // End if failed
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popIndices ()
/// <summary>
/// Restore the assigned index buffer to the one that was previously set in 
/// the most recent call to 'pushIndices()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popIndices( )
{
    // There must always be at least one value in the stack
    // which cannot be removed.
    cgAssert(mIndicesStack.size() > 1);
    
    // Shrink the stack and re-apply the prior element.
    mIndicesStack.pop();
    restoreIndices();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getIndices ()
/// <summary>
/// Retrieve the index buffer that is currently in use for rendering.
/// </summary>
//-----------------------------------------------------------------------------
const cgIndexBufferHandle & cgRenderDriver::getIndices( ) const
{
    cgAssert(mIndicesStack.size() > 0);
    return mIndicesStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setStreamSource ()
/// <summary>
/// Apply the specified vertex buffer to the indicated stream slot index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setStreamSource( cgUInt32 nStreamIndex, const cgVertexBufferHandle & hVertices )
{
    // Out of bounds check.
    cgAssert( nStreamIndex < MaxStreamSlots );

    // There should always be at least one stream in the stack.
    cgAssert( mVertexStreamStack[nStreamIndex].size() > 0 );

    // There should also always be a vertex format in the stack
    cgAssert( mVertexFormatStack.size() > 0 && mVertexFormatStack.top() != CG_NULL );

    // Replace the top level stack entry with the new handle.
    mVertexStreamStack[nStreamIndex].top().handle = hVertices;
    cgToDo( "Carbon General", "Vertex format system needs to be overhauled to maintain a stride per stream?" )
    mVertexStreamStack[nStreamIndex].top().stride = mVertexFormatStack.top()->getStride();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushStreamSource ()
/// <summary>
/// Backup the currently applied vertex stream and apply the newly specified
/// value. The prior state can be restored with a call to 'popStreamSource()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushStreamSource( cgUInt32 nStreamIndex, const cgVertexBufferHandle & hVertices )
{
    // Out of bounds check.
    cgAssert( nStreamIndex < MaxStreamSlots );

    // Grow the stack and then apply the new element.
    mVertexStreamStack[nStreamIndex].push( VertexStreamData() );
    if ( !setStreamSource( nStreamIndex, hVertices ) )
    {
        // Rollback.
        mVertexStreamStack[nStreamIndex].pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popStreamSource ()
/// <summary>
/// Restore the specified vertex stream to the state in which it existed prior 
/// to the most recent call to 'pushStreamSource()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popStreamSource( cgUInt32 nStreamIndex )
{
    // Out of bounds check.
    cgAssert( nStreamIndex < MaxStreamSlots );

    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mVertexStreamStack[nStreamIndex].size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mVertexStreamStack[nStreamIndex].pop();
    restoreStreamSource( nStreamIndex );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getStreamSource ()
/// <summary>
/// Retrieve the vertex buffer object currently applied to the specified
/// stream slot.
/// </summary>
//-----------------------------------------------------------------------------
const cgVertexBufferHandle & cgRenderDriver::getStreamSource( cgUInt32 nStreamIndex ) const
{
    // Out of bounds check.
    cgAssert( nStreamIndex < MaxStreamSlots );

    // Should always be at least 1 format entry in the stack.
    cgAssert( mVertexStreamStack[nStreamIndex].size() > 0 );
    return mVertexStreamStack[nStreamIndex].top().handle;
}

//-----------------------------------------------------------------------------
//  Name : setTexture ()
/// <summary>
/// Apply the specified texture to the indicated texture slot index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setTexture( cgUInt32 nTextureIndex, const cgTextureHandle & hTexture )
{
    // Out of bounds check.
    cgAssert( nTextureIndex < MaxTextureSlots );

    // There should always be at least one texture in the stack.
    cgAssert( mTextureStack[nTextureIndex].size() > 0 );

    // Replace the top level stack entry with the new handle.
    mTextureStack[nTextureIndex].top() = hTexture;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushTexture ()
/// <summary>
/// Backup the currently applied texture and apply the newly specified
/// value. The prior state can be restored with a call to 'popTexture()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushTexture( cgUInt32 nTextureIndex, const cgTextureHandle & hTexture )
{
    // Out of bounds check.
    cgAssert( nTextureIndex < MaxTextureSlots );

    // Grow the stack and then apply the new element.
    mTextureStack[nTextureIndex].push( cgTextureHandle() );
    if ( !setTexture( nTextureIndex, hTexture ) )
    {
        // Rollback.
        mTextureStack[nTextureIndex].pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popTexture ()
/// <summary>
/// Restore the specified vertex stream to the state in which it existed prior 
/// to the most recent call to 'pushTexture()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popTexture( cgUInt32 nTextureIndex )
{
    // Out of bounds check.
    cgAssert( nTextureIndex < MaxTextureSlots );

    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mTextureStack[nTextureIndex].size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mTextureStack[nTextureIndex].pop();
    restoreTexture( nTextureIndex );
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getTexture ()
/// <summary>
/// Retrieve the vertex buffer object currently applied to the specified
/// stream slot.
/// </summary>
//-----------------------------------------------------------------------------
const cgTextureHandle & cgRenderDriver::getTexture( cgUInt32 nTextureIndex ) const
{
    // Out of bounds check.
    cgAssert( nTextureIndex < MaxTextureSlots );

    // Should always be at least 1 format entry in the stack.
    cgAssert( mTextureStack[nTextureIndex].size() > 0 );
    return mTextureStack[nTextureIndex].top();
}

//-----------------------------------------------------------------------------
//  Name : setVertexShader ()
/// <summary>
/// Apply the specified vertex shader to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setVertexShader( const cgVertexShaderHandle & hVertexShader )
{
    // There should always be at least one shader in the stack.
    cgAssert( mVertexShaderStack.size() > 0 );

    // Replace the top level stack entry with the new handle or the
    // default vertex shader if a null handle is suppled.
    if ( !hVertexShader.isValid() )
        mVertexShaderStack.top() = mDriverShader->getVertexShader( _T("defaultVertexShader") );
    else
        mVertexShaderStack.top() = hVertexShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushVertexShader ()
/// <summary>
/// Backup the currently applied vertex shader and apply the newly specified
/// value. The prior state can be restored with a call to 'popVertexShader()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushVertexShader( const cgVertexShaderHandle & hVertexShader )
{
    // Grow the stack and then apply the new element.
    mVertexShaderStack.push( cgVertexShaderHandle::Null );
    if ( !setVertexShader( hVertexShader ) )
    {
        // Rollback.
        mVertexShaderStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popVertexShader ()
/// <summary>
/// Restore the specified vertex shader to the state in which it existed prior 
/// to the most recent call to 'pushVertexShader()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popVertexShader( )
{
    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mVertexShaderStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mVertexShaderStack.pop();
    restoreVertexShader();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getVertexShader ()
/// <summary>
/// Retrieve the vertex shader object currently applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
const cgVertexShaderHandle & cgRenderDriver::getVertexShader( ) const
{
    // Should always be at least 1 entry in the stack.
    cgAssert( mVertexShaderStack.size() > 0 );
    return mVertexShaderStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setPixelShader ()
/// <summary>
/// Apply the specified pixel shader to the driver.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setPixelShader( const cgPixelShaderHandle & hPixelShader )
{
    // There should always be at least one shader in the stack.
    cgAssert( mPixelShaderStack.size() > 0 );

    // Replace the top level stack entry with the new handle.
    mPixelShaderStack.top() = hPixelShader;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushPixelShader ()
/// <summary>
/// Backup the currently applied pixel shader and apply the newly specified
/// value. The prior state can be restored with a call to 'popPixelShader()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushPixelShader( const cgPixelShaderHandle & hPixelShader )
{
    // Grow the stack and then apply the new element.
    mPixelShaderStack.push( cgPixelShaderHandle::Null );
    if ( !setPixelShader( hPixelShader ) )
    {
        // Rollback.
        mPixelShaderStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popPixelShader ()
/// <summary>
/// Restore the specified pixel shader to the state in which it existed prior 
/// to the most recent call to 'pushPixelShader()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popPixelShader( )
{
    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mPixelShaderStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mPixelShaderStack.pop();
    restorePixelShader();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getPixelShader ()
/// <summary>
/// Retrieve the pixel shader object currently applied to the driver.
/// </summary>
//-----------------------------------------------------------------------------
const cgPixelShaderHandle & cgRenderDriver::getPixelShader( ) const
{
    // Should always be at least 1 entry in the stack.
    cgAssert( mPixelShaderStack.size() > 0 );
    return mPixelShaderStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setWorldTransform ()
/// <summary>
/// Set the world matrix state to the device. Providing a NULL argument will
/// result in an identity matrix being assumed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setWorldTransform( const cgMatrix * pMatrix )
{
    // There must always be at least 1 entry in the world matrix stack.
    cgAssert( mWorldTransformStack.size() > 0 );

    // Reset to identity matrix if CG_NULL passed.
    cgMatrix mtx;
    if ( !pMatrix )
    {
        cgMatrix::identity( mtx );
        pMatrix = &mtx;
    
    } // End if identity

    // Store matrix
    mWorldTransformStack.top() = *pMatrix;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushWorldTransform ()
/// <summary>
/// Backup the current world matrix state and apply the newly specified
/// transformation to the device. The prior matrix can be restored with a call
/// to 'popWorldTransform()'. Providing a NULL argument will result in an 
/// identity matrix being assumed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushWorldTransform( const cgMatrix * pMatrix )
{
    // Grow the stack and apply the new element.
    mWorldTransformStack.push(cgMatrix());
    if ( !setWorldTransform( pMatrix ) )
    {
        // Rollback.
        mWorldTransformStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popWorldTransform ()
/// <summary>
/// Restore the world transformation matrix to the state in which it existed
/// prior to the most recent call to 'pushWorldTransform()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popWorldTransform( )
{
    // There must always be at least one value in the stack
    // which cannot be removed.
    cgAssert(mWorldTransformStack.size() > 1);
    
    // Shrink the stack and re-apply the prior element.
    mWorldTransformStack.pop();
    cgToDo( "Carbon General", "Do we need a 'RestoreWorldTransform()' style mechanism like states?" );
    cgMatrix mtxNew = mWorldTransformStack.top();
    return setWorldTransform( &mtxNew );
}

//-----------------------------------------------------------------------------
//  Name : getWorldTransform ()
/// <summary>
/// Retrieve the world matrix state.
/// </summary>
//-----------------------------------------------------------------------------
const cgMatrix & cgRenderDriver::getWorldTransform( ) const
{
    cgAssert( mWorldTransformStack.size() > 0 );
    return mWorldTransformStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setCamera() 
/// <summary>
/// Gives the render driver access to the current camera. 
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setCamera( cgCameraNode * pCamera )
{
    // There must always be at least 1 entry in the camera stack.
    cgAssert( mCameraStack.size() > 0 );    

    // Store camera
    mCameraStack.back() = pCamera;

    // Update with the new camera details
    return cameraUpdated();
}

//-----------------------------------------------------------------------------
//  Name : pushCamera ()
/// <summary>
/// Backup the current camera reference and apply the newly specified
/// camera node to the device. The prior camera can be restored with a call
/// to 'popCamera()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushCamera( cgCameraNode * pCamera )
{
    // Grow the stack and apply the new element.
    mCameraStack.push_back(CG_NULL);
    if ( !setCamera( pCamera ) )
    {
        // Rollback.
        mCameraStack.pop_back();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popCamera ()
/// <summary>
/// Restore the current camera pointer to the state in which it existed
/// prior to the most recent call to 'pushCamera()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popCamera( )
{
    // There must always be at least one value in the stack
    // which cannot be removed.
    cgAssert(mCameraStack.size() > 1);
    
    // Shrink the stack and re-apply the prior element.
    mCameraStack.pop_back();
    cgToDo( "Carbon General", "Do we need a 'RestoreCamera()' style mechanism like states?" );
    return setCamera( mCameraStack.back() );
}

//-----------------------------------------------------------------------------
//  Name : getCamera ()
/// <summary>
/// Retrieve the camera currently applied to the device.
/// </summary>
//-----------------------------------------------------------------------------
cgCameraNode * cgRenderDriver::getCamera( )
{
    // There must always be at least one value in the stack.
    cgAssert(mCameraStack.size() > 0);
    return mCameraStack.back();
}

//-----------------------------------------------------------------------------
//  Name : cameraUpdated() 
/// <summary>
/// This function should be called by the application if the currently
/// active camera has changed any of it's properties (Viewport, view or
/// projection matrix) after a call to 'beginFrame'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::cameraUpdated()
{
    // There must always be at least one value in the stack.
    cgAssert(mCameraStack.size() > 0);

    // Update new camera
    cgCameraNode * pCamera = mCameraStack.back();
    if ( pCamera != CG_NULL ) 
    {
        // ToDo: If camera is orthographic, should set projection window to viewport when not locked?

        // Update the camera's aspect ratio based on the currently applied viewport, framebuffer and rendertarget settings
        if ( pCamera->isAspectLocked() == false )
        {
			/*// In windowed mode, we use the traditional ratio between current window width and height. 
 			// In fullscreen mode however, we use the adapter's original aspect ratio as it is useful
			// for reducing the image stretching common on widescreen displays.
			if ( isWindowed() == true )
			{
				cgSize ScreenSize = getScreenSize();
				cgFloat fAspect = (cgFloat)ScreenSize.width / (cgFloat)ScreenSize.height;
	            pCamera->setAspectRatio( fAspect );

			} // End if Windowed
			else
			{
                // Use aspect ratio computed during initialization.
	            pCamera->setAspectRatio( mAdapterAspectRatio );

			} // End if full screen*/

            // Update aspect ratio of camera to match the current dimensions
            // of the currently active render view.
            cgSize ViewSize = getActiveRenderView()->getSize();
			cgFloat fAspect = (cgFloat)ViewSize.width / (cgFloat)ViewSize.height;
            pCamera->setAspectRatio( fAspect );

        } // End if camera aspect is not locked

        // Store matrices
        mViewMatrix       = pCamera->getViewMatrix();
        mProjectionMatrix = pCamera->getProjectionMatrix();
        mViewProjectionMatrix   = mViewMatrix * mProjectionMatrix;

    } // End if camera available

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setViewport() (Virtual)
/// <summary>
/// Set the specified viewport details to the device. Specify a value of
/// CG_NULL to automatically revert to full size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setViewport( const cgViewport * pViewport )
{
    // There should always be at least one viewport in the stack.
    cgAssert( mViewportStack.size() > 0 );
    cgViewport & Viewport = mViewportStack.top();

    // Revert to full size?
    if ( pViewport == CG_NULL )
    {
        cgSize ViewSize = mRenderViewStack.top()->getSize();
        Viewport.x      = 0;
        Viewport.y      = 0;
        Viewport.width  = ViewSize.width;
        Viewport.height = ViewSize.height;
        Viewport.minimumZ   = 0;
        Viewport.maximumZ   = 1;

    } // End if full size
    else
    {
        Viewport = *pViewport;
        
    } // End if specified

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushViewport ()
/// <summary>
/// Backup the current viewport and apply the newly specified
/// value. The prior format can be restored with a call to 'popViewport()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushViewport( const cgViewport * pViewport )
{
    // Grow the stack and then set the new element.
    mViewportStack.push( cgViewport() );
    if ( !setViewport( pViewport ) )
    {
        // Rollback.
        mViewportStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popViewport ()
/// <summary>
/// Restore the viewport to the state in which it existed prior to the 
/// most recent call to 'pushViewport()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popViewport( )
{
    // Should always be at least 1 format entry in the stack.
    cgAssert( mViewportStack.size() > 1 );

    // Shrink the stack and then re-apply the prior element.
    mViewportStack.pop();
    return setViewport( &mViewportStack.top() );
}

//-----------------------------------------------------------------------------
//  Name : getViewport ()
/// <summary>
/// Retrieve the currently set viewport.
/// </summary>
//-----------------------------------------------------------------------------
const cgViewport & cgRenderDriver::getViewport( ) const
{
    cgAssert(mViewportStack.size() > 0);
    return mViewportStack.top();
}

//-----------------------------------------------------------------------------
//  Name : setUserClipPlanes() 
/// <summary>
/// Sets user defined clip planes (maximum equivalent to MaxClipPlaneSlots) with 
/// optional negation.
/// </summary>
//-----------------------------------------------------------------------------
// ToDo: 9999 - Should negated planes really be the default?
bool cgRenderDriver::setUserClipPlanes( const cgPlane pWorldSpacePlanes[], cgUInt32 nNumPlanes, bool bNegatePlanes /* = true */ )
{
    // There must always be at least 1 entry in the stack.
    cgAssert( mClipPlaneStack.size() > 0 );    

    // Validate other requirements
    cgAssert( (nNumPlanes > 0) ? (pWorldSpacePlanes != CG_NULL) : true );
    cgAssert( nNumPlanes <= MaxClipPlaneSlots );

    // If empty or NULL array, just clear all planes and exit.
    ClipPlaneData & Data = mClipPlaneStack.top();
	if ( !(pWorldSpacePlanes && nNumPlanes) ) 
    {
        Data.planeCount = 0;
        return true;
	
    } // End if clear

    // Copy and / or duplicate negated planes as needed onto
    // the internal stack.
    Data.planeCount = nNumPlanes;
	if( bNegatePlanes == true )
    {
        for ( cgUInt32 i = 0; i < nNumPlanes; i++ )
            Data.planes[ i ] = -pWorldSpacePlanes[ i ];
    
    } // End if negate
    else
    {
	    memcpy( Data.planes, pWorldSpacePlanes, nNumPlanes * sizeof(cgPlane) );
    
    } // End if !negate

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushUserClipPlanes()
/// <summary>
/// Backup the current user clip planes and apply the newly specified
/// values. The prior planes can be restored with a call to 
/// 'popUserClipPlanes()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushUserClipPlanes( const cgPlane pWorldSpacePlanes[], cgUInt32 nNumPlanes, bool bNegatePlanes /* = true */ )
{
    // Grow the stack and set the new element.
    mClipPlaneStack.push(ClipPlaneData());
    if ( !setUserClipPlanes( pWorldSpacePlanes, nNumPlanes, bNegatePlanes ) )
    {
        // Rollback.
        mClipPlaneStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popUserClipPlanes ()
/// <summary>
/// Restore the user defined clip plane values to the state in which they 
/// existed prior to the most recent call to 'pushUserClipPlanes()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popUserClipPlanes( )
{
    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mClipPlaneStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mClipPlaneStack.pop();
    cgToDo( "Carbon General", "Do we need a 'RestoreUserClipPlanes()' style mechanism like states?" );
    return setUserClipPlanes( mClipPlaneStack.top().planes, mClipPlaneStack.top().planeCount, false );
}

//-----------------------------------------------------------------------------
//  Name : setScissorRect() 
/// <summary>
/// Select the screen space scissor rectangle clipping area outside of which
/// pixels will be clipped away. Specifying a NULL pointer argument will 
/// cause the entire frame buffer to be selected.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::setScissorRect( const cgRect * pRect )
{
    // There must always be at least 1 entry in the stack.
    cgAssert( mScissorRectStack.size() > 0 );    

    // Revert to full size?
    if ( !pRect || (pRect->left >= pRect->right) || (pRect->top >= pRect->bottom) )
    {
        cgSize Size = getScreenSize();
        cgRect & Data = mScissorRectStack.top();
        Data.left = 0;
        Data.top  = 0;
        Data.right = Size.width;
        Data.bottom = Size.height;

    } // End if full size
    else
    {
        mScissorRectStack.top() = *pRect;

    } // End if specific size
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : pushScissorRect()
/// <summary>
/// Backup the current scissor rectangle and apply the newly specified
/// values. The prior rectangle can be restored with a call to 
/// 'popScissorRect()'. Specifying a NULL pointer argument will cause the 
/// entire frame buffer to be selected.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::pushScissorRect( const cgRect * pRect )
{
    // Grow the stack and set the new element.
    mScissorRectStack.push(cgRect());
    if ( !setScissorRect( pRect ) )
    {
        // Rollback.
        mScissorRectStack.pop();
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : popScissorRect ()
/// <summary>
/// Restore the scissor rectangle to the state in which it existed prior to the 
/// most recent call to 'pushScissorRect()'.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::popScissorRect( )
{
    // Should always be at least 1 entry in the stack.
    // This last element cannot be removed.
    cgAssert( mScissorRectStack.size() > 1 );

    // Shrink the stack and then re-apply the prior state.
    mScissorRectStack.pop();
    return setScissorRect( &mScissorRectStack.top() );
}

//-----------------------------------------------------------------------------
//  Name : drawLines()
/// <summary>
/// Draw a simple line list or strip to the screen with the specified color.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawLines( cgVector2 Points[], cgUInt32 nLineCount, const cgColorValue & Color, bool bStripBehavior /* = false */ )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    cgAssert( Points && nLineCount );

    // Compile / load required shaders.
    cgPixelShaderHandle hPixelShader = mDriverShader->getPixelShader( _T("drawScreenElement") );
    if ( !hPixelShader.isValid() )
        return;

    // Allocate buffer
    cgUInt32 nPointCount = (bStripBehavior == true) ? nLineCount + 1 : nLineCount * 2;
    cgScreenVertex * pLineBuffer = new cgScreenVertex[ nPointCount ];

    // ToDo: Should probably optimize with a hardware VB
    // Build the final buffer
    const cgUInt32 nColor = Color;
    cgScreenVertex * pBuffer = pLineBuffer;
    for ( cgUInt32 i = 0; i < nPointCount; ++i, ++pBuffer )
    {
        pBuffer->position = cgVector4( Points[i].x, Points[i].y, 0, 1 );
        pBuffer->color    = nColor;
    
    } // Next Point

    // Setup vertex and pixel shader.
    pushVertexShader( cgVertexShaderHandle::Null );
    pushPixelShader( hPixelShader );

    // Setup blending states (basic alpha blending enabled)
    pushBlendState( mElementBlendState );

    // Disable depth buffering and stencil operations
    pushDepthStencilState( mElementDepthState );

    // Use default rasterizer states.
    pushRasterizerState( mDefaultRasterizerState );

    // Set the vertex format
    pushVertexFormat( mScreenSpaceFormat );
    
    // Draw
    if ( bStripBehavior == false )
        drawPrimitiveUP( cgPrimitiveType::LineList, nLineCount, (void*)pLineBuffer );
    else
        drawPrimitiveUP( cgPrimitiveType::LineStrip, nLineCount, (void*)pLineBuffer );

    // Clean up
    delete []pLineBuffer;

    // Restore modified states.
    popVertexFormat();
    popRasterizerState();
    popDepthStencilState();
    popBlendState();
    popVertexShader();
    popPixelShader();
}

//-----------------------------------------------------------------------------
// Name : drawOOBB ( ) (Protected)
/// <summary>
/// Draw a representation of an object aligned bounding box.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawOOBB( const cgBoundingBox & Bounds, cgFloat fGrowAmount, const cgMatrix & mtxObject, const cgColorValue & Color, bool bSealedEdges /* = false */ )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Compile / load required shaders
    cgVertexShaderHandle hVertexShader = mDriverShader->getVertexShader( _T("transformSimpleDefault") );
    cgPixelShaderHandle hPixelShader = mDriverShader->getPixelShader( _T("drawScreenElement") );
    if ( !hVertexShader.isValid() || !hPixelShader.isValid() )
        return;

    // Grow the bounding box by the required amount in order to prevent
    // it from overdrawing the object itself. The amount to grow is specified
    // in world space, so we must adjust this value to take into account the
    // object scale.
    cgVector3 vScale;
    vScale.x = 1.0f / cgVector3::length( cgVector3( mtxObject._11, mtxObject._12, mtxObject._13 ) );
    vScale.y = 1.0f / cgVector3::length( cgVector3( mtxObject._21, mtxObject._22, mtxObject._23 ) );
    vScale.z = 1.0f / cgVector3::length( cgVector3( mtxObject._31, mtxObject._32, mtxObject._33 ) );
    cgVector3 vMin = Bounds.min, vMax = Bounds.max;
    vMin.x -= fGrowAmount * vScale.x; vMin.y -= fGrowAmount * vScale.y; vMin.z -= fGrowAmount * vScale.z;
    vMax.x += fGrowAmount * vScale.x; vMax.y += fGrowAmount * vScale.y; vMax.z += fGrowAmount * vScale.z;

    // What approach should be taken for rendering?
    cgShadedVertex Points[48];
    cgUInt32 nColor = Color;
    if ( bSealedEdges == false )
    {
        // Compute the maximum length of the AABB corner lines (we use a percentage
        // of the bounding box size)
        cgVector3 vLineLength( (vMax - vMin) * 0.25f );
        
        // Build box "segments" at each corner

        // Bottom Corners
        Points[0]  = cgShadedVertex( vMin.x, vMin.y, vMin.z, nColor );
        Points[1]  = cgShadedVertex( vMin.x + vLineLength.x, vMin.y, vMin.z, nColor );
        Points[2]  = cgShadedVertex( vMin.x, vMin.y, vMin.z, nColor );
        Points[3]  = cgShadedVertex( vMin.x, vMin.y + vLineLength.y, vMin.z, nColor );
        Points[4]  = cgShadedVertex( vMin.x, vMin.y, vMin.z, nColor );
        Points[5]  = cgShadedVertex( vMin.x, vMin.y, vMin.z + vLineLength.z, nColor );

        Points[6]  = cgShadedVertex( vMin.x, vMin.y, vMax.z, nColor );
        Points[7]  = cgShadedVertex( vMin.x + vLineLength.x, vMin.y, vMax.z, nColor );
        Points[8]  = cgShadedVertex( vMin.x, vMin.y, vMax.z, nColor );
        Points[9]  = cgShadedVertex( vMin.x, vMin.y + vLineLength.y, vMax.z, nColor );
        Points[10] = cgShadedVertex( vMin.x, vMin.y, vMax.z, nColor );
        Points[11] = cgShadedVertex( vMin.x, vMin.y, vMax.z - vLineLength.z, nColor );

        Points[12] = cgShadedVertex( vMax.x, vMin.y, vMax.z, nColor );
        Points[13] = cgShadedVertex( vMax.x - vLineLength.x, vMin.y, vMax.z, nColor );
        Points[14] = cgShadedVertex( vMax.x, vMin.y, vMax.z, nColor );
        Points[15] = cgShadedVertex( vMax.x, vMin.y + vLineLength.y, vMax.z, nColor );
        Points[16] = cgShadedVertex( vMax.x, vMin.y, vMax.z, nColor );
        Points[17] = cgShadedVertex( vMax.x, vMin.y, vMax.z - vLineLength.z, nColor );

        Points[18] = cgShadedVertex( vMax.x, vMin.y, vMin.z, nColor );
        Points[19] = cgShadedVertex( vMax.x - vLineLength.x, vMin.y, vMin.z, nColor );
        Points[20] = cgShadedVertex( vMax.x, vMin.y, vMin.z, nColor );
        Points[21] = cgShadedVertex( vMax.x, vMin.y + vLineLength.y, vMin.z, nColor );
        Points[22] = cgShadedVertex( vMax.x, vMin.y, vMin.z, nColor );
        Points[23] = cgShadedVertex( vMax.x, vMin.y, vMin.z + vLineLength.z, nColor );

        // Top Corners
        Points[24] = cgShadedVertex( vMin.x, vMax.y, vMin.z, nColor );
        Points[25] = cgShadedVertex( vMin.x + vLineLength.x, vMax.y, vMin.z, nColor );
        Points[26] = cgShadedVertex( vMin.x, vMax.y, vMin.z, nColor );
        Points[27] = cgShadedVertex( vMin.x, vMax.y - vLineLength.y, vMin.z, nColor );
        Points[28] = cgShadedVertex( vMin.x, vMax.y, vMin.z, nColor );
        Points[29] = cgShadedVertex( vMin.x, vMax.y, vMin.z + vLineLength.z, nColor );

        Points[30] = cgShadedVertex( vMin.x, vMax.y, vMax.z, nColor );
        Points[31] = cgShadedVertex( vMin.x + vLineLength.x, vMax.y, vMax.z, nColor );
        Points[32] = cgShadedVertex( vMin.x, vMax.y, vMax.z, nColor );
        Points[33] = cgShadedVertex( vMin.x, vMax.y - vLineLength.y, vMax.z, nColor );
        Points[34] = cgShadedVertex( vMin.x, vMax.y, vMax.z, nColor );
        Points[35] = cgShadedVertex( vMin.x, vMax.y, vMax.z - vLineLength.z, nColor );

        Points[36] = cgShadedVertex( vMax.x, vMax.y, vMax.z, nColor );
        Points[37] = cgShadedVertex( vMax.x - vLineLength.x, vMax.y, vMax.z, nColor );
        Points[38] = cgShadedVertex( vMax.x, vMax.y, vMax.z, nColor );
        Points[39] = cgShadedVertex( vMax.x, vMax.y - vLineLength.y, vMax.z, nColor );
        Points[40] = cgShadedVertex( vMax.x, vMax.y, vMax.z, nColor );
        Points[41] = cgShadedVertex( vMax.x, vMax.y, vMax.z - vLineLength.z, nColor );

        Points[42] = cgShadedVertex( vMax.x, vMax.y, vMin.z, nColor );
        Points[43] = cgShadedVertex( vMax.x - vLineLength.x, vMax.y, vMin.z, nColor );
        Points[44] = cgShadedVertex( vMax.x, vMax.y, vMin.z, nColor );
        Points[45] = cgShadedVertex( vMax.x, vMax.y - vLineLength.y, vMin.z, nColor );
        Points[46] = cgShadedVertex( vMax.x, vMax.y, vMin.z, nColor );
        Points[47] = cgShadedVertex( vMax.x, vMax.y, vMin.z + vLineLength.z, nColor );

    } // End if unsealed edges
    else
    {
        // Compute the maximum length of the AABB lines
        cgVector3 vLineLength( vMax - vMin );

        // Build box - bottom square
        Points[0]  = cgShadedVertex( vMin.x, vMin.y, vMin.z, nColor );
        Points[1]  = cgShadedVertex( vMin.x + vLineLength.x, vMin.y, vMin.z, nColor );
        Points[2]  = Points[1];
        Points[3]  = cgShadedVertex( vMin.x + vLineLength.x, vMin.y, vMin.z + vLineLength.z, nColor );
        Points[4]  = Points[3];
        Points[5]  = cgShadedVertex( vMin.x, vMin.y, vMin.z + vLineLength.z, nColor );
        Points[6]  = Points[5];
        Points[7]  = Points[0];

        // Top square
        Points[8]  = cgShadedVertex( vMin.x, vMax.y, vMin.z, nColor );
        Points[9]  = cgShadedVertex( vMin.x + vLineLength.x, vMax.y, vMin.z, nColor );
        Points[10] = Points[9];
        Points[11] = cgShadedVertex( vMin.x + vLineLength.x, vMax.y, vMin.z + vLineLength.z, nColor );
        Points[12] = Points[11];
        Points[13] = cgShadedVertex( vMin.x, vMax.y, vMin.z + vLineLength.z, nColor );
        Points[14] = Points[13];
        Points[15] = Points[8];

        // Connecting Lines
        Points[16] = Points[0];
        Points[17] = Points[8];
        Points[18] = Points[2];
        Points[19] = Points[10];
        Points[20] = Points[4];
        Points[21] = Points[12];
        Points[22] = Points[6];
        Points[23] = Points[14];

    } // End if sealed edges

    // Setup vertex and pixel shaders.
    pushVertexShader( hVertexShader );
    pushPixelShader( hPixelShader );

    // Setup blending state (basic alpha blending enabled)
    pushBlendState( mElementBlendState );

    // Use default depth stencil state (depth read/write)
    pushDepthStencilState( mDefaultDepthStencilState );

    // Use default rasterizer state.
    pushRasterizerState( mDefaultRasterizerState );

    // Set the vertex format
    pushVertexFormat( mWorldSpaceFormat );

    // Transform based on supplied matrix.
    pushWorldTransform( &mtxObject );

    // Draw
    drawPrimitiveUP( cgPrimitiveType::LineList, (bSealedEdges == false ) ? 24 : 12, Points );

    // Restore modified states.
    popWorldTransform();
    popVertexFormat();
    popRasterizerState();
    popDepthStencilState();
    popBlendState();
    popVertexShader();
    popPixelShader();
}

//-----------------------------------------------------------------------------
//  Name : drawRectangle()
/// <summary>
/// Draw a simple rectangle to the screen with the specified color.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawRectangle( const cgRect & rcScreen, const cgColorValue & Color, bool bFilled )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Compile / load required shaders.
    cgVertexShaderHandle hVertexShader = mDriverShader->getVertexShader( _T("transformScreenElement") );
    cgPixelShaderHandle hPixelShader = mDriverShader->getPixelShader( _T("drawScreenElement") );
    if ( !hPixelShader.isValid() )
        return;

    // ToDo: Should probably optimize with a hardware VB
    // Build the screen space rectangle
    const cgUInt32 nColor = Color;
    cgScreenVertex pRectangleBuffer[] = {
        cgScreenVertex( (cgFloat)rcScreen.left, (cgFloat)rcScreen.top, 0, 1, nColor ),
        cgScreenVertex( (cgFloat)rcScreen.right, (cgFloat)rcScreen.top, 0, 1, nColor ),
        cgScreenVertex( (cgFloat)rcScreen.right, (cgFloat)rcScreen.bottom, 0, 1, nColor ),
        cgScreenVertex( (cgFloat)rcScreen.left, (cgFloat)rcScreen.bottom, 0, 1, nColor ),
        cgScreenVertex( (cgFloat)rcScreen.left, (cgFloat)rcScreen.top, 0, 1, nColor )
    };

    // Setup vertex and pixel shaders.
    pushVertexShader( hVertexShader );
    pushPixelShader( hPixelShader );

    // Setup blending states (basic alpha blending enabled)
    pushBlendState( mElementBlendState );

    // Disable depth buffering and stencil operations
    pushDepthStencilState( mElementDepthState );

    // Use default rasterizer states.
    pushRasterizerState( mDefaultRasterizerState );

    // Set the vertex format
    pushVertexFormat( mScreenSpaceFormat );

    // Draw
    if ( bFilled )
    {
        cgUInt32 Indices[] = { 0, 1, 3, 2 };
        drawIndexedPrimitiveUP( cgPrimitiveType::TriangleStrip, 0, 4, 2, Indices, cgBufferFormat::Index32, (void*)pRectangleBuffer );
    
    } // End if filled
    else
    {
        drawPrimitiveUP( cgPrimitiveType::LineStrip, 4, (void*)pRectangleBuffer );
    
    } // End if line

    // Restore modified states.
    popVertexFormat();
    popRasterizerState();
    popDepthStencilState();
    popBlendState();
    popVertexShader();
    popPixelShader();
}

//-----------------------------------------------------------------------------
//  Name : drawCircle()
/// <summary>
/// Draw a simple circle to the screen with the specified thickness and color.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawCircle( const cgVector2 & position, cgFloat radius, const cgColorValue & color, cgFloat thickness, cgUInt32 requestedSegments )
{
    drawCircle( position, radius, color, thickness, requestedSegments, 0, 360.0f );
}

//-----------------------------------------------------------------------------
//  Name : drawCircle()
/// <summary>
/// Draw a simple circle to the screen with the specified thickness and color.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawCircle( const cgVector2 & position, cgFloat radius, const cgColorValue & color, cgFloat thickness, cgUInt32 requestedSegments, cgFloat arcBeginDegrees, cgFloat arcEndDegrees )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Compile / load required shaders
    cgVertexShaderHandle vertexShader = mDriverShader->getVertexShader( _T("transformScreenElement") );
    cgPixelShaderHandle pixelShader = mDriverShader->getPixelShader( _T("drawScreenElement") );
    if ( !vertexShader.isValid() || !pixelShader.isValid() )
        return;

    const cgFloat arcBegin = CGEToRadian((arcBeginDegrees < arcEndDegrees) ? arcBeginDegrees : arcEndDegrees);
    const cgFloat arcEnd   = CGEToRadian((arcBeginDegrees < arcEndDegrees) ? arcEndDegrees : arcBeginDegrees);
    const cgUInt32 colorValue = color;
    const bool thickLine = ( thickness > 1.0f );
    const cgInt segments = (cgInt)requestedSegments;
    cgScreenVertex * circleBuffer = new cgScreenVertex[(thickLine) ? (segments+1)*2 : segments+1];
    if ( thickLine )
    {
        const cgFloat halfThickness = thickness * 0.5f;

        // Build a screen space unit circle.
        const cgFloat delta = ((arcEnd-arcBegin) / (cgFloat)segments);
        for ( cgInt i = 0, point = 0; i < segments + 1; ++i )
        {
            const cgFloat ratio = (delta * (cgFloat)i) + arcBegin;
            const cgFloat sinr = sinf(ratio);
            const cgFloat cosr = cosf(ratio);
            circleBuffer[point].position.x = position.x + (sinr * (radius - halfThickness));
            circleBuffer[point].position.y = position.y - (cosr * (radius - halfThickness));
            circleBuffer[point].position.z = 0.0f;
            circleBuffer[point].position.w = 1.0f;
            circleBuffer[point++].color    = colorValue;
            circleBuffer[point].position.x = position.x + (sinr * (radius + halfThickness));
            circleBuffer[point].position.y = position.y - (cosr * (radius + halfThickness));
            circleBuffer[point].position.z = 0.0f;
            circleBuffer[point].position.w = 1.0f;
            circleBuffer[point++].color    = colorValue;
        
        } // Next point

    } // End if variable thickness
    else
    {
        // Build a screen space unit circle.
        const cgFloat delta = ((arcEnd-arcBegin) / (cgFloat)segments);
        for ( cgInt i = 0; i < segments + 1; ++i )
        {
            const cgFloat ratio = (delta * (cgFloat)i) + arcBegin;
            const cgFloat sinr = sinf(ratio);
            const cgFloat cosr = cosf(ratio);
            circleBuffer[i].position.x = position.x + (sinr * radius);
            circleBuffer[i].position.y = position.y - (cosr * radius);
            circleBuffer[i].position.z = 0.0f;
            circleBuffer[i].position.w = 1.0f;
            circleBuffer[i].color      = colorValue;
        
        } // Next point

    } // End if 1 pixel thick

    // Setup vertex and pixel shaders.
    pushVertexShader( vertexShader );
    pushPixelShader( pixelShader );

    // Setup blending states (basic alpha blending enabled)
    pushBlendState( mElementBlendState );

    // Disable depth buffering and stencil operations
    pushDepthStencilState( mElementDepthState );

    // Use default rasterizer states.
    pushRasterizerState( mDefaultRasterizerState );

    // Set the vertex format
    pushVertexFormat( mScreenSpaceFormat );

    // Draw
    if ( thickLine )
        drawPrimitiveUP( cgPrimitiveType::TriangleStrip, segments*2, (void*)circleBuffer );
    else
        drawPrimitiveUP( cgPrimitiveType::LineStrip, segments, (void*)circleBuffer );
    
    // Restore modified states.
    popVertexFormat();
    popRasterizerState();
    popDepthStencilState();
    popBlendState();
    popVertexShader();
    popPixelShader();

    // Clean up
    delete []circleBuffer;
}

//-----------------------------------------------------------------------------
//  Name : drawEllipse()
/// <summary>
/// Draw a simple ellipse to the screen with the specified color.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawEllipse( const cgRect & rcScreen, const cgColorValue & Color, bool bFilled )
{
    // Validate requirements
    cgAssert( isInitialized() == true );

    // Compile / load required shaders
    cgVertexShaderHandle hVertexShader = mDriverShader->getVertexShader( _T("transformClipElement") );
    cgPixelShaderHandle hPixelShader = mDriverShader->getPixelShader( _T("drawScreenElement") );
    if ( !hVertexShader.isValid() || !hPixelShader.isValid() )
        return;

    // ToDo: Should probably optimize with a hardware VB.
    // Build a clip space unit circle.
    const cgUInt32 nColor = Color;
    const cgInt nSegments = 40;
    cgShadedVertex pCircleBuffer[nSegments + 1];
    for ( cgInt i = 0; i < nSegments + 1; ++i )
    {
        cgFloat fRatio = (CGE_TWO_PI / (cgFloat)nSegments) * (cgFloat)i;
        pCircleBuffer[i].position.x = sinf(fRatio);
        pCircleBuffer[i].position.y = cosf(fRatio);
        pCircleBuffer[i].position.z = 0.0f;
        pCircleBuffer[i].color      = nColor;
    
    } // Next point

    // Compute the transformation matrix to fit the circle 
    // into the ellipse rectangle.
    cgMatrix mtxTransform;
    cgMatrix::identity( mtxTransform );
    const cgViewport & Viewport = getViewport();
    mtxTransform._11 = rcScreen.width()  / (cgFloat)Viewport.width;
    mtxTransform._22 = rcScreen.height() / (cgFloat)Viewport.height;
    mtxTransform._41 = (cgFloat)rcScreen.left - ((cgFloat)(Viewport.width - rcScreen.width()) * 0.5f);
    mtxTransform._41 /= (cgFloat)Viewport.width * 0.5f;
    mtxTransform._42 = (cgFloat)rcScreen.top - ((cgFloat)(Viewport.height - rcScreen.height()) * 0.5f);
    mtxTransform._42 /= -(cgFloat)Viewport.height * 0.5f;
    pushWorldTransform( &mtxTransform );
    
    // Setup vertex and pixel shaders.
    pushVertexShader( hVertexShader );
    pushPixelShader( hPixelShader );

    // Setup blending states (basic alpha blending enabled)
    pushBlendState( mElementBlendState );

    // Disable depth buffering and stencil operations
    pushDepthStencilState( mElementDepthState );

    // Use default rasterizer states.
    pushRasterizerState( mDefaultRasterizerState );

    // Set the vertex format
    pushVertexFormat( mWorldSpaceFormat );

    // Draw
    if ( bFilled == true )
    {
        // Build triangle list indices
        cgUInt32 * pIndices = new cgUInt32[(nSegments-1) * 3];
        for ( cgInt i = 0; i < nSegments - 1; ++i )
        {
            pIndices[i*3]   = 0;
            pIndices[i*3+1] = i+1;
            pIndices[i*3+2] = i+2;

        } // Next point
        drawIndexedPrimitiveUP( cgPrimitiveType::TriangleList, 0, nSegments + 1, nSegments-1, pIndices, 
                                cgBufferFormat::Index32, (void*)pCircleBuffer );
        delete []pIndices;
    
    } // End if filled
    else
    {
        drawPrimitiveUP( cgPrimitiveType::LineStrip, nSegments, (void*)pCircleBuffer );
    
    } // End if line

    // Restore modified states.
    popWorldTransform( );
    popVertexFormat();
    popRasterizerState();
    popDepthStencilState();
    popBlendState();
    popVertexShader();
    popPixelShader();
}

//-----------------------------------------------------------------------------
//  Name : drawScreenQuad ()
/// <summary>
/// Draw a quad in screen space.
/// Note : Optional texture coordinates are specified in TL,TR,BR,BL order.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawScreenQuad( )
{
    // Pass through to main method
    return drawScreenQuad( cgSurfaceShaderHandle::Null, -1.0f, CG_NULL );
}
void cgRenderDriver::drawScreenQuad( const cgSurfaceShaderHandle & hShader )
{
    // Pass through to main method
    return drawScreenQuad( hShader, -1.0f, CG_NULL );
}
void cgRenderDriver::drawScreenQuad( const cgSurfaceShaderHandle & hShader, cgFloat fDepth )
{
    // Pass through to main method
    return drawScreenQuad( hShader, fDepth, CG_NULL );
}
void cgRenderDriver::drawScreenQuad( cgSurfaceShaderHandle hShader, cgFloat fDepth, const cgVector2 TexCoords[] )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
	
    // Acquire the current render target data
    const cgViewport & Viewport = getViewport( );
    
	// Ensure that we're directly mapping texels to pixels by offset by 0.5
	cgFloat fX      = (cgFloat)Viewport.position.x - 0.5f;
	cgFloat fY      = (cgFloat)Viewport.position.y - 0.5f;
	cgFloat fWidth  = (cgFloat)Viewport.size.width;
	cgFloat fHeight = (cgFloat)Viewport.size.height;

	// Select correct depth
	if ( fDepth < 0.0f )
		fDepth = 0.0f;

    // Build the quad
    cgScreenVertex  Quad[4];
	Quad[0].position  = cgVector4(fX, fY, fDepth, 1.0f);
	Quad[1].position  = cgVector4(fX + fWidth, fY, fDepth, 1.0f);
	Quad[2].position  = cgVector4(fX, fY + fHeight, fDepth, 1.0f);
	Quad[3].position  = cgVector4(fX + fWidth, fY + fHeight, fDepth, 1.0f);
    
    // Compute / assign texture coordinates
    if ( TexCoords )
    {
        // Specific texture coordinates provided.
        Quad[0].textureCoords = TexCoords[0];
	    Quad[1].textureCoords = TexCoords[1];
	    Quad[2].textureCoords = TexCoords[3];
	    Quad[3].textureCoords = TexCoords[2];
    
    } // End if supplied
    else
    {
        // Compute default texture coordinates.
        cgToDo( "Effect Overhaul", "Should this be viewport coords?" )
        float fTexX = 0;
        float fTexY = 0;
        float fTexX2 = 1.0f;
        float fTexY2 = 1.0f;
        Quad[0].textureCoords = cgVector2(fTexX, fTexY);
	    Quad[1].textureCoords = cgVector2(fTexX2, fTexY);
	    Quad[2].textureCoords = cgVector2(fTexX, fTexY2);
	    Quad[3].textureCoords = cgVector2(fTexX2, fTexY2);
    
    } // End if default
    
	// Set the vertex format
    pushVertexFormat( mScreenSpaceFormat );

    // If a surface shader was provided, run it. Otherwise, just draw the quad.
    cgSurfaceShader * pShader = hShader.getResource(true);
	if ( !pShader || !pShader->isLoaded() )
    {
        drawPrimitiveUP( cgPrimitiveType::TriangleStrip, 2, Quad );
    
    } // End if no shader loaded
	else
	{
        // Render with this shader
        if ( pShader->beginTechnique() )
        {
            while ( pShader->executeTechniquePass() == cgTechniqueResult::Continue )
                drawPrimitiveUP( cgPrimitiveType::TriangleStrip, 2, Quad );
            pShader->endTechnique();
        
        } // End if begun
        
    } // End if shader available

    // Restore any modified states.
    popVertexFormat( );
}

//-----------------------------------------------------------------------------
//  Name : drawViewportClipQuad ()
/// <summary>
/// Draw a quad in clip space. Uses driver's current viewport as texture
/// coordinates in order to extract the relevant area of the source texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawViewportClipQuad( )
{
    // Pass through to main method
    return drawViewportClipQuad( cgSurfaceShaderHandle::Null, -1.0f );
}
void cgRenderDriver::drawViewportClipQuad( const cgSurfaceShaderHandle & hShader )
{
    // Pass through to main method
    return drawViewportClipQuad( hShader, -1.0f );
}
void cgRenderDriver::drawViewportClipQuad( const cgSurfaceShaderHandle & hShader, cgFloat fDepth )
{
    // Caller wants driver's viewport values to be used to
    // extracting the relevant area of the texture.
    cgSize ViewSize = getActiveRenderView()->getSize();

    // ToDo: 9999 - The 0.5 offset should be handled by the relevant shader.

    // Generate U/V tex coords.
    const cgViewport & Viewport = getViewport();
    cgFloat fLeft   = ((cgFloat)Viewport.x + 0.5f) / (cgFloat)ViewSize.width;
    cgFloat fRight  = ((cgFloat)Viewport.x + (cgFloat)Viewport.width + 0.5f) / (cgFloat)ViewSize.width;
    cgFloat fTop    = ((cgFloat)Viewport.y + 0.5f) / (cgFloat)ViewSize.height;
    cgFloat fBottom = ((cgFloat)Viewport.y + (cgFloat)Viewport.height + 0.5f) / (cgFloat)ViewSize.height;

    // Pass through to main method
    cgVector2 TexCoords[4];
    TexCoords[0] = cgVector2( fLeft, fTop );
    TexCoords[1] = cgVector2( fRight, fTop );
    TexCoords[2] = cgVector2( fRight, fBottom );
    TexCoords[3] = cgVector2( fLeft, fBottom );
    drawClipQuad( hShader, fDepth, TexCoords );
}


//-----------------------------------------------------------------------------
//  Name : drawClipQuad ()
/// <summary>
/// Draw a fullscreen quad at the camera's Z axis location given by the input parameter.
/// Note : Optional texture coordinates are specified in TL,TR,BR,BL order.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawClipQuad( cgFloat fDepth, bool bLinearZ )
{
    drawClipQuad( cgSurfaceShaderHandle::Null, fDepth, CG_NULL, bLinearZ );
}
void cgRenderDriver::drawClipQuad( const cgSurfaceShaderHandle & hShader, cgFloat fDepth, bool bLinearZ )
{
    drawClipQuad( hShader, fDepth, CG_NULL, bLinearZ );
}
void cgRenderDriver::drawClipQuad( cgSurfaceShaderHandle hShader, cgFloat fDepth, const cgVector2 TexCoords[], bool bLinearZ )
{
    if ( bLinearZ )
    {
        // Get the currently active camera's projection matrix
        cgCameraNode *pCamera = getCamera();
        const cgMatrix & mtxProj = pCamera->getProjectionMatrix();
        
        // Ensure the user doesn't specify a coordinate behind the near plane
        float viewZ = max( fDepth, pCamera->getNearClip() );

        // Draw a clip quad with the Z coordinate projected to a depth value.
        drawClipQuad( hShader, (viewZ * mtxProj._33 + mtxProj._43) / viewZ, TexCoords );
    }
    else
    {
        // Draw a clip quad assuming non-linear Z depth provided
        drawClipQuad( hShader, fDepth, TexCoords );
    }
}

//-----------------------------------------------------------------------------
//  Name : drawClipQuad ()
/// <summary>
/// Draw a quad in clip space.
/// Note : Optional texture coordinates are specified in TL,TR,BR,BL order.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::drawClipQuad( )
{
    // Pass through to main method
    return drawClipQuad( cgSurfaceShaderHandle::Null, -1.0f, (const cgVector2*)CG_NULL );
}
void cgRenderDriver::drawClipQuad( cgFloat fDepth )
{
    // Pass through to main method
    return drawClipQuad( cgSurfaceShaderHandle::Null, fDepth, (const cgVector2*)CG_NULL );
}
void cgRenderDriver::drawClipQuad( const cgSurfaceShaderHandle & hShader )
{
    // Pass through to main method
    return drawClipQuad( hShader, -1.0f, (const cgVector2*)CG_NULL );
}
void cgRenderDriver::drawClipQuad( const cgSurfaceShaderHandle & hShader, cgFloat fDepth )
{
    // Pass through to main method
    return drawClipQuad( hShader, fDepth, (const cgVector2*)CG_NULL );
}
void cgRenderDriver::drawClipQuad( cgSurfaceShaderHandle hShader, cgFloat fDepth, const cgVector2 TexCoords[] )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    
    // Select correct depth
    if ( fDepth < 0.0f )
        fDepth = 1.0f;
    
    // Build the quad
    cgClipVertex ClipQuad[4];
    ClipQuad[0].position  = cgVector3(-1.0f, 1.0f, fDepth );
    ClipQuad[0].textureCoords = (TexCoords) ? TexCoords[0] : cgVector2(0, 0);
    ClipQuad[1].position  = cgVector3(1.0f, 1.0f, fDepth );
    ClipQuad[1].textureCoords = (TexCoords) ? TexCoords[1] : cgVector2(1, 0);
    ClipQuad[2].position  = cgVector3(-1.0f, -1.0f, fDepth );
    ClipQuad[2].textureCoords = (TexCoords) ? TexCoords[3] : cgVector2(0, 1);
    ClipQuad[3].position  = cgVector3(1.0f, -1.0f, fDepth );
    ClipQuad[3].textureCoords = (TexCoords) ? TexCoords[2] : cgVector2(1, 1);
    
    // Set the vertex format
    setVertexFormat( mClipSpaceFormat );

    // If a surface shader was provided, run it. Otherwise, just draw the quad.
    cgSurfaceShader * pShader = hShader.getResource( true );
	if ( !pShader || !pShader->isLoaded() )
    {
        drawPrimitiveUP( cgPrimitiveType::TriangleStrip, 2, ClipQuad );
    
    } // End if no shader loaded
	else
	{
        // Render with this shader
        if ( pShader->beginTechnique() )
        {
            while ( pShader->executeTechniquePass() == cgTechniqueResult::Continue )
                drawPrimitiveUP( cgPrimitiveType::TriangleStrip, 2, ClipQuad );
            pShader->endTechnique();
        
        } // End if begun
    
    } // End if shader available
}

//-----------------------------------------------------------------------------
//  Name : onDeviceLost () (Protected Virtual)
/// <summary>
/// Notify all systems that the device has been lost.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::onDeviceLost()
{
    cgMessage ResetMessage;

    // Send pre-reset message
    ResetMessage.messageId   = cgSystemMessages::RenderDriver_DeviceLost;
    ResetMessage.messageData = (void*)this;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver, &ResetMessage );

    /*// Unset all states in order to ensure that state filtering
    // does not interfere on the next render pass.
    mDepthStencilStateStack.top() = DepthStencilStateData();
    mRasterizerStateStack.top() = cgRasterizerStateHandle::Null;
    mBlendStateStack.top() = cgBlendStateHandle::Null;
    mVertexFormatStack.top() = CG_NULL;
    mIndicesStack.top() = cgIndexBufferHandle::Null;
    mVertexShaderStack.top() = cgVertexShaderHandle::Null;
    mPixelShaderStack.top() = cgPixelShaderHandle::Null;
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxSamplerSlots; ++i )
        mSamplerStateStack[i].top() = cgSamplerStateHandle::Null;
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxStreamSlots; ++i )
        mVertexStreamStack[i].top() = VertexStreamData();
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxTextureSlots; ++i )
        mTextureStack[i].top() = cgTextureHandle::Null;
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxConstantBufferSlots; ++i )
    {
        cgConstantBuffer * pBuffer = mConstantBufferStack[i].top().getResource(false);
        if ( pBuffer )
        {
            mConstantsDirty |= (1 << i);
            pBuffer->mBoundRegister = -1;

        } // End if valid

    } // Next CB*/
}

//-----------------------------------------------------------------------------
//  Name : onDeviceReset () (Private)
/// <summary>
/// Notify all systems that the device has been reset.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::onDeviceReset()
{
    cgMessage ResetMessage;

    // Send the post reset message
    ResetMessage.messageId   = cgSystemMessages::RenderDriver_DeviceRestored;
    ResetMessage.messageData = (void*)this;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver, &ResetMessage );

    // Restore all necessary resources that are state filtered.
    restoreDepthStencilState( );
    restoreRasterizerState( );
    restoreBlendState( );
    restoreIndices( );
    restoreVertexFormat( );
    restoreVertexShader( );
    restorePixelShader( );
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxConstantBufferSlots; ++i )
        restoreConstantBuffer( i );
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxTextureSlots; ++i )
        restoreTexture( i );
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxSamplerSlots; ++i )
        restoreSamplerState( i );
    for ( cgUInt32 i = 0; i < cgRenderDriver::MaxStreamSlots; ++i )
        restoreStreamSource( i );
    
}

//-----------------------------------------------------------------------------
//  Name : beginFrame ()
/// <summary>
/// Stubs to begin rendering process for this frame
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::beginFrame( )
{
    // Pass through to main method.
    return beginFrame( true, 0xFF000000 );
}
bool cgRenderDriver::beginFrame( bool bClearTarget )
{
    // Pass through to main method.
    return beginFrame( bClearTarget, 0xFF000000 );
}

//-----------------------------------------------------------------------------
//  Name : beginFrame () (Virtual)
/// <summary>
/// Begin rendering process for this frame
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::beginFrame( bool bClearTarget, cgUInt32 nTargetColor )
{
    // Reset counters.
    mPrimitivesDrawn = 0;

    // Update the internal camera property states
    if ( !cameraUpdated() )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : beginTargetRender()
/// <summary>
/// Stubs to prepares the render target to receive data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandle & hRenderTarget )
{
    // Pass through to main method.
    return beginTargetRender( hRenderTarget, -1, true, getActiveRenderView()->getDepthStencilBuffer() );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandle & hRenderTarget, const cgDepthStencilTargetHandle & hDepthStencilTarget )
{
    // Pass through to main method.
    return beginTargetRender( hRenderTarget, -1, true, hDepthStencilTarget );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandle & hRenderTarget, cgInt32 nCubeFace )
{
    // Pass through to main method.
    return beginTargetRender( hRenderTarget, nCubeFace, true, getActiveRenderView()->getDepthStencilBuffer() );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandle & hRenderTarget, cgInt32 nCubeFace, const cgDepthStencilTargetHandle & hDepthStencilTarget )
{
    // Pass through to main method.
    return beginTargetRender( hRenderTarget, nCubeFace, true, hDepthStencilTarget );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandle & hRenderTarget, cgInt32 nCubeFace, bool bAutoUseMultiSample )
{   
    // Pass through to main method.
    return beginTargetRender( hRenderTarget, nCubeFace, bAutoUseMultiSample, getActiveRenderView()->getDepthStencilBuffer() );
}

bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandleArray & aRenderTargets )
{
    // Pass through to main method
    return beginTargetRender( aRenderTargets, true, getActiveRenderView()->getDepthStencilBuffer() );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandleArray & aRenderTargets, const cgDepthStencilTargetHandle & hDepthStencilTarget )
{
    // Pass through to main method
    return beginTargetRender( aRenderTargets, true, hDepthStencilTarget );
}
bool cgRenderDriver::beginTargetRender( const cgRenderTargetHandleArray & aRenderTargets, bool bAutoUseMultiSample )
{
    // Pass through to main method
    return beginTargetRender( aRenderTargets, bAutoUseMultiSample, getActiveRenderView()->getDepthStencilBuffer() );
}

//-----------------------------------------------------------------------------
//  Name : beginViewRender() (Protected)
/// <summary>
/// Internal method used by render view to notify the driver about its active
/// status.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::beginViewRender( cgRenderView * pView )
{
    mRenderViewStack.push( pView );

    // Camera details need to be updated to match aspect ratio
    // of new view.
    cameraUpdated();
}

//-----------------------------------------------------------------------------
//  Name : endViewRender() (Protected)
/// <summary>
/// Internal method used by render view to notify the driver about its active
/// status.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endViewRender( )
{
    mRenderViewStack.pop();

    // Camera details need to be updated to match aspect ratio
    // of new view.
    cameraUpdated();
}

//-----------------------------------------------------------------------------
//  Name : endFrame ()
/// <summary>
/// Stubs to end rendering process for this frame and present.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endFrame( )
{
    // Pass through to main method
    endFrame( CG_NULL, true );
}

//-----------------------------------------------------------------------------
//  Name : endFrame ()
/// <summary>
/// Stubs to end rendering process for this frame and optionally present.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endFrame( bool bPresent )
{
    // Pass through to main method
    endFrame( CG_NULL, bPresent );
}

//-----------------------------------------------------------------------------
//  Name : endFrame ()
/// <summary>
/// Stubs to end rendering process for this frame and present.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endFrame( cgAppWindow * pWndOverride )
{
    // Pass through to main method
    endFrame( pWndOverride, true );
}

//-----------------------------------------------------------------------------
//  Name : endFrame () (Virtual)
/// <summary>
/// End rendering process for this frame and present.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endFrame( cgAppWindow * pWndOverride, bool bPresent )
{
    // ToDo: 9999 - Consider making these debug asserts with
    // perhaps optional silent recovery in release?

    // Validate stacks.
    // Ensure that all active render views are removed
    if ( mRenderViewStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched render view 'begin' / 'end' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mRenderViewStack.size() > 2 )
            mRenderViewStack.pop();
        
        // Perform actual restoration of final element.
        endViewRender();
        
    } // End if not empty

    // Ensure that all active render passes are removed
    if ( mRenderPassStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushRenderPass' / 'popRenderPass' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mRenderPassStack.size() > 2 )
            mRenderPassStack.pop();
        
        // Perform actual restoration of final element.
        popRenderPass();
        
    } // End if not empty

    // Ensure that all active override methods are removed
    if ( mOverrideMethodStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushOverrideMethod' / 'popOverrideMethod' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mOverrideMethodStack.size() > 2 )
            mOverrideMethodStack.pop();

        // Perform actual restoration of final element.
        popOverrideMethod();
        
    } // End if not empty

    // Ensure that all active depth stencil states are removed
    if ( mDepthStencilStateStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushDepthStencilState' / 'popDepthStencilState' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mDepthStencilStateStack.size() > 2 )
            mDepthStencilStateStack.pop();

        // Perform actual restoration of final element.
        popDepthStencilState();
        
    } // End if not empty

    // Ensure that all active rasterizer states are removed
    if ( mRasterizerStateStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushRasterizerState' / 'popRasterizerState' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mRasterizerStateStack.size() > 2 )
            mRasterizerStateStack.pop();

        // Perform actual restoration of final element.
        popRasterizerState();
        
    } // End if not empty

    // Ensure that all active blend states are removed
    if ( mBlendStateStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushBlendState' / 'popBlendState' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mBlendStateStack.size() > 2 )
            mBlendStateStack.pop();

        // Perform actual restoration of final element.
        popBlendState();
        
    } // End if not empty

    // Ensure that all active vertex shaders are removed
    if ( mVertexShaderStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushVertexShader' / 'popVertexShader' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mVertexShaderStack.size() > 2 )
            mVertexShaderStack.pop();

        // Perform actual restoration of final element.
        popVertexShader();
        
    } // End if not empty

    // Ensure that all active pixel shaders are removed
    if ( mPixelShaderStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushPixelShader' / 'popPixelShader' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mPixelShaderStack.size() > 2 )
            mPixelShaderStack.pop();

        // Perform actual restoration of final element.
        popPixelShader();
        
    } // End if not empty

    // Ensure that all active world transforms are removed
    if ( mWorldTransformStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushWorldTransform' / 'popWorldTransform' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mWorldTransformStack.size() > 2 )
            mWorldTransformStack.pop();

        // Perform actual restoration of final element.
        popWorldTransform();
        
    } // End if not empty

    // Ensure that all active cameras are removed
    if ( mCameraStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushCamera' / 'popCamera' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mCameraStack.size() > 2 )
            mCameraStack.pop_back();

        // Perform actual restoration of final element.
        popCamera();
        
    } // End if not empty

    // Ensure that all active clip planes are removed
    if ( mClipPlaneStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushClipPlanes' / 'popClipPlanes' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mClipPlaneStack.size() > 2 )
            mClipPlaneStack.pop();

        // Perform actual restoration of final element.
        popUserClipPlanes();
        
    } // End if not empty

    // Ensure that all active vertex formats are removed
    if ( mVertexFormatStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushVertexFormat' / 'popVertexFormat' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mVertexFormatStack.size() > 2 )
            mVertexFormatStack.pop();

        // Perform actual restoration of final element.
        popVertexFormat();
        
    } // End if not empty

    // Ensure that all active material filters are removed
    if ( mMaterialFilterStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushMaterialFilter' / 'popMaterialFilter' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mMaterialFilterStack.size() > 2 )
        {
            delete mMaterialFilterStack.top();
            mMaterialFilterStack.pop();
        
        } // Next filter

        // Perform actual restoration of final element.
        popMaterialFilter();
        
    } // End if not empty

    // Ensure that all active scissor rectangles are removed
    if ( mScissorRectStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushScissorRect' / 'popScissorRect' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mScissorRectStack.size() > 2 )
            mScissorRectStack.pop();

        // Perform actual restoration of final element.
        popScissorRect();
        
    } // End if not empty

    // Ensure that all active index buffers are removed
    if ( mIndicesStack.size() > 1 )
    {
        static bool bLog = true;
        if ( bLog )
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushIndices' / 'popIndices' calls detected. Future messages will be surpressed.\n") );
        bLog = false;
        while ( mIndicesStack.size() > 2 )
            mIndicesStack.pop();

        // Perform actual restoration of final element.
        popIndices();
        
    } // End if not empty

    // Ensure that all active sampler states are removed
    for ( size_t i = 0; i < MaxSamplerSlots; ++i )
    {
        if ( mSamplerStateStack[i].size() > 1 )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushSamplerState' / 'popSamplerState' calls detected. Future messages will be surpressed.\n") );
            bLog = false;
            while ( mSamplerStateStack[i].size() > 2 )
                mSamplerStateStack[i].pop();

            // Perform actual restoration of final element.
            popSamplerState( i );
            
        } // End if not empty
    
    } // Next Sampler

    // Ensure that all active vertex streams are removed
    for ( size_t i = 0; i < MaxStreamSlots; ++i )
    {
        if ( mVertexStreamStack[i].size() > 1 )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushStreamSource' / 'popStreamSource' calls detected. Future messages will be surpressed.\n") );
            bLog = false;
            while ( mVertexStreamStack[i].size() > 2 )
                mVertexStreamStack[i].pop();

            // Perform actual restoration of final element.
            popStreamSource( i );
            
        } // End if not empty
    
    } // Next Vertex Stream

    // Ensure that all active constant buffers are removed
    for ( size_t i = 0; i < MaxConstantBufferSlots; ++i )
    {
        if ( mConstantBufferStack[i].size() > 1 )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushConstantBuffer' / 'popConstantBuffer' calls detected. Future messages will be surpressed.\n") );
            bLog = false;
            while ( mConstantBufferStack[i].size() > 2 )
                mConstantBufferStack[i].pop();

            // Perform actual restoration of final element.
            popConstantBuffer( i );
            
        } // End if not empty
    
    } // Next Constant Buffer

    // Ensure that all active textures are removed
    for ( size_t i = 0; i < MaxTextureSlots; ++i )
    {
        if ( mTextureStack[i].size() > 1 )
        {
            static bool bLog = true;
            if ( bLog )
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Mismatched 'pushTexture' / 'popTexture' calls detected. Future messages will be surpressed.\n") );
            bLog = false;
            while ( mTextureStack[i].size() > 2 )
                mTextureStack[i].pop();

            // Perform actual restoration of final element.
            popTexture( i );
            
        } // End if not empty
    
    } // Next Texture
}

//-----------------------------------------------------------------------------
//  Name : beginMaterialRender()
/// <summary>
/// Prepare to execute the necessary set of rendering passes relating to a
/// "rendering technique" defined within the currently applied material's 
/// surface shader (if any). The technique to execute will be automatically
/// selected based on the name of the current rendering pass (setRenderPass()). 
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::beginMaterialRender( )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    if ( !mCurrentSurfaceShader )
        return false;
    
    // Begin rendering.
    return mCurrentSurfaceShader->beginTechnique();
}

//-----------------------------------------------------------------------------
//  Name : beginMaterialRender()
/// <summary>
/// Prepare to execute the necessary set of rendering passes relating to the 
/// specified "rendering technique" defined within the currently applied 
/// material's surface shader (if any). If an empty technique identifier is 
/// supplied, the name of the current rendering pass (setRenderPass()) will be 
/// assumed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::beginMaterialRender( const cgString & strTechnique )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    if ( !mCurrentSurfaceShader )
        return false;
    
    // Begin rendering.
    return mCurrentSurfaceShader->beginTechnique( strTechnique );
} 

//-----------------------------------------------------------------------------
//  Name : executeMaterialPass()
/// <summary>
/// Execute the next rendering pass for the current material.
/// </summary>
//-----------------------------------------------------------------------------
cgTechniqueResult::Base cgRenderDriver::executeMaterialPass( )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    if ( !mCurrentSurfaceShader )
        return cgTechniqueResult::Abort;
    
    // Begin rendering.
    return mCurrentSurfaceShader->executeTechniquePass();
}

//-----------------------------------------------------------------------------
//  Name : commitChanges()
/// <summary>
/// Allow the materials' currently assigned surface change to re-process any
/// states and / or settings based on recently modified parameters. It is only
/// necessary to call this method if a material pass has already begun
/// (executeMaterialPass()) and states for that pass need to be updated.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::commitChanges( )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    if ( !mCurrentSurfaceShader )
        return false;
    
    // Begin rendering.
    return mCurrentSurfaceShader->commitChanges();
}

//-----------------------------------------------------------------------------
//  Name : endMaterialRender ()
/// <summary>
/// Notify the render driver that material rendering is completed.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::endMaterialRender( )
{
    // Validate requirements
    cgAssert( isInitialized() == true );
    if ( !mCurrentSurfaceShader )
        return;
    
    // Begin rendering.
    mCurrentSurfaceShader->endTechnique();
}

//-----------------------------------------------------------------------------
//  Name : checkQueryResults()
/// <summary>
/// Stub for attempting to retrieve results of an active query.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::checkQueryResults( cgInt32 nQueryId, cgUInt32 & nPixelCount )
{
    // Pass through to main method.
    return checkQueryResults( nQueryId, nPixelCount, false );
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::processMessage( cgMessage * pMessage )
{
    // Bail if the message source has been unregistered since it
    // was sent. It will no longer be of interest to us.
    if ( pMessage->sourceUnregistered == true )
        return cgReference::processMessage( pMessage );

    // Who is the source of the message?
    if ( mOutputWindow != CG_NULL && pMessage->fromId == mOutputWindow->getReferenceId() )
    {
        // This is from our OS window. What is the message?
        if ( pMessage->messageId == cgSystemMessages::AppWindow_OnSize )
        {
            // Process the resize event
            cgWindowSizeEventArgs * pArgs = (cgWindowSizeEventArgs*)pMessage->messageData;
            if ( pArgs->minimized == false )
            {
                // Window is now active
                mActive = true;

                // Trigger a resize event if we're in windowed mode
                if ( isWindowed() && !mSuppressResizeEvent  )
                    windowResized( pArgs->size.width, pArgs->size.height );

            } // End if !minimized
            else
            {
                // Window not active
                mActive = false;
            
            } // End if minimized

            // We processed this message.
            return true;

        } // End if AppWindow_OnSize

    } // End if window message

    // Message was not processed, pass to base.
    return cgReference::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : windowResized() (Virtual)
/// <summary>
/// Notify the render driver that the render window was resized.
/// Note : This should only be called directly by the application if the render 
/// driver was initialized using a custom window.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderDriver::windowResized( cgInt32 nWidth, cgInt32 nHeight )
{
    // Update the aspect ratio of every camera on the stack.
    CameraStack::iterator itCamera;
    cgFloat fAspect = (cgFloat)nWidth / (cgFloat)nHeight;
    for ( itCamera = mCameraStack.begin(); itCamera != mCameraStack.end(); ++itCamera )
    {
        cgCameraNode * pCamera = *itCamera;

        // Update the camera if applicable
        if ( pCamera && !pCamera->isAspectLocked() )
            pCamera->setAspectRatio( fAspect );

    } // Next Camera

    // Send layout change message.
    cgMessage LayoutMessage;
    LayoutMessage.messageId   = cgSystemMessages::RenderDriver_ScreenLayoutChange;
    LayoutMessage.messageData = (void*)this;
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver, &LayoutMessage );
}

//-----------------------------------------------------------------------------
//  Name : stretchRect ()
/// <summary>
/// Overloads for the stretchRect function
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderDriver::stretchRect( const cgTextureHandle & hSource, const cgTextureHandle & hDestination, cgFilterMethod::Base Filter )
{
    cgTextureHandle hSrc = hSource;
    return stretchRect( hSrc, CG_NULL, hDestination, CG_NULL, Filter );
}
bool cgRenderDriver::stretchRect( const cgTextureHandle & hSource, const cgTextureHandle & hDestination )
{
    cgTextureHandle hSrc = hSource;
    return stretchRect( hSrc, CG_NULL, hDestination, CG_NULL, cgFilterMethod::None );
}
bool cgRenderDriver::stretchRect( const cgTextureHandle & hSource, const cgRect * pSrcRect, const cgTextureHandle & hDestination, const cgRect * pDstRect )
{
    cgTextureHandle hSrc = hSource;
    return stretchRect( hSrc, pSrcRect, hDestination, pDstRect, cgFilterMethod::None );
}

///////////////////////////////////////////////////////////////////////////////
// cgRenderView Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRenderView () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView::cgRenderView( cgRenderDriver * pDriver, const cgString & strName, bool bPrimaryView ) : cgReference( cgReferenceManager::generateInternalRefId( ) )
{
    // Initialize variables to sensible defaults
    mDriver       = pDriver;
    mName       = strName;
    mRendering    = false;
    mViewLost     = true;
    mPrimaryView  = bPrimaryView;
    mScaleMode     = cgScaleMode::Absolute;
    mLayout        = cgRectF( 0,0,0,0 );
    mRenderCount  = 0;
    
    // Register us with the render driver group so that we
    // can response to device events.
    cgReferenceManager::subscribeToGroup( getReferenceId(), cgSystemMessageGroups::MGID_RenderDriver );

    // Add to the driver's list of active views.
    pDriver->mRenderViews[strName] = this;
}

//-----------------------------------------------------------------------------
//  Name : ~cgRenderView () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderView::~cgRenderView()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgRenderView::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Forcably end rendering if we haven't already (no present).
    end( false );

    // Release any active resources loaded via the resource manager.
    releaseBuffers();

    // Remove from the driver's list of allocated views.
    mDriver->mRenderViews.erase( mName );
    
    // Call base class implementation if required.
    if ( bDisposeBase == true )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_RenderView )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Initialize the render view. Creates internal full-sized buffers.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::initialize( )
{
    return initialize( cgScaleMode::Relative, cgRectF( 0, 0, 1, 1 ) );
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Initialize the render view. Creates internal buffers to match the proposed 
/// layout. With standard (non floating point) rectangle, scale mode is always
/// set to 'absolute'. See other overloads for additional features.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::initialize( const cgRect & Layout )
{
    return initialize( cgScaleMode::Absolute, cgRectF( (cgFloat)Layout.left, (cgFloat)Layout.top, (cgFloat)Layout.right, (cgFloat)Layout.bottom ) );
}

//-----------------------------------------------------------------------------
//  Name : initialize()
/// <summary>
/// Initialize the render view. Creates internal buffers to match the proposed 
/// layout. If scale mode is set to 'absolute', then the layout rectangle 
/// describes pixel offset / size. With a scale mode of 'relative', the layout 
/// rectangle describes an offset / size in the zero to one range designed to 
/// represent some portion of the current screen size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::initialize( cgScaleMode::Base ScaleMode, const cgRectF & Layout )
{
    cgAssert( mDriver != CG_NULL );

    // Access required systems.
    cgResourceManager * pResources = mDriver->getResourceManager();

    // Cache the main device frame buffer to save us having
    // to retrieve this during each call to 'present()'.
    if ( !pResources->getRenderTarget( &mDeviceFrameBuffer, _T("Core::Device::FrameBuffer") ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve device frame buffer while creating new render view.\n") );
        return false;
    
    } // End if not found

    // If this is the primary view, the view buffer represents the standard
    // device frame buffer. Otherwise, we initialize the view based on the
    // proposed layout.
    if ( mPrimaryView )
    {
        mScaleMode = cgScaleMode::Relative;
        mLayout = cgRectF( 0, 0, 1, 1 );
        if ( !createBuffers() )
            return false;
    
    } // End if primary view
    else
    {
        if ( !setLayout( ScaleMode, Layout ) )
            return false;
    
    } // End if !primary view

    // Begin the garbage collection scheme
    sendGarbageMessage();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : isViewLost()
/// <summary>
/// Determine if the data in the render view is lost (i.e. buffers were resized
/// since the last time we rendered to it). A call to 'begin()' resets this
/// state automatically.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::isViewLost( ) const
{
    return mViewLost;
}

//-----------------------------------------------------------------------------
//  Name : setLayout()
/// <summary>
/// Set the position and size of the render view. Initializes internal buffers
/// to match the proposed layout. With standard (non floating point) rectangle, 
/// scale mode is always set to 'absolute'. See other overloads for additional 
/// features.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::setLayout( const cgRect & Layout )
{
    return setLayout( cgScaleMode::Absolute, cgRectF( (cgFloat)Layout.left, (cgFloat)Layout.top, (cgFloat)Layout.right, (cgFloat)Layout.bottom ) );
}

//-----------------------------------------------------------------------------
//  Name : setLayout()
/// <summary>
/// Set the position and size of the render view. Initializes internal buffers
/// to match the proposed layout. If scale mode is set to 'absolute', then the
/// layout rectangle describes pixel offset / size. With a scale mode of 
/// 'relative', the layout rectangle describes an offset / size in the zero to
/// one range designed to represent some portion of the current screen size.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::setLayout( cgScaleMode::Base ScaleMode, const cgRectF & Layout )
{
    cgAssert( mDriver != CG_NULL );

    // Layout changes are not supported for the primary view
    if ( mPrimaryView )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Layout changes are not supported for the primary render view.\n") );
        return false;
    
    } // End if primary

    // No-op?
    if ( ScaleMode == mScaleMode && Layout == mLayout )
        return true;

    // Forcably end rendering (no present) if we haven't already.
    end( false );

    // In order to determine if the view surfaces need to be reallocated,
    // we only need to know if the *size* of the view changed.
    cgSize OldSize = getSize();

    // Store layout information
    mScaleMode = ScaleMode;
    mLayout = Layout;

    // Release previous buffers only if the size has changed.
    cgSize NewSize = getSize();
    if ( OldSize != NewSize )
        releaseBuffers();

    // A view size of 0 on either axis indicates that the view is disabled. 
    // This is a success in either case, but will prevent rendering to the buffer -- later calls
    // to 'begin()' will fail.
    if ( NewSize.width == 0 || NewSize.height == 0 )
        return true;

    // Now we need to allocate the new buffer target(s).
    if ( OldSize != NewSize || !mViewBuffer.isValid() )
    {
        if ( !createBuffers() )
            return false;
    
    } // End if create buffers
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : releaseBuffers() (Protected)
/// <summary>
/// Release any currently allocated render surfaces.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderView::releaseBuffers( )
{
    mViewBuffer.close(true);
    mDepthStencilBuffer.close(true);
    
    // Clear out render targets
    RenderSurfaceMap::iterator itSurface;
    for ( itSurface = mRenderSurfaces.begin(); itSurface != mRenderSurfaces.end(); ++itSurface )
        itSurface->second.handle.close(true);
    mRenderSurfaces.clear();

    // Clear out depth stencil targets
    DepthStencilSurfaceMap::iterator itDepthSurface;
    for ( itDepthSurface = mDepthStencilSurfaces.begin(); itDepthSurface != mDepthStencilSurfaces.end(); ++itDepthSurface )
        itDepthSurface->second.handle.close(true);
    mDepthStencilSurfaces.clear();

    // Any prior valid view data is now lost.
    mViewLost = true;
}

//-----------------------------------------------------------------------------
//  Name : createBuffers() (Protected)
/// <summary>
/// Create the necessary render view buffers.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::createBuffers( )
{
    cgAssert( mDriver != CG_NULL );

    // If this is the primary view, the view buffer represents the standard
    // device frame buffer. Otherwise, we create targets appropriate for the
    // proposed layout.
    if ( mPrimaryView )
    {
        // View buffer *is* the device frame buffer.
        mViewBuffer = mDeviceFrameBuffer;

        // Depth stencil buffer matches any primary device buffer.
        cgResourceManager * pResources = mDriver->getResourceManager();
        pResources->getDepthStencilTarget( &mDepthStencilBuffer, _T("Core::Device::DepthStencil") );
    
    } // End if primary
    else
    {
        // Access required systems / values.
        cgResourceManager * pResources = mDriver->getResourceManager();
        const cgBufferFormatEnum & Formats = pResources->getBufferFormats();
        cgSize ViewSize = getSize();

        // Cannot allocate if size of either axis is zero (essentially disabled).
        // This is not a failure case however and thus we return a 'true' result.
        if ( ViewSize.width == 0 || ViewSize.height == 0 )
            return true;

        // Allocate new view buffer target. This is the fixed 32bit UNORM 'Frame' buffer for this render view.
        cgImageInfo Desc;
        Desc.width      = ViewSize.width;
        Desc.height     = ViewSize.height;
        Desc.mipLevels  = 1;
        Desc.format = Formats.getBestFourChannelFormat( cgBufferType::RenderTarget, false, true, false );
        cgString strName = cgString::format( _T("Core::RenderView(0x%x)::FrameBuffer"), getReferenceId() );
        if ( !pResources->createRenderTarget( &mViewBuffer, Desc, cgResourceFlags::ForceNew, strName, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create primary frame buffer render target for new render view.\n") );
            return false;
        
        } // End if failed

        // Also allocate new depth stencil target of the same size. We'll prefer readable depth-stencil buffers.
		if ( mDriver->getCapabilities()->supportsDepthStencilReading() )
			Desc.format = cgBufferFormat::INTZ; // ToDo: 6767 - Make this more robust.
		else
			Desc.format = Formats.getBestDepthFormat( false, true );

        strName = cgString::format( _T("Core::RenderView(0x%x)::DepthStencil"), getReferenceId() );
        if ( !pResources->createDepthStencilTarget( &mDepthStencilBuffer, Desc, cgResourceFlags::ForceNew, strName, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create primary depth stencil target for new render view.\n") );
            return false;
        
        } // End if failed

    } // End if !primary

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : readableDepthStencilBuffer() (Public)
/// <summary>
/// Is the primary depth-stencil buffer readable?
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::readableDepthStencilBuffer( )
{
	return mDriver->getCapabilities()->supportsDepthStencilReading();
}

//-----------------------------------------------------------------------------
//  Name : getLayout()
/// <summary>
/// Retrieve the currently assigned layout of the render view.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderView::getLayout( cgScaleMode::Base & ScaleMode, cgRectF & Layout ) const
{
    ScaleMode = mScaleMode;
    Layout    = mLayout;
}

//-----------------------------------------------------------------------------
//  Name : getRectangle()
/// <summary>
/// Retrieve the rectangle describing the position and size of the render view
/// in pixel coordinates.
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgRenderView::getRectangle( ) const
{
    cgAssert( mDriver != CG_NULL );
    if ( mScaleMode == cgScaleMode::Absolute )
    {
        return cgRect( (cgInt32)mLayout.left, (cgInt32)mLayout.top, 
                       (cgInt32)mLayout.right, (cgInt32)mLayout.bottom );
    
    } // End if Absolute
    else
    {
        cgSize ScreenSize = mDriver->getScreenSize();
        return cgRect( (cgInt32)(mLayout.left * ScreenSize.width), (cgInt32)(mLayout.top * ScreenSize.height), 
                       (cgInt32)(mLayout.right * ScreenSize.width), (cgInt32)(mLayout.bottom * ScreenSize.height) );
    
    } // End if Relative
}

//-----------------------------------------------------------------------------
//  Name : getPosition()
/// <summary>
/// Retrieve a point describing the position of the render view in pixel 
/// coordinates.
/// </summary>
//-----------------------------------------------------------------------------
cgPoint cgRenderView::getPosition( ) const
{
    cgAssert( mDriver != CG_NULL );
    if ( mScaleMode == cgScaleMode::Absolute )
    {
        return cgPoint( (cgInt32)mLayout.left, (cgInt32)mLayout.top );
    
    } // End if Absolute
    else
    {
        cgSize ScreenSize = mDriver->getScreenSize();
        return cgPoint( (cgInt32)(mLayout.left * ScreenSize.width), (cgInt32)(mLayout.top * ScreenSize.height) );
    
    } // End if Relative
}

//-----------------------------------------------------------------------------
//  Name : getPosition()
/// <summary>
/// Retrieve the current size of the render view in pixel coordinates.
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgRenderView::getSize( ) const
{
    cgAssert( mDriver != CG_NULL );
    if ( mScaleMode == cgScaleMode::Absolute )
    {
        return cgSize( (cgInt32)mLayout.width(), (cgInt32)mLayout.height() );
    
    } // End if Absolute
    else
    {
        cgSize ScreenSize = mDriver->getScreenSize();
        return cgSize( (cgInt32)(mLayout.width() * ScreenSize.width), (cgInt32)(mLayout.height() * ScreenSize.height) );
    
    } // End if Relative
}

//-----------------------------------------------------------------------------
//  Name : getRenderCount()
/// <summary>
/// Retrieve the number of times that this view has successfully entered
/// a rendering state (begin()/end())
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgRenderView::getRenderCount( ) const
{
	return mRenderCount;
}

//-----------------------------------------------------------------------------
//  Name : begin()
/// <summary>
/// Begin rendering to this render view.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::begin( )
{
    cgAssert( mDriver != CG_NULL );
    cgAssert( mViewBuffer.isValid() );
    cgAssertEx( mRendering == false, "Cannot begin rendering to a view that is already active." );

    // Mark as active view.
    mDriver->beginViewRender( this );

    // Begin rendering to the view's 'frame' buffer by default.
    if ( !mDriver->beginTargetRender( mViewBuffer, mDepthStencilBuffer ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to bind internal device render and depth stencil targets to the selected device for render view 0x%x.\n"), getReferenceId() );
        return false;

    } // End if failed

    // Clear depth stencil buffer
    cgToDo( "Effect Overhaul", "Should we clear by default, or should it be the responsibility of the app to call Driver->Clear()?" )
    //if ( mDepthStencilBuffer.isValid() )
        //mDriver->Clear( cgClearFlags::Depth | cgClearFlags::Stencil, 0, 1.0f, 0 );

    // We are now rendering with this view.
    mRendering = true;
    mViewLost = false;
    
    // Increment the render counter
	mRenderCount++;

	// Success
	return true;
}

//-----------------------------------------------------------------------------
//  Name : end()
/// <summary>
/// Rendering operations for this render view have been completed. The contents
/// of the view buffer will automatically be presented to the device frame 
/// buffer based on the current view layout.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::end( )
{
    cgAssert( mDriver != CG_NULL );

    // Bail if rendering was not begun
    if ( !mRendering )
        return true;

    // Finish rendering to the view's 'frame' buffer.
    mDriver->endTargetRender();

    // View is no longer active.
    mDriver->endViewRender( );

    // Rendering is complete. Present to the device frame buffer.
    mRendering = false;
    return present();
}

//-----------------------------------------------------------------------------
//  Name : end()
/// <summary>
/// Rendering operations for this render view have been completed. The contents
/// of the view buffer can be optionally presented to the device frame  buffer 
/// based on the current view layout.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::end( bool bPresent )
{
    cgAssert( mDriver != CG_NULL );

    // Bail if rendering was not begun
    if ( !mRendering )
        return true;

    // Finish rendering to the view's 'frame' buffer.
    mDriver->endTargetRender();

    // View is no longer active.
    mDriver->endViewRender( );

    // Rendering is complete. Present to the device frame buffer if requested.
    mRendering = false;
    if ( bPresent )
        return present();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : present()
/// <summary>
/// Present the contents of the view buffer to the device frame buffer based on 
/// the current view layout.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::present( )
{
    // Presenting the primary view is a no-op.
    if ( mPrimaryView )
        return true;

    // Copy into the device frame buffer
    cgAssert( mDriver != CG_NULL );
    cgAssert( mViewBuffer.isValid() );
    cgAssert( mDeviceFrameBuffer.isValid() );
    return mDriver->stretchRect( mViewBuffer, CG_NULL, mDeviceFrameBuffer, &getRectangle() );
}

//-----------------------------------------------------------------------------
//  Name : getViewBuffer()
/// <summary>
/// Retrieve the render view's 32 bit (ARGB) rendering buffer that will store
/// the final rendered output for this view.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getViewBuffer( ) const
{
    return mViewBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilBuffer()
/// <summary>
/// Retrieve the render view's unique depth stencil buffer.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilBuffer( ) const
{
    return mDepthStencilBuffer;
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a full sized rendering surface (a.k.a. G-Buffer) of the requested
/// format that can be used as temporary storage for this render view. 
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format )
{
    return getRenderSurface( Format, 1.0f, 1.0f, cgMultiSampleType::None, 0, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a full sized rendering surface (a.k.a. G-Buffer) of the requested
/// format that can be used as temporary storage for this render view. A
/// specific surface instance name / identifier can be supplied to ensure that
/// the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, const cgString & strInstanceId )
{
    return getRenderSurface( Format, 1.0f, 1.0f, cgMultiSampleType::None, 0, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a rendering surface (a.k.a. G-Buffer) of the requested format and
/// dimensions that can be used as temporary storage for this render view.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight )
{
    return getRenderSurface( Format, fScalarWidth, fScalarHeight, cgMultiSampleType::None, 0, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a rendering surface (a.k.a. G-Buffer) of the requested format and
/// dimensions that can be used as temporary storage for this render view. A 
/// specific surface instance name / identifier can be supplied to ensure that
/// the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, const cgString & strInstanceId )
{
    return getRenderSurface( Format, fScalarWidth, fScalarHeight, cgMultiSampleType::None, 0, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a full sized rendering surface (a.k.a. G-Buffer) of the requested
/// format that can be used as temporary storage for this render view. 
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality )
{
    return getRenderSurface( Format, 1.0f, 1.0f, MultiSampleType, nMultiSampleQuality, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a full sized rendering surface (a.k.a. G-Buffer) of the requested
/// format that can be used as temporary storage for this render view. A
/// specific surface instance name / identifier can be supplied to ensure that
/// the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality, const cgString & strInstanceId )
{
    return getRenderSurface( Format, 1.0f, 1.0f, MultiSampleType, nMultiSampleQuality, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a rendering surface (a.k.a. G-Buffer) of the requested format and
/// dimensions that can be used as temporary storage for this render view.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality )
{
    return getRenderSurface( Format, fScalarWidth, fScalarHeight, MultiSampleType, nMultiSampleQuality, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getRenderSurface()
/// <summary>
/// Retrieve a rendering surface (a.k.a. G-Buffer) of the requested format and
/// dimensions that can be used as temporary storage for this render view. A 
/// specific surface instance name / identifier can be supplied to ensure that
/// the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgRenderView::getRenderSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality, const cgString & strInstanceId )
{
    cgAssert( mDriver != CG_NULL );

    // Build an appropriate description structure for this surface ready for
    // creation if it doesn't already exists. 
    cgImageInfo Desc;
    cgSize ViewSize = getSize();
    Desc.width      = (cgInt32)ceil(fScalarWidth  * (cgFloat)ViewSize.width);
    Desc.height     = (cgInt32)ceil(fScalarHeight * (cgFloat)ViewSize.height);
    Desc.format     = Format;
    Desc.mipLevels  = 1;
    
    // Store the multisampling info
    Desc.multiSampleType    = MultiSampleType;
    Desc.multiSampleQuality = nMultiSampleQuality;

    // Clamp to minimum size
    if ( Desc.width < 1 )
        Desc.width = 1;
    if ( Desc.height < 1 )
        Desc.height = 1;

    // Build an appropriate resource identifier string appropriate for the 
    // specified surface details.
    cgStringParser strResource;
    strResource << _T("RenderViewSurface::");
    strResource << Desc.width;
    strResource << _T("x");
    strResource << Desc.height;
    strResource << _T("x");
    strResource << (cgInt32)Desc.format;
    if ( MultiSampleType != cgMultiSampleType::None )
    {
        strResource << _T(" MSAA") << (cgUInt32)MultiSampleType  << _T("X");
        if ( nMultiSampleQuality != 0 )
            strResource << _T("(") << nMultiSampleQuality << _T(")");
    
    } // End if MSAA

    if ( !strInstanceId.empty() )
    {
        strResource << _T("::");
        strResource << strInstanceId;
    
    } // End if has identifier

    // Find and / or create a render target with these details.
    // Note: 'createRenderTarget()' will return an existing target
    // with a matching resource identifier if one was previously created.
    cgResourceManager * pResources = mDriver->getResourceManager();
    cgRenderTargetHandle hTarget;
    if ( !pResources->createRenderTarget( &hTarget, Desc, 0, strResource.str(), cgDebugSource() ) )
        return cgRenderTargetHandle::Null;

    // Brand new target?
    // These are no longer necessary for debugging purposes, but I'll leave them here regardless.
    /*const cgResource * pResource = (const cgResource*)hTarget.getResourceSilent();
    if ( pResource && pResource->getReferenceCount(true) == 2 )
        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Allocating '%s' for render view '%s'.\n"), strResource.str().c_str(), mName.c_str() );*/

    // Is this a resource that is new to this particular view?
    RenderSurfaceMap::iterator itSurface = mRenderSurfaces.find( hTarget.getReferenceId() );
    if ( itSurface == mRenderSurfaces.end() )
    {
        // It is new to this view. Build the internal management details structure.
        RenderSurface Surface;
        Surface.format              = Format;
        Surface.scalarWidth         = fScalarWidth;
        Surface.scalarHeight        = fScalarHeight;
        Surface.instanceId          = strInstanceId;
        Surface.multiSampleType     = MultiSampleType;
        Surface.multiSampleQuality  = nMultiSampleQuality;
        Surface.handle              = hTarget;
        Surface.lastRequested       = cgTimer::getInstance()->getTime();

        // Insert into map and return the inserted handle (reference).
        std::pair<RenderSurfaceMap::iterator,bool> InsertedData;
        InsertedData = mRenderSurfaces.insert( RenderSurfaceMap::value_type(hTarget.getReferenceId(),Surface) );
        //return InsertedData.first->second.handle; // Note: This approach appears to cause a crash in some cases with STLPort
        return mRenderSurfaces[hTarget.getReferenceId()].handle;
    
    } // End if new surface (to this view)
    else
    {
        // Simply update the last surface request time.
        itSurface->second.lastRequested = cgTimer::getInstance()->getTime();
        return itSurface->second.handle;

    } // End if existing surface (to this view)
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a full sized depth stencil surface of the requested format that 
/// can be used as temporary storage for this render view. 
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format )
{
    return getDepthStencilSurface( Format, 1.0f, 1.0f, cgMultiSampleType::None, 0, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a full sized depth buffer surface of the requested format that can
/// be used as temporary storage for this render view. A specific surface 
/// instance name / identifier can be supplied to ensure that the uniqueness of
/// this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, const cgString & strInstanceId )
{
    return getDepthStencilSurface( Format, 1.0f, 1.0f, cgMultiSampleType::None, 0, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a depth stencil surface of the requested format and dimensions 
/// that can be used as temporary storage for this render view.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle  & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight )
{
    return getDepthStencilSurface( Format, fScalarWidth, fScalarHeight, cgMultiSampleType::None, 0, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a depth stencil surface (a.k.a. G-Buffer) of the requested format and
/// dimensions that can be used as temporary storage for this render view. A 
/// specific surface instance name / identifier can be supplied to ensure that
/// the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, const cgString & strInstanceId )
{
    return getDepthStencilSurface( Format, fScalarWidth, fScalarHeight, cgMultiSampleType::None, 0, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a full sized depth stencil surface of the requested format that 
/// can be used as temporary storage for this render view. 
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality )
{
    return getDepthStencilSurface( Format, 1.0f, 1.0f, MultiSampleType, nMultiSampleQuality, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a full sized depth buffer surface of the requested format that can
/// be used as temporary storage for this render view. A specific surface 
/// instance name / identifier can be supplied to ensure that the uniqueness of
/// this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality, const cgString & strInstanceId )
{
    return getDepthStencilSurface( Format, 1.0f, 1.0f, MultiSampleType, nMultiSampleQuality, strInstanceId );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a depth stencil surface of the requested format and dimensions 
/// that can be used as temporary storage for this render view.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle  & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality )
{
    return getDepthStencilSurface( Format, fScalarWidth, fScalarHeight, MultiSampleType, nMultiSampleQuality, cgString::Empty );
}

//-----------------------------------------------------------------------------
//  Name : getDepthStencilSurface()
/// <summary>
/// Retrieve a depth stencil surface (a.k.a. G-Buffer) of the requested format
/// and dimensions that can be used as temporary storage for this render view.
/// A specific surface instance name / identifier can be supplied to ensure
/// that the uniqueness of this surface is guaranteed where necessary.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilTargetHandle & cgRenderView::getDepthStencilSurface( cgBufferFormat::Base Format, cgFloat fScalarWidth, cgFloat fScalarHeight, cgMultiSampleType::Base MultiSampleType, cgUInt32 nMultiSampleQuality, const cgString & strInstanceId )
{
    cgAssert( mDriver != CG_NULL );

    // Build an appropriate description structure for this surface ready for
    // creation if it doesn't already exists. 
    cgImageInfo Desc;
    cgSize ViewSize = getSize();
    Desc.width      = (cgInt32)ceil(fScalarWidth  * (cgFloat)ViewSize.width);
    Desc.height     = (cgInt32)ceil(fScalarHeight * (cgFloat)ViewSize.height);
    Desc.format     = Format;
    Desc.mipLevels  = 1;

    // Store the multisampling info
    Desc.multiSampleType    = MultiSampleType;
    Desc.multiSampleQuality = nMultiSampleQuality;

    // Clamp to minimum size
    if ( Desc.width < 1 )
        Desc.width = 1;
    if ( Desc.height < 1 )
        Desc.height = 1;

    // Build an appropriate resource identifier string appropriate for the 
    // specified surface details.
    cgStringParser strResource;
    strResource << _T("RenderViewDepthStencil::");
    strResource << Desc.width;
    strResource << _T("x");
    strResource << Desc.height;
    strResource << _T("x");
    strResource << (cgInt32)Desc.format;
    if ( MultiSampleType != cgMultiSampleType::None )
    {
        strResource << _T(" MSAA") << (cgUInt32)MultiSampleType  << _T("X");
        if ( nMultiSampleQuality != 0 )
            strResource << _T("(") << nMultiSampleQuality << _T(")");

    } // End if MSAA

    if ( !strInstanceId.empty() )
    {
        strResource << _T("::");
        strResource << strInstanceId;
    
    } // End if has identifier

    // Find and / or create a depth stencil target with these details.
    // Note: 'createDepthStencilTarget()' will return an existing target
    // with a matching resource identifier if one was previously created.
    cgResourceManager * pResources = mDriver->getResourceManager();
    cgDepthStencilTargetHandle hTarget;
    if ( !pResources->createDepthStencilTarget( &hTarget, Desc, 0, strResource.str(), cgDebugSource() ) )
        return cgDepthStencilTargetHandle::Null;

    // Brand new target?
    // These are no longer necessary for debugging purposes, but I'll leave them here regardless.
    /*const cgResource * pResource = (const cgResource*)hTarget.getResourceSilent();
    if ( pResource && pResource->getReferenceCount(true) == 2 )
        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Allocating '%s' for render view '%s'.\n"), strResource.str().c_str(), mName.c_str() );*/

    // Is this a resource that is new to this particular view?
    DepthStencilSurfaceMap::iterator itSurface = mDepthStencilSurfaces.find( hTarget.getReferenceId() );
    if ( itSurface == mDepthStencilSurfaces.end() )
    {
        // It is new to this view. Build the internal management details structure.
        DepthStencilSurface Surface;
        Surface.format              = Format;
        Surface.scalarWidth         = fScalarWidth;
        Surface.scalarHeight        = fScalarHeight;
        Surface.instanceId          = strInstanceId;
        Surface.multiSampleType     = MultiSampleType;
        Surface.multiSampleQuality  = nMultiSampleQuality;
        Surface.handle              = hTarget;
        Surface.lastRequested       = cgTimer::getInstance()->getTime();

        // Insert into map and return the inserted handle (reference).
        std::pair<DepthStencilSurfaceMap::iterator,bool> InsertedData;
        InsertedData = mDepthStencilSurfaces.insert( DepthStencilSurfaceMap::value_type(hTarget.getReferenceId(),Surface) );
        //return InsertedData.first->second.handle; // Note: This approach appears to cause a crash in some cases with STLPort
        return mDepthStencilSurfaces[hTarget.getReferenceId()].handle;
    
    } // End if new surface (to this view)
    else
    {
        // Simply update the last surface request time.
        itSurface->second.lastRequested = cgTimer::getInstance()->getTime();
        return itSurface->second.handle;

    } // End if existing surface (to this view)
}

//-----------------------------------------------------------------------------
//  Name : processMessage () (Virtual)
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgRenderView::processMessage( cgMessage * pMessage )
{
    // What message is this?
    switch ( pMessage->messageId )
    {
        case cgSystemMessages::RenderDriver_DeviceLost:
            
            releaseBuffers();

            // Processed message
            return true;

        case cgSystemMessages::RenderDriver_DeviceRestored:

            // Recreate buffers only if the view hasn't been disposed.
            if ( !mDisposed )
                createBuffers();
            
            // Processed message
            return true;

        case cgSystemMessages::Resources_CollectGarbage:
        {
            cgDouble fCurrentTime = cgTimer::getInstance()->getTime();

            // Check to see if any render surfaces need releasing.
            RenderSurfaceMap::iterator itSurface, itCurrent;
            for ( itSurface = mRenderSurfaces.begin(); itSurface != mRenderSurfaces.end(); )
            {
                RenderSurface & Surface = itSurface->second;

                // Increment iterator in case the current entry gets destroyed.
                itCurrent = itSurface++;

                // Due to be cleaned up (20 second TTL expired)?
                cgFloat fDestroyDelay = 20.0f;
                if ( fCurrentTime > (Surface.lastRequested + fDestroyDelay) )
                {
                    // These are no longer necessary for debugging purposes, but I'll leave them here regardless.
                    /*const cgResource * pResource = (const cgResource*)Surface.handle.getResourceSilent();
                    const cgString & strResourceName = (pResource) ? pResource->getResourceName() : cgString::Empty;
                    if ( strResourceName.empty() )
                        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Detaching render surface from render view '%s' because it was not requested within a %g second period.\n"), mName.c_str(), fDestroyDelay );
                    else
                        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Detaching '%s' from render view '%s' because it was not requested within a %g second period.\n"), strResourceName.c_str(), mName.c_str(), fDestroyDelay );*/

                    // Erase from the management list. This will automatically
                    // close the associated render target handle owned by this view.
                    mRenderSurfaces.erase( itCurrent );

                } // End if TTL expired
            
            } // End if destroy delay time has passed

            // Check to see if any depth stencil surfaces need releasing.
            DepthStencilSurfaceMap::iterator itDepthSurface, itDepthCurrent;
            for ( itDepthSurface = mDepthStencilSurfaces.begin(); itDepthSurface != mDepthStencilSurfaces.end(); )
            {
                DepthStencilSurface & Surface = itDepthSurface->second;

                // Increment iterator in case the current entry gets destroyed.
                itDepthCurrent = itDepthSurface++;

                // Due to be cleaned up (20 second TTL expired)?
                cgFloat fDestroyDelay = 20.0f;
                if ( fCurrentTime > (Surface.lastRequested + fDestroyDelay) )
                {
                    // These are no longer necessary for debugging purposes, but I'll leave them here regardless.
                    /*const cgResource * pResource = (const cgResource*)Surface.handle.getResourceSilent();
                    const cgString & strResourceName = (pResource) ? pResource->getResourceName() : cgString::Empty;
                    if ( strResourceName.empty() )
                        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Detaching depth stencil surface from render view '%s' because it was not requested within a %g second period.\n"), mName.c_str(), fDestroyDelay );
                    else
                        cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Detaching '%s' from render view '%s' because it was not requested within a %g second period.\n"), strResourceName.c_str(), mName.c_str(), fDestroyDelay );*/

                    // Erase from the management list. This will automatically
                    // close the associated depth stencil target handle owned by this view.
                    mDepthStencilSurfaces.erase( itDepthCurrent );

                } // End if TTL expired
            
            } // End if destroy delay time has passed

            // Send the garbage collect message again if we aren't in the process of
            // being unregistered.
            if ( !(pMessage->fromId == getReferenceId() && pMessage->sourceUnregistered == true) )
                sendGarbageMessage();

            // Processed message
            return true;
        
        } // End case Resources_CollectGarbage

    } // End message type switch
    
    // Message was not processed, pass to base.
    return cgReference::processMessage( pMessage );
}

//-----------------------------------------------------------------------------
//  Name : sendGarbageMessage () (Protected)
/// <summary>
/// Sends the garbage collection message to ourselves.
/// </summary>
//-----------------------------------------------------------------------------
void cgRenderView::sendGarbageMessage( )
{
    cgMessage Msg;

    // initialize message structure
    Msg.messageId = cgSystemMessages::Resources_CollectGarbage;

    // ToDo: Configurable garbage collection schedule?

    // Send the message to ourselves with a 1 second delay
    cgReferenceManager::sendMessageTo( getReferenceId(), getReferenceId(), &Msg, 1.0f );
}