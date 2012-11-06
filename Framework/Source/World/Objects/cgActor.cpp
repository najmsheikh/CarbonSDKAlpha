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
// Name : cgActor.cpp                                                        //
//                                                                           //
// Desc : An actor is a specialized object group that can be used to collect //
//        together objects (as children) that can be managed and animated    //
//        as one entity. The actor can then be used to manage animation data //
//        and behaviors for the collection as a whole.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgActor Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/cgActor.h>
#include <World/Objects/Elements/cgAnimationSetElement.h>
#include <World/cgScene.h>
#include <Animation/cgAnimationController.h>
#include <Resources/cgAnimationSet.h>
#include <Resources/cgResourceManager.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgActorObject::mInsertActor;
cgWorldQuery cgActorObject::mUpdateOpen;
cgWorldQuery cgActorObject::mLoadActor;

///////////////////////////////////////////////////////////////////////////////
// cgActorObject Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgActorObject () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorObject::cgActorObject( cgUInt32 referenceId, cgWorld * world ) : cgGroupObject( referenceId, world )
{
    // Initialize members to sensible defaults
}

//-----------------------------------------------------------------------------
//  Name : cgActorObject () (Constructor)
/// <summary>
/// Cloning constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorObject::cgActorObject( cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod ) : cgGroupObject( referenceId, world, init, initMethod )
{
    // Duplicate values from object to clone.
    cgActorObject * object = static_cast<cgActorObject*>(init);
}

//-----------------------------------------------------------------------------
//  Name : ~cgActorObject () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorObject::~cgActorObject()
{
    // Release allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgActorObject::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( disposeBase )
        cgGroupObject::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgActorObject::allocateNew( const cgUID & type, cgUInt32 referenceId, cgWorld * world )
{
    return new cgActorObject( referenceId, world );
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a world object of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgWorldObject * cgActorObject::allocateClone( const cgUID & type, cgUInt32 referenceId, cgWorld * world, cgWorldObject * init, cgCloneMethod::Base initMethod )
{
    return new cgActorObject( referenceId, world, init, initMethod );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ActorObject )
        return true;

    // Supported by base?
    return cgGroupObject::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgActorObject::getDatabaseTable( ) const
{
    return _T("Objects::Actor");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object.
    if ( !insertComponentData( ) )
        return false;

    // Call base class implementation last (skip group object, this 
    // class provides its serialization functionality in new tables).
    return cgWorldObject::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("ActorObject::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertActor.bindParameter( 1, mReferenceId );
        mInsertActor.bindParameter( 2, mOpen );
        mInsertActor.bindParameter( 3, mSoftRefCount );

        // Execute
        if ( !mInsertActor.step( true ) )
        {
            cgString error;
            mInsertActor.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for actor object '0x%x' into database. Error: %s\n"), mReferenceId, error.c_str() );
            mWorld->rollbackTransaction( _T("ActorObject::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("ActorObject::insertComponentData") );

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
bool cgActorObject::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the actor data.
    prepareQueries();
    mLoadActor.bindParameter( 1, e->sourceRefId );
    if ( !mLoadActor.step( ) || !mLoadActor.nextRow() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadActor.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for actor object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for actor object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadActor.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadActor;

    // Update our local members
    mLoadActor.getColumn( _T("Open"), mOpen );
    
    // Call base class implementation to read remaining data (skip group object, 
    // this class provides its serialization  functionality in new tables).
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
// Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorObject::onComponentDeleted( )
{
    // Call base class implementation last.
    cgGroupObject::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorObject::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertActor.isPrepared() )
            mInsertActor.prepare( mWorld, _T("INSERT INTO 'Objects::Actor' VALUES(?1,?2,?3)"), true );
        if ( !mUpdateOpen.isPrepared() )
            mUpdateOpen.prepare( mWorld, _T("UPDATE 'Objects::Actor' SET Open=?1 WHERE RefId=?2"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadActor.isPrepared() )
        mLoadActor.prepare( mWorld, _T("SELECT * FROM 'Objects::Actor' WHERE RefId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : setOpen() (Virtual)
/// <summary>
/// Mark the group as open such that its child objects can be selected
/// individually for manipulation -- or closed so they cant.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorObject::setOpen( bool open )
{
    // Is this a no-op?
    if ( mOpen == open )
        return;

    // Update world database
    if ( shouldSerialize() )
    {
        prepareQueries();
        mUpdateOpen.bindParameter( 1, open );
        mUpdateOpen.bindParameter( 2, mReferenceId );
        
        // Execute
        if ( !mUpdateOpen.step( true ) )
        {
            cgString error;
            mUpdateOpen.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update open status of actor object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
            return;
        
        } // End if failed
    
    } // End if serialize

    // Update value.
    mOpen = open;

    // Notify listeners that property was altered
    static const cgString modificationContext = _T("Open");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );
}

//-----------------------------------------------------------------------------
//  Name : addAnimationSet()
/// <summary>
/// Add the animation set for management / referencing by this actor.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::addAnimationSet( const cgAnimationSetHandle & animationSet )
{
    // Animation set must be valid.
    if ( !animationSet.isValid() )
        return false;

    // Create a sub element to represent this animation set.
    cgAnimationSetElement * element = (cgAnimationSetElement*)createSubElement( OSECID_AnimationSets, RTID_AnimationSetElement );
    if ( element )
        element->setAnimationSet( animationSet );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSetByName()
/// <summary>
/// Retrieve the first available animation set with the specified name.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSetHandle & cgActorObject::getAnimationSetByName( const cgString & name ) const
{
    const cgObjectSubElementArray & elements = getSubElements( OSECID_AnimationSets );
    for ( size_t i = 0; i < elements.size(); ++i )
    {
        const cgAnimationSet * animationSet = static_cast<cgAnimationSetElement*>(elements[i])->getAnimationSet().getResourceSilent();
        if ( animationSet && animationSet->getName() == name )
            return static_cast<cgAnimationSetElement*>(elements[i])->getAnimationSet();
    
    } // Next set
    return cgAnimationSetHandle::Null;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSetCount()
/// <summary>
/// Retrieve the total number of animation sets managed by this actor.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgActorObject::getAnimationSetCount( ) const
{
    return (cgUInt32)getSubElements( OSECID_AnimationSets ).size();
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSet()
/// <summary>
/// Retrieve the animation set at the specified index location.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSetHandle & cgActorObject::getAnimationSet( cgUInt32 index ) const
{
    const cgObjectSubElementArray & elements = getSubElements( OSECID_AnimationSets );
    if ( index >= (cgUInt32)elements.size() )
        return cgAnimationSetHandle::Null;
    return static_cast<cgAnimationSetElement*>(elements[index])->getAnimationSet();
}

//-----------------------------------------------------------------------------
//  Name : getSubElementCategories () (Virtual)
/// <summary>
/// Enumerate the list of sub-element categories and types that can be accessed
/// by the sandbox environment / application. Returns true if sub-elements are
/// supported.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::getSubElementCategories( cgObjectSubElementCategory::Map & Categories ) const
{
    // Call base class implementation to populate base elements.
    // We bypass 'cgGroupObject' intentionally.
    cgWorldObject::getSubElementCategories( Categories );

    // Actor objects also expose animation sets
    cgObjectSubElementCategory & Category = Categories[ OSECID_AnimationSets ];
    Category.identifier   = OSECID_AnimationSets;
    Category.name         = _T("Animation Sets");
    Category.canCreate    = true;
    Category.canDelete    = true;
    Category.canEnumerate = true;
    
    // Animation Set
    {
        cgObjectSubElementDesc & Type = Category.supportedTypes[ RTID_AnimationSetElement ];
        Type.identifier = RTID_AnimationSetElement;
        Type.name       = _T("Animation Set");
    
    } // End Animation Set
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : supportsSubElement () (Virtual)
/// <summary>
/// Determine if the specified object sub element type is supported by this
/// world object. Derived object types should implement this to extend the
/// allowable sub element types.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorObject::supportsSubElement( const cgUID & Category, const cgUID & Identifier ) const
{
    // Call base class implementation first. We bypass 'cgGroupObject' intentionally.
    if ( cgWorldObject::supportsSubElement( Category, Identifier ) )
        return true;

    // Validate supported categories.
    if ( Category == OSECID_AnimationSets )
    {
        // Validate supported types.
        if ( Identifier == RTID_AnimationSetElement )
             return true;

    } // End OSECID_AnimationSets
    
    // Unsupported
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cgActorNode Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgActorNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorNode::cgActorNode( cgUInt32 referenceId, cgScene * scene ) : cgGroupNode( referenceId, scene )
{
    // Initialize variables to sensible defaults.
    mController = CG_NULL;

    // Default update rate to 'always' by default.
    mUpdateRate = cgUpdateRate::Always;

    // Set default instance identifier
    mInstanceIdentifier = cgString::format( _T("Actor%X"), referenceId );
}

//-----------------------------------------------------------------------------
//  Name : cgActorNode () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorNode::cgActorNode( cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform ) : cgGroupNode( referenceId, scene, init, initMethod, initTransform )
{
    // Initialize variables to sensible defaults.
    mController = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : ~cgActorNode () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgActorNode::~cgActorNode()
{
    // Release memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgActorNode::dispose( bool disposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release memory.
    if ( mController )
        mController->scriptSafeDispose();
    mController = CG_NULL;
    mTargets.clear();
    
    // Dispose base.
    if ( disposeBase )
        cgGroupNode::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate a new node of the required type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgActorNode::allocateNew( const cgUID & type, cgUInt32 referenceId, cgScene * scene )
{
    return new cgActorNode( referenceId, scene );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate a new node of the required type, cloning data from the node
/// specified.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectNode * cgActorNode::allocateClone( const cgUID & type, cgUInt32 referenceId, cgScene * scene, cgObjectNode * init, cgCloneMethod::Base initMethod, const cgTransform & initTransform )
{
    return new cgActorNode( referenceId, scene, init, initMethod, initTransform );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorNode::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ActorNode )
        return true;

    // Supported by base?
    return cgGroupNode::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
// Name : onNodeCreated () (Virtual)
/// <summary>
/// Can be overriden or called by derived class when the object is being 
/// created in order to perform required tasks and notify listeners.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorNode::onNodeCreated( const cgUID & objectType, cgCloneMethod::Base cloneMethod )
{
    // Call base class implementation.
    if ( !cgObjectNode::onNodeCreated( objectType, cloneMethod ) )
        return false;

    // Create the animation controller and initialize it.
    mController = new cgAnimationController( );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : onNodeLoading ( ) (Virtual)
/// <summary>
/// Virtual method called when the node is being reloaded from an existing
/// database entry rather than created for the first time.
/// </summary>
//-----------------------------------------------------------------------------
bool cgActorNode::onNodeLoading( const cgUID & objectType, cgWorldQuery * nodeData, cgSceneCell * parentCell, cgCloneMethod::Base cloneMethod )
{
    // Call base class implementation first.
    if ( !cgObjectNode::onNodeLoading( objectType, nodeData, parentCell, cloneMethod ) )
        return false;

    // Create the animation controller and initialize.
    mController = new cgAnimationController( );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentModified() (Virtual)
/// <summary>
/// When the component is modified, derived objects can call this method in 
/// order to notify any listeners of this fact.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorNode::onComponentModified( cgComponentModifiedEventArgs * e )
{
    // Call base class implementation last
    cgObjectNode::onComponentModified( e );
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Allow the object to perform necessary updates during the frame update 
/// phase.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorNode::update( cgFloat timeDelta )
{
    // Call base class implementation first to allow
    // the node itself to update / move, etc.
    cgObjectNode::update( timeDelta );

    // Now allow the animation controller to advance.
    if ( mController )
    {
        // In sandbox mode, make sure that we don't serialize updates
        // to the database. Otherwise, we can just advance as normal.
        if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
        {
            bool oldWrites = mParentScene->isSceneWritingEnabled();
            mParentScene->enableSceneWrites( false );
            mController->advanceTime( timeDelta, mTargets );

            // Make sure that we resolve any pending updates before
            // re-enabling scene writes, otherwise updates to the database
            // *will* occur after the update process is complete.
            mParentScene->resolvePendingUpdates();
            mParentScene->enableSceneWrites( oldWrites );
        
        } // End if sandbox
        else
        {
            // Advance the animation by the specified amount
            mController->advanceTime( timeDelta, mTargets );

        } // End if !sandbox
    
    } // End if has controller
}


//-----------------------------------------------------------------------------
// Name : onGroupRefAdded( ) (Virtual)
/// <summary>
/// Increment the number of objects in the scene that reference this group as 
/// their owner. If this number ever falls to zero then the group will be 
/// removed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgActorNode::onGroupRefAdded( cgObjectNode * node )
{
    // Add to the list of animation targets for this actor based on the 
    // node's instance identifier unless there is already one with a matching name.
    TargetMap::const_iterator itTarget = mTargets.find( node->getInstanceIdentifier() );
    if ( itTarget == mTargets.end() )
    {
        // Add to the map.
        mTargets[ node->getInstanceIdentifier() ] = node;

        // Listen for changes to the instance identifier.
        node->registerEventListener( static_cast<cgObjectNodeEventListener*>(this) );

    } // End if not found

    // Call base class implementation last.
    return cgGroupNode::onGroupRefAdded( node );
}

//-----------------------------------------------------------------------------
// Name : onGroupRefRemoved( ) (Virtual)
/// <summary>
/// Decrement the number of objects in the scene that reference this group as 
/// their owner. If this number ever falls to zero then the group will be 
/// removed.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgActorNode::onGroupRefRemoved( cgObjectNode * node )
{
    // Remove this node from the target map if it exists there
    TargetMap::iterator itTarget = mTargets.find( node->getInstanceIdentifier() );
    if ( itTarget != mTargets.end() && itTarget->second == node )
    {
        // Remove from the map.
        mTargets.erase( itTarget );

        // We no longer want to listen for changes to the instance identifier.
        node->unregisterEventListener( static_cast<cgObjectNodeEventListener*>(this) );
    
    } // End if found

    // Call base class implementation last (warning: may destroy this node).
    return cgGroupNode::onGroupRefRemoved( node );
}

//-----------------------------------------------------------------------------
// Name : onInstanceIdentifierChange( ) (Virtual)
/// <summary>
/// Triggered when one of the node's attached to this group changes its
/// instance identifier.
/// </summary>
//-----------------------------------------------------------------------------
void cgActorNode::onInstanceIdentifierChange( cgObjectNodeNameChangeEventArgs * e )
{
    // First find the old entry in our target list (if any).
    TargetMap::iterator itTarget = mTargets.find( e->oldName );

    // If we found it, and it's associated with the node that's changing, remove
    // the old entry and check to see if we can associate it with the new one.
    if ( itTarget != mTargets.end() && itTarget->second == e->node )
    {
        // Remove old entry.
        mTargets.erase( itTarget );

        // Are we free to associate this node with the new name?
        itTarget = mTargets.find( e->node->getInstanceIdentifier() );
        if ( itTarget == mTargets.end() )
        {
            // Add to the map.
            mTargets[ e->node->getInstanceIdentifier() ] = e->node;

        } // End if not found

    } // End if exists        
}

//-----------------------------------------------------------------------------
// Name : getAnimationController( )
/// <summary>
/// Retrieve the animation controller managing animations for this node.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationController * cgActorNode::getAnimationController( ) const
{
    return mController;
}

//-----------------------------------------------------------------------------
// Name : getAnimationTargets( )
/// <summary>
/// Retrieve the container that provides a mapping between a specific instance 
/// identifier, and an associated object node (if any) contained somewhere
/// below this actor in the hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
const cgActorNode::TargetMap & cgActorNode::getAnimationTargets( ) const
{
    return mTargets;
}