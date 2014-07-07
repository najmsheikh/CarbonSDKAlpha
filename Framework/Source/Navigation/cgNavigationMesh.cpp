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
// Name : cgNavigationMesh.cpp                                               //
//                                                                           //
// Desc : The navigation mesh is used to construct and manage navigation     //
//        data for scene geometry that can be used by AI agents or other     //
//        systems in order to navigate and find paths through a scene.       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationMesh Module Includes
//-----------------------------------------------------------------------------
#include <Navigation/cgNavigationMesh.h>
#include <Navigation/cgNavigationTile.h>
#include <World/cgLandscape.h>
#include <Resources/cgMesh.h>
#include <Math/cgMathUtility.h>
#include <System/cgStringUtility.h>
#include "../../Lib/Detour/Include/DetourNavMesh.h"
#include "../../Lib/Recast/Include/Recast.h"

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgNavigationMesh::mInsertMesh;
cgWorldQuery cgNavigationMesh::mUpdateParameters;
cgWorldQuery cgNavigationMesh::mDeleteTiles;
cgWorldQuery cgNavigationMesh::mLoadMesh;
cgWorldQuery cgNavigationMesh::mLoadTiles;

///////////////////////////////////////////////////////////////////////////////
// cgNavigationMesh Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationMesh () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMesh::cgNavigationMesh( ) : cgWorldComponent( cgReferenceManager::generateInternalRefId(), CG_NULL )
{
    // Initialize variables to sensible defaults
    mMesh   = CG_NULL;
    mTilesX = 0;
    mTilesY = 0;
    mTilesZ = 0;
    mGeomBounds.reset();
}

//-----------------------------------------------------------------------------
//  Name : cgNavigationMesh () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMesh::cgNavigationMesh( cgUInt32 referenceId, cgWorld * world ) : cgWorldComponent( referenceId, world )
{
    // Initialize variables to sensible defaults
    mMesh   = CG_NULL;
    mTilesX = 0;
    mTilesY = 0;
    mTilesZ = 0;
    mGeomBounds.reset();
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationMesh () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMesh::~cgNavigationMesh( )
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
void cgNavigationMesh::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release allocated navigation tiles.
    for ( size_t i = 0; i < mTiles.size(); ++i )
        mTiles[i]->scriptSafeDispose();
    mTiles.clear();

    // Release any allocated resources.
    dtFreeNavMesh(mMesh);

    // Clear variables
    mMesh   = CG_NULL;
    mTilesX = 0;
    mTilesY = 0;
    mTilesZ = 0;
    mGeomBounds.reset();

    // Dispose base(s).
    if ( disposeBase )
        cgWorldComponent::dispose( true );
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
bool cgNavigationMesh::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_NavigationMesh )
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
cgString cgNavigationMesh::getDatabaseTable( ) const
{
    return _T("DataSources::NavigationMesh");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMesh::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new navigation mesh data.
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
bool cgNavigationMesh::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Create type tables as necessary.
        if ( !cgWorldComponent::createTypeTables( getReferenceType() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to create world database type tables for defined object type '%s'.\n"), cgStringUtility::toString( getReferenceType(), _T("B") ).c_str() );
            return false;
        
        } // End if failed

        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("NavigationMesh::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertMesh.bindParameter( 1, mReferenceId );
        mInsertMesh.bindParameter( 2, (cgUInt32)0 );  // Flags
        mInsertMesh.bindParameter( 3, mTilesX );
        mInsertMesh.bindParameter( 4, 1 );            // TilesY
        mInsertMesh.bindParameter( 5, mTilesZ );
        mInsertMesh.bindParameter( 6, mGeomBounds.min.x );
        mInsertMesh.bindParameter( 7, mGeomBounds.min.y );
        mInsertMesh.bindParameter( 8, mGeomBounds.min.z );
        mInsertMesh.bindParameter( 9, mGeomBounds.max.x );
        mInsertMesh.bindParameter( 10, mGeomBounds.max.y );
        mInsertMesh.bindParameter( 11, mGeomBounds.max.z );
        mInsertMesh.bindParameter( 12, mParams.cellSize );
        mInsertMesh.bindParameter( 13, mParams.cellHeight );
        mInsertMesh.bindParameter( 14, mParams.tileCells );
        mInsertMesh.bindParameter( 15, mParams.agentRadius );
        mInsertMesh.bindParameter( 16, mParams.agentHeight );
        mInsertMesh.bindParameter( 17, mParams.agentMaximumSlope );
        mInsertMesh.bindParameter( 18, mParams.agentMaximumStepHeight );
        mInsertMesh.bindParameter( 19, mParams.edgeMaximumLength );
        mInsertMesh.bindParameter( 20, mParams.edgeMaximumError );
        mInsertMesh.bindParameter( 21, mParams.regionMinimumSize );
        mInsertMesh.bindParameter( 22, mParams.regionMergedSize );
        mInsertMesh.bindParameter( 23, mParams.verticesPerPoly );
        mInsertMesh.bindParameter( 24, mParams.detailSampleDistance );
        mInsertMesh.bindParameter( 25, mParams.detailSampleMaximumError );
        mInsertMesh.bindParameter( 26, mSoftRefCount );
        
        // Execute
        if ( !mInsertMesh.step( true ) )
        {
            cgString error;
            mInsertMesh.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for navigation mesh data source '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("NavigationMesh::insertComponentData") );
            return false;
        
        } // End if failed

        // Serialize tiles.
        for ( size_t i = 0; i < mTiles.size(); ++i )
        {
            if ( !mTiles[i]->serialize( mReferenceId, mWorld ) )
            {
                mWorld->rollbackTransaction( _T("NavigationMesh::insertComponentData") );
                return false;
            
            } // End if failed

        } // Next tile.

        // Commit changes
        mWorld->commitTransaction( _T("NavigationMesh::insertComponentData") );

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
bool cgNavigationMesh::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the mesh data.
    prepareQueries();
    mLoadMesh.bindParameter( 1, e->sourceRefId );
    if ( !mLoadMesh.step( ) || !mLoadMesh.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadMesh.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh data source '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for navigation mesh data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadMesh.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadMesh;

    // Grab parameters.
    cgUInt32 flags;
    mLoadMesh.getColumn( _T("Flags"), flags );
    mLoadMesh.getColumn( _T("TilesX"), mTilesX );
    mLoadMesh.getColumn( _T("TilesY"), mTilesY );
    mLoadMesh.getColumn( _T("TilesZ"), mTilesZ );
    mLoadMesh.getColumn( _T("BoundsMinX"), mGeomBounds.min.x );
    mLoadMesh.getColumn( _T("BoundsMinY"), mGeomBounds.min.y );
    mLoadMesh.getColumn( _T("BoundsMinZ"), mGeomBounds.min.z );
    mLoadMesh.getColumn( _T("BoundsMaxX"), mGeomBounds.max.x );
    mLoadMesh.getColumn( _T("BoundsMaxY"), mGeomBounds.max.y );
    mLoadMesh.getColumn( _T("BoundsMaxZ"), mGeomBounds.max.z );
    mLoadMesh.getColumn( _T("CellSize"), mParams.cellSize );
    mLoadMesh.getColumn( _T("CellHeight"), mParams.cellHeight );
    mLoadMesh.getColumn( _T("TileCells"), mParams.tileCells );
    mLoadMesh.getColumn( _T("AgentRadius"), mParams.agentRadius );
    mLoadMesh.getColumn( _T("AgentHeight"), mParams.agentHeight );
    mLoadMesh.getColumn( _T("AgentMaximumSlope"), mParams.agentMaximumSlope );
    mLoadMesh.getColumn( _T("AgentMaximumStepHeight"), mParams.agentMaximumStepHeight );
    mLoadMesh.getColumn( _T("EdgeMaximumLength"), mParams.edgeMaximumLength );
    mLoadMesh.getColumn( _T("EdgeMaximumError"), mParams.edgeMaximumError );
    mLoadMesh.getColumn( _T("RegionMinimumSize"), mParams.regionMinimumSize );
    mLoadMesh.getColumn( _T("RegionMergedSize"), mParams.regionMergedSize );
    mLoadMesh.getColumn( _T("VerticesPerPoly"), mParams.verticesPerPoly );
    mLoadMesh.getColumn( _T("DetailSampleDistance"), mParams.detailSampleDistance );
    mLoadMesh.getColumn( _T("DetailSampleMaximumError"), mParams.detailSampleMaximumError );

    // Attempt to load tiles.
    mLoadTiles.bindParameter( 1, e->sourceRefId );
    if ( !mLoadTiles.step( ) )
    {
        // Log any error.
        cgString error;
        if ( !mLoadTiles.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve child tile data for navigation mesh data source '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve child tile data for navigation mesh data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadTiles.reset();
        mLoadMesh.reset();
        return false;
    
    } // End if failed

    // Process each tile.
    while ( mLoadTiles.nextRow() )
    {
        // Get the tile location.
        cgInt32 tileX, tileY, tileZ;
        mLoadTiles.getColumn( _T("TileX"), tileX );
        mLoadTiles.getColumn( _T("TileY"), tileY );
        mLoadTiles.getColumn( _T("TileZ"), tileZ );

        // Allocate a new tile and deserialize.
        cgNavigationTile * tile = new cgNavigationTile( tileX, tileY, tileZ, this );
        if ( !tile->deserialize( mLoadTiles, (mReferenceId != e->sourceRefId) ) )
        {
            // Clean up.
            tile->scriptSafeDispose();
            mLoadTiles.reset();
            mLoadMesh.reset();
            return false;
        
        } // End if failed

        // Add the tile to our set.
        mTiles.push_back( tile );

    } // Next tile

    // Close pending reads.
    mLoadTiles.reset();

    // Call base class implementation to read remaining data.
    if ( !cgWorldComponent::onComponentLoading( e ) )
        return false;

    // Allocate a new navigation mesh.
    mMesh = dtAllocNavMesh();
    if ( !mMesh )
	{
        cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation mesh object.\n") );
        return false;
	
    } // End if allocation failed

    // Compute tile grid details
    cgInt gridWidth = 0, gridHeight = 0;
	rcCalcGridSize( mGeomBounds.min, mGeomBounds.max, mParams.cellSize, &gridWidth, &gridHeight );
	const cgInt tilesX = (gridWidth + mParams.tileCells-1) / mParams.tileCells;
	const cgInt tilesZ = (gridHeight + mParams.tileCells-1) / mParams.tileCells;
	const cgFloat tileSize = (cgFloat)mParams.tileCells * mParams.cellSize; // ToDo: We need this to ultimately come out to 58.5216f

    // Max tiles and max polys affect how the tile IDs are caculated.
	// There are 22 bits available for identifying a tile and a polygon.
    const cgInt tileBits = rcMin((cgInt)cgMathUtility::log2(cgMathUtility::nextPowerOfTwo(tilesX*tilesZ)), 14);
	const cgInt polyBits = 22 - tileBits;
	const cgInt maxTiles = 1 << tileBits;
	const cgInt maxPolysPerTile = 1 << polyBits;
	
    // Initialize the navigation mesh ready for building.
    dtNavMeshParams dtparams;
    memcpy( dtparams.orig, &mGeomBounds.min, sizeof(cgVector3) ); // ToDo: Would prefer if this was centered.
	dtparams.tileWidth  = tileSize;
	dtparams.tileHeight = tileSize;
	dtparams.maxTiles   = maxTiles;
	dtparams.maxPolys   = maxPolysPerTile;
	dtStatus status     = mMesh->init(&dtparams);
	if ( dtStatusFailed(status) )
	{
        cgAppLog::write( cgAppLog::Error, _T("Unable to initialize navigation mesh object prior to construction.\n") );
        return false;
	
    } // End if failed

    // Add all loaded tiles.
    for ( size_t i = 0; i < mTiles.size(); ++i )
    {
        // Add tile data.
        cgNavigationTile * tile = mTiles[i];
        const cgByteArray & navData = tile->getNavigationData();
        if ( !navData.empty() )
        {
            dtTileRef tileRef;
            dtStatus status = mMesh->addTile( (cgByte*)&navData.front(), (cgInt)navData.size(), 0, 0, &tileRef );
		    if ( dtStatusFailed(status) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to add navigation tile to the mesh at <%d,%d,%d>.\n"), tile->mTileX, tile->mTileY, tile->mTileZ );
                continue;
    		
		    } // End if failed

        } // End if has data.

    } // Next tile

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
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMesh::onComponentDeleted( )
{
    // Call base class implementation last.
    cgWorldComponent::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMesh::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertMesh.isPrepared( mWorld ) )
            mInsertMesh.prepare( mWorld, _T("INSERT INTO 'DataSources::NavigationMesh' VALUES(?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11,?12,?13,?14,?15,?16,?17,?18,?19,?20,?21,?22,?23,?24,?25,?26)"), true );
        if ( !mUpdateParameters.isPrepared( mWorld ) )
            mUpdateParameters.prepare( mWorld, _T("UPDATE 'DataSources::NavigationMesh' SET Flags=?1, TilesX=?2, TilesY=?3, TilesZ=?4, BoundsMinX=?5, BoundsMinY=?6, BoundsMinZ=?7, BoundsMaxX=?8, BoundsMaxY=?9, BoundsMaxZ=?10, CellSize=?11, CellHeight=?12, TileCells=?13, AgentRadius=?14, AgentHeight=?15, AgentMaximumSlope=?16, AgentMaximumStepHeight=?17, EdgeMaximumLength=?18, EdgeMaximumError=?19, RegionMinimumSize=?20, RegionMergedSize=?21, VerticesPerPoly=?22, DetailSampleDistance=?23, DetailSampleMaximumError=?24  WHERE RefId=?25"), true );
        if ( !mDeleteTiles.isPrepared( mWorld ) )
            mDeleteTiles.prepare( mWorld, _T("DELETE FROM 'DataSources::NavigationMesh::Tiles' WHERE DataSourceId=?1"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadMesh.isPrepared( mWorld ) )
        mLoadMesh.prepare( mWorld, _T("SELECT * FROM 'DataSources::NavigationMesh' WHERE RefId=?1"), true );
    if ( !mLoadTiles.isPrepared( mWorld ) )
        mLoadTiles.prepare( mWorld, _T("SELECT * FROM 'DataSources::NavigationMesh::Tiles' WHERE DataSourceId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : build ()
/// <summary>
/// Construct the navigation mesh based on the configuration parameters 
/// and mesh data supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMesh::build( cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[], cgLandscape * landscape )
{
    return build( mParams, meshCount, meshData, meshTransforms, landscape );
}

//-----------------------------------------------------------------------------
//  Name : build ()
/// <summary>
/// Construct the navigation mesh based on the configuration parameters 
/// and mesh data supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMesh::build( const cgNavigationMeshCreateParams & params, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[], cgLandscape * landscape )
{
    cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Constructing walkable navigation data based on geometry collected from %i available meshes.\n"), meshCount );

    // ToDo: Note - Automatically compute a good cell size and height 
    // based on agent radius for the time being.
    cgFloat cellSize   = params.agentRadius * 0.5f;
    cgFloat cellHeight = params.cellSize * 0.66666666f;

    // Compute the overall bounding box of all meshes to be considered.
    cgBoundingBox geomBounds;
    cgArray<cgBoundingBox> meshBounds( meshCount );
    for ( cgUInt32 i = 0; i < meshCount; ++i )
    {
        if ( !meshData[i].isValid() )
            continue;

        // Transform the bounding box into world space.
        meshBounds[i] = meshData[i]->getBoundingBox();
        meshBounds[i].transform( meshTransforms[i] );

        // Grow overal bounding box.
        geomBounds.addPoint( meshBounds[i].min );
        geomBounds.addPoint( meshBounds[i].max );
    
    } // Next mesh

    // Add landscape if provided
    if ( landscape )
    {
        cgBoundingBox landscapeBounds = landscape->getBoundingBox();
        geomBounds.addPoint( landscapeBounds.min );
        geomBounds.addPoint( landscapeBounds.max );
    
    } // End if landscape provided

    // Compute tile grid details
    cgInt gridWidth = 0, gridHeight = 0;
	rcCalcGridSize( geomBounds.min, geomBounds.max, cellSize, &gridWidth, &gridHeight );
	const cgInt tilesX = (gridWidth + mParams.tileCells-1) / mParams.tileCells;
	const cgInt tilesZ = (gridHeight + mParams.tileCells-1) / mParams.tileCells;
	const cgFloat tileSize = (cgFloat)mParams.tileCells * mParams.cellSize; // ToDo: We need this to ultimately come out to 58.5216f

    // Max tiles and max polys affect how the tile IDs are caculated.
	// There are 22 bits available for identifying a tile and a polygon.
    const cgInt tileBits = rcMin((cgInt)cgMathUtility::log2(cgMathUtility::nextPowerOfTwo(tilesX*tilesZ)), 14);
	const cgInt polyBits = 22 - tileBits;
	const cgInt maxTiles = 1 << tileBits;
	const cgInt maxPolysPerTile = 1 << polyBits;

    // If we need to serialize parameter data, do so now.
    bool transactionBegun = false;
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("NavigationMesh::build") );
        transactionBegun = true;

        // Run the update.
        prepareQueries();
        mUpdateParameters.bindParameter( 1, (cgUInt32)0 );  // Flags
        mUpdateParameters.bindParameter( 2, tilesX );
        mUpdateParameters.bindParameter( 3, 1 );            // TilesY
        mUpdateParameters.bindParameter( 4, tilesZ );
        mUpdateParameters.bindParameter( 5, geomBounds.min.x );
        mUpdateParameters.bindParameter( 6, geomBounds.min.y );
        mUpdateParameters.bindParameter( 7, geomBounds.min.z );
        mUpdateParameters.bindParameter( 8, geomBounds.max.x );
        mUpdateParameters.bindParameter( 9, geomBounds.max.y );
        mUpdateParameters.bindParameter( 10, geomBounds.max.z );
        mUpdateParameters.bindParameter( 11, cellSize );
        mUpdateParameters.bindParameter( 12, cellHeight );
        mUpdateParameters.bindParameter( 13, params.tileCells );
        mUpdateParameters.bindParameter( 14, params.agentRadius );
        mUpdateParameters.bindParameter( 15, params.agentHeight );
        mUpdateParameters.bindParameter( 16, params.agentMaximumSlope );
        mUpdateParameters.bindParameter( 17, params.agentMaximumStepHeight );
        mUpdateParameters.bindParameter( 18, params.edgeMaximumLength );
        mUpdateParameters.bindParameter( 19, params.edgeMaximumError );
        mUpdateParameters.bindParameter( 20, params.regionMinimumSize );
        mUpdateParameters.bindParameter( 21, params.regionMergedSize );
        mUpdateParameters.bindParameter( 22, params.verticesPerPoly );
        mUpdateParameters.bindParameter( 23, params.detailSampleDistance );
        mUpdateParameters.bindParameter( 24, params.detailSampleMaximumError );
        mUpdateParameters.bindParameter( 25, mReferenceId );
        
        // Execute
        if ( !mUpdateParameters.step( true ) )
        {
            cgString error;
            mUpdateParameters.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update parameter data for navigation mesh data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

        // Attempt to delete prior tiles.
        mDeleteTiles.bindParameter( 1, mReferenceId );
        if ( !mDeleteTiles.step( true ) )
        {
            cgString error;
            mDeleteTiles.getLastError( error );
            mWorld->rollbackTransaction( _T("NavigationMesh::build") );
            cgAppLog::write( cgAppLog::Error, _T("Failed to remove prior tile data for navigation mesh data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return false;
        
        } // End if failed

    } // End if serialize

    // Release any prior data.
    for ( size_t i = 0; i < mTiles.size(); ++i )
        mTiles[i]->scriptSafeDispose();
    mTiles.clear();
    dtFreeNavMesh(mMesh);

    // Store necessary details for later use.
    mParams = params;
    mParams.cellSize = cellSize;
    mParams.cellHeight = cellHeight;
    mGeomBounds = geomBounds;
    mTilesX = tilesX;
    mTilesY = 1;
    mTilesZ = tilesZ;

    // Allocate a new navigation mesh.
    mMesh = dtAllocNavMesh();
    if ( !mMesh )
	{
        cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation mesh object.\n") );
        return false;
	
    } // End if allocation failed
	
    // Initialize the navigation mesh ready for building.
    dtNavMeshParams dtparams;
    memcpy( dtparams.orig, &geomBounds.min, sizeof(cgVector3) ); // ToDo: Would prefer if this was centered.
	dtparams.tileWidth  = tileSize;
	dtparams.tileHeight = tileSize;
	dtparams.maxTiles   = maxTiles;
	dtparams.maxPolys   = maxPolysPerTile;
	dtStatus status     = mMesh->init(&dtparams);
	if ( dtStatusFailed(status) )
	{
        cgAppLog::write( cgAppLog::Error, _T("Unable to initialize navigation mesh object prior to construction.\n") );
        return false;
	
    } // End if failed

    // Generate all potential cells.
    cgBoundingBox tileBounds;
    for ( cgInt z = 0; z < tilesZ; ++z)
	{
		for ( cgInt x = 0; x < tilesX; ++x)
		{
            // Remove previous tile data at this location
            mMesh->removeTile( mMesh->getTileRefAt(x,z,0), 0, 0 );

            // Compute the bounding box of this tile.
            tileBounds.min.x = geomBounds.min.x + x * tileSize;
            tileBounds.min.y = geomBounds.min.y;
            tileBounds.min.z = geomBounds.min.z + z * tileSize;

            tileBounds.max.x = geomBounds.min.x + (x+1) * tileSize;
            tileBounds.max.y = geomBounds.max.y;
            tileBounds.max.z = geomBounds.min.z + (z+1) * tileSize;

            // Get all meshes that fall within the bounding box of this tile, but
            // make sure it is inflated by at least 'agentRadius + (3*cellsize)'
            // to ensure that the mesh builder knows that the geometry continues
            // off the edge of the tile.
            cgBoundingBox expandedTileBounds = tileBounds;
            expandedTileBounds.inflate( (ceilf(mParams.agentRadius / mParams.cellSize) + 3) * mParams.cellSize );
            cgArray<cgMeshHandle> intersectedMeshes;
            cgArray<cgTransform> intersectedTransforms;
            for ( cgUInt32 i = 0; i < meshCount; ++i )
            {
                if ( expandedTileBounds.intersect( meshBounds[i] ) )
                {
                    // Reserve space for all meshes the first time we find one.
                    if ( intersectedMeshes.capacity() == 0 )
                    {
                        intersectedMeshes.reserve( meshCount );
                        intersectedTransforms.reserve( meshCount );
                    
                    } // End if first discovered mesh?

                    // Add this mesh to the intersected list.
                    intersectedMeshes.push_back( meshData[i] );
                    intersectedTransforms.push_back( meshTransforms[i] );
                
                } // End if intersects
            
            } // Next mesh

            // Get landscape terrain blocks if any.
            cgLandscape::TerrainBlockArray intersectedTerrainBlocks;
            if ( landscape )
                landscape->getTerrainBlocks( expandedTileBounds, intersectedTerrainBlocks );

            // Anything discovered?
            if ( intersectedMeshes.empty() && intersectedTerrainBlocks.empty() )
                continue;

            // Allocate a new navigation tile.
            cgNavigationTile * tile = new cgNavigationTile( x, 0, z, this );
            if ( !tile->buildTile( mParams, tileBounds, (cgUInt32)intersectedMeshes.size(), (intersectedMeshes.empty()) ? CG_NULL : &intersectedMeshes.front(), 
                                  (intersectedTransforms.empty()) ? CG_NULL : &intersectedTransforms.front(),
                                  (cgUInt32)intersectedTerrainBlocks.size(), (intersectedTerrainBlocks.empty()) ? CG_NULL : &intersectedTerrainBlocks.front() ) )
            {
                // ToDo: print warning?
                tile->scriptSafeDispose();
                continue;

            } // End if failed

            // Get the generated tile data.
            const cgByteArray & navData = tile->getNavigationData();
            if ( navData.empty() )
            {
                // Nothing was generated.
                tile->scriptSafeDispose();
                continue;
            
            } // End if no data

            // Add new tile data.
            dtTileRef tileRef;
            dtStatus status = mMesh->addTile( (cgByte*)&navData.front(), (cgInt)navData.size(), 0, 0, &tileRef );
			if ( dtStatusFailed(status) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to add navigation tile to the mesh at <%d,%d,%d>.\n"), x, 0, z );
                tile->scriptSafeDispose();
                continue;
			
			} // End if failed

            // Attempt to serialize.
            if ( shouldSerialize() )
            {
                if ( !tile->serialize( mReferenceId, mWorld ) )
                {
                    tile->scriptSafeDispose();
                    continue;
                
                } // End if failed
            
            } // End if failed

            // Store reference to the tile.
            mTiles.push_back( tile );
		
        } // Next column
	
    } // Next row

    // Save changes.
    if ( transactionBegun )
        mWorld->commitTransaction( _T("NavigationMesh::build") );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : debugDraw ()
/// <summary>
/// Render a representation of the navigation mesh for debug purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMesh::debugDraw( cgRenderDriver * driver )
{
    // Process tiles.
    for ( size_t i = 0; i < mTiles.size(); ++i )
        mTiles[i]->debugDraw( driver );
}

//-----------------------------------------------------------------------------
//  Name : getInternalMesh()
/// <summary>
/// Retrieve the internal mesh object specific to Detour. This is not
/// technically part of the public interface and should not be called
/// by the application directly.
/// </summary>
//-----------------------------------------------------------------------------
dtNavMesh * cgNavigationMesh::getInternalMesh( )
{
    return mMesh;
}

//-----------------------------------------------------------------------------
//  Name : setParameters()
/// <summary>
/// Set the parameters that describe how this navigation mesh should be 
/// constructed. NB: A full build *should* be performed at some point in the
/// future based on these new parameters to ensure that the tile data is in
/// synch.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationMesh::setParameters( const cgNavigationMeshCreateParams & params )
{
    // Is this a no-op?
    if ( memcmp( &params, &mParams, sizeof(cgNavigationMeshCreateParams) ) == 0 )
        return;

    // Update database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateParameters.bindParameter( 1, (cgUInt32)0 );  // Flags
        mUpdateParameters.bindParameter( 2, mTilesX );
        mUpdateParameters.bindParameter( 3, mTilesY );
        mUpdateParameters.bindParameter( 4, mTilesZ );
        mUpdateParameters.bindParameter( 5, mGeomBounds.min.x );
        mUpdateParameters.bindParameter( 6, mGeomBounds.min.y );
        mUpdateParameters.bindParameter( 7, mGeomBounds.min.z );
        mUpdateParameters.bindParameter( 8, mGeomBounds.max.x );
        mUpdateParameters.bindParameter( 9, mGeomBounds.max.y );
        mUpdateParameters.bindParameter( 10, mGeomBounds.max.z );
        mUpdateParameters.bindParameter( 11, params.cellSize );
        mUpdateParameters.bindParameter( 12, params.cellSize );
        mUpdateParameters.bindParameter( 13, params.tileCells );
        mUpdateParameters.bindParameter( 14, params.agentRadius );
        mUpdateParameters.bindParameter( 15, params.agentHeight );
        mUpdateParameters.bindParameter( 16, params.agentMaximumSlope );
        mUpdateParameters.bindParameter( 17, params.agentMaximumStepHeight );
        mUpdateParameters.bindParameter( 18, params.edgeMaximumLength );
        mUpdateParameters.bindParameter( 19, params.edgeMaximumError );
        mUpdateParameters.bindParameter( 20, params.regionMinimumSize );
        mUpdateParameters.bindParameter( 21, params.regionMergedSize );
        mUpdateParameters.bindParameter( 22, params.verticesPerPoly );
        mUpdateParameters.bindParameter( 23, params.detailSampleDistance );
        mUpdateParameters.bindParameter( 24, params.detailSampleMaximumError );
        mUpdateParameters.bindParameter( 25, mReferenceId );
        
        // Execute
        if ( !mUpdateParameters.step( true ) )
        {
            cgString error;
            mUpdateParameters.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update parameter data for navigation mesh data source '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed

    } // End if serialize.

    // Update local members.
    mParams = params;

    // Notify listeners that an update occurred.
    static const cgString strContext = _T("CreationParameters");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
//  Name : getParameters()
/// <summary>
/// Retrieve the parameters used during the construction of the navigation
/// mesh.
/// </summary>
//-----------------------------------------------------------------------------
const cgNavigationMeshCreateParams & cgNavigationMesh::getParameters() const
{
    return mParams;
}

//-----------------------------------------------------------------------------
//  Name : getTileCount()
/// <summary>
/// Retrieve the total number of tiles that have been allocated for this
/// navigation mesh.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgNavigationMesh::getTileCount( ) const
{
    return (cgUInt32)mTiles.size();
}