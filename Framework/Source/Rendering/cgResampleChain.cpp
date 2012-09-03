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
// Name : cgResampleChain.cpp                                                //
//                                                                           //
// Desc : Provides basic support for managing render target chains.          //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgResampleChain Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgResampleChain.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgRenderTarget.h>
#include <Resources/cgResourceManager.h>

///////////////////////////////////////////////////////////////////////////////
// cgResampleChain Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgResampleChain () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChain::cgResampleChain( ) : mLastLevelCountRequest( 0 )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgResampleChain () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChain::~cgResampleChain()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () 
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChain::dispose( bool bDisposeBase )
{
    // Clear the chain
    mChain.clear();
}

//-----------------------------------------------------------------------------
//  Name : getLevelCount () 
/// <summary>
/// Returns the number of levels in the resampling chain (including top level).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgResampleChain::getLevelCount() const
{
    return mChain.size();
}

//-----------------------------------------------------------------------------
//  Name : getLevel () 
/// <summary>
/// Returns the requested level texture from the chain.
/// </summary>
//-----------------------------------------------------------------------------
const cgRenderTargetHandle & cgResampleChain::getLevel( cgUInt32 nLevel ) const
{
    cgAssert( nLevel < getLevelCount() );
    return mChain[ nLevel ];
}

//-----------------------------------------------------------------------------
//  Name : getLevelIndex () 
/// <summary>
/// Returns the index of the level that matches the provided texture resolution.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgResampleChain::getLevelIndex( cgUInt32 nWidth, cgUInt32 nHeight )
{
    cgTexture * pSrcTexture;
    for ( cgUInt32 i = 0; i < getLevelCount(); ++i )
    {
        pSrcTexture = mChain[ i ].getResource( false );
        if ( pSrcTexture->getInfo().width == nWidth && pSrcTexture->getInfo().height == nHeight )
            return i;
    }
    
    // A matching level was not found
    return -1;
}

//-----------------------------------------------------------------------------
//  Name : SetTexture () 
/// <summary>
/// Sets a new top level texture and rebuilds the chain if it cannot be reused.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResampleChain::setSource( const cgRenderTargetHandle & hTexture )
{
    return setSource( CG_NULL, hTexture, DefaultDownSampleLevels, cgString::Empty );
}
bool cgResampleChain::setSource( const cgRenderTargetHandle & hTexture, cgUInt32 nLevels )
{
    return setSource( CG_NULL, hTexture, nLevels, cgString::Empty );
}
bool cgResampleChain::setSource( const cgRenderTargetHandle & hTexture, const cgString & strInstanceId )
{
    return setSource( CG_NULL, hTexture, DefaultDownSampleLevels, strInstanceId );
}
bool cgResampleChain::setSource( cgRenderView * pView, const cgRenderTargetHandle & hTexture, const cgString & strInstanceId )
{
    return setSource( pView, hTexture, DefaultDownSampleLevels, strInstanceId );
}
bool cgResampleChain::setSource( cgRenderView * pView, const cgRenderTargetHandle & hTexture, cgUInt32 nLevels, const cgString & strInstanceId )
{
    // If we received a null source texture, just clear the chain and exit
    if ( !hTexture.isValid() )
    {
        mChain.clear();
        mLastLevelCountRequest = 0;
        return true;
    }

    // If the texture is the same and the requested number of levels hasn't changed, we're good
    if ( !mChain.empty() && hTexture == mChain[0] && nLevels == mLastLevelCountRequest )
        return true;

    // Get the source texture and its properties
    cgTextureHandle hSrc = hTexture;
    cgTexture * pSrcTexture = hSrc.getResource( false );
    const cgImageInfo & SourceInfo = pSrcTexture->getInfo();

    // Get the resource manager from the texture
    cgResourceManager * pResources = pSrcTexture->getManager(); 

    // Compute size of chain that will result given starting dimensions (includes top level)
    cgUInt32 nMaxNumLevels = computeLevelCount( SourceInfo.width, SourceInfo.height );

    // Choose the smaller of user requested level count and maximum possible 
    cgUInt32 nNumLevels = (nLevels == DefaultDownSampleLevels) ? nMaxNumLevels : min( nMaxNumLevels, nLevels );

    // Check the current texture properties to see if we can reuse the existing chain
    bool bGenerateNewChain = true;
    if ( !mChain.empty() )
    {
        cgTexture * pCurrTexture = mChain[0].getResource( false );
        const cgImageInfo & CurrentInfo = pCurrTexture->getInfo();
        if ( nNumLevels == getLevelCount() && 
             SourceInfo.format == CurrentInfo.format && 
             SourceInfo.width  == CurrentInfo.width  &&
             SourceInfo.height == CurrentInfo.height )
        {
            bGenerateNewChain = false;
        
        } // End if reuse
    
    } // End if chains exist
    
    // If we need a new chain
    if ( bGenerateNewChain )
    {
        // Clear the current chain
        mChain.clear();

        // Push the new top slot texture
        mChain.push_back( hTexture );

        // Create an image structure for render target creation
        cgImageInfo TargetDesc;
        TargetDesc.width     = SourceInfo.width;
        TargetDesc.height    = SourceInfo.height;
        TargetDesc.format    = SourceInfo.format;
        TargetDesc.mipLevels = 1;

        // If a render view was provided, adjust the starting width and height scalars accordingly 
		// if the dimensions of the input surface do not match the dimensions of the current render view
        cgFloat fScalarWidth  = 1.0f;
        cgFloat fScalarHeight = 1.0f;
        if ( pView )
		{
			cgSize & ViewSize = pView->getSize( );
			if ( ViewSize.width != SourceInfo.width )
				fScalarWidth = (cgFloat)SourceInfo.width / (cgFloat)ViewSize.width;

			if ( ViewSize.height != SourceInfo.height )
				fScalarHeight = (cgFloat)SourceInfo.height / (cgFloat)ViewSize.height;
		
        } // End if view supplied

        for ( cgUInt32 i = 0; i < (nNumLevels - 1); i++ )
        {
            cgRenderTargetHandle hTarget;

            // Compute the next level's scalar adjustment factor
            fScalarWidth  /= 2.0f;
            fScalarHeight /= 2.0f;

            // If a render view was provided, we will ask it to manage the targets for us
            if ( pView )
            {
                hTarget = pView->getRenderSurface( TargetDesc.format, fScalarWidth, fScalarHeight, strInstanceId );                
            
            } // End if view supplied
            else
            {
                // Compute the final dimensions 
                TargetDesc.width  = max( 1, (cgInt32)ceil(fScalarWidth  * (cgFloat)SourceInfo.width) );
                TargetDesc.height = max( 1, (cgInt32)ceil(fScalarHeight * (cgFloat)SourceInfo.height) );

                // Create the target
                if ( !pResources->createRenderTarget( &hTarget, TargetDesc ) )
                    return false;
            
            } // End if no view

            // Add to the chain
            mChain.push_back( hTarget );
        
        } // Next level

    } // End if new chain

    // Keep track of how many levels were requested (not necessarily fulfilled)
    mLastLevelCountRequest = nLevels;

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : computeLevelCount() (Static)
/// <summary>
/// Given width and height, computes how many levels would this chain use to 
/// get to 1x1.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgResampleChain::computeLevelCount( cgUInt32 nWidth, cgUInt32 nHeight )
{
    if ( nWidth == 1 && nHeight == 1 )
        return 1;

    cgUInt32 nLevelCount = 1; 
    float fScalarWidth   = 1.0f;
    float fScalarHeight  = 1.0f;
    while ( 1 )
    {
        fScalarWidth  /= 2.0f;
        fScalarHeight /= 2.0f;
        const cgUInt32 nNewWidth  = max( 1, (cgInt32)ceil(fScalarWidth  * (cgFloat)nWidth) );
        const cgUInt32 nNewHeight = max( 1, (cgInt32)ceil(fScalarHeight * (cgFloat)nHeight) );
        nLevelCount++;

        if ( nNewWidth == 1 && nNewHeight == 1 )
            break;
    
    } // Next iteration

    return nLevelCount;    
}

//-----------------------------------------------------------------------------
//  Name : setLevel () 
/// <summary>
/// Sets a new level texture and backs up the original for later restoration
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChain::setLevel( cgUInt32 nLevel, const cgRenderTargetHandle & hTexture )
{
    // First ensure we have a valid cache to work with
    if ( mLevelCache.size() != mChain.size() )
    {
        mLevelCache.resize( mChain.size() );
        for ( cgUInt32 i = 0; i < mLevelCache.size(); ++i )
            mLevelCache[ i ] = cgRenderTargetHandle::Null;
    }
    
    // If we have not already backed up the original level in the chain, do so now
    if ( mLevelCache[ nLevel ] == cgRenderTargetHandle::Null )
        mLevelCache[ nLevel ] = mChain[ nLevel ];
    
    // Replace the existing level in the chain with the user supplied one
    mChain[ nLevel ] = hTexture;
}

//-----------------------------------------------------------------------------
//  Name : restoreLevel () 
/// <summary>
/// Restores the original (cached) chain texture back into its slot
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChain::restoreLevel( cgUInt32 nLevel )
{
    // If we have a non-null handle in the cache
    if ( mLevelCache[ nLevel ] != cgRenderTargetHandle::Null )
    {
        // Copy back to main chain
        mChain[ nLevel ] = mLevelCache[ nLevel ];

        // Null out the current entry in the cache
        mLevelCache[ nLevel ] = cgRenderTargetHandle::Null;
    }
}

//-----------------------------------------------------------------------------
//  Name : restoreLevels () 
/// <summary>
/// Restores all (cached) chain textures back into their slots
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChain::restoreLevels( )
{
    for ( cgUInt32 i = 0; i < mLevelCache.size(); ++i )
    {
        restoreLevel( i );
    }
}

///////////////////////////////////////////////////////////////////////////////
// cgResampleChain Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgResampleChainMRT () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChainMRT::cgResampleChainMRT()
{
}
cgResampleChainMRT::cgResampleChainMRT( cgInt32 nTargets )
{
	allocateChain( nTargets );
}

//-----------------------------------------------------------------------------
//  Name : ~cgResampleChainMRT () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResampleChainMRT::~cgResampleChainMRT()
{
    // Clean up
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () 
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::dispose( bool bDisposeBase )
{
	clearChain();
}

//-----------------------------------------------------------------------------
//  Name : allocateChain () 
/// <summary>
/// Releases the chain resources.
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::allocateChain( cgUInt32 nTargets )
{
	clearChain();
	mResampleChains.resize( nTargets );
	for ( cgUInt32 i = 0; i < nTargets; i++ )
		mResampleChains[ i ] = new cgResampleChain();
}

//-----------------------------------------------------------------------------
//  Name : clearChain () 
/// <summary>
/// Releases the chain resources.
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::clearChain( )
{
	for ( cgUInt32 i = 0; i < mResampleChains.size(); i++ )
	{
		if ( mResampleChains[ i ] )
			mResampleChains[ i ]->scriptSafeDispose();
	}
	
	// Clear the chain
    mResampleChains.clear();
}

//-----------------------------------------------------------------------------
//  Name : getLevelCount () 
/// <summary>
/// Returns the number of levels in the resampling chain (including top level).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgResampleChainMRT::getLevelCount() const
{
	return mResampleChains.empty() ? 0 : mResampleChains[ 0 ]->getLevelCount();
}

//-----------------------------------------------------------------------------
//  Name : getLevel () 
/// <summary>
/// Returns the requested level texture from the chain.
/// </summary>
//-----------------------------------------------------------------------------
cgRenderTargetHandleArray cgResampleChainMRT::getLevel( cgUInt32 nLevel )
{
    cgAssert( nLevel < getLevelCount() );
	cgRenderTargetHandleArray aMRT;
	for ( cgUInt32 i = 0; i < mResampleChains.size(); i++ )
		aMRT.push_back( mResampleChains[ i ]->getLevel( nLevel ) );

    return aMRT;
}

//-----------------------------------------------------------------------------
//  Name : getLevelIndex () 
/// <summary>
/// Returns the index of the level that matches the provided texture resolution.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgResampleChainMRT::getLevelIndex( cgUInt32 nWidth, cgUInt32 nHeight )
{
	return mResampleChains.empty() ? -1 : mResampleChains[ 0 ]->getLevelIndex( nWidth, nHeight);
}

//-----------------------------------------------------------------------------
//  Name : SetTexture () 
/// <summary>
/// Sets a new top level texture and rebuilds the chain if it cannot be reused.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResampleChainMRT::setSource( const cgRenderTargetHandleArray & aTextures )
{
    return setSource( CG_NULL, aTextures, DefaultDownSampleLevels, cgString::Empty );
}
bool cgResampleChainMRT::setSource( const cgRenderTargetHandleArray & aTextures, cgUInt32 nLevels )
{
    return setSource( CG_NULL, aTextures, nLevels, cgString::Empty );
}
bool cgResampleChainMRT::setSource( const cgRenderTargetHandleArray & aTextures, const cgString & strInstanceId )
{
    return setSource( CG_NULL, aTextures, DefaultDownSampleLevels, strInstanceId );
}
bool cgResampleChainMRT::setSource( cgRenderView * pView, const cgRenderTargetHandleArray & aTextures, const cgString & strInstanceId )
{
    return setSource( pView, aTextures, DefaultDownSampleLevels, strInstanceId );
}
bool cgResampleChainMRT::setSource( cgRenderView * pView, const cgRenderTargetHandleArray & aTextures, cgUInt32 nLevels, const cgString & strInstanceId )
{
    // If we received an empty array, clear out the chains and exit
    if ( aTextures.empty() )
    {
		for ( cgUInt32 i = 0; i < mResampleChains.size(); i++ )
			mResampleChains[ i ]->setSource( cgRenderTargetHandle::Null );
		
		return true;
    }

	// If the number of targets being managed has changed, build a new chain set...
	if ( aTextures.size() != mResampleChains.size() )
		allocateChain( aTextures.size() );

	// Populate each chain
	for ( cgUInt32 i = 0; i < aTextures.size(); i++ )
	{
		// Attach a designator to ensure each target is unique
		cgString strInstanceId_MRT = strInstanceId;
		strInstanceId_MRT += _T("_MRT") + i;
		
		// Set the chain source
		if ( !mResampleChains[ i ]->setSource( pView, aTextures[ i ], nLevels, strInstanceId_MRT ) )
		{
			cgAppLog::write( cgAppLog::Error, _T("Failed to setSource for resample chain MRT[%d].\n"), i );
			clearChain();
			return false;

		} // Failed

	} // Next texture 

    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setLevel () 
/// <summary>
/// Sets new level textures and backs up the original for later restoration
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::setLevel( cgUInt32 nLevel, const cgRenderTargetHandleArray & aTextures )
{
    if ( aTextures.size() == mResampleChains.size() )
	{
		for ( cgUInt32 i = 0; i < aTextures.size(); i++ )
			mResampleChains[ i ]->setLevel( nLevel, aTextures[ i ] );
	}
}

//-----------------------------------------------------------------------------
//  Name : restoreLevel () 
/// <summary>
/// Restores the original (cached) chain texture back into its slot
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::restoreLevel( cgUInt32 nLevel )
{
	for ( cgUInt32 i = 0; i < mResampleChains.size(); i++ )
		mResampleChains[ i ]->restoreLevel( nLevel );
}

//-----------------------------------------------------------------------------
//  Name : restoreLevels () 
/// <summary>
/// Restores all (cached) chain textures back into their slots
/// </summary>
//-----------------------------------------------------------------------------
void cgResampleChainMRT::restoreLevels( )
{
	for ( cgUInt32 i = 0; i < mResampleChains.size(); i++ )
		mResampleChains[ i ]->restoreLevels();
}
