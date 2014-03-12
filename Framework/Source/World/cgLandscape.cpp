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
// File: cgLandscape.cpp                                                     //
//                                                                           //
// Desc: Contains classes responsible for managing and rendering scene       //
//       landscape / terrain data.                                           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgLandscape Module Includes
//-----------------------------------------------------------------------------
#include <World/cgLandscape.h>
#include <World/cgScene.h>
#include <World/Objects/cgCameraObject.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgLandscapeLayerMaterial.h>
#include <Resources/cgHeightMap.h>
#include <Resources/cgTexture.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgVertexBuffer.h>
#include <Resources/cgIndexBuffer.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include <Rendering/cgSampler.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>
#include <System/cgImage.h>

// ToDo: 9999 - When the spatial tree moves, all objects contained within it need to undergo
// an UpdateObjectOwnership process.

// ToDo: 9999 - When the landscape is modified, the spatial tree LSSums planes need to be updated again.

//-----------------------------------------------------------------------------
// Module Local Structures
//-----------------------------------------------------------------------------
struct ProceduralBatchKey 
{
    cgUInt32 nLayers[3];
    cgUInt32 nLayerCount;

    ProceduralBatchKey( cgUInt32 _Layers[], cgUInt32 _LayerCount ) : nLayerCount( _LayerCount )
    {
        memcpy( nLayers, _Layers, _LayerCount * sizeof(int) );
    }
};

bool operator < ( const ProceduralBatchKey & v1, const ProceduralBatchKey & v2 )
{
    // Check values
    cgUInt32 nLength = min( v1.nLayerCount, v2.nLayerCount );
    for ( cgUInt32 i = 0; i < nLength; ++i )
    {
        if ( v1.nLayers[i] < v2.nLayers[i] )
            return true;
        else if ( v1.nLayers[i] > v2.nLayers[i] )
            return false;

    } // Next Layer Reference

    // Assuming we got here, item with the lowest layer count "wins"
    return (v1.nLayerCount < v2.nLayerCount);
}

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgLandscape::mInsertLandscape;
cgWorldQuery cgLandscape::mInsertProceduralLayer;
cgWorldQuery cgLandscape::mUpdateLandscapeLayout;
cgWorldQuery cgLandscape::mUpdateTextureConfig;
cgWorldQuery cgLandscape::mDeleteProceduralLayers;
cgWorldQuery cgLandscape::mUpdateProceduralLayer;
cgWorldQuery cgLandscape::mLoadLandscape;
cgWorldQuery cgLandscape::mLoadTerrainBlocks;
cgWorldQuery cgLandscape::mLoadProceduralLayers;
cgWorldQuery cgTerrainBlock::mInsertBlock;
cgWorldQuery cgTerrainBlock::mInsertBlockLOD;
cgWorldQuery cgTerrainBlock::mUpdateBlockHeights;
cgWorldQuery cgTerrainBlock::mDeleteBlockLODs;
cgWorldQuery cgTerrainBlock::mLoadBlockLODs;
cgWorldQuery cgLandscapeTextureData::mInsertLayer;
cgWorldQuery cgLandscapeTextureData::mDeleteLayer;
cgWorldQuery cgLandscapeTextureData::mUpdateLayerPaintData;
cgWorldQuery cgLandscapeTextureData::mLoadLayers;

///////////////////////////////////////////////////////////////////////////////
// cgTerrainVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgTerrainVertex::declarator definition
const D3DVERTEXELEMENT9 cgTerrainVertex::declarator[4] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgVector3 normal;
    {0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},
    // cgUInt32 color;
    {0,24,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgLandscape Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgLandscape() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgLandscape::cgLandscape( cgScene * pScene ) : cgSpatialTree( )
{
    // Initialize variables to sensible defaults
    mLandscapeId        = 0;
    mParentScene        = pScene;
    mBlockLayout        = cgSize(0,0);
    mBlockSize          = cgSize(0,0);
    mScale              = cgVector3(1,1,1);
    mDimensions         = cgVector3(0,0,0);
    mOffset             = cgVector3(0,0,0);
    mHeightMap          = CG_NULL;
    mNoiseSampler       = CG_NULL;
    mNormalSampler      = CG_NULL;
    mBlendMapSampler    = CG_NULL;
    mVertexFormat       = CG_NULL;
    mLandscapeFlags     = cgLandscapeFlags::None;
    mTerrainDetail      = 0.1f;
    mMaxTreeDepth       = 0;
    mYVTree             = false;
    mBlendMapSize       = cgSize( 32, 32 );
    mOcclusionSuccess   = 0;
    mOcclusionFailure   = 0;
    mNextOcclusionTest  = 0;
    mOcclusionCull      = true;
    mIsPainting         = false;

    // Clear arrays
    memset( mLayerColorSamplers, 0, 4 * sizeof(cgSampler*) );
    memset( mLayerNormalSamplers, 0, 4 * sizeof(cgSampler*) );

    // Default object culling distances
    CullDescriptor dist;
    mCullDistances.resize( 3 );
    dist.maximumSize = 50;
    dist.distance = 1000;
    dist.maximumSizeSquared  = dist.maximumSize * dist.maximumSize;
    dist.distanceSquared = dist.distance * dist.distance;
    mCullDistances[0] = dist;
    dist.maximumSize = 150;
    dist.distance = 2000;
    dist.maximumSizeSquared  = dist.maximumSize * dist.maximumSize;
    dist.distanceSquared = dist.distance * dist.distance;
    mCullDistances[1] = dist;
    dist.maximumSize = 300;
    dist.distance = 4000;
    dist.maximumSizeSquared  = dist.maximumSize * dist.maximumSize;
    dist.distanceSquared = dist.distance * dist.distance;
    mCullDistances[2] = dist;
    mObjectCullDistance = 8000;
    mObjectCullDistanceSq = mObjectCullDistance * mObjectCullDistance;

    // ToDo: Testing!
    //cgHeightMap ^ oHeightMap = gcnew cgHeightMap( 2049, 2049, 1000 );
    //oHeightMap->LoadSquareRaw( Program::GetAppPath() + "/Oblivion - 2049x2049.raw", cgHeightMap::RawFormat::Gray16 );
    //oHeightMap->LoadSquareRaw( Program::GetAppPath() + "/Extinction3 - Final.raw", cgHeightMap::RawFormat::Gray16 );

    //cgHeightMap ^ oHeightMap = gcnew cgHeightMap( 2689, 2817, 1000 );
    //oHeightMap->LoadRaw( Program::GetAppPath() + "/Morrowind.raw", 2689, 2817, cgHeightMap::RawFormat::Gray16 );
    //oHeightMap->Normalize( -3453, 30422 );

    //cgHeightMap ^ oHeightMap = gcnew cgHeightMap( 4097, 4097, 1000 );
    //oHeightMap->LoadSquareRaw( Program::GetAppPath() + "/Oblivion - 4097x4097.raw", cgHeightMap::RawFormat::Gray16 );

#if 0
    cgLandscapeLayerType * pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Rock" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/terrainhdrock01.dds" );
    pLayerType->LoadNormalSampler( L"Test Assets/Landscape/terrainhdrock01_n.dds" );
    pLayerType->SetScale( cgVector2( 512, 512 ) );
    pLayerType->SetAngle( 0 ); //45;
    m_aLayerTypes->push_back( pLayerType );
    
    pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Grass" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/terraingcgrass01.dds" );
    pLayerType->LoadNormalSampler( L"Test Assets/Landscape/terraingcgrass01_n.dds" );
    pLayerType->SetScale( cgVector2( 512, 512 ) );
    pLayerType->SetAngle( 0 );
    m_aLayerTypes->push_back( pLayerType );
    
    pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Mud" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/terrainmud03.dds" );
    pLayerType->LoadNormalSampler( L"Test Assets/Landscape/terrainmud03_n.dds" );
    pLayerType->SetScale( cgVector2( 512, 512 ) );
    pLayerType->SetAngle( 0 ); //15; 
    m_aLayerTypes->push_back( pLayerType );
    
    pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Snow" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/snow031.jpg" );
    pLayerType->LoadNormalSampler( L"Test Assets/Landscape/snow031_NRM.png" );
    pLayerType->SetScale( cgVector2( 128, 128 ) );
    pLayerType->SetAngle( 0 );
    m_aLayerTypes->push_back( pLayerType );
    
    pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Snow 02" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/snow031.jpg" );
    pLayerType->SetScale( cgVector2( 128, 128 ) );
    pLayerType->SetAngle( 32 );
    m_aLayerTypes->push_back( pLayerType );
    
    pLayerType = new cgLandscapeLayerType( this );
    pLayerType->SetName( L"Hull_004" );
    pLayerType->LoadColorSampler( L"Test Assets/Landscape/Hull_004.dds" );
    pLayerType->LoadNormalSampler( L"Test Assets/Landscape/Hull_004N.dds" );
    pLayerType->SetScale( cgVector2( 128, 128 ) );
    pLayerType->SetAngle( 13 );
    m_aLayerTypes->push_back( pLayerType );

#endif

    // ToDo: Testing procedurals.
    /*ProceduralLayerRef layer;
    
    // Rock.
    layer.oLayer              = 0; 
    layer.minimumHeight          = -1;
    layer.maximumHeight          = 3000;
    layer.heightAttenuationBand    = 0.0f;
    layer.slopeAxis        = Vector3(0,1,0);
    layer.slopeScale         = 0.0f;
    layer.slopeBias          = 1.0f;
    layer.weight             = 1.0f;
    mProceduralLayers.Add( layer );*/

#if 0
    // ToDo: Testing procedurals.
    ProceduralLayerRef layer;
    
    // Grass.
    layer.Name                = L"Procedural Grass";
    layer.nLayerType          = 1;
    layer.fMinHeight          = -1;
    layer.fMaxHeight          = 3000;
    layer.fHeightAttenBand    = 0.0f;
    layer.vecSlopeAxis        = cgVector3(0,1,0);
    layer.fSlopeScale         = 0.0f;
    layer.fSlopeBias          = 1.0f;
    layer.fWeight             = 1.0f;
    layer.bEnableHeight       = false;
    m_aProceduralLayers.push_back( layer );

    // Mud.
    layer.Name                = L"Procedural Mud";
    layer.nLayerType          = 2;
    layer.fMinHeight          = -1;
    layer.fMaxHeight          = 228.3f;
    layer.fHeightAttenBand    = 20.0f;
    layer.vecSlopeAxis        = cgVector3(0,1,0);
    layer.fSlopeScale         = 0.0f;
    layer.fSlopeBias          = 1.0f;
    layer.fWeight             = 1.0f;
    layer.bEnableHeight       = true;
    m_aProceduralLayers.push_back( layer );
    
    /*// Rock.
    layer.Name                = L"Procedural Rock";
    layer.nLayerType          = 0;
    layer.fMinHeight          = -1;
    layer.fMaxHeight          = 3000;
    layer.fHeightAttenBand    = 0;
    layer.vecSlopeAxis        = cgVector3(0,1,0);
    layer.fSlopeScale         = 3.0f;
    layer.fSlopeBias          = -0.5f;
    layer.fWeight             = 1.0f;
    layer.bEnableHeight       = false;
    m_aProceduralLayers.push_back( layer );*/

    // Snow.
    layer.Name                = L"Procedural Snow";
    layer.nLayerType          = 3;
    layer.fMinHeight          = 700;
    layer.fMaxHeight          = 3000;
    layer.fHeightAttenBand    = 100.0f;
    CGEVec3Normalize( &layer.vecSlopeAxis, &cgVector3(-0.25f,0.1f,-1) );
    //layer.fSlopeScale         = -2.0f;
    //layer.fSlopeBias          = 2.0f + (0.5f);
    layer.fSlopeScale         = 3.0f;
    layer.fSlopeBias          = -0.5f;
    layer.fWeight             = 1.2f;
    layer.bEnableHeight       = true;
    m_aProceduralLayers.push_back( layer );

    /*// Snow2.
    layer.nLayerType              = 3;
    layer.fMinHeight          = 1100;
    layer.fMaxHeight          = 3000;
    layer.fHeightAttenBand    = 200.0f;
    //layer.fSlopeScale         = 3.0f;
    //layer.fSlopeBias          = -0.5f;
    layer.fSlopeScale         = 3.0f;
    layer.fSlopeBias          = -0.5f;
    layer.fWeight             = 1.0f;
    m_aProceduralLayers.Add( layer );*/

#endif
    
    // Default
    //cgHeightMap ^ oHeightMap = gcnew cgHeightMap( 257, 257, 0 );
    //load( oHeightMap, Vector3(256, 1500, 256), LandscapeFlags::Dynamic | LandscapeFlags::LODIgnoreY );

    // Morrowind
    //load( oHeightMap, Vector3( 10000, 1500, 10000 ), LandscapeFlags::Dynamic | LandscapeFlags::LODIgnoreY );

    // Oblivion
    //load( oHeightMap, Vector3( 10000, 3500, 10000 ), LandscapeFlags::Dynamic | LandscapeFlags::LODIgnoreY );
}

//-----------------------------------------------------------------------------
// Name : ~cgLandscape() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgLandscape::~cgLandscape()
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
void cgLandscape::dispose( bool bDisposeBase )
{
    // Destroy any active terrain blocks.
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
        delete mTerrainBlocks[i];
    mTerrainBlocks.clear();

    // Destroy LOD information.
    for ( size_t i = 0; i < mLODData.size(); ++i )
        delete mLODData[i];
    mLODData.clear();
        
    // Destroy procedural batch drawing information
    mProceduralBatches.clear();
    
    // Destroy procedural layer information
    mProceduralLayers.clear();
    
    // Destroy geo-mip optimization table
    mMipLookUp.clear();

    // Destroy any resident heightmap data
    delete mHeightMap;
    mHeightMap = CG_NULL;

    // Destroy any resident color map data
    mColorMap.clear();

    // Destroy occlusion horizon data.
    mHorizonBuffer.clear();
    
    // Destroy custom samplers
    if ( mNoiseSampler != CG_NULL )
        mNoiseSampler->scriptSafeDispose();
    mNoiseSampler = CG_NULL;
    if ( mNormalSampler != CG_NULL )
        mNormalSampler->scriptSafeDispose();
    mNormalSampler = CG_NULL;
    if ( mBlendMapSampler != CG_NULL )
        mBlendMapSampler->scriptSafeDispose();
    mBlendMapSampler = CG_NULL;

    // Destroy layer samplers
    for ( size_t i = 0; i < 4; ++i )
    {
        if ( mLayerColorSamplers[i] != CG_NULL )
            mLayerColorSamplers[i]->scriptSafeDispose();
        if ( mLayerNormalSamplers[i] != CG_NULL )
            mLayerNormalSamplers[i]->scriptSafeDispose();

    } // Next Sampler
    memset( mLayerColorSamplers, 0, 4 * sizeof(cgSampler*) );
    memset( mLayerNormalSamplers, 0, 4 * sizeof(cgSampler*) );

    // Release resources.
    mNormalTexture.close( );
    mDepthFillDepthState.close( );
    mProceduralDepthState.close( );
    mPaintedDepthState.close( );
    mPostProcessDepthState.close( );
    mPostProcessBlendState.close( );
    mProceduralBlendState.close( );
    mPaintedBlendState.close( );
    mTerrainBaseDataBuffer.close( );
    mTerrainLayerDataBuffer.close( );
    mTerrainProcDataBuffer.close( );

    // Clear any remaining vars.
    mVertexFormat = CG_NULL;
    mLandscapeId  = 0;

    // Call base class implementation
    if ( bDisposeBase == true )
        cgSpatialTree::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : getScene()
/// <summary>
/// Retrieve the scene to which this landscape belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgScene * cgLandscape::getScene( ) const
{
    return mParentScene;
}

//-----------------------------------------------------------------------------
//  Name : load()
/// <summary>
/// Load the landscape data with the specified identifier from the world 
/// database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::load( cgUInt32 nLandscapeId )
{
    cgInt32  i, x, z, nTemp, nLODCount;

    // Dispose of any prior data.
    dispose( true );

    // Load the internal landscape surface shader ready for landscape initialization
    // and drawing.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    if ( !pResources->createSurfaceShader( &mLandscapeShader, _T("sys://Shaders/Landscape.sh"), 0, cgDebugSource() ) )
        return false;

    // Load the landscape data.
    prepareQueries();
    mLoadLandscape.bindParameter( 1, nLandscapeId );
    if ( !mLoadLandscape.step( ) || !mLoadLandscape.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadLandscape.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for landscape '0x%x'. World database has potentially become corrupt.\n"), nLandscapeId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for landscape '0x%x'. Error: %s\n"), nLandscapeId, strError.c_str() );

        // Release any pending read operation.
        mLoadLandscape.reset();
        return false;
    
    } // End if failed
    
    // Grab basic object properties.
    mLoadLandscape.getColumn( _T("LayoutU"), mBlockLayout.width );
    mLoadLandscape.getColumn( _T("LayoutV"), mBlockLayout.height );
    mLoadLandscape.getColumn( _T("BlockSizeU"), mBlockSize.width );
    mLoadLandscape.getColumn( _T("BlockSizeV"), mBlockSize.height );
    mLoadLandscape.getColumn( _T("BlendMapSizeU"), mBlendMapSize.width );
    mLoadLandscape.getColumn( _T("BlendMapSizeV"), mBlendMapSize.height );
    mLoadLandscape.getColumn( _T("DimensionsX"), mDimensions.x );
    mLoadLandscape.getColumn( _T("DimensionsY"), mDimensions.y );
    mLoadLandscape.getColumn( _T("DimensionsZ"), mDimensions.z );
    mLoadLandscape.getColumn( _T("OffsetX"), mOffset.x );
    mLoadLandscape.getColumn( _T("OffsetY"), mOffset.y );
    mLoadLandscape.getColumn( _T("OffsetZ"), mOffset.z );
    // ToDo: 9999 flags

    // We're done with the main landscape query.
    mLoadLandscape.reset();

    // Heightmap should always be resident in sandbox mode.
    mLandscapeFlags = cgLandscapeFlags::LODIgnoreY;
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        mLandscapeFlags |= cgLandscapeFlags::Dynamic | cgLandscapeFlags::HeightMapResident;

    // Compute additional dimension related values (such as vertex separation scale).
    setDimensions( mDimensions, false );

    // Allocate enough space for every terrain block instance
    mTerrainBlocks.resize( mBlockLayout.width * mBlockLayout.height, CG_NULL );

    // Calculate the total number of LOD levels that can be generated from the specified block sizes
    nTemp = min( mBlockSize.width, mBlockSize.height ) - 1;
    for ( nLODCount = 0; nTemp >= 2 && nLODCount < MaxLandscapeLOD; nTemp >>= 1 )
        nLODCount++;

    // Allocate enough LOD data entries for the maximum LOD level
    mLODData.resize( nLODCount, CG_NULL );
    for ( i = 0; i < nLODCount; ++i )
        mLODData[i] = new cgTerrainLOD();

    // Build the geo-mip optimization lookup table
    buildMipLookUp( );

    // Build the level of detail information
    for ( i = 0; i < nLODCount;  ++i )
    {
        if ( !buildLODLevel( i ) )
            return false;
        
    } // Next LOD Level

    // Reset bounding box so we can compute actual size
    mBounds.reset();

    // Allocate full size heightmap and color maps as required.
    // ToDo: 9999 - Heightmap is always resident at this point in the implementation.
    //              Eventually, we'll have the block page the required data.
    cgSize MapSize = getHeightMapSize( );
    mHeightMap = new cgHeightMap( MapSize );
    mColorMap.resize( MapSize.width * MapSize.height, 0xFFFFFFFF );

    // Load terrain blocks
    mLoadTerrainBlocks.bindParameter( 1, nLandscapeId );
    if ( !mLoadTerrainBlocks.step( ) )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadTerrainBlocks.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve block data for landscape '0x%x'. World database has potentially become corrupt.\n"), nLandscapeId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve block data for landscape '0x%x'. Error: %s\n"), nLandscapeId, strError.c_str() );

        // Release any pending read operation.
        mLoadTerrainBlocks.reset();
        return false;
    
    } // End if failed

    // Process terrain block data.
    for ( ; mLoadTerrainBlocks.nextRow(); )
    {
        cgInt32 nBlockIndex = 0;
        if ( !mLoadTerrainBlocks.getColumn( _T("BlockIndex"), nBlockIndex ) )
            continue;
        
        // Compute the location of this block based on its index.
        cgInt32 x = nBlockIndex % mBlockLayout.width;
        cgInt32 z = nBlockIndex / mBlockLayout.width;

        // Construct terrain block initialization information.
        cgTerrainBlock::TerrainSection Section;
        Section.heightMapBuffer = &mHeightMap->getImageData()[0];
        Section.blockBounds.left     = x * (mBlockSize.width - 1);
        Section.blockBounds.top      = z * (mBlockSize.height - 1);
        Section.blockBounds.right    = ((x + 1) * (mBlockSize.width - 1)) + 1;
        Section.blockBounds.bottom   = ((z + 1) * (mBlockSize.height - 1)) + 1;
        Section.blockBounds.pitchX   = MapSize.width;
        Section.blockBounds.pitchY   = MapSize.height;

        // Create the block
        cgTerrainBlock * pBlock = new cgTerrainBlock( this, nBlockIndex, Section );
        if ( !pBlock->loadBlock( mLoadTerrainBlocks ) )
        {
            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to load terrain block at location %i, %i.\n"), x, z );
            continue;
        
        } // End if failed

        // Retrieve the calculated bounding box of the newly loaded terrain block
        const cgBoundingBox & BlockBounds = pBlock->getBoundingBox();

        // Expand terrain bounding box
        mBounds.addPoint( BlockBounds.min );
        mBounds.addPoint( BlockBounds.max );

        // Store terrain block in main array
        mTerrainBlocks[ nBlockIndex ] = pBlock;

    } // Next Block

    // We're done with the block read query.
    mLoadTerrainBlocks.reset();

    // Finally, set the terrain block neighbor information for LOD testing etc.
    for ( z = 0; z < mBlockLayout.height; ++z )
    {
        for ( x = 0; x < mBlockLayout.width; ++x )
        {
            cgTerrainBlock * pBlock = mTerrainBlocks[ x + z * mBlockLayout.width ];
            if ( !pBlock ) continue;

            // Set neighbor information if we're not on a relevant edge
            if ( z > 0 )
                pBlock->setNeighbor( cgTerrainBlock::North, mTerrainBlocks[ x + (z - 1) * mBlockLayout.width ] );
            if ( x < mBlockLayout.width - 1 )
                pBlock->setNeighbor( cgTerrainBlock::East, mTerrainBlocks[ (x + 1) + z * mBlockLayout.width ] );
            if ( z < mBlockLayout.height - 1 )
                pBlock->setNeighbor( cgTerrainBlock::South, mTerrainBlocks[ x + (z + 1) * mBlockLayout.width ] );
            if ( x > 0 )
                pBlock->setNeighbor( cgTerrainBlock::West, mTerrainBlocks[ (x - 1) + z * mBlockLayout.width ] );

        } // Next Column

    } // Next Row

    // Load procedural layer data
    mLoadProceduralLayers.bindParameter( 1, nLandscapeId );
    if ( !mLoadProceduralLayers.step( ) )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadProceduralLayers.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve procedural layer data for landscape '0x%x'. World database has potentially become corrupt.\n"), nLandscapeId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve procedural layer data for landscape '0x%x'. Error: %s\n"), nLandscapeId, strError.c_str() );

        // Release any pending read operation.
        mLoadProceduralLayers.reset();
        return false;
    
    } // End if failed

    // Process procedural layer data.
    for ( ; mLoadProceduralLayers.nextRow(); )
    {
        // Allocate new slot in the procedural layer array.
        mProceduralLayers.resize( mProceduralLayers.size() + 1 );
        ProceduralLayerRef & Layer = mProceduralLayers.back();

        // Recieve standard parameters
        mLoadProceduralLayers.getColumn( _T("LayerId"), Layer.layerId );
        mLoadProceduralLayers.getColumn( _T("Name"), Layer.name );
        mLoadProceduralLayers.getColumn( _T("EnableHeight"), Layer.enableHeight );
        mLoadProceduralLayers.getColumn( _T("MinHeight"), Layer.minimumHeight );
        mLoadProceduralLayers.getColumn( _T("MaxHeight"), Layer.maximumHeight );
        mLoadProceduralLayers.getColumn( _T("HeightAttenBand"), Layer.heightAttenuationBand );
        mLoadProceduralLayers.getColumn( _T("SlopeAxisX"), Layer.slopeAxis.x );
        mLoadProceduralLayers.getColumn( _T("SlopeAxisY"), Layer.slopeAxis.y );
        mLoadProceduralLayers.getColumn( _T("SlopeAxisZ"), Layer.slopeAxis.z );
        mLoadProceduralLayers.getColumn( _T("SlopeScale"), Layer.slopeScale );
        mLoadProceduralLayers.getColumn( _T("SlopeBias"), Layer.slopeBias );
        mLoadProceduralLayers.getColumn( _T("InvertSlope"), Layer.invertSlope );
        mLoadProceduralLayers.getColumn( _T("Weight"), Layer.weight );

        // Retrieve layer type material.
        cgUInt32 nMaterialId = 0;
        mLoadProceduralLayers.getColumn( _T("MaterialId"), nMaterialId );
        pResources->loadMaterial( &Layer.layerType, mParentScene->getParentWorld(), cgMaterialType::LandscapeLayer, nMaterialId, false, 0, cgDebugSource() );

    } // Next Block

    // We're done with the layer read query.
    mLoadProceduralLayers.reset();

    // Perform post initialization tasks
    if ( !postInit() )
        return false;

    // Generate initial normal map data.
    updateNormalTexture();

    // Load success!
    mLandscapeId = nLandscapeId;
    return true;
}

/*//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mInsertLandscape.bindParameter( 1, m_nReferenceId );
        mInsertLandscape.bindParameter( 2, mBlockLayout.width );
        mInsertLandscape.bindParameter( 3, mBlockLayout.height );
        mInsertLandscape.bindParameter( 4, mBlockSize.width );
        mInsertLandscape.bindParameter( 5, mBlockSize.height );
        mInsertLandscape.bindParameter( 6, mBlendMapSize.width );
        mInsertLandscape.bindParameter( 7, mBlendMapSize.height );
        mInsertLandscape.bindParameter( 8, mDimensions.x );
        mInsertLandscape.bindParameter( 9, mDimensions.y );
        mInsertLandscape.bindParameter( 10, mDimensions.z );
        mInsertLandscape.bindParameter( 11, mOffset.x );
        mInsertLandscape.bindParameter( 12, mOffset.y );
        mInsertLandscape.bindParameter( 13, mOffset.z );
        mInsertLandscape.bindParameter( 14, (cgUInt32)0 ); // Flags
        
        // Execute
        if ( mInsertLandscape.step( true ) == false )
        {
            cgString strError;
            mInsertLandscape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for landscape object '0x%x' into database. Error: %s\n"), m_nReferenceId, strError.c_str() );
            return false;
        
        } // End if failed

    } // End if !internal

    // Call base class implementation last.
    return cgSpatialTreeObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::onComponentDeleted( )
{
    // ToDo: 9999

    // Call base class implementation last.
    cgSpatialTreeObject::onComponentDeleted( );
}*/

//-----------------------------------------------------------------------------
// Name : shouldSerialize ( )
/// <summary>
/// Determine if we should serialize our information to the currently open
/// world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::shouldSerialize( ) const
{
    // ToDo: 9999 - && m_bDisposing
    return ((cgGetSandboxMode() == cgSandboxMode::Enabled) && mLandscapeId != 0);
}

//-----------------------------------------------------------------------------
// Name : getDatabaseId ( )
/// <summary>
/// Retrieve the unique value used to identify this landscape within the world
/// database.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgLandscape::getDatabaseId( ) const
{
    return mLandscapeId;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParentScene->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertLandscape.isPrepared() == false )
            mInsertLandscape.prepare( pWorld, _T("INSERT INTO 'Landscapes' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13)"), true );
        if ( mInsertProceduralLayer.isPrepared() == false )
            mInsertProceduralLayer.prepare( pWorld, _T("INSERT INTO 'Landscapes::ProceduralLayers' VALUES(NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15)"), true );
        if ( mUpdateLandscapeLayout.isPrepared() == false )
            mUpdateLandscapeLayout.prepare( pWorld, _T("UPDATE 'Landscapes' SET LayoutU=?1, LayoutV=?2, BlockSizeU=?3, BlockSizeV=?4, DimensionsX=?5, DimensionsY=?6, DimensionsZ=?7, OffsetX=?8, OffsetY=?9, OffsetZ=?10 WHERE LandscapeId=?11"), true );
        if ( mUpdateTextureConfig.isPrepared() == false )
            mUpdateTextureConfig.prepare( pWorld, _T("UPDATE 'Landscapes' SET BlendMapSizeU=?1, BlendMapSizeV=?2 WHERE LandscapeId=?3"), true );
        if ( mDeleteProceduralLayers.isPrepared() == false )
            mDeleteProceduralLayers.prepare( pWorld, _T("DELETE FROM 'Landscapes::ProceduralLayers' WHERE LandscapeId=?1"), true );
        if ( mUpdateProceduralLayer.isPrepared() == false )
            mUpdateProceduralLayer.prepare( pWorld, _T("UPDATE 'Landscapes::ProceduralLayers' SET Name=?1, MaterialId=?2, EnableHeight=?3, MinHeight=?4, MaxHeight=?5, HeightAttenBand=?6, SlopeAxisX=?7, SlopeAxisY=?8, SlopeAxisZ=?9, SlopeScale=?10, SlopeBias=?11, InvertSlope=?12, Weight=?13 WHERE LayerId=?14"), true ); 
    
    } // End if sandbox

    // Read queries
    if ( mLoadLandscape.isPrepared() == false )
        mLoadLandscape.prepare( pWorld, _T("SELECT * FROM 'Landscapes' WHERE LandscapeId=?1"), true );
    if ( mLoadTerrainBlocks.isPrepared() == false )
        mLoadTerrainBlocks.prepare( pWorld, _T("SELECT * FROM 'Landscapes::Blocks' WHERE LandscapeId=?1"), true );
    if ( mLoadProceduralLayers.isPrepared() == false )
        mLoadProceduralLayers.prepare( pWorld, _T("SELECT * FROM 'Landscapes::ProceduralLayers' WHERE LandscapeId=?1 ORDER BY LayerIndex ASC"), true );
}

// ToDo: Temp - Reload the effect file
/*void cgLandscape::ReloadEffect::set( bool value )
{
    // Load the landscape specific effect file ready for drawing
    cgResourceManager * pResources oResources = m_oParentScene->ResourceManager;
    oResources->RemoveEffect( m_oLandscapeEffect );
    m_oLandscapeEffect = oResources->LoadEffect( Program::GetAppPath() + L"/System/Effects/Landscape.fx" );
}*/

/*//-----------------------------------------------------------------------------
// Name : InsertObject() (Virtual)
/// <summary>
/// When the object is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the object to do so.
/// </summary>
//-----------------------------------------------------------------------------
int cgLandscape::InsertObject( int typeId, cgWorld ^ world )
{
    // Call base class implementation
    cgBaseObject::InsertObject( typeId, world );

    // Insert the new object.
    //prepareQueries();
    //m_oqInsertDummy->bindParameter( 1, m_fSize );
    //m_oqInsertDummy->step( true );
    //m_nObjectId = m_oqInsertDummy->LastInsertId;
    return m_nObjectId;
}

//-----------------------------------------------------------------------------
// Name : RemoveObject() (Virtual)
/// <summary>
/// When the object is removed from the scene, all of its data needs to be
/// removed from the world database. This virtual method allows the object to 
/// do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::RemoveObject( cgWorld ^ world )
{
    // Call base class implementation
    cgBaseObject::RemoveObject( world );

    // Remove the object.
    if ( m_nObjectId != 0 )
    {
        //prepareQueries();
        //m_oqRemoveDummy->bindParameter( 1, m_nObjectId );
        //m_oqInsertDummy->step( true );
    
    } // End if valid object
}

//-----------------------------------------------------------------------------
// Name : GetTypeInfo() (Static)
/// <summary>
/// Retrieve information about the type of this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::GetTypeInfo( Guid typeIdentifier, cgObjectTypeInfo % info )
{
    // Is this our type?
    if ( typeIdentifier == ElementIdentifiers::Landscape )
    {
        info.Author     = L"Game Institute";
        info.Identifier = typeIdentifier;
        info.name       = L"Landscape";
        info.TableName  = L"Elements::Landscape";
        info.TypeName   = (cgLandscape::typeid)->FullName;

        // Success!
        return true;

    } // End if matches

    // Unknown type identifier.
    return false;
}*/

//-----------------------------------------------------------------------------
// Name : import()
/// <summary>
/// Load / initialize the terrain based on the specified heightmap.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::import( cgHeightMap * pHeightMap, const cgVector3 & Dimensions, cgUInt32 InitFlags )
{
    cgLandscapeImportParams Params;
    Params.heightMap        = pHeightMap;
    Params.dimensions        = Dimensions;
    Params.initFlags         = InitFlags;
    Params.blockGrid         = cgSize(128,128);
    Params.blockBlendMapSize = cgSize(128,128);
    
    // Recompute the terrain offset vector that will position the terrain
    // in the center of the world.
    cgSize HeightMapSize = pHeightMap->getSize();
    Params.offset.x = (-(cgFloat)(HeightMapSize.width - 1) * 0.5f) * (Dimensions.x / (cgFloat)(HeightMapSize.width-1));
    Params.offset.y = 0;
    Params.offset.z = ((cgFloat)(HeightMapSize.height - 1) * 0.5f) * (Dimensions.z / (cgFloat)(HeightMapSize.height-1));

    // Load!
    return import( Params );
}

//-----------------------------------------------------------------------------
// Name : import()
/// <summary>
/// Load / initialize the terrain based on the specified parameters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::import( const cgLandscapeImportParams & Params )
{
    cgInt32  i, x, z, nTemp, nLODCount;

    // Load the internal landscape surface shader ready for landscape initialization
    // and drawing.
    cgResourceManager * pResources = mParentScene->getResourceManager();
    if ( !pResources->createSurfaceShader( &mLandscapeShader, _T("sys://Shaders/Landscape.sh"), 0, cgDebugSource() ) )
        return false;

    // Store required values
    mLandscapeFlags  = Params.initFlags;
    mBlendMapSize    = Params.blockBlendMapSize;
    mHeightMap      = Params.heightMap;
    mColorMap       = Params.colorMap;
    mOffset       = Params.offset;

    // Use default (white) color map if necessary.
    if ( mColorMap.empty() )
        mColorMap.resize( mHeightMap->getSize().width * mHeightMap->getSize().height, 0xFFFFFFFF );
    
    // Set the width and height of each block (number of vertices)
    mBlockSize.width  = Params.blockGrid.width + 1;
    mBlockSize.height = Params.blockGrid.height + 1;

    // Calculate how many terrain blocks we need for this terrain
    cgSize Size = mHeightMap->getSize();
    mBlockLayout.width  = ((Size.width - 1) / (mBlockSize.width - 1));   
    mBlockLayout.height = ((Size.height - 1) / (mBlockSize.height - 1));

    // Validate sizes
    if ( Size.width < mBlockSize.width || Size.height < mBlockSize.height )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Height map size is below required minimum of %i x %i. Size specified was %i x %i.\n"), mBlockSize.width, mBlockSize.height, Size.width, Size.height );
        return false;
    
    } // End if out of memory

    // Dimensions must be multiples of the number of quads required for each block, if this is not the case we 
    // cannot generate the terrain blocks correctly. The number of blocks on any given row / column must also
    // be either 1, or a power of two for the hierarchical occlusion system to function correctly.
    // ToDo: Power of two really required, or just multiples of block sizes?
    if ( ((Size.width - 1) % (mBlockSize.width - 1)) != 0 || ((Size.height - 1) % (mBlockSize.height - 1)) != 0 /* ||
         MathUtility::isPowerOfTwo( mBlockLayout.width ) == false || MathUtility::isPowerOfTwo( mBlockLayout.height ) == false */ )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Height map uses dimensions which are not compatible with block sizes of %i x %i. Size specified was %i x %i.\n"), mBlockSize.width, mBlockSize.height, Size.width, Size.height );
        return false;
    
    } // End if not power of two

    // Heightmap should always be resident in sandbox mode.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        mLandscapeFlags |= cgLandscapeFlags::HeightMapResident;

    // Calculate the landscape scale details using the setDimensions function
    if ( setDimensions( Params.dimensions ) == false )
        return false;

    // If we're creating a new landscape for a serialized scene
    // in sandbox mode then insert a new entry into the database
    // prior to performing the import (so that we correctly serialize
    // other aspects of the landscape such as the blocks).
    bool bTransactionStarted = false;
    cgWorld * pWorld = mParentScene->getParentWorld();
    if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && mParentScene->getSceneId() != 0 )
    {
        // Begin a transaction.
        pWorld->beginTransaction( _T("landscapeImport") );
        bTransactionStarted = true;

        // Insert main landscape
        prepareQueries();
        mInsertLandscape.bindParameter( 1, mBlockLayout.width );
        mInsertLandscape.bindParameter( 2, mBlockLayout.height );
        mInsertLandscape.bindParameter( 3, mBlockSize.width );
        mInsertLandscape.bindParameter( 4, mBlockSize.height );
        mInsertLandscape.bindParameter( 5, mBlendMapSize.width );
        mInsertLandscape.bindParameter( 6, mBlendMapSize.height );
        mInsertLandscape.bindParameter( 7, mDimensions.x );
        mInsertLandscape.bindParameter( 8, mDimensions.y );
        mInsertLandscape.bindParameter( 9, mDimensions.z );
        mInsertLandscape.bindParameter( 10, mOffset.x );
        mInsertLandscape.bindParameter( 11, mOffset.y );
        mInsertLandscape.bindParameter( 12, mOffset.z );
        mInsertLandscape.bindParameter( 13, (cgUInt32)0 ); // ToDo: 9999 Flags?
        
        // Execute
        if ( mInsertLandscape.step( true ) == false )
        {
            cgString strError;
            mInsertLandscape.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert layout data for new landscape in scene id '0x%x'. Error: %s\n"), mParentScene->getSceneId(), strError.c_str() );
            pWorld->rollbackTransaction( _T("landscapeImport") );
            return false;
        
        } // End if failed

        // Retrieve the newly generated landscape record identifier.
        mLandscapeId = mInsertLandscape.getLastInsertId();

    } // End if serialize

    // Allocate enough space for every terrain block instance
    mTerrainBlocks.resize( mBlockLayout.width * mBlockLayout.height, CG_NULL );
    
    // Calculate the total number of LOD levels that can be generated from the specified block sizes
    nTemp = min( mBlockSize.width, mBlockSize.height ) - 1;
    for ( nLODCount = 0; nTemp >= 2 && nLODCount < MaxLandscapeLOD; nTemp >>= 1 )
        nLODCount++;

    // Allocate enough LOD data entries for the maximum LOD level
    mLODData.resize( nLODCount, CG_NULL );
    for ( i = 0; i < nLODCount; ++i )
        mLODData[i] = new cgTerrainLOD();

    // Build the geo-mip optimization lookup table
    buildMipLookUp( );
    
    // Build the level of detail information
    for ( i = 0; i < nLODCount;  ++i )
    {
        if ( !buildLODLevel( i ) )
        {
            if ( bTransactionStarted )
                pWorld->rollbackTransaction( _T("landscapeImport") );
            mLandscapeId = 0;
            return false;
        
        } // End if failed

    } // Next LOD Level
    
    // Reset bounding box so we can compute actual size
    mBounds.reset();
    
    // Create terrain blocks.
    for ( z = 0; z < mBlockLayout.height; ++z )
    {
        for ( x = 0; x < mBlockLayout.width; ++x )
        {
            cgTerrainBlock::TerrainSection Section;

            // Should this block be allocated?
            if ( !Params.blockMask.empty() && !Params.blockMask[ x + z * mBlockLayout.width ] )
                continue;
            
            // Construct terrain block initialization information.
            Section.heightMapBuffer = &mHeightMap->getImageData()[0];
            Section.blockBounds.left     = x * (mBlockSize.width - 1);
            Section.blockBounds.top      = z * (mBlockSize.height - 1);
            Section.blockBounds.right    = ((x + 1) * (mBlockSize.width - 1)) + 1;
            Section.blockBounds.bottom   = ((z + 1) * (mBlockSize.height - 1)) + 1;
            Section.blockBounds.pitchX   = Size.width;
            Section.blockBounds.pitchY   = Size.height;

            // Create the block
            cgTerrainBlock * pBlock = new cgTerrainBlock( this, x + z * mBlockLayout.width, Section );
            if ( !pBlock->importBlock() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to load terrain block at location %i, %i.\n"), x, z );
                continue;
            
            } // End if failed

            // Retrieve the calculated bounding box of the newly loaded terrain block
            const cgBoundingBox & BlockBounds = pBlock->getBoundingBox();

            // Expand terrain bounding box
            mBounds.addPoint( BlockBounds.min );
            mBounds.addPoint( BlockBounds.max );

            // Store terrain block in main array
            mTerrainBlocks[ x + z * mBlockLayout.width ] = pBlock;

        } // Next Column

    } // Next Row

    // Finally, set the terrain block neighbor information for LOD testing etc.
    for ( z = 0; z < mBlockLayout.height; ++z )
    {
        for ( x = 0; x < mBlockLayout.width; ++x )
        {
            cgTerrainBlock * pBlock = mTerrainBlocks[ x + z * mBlockLayout.width ];
            if ( !pBlock ) continue;

            // Set neighbor information if we're not on a relevant edge
            if ( z > 0 )
                pBlock->setNeighbor( cgTerrainBlock::North, mTerrainBlocks[ x + (z - 1) * mBlockLayout.width ] );
            if ( x < mBlockLayout.width - 1 )
                pBlock->setNeighbor( cgTerrainBlock::East, mTerrainBlocks[ (x + 1) + z * mBlockLayout.width ] );
            if ( z < mBlockLayout.height - 1 )
                pBlock->setNeighbor( cgTerrainBlock::South, mTerrainBlocks[ x + (z + 1) * mBlockLayout.width ] );
            if ( x > 0 )
                pBlock->setNeighbor( cgTerrainBlock::West, mTerrainBlocks[ (x - 1) + z * mBlockLayout.width ] );

        } // Next Column

    } // Next Row

    // Perform post initialization tasks
    if ( !postInit() )
    {
        if ( bTransactionStarted )
            pWorld->rollbackTransaction( _T("landscapeImport") );
        mLandscapeId = 0;
        return false;
    
    } // End if failed

    // Generate initial normal map data.
    updateNormalTexture();

    // Update landscape entry?
    if ( shouldSerialize() )
    {
        prepareQueries();

        // Update the layout information in the database if necessary.
        mUpdateLandscapeLayout.bindParameter( 1, mBlockLayout.width );
        mUpdateLandscapeLayout.bindParameter( 2, mBlockLayout.height );
        mUpdateLandscapeLayout.bindParameter( 3, mBlockSize.width );
        mUpdateLandscapeLayout.bindParameter( 4, mBlockSize.height );
        mUpdateLandscapeLayout.bindParameter( 5, mDimensions.x );
        mUpdateLandscapeLayout.bindParameter( 6, mDimensions.y );
        mUpdateLandscapeLayout.bindParameter( 7, mDimensions.z );
        mUpdateLandscapeLayout.bindParameter( 8, mOffset.x );
        mUpdateLandscapeLayout.bindParameter( 9, mOffset.y );
        mUpdateLandscapeLayout.bindParameter( 10, mOffset.z );
        mUpdateLandscapeLayout.bindParameter( 11, mLandscapeId );

        // Execute
        if ( !mUpdateLandscapeLayout.step( true ) )
        {
            cgString strError;
            mUpdateLandscapeLayout.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update layout data for landscape '0x%x'. Error: %s\n"), mLandscapeId, strError.c_str() );
            pWorld->rollbackTransaction( _T("landscapeImport") );
            mLandscapeId = 0;
            return false;
        
        } // End if failed

        // Update the texture config information in the database if necessary.
        mUpdateTextureConfig.bindParameter( 1, mBlendMapSize.width );
        mUpdateTextureConfig.bindParameter( 2, mBlendMapSize.height );
        mUpdateTextureConfig.bindParameter( 3, mLandscapeId );

        // Execute
        if ( !mUpdateTextureConfig.step( true ) )
        {
            cgString strError;
            mUpdateTextureConfig.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update texture configuration data for landscape '0x%x'. Error: %s\n"), mLandscapeId, strError.c_str() );
            pWorld->rollbackTransaction( _T("landscapeImport") );
            mLandscapeId = 0;
            return false;
        
        } // End if failed

    } // End if serialize

    // If a layer initialization map was provided, build initial block texture data.
    if ( !Params.layerInit.empty() )
        initializeTextureData( Params.layerInit );

    // Commit database changed.
    if ( bTransactionStarted )
        pWorld->commitTransaction( _T("landscapeImport") );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : postInit ( ) (Protected)
/// <summary>
/// Perform common post initialization tasks including loading effect files,
/// creating required samplers and so on.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::postInit( )
{
    // Retrieve the correct vertex format used for the terrain geometry representation.
    mVertexFormat = cgVertexFormat::formatFromDeclarator(cgTerrainVertex::declarator);

    // Create the terrain normal texture.
    cgSize Size = getHeightMapSize();
    cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+ (Use GetBestFormat()?)" );
    cgResourceManager * pResources = mParentScene->getResourceManager();
    pResources->createTexture( &mNormalTexture, cgBufferType::Texture2D, Size.width - 1, Size.height - 1, 0, 1, 
                               cgBufferFormat::B8G8R8A8, cgMemoryPool::Managed, false, cgResourceFlags::ForceNew, 
                               _T("Core::Textures::TerrainNormal"), cgDebugSource() );

    // Create a sampler that will allow us to apply the normal map.
    mNormalSampler = pResources->createSampler( _T("TerrainNormalMap"), mLandscapeShader );
    mNormalSampler->setTexture( mNormalTexture );
    mNormalSampler->setAddressU( cgAddressingMode::Clamp );
    mNormalSampler->setAddressV( cgAddressingMode::Clamp );

    // Load the noise sampler used to introduce detail and to reducing tiling artefacts during texturing.
    mNoiseSampler = pResources->createSampler( _T("TerrainNoise"), mLandscapeShader );
    mNoiseSampler->loadTexture( _T("sys://Textures/DUDVFadedNoise.dds" ), 0, cgDebugSource() );
    mNoiseSampler->setMipmapLODBias( 0.0f );

    // Create the common blend map sampler that will be used to bind the
    // main, batched, blend maps to the landscape rendering effect.
    mBlendMapSampler = pResources->createSampler( _T("TerrainBlendMap"), mLandscapeShader );
    mBlendMapSampler->setAddressU( cgAddressingMode::Clamp );
    mBlendMapSampler->setAddressV( cgAddressingMode::Clamp );
    mBlendMapSampler->setMinificationFilter( cgFilterMethod::Linear );
    mBlendMapSampler->setMagnificationFilter( cgFilterMethod::Linear );
    mBlendMapSampler->setMipmapFilter( cgFilterMethod::None );

    // Construct a set of common samplers that can be used to bind layer color
    // and normal map data to the landscape rendering effect.
    for ( size_t i = 0; i < 4; ++i )
    {
        cgString strSamplerName = cgString::format( _T("TerrainLayerColor%i"), i );
        mLayerColorSamplers[i] = pResources->createSampler( strSamplerName, mLandscapeShader );
        strSamplerName = cgString::format( _T("TerrainLayerNormal%i"), i );
        mLayerNormalSamplers[i] = pResources->createSampler( strSamplerName, mLandscapeShader );
    
    } // Next Sampler

    // Generate the constant buffer(s) used to connect landscape rendering
    // parameters to the vertex / pixel shaders.
    if ( !pResources->createConstantBuffer( &mTerrainBaseDataBuffer, mLandscapeShader, _T("cbTerrainBaseData"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mTerrainLayerDataBuffer, mLandscapeShader, _T("cbTerrainLayerData"), cgDebugSource() ) ||
         !pResources->createConstantBuffer( &mTerrainProcDataBuffer, mLandscapeShader, _T("cbTerrainProcData"), cgDebugSource() ) )
        return false;

    // Create the states for the terrain rendering process.
    ///////////////////////////////////////////////
    // Terrain depth fill process.
    ///////////////////////////////////////////////
    // Depth stencil states
    cgDepthStencilStateDesc DepthStencilDesc;
    DepthStencilDesc.stencilEnable                      = true;
    DepthStencilDesc.frontFace.stencilFunction              = cgComparisonFunction::Always;
    DepthStencilDesc.frontFace.stencilPassOperation            = cgStencilOperation::Replace;
    if ( !pResources->createDepthStencilState( &mDepthFillDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;

    ///////////////////////////////////////////////
    // Terrain procedural layer rendering process.
    ///////////////////////////////////////////////
    // Blend states
    cgBlendStateDesc BlendDesc;
    BlendDesc.renderTarget[0].blendEnable               = true;
    BlendDesc.renderTarget[0].sourceBlend                  = cgBlendMode::SrcAlpha;
    BlendDesc.renderTarget[0].destinationBlend                 = cgBlendMode::InvSrcAlpha;
    if ( !pResources->createBlendState( &mProceduralBlendState, BlendDesc, 0, cgDebugSource() ) )
        return false;

    // Depth stencil states
    DepthStencilDesc                                    = cgDepthStencilStateDesc(); // Defaults
    DepthStencilDesc.depthEnable                        = true;
    DepthStencilDesc.depthWriteEnable                   = false;
    DepthStencilDesc.depthFunction                          = cgComparisonFunction::Equal;
    if ( !pResources->createDepthStencilState( &mProceduralDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;

    ///////////////////////////////////////////////
    // Terrain painted layer rendering process.
    ///////////////////////////////////////////////
    // Blend states
    BlendDesc                                           = cgBlendStateDesc();       // Defaults
    BlendDesc.renderTarget[0].blendEnable               = true;
    BlendDesc.renderTarget[0].sourceBlend                  = cgBlendMode::One;
    BlendDesc.renderTarget[0].destinationBlend                 = cgBlendMode::One;
    if ( !pResources->createBlendState( &mPaintedBlendState, BlendDesc, 0, cgDebugSource() ) )
        return false;

    // Depth stencil states
    DepthStencilDesc                                    = cgDepthStencilStateDesc(); // Defaults
    DepthStencilDesc.depthEnable                        = true;
    DepthStencilDesc.depthWriteEnable                   = false;
    DepthStencilDesc.depthFunction                          = cgComparisonFunction::Equal;
    if ( !pResources->createDepthStencilState( &mPaintedDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;

    ///////////////////////////////////////////////
    // Terrain G-Buffer post process
    ///////////////////////////////////////////////
    // Blend states
    BlendDesc                                           = cgBlendStateDesc();   // Defaults
    BlendDesc.renderTarget[0].blendEnable               = true;
    BlendDesc.renderTarget[0].sourceBlend                  = cgBlendMode::One;     // Add the source rgb to dest rgb.
    BlendDesc.renderTarget[0].destinationBlend                 = cgBlendMode::One;
    BlendDesc.renderTarget[0].separateAlphaBlendEnable  = true;
    BlendDesc.renderTarget[0].sourceBlendAlpha             = cgBlendMode::One;     // Replace the dest alpha with the source alpha.
    BlendDesc.renderTarget[0].destinationBlendAlpha            = cgBlendMode::Zero;
    if ( !pResources->createBlendState( &mPostProcessBlendState, BlendDesc, 0, cgDebugSource() ) )
        return false;
    
    // Depth stencil states
    DepthStencilDesc                                    = cgDepthStencilStateDesc(); // Defaults
    DepthStencilDesc.depthEnable                        = false;
    DepthStencilDesc.depthWriteEnable                   = false;
    DepthStencilDesc.stencilEnable                      = true;
    DepthStencilDesc.frontFace.stencilFunction          = cgComparisonFunction::Equal;
    DepthStencilDesc.stencilReadMask                    = cgStencilId::Terrain;
    if ( !pResources->createDepthStencilState( &mPostProcessDepthState, DepthStencilDesc, 0, cgDebugSource() ) )
        return false;
    
    // Generate procedural rendering batches.
    batchProceduralDraws( );

    // Select the necessary tree construction options.
    // First auto detect depth based on user's requested cell size.
    mMaxTreeDepth = 0;
    while ( (1 << mMaxTreeDepth) < mBlockLayout.width ||
            (1 << mMaxTreeDepth) < mBlockLayout.height )
        mMaxTreeDepth++;
    mYVTree = true;

    // Compile the spatial tree.
    if ( compileSpatialTree( ) == false )
        return false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : initializeTextureData ( ) (Protected)
/// <summary>
/// If a layer initialization map was provided, build initial block texture 
/// data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::initializeTextureData( const cgInt32Array & aInit )
{
    cgInt32 x, y, x2, y2, nCurrentLayer, nLayer;
    cgInt32 nIndexX, nIndexY, xStart, yStart, xEnd, yEnd, xStart2;

    // Determine the size of the supplied map.
    cgInt32 nWidth  = mBlockLayout.width * mBlendMapSize.width;
    cgInt32 nHeight = mBlockLayout.height * mBlendMapSize.height;

    // Make a backup of the initialization data. 
    // Due to the fact that block blend masks can technically overlap
    // depending on the size of their border, we may need to restore 
    // data back to the original buffer before we move on to another
    // block.
    cgInt32Array aInitData = aInit;

    // Initialize each terrain block appropriately.
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        // Any block exists here?
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock == CG_NULL )
            continue;

        // Get the rectangle that describes how this block's blend map
        // maps to the world as a whole. This tells us how this block
        // overlays the supplied initialization map.
        cgLandscapeTextureData * pData = pBlock->getTextureData();
        const cgRect & rcBlendMapArea = pData->getBlendMapArea();

        // Process each pixel in the blend map.
        yStart = max( 0, rcBlendMapArea.top );
        yEnd   = min( nHeight, rcBlendMapArea.bottom );
        xStart = max( 0, rcBlendMapArea.left );
        xEnd   = min( nWidth, rcBlendMapArea.right );
        for ( y = yStart; y < yEnd; ++y )
        {
            // For each row
            for ( x = xStart; x < xEnd; ++x )
            {
                // Skip if no init data provided
                if ( (nCurrentLayer = aInitData[x + y * nWidth]) < 0 )
                    continue;

                // Retrieve the referenced material.
                cgMaterialHandle hMaterial;
                mParentScene->getResourceManager()->getMaterialFromId( &hMaterial, nCurrentLayer );
                if ( hMaterial.isValid() == false )
                    continue;

                // Start laying down data for this layer type.
                cgLandscapePaintParams params;
                params.erase              = false;
                params.strength           = 100;
                params.enableHeightEffect = false;
                params.enableSlopeEffect  = false;
                params.innerRadius        = 0;
                params.outerRadius        = 0;
                if ( pData->beginPaint( hMaterial, params ) == false )
                    continue;

                // Find all entries in the initialization map that fall within the
                // confines of this block (including the current). If the layer index
                // matches that of the layer type we're currently processing then apply it.
                xStart2 = x - rcBlendMapArea.left;
                for ( y2 = y - rcBlendMapArea.top; y2 < rcBlendMapArea.height(); ++y2 )
                {
                    // Compute the correct offset for this row within the initialization map
                    nIndexY = y2 + rcBlendMapArea.top;

                    // Skip if out of bounds
                    if ( nIndexY >= 0 && nIndexY < nHeight )
                    {
                        for ( x2 = xStart2; x2 < rcBlendMapArea.width(); ++x2 )
                        {
                            // Compute the correct offset for this column within the initialization map
                            nIndexX = x2 + rcBlendMapArea.left;

                            // Skip if out of bounds
                            if ( nIndexX < 0 || nIndexX >= nWidth )
                                continue;
                            
                            // Skip if this is not the same type of layer.
                            nLayer = aInitData[nIndexY * nWidth + nIndexX];
                            if ( nLayer != nCurrentLayer )
                                continue;

                            // Set at this location
                            pData->setPixel( x2, y2 );
                            
                            // This pixel has been processed. Ignore it later.
                            aInitData[nIndexY * nWidth + nIndexX] = -2;

                        } // Next Column
                    
                    } // End if in bounds
                    xStart2 = 0;

                } // Next Row

                // We've finished processing this layer type for this block.
                pData->endPaint();

            } // Next Column

        } // Next Row

        // Reset any data we overwrite with our "used" code of -2.
        for ( y = yStart; y < yEnd; ++y )
        {
            // For each row
            for ( x = xStart; x < xEnd; ++x )
            {
                cgInt32 nIndex = y * nWidth + x;
                if ( aInitData[nIndex] == -2 )
                    aInitData[nIndex] = aInit[nIndex];

            } // Next Column

        } // Next Row

    } // Next Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : beginPaint()
/// <summary>
/// Called in order to enable painting for the landscape.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::beginPaint( const cgMaterialHandle & hType, const cgLandscapePaintParams & params )
{
    // Is this a no-op?
    if ( mIsPainting == true )
        return;

    // Painting is now enabled.
    mIsPainting     = true;
    mPaintLayerType = hType;
    mPaintParams     = params;
}

//-----------------------------------------------------------------------------
// Name : paint()
/// <summary>
/// Paint the selected layer at the given location.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::paint( cgFloat fXFrom, cgFloat fZFrom, cgFloat fXTo, cgFloat fZTo )
{
    cgBoundingBox BlendMap, Cursor;
    cgFloat       fCursorRadius;

    // Must begin painting.
    if ( mIsPainting == false )
        return false;

    // Retrieve properties we need to convert our cursor values into terrain space.
    // NB: Cursor radius should include the additional size necessary for the filter.
    fCursorRadius = mPaintParams.outerRadius + ((mPaintParams.outerRadius - mPaintParams.innerRadius) * 0.5f) + CGE_EPSILON_1MM;
    Cursor.min = cgVector3( min( fXTo, fXFrom ) - fCursorRadius, 0.0f, min( fZTo, fZFrom ) - fCursorRadius );
    Cursor.max = cgVector3( max( fXTo, fXFrom ) + fCursorRadius, 0.0f, max( fZTo, fZFrom ) + fCursorRadius );

    // Process all terrain blocks that have blend map areas which fall within the specified cursor area.
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock == CG_NULL )
            continue;
        cgLandscapeTextureData * pData = pBlock->getTextureData();
        if ( pData == CG_NULL )
            continue;

        // Intersected by the cursor?
        const cgRectF & BlendMapWorldArea = pData->getBlendMapWorldArea();
        BlendMap.min = cgVector3( BlendMapWorldArea.left - CGE_EPSILON_1MM, 0.0f, BlendMapWorldArea.bottom - CGE_EPSILON_1MM );
        BlendMap.max = cgVector3( BlendMapWorldArea.right + CGE_EPSILON_1MM, 0.0f, BlendMapWorldArea.top + CGE_EPSILON_1MM );
        if ( cgCollision::AABBIntersectAABB( Cursor, BlendMap, false, true, false ) == false )
            continue;

        // Block is intersected by the cursor. Begin painting onto this block if we haven't already
        if ( pData->isPainting() == false )
        {
            if ( pData->beginPaint( mPaintLayerType, mPaintParams ) == false )
                continue;

        } // End if !painting

        // Inform system that we would like to paint at the specified cursor location
        if ( fXFrom == fXTo && fZFrom == fZTo )
            pData->paint( fXTo, fZTo );
        else
            pData->paintLine( fXFrom, fZFrom, fXTo, fZTo );

    } // Next Block

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : updatePaintPreview() (Protected)
/// <summary>
/// Update textures for preview during the painting process.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::updatePaintPreview(  )
{
    // Must begin painting.
    if ( mIsPainting == false )
        return;

    // Ensure all paint maps are up to date
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock != CG_NULL )
        {
            cgLandscapeTextureData * pData = pBlock->getTextureData();
            if ( pData != CG_NULL && pData->isPainting() == true )
                pData->updatePaintPreview();

        } // End if valid block
    
    } // Next Block
}

//-----------------------------------------------------------------------------
// Name : endPaint()
/// <summary>
/// Called when layer updates have fully completed (i.e. user let go of the
/// mouse button).
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::endPaint( )
{
    // Must begin painting.
    if ( mIsPainting == false )
        return;

    // Ensure all paint maps are up to date
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock != CG_NULL )
        {
            cgLandscapeTextureData * pData = pBlock->getTextureData();
            if ( pData != CG_NULL && pData->isPainting() == true )
                pData->updatePaintPreview();

        } // End if valid block
    
    } // Next Block

    // Finish painting
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock != CG_NULL )
        {
            cgLandscapeTextureData * pData = pBlock->getTextureData();
            if ( pData != CG_NULL && pData->isPainting() == true )
                pData->endPaint();

        } // End if valid block
    
    } // Next Block

    // No longer painting
    mIsPainting = false;
}

//-----------------------------------------------------------------------------
// Name : compileSpatialTree ( )
/// <summary>
/// Compile the spatial tree based on the specified configuration and data
/// added to this tree object (geometry, construction data etc.)
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::compileSpatialTree( )
{
    cgBoundingBox RootAABB;

    // Calculate initial tree construction bounding box (this must be perfectly accurate)
    // Tree size (in terms of blocks) must be a power of two even if the level itself is not.
    cgDouble fTreeWidth  = (mDimensions.x / (cgDouble)mBlockLayout.width) * (cgDouble)cgMathUtility::nextPowerOfTwo( mBlockLayout.width );
    cgDouble fTreeHeight = (mDimensions.z / (cgDouble)mBlockLayout.height) * (cgDouble)cgMathUtility::nextPowerOfTwo( mBlockLayout.height );

    // ToDo: 9999 This doesn't seem like it is actually necessary at all, and will actually
    // cause some problems if we do so. For now it is commented out such that the bounding
    // box is always positioned at the upper left (<0,0>) of the landscape
    /*// Compute the correct offset for the tree in order to centralize it over the world.
    // Special care needs to be taken to correctly snap to generated terrain blocks in
    // cases where the block count is odd.
    cgFloat fTreeOffsetX = mOffset.x;
    cgFloat fTreeOffsetZ = mOffset.z;
    if ( cgMathUtility::IsEven( mBlockLayout.width ) == false )
        fTreeOffsetX += (cgFloat)(mBlockSize.width-1) * mScale.x * 0.5f;
    if ( cgMathUtility::IsEven( mBlockLayout.height ) == false )
        fTreeOffsetZ += (cgFloat)(mBlockSize.height-1) * mScale.z * 0.5f;*/
    
    // Build AABB
    cgFloat fTreeOffsetX = mOffset.x;
    cgFloat fTreeOffsetZ = mOffset.z;
    RootAABB.min = cgVector3( fTreeOffsetX, mBounds.min.y, (cgFloat)(-fTreeHeight) + fTreeOffsetZ );
    RootAABB.max = cgVector3( (cgFloat)fTreeWidth + fTreeOffsetX, mBounds.max.y, fTreeOffsetZ );
    
    // Verify that the bounding box has any size at all
    if ( RootAABB.isDegenerate() == true )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Unable to compile landscape spatial tree because the root bounding box was found to be degenerate.\r\n") );
        return false;
    
    } // End if degenerate root node

    /*// Since the /scene graph/ node(s) will ultimately store the leaves that 
    // we construct during this process such that instancing/referencing can 
    // continue to function without us needing to duplicate all other spatial 
    // tree elements, our first job is to find all referencing nodes and clear
    // out any leaves that they contain.
    cgReference::ReferenceMap::iterator itTree;
    for ( itTree = m_ReferencedBy->begin(); itTree != m_ReferencedBy->end(); ++itTree )
    {
        cgReference * pReference = (itTree->second).pReference;
        if ( pReference != CG_NULL && pReference->QueryReferenceType( RTID_LandscapeNode ) == true )
        {
            cgLandscapeNode * pTree = (cgLandscapeNode*)pReference;
            pTree->ClearTreeData();
        
        } // Next LandscapeNode
    
    } // Next Reference Holder*/
    
    // Allocate the root node
    mRootNode = allocateNode();

    // Build the spatial tree.
    if ( buildTree( 0, (cgLandscapeSubNode*)mRootNode, RootAABB ) == false )
        return false;

    // Allocate the horizon buffer used as the means to provide occlusion culling.
    // ToDo: Use configurable buffer?
    mHorizonBuffer.resize( 2048 );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildTree () (Protected, Recursive)
/// <summary>
/// Performs the recursive build behavior for this tree type.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::buildTree( cgUInt32 nLevel, cgLandscapeSubNode * pNode, const cgBoundingBox & NodeAABB )
{
    bool bStopCode = false;

    // ToDo: 9999 - Remove this node counter tracking
    static cgInt32 nTotalNodes = 0;
    if ( pNode == mRootNode )
        nTotalNodes = 1;

    // Store the initial position / size properties of the node.
    pNode->setBoundingBox( NodeAABB );
    pNode->setNodeDepth( nLevel );

    // Determine if we should stop building here
    if ( mMaxTreeDepth >= 0 )
        bStopCode |= (nLevel == mMaxTreeDepth);

    // If we reached our stop limit, build a leaf here
    if ( bStopCode == true )
    {
        // Compute the index of the terrain block in which this leaf exists
        cgInt32 nDataGroupId = -1;
        cgVector2 vBlock( (NodeAABB.min.x - mOffset.x) + CGE_EPSILON_1CM, -((NodeAABB.min.z - mOffset.z) + CGE_EPSILON_1CM) );
        vBlock.x /= (mDimensions.x / (cgFloat)mBlockLayout.width);
        vBlock.y /= (mDimensions.z / (cgFloat)mBlockLayout.height);

        // In bounds?
        if ( vBlock.x >= 0 && vBlock.x <= mBlockLayout.width &&
             vBlock.y >= 0 && vBlock.y <= mBlockLayout.height )
        {
            // Terrain block actually exists here?
            nDataGroupId = (cgInt32)vBlock.x + (cgInt32)vBlock.y * mBlockLayout.width;
            if ( mTerrainBlocks[nDataGroupId] == CG_NULL )
                nDataGroupId = -1;

        } // End if within bounds

        // Construct a leaf.
        cgSpatialTreeLeaf * pLeaf = allocateLeaf();
        pLeaf->setLeafIndex( (cgUInt32)mLeaves.size() );
        pLeaf->setBoundingBox( NodeAABB );
        pLeaf->setDataGroupId( nDataGroupId );

        // Insert into the leaf list
        mLeaves.push_back( pLeaf );

        // Set the new leaf index to the node.
        pNode->setLeaf( pLeaf->getLeafIndex() );

        // Notify the node that it has been fully constructed
        pNode->nodeConstructed();
        
        // We have reached a leaf, so we can stop compiling this tree branch
        return true;

    } // End if reached stop code

    // Build each of the 4 children here
    for( cgInt i = 0; i < 4; ++i )
    {
        cgBoundingBox ChildAABB = NodeAABB;
        cgVector3 MidPoint = NodeAABB.getCenter();

        // Calculate quater size child bounding box values
        switch( i )
        {
            case 0: // Behind left
                ChildAABB.max.x = MidPoint.x;
                ChildAABB.max.z = MidPoint.z;
                break;

            case 1: // InFront left
                ChildAABB.min.z = MidPoint.z;
                ChildAABB.max.x = MidPoint.x;
                break;

            case 2: // Behind Right
                ChildAABB.min.x = MidPoint.x;
                ChildAABB.max.z = MidPoint.z;
                break;

            case 3: // InFront Right
                ChildAABB.min.x = MidPoint.x;
                ChildAABB.min.z = MidPoint.z;
                break;

        } // End Child Type Switch

        // If this new child bounding box does not intersect the main
        // bounding box of the landscape then do not construct further.
        // We shrink the landscape bounding box slightly in order to ensure
        // that we don't recurse further than absolutely necessary.
        {
            cgBoundingBox WorldBounds( mBounds.min.x + CGE_EPSILON_1CM, mBounds.min.y + CGE_EPSILON_1CM, mBounds.min.z + CGE_EPSILON_1CM,
                                       mBounds.max.x - CGE_EPSILON_1CM, mBounds.max.y - CGE_EPSILON_1CM, mBounds.max.z - CGE_EPSILON_1CM );
            if ( cgCollision::AABBIntersectAABB( WorldBounds, ChildAABB, false, true, false ) == false )
                continue;

            // ToDo: Should stop building if there are no blocks here either?
        
        } // End Scope

        // Allocate child node
        pNode->setChildNode(i, allocateNode() );
        nTotalNodes++;

        // Recurse into this new node
        if ( buildTree( nLevel + 1, (cgLandscapeSubNode*)pNode->getChildNode(i), ChildAABB ) == false )
            return false;

    } // Next Child

    // Notify the node that it has been fully constructed
    pNode->nodeConstructed();

    if ( pNode == mRootNode )
        cgAppLog::write( _T("%i Total nodes constructed\r\n"), nTotalNodes );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : allocateNode () (Virtual)
/// <summary>
/// Default behavior for allocating new nodes. Can be overriden to return an 
/// alternate node type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeSubNode * cgLandscape::allocateNode( cgSpatialTreeSubNode * pInit /* = CG_NULL */ )
{
    if ( pInit == CG_NULL )
        return new cgLandscapeSubNode( this );
    else
        return new cgLandscapeSubNode( pInit, this );
}

//-----------------------------------------------------------------------------
// Name : allocateLeaf () (Virtual)
/// <summary>
/// Default behavior for allocating new leaves. Can be overriden to return an 
/// alternate leaf type.
/// </summary>
//-----------------------------------------------------------------------------
cgSpatialTreeLeaf * cgLandscape::allocateLeaf( cgSpatialTreeLeaf * pInit /* = CG_NULL */ )
{
    if ( pInit == CG_NULL )
        return new cgLandscapeCell( );
    else
        return new cgLandscapeCell( pInit, this );
}

//-----------------------------------------------------------------------------
// Name : getHeightMapNormal ()
/// <summary>
/// Retrieves the normal at this position in the heightmap.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgLandscape::getHeightMapNormal( cgInt32 x, cgInt32 z ) const
{
	cgVector3 vecNormal, vecEdge1, vecEdge2, vecSum( 0, 0, 0 );
    cgFloat     yNorth, yEast, ySouth, yWest, yCenter, fCount = 0.0f;

    // Retrieve the correct index into the heightmap
    const cgSize & Size = mHeightMap->getSize();
    cgInt32 nHMIndex = x + z * Size.width;

    // Get the various height values
    cgInt16 * pHeightMap = &mHeightMap->getImageData()[nHMIndex];
    yCenter = (cgFloat)*pHeightMap * mScale.y;
    if ( z > 0 )
        yNorth  = (cgFloat)*(pHeightMap - Size.width) * mScale.y;
    if ( x < Size.width - 1 )
        yEast   = (cgFloat)*(pHeightMap + 1) * mScale.y;
    if ( z < Size.height - 1 )
        ySouth  = (cgFloat)*(pHeightMap + Size.width) * mScale.y;
    if ( x > 0 )
        yWest   = (cgFloat)*(pHeightMap - 1) * mScale.y;

    // Top Right Quadrant
    if ( x < Size.width - 1 && z > 0 )
    {
	    // Compute normal
	    vecEdge1 = cgVector3( 0.0f, yNorth - yCenter, mScale.z );
	    vecEdge2 = cgVector3( mScale.x, yEast - yCenter, 0.0f );
        cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Bottom Right Quadrant
    if ( x < Size.width - 1 && z < Size.height - 1 )
    {
        // Compute normal
	    vecEdge1 = cgVector3( mScale.x, yEast - yCenter, 0.0f );
        vecEdge2 = cgVector3( 0.0f, ySouth - yCenter, -mScale.z );
        cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Bottom left Quadrant
    if ( x > 0 && z < Size.height - 1 )
    {
        // Compute normal
	    vecEdge1 = cgVector3( 0.0f, ySouth - yCenter, -mScale.z );
	    vecEdge2 = cgVector3( -mScale.x, yWest - yCenter, 0.0f );
	    cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Top left Quadrant
    if ( x > 0 && z > 0 )
    {
        // Compute normal
        vecEdge1 = cgVector3( -mScale.x, yWest - yCenter, 0.0f );
	    vecEdge2 = cgVector3( 0.0f, yNorth - yCenter, mScale.z );
	    cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds
	
    // Valid?
    if ( fCount == 0.0f ) 
        return cgVector3( 0.0f, 1.0f, 0.0f );

    // Return the averaged normal
	return cgVector3(vecSum / fCount);
}

//-----------------------------------------------------------------------------
// Name : setDimensions ()
/// <summary>
/// Set the details describing the dimensions of the terrain. This function
/// will recalculate the scale values and as a result the terrain may need to 
/// be rebuilt.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::setDimensions( const cgVector3 &  Dimensions )
{
    return setDimensions( Dimensions, false );
}
bool cgLandscape::setDimensions( const cgVector3 & Dimensions, bool bRebuildTerrain )
{
    // Store the overall dimensions.
    mDimensions = Dimensions;

    // Calculate the new scale factor based on the dimensions specified
    cgSize HeightMapSize( mBlockLayout.width * (mBlockSize.width - 1) + 1, mBlockLayout.height * (mBlockSize.height - 1) + 1 );
    mScale.x = Dimensions.x / (cgFloat)(HeightMapSize.width-1);
    mScale.y = Dimensions.y / (cgFloat)(cgHeightMap::MaxCellHeight - cgHeightMap::MinCellHeight);
    mScale.z = Dimensions.z / (cgFloat)(HeightMapSize.height-1);

    // Rebuild all the terrain blocks?
    if ( bRebuildTerrain && !mTerrainBlocks.empty() )
    {
        // Loop through all loaded terrain blocks
        for ( cgInt32 i = 0; i < mBlockLayout.width * mBlockLayout.height; ++i )
        {
            cgTerrainBlock * pBlock = mTerrainBlocks[i];
            
            // Trigger the update if a block was available here
            if ( pBlock && !pBlock->dataUpdated() )
                return false;

        } // Next terrain block

    } // End if rebuild

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : updateNormalTexture () (Protected)
/// <summary>
/// Called in order to update the normal texture map necessary for shader based
/// procedural terrain texture mapping.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::updateNormalTexture()
{
    if ( mHeightMap == CG_NULL )
        return false;

    // Update entire texture
    return updateNormalTexture( cgRect( 0, 0, mHeightMap->getSize().width, mHeightMap->getSize().height ) );
}
bool cgLandscape::updateNormalTexture( cgRect rcUpdate )
{
    cgVector3 vecNormal, vecEdge1, vecEdge2, vecSum( 0, 0, 0 );
    cgFloat   yNorth, yEast, ySouth, yWest, yCenter, fCount = 0.0f;
    cgInt32   x, y, nWidth = mHeightMap->getSize().width, nHeight = mHeightMap->getSize().height;
    cgInt32   nTextureWidth = nWidth - 1, nTextureHeight = nHeight - 1;
    cgUInt32  nPitch, nHMIndex, * pBuffer, nNormal;

    // Clamp to maximum size of texture
    rcUpdate = cgRect::intersect( rcUpdate, cgRect( 0, 0, nTextureWidth, nTextureHeight ) );

    // Lock the normal texture
    cgTexture * pTexture = mNormalTexture.getResource(true);
    if ( pTexture == CG_NULL )
        return false;
    pBuffer = (cgUInt32*)pTexture->lock( rcUpdate, nPitch, cgLockFlags::WriteOnly );
    nPitch /= sizeof(cgUInt32);
    
    // Lock and fill the texture based on terrain data.
    const cgInt16 * pHeightMap = &mHeightMap->getImageData()[0];
    for ( y = rcUpdate.top; y < rcUpdate.bottom; ++y )
    {
        for ( x = rcUpdate.left; x < rcUpdate.right; ++x )
        {
            // Reset average weights
            vecSum = cgVector3(0,0,0);
            fCount = 0.0f;

            // Get the various height values
            nHMIndex = x + y * nWidth;
            yCenter = (cgFloat)pHeightMap[ nHMIndex ] * mScale.y;
            if ( y > 0           ) yNorth  = (cgFloat)pHeightMap[ nHMIndex - nWidth ] * mScale.y;
            if ( x < nWidth - 1  ) yEast   = (cgFloat)pHeightMap[ nHMIndex + 1 ] * mScale.y;
            if ( y < nHeight - 1 ) ySouth  = (cgFloat)pHeightMap[ nHMIndex + nWidth ] * mScale.y;
            if ( x > 0           ) yWest   = (cgFloat)pHeightMap[ nHMIndex - 1 ] * mScale.y;

            // Top Right Quadrant
            if ( x < nWidth - 1 && y > 0 )
            {
	            // Compute normal
	            vecEdge1 = cgVector3( 0.0f, yNorth - yCenter, mScale.z );
	            vecEdge2 = cgVector3( mScale.x, yEast - yCenter, 0.0f );
                cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
                vecSum += vecNormal;
                fCount += 1.0f;
            
            } // End if in bounds

            // Bottom Right Quadrant
            if ( x < nWidth - 1 && y < nHeight - 1 )
            {
                // Compute normal
	            vecEdge1 = cgVector3( mScale.x, yEast - yCenter, 0.0f );
                vecEdge2 = cgVector3( 0.0f, ySouth - yCenter, -mScale.z );
                cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
                vecSum += vecNormal;
                fCount += 1.0f;
            
            } // End if in bounds

            // Bottom left Quadrant
            if ( x > 0 && y < nHeight - 1 )
            {
                // Compute normal
	            vecEdge1 = cgVector3( 0.0f, ySouth - yCenter, -mScale.z );
	            vecEdge2 = cgVector3( -mScale.x, yWest - yCenter, 0.0f );
	            cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
                vecSum += vecNormal;
                fCount += 1.0f;
            
            } // End if in bounds

            // Top left Quadrant
            if ( x > 0 && y > 0 )
            {
                // Compute normal
                vecEdge1 = cgVector3( -mScale.x, yWest - yCenter, 0.0f );
	            vecEdge2 = cgVector3( 0.0f, yNorth - yCenter, mScale.z );
	            cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
                vecSum += vecNormal;
                fCount += 1.0f;
            
            } // End if in bounds
        	
            // Average normal
            if ( fCount == 0.0f ) 
                vecSum = cgVector3( 0.0f, 1.0f, 0.0f );
            else
                vecSum /= fCount;

            // Transform to unsigned format.
            vecSum = (vecSum * 0.5f) + cgVector3(0.5f, 0.5f, 0.5f);

            // Encode normal (using cgColorValue helpers)
            nNormal = cgColorValue( vecSum.x, vecSum.y, vecSum.z, 1.0f );
            
            // Store
            *pBuffer++ = nNormal;
        }

        // Move down to next row
        pBuffer += nPitch - rcUpdate.width();

    } // Next Row
    pTexture->unlock( false );

    // Data is restored.
    pTexture->setResourceLost( false );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildMipLookUp () (Protected)
/// <summary>
/// Builds a look up table which provides a best ordering for the vertex buffer
/// such that as few vertices are used as possible when rendering specific geo 
/// mip levels of detail with software transformations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::buildMipLookUp( )
{
    // In order to ensure that rendering the block with differing levels of detail in software is efficient,
    // it is important that only the specific vertices we need are ever used during the rendering process.
    // As a result, we will order our vertex buffer such that the vertices are grouped together into
    // areas specific to each LOD (i.e. lowest LOD might simply reference the four corner vertices,
    // so these 4 corner vertices will be placed first in the VB). Here we generate a simple look up
    // table which indicates this ordering, allowing us to rapidly build and reference the VB in the correct order.

    cgInt32 i, Counter = 0, x, z, StepCount;

    // Validate requirements
    if ( mLODData.size() < 1 || mBlockSize.width < 2 || mBlockSize.height < 2 )
        return false;

    // Allocate the array
    cgInt32 nBlockElements = mBlockSize.width * mBlockSize.height;
    mMipLookUp.resize( nBlockElements );

    // Clear out the look up table to a recongizable default value
    for ( i = 0; i < nBlockElements; ++i )
        mMipLookUp[ i ] = -1;

    // Loop through each mip level from lowest to highest
    for ( i = (cgInt32)mLODData.size() - 1; i >= 0; --i )
    {
        // Generate altered details based on level
        StepCount = 1 << i;

        // Generate entries
        for ( z = 0; z < mBlockSize.height; z += StepCount )
        {
            for ( x = 0; x < mBlockSize.width; x+= StepCount )
            {
                // Generate entry
                if ( mMipLookUp[ x + z * mBlockSize.width ] < 0 ) 
                    mMipLookUp[ x + z * mBlockSize.width ] = Counter++;

            } // Next Column

        } // Next Row

    } // Next Mip Level

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildLODLevel () (Private)
// Desc : Pre-builds all of the required information for rendering the terrain
//        block at the specified detail level. This includes the primary index
//        buffer, as well as the skirts.
//-----------------------------------------------------------------------------
bool cgLandscape::buildLODLevel( cgUInt32 nMipLevel )
{
    cgInt32 pVBIndices[3];
    
    // Get access to required systems
    cgResourceManager * pResources = mParentScene->getResourceManager();

    // Calculate altered details based on mip level
    cgInt32 nStepCount = 1 << nMipLevel;
    cgInt32 nQuadsWide = (mBlockSize.width - 1) >> nMipLevel;
    cgInt32 nQuadsHigh = (mBlockSize.height - 1) >> nMipLevel;
    cgInt32 nRowStep   = mBlockSize.width * nStepCount;

    // Requires a 32 bit index buffer?
    bool b32BitIndices = false;
    if ( (nQuadsWide + 1) * (nQuadsHigh+1) > 65536 )
        b32BitIndices = true;

    // Retrieve the correct LOD data item for easy access
    cgTerrainLOD  * pLODData = mLODData[ nMipLevel ];

    // Calculate total number of primitives for the interior of the block
    pLODData->interiorPrimitiveCount = ((nQuadsWide - 2) * (nQuadsHigh - 2)) * 2;
    
    // Increase index buffer length to store these primitives
    cgInt32 nBufferLength = pLODData->interiorPrimitiveCount * 3;

    // Allocate the total number of required connection LODs in each skirt
    for ( cgInt i = 0; i < 4; ++i )
        pLODData->skirts[i].levels.resize( mLODData.size() - nMipLevel );

    // Now calculate the skirt index buffer offsets and lengths for each LOD
    cgInt32 nIndex = nBufferLength;
    for ( cgInt32 i = 0; i < ((cgInt32)mLODData.size() - (cgInt32)nMipLevel); ++i )
    {
        // Loop through each edge skirt
        for ( cgInt32 j = 0; j < 4; ++j )
        {
            // Retrieve the skirt LOD connection item for easy access
            cgTerrainLOD::BlockSkirt::LODLevel & LODConnect = pLODData->skirts[j].levels[i];
            
            // Compute the number of connecting edges in the current, and the LOD we're calculating
            cgInt32 nCurrentLODEdges, nConnectLODEdges;
            switch ( j )
            {
                case cgTerrainBlock::North:
                case cgTerrainBlock::South:
                    nCurrentLODEdges = nQuadsWide;
                    nConnectLODEdges = nQuadsWide >> i;
                    break;

                case cgTerrainBlock::East:
                case cgTerrainBlock::West:
                    nCurrentLODEdges = nQuadsHigh;
                    nConnectLODEdges = nQuadsHigh >> i;
                    break;

            } // End edge type switch
            
            // Compute the primitive count and beginning index
            // Number of triangles in a FULL skirt is the sum of each of the edges in the two rows
            // of vertices. Because the skirts are mitered so they fit together around the interior
            // we need to remove the two outermost triangles so we subtract 2 from this resulting value.
            LODConnect.primitiveCount = (nCurrentLODEdges + nConnectLODEdges) - 2;
            LODConnect.indexStart     = nIndex;
            nIndex                    += LODConnect.primitiveCount * 3;

            // Increase index buffer length to store these primitives
            nBufferLength += LODConnect.primitiveCount * 3;

        } // Next Edge

    } // Next LOD

    // Convert index buffer length into number of bytes ready for use
    nBufferLength *= (b32BitIndices == true) ? sizeof(cgUInt32) : sizeof(cgUInt16);

    // Allocate the index buffer
    pResources->createIndexBuffer( &pLODData->indexBuffer, nBufferLength, cgBufferUsage::WriteOnly, 
                                  (b32BitIndices) ? cgBufferFormat::Index32 : cgBufferFormat::Index16, 
                                  cgMemoryPool::Managed, cgDebugSource() );
    if ( pLODData->indexBuffer.isValid() == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to allocate terrain block index buffer of size %i bytes.\n"), nBufferLength );
        return false;

    } // End if failed to allocate IB
    
    // Loop through and generate the IB data for the interior portion of the block
    nIndex = 0;
    cgByte * pIndices = new cgByte[ nBufferLength ];
    for ( cgInt32 z = nStepCount; z < (mBlockSize.height - nStepCount) - 1; z+= nStepCount )
    {
        for ( cgInt32 x = nStepCount; x < (mBlockSize.width - nStepCount) - 1; x+= nStepCount )
        {
            // Calculate the vertex buffer index
            cgInt32 nVBIndex = x + z * mBlockSize.width;

            // Insert next two triangles using the mip lookup table
            if ( !b32BitIndices )
            {
                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex ];
                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex + nStepCount + nRowStep ];
                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex + nRowStep ];

                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex ];
                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex + nStepCount ];
                ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ nVBIndex + nStepCount + nRowStep ];
            
            } // End if 16 bit
            else 
            {
                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex ];
                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex + nStepCount + nRowStep ];
                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex + nRowStep ];

                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex ];
                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex + nStepCount ];
                ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ nVBIndex + nStepCount + nRowStep ];
            
            } // End if 32 bit

        } // Next Column

    } // Next Row

    // Calculate the skirt indices
    for ( cgInt32 i = 0; i < ((cgInt32)mLODData.size() - (cgInt32)nMipLevel); ++i  )
    {
        // Loop through each edge skirt
        for ( cgInt32 j = 0; j < 4; ++j )
        {
            // Retrieve the skirt item for easy access
            cgTerrainLOD::BlockSkirt::LODLevel & LODConnect = pLODData->skirts[j].levels[i];

            // Compute required values
            cgInt32 nConnectStepCount = nStepCount << i;
            nIndex = LODConnect.indexStart;
            
            // Determine the total pitch for the edge we're processing
            cgInt32 nPitch;
            if ( j == cgTerrainBlock::North || j == cgTerrainBlock::South )
                nPitch = mBlockSize.width;
            else
                nPitch = mBlockSize.height;
            
            // Is this the same LOD as the one we're building?
            if ( i == 0 )
            {
                // Highest LOD skirts needs to be built using a slightly different process
                cgInt32 nCounter = 0;
                for ( cgInt32 x = 0; x < nPitch - 1; x += nStepCount, ++nCounter )
                {
                    // Compute Indices
                    switch ( j )
                    {
                        case cgTerrainBlock::North:
                            pVBIndices[0] = x;
                            pVBIndices[1] = x + nStepCount;
                            pVBIndices[2] = x + ((nCounter %2 == 0) ? nStepCount : 0) + nRowStep;
                            break;

                        case cgTerrainBlock::East:
                            pVBIndices[0] = (mBlockSize.width - 1) + x * mBlockSize.width;
                            pVBIndices[1] = (mBlockSize.width - 1) + (x + nStepCount) * mBlockSize.width;
                            pVBIndices[2] = ((mBlockSize.width - nStepCount) - 1) + (x + ((nCounter %2 == 0) ? nStepCount : 0)) * mBlockSize.width;
                            break;

                        case cgTerrainBlock::South:
                            pVBIndices[0] = (x + nStepCount) + (mBlockSize.height - 1) * mBlockSize.width;
                            pVBIndices[1] = x + (mBlockSize.height - 1) * mBlockSize.width;
                            pVBIndices[2] = (x + ((nCounter %2 == 0) ? nStepCount : 0)) + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;
                            break;

                        case cgTerrainBlock::West:
                            pVBIndices[0] = (x + nStepCount) * mBlockSize.width;
                            pVBIndices[1] = x * mBlockSize.width;
                            pVBIndices[2] = nStepCount + (x + ((nCounter %2 == 0) ? nStepCount : 0)) * mBlockSize.width;
                            break;

                    } // End edge type switch

                    // Insert first triangle using the mip lookup table
                    if ( !b32BitIndices )
                    {
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[2] ];
                    
                    } // End if 16 bit
                    else
                    {
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[2] ];

                    } // End if 32 bit

                    // Skip first and last triangle at the bottom (to give us the mitered skirt sides)
                    if ( x <= 0 || x >= (nPitch - nStepCount) - 1 )
                        continue;

                    // Compute Indices
                    switch ( j )
                    {
                        case cgTerrainBlock::North:
                            pVBIndices[0] = x + nRowStep;
                            pVBIndices[1] = x + ((nCounter %2 == 0) ? 0 : nStepCount );
                            pVBIndices[2] = x + nStepCount + nRowStep;
                            break;

                        case cgTerrainBlock::East:
                            pVBIndices[0] = ((mBlockSize.width - nStepCount) - 1) + x * mBlockSize.width;
                            pVBIndices[1] = (mBlockSize.width - 1) + (x + ((nCounter %2 == 0) ? 0 : nStepCount )) * mBlockSize.width;
                            pVBIndices[2] = ((mBlockSize.width - nStepCount) - 1) + (x + nStepCount) * mBlockSize.width;
                            break;

                        case cgTerrainBlock::South:
                            pVBIndices[0] = (x + nStepCount) + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;
                            pVBIndices[1] = (x + ((nCounter %2 == 0) ? 0 : nStepCount )) + (mBlockSize.height - 1) * mBlockSize.width;
                            pVBIndices[2] = x + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;
                            break;

                        case cgTerrainBlock::West:
                            pVBIndices[0] = nStepCount + (x + nStepCount) * mBlockSize.width;
                            pVBIndices[1] = (x + ((nCounter %2 == 0) ? 0 : nStepCount )) * mBlockSize.width;
                            pVBIndices[2] = nStepCount + x * mBlockSize.width;
                            break;

                    } // End edge type switch

                    // Insert second triangle using the mip lookup table
                    if ( !b32BitIndices )
                    {
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[2] ];
                    
                    } // End if 16 bit
                    else
                    {
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[2] ];

                    } // End if 32 bit
                    
                } // Next connect LOD vertex

            } // End if top level LOD
            else
            {
                // Loop through lower resolution edge
                for ( cgInt32 x = 0; x < nPitch - 1; x += nConnectStepCount )
                {
                    // Compute Indices
                    switch ( j )
                    {
                        case cgTerrainBlock::North:
                            pVBIndices[0] = x;
                            pVBIndices[1] = x + nConnectStepCount;
                            pVBIndices[2] = x + (nConnectStepCount / 2) + nRowStep;
                            break;

                        case cgTerrainBlock::East:
                            pVBIndices[0] = (mBlockSize.width - 1) + x * mBlockSize.width;
                            pVBIndices[1] = (mBlockSize.width - 1) + (x + nConnectStepCount) * mBlockSize.width;
                            pVBIndices[2] = ((mBlockSize.width - nStepCount) - 1) + (x + (nConnectStepCount / 2)) * mBlockSize.width;
                            break;

                        case cgTerrainBlock::South:
                            pVBIndices[0] = x + (mBlockSize.height - 1) * mBlockSize.width;;
                            pVBIndices[1] = (x + (nConnectStepCount / 2)) + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;;
                            pVBIndices[2] = (x + nConnectStepCount) + (mBlockSize.height - 1) * mBlockSize.width;;

                            break;

                        case cgTerrainBlock::West:
                            pVBIndices[0] = (x + nConnectStepCount) * mBlockSize.width;
                            pVBIndices[1] = x * mBlockSize.width;
                            pVBIndices[2] = nStepCount + (x + (nConnectStepCount / 2)) * mBlockSize.width;
                            break;

                    } // End edge type switch

                    // Insert triangle using the mip lookup table
                    if ( !b32BitIndices )
                    {
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[2] ];
                    
                    } // End if 16 bit
                    else
                    {
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[0] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[1] ];
                        ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[2] ];

                    } // End if 32 bit

                    // Connect to each vertex of the high resolution edge
                    // that fall within the 'interior' of the lower resolution edge.
                    for ( cgInt32 k = x; k < x + nConnectStepCount; k += nStepCount )
                    {
                        // Skip first and last triangle
                        if ( k <= 0 || k >= (nPitch - nStepCount) - 1 )
                            continue;

                        // Calculate the delta for the lower LOD edge (used to select the best
                        // point in the low resolution edge to connect this triangle to).
                        cgFloat fDelta = (cgFloat)(k - x) / (cgFloat)nConnectStepCount;

                        // Compute Indices
                        switch ( j )
                        {
                            case cgTerrainBlock::North:
                                pVBIndices[0] = x + ((int)(fDelta + 0.5f) * nConnectStepCount);
                                pVBIndices[1] = k + nStepCount + nRowStep;
                                pVBIndices[2] = k + nRowStep;
                                break;

                            case cgTerrainBlock::East:
                                pVBIndices[0] = (mBlockSize.width - 1) + (x + ((int)(fDelta + 0.5f) * nConnectStepCount)) * mBlockSize.width;
                                pVBIndices[1] = ((mBlockSize.width - nStepCount) - 1) + (k + nStepCount) * mBlockSize.width;
                                pVBIndices[2] = ((mBlockSize.width - nStepCount) - 1) + k * mBlockSize.width;
                                break;

                            case cgTerrainBlock::South:
                                pVBIndices[0] = (x + ((int)(fDelta + 0.5f) * nConnectStepCount)) + (mBlockSize.height - 1) * mBlockSize.width;
                                pVBIndices[1] = k + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;
                                pVBIndices[2] = (k + nStepCount) + ((mBlockSize.height - nStepCount) - 1) * mBlockSize.width;
                                break;

                            case cgTerrainBlock::West:
                                pVBIndices[0] = (x + ((int)(fDelta + 0.5f) * nConnectStepCount)) * mBlockSize.width;
                                pVBIndices[1] = nStepCount + k * mBlockSize.width;
                                pVBIndices[2] = nStepCount + (k + nStepCount) * mBlockSize.width;
                                break;

                        } // End edge type switch

                        // Insert triangle using the mip lookup table
                        if ( b32BitIndices == false )
                        {
                            ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[0] ];
                            ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[1] ];
                            ((cgUInt16*)pIndices)[ nIndex++ ] = (cgUInt16)mMipLookUp[ pVBIndices[2] ];
                        
                        } // End if 16 bit
                        else
                        {
                            ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[0] ];
                            ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[1] ];
                            ((cgUInt32*)pIndices)[ nIndex++ ] = (cgUInt32)mMipLookUp[ pVBIndices[2] ];

                        } // End if 32 bit
                        
                    } // Next current LOD vertex
             
                } // Next connect LOD vertex

            } // End if LOD level lower than current

        } // Next Edge

    } // Next LOD

    // Update the hardware index buffer
    bool bResult = true;
    cgIndexBuffer * pBuffer = pLODData->indexBuffer.getResource(true);
    if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, pIndices ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to update terrain block index buffer.\n") );
        bResult = false;
    
    } // End if failed to lock

    // Clean up and return.
    delete []pIndices;
    return bResult;
}

/*//-----------------------------------------------------------------------------
//  Name : SandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::SandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // Sandbox render is only necessary in wireframe mode.
    // Standard scene rendering behavior applies in other modes.
    if ( bWireframe == false )
        return;

    // ToDo: 9999
    // Get access to required systems.
    cgRenderDriver    * pDriver    = cgRenderDriver::getInstance();
    cgResourceManager * pResources = cgResourceManager::getInstance();

    // Retrieve the underlying mesh resource if available
    cgMesh * pMesh = m_hMesh;
    if ( pMesh == CG_NULL || pMesh->IsLoaded() == false )
        return;

    // Load the sandbox rendering shader if it is not already loaded.
    if ( m_hSandboxEffect.isValid() == false )
        pResources->LoadEffectFile( &m_hSandboxEffect, _T("sys://Shaders/SandboxElements.sh"), 0, cgDebugSource() );
    
    // Use wireframe drawing for every subset using mesh color
    sgEffectFile * pEffect = m_hSandboxEffect;
    if ( pEffect == CG_NULL || pEffect->IsLoaded() == false )
        return;
    pEffect->SetTechnique( _T("drawWireframeMesh") );

    // Select the color to render with
    cgColorValue color = (pIssuer->IsSelected() == false) ? pIssuer->GetNodeColor() : cgColorValue( 0xFFFFFFFF );
    pEffect->SetValue( _T("DiffuseReflectance"), &color, sizeof(cgColorValue) );

    // Draw the entire mesh
    pDriver->FlushMaterials();
    pDriver->SetEffect( m_hSandboxEffect );
    pDriver->setWorldTransform( &pIssuer->getWorldTransform( false ) );
    pMesh->draw( cgMeshDrawMode::EffectPasses );
}*/

//-----------------------------------------------------------------------------
//  Name : Pick ( ) (Virtual)
/// <summary>
/// Given the specified object space ray, determine if this object is 
/// intersected and also compute the object space intersection distance. 
/// </summary>
//-----------------------------------------------------------------------------
/*bool cgLandscape::Pick( cgCameraNode * pCamera, cgObjectNode * pIssuer, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, const cgVector3 & vWireTolerance, cgFloat & fDistance )
{
    // ToDo: 9999 - Ignoring wireframe for the moment.
    if ( bWireframe == true )
        return false;

    // Landscape populated?
    if ( mRootNode == CG_NULL )
        return false;

    // First, test for broadphase intersection against our bounding box
    if ( mRootNode->getBoundingBox().intersect( vOrigin, vDir, fDistance, false ) == false )
        return false;

    // Now test the physical terrain data.
    // ToDo: Not /critically/ important, but we should compute a
    // reasonable ray 'velocity' (length) rather than hardcoding.
    if ( getRayIntersect( vOrigin, vDir * 40000.0f, fDistance, 10.0f ) == false )
        return false;

    // Return final (local) distance
    fDistance = 40000.0f * fDistance;
    return true;
}*/

//-----------------------------------------------------------------------------
//  Name : renderPass ()
/// <summary>
/// Render the landscape using the requested rendering method (i.e. depth fill,
/// g-buffer fill, etc. )
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::renderPass( cgLandscapeRenderMethod::Base Pass, cgCameraNode * pCamera, cgVisibilitySet * pVisData )
{
    // Validate requirements
    cgAssert( mLandscapeShader.isValid() );
    cgAssert( mTerrainBaseDataBuffer.isValid() );
    if ( mTerrainBlocks.empty() )
        return false;

    // ToDo: Always use highest LOD for non perspective viewports?
    // ToDo: Allow user to select 2D preview mode (Full, Box, None etc.)
    // ToDo: Allow user to customize Preview detail level?
    // ToDo: This will completely change when the landscape gets the opportunity to compute LOD in advance!

    // First allow blocks to compute their required LOD
    for ( size_t i = 0; i < mTerrainBlocks.size(); ++i )
    {
        if ( mTerrainBlocks[i] != CG_NULL )
            mTerrainBlocks[i]->calculateLOD( pCamera, mTerrainDetail );
    
    } // Next Block

    // Set the format code for the terrain meshes, these will always
    // be identical between each block stored here.
    cgRenderDriver * pDriver = mParentScene->getRenderDriver();
    pDriver->setVertexFormat( mVertexFormat );

    // Clear world transform (landscape is always defined in world space)
    pDriver->setWorldTransform( CG_NULL );

    // ToDo: 9999 - If we change the visibility set to store data groups
    // rather than leaves (like Prescience did), this will no longer be
    // necessary. As another plus, the 'VisibilitySet::Apply()' concept can go away
    // allowing mesh based spatial trees to pull directly from the pre-prepared
    // (ordered) data group set.
    
    // Process all visible 'cells' in the current visibility set in order to retrieve a list
    // of blocks that should be rendered.
    // ToDo: 9999 -VISIBILITY
    /*cgInt32Set DataGroups;
    cgSceneLeafSet::const_iterator itLeaf;
    const cgSceneLeafSet & Leaves = pVisData->GetVisibleLeaves( pIssuer );
    for ( itLeaf = Leaves.begin(); itLeaf != Leaves.end(); ++itLeaf )
    {
        cgInt32 nDataGroup = (*itLeaf)->getDataGroupId();
        if ( nDataGroup >= 0 )
            DataGroups.insert( nDataGroup );

    } // Next visible leaf*/
    cgInt32Set & DataGroups = pVisData->getVisibleGroups(this);

    // Retrieve shader constant buffers ready for population.
    cgSurfaceShader * pShader = mLandscapeShader.getResource(true);
    cgConstantBuffer * pBaseBuffer = mTerrainBaseDataBuffer.getResource( true );
    cgConstantBuffer * pLayerBuffer = mTerrainLayerDataBuffer.getResource( true );
    cgConstantBuffer * pProcBuffer  = mTerrainProcDataBuffer.getResource( true );
    cgAssert( pBaseBuffer && pLayerBuffer && pProcBuffer );
    
    // First populate the base terrain data (common constants) buffer.
    cbTerrainBaseData * pBaseData = CG_NULL;
    if ( !(pBaseData = (cbTerrainBaseData*)pBaseBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock terrain base data constant buffer in preparation for device update.\n") );
        return false;

    } // End if failed

    // Set common shader constants.
    cgVector4 vecScale( mScale.x, mScale.y, mScale.z, 65535.0f * mScale.y );
    cgVector4 vecInvScale( 1.0f / vecScale.x, 1.0f / vecScale.y, 1.0f / vecScale.z, 1.0f / vecScale.w );
    cgVector4 vecSize( (cgFloat)(mBlockLayout.width * (mBlockSize.width-1)) * vecScale.x, (cgFloat)(mBlockLayout.height * (mBlockSize.height-1)) * -vecScale.z, 0.0f, 0.0f );
    vecSize.z = 1.0f / vecSize.x;
    vecSize.w = 1.0f / vecSize.y;
    pBaseData->color = cgColorValue( 1, 1, 1, 1 );
    pBaseData->terrainOffset = mOffset;
    pBaseData->terrainSize   = vecSize;
    pBaseData->noiseStrength = 0.5f;
    
    // Unlock the buffer and then bind it to the device. The appropriate constants 
    // will be automatically updated next time 'DrawPrimitive*' is called.
    pBaseBuffer->unlock();
    pDriver->setConstantBufferAuto( mTerrainBaseDataBuffer );
    
    // Render terrain in the appropriate fashion. Depth and shadow fill are relatively simple.
    if ( Pass == cgLandscapeRenderMethod::TerrainDepthFill 
        || Pass == cgLandscapeRenderMethod::TerrainShadowFill )
    {
        // Setup states based on the rendering method that was requested.
        if ( Pass == cgLandscapeRenderMethod::TerrainDepthFill )
        {
            // Draws the terrain to the depth texture.
            // First apply the depth fill states.
            pDriver->setDepthStencilState( mDepthFillDepthState, cgStencilId::Terrain );

            // Use default rasterizer and blend states.
            pDriver->setBlendState( cgBlendStateHandle::Null );
            pDriver->setRasterizerState( cgRasterizerStateHandle::Null );

            // Build vertex / pixel shader permutation arguments.
            // ToDo: This can likely be pre-created in postInit (essentially binding address 
            // of system export 'depthType' variable to the shader permutation selector).
            static cgScriptArgument::Array aArgs(3);
            cgInt32 nDepthOutputType = pDriver->getSystemState( cgSystemState::DepthType );
            cgInt32 nNormalOutputType = pDriver->getSystemState( cgSystemState::SurfaceNormalType );
            bool    bOrthographicCamera = (pDriver->getSystemState( cgSystemState::OrthographicCamera ) != 0);
            aArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nDepthOutputType );
            aArgs[1] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nNormalOutputType );
            aArgs[2] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bOrthographicCamera );

            // Select the vertex and pixel shader.
            if ( !pShader->selectVertexShader( _T("transformDepth"), aArgs ) ||
                 !pShader->selectPixelShader( _T("terrainDrawDepth"), aArgs ) )
                 return false;

        } // End if TerrainDepthFill
        else if ( Pass == cgLandscapeRenderMethod::TerrainShadowFill )
        {
            cgToDo( "Effect Overhaul", "Variable states from shadow frustum?." );
            /*// Setup biasing and culling
            DepthBias           = ( State_DepthBias );
            SlopeScaleDepthBias = ( State_SlopeScaleDepthBias );
            CullMode            = CCW; // Always use CCW mode for landscape.
            // Enable / disable color writes depending on application request.
            ColorWriteEnable    = ( State_ColorWriteEnable );*/
                    
            // Use default depth stencil and blend states.
            pDriver->setRasterizerState( cgRasterizerStateHandle::Null );
            pDriver->setDepthStencilState( cgDepthStencilStateHandle::Null );
            pDriver->setBlendState( cgBlendStateHandle::Null );

            // Select the vertex and pixel shader.
            // ToDo: This can likely be pre-created in postInit (essentially binding address 
            // of system export 'Light' variable to the shader permutation selector).
            cgScriptArgument::Array aArgs( 1 );
            cgScriptObject * pSystemExports = pDriver->getShaderInterface();
            aArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), pSystemExports->getAddressOfMember( _T("lightType") ) );
            if ( !pShader->selectVertexShader( _T("transformShadowMap"), aArgs ) ||
                 !pShader->selectPixelShader( _T("terrainDrawShadowDepth") ) )
            {
                pSystemExports->release();
                return false;
            
            } // End if failed
            pSystemExports->release();

        } // End if TerrainShadowFill

        // Render entire (visible) terrain.
        if ( DataGroups.empty() == false )
        {
            cgInt32Set::const_iterator itGroup;
            for ( itGroup = DataGroups.begin(); itGroup != DataGroups.end(); ++itGroup )
            {
                cgTerrainBlock * pBlock = mTerrainBlocks[*itGroup];
                if ( pBlock == CG_NULL ) continue;
            
                // Just render the block
                pBlock->draw( pDriver, cgTerrainBlock::Simple );

            } // Next Block

        } // End if anything visible
    
    } // End if TerrainDepthFill || TerrainShadowFill
    else if ( Pass == cgLandscapeRenderMethod::TerrainGBufferFill )
    {
        // Setup noise sampler for texturing passes
        mNoiseSampler->apply();

        // Grant shaders access to the terrain normal data.
        if ( mNormalTexture.isResourceLost() == true )
            updateNormalTexture();
        mNormalTexture.setResourceLost( false );
        mNormalSampler->apply();

        // Render each visible block using its paint map.
        if ( !DataGroups.empty() )
        {
            cgInt32Set::const_iterator itGroup;
            for ( itGroup = DataGroups.begin(); itGroup != DataGroups.end(); ++itGroup )
            {
                cgTerrainBlock * pBlock = mTerrainBlocks[*itGroup];
                if ( !pBlock ) continue;
            
                // Just render the block
                pBlock->draw( pDriver, cgTerrainBlock::Painted );

            } // Next Block

        } // End if anything visible

        /*// Any procedural batches?
        if ( !mProceduralBatches.empty() )
        {
            bool TilingReduction[3];

            // Bind the appropriate constant buffers ready for procedural rendering.
            // The hardware side constant registers in these bound buffers will be 
            // automatically updated as necessary whenever the buffer contents are 
            // modified within the following loops.
            pDriver->setConstantBufferAuto( mTerrainLayerDataBuffer );
            pDriver->setConstantBufferAuto( mTerrainProcDataBuffer );

            // Setup states required for this process (all batches share the same states).
            pDriver->setBlendState( mProceduralBlendState );
            pDriver->setDepthStencilState( mProceduralDepthState );
            pDriver->setRasterizerState( cgRasterizerStateHandle::Null );
            
            // Retrieve default diffuse / normal sampler(s) to save time during batch construction
            // in case certain layers do not utilize a particular sampler.
            cgResourceManager * pResources = pDriver->getResourceManager();
            cgSampler * pDefaultNormalSampler = pResources->getDefaultSampler( cgResourceManager::DefaultNormalSampler );
            cgSampler * pDefaultDiffuseSampler = pResources->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler );

            // ToDo: 9999 - Remove whatever we don't keep
            // Process batches in reverse order.
            //for ( cgInt32 nCurrentBatch = (cgInt32)mProceduralBatches.size() - 1; nCurrentBatch >= 0; --nCurrentBatch )
            // Process batches.
            for ( size_t nCurrentBatch = 0; nCurrentBatch < mProceduralBatches.size() - 1; ++nCurrentBatch )
            {
                ProceduralDrawBatch & Batch = mProceduralBatches[nCurrentBatch];

                // Clear tile reduction array. Must be 'false' for unused layers.
                memset( TilingReduction, 0, 3 * sizeof(bool) );

                // Lock the layer data and procedural constant buffers ready for population
                cbTerrainLayerData * pLayerData = CG_NULL;
                cbTerrainProcData  * pProcData = CG_NULL;
                if ( !(pLayerData = (cbTerrainLayerData*)pLayerBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) ||
                     !(pProcData  = (cbTerrainProcData*)pProcBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Failed to lock terrain layer and / or procedural data constant buffer in preparation for device update.\n") );
                    return false;

                } // End if failed

                // Collect together and set necessary batch data.
                for ( size_t nCurrentLayer = 0; nCurrentLayer < Batch.aLayers.size(); ++nCurrentLayer )
                {
                    ProceduralLayerRef & LayerRef = mProceduralLayers[Batch.aLayers[nCurrentLayer]];
                    cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)LayerRef.layerType.getResource();
                    
                    // Skip if this is a NULL layer
                    if ( pType == CG_NULL )
                        continue;

                    // ToDo: 9999 - This is kind of manual, vs simply being able to call 'pType->Apply()'.
                    // I'm not sure how we can make this more generic yet, but the following works for now.

                    // Copy layer color map data into the appropriate common sampler and apply.
                    cgSampler * pDstSampler = mLayerColorSamplers[nCurrentLayer];
                    cgSampler * pSrcSampler = pType->getColorSampler();
                    if ( pSrcSampler == CG_NULL || pSrcSampler->isTextureValid() == false )
                        pSrcSampler = pDefaultDiffuseSampler;
                    pDstSampler->setTexture( pSrcSampler->getTexture() );
                    pDstSampler->setStates( pSrcSampler->getStates() );
                    pDstSampler->apply();

                    // Same for layer normal map data (if one is available)
                    pDstSampler = mLayerNormalSamplers[nCurrentLayer];
                    pSrcSampler = pType->getNormalSampler();
                    if ( pSrcSampler == CG_NULL || pSrcSampler->isTextureValid() == false )
                        pSrcSampler = pDefaultNormalSampler;
                    pDstSampler->setTexture( pSrcSampler->getTexture() );
                    pDstSampler->setStates( pSrcSampler->getStates() );
                    pDstSampler->apply();

                    // ToDo: All of these can be cached on change and simply set.

                    // Collect texturing properties
                    ctLayerData & LayerData = pLayerData->layers[nCurrentLayer];
                    LayerData.scale       = pType->getScale();
                    LayerData.baseScale   = pType->getBaseScale();
                    LayerData.offset      = -pType->getOffset();
                    LayerData.rotation.x  = cosf(CGEToRadian(360.0f - pType->getAngle()));
                    LayerData.rotation.y  = -sinf(CGEToRadian(360.0f - pType->getAngle()));
                    LayerData.rotation.z  = -LayerData.rotation.y;
                    LayerData.rotation.w  = LayerData.rotation.x;

                    // Pixel shader permutation is selected based on the various
                    // combinations of tiling reduction states for each layer.
                    TilingReduction[nCurrentLayer] = pType->getTilingReduction();

                    // Collect procedural generation properties
                    ctProceduralData & ProceduralData = pProcData->Procedural[nCurrentLayer];
                    ProceduralData.Weight = LayerRef.weight;

                    // First compute height parameters
                    cgFloat maximumHeight = (LayerRef.enableHeight) ? LayerRef.maximumHeight : mBounds.max.y + 1.0f;
                    cgFloat minimumHeight = (LayerRef.enableHeight) ? LayerRef.minimumHeight : mBounds.min.y - 1.0f;
                    cgFloat fAttenBand = (LayerRef.enableHeight) ? LayerRef.heightAttenuationBand : 0.0f;
                    cgFloat fCenter    = (maximumHeight + minimumHeight) * 0.5f;
                    cgFloat fRange     = (fAttenBand + CGE_EPSILON_1MM);
                    cgFloat fMax       = (maximumHeight - fCenter) + fRange;
                    ProceduralData.HeightParams.x = fCenter;           // Center
                    ProceduralData.HeightParams.y = -(1.0f / fRange);  // Negative one over fade range (multiply instead of divide, automatically negating result)
                    ProceduralData.HeightParams.z = fMax / fRange;     // ((max-Center)+FadeRange)/FadeRange

                    // Now slope parameters 
                    // NOTE: Inverse slope scale formula (i.e. (1-Slope)*Scale+Bias) == Slope * -Scale + (Scale + Bias)
                    if ( LayerRef.invertSlope == false )
                    {
                        ProceduralData.slopeAxis     = LayerRef.slopeAxis;
                        ProceduralData.SlopeParams.x = LayerRef.slopeScale;
                        ProceduralData.SlopeParams.y = LayerRef.slopeBias;
                    
                    } // End if no inversion
                    else
                    {
                        ProceduralData.slopeAxis     = LayerRef.slopeAxis;
                        ProceduralData.SlopeParams.x = -LayerRef.slopeScale;
                        ProceduralData.SlopeParams.y = (LayerRef.slopeScale + LayerRef.slopeBias);

                    } // End if invert

                } // Next Layer

                // Unlock constant buffers. Data will be automatically transferred
                // on the next call to 'DrawPrimitive()'.
                pLayerBuffer->unlock();
                pProcBuffer->unlock();
                
                // Select the vertex shader (1 argument: layer type).
                cgScriptArgument::Array aArgs;
                aArgs.reserve( 4 );
                cgInt32 nLayerCount = (cgInt32)Batch.aLayers.size();
                aArgs.push_back( cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nLayerCount ) );
                if ( !pShader->selectVertexShader( _T("transformProcedural"), aArgs ) )
                    return false;

                // ToDo: Can we optimize by precomputing arguments / terms ?
                // Select the pixel shader (4 arguments: layer type + the 3 tiling reduction bools).
                aArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &TilingReduction[0] ) );
                aArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &TilingReduction[1] ) );
                aArgs.push_back( cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &TilingReduction[2] ) );
                if ( !pShader->selectPixelShader( _T("terrainDrawProcedural") ) )
                    return false;

                // Render each referenced block using the procedural layers
                for ( size_t i = 0; i < Batch.aBlocks.size(); i++ )
                {
                    cgTerrainBlock * pBlock = Batch.aBlocks[i];
                    if ( pBlock == CG_NULL ) continue;

                    // Skip if block is not visible
                    if ( DataGroups.find( pBlock->getBlockIndex() ) == DataGroups.end() )
                        continue;
                    
                    // Render the block
                    pBlock->draw( pDriver, cgTerrainBlock::Procedural );

                } // Next Mesh

            } // Next Batch

        } // End if any batches*/

        // Clear out common samplers to ensure their continued referencing
        // of texture data will not prevent resources from being unloaded.
        for ( size_t i = 0; i < 4; ++i )
        {
            mLayerColorSamplers[i]->setTexture( cgTextureHandle::Null );
            mLayerNormalSamplers[i]->setTexture( cgTextureHandle::Null );
        
        } // Next Sampler

    } // End if TerrainGBufferFill
    else
    {
        // Setup states required for this process.
        pDriver->setBlendState( mPostProcessBlendState );
        pDriver->setDepthStencilState( mPostProcessDepthState, cgStencilId::Terrain );
        pDriver->setRasterizerState( cgRasterizerStateHandle::Null );

        // ToDo: This can likely be pre-created in postInit (essentially binding address 
        // of system export 'depthType' variable to the shader permutation selector).
        static cgScriptArgument::Array aPSArgs(1);
        cgInt32 nShadingQuality = pDriver->getSystemState( cgSystemState::ShadingQuality );
        aPSArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &nShadingQuality );

        // Select the vertex and pixel shader.
        if ( !pShader->selectVertexShader( cgVertexShaderHandle::Null ) ||
             !pShader->selectPixelShader( _T("terrainGBufferPostProcess"), aPSArgs ) )
             return false;

        // Execute post processing shader by drawing a simple screen quad
        pDriver->drawScreenQuad();

    } // End if TerrainGBufferPost

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : batchProceduralDraws() (Protected)
/// <summary>
/// Collect together the batches of three procedural layers that we can draw
/// for each block ready for rendering.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::batchProceduralDraws( )
{
    cgBoundingBox Bounds;
    std::map< ProceduralBatchKey, cgArray<cgInt32> > ProceduralBatches;

    // Clear previous information.
    mProceduralBatches.clear();
    
    // Validate requirements
    if ( mTerrainBlocks.empty() == true || mProceduralLayers.empty() == true )
        return;
    
    // For each block, determine which (if any) procedural layers can apply.
    for ( size_t i = 0; i < mTerrainBlocks.size(); i++ )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[i];
        if ( pBlock == CG_NULL ) continue;

        // For each procedural layer
        for ( size_t nCurrentLayer = 0; nCurrentLayer < mProceduralLayers.size(); )
        {
            cgUInt32 nLayerCount = 0;
            cgUInt32  nLayers[3] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

            // We need to collect together three layers at a time.
            for ( ; nCurrentLayer < mProceduralLayers.size() && nLayerCount < 3; ++nCurrentLayer )
            {
                const ProceduralLayerRef & LayerRef = mProceduralLayers[nCurrentLayer];

                // Skip if this is a NULL layer
                if ( LayerRef.layerType.isValid() == false )
                    continue;
                
                // Can this procedural layer possibly apply here based on height?
                if ( LayerRef.enableHeight == true )
                {
                    Bounds = pBlock->getBoundingBox( );

                    // Height parameters are enabled
                    if ( (Bounds.min.y > (LayerRef.maximumHeight + LayerRef.heightAttenuationBand)) ||
                         (Bounds.max.y < (LayerRef.minimumHeight - LayerRef.heightAttenuationBand)) )
                        continue;
                
                } // End if check
                     
                //cgLandscapeLayerType * pType = m_aLayerTypes[LayerRef.nLayerType];
                // ToDo: If the procedural layer can't apply here because it is excluded
                // from this block, then skip

                // ToDo: There is no point in rendering a block procedurally if it will
                // be entirely obscured by painted layer data.

                // This is a valid procedural layer for this block.
                nLayers[nLayerCount++] = (cgInt32)nCurrentLayer;
                
            } // Next Layer

            // Find the batch and insert the block into it.
            if ( nLayerCount > 0 )
            {
                ProceduralBatchKey Key( nLayers, nLayerCount );
                ProceduralBatches[ Key ].push_back( i );
            
            } // End if layers found

        } // Next Batch

    } // Next Block

    // Allocate space for procedural block batch rendering information
    if ( ProceduralBatches.size() > 0 )
    {
        mProceduralBatches.resize( ProceduralBatches.size() );

        // Construct procedural block batch rendering information
        std::map< ProceduralBatchKey, cgArray<cgInt32> >::iterator itBatch = ProceduralBatches.begin();
        for ( cgInt32 i = 0; itBatch != ProceduralBatches.end(); ++itBatch, ++i )
        {
            const ProceduralBatchKey & Key = itBatch->first;

            // Copy layer and block data through
            mProceduralBatches[i].layers.resize( Key.nLayerCount );
            mProceduralBatches[i].blocks.resize( itBatch->second.size() );
            for ( cgUInt32 j = 0; j < Key.nLayerCount; ++j )
                mProceduralBatches[i].layers[j] = Key.nLayers[j];
            for ( size_t j = 0; j < itBatch->second.size(); ++j )
                mProceduralBatches[i].blocks[j] = mTerrainBlocks[ itBatch->second[j] ];
            
        } // Next Batch

    } // End if any batches
}

//-----------------------------------------------------------------------------
// Name : heightMapUpdated ()
/// <summary>
/// Called in order to notify us that a specific piece of the heightmap has 
/// been updated. Note : Only applies if the Dynamic flag has been used during 
/// load.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::heightMapUpdated( const cgRect &rcUpdate )
{
    cgInt nStartX, nStartZ, nEndX, nEndZ, x, z;

    // Validate requirements
    if ( (mLandscapeFlags & cgLandscapeFlags::Dynamic) != cgLandscapeFlags::Dynamic )
        return false;

    // Determine which blocks need to be notified
    nStartX = (cgInt)(((cgFloat)rcUpdate.left - 0.5f) / ((cgFloat)mBlockSize.width - 1));
    nEndX   = (cgInt)(((cgFloat)rcUpdate.right + 0.5f) / ((cgFloat)mBlockSize.width - 1));
    nStartZ = (cgInt)(((cgFloat)rcUpdate.top - 0.5f) / ((cgFloat)mBlockSize.height - 1));
    nEndZ   = (cgInt)(((cgFloat)rcUpdate.bottom  + 0.5f) / ((cgFloat)mBlockSize.height - 1));

    // Clamp values
    if ( nStartX < 0 ) nStartX = 0;
    if ( nEndX > mBlockLayout.width - 1 ) nEndX = mBlockLayout.width - 1;
    if ( nStartZ < 0 ) nStartZ = 0;
    if ( nEndZ > mBlockLayout.height - 1 ) nEndZ = mBlockLayout.height - 1;

    // Update the blocks
    for ( z = nStartZ; z <= nEndZ; ++z )
    {
        for ( x = nStartX; x <= nEndX; ++x )
        {
            cgTerrainBlock * pBlock = mTerrainBlocks[ x + z * mBlockLayout.width ];
            if ( pBlock == CG_NULL ) continue;

            // Notify
            pBlock->dataUpdated();

        } // Next Column

    } // Next Row

    // Update the normal map
    updateNormalTexture( rcUpdate );

    // Re-generate procedural rendering batches.
    batchProceduralDraws( );

    // ToDo: Update the occlusion data as necessary.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : getTerrainHeight ()
/// <summary>
/// Retrieve the height of the terrain at the specified location.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLandscape::getTerrainHeight( cgFloat fX, cgFloat fZ ) const
{
    return getTerrainHeight( fX, fZ, false );
}
cgFloat cgLandscape::getTerrainHeight( cgFloat fX, cgFloat fZ, bool bAccountForLOD ) const
{
    // Calculate block indices
    cgFloat fBlockX = ((fX - mOffset.x) / ((cgFloat)(mBlockSize.width - 1) * mScale.x));
    cgFloat fBlockY = (-(fZ - mOffset.z) / ((cgFloat)(mBlockSize.height - 1) * mScale.z));
    cgInt nBlockX = (cgInt)fBlockX;
    cgInt nBlockY = (cgInt)fBlockY;

    // Ensure this is not out of bounds
    if ( nBlockX < 0 || nBlockX >= mBlockLayout.width ) return 0.0f;
    if ( nBlockY < 0 || nBlockY >= mBlockLayout.height ) return 0.0f;

    // Retrieve the terrain block at this location
    cgTerrainBlock * pBlock = mTerrainBlocks[ nBlockX + nBlockY * mBlockLayout.width ];
    if ( pBlock == CG_NULL )
        return 0.0f;

    // Retrieve height from the block
    return pBlock->getTerrainHeight( fX, fZ, bAccountForLOD );
}

//-----------------------------------------------------------------------------
// Name : getRayIntersect ()
/// <summary>
/// Retrieve the 'time' at which a ray intersects the terrain (if at all).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::getRayIntersect( const cgVector3 & vecOrigin, const cgVector3 & vecVelocity, cgFloat & t ) const
{
    return getRayIntersect( vecOrigin, vecVelocity, t, 1.0f );
}
bool cgLandscape::getRayIntersect( const cgVector3 & vecOrigin, const cgVector3 & vecVelocity, cgFloat & t, cgFloat fAccuracy ) const
{
    cgVector3       vecRayStart, vecRayEnd, vecRayDir, vecPos, vecPosAdd;
    bool            bSourceInBounds, bDestInBounds, bFound;
    cgFloat         fOffset, fDelta, fRecipRayLen;
    cgBoundingBox   BlockBounds;
    cgInt32         nStepCount, i, j;

    // Cache the length of the incoming ray as an optimization.
    fRecipRayLen = cgVector3::length( vecVelocity );
    if ( fRecipRayLen < CGE_EPSILON )
        return false;
    fRecipRayLen = 1.0f / fRecipRayLen;

    // Initialize t value (important for 'closest point' matching)
    t = 1.0f;
    
    // Loop through each block and test to see if the ray intersects its bounding box
    for ( i = 0; i < mBlockLayout.width * mBlockLayout.height; ++i )
    {
        cgTerrainBlock * pBlock = mTerrainBlocks[ i ];
        if ( pBlock == CG_NULL ) continue;

        // Retrieve the bounding box for this block
        BlockBounds = pBlock->getBoundingBox();

        // Add a tolerance to ensure that we don't miss the terrain.
        cgFloat fAdjust = (1.0f / fAccuracy) * 2.0f;
        BlockBounds.min.y -= fAdjust;
        BlockBounds.max.y += fAdjust;
        
        // Skip if there is no intersection at all
        if ( BlockBounds.intersect( vecOrigin, vecVelocity, fDelta ) == false )
            continue;

        // Store copy of ray start and end point so we can update them
        vecRayStart = vecOrigin; 
        vecRayEnd   = vecOrigin + vecVelocity;

        // Correct the ray if either point is outside the terrain bounds
        bSourceInBounds = BlockBounds.containsPoint( vecRayStart );
        bDestInBounds   = BlockBounds.containsPoint( vecRayEnd );

        // If either points are ourside the bounding box, lets correct them
        if ( bSourceInBounds == false )
        {
            // Calculate the new ray starting point (we've already done the actual intersection 
            // test above as our early out, so there is no need to do this one again)
            vecRayStart = vecOrigin + (vecVelocity * fDelta);

        } // End if source outside bounds ?

        if ( bDestInBounds == false )
        {
            // Get the bounding box intersection point
            if ( BlockBounds.intersect( vecRayEnd, (vecRayStart - vecRayEnd), fDelta ) == false )
                continue;
            vecRayEnd = vecRayEnd + ((vecRayStart - vecRayEnd) * fDelta );

        } // End if dest outside bounds ?

        // Cache values ready to start the process
        vecPos       = vecRayStart;
        vecRayDir    = (vecRayEnd - vecRayStart);
	    fOffset      = 1.0f / ( cgVector3::length( vecRayDir ) * fAccuracy );
	    vecPosAdd    = vecRayDir * fOffset;
        nStepCount   = (cgInt32)(1.0f / fOffset);

        // Itterate across the terrain
        bFound = false;
	    for ( j = 0; j < nStepCount && bFound == false; ++j ) 
        {
		    // Does this point dip under the terrain ?
            if ( vecPos.y < pBlock->getTerrainHeight( vecPos.x, vecPos.z ) )
            {
                // Calculate the time value and store if it is closer
                fDelta = cgVector3::length( vecPos - vecOrigin ) * fRecipRayLen;
                if ( fDelta < t ) t = fDelta;
                
                // Bail from this loop and keep searching blocks
                bFound = true;

            } // End if falls under terrain

            // Move to next point
		    vecPos += vecPosAdd;

	    } // Next point along ray

    } // Next Block

    // Intersection found?
    return (t < 1.0f);
}

//-----------------------------------------------------------------------------
// Name : getSharedColorSampler ()
/// <summary>
/// Retrieve one of the common samplers used to bind layer color maps to 
/// the rendering effect.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLandscape::getSharedColorSampler( cgUInt32 nIndex ) const
{
    if ( nIndex >= 4 )
        return CG_NULL;
    return mLayerColorSamplers[nIndex];
}

//-----------------------------------------------------------------------------
// Name : getSharedNormalSampler ()
/// <summary>
/// Retrieve one of the common samplers used to bind layer normal maps to 
/// the rendering effect.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLandscape::getSharedNormalSampler( cgUInt32 nIndex ) const
{
    if ( nIndex >= 4 )
        return CG_NULL;
    return mLayerNormalSamplers[nIndex];
}

//-----------------------------------------------------------------------------
// Name : getBlendMapSampler ()
/// <summary>
/// Retrieve the common sampler used to bind the blend map textures to the 
/// rendering effect.
/// </summary>
//-----------------------------------------------------------------------------
cgSampler * cgLandscape::getBlendMapSampler( ) const
{
    return mBlendMapSampler;
}

//-----------------------------------------------------------------------------
// Name : getBlendMapPaintData ()
/// <summary>
/// Retrieve a copy of any active "paint" layer data from any terrain block
/// that falls within the source rectangle.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::getBlendMapPaintData( const cgRect & rcSource, cgByteArray & aData ) const
{
    // Allocate a buffer with enough room for the requested paint area.
    aData.resize( rcSource.width() * rcSource.height() );

    // Process all blocks that have an active paint region.
    for ( cgInt32 y = 0; y < mBlockLayout.height; ++y )
    {
        for ( cgInt32 x = 0; x < mBlockLayout.width; ++x )
        {
            cgTerrainBlock * pBlock = mTerrainBlocks[ x + y * mBlockLayout.width ];
            if ( pBlock == CG_NULL ) continue;

            // Does this block have an active paint layer?
            cgLandscapeTextureData * pTextureData = pBlock->getTextureData();
            if ( pTextureData == CG_NULL || pTextureData->isPainting() == false )
                continue;

            // Retrieve a rectangle for this block's blend map data in order to
            // determine if it intersects the requested rectangle, and if so by how much.
            cgRect rcBlendMap  = pTextureData->getBlendMapArea();
            cgRect rcIntersect = cgRect::intersect( rcSource, rcBlendMap );
            if ( rcIntersect.isEmpty() == true )
                continue;

            // This block intersects the requested rectangle. Convert the intersection area into
            // the area of the final buffer into which this block's data will be placed.
            cgRect rcOutput;
            rcOutput.left   = (rcIntersect.left - rcSource.left);
            rcOutput.top    = (rcIntersect.top - rcSource.top);
            rcOutput.right  = rcOutput.left + rcIntersect.width();
            rcOutput.bottom = rcOutput.top + rcIntersect.height();

            // Also compute the area of the block's blend map that should be copied.
            cgRect rcInput;
            rcInput.left   = (rcIntersect.left - rcBlendMap.left);
            rcInput.top    = (rcIntersect.top - rcBlendMap.top);
            rcInput.right  = rcInput.left + rcIntersect.width();
            rcInput.bottom = rcInput.top + rcIntersect.height();

            // Get the paint layer data
            const cgByteArray & aPaintData = pTextureData->getPaintData();
            
            // Select iteration values for each pixel we need to write
            cgInt32 xStart    = rcOutput.left;
            cgInt32 xEnd      = xStart + rcOutput.width();
            cgInt32 yStart    = rcOutput.top;
            cgInt32 yEnd      = yStart + rcOutput.height();
            cgInt32 xOffset   = rcInput.left - rcOutput.left;
            cgInt32 yOffset   = rcInput.top - rcOutput.top;
            cgInt32 nDstPitch = rcSource.width();
            cgInt32 nSrcPitch = rcBlendMap.width();
            
            // Write!
            cgByte * pDest = (&aData[0]) + (xStart + yStart * nDstPitch);
            const cgByte * pSrc  = (&aPaintData[0])  + ((xStart + xOffset) + (yStart + yOffset) * nSrcPitch);
            for ( cgInt32 y2 = yStart; y2 < yEnd; ++y2 )
            {
                memcpy( pDest, pSrc, rcOutput.width() );
                pDest += nDstPitch;
                pSrc  += nSrcPitch;

            } // Next Row

        } // Next Column
        
    } // Next Row

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : computeVisibility () (Virtual)
/// <summary>
/// Called just prior to drawing the landscape in order to compute the list of
/// visible terrain blocks and landscape cells.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::computeVisibility( const cgFrustum & Frustum, cgVisibilitySet * pVisData, cgUInt32 nFlags, cgSpatialTreeInstance * pIssuer )
{
    // Enable / Disable occlusion culling depending on active camera's projection mode.
    cgCameraNode * pCamera = mParentScene->getActiveCamera();
    if ( pCamera->getProjectionMode() == cgProjectionMode::Orthographic )
    {
        mOcclusionCull = false;
    
    } // End if Ortho
    else
    {
        mOcclusionCull = false;
        if ( mNextOcclusionTest == 0 || (cgUInt32)cgTimer::getInstance()->getTime() >= mNextOcclusionTest )
            mOcclusionCull = true;
    
    } // End if perspective
    
    // Flatten the camera's world matrix to the XZ plane to allow us to easily treat this as a 2D problem
    cgMatrix mtxView = pCamera->getWorldTransform( false );
    cgVector3 vecUp, vecLook = (cgVector3&)mtxView._31, vecRight = (cgVector3&)mtxView._11, vecPos = (cgVector3&)mtxView._41;
    vecLook.y = 0;
    cgVector3::normalize( vecLook, vecLook );
    cgVector3::cross( vecUp, vecLook, vecRight );
    cgVector3::normalize( vecUp, vecUp );

    // Generate view matrix from the above values
    mtxView._11 = vecRight.x; mtxView._12 = vecUp.x; mtxView._13 = vecLook.x; mtxView._14 = 0.0f;
    mtxView._21 = vecRight.y; mtxView._22 = vecUp.y; mtxView._23 = vecLook.y; mtxView._24 = 0.0f;
    mtxView._31 = vecRight.z; mtxView._32 = vecUp.z; mtxView._33 = vecLook.z; mtxView._34 = 0.0f;
    mtxView._41 = -cgVector3::dot( vecPos, vecRight );
    mtxView._42 = -cgVector3::dot( vecPos, vecUp );
    mtxView._43 = -cgVector3::dot( vecPos, vecLook  );
    mtxView._44 = 1.0f;

    // Compute the render viewport size
    cgToDo( "Effect Overhaul", "Get actual view size!" );
    //cgRect rcRenderArea = mParentScene->getRenderDriver()->GetRenderArea();
    cgRect rcRenderArea( 0, 0, 1000, 1000 );
    cgInt32 nViewportWidth = rcRenderArea.width();

    // Interpolate every other pixel (lower resolution raster / test)
    // ToDo: Make this optional?
    nViewportWidth /= 2;
    
    // Store the combined view and projection matrix of the camera
    // we are testing visibility for.
    cgMatrix mtxVisibility = mtxView * pCamera->getProjectionMatrix();
    mViewHalfWidth  = (cgFloat)nViewportWidth * 0.5f;
    mViewHalfHeight = (cgFloat)rcRenderArea.height() * 0.5f;

    // Near clipping plane
    mHorizonNearPlane.a = mtxVisibility._13;
    mHorizonNearPlane.b = mtxVisibility._23;
    mHorizonNearPlane.c = mtxVisibility._33;
    mHorizonNearPlane.d = mtxVisibility._43;
    cgPlane::normalize( mHorizonNearPlane, mHorizonNearPlane );

    // Clear the horizon buffer ready for recording new max heights
    mHorizonBuffer.resize( nViewportWidth );
    for ( cgInt32 i = 0; i < nViewportWidth; ++i )
        mHorizonBuffer[i] = -FLT_MAX;

    // Start the traversal.
    mOcclusionSuccess = 0;
    mOcclusionFailure = 0;
    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)mRootNode, Frustum, vecPos, mtxVisibility, mOcclusionCull, pVisData, nFlags );

    // Is it worth sampling occlusion again next frame?
    if ( mOcclusionCull == true )
    {
        // Keep occlusion testing while failure rate is <= 70%.
        // Otherwise, try again in a second.
        mNextOcclusionTest = 0;
        if ( mOcclusionSuccess + mOcclusionFailure > 0 )
        {
            if ( (cgFloat)mOcclusionFailure / (cgFloat)(mOcclusionFailure+mOcclusionSuccess) > 0.7f )
                mNextOcclusionTest = (cgUInt32)cgTimer::getInstance()->getTime() + 1;

        } // End if nothing to count
    
    } // End if occlusion testing
}

//-----------------------------------------------------------------------------
// Name : updateTreeVisibility () (Protected)
/// <summary>
/// The actual recursive function which traverses the tree and updates the 
/// visible status of the tree.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::updateTreeVisibility( cgSpatialTreeInstance * pIssuer, cgLandscapeSubNode * pNode, const cgFrustum & ViewFrustum, const cgVector3 & vecSortOrigin, const cgMatrix & mtxVisibility, bool bOcclusionCull, cgVisibilitySet * pVisData, cgUInt32 nFlags )
{
    cgFloat fDistX, fDistZ;
    
    // Valid node?
    if ( pNode == CG_NULL )
        return;
    
    // Classify the node bounding box against the frustum
    const cgBoundingBox & NodeBounds = pNode->getBoundingBox();
    cgVolumeQuery::Class FrustumResult = ViewFrustum.classifyAABB( NodeBounds );

    // If frustum test rejected this entirely, we can just return
    if ( FrustumResult == cgVolumeQuery::Outside )
        return;

    // Occlusion culling enabled?
    if ( bOcclusionCull == false )
    {
        // Process the leaf if there is one stored here
        if ( pNode->isLeafNode() == true )
        {
            // Mark terrain block in which this cell exists as visible
            cgLandscapeCell * pCell = (cgLandscapeCell*)mLeaves[pNode->getLeaf()];
            if ( pCell->getDataGroupId() >= 0 )
                pVisData->addVisibleGroup( this, pCell->getDataGroupId() );

            // ToDo: Perhaps record the minimum and maximum object size in the leaf and skip 
            // if entire range can be immediately culled. Use a binary search in the distance array?

            // Are there any objects stored in this leaf?
            if ( pIssuer )
            {
                const cgObjectNodeSet & LeafObjects = pIssuer->getOwnedObjects( pNode->getLeaf() );
                if ( LeafObjects.empty() == false )
                {
                    cgVector3 vClosest = NodeBounds.closestPoint( vecSortOrigin );
                    cgFloat fDistanceSq = cgVector3::lengthSq( vClosest - vecSortOrigin );
                    if ( fDistanceSq <= mObjectCullDistanceSq )
                    {
                        cgFloat fSize;
                        bool    bCull = false;
                        size_t  i, nMinCullRecord, nMaxCullRecord = mCullDistances.size();

                        // ToDo: Re-enable this?
                        // Leaf is within the maximum culling range. We need to cull further.
                        // Work out which of the selecting culling distances we should start
                        // at based on the leaf's closest distance (optimization).
                        //for ( i = 0, nMinCullRecord = 0; i < nMaxCullRecord; ++i )
                        //{
                            //if ( mCullDistances[i].fDistanceSq > fDistanceSq )
                            //{
                                //nMinCullRecord = i;
                                //break;

                            //} // End if ignore entry.

                        //} // Next Entry
                        nMinCullRecord = 0;

                        // Iterate through each object in the cell and work out whether
                        // or not we want to keep or discard it based on distance and size.
                        cgObjectNodeSet::const_iterator itObject;
                        for ( itObject = LeafObjects.begin(); itObject != LeafObjects.end(); ++itObject )
                        {
                            cgObjectNode * pObject = *itObject;

                            // Determine the size of this object
                            fDistanceSq = cgVector3::lengthSq( pObject->getPosition() - vecSortOrigin );
                            bCull       = false;
                            if ( fDistanceSq > mObjectCullDistanceSq )
                            {
                                bCull = true;
                            
                            } // End if outside max range
                            else
                            {
                                // Check precise records
                                fSize = pObject->getObjectSize();
                                for ( i = nMinCullRecord; i < nMaxCullRecord && bCull == false; ++i )
                                {
                                    const CullDescriptor & Desc = mCullDistances[i];
                                    // ToDo: Remove debug output
                                    if ( fSize < Desc.maximumSize && fDistanceSq > Desc.distanceSquared )
                                    {
                                        //Console::WriteLine( L"Opted to cull object with sizeSq {0} and distSq of {1} against maxSizeSq {2} and maxDistSq {3}", fSizeSq, fDistanceSq, desc.maximumSizeSquared, desc.fDistanceSq );
                                        bCull = true;
                                    }
                                    else
                                    {
                                        //Console::WriteLine( L"Opted to RETAIN object with sizeSq {0} and distSq of {1} against maxSizeSq {2} and maxDistSq {3}", fSizeSq, fDistanceSq, desc.maximumSizeSquared, desc.fDistanceSq );
                                    }

                                } // End if

                            } // End if within max range

                            // If not culled, ask the object to perform final filters and checks and
                            // to register any information it needs to this visibility set if it 
                            // passes any tests it wants/needs to make.
                            if ( !bCull )
                                pObject->registerVisibility( pVisData );

                        } // Next Object

                    } // End if leaf within max cull range

                } // End if any objects to cull

            } // End if valid issuer

            // No further recursion required
            return;

        } // End if process leaf
        
    } // End if no occlusion cull
    else
    {

        // Only test against the horizon buffer if it is in front of the horizon view plane
        // (Works without spanning case, but since these are pretty much guaranteed to be visible anyway,
        //  we don't bother testing them.)
        cgPlaneQuery::Class AABBLocation = cgCollision::AABBClassifyPlane( NodeBounds, (cgVector3&)mHorizonNearPlane, mHorizonNearPlane.d );
        if ( AABBLocation != cgPlaneQuery::Back && AABBLocation != cgPlaneQuery::Spanning )
        {
            // If the node was completely occluded by data already in the
            // horizon buffer, then we can immediately bail
            if ( testNodeOcclusion( pNode, vecSortOrigin, mtxVisibility ) == true )
            {
                // Count occlusion successes (approximate?).
                mOcclusionSuccess += (cgUInt32)pow(4.0, cgInt(mMaxTreeDepth - pNode->getNodeDepth()));
                return;
            
            } // End if occluded
        
        } // End if node is not behind test plane

        // Draw minimum geometry if it's stored here. If the node was behind the horizon 
        // view plane, we can skip this (i.e. don't draw to horizon buffer) because the 
        // horizon data will just be clipped away anyway.
        if ( pNode->minimumHorizonPoints != CG_NULL && AABBLocation != cgPlaneQuery::Back )
        {
            // Draw the node's minimum plane data to the current horizon buffer
            const cgVector3 * pData = pNode->minimumHorizonPoints;
            cgUInt32 i0, i1, i2, i3;
        
            // Back face test against the node minimum plane
            cgFloat Result = cgPlane::dotCoord( pNode->minimumHorizonPlane, vecSortOrigin );
            
            // Were we in front of or behind the plane?
            if ( Result >= 0 )
            {
                // Camera can potentially see the front
                i0 = 0; i1 = 1; i2 = 2; i3 = 3;
            
            } // End if front facing
            else
            {
                // Camera can potentially see the back
                i0 = 3; i1 = 2; i2 = 1; i3 = 0;
            
            } // End if back facing

            cgVector4 vecA, vecB, vecC, vecD;
            cgVector3::transform( vecA, pData[i0], mtxVisibility );
            cgVector3::transform( vecB, pData[i1], mtxVisibility );
            cgVector3::transform( vecC, pData[i2], mtxVisibility );
            cgVector3::transform( vecD, pData[i3], mtxVisibility );
            
            // Draw the horizon data
            drawHorizonEdge( vecA, vecB );
            drawHorizonEdge( vecB, vecC );
            drawHorizonEdge( vecC, vecD );
            drawHorizonEdge( vecD, vecA );

            // Count occlusion failures (approximate?).
            mOcclusionFailure += (int)pow(4.0, cgInt(mMaxTreeDepth - pNode->getNodeDepth()));

        } // End if process min geom
        
        // Process the leaf if there is one stored here
        if ( pNode->isLeafNode() == true )
        {
            // Mark terrain block in which this cell exists as visible
            cgLandscapeCell * pCell = (cgLandscapeCell*)mLeaves[pNode->getLeaf()];
            if ( pCell->getDataGroupId() >= 0 )
                pVisData->addVisibleGroup( this, pCell->getDataGroupId() );

            // ToDo: Perhaps record the minimum and maximum object size in the leaf and skip 
            // if entire range can be immediately culled. Use a binary search in the distance array?

            // Are there any objects stored in this leaf?
            if ( pIssuer )
            {
                const cgObjectNodeSet & LeafObjects = pIssuer->getOwnedObjects(pNode->getLeaf());
                if ( LeafObjects.empty() == false )
                {
                    cgVector3 vClosest = NodeBounds.closestPoint( vecSortOrigin );
                    cgFloat fDistanceSq = cgVector3::lengthSq( vClosest - vecSortOrigin );
                    if ( fDistanceSq <= mObjectCullDistanceSq )
                    {
                        cgFloat fSize;
                        bool    bCull = false;
                        size_t  i, nMinCullRecord, nMaxCullRecord = mCullDistances.size();

                        // Leaf is within the maximum culling range. We need to cull further.
                        // Work out which of the selecting culling distances we should start
                        // at based on the leaf's closest distance (optimization).
                        /*for ( i = 0, nMinCullRecord = 0; i < nMaxCullRecord; ++i )
                        {
                            if ( mCullDistances[i].fDistanceSq > fDistanceSq )
                            {
                                nMinCullRecord = i;
                                break;

                            } // End if ignore entry.

                        } // Next Entry*/
                        nMinCullRecord = 0;

                        // Iterate through each object on the cell and work out whether
                        // or not we want to keep or discard it based on distance and size.
                        cgObjectNodeSet::const_iterator itObject;
                        for ( itObject = LeafObjects.begin(); itObject != LeafObjects.end(); ++itObject )
                        {
                            cgObjectNode * pObject = *itObject;

                            // Determine the size of this object
                            fDistanceSq = cgVector3::lengthSq( pObject->getPosition() - vecSortOrigin );
                            bCull       = false;
                            if ( fDistanceSq > mObjectCullDistanceSq )
                            {
                                bCull = true;
                            
                            } // End if outside max range
                            else
                            {
                                // Check precise records
                                fSize = pObject->getObjectSize();
                                for ( i = nMinCullRecord; i < nMaxCullRecord && bCull == false; ++i )
                                {
                                    const CullDescriptor & Desc = mCullDistances[i];
                                    if ( fSize < Desc.maximumSize && fDistanceSq > Desc.distanceSquared )
                                        bCull = true;

                                } // End if

                            } // End if within max range

                            // If not culled, ask the object to perform final filters and checks and
                            // to register any information it needs to this visibility set if it 
                            // passes any tests it wants/needs to make.
                            if ( !bCull )
                                pObject->registerVisibility( pVisData );

                        } // Next Object

                    } // End if leaf within max cull range

                } // End if any objects to cull

            } // End if valid Issuer
            
            // No further recursion required
            return;

        } // End if leaf
        
        // ToDo: Is this wise given how many thousands of objects may be
        // in the collapsed children?

        // Disable occlusion processing from this point on if min horizon 
        // geometry was stored here (i.e. perhasq collapsed to a location
        // higher up the tree from child nodes.)
        if ( pNode->minimumHorizonPoints != CG_NULL )
            bOcclusionCull = false;

    } // End if not auto visible

    // Recurse into all children
    if ( bOcclusionCull == false )
    {
        updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, false, pVisData, nFlags );
        updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, false, pVisData, nFlags );
        updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, false, pVisData, nFlags );
        updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, false, pVisData, nFlags );
    
    } // End if !bOcclusionCull
    else
    {
        // Force temporary vars out of scope
        {
            // Classify the sort origin against both separating planes (X and Z)
            const cgVector3 & vCenter = NodeBounds.getCenter();
            fDistX = cgVector3::dot( vecSortOrigin - vCenter, cgVector3(1,0,0) );
            fDistZ = cgVector3::dot( vecSortOrigin - vCenter, cgVector3(0,0,1) );
        
        } // End Scope

        // ToDo: Remove fDistX>=fDistZ cases if no further bugs found (notice the '|| true' that always selects the first case).
        // Select correct traversal order based on classifications.
        if ( fDistX >= 0 )
        {
            if ( fDistZ >= 0 )
            {
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                if ( fDistX < fDistZ || true )
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                else
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );

            } // End if Front
            else
            {
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                if ( fDistX < fabsf(fDistZ) || true )
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                else
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );

            } // End if Back

        } // End if Front
        else
        {
            if ( fDistZ >= 0 )
            {
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                if ( fabsf(fDistX) < fDistZ || true )
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                else
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );

            } // End if Front
            else
            {
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(0), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                if ( fabsf(fDistX) < fabsf(fDistZ) || true )
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                else
                {
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(1), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                    updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(2), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );
                }
                updateTreeVisibility( pIssuer, (cgLandscapeSubNode*)pNode->getChildNode(3), ViewFrustum, vecSortOrigin, mtxVisibility, true, pVisData, nFlags );

            } // End if Back

        } // End if Back

    } // End if bOcclusionCull
}

//-----------------------------------------------------------------------------
// Name : testNodeOcclusion () (Protected)
/// <summary>
/// Performs the test to see if the specified node was occluded based on the 
/// information currently contained in the horizon buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::testNodeOcclusion( cgLandscapeSubNode * pNode, const cgVector3 & vecSortOrigin, const cgMatrix & mtxVisibility )
{
    cgUInt32          i0, i1, i2, i3, nClipped = 0, nInvalid = 0;
    DrawResult        Visibility;
    cgFloat           Result;
    const cgVector3 * pData;

    // We're testing the node's maximum horizon data to ensure we don't
    // get any false positives for occlusion.
    pData = pNode->maximumHorizonPoints;
    if ( pData == CG_NULL ) return false;
        
    // Back face test against the maximum plane
    Result = cgPlane::dotCoord( pNode->maximumHorizonPlane, vecSortOrigin );
    
    // Were we in front of or behind the plane?
    if ( Result >= 0 )
    {
        // Camera can potentially see the front
        i0 = 0; i1 = 1; i2 = 2; i3 = 3;
    
    } // End if front facing
    else
    {
        // Camera can potentially see the back
        i0 = 3; i1 = 2; i2 = 1; i3 = 0;
    
    } // End if back facing

    // Transform all vertices ready for clipping / testing
    cgVector4 vecA, vecB, vecC, vecD;
    cgVector3::transform( vecA, pData[i0], mtxVisibility );
    cgVector3::transform( vecB, pData[i1], mtxVisibility );
    cgVector3::transform( vecC, pData[i2], mtxVisibility );
    cgVector3::transform( vecD, pData[i3], mtxVisibility );
    
    // Test first edge
    Visibility = testHorizonEdge( vecA, vecB );
    if ( Visibility == HorizonVisible ) return false;
    else if ( Visibility == HorizonClipped ) nClipped++;
    else if ( Visibility == HorizonInvalid ) nInvalid++;

    // Test second edge
    Visibility = testHorizonEdge( vecB, vecC );
    if ( Visibility == HorizonVisible ) return false;
    else if ( Visibility == HorizonClipped ) nClipped++;
    else if ( Visibility == HorizonInvalid ) nInvalid++;

    // Test third edge
    Visibility = testHorizonEdge( vecC, vecD );
    if ( Visibility == HorizonVisible ) return false;
    else if ( Visibility == HorizonClipped ) nClipped++;
    else if ( Visibility == HorizonInvalid ) nInvalid++;

    // Test fourth edge
    Visibility = testHorizonEdge( vecD, vecA );
    if ( Visibility == HorizonVisible ) return false;
    else if ( Visibility == HorizonClipped ) nClipped++;
    else if ( Visibility == HorizonInvalid ) nInvalid++;

    // Node was occluded? This is only true if at least
    // one of the edges was NOT clipped away. Otherwise we
    // couldn't make a determination and we must assume it
    // was visible.
    return (nClipped < (4 - nInvalid));

}

// ToDo: Remove
/*void cgLandscape::DebugDraw( cgLandscapeSubNode ^ oNode, cgCameraObject ^ oCamera, ULONG & nVisitOrder )
{
    if ( oNode == nullptr )
        return;

    if ( oNode == m_oRootNode )
    {
        cgRenderDriver ^ oDriver = m_oParentScene->RenderDriver;
        cgVertexScreenPrim Points[2];
        oDriver->setVertexFormat( VertexFormats::ScreenPrimitive );
        
        for ( int i = 0; i < (signed)m_ViewportSize.width; ++i )
        {
            Points[0].position = cgVector4( (cgFloat)i, m_ViewportSize.height- m_pHorizonBuffer[i], 0.0f, 1.0f );
            Points[1].position = cgVector4( (cgFloat)i + 1, m_ViewportSize.height - m_pHorizonBuffer[i + 1], 0.0f, 1.0f );
            oDriver->DrawPrimitiveUP( D3DPT_LINELIST, 1, IntPtr( (void*)Points ) );
        }
    }
    
    if ( oNode->minimumHorizonPoints != CG_NULL )
    {
        cgLandscapeCell ^ oCell = (cgLandscapeCell^)oNode->Leaf;
        cgVertexColored Points[4];
        ULONG nColor = 0xFF000000 | ((rand() * 0xFFFFFF) / RAND_MAX);
        //ULONG nColor = nVisitOrder++;
        //cgFloat fColor = (cgFloat)nColor / 4096.0f;
        //nColor = cgColorValue( fColor, fColor, fColor, 1 );
        cgRenderDriver ^ oDriver = m_oParentScene->RenderDriver;
        oDriver->setVertexFormat( VertexFormats::Colored );
        for ( int i = 0; i < 4; ++i )
        {
            Points[i].color = nColor;
            Points[i].position = oNode->minimumHorizonPoints[i];
        }
        oDriver->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, IntPtr( (void*)Points ) );
    }
    
    if ( oNode->isLeafNode == false )
    {
        // Classify the sort origin against both separating planes (X and Z)
        Vector3 vMin, vMax;
        oNode->getBoundingBox( vMin, vMax );
        cgVector3 vCenter = cgVector3(vMin.x+vMax.x, vMin.y+vMax.y, vMin.z+vMax.z) * 0.5f;

        //Console::WriteLine( Vector3(vCenter).ToString() );
        // ToDo Remove
        cgPlane xPlane, zPlane;
        cgPlane::fromPointNormal( &xPlane, &vCenter, &cgVector3(1,0,0) );
        cgPlane::fromPointNormal( &zPlane, &vCenter, &cgVector3(0,0,1) );

        cgVector3 vecSortOrigin = oCamera->position;
        cgFloat fDistX = cgPlane::dotCoord( &xPlane, &vecSortOrigin );
        cgFloat fDistZ = cgPlane::dotCoord( &zPlane, &vecSortOrigin );

        // Select correct traversal order based on classifications.
        if ( fDistX >= 0 )
        {
            if ( fDistZ >= 0 )
            {
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );
                if ( fDistX < fDistZ )
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );
                }
                else
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );
                }
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );

            } // End if Front
            else
            {
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );
                if ( fDistX < fabsf(fDistZ) )
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );
                }
                else
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );
                }
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );

            } // End if Back

        } // End if Front
        else
        {
            if ( fDistZ >= 0 )
            {
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );
                if ( fabsf(fDistX) < fDistZ )
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );
                }
                else
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );
                }
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );

            } // End if Front
            else
            {
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera, nVisitOrder );
                if ( fabsf(fDistX) < fabsf(fDistZ) )
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );
                }
                else
                {
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera, nVisitOrder );
                    DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera, nVisitOrder );
                }
                DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera, nVisitOrder );

            } // End if Back

        } // End if Back

        //DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[0], oCamera );
        //DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[1], oCamera );
        //DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[2], oCamera );
        //DebugDraw( (cgLandscapeSubNode^)oNode->ChildNode[3], oCamera );

    }
    
    //Vector3 vMin, vMax;
    //oNode->getBoundingBox( vMin, vMax );
    //ULONG nColor = oNode->NodeDepth;
    //cgFloat fColor = (cgFloat)nColor / 6.0f;
    //nColor = cgColorValue( fColor, fColor, fColor, 1 );
    //if ( CCollision::PointInAABB( oCamera->position, vMin, vMax, false, true, false ) == true )
        //m_oParentScene->RenderDriver->DrawOOBB( vMin, vMax, 0.0f, Matrix::Identity, nColor );
}*/

//-----------------------------------------------------------------------------
// Name : drawHorizonEdge() (Protected)
/// <summary>
/// Record the maximum height of the edge specified to the horizon buffer.
/// Note : Assumes edges are always passed as if the polygon had a clockwise
/// winding in screen space (i.e. caller must flip the order of the points if 
/// the polygon was backfacing).
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::drawHorizonEdge( cgVector4 vecA, cgVector4 vecB )
{
    cgFloat  recipWA = 0.0f, recipWB = 0.0f;
    cgUInt32 nCodeA = 0, nCodeB = 0;

    // Retrieve clipping codes against the near plane only.
    if ( vecA.z < 0.0f ) nCodeA = 1;
    if ( vecB.z < 0.0f ) nCodeB = 1;

    // If both are behind the near plane, we can bail early
    if ( nCodeA & nCodeB ) return;

    // If they span the near plane, we need to clip the line
    if ( (nCodeA | nCodeB) != 0 )
    {
        // Clip the edge starting at either the first or second vertex
        if ( nCodeA )
        {
            const cgFloat dx = vecA.x - vecB.x;
	        const cgFloat dy = vecA.y - vecB.y;
	        const cgFloat dz = vecA.z - vecB.z;
	        const cgFloat dw = vecA.w - vecB.w;
            const cgFloat Delta = -vecB.z / dz;
            vecA.x = vecB.x + dx * Delta;
            vecA.y = vecB.y + dy * Delta;
            vecA.z = vecB.z + dz * Delta;
            vecA.w = vecB.w + dw * Delta;
        
        } // End if A behind
        else
        {
            const cgFloat dx = vecB.x - vecA.x;
	        const cgFloat dy = vecB.y - vecA.y;
	        const cgFloat dz = vecB.z - vecA.z;
	        const cgFloat dw = vecB.w - vecA.w;
            const cgFloat Delta = -vecA.z / dz;
            vecB.x = vecA.x + dx * Delta;
            vecB.y = vecA.y + dy * Delta;
            vecB.z = vecA.z + dz * Delta;
            vecB.w = vecA.w + dw * Delta;
        
        } // End if B behind

    } // End if spans near plane

    // Compute the reciprocals of the w value to allow us to
    // multiply instead of divide (optimization).
    recipWA  = 1.0f / vecA.w;
    recipWB  = 1.0f / vecB.w;

    // Project the X components first, this allows us to perform
    // an early out before projecting the Y component.
    vecA.x *= recipWA;
    vecB.x *= recipWB;

    // We can bail early if the edge does not run from the left hand side of the screen
    // to the right. This is because only the top most edges (screen space) will need to have
    // their heights recorded in the horizon buffer. Assuming a clockwise winding edge
    // this will always be the case, hence the caller being required to swap the order
    // if this is a backfacing polygon.
    if ( vecA.x > vecB.x )
        return;
    
    // Project the Y component
    vecA.y *= recipWA;
    vecB.y *= recipWB;

    // Transform to 'inverted' screen space
    // (where 0,0 is located at the BOTTOM left of the screen)
    vecA.x  = (vecA.x + 1) * mViewHalfWidth;
    vecA.y  = (vecA.y + 1) * mViewHalfHeight;
    vecB.x  = (vecB.x + 1) * mViewHalfWidth;
    vecB.y  = (vecB.y + 1) * mViewHalfHeight;
	
    // Rasterize the horizon line
    rasterHorizonLine( vecA.x, vecA.y, vecB.x, vecB.y );
}

//-----------------------------------------------------------------------------
// Name : testHorizonEdge() (Protected)
/// <summary>
/// Test the specified edge against the data already contained in the horizon 
/// buffer. This does not record any information to the buffer and will return
/// at the earliest possible opportunity.
/// Note : Does not perform clipping, due to the fact that the system 
/// automatically discards any polygons spanning or behind the horizon "near" 
/// plane.
/// Note : Assumes edges are always passed as if the polygon had a clockwise
/// winding in screen space (i.e. caller must flip the order of the points if 
/// the polygon was backfacing).
//-----------------------------------------------------------------------------
cgLandscape::DrawResult cgLandscape::testHorizonEdge( cgVector4 vecA, cgVector4 vecB )
{
    cgFloat recipWA = 0.0f, recipWB = 0.0f;

    // ToDo: AMH - Clipping logic in case necessary in the future
    /*int   nCodeA = 0, nCodeB = 0;

    // Retrieve clipping codes against the near plane only.
    if ( vecA.z < 0.0f ) nCodeA = 1;
    if ( vecB.z < 0.0f ) nCodeB = 1;

    // If both are behind the near plane, we can bail early
    if ( nCodeA & nCodeB ) 
        return HorizonClipped;

    // If they span the near plane, we need to clip the line
    if ( (nCodeA | nCodeB) != 0 )
    {
        // Clip the edge starting at either the first or second vertex
        if ( nCodeA )
        {
            const cgFloat dx = vecA.x - vecB.x;
	        const cgFloat dy = vecA.y - vecB.y;
	        const cgFloat dz = vecA.z - vecB.z;
	        const cgFloat dw = vecA.w - vecB.w;
            const cgFloat Delta = -vecB.z / dz;
            vecA.x = vecB.x + dx * Delta;
            vecA.y = vecB.y + dy * Delta;
            vecA.z = vecB.z + dz * Delta;
            vecA.w = vecB.w + dw * Delta;
        
        } // End if A behind
        else
        {
            const cgFloat dx = vecB.x - vecA.x;
	        const cgFloat dy = vecB.y - vecA.y;
	        const cgFloat dz = vecB.z - vecA.z;
	        const cgFloat dw = vecB.w - vecA.w;
            const cgFloat Delta = -vecA.z / dz;
            vecB.x = vecA.x + dx * Delta;
            vecB.y = vecA.y + dy * Delta;
            vecB.z = vecA.z + dz * Delta;
            vecB.w = vecA.w + dw * Delta;
        
        } // End if B behind

    } // End if spans near plane*/

    // Compute the reciprocals of the w value to allow us to
    // multiply instead of divide (optimization).
    recipWA  = 1.0f / vecA.w;
    recipWB  = 1.0f / vecB.w;

    // Project the X components first, this allows us to perform
    // an early out before projecting the Y component.
    vecA.x *= recipWA;
    vecB.x *= recipWB;

    // We can bail early if the edge does not run from the left hand side of the screen
    // to the right. This is because only the top most edges (screen space) will need to have
    // their heights tested against the horizon buffer. Assuming a clockwise winding edge
    // this will always be the case, hence the caller being required to swap the order
    // if this is a backfacing polygon.
    if ( vecA.x > vecB.x )
        return HorizonInvalid;
    
    // Project the Y component
    vecA.y *= recipWA;
    vecB.y *= recipWB;

    // Transform to 'inverted' screen space
    // (where 0,0 is located at the BOTTOM left of the screen)
    vecA.x  = (vecA.x + 1) * mViewHalfWidth;
    vecA.y  = (vecA.y + 1) * mViewHalfHeight;
    vecB.x  = (vecB.x + 1) * mViewHalfWidth;
    vecB.y  = (vecB.y + 1) * mViewHalfHeight;
	
    // Test the horizon line
    return testHorizonLine( vecA.x, vecA.y, vecB.x, vecB.y );
}

//-----------------------------------------------------------------------------
// Name : rasterHorizonLine() (Protected)
/// <summary>
/// Clips and draws the specified line, recording the heights to the horizon 
/// buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::rasterHorizonLine( cgFloat x1, cgFloat y1, cgFloat x2, cgFloat y2 )
{
    cgFloat fCurrentHeight;
    cgInt32 x;

    // Cache right screen edge value
    const cgFloat fRightEdge = (cgFloat)mHorizonBuffer.size() - 1;

    // Completely outside screen? (Left edge outside the right side
    // of screen, or right edge to the left)
    if ( x2 < 0 || x1 >= fRightEdge )
        return;

    // Compute the edge delta values
    const cgFloat fDeltaX = x2 - x1;
    const cgFloat fDeltaY = y2 - y1;

    // Protect against divide by zero
    if ( fDeltaX < CGE_EPSILON )
        return;

    // Compute the height interpolation value which will
    // be added to the current height each time we move
    // along to a new column in the horizon buffer
    const cgFloat fYStep = fDeltaY / fDeltaX;

    // Clip the line to the left edge of the screen
    if ( x1 < 0 )
    {
        y1 += -x1 * fYStep;
        x1 = 0;
    
    } // End if clip left

    // Clip the line to the right edge of the screen
    if ( x2 > fRightEdge )
    {
        y2 -= (fRightEdge - x2) * fDeltaY;
        x2  = fRightEdge;
    
    } // End if clip right

    // Generate the integer values that specify the pixel to
    // start and end the line rasterization
    const cgInt nXBegin = cgMathUtility::integerCeil(x1-0.5f);
    const cgInt nXEnd   = cgMathUtility::integerFloor(x2-0.5f);

    // Step across each pixel in the line and record max heights
    fCurrentHeight = y1;
    cgFloat * pBuffer = &mHorizonBuffer[0];
    for ( x = nXBegin; x <= nXEnd; ++x )
    {
        #if defined( _DEBUG )
            // Perform overflow check if we are in debug mode, this should never realistically happen
            if ( x < 0 || x >= (signed)mHorizonBuffer.size() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Attempting to write outside of horizon buffer safe range (0 to %i). Overflow occured writing to element %i.\n"), mHorizonBuffer.size() - 1, x );
                return;
            
            } // End if OOB
        #endif

        // Exceeds the value currently stored in the horizon buffer?
        if ( pBuffer[x] < fCurrentHeight )
            pBuffer[x] = fCurrentHeight;

        // Add our Y step value ready for the next pixel
        fCurrentHeight += fYStep;
    
    } // Next pixel
}

//-----------------------------------------------------------------------------
// Name : testHorizonLine() (Protected)
/// <summary>
/// Clips and samples the specified line, testing against the current values in
/// the horizon buffer to see if any point falls above the current horizon.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscape::DrawResult cgLandscape::testHorizonLine( cgFloat x1, cgFloat y1, cgFloat x2, cgFloat y2 )
{
    cgFloat fCurrentHeight;
    cgInt32 x;

    // Cache right screen edge value
    const cgFloat fRightEdge = (cgFloat)mHorizonBuffer.size() - 1;

    // Completely outside screen? (Left edge outside the right side
    // of screen, or right edge to the left)
    if ( x2 < 0 || x1 >= fRightEdge )
        return HorizonClipped;

    // Compute the edge delta values
    const cgFloat fDeltaX = x2 - x1;
    const cgFloat fDeltaY = y2 - y1;

    // Protect against divide by zero
    if ( fDeltaX < CGE_EPSILON )
        return HorizonInvalid;

    // Compute the height interpolation value which will
    // be added to the current height each time we move
    // along to a new column in the horizon buffer
    const cgFloat fYStep = fDeltaY / fDeltaX;

    // Clip the line to the left edge of the screen
    if ( x1 < 0 )
    {
        y1 += -x1 * fYStep;
        x1 = 0;
    
    } // End if clip left

    // Clip the line to the right edge of the screen
    if ( x2 > fRightEdge )
    {
        y2 -= (fRightEdge - x2) * fDeltaY;
        x2  = fRightEdge;
    
    } // End if clip right

    // Generate the integer values that specify the pixel to
    // start and end the line rasterization
    const cgInt nXBegin = cgMathUtility::integerCeil(x1-0.5f);
    const cgInt nXEnd   = cgMathUtility::integerFloor(x2-0.5f);
    
    // Valid line?
    if ( nXEnd < nXBegin ) 
        return HorizonInvalid;

    // Step across each pixel in the line and record max heights
    fCurrentHeight = y1;
    cgFloat * pBuffer = &mHorizonBuffer[0];
    for ( x = nXBegin; x <= nXEnd; ++x )
    {
        #if defined( _DEBUG )
            // Perform overflow check if we are in debug mode, this should never realistically happen
            if ( x < 0 || x >= (signed)mHorizonBuffer.size() )
            {
                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Attempting to read outside of horizon buffer safe range (0 to %i). Overflow occured reading from element %i.\n"), mHorizonBuffer.size() - 1, x );
                return HorizonInvalid;
            
            } // End if OOB
        #endif

        // Exceeds the value currently stored in the horizon buffer?
        if ( fCurrentHeight > pBuffer[x] )
            return HorizonVisible;

        // Add our Y step value ready for the next pixel
        fCurrentHeight += fYStep;
    
    } // Next pixel

    // No pixels were actually drawn
    return HorizonOccluded;
}

//-----------------------------------------------------------------------------
// Name : getTerrainDetail ( )
/// <summary>
/// Retrieve the global terrain detail scalar influences the detail level of 
/// the terrain.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgLandscape::getTerrainDetail( ) const
{
    return mTerrainDetail;
}

//-----------------------------------------------------------------------------
// Name : setTerrainDetail ( )
/// <summary>
/// Set the global terrain detail scalar influences the detail level of 
/// the terrain.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::setTerrainDetail( cgFloat value )
{
    mTerrainDetail = value;

    // ToDo: 9999 - This needs to be serialized / deserialized!
    // We might consider renaming this 'Geometry detail', or perhaps
    // something else that makes a little more sense to the artist.
}

//-----------------------------------------------------------------------------
// Name : getMipLookUp ( )
/// <summary>
/// Retrieve the geo-mip lookup table used to provide optimal vertex ordering.
/// </summary>
//-----------------------------------------------------------------------------
const cgInt32Array & cgLandscape::getMipLookUp( ) const
{
    return mMipLookUp;
}

//-----------------------------------------------------------------------------
// Name : getFlags ( )
/// <summary>
/// Retrieve the landscape flags that describe how the landscape should be
/// constructed and / or managed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgLandscape::getFlags( ) const
{
    return mLandscapeFlags;
}

//-----------------------------------------------------------------------------
// Name : getTerrainScale ( )
/// <summary>
/// Retrieve the scale of the terrain (vertex spacing and vertical scale)
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgLandscape::getTerrainScale( ) const
{
    return mScale;
}

//-----------------------------------------------------------------------------
// Name : getTerrainOffset ( )
/// <summary>
/// Retrieve the overall offset of the terrain.
/// </summary>
//-----------------------------------------------------------------------------
const cgVector3 & cgLandscape::getTerrainOffset( ) const
{
    return mOffset;
}

//-----------------------------------------------------------------------------
// Name : getHeightMapSize ( )
/// <summary>
/// Retrieve the size of the height map in its entirety (even if the heightmap
/// on which the landscape is based is no longer resident).
/// </summary>
//-----------------------------------------------------------------------------
cgSize cgLandscape::getHeightMapSize( ) const 
{
    return cgSize( (mBlockLayout.width * (mBlockSize.width - 1)) + 1, (mBlockLayout.height * (mBlockSize.height - 1)) + 1 );
}

//-----------------------------------------------------------------------------
// Name : getBlockSize ( )
/// <summary>
/// Retrieve the size of an individual terrain block in vertices.
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgLandscape::getBlockSize( ) const
{
    return mBlockSize;
}

//-----------------------------------------------------------------------------
// Name : getBlockBlendMapSize ( )
/// <summary>
/// Retrieve the resolution of the blend map data that will be maintained at
/// each terrain block for layer painting / splatting.
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgLandscape::getBlockBlendMapSize( ) const
{
    return mBlendMapSize;
}

//-----------------------------------------------------------------------------
// Name : getBlockLayout ( )
/// <summary>
/// Retrieve the grid dimensions / layout of the terrain blocks for this
/// landscape.
/// </summary>
//-----------------------------------------------------------------------------
const cgSize & cgLandscape::getBlockLayout( ) const
{
    return mBlockLayout;
}

//-----------------------------------------------------------------------------
// Name : getHeightMap ( )
/// <summary>
/// Retrieve the underlying heightmap data if resident.
/// </summary>
//-----------------------------------------------------------------------------
cgHeightMap * cgLandscape::getHeightMap( ) const
{
    return mHeightMap;
}

//-----------------------------------------------------------------------------
// Name : getColorMap ( )
/// <summary>
/// Retrieve the vertex color map data if supplied.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32Array & cgLandscape::getColorMap( )
{
    return mColorMap;
}

//-----------------------------------------------------------------------------
// Name : getLODData ( )
/// <summary>
/// Retrieve the array containing the various LOD definitions for each
/// individual terrain blocks.
/// </summary>
//-----------------------------------------------------------------------------
const cgLandscape::TerrainLODArray & cgLandscape::getLODData( ) const
{
    return mLODData;
}

//-----------------------------------------------------------------------------
// Name : getTerrainBlocks ( )
/// <summary>
/// Retrieve the array containing the terrain blocks that constitute this
/// landscape's surface.
/// </summary>
//-----------------------------------------------------------------------------
const cgLandscape::TerrainBlockArray & cgLandscape::getTerrainBlocks( ) const
{
    return mTerrainBlocks;
}

//-----------------------------------------------------------------------------
// Name : getProceduralLayers ( )
/// <summary>
/// Retrieve the array containing the currently defined procedural rendering
/// layers.
/// </summary>
//-----------------------------------------------------------------------------
const cgLandscape::ProceduralLayerArray & cgLandscape::getProceduralLayers( ) const
{
    return mProceduralLayers;
}

//-----------------------------------------------------------------------------
// Name : getProceduralLayer ( )
/// <summary>
/// Retrieve the specified procedural rendering layer.
/// </summary>
//-----------------------------------------------------------------------------
const cgLandscape::ProceduralLayerRef & cgLandscape::getProceduralLayer( cgUInt32 nLayerId ) const
{
    return mProceduralLayers[ nLayerId ];
}

//-----------------------------------------------------------------------------
// Name : setProceduralLayer ( )
/// <summary>
/// Update the specified procedural rendering layer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::setProceduralLayer( cgUInt32 nLayerId, const cgLandscape::ProceduralLayerRef & Layer )
{
    // Validate
    if ( nLayerId >= mProceduralLayers.size() )
        return false;

    // Serialize as necessary.
    cgUInt32 nDBId = mProceduralLayers[nLayerId].layerId;
    if ( shouldSerialize() == true && nDBId != 0  )
    {
        // Begin a transaction that we can roll back.
        cgWorld * pWorld = mParentScene->getParentWorld();
        pWorld->beginTransaction( _T("setProceduralLayer") );

        // Update layer data.
        mUpdateProceduralLayer.bindParameter( 1, Layer.name );
        mUpdateProceduralLayer.bindParameter( 2, Layer.layerType.getReferenceId() );
        mUpdateProceduralLayer.bindParameter( 3, Layer.enableHeight );
        mUpdateProceduralLayer.bindParameter( 4, Layer.minimumHeight );
        mUpdateProceduralLayer.bindParameter( 5, Layer.maximumHeight );
        mUpdateProceduralLayer.bindParameter( 6, Layer.heightAttenuationBand );
        mUpdateProceduralLayer.bindParameter( 7, Layer.slopeAxis.x );
        mUpdateProceduralLayer.bindParameter( 8, Layer.slopeAxis.y );
        mUpdateProceduralLayer.bindParameter( 9, Layer.slopeAxis.z );
        mUpdateProceduralLayer.bindParameter( 10, Layer.slopeScale );
        mUpdateProceduralLayer.bindParameter( 11, Layer.slopeBias );
        mUpdateProceduralLayer.bindParameter( 12, Layer.invertSlope );
        mUpdateProceduralLayer.bindParameter( 13, Layer.weight );
        mUpdateProceduralLayer.bindParameter( 14, nDBId );
    
        // Execute
        if ( mUpdateProceduralLayer.step( true ) == false )
        {
            cgString strError;
            mUpdateProceduralLayer.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update procedural layer data '%i' for landscape '0x%x'. Error: %s\n"), nDBId, mLandscapeId, strError.c_str() );
            pWorld->rollbackTransaction( _T("setProceduralLayer") );
            return false;
        
        } // End if failed

        // Commit changes
        pWorld->commitTransaction( _T("setProceduralLayer") );

    } // End if !internal

    // Update internal array.
    mProceduralLayers[ nLayerId ] = Layer;
    mProceduralLayers[ nLayerId ].layerId = nDBId;

    // Re-batch procedural layers as required.
    batchProceduralDraws();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : updateProceduralLayers ()
/// <summary>
/// Replace procedural layer list with the newly specified version.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscape::updateProceduralLayers( const ProceduralLayerArray & aLayers )
{
    cgUInt32Array aInsertIds;

    // Insert the new object.
    if ( shouldSerialize() == true )
    {
        // Begin a transaction that we can roll back.
        cgWorld * pWorld = mParentScene->getParentWorld();
        pWorld->beginTransaction( _T("updateProceduralLayers") );

        // First, destroy old procedural layer data.
        prepareQueries();
        mDeleteProceduralLayers.bindParameter( 1, mLandscapeId );
        if ( mDeleteProceduralLayers.step( true ) == false )
        {
            cgString strError;
            mDeleteProceduralLayers.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to delete prior procedural layer data for landscape '0x%x'. Error: %s\n"), mLandscapeId, strError.c_str() );
            pWorld->rollbackTransaction( _T("updateProceduralLayers") );
            return;
        
        } // End if failed

        // Insert new layer data.
        for ( size_t i = 0; i < aLayers.size(); ++i )
        {
            const ProceduralLayerRef & Layer = aLayers[i];
            mInsertProceduralLayer.bindParameter( 1, mLandscapeId );
            mInsertProceduralLayer.bindParameter( 2, (cgUInt32)i );
            mInsertProceduralLayer.bindParameter( 3, Layer.name );
            mInsertProceduralLayer.bindParameter( 4, Layer.layerType.getReferenceId() );
            mInsertProceduralLayer.bindParameter( 5, Layer.enableHeight );
            mInsertProceduralLayer.bindParameter( 6, Layer.minimumHeight );
            mInsertProceduralLayer.bindParameter( 7, Layer.maximumHeight );
            mInsertProceduralLayer.bindParameter( 8, Layer.heightAttenuationBand );
            mInsertProceduralLayer.bindParameter( 9, Layer.slopeAxis.x );
            mInsertProceduralLayer.bindParameter( 10, Layer.slopeAxis.y );
            mInsertProceduralLayer.bindParameter( 11, Layer.slopeAxis.z );
            mInsertProceduralLayer.bindParameter( 12, Layer.slopeScale );
            mInsertProceduralLayer.bindParameter( 13, Layer.slopeBias );
            mInsertProceduralLayer.bindParameter( 14, Layer.invertSlope );
            mInsertProceduralLayer.bindParameter( 15, Layer.weight );
        
            // Execute
            if ( mInsertProceduralLayer.step( true ) == false )
            {
                cgString strError;
                mInsertProceduralLayer.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert procedural layer data into database for landscape '0x%x'. Error: %s\n"), mLandscapeId, strError.c_str() );
                pWorld->rollbackTransaction( _T("updateProceduralLayers") );
                return;
            
            } // End if failed

            // Record the most recent inserted identifier for this layer.
            aInsertIds.push_back( mInsertProceduralLayer.getLastInsertId() );

        } // Next layer

        // Commit changes
        pWorld->commitTransaction( _T("updateProceduralLayers") );

    } // End if !internal

    // Replace array and re-batch.
    mProceduralLayers = aLayers;
    batchProceduralDraws();

    // Store the inserted DB ids if recorded.
    if ( aInsertIds.empty() == false )
    {
        for ( size_t i = 0; i < mProceduralLayers.size(); ++i )
            mProceduralLayers[i].layerId = aInsertIds[i];
    
    } // End if inserted layers
}

//-----------------------------------------------------------------------------
// Name : getNormalTexture ( )
/// <summary>
/// Retrieve the normal texture used to store pre-computed terrain normals.
/// </summary>
//-----------------------------------------------------------------------------
cgTextureHandle cgLandscape::getNormalTexture( ) const
{
    return mNormalTexture;
}

//-----------------------------------------------------------------------------
// Name : isYVariant ( )
/// <summary>
/// Retrieve the flag that indicates if the landscape's internal spatial tree
/// records variable heights at each node.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscape::isYVariant( ) const
{
    return mYVTree;
}

/*///////////////////////////////////////////////////////////////////////////////
// cgLandscapeNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgLandscapeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeNode::cgLandscapeNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgSpatialTreeNode( nReferenceId, pScene )
{
    // Initialize variables to sensible defaults.
    m_strObjectClass = _T("Landscape");
}

//-----------------------------------------------------------------------------
//  Name : cgLandscapeNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeNode::cgLandscapeNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgSpatialTreeNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgSpatialTreeNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeNode::~cgLandscapeNode()
{
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose() (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgLandscapeNode::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    m_bDisposing = true;

    
    // Call base class implementation
    if ( bDisposeBase == true )
        cgSpatialTreeNode::dispose( true );
    else
        m_bDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : AllocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgLandscapeNode::AllocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgLandscapeNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : AllocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgLandscapeNode::AllocateClone( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform )
{
    return new cgLandscapeNode( nReferenceId, pScene, pInit, InitMethod, InitTransform );
}

//-----------------------------------------------------------------------------
//  Name : QueryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeNode::QueryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_LandscapeNode )
        return true;

    // Supported by base?
    return cgSpatialTreeNode::QueryReferenceType( Type );
}

//-----------------------------------------------------------------------------
// Name : UpdateObjectOwnership () (Virtual)
/// <summary>
/// Allows the spatial tree to determine if it can / wants to own the specified 
/// object and optionally inserts it into the relevant leaf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeNode::UpdateObjectOwnership( cgObjectNode * pNode, bool bTestOnly = false )
{
    // ToDo: Keep a list of all child spatial tree objects
    // and ask them to update ownership first.

    // Retrieve the world space bounding box
    cgBoundingBox Bounds = pNode->getBoundingBox( );

    // Find the leaves into which this object falls
    cgSceneLeafArray Leaves;
    if ( collectLeaves( Leaves, Bounds ) == false )
    {
        // Was not contained within any leaf, we can't take ownership.
        return cgSpatialTreeNode::UpdateObjectOwnership( pNode, bTestOnly );
        
    } // End if not contained
    else
    {
        // Attach to this spatial tree
        if ( bTestOnly == false )
        {
            pNode->SetSpatialTree( this );
            pNode->SetSpatialTreeLeaves( Leaves );

        } // End if !Test

        // We assumed ownership of the node
        return true;

    } // End if contained
}

//-----------------------------------------------------------------------------
// Name : getRayIntersect ()
/// <summary>
/// Retrieve the 'time' at which a ray intersects the terrain (if at all).
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeNode::getRayIntersect( const cgVector3 & vecOrigin, const cgVector3 & vecVelocity, cgFloat & t ) const
{
    return getRayIntersect( vecOrigin, vecVelocity, t, 1.0f );
}
bool cgLandscapeNode::getRayIntersect( const cgVector3 & vecOrigin, const cgVector3 & vecVelocity, cgFloat & t, cgFloat fAccuracy ) const
{
    // Transform input values into object space.
    cgMatrix mtxInv;
    cgMatrix::inverse( &mtxInv, CG_NULL, &getWorldTransform( false ) );
    cgVector3 vObjectOrigin, vObjectEnd = vecOrigin + vecVelocity;
    CGEVec3TransformCoord( &vObjectOrigin, &vecOrigin, &mtxInv );
    CGEVec3TransformCoord( &vObjectEnd, &vObjectEnd, &mtxInv );

    // Call referenced object implementations.
    bool bResult = ((cgLandscape*)m_pReferencedObject)->getRayIntersect( vObjectOrigin, vObjectEnd - vObjectOrigin, t );
    if ( bResult == false )
        return false;

    // Scale is not allowed on landscape, so we don't need to recompute the 't' value.
    // Simply return the intersection success.
    return true;
}

//-----------------------------------------------------------------------------
// Name : CanScale ( )
/// <summary>Determine if scaling of this node's transform is allowed.</summary>
//-----------------------------------------------------------------------------
bool cgLandscapeNode::CanScale( ) const
{
    // Landscape cannot be scaled.
    return false;
}*/

///////////////////////////////////////////////////////////////////////////////
// cgTerrainLOD Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgTerrainLOD() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgTerrainLOD::cgTerrainLOD( )
{
    // Initialize variables to sensible defaults
    interiorPrimitiveCount = 0;
}

//-----------------------------------------------------------------------------
// Name : ~cgTerrainLOD() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgTerrainLOD::~cgTerrainLOD()
{
    // Release index buffer
    indexBuffer.close();
    
    // dispose of arrays
    for ( cgInt i = 0; i < 4; ++i )
        skirts[i].levels.clear();
}

///////////////////////////////////////////////////////////////////////////////
// cgTerrainBlock Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgTerrainBlock() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgTerrainBlock::cgTerrainBlock( cgLandscape * pParent, cgUInt32 nBlockIndex, const TerrainSection & Info )
{
    // Copy over any required values
    mParent       = pParent;
    mSection      = Info;
    mBlockIndex   = nBlockIndex;
    mVisible      = false;

    // Clear remaining variables
    mBlockId          = 0;
    mOwnsHeightMap    = false;
    mHeightMap        = CG_NULL;
    mColorMap         = CG_NULL;
    mCurrentLOD       = -1;
    mVarianceCount    = 0;
    mBounds.reset();
    
    // Allocate and clear necessary storage arrays / management objects
    memset( mNeighbors, 0, 4 * sizeof(cgTerrainBlock*) );
    memset( mLODVariance, 0, cgLandscape::MaxLandscapeLOD * sizeof(cgFloat) );
    mTextureData = new cgLandscapeTextureData( mParent, this );
}

//-----------------------------------------------------------------------------
// Name : ~cgTerrainBlock() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgTerrainBlock::~cgTerrainBlock()
{
    // Release heightmap data if we own it.
    if ( mHeightMap != CG_NULL && mOwnsHeightMap == true )
        delete []mHeightMap;
    mHeightMap = CG_NULL;
    mOwnsHeightMap = false;

    // Release management objects
    delete mTextureData;
    mTextureData = CG_NULL;

    // Release managed resources
    mVertexBuffer.close();

    // Clear variables
    mColorMap = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : importBlock ()
/// <summary>
/// Import the terrain block based on the settings specified in the class 
/// constructor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::importBlock( )
{
    cgInt32 nHeightMapWidth, nHeightMapHeight, nBlockWidth, nBlockHeight;
    cgRect  rcBlock, rcTerrain, rcIntersect;

    // Get access to required systems
    cgResourceManager * pResources = mParent->getScene()->getResourceManager();

    // Calculate the intersection of the terrain total rectangle and the specified
    // rectangle to ensure that we are not attempting to read out of bounds.
    rcBlock      = mSection.blockBounds;
    rcTerrain    = cgRect( 0, 0, mSection.blockBounds.pitchX, mSection.blockBounds.pitchY );
    rcIntersect  = cgRect::intersect( rcTerrain, rcBlock );
    nBlockWidth  = rcBlock.width();
    nBlockHeight = rcBlock.height();

    // Ensure that the specified rectangle is valid in position and size.
    if ( rcIntersect != rcBlock || nBlockWidth < 2 || nBlockHeight < 2 )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Terrain block section specifies an invalid area of the height map (%i,%i,%i,%i).\n"),
                            rcBlock.left, rcBlock.top, rcBlock.right, rcBlock.bottom );
        return false;

    } // End if invalid call

    // Must be power of two in size, if this is not the case we cannot generate the correct LOD mappings
    if ( cgMathUtility::isPowerOfTwo( nBlockWidth - 1 ) == false ||
         cgMathUtility::isPowerOfTwo( nBlockHeight - 1 ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Terrain block section uses dimensions which are not compatible (%ix%i).\n"), rcBlock.width(), rcBlock.height() );
        return false;
    
    } // End if not power of two

    // Our block actually needs to have access to an extra 1 pixel border from 
    // the heightmap (for correct normal / interpolated height calculations)
    mManagedTerrainBounds = mSection.blockBounds;
    mManagedTerrainBounds.left   -= 1;
    mManagedTerrainBounds.right  += 1;
    mManagedTerrainBounds.top    -= 1;
    mManagedTerrainBounds.bottom += 1;

    // Intersect with the terrain rectangle to ensure we aren't out of bounds
    rcIntersect = cgRect( mManagedTerrainBounds.left, mManagedTerrainBounds.top, mManagedTerrainBounds.right, mManagedTerrainBounds.bottom );
    rcIntersect = cgRect::intersect( rcIntersect, rcTerrain );
    mManagedTerrainBounds.left   = rcIntersect.left;
    mManagedTerrainBounds.top    = rcIntersect.top;
    mManagedTerrainBounds.right  = rcIntersect.right;
    mManagedTerrainBounds.bottom = rcIntersect.bottom;
    
    // Cache width and height values for quick access
    nHeightMapWidth  = mManagedTerrainBounds.width();
    nHeightMapHeight = mManagedTerrainBounds.height();

    // Loading from file, or simply reading from an existing memory buffer?
    if ( mSection.heightMapBuffer == CG_NULL )
    {
        // Load internal heightmap data area
        cgInt16Array aHeightMap;
        cgHeightMap::loadRawSection( mSection.heightMapStream, rcTerrain.size(), cgHeightMap::Gray16, rcIntersect, aHeightMap );
        if ( aHeightMap.empty() == true )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to open specified terrain heightmap file '%s'.\n"), mSection.heightMapStream.getName().c_str() );
            return false;
        
        } // End if failed to load section
        
        // We own this heightmap memory buffer
        mHeightMap = new cgInt16[ aHeightMap.size() ];
        memcpy( mHeightMap, &aHeightMap[0], aHeightMap.size() );
        aHeightMap.clear();
        mOwnsHeightMap = true;

        // Set our active rectangle within the loaded heightmap data
        mActiveHeightMapBounds.left   = mSection.blockBounds.left - mManagedTerrainBounds.left;
        mActiveHeightMapBounds.top    = mSection.blockBounds.top  - mManagedTerrainBounds.top;
        mActiveHeightMapBounds.right  = mActiveHeightMapBounds.left + mSection.blockBounds.width();
        mActiveHeightMapBounds.bottom = mActiveHeightMapBounds.top + mSection.blockBounds.height();
        mActiveHeightMapBounds.pitchX = mManagedTerrainBounds.width();
        mActiveHeightMapBounds.pitchY = mManagedTerrainBounds.height();

    } // End if loading from file
    else
    {
        // Simply reference the heightmap
        mHeightMap     = mSection.heightMapBuffer;
        mOwnsHeightMap = false;

        // Our active rectangle is the same as that passed in (we're simply referencing the full heightmap)
        mActiveHeightMapBounds = mSection.blockBounds;

    } // End if using an existing memory buffer

    // Reference remaining properties.
    mColorMap = &mParent->getColorMap()[0];
    
    // Build the vertex buffer using our global optimizing lookup table
    if ( buildVertexBuffer( mParent->getMipLookUp() ) == false )
        return false;

    // Calculate the LOD variance values
    calculateLODVariance( );

    // Insert the block into the database.
    if ( mParent->shouldSerialize() == true )
    {
        // Store the data pertinant to this block's managed rectangle.
        // This potentially includes the 1 pixel border necessary for
        // maintaining neighboring pixels used during blending of
        // normal / interpolated height data.
        size_t nElements = mManagedTerrainBounds.width() * mManagedTerrainBounds.height();
        cgInt16Array  HeightData( nElements );
        cgUInt32Array ColorData( nElements );

        // Compute the array element offsets of the /managed/ map area
        // (not the active) so that we can correctly extract the vertex
        // data irrespective of whether or not the landscape is managing
        // a full or partial (per block) height and color map.
        cgInt32 nXBegin = (mManagedTerrainBounds.left - mSection.blockBounds.left) + mActiveHeightMapBounds.left;
        cgInt32 nZBegin = (mManagedTerrainBounds.top - mSection.blockBounds.top) + mActiveHeightMapBounds.top;
        cgInt32 nXEnd   = nXBegin + mManagedTerrainBounds.width();
        cgInt32 nZEnd   = nZBegin + mManagedTerrainBounds.height();

        // Extract data from the height and color maps.
        nElements = 0;
        for ( cgInt32 z = nZBegin; z < nZEnd; ++z )
        {
            for ( cgInt32 x = nXBegin; x < nXEnd; ++x )
            {
                // Calculate the map indices
                cgInt32 nElementIndex = x + z * mActiveHeightMapBounds.pitchX;
                
                // Copy values
                if ( mColorMap )
                    ColorData[ nElements ] = mColorMap[ nElementIndex ];
                HeightData[ nElements++ ] = mHeightMap[ nElementIndex ];

            } // Next Column

        } // Next Row
        
        // Build query
        prepareQueries();
        mInsertBlock.bindParameter( 1, mParent->getDatabaseId() );
        mInsertBlock.bindParameter( 2, mBlockIndex );
        mInsertBlock.bindParameter( 3, &HeightData[0], nElements * sizeof(cgInt16) );
        mInsertBlock.bindParameter( 4, &ColorData[0], nElements * sizeof(cgUInt32) );

        // Execute
        if ( mInsertBlock.step( true ) == false )
        {
            cgString strError;
            mInsertBlock.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for block index '%i' for landscape '0x%x' into database. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );
            return false;
        
        } // End if failed

        // Record the identifier of the block so that we can modify the database
        // entry as necessary.
        mBlockId = mInsertBlock.getLastInsertId();

        // Now we need to insert the LOD and height data into the database.
        cgInt8 nNumLODs = (cgInt8)mParent->getLODData().size();
        for ( cgInt8 i = 0; i < nNumLODs; ++i )
        {
            // Skip the lowest LOD for now since there is nothing to store 
            // (no variance distance). This may change later.
            if ( i == (nNumLODs - 1) )
                continue;
            
            // Start to build database entry. First, LOD identifier data.
            mInsertBlockLOD.bindParameter( 1, mBlockId );
            mInsertBlockLOD.bindParameter( 2, (cgUInt32)i );
            
            // All but the last LOD store a variance based distance.
            if ( i != (nNumLODs - 1) )
                mInsertBlockLOD.bindParameter( 3, mLODVariance[i] );
            else
                mInsertBlockLOD.bindParameter( 3, 0.0 );

            // Execute
            if ( mInsertBlockLOD.step( true ) == false )
            {
                cgString strError;
                mInsertBlockLOD.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert LOD data for block index '%i' for landscape '0x%x' into database. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );
                return false;
            
            } // End if failed
            
            // ToDo: 9999 - Retrieve LOD entry index so we can update the rows later?
        
        } // Next LOD

    } // End if serialize

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : loadBlock ()
/// <summary>
/// Load the terrain block from the database based on the supplied query.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::loadBlock( cgWorldQuery & BlockQuery )
{
    // Retrieve base block properties.
    BlockQuery.getColumn( _T("BlockId"), mBlockId );

    // Calculate the intersection of the terrain total rectangle and the specified
    // rectangle to ensure that we are not attempting to read out of bounds.
    cgRect  rcBlock      = mSection.blockBounds;
    cgRect  rcTerrain    = cgRect( 0, 0, mSection.blockBounds.pitchX, mSection.blockBounds.pitchY );
    cgRect  rcIntersect  = cgRect::intersect( rcTerrain, rcBlock );
    cgInt32 nBlockWidth  = rcBlock.width();
    cgInt32 nBlockHeight = rcBlock.height();

    // Ensure that the specified rectangle is valid in position and size.
    if ( rcIntersect != rcBlock || nBlockWidth < 2 || nBlockHeight < 2 )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Terrain block section specifies an invalid area of the height map (%i,%i,%i,%i).\n"),
                            rcBlock.left, rcBlock.top, rcBlock.right, rcBlock.bottom );
        return false;

    } // End if invalid call

    // Must be power of two in size, if this is not the case we cannot generate the correct LOD mappings
    if ( cgMathUtility::isPowerOfTwo( nBlockWidth - 1 ) == false ||
         cgMathUtility::isPowerOfTwo( nBlockHeight - 1 ) == false )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Terrain block section uses dimensions which are not compatible (%ix%i).\n"), rcBlock.width(), rcBlock.height() );
        return false;
    
    } // End if not power of two

    // Our block actually needs to have access to an extra 1 pixel border from 
    // the heightmap (for correct normal / interpolated height calculations)
    mManagedTerrainBounds = mSection.blockBounds;
    mManagedTerrainBounds.left   -= 1;
    mManagedTerrainBounds.right  += 1;
    mManagedTerrainBounds.top    -= 1;
    mManagedTerrainBounds.bottom += 1;

    // Intersect with the terrain rectangle to ensure we aren't out of bounds
    rcIntersect = cgRect( mManagedTerrainBounds.left, mManagedTerrainBounds.top, mManagedTerrainBounds.right, mManagedTerrainBounds.bottom );
    rcIntersect = cgRect::intersect( rcIntersect, rcTerrain );
    mManagedTerrainBounds.left   = rcIntersect.left;
    mManagedTerrainBounds.top    = rcIntersect.top;
    mManagedTerrainBounds.right  = rcIntersect.right;
    mManagedTerrainBounds.bottom = rcIntersect.bottom;
    
    // Cache width and height values for quick access
    cgInt32 nHeightMapWidth  = mManagedTerrainBounds.width();
    cgInt32 nHeightMapHeight = mManagedTerrainBounds.height();

    // ToDo: 9999 - Following logic to be applied when not keeping full height map resident.
    /*// Loading from file, or simply reading from an existing memory buffer?
    if ( mSection.heightMapBuffer == CG_NULL )
    {
        // Load internal heightmap data area
        cgInt16Array aHeightMap;
        cgHeightMap::loadRawSection( mSection.heightMapStream, rcTerrain.size(), cgHeightMap::Gray16, rcIntersect, aHeightMap );
        if ( aHeightMap.empty() == true )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to open specified terrain heightmap file '%s'.\n"), mSection.heightMapStream.getName().c_str() );
            return false;
        
        } // End if failed to load section
        
        // We own this heightmap memory buffer
        mHeightMap = new cgInt16[ aHeightMap.size() ];
        memcpy( mHeightMap, &aHeightMap[0], aHeightMap.size() );
        aHeightMap.clear();
        mOwnsHeightMap = true;

        // Set our active rectangle within the loaded heightmap data
        mActiveHeightMapBounds.left   = mSection.blockBounds.left - mManagedTerrainBounds.left;
        mActiveHeightMapBounds.top    = mSection.blockBounds.top  - mManagedTerrainBounds.top;
        mActiveHeightMapBounds.right  = mActiveHeightMapBounds.left + mSection.blockBounds.width();
        mActiveHeightMapBounds.bottom = mActiveHeightMapBounds.top + mSection.blockBounds.height();
        mActiveHeightMapBounds.pitchX = mManagedTerrainBounds.width();
        mActiveHeightMapBounds.pitchY = mManagedTerrainBounds.height();

    } // End if loading from file
    else*/
    {
        // Simply reference the heightmap
        mHeightMap     = mSection.heightMapBuffer;
        mOwnsHeightMap = false;

        // And the same for the color map.
        mColorMap = &mParent->getColorMap()[0];

        // Our active rectangle is the same as that passed in (we're simply referencing the full heightmap)
        mActiveHeightMapBounds = mSection.blockBounds;

        // Retrieve the height data from the database.
        cgUInt32 nHeightMapSize = 0;
        cgInt16 * pHeightData = CG_NULL;
        BlockQuery.getColumn( _T("HeightData"), (void**)&pHeightData, nHeightMapSize );
        
        // Retrieve the color data from the database
        cgUInt32 nColorMapSize = 0;
        cgUInt32 * pColorData = CG_NULL;
        BlockQuery.getColumn( _T("ColorData"), (void**)&pColorData, nColorMapSize );

        // ToDo: 9999 - Can optimize by block copying a row at a time
        //       in the full height map case, or storing the entire
        //       heightmap in local mode.
        // Extract data from the height and color maps.
        if ( pColorData )
        {
            for ( cgInt32 z = 0; z < nHeightMapHeight; ++z )
            {
                for ( cgInt32 x = 0; x < nHeightMapWidth; ++x )
                {
                    // Calculate the map indices
                    cgInt32 nSrcIndex = x + z * nHeightMapWidth;
                    cgInt32 nDstIndex = (x + mManagedTerrainBounds.left) + (z + mManagedTerrainBounds.top) * mManagedTerrainBounds.pitchX;
                    
                    // Copy values
                    mHeightMap[ nDstIndex ] = pHeightData[ nSrcIndex ];
                    mColorMap[ nDstIndex ] = pColorData[ nSrcIndex ];
                    
                } // Next Column

            } // Next Row
        
        } // End if has color data
        else
        {
            for ( cgInt32 z = 0; z < nHeightMapHeight; ++z )
            {
                for ( cgInt32 x = 0; x < nHeightMapWidth; ++x )
                {
                    // Calculate the map indices
                    cgInt32 nSrcIndex = x + z * nHeightMapWidth;
                    cgInt32 nDstIndex = (x + mManagedTerrainBounds.left) + (z + mManagedTerrainBounds.top) * mManagedTerrainBounds.pitchX;
                    
                    // Copy values
                    mHeightMap[ nDstIndex ] = pHeightData[ nSrcIndex ];
                    
                } // Next Column

            } // Next Row

        } // End if no color data.

    } // End if using an existing memory buffer
    
    // Build the vertex buffer using our global optimizing lookup table
    if ( buildVertexBuffer( mParent->getMipLookUp() ) == false )
        return false;

    // Load the LOD variance values.
    prepareQueries();
    mVarianceCount = mParent->getLODData().size() - 1;
    mLoadBlockLODs.bindParameter( 1, mBlockId );
    if ( mLoadBlockLODs.step() == false )
    {
        // Log any error.
        cgString strError;
        if ( mLoadBlockLODs.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve LOD data for block %i in landscape '0x%x'. World database has potentially become corrupt.\n"), mBlockIndex, mParent->getDatabaseId() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve LOD data for block %i in landscape '0x%x'. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );

        // Release any pending read operation.
        mLoadBlockLODs.reset();
        return false;
    
    } // End if failed

    // Process LOD data.
    for ( ; mLoadBlockLODs.nextRow(); )
    {
        cgInt32 nLODIndex = 0;
        mLoadBlockLODs.getColumn( _T("LevelIndex"), nLODIndex );
        mLoadBlockLODs.getColumn( _T("Variance"), mLODVariance[nLODIndex] );
        
    } // Next Block

    // We're done with the LOD read query.
    mLoadBlockLODs.reset();

    // Load texture data (if any).
    return mTextureData->loadLayers();
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParent->getScene()->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertBlock.isPrepared() == false )
            mInsertBlock.prepare( pWorld, _T("INSERT INTO 'Landscapes::Blocks' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( mInsertBlockLOD.isPrepared() == false )
            mInsertBlockLOD.prepare( pWorld, _T("INSERT INTO 'Landscapes::BlockLOD' VALUES(NULL,?1,?2,?3)"), true );
        if ( mUpdateBlockHeights.isPrepared() == false )
            mUpdateBlockHeights.prepare( pWorld, _T("UPDATE 'Landscapes::Blocks' SET HeightData=?1 WHERE BlockId=?2"), true );
        if ( mDeleteBlockLODs.isPrepared() == false )
            mDeleteBlockLODs.prepare( pWorld, _T("DELETE FROM 'Landscapes::BlockLOD' WHERE BlockId=?1"), true );
        
    } // End if sandbox

    // Read queries
    if ( mLoadBlockLODs.isPrepared() == false )
        mLoadBlockLODs.prepare( pWorld, _T("SELECT * FROM 'Landscapes::BlockLOD' WHERE BlockId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : dataUpdated ()
/// <summary>
/// The heightmap data has been updated, we should rebuild any data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::dataUpdated( )
{
    // Parent allows dynamic data updates?
    if ( mParent == CG_NULL || (mParent->getFlags() & cgLandscapeFlags::Dynamic) != cgLandscapeFlags::Dynamic )
        return false;

    // First update the vertex buffer
    if ( updateVertexBuffer( mParent->getMipLookUp() ) == false )
        return false;

    // Recalculate the LOD variances
    calculateLODVariance();

    // Insert the block into the database.
    if ( mParent->shouldSerialize() == true )
    {
        prepareQueries();

        // Begin a transaction for optimization and rollback purposes.
        cgWorld * pWorld = mParent->getScene()->getParentWorld();
        pWorld->beginTransaction( _T("blockDataUpdated") );

        // Update the data pertinent to this block's managed rectangle.
        // This potentially includes the 1 pixel border necessary for
        // maintaining neighboring pixels used during blending of
        // normal / interpolated height data.
        size_t nElements = mManagedTerrainBounds.width() * mManagedTerrainBounds.height();
        cgInt16Array HeightData( nElements );

        // Compute the array element offsets of the /managed/ map area
        // (not the active) so that we can correctly extract the vertex
        // data irrespective of whether or not the landscape is managing
        // a full or partial (per block) height and color map.
        cgInt32 nXBegin = (mManagedTerrainBounds.left - mSection.blockBounds.left) + mActiveHeightMapBounds.left;
        cgInt32 nZBegin = (mManagedTerrainBounds.top - mSection.blockBounds.top) + mActiveHeightMapBounds.top;
        cgInt32 nXEnd   = nXBegin + mManagedTerrainBounds.width();
        cgInt32 nZEnd   = nZBegin + mManagedTerrainBounds.height();

        // ToDo: 9999 - Can optimize by block copying a row at a time
        //       in the full height map case, or storing the entire
        //       heightmap in local mode.
        // Extract data from the height map.
        nElements = 0;
        for ( cgInt32 z = nZBegin; z < nZEnd; ++z )
        {
            for ( cgInt32 x = nXBegin; x < nXEnd; ++x )
                HeightData[ nElements++ ] = mHeightMap[ x + z * mActiveHeightMapBounds.pitchX ];
                
        } // Next Row
        
        // Build query
        mUpdateBlockHeights.bindParameter( 1, &HeightData[0], nElements * sizeof(cgInt16) );
        mUpdateBlockHeights.bindParameter( 2, mBlockId );

        // Execute
        if ( mUpdateBlockHeights.step( true ) == false )
        {
            cgString strError;
            mUpdateBlockHeights.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update height data for block index '%i' within landscape '0x%x'. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );
            pWorld->rollbackTransaction( _T("blockDataUpdated") );
            return false;
        
        } // End if failed

        // Since LOD values have potentially been updated, next we need to
        // replace any existing variances with new ones. First delete
        // the existing.
        mDeleteBlockLODs.bindParameter( 1, mBlockId );
        if ( mDeleteBlockLODs.step( true ) == false )
        {
            cgString strError;
            mUpdateBlockHeights.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to delete prior LOD variance distance values for block index '%i' within landscape '0x%x'. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );
            pWorld->rollbackTransaction( _T("blockDataUpdated") );
            return false;

        } // End if failed

        // Re-insert variance data.
        cgInt8 nNumLODs = (cgInt8)mParent->getLODData().size();
        for ( cgInt8 i = 0; i < nNumLODs; ++i )
        {
            // Skip the lowest LOD for now since there is nothing to store 
            // (no variance distance). This may change later.
            if ( i == (nNumLODs - 1) )
                continue;
            
            // Start to build database entry. First, LOD identifier data.
            mInsertBlockLOD.bindParameter( 1, mBlockId );
            mInsertBlockLOD.bindParameter( 2, (cgUInt32)i );
            
            // All but the last LOD store a variance based distance.
            if ( i != (nNumLODs - 1) )
                mInsertBlockLOD.bindParameter( 3, mLODVariance[i] );
            else
                mInsertBlockLOD.bindParameter( 3, 0.0 );

            // Execute
            if ( mInsertBlockLOD.step( true ) == false )
            {
                cgString strError;
                mInsertBlockLOD.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert LOD data for block index '%i' for landscape '0x%x' into database. Error: %s\n"), mBlockIndex, mParent->getDatabaseId(), strError.c_str() );
                pWorld->rollbackTransaction( _T("blockDataUpdated") );
                return false;
            
            } // End if failed
        
        } // Next LOD

        // Update completed.
        pWorld->commitTransaction( _T("blockDataUpdated") );

    } // End if serialize

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : buildVertexBuffer () (Protected)
/// <summary>
/// Construct the vertex buffer for this terrain block. Use the mip look up 
/// table if provided.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::buildVertexBuffer( const cgInt32Array & aMipLookUp )
{
    // Get access to required systems
    cgResourceManager * pResources = mParent->getScene()->getResourceManager();

    // Compute the overall size of the vertex buffer
    cgUInt32 nBufferLength = mActiveHeightMapBounds.width() * mActiveHeightMapBounds.height() * sizeof(cgTerrainVertex);
    
    // Allocate a new vertex buffer of the correct size
    cgVertexFormat * pFormat = cgVertexFormat::formatFromDeclarator(cgTerrainVertex::declarator);
    pResources->createVertexBuffer( &mVertexBuffer, nBufferLength, cgBufferUsage::WriteOnly, pFormat, cgMemoryPool::Managed, cgDebugSource() );
    if ( mVertexBuffer.isValid() == false )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to allocate terrain block vertex buffer of size %i bytes.\n"), nBufferLength );
        return false;
    
    } // End if failed to allocate VB

    // Populate the vertex buffer
    return updateVertexBuffer( aMipLookUp );
}

//-----------------------------------------------------------------------------
// Name : updateVertexBuffer () (Protected)
/// <summary>
/// Update the contents of the vertex buffer based on the heightmap.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::updateVertexBuffer( const cgInt32Array & aMipLookUp )
{
    cgArray<cgTerrainVertex> Vertices(mActiveHeightMapBounds.width() * mActiveHeightMapBounds.height());

    // Optimization: Record height map size / scale.
    cgVector3 vecScale       = mParent->getTerrainScale();
    cgVector3 vecOffset      = mParent->getTerrainOffset();
    cgFloat   fQuadsWide     = (cgFloat)mParent->getHeightMapSize().width - 1;
    cgFloat   fQuadsHigh     = (cgFloat)mParent->getHeightMapSize().height - 1;

    // Reset the bounding box
    mBounds.min = cgVector3( (cgFloat)mSection.blockBounds.left * vecScale.x,  FLT_MAX, (cgFloat)(mSection.blockBounds.bottom - 1) * -vecScale.z );
    mBounds.max = cgVector3( (cgFloat)(mSection.blockBounds.right - 1) * vecScale.x, -FLT_MAX, (cgFloat)mSection.blockBounds.top * -vecScale.z );
    mBounds.min += vecOffset;
    mBounds.max += vecOffset;

    // Output vertex buffer data.
    cgInt32 nVBIndex, nCounter = 0, nHMIndex = mActiveHeightMapBounds.left + mActiveHeightMapBounds.top * mActiveHeightMapBounds.pitchX;
    for ( cgInt32 z = mSection.blockBounds.top; z < mSection.blockBounds.bottom; ++z, nHMIndex += mActiveHeightMapBounds.pitchX - mActiveHeightMapBounds.width() )
    {
        // Loop through the row
        for ( cgInt32 x = mSection.blockBounds.left; x < mSection.blockBounds.right; ++x, ++nHMIndex )
        {
            // Find the correct vertex buffer index from the mip lookup
            if ( !aMipLookUp.empty() )
                nVBIndex = aMipLookUp[ nCounter++ ];
            else
                nVBIndex = nCounter++;

            // Calculate vertex position
            cgTerrainVertex & v = Vertices[nVBIndex];
            v.position.x =  (cgFloat)x * vecScale.x;
            v.position.y =  (cgFloat)mHeightMap[ nHMIndex ] * vecScale.y;
            v.position.z = -(cgFloat)z * vecScale.z;
            v.position  += vecOffset;
            
            // Set additional vertex information
            v.color = (mColorMap != CG_NULL) ? mColorMap[ nHMIndex ] : 0xFFFFFFFF;
            
            // Compute normal and base texture coordinates if needed
            v.normal = getHeightMapNormal( x, z );
            /*v.TexBase.x = (cgFloat)x / fHeightMapWidth;
            v.TexBase.y = (cgFloat)z / fHeightMapHeight;*/
            
            // Update bounding box
            if ( v.position.y < mBounds.min.y ) mBounds.min.y = v.position.y;
            if ( v.position.y > mBounds.max.y ) mBounds.max.y = v.position.y;
            
        } // Next Column
    
    } // Next Row
    
    // Grow bounding box on the Y axis to ensure we have no degenerates
    mBounds.min.y -= 1.0f;
    mBounds.max.y += 1.0f;

    // Populate the hardware vertex buffer.
    cgVertexBuffer * pBuffer = mVertexBuffer.getResource();
    if ( !pBuffer || !pBuffer->updateBuffer( 0, 0, &Vertices.front() ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to update terrain block vertex buffer.\n") );
        return false;
    
    } // End if failed
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : calculateLODVariance () (Protected)
// Desc : Calculate the variance / error metric for each LOD which describes
//        the level of error which will be caused by switching to a lower LOD.
//-----------------------------------------------------------------------------
void cgTerrainBlock::calculateLODVariance( )
{
    cgInt32     i, x, z, nCurrentStep, nLowerStep;
    cgUInt32    nHeightIndex;
    cgFloat     fHeight, fMaxVariance, fVariance;
    cgFloat     fCurrentLOD, fPreviousLOD;

    // Retrieve the Y scale of the terrain so we can calculate an accurate variance
    const cgVector3 & vecScale = mParent->getTerrainScale();

    // Loop through each possible level of detail except the lowest in order
    // to populate this block's variance distance storage array.
    mVarianceCount = mParent->getLODData().size() - 1;
    for ( i = 0; i < mVarianceCount; ++i )
    {
        // Calculate the step values used to iterate through the heightmap
        nCurrentStep = 1 << i;
        nLowerStep   = 2 << i;

        // Calculate the maximum variance for each height value in relation to the LOD above.
        // An individual variance is calculated by computing the average height between
        // two edge points (for each edge of the quad) of the lower level of detail, and 
        // comparing the difference in that 'guess' against the actual height of the same
        // point in the higher of the two levels of detail.
        fMaxVariance = 0.0f;
        for ( z = mActiveHeightMapBounds.top; z < mActiveHeightMapBounds.bottom - 1; z += nLowerStep )
        {
            for ( x = mActiveHeightMapBounds.left; x < mActiveHeightMapBounds.right - 1; x += nLowerStep )
            {
                // Calculate initial heightmap point for easy access
                nHeightIndex = x + z * mActiveHeightMapBounds.pitchX;
                
                // Top of Quad (Left to Right)
                fHeight   = (((cgFloat)mHeightMap[ nHeightIndex + 0] + (cgFloat)mHeightMap[ nHeightIndex + nLowerStep ]) * vecScale.y) * 0.5f;
                fVariance = fabsf( ((cgFloat)mHeightMap[ nHeightIndex + nCurrentStep ] * vecScale.y) - fHeight );
                if ( fVariance > fMaxVariance ) fMaxVariance = fVariance;

                // Right of Quad (Top to Bottom)
                fHeight   = (((cgFloat)mHeightMap[ nHeightIndex + nLowerStep ] + (cgFloat)mHeightMap[ nHeightIndex + nLowerStep + (nLowerStep * mActiveHeightMapBounds.pitchX) ]) * vecScale.y) * 0.5f;
                fVariance = fabsf( ((cgFloat)mHeightMap[ nHeightIndex + nLowerStep + (nCurrentStep * mActiveHeightMapBounds.pitchX) ] * vecScale.y) - fHeight );
                if ( fVariance > fMaxVariance ) fMaxVariance = fVariance;

                // Bottom of Quad (Left to Right)
                fHeight   = (((cgFloat)mHeightMap[ nHeightIndex + nLowerStep * mActiveHeightMapBounds.pitchX ] + (cgFloat)mHeightMap[ nHeightIndex + nLowerStep + (nLowerStep * mActiveHeightMapBounds.pitchX) ]) * vecScale.y) * 0.5f;
                fVariance = fabsf( ((cgFloat)mHeightMap[ nHeightIndex + nCurrentStep + (nLowerStep * mActiveHeightMapBounds.pitchX) ] * vecScale.y) - fHeight );
                if ( fVariance > fMaxVariance ) fMaxVariance = fVariance;

                // Left of Quad (Top to Bottom)
                fHeight   = (((cgFloat)mHeightMap[ nHeightIndex + 0] + (cgFloat)mHeightMap[ nHeightIndex + nLowerStep * mActiveHeightMapBounds.pitchX ]) * vecScale.y) * 0.5f;
                fVariance = fabsf( ((cgFloat)mHeightMap[ nHeightIndex + nCurrentStep * mActiveHeightMapBounds.pitchX ] * vecScale.y) - fHeight );
                if ( fVariance > fMaxVariance ) fMaxVariance = fVariance;

                // Center of Quad (Diagonal top left to bottom right)
                fHeight   = (((cgFloat)mHeightMap[ nHeightIndex + 0] + (cgFloat)mHeightMap[ nHeightIndex + nLowerStep + (nLowerStep * mActiveHeightMapBounds.pitchX) ]) * vecScale.y) * 0.5f;
                fVariance = fabsf( ((cgFloat)mHeightMap[ nHeightIndex + nCurrentStep + (nCurrentStep * mActiveHeightMapBounds.pitchX) ] * vecScale.y) - fHeight );
                if ( fVariance > fMaxVariance ) fMaxVariance = fVariance;

            } // Next Column

        } // Next Row

        // Store the terrain variance value
        mLODVariance[ i ] = fMaxVariance;

        // If this variance is smaller than that of the level of detail prior to this
        // then just set our variance to the same value
        if ( i > 0 && fPreviousLOD > fMaxVariance )
            mLODVariance[ i ] = fPreviousLOD;

        // Record LOD for next iteration
        fPreviousLOD = mLODVariance[ i ];

    } // Next detail level

    // Perform final processing on the variance values
    for ( i = 0; i < mVarianceCount; ++i )
    {
        fCurrentLOD = mLODVariance[ i ];

        // Convert the variance into a distance
        // ToDo: Remove commented
        //fCurrentLOD *= Vector3( vecScale.x, 0, vecScale.z ).Length() * (cgFloat)(1 << i);
        //fCurrentLOD *= (min(mActiveHeightMapBounds.width(),mActiveHeightMapBounds.height()) - 1) * (1 << i);
        fCurrentLOD *= 128 * (1 << i);
        
        // Store the SQUARED variance distance value so that we can use this directly when
        // used in a comparison with a squared camera distance (prevents the sqrt).
        mLODVariance[i] = fCurrentLOD * fCurrentLOD;
        
    } // Next detail level

}

//-----------------------------------------------------------------------------
// Name : draw()
/// <summary>
/// Render this terrain block.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::draw( cgRenderDriver * pDriver, RenderMode Mode )
{
    cgInt32  i, nNeighborLOD, nVertexCount, nIndexStart, nPrimitiveCount;
    cgUInt32 nTotalPrimitives = 0;

    // Validate requirements
    cgAssert( mParent && mCurrentLOD >= 0 && mCurrentLOD < (cgInt32)mParent->getLODData().size() );

    // If we're rendering in painted mode but have no blend map, bail immediately.
    if ( Mode == Painted && mTextureData->isEmpty() )
        return;

    // Retrieve the LOD information from the parent landscape
    cgTerrainLOD * pLODData = mParent->getLODData()[ mCurrentLOD ];
    if ( !pLODData ) return;

    // Setup streams ready for rendering (vertex format set by parent)
    pDriver->setStreamSource( 0, mVertexBuffer );
    pDriver->setIndices( pLODData->indexBuffer );

    // Compute the total number of vertices we need to use from the VB for this detail level
    nVertexCount = (((mActiveHeightMapBounds.width() - 1) >> mCurrentLOD) + 1) * 
                   (((mActiveHeightMapBounds.height() - 1) >> mCurrentLOD) + 1);

    // Render using painted shaders?
    if ( Mode == Painted && !mTextureData->beginDraw( pDriver ) )
        return;

    // Iterate as often as is necessary.
    for ( ;; )
    {
        // Break out if all layers have been processed.
        if ( Mode == Painted && !mTextureData->beginDrawPass( pDriver ) )
            break;

        // Render, starting with the interior portion of the block at this LOD
        nIndexStart     = 0;
        nPrimitiveCount = pLODData->interiorPrimitiveCount;

        // Loop through each connecting edge piece
        for ( i = 0; i < 4; ++i )
        {
            // Initialize neighbor LOD level to our own in case it is CG_NULL
            nNeighborLOD = mCurrentLOD;
            
            // Retrieve the neighboring block on this edge
            cgTerrainBlock * pNeighbor = mNeighbors[ i ];

            // Retrieve the LOD for the neighbor (if it exists) ensuring we never select a HIGHER LOD than our own
            // (remember that a higher LOD has a smaller index value, which is why we use 'max' here rather than 'min')
            if ( pNeighbor )
                nNeighborLOD = max( mCurrentLOD, pNeighbor->getCurrentLOD() );
            
            // Finally, transform this value into a valid index for our skirt data array
            nNeighborLOD = nNeighborLOD - mCurrentLOD;
            cgTerrainLOD::BlockSkirt::LODLevel & LODConnect = pLODData->skirts[i].levels[nNeighborLOD];

            // If this connecting piece just runs on from the previous range we selected in the index buffer,
            // just increment the primitive count. Otherwise, render the previous index buffer
            // range, and start building again.
            if ( LODConnect.indexStart == (nIndexStart + (nPrimitiveCount * 3)) )
            {
                nPrimitiveCount += LODConnect.primitiveCount;
            
            } // End if range follows on
            else
            {
                // Draw the previously computed range
                if ( nPrimitiveCount > 0 )
                    pDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, 0, nVertexCount, nIndexStart, nPrimitiveCount );

                // Statistics gathering.
                nTotalPrimitives += nPrimitiveCount;

                // Start a new index buffer range
                nIndexStart     = LODConnect.indexStart;
                nPrimitiveCount = LODConnect.primitiveCount;
            
            } // End if range does NOT follow on
        
        } // Next edge

        // Render any remaining geometry
        if ( nPrimitiveCount > 0 )
            pDriver->drawIndexedPrimitive( cgPrimitiveType::TriangleList, 0, 0, nVertexCount, nIndexStart, nPrimitiveCount );

        // Statistics gathering.
        nTotalPrimitives += nPrimitiveCount;

        // Are we done?
        if ( Mode == Painted )
        {
            mTextureData->endDrawPass( pDriver );

        } // End if textured
        else
        {
            // Just bail, parent is handling passes.
            break;
        
        } // End if wire

    } // Next draw Pass
}

//-----------------------------------------------------------------------------
// Name : calculateLOD ()
/// <summary>
/// Calculates the current LOD for the block based on distance.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::calculateLOD( cgCameraNode * pCamera )
{
    return calculateLOD( pCamera, 1.0f, 0 );
}
void cgTerrainBlock::calculateLOD( cgCameraNode * pCamera, cgFloat fTerrainDetail )
{
    return calculateLOD( pCamera, fTerrainDetail, 0 );
}
void cgTerrainBlock::calculateLOD( cgCameraNode * pCamera, cgFloat fTerrainDetail, cgInt32 nLODAdjust )
{
    cgInt32   i, nLOD;
    cgFloat   fDistance;
    cgVector3 vecVelocity, vecCameraPos;

    // Calculate ray from camera to bounding box center
    vecCameraPos = pCamera->getPosition();
    vecVelocity  = mBounds.getCenter() - vecCameraPos;

    // Calculate the intersection point with the block's bounding box to give ray's delta to this block's exterior
    cgCollision::rayIntersectAABB( vecCameraPos, vecVelocity, mBounds, fDistance, false,
                                  (mParent->getFlags() & cgLandscapeFlags::LODIgnoreY) == cgLandscapeFlags::LODIgnoreY,
                                  false, false );
    
    // Convert to a squared distance (remember LOD variance was squared so we don't have to perform the sqrt here).
    fDistance = cgVector3::lengthSq( vecVelocity * fDistance );

    // Ensure distance is never equal to 0 so we use the lowest LOD necessary when we're in a completely flat block
    if ( fDistance < 1.0f )
        fDistance = 1.0f;
	
    // Select the best LOD based on the variance
    nLOD = 0;
    for ( i = 0; i < mVarianceCount; ++i )
    {
        if ( fDistance > mLODVariance[i] * fTerrainDetail )
            nLOD = i + 1;

    } // Next level of detail

    // Adjust level of detail as requested
    nLOD = max( 0, min( mVarianceCount, nLOD + nLODAdjust ) );

    // Commit the LOD value
    setCurrentLOD( nLOD );
}

//-----------------------------------------------------------------------------
// Name : getHeightMapNormal ()
/// <summary>
/// Retrieves the normal at this position in the heightmap.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 cgTerrainBlock::getHeightMapNormal( cgInt32 x, cgInt32 z ) const
{
	cgVector3 vecNormal, vecEdge1, vecEdge2, vecScale, vecSum( 0, 0, 0 );
    cgFloat   yNorth, yEast, ySouth, yWest, yCenter, fCount = 0.0f;

    // Transform X/Z values into our active rectangle.
    x -= mSection.blockBounds.left;
    z -= mSection.blockBounds.top;
    x += mActiveHeightMapBounds.left;
    z += mActiveHeightMapBounds.top;

    // Retrieve the correct index into the heightmap
    cgInt32 nPitch   = mActiveHeightMapBounds.pitchX;
    cgInt32 nHMIndex = x + z * nPitch;
    
    // Get the various height values
    vecScale = mParent->getTerrainScale();
    yCenter = (cgFloat)mHeightMap[ nHMIndex ] * vecScale.y;
    if ( z > mManagedTerrainBounds.top        ) yNorth  = (cgFloat)mHeightMap[ nHMIndex - nPitch ] * vecScale.y;
    if ( x < mManagedTerrainBounds.right - 1  ) yEast   = (cgFloat)mHeightMap[ nHMIndex + 1 ] * vecScale.y;
    if ( z < mManagedTerrainBounds.bottom - 1 ) ySouth  = (cgFloat)mHeightMap[ nHMIndex + nPitch ] * vecScale.y;
    if ( x > mManagedTerrainBounds.left       ) yWest   = (cgFloat)mHeightMap[ nHMIndex - 1 ] * vecScale.y;

    // Top Right Quadrant
    if ( x < mManagedTerrainBounds.right - 1 && z > mManagedTerrainBounds.top  )
    {
	    // Compute normal
	    vecEdge1 = cgVector3( 0.0f, yNorth - yCenter, vecScale.z );
	    vecEdge2 = cgVector3( vecScale.x, yEast - yCenter, 0.0f );
        cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Bottom Right Quadrant
    if ( x < mManagedTerrainBounds.right - 1 && z < mManagedTerrainBounds.bottom - 1 )
    {
        // Compute normal
	    vecEdge1 = cgVector3( vecScale.x, yEast - yCenter, 0.0f );
        vecEdge2 = cgVector3( 0.0f, ySouth - yCenter, -vecScale.z );
        cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Bottom Left Quadrant
    if ( x > mManagedTerrainBounds.left && z < mManagedTerrainBounds.bottom - 1 )
    {
        // Compute normal
	    vecEdge1 = cgVector3( 0.0f, ySouth - yCenter, -vecScale.z );
	    vecEdge2 = cgVector3( -vecScale.x, yWest - yCenter, 0.0f );
	    cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds

    // Top Left Quadrant
    if ( x > mManagedTerrainBounds.left && z > mManagedTerrainBounds.top )
    {
        // Compute normal
        vecEdge1 = cgVector3( -vecScale.x, yWest - yCenter, 0.0f );
	    vecEdge2 = cgVector3( 0.0f, yNorth - yCenter, vecScale.z );
	    cgVector3::normalize( vecNormal, *cgVector3::cross( vecNormal, vecEdge1, vecEdge2 ) );
        vecSum += vecNormal;
        fCount += 1.0f;
    
    } // End if in bounds
	
    // Valid?
    if ( fCount == 0.0f ) 
        return cgVector3( 0.0f, 1.0f, 0.0f );

    // Return the averaged normal
	return (vecSum / fCount);
}

//-----------------------------------------------------------------------------
// Name : getTerrainHeight ()
/// <summary>
/// Retrieve the height of the terrain at the specified location.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgTerrainBlock::getTerrainHeight( cgFloat fX, cgFloat fZ ) const
{
    return getTerrainHeight( fX, fZ, false );
}
cgFloat cgTerrainBlock::getTerrainHeight( cgFloat fX, cgFloat fZ, bool bAccountForLOD ) const
{
    cgFloat fTopLeft, fTopRight, fBottomLeft, fBottomRight;

    // ToDo: Account for LOD

    // Retrieve terrain scale from parent
    const cgVector3 & vecScale  = mParent->getTerrainScale();
    const cgVector3 & vecOffset = mParent->getTerrainOffset();

    // Adjust Input Values
    fX =  (fX - vecOffset.x) / vecScale.x;
    fZ = -(fZ - vecOffset.z) / vecScale.z;

    // Make sure we are not out of bounds
    if ( fX < mSection.blockBounds.left || fZ < mSection.blockBounds.top || 
         fX >= mSection.blockBounds.right || fZ >= mSection.blockBounds.bottom )
         return -1.0f;	

    // First retrieve the Heightmap Points
    cgInt32 iX = (cgInt32)fX;
    cgInt32 iZ = (cgInt32)fZ;
	
    // Calculate the remainder (percent across quad)
    cgFloat fPercentX = fX - (cgFloat)iX;
    cgFloat fPercentZ = fZ - (cgFloat)iZ;

    // ToDo: Remove
    /*// First retrieve the height of each point in the dividing edge
    fTopRight   = (cgFloat)getHeightMapHeight( iX + 1, iZ ) * vecScale.y;
    fBottomLeft = (cgFloat)getHeightMapHeight( iX, iZ + 1 ) * vecScale.y;

    // Calculate which triangle of the quad are we in ?
    if ( fPercentX < (1.0f - fPercentZ)) 
    {
        fTopLeft = (cgFloat)getHeightMapHeight( iX, iZ ) * vecScale.y;
        fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
    
    } // End if left Triangle
    else
    {
        fBottomRight = (cgFloat)getHeightMapHeight( iX + 1, iZ + 1 ) * vecScale.y;
        fTopLeft = fTopRight + (fBottomLeft - fBottomRight);

    } // End if Right Triangle*/

    // First retrieve the height of each point in the dividing edge
    fTopLeft     = (cgFloat)getHeightMapHeight( iX, iZ ) * vecScale.y;
    fBottomRight = (cgFloat)getHeightMapHeight( iX + 1, iZ + 1 ) * vecScale.y;

    // Which triangle of the quad are we in ?
    if ( fPercentX < fPercentZ )
    {
        fBottomLeft = (cgFloat)getHeightMapHeight( iX, iZ + 1 ) * vecScale.y;
	    fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
    
    } // End if left Triangle
    else
    {
        fTopRight   = (cgFloat)getHeightMapHeight( iX + 1, iZ ) * vecScale.y;
	    fBottomLeft = fTopLeft + (fBottomRight - fTopRight);

    } // End if Right Triangle
    
    // Calculate the height interpolated across the top and bottom edges
    cgFloat fTopHeight    = fTopLeft    + ((fTopRight - fTopLeft) * fPercentX );
    cgFloat fBottomHeight = fBottomLeft + ((fBottomRight - fBottomLeft) * fPercentX );

    // Calculate the resulting height interpolated between the two heights
    return fTopHeight + ((fBottomHeight - fTopHeight) * fPercentZ );
}

//-----------------------------------------------------------------------------
// Name : getHeightMapHeight ()
/// <summary>
/// Retrieve the height of the heightmap at the specified location.
/// Note : The coordinates specified are relative to the overall heightmap
/// i.e. not to the section of heightmap stored locally
/// </summary>
//-----------------------------------------------------------------------------
cgInt16 cgTerrainBlock::getHeightMapHeight( cgInt32 x, cgInt32 z ) const
{
    // Retrieve the correct index into the heightmap
    cgUInt32 nHMIndex = GetHeightMapIndex( x, z );
    if ( nHMIndex < 0 ) return 0;

    // Get the heightmap height value
    return mHeightMap[ nHMIndex ];
}

//-----------------------------------------------------------------------------
// Name : GetHeightMapIndex ()
/// <summary>
/// Retrieve the correct index into our internal heightmap buffer given the 
/// specified coordinates. Note : The coordinates specified are relative to the 
/// overall heightmap i.e. not to the section of heightmap stored locally
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTerrainBlock::GetHeightMapIndex( cgInt32 x, cgInt32 z ) const
{
    // Test to ensure it is not out of bounds
    if ( x < mManagedTerrainBounds.left || x >= mManagedTerrainBounds.right ||
         z < mManagedTerrainBounds.top || z >= mManagedTerrainBounds.bottom ) return -1;

    // Transform into the active rectangle
    x -= mSection.blockBounds.left;
    z -= mSection.blockBounds.top;
    x += mActiveHeightMapBounds.left;
    z += mActiveHeightMapBounds.top;

    // Return the index
    return (cgUInt32)(x + z * mActiveHeightMapBounds.pitchX);
}

//-----------------------------------------------------------------------------
// Name : getBoundingBox ()
/// <summary>
/// Retrieve the axis aligned bounding box that describes the extents of this
/// terrain block.
/// </summary>
//-----------------------------------------------------------------------------
const cgBoundingBox & cgTerrainBlock::getBoundingBox( ) const
{
    return mBounds;
}

//-----------------------------------------------------------------------------
// Name : isVisible ( )
/// <summary>
/// Get the visibility status of this terrain block.
/// </summary>
//-----------------------------------------------------------------------------
bool cgTerrainBlock::isVisible( ) const
{
    return mVisible;
}

//-----------------------------------------------------------------------------
// Name : setVisible ( )
/// <summary>
/// Set the visibility status of this terrain block.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::setVisible( bool value )
{
    mVisible = value;
}

//-----------------------------------------------------------------------------
// Name : getNeighbor( )
/// <summary>
/// Get the neighbors that connect to the north, east, south and western edges 
/// of this terrain block.
/// </summary>
//-----------------------------------------------------------------------------
cgTerrainBlock * cgTerrainBlock::getNeighbor( EdgeSide index ) const
{
    return mNeighbors[ (cgInt)index ];
}

//-----------------------------------------------------------------------------
// Name : setNeighbor( )
/// <summary>
/// Set the neighbors that connect to the north, east, south and western edges 
/// of this terrain block.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::setNeighbor( EdgeSide index, cgTerrainBlock * value )
{
    mNeighbors[ (cgInt)index ] = value;
}

//-----------------------------------------------------------------------------
// Name : getTextureData ()
/// <summary>
/// Retrieve the texture data object responsible for managing texture layer
/// data and blend maps for this block.
/// </summary>
//-----------------------------------------------------------------------------
cgLandscapeTextureData * cgTerrainBlock::getTextureData( ) const
{
    return mTextureData;
}

//-----------------------------------------------------------------------------
// Name : getBlockIndex ()
/// <summary>
/// Retrieve the index of this block as it exists in the parent landscape's
/// container array.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTerrainBlock::getBlockIndex( ) const
{
    return mBlockIndex;
}

//-----------------------------------------------------------------------------
// Name : getDatabaseId ()
/// <summary>
/// Retrieve the unique identifier of this block as it exists in the database 
/// (if any).
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgTerrainBlock::getDatabaseId( ) const
{
    return mBlockId;
}

//-----------------------------------------------------------------------------
// Name : getCurrentLOD ( )
/// <summary>
/// Get the currently selected level of detail for this block.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgTerrainBlock::getCurrentLOD( ) const
{
    return mCurrentLOD;
}

//-----------------------------------------------------------------------------
// Name : setCurrentLOD ( )
/// <summary>
/// Set the currently selected level of detail for this block.
/// </summary>
//-----------------------------------------------------------------------------
void cgTerrainBlock::setCurrentLOD( cgInt32 value )
{
    mCurrentLOD = min( mVarianceCount, value );
}

///////////////////////////////////////////////////////////////////////////////
// cgLandscapeSubNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgLandscapeSubNode () (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgLandscapeSubNode::cgLandscapeSubNode( cgLandscape * pParent ) : cgSpatialTreeSubNode()
{
    // Initialize variables to sensible defaults.
    setChildNodeCount( 4 );
    mLandscape = pParent;
    minimumHorizonPoints  = CG_NULL;
    maximumHorizonPoints  = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : cgLandscapeSubNode () (Constructor)
/// <summary>Class constructor.</summary>
//-----------------------------------------------------------------------------
cgLandscapeSubNode::cgLandscapeSubNode( cgSpatialTreeSubNode * pInit, cgSpatialTree * pParent ) : cgSpatialTreeSubNode( pInit, pParent )
{
    // Initialize variables to sensible defaults
    cgLandscapeSubNode * pNode = (cgLandscapeSubNode*)pInit;
    minimumHorizonPlane = pNode->minimumHorizonPlane;
    maximumHorizonPlane = pNode->maximumHorizonPlane;
    mLSSums        = pNode->mLSSums;
    minimumHorizonPoints     = CG_NULL;
    maximumHorizonPoints     = CG_NULL;
    mLandscape    = (cgLandscape*)pParent;

    // Deep copy required elements
    if ( pNode->minimumHorizonPoints != CG_NULL )
    {
        minimumHorizonPoints = new cgVector3[4];
        memcpy( minimumHorizonPoints, pNode->minimumHorizonPoints, 4 * sizeof(cgVector3) );
    
    } // End if valid min horizon
    if ( pNode->maximumHorizonPoints != CG_NULL )
    {
        maximumHorizonPoints = new cgVector3[4];
        memcpy( maximumHorizonPoints, pNode->maximumHorizonPoints, 4 * sizeof(cgVector3) );
    
    } // End if valid min horizon
}

//-----------------------------------------------------------------------------
// Name : ~cgLandscapeSubNode () (Destructor)
/// <summary>Class destructor.</summary>
//-----------------------------------------------------------------------------
cgLandscapeSubNode::~cgLandscapeSubNode( )
{    
    // Destroy resources
    delete []minimumHorizonPoints;
    delete []maximumHorizonPoints;
    minimumHorizonPoints = CG_NULL;
    maximumHorizonPoints = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : collectLeaves () (Virtual, Recursive)
/// <summary>
/// The default recursive function which traverses the tree nodes in order to 
/// build a list of intersecting leaves.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeSubNode::collectLeaves( cgSpatialTreeInstance * pIssuer, cgSpatialTree * pTree, cgSceneLeafArray & Leaves, const cgBoundingBox & Bounds, bool bAutoCollect /* = false */ )
{
    bool bResult = false;

    // Does the specified box intersect this node (IGNORE Y AXIS!)?
    if ( bAutoCollect == false && cgCollision::AABBIntersectAABB( bAutoCollect, Bounds, mNodeBounds, false, true, false ) == false )
        return false;
    
    // Is there a leaf here, add it to the list
    if ( isLeafNode() == true )
    {
        // ToDo: Previously used AddFirst... Any specific reason?
        //Leaves.AddFirst( m_oLeaf );
        Leaves.push_back( pTree->getLeaves()[mLeaf] );
        return true;
    
    } // End if leaf node

    // Test all children for intersection
    for ( cgUInt32 i = 0; i < mChildNodeCount; ++i )
    {
        // Recurse into the child node.
        if ( mChildNodes[i] != CG_NULL )
        {
            if ( mChildNodes[i]->collectLeaves( pIssuer, pTree, Leaves, Bounds, bAutoCollect ) == true )
                bResult = true;

        } // End if child node available

    } // Next child node
    
    // Return the 'was anything added' result.
    return bResult;
}

//-----------------------------------------------------------------------------
// Name : nodeConstructed () (Virtual)
/// <summary>
/// This function will be called whenever a node has been fully constructed by 
/// whichever build function is executing. This allows any derived class to 
/// perform any post build processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeSubNode::nodeConstructed( )
{
    // Update the landscape occlusion data.
    updateOcclusionData();
}

//-----------------------------------------------------------------------------
// Name : updateOcclusionData ()
/// <summary>
/// The terrain data contained within this node has either been updated or 
/// we're loading / compiling for the first time. Compute the occlusion planes
/// and geometry required.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeSubNode::updateOcclusionData( )
{
    cgVector3 * pSamplePoints = CG_NULL;
    cgUInt32    i, nSampleCount = 0;
    cgPlane     LSPlane;

    // Update bounding box heights here too if we were
    // constructing a Y variant tree.
    bool bYVTree = mLandscape->isYVariant();
    if ( bYVTree == true )
    {
        mNodeBounds.min.y = FLT_MAX;
        mNodeBounds.max.y = -FLT_MAX;
    
    } // End if YVariant

    // The sample points we use will be either the landscape points,
    // or the plane geometry depending on whether this is a leaf node or not
    if ( isLeafNode() == true )
    {
        cgRect  rcSection;
        cgFloat fHeight;
        cgInt32 x, z, nPitch;

        // First, retrieve the various pieces of scene data we need
        // ToDo: 9999 - Danger -- Based on full heightmap
        cgHeightMap * pHeightMap = mLandscape->getHeightMap();

        // Retrieve the landscape scale, this is needed if we are to 
        // build accurate least squares sums data which follows the terrain
        // once it is finally constructed, because we have constructed based
        // purely on the heightmap data.
        const cgVector3 & vecScale  = mLandscape->getTerrainScale();
        const cgVector3 & vecOffset = mLandscape->getTerrainOffset();
            
        // Compute the heightmap rectangles for this leaf
        rcSection.left   =  (cgInt32)((mNodeBounds.min.x - vecOffset.x) / vecScale.x);
        rcSection.top    = -(cgInt32)((mNodeBounds.max.z - vecOffset.z) / vecScale.z);
        rcSection.right  = rcSection.left + ((cgInt32)((mNodeBounds.max.x - mNodeBounds.min.x) / vecScale.x) + 1);
        rcSection.bottom = rcSection.top + (-(cgInt32)((mNodeBounds.min.z - mNodeBounds.max.z) / vecScale.z) + 1);

        // Now iterate through and populate sums structure
        if ( pHeightMap != CG_NULL )
        {
            // Allocate enough sample points to describe each point in the heightmap
            nSampleCount = rcSection.width() * rcSection.height();
            pSamplePoints = new cgVector3[nSampleCount];

            // Iterate through all the block sample points and store
            i = 0;
            cgInt16 * pHeightData = &pHeightMap->getImageData()[0];
            nPitch = pHeightMap->getSize().width;
            for ( z = rcSection.top; z < rcSection.bottom; ++z )
            {
                for ( x = rcSection.left; x < rcSection.right; ++x )
                {
                    // Compute the height at this location
                    fHeight  = (cgFloat)pHeightData[ x + z * nPitch ];
                    fHeight *= vecScale.y;

                    // Record AABB heights
                    if ( bYVTree == true )
                    {
                        if ( fHeight < mNodeBounds.min.y )
                            mNodeBounds.min.y = fHeight;
                        if ( fHeight > mNodeBounds.max.y )
                            mNodeBounds.max.y = fHeight;
                    
                    } // End if YV tree
                    
                    // Calculate sums relative to the leaf center point
                    pSamplePoints[i++] = cgVector3( ((cgFloat)x * vecScale.x) + vecOffset.x, fHeight, ((cgFloat)z * -vecScale.z) + vecOffset.z );         

                } // Next Column

            } // Next Row

        } // End if heightmap data found

    } // End if leaf node
    else
    {
        // If this is a standard node, just include the four corner points 
        // of the maximum and minimum plane horizon geometry for each child. 
        // These will be used to generate the new planes which combine all 
        // child nodes.
        pSamplePoints = new cgVector3[32];
        nSampleCount  = 0;
        
        // Retrieve sample data from children
        for ( i = 0; i < mChildNodeCount; ++i )
        {
            cgLandscapeSubNode * pNode = (cgLandscapeSubNode*)mChildNodes[i];
            if ( pNode == CG_NULL ) continue;

            // Add child max sample points for new plane generation
            pSamplePoints[ nSampleCount++ ] = pNode->maximumHorizonPoints[0];
            pSamplePoints[ nSampleCount++ ] = pNode->maximumHorizonPoints[1];
            pSamplePoints[ nSampleCount++ ] = pNode->maximumHorizonPoints[2];
            pSamplePoints[ nSampleCount++ ] = pNode->maximumHorizonPoints[3];
            
            // Add min points in order to ensure minimum plane is adjusted appropriately.
            if ( pNode->minimumHorizonPoints != CG_NULL )
            {
                pSamplePoints[ nSampleCount++ ] = pNode->minimumHorizonPoints[0];
                pSamplePoints[ nSampleCount++ ] = pNode->minimumHorizonPoints[1];
                pSamplePoints[ nSampleCount++ ] = pNode->minimumHorizonPoints[2];
                pSamplePoints[ nSampleCount++ ] = pNode->minimumHorizonPoints[3];
            
            } // End if min exists

            // Record AABB heights relative to child bounds
            if ( bYVTree == true )
            {
                if ( pNode->mNodeBounds.min.y < mNodeBounds.min.y )
                    mNodeBounds.min.y = pNode->mNodeBounds.min.y;
                if ( pNode->mNodeBounds.max.y > mNodeBounds.max.y )
                    mNodeBounds.max.y = pNode->mNodeBounds.max.y;
            
            } // End if YV tree

        } // End if node exists

    } // End if standard node

    // Reset our internal least squares sums
    mLSSums.clear();

    // Compute a reasonable center point for the least squares sums process
    // in order to ensure we get accurate data.
    cgVector3 vecCenter = mNodeBounds.getCenter();
    vecCenter.y = 0.0f;

    // Iterate through all the recorded sample points and generate sums
    for ( i = 0; i < nSampleCount; ++i )
    {
        // Calculate sums relative to the center point
        cgVector3 vecSumPoint = pSamplePoints[i] - vecCenter;
        
        // Sum least squares values for occlusion data
        mLSSums.x += vecSumPoint.x;
        mLSSums.y += vecSumPoint.y;
        mLSSums.z += vecSumPoint.z;
        mLSSums.xx += vecSumPoint.x * vecSumPoint.x;
        mLSSums.xy += vecSumPoint.x * vecSumPoint.y;
        mLSSums.xz += vecSumPoint.x * vecSumPoint.z;
        mLSSums.zy += vecSumPoint.z * vecSumPoint.y;
        mLSSums.zz += vecSumPoint.z * vecSumPoint.z;
        mLSSums.samples++;

    } // Next sample

    // Enough sample points?
    if ( mLSSums.samples < 3.0f )
    {
        // There were not enough sample points, just choose an upward facing plane
        LSPlane = cgPlane( 0, 1, 0, 0 );
        
    } // End if not enough samples
    else
    {
        // Compute the plane data from these sums
        LSPlane = mLSSums.computePlane();
        
    } // End if enough samples taken
    
    // Sample data available?
    if ( nSampleCount > 0 )
    {
        cgFloat fErr, fErrLow = FLT_MAX, fErrHigh = -FLT_MAX;

        // Calculate the error values and generate a minimum and maximum plane
        for ( i = 0; i < nSampleCount; ++i )
        {
            // Calculate the distance to the plane along the vertical. This is our 'error' value.
            fErr = cgMathUtility::distanceToPlane( pSamplePoints[i], LSPlane, cgVector3( 0, 1, 0 ) );

            // If it's below the lowest error threshold so far
            // set this as our minimum horizon plane.
            if ( fErr < fErrLow )
            {
                cgPlane::fromPointNormal( minimumHorizonPlane, pSamplePoints[i], (cgVector3&)LSPlane );
                fErrLow = fErr;
            
            } // End if below low error threshold

            // Do the same for the maximum plane
            if ( fErr > fErrHigh )
            {
                cgPlane::fromPointNormal( maximumHorizonPlane, pSamplePoints[i], (cgVector3&)LSPlane );
                fErrHigh = fErr;
            
            } // End if above high error threshold

        } // Next sample

    } // End if sample data available
    else
    {
        // Just set planes to size of bounding box
        cgPlane::fromPointNormal( minimumHorizonPlane, pSamplePoints[i], mNodeBounds.min );
        cgPlane::fromPointNormal( maximumHorizonPlane, pSamplePoints[i], mNodeBounds.max );
    
    } // End if no sample data

    // We're finished with any computed sample points
    delete []pSamplePoints;

    // Optimization: If there is no significant error introduced by collapsing
    // the child MINIMUM horizon geometry (the raster geometry) into this node
    // then do so.
    bool bCollapsed = false;
    if ( isLeafNode() == false )
    {
        // If the new minimum plane differs in angle significantly from that
        // of it's children, we cannot collapse.
        for ( i = 0; i < 4; ++i )
        {
            cgLandscapeSubNode * pChild = (cgLandscapeSubNode*)mChildNodes[i];

            // We also cannot collapse if the child doesn't already store
            // the minimum (raster) horizon geometry.
            if ( pChild == CG_NULL || pChild->minimumHorizonPoints == CG_NULL )
                break;

            // Plane normals match (within tolerance)
            //if ( cgMathUtility::compareVectors( (cgVector3&)minimumHorizonPlane, (cgVector3&)pChild->minimumHorizonPlane, 0.3f ) != 0 )
                //break;
            if ( cgMathUtility::compareVectors( (cgVector3&)minimumHorizonPlane, (cgVector3&)pChild->minimumHorizonPlane, 0.1f ) != 0 )
                break;
        
        } // Next Child

        // Only if all children matched can we collapse
        if ( i == 4 )
        {
            for ( i = 0; i < 4; ++i )
            {
                cgLandscapeSubNode * pChild = (cgLandscapeSubNode*)mChildNodes[i];
                delete []pChild->minimumHorizonPoints;
                delete []pChild->maximumHorizonPoints;
                pChild->minimumHorizonPoints = CG_NULL;
                pChild->maximumHorizonPoints = CG_NULL;

            } // Next Child
            bCollapsed = true;
        
        } // End if all matched

    } // End if standard node

    // Allocate memory for max horizon geometry (test geom).
    if ( maximumHorizonPoints == CG_NULL )
        maximumHorizonPoints = new cgVector3[4];
    
    // Allocate or release memory for minimum horizon geometry (raster) as necessary
    if ( minimumHorizonPoints == CG_NULL && (bCollapsed == true || isLeafNode() == true) )
        minimumHorizonPoints = new cgVector3[4];
    else if ( minimumHorizonPoints != CG_NULL )
    {
        // Any existing minimum horizon geometry is no longer needed.
        delete []minimumHorizonPoints;
        minimumHorizonPoints = CG_NULL;

    } // End if

    // Just build two simple quads filling the bounding box to begin with.
    maximumHorizonPoints[0] = cgVector3( mNodeBounds.min.x, mNodeBounds.min.y, mNodeBounds.min.z );
    maximumHorizonPoints[1] = cgVector3( mNodeBounds.min.x, mNodeBounds.min.y, mNodeBounds.max.z );
    maximumHorizonPoints[2] = cgVector3( mNodeBounds.max.x, mNodeBounds.min.y, mNodeBounds.max.z );
    maximumHorizonPoints[3] = cgVector3( mNodeBounds.max.x, mNodeBounds.min.y, mNodeBounds.min.z );
    if ( minimumHorizonPoints != CG_NULL )
        memcpy( minimumHorizonPoints, maximumHorizonPoints, 4 * sizeof(cgVector3) );

    // Now calculate the distance to the relevant plane and shift the point up / down
    for ( i = 0; i < 4; ++i )
    {
        maximumHorizonPoints[i].y += -cgMathUtility::distanceToPlane( maximumHorizonPoints[i], maximumHorizonPlane, cgVector3(0,1,0) );
        
        // Shift min geometry too.
        if ( minimumHorizonPoints != CG_NULL )
            minimumHorizonPoints[i].y += -cgMathUtility::distanceToPlane( minimumHorizonPoints[i], minimumHorizonPlane, cgVector3(0,1,0) );

    } // Next point
}

///////////////////////////////////////////////////////////////////////////////
// cgLandscapeCell Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgLandscapeCell () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgLandscapeCell::cgLandscapeCell()
{
    // Initialize variables to sensible defaults
}

//-----------------------------------------------------------------------------
// Name : cgLandscapeCell () (Constructor)
// Desc : Constructor for this class.
//-----------------------------------------------------------------------------
cgLandscapeCell::cgLandscapeCell( cgSpatialTreeLeaf * pInit, cgSpatialTree * pParent ) : cgSpatialTreeLeaf( pInit, pParent )
{
    // Initialize variables to sensible defaults
    cgLandscapeCell * pLeaf = (cgLandscapeCell*)pInit;

    // Nothing in current implementation
}

///////////////////////////////////////////////////////////////////////////////
// cgLandscapeTextureData Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgLandscapeTextureData() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgLandscapeTextureData::cgLandscapeTextureData( cgLandscape * pLandscape, cgTerrainBlock * pParentBlock )
{
    // Initialize variables to sensible defaults
    mParentBlock          = pParentBlock;
    mParentLandscape      = pLandscape;
    mIsPainting           = false;
    mPaintLayer           = -1;

    // Compute the sizes of the internal blend maps (including borders)
    mBlendMapSize = pLandscape->getBlockBlendMapSize();
    mBlendMapSize.width  = mBlendMapSize.width + 2;
    mBlendMapSize.height = mBlendMapSize.height + 2;

    // Compute the "active" interior region of the blend maps that are actually mapped to the blocks
    mActiveBlendMapBounds.left   = 1;
    mActiveBlendMapBounds.top    = 1;
    mActiveBlendMapBounds.right  = mBlendMapSize.width - 1;
    mActiveBlendMapBounds.bottom = mBlendMapSize.height - 1;

    // Compute the "resolution" of the blend map (number of texels per terrain quad)
    mBlendMapResolution.width  = (cgFloat)mActiveBlendMapBounds.width()  / (cgFloat)(pLandscape->getBlockSize().width - 1);
    mBlendMapResolution.height = (cgFloat)mActiveBlendMapBounds.height() / (cgFloat)(pLandscape->getBlockSize().height - 1);
}

//-----------------------------------------------------------------------------
// Name : ~cgLandscapeTextureData() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgLandscapeTextureData::~cgLandscapeTextureData()
{
    // Release layer reference data
    mLayers.clear();
    
    // Dispose of any allocated blend maps
    mCombinedBlendMaps.clear();
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    cgWorld * pWorld = mParentLandscape->getScene()->getParentWorld();
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( mInsertLayer.isPrepared() == false )
            mInsertLayer.prepare( pWorld, _T("INSERT INTO 'Landscapes::BlockLayers' VALUES(NULL,?1,?2,?3,?4)"), true );
        if ( mDeleteLayer.isPrepared() == false )
            mDeleteLayer.prepare( pWorld, _T("DELETE FROM 'Landscapes::BlockLayers' WHERE LayerId=?1"), true );
        if ( mUpdateLayerPaintData.isPrepared() == false )
            mUpdateLayerPaintData.prepare( pWorld, _T("UPDATE 'Landscapes::BlockLayers' SET BlendMapData=?1, PixelRefCount=?2 WHERE LayerId=?3"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadLayers.isPrepared() == false )
        mLoadLayers.prepare( pWorld, _T("SELECT * FROM 'Landscapes::BlockLayers' WHERE BlockId=?1"), true );
}

//-----------------------------------------------------------------------------
// Name : loadLayers ( )
/// <summary>
/// Load layer data from the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::loadLayers()
{
    // Get access to required systems
    cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();

    // Load terrain blocks
    prepareQueries();
    mLoadLayers.bindParameter( 1, mParentBlock->getDatabaseId() );
    if ( mLoadLayers.step( ) == false )
    {
        // Log any error.
        cgString strError;
        if ( mLoadLayers.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve texture layer data for block '%i' within landscape object '0x%x'. World database has potentially become corrupt.\n"), mParentBlock->getDatabaseId(), mParentLandscape->getDatabaseId() );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve texture layer data for block '%i' within landscape object '0x%x'. Error: %s\n"), mParentBlock->getDatabaseId(), mParentLandscape->getDatabaseId(), strError.c_str() );

        // Release any pending read operation.
        mLoadLayers.reset();
        return false;
    
    } // End if failed

    // Process layer data.
    for ( ; mLoadLayers.nextRow(); )
    {
        // Skip any layers which somehow have a 0 pixel reference count in the database.
        cgUInt32 nRefCount = 0;
        mLoadLayers.getColumn( _T("PixelRefCount"), nRefCount );
        if ( nRefCount == 0 )
            continue;

        // Create a new in-memory layer.
        mLayers.resize( mLayers.size() + 1 );
        LayerReference & NewLayer = mLayers.back();

        // Load the referenced layer type
        cgUInt32 nLayerType = 0;
        mLoadLayers.getColumn( _T("MaterialId"), nLayerType );
        pResources->loadMaterial( &NewLayer.layerType, mParentLandscape->getScene()->getParentWorld(), 
                                  cgMaterialType::LandscapeLayer, nLayerType, false, 0, cgDebugSource() );

        // Setup remaining layer details.
        mLoadLayers.getColumn( _T("LayerId"), NewLayer.layerId );
        NewLayer.referenceCount          = nRefCount;
        NewLayer.combinedBlendMap  = 0xFFFFFFFF;   // Unknown
        NewLayer.channel           = 0xFF;         // Unknown
        NewLayer.eraseLayer        = false;
        
        // Reset texture dirty status.
        NewLayer.dirty             = false;
        NewLayer.dirtyRectangle            = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );

        // Reset final blend map dirty status.
        NewLayer.finalDirty        = false;
        NewLayer.finalDirtyRectangle       = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );

        // Populate blend map from the loaded layer data.
        cgUInt32 nBlendMapSize = 0;
        cgByte * pBlendMapData = CG_NULL;
        mLoadLayers.getColumn( _T("BlendMapData"), (void**)&pBlendMapData, nBlendMapSize );
        NewLayer.blendMap.resize( mBlendMapSize.width * mBlendMapSize.height );
        if ( nBlendMapSize == NewLayer.blendMap.size() && pBlendMapData != CG_NULL )
            memcpy( &NewLayer.blendMap[0], pBlendMapData, nBlendMapSize );
        
    } // Next Block

    // We're done with the block read query.
    mLoadLayers.reset();

    // Optimize the blend map textures and construct render pass data 
    // ready for rendering in an optimal fashion (max. 4 layers per pass).
    optimizeLayers();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : getPaintRectangle()
/// <summary>
/// Given the terrain space 2D bounding box (x and z only) determine the 
/// image space rectangle that contains every blend map texel that falls within
/// the box. Also computes the world space origin and stride / delta that
/// allows the caller to accurately determine the world space location of each
/// texel it may be processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::getPaintRectangle( cgRectF Bounds, cgInt32 & xStart, cgInt32 & yStart, cgInt32 & xEnd, cgInt32 & yEnd, cgVector2 & vPaintCenter, cgVector2 & vPaintOrigin, cgVector2 & vPaintDelta ) const
{
    const cgVector3 & vecScale = mParentLandscape->getTerrainScale();

    // Compute the texel delta (distance in world space units between each texel)
    vPaintDelta.x = vecScale.x / (cgFloat)mBlendMapResolution.width;
    vPaintDelta.y = -vecScale.z / (cgFloat)mBlendMapResolution.height;

    // First, round the cursor bounding box to the nearest texel
    // such that it will entirely surround it.
    Bounds.left = floorf(Bounds.left / vPaintDelta.x) * vPaintDelta.x;
    Bounds.top = floorf(Bounds.top / -vPaintDelta.y) * -vPaintDelta.y;
    Bounds.right = ceilf (Bounds.right / vPaintDelta.x) * vPaintDelta.x;
    Bounds.bottom = ceilf (Bounds.bottom / -vPaintDelta.y) * -vPaintDelta.y;

    // Compute the world space origin, at the center of the upper-left texel.
    const cgRectF & rcBlendMapWorldArea = getBlendMapWorldArea();
    vPaintOrigin.x = rcBlendMapWorldArea.left + (vPaintDelta.x * 0.5f);
    vPaintOrigin.y = rcBlendMapWorldArea.top + (vPaintDelta.y * 0.5f);

    // Offset selected bounding box into the space of the parent block.
    // Take care to consider the border offset.
    const cgBoundingBox & BlockBounds = mParentBlock->getBoundingBox( );
    Bounds.left -= BlockBounds.min.x; Bounds.right -= BlockBounds.min.x;
    Bounds.top -= BlockBounds.max.z; Bounds.bottom -= BlockBounds.max.z;
    Bounds.left += ((cgFloat)mActiveBlendMapBounds.left * vPaintDelta.x);
    Bounds.top += ((cgFloat)mActiveBlendMapBounds.top * vPaintDelta.y);
    Bounds.right += ((cgFloat)mActiveBlendMapBounds.left * vPaintDelta.x);
    Bounds.bottom += ((cgFloat)mActiveBlendMapBounds.top * vPaintDelta.y);
    
    // Calculate integer processing rectangle
    xStart = (long)(Bounds.left / vPaintDelta.x);
    yStart = (long)(-Bounds.bottom / -vPaintDelta.y);
    xEnd   = (long)(Bounds.right / vPaintDelta.x) - 1;
    yEnd   = (long)(-Bounds.top / -vPaintDelta.y) - 1;

    // Also output the clamped center of the paint rectangle. If we assume that 
    // the cursor radius is centered about the mid-point of the paint rectangle,
    // this helsq to ensure that we can accurately paint small details even when
    // the radius of the cursor is smaller than an individual texel.
    vPaintCenter.x = vPaintOrigin.x + (vPaintDelta.x * ((cgFloat)(xStart + xEnd + 1) * 0.5f));
    vPaintCenter.y = vPaintOrigin.y + (vPaintDelta.y * ((cgFloat)(yStart + yEnd + 1) * 0.5f));

    // Clamp to the interior area of this block's blend map.
    xStart  = max( 0, min( mBlendMapSize.width - 1, xStart ) );
    yStart  = max( 0, min( mBlendMapSize.height - 1, yStart ) );
    xEnd    = max( 0, min( mBlendMapSize.width - 1, xEnd ) );
    yEnd    = max( 0, min( mBlendMapSize.height - 1, yEnd ) );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : beginPaint()
/// <summary>
/// Called in order to enable painting for this block.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::beginPaint( const cgMaterialHandle & hType, const cgLandscapePaintParams & Params )
{
    cgInt32 nLayerIndex = -1;
    
    // Is this a no-op?
    if ( mIsPainting )
        return false;

    // Erasing or painting?
    if ( !Params.erase )
    {
        // Is this layer already referenced within this block?
        for ( size_t i = 0; i < mLayers.size(); ++i )
        {
            if ( mLayers[i].layerType == hType )
            {
                nLayerIndex = (cgInt32)i;
                break;

            } // End if exists

        } // Next layer

    } // End if painting

    // Do we need to add a new layer? In the case of erasing, we will ALWAYS add
    // a temporary new layer that will house the erase data, but will eventually
    // discard it (its refcount will ultimately always be 0 and will be destroyed
    // again during the call to endPaint()).
    if ( nLayerIndex == -1 )
    {
        // Attempt to insert new layer into the database if necessary.
        cgUInt32 nLayerDatabaseId = 0;
        if ( !Params.erase && mParentLandscape->shouldSerialize() )
        {
            prepareQueries();
            mInsertLayer.bindParameter( 1, mParentBlock->getDatabaseId() );
            mInsertLayer.bindParameter( 2, hType.getReferenceId() );
            mInsertLayer.bindParameter( 3, CG_NULL, 0 );
            mInsertLayer.bindParameter( 4, (cgUInt32)0 ); // PixelRefCount

            // Execute
            if ( !mInsertLayer.step( true ) )
            {
                cgString strError;
                mInsertLayer.getLastError( strError );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert new block texture layer for block '%i' within landscape object '0x%x'. Error: %s\n"), mParentBlock->getDatabaseId(), mParentLandscape->getDatabaseId(), strError.c_str() );
                return false;
            
            } // End if failed

            // Retrieve the unique database identifier for this layer.
            nLayerDatabaseId = mInsertLayer.getLastInsertId();

        } // End if serialize

        // Create a new in-memory layer.
        mLayers.resize( mLayers.size() + 1 );
        LayerReference & NewLayer = mLayers.back();
        nLayerIndex = cgInt32(mLayers.size() - 1);

        // If we are adding a new layer, the first thing we need to do is to
        // allocate the blend map memory into which we will paint.
        NewLayer.blendMap.resize( mBlendMapSize.width * mBlendMapSize.height );

        // Setup remaining layer details.
        NewLayer.layerType         = hType;
        NewLayer.referenceCount          = 0;
        NewLayer.combinedBlendMap  = 0xFFFFFFFF;   // Unknown
        NewLayer.channel           = 0xFF;         // Unknown
        NewLayer.layerId           = nLayerDatabaseId;
        NewLayer.eraseLayer        = Params.erase;
        
        // Reset texture dirty status.
        NewLayer.dirty             = false;
        NewLayer.dirtyRectangle            = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );

        // Reset final blend map dirty status.
        NewLayer.finalDirty        = false;
        NewLayer.finalDirtyRectangle       = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );

        // A new layer was added. We need to re-optimize the blend map textures
        // and construct render pass data ready for rendering in an optimal fashion 
        // (max. 4 layers per pass) -- unless this is a temporary erase layer.
        if ( !NewLayer.eraseLayer )
            optimizeLayers();

    } // End if adding

    // We also need to create a temporary area into which to paint. This buffer
    // will not actually be committed to the layer until the user completes 
    // the painting operation (click drag release).
    mLayers[ nLayerIndex ].blendMapPaint.resize( mBlendMapSize.width * mBlendMapSize.height );

    // Painting is now enabled for this block.
    mIsPainting  = true;
    mParams       = Params;
    mPaintLayer  = nLayerIndex;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : paint()
/// <summary>
/// Paint into the current layer at the given location.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::paint( cgFloat fX, cgFloat fZ )
{
    // ToDo: 9999 - Remove when we are completely satisfied with the new technique (below).
    /*cgInt32     x, y, xStart, yStart, xEnd, yEnd;
    cgVector2 vPaintOrigin, vPaintDelta, vPaintPos, vCursorPos;
    
    // Must begin painting.
    if ( mIsPainting == false )
        return false;

    // Retrieve the layer reference into which we will paint.
    LayerReference & Layer = mLayers[ mPaintLayer ];

    // Compute the rectangle around the area we will paint into. We paint
    // into an area half way between the inner and outer radius that will
    // later be filtered out to the correct outer radius.
    cgFloat fCursorRadius = ((mParams.innerRadius + mParams.outerRadius) * 0.5f) + CGE_EPSILON_1MM;
    cgRectF Bounds( fX - fCursorRadius, fZ - fCursorRadius, fX + fCursorRadius, fZ + fCursorRadius );
    getPaintRectangle( Bounds, xStart, yStart, xEnd, yEnd, vCursorPos, vPaintOrigin, vPaintDelta );

    // Loop through the cursor rectangle and paint into the required area.
    fCursorRadius = ((mParams.innerRadius + mParams.outerRadius) * 0.5f) * ((mParams.innerRadius + mParams.outerRadius) * 0.5f);
    for ( y = yStart; y <= yEnd; ++y )
    {
        for ( x = xStart; x <= xEnd; ++x )
        {
            // Should we output at this location?
            vPaintPos = cgVector2( vPaintOrigin.x + (((cgFloat)x + 0.5f) * vPaintDelta.x), (vPaintOrigin.y + (((cgFloat)y + 0.5f) * vPaintDelta.y)) );
            if ( cgVector2::lengthSq( &(vPaintPos - vCursorPos) ) <= fCursorRadius )
            {
                // Paint 1.0 into the temporary blend map buffer.
                Layer.blendMapPaint[ x + y * mBlendMapSize.width ] = 255;

            } // End if within paint area

        } // Next Column

    } // Next Row

    // Compute the full rectangle of all texels that will ultimately 
    // be modified after filtering has taken place (texture dirty rectangle).
    fCursorRadius = mParams.outerRadius + CGE_EPSILON_1MM;
    Bounds = cgRectF( fX - fCursorRadius, fZ - fCursorRadius, fX + fCursorRadius, fZ + fCursorRadius );
    getPaintRectangle( Bounds, xStart, yStart, xEnd, yEnd, vCursorPos, vPaintOrigin, vPaintDelta );

    // Update smallest possible area
    cgRect & rc = Layer.dirtyRectangle;
    if ( xStart   < rc.left   ) rc.left   = xStart;
    if ( yStart   < rc.top    ) rc.top    = yStart;
    if ( xEnd + 1 > rc.right  ) rc.right  = xEnd + 1;
    if ( yEnd + 1 > rc.bottom ) rc.bottom = yEnd + 1;
    Layer.dirty = (rc.isEmpty() == false);

    // Add the current dirty rectangle to our final blend map update area if necessary.
    cgRect & rcf = Layer.finalDirtyRectangle;
    if ( rc.left   < rcf.left   ) rcf.left   = rc.left;
    if ( rc.top    < rcf.top    ) rcf.top    = rc.top;
    if ( rc.right  > rcf.right  ) rcf.right  = rc.right;
    if ( rc.bottom > rcf.bottom ) rcf.bottom = rc.bottom;
    Layer.finalDirty  = (rcf.isEmpty() == false);
    
    // Success!
    return true; */

    cgInt32   xStart, yStart, xEnd, yEnd;
    cgVector2 vCursorPos, vPaintOrigin, vPaintDelta, vPaintPos;
    cgUInt32  nValue;
    
    // Must begin painting.
    if ( mIsPainting == false )
        return false;

    // Retrieve the layer reference into which we will paint.
    LayerReference & Layer = mLayers[ mPaintLayer ];

    // Compute the full rectangle of the cells that will ultimately 
    // be modified after filtering has taken place (texture dirty rectangle).
    // Note: We add a padding of 2 texels around all sides to ensure any
    // anti-aliasing applied (currently disabled) is correctly captured.
    const cgVector3 & vScale = mParentLandscape->getTerrainScale();
    cgFloat fXPadding = 2.0f * (vScale.x / (cgFloat)mBlendMapResolution.width);
    cgFloat fZPadding = 2.0f * (vScale.z / (cgFloat)mBlendMapResolution.height);
    cgFloat fCursorRadius = mParams.outerRadius + CGE_EPSILON_1MM;
    cgRectF Bounds( fX - (fCursorRadius + fXPadding), fZ - (fCursorRadius + fZPadding),
                    fX + (fCursorRadius + fXPadding), fZ + (fCursorRadius + fZPadding) );
    getPaintRectangle( Bounds, xStart, yStart, xEnd, yEnd, vCursorPos, vPaintOrigin, vPaintDelta );
    
    // Convert the terrain relative position into relative blend map pixel coordinates.
    fX -= vPaintOrigin.x;
    fZ -= vPaintOrigin.y;
    fX /= vPaintDelta.x;
    fZ /= vPaintDelta.y;
    
    // Create a temporary bitmap to use for circle drawing
    cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+" );
    cgImage * pData = cgImage::createInstance( (xEnd - xStart) + 1, (yEnd - yStart) + 1, cgBufferFormat::B8G8R8, true );
    if ( pData == CG_NULL || pData->isValid() == false )
    {
        delete pData;
        return false;
    
    } // End if failed

    // Prepare to draw.
    pData->beginDraw( );
    
    // Setup brush style
    pData->setBrushColor( 0xFFFFFFFF );
    
    // Draw!
    cgFloat fDrawRadius = ((mParams.innerRadius + mParams.outerRadius) / vPaintDelta.x) / 2.0f;
    pData->fillCircle( fX - (cgFloat)xStart, fZ - (cgFloat)yStart, fDrawRadius );

    // Copy into relevant location of our paint buffer.
    cgByte * pDest = &Layer.blendMapPaint[xStart + yStart * mBlendMapSize.width];
    const cgByte * pSrc = pData->getBuffer();
    for ( cgUInt32 y = 0; y < pData->getHeight(); ++y )
    {
        for ( cgUInt32 x = 0; x < pData->getWidth(); ++x )
        {
            nValue   = (cgInt)(*pDest) + (cgInt)(*pSrc);
            *pDest++ = (cgByte)min( 0xFF, nValue );
            pSrc    += 3;
        
        } // Next Column

        // Move down to next row
        pSrc  += pData->getPitch() - (pData->getWidth() * 3);
        pDest += mBlendMapSize.width - pData->getWidth();

    } // Next Row
    
    // Clean up
    pData->endDraw();
    delete pData;

    // Update smallest possible area
    cgRect & rc = Layer.dirtyRectangle;
    if ( xStart   < rc.left   ) rc.left   = xStart;
    if ( yStart   < rc.top    ) rc.top    = yStart;
    if ( xEnd + 1 > rc.right  ) rc.right  = xEnd + 1;
    if ( yEnd + 1 > rc.bottom ) rc.bottom = yEnd + 1;
    Layer.dirty = (rc.isEmpty() == false);

    // Add the current dirty rectangle to our final blend map update area if necessary.
    cgRect & rcf = Layer.finalDirtyRectangle;
    if ( rc.left   < rcf.left   ) rcf.left   = rc.left;
    if ( rc.top    < rcf.top    ) rcf.top    = rc.top;
    if ( rc.right  > rcf.right  ) rcf.right  = rc.right;
    if ( rc.bottom > rcf.bottom ) rcf.bottom = rc.bottom;
    Layer.finalDirty  = (rcf.isEmpty() == false);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : setPixel()
/// <summary>
/// Set the very specific pixel in the blend map to the current paint value.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::setPixel( cgInt32 x, cgInt32 y )
{
    // Must begin painting.
    if ( mIsPainting == false )
        return false;

    // Retrieve the layer reference into which we will paint.
    LayerReference & Layer = mLayers[ mPaintLayer ];

    // Paint 1.0 into the temporary blend map buffer.
    Layer.blendMapPaint[ y * mBlendMapSize.width + x ] = 255;
    
    // Update smallest possible area
    cgRect & rc = Layer.dirtyRectangle;
    if ( x < rc.left       ) rc.left   = x;
    if ( y < rc.top        ) rc.top    = y;
    if ( (x+1) > rc.right  ) rc.right  = x + 1;
    if ( (y+1) > rc.bottom ) rc.bottom = y + 1;
    Layer.dirty = (rc.isEmpty() == false);

    // Add the current dirty rectangle to our final blend map update area if necessary.
    cgRect & rcf = Layer.finalDirtyRectangle;
    if ( rc.left   < rcf.left   ) rcf.left   = rc.left;
    if ( rc.top    < rcf.top    ) rcf.top    = rc.top;
    if ( rc.right  > rcf.right  ) rcf.right  = rc.right;
    if ( rc.bottom > rcf.bottom ) rcf.bottom = rc.bottom;
    Layer.finalDirty  = (rcf.isEmpty() == false);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : paintLine()
/// <summary>
/// Paint a line into the current layer at the given location.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::paintLine( cgFloat fXFrom, cgFloat fZFrom, cgFloat fXTo, cgFloat fZTo )
{
    cgInt32   xStart, yStart, xEnd, yEnd;
    cgVector2 vCursorPos, vPaintOrigin, vPaintDelta, vPaintPos;
    cgUInt32  nValue;
    
    // Must begin painting.
    if ( mIsPainting == false )
        return false;

    // Retrieve the layer reference into which we will paint.
    LayerReference & Layer = mLayers[ mPaintLayer ];

    // Compute the full rectangle of the cells that will ultimately 
    // be modified after filtering has taken place (texture dirty rectangle).
    // Note: We add a padding of 2 texels around all sides to ensure any
    // anti-aliasing applied (currently disabled) is correctly captured.
    const cgVector3 & vScale = mParentLandscape->getTerrainScale();
    cgFloat fXPadding = 2.0f * (vScale.x / (cgFloat)mBlendMapResolution.width);
    cgFloat fZPadding = 2.0f * (vScale.z / (cgFloat)mBlendMapResolution.height);
    cgFloat fCursorRadius = mParams.outerRadius + CGE_EPSILON_1MM;
    cgRectF Bounds( min( fXFrom, fXTo ) - (fCursorRadius + fXPadding), min( fZFrom, fZTo ) - (fCursorRadius + fZPadding),
                    max( fXFrom, fXTo ) + (fCursorRadius + fXPadding), max( fZFrom, fZTo ) + (fCursorRadius + fZPadding) );
    getPaintRectangle( Bounds, xStart, yStart, xEnd, yEnd, vCursorPos, vPaintOrigin, vPaintDelta );
    
    // Convert the terrain relative line positions into relative blend map pixel coordinates.
    fXFrom -= vPaintOrigin.x; fXTo -= vPaintOrigin.x;
    fZFrom -= vPaintOrigin.y; fZTo -= vPaintOrigin.y;
    fXFrom /= vPaintDelta.x; fXTo /= vPaintDelta.x;
    fZFrom /= vPaintDelta.y; fZTo /= vPaintDelta.y;
    
    // Create a temporary bitmap to use for line drawing
    cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+" );
    cgImage * pData = cgImage::createInstance( (xEnd - xStart) + 1, (yEnd - yStart) + 1, cgBufferFormat::B8G8R8, true );
    if ( pData == CG_NULL || pData->isValid() == false )
    {
        delete pData;
        return false;
    
    } // End if failed

    // Prepare to draw.
    pData->beginDraw( );
    
    // Setup line style
    pData->setPenWidth( (mParams.innerRadius + mParams.outerRadius) / vPaintDelta.x );
    pData->setPenColor( 0xFFFFFFFF );
    pData->setPenStartCap( cgImage::PenCapRound );
    pData->setPenEndCap( cgImage::PenCapRound );
    
    // Draw!
    pData->drawLine( fXFrom - (cgFloat)xStart, fZFrom - (cgFloat)yStart,
                     fXTo - (cgFloat)xStart, fZTo - (cgFloat)yStart );

    // Copy into relevant location of our paint buffer.
    cgByte * pDest = &Layer.blendMapPaint[xStart + yStart * mBlendMapSize.width];
    const cgByte * pSrc = pData->getBuffer();
    for ( cgUInt32 y = 0; y < pData->getHeight(); ++y )
    {
        for ( cgUInt32 x = 0; x < pData->getWidth(); ++x )
        {
            nValue   = (cgInt)(*pDest) + (cgInt)(*pSrc);
            *pDest++ = (cgByte)min( 0xFF, nValue );
            pSrc    += 3;
        
        } // Next Column

        // Move down to next row
        pSrc  += pData->getPitch() - (pData->getWidth() * 3);
        pDest += mBlendMapSize.width - pData->getWidth();

    } // Next Row
    
    // Clean up
    pData->endDraw();
    delete pData;

    // Update smallest possible area
    cgRect & rc = Layer.dirtyRectangle;
    if ( xStart   < rc.left   ) rc.left   = xStart;
    if ( yStart   < rc.top    ) rc.top    = yStart;
    if ( xEnd + 1 > rc.right  ) rc.right  = xEnd + 1;
    if ( yEnd + 1 > rc.bottom ) rc.bottom = yEnd + 1;
    Layer.dirty = (rc.isEmpty() == false);

    // Add the current dirty rectangle to our final blend map update area if necessary.
    cgRect & rcf = Layer.finalDirtyRectangle;
    if ( rc.left   < rcf.left   ) rcf.left   = rc.left;
    if ( rc.top    < rcf.top    ) rcf.top    = rc.top;
    if ( rc.right  > rcf.right  ) rcf.right  = rc.right;
    if ( rc.bottom > rcf.bottom ) rcf.bottom = rc.bottom;
    Layer.finalDirty  = (rcf.isEmpty() == false);

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : endPaint()
/// <summary>
/// Called when layer updates have fully completed (i.e. user let go of the
/// mouse button).
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::endPaint( )
{
    cgInt32     x, y;
    cgUInt32    nSrcPitch;
    cgByte      nOldWeight, nNewWeight;

    // Must begin painting.
    if ( mIsPainting == false )
        return;

    // Get access to required systems
    cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();
    
    // We'll be commiting changes to the paint layer.
    LayerReference & PaintLayer = mLayers[mPaintLayer];
    if ( PaintLayer.finalDirty == false )
    {
        mIsPainting = false;
        mPaintLayer = -1;
        return;

    } // End if update occurred

    // Trigger a final preview texture update if necessary.
    if ( PaintLayer.dirty == true )
        updatePaintPreview();

    // Remove temporary paint buffers to conserve memory.
    PaintLayer.blendMapPaint.clear();

    // Use the texture data as the source for the final buffers.
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        // Skip erase layers (they have no texture).
        LayerReference & Layer = mLayers[i];
        if ( Layer.eraseLayer == true )
            continue;
        
        // Lock source texture for read access.
        cgTexture * pSourceTexture = mCombinedBlendMaps[Layer.combinedBlendMap].texture.getResource(true);
        cgByte    * pSrc           = (cgByte*)pSourceTexture->lock( PaintLayer.finalDirtyRectangle, nSrcPitch, cgLockFlags::ReadOnly );
        
        // ToDo: Handle errors.
        
        // Offset to the correct 8bit layer location (BGRA)
        pSrc += (3 - Layer.channel);

        // Iterate and update output layer from paint layer.
        cgByte * pDest = &Layer.blendMap[PaintLayer.finalDirtyRectangle.left + PaintLayer.finalDirtyRectangle.top * mBlendMapSize.width];
        for ( y = 0; y < PaintLayer.finalDirtyRectangle.height(); ++y )
        {
            for ( x = 0; x < PaintLayer.finalDirtyRectangle.width(); ++x )
            {
                // Retrieve the previous cell weight at this location.
                nOldWeight = *pDest;

                // Store the new cell weight.
                nNewWeight = *pSrc;
                pSrc      += 4;
                *pDest++   = nNewWeight;

                // Determine if this is a new reference to this layer (i.e. a 0 weight
                // is about to become a valid weight) or vice versa.
                if ( nOldWeight == 0 && nNewWeight != 0 )
                    Layer.referenceCount++;
                else if ( nOldWeight != 0 && nNewWeight == 0 )
                    Layer.referenceCount--;

                // ToDo: remove.
                #if defined(_DEBUG)
                    if ( Layer.referenceCount < 0 )
                        cgAppLog::write( cgAppLog::Debug, _T("An invalid landscape reference count of %i was discovered.\n"), Layer.referenceCount );
                #endif

            } // Next Column

            // Move to next row.
            pDest += mBlendMapSize.width - PaintLayer.finalDirtyRectangle.width();
            pSrc  += nSrcPitch - (PaintLayer.finalDirtyRectangle.width() * 4);

        } // Next Row
    
        pSourceTexture->unlock( false );

    } // Next Layer

    // Mark painting as complete.
    PaintLayer.finalDirty  = false;
    PaintLayer.finalDirtyRectangle = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );
    mIsPainting = false;
    mPaintLayer = -1;

    // ToDo: 9999 - Wrap in a transaction!

    // Remove any layers that are no longer referenced and
    // update database entry for those that are.
    cgInt32 nCount = cgInt32(mLayers.size());
    for ( cgInt32 i = 0; i < nCount; ++i )
    {
        // Remove unreferenced layers.
        if ( mLayers[i].referenceCount == 0 )
        {
            // Remove from database if necessary.
            if ( mLayers[i].layerId != 0 && mParentLandscape->shouldSerialize() == true )
            {
                prepareQueries();
                mDeleteLayer.bindParameter( 1, mLayers[i].layerId );
                if ( mDeleteLayer.step( true ) == false )
                    continue;
            
            } // End if should serialize

            // ToDo: remove.
            #if defined(_DEBUG)
                cgAppLog::write( cgAppLog::Debug, _T("Disposing of layer %i.\n"), i );
            #endif

            // Dispose of unnecessary data.
            mLayers[i].blendMap.clear();
            mLayers[i].blendMapPaint.clear();
            
            // Shuffle data back.
            for ( cgInt32 j = i + 1; j < nCount; ++j )
                mLayers[j - 1] = mLayers[j];
            
            // Test next.
            nCount--;
            i--;

        } // End if unreferenced
        else
        {
            // Update the database as necessary.
            if ( mLayers[i].layerId != 0 && mParentLandscape->shouldSerialize() == true )
            {
                prepareQueries();
                const cgLandscapeTextureData::LayerReference & Layer = mLayers[i];
                mUpdateLayerPaintData.bindParameter( 1, &Layer.blendMap[0], mBlendMapSize.width * mBlendMapSize.height );
                mUpdateLayerPaintData.bindParameter( 2, Layer.referenceCount );
                mUpdateLayerPaintData.bindParameter( 3, Layer.layerId );
                
                // Execute
                if ( mUpdateLayerPaintData.step( true ) == false )
                {
                    cgString strError;
                    mUpdateLayerPaintData.getLastError( strError );
                    cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Failed to update block blend map layer '%i' for block '%i' within landscape object '0x%x'. Error: %s\n"), Layer.layerId, mParentBlock->getDatabaseId(), mParentLandscape->getDatabaseId(), strError.c_str() );
                
                } // End if failed

            } // End if should serialize
        
        } // End if referenced

    } // Next layer

    // Resize, or dispose if nothing remained.
    if ( nCount == 0 )
    {
        mLayers.clear();
        
        // Re-optimize layer data
        optimizeLayers();

    } // End if empty
    else if ( nCount != mLayers.size() )
    {
        mLayers.resize( nCount );

        // Re-optimize layer data
        optimizeLayers();

    } // End if smaller    
}

//-----------------------------------------------------------------------------
// Name : filterPaintLayer() (Protected)
/// <summary>
/// Filter the data contained in the current paint layer's temporary blend map
/// buffer. This will be filtered based on the current radii of the painting
/// tool to give us the smooth / feathered edge we're after.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::filterPaintLayer( cgByteArray & aData )
{
    cgByteArray aBuffer;
    cgInt32     x, y, x2, y2, nKernelSizeX, nKernelSizeY;
    cgInt32     xStart, yStart, xEnd, yEnd;
    cgInt32     nHalfKernelSizeX, nHalfKernelSizeY, xStart2, yStart2, xPitch2, yPitch2;
    cgVector2   vecTexelSize;
    
    // Clone the paint layer.
    LayerReference & PaintLayer = mLayers[ mPaintLayer ];
    
    // Compute the world space "size" of an individual cell in the blend map
    const cgVector3 & vecScale = mParentLandscape->getTerrainScale();
    vecTexelSize.x = vecScale.x / mBlendMapResolution.width;
    vecTexelSize.y = vecScale.z / mBlendMapResolution.height;
    
    // Compute the size of the filter kernel used to apply the filter to the blend map.
    nHalfKernelSizeX = (cgInt32)(((mParams.outerRadius - mParams.innerRadius) * 0.5f) / vecTexelSize.x);
    nHalfKernelSizeY = (cgInt32)(((mParams.outerRadius - mParams.innerRadius) * 0.5f) / vecTexelSize.y);
    nKernelSizeX     = 1 + (nHalfKernelSizeX * 2);
    nKernelSizeY     = 1 + (nHalfKernelSizeY * 2);
    if ( nKernelSizeX < 3 || nKernelSizeY < 3 )
    {
        // No filtering, just duplicate the paint buffer.
        aData = PaintLayer.blendMapPaint;
        return;
    
    } // End if no filter

    // We need a copy of the painted area for any part of the entire landscape that falls
    // within nHalfKernelSize around the dirty rectangle of this block. This can then
    // be used to seamlessly filter across terrain block boundaries etc.
    cgRect rcSource;
    rcSource.left   = PaintLayer.dirtyRectangle.left - nHalfKernelSizeX;
    rcSource.top    = PaintLayer.dirtyRectangle.top - nHalfKernelSizeY;
    rcSource.right  = rcSource.left + PaintLayer.dirtyRectangle.width() + (nHalfKernelSizeX*2);
    rcSource.bottom = rcSource.top + PaintLayer.dirtyRectangle.height() + (nHalfKernelSizeY*2);

    // Translate the source rectangle into a global terrain rectangle
    cgRect rcBlendMapArea = getBlendMapArea();
    cgRect rcGlobal = rcSource;
    rcGlobal.left   += rcBlendMapArea.left;
    rcGlobal.top    += rcBlendMapArea.top;
    rcGlobal.right  += rcBlendMapArea.left;
    rcGlobal.bottom += rcBlendMapArea.top;

    // Retrieve a copy of the painted area of the entire lanscape as required.
    aData.clear();
    mParentLandscape->getBlendMapPaintData( rcGlobal, aData );

    // Compute the region of the scene paint data that will be filtered
    // NB: With a separable filter, the first pass must filter nHalfKernelSize
    // outside of the bounds of the dirty region (so that the second pass can
    // correctly consume filtered data).
    xStart = 0;
    xEnd   = rcSource.width();
    yStart = 0;
    yEnd   = rcSource.height();
    
    // Filter each texel in the dirty rectangle - Seperable U axis filter.
    aBuffer = aData;
    cgByte * pDest = &aBuffer[xStart + yStart * rcSource.width()];
    for ( y = yStart; y < yEnd; ++y )
    {
        for ( x = xStart; x < xEnd; ++x )
        {
            cgInt32 nValue   = 0;
            cgInt32 nSamples = 0;
            
            // Select all texels within the following kernel rectangle.
            xStart2 = x - nHalfKernelSizeX;
            xStart2 = max( 0, xStart2 );
            xPitch2 = (x + nHalfKernelSizeX) + 1;
            xPitch2 = min( rcSource.width(), xPitch2 ) - xStart2;
            
            // Sample
            cgByte * pSrc = &aData[xStart2 + y * rcSource.width()];
            for ( x2 = 0; x2 < xPitch2; ++x2 )
            {
                nValue += (cgInt32)(*pSrc++);
                nSamples++;

            } // Next Row

            // Write new value
            if ( nSamples == 0 )
            {
                *pDest++ = 0;
            
            } // End if nothing found
            else
            {
                nValue = (cgInt32)((cgFloat)nValue / (cgFloat)nSamples);
                *pDest++ = (cgByte)min(255,nValue);

            } // End if sampled

        } // Next Column

        // Move down to next row
        pDest += rcSource.width() - (xEnd - xStart);

    } // Next Row

    // Switch buffers (ping-pong)
    aData = PaintLayer.blendMapPaint;
    
    // The second pass filter need only update the dirty region.
    xStart = nHalfKernelSizeX;
    xEnd   = xStart + PaintLayer.dirtyRectangle.width();
    yStart = nHalfKernelSizeY;
    yEnd   = yStart + PaintLayer.dirtyRectangle.height();

    // Filter each texel in the dirty rectangle - Seperable V axis filter.
    pDest = &aData[PaintLayer.dirtyRectangle.left + PaintLayer.dirtyRectangle.top * mBlendMapSize.width];
    for ( y = yStart; y < yEnd; ++y )
    {
        for ( x = xStart; x < xEnd; ++x )
        {
            int nValue   = 0;
            int nSamples = 0;
            
            // Select all texels within the following kernel rectangle.
            yStart2 = y - nHalfKernelSizeY;
            yStart2 = max( 0, yStart2 );
            yPitch2 = (y + nHalfKernelSizeY) + 1;
            yPitch2 = min( rcSource.height(), yPitch2 ) - yStart2;

            // Sample
            cgByte * pSrc = &aBuffer[x + yStart2 * rcSource.width()];
            for ( y2 = 0; y2 < yPitch2; ++y2 )
            {
                nValue += (int)*pSrc;
                nSamples++;
                
                // Move down to next row
                pSrc += rcSource.width();

            } // Next Column

            // Write new value
            if ( nSamples == 0 )
            {
                *pDest++ = 0;
            
            } // End if nothing found
            else
            {
                nValue = (cgInt32)((cgFloat)nValue / (cgFloat)nSamples);
                *pDest++ = (cgByte)min(255,nValue);

            } // End if sampled

        } // Next Column

        // Move down to next row
        pDest += mBlendMapSize.width - (xEnd - xStart);

    } // Next Row

    // aData now contains filtered buffer.
}

//-----------------------------------------------------------------------------
// Name : getLayers()
/// <summary>
/// Retrieve a list of all the terrain texture layers associated with this
/// block.
/// </summary>
//-----------------------------------------------------------------------------
const cgLandscapeTextureData::LayerReferenceArray & cgLandscapeTextureData::getLayers( ) const
{
    return mLayers;
}

//-----------------------------------------------------------------------------
// Name : updatePaintPreview() (Protected)
/// <summary>
/// Update textures for preview during the painting process.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::updatePaintPreview(  )
{
    cgInt32     x, y, nLayerCount, nPaintValue;
    cgUInt32    nPitch, nNewValue, * pPitchData = CG_NULL;
    cgByte    * pDest, **ppLockData = CG_NULL;
    cgFloat   * pWeights = CG_NULL;
    cgFloat     fWeightScale, fRemainingWeight, fLayerWeights;
    
    // Validate Requirements
    if ( mIsPainting == false )
        return;

    // Is anything to be done?
    LayerReference & PaintLayer = mLayers[ mPaintLayer ];
    if ( PaintLayer.dirty == false )
        return;

    // Filter the dirty rectangle of the paint texture as required based on the selected 
    // "hardness" of the painting tool.
    // ToDo: Statically allocated buffer?
    cgByteArray aFilteredBuffer;
    filterPaintLayer( aFilteredBuffer );

    // Get access to required systems
    cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();

    // Compute block details
    const cgVector3 & vScale  = mParentLandscape->getTerrainScale();
    const cgSize & BlockLayout  = mParentLandscape->getBlockLayout();
    const cgSize & BlockSize    = mParentLandscape->getBlockSize();
    const cgSize  BlockQuads    = cgSize( BlockSize.width - 1, BlockSize.height - 1 );
    const cgSize  HeightMapSize = mParentLandscape->getHeightMapSize();
    const cgSize  TerrainQuads  = cgSize( HeightMapSize.width - 1, HeightMapSize.height - 1 );
    const cgInt32 nBlockStartX  = (mParentBlock->getBlockIndex() % BlockLayout.width) * BlockQuads.width;
    const cgInt32 nBlockStartY  = (mParentBlock->getBlockIndex() / BlockLayout.width) * BlockQuads.height;
    
    // First, lock each of the combined blend map textures.
    ppLockData = new cgByte*[mCombinedBlendMaps.size()];
    pPitchData = new cgUInt32[mCombinedBlendMaps.size()];
    for ( size_t i = 0; i < mCombinedBlendMaps.size(); ++i )
    {
        cgTexture * pTexture = mCombinedBlendMaps[ i ].texture.getResource(true);
        ppLockData[i] = (cgByte*)pTexture->lock( PaintLayer.dirtyRectangle, pPitchData[i], cgLockFlags::WriteOnly );

        // ToDo: Handle errors
    
    } // Next Texture

    // We're only updating the area indicated by the dirty rectangle.
    cgByte * pOriginal = &PaintLayer.blendMap[PaintLayer.dirtyRectangle.left + PaintLayer.dirtyRectangle.top * mBlendMapSize.width];
    cgByte * pPaint    = &aFilteredBuffer[PaintLayer.dirtyRectangle.left + PaintLayer.dirtyRectangle.top * mBlendMapSize.width];

    // To save some time, we'll also keep a pointer to the paint layer's 
    // texture data (Offset to the correct 8bit layer location (BGRA)) /unless/
    // this is a temporary erase layer (which has no physical texture data).
    if ( PaintLayer.eraseLayer == false )
    {
        pDest  = ppLockData[ PaintLayer.combinedBlendMap ] + (3 - PaintLayer.channel);
        nPitch = pPitchData[ PaintLayer.combinedBlendMap ];
    
    } // End if !erase layer

    // Allocate enough space in a temporary buffer to hold a weight for a 
    // single pixel of each layer's blend map (used for normalization).
    nLayerCount = (cgInt32)mLayers.size();
    pWeights = new cgFloat[ nLayerCount ];

    // Lock the landscape's normal texture so we can rapidly access
    // normal data for advanced procedural mapping.
    cgUInt32 * pNormalData = CG_NULL;
    cgUInt32 nNormalPitch = 0;
    if ( mParams.enableSlopeEffect == true )
    {
        cgTexture * pNormalTex = mParentLandscape->getNormalTexture().getResource(true);
        pNormalData = (cgUInt32*)pNormalTex->lock( nNormalPitch, cgLockFlags::ReadOnly );
        nNormalPitch /= sizeof(cgUInt32);
    
    } // End if lock normals

    // Iterate through each texel in the dirty rectangle
    for ( y = 0; y < PaintLayer.dirtyRectangle.height(); ++y )
    {
        for ( x = 0; x < PaintLayer.dirtyRectangle.width(); ++x, ++pOriginal, ++pPaint, pDest += 4 )
        {
            // Advanced parameters selected?
            nPaintValue = 0;
            if ( mParams.enableHeightEffect == true || mParams.enableSlopeEffect == true )
            {
                bool bCull = false;

                // First compute the terrain U/V location (0 to 1 range) for the center of this texel.
                cgFloat fX = (cgFloat)nBlockStartX + (((cgFloat)(PaintLayer.dirtyRectangle.left + x) + 0.5f) / mBlendMapResolution.width);
                cgFloat fY = (cgFloat)nBlockStartY + (((cgFloat)(PaintLayer.dirtyRectangle.top + y) + 0.5f) / mBlendMapResolution.height);
                cgFloat fU = fX / (cgFloat)TerrainQuads.width;
                cgFloat fV = fY / (cgFloat)TerrainQuads.height;

                // Height parameters enabled?
                cgFloat fParamWeight = 1.0f;
                if ( mParams.enableHeightEffect == true )
                {
                    // Retrieve the height at this location in the terrain.
                    cgFloat fHeight = mParentBlock->getTerrainHeight( fX * vScale.x, fY * -vScale.z, false );

                    // Can be culled?
                    if ( fHeight < (mParams.minimumHeight - mParams.heightAttenuation) || fHeight > (mParams.maximumHeight + mParams.heightAttenuation) )
                    {
                        // Just cull, completely outside range.
                        bCull = true;
                    
                    } // End if outside range.
                    else
                    {
                        // Compute parameter weight
                        if ( fHeight > mParams.maximumHeight )
                            fParamWeight = (mParams.heightAttenuation - (fHeight - mParams.maximumHeight)) / mParams.heightAttenuation;
                        else if ( fHeight < mParams.minimumHeight )
                            fParamWeight = (mParams.heightAttenuation - (mParams.minimumHeight - fHeight)) / mParams.heightAttenuation;

                        // Clamp
                        if ( fParamWeight < 0.0f )
                            fParamWeight = 0.0f;
                        else if ( fParamWeight > 1.0f )
                            fParamWeight = 1.0f;

                    } // End if within range.

                } // End if height effect enabled

                if ( mParams.enableSlopeEffect == true )
                {
                    cgVector3 vNormal;

                    // We're going to bilinearly sample the normal from the 
                    // landscape's normal map. First compute base bilinear coordinates
                    cgFloat fUBilin = fX - 0.5f;
                    cgFloat fVBilin = fY - 0.5f;

                    // Compute the four pixel locations based on this coordinate
                    cgPoint ptPixels[4];
                    ptPixels[0].x = (cgInt32)floorf(fUBilin);
                    ptPixels[0].y = (cgInt32)floorf(fVBilin);
                    ptPixels[1].x = ptPixels[0].x + 1;
                    ptPixels[1].y = ptPixels[0].y;
                    ptPixels[2].x = ptPixels[0].x;
                    ptPixels[2].y = ptPixels[0].y + 1;
                    ptPixels[3].x = ptPixels[0].x + 1;
                    ptPixels[3].y = ptPixels[0].y + 1;

                    // Retrieve the four color values from the normal map.
                    cgColorValue NormalColors[4];
                    cgInt32 nMaxWidth = TerrainQuads.width - 1;
                    cgInt32 nMaxHeight = TerrainQuads.height - 1;
                    for ( cgInt32 i = 0; i < 4; ++i )
                    {
                        cgPoint ptPixel = ptPixels[i];
                        
                        // Retrieve and clamp pixel location
                        if ( ptPixel.x < 0 )
                            ptPixel.x = 0;
                        else if ( ptPixel.x >= nMaxWidth )
                            ptPixel.x = nMaxWidth - 1;

                        if ( ptPixel.y < 0 )
                            ptPixel.y = 0;
                        else if ( ptPixel.y >= nMaxHeight )
                            ptPixel.y = nMaxHeight - 1;

                        // Retrieve color value
                        NormalColors[i] = cgColorValue( *(pNormalData + ptPixel.x + ptPixel.y * nNormalPitch) );

                    } // Next Location

                    // Retrieve the fractional component used to generate weights
                    cgFloat fUFrac = fmodf( fUBilin, 1.0f );
                    cgFloat fVFrac = fmodf( fVBilin, 1.0f );

                    // Compute final weighted normal
                    vNormal = (cgVector3&)NormalColors[0] * (1.0f - fUFrac) * (1.0f - fVFrac) +
                              (cgVector3&)NormalColors[1] * fUFrac * (1.0f - fVFrac) +
                              (cgVector3&)NormalColors[2] * (1.0f - fUFrac) * fVFrac +
                              (cgVector3&)NormalColors[3] * fUFrac * fVFrac;                    

                    // Signed value. Scale into -1 to +1 range.
                    vNormal = 2.0f * vNormal - cgVector3(1.0f, 1.0f, 1.0f );

                    // Generate final weight
                    cgFloat fDot = cgVector3::dot( vNormal, mParams.slopeAxis );
                    if ( mParams.invertSlope == false )
                        fDot = (1.0f - (fDot*fDot)) * mParams.slopeScale + mParams.slopeBias;
                    else
                        fDot = (fDot*fDot) * mParams.slopeScale + mParams.slopeBias;

                    // Saturate
                    if ( fDot < 0.0f )
                        fDot = 0.0f;
                    else if ( fDot > 1.0f )
                        fDot = 1.0f;

                    // apply
                    fParamWeight *= fDot;
                    
                } // End if height effect enabled

                // First compute weighted paint value.
                if ( bCull == false )
                    nPaintValue = (cgInt32)( ((cgFloat)mParams.strength / 100.0f) * fParamWeight * (cgFloat)(*pPaint));

            } // End if advanced
            else
            {
                // First compute simple weighted paint value.
                nPaintValue = (cgInt32)( ((cgFloat)mParams.strength / 100.0f) * (cgFloat)(*pPaint));
            
            } // End if no advanced

            // Pixel survived?
            if ( nPaintValue > 0 )
            {
                // Compute the new weight for the paint layer
                nNewValue = (cgInt32)(*pOriginal) + nPaintValue;
                nNewValue = min(0xFF,nNewValue);

                // Write out to the paint layer's physical texture (if not an
                // erase layer which has no physical texture)
                if ( PaintLayer.eraseLayer == false )
                    *pDest = (cgByte)nNewValue;

                // Compute the remaining "weight" available for all other layers
                // at this location.
                fRemainingWeight = 1.0f - ((cgFloat)(nNewValue) / 255.0f);

                // Sum together the current weights of the alternate layers.
                fLayerWeights = 0.0f;
                for ( cgInt32 i = 0; i < nLayerCount; ++i )
                {
                    // Skip the paint layer.
                    if ( i == mPaintLayer )
                        continue;

                    // Sum the weight for this layer.
                    LayerReference & Layer = mLayers[i];
                    pWeights[i] = ((cgFloat)*(ppLockData[Layer.combinedBlendMap] + (3 - Layer.channel))) / 255.0f;
                    fLayerWeights += pWeights[i];

                } // Next Layer

                // If the new value we wrote above causes a total weight larger than that allowed 
                // within the normalized range (0-1), normalize the remaining layers appropriately.
                if ( fLayerWeights > 0 && fLayerWeights > fRemainingWeight )
                {
                    fWeightScale = fRemainingWeight / fLayerWeights;

                    // Write new weight value.
                    for ( cgInt32 i = 0; i < nLayerCount; ++i )
                    {
                        // Skip the paint layer.
                        if ( i == mPaintLayer )
                            continue;

                        // Compute new weight
                        LayerReference & Layer = mLayers[i];
                        nNewValue = (cgInt32)((pWeights[i] * fWeightScale) * 255.0f);
                        *(ppLockData[Layer.combinedBlendMap] + (3 - Layer.channel)) = (cgByte)min( 0xFF, nNewValue );

                    } // Next Layer
                
                } // End if normalize

            } // End if pixel survived
            
            // Next column
            for ( size_t i = 0; i < mCombinedBlendMaps.size(); ++i )
                ppLockData[i] += 4;

        } // Next Column
        
        // Move down to next row in destination texture.
        pDest     += nPitch - (PaintLayer.dirtyRectangle.width() * 4);
        pOriginal += mBlendMapSize.width - PaintLayer.dirtyRectangle.width();
        pPaint    += mBlendMapSize.width - PaintLayer.dirtyRectangle.width();
        
        
        // Increment lock data ready for next row
        for ( size_t i = 0; i < mCombinedBlendMaps.size(); ++i )
            ppLockData[i] += pPitchData[i] - (PaintLayer.dirtyRectangle.width() * 4);

    } // Next Row

    // Clean up
    delete []pWeights;
    aFilteredBuffer.clear();

    // Unlock each of the combined blend map textures.
    for ( size_t i = 0; i < mCombinedBlendMaps.size(); ++i )
    {
        cgTexture * pTexture = mCombinedBlendMaps[ i ].texture.getResource(true);
        pTexture->unlock( false );
    
    } // Next Texture

    // Unlock landscape's normal texture if locked.
    if ( nNormalPitch > 0 )
    {
        cgTexture * pNormalTexture = mParentLandscape->getNormalTexture().getResource(true);
        pNormalTexture->unlock( false );
    
    } // End if unlock normals

    // Reset paint layer dirty status.
    PaintLayer.dirty  = false;
    PaintLayer.dirtyRectangle = cgRect( INT_MAX, INT_MAX, -INT_MAX, -INT_MAX );
}

//-----------------------------------------------------------------------------
// Name : updateLayerTexture() (Protected)
/// <summary>
/// Called in order to reconstruct the renderable blend map for a layer.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::updateLayerTexture( LayerReference & Layer, const cgRect & rcUpdate )
{
    cgInt32  x, y;
    cgUInt32 nPitch;

    // Skip if this is an erase layer (it has no texture)
    if ( Layer.eraseLayer == true )
        return;

    // Lock and fill the texture
    cgTexture * pTexture = mCombinedBlendMaps[ Layer.combinedBlendMap ].texture.getResource(true);
    cgByte    * pDest    = (cgByte*)pTexture->lock( rcUpdate, nPitch, cgLockFlags::WriteOnly );
    
    // ToDo: Handle errors

    // Offset to the correct 8bit layer location (BGRA)
    pDest += (3 - Layer.channel);
 
    // We're only updating the area indicated by the rectangle.
    cgByte * pSrc = &Layer.blendMap[rcUpdate.left + rcUpdate.top * mBlendMapSize.width];

    // Simply duplicate the data (interleaved between other layer channels).
    for ( y = 0; y < rcUpdate.height(); ++y )
    {
        for ( x = 0; x < rcUpdate.width(); ++x, pDest += 4, ++pSrc )
            *pDest = *pSrc;

        // Move down to next row.
        pDest += nPitch - (rcUpdate.width() * 4);
        pSrc  += mBlendMapSize.width - rcUpdate.width();

    } // Next Row
    
    // Clean up
    pTexture->unlock( false );
}

//-----------------------------------------------------------------------------
// Name : optimizeLayers() (Protected)
/// <summary>
/// In order to render the terrain as efficiently as possible, layers are
/// collected together into groups of 4 for batch drawing. This requires that
/// the blend maps for these four layers exist together in the four available
/// channels of the same texture.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::optimizeLayers()
{
    cgInt32 nRemainingLayers, nRemainingBumpLayers, nBumpLayersInBatch = 0, nLayersInBatch = 0;
    cgInt32 nSelectedLayer = -1, nCurrentTexture = -1;
    bool    bNewBatch = true, bSwizzledLast = false;

    ///////////////////////////////////////////////////////////////////////////
    // Generate optimized batch ordering.
    ///////////////////////////////////////////////////////////////////////////

    // Reset draw pass data.
    mRenderBatches.clear();

    // Build a temporary array used to record whether or not a layer
    // has already been assigned or not.
    cgArray<bool> aLayerAssigned( mLayers.size(), false );

    // Count the number of remaining layers of required types.
    nRemainingLayers     = 0;
    nRemainingBumpLayers = 0;
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        // If this is an erase layer, it doesn't count.
        if ( mLayers[i].eraseLayer == true )
        {
            aLayerAssigned[i] = true;
            continue;
        
        } // End if erase layer
        
        // Valid layer requires a channel in a texture.
        nRemainingLayers++;

        // This is a bump layer too?
        cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)mLayers[i].layerType.getResource(false);
        if ( pType->getNormalSampler() != CG_NULL )
            nRemainingBumpLayers++;

    } // Next Layer

    // Search for available layers until all are batched appropriately
    for ( ; nRemainingLayers != 0; )
    {
        // Reset tracking variables as required
        if ( bNewBatch == true )
        {
            mRenderBatches.resize( mRenderBatches.size() + 1 );
            RenderBatch & NewBatch = mRenderBatches.back();

            // Construct initial default parameters.
            NewBatch.layerCount       = 0;
            NewBatch.swizzle          = false;
            NewBatch.maximumLayers        = 4;
            
            // Have prior batches been constructed (before this one)?
            if ( mRenderBatches.size() > 1 )
            {
                // This is not the first batch we've added. Can we continue 
                // to pack data into the prior texture?
                if ( nLayersInBatch <= 2 && bSwizzledLast == false )
                {
                    // The prior batch used only 2 of the 4 available channels in 
                    // the texture. We can add some more.
                    NewBatch.maximumLayers = 2;
                    NewBatch.swizzle   = true;
                    bSwizzledLast       = true;
                
                } // End if we can swizzle.
                else
                {
                    // We need a new texture.
                    ++nCurrentTexture;
                    bSwizzledLast = false;

                } // End if we can't swizzle

            } // End if not first
            else
            {
                // We need a new texture
                ++nCurrentTexture;
                bSwizzledLast = false;
            
            } // End if first

            // Add to the appropriate texture.
            NewBatch.combinedBlendMap = nCurrentTexture;

            // Reset layer counters
            nBumpLayersInBatch = 0;
            nLayersInBatch     = 0;

            // Ready!
            bNewBatch = false;
        
        } // End if creating new batch

        // If there are bump slots available (and bump maps remaining), first 
        // search for a valid bump layer.
        nSelectedLayer = -1;
        if ( nBumpLayersInBatch < 2 && nRemainingBumpLayers > 0 )
        {
            for ( size_t i = 0; i < mLayers.size(); ++i )
            {
                // If this layer has already been assigned, skip it
                if ( aLayerAssigned[i] == true )
                    continue;

                // Select this if it is a bump layer.
                cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)mLayers[i].layerType.getResource(false);
                if ( pType->getNormalSampler() != CG_NULL )
                {
                    nSelectedLayer = i;
                    break;
                
                } // End if bump layer                

            } // Next active layer.

        } // End if bump slots available

        // Did we find a bump layer?
        if ( nSelectedLayer >= 0 )
        {
            // Update counters
            nBumpLayersInBatch++;
            nLayersInBatch++;
            nRemainingLayers--;
            nRemainingBumpLayers--;

            // Add to batch.
            RenderBatch & Batch = mRenderBatches.back();
            Batch.layers[ Batch.layerCount++ ] = nSelectedLayer;
            aLayerAssigned[ nSelectedLayer ] = true;
        
        } // End if add bump layer
        else
        {
            // Search for a color only layer
            for ( size_t i = 0; i < mLayers.size(); ++i )
            {
                // If this layer has already been assigned, skip it
                if ( aLayerAssigned[i] == true )
                    continue;

                // Select this if it is NOT a bump layer.
                cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)mLayers[i].layerType.getResource(false);
                if ( pType->getNormalSampler() == CG_NULL )
                {
                    nSelectedLayer = i;
                    break;
                
                } // End if bump layer                

            } // Next active layer.

            // Did we find an available color layer?
            if ( nSelectedLayer >= 0 )
            {
                // Update counters
                nLayersInBatch++;
                nRemainingLayers--;

                // Add to batch.
                RenderBatch & Batch = mRenderBatches.back();
                Batch.layers[ Batch.layerCount++ ] = nSelectedLayer;
                aLayerAssigned[ nSelectedLayer ] = true;

            } // End if found color
            else
            {
                // Nothing suitable found, commit the batch.
                bNewBatch = true;
            
            } // End if no color

        } // End if add color layer

        // Have we fully populated this batch?
        RenderBatch & Batch = mRenderBatches.back();
        if ( Batch.layerCount == Batch.maximumLayers )
            bNewBatch = true;

    } // Next Layer

    // Clean up
    aLayerAssigned.clear();

    ///////////////////////////////////////////////////////////////////////////
    // Create enough blend maps for the final packed data.
    ///////////////////////////////////////////////////////////////////////////

    // Get access to required systems
    cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();

    // Ensure we have enough "combined" blend map textures to house
    // the necessary rendering data.
    size_t nTextureCount = nCurrentTexture + 1;
    if ( nTextureCount != mCombinedBlendMaps.size() )
    {
        // Larger or smaller?
        if ( nTextureCount < mCombinedBlendMaps.size() )
        {
            // Smaller. Release prior data.
            for ( size_t i = nTextureCount; i < mCombinedBlendMaps.size(); ++i )
            {
                BatchedMap & BlendMap = mCombinedBlendMaps[i];
                BlendMap.texture.close();
            
            } // Next batch texture.

        } // End if smaller
        
        // Resize
        mCombinedBlendMaps.resize( nTextureCount );
    
    } // End if resizing

    // Allocate required textures / reset usage data for all combined maps
    for ( size_t i = 0; i < mCombinedBlendMaps.size(); ++i )
    {
        BatchedMap & BlendMap = mCombinedBlendMaps[i];

        // Allocate texture?
        if ( BlendMap.texture.isValid() == false )
        {
            // Create a new 32 bit texture for packing.
            cgToDo( "DX11", "Texture format needs considering more given lack of ARGB in DX10+ (Use GetBestFormat()?)" );
            pResources->createTexture( &BlendMap.texture, cgBufferType::Texture2D, mBlendMapSize.width, mBlendMapSize.height, 0, 1,
                                       cgBufferFormat::B8G8R8A8, cgMemoryPool::Managed, false, cgResourceFlags::ForceNew, cgString::Empty, 
                                       cgDebugSource() );
                                         
            // ToDo: Test for error.

        } // End if allocate texture

        // Reset channel usage.
        BlendMap.channelUsage = 0;

    } // Next Map

    ///////////////////////////////////////////////////////////////////////////
    // Set final layer location and update textures.
    ///////////////////////////////////////////////////////////////////////////

    // Update layers to point to their new texture / channel.
    for ( size_t i = 0; i < mRenderBatches.size(); ++i )
    {
        RenderBatch & Batch    = mRenderBatches[i];
        BatchedMap  & BlendMap = mCombinedBlendMaps[ Batch.combinedBlendMap ];

        // For each layer referenced by the batch
        for ( cgInt32 j = 0; j < Batch.layerCount; ++j )
        {
            LayerReference & Layer = mLayers[Batch.layers[j]];

            // Which channel will this layer reference?
            cgInt32 nChannel = j + ((Batch.swizzle == true) ? 2 : 0);

            // Has layer location been altered?
            if ( Layer.combinedBlendMap != Batch.combinedBlendMap || Layer.channel != nChannel )
            {
                // Layer data has "moved". Update final layer location.
                Layer.combinedBlendMap = Batch.combinedBlendMap;
                Layer.channel          = (cgByte)nChannel;
                
                // Update the relevant channel in the texture.
                updateLayerTexture( Layer, cgRect( 0, 0, mBlendMapSize.width, mBlendMapSize.height ) );
            
            } // End if changed

            // Update combined blend map channel usage information
            BlendMap.channelUsage |= (1 << Layer.channel);

        } // Next Layer

    } // Next render batch

    ///////////////////////////////////////////////////////////////////////////
    // Update the rendering constant data.
    ///////////////////////////////////////////////////////////////////////////

    // First generate and populate the layer data constant buffers for 
    // each rendering batch.
    for ( size_t i = 0; i < mRenderBatches.size(); ++i )
    {
        RenderBatch & Batch = mRenderBatches[i];

        // Allocate the layer data constant buffer for this batch.
        if ( !pResources->createConstantBuffer( &Batch.layerDataBuffer, mParentLandscape->mLandscapeShader, _T("cbTerrainLayerData"), cgDebugSource() ) )
        {
            // ToDo: Log error.
            return;
        
        } // End if failed

        // Lock the buffer ready for population.
        cgConstantBuffer * pBatchBuffer = Batch.layerDataBuffer.getResource( true );
        cgAssert( pBatchBuffer != CG_NULL );
        cgLandscape::cbTerrainLayerData * pBatchData = CG_NULL;
        if ( !(pBatchData = (cgLandscape::cbTerrainLayerData*)pBatchBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to lock terrain painted layer data constant buffer in preparation for device update.\n") );
            return;

        } // End if failed

        // Collect the layer states
        for ( cgInt32 j = 0; j < Batch.layerCount; ++j )
        {
            cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)mLayers[Batch.layers[j]].layerType.getResource();

            // Collect texturing properties
            cgLandscape::ctLayerData & Data = pBatchData->layers[j];
            cgToDo( "Carbon General", "Use actual texture size?" )
            Data.textureSize  = cgVector4( 512.0f, 512.0f, 1.0f / 512.0f, 1.0f / 512.0f );
            Data.scale        = pType->getScale();
            Data.baseScale    = pType->getBaseScale();
            Data.offset       = -pType->getOffset();
            Data.rotation.x   = cosf(CGEToRadian(360.0f - pType->getAngle()));
            Data.rotation.y   = -sinf(CGEToRadian(360.0f - pType->getAngle()));
            Data.rotation.z   = -Data.rotation.y;
            Data.rotation.w   = Data.rotation.x;

        } // Next Layer

        // Buffer is populated.
        pBatchBuffer->unlock();

    } // Next batch

    // Now, create the high level constant buffer that describes the 
    // overall painted layer rendering procedure.
    if ( !pResources->createConstantBuffer( &mTerrainPaintDataBuffer, mParentLandscape->mLandscapeShader, _T("cbTerrainPaintData"), cgDebugSource() ) )
    {
        // ToDo: Log error.
        return;
    
    } // End if failed

    // Lock the buffer ready for population.
    cgConstantBuffer * pPaintedDataBuffer = mTerrainPaintDataBuffer.getResource( true );
    cgAssert( pPaintedDataBuffer != CG_NULL );
    cbTerrainPaintData * pPaintData = CG_NULL;
    if ( !(pPaintData = (cbTerrainPaintData*)pPaintedDataBuffer->lock( 0, 0, cgLockFlags::WriteOnly | cgLockFlags::Discard ) ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to lock terrain painted layer process configuration constant buffer in preparation for device update.\n") );
        return;

    } // End if failed

    // Compute the correct scale and offset to use in order to
    // compute blend map coordinates for this block.
    const cgSize & BlockLayout = mParentLandscape->getBlockLayout();
    cgInt32 nBlockX     = mParentBlock->getBlockIndex() % BlockLayout.width;
    cgInt32 nBlockY     = mParentBlock->getBlockIndex() / BlockLayout.width;
    cgVector2 vecScale  = cgVector2 ( (cgFloat)BlockLayout.width, (cgFloat)BlockLayout.height );
    cgVector2 vecOffset = cgVector2 ( (cgFloat)-nBlockX, (cgFloat)-nBlockY );

    // Actual texture has a 1 texel bilinear filtering correction border
    cgVector2 vecBorderScale, vecBorderOffset;
    vecBorderScale.x  = (cgFloat)mActiveBlendMapBounds.width() / (cgFloat)mBlendMapSize.width;
    vecBorderScale.y  = (cgFloat)mActiveBlendMapBounds.height() / (cgFloat)mBlendMapSize.height;
    vecBorderOffset.x = 1.0f / (cgFloat)mBlendMapSize.width;
    vecBorderOffset.y = 1.0f / (cgFloat)mBlendMapSize.height;

    // Adjust our single scale and bias factor and store in the buffer.
    pPaintData->blendMapScale.x  = vecScale.x * vecBorderScale.x;
    pPaintData->blendMapScale.y  = vecScale.y * vecBorderScale.x;
    pPaintData->blendMapOffset.x = vecOffset.x * vecBorderScale.x + vecBorderOffset.x;
    pPaintData->blendMapOffset.y = vecOffset.y * vecBorderScale.y + vecBorderOffset.y;

    // Shader needs to understand how many passes are incoming.
    pPaintData->passCount.x = (cgFloat)mRenderBatches.size();
    pPaintData->passCount.y = 1.0f / (cgFloat)mRenderBatches.size();

    // Buffer is populated.
    pPaintedDataBuffer->unlock();
}

//-----------------------------------------------------------------------------
// Name : beginDraw()
/// <summary>
/// Called during rendering in order to setup necessary device states.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::beginDraw( cgRenderDriver * pDriver )
{
    // Reset current rendering pass.
    mCurrentPass = 0;

    // We only need to draw in passes if we have any data.
    if ( mRenderBatches.empty() )
        return false;

    // Bind the appropriate constant buffers ready for procedural rendering.
    // The hardware side constant registers in these bound buffers will be 
    // automatically updated as necessary whenever the buffer contents are 
    // modified within the following rendering loops.
    pDriver->setConstantBufferAuto( mTerrainPaintDataBuffer );

    // ToDo: Remove
    /*for ( int i = 0; i < mCombinedBlendMaps.size(); ++i )
    {
        wchar_t Buffer[1024];
        wsprintf( Buffer, L"BatchMap%i.dds", i );
        LPDIRECT3DBASETEXTURE9 pTexture = (LPDIRECT3DBASETEXTURE9)mCombinedBlendMaps[i].texture.GetManagedData();
        D3DXSaveTextureToFile( Buffer, D3DXIFF_DDS, pTexture, CG_NULL );
        pTexture->release();
    }*/

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : beginDrawPass()
/// <summary>
/// Called during rendering in order to setup the device for each necessary
/// rendering pass. Returns false when no more passes are required.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::beginDrawPass( cgRenderDriver * pDriver )
{
    // Are we out of passes?
    if ( mRenderBatches.empty() || mCurrentPass >= (cgInt32)mRenderBatches.size() )
        return false;

    // Setup batch constants
    RenderBatch & Batch = mRenderBatches[ mCurrentPass ];
    pDriver->setConstantBufferAuto( Batch.layerDataBuffer );

    // Initialize shader permutation argument arrays.
    // ToDo: This can likely be pre-created.
    static cgScriptArgument::Array aVSArgs(1), aPSArgs(7);
    static bool pTileReduction[4] = {0};
    memset( pTileReduction, 0, sizeof(pTileReduction) );
    bool bViewSpaceNormals = (pDriver->getSystemState( cgSystemState::ViewSpaceLighting ) != 0);
    aPSArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &Batch.layerCount );
    aPSArgs[1] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool "), &Batch.swizzle );
    aPSArgs[2] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool "), &pTileReduction[0] );
    aPSArgs[3] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool "), &pTileReduction[1] );
    aPSArgs[4] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool "), &pTileReduction[2] );
    aPSArgs[5] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool "), &pTileReduction[3] );
    aPSArgs[6] = cgScriptArgument( cgScriptArgumentType::Bool, _T("bool"), &bViewSpaceNormals );
    aVSArgs[0] = cgScriptArgument( cgScriptArgumentType::DWord, _T("int"), &Batch.layerCount );

    // Set the necessary samplers and collect layer states
    for ( cgInt32 i = 0; i < Batch.layerCount; ++i )
    {
        cgLandscapeLayerMaterial * pType = (cgLandscapeLayerMaterial*)mLayers[Batch.layers[i]].layerType.getResource();

        // ToDo: 9999 - This is kinda manual, vs simply being able to call 'pType->Apply()'.
        // I'm not sure how we can make this more generic yet, but the following works for now.
        
        // Set the layer color sampler
        cgSampler * pSrcSampler = pType->getColorSampler();
        cgSampler * pDstSampler = mParentLandscape->getSharedColorSampler( i );
        if ( pSrcSampler == CG_NULL || pSrcSampler->isTextureValid() == false )
        {
            cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();
            pSrcSampler = pResources->getDefaultSampler( cgResourceManager::DefaultDiffuseSampler );
        
        } // End if no diffuse
        pDstSampler->setTexture( pSrcSampler->getTexture() );
        pDstSampler->setStates( pSrcSampler->getStates() );
        pDstSampler->apply();
        
        // Set the layer normal sampler (if this is a "bump" slot)
        if ( i < 2 )
        {
            pSrcSampler = pType->getNormalSampler();
            pDstSampler = mParentLandscape->getSharedNormalSampler( i );
            if ( pSrcSampler == CG_NULL || pSrcSampler->isTextureValid() == false )
            {
                cgResourceManager * pResources = mParentLandscape->getScene()->getResourceManager();
                pSrcSampler = pResources->getDefaultSampler( cgResourceManager::DefaultNormalSampler );
            
            } // End if no normal
            pDstSampler->setTexture( pSrcSampler->getTexture() );
            pDstSampler->setStates( pSrcSampler->getStates() );
            pDstSampler->apply();
        
        } // End if bump slot

        // Retrieve permutation parameters.
        pTileReduction[i] = pType->getTilingReduction();

    } // Next Layer

    // Select the vertex and pixel shader.
    cgSurfaceShader * pShader = mParentLandscape->mLandscapeShader.getResource(true);
    if ( !pShader->selectVertexShader( _T("transformPainted"), aVSArgs ) ||
         !pShader->selectPixelShader( _T("terrainDrawPainted"), aPSArgs ) )
         return false;

    // Set blend map
    cgSampler * pSampler = mParentLandscape->getBlendMapSampler();
    pSampler->apply( mCombinedBlendMaps[Batch.combinedBlendMap].texture );

    // Set required blend states.
    pDriver->setDepthStencilState( mParentLandscape->mPaintedDepthState );
    pDriver->setRasterizerState( cgRasterizerStateHandle::Null );
    if ( mCurrentPass == 0 )
    {
        // Set states required for the initial draw operation (blending disabled).
        pDriver->setBlendState( cgBlendStateHandle::Null );
    
    } // End if initial
    else
    {
        // Set states required for the remaining draw operations.
        pDriver->setBlendState( mParentLandscape->mPaintedBlendState );

    } // End if remaining

    // More passes potentially required.
    return true;
}

//-----------------------------------------------------------------------------
// Name : endDrawPass()
/// <summary>
/// Called during rendering in order to complete a rendering pass. Returns 
/// false when no more passes are required.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::endDrawPass( cgRenderDriver * pDriver )
{
    // Move on to next pass.
    mCurrentPass++;

    // Are we out of passes?
    if ( mRenderBatches.empty() || mCurrentPass >= (cgInt32)mRenderBatches.size() )
        return false;
    return true;
}

/*//-----------------------------------------------------------------------------
// Name : LayerTypeRemoved()
/// <summary>
/// Called by the parent landscape to let us know when a layer type has
/// been removed. This allows us to update our data sets as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::LayerTypeRemoved( cgLandscapeLayerType * pType, cgInt32 nTypeId )
{
    cgInt32 nRemoveItem = -1;

    // Validate requirements
    if ( mLayers.empty() )
        return;

    // Were we referencing this layer specifically? Or are we referencing 
    // any layers whose identifier will have changed?
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        LayerReference & Layer = mLayers[i];
        if ( Layer.nLayerType > nTypeId )
            Layer.nLayerType = Layer.nLayerType - 1;
        else if ( Layer.nLayerType == nTypeId )
            nRemoveItem = i;

    } // Next Layer

    // Anything to remove?
    if ( nRemoveItem >= 0 )
    {
        // This layer has to be physically removed
        // ToDo: Remove
        cgAppLog::write( cgAppLog::Debug, _T("Disposing of layer %i.\n"), nRemoveItem );

        // Dispose of unnecessary data.
        mLayers[nRemoveItem].blendMap.clear();
        mLayers[nRemoveItem].blendMapPaint.clear();
        
        // Shuffle data back.
        for ( cgInt32 j = nRemoveItem + 1; j < (cgInt32)mLayers.size(); ++j )
            mLayers[j - 1] = mLayers[j];
        
        // Destroy or resize container?
        if ( mLayers.size() == 1 )
            mLayers.clear();
        else
            mLayers.pop_back();

        // Re-optimize layer data
        optimizeLayers();

    } // End if something removed
}

//-----------------------------------------------------------------------------
// Name : LayerTypeUpdated()
/// <summary>
/// Called by the parent landscape to let us know when a layer type has
/// been updated or replaced. This allows us to update our data sets as 
/// necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgLandscapeTextureData::LayerTypeUpdated( cgLandscapeLayerType * pType, cgLandscapeLayerType * pNewType, cgInt32 nTypeId )
{
    // Validate requirements
    if ( mLayers.empty() == true )
        return;

    // The specified layer has been updated. If we reference this
    // layer at all, reoptimize our batches in case a normal map has
    // been removed / applied.
    for ( size_t i = 0; i < mLayers.size(); ++i )
    {
        LayerReference & Layer = mLayers[i];
        if ( Layer.nLayerType == nTypeId )
        {
            // Re-optimize layer data
            optimizeLayers();
            return;
        
        } // End if match

    } // Next Layer
}*/

//-----------------------------------------------------------------------------
// Name : isPainting ( )
/// <summary>
/// Determine if painting is currently enabled for this block.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::isPainting( ) const
{
    return mIsPainting;
}

//-----------------------------------------------------------------------------
// Name : isEmpty( )
/// <summary>
/// Determine if there is any renderable data stored here.
/// </summary>
//-----------------------------------------------------------------------------
bool cgLandscapeTextureData::isEmpty( ) const
{
    return mRenderBatches.empty();
}

//-----------------------------------------------------------------------------
// Name : getPaintData ( )
/// <summary>
/// Retrieve the blend map buffer into which we are currently painting.
/// </summary>
//-----------------------------------------------------------------------------
const cgByteArray & cgLandscapeTextureData::getPaintData( ) const
{
    static const cgByteArray Empty;
    if ( mIsPainting == false )
        return Empty;

    // Get the paint layer.
    return mLayers[ mPaintLayer ].blendMapPaint;
}

//-----------------------------------------------------------------------------
// Name : getBlendMapWorldArea ( )
/// <summary>
/// Retrieve the world space XZ rectangle for our blend map (including border 
/// texels).
/// </summary>
//-----------------------------------------------------------------------------
cgRectF cgLandscapeTextureData::getBlendMapWorldArea( ) const
{
    cgRectF rcArea;

    // Compute the XZ area of the terrain covered by the blend maps including their border
    const cgVector3 & vScale = mParentLandscape->getTerrainScale();
    const cgBoundingBox & BlockBounds = mParentBlock->getBoundingBox( );
    rcArea.left   = BlockBounds.min.x - (vScale.x / (cgFloat)mBlendMapResolution.width);
    rcArea.top    = BlockBounds.max.z + (vScale.z / (cgFloat)mBlendMapResolution.height);
    rcArea.right  = rcArea.left + ((cgFloat)mBlendMapSize.width * (vScale.x / (cgFloat)mBlendMapResolution.width));
    rcArea.bottom = rcArea.top  + (-(cgFloat)mBlendMapSize.height * (vScale.z / (cgFloat)mBlendMapResolution.height));
    return rcArea;
}

//-----------------------------------------------------------------------------
// Name : getBlendMapArea ( )
/// <summary>
/// Retrieve the XZ rectangle for our blend map (including border texels) in
/// "global" space (that is the space that takes into account the location of
/// this blend map as it connects to all others).
/// </summary>
//-----------------------------------------------------------------------------
cgRect cgLandscapeTextureData::getBlendMapArea( ) const
{
    // Compute the XZ area of the terrain covered by the blend maps including their border
    cgRect rcArea;
    rcArea.left  = -mActiveBlendMapBounds.left;
    rcArea.top   = -mActiveBlendMapBounds.top;
    rcArea.left += (mParentBlock->getBlockIndex() % mParentLandscape->getBlockLayout().width) * mActiveBlendMapBounds.width();
    rcArea.top  += (mParentBlock->getBlockIndex() / mParentLandscape->getBlockLayout().width) * mActiveBlendMapBounds.height();
    rcArea.right = rcArea.left + mBlendMapSize.width;
    rcArea.bottom = rcArea.top + mBlendMapSize.height;
    return rcArea;
}