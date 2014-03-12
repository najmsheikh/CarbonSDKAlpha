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
// File: cgBSPVisTree.cpp                                                    //
//                                                                           //
// Desc: BSP tree / PVS combination used to provide static visibility and    //
//       occlusion data for the scene based on any static scene objects.     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgBSPVisTree Module Includes
//-----------------------------------------------------------------------------
#include <World/cgBSPVisTree.h>
#include <Resources/cgMesh.h>
#include <Rendering/cgVertexFormats.h>
#include <Math/cgCollision.h>
#include <Math/cgMathUtility.h>
#include <Math/cgBoundingSphere.h>

///////////////////////////////////////////////////////////////////////////////
// cgBSPTree Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// Name : cgBSPTree() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgBSPTree::cgBSPTree( )
{
    // Initialize variables to sensible defaults
    mSplitterSample = 120;
    mSplitHeuristic = 2.0f;
    mInputWindings  = CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : ~cgBSPTree() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgBSPTree::~cgBSPTree()
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
void cgBSPTree::dispose( bool disposeBase )
{
    // Clear containers
    mNodePlanes.clear();
    mNodes.clear();
    mLeaves.clear();
    mPortals.clear();
    mPortalVertices.clear();
    mInputVertices.clear();
    mPVSData.clear();

    // Clear out winding data.
    releaseWindings( mInputWindings );
    mInputWindings = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getPVSData ()
/// <summary>
/// Retrieve the compiled PVS data (if any) for this BSP tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgByteArray & cgBSPTree::getPVSData( ) const
{
    return mPVSData;
}

//-----------------------------------------------------------------------------
//  Name : getLeaves ()
/// <summary>
/// Retrieve the list of leaves generated during the compilation of this BSP
/// tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgBSPTree::LeafArray & cgBSPTree::getLeaves( ) const
{
    return mLeaves;
}

//-----------------------------------------------------------------------------
//  Name : getNodes ()
/// <summary>
/// Retrieve the list of nodes generated during the compilation of this BSP
/// tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgBSPTree::NodeArray & cgBSPTree::getNodes( ) const
{
    return mNodes;
}

//-----------------------------------------------------------------------------
//  Name : getNodePlanes ()
/// <summary>
/// Retrieve the list of node planes generated during the compilation of this
/// BSP tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgBSPTree::PlaneArray & cgBSPTree::getNodePlanes( ) const
{
    return mNodePlanes;
}

//-----------------------------------------------------------------------------
//  Name : getPortals ()
/// <summary>
/// Retrieve the list of portals generated during the compilation of the PVS
/// for this BSP tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgBSPTree::PortalArray & cgBSPTree::getPortals( ) const
{
    return mPortals;
}

//-----------------------------------------------------------------------------
//  Name : getPortalVertices ()
/// <summary>
/// Retrieve the vertex buffer that contains the boundary points describing
/// the portals generated during the compilation of the PVS for this BSP tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgBSPTree::PointArray & cgBSPTree::getPortalVertices( ) const
{
    return mPortalVertices;
}

//-----------------------------------------------------------------------------
// Name : releaseWindings ( ) (Protected)
/// <summary>
/// Release allocated winding data in the supplied list.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::releaseWindings( Winding * windingList )
{
    for ( Winding * winding = windingList, * next = CG_NULL; winding; winding = next )
    {
        next = winding->next;
        delete winding;
    
    } // Next winding
}

//-----------------------------------------------------------------------------
// Name : countSplitters ( ) (Protected)
/// <summary>
/// Count the number of windings that remain in the specified list that have
/// *not* yet been considered as a splitter.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBSPTree::countSplitters( Winding * windingList )
{
    cgUInt32 splitters = 0;
    for ( Winding * winding = windingList; winding; winding = winding->next )
    {
        if ( !winding->used )
            ++splitters;
    
    } // Next winding
    return splitters;
}

//-----------------------------------------------------------------------------
// Name : buildTree ( )
/// <summary>
/// Compile the spatial tree based on the specified configuration and geometry
/// data specified.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPTree::buildTree( cgUInt32 meshCount, const cgMeshHandle meshes[], const cgTransform transforms[] )
{
    // Merge the supplied geometry into the final combined data set.
    if ( !mergeMeshes( meshCount, meshes, transforms ) )
    {
        cgAppLog::write( cgAppLog::Warning, _T("Unable to compile BSP tree because no valid mesh data was supplied.\n") );
        return false;

    } // End if failed

    // Compute the combined set of node planes.
    buildPlaneSet();

    // Allocate the root node and build the tree.
    mNodes.push_back( cgBSPTreeSubNode() );
    buildTree( 0, 0, mInputWindings );

    // Clear out input data.
    mInputWindings = CG_NULL; // Released during construction.
    mInputVertices.clear();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : findLeaf ( )
/// <summary>
/// Find the leaf (if any) into which the specified point falls. If the point
/// is not contained in a valid empty leaf, this method returns a value of
/// 'InvalidLeaf' if no leaf was found, or 'SolidLeaf' if the point falls into
/// solid space.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBSPTree::findLeaf( const cgVector3 & point )
{
    if ( mNodes.empty() )
        return InvalidLeaf;
    return findLeaf( 0, point );
}

//-----------------------------------------------------------------------------
// Name : findLeaf ( ) (Protected, Recursive)
/// <summary>
/// Find the leaf (if any) into which the specified point falls. If the point
/// is not contained in a valid empty leaf, this method returns a value of
/// 'InvalidLeaf' if no leaf was found, or 'SolidLeaf' if the point falls into
/// solid space.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBSPTree::findLeaf( cgUInt32 nodeIndex, const cgVector3 & point )
{
    cgUInt32 output;

    // Local scope
    {
        // Classify the point against the node plane.
        const cgBSPTreeSubNode & node = mNodes[nodeIndex];
        const cgPlane & plane = mNodePlanes[node.plane];
        switch ( cgCollision::pointClassifyPlane( point, (cgVector3&)plane, plane.d, CGE_EPSILON_1MM ) )
        {
            case cgPlaneQuery::On:
                // Pass down the front in the on plane case
            case cgPlaneQuery::Front:
                output = node.front;
                break;
            case cgPlaneQuery::Back:
                output = node.back;
                break;            
        } // End switch 

    } // End scope

    // Is this a leaf?
    if ( (output & LeafNodeBit) )
    {
        // Return the leaf index
        return ( output == SolidLeaf ) ? SolidLeaf : (output & LeafIndexMask);

    } // End if leaf
    else
    {
        // Recurse into this node.
        return findLeaf( output, point );

    } // End if node

}

void cgBSPTree::findLeaves( cgUInt32 * leaves, cgUInt32 & leafCount, const cgBoundingSphere & sphere, cgUInt32 leafSourceVisRestrict )
{
    leafCount = 0;
    if ( !mNodes.empty() )
    {
        cgByte * sourceVis = CG_NULL;
        if ( !mPVSData.empty() && leafSourceVisRestrict != InvalidLeaf && leafSourceVisRestrict != SolidLeaf )
            sourceVis = &mPVSData[mLeaves[leafSourceVisRestrict].visibilityOffset];
        findLeaves( 0, leaves, leafCount, sphere, sourceVis );
    
    } // End if has nodes
}

void cgBSPTree::findLeaves( cgUInt32 nodeIndex, cgUInt32 * leaves, cgUInt32 & leafCount, const cgBoundingSphere & sphere, cgByte sourceVis[] )
{
    cgUInt32 outgoing1, outgoing2;

    // Local scope
    {
        // Classify the sphere against the node plane.
        const cgBSPTreeSubNode & node = mNodes[nodeIndex];
        const cgPlane & plane = mNodePlanes[node.plane];
        cgFloat distance = cgPlane::dotCoord( plane, sphere.position );

        // What's the scoop?
        if ( distance >= sphere.radius )
        {
            // Sphere is entirely in front of the plane.
            outgoing1 = node.front;
            outgoing2 = SolidLeaf;

        } // End if in front
        else if ( distance <= -sphere.radius )
        {
            // Sphere is entirely behind the plane.
            outgoing1 = node.back;
            outgoing2 = SolidLeaf;
        
        } // End if behind
        else
        {
            // Sphere is spanning the plane.
            outgoing1 = node.front;
            outgoing2 = node.back;

        } // End if spanning

    } // End scope

    // Test the first result
    if ( outgoing1 != SolidLeaf )
    {
        // Is this a leaf or node?
        if ( (outgoing1 & LeafNodeBit) )
        {
            if ( !sourceVis || cgBSPTree::getPVSBit( sourceVis, (outgoing1 & LeafIndexMask) ) )
                leaves[leafCount++] = (outgoing1 & LeafIndexMask);
        
        } // End if leaf
        else
            findLeaves( outgoing1, leaves, leafCount, sphere, sourceVis );

    } // End if valid

    // Test the second result
    if ( outgoing2 != SolidLeaf )
    {
        // Is this a leaf or node?
        if ( (outgoing2 & LeafNodeBit) )
        {
            if ( !sourceVis || cgBSPTree::getPVSBit( sourceVis, (outgoing2 & LeafIndexMask) ) )
                leaves[leafCount++] = (outgoing2 & LeafIndexMask);

        } // End if leaf
        else
            findLeaves( outgoing2, leaves, leafCount, sphere, sourceVis );

    } // End if valid
}

//-----------------------------------------------------------------------------
// Name : isVolumeVisible ( )
/// <summary>
/// Determine if the specified volume is visible with respect the supplied
/// source leaf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPTree::isVolumeVisible( cgUInt32 sourceLeaf, const cgBoundingSphere & sphere )
{
    if ( mPVSData.empty() || mNodes.empty() || sourceLeaf == InvalidLeaf || sourceLeaf == SolidLeaf )
        return true;

    // Get the visibility data for the specified source leaf.
    cgByte * visData = &mPVSData[mLeaves[sourceLeaf].visibilityOffset];
    return isVolumeVisible( 0, visData, sphere );
}

//-----------------------------------------------------------------------------
// Name : isVolumeVisible ( ) (Protected, Recursive)
/// <summary>
/// Determine if the specified volume is visible with respect the supplied
/// source leaf.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPTree::isVolumeVisible( cgUInt32 nodeIndex, cgByte * leafVisData, const cgBoundingSphere & sphere )
{
    cgUInt32 outgoing1, outgoing2;

    // Local scope
    {
        // Classify the sphere against the node plane.
        const cgBSPTreeSubNode & node = mNodes[nodeIndex];
        const cgPlane & plane = mNodePlanes[node.plane];
        cgFloat distance = cgPlane::dotCoord( plane, sphere.position );

        // What's the scoop?
        if ( distance >= sphere.radius )
        {
            // Sphere is entirely in front of the plane.
            outgoing1 = node.front;
            outgoing2 = SolidLeaf;

        } // End if in front
        else if ( distance <= -sphere.radius )
        {
            // Sphere is entirely behind the plane.
            outgoing1 = node.back;
            outgoing2 = SolidLeaf;
        
        } // End if behind
        else
        {
            // Sphere is spanning the plane.
            outgoing1 = node.front;
            outgoing2 = node.back;

        } // End if spanning

    } // End scope

    // Test the first result
    if ( outgoing1 != SolidLeaf )
    {
        // Is this a leaf?
        if ( (outgoing1 & LeafNodeBit) )
            return getPVSBit( leafVisData, (outgoing1 & LeafIndexMask) );
        else
        {
            // We can skip any further processing the moment that
            // a result of 'true' is returned from our children.
            // Otherwise, we need to continue processing.
            if ( isVolumeVisible( outgoing1, leafVisData, sphere ) )
                return true;

        } // End if node

    } // End if valid

    // Test the second result
    if ( outgoing2 != SolidLeaf )
    {
        // Is this a leaf?
        if ( (outgoing2 & LeafNodeBit) )
            return getPVSBit( leafVisData, (outgoing2 & LeafIndexMask) );
        else
        {
            // We can skip any further processing the moment that
            // a result of 'true' is returned from our children.
            // Otherwise, we need to continue processing.
            if ( isVolumeVisible( outgoing2, leafVisData, sphere ) )
                return true;

        } // End if node

    } // End if valid

    // Not visible.
    return false;
}

//-----------------------------------------------------------------------------
// Name : compilePVS ( )
/// <summary>
/// Precompute visibility information for the compiled BSP tree.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPTree::compilePVS( )
{
    size_t totalVertices = 0;

    // ToDo: Modify winding to just store the points. InputVertices array gets out of hand during portal generation.

    // Generate a portal for each spatial tree node
    size_t nodeCount = mNodes.size();
    for ( size_t i = 0; i < nodeCount; ++i ) 
    {
        const cgBSPTreeSubNode & node = mNodes[i];
        
        // Skip any leaf nodes that have solid space behind them.
        if ( node.back == SolidLeaf )
            continue;
        
        // Generate the initial portal for this node to fit inside 
        // the root node bounding box.
        Portal * portal = generatePortal( i, mBounds );
        
        // Clip the portal and obtain a list of all fragments
        Portal * portalList = clipPortal( 0, portal, NoOwner );

        // Store final portals and release the temporary versions.
        Portal * next = CG_NULL;
        for ( portal = portalList; portal; portal = next )
        {
            next = (Portal*)portal->next;

            // Push into leaves that contain this fragment.
            if ( portal->leafOwner[FrontOwner] != cgUInt32(-1) )
                mLeaves[portal->leafOwner[FrontOwner]].portals.push_back( mPortals.size() );
            if ( portal->leafOwner[BackOwner] != cgUInt32(-1) )
                mLeaves[portal->leafOwner[BackOwner]].portals.push_back( mPortals.size() );

            // Sum up total vertices used by all surviving portal fragments.
            totalVertices += portal->vertexCount;

            // Add to the final portal list.
            portal->next = CG_NULL;
            mPortals.push_back( *portal );

            // Clean up.
            delete portal;
        
        } // Next portal

    } // Next node

    // Consolidate final vertex list.
    mPortalVertices.resize( totalVertices );
    totalVertices = 0;
    for ( size_t i = 0; i < mPortals.size(); ++i )
    {
        memcpy( &mPortalVertices[totalVertices], &mInputVertices[mPortals[i].firstVertex], mPortals[i].vertexCount * sizeof(cgVector3) );
        mPortals[i].firstVertex = totalVertices;
        totalVertices += mPortals[i].vertexCount;

    } // Next portal
    mInputVertices.clear();

    // Calculate Number Of Bytes needed to store each leaf's visibility
	// array in BIT form (i.e 8 leafs vis per byte uncompressed). 32 bit 
    // align the bytes per set to allow for our early out long conversion.
    mPVSBytesPerSet = (mLeaves.size() + 7) / 8;
	mPVSBytesPerSet = (mPVSBytesPerSet * 3 + 3) & 0xFFFFFFFC;

    // Retrieve all of our one way portals
    PVSPortalArray portals;
	generatePVSPortals( portals );
    
    // Calculate initial portal visibility
    initialPortalVis( portals );
    
    // Perform actual full PVS calculation
    calculatePortalVis( portals );

    // Just use possible vis.
    /*for ( size_t i = 0; i < portals.size(); ++i )
        portals[i].actualVis = portals[i].possibleVis;*/
    
    // Export the visibility set to the final master array.
    // First reserve enough space to hold every leaf's PVS set.
    mPVSData.clear();
    mPVSData.resize( mLeaves.size() * (mPVSBytesPerSet * 2), 0 );

    // Loop round each leaf and collect the vis info
    // this is all OR'd together and ZRLE compressed
    // then finally stored in the master array.
    cgUInt32 visibilityOffset = 0;
    cgByteArray leafPVS( mPVSBytesPerSet );
    for ( size_t i = 0; i < mLeaves.size(); ++i ) 
    {
        cgBSPTreeLeaf & leaf = mLeaves[i];
        cgByte * leafSet = &leafPVS[0];
        memset( leafSet, 0, mPVSBytesPerSet );

        // Write PVS starting location to the leaf.
        mLeaves[i].visibilityOffset = visibilityOffset;
    
        // Current leaf is always visible
        setPVSBit( leafSet, i );
        
        // Loop through all portals in this leaf
        for ( size_t p = 0; p < leaf.portals.size(); ++p )
        {
            // Find correct portal index (the one IN this leaf)
            cgUInt32 portalIndex = leaf.portals[ p ] * 2;
            if ( portals[portalIndex].leaf == i )
                ++portalIndex;
            
            // Combine the vis bits together
            PVSPortal & portal = portals[portalIndex];
            for ( cgUInt32 j = 0; j < mPVSBytesPerSet; ++j ) 
                leafSet[j] |= portal.actualVis[j];
            
        } // Next portal

        #define PVS_COMPRESSDATA 0
        #if ( PVS_COMPRESSDATA )
 
            // Compress the leaf set here and update our master write pointer
            visibilityOffset += compressLeafSet( &mPVSData[0], leafSet, visibilityOffset );

        #else

            // Copy the data into the Master PVS Set
            memcpy( &mPVSData[ visibilityOffset ], leafSet, mPVSBytesPerSet );
            visibilityOffset += mPVSBytesPerSet;

        #endif

    } // Next Leaf

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : compressLeafSet () (Protected)
/// <summary>
/// ZRLE Compresses the uncompressed vis bit array which was passed in, and
/// compresses and adds it to the master PVS Array, this function returns
/// the size of the compressed set so we can update our write pointer.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBSPTree::compressLeafSet( cgByte masterPVS[], const cgByte visArray[], cgUInt32 writePos )
{
	// Set dynamic pointer to start position
    cgByte * dest = &masterPVS[writePos];
	
    // Loop through and compress the set
	for ( cgUInt32 j = 0; j < mPVSBytesPerSet; ++j ) 
    {
        // Store the current 8 leaves
		*dest++ = visArray[j];

        // Don't compress if all bits are not zero
		if ( visArray[j] )
            continue;

        // Count the number of 0 bytes
		cgUInt32 repeatCount = 1;
		for ( ++j; j < mPVSBytesPerSet; ++j ) 
        {
            // Keep counting until byte != 0 or we reach our max repeat count
			if ( visArray[j] || repeatCount == 255)
                break;
            else
                ++repeatCount;
		
        } // Next byte
		
        // Store our repeat count
        *dest++ = (cgByte)repeatCount;

        // Step back one byte because the outer loop
        // will increment. We are already at the correct pos.
		j--;
	
    } // Next byte
	
    // Return written size
	return (cgUInt32)(dest - &masterPVS[ writePos ]);
}

//-----------------------------------------------------------------------------
// Name : calculatePortalVis() (Protected)
/// <summary>
/// Top level PVS calculation function which starts the recursion for each 
/// portal.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::calculatePortalVis( PVSPortalArray & portals )
{
    PVSData data;

    // Lets process those portal bad boys!! ;)
    for ( cgUInt32 i = getNextPVSPortal( portals ); i != cgUInt32(-1); i = getNextPVSPortal( portals ) )
    {
        PVSPortal & portal = portals[i];

        // Fill our our initial data structure
        data.sourcePoints = portal.points;
        data.visBits      = portal.possibleVis;
        data.targetPlane  = getPVSPortalPlane( portal );
	    
        // Allocate the portal's actual visibility array
        portal.actualVis.resize( mPVSBytesPerSet, 0 );
        
        // Step in and begin processing this portal
        recursePVS( portals, portal.leaf, portal, data );
        
        // We've finished processing this portal
        portal.status = Processed;

    } // Next portal
}

//-----------------------------------------------------------------------------
// Name : getNextPVSPortal() (Protected)
/// <summary>
/// Function that returns the next portal in order of complexity. This ensures
/// that all the least complex portals are processed first so that these 
/// portal's vis info can be used in the early out system in recursePVS to help 
/// speed things up.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgBSPTree::getNextPVSPortal( PVSPortalArray & portals )
{
    // ToDo: optimize by having dedicated unprocessed list separate from processed.
    cgUInt32 portalIndex = cgUInt32(-1), minVis = 9999999;
	
    // Loop through all portals
	for ( size_t i = 0; i < portals.size(); ++i ) 
    {
        // If this portal's complexity is the lowest and it has 
        // not already been processed then we could use it.
        const PVSPortal & portal = portals[i];
		if ( portal.status == Unprocessed && portal.possibleVisCount < minVis ) 
        {
			minVis = portal.possibleVisCount;
			portalIndex = i;

		} // End if less complex

	} // Next portal

	// Set the status flag of the selected portal to 'being worked on'.
    if ( portalIndex != cgUInt32(-1) )
        portals[portalIndex].status = Processing;

    // Return the next portal
	return portalIndex;
}

//-----------------------------------------------------------------------------
// Name : clipPVSPortalPoints () (Protected)
/// <summary>
/// TODO
/// </summary>
//-----------------------------------------------------------------------------
cgBSPTree::PVSPortalPoints * cgBSPTree::clipPVSPortalPoints( PVSPortalPoints * points, const cgPlane & plane, bool keepOnPlane )
{
    switch ( cgCollision::polyClassifyPlane( points->vertices, points->vertexCount, sizeof(cgVector3), (cgVector3&)plane, plane.d, CGE_EPSILON_1MM ) )
    {
        case cgPlaneQuery::Front:
            return points;

        case cgPlaneQuery::Back:
            return CG_NULL;

        case cgPlaneQuery::On:
            return ( keepOnPlane ) ? points : CG_NULL;

        case cgPlaneQuery::Spanning:
        {
            static cgArray<cgPlaneQuery::Class> pointLocation;
            if ( pointLocation.size() < points->vertexCount )
            {
                pointLocation.clear();
                pointLocation.resize( points->vertexCount );
            
            } // End if buffer too small

            // Determine each point's location relative to the plane.
            size_t inFront = 0, behind = 0, onPlane = 0;
	        for ( size_t i = 0; i < points->vertexCount; ++i )
            {
                // Classify the vertex.
                switch ( pointLocation[i] = cgCollision::pointClassifyPlane( points->vertices[i], (cgVector3&)plane, plane.d, CGE_EPSILON_1MM ) )
                {
                    case cgPlaneQuery::Front:
                        ++inFront;
                        break;
                    
                    case cgPlaneQuery::Back:
                        ++behind;
                        break;
                    
                    default:
                        ++onPlane;
                        break;
                
                } // End switch location

	        } // Next Vertex

            // Allocate space for split winding fragments
            static PointArray frontVerts;
            if ( frontVerts.size() < (points->vertexCount + 1) )
            {
                frontVerts.clear();
                frontVerts.resize( points->vertexCount + 1 );
            
            } // End if too small

            // Split the winding.
            size_t frontCount = 0, next = 0;
            for ( size_t i = 0; i < points->vertexCount; ++i ) 
            {
                next = (i+1) % points->vertexCount;

                // Place / duplicate vertex into relevant lists.
                if ( pointLocation[i] != cgPlaneQuery::Back )
                    frontVerts[frontCount++] = points->vertices[i];
                if ( pointLocation[i] == cgPlaneQuery::On )
                    continue;

                // If the next vertex is not causing us to span the plane then continue
                if ( pointLocation[next] == cgPlaneQuery::On || pointLocation[next] == pointLocation[i])
                    continue;
                
		        // Calculate the intersection point
                cgFloat t;
                const cgVector3 & origin = points->vertices[i];
                const cgVector3 vel( points->vertices[next] - origin );
                cgCollision::rayIntersectPlane( origin, vel, plane, t, true, false );
                const cgVector3 newPos = origin + vel * t;

                // Store new vertex
                frontVerts[frontCount++] = newPos;

            } // Next Vertex

            // Build the new points and return them.
            PVSPortalPoints * frontSplit = new PVSPortalPoints();
            frontSplit->ownsVertices    = true;
            frontSplit->vertexCount     = frontCount;
            frontSplit->vertices        = new cgVector3[ frontCount ];
            memcpy( frontSplit->vertices, &frontVerts[0], frontCount * sizeof(cgVector3) );
            return frontSplit;

        } // End case spanning
    
    } // End switch location

    return CG_NULL;
}

//-----------------------------------------------------------------------------
// Name : recursePVS() (Protected, Recursive)
/// <summary>
/// PVS recursion function, steps through the portals and calcs true visibility
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::recursePVS( PVSPortalArray & portals, cgUInt32 leafIndex, PVSPortal & sourcePortal, PVSData & prevData )
{
    // Mark this leaf as visible.
    setPVSBit( &sourcePortal.actualVis[0], leafIndex );
    
    // Allocate our current visibility buffer for this step.
    PVSData data;
    data.visBits.resize( mPVSBytesPerSet );

    // Retrieve data we will be using inside the loop
    cgUInt32 * possibleSet = (cgUInt32*)&data.visBits[0];
    cgUInt32 * visSet      = (cgUInt32*)&sourcePortal.actualVis[0];
    cgPlane sourcePlane    = getPVSPortalPlane( sourcePortal );

    // Check all portals for flow into other leaves
    cgBSPTreeLeaf & leaf = mLeaves[leafIndex];
    for ( size_t i = 0; i < leaf.portals.size(); ++i )
    {
        // Find correct portal index (the one IN this leaf (not neighbouring))
        cgUInt32 portalIndex = leaf.portals[i] * 2;
        if ( portals[portalIndex].leaf == leafIndex )
            ++portalIndex;
        
        // We can't possibly recurse through this portal if it's neighbour
        // leaf is set to invisible in the target portals PVS
        PVSPortal & generatorPortal = portals[portalIndex];
        if ( !getPVSBit( &prevData.visBits[0], generatorPortal.leaf ) )
            continue;

        // If the portal can't see anything we haven't already seen, skip it
        cgUInt32 * testSet = ( generatorPortal.status == Processed ) ? 
            (cgUInt32*)&generatorPortal.actualVis[0] :
            (cgUInt32*)&generatorPortal.possibleVis[0];
        
        // Check to see if we have processed as much as we need to
        // this is an early out system. We check in 32 bit chunks to
        // help speed the process up a little.
        bool furtherProcess = false;
        for ( size_t j = 0; j < mPVSBytesPerSet / sizeof(cgUInt32); ++j )
        {
            possibleSet[j] = ((cgUInt32*)&prevData.visBits[0])[j] & testSet[j];
            if ( possibleSet[j] & ~visSet[j] )
                furtherProcess = true;

        } // Next 32 bit chunk

        // Can we see anything new ??
        if ( !furtherProcess )
            continue;

        // The current generator plane will become the next recursions target plane
        data.targetPlane = getPVSPortalPlane( generatorPortal );
        
        // We can't recurse out of a coplanar face, so check it
        cgPlane reverseGenPlane = -data.targetPlane;
        if ( fabsf(reverseGenPlane.a - prevData.targetPlane.a) < CGE_EPSILON &&
             fabsf(reverseGenPlane.b - prevData.targetPlane.b) < CGE_EPSILON &&
             fabsf(reverseGenPlane.c - prevData.targetPlane.c) < CGE_EPSILON &&
             cgMathUtility::dynamicEpsilonTest( reverseGenPlane.d, prevData.targetPlane.d, 10 )
             /*fabsf(reverseGenPlane.d - prevData.targetPlane.d) < CGE_EPSILON*/ )
             continue;

        // Clip the generator portal to the source. If none remains, continue.
        PVSPortalPoints * generatorPoints = clipPVSPortalPoints( generatorPortal.points, sourcePlane, false );
        if ( generatorPoints != generatorPortal.points ) 
            releasePVSPortalPoints( generatorPortal.points );
        if ( !generatorPoints )
            continue;

        // The second leaf can only be blocked if coplanar
        if ( !prevData.targetPoints )
        {
            data.sourcePoints = prevData.sourcePoints;
            data.targetPoints = generatorPoints;
            recursePVS( portals, generatorPortal.leaf, sourcePortal, data );
            releasePVSPortalPoints( generatorPoints );
            continue;

        } // End if Previous Points

        // Clip the generator portal to the previous target. If none remains, continue.
        PVSPortalPoints * newPoints = clipPVSPortalPoints( generatorPoints, prevData.targetPlane, false );
        if ( newPoints != generatorPoints ) 
            releasePVSPortalPoints( generatorPoints );
        generatorPoints = newPoints;
        if ( !generatorPoints )
            continue;

        // Make a copy of the source portals points
        PVSPortalPoints * sourcePoints = new PVSPortalPoints();
        if ( prevData.sourcePoints )
        {
            sourcePoints->ownsVertices = true;
            sourcePoints->vertexCount = prevData.sourcePoints->vertexCount;
            sourcePoints->vertices = new cgVector3[ sourcePoints->vertexCount ];
            memcpy( sourcePoints->vertices, prevData.sourcePoints->vertices, sourcePoints->vertexCount * sizeof(cgVector3) );
        
        } // End if deep copy

        // Clip the source portal
        newPoints = clipPVSPortalPoints( sourcePoints, reverseGenPlane, false );
        if ( newPoints != sourcePoints )
            releasePVSPortalPoints( sourcePoints );
        sourcePoints = newPoints;

        // If none remains, continue to the next portal
        if ( !sourcePoints)
        {
            releasePVSPortalPoints( generatorPoints );
            continue;
        
        } // End if no source

        // Lets go Clipping :)
        generatorPoints = clipToAntiPenumbra( sourcePoints, prevData.targetPoints, generatorPoints, false ); 
        if ( !generatorPoints )
        {
            releasePVSPortalPoints( sourcePoints );
            continue;
        
        } // End if exhausted
        
        generatorPoints = clipToAntiPenumbra( prevData.targetPoints, sourcePoints, generatorPoints, true ); 
        if ( !generatorPoints )
        {
            releasePVSPortalPoints( sourcePoints );
            continue;
        
        } // End if exhausted
        
        sourcePoints = clipToAntiPenumbra( generatorPoints, prevData.targetPoints, sourcePoints, false ); 
        if ( !sourcePoints )
        {
            releasePVSPortalPoints( generatorPoints );
            continue;
        
        } // End if exhausted
        
        sourcePoints = clipToAntiPenumbra( prevData.targetPoints, generatorPoints, sourcePoints, true ); 
        if ( !sourcePoints )
        {
            releasePVSPortalPoints( generatorPoints );
            continue;
        
        } // End if exhausted

        // Store data for next recursion
        data.sourcePoints = sourcePoints;
        data.targetPoints = generatorPoints;

        // Flow through it for real
        recursePVS( portals, generatorPortal.leaf, sourcePortal, data );

        // Clean up
        releasePVSPortalPoints( sourcePoints );
        releasePVSPortalPoints( generatorPoints );

    } // Next portal
}

//-----------------------------------------------------------------------------
// Name : clipToAntiPenumbra() (Protected)
/// <summary>
/// Clips the portals to one another using the generated anti-penumbra.
/// </summary>
//-------------------------------------------------------------------------------------
cgBSPTree::PVSPortalPoints * cgBSPTree::clipToAntiPenumbra( PVSPortalPoints * source, PVSPortalPoints * target, PVSPortalPoints * generator, bool reverseClip )
{
    cgPlane plane;

    // Check all combinations
    for ( cgUInt32 i = 0; i < source->vertexCount; ++i )
    {
        // Build first edge
        cgUInt32 l = ( i + 1 ) % source->vertexCount;
        const cgVector3 v1 = source->vertices[l] - source->vertices[i];

        // Find a vertex belonging to the generator that makes a plane
        // which puts all of the vertices of the target on the front side
        // and all of the vertices of the source on the back side
        for ( cgUInt32 j = 0; j < target->vertexCount; ++j )
        {
            // Build second edge
            const cgVector3 v2 = target->vertices[ j ] - source->vertices[ i ];
            cgVector3::cross( (cgVector3&)plane, v1, v2 );

            // If points don't make a valid plane, skip it
            cgFloat length = plane.a * plane.a +
                             plane.b * plane.b +
                             plane.c * plane.c;
            if ( length < 0.1f ) continue;

            // Normalize the plane normal
            length = 1 / sqrtf( length );
            (cgVector3&)plane *= length;

            // Calculate the plane distance
            plane.d = -cgVector3::dot( target->vertices[ j ], (cgVector3&)plane );

            // Find out which side of the generated separating plane has the source portal
            cgUInt32 k;
            bool reverseTest = false;
            for ( k = 0; k < source->vertexCount; ++k )
            {
                // Skip if it matches other verts
                if ( k == i || k == l )
                    continue;

                // Classify the point
                cgPlaneQuery::Class location = cgCollision::pointClassifyPlane( source->vertices[k], (cgVector3&)plane, plane.d, CGE_EPSILON_1MM );
                if ( location == cgPlaneQuery::Back )
                {
                    // Source is on the negative side, so we want all pass
                    // and target on the positive side.
                    reverseTest = false;
                    break;
                
                } // End If Behind
                else if ( location == cgPlaneQuery::Front )
                {
                    // Source is on the positive sode, so we want all pass
                    // and target on the negative side.
                    reverseTest = true;
                    break;
                
                } // End if In Front

            } // Next source vertex

            // Planar with the source portal ?
            if ( k == source->vertexCount )
                continue;

            // Flip the normal if the source portal is backwards
            if ( reverseTest )
                plane = -plane;

            // If all of the pass portal points are now on the positive 
            // side then this is the separating plane.
            cgUInt32 counts[] = {0,0,0};
            for ( k = 0; k < target->vertexCount; ++k )
            {
                // Skip if the two match
                if ( k == j )
                    continue;

                // Classify the point
                cgPlaneQuery::Class location = cgCollision::pointClassifyPlane( target->vertices[k], (cgVector3&)plane, plane.d, CGE_EPSILON_1MM );
                if ( location == cgPlaneQuery::Back )
                    break;
                else if ( location == cgPlaneQuery::Front )
                    counts[0]++;
                else
                    counts[2]++;

            } // Next Target Vertex

            // Points on the negative side ?
            if ( k != target->vertexCount )
                continue;

            // Planar with separating plane ?
            if ( !counts[0] )
                continue;

            // Flip the normal if we want the back side
            if ( reverseClip )
                plane = -plane;

            // Clip the target by the separating plane
            PVSPortalPoints * newPoints = clipPVSPortalPoints( generator, plane, false );
            if ( newPoints != generator )
                releasePVSPortalPoints( generator );
            generator = newPoints;

            // Target is not visible ?
            if (!generator)
                return CG_NULL;

        } // Next target vertex

    } // Next source vertex

    // Success!!
    return generator;
}

//-----------------------------------------------------------------------------
// Name : releasePVSPortalPoints() (Static, Protected)
/// <summary>
/// Releases a set of portal points only if it is not owned by a physical 
/// portal.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::releasePVSPortalPoints( PVSPortalPoints * points )
{
    if ( points && !points->ownerPortal )
        delete points;
}

//-----------------------------------------------------------------------------
// Name : generatePVSPortals () (Protected)
/// <summary>
/// Given the computed set of two-way portals, create a set of one-way 
/// portals as required by the PVS compilation process.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::generatePVSPortals( PVSPortalArray & portals )
{
    // Allocate enough PVS portals to store one-way copies.
    portals.resize( mPortals.size() * 2 );

    // Loop through each portal, and duplicate necessary information.
    for ( size_t i = 0, p = 0; i < mPortals.size(); ++i, p += 2 ) 
    {
        // Link this PVS portal the base portal (note, we do not duplicate
        // because the vertices can remain under the ownership of the original 
        // BSP portal to save some memory). (Note only one portal will ultimately
        // own this outer pointer so only one should have 'OwnsPoints' set to true)
        PVSPortalPoints * pp    = new PVSPortalPoints();
        pp->vertices            = &mPortalVertices[ mPortals[i].firstVertex ];
        pp->vertexCount         = mPortals[i].vertexCount;
        pp->ownsVertices        = false;
        pp->ownerPortal         = &portals[p];

        // Create link information for front facing portal
        portals[p].points       = pp;
        portals[p].ownsPoints   = true;
        portals[p].plane        = mPortals[i].plane;
        portals[p].leafSide     = FrontOwner;
        portals[p].leaf         = mPortals[i].leafOwner[FrontOwner];
        
        // Create link information for back facing portal
        portals[p+1].points     = pp;
        portals[p+1].ownsPoints = false;
        portals[p+1].plane      = mPortals[i].plane;
        portals[p+1].leafSide   = BackOwner;
        portals[p+1].leaf       = mPortals[i].leafOwner[BackOwner];
        
    } // Next portal
}

//-----------------------------------------------------------------------------
// Name : initialPortalVis () (Protected)
/// <summary>
/// Performs the first set of visibility calculations between portals. This is 
/// essentially a pre-process to speed up the main PVS processing.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::initialPortalVis( PVSPortalArray & portals )
{
    // Allocate temporary visibility buffer
    cgByteArray portalVis( portals.size() );

    // Loop through the portal array allocating and checking 
    // portal visibility against every other portal
    for ( size_t p1 = 0; p1 < portals.size(); ++p1 ) 
    {
        PVSPortal & portal1 = portals[p1];
        const cgPlane plane1 = getPVSPortalPlane( portal1 );

        // Allocate memory for portal visibility info
        portal1.possibleVis.resize( mPVSBytesPerSet, 0 );
        
        // Clear temporary buffer
        memset( &portalVis[0], 0, portals.size() );
        
        // For this portal, loop through all other portals
        for ( size_t p2 = 0; p2 < portals.size(); ++p2 ) 
        {
            // Don't test against self
            if ( p2 == p1 )
                continue;

            // Test to see if any of p2's points are in front of p1's plane
            size_t i;
            PVSPortal & portal2 = portals[p2];
            const cgPlane plane2 = getPVSPortalPlane( portal2 );
            for ( i = 0; i < portal2.points->vertexCount; ++i ) 
            {
                if ( cgCollision::pointClassifyPlane( portal2.points->vertices[i], (cgVector3&)plane1, plane1.d, CGE_EPSILON_1MM ) == cgPlaneQuery::Front ) 
                    break;
            
            } // Next vertex

            // If the loop reached the end, there were no points in front so continue
            if ( i == portal2.points->vertexCount )
                continue;
            
            // Test to see if any of p1's portal points are behind p2's plane.
            for ( i = 0; i < portal1.points->vertexCount; ++i ) 
            {
                if ( cgCollision::pointClassifyPlane( portal1.points->vertices[i], (cgVector3&)plane2, plane2.d, CGE_EPSILON_1MM ) == cgPlaneQuery::Back )
                    break;
            
            } // Next Portal Vertex

            // If the loop reached the end, there were no points in front so continue
            if ( i == portal1.points->vertexCount )
                continue;

            // Fill out the temporary portal visibility array
            portalVis[p2] = 1;		

        } // Next Portal 2

        // Now flood through all the portals which are visible
        // from the source portal through into the neighbour leaf
        // and flag any leaves which are visible (the leaves which
        // remain set to 0 can never possibly be seen from this portal)
        portal1.possibleVisCount = 0;
        portalFlood( portals, portal1, &portalVis[0], portal1.leaf );

    } // Next portal
}

//-----------------------------------------------------------------------------
// Name : portalFlood() (Protected, Recursive)
/// <summary>
/// This function does a basic flood fill THROUGH all of the portals from this 
/// leaf  recursively and flags all leaves which the flood fill reached.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::portalFlood( PVSPortalArray & portals, PVSPortal & sourcePortal, cgByte * portalVis, cgUInt32 leafIndex )
{
    // Test the source portals 'Possible Visibility' list
    // to see if this leaf has already been set.
    if ( getPVSBit( &sourcePortal.possibleVis[0], leafIndex ) )
        return;

    // Set the possible visibility bit for this leaf
    setPVSBit( &sourcePortal.possibleVis[0], leafIndex );

    // Increase portal's 'complexity' level
    sourcePortal.possibleVisCount++;

    // Loop through all portals in this leaf (remember the portal numbering
    // in the leaves match up with the originals, not our PVS portals )
    cgBSPTreeLeaf & leaf = mLeaves[leafIndex];
    for ( size_t i = 0; i < leaf.portals.size(); ++i )	
    {
        // Find correct portal index (the one IN this leaf (not neighbouring)).
        cgUInt32 portalIndex = leaf.portals[ i ] * 2;
        if ( portals[portalIndex].leaf == leafIndex )
            ++portalIndex;

        // If this portal was not flagged as allowed to pass through, then continue to next portal
        if ( !portalVis[ portalIndex ] )
            continue;

        // Flood fill out through this portal
        portalFlood( portals, sourcePortal, portalVis, portals[portalIndex].leaf );
    
    } // Next leaf portal
}

//-----------------------------------------------------------------------------
// Name : clipPortal () (Protected, Recursive)
/// <summary>
/// This recursive function repeatedly clips the current portal to the tree 
/// until it ends up in a leaf at which point it is returned.
/// </summary>
//-----------------------------------------------------------------------------
cgBSPTree::Portal * cgBSPTree::clipPortal( cgUInt32 nodeIndex, Portal * portal, LeafOwner ownerNodeSide )
{
    const cgBSPTreeSubNode & node = mNodes[nodeIndex];
    const cgPlane & plane = mNodePlanes[node.plane];

    // Classify the portal against this node's plane
    cgPlaneQuery::Class location = cgPlaneQuery::On;
    if ( portal->plane != node.plane )
        location = cgCollision::polyClassifyPlane( &mInputVertices[portal->firstVertex], portal->vertexCount, sizeof(cgVector3), (cgVector3&)plane, plane.d, CGE_EPSILON_1MM );
    switch ( location )
    {
        case cgPlaneQuery::On:
        {
            // The portal has to be sent down both sides of the tree and tracked. 
            // Send it down front first but DO NOT delete any bits that end up in 
            // solid space, just ignore them.
            Portal * frontPortals = CG_NULL;
            if ( node.front & LeafNodeBit ) 
            {
			    // The front is a leaf.
                const cgUInt32 leafIndex = (node.front & LeafIndexMask);

                // Leaf is below the the portals' owner node?
                if ( ownerNodeSide != NoOwner )
                {
                    // This portal is used directly.
                    portal->leafOwner[ownerNodeSide] = leafIndex;
                    portal->next = CG_NULL;
                    frontPortals = portal;

                } // End if leaf found
                else 
                {
                    // Not in originator sub-tree
                    delete portal;
                    return CG_NULL;

                } // End if no leaf found

		    } // End if child leaf
            else 
            {
			    // Send the portal down the front of this node and 
                // retrieve a list of portal fragments that survived.
                LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == portal->node) ? FrontOwner : ownerNodeSide;
			    frontPortals = clipPortal( node.front, portal, childSide );

		    } // End if child node

		    // If nothing survived return here.
            if ( !frontPortals )
                return CG_NULL;

            // If the back is solid, just return the front list as it is.
            if ( node.back == SolidLeaf )
                return frontPortals;

		    // Loop through each front list fragment and send it down the back branch
            Portal * finalFragments = CG_NULL, * next = CG_NULL;
            for ( portal = frontPortals; portal; portal = next )
            {
                next = (Portal*)portal->next;
                
                // Empty leaf behind?
                Portal * backPortals = CG_NULL;
                if ( node.back & LeafNodeBit )
                {
                    // The back is a leaf. Determine which side of the node the portal fell.
                    const cgUInt32 leafIndex = (node.back & LeafIndexMask);

                    // Leaf is below the the portals' owner node?
                    if ( ownerNodeSide != NoOwner )
                    {
                        // This portal is used directly.
                        portal->leafOwner[ownerNodeSide] = leafIndex;
                        portal->next = CG_NULL;
                        backPortals  = portal;

                    } // End if leaf found
                    else 
                    {
                        // Not in originator sub-tree
                        delete portal;
                        continue;

                    } // End if no leaf found

                } // End if child leaf
                else 
                {
                    // Send the portal down the back of this node and 
                    // retrieve a list of portal fragments that survived.
                    LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == portal->node) ? BackOwner : ownerNodeSide;
			        backPortals = clipPortal( node.back, portal, childSide );

                } // End If child node

			    // Anything in the back list?
			    if ( backPortals ) 
                {
                    // Iterate to the end to get the last item in the back list
				    Portal * itPortal = backPortals;
                    while ( itPortal->next )
                        itPortal = (Portal*)itPortal->next;
					
                    // Attach the last fragment to the first fragment from the
                    // previous iteration.
                    itPortal->next = finalFragments;
                    finalFragments = backPortals;
                    
			    } // End if survived

		    } // Next portal

            // Return fragments that survived.
            return finalFragments;

        } // End case on plane
        case cgPlaneQuery::Front:
        {
            // Either send it down the front tree or add it to the portal 
			// list because it has come out in Empty Space
            if ( node.front & LeafNodeBit ) 
            {
			    // The front is a leaf.
                const cgUInt32 leafIndex = (node.front & LeafIndexMask);

                // Leaf is below the the portals' owner node?
                if ( ownerNodeSide != NoOwner )
                {
                    // This portal is used directly.
                    portal->leafOwner[ownerNodeSide] = leafIndex;
                    portal->next = CG_NULL;
                    return portal;

                } // End if leaf found
                else 
                {
                    // Not in originator sub-tree
                    delete portal;
                    return CG_NULL;

                } // End if no leaf found

		    } // End if child leaf
            else 
            {
			    // Send the portal down the front of this node and 
                // retrieve a list of portal fragments that survived.
                LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == portal->node) ? FrontOwner : ownerNodeSide;
			    return clipPortal( node.front, portal, childSide );

		    } // End if child node
            break;

        } // End case in front
        case cgPlaneQuery::Back:
        {
            // Either send it down the back tree, add it to the portal 
			// list because it has come out in empty space or destroy
            // it because it ended up in solid space.
            if ( node.back == SolidLeaf )
            {
                // Clip away fragments in solid space.
                delete portal;
                return CG_NULL;

            } // End if solid leaf
            else if ( node.back & LeafNodeBit ) 
            {
			    // The back is a leaf.
                const cgUInt32 leafIndex = (node.back & LeafIndexMask);

                // Leaf is below the the portals' owner node?
                if ( ownerNodeSide != NoOwner )
                {
                    // This portal is used directly.
                    portal->leafOwner[ownerNodeSide] = leafIndex;
                    portal->next = CG_NULL;
                    return portal;

                } // End if leaf found
                else 
                {
                    // Not in originator sub-tree
                    delete portal;
                    return CG_NULL;

                } // End if no leaf found

		    } // End if child leaf
            else 
            {
			    // Send the portal down the back of this node and 
                // retrieve a list of portal fragments that survived.
                LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == portal->node) ? BackOwner : ownerNodeSide;
			    return clipPortal( node.back, portal, childSide );

		    } // End if child node
            break;

        } // End case behind
        case cgPlaneQuery::Spanning:
        {
            // Split the portal against the plane.
            Portal * frontSplit = new Portal(), * backSplit = new Portal();
            splitWinding( portal, plane, frontSplit, backSplit );

            // Copy over additional duplicated data.
            frontSplit->node = portal->node;
            backSplit->node = portal->node;
            frontSplit->leafOwner[0] = portal->leafOwner[0];
            frontSplit->leafOwner[1] = portal->leafOwner[1];
            backSplit->leafOwner[0] = portal->leafOwner[0];
            backSplit->leafOwner[1] = portal->leafOwner[1];

            // We're done with the original portal fragment.
            delete portal;

            // There is another front node ?
            Portal * frontPortals = CG_NULL;
			if ( node.front & LeafNodeBit ) 
            {
                // The front is a leaf.
                const cgUInt32 leafIndex = (node.front & LeafIndexMask);

                // Leaf is below the the portals' owner node?
                if ( ownerNodeSide != NoOwner ) 
                {
                    frontSplit->leafOwner[ownerNodeSide] = leafIndex;
                    frontSplit->next = CG_NULL;
                    frontPortals = frontSplit;
                
                } // End if leaf found
                else 
                {
                    delete frontSplit;
                
                } // End if no leaf found
			
            } // End if child leaf
            else 
            {
                LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == frontSplit->node) ? FrontOwner : ownerNodeSide;
				frontPortals = clipPortal( node.front, frontSplit, childSide );
			
            } // End if child node

            // There is another back node?
            Portal * backPortals = CG_NULL;
            if ( node.back == SolidLeaf ) 
            {
                // We ended up in solid space
                delete backSplit;
                backSplit = CG_NULL;
            
            } // End if solid leaf
            else if ( node.back & LeafNodeBit ) 
            {
                // The back is a leaf.
                const cgUInt32 leafIndex = (node.back & LeafIndexMask);

                // Leaf is below the the portals' owner node?
                if ( ownerNodeSide != NoOwner ) 
                {
                    backSplit->leafOwner[ownerNodeSide] = leafIndex;
                    backSplit->next = CG_NULL;
                    backPortals = backSplit;

                } // End if leaf found
                else 
                {
                    // Not in originator sub-tree
                    delete backSplit;

                } // End if no leaf found

            } // End if child leaf
            else 
            {
                LeafOwner childSide = (ownerNodeSide == NoOwner && nodeIndex == backSplit->node) ? BackOwner : ownerNodeSide;
                backPortals = clipPortal( node.back, backSplit, childSide );

            } // End if child node

            // Find the end of the front list and attach it to back List
			if ( frontPortals ) 
            {
				// There is something in the front list
				Portal * itPortal = frontPortals;
                while ( itPortal->next )
                    itPortal = (Portal*)itPortal->next;
                if ( backPortals )
                    itPortal->next = backPortals;
                return frontPortals;
			
            } // End if front list
            else 
            {
				// There is nothing in the front list simply return the back list
                return backPortals;

			} // End if no front list

        } // End case spanning

    } // End switch classify

	return NULL;
}

//-----------------------------------------------------------------------------
// Name : generatePortal ( ) (Protected)
/// <summary>
/// Generate a new portal winding to represent the specified node that is
/// guaranteed to fully surround the specified bounding box (although may be
/// much larger).
/// </summary>
//-----------------------------------------------------------------------------
cgBSPTree::Portal * cgBSPTree::generatePortal( cgUInt32 nodeIndex, const cgBoundingBox & bounds )
{
    const cgBSPTreeSubNode & node = mNodes[nodeIndex];
    const cgPlane & plane = mNodePlanes[node.plane];

    // Create a new portal winding.
    Portal * portal = new Portal();
    portal->node         = nodeIndex;
    portal->firstVertex  = mInputVertices.size();
    portal->vertexCount  = 4;
    portal->plane        = node.plane;

    // Make space for the four required vertices.
    mInputVertices.resize( mInputVertices.size() + 4 );

	// Calculate bounding box centre point.
	const cgVector3 boundsCenter = bounds.getCenter();
	
    // Project center of bounding box onto the plane.
    const cgVector3 planeCenter = boundsCenter - ((cgVector3&)plane * cgPlane::dotCoord( plane, boundsCenter ));

	// Calculate major axis vector.
    cgInt axisIndex = 0;
    if( fabs(plane.b) > fabs(plane.c) )
        axisIndex = (fabs(plane.c) < fabs(plane.a)) ? 2 : 0;
	else
        axisIndex = (fabs(plane.b) <= fabs(plane.a)) ? 1 : 0;
	cgVector3 axis( 0, 0, 0 );
    axis[axisIndex] = 1;

	// Generate U and V vectors
    cgVector3 u, v;
    cgVector3::cross( u, axis, (cgVector3&)plane );
    cgVector3::cross( v, u, (cgVector3&)plane );
    cgVector3::normalize( u, u );
    cgVector3::normalize( v, v );
    
	// Scale the UV Vectors up by half the length.
    cgFloat length = cgVector3::length(bounds.getExtents());
	u *= length; v *= length;

    // Generate the final points.
    mInputVertices[ portal->firstVertex + 0 ] = planeCenter + u - v; // Bottom Right
    mInputVertices[ portal->firstVertex + 1 ] = planeCenter + u + v; // Top Right
	mInputVertices[ portal->firstVertex + 2 ] = planeCenter - u + v; // Top Left
	mInputVertices[ portal->firstVertex + 3 ] = planeCenter - u - v; // Bottom Left
    
    // Success!
    return portal;
}

//-----------------------------------------------------------------------------
// Name : mergeMeshes ( ) (Protected)
/// <summary>
/// Construct the merged mesh data that will be used to compile the BSP tree
/// based on the supplied geometry.
/// </summary>
//-----------------------------------------------------------------------------
bool cgBSPTree::mergeMeshes( cgUInt32 meshCount, const cgMeshHandle meshes[], const cgTransform transforms[] )
{
    // Iterate the supplied list of meshes and count the number
    // of required vertices / triangles first of all. Note that
    // the triangle vertices are physically extracted / duplicated
    // where necessary -- no vertex will be shared and thus index 
    // data is not required.
    size_t totalTris = 0, totalVertices = 0;
    for ( size_t i = 0; i < meshCount; ++i )
    {
        cgMeshHandle meshHandle = meshes[i];
        cgMesh * mesh = meshHandle.getResource( true ) ;
        if ( !mesh || !mesh->isLoaded() )
            continue;

        // Sum components.
        totalTris += mesh->getFaceCount();
        totalVertices += mesh->getFaceCount() * 3;

    } // Next mesh

    // Allocate enough room for the required data.
    mInputVertices.resize( totalVertices );
    
    // Extract vertex and triangle data from each mesh.
    totalTris = 0;
    totalVertices = 0;
    for ( size_t i = 0; i < meshCount; ++i )
    {
        cgMeshHandle meshHandle = meshes[i];
        cgMesh * mesh = meshHandle.getResource( true ) ;
        if ( !mesh || !mesh->isLoaded() )
            continue;

        // Add windings for this data set.
        size_t numTris = mesh->getFaceCount();
        for ( size_t j = 0; j < numTris; ++j )
        {
            Winding * winding = new Winding();
            winding->firstVertex    = totalVertices + (j * 3);
            winding->vertexCount    = 3;
            winding->used           = false;
            winding->plane          = cgUInt32(-1);
            winding->next           = mInputWindings;
            mInputWindings = winding;

        } // Next triangle

        // Extract vertex data.
        size_t numIndices = numTris * 3;
        cgVertexFormat * format = mesh->getVertexFormat();
        size_t stride = format->getStride();
        cgUInt32 * indexData = mesh->getSystemIB();
        cgByte * vertexData = mesh->getSystemVB() + format->getElementOffset( D3DDECLUSAGE_POSITION );
        for ( size_t j = 0; j < numIndices; ++j )
            transforms[i].transformCoord( mInputVertices[totalVertices++], *((cgVector3*)(vertexData + stride * indexData[j])) );

    } // Next mesh

    // Generate bounding box for entire tree from point data.
    if ( !mInputVertices.empty() )
        mBounds.fromPoints( (cgByte*)&mInputVertices[0], totalVertices, sizeof(cgVector3) );
    
    // Success?
    return !mInputVertices.empty();
}

//-----------------------------------------------------------------------------
// Name : buildPlaneSet ( ) (Protected)
/// <summary>
/// Build the set of planes that define the boundaries of the leaves to be
/// constructed, based on the faces of the geometry data supplied. Where 
/// possible, planes will be merged in order to improve the reliability of
/// the BSP compilation process.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::buildPlaneSet( )
{
    // Process each imported triangle (windings are all guaranteed to be
    // 3 vertices each at this point).
    for ( Winding * winding = mInputWindings; winding; winding = winding->next )
    {
        // Get the three vertices.
        cgVector3 & v0 = mInputVertices[winding->firstVertex];
        cgVector3 & v1 = mInputVertices[winding->firstVertex+1];
        cgVector3 & v2 = mInputVertices[winding->firstVertex+2];

        // Calculate the center point of the triangle. This should provide us
        // with a more reliable representation of a point that sits on the
        // triangle's plane, with error distributed on opposing sides of the center.
        cgVector3 center = v0;
        center += v1;
        center += v2;
        center /= 3.0f;

        // Compute the triangle normal.
        cgVector3 normal;
        cgVector3::cross( normal, (v1-v0), (v2-v0) );
        cgVector3::normalize( normal, normal );

        // Calculate triangle's final plane.
        cgPlane plane;
        cgPlane::fromPointNormal( plane, center, normal );

        // Search through the existing plane list to see if one 
        // already exists (must use small tolerances).
        const size_t numPlanes = mNodePlanes.size();
        for ( size_t j = 0; j < numPlanes; ++j )
        {
            const cgPlane & testPlane = mNodePlanes[j];
            if ( fabsf(testPlane.a - plane.a) < CGE_EPSILON &&
                 fabsf(testPlane.b - plane.b) < CGE_EPSILON &&
                 fabsf(testPlane.c - plane.c) < CGE_EPSILON &&
                 cgMathUtility::dynamicEpsilonTest( testPlane.d, plane.d, 10 )  )
                 /*fabsf(testPlane.d - plane.d) < CGE_EPSILON )*/
            {
                // Just use this plane
                winding->plane = j;
                plane = testPlane;
                break;

            } // End if match
                 
        } // Next plane

        // If no matching plane was found, add one.
        if ( winding->plane == cgUInt32(-1) )
        {
            winding->plane = numPlanes;
            mNodePlanes.push_back( plane );
        
        } // End if add plane

        // Ensure that all vertices are on the selected plane
        cgFloat offset = cgPlane::dotCoord( plane, v0 );
        v0 -= (cgVector3&)plane * offset;
        offset = cgPlane::dotCoord( plane, v1 );
        v1 -= (cgVector3&)plane * offset;
        offset = cgPlane::dotCoord( plane, v2 );
        v2 -= (cgVector3&)plane * offset;

    } // Next triangle
}

//-----------------------------------------------------------------------------
// Name : buildTree () (Protected, Recursive)
/// <summary>
/// Performs the recursive build behavior for this tree type.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::buildTree( cgUInt32 level, cgUInt32 node, Winding * windingList )
{
    Winding * backList = CG_NULL, * frontList = CG_NULL;

    // Select the best splitter from the list of windings supplied
    Winding * splitter = selectBestSplitter( windingList, mSplitterSample, mSplitHeuristic );
    splitter->used = true;

    // Local scope
    {
        const cgPlane & splitterPlane = mNodePlanes[splitter->plane];

        // This node splits against the chosen splitter.
        mNodes[node].plane = splitter->plane;

        // Classify each winding.
        Winding * next = CG_NULL;
        for ( Winding * winding = windingList; winding; winding = next )
        {
            const cgPlane & windingPlane = mNodePlanes[winding->plane];

            // Store the next winding pointer, this may be replaced.
            next = winding->next;

            // If the plane index of this winding matches that of
            // the splitter, we immediately know that it is on-plane.
            // Otherwise, we must test.
            cgPlaneQuery::Class result = cgPlaneQuery::On;
            if ( winding->plane != splitter->plane )
                result = cgCollision::polyClassifyPlane( &mInputVertices[winding->firstVertex], winding->vertexCount, sizeof(cgVector3), (cgVector3&)splitterPlane, splitterPlane.d, CGE_EPSILON_1MM );

            // Process the winding based on its classification.
            switch ( result )
            {
                case cgPlaneQuery::On:
                {
                    // Planes face the same way?
                    if ( winding->plane == splitter->plane || 
                         cgVector3::dot( (cgVector3&)windingPlane, (cgVector3&)splitterPlane ) > 0 )
                    {
                        // It sits on the splitter's plane and thus it should not be 
                        // used as a splitter either.
                        winding->used = true;

                        // Attach to the front list.
                        winding->next = frontList;
                        frontList = winding;

                    } // End if facing same way
                    else
                    {
                        // Planes are facing in opposing directions so attach to the back list.
                        winding->next = backList;
                        backList = winding;

                    } // End if !facing same way
                    break;
                
                } // End case on plane
                case cgPlaneQuery::Front:
                    // Entirely in front, pass down the front side.
                    winding->next = frontList;
                    frontList = winding;
                    break;

                case cgPlaneQuery::Back:
                    // Entirely behind, pass down the back side.
                    winding->next = backList;
                    backList = winding;
                    break;

                case cgPlaneQuery::Spanning:
                {
                    // Split the winding in two.
                    Winding * frontSplit = new Winding(), * backSplit = new Winding();
                    splitWinding( winding, splitterPlane, frontSplit, backSplit );

                    // Process new front fragment.
                    if ( frontSplit->vertexCount )
                    {
                        // Ensure that all vertices of the new front fragment are on the original plane.
                        for ( size_t j = frontSplit->firstVertex; j < frontSplit->firstVertex + frontSplit->vertexCount; ++j )
                        {
                            cgFloat offset = cgPlane::dotCoord( windingPlane, mInputVertices[j] );
                            mInputVertices[j] -= (cgVector3&)windingPlane * offset;

                        } // Next vertex

                        // Add to the list for the new front node.
                        frontSplit->next = frontList;
                        frontList = frontSplit;

                    } // End if has front split
                    else
                        delete frontSplit;

                    // Process new back fragment
                    if ( backSplit->vertexCount )
                    {
                        // Ensure that all vertices of the new back fragment are on the original plane.
                        for ( size_t j = backSplit->firstVertex; j < backSplit->firstVertex + backSplit->vertexCount; ++j )
                        {
                            cgFloat offset = cgPlane::dotCoord( windingPlane, mInputVertices[j] );
                            mInputVertices[j] -= (cgVector3&)windingPlane * offset;

                        } // Next vertex

                        // Add to the list for the new back node.
                        backSplit->next = backList;
                        backList = backSplit;

                    } // End if has back split
                    else
                        delete backSplit;

                    // The original winding is now removed from consideration.
                    delete winding;

                    break;
                
                } // End case spanning

            } // End switch result

        } // Next winding
    
    } // End scope

    // If there are no splitters remaining in the front list,
    // add a leaf here. Otherwise, step into a new front node.
    if ( !countSplitters( frontList ) )
    {
        mNodes[node].front = (mLeaves.size() & LeafIndexMask) | LeafNodeBit;
        mLeaves.push_back( cgBSPTreeLeaf() );

        /*Winding * winding = frontList;
        while ( winding )
        {
            Portal portal;
            portal.vertexCount = winding->vertexCount;
            portal.firstVertex = mPortalVertices.size();
            portal.plane = winding->plane;
            mPortals.push_back( portal );
            for ( cgUInt32 i = winding->firstVertex; i < winding->firstVertex + winding->vertexCount; ++i )
                mPortalVertices.push_back( mInputVertices[i] );
            winding = winding->next;
        
        } // Next winding*/

        // Front list has been processed.
        releaseWindings( frontList );

    } // End if no splitters
    else
    {
        mNodes[node].front = (mNodes.size() & LeafIndexMask);
        mNodes.push_back( cgBSPTreeSubNode() );
        buildTree( level + 1, mNodes[node].front, frontList );
        
    } // End if has splitters

    // If there are no splitters remaining in the back list, 
    // mark the back as solid. Otherwise, step into a new back node.
    if ( !countSplitters( backList ) )
    {
        mNodes[node].back = SolidLeaf;

        /*Winding * winding = backList;
        while ( winding )
        {
            Portal portal;
            portal.vertexCount = winding->vertexCount;
            portal.firstVertex = mPortalVertices.size();
            portal.plane = winding->plane;
            mPortals.push_back( portal );
            for ( cgUInt32 i = winding->firstVertex; i < winding->firstVertex + winding->vertexCount; ++i )
                mPortalVertices.push_back( mInputVertices[i] );
            winding = winding->next;
        
        } // Next winding*/

        // Back list has been processed.
        releaseWindings( backList );

    } // End if no splitters
    else
    {
        mNodes[node].back = (mNodes.size() & LeafIndexMask);
        mNodes.push_back( cgBSPTreeSubNode() );
        buildTree( level + 1, mNodes[node].back, backList );
    
    } // End if has splitters
}

//-----------------------------------------------------------------------------
// Name : selectBestSplitter () (Protected)
/// <summary>
/// Picks the next winding in the list to be used as the splitting plane.
/// Note : You can pass a value to splitHeuristic; the higher the value
/// the higher the importance is put on reducing splits.
/// </summary>
//-----------------------------------------------------------------------------   
cgBSPTree::Winding * cgBSPTree::selectBestSplitter( Winding * windingList, cgUInt32 splitterSample, cgFloat splitHeuristic )
{
	cgUInt32 bestScore = 10000000, splitterCount = 0;
    Winding * selectedWinding = NULL;
	
	// Traverse the winding list
	for ( Winding * splitter = windingList; splitter; splitter = splitter->next )
    {
        // If this has NOT been used as a splitter yet, then consider it.
		if ( !splitter->used ) 
        {
            const cgPlane & splitterPlane = mNodePlanes[splitter->plane];
            
            // Test against the other windings and count the score
			cgUInt32 splits = 0, backFaces = 0, frontFaces = 0;
            for ( Winding * winding = windingList; winding; winding = winding->next )
            {
                cgPlaneQuery::Class result = cgCollision::polyClassifyPlane( mInputVertices[winding->firstVertex], winding->vertexCount, sizeof(cgVector3), (cgVector3&)splitterPlane, splitterPlane.d, CGE_EPSILON_1MM );
				switch ( result ) 
                {
                    case cgPlaneQuery::Front:
				        ++frontFaces;
					    break;

                    case cgPlaneQuery::Back:
					    ++backFaces;
					    break;

                    case cgPlaneQuery::Spanning:
					    ++splits;
					    break;

				} // End classification

            } // Next winding
			
            // Tally the score (modify the splits * n )
			cgUInt32 score = (cgUInt32)((cgInt32)abs( (cgInt32)(frontFaces - backFaces) ) + (splits * splitHeuristic));

			// Is this the best score ?
			if ( score < bestScore)  
            {
				bestScore = score;
				selectedWinding = splitter;

            } // End if better score

            splitterCount++;

		} // End if !used
        
        // Break if we reached our splitter sample limit.
        if ( splitterSample && splitterCount >= splitterSample && selectedWinding )
            break;

    } // Next Splitter

    // This will be NULL if no faces remained to be used
    return selectedWinding;
}

//-----------------------------------------------------------------------------
// Name : splitWinding() (Protected)
/// <summary>
/// Given a winding that is guaranteed to span the specified plane, split it 
/// into two pieces along the plane intersection.
/// </summary>
//-----------------------------------------------------------------------------
void cgBSPTree::splitWinding( Winding * winding, const cgPlane & plane, Winding * frontSplit, Winding * backSplit )
{
    static cgArray<cgPlaneQuery::Class> pointLocation;
    if ( pointLocation.size() < winding->vertexCount )
    {
        pointLocation.clear();
        pointLocation.resize( winding->vertexCount );
    
    } // End if buffer too small

    // Determine each point's location relative to the plane.
    size_t inFront = 0, behind = 0, onPlane = 0;
	for ( size_t i = 0; i < winding->vertexCount; ++i )
    {
        // Classify the vertex.
        switch ( pointLocation[i] = cgCollision::pointClassifyPlane( mInputVertices[winding->firstVertex + i], (cgVector3&)plane, plane.d, CGE_EPSILON_1MM ) )
        {
            case cgPlaneQuery::Front:
                ++inFront;
                break;
            
            case cgPlaneQuery::Back:
                ++behind;
                break;
            
            default:
                ++onPlane;
                break;
        
        } // End switch location

	} // Next Vertex

    // Allocate space for split winding fragments
    static PointArray frontVerts, backVerts;
    if ( frontVerts.size() < (winding->vertexCount + 1) )
    {
        frontVerts.clear();
        frontVerts.resize( winding->vertexCount + 1 );
    
    } // End if too small
    if ( backVerts.size() < (winding->vertexCount + 1) )
    {
        backVerts.clear();
        backVerts.resize( winding->vertexCount + 1 );
    
    } // End if too small

    // Split the winding.
    size_t frontCount = 0, backCount = 0, next = 0;
    for ( size_t i = 0; i < winding->vertexCount; ++i ) 
    {
        next = (i+1) % winding->vertexCount;

        // Place / duplicate vertex into relevant lists.
        if ( pointLocation[i] == cgPlaneQuery::On )
        {
            frontVerts[frontCount++] = mInputVertices[winding->firstVertex + i];
            backVerts[backCount++] = mInputVertices[winding->firstVertex + i];
            continue;
        
        } // End if on plane
        else
        {
            if ( pointLocation[i] == cgPlaneQuery::Front )
                frontVerts[frontCount++] = mInputVertices[winding->firstVertex + i];
            else
                backVerts[backCount++] = mInputVertices[winding->firstVertex + i];

        } // End if front / back
		
        // If the next vertex is not causing us to span the plane then continue
        if ( pointLocation[next] == cgPlaneQuery::On || pointLocation[next] == pointLocation[i])
            continue;
        
		// Calculate the intersection point
        cgFloat t;
        const cgVector3 & origin = mInputVertices[winding->firstVertex + i];
        const cgVector3 vel( mInputVertices[winding->firstVertex + next] - origin );
        cgCollision::rayIntersectPlane( origin, vel, plane, t, true, false );
        const cgVector3 newPos = origin + vel * t;

        // Store new vertex in both front and back lists.
        backVerts[backCount++] = newPos;
        frontVerts[frontCount++] = newPos;

    } // Next Vertex

    // Build the new front fragment winding.
    frontSplit->plane       = winding->plane;
    frontSplit->firstVertex = mInputVertices.size();
    frontSplit->vertexCount = frontCount;
    frontSplit->used        = winding->used;

    // Add front split vertices.
    mInputVertices.insert( mInputVertices.end(), frontVerts.begin(), frontVerts.begin() + frontCount );

    // Build the new back fragment winding.
    backSplit->plane        = winding->plane;
    backSplit->firstVertex  = mInputVertices.size();
    backSplit->vertexCount  = backCount;
    backSplit->used         = winding->used;

    // Add front split vertices.
    mInputVertices.insert( mInputVertices.end(), backVerts.begin(), backVerts.begin() + backCount );
}