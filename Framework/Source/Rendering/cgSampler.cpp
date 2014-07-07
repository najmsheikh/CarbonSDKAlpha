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
// File : cgSampler.cpp                                                      //
//                                                                           //
// Desc : Primary texture sampling classes. Samplers allows the application  //
//        to define how the rendering device should sample from the attached //
//        texture, and provides the means to bind the texture data to any    //
//        necessary effect parameters.                                       //
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
#include <Rendering/cgSampler.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgTexture.h>
#include <Resources/cgSurfaceShader.h>
#include <System/cgMessageTypes.h>

//-----------------------------------------------------------------------------
// Static Member Definitions.
//-----------------------------------------------------------------------------
cgWorldQuery cgSampler::mInsertSampler;
cgWorldQuery cgSampler::mUpdateTexture;
cgWorldQuery cgSampler::mUpdateAddressModes;
cgWorldQuery cgSampler::mUpdateFilterMethods;
cgWorldQuery cgSampler::mUpdateLODDetails;
cgWorldQuery cgSampler::mUpdateMaxAnisotropy;
cgWorldQuery cgSampler::mUpdateBorderColor;
cgWorldQuery cgSampler::mUpdateStrength;
cgWorldQuery cgSampler::mLoadSampler;

///////////////////////////////////////////////////////////////////////////////
// cgSampler Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgSampler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::cgSampler( cgSampler * pInit ) : 
    cgWorldComponent( cgReferenceManager::generateInternalRefId(), CG_NULL )
{
    // Duplicate values.
    mStatesDirty    = pInit->mStatesDirty;
    mName           = pInit->mName;
	mTexture        = pInit->mTexture;
	mStateDesc      = pInit->mStateDesc;
    mStates         = pInit->mStates;
    mSystemSampler  = pInit->mSystemSampler;
    mDriver         = pInit->mDriver;
    mShader         = pInit->mShader;
    mSamplerIndex   = pInit->mSamplerIndex;
    mStrength       = pInit->mStrength;

    // Update cache.
    updateCachedValues();
}

//-----------------------------------------------------------------------------
//  Name : cgSampler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::cgSampler( cgRenderDriver * pDriver, const cgString & strName, const cgSurfaceShaderHandle & hShader ) :
    cgWorldComponent( cgReferenceManager::generateInternalRefId(), CG_NULL )
{
    // Reset variables to sensible defaults
    mStatesDirty    = true;
    mSamplerIndex   = -1;
    mName           = strName;
    mDriver         = pDriver;
    mShader         = hShader;
    mSystemSampler  = false;
    mStrength       = 1.0f;

    // Setup sampler state defaults
    defaultSamplerStates();

    // Update cache
    updateCachedValues();
}

//-----------------------------------------------------------------------------
//  Name : cgSampler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::cgSampler( cgUInt32 nReferenceId, cgWorld * pWorld, cgRenderDriver * pDriver, const cgString & strName, const cgSurfaceShaderHandle & hShader ) :
    cgWorldComponent( nReferenceId, pWorld )
{
    // Reset variables to sensible defaults
    mStatesDirty    = true;
    mSamplerIndex   = -1;
    mName           = strName;
    mDriver         = pDriver;
    mShader         = hShader;
    mSystemSampler  = false;
    mStrength       = 1.0f;

    // Setup sampler state defaults
    defaultSamplerStates();

    // Update cache.
    updateCachedValues();
}

//-----------------------------------------------------------------------------
//  Name : cgSampler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::cgSampler( cgRenderDriver * pDriver, const cgSurfaceShaderHandle & hShader, cgSampler * pInit ) : 
    cgWorldComponent( cgReferenceManager::generateInternalRefId(), CG_NULL )
{
    // Setup new values
    mDriver           = pDriver;
    mShader           = hShader;
    mSamplerIndex     = -1;

    // Duplicate values.
    mStatesDirty    = pInit->mStatesDirty;
    mName           = pInit->mName;
	mTexture        = pInit->mTexture;
	mStateDesc      = pInit->mStateDesc;
    mStates         = pInit->mStates;
    mSystemSampler  = pInit->mSystemSampler;
    mStrength       = pInit->mStrength;

    // Duplicate conditional values.
    if ( mDriver == pInit->mDriver && mShader == pInit->mShader )
        mSamplerIndex = pInit->mSamplerIndex;

    // Update cache
    updateCachedValues();
}

//-----------------------------------------------------------------------------
//  Name : cgSampler () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::cgSampler( cgUInt32 nReferenceId, cgWorld * pWorld, cgRenderDriver * pDriver, const cgSurfaceShaderHandle & hShader, cgSampler * pInit ) : 
    cgWorldComponent( nReferenceId, pWorld, pInit )
{
    // Setup new values
    mDriver         = pDriver;
    mShader         = hShader;
    mSamplerIndex   = -1;

    // Duplicate values.
    mStatesDirty    = pInit->mStatesDirty;
    mName           = pInit->mName;
    mTexture        = pInit->mTexture;
	mStateDesc      = pInit->mStateDesc;
    mStates         = pInit->mStates;
    mSystemSampler  = pInit->mSystemSampler;
    mStrength       = pInit->mStrength;

    // Duplicate conditional values.
    if ( mDriver == pInit->mDriver && mShader == pInit->mShader )
        mSamplerIndex = pInit->mSamplerIndex;

    // Update cache
    updateCachedValues();
}

//-----------------------------------------------------------------------------
//  Name : ~cgSampler () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler::~cgSampler( )
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
void cgSampler::dispose( bool bDisposeBase )
{
    // Release resources
    mStates.close();
    mShader.close();
    mTexture.close();
}

//-----------------------------------------------------------------------------
//  Name : defaultSamplerStates () (Protected)
/// <summary>
/// Populates sampler state structure with defaults. These default values may
/// not match the API defaults (i.e. D3D), but instead default to values that
/// make the most sense for the engine / editor combination.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::defaultSamplerStates( )
{
    mStateDesc.addressU             = cgAddressingMode::Wrap;
	mStateDesc.addressV             = cgAddressingMode::Wrap;
	mStateDesc.addressW             = cgAddressingMode::Clamp;
    mStateDesc.magnificationFilter  = cgFilterMethod::Linear;
    mStateDesc.minificationFilter   = cgFilterMethod::Linear;
    mStateDesc.mipmapFilter         = cgFilterMethod::Linear;
    mStateDesc.mipmapLODBias        = -0.6f;
    mStateDesc.maximumAnisotropy    = 16;
    mStateDesc.comparisonFunction   = cgComparisonFunction::Never;
    mStateDesc.borderColor          = 0;
    mStateDesc.minimumMipmapLOD     = 0.0f;
    mStateDesc.maximumMipmapLOD     = 3.402823466e+38f;	
}

//-----------------------------------------------------------------------------
//  Name : updateCachedValues () (Protected)
/// <summary>
/// This method should be called whenever the name of this sampler is adjusted,
/// or when the assigned texture is replaced. Certain necessary values based on
/// these properties will then be cached to save runtime overhead.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::updateCachedValues( )
{
    mApplyDefaultDiffuse = false;
    mApplyDefaultNormal  = false;

    static const cgString diffuseSamplerName = _T("Diffuse");
    static const cgString normalSamplerName = _T("Normal");
    if ( mName == diffuseSamplerName && (!mTexture.isValid() || !mTexture->isValid()) )
        mApplyDefaultDiffuse = true;
    else if ( mName == normalSamplerName && (!mTexture.isValid() || !mTexture->isValid()) )
        mApplyDefaultNormal = true;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Sets this sampler's properties to the global or custom shader using
/// the specified texture to override the internal version.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::apply( const cgTextureHandle & hTexture )
{
    // Have we already determined the sampler register index to which we are 
    // being applied? If not, query the surface shader.
    if ( mSamplerIndex < 0 )
    {
        // Surface shader must be resident
        cgSurfaceShader * pShader = mShader.getResource(true);
        cgAssertEx( pShader != CG_NULL, "Unable to automatically discover the applicable sampler register index without an associated surface shader." );
        
        // Find the appropriate binding index for the sampler with this name.
        mSamplerIndex = pShader->findSamplerRegister( mName );
        if ( mSamplerIndex < 0 )
            return false;

    } // End if no sampler index.

    // Generate new sampler states if the current states have been modified.
    if ( mStatesDirty )
    {
        cgResourceManager * pManager = mDriver->getResourceManager();
        if ( pManager->createSamplerState( &mStates, mStateDesc, 0, cgDebugSource() ) == false )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to generate new sampler state object while applying sampler '%s'.\n"), mName.c_str() );
            return false;
        
        } // End if failed

        // States are no longer dirty
        mStatesDirty = false;

    } // End if dirty

    // Apply the current sampler state.
    mDriver->setSamplerState( mSamplerIndex, mStates );

    // Allow texture to update if it needs to.
    cgTextureHandle hNonConstTexture = hTexture;
    cgTexture * pTexture = hNonConstTexture.getResource(false);
    if ( pTexture )
        pTexture->update();

    // Bind to the device.
    mDriver->setTexture( mSamplerIndex, hTexture );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Sets this sampler's properties to the global or custom shader using
/// the specified texture to override the internal version.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::apply( cgUInt32 nSamplerRegister, const cgTextureHandle & hTexture )
{
    // Generate new sampler states if the current states have been modified.
    if ( mStatesDirty )
    {
        cgResourceManager * pManager = mDriver->getResourceManager();
        if ( pManager->createSamplerState( &mStates, mStateDesc, 0, cgDebugSource() ) == false )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to generate new sampler state object while applying sampler '%s'.\n"), mName.c_str() );
            return false;
        
        } // End if failed

        // States are no longer dirty
        mStatesDirty = false;

    } // End if dirty

    // Apply the current sampler state.
    mDriver->setSamplerState( nSamplerRegister, mStates );

    // Allow texture to update if it needs to.
    cgTextureHandle hNonConstTexture = hTexture;
    cgTexture * pTexture = hNonConstTexture.getResource(false);
    if ( pTexture )
        pTexture->update();

    // Bind to the device.
    mDriver->setTexture( nSamplerRegister, hTexture );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Set this sampler to the global or custom shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::apply( )
{
    // Have we already determined the sampler register index to which we are 
    // being applied? If not, query the surface shader.
    if ( mSamplerIndex < 0 )
    {
        // Surface shader must be resident
        cgSurfaceShader * pShader = mShader.getResource(true);
        cgAssertEx( pShader != CG_NULL, "Unable to automatically discover the applicable sampler register index without an associated surface shader." );
        
        // Find the appropriate binding index for the sampler with this name.
        mSamplerIndex = pShader->findSamplerRegister( mName );
        if ( mSamplerIndex < 0 )
            return false;

    } // End if no sampler index.

    // Generate new sampler states if the current states have been modified.
    if ( mStatesDirty )
    {
        cgResourceManager * pManager = mDriver->getResourceManager();
        if ( pManager->createSamplerState( &mStates, mStateDesc, 0, cgDebugSource() ) == false )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to generate new sampler state object while applying sampler '%s'.\n"), mName.c_str() );
            return false;
        
        } // End if failed

        // States are no longer dirty
        mStatesDirty = false;

    } // End if dirty

    // Apply the current sampler state.
    mDriver->setSamplerState( mSamplerIndex, mStates );

    // Allow texture to update if it needs to.
    cgTexture * pTexture = mTexture.getResource(false);
    if ( pTexture )
        pTexture->update();

    // If no texture is available, and this is the diffuse / normal sampler, apply the 
    // appropriate default texture. Otherwise, simply bind the current texture to the device.
    if ( mApplyDefaultDiffuse )
        mDriver->setTexture( mSamplerIndex, mDriver->getResourceManager()->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->getTexture() );
    else if ( mApplyDefaultNormal )
        mDriver->setTexture( mSamplerIndex, mDriver->getResourceManager()->getDefaultSampler( cgResourceManager::DefaultNormalSampler )->getTexture() );
    else
        mDriver->setTexture( mSamplerIndex, mTexture );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Set this sampler to the global or custom shader.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::apply( cgUInt32 nSamplerRegister )
{
    // Generate new sampler states if the current states have been modified.
    if ( mStatesDirty )
    {
        cgResourceManager * pManager = mDriver->getResourceManager();
        if ( pManager->createSamplerState( &mStates, mStateDesc, 0, cgDebugSource() ) == false )
        {
            cgAppLog::write( cgAppLog::Warning, _T("Failed to generate new sampler state object while applying sampler '%s'.\n"), mName.c_str() );
            return false;
        
        } // End if failed

        // States are no longer dirty
        mStatesDirty = false;

    } // End if dirty

    // Apply the current sampler state.
    mDriver->setSamplerState( nSamplerRegister, mStates );

    // Allow texture to update if it needs to.
    cgTexture * pTexture = mTexture.getResource(false);
    if ( pTexture )
        pTexture->update();

    // If no texture is available, and this is the diffuse / normal sampler, apply the 
    // appropriate default texture. Otherwise, simply bind the current texture to the device.
    if ( mApplyDefaultDiffuse )
        mDriver->setTexture( nSamplerRegister, mDriver->getResourceManager()->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler )->getTexture() );
    else if ( mApplyDefaultNormal )
        mDriver->setTexture( nSamplerRegister, mDriver->getResourceManager()->getDefaultSampler( cgResourceManager::DefaultNormalSampler )->getTexture() );
    else
        mDriver->setTexture( nSamplerRegister, mTexture );
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setTexture ()
/// <summary>
/// Set the texture to use for this sampler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::setTexture( const cgTextureHandle & hTexture )
{
    return setTexture( hTexture, false );
}

//-----------------------------------------------------------------------------
//  Name : setTexture ()
/// <summary>
/// Set the texture to use for this sampler. Optionally, database updates can 
/// be disabled by specifying a value of 'true' to the final -- bNoSerialize --
/// parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::setTexture( const cgTextureHandle & hTexture, bool bNoSerialize )
{
    // Retrieve the underlying texture resource (no need to load at this stage)
    const cgTexture * pTexture = hTexture.getResourceSilent();
    
    // Update database texture filename.
    if ( !bNoSerialize && shouldSerialize() )
    {
        if ( pTexture )
            mUpdateTexture.bindParameter( 1, pTexture->getResourceName() );
        else
            mUpdateTexture.bindParameter( 1, CG_NULL, 0 );
        mUpdateTexture.bindParameter( 2, mReferenceId );
    
        // Process!
        if ( mUpdateTexture.step( true ) == false )
        {
            cgString strError;
            mUpdateTexture.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update texture data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return false;

        } // End if failed

    } // End if serialize

    // Store the new texture handle
    mTexture = hTexture;

    // Update cache
    updateCachedValues();

    // Notify subscribers that the texture has changed.
    cgMessage Msg;
    Msg.messageId = cgSystemMessages::Resources_ResourceUpdated;
    cgReferenceManager::sendMessageToSubscribers( getReferenceId(), &Msg );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadTexture()
/// <summary>
/// Load the specified texture for application with this sampler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::loadTexture( const cgInputStream & Stream, cgUInt32 nFlags )
{
    return loadTexture( Stream, nFlags, cgDebugSource() );
}
bool cgSampler::loadTexture( const cgInputStream & Stream, cgUInt32 nFlags, const cgDebugSourceInfo & _debugSource )
{
    // Validate requirements
    if ( !mDriver )
        return false;
    
    // Ask the resource manager to load this texture.
    cgTextureHandle hTexture;
    cgResourceManager * pResources = mDriver->getResourceManager();
    if ( pResources->loadTexture( &hTexture, Stream, nFlags, _debugSource ) == false )
        return false;

    // Apply to this sampler
    return setTexture( hTexture );
}

// ToDo: 9999 - Remove when complete
/*//-----------------------------------------------------------------------------
//  Name : ParseXML ()
/// <summary>
/// Process the specified XML and import all sampler information.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::ParseXML( cgXMLNode & xData, cgUInt32 nFlags = 0, const cgDebugSourceInfo & _debugSource = cgDebugSourceInfo(_T(""),0)  )
{
    cgXMLNode xSection, xState;

    // Default the sampler states to values that Prescience
    // considers to be "default" (and thus doesn't write a property).
    mStateDesc.addressU       = cgAddressingMode::Wrap;
	mStateDesc.addressV       = cgAddressingMode::Wrap;
	mStateDesc.addressW       = cgAddressingMode::Clamp;
    mStateDesc.magnificationFilter      = cgFilterMethod::Linear;
	mStateDesc.minificationFilter      = cgFilterMethod::Linear;
	mStateDesc.mipmapFilter      = cgFilterMethod::Linear;
    mStateDesc.borderColor    = 0;
	mStateDesc.MipMapLODBias  = 0;
	mStateDesc.MaxMipLevel    = 0;
	mStateDesc.maximumAnisotropy  = 1;
    memset( mStateDesc.TextureSize, 0, 4 * sizeof(cgFloat) );

    // Search for sampler entries
    for ( cgUInt32 i = 0; i < xData.GetChildNodeCount(); ++i )
    {
        xSection = xData.GetChildNode( i );

        // What type of node is this?
        if ( xSection.IsOfType( _T("Texture") ) )
        {
            ResourceHandle hTexture;

            // Get access to required systems
            cgResourceManager * pResources = mDriver->getResourceManager();

            // Load the texture
            if ( pResources->loadTexture( &hTexture, xSection.GetText(), nFlags, _debugSource ) == true )
                this->setTexture( hTexture );

        } // End if Texture
        else if ( xSection.IsOfType( _T("States") ) )
        {
            for ( cgUInt32 j = 0; j < xSection.GetChildNodeCount(); ++j )
            {
                bool bMinFilter = false, bMagFilter = false, bMipFilter = false;
                bool bAddressU  = false, bAddressV  = false, bAddressW  = false;

                // Get child node
                xState = xSection.GetChildNode( j );
                
                // What state is this?
                if ( (bMinFilter = xState.IsOfType( _T("minificationFilter") )) ||
                     (bMagFilter = xState.IsOfType( _T("magnificationFilter") )) ||
                     (bMipFilter = xState.IsOfType( _T("mipmapFilter") )) )
                {
                    cgInt nValue = cgFilterMethod::Linear;
                    if ( xState.GetText().compare( _T("Point"), true ) == 0 )
                        nValue = cgFilterMethod::Point;
                    else if ( xState.GetText().compare( _T("Anisotropic"), true ) == 0 )
                        nValue = cgFilterMethod::Anisotropic;

                    // Store
                    if ( bMinFilter == true )
                        mStateDesc.minificationFilter = nValue;
                    else if ( bMagFilter == true )
                        mStateDesc.magnificationFilter = nValue;
                    else if ( bMipFilter == true )
                        mStateDesc.mipmapFilter = nValue;

                } // End if Filters
                else if ( (bAddressU = xState.IsOfType( _T("addressU") )) ||
                          (bAddressV = xState.IsOfType( _T("addressV") )) ||
                          (bAddressW = xState.IsOfType( _T("addressW") )) )
                {
                    cgInt nValue = (bAddressW) ? cgAddressingMode::Clamp : cgAddressingMode::Wrap;
                    if ( xState.GetText().compare( _T("Wrap"), true ) == 0 )
                        nValue = cgAddressingMode::Wrap;
                    else if ( xState.GetText().compare( _T("Mirror"), true ) == 0 )
                        nValue = cgAddressingMode::Mirror;
                    else if ( xState.GetText().compare( _T("Clamp"), true ) == 0 )
                        nValue = cgAddressingMode::Clamp;
                    else if ( xState.GetText().compare( _T("Border"), true ) == 0 )
                        nValue = cgAddressingMode::Border;
                    else if ( xState.GetText().compare( _T("MirrorOnce"), true ) == 0 )
                        nValue = cgAddressingMode::MirrorOnce;

                    // Store
                    if ( bAddressU == true )
                        mStateDesc.addressU = nValue;
                    else if ( bAddressV == true )
                        mStateDesc.addressV = nValue;
                    else if ( bAddressW == true )
                        mStateDesc.addressW = nValue;

                } // End if Addressing
                else if ( xState.IsOfType( _T("mipmapLODBias") ) )
                {
                    cgStringParser( xState.GetText() ) >> mStateDesc.MipMapLODBias;

                } // End if mipmapLODBias
                else if ( xState.IsOfType( _T("MaxMipLevel") ) )
                {
                    cgStringParser( xState.GetText() ) >> mStateDesc.MaxMipLevel;

                } // End if MaxMipLevel
                else if ( xState.IsOfType( _T("maximumAnisotropy") ) )
                {
                    cgStringParser( xState.GetText() ) >> mStateDesc.maximumAnisotropy;

                } // End if maximumAnisotropy
                else if ( xState.IsOfType( _T("borderColor") ) )
                {
                    cgColorValue Color;
                    if ( cgStringUtility::TryParse( xState.GetText(), Color ) == true )
                        mStateDesc.borderColor = Color;

                } // End if borderColor
            
            } // Next State

        } // End if States

    } // Next Child Node

    // Success!
    return true;

}*/

//-----------------------------------------------------------------------------
//  Name : getTexture () (const overload)
/// <summary>
/// Retrieve the texture used by this sampler.
/// </summary>
//-----------------------------------------------------------------------------
const cgTextureHandle & cgSampler::getTexture( ) const
{
    return mTexture;
}

//-----------------------------------------------------------------------------
//  Name : getTexture ()
/// <summary>
/// Retrieve the texture used by this sampler.
/// </summary>
//-----------------------------------------------------------------------------
cgTextureHandle cgSampler::getTexture( )
{
    return mTexture;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader () (const overload)
/// <summary>
/// Retrieve the custom surface shader used by this sampler (if any).
/// </summary>
//-----------------------------------------------------------------------------
const cgSurfaceShaderHandle & cgSampler::getSurfaceShader( ) const
{
    return mShader;
}

//-----------------------------------------------------------------------------
//  Name : getSurfaceShader ()
/// <summary>
/// Retrieve the custom surface shader used by this sampler (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgSurfaceShaderHandle cgSampler::getSurfaceShader( )
{
    return mShader;
}

//-----------------------------------------------------------------------------
//  Name : isSystemSampler ()
/// <summary>
/// Determine if this is a system sampler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::isSystemSampler() const
{
    return mSystemSampler;
}

//-----------------------------------------------------------------------------
//  Name : isTextureValid ()
/// <summary>
/// Determine if any valid texture is assigned to this sampler.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::isTextureValid( ) const
{
    return mTexture.isValid();
}

//-----------------------------------------------------------------------------
//  Name : getStates ()
/// <summary>
/// Retrieve const reference to sampler states.
/// </summary>
//-----------------------------------------------------------------------------
const cgSamplerStateDesc & cgSampler::getStates( ) const
{
    return mStateDesc;
}

//-----------------------------------------------------------------------------
//  Name : getStrength ()
/// <summary>
/// Retrieve the strength of the sampled data, i.e. how much it should 
/// influence the result of whatever operation it is being sampled within.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgSampler::getStrength( ) const
{
    return mStrength;
}

//-----------------------------------------------------------------------------
//  Name : setStrength ()
/// <summary>
/// Set the strength of the sampled data, i.e. how much it should 
/// influence the result of whatever operation it is being sampled within.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setStrength( cgFloat strength )
{
    // No-op?
    if ( strength == mStrength )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateStrength.bindParameter( 1, strength );
        mUpdateStrength.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateStrength.step( true ) == false )
        {
            cgString strError;
            mUpdateStrength.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update strength for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStrength = strength;
}

//-----------------------------------------------------------------------------
//  Name : setStates ()
/// <summary>
/// Update sampler states.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setStates( const cgSamplerStateDesc & States )
{
    // Update database
    if ( shouldSerialize() )
    {
        // Begin a transaction so that we can roll back on failure.
        mWorld->beginTransaction( _T("Sampler::setStates") );

        try
        {
            // Update addressing modes.
            prepareQueries();
            mUpdateAddressModes.bindParameter( 1, States.addressU );
            mUpdateAddressModes.bindParameter( 2, States.addressV );
            mUpdateAddressModes.bindParameter( 3, States.addressW );
            mUpdateAddressModes.bindParameter( 4, mReferenceId );
            if ( !mUpdateAddressModes.step( true ) )
            {
                cgString strError;
                mUpdateAddressModes.getLastError( strError );
                throw strError;

            } // End if failed

            // Update border color
            mUpdateBorderColor.bindParameter( 1, States.borderColor.r );
            mUpdateBorderColor.bindParameter( 2, States.borderColor.g );
            mUpdateBorderColor.bindParameter( 3, States.borderColor.b );
            mUpdateBorderColor.bindParameter( 4, States.borderColor.a );
            mUpdateBorderColor.bindParameter( 5, mReferenceId );
            if ( !mUpdateBorderColor.step( true ) )
            {
                cgString strError;
                mUpdateBorderColor.getLastError( strError );
                throw strError;

            } // End if failed

            // Update filter methods
            mUpdateFilterMethods.bindParameter( 1, States.minificationFilter );
            mUpdateFilterMethods.bindParameter( 2, States.magnificationFilter );
            mUpdateFilterMethods.bindParameter( 3, States.mipmapFilter );
            mUpdateFilterMethods.bindParameter( 4, mReferenceId );
            if ( !mUpdateFilterMethods.step( true ) )
            {
                cgString strError;
                mUpdateFilterMethods.getLastError( strError );
                throw strError;

            } // End if failed

            // Update LOD values
            mUpdateLODDetails.bindParameter( 1, States.mipmapLODBias );
            mUpdateLODDetails.bindParameter( 2, States.minimumMipmapLOD );
            mUpdateLODDetails.bindParameter( 3, States.maximumMipmapLOD );
            mUpdateLODDetails.bindParameter( 4, mReferenceId );
            if ( !mUpdateLODDetails.step( true ) )
            {
                cgString strError;
                mUpdateLODDetails.getLastError( strError );
                throw strError;

            } // End if failed

            // Update anisotropy values
            mUpdateMaxAnisotropy.bindParameter( 1, States.maximumAnisotropy );
            mUpdateMaxAnisotropy.bindParameter( 2, mReferenceId );
            if ( !mUpdateMaxAnisotropy.step( true ) )
            {
                cgString strError;
                mUpdateMaxAnisotropy.getLastError( strError );
                throw strError;

            } // End if failed

        } // End try

        catch ( const cgString & e )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, e.c_str() );
            mWorld->rollbackTransaction( _T("Sampler::setStates") );
            return;

        } // End catch

        // Commit!
        mWorld->commitTransaction( _T("Sampler::setStates") );

    } // End if should serialize

    // Update local member
    mStateDesc = States;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setAddressU ()
/// <summary>
/// Set texture addressing mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setAddressU( cgAddressingMode::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.addressU )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAddressModes.bindParameter( 1, (cgInt32)Value );
        mUpdateAddressModes.bindParameter( 2, mStateDesc.addressV );
        mUpdateAddressModes.bindParameter( 3, mStateDesc.addressW );
        mUpdateAddressModes.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateAddressModes.step( true ) == false )
        {
            cgString strError;
            mUpdateAddressModes.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.addressU = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setAddressV ()
/// <summary>
/// Set texture addressing mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setAddressV( cgAddressingMode::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.addressV )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAddressModes.bindParameter( 1, mStateDesc.addressU );
        mUpdateAddressModes.bindParameter( 2, (cgInt32)Value );
        mUpdateAddressModes.bindParameter( 3, mStateDesc.addressW );
        mUpdateAddressModes.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateAddressModes.step( true ) == false )
        {
            cgString strError;
            mUpdateAddressModes.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.addressV = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setAddressW ()
/// <summary>
/// Set texture addressing mode.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setAddressW( cgAddressingMode::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.addressW )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateAddressModes.bindParameter( 1, mStateDesc.addressU );
        mUpdateAddressModes.bindParameter( 2, mStateDesc.addressV );
        mUpdateAddressModes.bindParameter( 3, (cgInt32)Value );
        mUpdateAddressModes.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateAddressModes.step( true ) == false )
        {
            cgString strError;
            mUpdateAddressModes.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.addressW = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setBorderColor ()
/// <summary>
/// Set texture border color to use when oversampling texture edges.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setBorderColor( const cgColorValue & Value )
{
    // No-op?
    if ( Value == mStateDesc.borderColor )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateBorderColor.bindParameter( 1, Value.r );
        mUpdateBorderColor.bindParameter( 2, Value.g );
        mUpdateBorderColor.bindParameter( 3, Value.b );
        mUpdateBorderColor.bindParameter( 4, Value.a );
        mUpdateBorderColor.bindParameter( 5, mReferenceId );
    
        // Process!
        if ( mUpdateBorderColor.step( true ) == false )
        {
            cgString strError;
            mUpdateBorderColor.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.borderColor = Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMagnificationFilter ()
/// <summary>
/// Set texture magnification filter.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMagnificationFilter( cgFilterMethod::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.magnificationFilter )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateFilterMethods.bindParameter( 1, mStateDesc.minificationFilter );
        mUpdateFilterMethods.bindParameter( 2, (cgInt32)Value );
        mUpdateFilterMethods.bindParameter( 3, mStateDesc.mipmapFilter );
        mUpdateFilterMethods.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateFilterMethods.step( true ) == false )
        {
            cgString strError;
            mUpdateFilterMethods.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.magnificationFilter = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMinificationFilter ()
/// <summary>
/// Set texture minification filter.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMinificationFilter( cgFilterMethod::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.minificationFilter )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateFilterMethods.bindParameter( 1, (cgInt32)Value );
        mUpdateFilterMethods.bindParameter( 2, mStateDesc.magnificationFilter );
        mUpdateFilterMethods.bindParameter( 3, mStateDesc.mipmapFilter );
        mUpdateFilterMethods.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateFilterMethods.step( true ) == false )
        {
            cgString strError;
            mUpdateFilterMethods.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.minificationFilter = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMipmapFilter ()
/// <summary>
/// Set texture mip-mapping filter.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMipmapFilter( cgFilterMethod::Base Value )
{
    // No-op?
    if ( Value == mStateDesc.mipmapFilter )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateFilterMethods.bindParameter( 1, mStateDesc.minificationFilter );
        mUpdateFilterMethods.bindParameter( 2, mStateDesc.magnificationFilter );
        mUpdateFilterMethods.bindParameter( 3, (cgInt32)Value );
        mUpdateFilterMethods.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateFilterMethods.step( true ) == false )
        {
            cgString strError;
            mUpdateFilterMethods.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.mipmapFilter = (cgInt32)Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMipmapLODBias ()
/// <summary>
/// Set texture mip-mapping level of detail bias value.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMipmapLODBias( cgFloat Value )
{
    // No-op?
    if ( Value == mStateDesc.mipmapLODBias )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateLODDetails.bindParameter( 1, Value );
        mUpdateLODDetails.bindParameter( 2, mStateDesc.minimumMipmapLOD );
        mUpdateLODDetails.bindParameter( 3, mStateDesc.maximumMipmapLOD );
        mUpdateLODDetails.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateLODDetails.step( true ) == false )
        {
            cgString strError;
            mUpdateLODDetails.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.mipmapLODBias = Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMagnificationFilter ()
/// <summary>
/// Set maximum anisotropic texture filtering level.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMaximumAnisotropy( cgInt32 Value )
{
    // No-op?
    if ( Value == mStateDesc.maximumAnisotropy )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateMaxAnisotropy.bindParameter( 1, Value );
        mUpdateMaxAnisotropy.bindParameter( 2, mReferenceId );
    
        // Process!
        if ( mUpdateMaxAnisotropy.step( true ) == false )
        {
            cgString strError;
            mUpdateMaxAnisotropy.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.maximumAnisotropy = Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMaximumMipmapLOD ()
/// <summary>
/// Set maximum allowed mip level where 0 is the highest detail and any value
/// larger than this represents a mip at a lower detail.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMaximumMipmapLOD( cgFloat Value )
{
    // No-op?
    if ( Value == mStateDesc.maximumMipmapLOD )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateLODDetails.bindParameter( 1, mStateDesc.mipmapLODBias );
        mUpdateLODDetails.bindParameter( 2, mStateDesc.minimumMipmapLOD );
        mUpdateLODDetails.bindParameter( 3, Value );
        mUpdateLODDetails.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateLODDetails.step( true ) == false )
        {
            cgString strError;
            mUpdateLODDetails.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.maximumMipmapLOD = Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : setMinimumMipmapLOD ()
/// <summary>
/// Set minimum allowed mip level where 0 is the highest detail and any value
/// larger than this represents a mip at a lower detail.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::setMinimumMipmapLOD( cgFloat Value )
{
    // No-op?
    if ( Value == mStateDesc.minimumMipmapLOD )
        return;

    // Update database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateLODDetails.bindParameter( 1, mStateDesc.mipmapLODBias );
        mUpdateLODDetails.bindParameter( 2, Value );
        mUpdateLODDetails.bindParameter( 3, mStateDesc.maximumMipmapLOD );
        mUpdateLODDetails.bindParameter( 4, mReferenceId );
    
        // Process!
        if ( mUpdateLODDetails.step( true ) == false )
        {
            cgString strError;
            mUpdateLODDetails.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update state data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;

        } // End if failed

    } // End if should serialize

    // Update local member
    mStateDesc.minimumMipmapLOD = Value;

    // State object needs to be re-generated
    mStatesDirty = true;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Retrieve the name of this sampler (i.e. Diffuse).
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgSampler::getName( ) const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : compare ()
/// <summary>
/// Performs a deep compare allowing us to perform rapid binary tree 
/// style searches (such as the map in our resource manager).
/// </summary>
//-----------------------------------------------------------------------------
cgInt cgSampler::compare( const cgSampler & Sampler ) const
{
    cgInt64 nDifference = 0;
    cgFloat fDifference = 0;

    // Rapid numeric / handle tests first.
    fDifference = (mStrength - Sampler.mStrength);
    if ( fDifference ) return (fDifference < 0) ? -1 : 1;
    nDifference = (mTexture == Sampler.mTexture) ? 0 : (mTexture < Sampler.mTexture) ? -1 : 1;
    if ( nDifference != 0 ) return (cgInt)nDifference;
    nDifference = (mShader == Sampler.mShader) ? 0 : (mShader < Sampler.mShader) ? -1 : 1;
    if ( !mShader.isValid() )
    {
        // Sampler index must be defined if there is no shader.
        nDifference = mSamplerIndex - Sampler.mSamplerIndex;
        if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
    
    } // End if no shader
    nDifference = memcmp( &mStateDesc, &Sampler.mStateDesc, sizeof(cgSamplerStateDesc) );
    if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
    
    // compare names
    nDifference = mName.compare( Sampler.mName );
    if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
    
    // Totally equal
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Sampler )
        return true;

    // Supported by base?
    return cgWorldComponent::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgSampler::getDatabaseTable( ) const
{
    return _T("Samplers");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new sampler into the database if applicable.
    if ( !insertComponentData() )
        return false;

    // Call base class implementation last.
    return cgWorldComponent::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        const cgTexture * pTexture = mTexture.getResourceSilent();

        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("Sampler::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertSampler.bindParameter( 1, mReferenceId );
        mInsertSampler.bindParameter( 2, mName );
        if ( pTexture )
            mInsertSampler.bindParameter( 3, pTexture->getResourceName() );
        else
            mInsertSampler.bindParameter( 3, CG_NULL, 0 );
        mInsertSampler.bindParameter( 4, mStateDesc.addressU );
        mInsertSampler.bindParameter( 5, mStateDesc.addressV );
        mInsertSampler.bindParameter( 6, mStateDesc.addressW );
        mInsertSampler.bindParameter( 7, mStateDesc.minificationFilter );
        mInsertSampler.bindParameter( 8, mStateDesc.magnificationFilter );
        mInsertSampler.bindParameter( 9, mStateDesc.mipmapFilter );
        mInsertSampler.bindParameter( 10, mStateDesc.borderColor.r );
        mInsertSampler.bindParameter( 11, mStateDesc.borderColor.g );
        mInsertSampler.bindParameter( 12, mStateDesc.borderColor.b );
        mInsertSampler.bindParameter( 13, mStateDesc.borderColor.a );
        mInsertSampler.bindParameter( 14, mStateDesc.maximumAnisotropy );
        mInsertSampler.bindParameter( 15, mStateDesc.mipmapLODBias );
        mInsertSampler.bindParameter( 16, mStateDesc.minimumMipmapLOD );
        mInsertSampler.bindParameter( 17, mStateDesc.maximumMipmapLOD );
        mInsertSampler.bindParameter( 18, mStateDesc.comparisonFunction );
        mInsertSampler.bindParameter( 19, mStrength );
        mInsertSampler.bindParameter( 20, mSoftRefCount );

        // Execute
        if ( !mInsertSampler.step( true ) )
        {
            cgString strError;
            mInsertSampler.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for sampler '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("Sampler::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("Sampler::insertComponentData") );

    } // End if !internal

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentLoading() (Virtual)
/// <summary>
/// Virtual method called when the component is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgSampler::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the sampler data.
    prepareQueries();
    mLoadSampler.bindParameter( 1, e->sourceRefId );
    if ( mLoadSampler.step( ) == false || mLoadSampler.nextRow() == false )
    {
        // Log any error.
        cgString strError;
        if ( mLoadSampler.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sampler '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for sampler '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadSampler.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadSampler;

    // Update our local members
    mLoadSampler.getColumn( _T("Name")          , mName );
    mLoadSampler.getColumn( _T("AddressU")      , mStateDesc.addressU );
    mLoadSampler.getColumn( _T("AddressV")      , mStateDesc.addressV );
    mLoadSampler.getColumn( _T("AddressW")      , mStateDesc.addressW );
    mLoadSampler.getColumn( _T("MinFilter")     , mStateDesc.minificationFilter );
    mLoadSampler.getColumn( _T("MagFilter")     , mStateDesc.magnificationFilter );
    mLoadSampler.getColumn( _T("MipFilter")     , mStateDesc.mipmapFilter );
    mLoadSampler.getColumn( _T("BorderColorR")  , mStateDesc.borderColor.r );
    mLoadSampler.getColumn( _T("BorderColorG")  , mStateDesc.borderColor.g );
    mLoadSampler.getColumn( _T("BorderColorB")  , mStateDesc.borderColor.b );
    mLoadSampler.getColumn( _T("BorderColorA")  , mStateDesc.borderColor.a );
    mLoadSampler.getColumn( _T("MaxAnisotropy") , mStateDesc.maximumAnisotropy );
    mLoadSampler.getColumn( _T("MipLODBias")    , mStateDesc.mipmapLODBias );
    mLoadSampler.getColumn( _T("MinLOD")        , mStateDesc.minimumMipmapLOD );
    mLoadSampler.getColumn( _T("MaxLOD")        , mStateDesc.maximumMipmapLOD );
    mLoadSampler.getColumn( _T("ComparisonFunc"), mStateDesc.comparisonFunction );
    mLoadSampler.getColumn( _T("Strength")      , mStrength );

    // Load the specified texture.
    cgString strFile;
    cgTextureHandle hTexture;
    cgResourceManager * pResources = mDriver->getResourceManager();
    mLoadSampler.getColumn( _T("Filename"), strFile );
    // ToDo: 9999 - Child flags and debug source.
    if ( !strFile.empty() && pResources->loadTexture( &hTexture, strFile, 0, cgDebugSource() /*nFlags, _debugSource*/ ) == true )
        setTexture( hTexture, true );
    else
        updateCachedValues();

    // Call base class implementation to read remaining data.
    if ( !cgWorldComponent::onComponentLoading( e ) )
        return false;

    // If our reference identifier doesn't match the source identifier, we were cloned.
    // As a result, make sure that we are serialized to the database accordingly.
    if ( mReferenceId != e->sourceRefId )
    {
        if ( !insertComponentData() )
            return false;

    } // End if cloned

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgSampler::prepareQueries()
{
    // Any database supplied?
    if ( mWorld == CG_NULL )
        return;

    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertSampler.isPrepared( mWorld ) )
            mInsertSampler.prepare( mWorld, _T("INSERT INTO 'Samplers' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20)"), true ); 
        if ( !mUpdateTexture.isPrepared( mWorld ) )
            mUpdateTexture.prepare( mWorld, _T("UPDATE 'Samplers' SET Filename=?1 WHERE RefId=?2"), true );
        if ( !mUpdateAddressModes.isPrepared( mWorld ) )
            mUpdateAddressModes.prepare( mWorld, _T("UPDATE 'Samplers' SET AddressU=?1,AddressV=?2,AddressW=?3 WHERE RefId=?4"), true );
        if ( !mUpdateFilterMethods.isPrepared( mWorld ) )
            mUpdateFilterMethods.prepare( mWorld, _T("UPDATE 'Samplers' SET MinFilter=?1,MagFilter=?2,MipFilter=?3 WHERE RefId=?4"), true );
        if ( !mUpdateLODDetails.isPrepared( mWorld ) )
            mUpdateLODDetails.prepare( mWorld, _T("UPDATE 'Samplers' SET MipLODBias=?1,MinLOD=?2,MaxLOD=?3 WHERE RefId=?4"), true );
        if ( !mUpdateMaxAnisotropy.isPrepared( mWorld ) )
            mUpdateMaxAnisotropy.prepare( mWorld, _T("UPDATE 'Samplers' SET MaxAnisotropy=?1 WHERE RefId=?2"), true );
        if ( !mUpdateBorderColor.isPrepared( mWorld ) )
            mUpdateBorderColor.prepare( mWorld, _T("UPDATE 'Samplers' SET BorderColorR=?1,BorderColorG=?2,BorderColorB=?3,BorderColorA=?4 WHERE RefId=?5"), true );
        if ( !mUpdateStrength.isPrepared( mWorld ) )
            mUpdateStrength.prepare( mWorld, _T("UPDATE 'Samplers' SET Strength=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadSampler.isPrepared( mWorld ) )
        mLoadSampler.prepare( mWorld, _T("SELECT * FROM 'Samplers' WHERE RefId=?1"), true );
}