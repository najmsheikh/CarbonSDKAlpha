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
// Name : cgDX11StateBlocks.cpp                                              //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//        (DX11 implementation).                                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX11_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX11StateBlocks Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX11StateBlocks.h>
#include <Rendering/Platform/cgDX11RenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <System/cgExceptions.h>
#include <D3D11.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX11SamplerState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11SamplerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11SamplerState::cgDX11SamplerState( cgUInt32 nReferenceId, const cgSamplerStateDesc & States ) : 
    cgSamplerState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11SamplerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11SamplerState::~cgDX11SamplerState( )
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
void cgDX11SamplerState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
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
bool cgDX11SamplerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11SamplerStateResource )
        return true;

    // Supported by base?
    return cgSamplerState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to create and / or retrieve an internal D3D11 specific state block
/// object that represents the state values to apply to the specified sampler 
/// index.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11SamplerState * cgDX11SamplerState::getD3DStateBlock( )
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
void cgDX11SamplerState::deviceLost()
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
void cgDX11SamplerState::deviceRestored()
{
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlock () (Private)
/// <summary>
/// Release internal state block.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11SamplerState::releaseStateBlock( )
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
bool cgDX11SamplerState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX11 class driver)
    ID3D11Device * pDevice = CG_NULL;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Handle exceptions
    try
    {
        // Create the D3D11 compatible state block description
        D3D11_SAMPLER_DESC Desc;
        Desc.AddressU       = (D3D11_TEXTURE_ADDRESS_MODE)mValues.addressU;
        Desc.AddressV       = (D3D11_TEXTURE_ADDRESS_MODE)mValues.addressV;
        Desc.AddressW       = (D3D11_TEXTURE_ADDRESS_MODE)mValues.addressW;
        Desc.MipLODBias     = mValues.mipmapLODBias;
        Desc.MaxAnisotropy  = mValues.maximumAnisotropy;
        Desc.ComparisonFunc = (D3D11_COMPARISON_FUNC)mValues.comparisonFunction;
        Desc.BorderColor[0] = mValues.borderColor.r;
        Desc.BorderColor[1] = mValues.borderColor.g;
        Desc.BorderColor[2] = mValues.borderColor.b;
        Desc.BorderColor[3] = mValues.borderColor.a;
        Desc.MinLOD         = mValues.minimumMipmapLOD;
        Desc.MaxLOD         = mValues.maximumMipmapLOD;

        // Build the combined D3D11 filter description
        Desc.Filter = D3D11_FILTER_ANISOTROPIC;
        cgInt MinFilter = (mValues.minificationFilter == cgFilterMethod::None) ? cgFilterMethod::Point : mValues.minificationFilter;
        cgInt MagFilter = (mValues.magnificationFilter == cgFilterMethod::None) ? cgFilterMethod::Point : mValues.magnificationFilter;
        cgInt MipFilter = (mValues.mipmapFilter == cgFilterMethod::None) ? cgFilterMethod::Point : mValues.mipmapFilter;
        if ( MinFilter == cgFilterMethod::Point )
        {
            if ( MagFilter == cgFilterMethod::Point )
            {
                if ( MipFilter == cgFilterMethod::Point )
                    Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                else if ( MipFilter == cgFilterMethod::Linear )
                    Desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;

            } // End if Mag == Point
            else if ( MagFilter == cgFilterMethod::Linear )
            {
                if ( MipFilter == cgFilterMethod::Point )
                    Desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                else if ( MipFilter == cgFilterMethod::Linear )
                    Desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;

            } // End if Mag == Linear

        } // End if Min == Point
        else if ( MinFilter == cgFilterMethod::Linear )
        {
            if ( MagFilter == cgFilterMethod::Point )
            {
                if ( MipFilter == cgFilterMethod::Point )
                    Desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                else if ( MipFilter == cgFilterMethod::Linear )
                    Desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;

            } // End if Mag == Point
            else if ( MagFilter == cgFilterMethod::Linear )
            {
                if ( MipFilter == cgFilterMethod::Point )
                    Desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                else if ( MipFilter == cgFilterMethod::Linear )
                    Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

            } // End if Mag == Linear

        } // End if Min == Linear

        // If anisotropic filtering was selected, but mip mapping was disabled,
        // restrict the map-map selection to the top-most sub-level by forcing MinLOD 
        // to match MaxLOD.
        if ( Desc.Filter == D3D11_FILTER_ANISOTROPIC && mValues.mipmapFilter == cgFilterMethod::None )
            Desc.MinLOD = Desc.MaxLOD;

        // Switch to comparison versions of the filter states?
        if ( mValues.comparisonFunction != D3D11_COMPARISON_NEVER )
            Desc.Filter = (D3D11_FILTER)((cgUInt32)Desc.Filter + 0x80);

        // Create the new state block
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->CreateSamplerState( &Desc, &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("ID3D11Device::CreateSamplerState"), cgDebugSource() );

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
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
bool cgDX11SamplerState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the state block
    if ( !createStateBlock() )
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
bool cgDX11SamplerState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11RasterizerState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11RasterizerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RasterizerState::cgDX11RasterizerState( cgUInt32 nReferenceId, const cgRasterizerStateDesc & States ) : 
    cgRasterizerState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11RasterizerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11RasterizerState::~cgDX11RasterizerState( )
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
void cgDX11RasterizerState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
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
bool cgDX11RasterizerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11RasterizerStateResource )
        return true;

    // Supported by base?
    return cgRasterizerState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D11 specific state block object that 
/// represents the state values to apply.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11RasterizerState * cgDX11RasterizerState::getD3DStateBlock( )
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
void cgDX11RasterizerState::deviceLost()
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
void cgDX11RasterizerState::deviceRestored()
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
void cgDX11RasterizerState::releaseStateBlock( )
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
bool cgDX11RasterizerState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX11 class driver)
    ID3D11Device * pDevice = CG_NULL;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Handle exceptions
    try
    {
        // Create the D3D11 compatible state block description
        D3D11_RASTERIZER_DESC Desc;
        Desc.FillMode               = (D3D11_FILL_MODE)mValues.fillMode;
        Desc.CullMode               = (D3D11_CULL_MODE)mValues.cullMode;
        Desc.FrontCounterClockwise  = mValues.frontCounterClockwise;
        Desc.DepthBias              = mValues.depthBias;
        Desc.DepthBiasClamp         = mValues.depthBiasClamp;
        Desc.SlopeScaledDepthBias   = mValues.slopeScaledDepthBias;
        Desc.DepthClipEnable        = mValues.depthClipEnable;
        Desc.ScissorEnable          = mValues.scissorTestEnable;
        Desc.MultisampleEnable      = mValues.multisampleEnable;
        Desc.AntialiasedLineEnable  = mValues.antialiasedLineEnable;
        
        // Create the new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->CreateRasterizerState( &Desc, &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("ID3D11Device::CreateRasterizerState"), cgDebugSource() );
        
    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
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
bool cgDX11RasterizerState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the state block
    if ( !createStateBlock() )
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
bool cgDX11RasterizerState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11DepthStencilState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11DepthStencilState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11DepthStencilState::cgDX11DepthStencilState( cgUInt32 nReferenceId, const cgDepthStencilStateDesc & States ) : 
    cgDepthStencilState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11DepthStencilState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11DepthStencilState::~cgDX11DepthStencilState( )
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
void cgDX11DepthStencilState::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base(s).
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
bool cgDX11DepthStencilState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11DepthStencilStateResource )
        return true;

    // Supported by base?
    return cgDepthStencilState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D11 specific state block object that 
/// represents the state values to apply. States to apply in DX11 version are 
/// dependant on the status of the 'frontCounterClockwise' rasterizer state.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11DepthStencilState * cgDX11DepthStencilState::getD3DStateBlock( )
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
void cgDX11DepthStencilState::deviceLost()
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
void cgDX11DepthStencilState::deviceRestored()
{
    // Rebuild the resource (don't use loadResource, this was only
    // temporary)
    createStateBlock();
}

//-----------------------------------------------------------------------------
//  Name : releaseStateBlock () (Private)
/// <summary>
/// Release internal state block.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX11DepthStencilState::releaseStateBlock( )
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
bool cgDX11DepthStencilState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX11 class driver)
    ID3D11Device * pDevice = CG_NULL;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Handle exceptions
    try
    {
        // Create the D3D11 compatible state block description
        D3D11_DEPTH_STENCIL_DESC Desc;
        Desc.DepthEnable        = mValues.depthEnable;
        Desc.DepthWriteMask     = (mValues.depthWriteEnable) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        Desc.DepthFunc          = (D3D11_COMPARISON_FUNC)mValues.depthFunction;
        Desc.StencilEnable      = mValues.stencilEnable;
        Desc.StencilReadMask    = mValues.stencilReadMask;
        Desc.StencilWriteMask   = mValues.stencilWriteMask;

        // Front face stencil details
        Desc.FrontFace.StencilFailOp        = (D3D11_STENCIL_OP)mValues.frontFace.stencilFailOperation;
        Desc.FrontFace.StencilDepthFailOp   = (D3D11_STENCIL_OP)mValues.frontFace.stencilDepthFailOperation;
        Desc.FrontFace.StencilPassOp        = (D3D11_STENCIL_OP)mValues.frontFace.stencilPassOperation;
        Desc.FrontFace.StencilFunc          = (D3D11_COMPARISON_FUNC)mValues.frontFace.stencilFunction;

        // Back face stencil details
        Desc.BackFace.StencilFailOp         = (D3D11_STENCIL_OP)mValues.backFace.stencilFailOperation;
        Desc.BackFace.StencilDepthFailOp    = (D3D11_STENCIL_OP)mValues.backFace.stencilDepthFailOperation;
        Desc.BackFace.StencilPassOp         = (D3D11_STENCIL_OP)mValues.backFace.stencilPassOperation;
        Desc.BackFace.StencilFunc           = (D3D11_COMPARISON_FUNC)mValues.backFace.stencilFunction;

        // Create the new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->CreateDepthStencilState( &Desc, &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("ID3D11Device::CreateDepthStencilState"), cgDebugSource() );

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
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
bool cgDX11DepthStencilState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the state block
    if ( !createStateBlock() )
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
bool cgDX11DepthStencilState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX11BlendState Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX11BlendState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11BlendState::cgDX11BlendState( cgUInt32 nReferenceId, const cgBlendStateDesc & States ) : 
    cgBlendState( nReferenceId, States )
{
    // Initialize variables to sensible defaults
    mStateBlock = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX11BlendState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX11BlendState::~cgDX11BlendState( )
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
void cgDX11BlendState::dispose( bool bDisposeBase )
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
bool cgDX11BlendState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX11BlendStateResource )
        return true;

    // Supported by base?
    return cgBlendState::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getD3DStateBlock ()
/// <summary>
/// Called to retrieve an internal D3D11 specific state block object that 
/// represents the state values to apply.
/// </summary>
//-----------------------------------------------------------------------------
ID3D11BlendState * cgDX11BlendState::getD3DStateBlock( )
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
void cgDX11BlendState::deviceLost()
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
void cgDX11BlendState::deviceRestored()
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
void cgDX11BlendState::releaseStateBlock( )
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
bool cgDX11BlendState::createStateBlock( )
{
    // Validate requirements
    cgAssert( mManager != CG_NULL );

    // Already loaded?
    if ( mStateBlock )
        return true;
    
    // Retrieve D3D device for state block creation (required DX11 class driver)
    ID3D11Device * pDevice = CG_NULL;
    cgDX11RenderDriver * pDriver = dynamic_cast<cgDX11RenderDriver*>(mManager->getRenderDriver());
    if ( !pDriver || !(pDevice = pDriver->getD3DDevice()) )
        return false;

    // Handle exceptions
    try
    {
        // Create the D3D11 compatible state block description
        D3D11_BLEND_DESC Desc;
        Desc.AlphaToCoverageEnable  = mValues.alphaToCoverageEnable;
        Desc.IndependentBlendEnable = mValues.independentBlendEnable;

        // Populate render target details
        for ( size_t i = 0; i < 8; ++i )
        {
            D3D11_RENDER_TARGET_BLEND_DESC & TargetOut = Desc.RenderTarget[i];
            cgTargetBlendStateDesc & TargetIn = mValues.renderTarget[i];
            TargetOut.BlendEnable            = TargetIn.blendEnable;
            TargetOut.SrcBlend               = (D3D11_BLEND)TargetIn.sourceBlend;
            TargetOut.DestBlend              = (D3D11_BLEND)TargetIn.destinationBlend;
            TargetOut.BlendOp                = (D3D11_BLEND_OP)TargetIn.blendOperation;
            TargetOut.RenderTargetWriteMask  = TargetIn.renderTargetWriteMask;

            // Separate alpha channel handling?
            if ( TargetIn.separateAlphaBlendEnable )
            {
                TargetOut.SrcBlendAlpha          = (D3D11_BLEND)TargetIn.sourceBlendAlpha;
                TargetOut.DestBlendAlpha         = (D3D11_BLEND)TargetIn.destinationBlendAlpha;
                TargetOut.BlendOpAlpha           = (D3D11_BLEND_OP)TargetIn.blendOperationAlpha;
            
            } // End if separate
            else
            {
                TargetOut.SrcBlendAlpha          = (D3D11_BLEND)TargetIn.sourceBlend;
                TargetOut.DestBlendAlpha         = (D3D11_BLEND)TargetIn.destinationBlend;
                TargetOut.BlendOpAlpha           = (D3D11_BLEND_OP)TargetIn.blendOperation;

            } // End if !separate

            // If SrcAlpha contains a 'Color' mode, swap it with the
            // alpha equivalent.
            cgToDo( "DX11", "Potentially do this to DX9 states too?" )
            if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_SRC_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
            else if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_INV_SRC_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            else if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_SRC1_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_SRC1_ALPHA;
            else if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_INV_SRC1_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_INV_SRC1_ALPHA;
            else if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_DEST_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
            else if ( TargetOut.SrcBlendAlpha == D3D11_BLEND_INV_DEST_COLOR )
                TargetOut.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
            
            // Same for DestAlpha
            if ( TargetOut.DestBlendAlpha == D3D11_BLEND_SRC_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
            else if ( TargetOut.DestBlendAlpha == D3D11_BLEND_INV_SRC_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
            else if ( TargetOut.DestBlendAlpha == D3D11_BLEND_SRC1_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_SRC1_ALPHA;
            else if ( TargetOut.DestBlendAlpha == D3D11_BLEND_INV_SRC1_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_INV_SRC1_ALPHA;
            else if ( TargetOut.DestBlendAlpha == D3D11_BLEND_DEST_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
            else if ( TargetOut.DestBlendAlpha == D3D11_BLEND_INV_DEST_COLOR )
                TargetOut.DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;

        } // Next render target

        // Begin recording new state block.
        HRESULT hRet = D3DERR_INVALIDCALL;
        if ( FAILED( hRet = pDevice->CreateBlendState( &Desc, &mStateBlock ) ) )
            throw cgExceptions::ResultException( hRet, _T("ID3D11Device::CreateBlendState"), cgDebugSource() );
        
    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );

        // Release whatever we got and then bail.
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
bool cgDX11BlendState::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the state block
    if ( !createStateBlock() )
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
bool cgDX11BlendState::unloadResource( )
{
    // Release the state block
    releaseStateBlock();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

#endif // CGE_DX11_RENDER_SUPPORT