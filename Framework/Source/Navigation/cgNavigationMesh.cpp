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
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
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
#include <Resources/cgMesh.h>
#include <Math/cgMathUtility.h>
#include "../../Lib/Detour/Include/DetourNavMesh.h"
#include "../../Lib/Recast/Include/Recast.h"

///////////////////////////////////////////////////////////////////////////////
// cgNavigationMesh Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationMesh () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMesh::cgNavigationMesh( )
{
    // Initialize variables to sensible defaults
    mMesh = CG_NULL;
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
    // Release allocated navigation tiles.
    for ( size_t i = 0; i < mTiles.size(); ++i )
        mTiles[i]->scriptSafeDispose();
    mTiles.clear();

    // Release any allocated resources.
    dtFreeNavMesh(mMesh);

    // Clear variables
    mMesh = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : build ()
/// <summary>
/// Construct the navigation mesh based on the configuration parameters 
/// and mesh data supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationMesh::build( const cgNavigationMeshCreateParams & params, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[] )
{
    cgAppLog::write( cgAppLog::Info | cgAppLog::Debug, _T("Constructing walkable navigation data based on geometry collected from %i available meshes.\n"), meshCount );

    // Release any prior data.
    for ( size_t i = 0; i < mTiles.size(); ++i )
        mTiles[i]->scriptSafeDispose();
    mTiles.clear();
    dtFreeNavMesh(mMesh);

    // Store necessary details for later use.
    mParams = params;

    // ToDo: Note - Automatically compute a good cell size and height 
    // based on agent radius for the time being.
    mParams.cellSize   = mParams.agentRadius * 0.5f;
    mParams.cellHeight = mParams.cellSize * 0.66666666f;

    // Allocate a new navigation mesh.
    mMesh = dtAllocNavMesh();
    if ( !mMesh )
	{
        cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation mesh object.\n") );
        return false;
	
    } // End if allocation failed

    // Compute the overall bounding box of all meshes to be considered.
    cgBoundingBox geomBounds;
    std::vector<cgBoundingBox> meshBounds( meshCount );
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

    // Compute tile grid details
    cgInt gridWidth = 0, gridHeight = 0;
	rcCalcGridSize( geomBounds.min, geomBounds.max, mParams.cellSize, &gridWidth, &gridHeight );
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
            std::vector<cgMeshHandle> intersectedMeshes;
            std::vector<cgTransform> intersectedTransforms;
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

            // Anything discovered?
            if ( intersectedMeshes.empty() )
                continue;

            // Allocate a new navigation tile.
            cgNavigationTile * tile = new cgNavigationTile( x, 0, z, this );
            if ( !tile->buildTile( mParams, tileBounds, (cgUInt32)intersectedMeshes.size(), &intersectedMeshes.front(), &intersectedTransforms.front() ) )
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

            // Remove any previous data, and add the new data.
            dtTileRef tileRef;
            mMesh->removeTile( mMesh->getTileRefAt(x,z,0), 0, 0 );
            dtStatus status = mMesh->addTile( (cgByte*)&navData.front(), (cgInt)navData.size(), 0, 0, &tileRef );
			if ( dtStatusFailed(status) )
            {
                cgAppLog::write( cgAppLog::Warning, _T("Failed to add navigation tile to the mesh at <%d,%d,%d>.\n"), x, 0, z );
                tile->scriptSafeDispose();
                continue;
			
			} // End if failed

            // Store reference to the tile.
            mTiles.push_back( tile );
		
        } // Next column
	
    } // Next row

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
//  Name : getParameters()
/// <summary>
/// Retrieve the parameters used during the construction of the navigation
/// mesh/
/// </summary>
//-----------------------------------------------------------------------------
const cgNavigationMeshCreateParams & cgNavigationMesh::getParameters() const
{
    return mParams;
}