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
// Name : cgDX9StateBlocks.cpp                                               //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//        (DX9 implementation).                                              //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9StateBlocks Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9StateBlocks.h>
#include <Rendering/Platform/cgDX9RenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <System/cgExceptions.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9SamplerState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9SamplerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9SamplerState::cgDX9SamplerState( cgUInt32 nReferenceId, const cgSamplerStateDesc & States ) : 
    cgSamplerState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    memset( mStateBlocks, 0, sizeof(mStateBlocks) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9SamplerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9SamplerState::~cgDX9SamplerState( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9SamplerState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // dispose base(s).
    if ( bDisposeBase == true )
        cgSamplerState::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9SamplerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9SamplerStateResource )
        return true;

    // Supported by base?
    return cgSamplerState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to create and / or retrieve an internal D3D9 specific state block
/// object that represents the state values to apply to the specified sampler 
/// index.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DStateBlock9 * cgDX9SamplerState::getD3DStateBlock( cgUInt32 nSamplerIndex )
{
    // Attempt to create the state block if necessary.
    if ( createStateBlock( nSamplerIndex ) )
    {
        // Add reference, we're returning a pointer
        mStateBlocks[nSamplerIndex]->AddRef();
        return mStateBlocks[nSamplerIndex];
    
    } // End if valid
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9SamplerState::deviceLost()
{
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseStateBlocks();
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9SamplerState::deviceRestored()
{
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlocks () (Private)
/// <summary>
/// Release all internal state blocks.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9SamplerState::releaseStateBlocks( )
{
    cgInt nBlocks = sizeof(mStateBlocks) / sizeof(void*);
    for ( cgInt i = 0; i < nBlocks; ++i )
    {
        if ( mStateBlocks[i] )
            mStateBlocks[i]->Release();
    
    } // Next Block
    memset( mStateBlocks, 0, sizeof(mStateBlocks) );
}

//-----------------------------------------------------------------------------
//  Name : createStateBlock () (Private)
/// <summary>
/// Create the internal state block for the specified sampler index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9SamplerState::createStateBlock( cgUInt32 nSamplerIndex )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );
    cgAssert( nSamplerIndex < (sizeof(mStateBlocks) / sizeof(void*)) );

    // Already loaded?
    if ( mStateBlocks[nSamplerIndex] )
        return true;
    
    // Retrieve D3D device for state block creation (required DX9 class driver)
    bool bRecording  = false;
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
        return false;

    // Handle exceptions
    try
    {
        // Begin recording new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->BeginStateBlock() ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::BeginStateBlock"), cgDebugSource() );
        bRecording = true;

        // Set states to record.
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MAGFILTER, mValues.magnificationFilter );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MINFILTER, mValues.minificationFilter );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MIPFILTER, mValues.mipmapFilter );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_ADDRESSU, mValues.addressU );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_ADDRESSV, mValues.addressV );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_ADDRESSW, mValues.addressW );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MIPMAPLODBIAS, (cgUInt32&)mValues.mipmapLODBias );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MAXANISOTROPY, mValues.maximumAnisotropy );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_BORDERCOLOR, mValues.borderColor );
        pDevice->SetSamplerState( nSamplerIndex, D3DSAMP_MAXMIPLEVEL, (cgUInt32)mValues.minimumMipmapLOD );
        // D3D11: ComparisonFunc
        // D3D11: MaxLOD

        // We're done.
        if ( FAILED( hRet = pDevice->EndStateBlock( &mStateBlocks[nSamplerIndex] ) ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::EndStateBlock"), cgDebugSource() );
        bRecording = false;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( bRecording )
            pDevice->EndStateBlock( CG_NULL ); // ToDo: 9999 - Confirum that this does indeed simply end recording.
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9SamplerState::loadResource( )
{
    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9SamplerState::unloadResource( )
{
    // Release the state block
    releaseStateBlocks();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9RasterizerState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9RasterizerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RasterizerState::cgDX9RasterizerState( cgUInt32 nReferenceId, const cgRasterizerStateDesc & States ) : 
    cgRasterizerState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9RasterizerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9RasterizerState::~cgDX9RasterizerState( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9RasterizerState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // dispose base(s).
    if ( bDisposeBase == true )
        cgRasterizerState::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RasterizerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9RasterizerStateResource )
        return true;

    // Supported by base?
    return cgRasterizerState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D9 specific state block object that 
/// represents the state values to apply.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DStateBlock9 * cgDX9RasterizerState::getD3DStateBlock( )
{
    // Add reference, we're returning a pointer
    if ( mStateBlock )
        mStateBlock->AddRef();
    return mStateBlock;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RasterizerState::deviceLost()
{
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RasterizerState::deviceRestored()
{
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlock () (Private)
/// <summary>
/// Release the internal state block.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9RasterizerState::releaseStateBlock( )
{
    if ( mStateBlock )
        mStateBlock->Release();
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createStateBlock () (Private)
/// <summary>
/// Create the internal state block.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RasterizerState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX9 class driver)
    bool bRecording  = false;
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
        return false;

    // Handle exceptions
    try
    {
        // Begin recording new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->BeginStateBlock() ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::BeginStateBlock"), cgDebugSource() );
        bRecording = true;

        // Set states to record.
        pDevice->SetRenderState( D3DRS_FILLMODE, mValues.fillMode );
        pDevice->SetRenderState( D3DRS_DEPTHBIAS, (cgUInt32&)mValues.depthBias );
        pDevice->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, (cgUInt32&)mValues.slopeScaledDepthBias );
        pDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, mValues.scissorTestEnable );
        pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, mValues.multisampleEnable );
        pDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, mValues.antialiasedLineEnable );
        if ( mValues.cullMode == cgCullMode::None )
            pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
        else if ( mValues.cullMode == cgCullMode::Front && mValues.frontCounterClockwise ||
             mValues.cullMode == cgCullMode::Back  && !mValues.frontCounterClockwise )
            pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        else
            pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

        // D3D11: DepthBiasClamp
        // D3D11: DepthClipEnable

        // We're done.
        if ( FAILED( hRet = pDevice->EndStateBlock( &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::EndStateBlock"), cgDebugSource() );
        bRecording = false;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( bRecording )
            pDevice->EndStateBlock( CG_NULL ); // ToDo: 9999 - Confirum that this does indeed simply end recording.
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RasterizerState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded == true )
        return true;

    // Attempt to create the state block
    if ( createStateBlock() == false )
        return false;

    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9RasterizerState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9DepthStencilState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9DepthStencilState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9DepthStencilState::cgDX9DepthStencilState( cgUInt32 nReferenceId, const cgDepthStencilStateDesc & States ) : 
    cgDepthStencilState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    memset( mStateBlocks, 0, sizeof(mStateBlocks) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9DepthStencilState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9DepthStencilState::~cgDX9DepthStencilState( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9DepthStencilState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // dispose base(s).
    if ( bDisposeBase == true )
        cgDepthStencilState::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9DepthStencilState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9DepthStencilStateResource )
        return true;

    // Supported by base?
    return cgDepthStencilState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D9 specific state block object that 
/// represents the state values to apply. States to apply in DX9 version are 
/// dependant on the status of the 'frontCounterClockwise' rasterizer state.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DStateBlock9 * cgDX9DepthStencilState::getD3DStateBlock( bool bFrontCounterClockwise )
{
    // Attempt to create the state block if necessary.
    if ( createStateBlock( bFrontCounterClockwise ) )
    {
        // Add reference, we're returning a pointer
        cgInt nBlockIndex = bFrontCounterClockwise ? 1 : 0;
        mStateBlocks[nBlockIndex]->AddRef();
        return mStateBlocks[nBlockIndex];
    
    } // End if valid
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9DepthStencilState::deviceLost()
{
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseStateBlocks();
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9DepthStencilState::deviceRestored()
{
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlocks () (Private)
/// <summary>
/// Release all internal state blocks.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9DepthStencilState::releaseStateBlocks( )
{
    cgInt nBlocks = sizeof(mStateBlocks) / sizeof(void*);
    for ( cgInt i = 0; i < nBlocks; ++i )
    {
        if ( mStateBlocks[i] )
            mStateBlocks[i]->Release();
    
    } // Next Block
    memset( mStateBlocks, 0, sizeof(mStateBlocks) );
}

//-----------------------------------------------------------------------------
//  Name : createStateBlock () (Private)
/// <summary>
/// Create the internal state block for the specified sampler index.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9DepthStencilState::createStateBlock( bool bFrontCounterClockwise )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    cgInt nBlockIndex = bFrontCounterClockwise ? 1 : 0;
    if ( mStateBlocks[nBlockIndex] )
        return true;
    
    // Retrieve D3D device for state block creation (required DX9 class driver)
    bool bRecording  = false;
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
        return false;

    // Handle exceptions
    try
    {
        // Begin recording new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->BeginStateBlock() ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::BeginStateBlock"), cgDebugSource() );
        bRecording = true;

        // Set states to record.
        pDevice->SetRenderState( D3DRS_ZENABLE, mValues.depthEnable );
        pDevice->SetRenderState( D3DRS_ZWRITEENABLE, mValues.depthWriteEnable );
        pDevice->SetRenderState( D3DRS_ZFUNC, mValues.depthFunction );
        pDevice->SetRenderState( D3DRS_STENCILENABLE, mValues.stencilEnable );
        pDevice->SetRenderState( D3DRS_STENCILMASK, mValues.stencilReadMask );
        pDevice->SetRenderState( D3DRS_STENCILWRITEMASK, mValues.stencilWriteMask );
        pDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE ); // Always force enabled to comply with D3D11
        
        // Apply the correct depth/stencil operation states depending on the
        // application's instruction about the direction in which a front face winds.
        cgDepthStencilOpDesc * pCWOp = CG_NULL, * pCCWOp = CG_NULL;
        if ( bFrontCounterClockwise )
        {
            pCCWOp = &mValues.frontFace;
            pCWOp = &mValues.backFace;
        
        } // End if Front=CCW
        else
        {
            pCCWOp = &mValues.backFace;
            pCWOp = &mValues.frontFace;
        
        } // End if Front=CCW

        // Setup CW states first.
        pDevice->SetRenderState( D3DRS_STENCILFAIL, pCWOp->stencilFailOperation );
        pDevice->SetRenderState( D3DRS_STENCILZFAIL, pCWOp->stencilDepthFailOperation );
        pDevice->SetRenderState( D3DRS_STENCILPASS, pCWOp->stencilPassOperation );
        pDevice->SetRenderState( D3DRS_STENCILFUNC, pCWOp->stencilFunction );

        // Now setup CCW states.
        pDevice->SetRenderState( D3DRS_CCW_STENCILFAIL, pCCWOp->stencilFailOperation );
        pDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, pCCWOp->stencilDepthFailOperation );
        pDevice->SetRenderState( D3DRS_CCW_STENCILPASS, pCCWOp->stencilPassOperation );
        pDevice->SetRenderState( D3DRS_CCW_STENCILFUNC, pCCWOp->stencilFunction );
    
        // We're done.
        if ( FAILED( hRet = pDevice->EndStateBlock( &mStateBlocks[nBlockIndex] ) ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::EndStateBlock"), cgDebugSource() );
        bRecording = false;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( bRecording )
            pDevice->EndStateBlock( CG_NULL ); // ToDo: 9999 - Confirum that this does indeed simply end recording.
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9DepthStencilState::loadResource( )
{
    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9DepthStencilState::unloadResource( )
{
    // Release the state block
    releaseStateBlocks();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9BlendState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9BlendState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9BlendState::cgDX9BlendState( cgUInt32 nReferenceId, const cgBlendStateDesc & States ) : 
    cgBlendState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9BlendState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9BlendState::~cgDX9BlendState( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9BlendState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase == true )
        cgBlendState::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9BlendState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9BlendStateResource )
        return true;

    // Supported by base?
    return cgBlendState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D9 specific state block object that 
/// represents the state values to apply.
/// </summary>
//-----------------------------------------------------------------------------
IDirect3DStateBlock9 * cgDX9BlendState::getD3DStateBlock( )
{
    // Add reference, we're returning a pointer
    if ( mStateBlock )
        mStateBlock->AddRef();
    return mStateBlock;
}

//-----------------------------------------------------------------------------
//  Name : deviceLost ()
/// <summary>
/// Notification that the device has been lost
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9BlendState::deviceLost()
{
    // Release the resource (don't use unloadResource, this is only
    // temporary)
    releaseStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : deviceRestored ()
/// <summary>
/// Notification that the device has been restored
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9BlendState::deviceRestored()
{
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlock () (Private)
/// <summary>
/// Release the internal state block.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9BlendState::releaseStateBlock( )
{
    if ( mStateBlock )
        mStateBlock->Release();
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createStateBlock () (Private)
/// <summary>
/// Create the internal state block.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9BlendState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX9 class driver)
    bool bRecording  = false;
    IDirect3DDevice9 * pDevice = CG_NULL;
    cgDX9RenderDriver * pDriver = dynamic_cast<cgDX9RenderDriver*>(mManager->getRenderDriver());
    if ( pDriver == CG_NULL || (pDevice = pDriver->getD3DDevice()) == CG_NULL )
        return false;

    // Handle exceptions
    try
    {
        // Begin recording new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->BeginStateBlock() ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::BeginStateBlock"), cgDebugSource() );
        bRecording = true;

        // Set states to record.
        const cgTargetBlendStateDesc & State = mValues.renderTarget[0];
        pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, State.blendEnable );
        pDevice->SetRenderState( D3DRS_SRCBLEND, State.sourceBlend );
        pDevice->SetRenderState( D3DRS_DESTBLEND, State.destinationBlend );
        pDevice->SetRenderState( D3DRS_BLENDOP, State.blendOperation );
        pDevice->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, State.separateAlphaBlendEnable );
        pDevice->SetRenderState( D3DRS_SRCBLENDALPHA, State.sourceBlendAlpha );
        pDevice->SetRenderState( D3DRS_DESTBLENDALPHA, State.destinationBlendAlpha );
        pDevice->SetRenderState( D3DRS_BLENDOPALPHA, State.blendOperationAlpha );
        pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, State.renderTargetWriteMask );
        
        // D3D11 - AlphaToCoverageEnable
        // D3D11 - IndependentBlendEnable

        // We're done.
        if ( FAILED( hRet = pDevice->EndStateBlock( &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("IDirect3DDevice9::EndStateBlock"), cgDebugSource() );
        bRecording = false;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
        if ( bRecording )
            pDevice->EndStateBlock( CG_NULL ); // ToDo: 9999 - Confirm that this does indeed simply end recording.
        pDevice->Release();
        return false;

    } // End Catch

    // Release the D3D device, we're all done with it
    pDevice->Release();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9BlendState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded == true )
        return true;

    // Attempt to create the state block
    if ( createStateBlock() == false )
        return false;

    // We're now loaded
    mResourceLoaded = true;
    mResourceLost   = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9BlendState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX9_RENDER_SUPPORT