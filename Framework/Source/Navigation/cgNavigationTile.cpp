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
// Name : cgNavigationTile.cpp                                               //
//                                                                           //
// Desc : Maintains navigation data for a pre-defined region of the larger   //
//        navigation mesh. Allows navigation data for individual sections of //
//        the scene to be paged in and out of memory as necessary.           //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgNavigationTile Module Includes
//-----------------------------------------------------------------------------
#include <Navigation/cgNavigationTile.h>
#include <Navigation/cgNavigationTypes.h>
#include <World/cgLandscape.h>
#include <Resources/cgMesh.h>
#include <Resources/cgResourceManager.h>
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <Rendering/cgRenderDriver.h>
#include <Rendering/cgVertexFormats.h>
#include "../../Lib/Detour/Include/DetourNavMesh.h"
#include "../../Lib/Detour/Include/DetourNavMeshBuilder.h"
#include "../../Lib/Recast/Include/Recast.h"
#include "../../Lib/Recast/Include/RecastAlloc.h"

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgNavigationTile::mInsertTile;

///////////////////////////////////////////////////////////////////////////////
// cgNavigationTile Members
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgNavigationTile () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationTile::cgNavigationTile( cgInt32 tileX, cgInt32 tileY, cgInt32 tileZ, cgNavigationMesh * navMesh )
{
    // Initialize variables to sensible defaults
    mNavMesh    = navMesh;
    mTileX      = tileX;
    mTileY      = tileY;
    mTileZ      = tileZ;
    mTileRef    = 0xFFFFFFFF;
    mPolyMesh   = CG_NULL;
    mDetailMesh = CG_NULL;
    mDatabaseId = 0;
}

//-----------------------------------------------------------------------------
//  Name : ~cgNavigationTile () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationTile::~cgNavigationTile( )
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
void cgNavigationTile::dispose( bool disposeBase )
{
    // Release allocated resources.
    mDebugMesh.close();
    rcFreePolyMesh( mPolyMesh );
    rcFreePolyMeshDetail( mDetailMesh );
    mNavData.clear();

    // Clear variables
    mPolyMesh   = CG_NULL;
    mDetailMesh = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : buildTile ()
/// <summary>
/// Construct the navigation data for this tile based on the supplied geometry
/// and construction parameters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationTile::buildTile( const cgNavigationMeshCreateParams & params, const cgBoundingBox & tileBounds, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[] )
{
    return buildTile( params, tileBounds, meshCount, meshData, meshTransforms, 0, CG_NULL );
}

//-----------------------------------------------------------------------------
//  Name : buildTile ()
/// <summary>
/// Construct the navigation data for this tile based on the supplied geometry
/// and construction parameters.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationTile::buildTile( const cgNavigationMeshCreateParams & params, const cgBoundingBox & tileBounds, cgUInt32 meshCount, cgMeshHandle meshData[], cgTransform meshTransforms[], cgUInt32 terrainBlockCount, cgTerrainBlock * blockData[] )
{
    // Deriving configuration values - http://digestingduck.blogspot.fi/2009/08/recast-settings-uncovered.html
    
    // Release the prior data.
    mDebugMesh.close();

    // Merge the geometry into a single vertex buffer. First
    // count the total number of vertices and triangles.
    cgInt totalVertices = 0, totalTriangles = 0;
    for ( cgUInt32 i = 0; i < meshCount; ++i )
    {
        cgMesh * mesh = meshData[i].getResource(true);
        totalVertices  += (cgInt)mesh->getVertexCount();
        totalTriangles += (cgInt)mesh->getFaceCount();

    } // Next mesh

    // Add terrain block geometry.
    for ( cgUInt32 i = 0; i < terrainBlockCount; ++i )
    {
        cgTerrainBlock * block = blockData[i];
        cgLandscape * landscape = block->getLandscape();
        const cgSize & size = landscape->getBlockSize();
        totalVertices += size.width * size.height;
        totalTriangles += ((size.width-1) * (size.height-1)) * 2;
    
    } // Next terrain block

    // Allocate space for vertices / triangles.
    cgArray<cgVector3> vertices( totalVertices );
    cgArray<cgInt> indices( totalTriangles * 3 );

    // Extract the mesh data.
    totalVertices = totalTriangles = 0;
    for ( cgUInt32 i = 0; i < meshCount; ++i )
    {
        cgMesh * mesh = meshData[i].getResource(true);
        
        // Copy vertex positions (transformed).
        cgInt    meshVertexCount  = mesh->getVertexCount();
        cgInt    meshVertexStride = mesh->getVertexFormat()->getStride();
        cgByte * meshVertices     = mesh->getSystemVB();
        for ( cgInt j = 0; j < meshVertexCount; ++j, meshVertices += meshVertexStride )
            meshTransforms[i].transformCoord( vertices[totalVertices+j], *(cgVector3*)meshVertices );

        // Copy indices (flip winding)
        cgInt meshTriangleCount = (cgInt)mesh->getFaceCount();
        cgUInt32 * meshIndices  = mesh->getSystemIB();
        for ( cgInt j = 0; j < meshTriangleCount; ++j )
        {
            indices[(totalTriangles+j)*3+0] = (cgInt)meshIndices[(j*3)+0] + totalVertices;
            indices[(totalTriangles+j)*3+1] = (cgInt)meshIndices[(j*3)+1] + totalVertices;
            indices[(totalTriangles+j)*3+2] = (cgInt)meshIndices[(j*3)+2] + totalVertices;
        
        } // Next triangle
        
        // Progress to next mesh.
        totalVertices  += meshVertexCount;
        totalTriangles += meshTriangleCount;

    } // Next mesh

    // Extract terrain block data.
    for ( cgUInt32 i = 0; i < terrainBlockCount; ++i )
    {
        // Retrieve the data we need to perform this computation.
        cgTerrainBlock* block        = blockData[i];
        cgLandscape   * landscape    = block->getLandscape();
        cgSize          size         = landscape->getBlockSize();
        cgVector3       scale        = landscape->getTerrainScale();
        cgVector3       offset       = landscape->getTerrainOffset();
        cgPoint         heightOffset = block->getHeightMapOffset();

        // Process each triangle
        for ( cgInt y = 0; y < size.height - 1; ++y )
        {
            for ( cgInt x = 0; x < size.width - 1; ++x )
            {
                indices[totalTriangles*3+0] = totalVertices + (x + y * size.height);
                indices[totalTriangles*3+1] = totalVertices + ((x+1) + y * size.height);
                indices[totalTriangles*3+2] = totalVertices + (x + (y+1) * size.height);
                indices[totalTriangles*3+3] = totalVertices + (x + (y+1) * size.height);
                indices[totalTriangles*3+4] = totalVertices + ((x+1) + y * size.height);
                indices[totalTriangles*3+5] = totalVertices + ((x+1) + (y+1) * size.height);
                totalTriangles += 2;
            
            } // Next column
        
        } // Next row

        // Process each vertex.
        for ( cgInt y = heightOffset.y, endY = heightOffset.y + size.height; y < endY; ++y )
        {
            for ( cgInt x = heightOffset.x, endX = heightOffset.x + size.width; x < endX; ++x )
            {
                cgInt16 height = block->getHeightMapHeight( x, y );
                vertices[totalVertices++] = cgVector3( x * scale.x + offset.x, (cgFloat)height * scale.y + offset.y, -y * scale.z + offset.z );
            
            } // Next column
        
        } // Next row

    } // Next block
    
	// Init build configuration
    rcConfig config;
	memset(&config, 0, sizeof(rcConfig));
	config.cs                       = params.cellSize;
	config.ch                       = params.cellHeight;
	config.walkableSlopeAngle       = params.agentMaximumSlope;
	config.walkableHeight           = (cgInt)ceilf(params.agentHeight / config.ch);
	config.walkableClimb            = (cgInt)floorf(params.agentMaximumStepHeight / config.ch);
	config.walkableRadius           = (cgInt)ceilf(params.agentRadius / config.cs);
	config.maxEdgeLen               = (cgInt)(params.edgeMaximumLength / config.cs);
	config.maxSimplificationError   = params.edgeMaximumError;
	config.minRegionArea            = (cgInt)rcSqr(params.regionMinimumSize);   // Note: area = size*size
	config.mergeRegionArea          = (cgInt)rcSqr(params.regionMergedSize);    // Note: area = size*size
	config.maxVertsPerPoly          = (cgInt)params.verticesPerPoly;
	config.tileSize                 = (cgInt)params.tileCells;
	config.borderSize               = config.walkableRadius + 3;        // Reserve enough padding.
	config.width                    = config.tileSize + config.borderSize * 2;
	config.height                   = config.tileSize + config.borderSize * 2;
	config.detailSampleDist         = params.detailSampleDistance < 0.9f ? 0 : config.cs * params.detailSampleDistance;
	config.detailSampleMaxError     = config.ch * params.detailSampleMaximumError;
	memcpy( config.bmin, &tileBounds.min, sizeof(cgVector3) );
    memcpy( config.bmax, &tileBounds.max, sizeof(cgVector3) );
	config.bmin[0] -= config.borderSize * config.cs;
	config.bmin[2] -= config.borderSize * config.cs;
	config.bmax[0] += config.borderSize * config.cs;
	config.bmax[2] += config.borderSize * config.cs;
	
	// Allocate voxel heightfield where we rasterize our input data to.
    rcContext context;
	rcHeightfield * solid = rcAllocHeightfield();
	if (!solid)
	{
        cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation tile heightfield.\n") );
		return false;
	
    } // End if failed
	if (!rcCreateHeightfield( &context, *solid, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch))
	{
        rcFreeHeightField( solid );
        cgAppLog::write( cgAppLog::Error, _T("Unable to initialize navigation tile heightfield prior to construction.\n") );
		return false;
	
    } // End if failed

    // ToDo: Note - The following steps (up to rasterization) can be done once 
    // per mesh / chunk rather than using a mesh merge step
	
	// Allocate an array that can hold triangle flags.
    cgByteArray triangleFlags( totalTriangles, 0 );

    // Find walkable triangles.
    rcMarkWalkableTriangles( &context, config.walkableSlopeAngle, (cgFloat*)&vertices.front(), totalVertices, &indices.front(), totalTriangles, &triangleFlags.front() );
		
    // Rasterize into solid voxel heightfield
    rcRasterizeTriangles( &context, (cgFloat*)&vertices.front(), totalVertices, &indices.front(), &triangleFlags.front(), totalTriangles, *solid, config.walkableClimb );
    
    // Clean up (we can keep this if we want).
    triangleFlags.clear();
    indices.clear();
    vertices.clear();
	
	// Once all geometry is rasterized, we do initial filtering pass to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles( &context, config.walkableClimb, *solid );
	rcFilterLedgeSpans( &context, config.walkableHeight, config.walkableClimb, *solid );
	rcFilterWalkableLowHeightSpans( &context, config.walkableHeight, *solid );
	
	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield * compactSolid = rcAllocCompactHeightfield();
	if (!compactSolid)
	{
        rcFreeHeightField( solid );
		cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate compacted navigation tile heightfield.\n") );
		return false;
	
    } // End if failed
	if (!rcBuildCompactHeightfield( &context, config.walkableHeight, config.walkableClimb, *solid, *compactSolid ))
	{
        rcFreeHeightField( solid );
        rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Unable to compact navigation tile heightfield.\n") );
		return false;
	
    } // End if failed
	
    // Release intermediate data.
    rcFreeHeightField( solid );
	
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea( &context, config.walkableRadius, *compactSolid ))
	{
        rcFreeCompactHeightfield( compactSolid );
        cgAppLog::write( cgAppLog::Error, _T("Unable to erode navigation tile heightfield.\n") );
		return false;
	
    } // End if failed

	// (Optional) Mark areas.
	/*const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);*/
	
	/*if (m_monotonePartitioning)
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegionsMonotone(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
			return 0;
		}
	} 
	else
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(m_ctx, *m_chf))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return 0;
		}
		
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
			return 0;
		}
	}*/

    // Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField( &context, *compactSolid ) )
	{
        rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Unable to build navigation tile distance field.\n") );
		return false;
	
    } // End if failed
	
	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions( &context, *compactSolid, config.borderSize, config.minRegionArea, config.mergeRegionArea))
	{
        rcFreeCompactHeightfield( compactSolid );
        cgAppLog::write( cgAppLog::Error, _T("Unable to build navigation tile regions.\n") );
		return false;
	
    } // End if failed
 	
	// Create contours.
	rcContourSet * contours = rcAllocContourSet();
	if (!contours)
	{
        rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation tile contour set.\n") );
		return false;
	
    } // End if failed
	if (!rcBuildContours( &context, *compactSolid, config.maxSimplificationError, config.maxEdgeLen, *contours ))
	{
		rcFreeContourSet( contours );
        rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Unable to create navigation tile contour set.\n") );
		return false;

	} // End if failed
	
    // No countours to build?
	if ( contours->nconts == 0 )
    {
        rcFreeContourSet( contours );
        rcFreeCompactHeightfield( compactSolid );
        return true;
    
    } // End if no contours
	
	// Build polygon navmesh from the contours.
	mPolyMesh = rcAllocPolyMesh();
	if (!mPolyMesh)
	{
        rcFreeContourSet( contours );
        rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation tile polygon mesh.\n") );
		return false;
	
    } // End if failed
	if (!rcBuildPolyMesh( &context, *contours, config.maxVertsPerPoly, *mPolyMesh ))
	{
        rcFreeContourSet( contours );
        rcFreeCompactHeightfield( compactSolid );
        cgAppLog::write( cgAppLog::Error, _T("Unable to triangulate navigation tile contour set.\n") );
		return false;
	
    } // End if failed

    // Release intermediate data.
    rcFreeContourSet( contours );
	
	// Build detail mesh.
	mDetailMesh = rcAllocPolyMeshDetail();
	if (!mDetailMesh)
	{
		rcFreeCompactHeightfield( compactSolid );
		cgAppLog::write( cgAppLog::Error, _T("Out of memory while attempting to allocate navigation tile detail mesh.\n") );
		return false;
	
    } // End if failed
	if (!rcBuildPolyMeshDetail( &context, *mPolyMesh, *compactSolid,
							   config.detailSampleDist, config.detailSampleMaxError,
							   *mDetailMesh))
	{
		rcFreeCompactHeightfield( compactSolid );
        cgAppLog::write( cgAppLog::Error, _T("Unable to create navigation tile detail mesh.\n") );
		return false;
	
    } // End if failed

    // Release intermediate data.
    rcFreeCompactHeightfield( compactSolid );

    // Validate final data.
    if ( mPolyMesh->nverts >= 0xFFFF )
	{
		// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
        cgAppLog::write( cgAppLog::Error, _T("Too many vertices were generated during navigation tile generation (%d generated, maximum).\n"), mPolyMesh->nverts, 0xFFFF );
		return false;
	
    } // End if too many vertices
	
	// Update poly flags from areas.
	for ( cgInt i = 0; i < mPolyMesh->npolys; ++i )
	{
        // Mark all walkable areas as 'ground' type.
		if (mPolyMesh->areas[i] == RC_WALKABLE_AREA)
            mPolyMesh->areas[i] = cgNavigationRegionType::Ground;
		
        // Mark walkable areas as requiring the walk ability.
		if (mPolyMesh->areas[i] == cgNavigationRegionType::Ground ||
			mPolyMesh->areas[i] == cgNavigationRegionType::Grass ||
			mPolyMesh->areas[i] == cgNavigationRegionType::Road )
		{
            mPolyMesh->flags[i] = cgNavigationPolyFlags::Walk;
		
        } // End if ground | grass | road
        
        // Mark water areas as requiring the swim ability
		else if (mPolyMesh->areas[i] == cgNavigationRegionType::Water)
		{
            mPolyMesh->flags[i] = cgNavigationPolyFlags::Swim;
		} // End if water

        // Mark door areas as requiring the walk and traverse door abilities.
		else if (mPolyMesh->areas[i] == cgNavigationRegionType::Door)
		{
            mPolyMesh->flags[i] = cgNavigationPolyFlags::Walk | cgNavigationPolyFlags::Door;
		} // End if door

    } // Next poly
	
    // Build creation params.
	dtNavMeshCreateParams dtparams;
	memset(&dtparams, 0, sizeof(dtNavMeshCreateParams));
	dtparams.verts            = mPolyMesh->verts;
	dtparams.vertCount        = mPolyMesh->nverts;
	dtparams.polys            = mPolyMesh->polys;
	dtparams.polyAreas        = mPolyMesh->areas;
	dtparams.polyFlags        = mPolyMesh->flags;
	dtparams.polyCount        = mPolyMesh->npolys;
	dtparams.nvp              = mPolyMesh->nvp;
	dtparams.detailMeshes     = mDetailMesh->meshes;
	dtparams.detailVerts      = mDetailMesh->verts;
	dtparams.detailVertsCount = mDetailMesh->nverts;
	dtparams.detailTris       = mDetailMesh->tris;
	dtparams.detailTriCount   = mDetailMesh->ntris;
	/*dtparams.offMeshConVerts  = m_geom->getOffMeshConnectionVerts();
	dtparams.offMeshConRad    = m_geom->getOffMeshConnectionRads();
	dtparams.offMeshConDir    = m_geom->getOffMeshConnectionDirs();
	dtparams.offMeshConAreas  = m_geom->getOffMeshConnectionAreas();
	dtparams.offMeshConFlags  = m_geom->getOffMeshConnectionFlags();
	dtparams.offMeshConUserID = m_geom->getOffMeshConnectionId();
	dtparams.offMeshConCount  = m_geom->getOffMeshConnectionCount();*/
	dtparams.walkableHeight   = params.agentHeight;
	dtparams.walkableRadius   = params.agentRadius;
	dtparams.walkableClimb    = params.agentMaximumStepHeight;
    dtparams.tileX            = mTileX;
	dtparams.tileY            = mTileZ;
	dtparams.tileLayer        = mTileY;
	dtparams.cs               = mPolyMesh->cs;
	dtparams.ch               = mPolyMesh->ch;
    dtparams.buildBvTree      = true;
    rcVcopy(dtparams.bmin, mPolyMesh->bmin);
	rcVcopy(dtparams.bmax, mPolyMesh->bmax);
	
    // Generate final nav mesh data.
    cgByte * navData = CG_NULL; cgInt navDataSize = 0;
	if (!dtCreateNavMeshData( &dtparams, &navData, &navDataSize))
	{
        cgAppLog::write( cgAppLog::Error, _T("Unable to generate final navigation tile data.\n") );
		return false;
	
    } // End if failed

    // Copy data into our own local array.
    mNavData.resize( navDataSize );
    if ( navDataSize > 0 )
        memcpy( &mNavData.front(), navData, navDataSize );
    dtFree( navData );
	
    // Return success
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getNavigationMesh ()
/// <summary>
/// Retrieve the navigation mesh that owns this tile.
/// </summary>
//-----------------------------------------------------------------------------
cgNavigationMesh * cgNavigationTile::getNavigationMesh( ) const
{
    return mNavMesh;
}

//-----------------------------------------------------------------------------
//  Name : getNavigationData ()
/// <summary>
/// Retrieve the final compiled version of the generated navigation data.
/// </summary>
//-----------------------------------------------------------------------------
const cgByteArray & cgNavigationTile::getNavigationData( ) const
{
    return mNavData;
}

//-----------------------------------------------------------------------------
//  Name : debugDraw ()
/// <summary>
/// Render a representation of the navigation tile for debug purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationTile::debugDraw( cgRenderDriver * driver )
{
    // Build new debug mesh if it's required.
    buildDebugMeshes( driver->getResourceManager() );

    // Retrieve the underlying rendering resources if available
    cgMesh * mesh = mDebugMesh.getResource(true);
    if ( !mesh || !mesh->isLoaded() )
        return;

    // Retrieve the debug rendering constants (only available in sandbox mode).
    cgConstantBuffer * constants = driver->getSandboxConstantBuffer().getResource( true );
    if ( !constants )
        return;

    // Setup constants.
    cgColorValue interiorColor = cgColorValue( 0.7f, 0.2f, 0.1f, 0.4f );
    interiorColor.a = 0.4f;
    cgColorValue wireColor = cgColorValue( 0xFF000000 );
    constants->setVector( _T("shapeInteriorColor"), (cgVector4&)interiorColor );
    constants->setVector( _T("shapeWireColor"), (cgVector4&)wireColor );
    driver->setConstantBufferAuto( driver->getSandboxConstantBuffer() );
    driver->setWorldTransform( CG_NULL );

    // Execute technique.
    cgSurfaceShader * shader = driver->getSandboxSurfaceShader().getResource(true);
    if ( shader->beginTechnique( _T("drawGhostedShapeMesh") ) )
    {
        while ( shader->executeTechniquePass( ) == cgTechniqueResult::Continue )
            mesh->draw( cgMeshDrawMode::Simple );
        shader->endTechnique();
    
    } // End if success
}

//-----------------------------------------------------------------------------
//  Name : buildDebugMeshes () (Protected)
/// <summary>
/// Construct the renderable representation of the navigation tile data for
/// debug purposes.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationTile::buildDebugMeshes( cgResourceManager * resources )
{
    // Skip if there is nothing to do.
    if ( mDebugMesh.isValid() && mDebugMesh->getManager() == resources )
        return;

    // Clear out previous mesh data.
    mDebugMesh.close();

    // Anything to do?
    if ( !mPolyMesh || !resources )
        return;

    // Extract vertex data.
    const cgFloat cellSize = mPolyMesh->cs;
    const cgFloat cellHeight = mPolyMesh->ch;
    const cgVector3 origin = mPolyMesh->bmin;
    cgArray<cgVector3> vertices( mPolyMesh->nverts );
    for ( cgInt i = 0; i < mPolyMesh->nverts; ++i )
    {
        const cgUInt16 * v = &mPolyMesh->verts[i*3];
        vertices[i].x = origin.x + v[0] * cellSize;
        vertices[i].y = origin.y + (v[1]+1) * cellHeight;
		vertices[i].z = origin.z + v[2] * cellSize;

    } // Next vertex

    // Extract index data.
    cgArray<cgUInt32> indices;
    const cgInt maxVertsPerPoly = mPolyMesh->nvp;
    indices.reserve( mPolyMesh->npolys * ((maxVertsPerPoly - 2) * 3) );
    for ( cgInt i = 0; i < mPolyMesh->npolys; ++i )
	{
        // Process the polygon (triangle fan)
		const cgUInt16 * p = &mPolyMesh->polys[i*maxVertsPerPoly*2];
		for ( cgInt j = 2; j < maxVertsPerPoly; ++j )
		{
            // No further verts?
			if ( p[j] == RC_MESH_NULL_IDX )
                break;

            // Add triangle from fan.
            indices.push_back( p[0] );
            indices.push_back( p[j-1] );
            indices.push_back( p[j] );

		} // Next triangle
	
    } // Next polygon
    
    // Allocate a new mesh and populate it.
    cgMesh * renderMesh = new cgMesh( 0, CG_NULL );
    renderMesh->prepareMesh( cgVertexFormat::formatFromDeclarator( cgVertex::Declarator ), false, resources );
    renderMesh->setVertexSource( &vertices.front(), vertices.size(), cgVertexFormat::formatFromFVF( D3DFVF_XYZ ) );
    if ( !renderMesh->addPrimitives( cgPrimitiveType::TriangleList, &indices.front(), 0, indices.size(), cgMaterialHandle::Null ) )
    {
        renderMesh->deleteReference();
        cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Failed to add primitives to navigation tile debug mesh.\n") );
        return;
    
    } // End if failed

    // Finalize the mesh
    if ( !renderMesh->endPrepare() )
    {
        renderMesh->deleteReference();
        cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Failed to finalize navigation tile debug mesh.\n") );
        return;
    
    } // End if failed

    // Add to the resource manager
    if ( !resources->addMesh( &mDebugMesh, renderMesh, cgResourceFlags::ForceNew, cgString::Empty, cgDebugSource() ) )
    {
        renderMesh->deleteReference();
        cgAppLog::write( cgAppLog::Warning | cgAppLog::Debug, _T("Failed to add navigation tile debug mesh to the resource manager.\n") );
        return;
    
    } // End if failed

	/*const int nvp = mesh.nvp;
	const float cs = mesh.cs;
	const float ch = mesh.ch;
	const float* orig = mesh.bmin;
	
	dd->begin(DU_DRAW_TRIS);
	
	for (int i = 0; i < mesh.npolys; ++i)
	{
		const unsigned short* p = &mesh.polys[i*nvp*2];
		
		unsigned int color;
		if (mesh.areas[i] == RC_WALKABLE_AREA)
			color = duRGBA(0,192,255,64);
		else if (mesh.areas[i] == RC_NULL_AREA)
			color = duRGBA(0,0,0,64);
		else
			color = duIntToCol(mesh.areas[i], 255);
		
		unsigned short vi[3];
		for (int j = 2; j < nvp; ++j)
		{
			if (p[j] == RC_MESH_NULL_IDX) break;
			vi[0] = p[0];
			vi[1] = p[j-1];
			vi[2] = p[j];
			for (int k = 0; k < 3; ++k)
			{
				const unsigned short* v = &mesh.verts[vi[k]*3];
				const float x = orig[0] + v[0]*cs;
				const float y = orig[1] + (v[1]+1)*ch;
				const float z = orig[2] + v[2]*cs;
				dd->vertex(x,y,z, color);
			}
		}
	}
	dd->end();

	// Draw neighbours edges
	const unsigned int coln = duRGBA(0,48,64,32);
	dd->begin(DU_DRAW_LINES, 1.5f);
	for (int i = 0; i < mesh.npolys; ++i)
	{
		const unsigned short* p = &mesh.polys[i*nvp*2];
		for (int j = 0; j < nvp; ++j)
		{
			if (p[j] == RC_MESH_NULL_IDX) break;
			if (p[nvp+j] & 0x8000) continue;
			const int nj = (j+1 >= nvp || p[j+1] == RC_MESH_NULL_IDX) ? 0 : j+1; 
			const int vi[2] = {p[j], p[nj]};
			
			for (int k = 0; k < 2; ++k)
			{
				const unsigned short* v = &mesh.verts[vi[k]*3];
				const float x = orig[0] + v[0]*cs;
				const float y = orig[1] + (v[1]+1)*ch + 0.1f;
				const float z = orig[2] + v[2]*cs;
				dd->vertex(x, y, z, coln);
			}
		}
	}
	dd->end();
	
	// Draw boundary edges
	const unsigned int colb = duRGBA(0,48,64,220);
	dd->begin(DU_DRAW_LINES, 2.5f);
	for (int i = 0; i < mesh.npolys; ++i)
	{
		const unsigned short* p = &mesh.polys[i*nvp*2];
		for (int j = 0; j < nvp; ++j)
		{
			if (p[j] == RC_MESH_NULL_IDX) break;
			if ((p[nvp+j] & 0x8000) == 0) continue;
			const int nj = (j+1 >= nvp || p[j+1] == RC_MESH_NULL_IDX) ? 0 : j+1; 
			const int vi[2] = {p[j], p[nj]};
			
			unsigned int col = colb;
			if ((p[nvp+j] & 0xf) != 0xf)
				col = duRGBA(255,255,255,128);
			for (int k = 0; k < 2; ++k)
			{
				const unsigned short* v = &mesh.verts[vi[k]*3];
				const float x = orig[0] + v[0]*cs;
				const float y = orig[1] + (v[1]+1)*ch + 0.1f;
				const float z = orig[2] + v[2]*cs;
				dd->vertex(x, y, z, col);
			}
		}
	}
	dd->end();
	
	dd->begin(DU_DRAW_POINTS, 3.0f);
	const unsigned int colv = duRGBA(0,0,0,220);
	for (int i = 0; i < mesh.nverts; ++i)
	{
		const unsigned short* v = &mesh.verts[i*3];
		const float x = orig[0] + v[0]*cs;
		const float y = orig[1] + (v[1]+1)*ch + 0.1f;
		const float z = orig[2] + v[2]*cs;
		dd->vertex(x,y,z, colv);
	}
	dd->end();*/
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Insert (or update) the database entries associated with this navigation
/// tile.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationTile::serialize( cgUInt32 parentId, cgWorld * world )
{
    if ( (cgGetSandboxMode() == cgSandboxMode::Enabled) && !mDatabaseId )
    {
        cgByte * dataOut = CG_NULL;

        // Insert data for this controller.
        prepareQueries( world );
        mInsertTile.bindParameter( 1, parentId );
        mInsertTile.bindParameter( 2, mTileX );
        mInsertTile.bindParameter( 3, mTileY );
        mInsertTile.bindParameter( 4, mTileZ );
        if ( !mNavData.empty() )
            mInsertTile.bindParameter( 5, &mNavData[0], mNavData.size() );
        else
            mInsertTile.bindParameter( 5, CG_NULL, 0 );

        // Build data for poly mesh.
        mInsertTile.bindParameter( 6, CG_NULL, 0 );
        if ( mPolyMesh )
        {
            // Compute size.
            cgUInt32 dataSize = 52; // Basic properties
            dataSize += sizeof(unsigned short)*mPolyMesh->nverts*3; // verts
            dataSize += sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2; // polys
            dataSize += sizeof(unsigned short)*mPolyMesh->npolys; // regs
            dataSize += sizeof(unsigned char)*mPolyMesh->npolys; // areas
            dataSize += sizeof(unsigned short)*mPolyMesh->npolys; // flags

            // Allocate storage
            dataOut = new cgByte[dataSize];
            cgByte * data = dataOut;
            
            // Build basic properties.
            memcpy( data, &mPolyMesh->nverts, sizeof(cgInt32) );
            data += sizeof(cgInt32);
            memcpy( data, &mPolyMesh->npolys, sizeof(cgInt32) );
            data += sizeof(cgInt32);
            memcpy( data, &mPolyMesh->maxpolys, sizeof(cgInt32) );
            data += sizeof(cgInt32);
            memcpy( data, &mPolyMesh->nvp, sizeof(cgInt32) );
            data += sizeof(cgInt32);
            memcpy( data, mPolyMesh->bmin, sizeof(cgFloat) * 3 );
            data += sizeof(cgFloat) * 3;
            memcpy( data, mPolyMesh->bmax, sizeof(cgFloat) * 3 );
            data += sizeof(cgFloat) * 3;
            memcpy( data, &mPolyMesh->cs, sizeof(cgFloat) );
            data += sizeof(cgFloat);
            memcpy( data, &mPolyMesh->ch, sizeof(cgFloat) );
            data += sizeof(cgFloat);
            memcpy( data, &mPolyMesh->borderSize, sizeof(cgInt32) );
            data += sizeof(cgInt32);

            // Copy data.
            memcpy( data, mPolyMesh->verts, sizeof(unsigned short)*mPolyMesh->nverts*3 );
            data += sizeof(unsigned short)*mPolyMesh->nverts*3;
            memcpy( data, mPolyMesh->polys, sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2 );
            data += sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2;
            memcpy( data, mPolyMesh->regs, sizeof(unsigned short)*mPolyMesh->npolys );
            data += sizeof(unsigned short)*mPolyMesh->npolys;
            memcpy( data, mPolyMesh->areas, sizeof(unsigned char)*mPolyMesh->npolys );
            data += sizeof(unsigned char)*mPolyMesh->npolys;
            memcpy( data, mPolyMesh->flags, sizeof(unsigned short)*mPolyMesh->npolys );
            data += sizeof(unsigned short)*mPolyMesh->npolys;

            // Output!
            mInsertTile.bindParameter( 6, dataOut, dataSize );        

        } // End if valid poly mesh
        
        // Process!
        if ( !mInsertTile.step( true ) )
        {
            delete []dataOut;
            cgString error;
            mInsertTile.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for navigation tile <%i, %i, %i>. Error: %s\n"), mTileX, mTileY, mTileZ, error.c_str() );
            return false;

        } // End if failed

        // Clean up
        delete []dataOut;

        // Retrieve the new database record identifier for
        // later alterations.
        mDatabaseId = mInsertTile.getLastInsertId();

    } // End if can insert

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Import the data associated with this animation tile based on the supplied 
/// query object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgNavigationTile::deserialize( cgWorldQuery & tileQuery, bool cloning )
{
    // Get the required basic parameters for this tile
    cgUInt32 databaseId;
    tileQuery.getColumn( _T("TileId"), databaseId );
    tileQuery.getColumn( _T("TileX"), mTileX );
    tileQuery.getColumn( _T("TileY"), mTileY );
    tileQuery.getColumn( _T("TileZ"), mTileZ );

    // Retrieve navigation data
    cgByte * data;
    cgUInt32 dataSize;
    tileQuery.getColumn( _T("NavData"), (void**)&data, dataSize );
    if ( data && dataSize > 0 )
    {
        mNavData.resize( dataSize );
        memcpy( &mNavData[0], data, dataSize );
    
    } // End if valid nav data

    // Retrieve poly mesh.
    tileQuery.getColumn( _T("PolyData"), (void**)&data, dataSize );
    if ( data && dataSize > 0 )
    {
        // Allocate new mesh
        mPolyMesh = rcAllocPolyMesh();

        // Get basic properties.
        memcpy( &mPolyMesh->nverts, data, sizeof(cgInt32) );
        data += sizeof(cgInt32);
        memcpy( &mPolyMesh->npolys, data, sizeof(cgInt32) );
        data += sizeof(cgInt32);
        memcpy( &mPolyMesh->maxpolys, data, sizeof(cgInt32) );
        data += sizeof(cgInt32);
        memcpy( &mPolyMesh->nvp, data, sizeof(cgInt32) );
        data += sizeof(cgInt32);
        memcpy( mPolyMesh->bmin, data, sizeof(cgFloat) * 3 );
        data += sizeof(cgFloat) * 3;
        memcpy( mPolyMesh->bmax, data, sizeof(cgFloat) * 3 );
        data += sizeof(cgFloat) * 3;
        memcpy( &mPolyMesh->cs, data, sizeof(cgFloat) );
        data += sizeof(cgFloat);
        memcpy( &mPolyMesh->ch, data, sizeof(cgFloat) );
        data += sizeof(cgFloat);
        memcpy( &mPolyMesh->borderSize, data, sizeof(cgInt32) );
        data += sizeof(cgInt32);

        // Allocate storage.
        mPolyMesh->verts = (unsigned short*)rcAlloc(sizeof(unsigned short)*mPolyMesh->nverts*3, RC_ALLOC_PERM);
        mPolyMesh->polys = (unsigned short*)rcAlloc(sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2, RC_ALLOC_PERM);
        mPolyMesh->regs  = (unsigned short*)rcAlloc(sizeof(unsigned short)*mPolyMesh->npolys, RC_ALLOC_PERM);
        mPolyMesh->areas = (unsigned char*)rcAlloc(sizeof(unsigned char)*mPolyMesh->npolys, RC_ALLOC_PERM);
        mPolyMesh->flags = (unsigned short*)rcAlloc(sizeof(unsigned short)*mPolyMesh->npolys, RC_ALLOC_PERM);

        // Load data.
        memcpy( mPolyMesh->verts, data, sizeof(unsigned short)*mPolyMesh->nverts*3 );
        data += sizeof(unsigned short)*mPolyMesh->nverts*3;
        memcpy( mPolyMesh->polys, data, sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2 );
        data += sizeof(unsigned short)*mPolyMesh->npolys*mPolyMesh->nvp*2;
        memcpy( mPolyMesh->regs, data, sizeof(unsigned short)*mPolyMesh->npolys );
        data += sizeof(unsigned short)*mPolyMesh->npolys;
        memcpy( mPolyMesh->areas, data, sizeof(unsigned char)*mPolyMesh->npolys );
        data += sizeof(unsigned char)*mPolyMesh->npolys;
        memcpy( mPolyMesh->flags, data, sizeof(unsigned short)*mPolyMesh->npolys );
        data += sizeof(unsigned short)*mPolyMesh->npolys;
    
    } // End if valid poly data

    // Only associate with the original database row identifier if we were
    // not instructed to clone data. By ensuring that the 'databaseId'
    // is '0', this will force the insertion of a new entry on next serialize.
    if ( cloning )
        mDatabaseId = 0;
    else
        mDatabaseId = databaseId;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgNavigationTile::prepareQueries( cgWorld * world )
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        // Prepare the SQL statements as necessary.
        if ( !mInsertTile.isPrepared( world ) )
            mInsertTile.prepare( world, _T("INSERT INTO 'DataSources::NavigationMesh::Tiles' VALUES(NULL,?1,?2,?3,?4,?5,?6)"), true );
        
    } // End if sandbox
}