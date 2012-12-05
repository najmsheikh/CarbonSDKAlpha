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
// Name : cgAtmosphericsProcessor.cpp                                        //
//                                                                           //
// Desc : Image processing class designed to apply atmospherics effects such //
//        as fog to a rendered scene image.                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAtmosphericsProcessor Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/Processors/cgAtmosphericsProcessor.h>
#include <Rendering/cgSampler.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <World/Elements/cgSkyElement.h>

// ToDo: 6767 -- Cache all shaders.

///////////////////////////////////////////////////////////////////////////////
// cgAtmosphericsProcessor Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAtmosphericsProcessor () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAtmosphericsProcessor::cgAtmosphericsProcessor(  )
{
    // Initialize variables to sensible defaults
    mSkyBoxSampler      = CG_NULL;
    mDepthSampler       = CG_NULL;
    mSkySamplerRegister = -1;
    
    // Configuration defaults
    mSkyConfig.hdrScale     = 1.0f;
    mSkyConfig.baseColor    = cgColorValue( 0xFF7D7D7D );
}

//-----------------------------------------------------------------------------
//  Name : ~cgImageProcessor () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAtmosphericsProcessor::~cgAtmosphericsProcessor( )
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAtmosphericsProcessor::dispose( bool disposeBase )
{
    // Close resource handles.
    mAtmosphericsShader.close();
    mSkyConstants.close();
    
    // Release memory
    if ( mSkyBoxSampler )
        mSkyBoxSampler->scriptSafeDispose();
    if ( mDepthSampler )
        mDepthSampler->scriptSafeDispose();
    
    // Clear variables
    mSkyBoxSampler      = CG_NULL;
    mDepthSampler       = CG_NULL;
    mSkySamplerRegister = -1;

    // Configuration defaults
    mSkyConfig.hdrScale     = 1.0f;
    mSkyConfig.baseColor    = cgColorValue( 0xFF7D7D7D );
    
    // Dispose base if requested.
    if ( disposeBase )
        cgImageProcessor::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : initialize ()
/// <summary>
/// Initialize the image processor class and allow it to create any internal
/// resources that may be necessary to execute requested operations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAtmosphericsProcessor::initialize( cgRenderDriver * driver )
{
    // Call base class implementation first.
    if ( !cgImageProcessor::initialize( driver ) )
        return false;

    // Create an instance of the atmospheric processing surface shader
    cgResourceManager * resources = driver->getResourceManager();
    if ( !resources->createSurfaceShader( &mAtmosphericsShader, _T("sys://Shaders/Atmospheric.sh"), 0, cgDebugSource() ) )
        return false;

    // Create constant buffers that map operation data
    // to the physical processing shader.
    if ( !resources->createConstantBuffer( &mSkyConstants, mAtmosphericsShader, _T("cbSkyData"), cgDebugSource() ) )
        return false;
    cgAssert( mSkyConstants->getDesc().length == sizeof(_cbSkyData) );

    // Create samplers
    mDepthSampler  = resources->createSampler( _T("Depth"), mAtmosphericsShader );
    mSkyBoxSampler = resources->createSampler( _T("SkyBox"), mAtmosphericsShader );
    mDepthSampler->setStates( mSamplers.point->getStates() );
    mSkyBoxSampler->setStates( mSamplers.linear->getStates() );

    // Retrieve the index of the skybox sampler register.
    mSkySamplerRegister = mAtmosphericsShader->findSamplerRegister( L"SkyBox" );

    // Return success
    return true;
}

//-------------------------------------------------------------------------
// Name : drawSky ()
/// <summary>
/// Process and render the sky based on the properties defined in the 
/// supplied element.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSky( cgSkyElement * element, bool decodeSRGB  )
{
    drawSky( element, decodeSRGB, cgRenderTargetHandle::Null );
}

//-------------------------------------------------------------------------
// Name : drawSky ()
/// <summary>
/// Process and render the sky based on the properties defined in the 
/// supplied element.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSky( cgSkyElement * element, bool decodeSRGB, const cgRenderTargetHandle & destination )
{
    // Atmospherics shader must be valid and loaded at this point.
    if ( !element || !mAtmosphericsShader.getResource(true) || !mAtmosphericsShader.isLoaded() )
        return;

    // Retrieve the sampler for rendering.
    cgSampler * skySampler = element->getBaseSampler();
    if ( !skySampler || mSkySamplerRegister < 0 )
        return;

    // Setup depth-stencil state and rasterizer state (default).
    mDriver->setDepthStencilState( mLessEqualDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set blend states
    mDriver->setBlendState( mDefaultRGBABlendState );

    // Set the texture and sampler states
    skySampler->apply( mSkySamplerRegister );
    
    // Select shaders
    bool hdrLighting = (mDriver->getSystemState( cgSystemState::HDRLighting ) != 0);
    if ( !mAtmosphericsShader->selectVertexShader( _T("transform"), false, true ) ||
         !mAtmosphericsShader->selectPixelShader( _T("drawSkyBox"), decodeSRGB, hdrLighting ) )
        return;

    // Update the constant buffer
    mSkyConfig.hdrScale = element->getBaseHDRScale();
    mSkyConfig.baseColor = 0xFF7D7D7D;
    mSkyConstants->updateBuffer( 0, 0, &mSkyConfig );
    mDriver->setConstantBufferAuto( mSkyConstants );

    // Draw a clip quad
    if ( mDriver->beginTargetRender( destination ) )
    {
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

    } // End if beginTargetRender
}

//-------------------------------------------------------------------------
// Name : drawSkyColor ()
/// <summary>
/// Process and render the sky, simply by filling with the specified color.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSkyColor( cgColorValue color, bool decodeSRGB  )
{
    drawSkyColor( color, decodeSRGB, cgRenderTargetHandle::Null );
}

//-------------------------------------------------------------------------
// Name : drawSkyColor ()
/// <summary>
/// Process and render the sky, simply by filling with the specified color.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSkyColor( cgColorValue color, bool decodeSRGB, const cgRenderTargetHandle & destination )
{
    // Atmospherics shader must be valid and loaded at this point.
    if ( !mAtmosphericsShader.getResource(true) || !mAtmosphericsShader.isLoaded() )
        return;

    // Setup depth-stencil state and rasterizer state (default).
    mDriver->setDepthStencilState( mLessEqualDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set blend states
    mDriver->setBlendState( mDefaultRGBABlendState );

    // Select shaders
    bool hdrLighting = (mDriver->getSystemState( cgSystemState::HDRLighting ) != 0);
    if ( !mAtmosphericsShader->selectVertexShader( _T("transform"), false, true ) ||
         !mAtmosphericsShader->selectPixelShader( _T("drawSkyColor"), decodeSRGB, hdrLighting ) )
        return;

    // Update the constant buffer
    mSkyConfig.hdrScale  = 1.0f;
    mSkyConfig.baseColor = color;
    mSkyConstants->updateBuffer( 0, 0, &mSkyConfig );
    mDriver->setConstantBufferAuto( mSkyConstants );

    // Draw a clip quad
    if ( mDriver->beginTargetRender( destination ) )
    {
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

    } // End if beginTargetRender
}

//-------------------------------------------------------------------------
// Name : drawSkyBox ()
/// <summary>
/// Basic sky box rendering. Reads the 6 faces from the specified cube map 
/// texture.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSkyBox( const cgTextureHandle & skyCubeTexture, bool decodeSRGB  )
{
    drawSkyBox( skyCubeTexture, decodeSRGB, cgRenderTargetHandle::Null );
}

//-------------------------------------------------------------------------
// Name : drawSkyBox ()
/// <summary>
/// Basic sky box rendering. Reads the 6 faces from the specified cube map 
/// texture and outputs to the specified destination target if supplied.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawSkyBox( const cgTextureHandle & skyCubeTexture, bool decodeSRGB, const cgRenderTargetHandle & destination )
{
    // Atmospherics shader must be valid and loaded at this point.
    if ( !mAtmosphericsShader.getResource(true) || !mAtmosphericsShader.isLoaded() )
        return;

    // Setup depth-stencil state and rasterizer state (default).
    mDriver->setDepthStencilState( mLessEqualDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set blend states
    mDriver->setBlendState( mDefaultRGBABlendState );

    // Set the texture and sampler states
    mSkyBoxSampler->apply( skyCubeTexture );

    // Select shaders
    bool hdrLighting = (mDriver->getSystemState( cgSystemState::HDRLighting ) != 0);
    if ( !mAtmosphericsShader->selectVertexShader( _T("transform"), false, true ) ||
         !mAtmosphericsShader->selectPixelShader( _T("drawSkyBox"), decodeSRGB, hdrLighting ) )
        return;

    // Update the constant buffer
    mSkyConfig.hdrScale  = 1.0f;
    mSkyConfig.baseColor = 0xFF7D7D7D;
    mSkyConstants->updateBuffer( 0, 0, &mSkyConfig );
    mDriver->setConstantBufferAuto( mSkyConstants );
    
    // Draw a clip quad
    if ( mDriver->beginTargetRender( destination ) )
    {
        mDriver->drawClipQuad( );
        mDriver->endTargetRender( );

    } // End if beginTargetRender
}

//-------------------------------------------------------------------------
// Name : drawFog ()
/// <summary>
/// Apply fog to the entire image based on the depth supplied in the depth
/// texture.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawFog( cgFogModel::Base model, const cgTextureHandle & depthTexture, cgDepthType::Base depthType )
{
    drawFog( model, depthTexture, depthType, cgRenderTargetHandle::Null );
}

//-------------------------------------------------------------------------
// Name : drawFog ()
/// <summary>
/// Apply fog to the entire image based on the depth supplied in the depth
/// texture and output to the specified destination target if supplied.
/// </summary>
//-------------------------------------------------------------------------
void cgAtmosphericsProcessor::drawFog( cgFogModel::Base model, const cgTextureHandle & depthTexture, cgDepthType::Base depthType, const cgRenderTargetHandle & destination )
{
    // Setup depth-stencil state and rasterizer state (default).
    mDriver->setDepthStencilState( mGreaterDepthState );
    mDriver->setRasterizerState( cgRasterizerStateHandle::Null );

    // Set blend states
    mDriver->setBlendState( mAlphaBlendState );

    // Set the texture and sampler states
    mDepthSampler->apply( depthTexture );

    // Select shaders
    if ( !mAtmosphericsShader->selectVertexShader( _T("transform"), true, false ) ||
         !mAtmosphericsShader->selectPixelShader( _T("drawFog"), (cgInt32)model, (cgInt32)depthType ) )
        return;

    // Draw a clip quad
    drawClipQuad( destination );
}