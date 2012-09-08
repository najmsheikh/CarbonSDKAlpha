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
cgWorldQuery cgActorObject::mInsertSetReference;
cgWorldQuery cgActorObject::mLoadSetReferences;

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
    
    // Clone any referenced animation sets.
    if ( initMethod == cgCloneMethod::Copy )
    {
        for ( size_t i = 0; i < object->mAnimationSets.size(); ++i )
        {
            // Get source animation set and ensure it is loaded.
            cgAnimationSet * animationSet = object->mAnimationSets[i].getResource( true );

            // Allocate a new animation set into which we will copy the data.
            cgAnimationSet * newSet = CG_NULL;
            if ( mWorld )
                newSet = new cgAnimationSet( mWorld->generateRefId( isInternalReference() ), mWorld, animationSet );
            else
                newSet = new cgAnimationSet( cgReferenceManager::generateInternalRefId(), CG_NULL, animationSet );

            // Add for management.
            cgAnimationSetHandle newSetHandle;
            cgResourceManager * resources = cgResourceManager::getInstance();
            resources->addAnimationSet( &newSetHandle, newSet, cgResourceFlags::ForceNew, animationSet->getResourceName(), cgDebugSource() );

            // Store! Since we're creating a new connection to existing information, the 
            // database ref count SHOULD be incremented when we attach (i.e. we're not
            // simply reconnecting as we would be during a load) assuming we are not an 
            // internal reference outselves.
            mAnimationSets.push_back( cgAnimationSetHandle( (isInternalReference() == false), this ) );
            mAnimationSets.back() = newSetHandle;

        } // Next animation set
    
    } // End if Copy
    else if ( initMethod == cgCloneMethod::DataInstance )
    {
        for ( size_t i = 0; i < object->mAnimationSets.size(); ++i )
        {
            // Store! Since we're creating a new connection to existing information, the 
            // database ref count SHOULD be incremented when we attach (i.e. we're not
            // simply reconnecting as we would be during a load) assuming we are not an 
            // internal reference outselves.
            mAnimationSets.push_back( cgAnimationSetHandle( (isInternalReference() == false), this ) );
            mAnimationSets.back() = object->mAnimationSets[i];

        } // Next animation set

    } // End if DataInstance
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

    // Disconnect from referenced animation sets. We should always
    // simply 'disconnect' in the dispose method, never physically 
    // remove the reference from the database. Complete 'removal'
    // should be handled in the object's deleted event.
    for ( size_t i = 0; i < mAnimationSets.size(); ++i )
        mAnimationSets[i].enableDatabaseUpdate( false );
    mAnimationSets.clear();
    
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

        // Insert existing animation set references.
        for ( size_t i = 0; i < mAnimationSets.size(); ++i )
        {
            // Skip internal animation sets. We can't reload them.
            cgAnimationSet * animationSet = mAnimationSets[i].getResource(false);
            if ( !animationSet || animationSet->isInternalReference() )
                continue;

            // Insert the reference.
            mInsertSetReference.bindParameter( 1, getReferenceId() );
            mInsertSetReference.bindParameter( 2, animationSet->getReferenceId() );
        
            // Execute
            if ( !mInsertSetReference.step( true ) )
            {
                cgString error;
                mInsertSetReference.getLastError( error );
                cgAppLog::write( cgAppLog::Error, _T("Failed to insert animation set data reference for actor object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );
                mWorld->rollbackTransaction( _T("ActorObject::insertComponentData") );
                return false;
            
            } // End if failed

        } // Next animation set

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

    // Load the animation set references.
    cgResourceManager * resources = mWorld->getResourceManager();
    mLoadSetReferences.bindParameter( 1, e->sourceRefId );
    if ( !mLoadSetReferences.step() )
    {
        // Log any error.
        cgString error;
        if ( !mLoadSetReferences.getLastError( error ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve animation set references for actor object '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve animation set references for actor object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

        // Release any pending read operation.
        mLoadActor.reset();
        mLoadSetReferences.reset();
        return false;

    } // End if failed
    for ( ; mLoadSetReferences.nextRow(); )
    {
        // If we're cloning, we potentially need a new copy of the mesh data,
        // otherwise we can load it as a straight forward wrapped resource.
        cgUInt32 flags = 0;
        bool internalAnimationSet = false;
        if ( e->cloneMethod == cgCloneMethod::Copy )
        {
            internalAnimationSet = isInternalReference();
            flags = cgResourceFlags::ForceNew;
        
        } // End if copying

        // Load the set.
        cgUInt32 animationSetRefId;
        cgAnimationSetHandle animationSet;
        mLoadSetReferences.getColumn( _T("DataSourceId"), animationSetRefId );
        if ( !resources->loadAnimationSet( &animationSet, mWorld, animationSetRefId, internalAnimationSet, flags, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to instantiate or load animation set data source '0x%x' for actor object '0x%x'. Refer to any previous errors for more information.\n"), animationSetRefId, mReferenceId );
            
            // Release any pending read operations.
            mLoadActor.reset();
            mLoadSetReferences.reset();
            return false;

        } // End if failed

        // Store! Since we're reloading prior information, the database ref count 
        // should NOT be incremented when we attach (i.e. we're just reconnecting),
        // but we should be marked as an owner.
        mAnimationSets.push_back( cgAnimationSetHandle() );
        mAnimationSets.back().setOwnerDetails( this );
        mAnimationSets.back() = animationSet;

        // Restore handle database update options.
        mAnimationSets.back().enableDatabaseUpdate( (isInternalReference() == false) );

    } // Next reference

    // We're done reading references.
    mLoadSetReferences.reset();

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
    // Remove our physical reference to any animation set data. Full database 
    // update and potential removal should be allowed to occur (i.e. a full 
    // de-reference rather than a simple disconnect) in this case. By clearing
    // the array, the resource handle should automatically close the resource
    // and trigger its automatic database update behavior.
    mAnimationSets.clear();
    
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
        if ( !mInsertSetReference.isPrepared() )
            mInsertSetReference.prepare( mWorld, _T("INSERT INTO 'Objects::Actor::AnimationSets' VALUES(NULL,?1,?2)"), true );
        if ( !mUpdateOpen.isPrepared() )
            mUpdateOpen.prepare( mWorld, _T("UPDATE 'Objects::Actor' SET Open=?1 WHERE RefId=?2"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadActor.isPrepared() )
        mLoadActor.prepare( mWorld, _T("SELECT * FROM 'Objects::Actor' WHERE RefId=?1"), true );
    if ( !mLoadSetReferences.isPrepared() )
        mLoadSetReferences.prepare( mWorld, _T("SELECT * FROM 'Objects::Actor::AnimationSets' WHERE ObjectId=?1"), true );
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

    // Create space for a new animation set in the list.
    // The set handle should manage the resource as an 'owner' handle. 
    // This will ensure that soft (database) reference counting will be
    // considered and that the object ownership is correctly recorded.
    mAnimationSets.push_back( cgAnimationSetHandle() );
    mAnimationSets.back().enableDatabaseUpdate( (isInternalReference() == false) );
    mAnimationSets.back().setOwnerDetails( this );

    // Take ownership of the set.
    mAnimationSets.back() = animationSet;

    // Insert new reference into world database only if the animation set is
    // not an internal reference.
    if ( shouldSerialize() && !animationSet->isInternalReference() )
    {
        prepareQueries();
        mInsertSetReference.bindParameter( 1, getReferenceId() );
        mInsertSetReference.bindParameter( 2, animationSet->getReferenceId() );
        
        // Execute
        if ( !mInsertSetReference.step( true ) )
        {
            cgString error;
            mInsertSetReference.getLastError( error );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert animation set data reference for actor object '0x%x'. Error: %s\n"), mReferenceId, error.c_str() );

            // Remove our reference to the animation set in memory 
            // (this should also remove animation set from the database 
            // entirely if this was the last DB reference) and fail.
            mAnimationSets.pop_back();
            return false;
        
        } // End if failed
    
    } // End if serialize

    // Notify listeners that object data has changed.
    static const cgString modificationContext = _T("AnimationSetAdded");
    onComponentModified( &cgComponentModifiedEventArgs( modificationContext ) );

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
    for ( size_t i = 0; i < mAnimationSets.size(); ++i )
    {
        const cgAnimationSet * animationSet = mAnimationSets[i].getResourceSilent();
        if ( animationSet && animationSet->getName() == name )
            return mAnimationSets[i];
    
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
    return (cgUInt32)mAnimationSets.size();
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSet()
/// <summary>
/// Retrieve the animation set at the specified index location.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSetHandle & cgActorObject::getAnimationSet( cgUInt32 index ) const
{
    if ( index >= (cgUInt32)mAnimationSets.size() )
        return cgAnimationSetHandle::Null;
    return mAnimationSets[index];
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
    // What was modified?
    if ( e->context == _T("AnimationSetAdded") )
    {
        
    } // End if AnimationSetAdded

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
        /*static std::map<cgActorNode*,bool> SetSet;
        if ( SetSet.find( this ) == SetSet.end() )
        {
            if ( !((cgActorObject*)m_pReferencedObject)->mAnimationSets.empty() )
            {
                mController->SetTrackAnimationSet( 0, ((cgActorObject*)m_pReferencedObject)->mAnimationSets.front() );
                SetSet[this] = true;
            }
        }*/

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
            mController->advanceTime( timeDelta, mTargets );

            for ( TargetMap::iterator itTarget = mTargets.begin(); itTarget != mTargets.end(); ++itTarget )
                ((cgObjectNode*)itTarget->second)->nodeUpdated(0,0);

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