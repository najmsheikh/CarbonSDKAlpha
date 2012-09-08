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
// Name : cgGroupObject.cpp                                                  //
//                                                                           //
// Desc : Contains classes responsible for providing a scene level "group"   //
//        object that can be used to combine objects into higher level       //
//        collections that can be manipulated as one.                        //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgGroupObject Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgGroupObject.h>
#include <World/cgScene.h>
#include <Rendering/cgRenderDriver.h>
#include <Math/cgCollision.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgGroupObject::mInsertGroup;
cgWorldQuery cgGroupObject::mUpdateOpen;
cgWorldQuery cgGroupObject::mLoadGroup;

// ToDo: Consider removing groups own internal reference counting scheme
//       and instead move over to the cgReference scheme from which the 
//       node already derives? Potential problems exist however if other 
//       parts of the system are referencing it but the group really does 
//       need to be removed / destroyed.

///////////////////////////////////////////////////////////////////////////////
// cgGroupObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgGroupObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupObject::cgGroupObject( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldObject( nReferenceId, pWorld )
{
    // Initialize members to sensible defaults
    mOpen = false;
}

//-----------------------------------------------------------------------------
//  Name : cgActorObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupObject::cgGroupObject( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldObject * pInit, cgCloneMethod::Base InitMethod ) : cgWorldObject( nReferenceId, pWorld, pInit, InitMethod )
{
    // Duplicate values from object to clone.
    cgGroupObject * pObject = (cgGroupObject*)pInit;

    // ToDo: 9999 - Clone

}

//-----------------------------------------------------------------------------
//  Name : ~cgGroupObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupObject::~cgGroupObject()
{
    // Release allocated memory
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgGroupObject::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorld * pWorld )
{
    return new cgGroupObject( nReferenceId, pWorld );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_GroupObject )
        return true;

    // Supported by base?
    return cgWorldObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgGroupObject::getDatabaseTable( ) const
{
    return _T("Objects::Group");
}

//-----------------------------------------------------------------------------
//  Name : isOpen()
/// <summary>
/// Determine if the group is currently open such that its child objects can be
/// selected individually for manipulation -- or closed so they cant.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupObject::isOpen( ) const
{
    return mOpen;
}

//-----------------------------------------------------------------------------
//  Name : setOpen() (Virtual)
/// <summary>
/// Mark the group as open such that its child objects can be selected
/// individually for manipulation -- or closed so they cant.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupObject::setOpen( bool bOpen )
{
    // Is this a no-op?
    if ( mOpen == bOpen )
        return;

    // Update world database
    if ( shouldSerialize() == true )
    {
        prepareQueries();
        mUpdateOpen.bindParameter( 1, bOpen );
        mUpdateOpen.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( mUpdateOpen.step( true ) == false )
        {
            cgString strError;
            mUpdateOpen.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update open status of group object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mOpen = bOpen;

    // Notify listeners that property was altered
    static const cgString strContext = _T("Open");
    onComponentModified( &cgComponentModifiedEventArgs( strContext ) );
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
        return false;

    // Call base class implementation last.
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("GroupObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertGroup.bindParameter( 1, mReferenceId );
        mInsertGroup.bindParameter( 2, mOpen );

        // Execute
        if ( mInsertGroup.step( true ) == false )
        {
            cgString strError;
            mInsertGroup.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for group object '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("GroupObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("GroupObject::insertComponentData") );

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
bool cgGroupObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the dummy data.
    prepareQueries();
    mLoadGroup.bindParameter( 1, e->sourceRefId );
    if ( !mLoadGroup.step( ) || !mLoadGroup.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( mLoadGroup.getLastError( strError ) == false )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for group object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for group object '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadGroup.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadGroup;

    // Update our local members
    mLoadGroup.getColumn( _T("Open"), mOpen );

    // Call base class implementation to read remaining data.
    if ( !cgWorldObject::onComponentLoading( e ) )
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
void cgGroupObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( mInsertGroup.isPrepared() == false )
            mInsertGroup.prepare( mWorld, _T("INSERT INTO 'Objects::Group' VALUES(?1,?2,NULL)"), true );
        if ( mUpdateOpen.isPrepared() == false )
            mUpdateOpen.prepare( mWorld, _T("UPDATE 'Objects::Group' SET Open=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( mLoadGroup.isPrepared() == false )
        mLoadGroup.prepare( mWorld, _T("SELECT * FROM 'Objects::Group' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : sandboxRender ( ) (Virtual)
/// <summary>
/// Allow the object to render its 'sandbox' representation -- that is the
/// representation to be displayed within an editing environment.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupObject::sandboxRender( cgCameraNode * pCamera, cgVisibilitySet * pVisData, bool bWireframe, const cgPlane & GridPlane, cgObjectNode * pIssuer )
{
    // Get access to required systems.
    cgRenderDriver * pDriver = mWorld->getRenderDriver();

    // Always draw the computed AABB if the group is open
    if ( isOpen() )
        pDriver->drawOOBB( pIssuer->getLocalBoundingBox( ), 2.5f, pIssuer->getWorldTransform(false), (pIssuer->isSelected()) ? 0xFFFFFFFF : 0xFFFF7FB2 );

    // Call base class implementation last.
    cgWorldObject::sandboxRender( pCamera, pVisData, bWireframe, GridPlane, pIssuer );
}

//-----------------------------------------------------------------------------
//  Name : applyObjectRescale ()
/// <summary>
/// Apply a scale to all *local* data internal to this object. For instance,
/// in the case of a light source its range parameters will be scaled. For a 
/// mesh, the vertex data will be scaled, etc.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupObject::applyObjectRescale( cgFloat fScale )
{
    // Nothing to re-scale in this implementation

    // Call base class implementation.
    cgWorldObject::applyObjectRescale( fScale );
}

///////////////////////////////////////////////////////////////////////////////
// cgGroupNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgGroupNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupNode::cgGroupNode( cgUInt32 nReferenceId, cgScene * pScene ) : cgObjectNode( nReferenceId, pScene )
{
    mAutoSelfDestruct = true;
    mGroupRefCount = 0;
    mBounds.reset();

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Group%X"), nReferenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgGroupNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupNode::cgGroupNode( cgUInt32 nReferenceId, cgScene * pScene, cgObjectNode * pInit, cgCloneMethod::Base InitMethod, const cgTransform & InitTransform ) : cgObjectNode( nReferenceId, pScene, pInit, InitMethod, InitTransform )
{
    mAutoSelfDestruct = true;
    mGroupRefCount = 0;
    mBounds.reset();

    // ToDo: 9999 - Clone
}

//-----------------------------------------------------------------------------
//  Name : ~cgGroupNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgGroupNode::~cgGroupNode()
{
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgGroupNode::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgScene * pScene )
{
    return new cgGroupNode( nReferenceId, pScene );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_GroupNode )
        return true;

    // Supported by base?
    return cgObjectNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getLocalBoundingBox () (Virtual)
/// <summary>
/// Retrieve the axis aligned bounding box for this node (encompassing
/// its referenced object) as it exists in the object's local space.
/// </summary>
//-----------------------------------------------------------------------------
cgBoundingBox cgGroupNode::getLocalBoundingBox( )
{
    cgTransform InverseGroupTransform;
    cgTransform::inverse( InverseGroupTransform, getWorldTransform() );

    // Reset cached bounding box
    mBounds.reset();

    // Compute the bounding box of all children (and their children) in the space of the group.
    cgObjectNodeList::iterator itChild;
    for ( itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        cgObjectNode * pNode = *itChild;
        if ( pNode->getOwnerGroup() == this )
            computeGroupAABB( pNode, mBounds, InverseGroupTransform );
    
    } // Next Child

    // Return computed bounding box.
    return mBounds;
}

//-----------------------------------------------------------------------------
//  Name : computeGroupAABB() (Protected, Recursive)
/// <summary>
/// Recursively computes the AABB of this group based on all children contained
/// in the group.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::computeGroupAABB( cgObjectNode * pNode, cgBoundingBox & Bounds, cgTransform & InverseGroupTransform )
{
    cgBoundingBox ChildBounds;
    
    // Retrieve the bounding box of this node
    ChildBounds = pNode->getLocalBoundingBox( );
    if ( ChildBounds.isPopulated() )
    {
        // Transform the bounding box into the space of this group.
        // In order to do so we must compute the relative transformation
        // to transform from one space to the other.
        cgTransform RelativeTransform;
        cgTransform::multiply( RelativeTransform, pNode->getWorldTransform(), InverseGroupTransform );
        ChildBounds.transform( RelativeTransform );
        
        // Grow the current bounding box as required
        Bounds.addPoint( ChildBounds.min );
        Bounds.addPoint( ChildBounds.max );

    } // End if has bounding box

    // Continue to process all children that belong to this group.
    cgObjectNodeList::iterator itChild;
    for ( itChild = pNode->getChildren().begin(); itChild != pNode->getChildren().end(); ++itChild )
    {
        cgObjectNode * pChildNode = *itChild;
        if ( pChildNode->getOwnerGroup() == this )
            computeGroupAABB( pChildNode, Bounds, InverseGroupTransform );
    
    } // Next Child
}

//-----------------------------------------------------------------------------
// Name : alterSelectionState() (Protected, Recursive)
/// <summary>
/// Adjust the selection state of any of our direct children who are belong
/// to this group ;)
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::alterSelectionState( bool bState, cgObjectNode * pNode )
{
    // Alter the current node's own selection state first of all
    if ( pNode == this )
        pNode->setSelected( bState, true, true );
    else
        pNode->setSelected( bState, true, false );

    // Now traverse the child hierarchy and update any who
    // belong to this group.
    cgObjectNodeList::iterator itChild;
    for ( itChild = pNode->getChildren().begin(); itChild != pNode->getChildren().end(); ++itChild )
    {
        cgObjectNode * pChildNode = *itChild;
        if ( pChildNode->getOwnerGroup() == this )
            alterSelectionState( bState, pChildNode );
    
    } // Next Child
}

//-----------------------------------------------------------------------------
// Name : alterOpenState() (Protected, Recursive)
/// <summary>
/// Adjust the "open" state of any of our direct child groups who belong to
/// this group.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::alterOpenState( bool bState, cgObjectNode * pNode )
{
    // Alter the current node's own open state first of all if it is a group.
    if ( pNode->queryReferenceType( RTID_GroupNode ) == true )
        ((cgGroupNode*)pNode)->setOpen( bState );
    
    // Now traverse the child hierarchy and update any who
    // also belong to this group.
    cgObjectNodeList::iterator itChild;
    for ( itChild = pNode->getChildren().begin(); itChild != pNode->getChildren().end(); ++itChild )
    {
        cgObjectNode * pChildNode = *itChild;
        if ( pChildNode->getOwnerGroup() == this )
            alterOpenState( bState, pChildNode );
    
    } // Next Child
}

//-----------------------------------------------------------------------------
//  Name : pick ( ) (Virtual)
/// <summary>
/// Given the specified world space ray, determine if this object is 
/// intersected (or any of its children) and also compute the world space 
/// intersection distance. Note: The "distance" parameter is an in/out that will
/// be used to determine if this is the closest hit so far. Pass in a value
/// of FLT_MAX initially.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupNode::pick( cgCameraNode * pCamera, const cgSize & ViewportSize, const cgVector3 & vOrigin, const cgVector3 & vDir, bool bWireframe, cgFloat fWireTolerance, cgFloat & fDistance, cgObjectNode *& pClosestNode )
{
    bool bChildHit = false;

    // Determine if any of our children were hit.
    cgObjectNodeList::iterator itChild;
    for ( itChild = mChildren.begin(); itChild != mChildren.end(); ++itChild )
    {
        cgObjectNode * pChildNode = *itChild;
        bChildHit |= pChildNode->pick( pCamera, ViewportSize, vOrigin, vDir, bWireframe, fWireTolerance, fDistance, pClosestNode );
    
    } // Next Child

    // If the group is open, pass through to children as normal and
    // do not interfere with the picking process.
    if ( isOpen() )
    {
        return bChildHit;
    
    } // End if group open
    else
    {
        // Otherwise, the group is closed and we should override
        // the picking process such that this group is the "picked"
        // object if any object belonging to this group is the one
        // actually picked.
        if ( bChildHit == true && pClosestNode->getOwnerGroup() == this )
            pClosestNode = this;
        return bChildHit;

    } // End if group closed
}

//-----------------------------------------------------------------------------
// Name : detachNode ( )
/// <summary>
/// Detach the specified node from this group (and any children also 
/// belonging to this group).
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::detachNode( cgObjectNode * pNode )
{
    // Validate requirements
    if ( pNode->getOwnerGroup() != this )
        return;

    // Detach any children first (so that the final de-reference
    // that follows it will safely destroy the group).
    cgObjectNodeList::iterator itChild;
    for ( itChild = pNode->getChildren().begin(); itChild != pNode->getChildren().end(); ++itChild )
    {
        cgObjectNode * pChildNode = *itChild;
        if ( pChildNode->getOwnerGroup() == this )
            detachNode( pChildNode );

    } // Next Child

    // Finally de-reference this node
    pNode->setOwnerGroup( CG_NULL );
}

//-----------------------------------------------------------------------------
// Name : getGroupedNodes ( )
/// <summary>
/// Return a list of all nodes belonging to this group. Can also optionally
/// traverse any child group hierarchy in order to retrieve their nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::getGroupedNodes( cgObjectNodeMap & NodesOut, bool bExplodeGroups )
{
    NodesOut.clear();
    getGroupedNodes( NodesOut, this, this, bExplodeGroups );
}

//-----------------------------------------------------------------------------
// Name : getGroupedNodes ( ) (Protected, Recursive)
/// <summary>
/// Return a list of all nodes belonging to this group. Can also optionally
/// traverse any child group hierarchy in order to retrieve their nodes.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::getGroupedNodes( cgObjectNodeMap & NodesOut, cgGroupNode * pParentGroup, cgObjectNode * pNode, bool bExplodeGroups )
{
    cgObjectNode * pChildNode;
    cgObjectNodeList::iterator itChild;
    for ( itChild = pNode->getChildren().begin(); itChild != pNode->getChildren().end(); ++itChild )
    {
        // If this node does not belong to the current
        // owner group then it can safely be ignored.
        pChildNode = *itChild;
        if ( pChildNode->getOwnerGroup() != pParentGroup )
            continue;

        // If we're exploding groups, we need to check if this
        // child is also a group.
        if ( bExplodeGroups == true )
        {
            if ( pChildNode->queryReferenceType( RTID_GroupNode ) == true )
            {
                // Recurse into this group
                getGroupedNodes( NodesOut, (cgGroupNode*)pChildNode, pChildNode, true );
                continue;
            
            } // End if group node

        } // End if exploding groups

        // Record this node (it belongs to this group) and recurse into its children
        NodesOut[ pChildNode->getReferenceId() ] = pChildNode;
        getGroupedNodes( NodesOut, pParentGroup, pChildNode, bExplodeGroups );
        
    } // Next Child Node
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // What was modified?
    if ( e->context == _T("Open") )
    {
        // If opening, then deselect the group (will automatically deselect children)
        bool bNewValue = isOpen();
        if ( bNewValue == true )
            setSelected( false );

        // If closing, we must close any open child groups.
        if ( bNewValue == false )
            alterOpenState( false, this );

        // If closing, then also select the group (will automatically select children)
        if ( bNewValue == false )
            alterSelectionState( true, this );

    } // End if Open

    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
// Name : onGroupRefAdded( ) (Virtual)
/// <summary>
/// Increment the number of objects in the scene that reference this group as 
/// their owner. If this number ever falls to zero then the group will be 
/// removed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgGroupNode::onGroupRefAdded( cgObjectNode * pNode )
{
    return ++mGroupRefCount;
}

//-----------------------------------------------------------------------------
// Name : onGroupRefRemoved( ) (Virtual)
/// <summary>
/// Decrement the number of objects in the scene that reference this group as 
/// their owner. If this number ever falls to zero then the group will be 
/// removed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgGroupNode::onGroupRefRemoved( cgObjectNode * pNode )
{
    --mGroupRefCount;

    // If this group is now essentially empty, remove it
    // unless our parent scene is in the process of disposing or
    // the auto destruct behavior was disabled.
    if ( mGroupRefCount == 0 && mAutoSelfDestruct && !mParentScene->isDisposing() )
    {
        mParentScene->deleteObjectNode( this );
        return 0;
    
    } // End if no refs
    return mGroupRefCount;
}

//-----------------------------------------------------------------------------
//  Name : setSelected () (Virtual)
/// <summary>
/// Set the node's selection status.
/// </summary>
//-----------------------------------------------------------------------------
void cgGroupNode::setSelected( bool bSelected, bool bUpdateDependents /* = true */, bool bSendNotifications /* = true */  )
{
    // Skip if this is a no-op (also prevents infinite recursion)
    if ( mSelected == bSelected )
        return;

    // Call base class implementation
    cgObjectNode::setSelected( bSelected, bUpdateDependents, bSendNotifications );

    // If we are currently a closed group, affect all children too
    if ( isOpen() == false )
        alterSelectionState( bSelected, this );
}

//-----------------------------------------------------------------------------
// Name : onNodeDeleted () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// deleted in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgGroupNode::onNodeDeleted( )
{
    // Disable auto self-destruct behavior (automatically deletes
    // the group from the scene when all child nodes are detached). 
    // We are in the process of being deleted and we don't want to 
    // trigger this.
    mAutoSelfDestruct = false;

    // Call base class implementation
    return cgObjectNode::onNodeDeleted( );
}