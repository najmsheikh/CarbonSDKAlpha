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
// Name : cgVisibilitySet.cpp                                                //
//                                                                           //
// Desc : Classes responsible for collecting, storing and managing the       //
//        visibility information for parts of the scene including            //
//        objects and spatial tree leaves.                                   //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgVisibilitySet Module Includes
//-----------------------------------------------------------------------------
#include <World/cgVisibilitySet.h>
#include <World/Objects/cgSpatialTreeObject.h>
#include <World/Objects/cgLightObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>   // ToDo: Only if query support stays in
#include <System/cgTimer.h>

//-----------------------------------------------------------------------------
// Static Member Variable Definitions
//-----------------------------------------------------------------------------
cgUInt32 cgVisibilitySet::mNextResultId = 1;
cgUInt32 cgVisibilitySet::mAppliedSet   = 0;

///////////////////////////////////////////////////////////////////////////////
// cgVisibilitySet Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgVisibilitySet () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::cgVisibilitySet( )
{
    // Initialize variables to sensible defaults.
    mResultId             = 0;
    mScene                = CG_NULL;
    mLightOcclusion       = false;
    mLastComputedFrame    = 0;
    mLastFlags            = 0;
    mLastLightOcclusion   = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgVisibilitySet () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::~cgVisibilitySet( )
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
void cgVisibilitySet::dispose( bool bDisposeBase )
{
    // Clear the set.
    clear();
}

//-----------------------------------------------------------------------------
//  Name : compute ()
/// <summary>
/// Compute the visibility for all scene objects and scenery based on the
/// frustum specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::compute( cgScene * pScene, const cgFrustum & Frustum, cgUInt32 nFlags /* = 0 */, bool bAutoApply /* = false */ )
{
    // Is it really necessary for us to recompute visibility in this frame?
    cgTimer * pTimer = cgTimer::getInstance();
    if ( mLastComputedFrame == pTimer->getFrameCounter() && mLastFlags == nFlags &&
         mLastLightOcclusion == mLightOcclusion && mLastFrustum == Frustum )
         return;
    
    // Clear the current visibility information
    clear();

    // Retrieve required system objects
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();
    mScene = pScene;

    // Store the frustum used for visibility computation so that other
    // parts of the system can query for the visibility volume later.
    mFrustum = Frustum;

    // Ask the scene to register all visible objects based on its broadphase data.
    pScene->computeVisibility( Frustum, this, nFlags );
   
    // Record information about this computation in order to prevent
    // it from being accidentally recomputed a second time in any given frame.
    mLastComputedFrame    = pTimer->getFrameCounter();
    mLastFlags            = nFlags;
    mLastLightOcclusion   = mLightOcclusion;
    mLastFrustum           = mFrustum;

    // Apply visibility if requested
    if ( bAutoApply )
        apply();
}

//-----------------------------------------------------------------------------
//  Name : compute ()
/// <summary>
/// Compute the visibility for all objects and scenery based on the
/// bounding box specified.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::compute( cgScene * pScene, const cgBoundingBox & AABB, cgUInt32 nFlags /* = 0 */, bool bAutoApply /* = false */ )
{
    // Is it really necessary for us to recompute visibility in this frame?
    cgFrustum NewFrustum( AABB );
    cgTimer * pTimer = cgTimer::getInstance();
    if ( mLastComputedFrame == pTimer->getFrameCounter() && mLastFlags == nFlags &&
         mLastLightOcclusion == mLightOcclusion && mLastFrustum == NewFrustum )
         return;

     // Clear the current visibility information
    clear();

    // Retrieve required system objects
    cgRenderDriver * pDriver = cgRenderDriver::getInstance();
    mScene = pScene;

    // Store the frustum used for visibility computation so that other
    // parts of the system can query for the visibility volume later.
    mFrustum = NewFrustum;

    // ToDo: 9999 - This AABB version of compute visibility is a bit of a pain,
    // especially for those systems that can't easily detect based on a simple 
    // bounding box (for instance, landscape occlusion).
    // Can we just use the above frustum instead and boil this down to a single version?

    // Ask the scene to register all visible objects based on its broadphase data.
    pScene->computeVisibility( AABB, this, nFlags );

    // Record information about this computation in order to prevent
    // it from being accidentally recomputed a second time in any given frame.
    mLastComputedFrame    = pTimer->getFrameCounter();
    mLastFlags            = nFlags;
    mLastLightOcclusion   = mLightOcclusion;
    mLastFrustum           = mFrustum;

    // Apply visibility if requested
    if ( bAutoApply )
        apply();
}

//-----------------------------------------------------------------------------
//  Name : intersect( ) 
/// <summary>
/// Intersects the input visibility set with the current visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::intersect( cgVisibilitySet * pOperator, cgVisibilitySet * pOutput )
{
    /*// Intersect objects
    {
        cgObjectNodeSet::iterator itObject;
        cgObjectNodeSet & OperatorTable = pOperator->mObjectNodes;
        cgObjectNodeSet & QueryTable    = (OperatorTable.size() > mObjectNodes.size()) ? OperatorTable : mObjectNodes;
        cgObjectNodeSet & LoopTable     = (OperatorTable.size() > mObjectNodes.size()) ? mObjectNodes : OperatorTable;

        // Perform intersection
        for ( itObject = LoopTable.begin(); itObject != LoopTable.end(); ++itObject )
		{
			if ( QueryTable.find( *itObject ) != QueryTable.end() ) 
				pOutput->mObjectNodes.insert( *itObject );
		
        } // Next Object
	
    } // End intersect objects

	// Intersect lights
    {
        cgObjectNodeSet::iterator itObject;
        cgObjectNodeSet & OperatorTable = pOperator->mLights;
        cgObjectNodeSet & QueryTable    = (OperatorTable.size() > mLights.size()) ? OperatorTable : mLights;
        cgObjectNodeSet & LoopTable     = (OperatorTable.size() > mLights.size()) ? mLights : OperatorTable;

        // Perform intersection
        for ( itObject = LoopTable.begin(); itObject != LoopTable.end(); ++itObject )
		{
			if ( QueryTable.find( *itObject ) != QueryTable.end() ) 
				pOutput->mLights.insert( *itObject );
		
        } // Next Light
	
    } // End intersect lights

    // Intersect materials
    {
        VisibleMaterialMap::iterator itMaterial;
        RenderClassMap::iterator     itClass;
        cgObjectNodeSet::iterator    itObject;
        VisibleMaterialMap & OperatorMaterialTable = pOperator->m_Materials;
        VisibleMaterialMap & QueryMaterialTable    = (OperatorMaterialTable.size() > m_Materials.size()) ? OperatorMaterialTable : m_Materials;
        VisibleMaterialMap & LoopMaterialTable     = (OperatorMaterialTable.size() > m_Materials.size()) ? m_Materials : OperatorMaterialTable;

        // Perform intersection of top level material groups
        for ( itMaterial = LoopMaterialTable.begin(); itMaterial != LoopMaterialTable.end(); ++itMaterial )
        {
            const cgMaterialHandle & hMaterial  = itMaterial->first;
            RenderClassMap         & ClassTable = itMaterial->second.RenderClasses;

            // Does a list of render classes for this material exist in the query set?
            VisibleMaterialMap::iterator itQueryRenderClassMap = QueryMaterialTable.find( hMaterial );
            if ( itQueryRenderClassMap != QueryMaterialTable.end() )
            {
                // Since the material exists in both sets, we must now test the individual render classes.
                RenderClassMap & OuterClassTable = itQueryRenderClassMap->second.RenderClasses;
                RenderClassMap & QueryClassTable = (ClassTable.size() > OuterClassTable.size()) ? ClassTable : OuterClassTable;
                RenderClassMap & LoopClassTable  = (ClassTable.size() > OuterClassTable.size()) ? OuterClassTable : ClassTable;

                // Perform intersection
                for ( itClass = LoopClassTable.begin(); itClass != LoopClassTable.end(); ++itClass )
                {
                    cgUInt32          nClassId    = itClass->first;
                    cgObjectNodeSet & ObjectTable = itClass->second;

                    // Does a list of objects for this material exist in the query set?
                    RenderClassMap::iterator itQueryObjectList = QueryClassTable.find( nClassId );
                    if ( itQueryObjectList != QueryClassTable.end() )
                    {
                        // Since the material exists in both sets, we must now test the individual objects.
                        cgObjectNodeSet & QueryTable = (ObjectTable.size() > itQueryObjectList->second.size()) ? ObjectTable : itQueryObjectList->second;
                        cgObjectNodeSet & LoopTable  = (ObjectTable.size() > itQueryObjectList->second.size()) ? itQueryObjectList->second : ObjectTable;

                        // Perform intersection
                        for ( itObject = LoopTable.begin(); itObject != LoopTable.end(); ++itObject )
                        {
                            if ( QueryTable.find( *itObject ) != QueryTable.end() ) 
				                pOutput->m_Materials[hMaterial].RenderClasses[nClassId].insert( *itObject );

                        } // Next Leaf

                    } // End if exists

                } // Next Render Class

            } // End if exists

        } // Next Material

    } // End intersect materials

    // Intersect leaves
	{
        TreeLeafMap::iterator    itLeafList;
        cgSceneLeafSet::iterator itLeaf;
        TreeLeafMap & OperatorTreeTable = pOperator->mTreeLeaves;
        TreeLeafMap & QueryTreeTable    = (OperatorTreeTable.size() > mTreeLeaves.size()) ? OperatorTreeTable : mTreeLeaves;
        TreeLeafMap & LoopTreeTable     = (OperatorTreeTable.size() > mTreeLeaves.size()) ? mTreeLeaves : OperatorTreeTable;

        // Perform intersection of top level spatial tree groups
        for ( itLeafList = LoopTreeTable.begin(); itLeafList != LoopTreeTable.end(); ++itLeafList )
        {
            cgSpatialTreeNode * pTree  = (cgSpatialTreeNode*)itLeafList->first;
            cgSceneLeafSet & LeafTable = itLeafList->second;

            // Does a list of leaves for this spatial tree exist in the query set?
            TreeLeafMap::iterator itQueryLeafList = QueryTreeTable.find( pTree );
            if ( itQueryLeafList != QueryTreeTable.end() )
            {
                // Since the tree exists in both sets, we must now test the individual leaves.
                cgSceneLeafSet & QueryTable    = (LeafTable.size() > itQueryLeafList->second.size()) ? LeafTable : itQueryLeafList->second;
                cgSceneLeafSet & LoopTable     = (LeafTable.size() > itQueryLeafList->second.size()) ? itQueryLeafList->second : LeafTable;

                // Perform intersection
                for ( itLeaf = LoopTable.begin(); itLeaf != LoopTable.end(); ++itLeaf )
                {
                    if ( QueryTable.find( *itLeaf ) != QueryTable.end() ) 
				        (pOutput->mTreeLeaves[pTree]).insert( *itLeaf );

                } // Next Leaf

            } // End if exists

        } // Next Leaf Group

	}

    // Intersect data groups
	{
        AssociatedGroupMap::iterator itGroupList;
        cgInt32Set::iterator         itGroup;
        AssociatedGroupMap & OperatorGroupTable = pOperator->mAssociatedGroups;
        AssociatedGroupMap & QueryGroupTable    = (OperatorGroupTable.size() > mAssociatedGroups.size()) ? OperatorGroupTable : mAssociatedGroups;
        AssociatedGroupMap & LoopGroupTable     = (OperatorGroupTable.size() > mAssociatedGroups.size()) ? mAssociatedGroups : OperatorGroupTable;

        // Perform intersection of top level contexts
        for ( itGroupList = LoopGroupTable.begin(); itGroupList != LoopGroupTable.end(); ++itGroupList )
        {
            void * pContext = itGroupList->first;
            cgInt32Set & GroupTable = itGroupList->second;

            // Does a list of data group identifiers for this same context exist in the query set?
            AssociatedGroupMap::iterator itQueryGroupList = QueryGroupTable.find( pContext );
            if ( itQueryGroupList != QueryGroupTable.end() )
            {
                // Since the context exists in both sets, we must now test the individual groups.
                cgInt32Set & QueryTable = (GroupTable.size() > itQueryGroupList->second.size()) ? GroupTable : itQueryGroupList->second;
                cgInt32Set & LoopTable  = (GroupTable.size() > itQueryGroupList->second.size()) ? itQueryGroupList->second : GroupTable;

                // Perform intersection
                for ( itGroup = LoopTable.begin(); itGroup != LoopTable.end(); ++itGroup )
                {
                    if ( QueryTable.find( *itGroup ) != QueryTable.end() ) 
				        (pOutput->mAssociatedGroups[pContext]).insert( *itGroup );

                } // Next Group

            } // End if exists

        } // Next Group Context

	}

    // Select the next unique visibility result identifier
    pOutput->mResultId = mNextResultId++;

    // If the result identifier variable overflowed and wrapped around,
    // ensure that the reserved code '0' is skipped.
    if ( mNextResultId == 0 )
        mNextResultId = 1;

    // Visibility volume in this case will be equal to the original.
    pOutput->mFrustum = mFrustum;*/

}

//-----------------------------------------------------------------------------
//  Name : intersect( ) 
/// <summary>
/// Intersects the input frustum with the current visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::intersect( const cgFrustum & Operator, cgVisibilitySet * pOutput )
{
    cgObjectNodeSet::iterator itObject;
    //cgObjectNode      * pObject;
    cgSpatialTreeNode * pTree;
    cgBoundingBox       AABB;

    // ToDo: 9999 - WARNING.. We can't intersect data groups.
    
    // Intersect leaves
    for ( TreeLeafMap::iterator itLeafList = mTreeLeaves.begin(); itLeafList != mTreeLeaves.end(); ++itLeafList )
    {
        pTree = (cgSpatialTreeNode*)itLeafList->first;
        cgSceneLeafSet & LeafTable = itLeafList->second;

        // Add leaves only if the tree itself is visible
        if ( Operator.testAABB( pTree->getBoundingBox( ) ) == true )
        {
            // Perform same test on the individual leaves.
            for ( cgSceneLeafSet::iterator itLeaf = LeafTable.begin(); itLeaf != LeafTable.end(); ++itLeaf )
            {
                // If intersects frustum, add to output set.
                if ( Operator.testAABB( (*itLeaf)->getBoundingBox() ) == true )
                    (pOutput->mTreeLeaves[pTree]).insert( *itLeaf );

            } // Next Leaf

        } // End if tree visible

    } // Next Tree

    cgToDo( "Visibility", "Intersect render class list?" )
    /*// Intersect materials
    for ( VisibleMaterialMap::iterator itObjectList = m_Materials.begin(); itObjectList != m_Materials.end(); ++itObjectList )
    {
        const cgMaterialHandle & hMaterial   = itObjectList->first;
        RenderClassMap         & ClassTable  = itObjectList->second.RenderClasses;

        // Intersect render classes.
        for ( RenderClassMap::iterator itClass = ClassTable.begin(); itClass != ClassTable.end(); ++itClass )
        {
            cgObjectNodeSet & ObjectTable = itClass->second;

            // ToDo: Potentially we could record the overall bounding box of the objects
            // that use the specified material and reject them all in one go (similar
            // to the spatial tree root AABB test above).

            // Test the individual objects for intersection against the frustum
            for ( itObject = ObjectTable.begin(); itObject != ObjectTable.end(); ++itObject )
            {
                // If intersects frustum, add to output set.
                if ( Operator.testAABB( (*itObject)->getBoundingBox() ) == true )
                    pOutput->m_Materials[hMaterial].RenderClasses[itClass->first].insert( *itObject );

            } // Next Leaf

        } // Next render class

    } // Next Tree*/
	
	/*// Intersect objects
    for( size_t i = 0; i < mObjectNodes.size(); ++i )
	{
        // Object intersects input frustum?
        pObject = mObjectNodes[i];
        if ( Operator.testAABB( pObject->getBoundingBox() ) )
        {
            pOutput->mObjectNodeLUT[ pObject ] = pOutput->mObjectNodes.size();
            pOutput->mObjectNodes.push_back( pObject );
        
        } // End if intersects
		
    } // Next Object
	
	// Intersect lights
    for( itObject = mLights.begin(); itObject != mLights.end(); ++itObject )
	{
        if ( (pObject = (*itObject)) == CG_NULL )
            continue;

        // Object intersects input frustum?
        if ( Operator.testAABB( pObject->getBoundingBox() ) == true )
            pOutput->mLights.insert( pObject );
		
    } // Next Light*/

    // Select the next unique visibility result identifier
    pOutput->mResultId = mNextResultId++;
    
    // If the result identifier variable overflowed and wrapped around,
    // ensure that the reserved code '0' is skipped.
    if ( mNextResultId == 0 )
        mNextResultId = 1;

    // Visibility volume in this case will be equal to the specified frustum.
    pOutput->mFrustum = Operator;
}

//-----------------------------------------------------------------------------
//  Name : intersect( ) 
/// <summary>
/// Intersects the input frustum with the current visibility set.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::intersect( const cgFrustum & Operator, const cgVector3 & vSweepDir, cgVisibilitySet * pOutput, cgBoundingBox & Bounds )
{
    cgObjectNode      * pObject;
    cgSpatialTreeNode * pTree;
    cgBoundingBox       AABB;
    cgVector3           vCenter;
    cgFloat             fRadius;
    
    // Reset output bounding box just to be polite
    Bounds.reset();

    // ToDo: Intersect materials?
    // ToDo: 9999 - WARNING.. We can't intersect data groups.

    // Intersect leaves
    for ( TreeLeafMap::iterator itLeafList = mTreeLeaves.begin(); itLeafList != mTreeLeaves.end(); ++itLeafList )
    {
        pTree = (cgSpatialTreeNode*)itLeafList->first;
        cgSceneLeafSet & LeafTable = itLeafList->second;

        // Add leaves only if the tree itself is visible
        AABB = pTree->getBoundingBox( );
        
        // First test to see if the tree as a whole any possibility of being "swept" 
        // into the frustum (i.e. perhaps it might cast a shadow onto something within
        // the frustum). We use a swept sphere for this.
        vCenter = AABB.getCenter();
        fRadius = cgVector3::length( AABB.getExtents() );
        if ( Operator.testSweptSphere( vCenter, fRadius, vSweepDir ) == true )
        {
            // Perform same test on the individual leaves.
            for ( cgSceneLeafSet::iterator itLeaf = LeafTable.begin(); itLeaf != LeafTable.end(); ++itLeaf )
            {
                AABB    = (*itLeaf)->getBoundingBox();
                vCenter = AABB.getCenter();
                fRadius = cgVector3::length( AABB.getExtents() );

                // If swept into frustum, add to output set.
                if ( Operator.testSweptSphere( vCenter, fRadius, vSweepDir ) == true )
                {
                    (pOutput->mTreeLeaves[pTree]).insert( *itLeaf );
                    Bounds.addPoint( AABB.min );
                    Bounds.addPoint( AABB.max );
                
                } // End if intersects

            } // Next Leaf

        } // End if tree visible

    } // Next Tree
	
	// Intersect objects
    for ( size_t i = 0; i < mObjectNodes.size(); ++i )
	{
        // Test to see if the object has any possibility of being "swept" into
        // the frustum (i.e. perhaps it might cast a shadow onto something within
        // the frustum). We use a swept sphere for this
        pObject = mObjectNodes[i];
        AABB    = pObject->getBoundingBox();
        vCenter = AABB.getCenter();
        fRadius = cgVector3::length( AABB.getExtents() );

        // Object swept into input frustum?
        if ( Operator.testSweptSphere( vCenter, fRadius, vSweepDir ) == true )
        {
            pOutput->mObjectNodeLUT[ pObject ] = pOutput->mObjectNodes.size();
            pOutput->mObjectNodes.push_back( pObject );
            Bounds.addPoint( AABB.min );
            Bounds.addPoint( AABB.max );
        
        } // End if intersects
		
    } // Next Object
	
	/*// Intersect lights
    cgObjectNodeSet::iterator itObject;
    for ( itObject = mLights.begin(); itObject != mLights.end(); ++itObject )
	{
        if ( (pObject = (*itObject)) == CG_NULL )
            continue;

        // Test to see if the object has any possibility of being "swept" into
        // the frustum (i.e. perhaps it might cast a shadow onto something within
        // the frustum). We use a swept sphere for this
        AABB    = pObject->getBoundingBox();
        vCenter = AABB.getCenter();
        fRadius = cgVector3::length(&AABB.getExtents());

        // Object swept into input frustum?
        if ( Operator.testSweptSphere( vCenter, fRadius, vSweepDir ) == true )
        {
            pOutput->mLights.insert( pObject );
            Bounds.addPoint( AABB.min );
            Bounds.addPoint( AABB.max );
        
        } // End if intersects
		
    } // Next Light*/

    // Select the next unique visibility result identifier
    pOutput->mResultId = mNextResultId++;
    
    // If the result identifier variable overflowed and wrapped around,
    // ensure that the reserved code '0' is skipped.
    if ( mNextResultId == 0 )
        mNextResultId = 1;

    // Visibility volume in this case will be equal to the specified frustum.
    pOutput->mFrustum = Operator;
}

//-----------------------------------------------------------------------------
//  Name : query ()
/// <summary>
/// Determine if the specified object is contained within the visibility
/// set or not.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::query( cgObjectNode * pObject ) const
{
    if ( pObject == CG_NULL )
        return false;

    // Test the correct set.
    if ( pObject->queryReferenceType( RTID_LightNode ) )
        return (mLightLUT.find( pObject ) != mLightLUT.end());
    else 
        return (mObjectNodeLUT.find( pObject ) != mObjectNodeLUT.end());
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// After visibility has been computed, it can be applied to all
/// applicable spatial trees / objects as necessary by calling this
/// method.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::apply( )
{
    // ToDo: 9999 - Still necessary?
    /*// Early out if this visibility set was most recently applied.
    if ( mAppliedSet == mResultId )
        return;
    
    // Allow spatial tree(s) to apply the visibility result.
    TreeLeafMap::iterator   itLeafList;
    cgSpatialTreeNode     * pTree;
    for ( itLeafList = mTreeLeaves.begin(); itLeafList != mTreeLeaves.end(); ++itLeafList )
    {
        pTree = (cgSpatialTreeNode*)itLeafList->first;
        if ( pTree != CG_NULL )
            pTree->ApplyVisibility( this );
    
    } // Next List

    // This set was most recently applied
    mAppliedSet = mResultId;*/
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Clear out all of the currently computed visibility information.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::clear( )
{
    // Clear out tree visibility data
    mScene = CG_NULL;
    mTreeLeaves.clear();
    mAssociatedGroups.clear();
    mObjectNodes.clear();
    mObjectNodeLUT.clear();
    mLights.clear();
    mLightLUT.clear();
    mRenderClasses.clear();

    // Select the next unique visibility result identifier
    mResultId = mNextResultId++;

    // If the result identifier variable overflowed and wrapped around,
    // ensure that the reserved code '0' is skipped.
    if ( mNextResultId == 0 )
        mNextResultId = 1;

    // Visibility set is now dirty.
    mLastComputedFrame    = 0;
    mLastFlags            = 0;
    mLastLightOcclusion   = false;
}

//-----------------------------------------------------------------------------
//  Name : getResultId ()
/// <summary>
/// Retrieve the unique visibility result identifier for the call to 'compute'
/// that generated this visibility set information.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVisibilitySet::getResultId( ) const
{
    return mResultId;
}

//-----------------------------------------------------------------------------
//  Name : addVisibleObject ()
/// <summary>
/// If an object is deemed to be visible, this method should be called
/// in order for the object to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleObject( cgObjectNode * pObject )
{
    /*// Objects must have a valid reference identifier in order to be considered.
    if ( !pObject || !pObject->GetReferenceId() )
        return;*/
    
    cgToDo( "Optimization", "Use lower_bound trick for insertion hint" )
    
    // Skip object if it already exists. Note: Could use 'lower_bound' approach
    // for insertion hint if we ever switch back to sorted map.
    if ( mObjectNodeLUT.find( pObject ) == mObjectNodeLUT.end() )
    {
        // Store in the list (will automatically overwrite if already exists).
        mObjectNodeLUT.insert( EntryLUT::value_type( pObject, mObjectNodes.size() ) );
        mObjectNodes.push_back( pObject );

        // Also store in render class list.
        RenderClass & Class = mRenderClasses[pObject->getRenderClassId()];
        Class.objectNodes.push_back( pObject );

        // Add to the combined bounding box for the entire render class.
        const cgBoundingBox & Bounds = pObject->getBoundingBox();
        Class.combinedBounds.addPoint( Bounds.min );
        Class.combinedBounds.addPoint( Bounds.max );

    } // End if does not exist.
}

//-----------------------------------------------------------------------------
//  Name : addVisibleLight ()
/// <summary>
/// If a light is deemed to be visible, this method should be called
/// in order for the object to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleLight( cgObjectNode * pLight )
{
    /*// Objects must have a valid reference identifier in order to be considered.
    if ( !pLight || !pLight->GetReferenceId() )
        return;*/
    
    cgToDo( "Optimization", "Use lower_bound trick for insertion hint" )
    
    // Skip object if it already exists. Note: Could use 'lower_bound' approach
    // for insertion hint if we ever switch back to sorted map.
    if ( mLightLUT.find( pLight ) == mLightLUT.end() )
    {
        // Store in the list (will automatically overwrite if already exists).
        mLightLUT.insert( EntryLUT::value_type( pLight, mLights.size() ) );
        mLights.push_back( pLight );

    } // End if does not exist.
}

//-----------------------------------------------------------------------------
//  Name : addVisibleMaterial ()
/// <summary>
/// If an object is deemed to be visible, this method should be called
/// for each material that it utilizes in order for us to record the need to
/// potentially render that object in conjunction with the specified material.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleMaterial( const cgMaterialHandle & hMaterial, cgObjectNode * pObject )
{
    MaterialBatch & Material = mRenderClasses[pObject->getRenderClassId()].materials[ hMaterial ];

    cgToDo( "Optimization", "Use lower_bound trick for insertion hint" );

    if ( Material.objectNodeLUT.find( pObject ) == Material.objectNodeLUT.end() )
    {
        Material.objectNodeLUT[ pObject ] = Material.objectNodes.size();
        Material.objectNodes.push_back( pObject );

        // Add to the combined bounding box for the entire material match.
        const cgBoundingBox & Bounds = pObject->getBoundingBox();
        Material.combinedBounds.addPoint( Bounds.min );
        Material.combinedBounds.addPoint( Bounds.max );
    
    } // End if not existing

    /*// Objects must have a valid reference identifier in order to be considered.
    if ( pObject == CG_NULL || pObject->GetReferenceId() == 0 || hMaterial.IsValid() == false )
        return;
    
    // Store in the list (will automatically overwrite if already exists).
    mRenderClasses[pObject->getRenderClassId()].materials[hMaterial].insert( pObject );*/
}

//-----------------------------------------------------------------------------
//  Name : addVisibleLeaf ()
/// <summary>
/// If a spatial tree leaf is deemed to be visible, this method should
/// be called in order for it to be marked as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleLeaf( cgSpatialTreeInstance * pTree, cgSpatialTreeLeaf * pLeaf )
{
    // Store in the list (will automatically overwrite if already exists)
    (mTreeLeaves[pTree]).insert( pLeaf );
}

//-----------------------------------------------------------------------------
//  Name : addVisibleGroup ()
/// <summary>
/// If a data group (associated with a given context object) is deemed to be 
/// visible, this method should be called in order for it to be marked 
/// as such.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::addVisibleGroup( void * pContext, cgInt32 nDataGroupId )
{
    // Store in the list (will automatically overwrite if already exists)
    (mAssociatedGroups[pContext]).insert( nDataGroupId );
}

//-----------------------------------------------------------------------------
//  Name : enableLightOcclusion ()
/// <summary>
/// Enable or disable the occlusion cull testing for light sources.
/// </summary>
//-----------------------------------------------------------------------------
void cgVisibilitySet::enableLightOcclusion( bool bEnable )
{
    mLightOcclusion = bEnable;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleObjects ()
/// <summary>
/// Retrieve the list of visible objects.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeArray & cgVisibilitySet::getVisibleObjects()
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleObjects () (const overload)
/// <summary>
/// Retrieve the list of visible objects.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeArray & cgVisibilitySet::getVisibleObjects() const
{
    return mObjectNodes;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleRenderClasses ()
/// <summary>
/// Retrieve the list of visible object render classes (and their associated 
/// objects batched by material).
/// </summary>
//-----------------------------------------------------------------------------
cgVisibilitySet::RenderClassMap & cgVisibilitySet::getVisibleRenderClasses()
{
    return mRenderClasses;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleRenderClasses () (const overload)
/// <summary>
/// Retrieve the list of visible object render classes (and their associated 
/// objects batched by material).
/// </summary>
//-----------------------------------------------------------------------------
const cgVisibilitySet::RenderClassMap & cgVisibilitySet::getVisibleRenderClasses() const
{
    return mRenderClasses;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLights ()
/// <summary>
/// Retrieve the list of visible lights.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNodeArray & cgVisibilitySet::getVisibleLights()
{
    return mLights;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLights () (const overload)
/// <summary>
/// Retrieve the list of visible lights.
/// </summary>
//-----------------------------------------------------------------------------
const cgObjectNodeArray & cgVisibilitySet::getVisibleLights() const
{
    return mLights;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLeaves ()
/// <summary>
/// Retrieve the list of visible leaves associated with the specified
/// spatial tree.
/// </summary>
//-----------------------------------------------------------------------------
cgSceneLeafSet & cgVisibilitySet::getVisibleLeaves( cgSpatialTreeInstance * pTree )
{
    return mTreeLeaves[pTree];
}

//-----------------------------------------------------------------------------
//  Name : getVisibleLeaves () (const overload)
/// <summary>
/// Retrieve the list of visible leaves associated with the specified
/// spatial tree.
/// </summary>
//-----------------------------------------------------------------------------
const cgSceneLeafSet & cgVisibilitySet::getVisibleLeaves( cgSpatialTreeInstance * pTree ) const
{
    static cgSceneLeafSet emptySet;
    TreeLeafMap::const_iterator itLeaf = mTreeLeaves.find( pTree );
    if ( itLeaf == mTreeLeaves.end() )
        return emptySet;
    return itLeaf->second;
}

//-----------------------------------------------------------------------------
//  Name : getVisibleGroups ()
/// <summary>
/// Retrieve the list of visible data groups associated with the specified
/// context.
/// </summary>
//-----------------------------------------------------------------------------
cgInt32Set & cgVisibilitySet::getVisibleGroups( void * pContext )
{
    return mAssociatedGroups[pContext];
}

//-----------------------------------------------------------------------------
//  Name : getVisibleGroups () (const overload)
/// <summary>
/// Retrieve the list of visible data groups associated with the specified
/// context.
/// </summary>
//-----------------------------------------------------------------------------
const cgInt32Set & cgVisibilitySet::getVisibleGroups( void * pContext ) const
{
    static cgInt32Set emptySet;
    AssociatedGroupMap::const_iterator itGroupList = mAssociatedGroups.find( pContext );
    if ( itGroupList == mAssociatedGroups.end() )
        return emptySet;
    return itGroupList->second;
}

//-----------------------------------------------------------------------------
//  Name : getVolume ()
/// <summary>
/// Retrieve the visibility volume / frustum used to construct this set.
/// </summary>
//-----------------------------------------------------------------------------
cgFrustum & cgVisibilitySet::getVolume( )
{
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : getVolume ()
/// <summary>
/// Retrieve the visibility volume / frustum used to construct this set.
/// </summary>
//-----------------------------------------------------------------------------
const cgFrustum & cgVisibilitySet::getVolume( ) const
{
    return mFrustum;
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if anything is contained in the visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isEmpty( ) const
{
    return ( mObjectNodes.empty() && mLights.empty() 
             && mTreeLeaves.empty() && mAssociatedGroups.empty() 
             && mRenderClasses.empty() );
}

//-----------------------------------------------------------------------------
//  Name : isObjectVisible ()
/// <summary>
/// Determine if the specified object is contained in this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isObjectVisible( cgObjectNode * object ) const
{
    return (mObjectNodeLUT.find( object ) != mObjectNodeLUT.end());
}

//-----------------------------------------------------------------------------
//  Name : isLightVisible ()
/// <summary>
/// Determine if the specified light is contained in this visibility set.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVisibilitySet::isLightVisible( cgObjectNode * light ) const
{
    return (mLightLUT.find( light ) != mLightLUT.end());
}