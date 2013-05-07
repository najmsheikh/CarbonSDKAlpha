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
// Name : cgStateBlocks.cpp                                                  //
//                                                                           //
// Desc : Contains classes that represent the various state blocks that can  //
//        be applied to the render driver / hardware. Such states include    //
//        sampler, rasterizer, depth stencil and blend state objects. See    //
//        cgResourceTypes.h for information relating to the individual       //
//        description structures that are used to initialize these blocks.   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgStateBlocks Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgStateBlocks.h>
#include <Resources/Platform/cgDX9StateBlocks.h>
#include <Resources/Platform/cgDX11StateBlocks.h>

///////////////////////////////////////////////////////////////////////////////
// cgSamplerState Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSamplerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSamplerState::cgSamplerState( cgUInt32 nReferenceId, const cgSamplerStateDesc & States ) : 
    cgResource( nReferenceId ), mValues( States )
{
    // Initialize variables to sensible defaults

    // Setup initial cached responses
    mResourceType      = cgResourceType::SamplerState;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgSamplerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSamplerState::~cgSamplerState()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgSamplerState * cgSamplerState::createInstance( cgUInt32 nReferenceId, const cgSamplerStateDesc & States )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9SamplerState( nReferenceId, States );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11SamplerState( nReferenceId, States );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSamplerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_SamplerStateResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getValues ()
/// <summary>
/// Retrieve the description / state values for this state block.
/// </summary>
//-----------------------------------------------------------------------------
const cgSamplerStateDesc & cgSamplerState::getValues( ) const
{
    return mValues;
}

///////////////////////////////////////////////////////////////////////////////
// cgDepthStencilState Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDepthStencilState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilState::cgDepthStencilState( cgUInt32 nReferenceId, const cgDepthStencilStateDesc & States ) : 
    cgResource( nReferenceId ), mValues( States )
{
    // Initialize variables to sensible defaults

    // Setup initial cached responses
    mResourceType      = cgResourceType::DepthStencilState;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDepthStencilState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilState::~cgDepthStencilState()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgDepthStencilState * cgDepthStencilState::createInstance( cgUInt32 nReferenceId, const cgDepthStencilStateDesc & States )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9DepthStencilState( nReferenceId, States );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11DepthStencilState( nReferenceId, States );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDepthStencilState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DepthStencilStateResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getValues ()
/// <summary>
/// Retrieve the description / state values for this state block.
/// </summary>
//-----------------------------------------------------------------------------
const cgDepthStencilStateDesc & cgDepthStencilState::getValues( ) const
{
    return mValues;
}

///////////////////////////////////////////////////////////////////////////////
// cgRasterizerState Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgRasterizerState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRasterizerState::cgRasterizerState( cgUInt32 nReferenceId, const cgRasterizerStateDesc & States ) : 
    cgResource( nReferenceId ), mValues( States )
{
    // Initialize variables to sensible defaults

    // Setup initial cached responses
    mResourceType      = cgResourceType::RasterizerState;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgRasterizerState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgRasterizerState::~cgRasterizerState()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgRasterizerState * cgRasterizerState::createInstance( cgUInt32 nReferenceId, const cgRasterizerStateDesc & States )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9RasterizerState( nReferenceId, States );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11RasterizerState( nReferenceId, States );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgRasterizerState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_RasterizerStateResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getValues ()
/// <summary>
/// Retrieve the description / state values for this state block.
/// </summary>
//-----------------------------------------------------------------------------
const cgRasterizerStateDesc & cgRasterizerState::getValues( ) const
{
    return mValues;
}

///////////////////////////////////////////////////////////////////////////////
// cgBlendState Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgBlendState () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBlendState::cgBlendState( cgUInt32 nReferenceId, const cgBlendStateDesc & States ) : 
    cgResource( nReferenceId ), mValues( States )
{
    // Initialize variables to sensible defaults

    // Setup initial cached responses
    mResourceType      = cgResourceType::BlendState;
    mCanEvict         = true;
}

//-----------------------------------------------------------------------------
//  Name : ~cgBlendState () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgBlendState::~cgBlendState()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgBlendState * cgBlendState::createInstance( cgUInt32 nReferenceId, const cgBlendStateDesc & States )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9BlendState( nReferenceId, States );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11BlendState( nReferenceId, States );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI            

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBlendState::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_BlendStateResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getValues ()
/// <summary>
/// Retrieve the description / state values for this state block.
/// </summary>
//-----------------------------------------------------------------------------
const cgBlendStateDesc & cgBlendState::getValues( ) const
{
    return mValues;
}