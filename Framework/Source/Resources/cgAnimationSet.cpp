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
// Name : cgAnimationSet.cpp                                                 //
//                                                                           //
// Desc : Contains resource data for an individual animation set, providing  //
//        animated transformation, event and property information where      //
//        available.                                                         //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationSet Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgAnimationSet.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>
#include <Math/cgMathTypes.h>
#include <Math/cgEulerAngles.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgAnimationSet::mInsertSet;
cgWorldQuery cgAnimationSet::mInsertTargetData;
cgWorldQuery cgAnimationSet::mUpdateFrameRate;
cgWorldQuery cgAnimationSet::mUpdateName;
cgWorldQuery cgAnimationSet::mLoadSet;
cgWorldQuery cgAnimationSet::mLoadTargetData;
cgWorldQuery cgAnimationSet::mLoadTargetControllers;

cgToDo( "Animation System", "Add support for skew!" );

///////////////////////////////////////////////////////////////////////////////
// cgAnimationSet Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationSet () (Constructor)
/// <summary>
/// cgAnimationSet Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSet::cgAnimationSet( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Set variables to sensible defaults
    mFirstFrame         = INT_MAX;
    mLastFrame         = INT_MIN;
    mFramesPerSecond  = 30.0f;

    // Loading and serialization
    mSourceRefId      = 0;
    mSetSerialized    = false;
    mDBDirtyFlags     = 0;
    mSuspendSerialization = false;
    
    // Cached resource responses
    mResourceType      = cgResourceType::AnimationSet;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationSet () (Constructor)
/// <summary>
/// cgAnimationSet Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSet::cgAnimationSet( cgUInt32 nReferenceId, cgWorld * pWorld, cgFloat fFrameRate ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Set variables to sensible defaults
    mFirstFrame         = INT_MAX;
    mLastFrame         = INT_MIN;
    mFramesPerSecond  = fFrameRate;

    // Loading and serialization
    mSourceRefId      = 0;
    mSetSerialized    = false;
    mDBDirtyFlags     = 0;
    mSuspendSerialization = false;
    
    // Cached resource responses
    mResourceType      = cgResourceType::AnimationSet;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationSet () (Constructor)
/// <summary>
/// cgAnimationSet Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSet::cgAnimationSet( cgUInt32 nReferenceId, cgWorld * pWorld, cgAnimationSet * pInit ) : cgWorldResourceComponent( nReferenceId, pWorld, pInit )
{
    // ToDo: 9999 - INSERT INTO DB!

    // Set variables to sensible defaults
    mFirstFrame         = pInit->mFirstFrame;
    mLastFrame         = pInit->mLastFrame;
    mFramesPerSecond  = pInit->mFramesPerSecond;
    mTargetData        = pInit->mTargetData;

    // Loading and serialization
    mSourceRefId      = 0;
    mSetSerialized    = false;
    mDBDirtyFlags     = 0;
    mSuspendSerialization = false;
    
    // Cached resource responses
    mResourceType      = cgResourceType::AnimationSet;
    mResourceLoaded   = pInit->mResourceLoaded;
    mResourceLost     = false;
    mCanEvict         = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationSet () (Constructor)
/// <summary>
/// cgAnimationSet Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSet::cgAnimationSet( cgUInt32 nReferenceId, cgWorld * pWorld, cgUInt32 nSourceRefId ) : cgWorldResourceComponent( nReferenceId, pWorld )
{
    // Set variables to sensible defaults
    mFirstFrame         = INT_MAX;
    mLastFrame         = INT_MIN;
    mFramesPerSecond  = 30.0f;
    
    // Loading and serialization
    mSourceRefId      = nSourceRefId;
    mSetSerialized    = false;
    mDBDirtyFlags     = 0;
    mSuspendSerialization = false;
    
    // Cached resource responses
    mResourceType      = cgResourceType::AnimationSet;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = (isInternalReference() == false);
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationSet () (Destructor)
/// <summary>
/// cgAnimationSet Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSet::~cgAnimationSet()
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
void cgAnimationSet::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources
    unloadResource();

    // Dispose base.
    if ( bDisposeBase == true )
        cgResource::dispose( true );
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
bool cgAnimationSet::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AnimationSetResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgAnimationSet::getDatabaseTable( ) const
{
    return _T("DataSources::AnimationSet");
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertSet.isPrepared() )
            mInsertSet.prepare( mWorld, _T("INSERT INTO 'DataSources::AnimationSet' VALUES(?1,?2,?3,?4,?5)"), true );
        if ( !mInsertTargetData.isPrepared() )
            mInsertTargetData.prepare( mWorld, _T("INSERT INTO 'DataSources::AnimationSet::Targets' VALUES(NULL,?1,?2)"), true );
        if ( !mUpdateFrameRate.isPrepared() )
            mUpdateFrameRate.prepare( mWorld, _T("UPDATE 'DataSources::AnimationSet' SET FrameRate=?1 WHERE RefId=?2"), true );
        if ( !mUpdateName.isPrepared() )
            mUpdateName.prepare( mWorld, _T("UPDATE 'DataSources::AnimationSet' SET Name=?1 WHERE RefId=?2"), true );
    
    } // End if sandbox

    // Read queries
    if ( !mLoadSet.isPrepared() )
        mLoadSet.prepare( mWorld, _T("SELECT * FROM 'DataSources::AnimationSet' WHERE RefId=?1"), true );
    if ( !mLoadTargetData.isPrepared() )
        mLoadTargetData.prepare( mWorld, _T("SELECT * FROM 'DataSources::AnimationSet::Targets' WHERE DataSourceId=?1"), true );
    if ( !mLoadTargetControllers.isPrepared() )
        mLoadTargetControllers.prepare( mWorld, _T("SELECT * FROM 'DataSources::AnimationSet::TargetControllers' WHERE TargetDataId=?1"), true );
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::loadResource( )
{    
    bool bSerializeNecessary = false;

    // Is resource already loaded?
    if ( isLoaded() )
        return true;

    // If we should be loading data, do so now.
    bool bResult = true;
    if ( mSourceRefId != 0 )
    {
        if ( !(bResult = loadSet( mSourceRefId, CG_NULL )) )
            return false;

        // If we are not simply wrapping the original database resource (i.e. the source 
        // reference identifier does not match this resource's actual identifier) then we 
        // potentially need to insert the loaded data into a new set of database records.
        if ( mSourceRefId != mReferenceId )
            bSerializeNecessary = true;
        
    } // End if loading
    else
    {
        // Any data that has been provided before this call (i.e. the
        // application has added geometry) needs to be written to
        // the database.
        bSerializeNecessary = true;

    } // End if not loading

    // If we are not an internal resource, write any requested data.
    if ( bSerializeNecessary && shouldSerialize() )
    {
        // Forcably resume serialization if suspended.
        bool bOldSuspendState = mSuspendSerialization;
        mSuspendSerialization = false;
        
        // Serialize
        if ( !serializeSet( ) )
            return false;

        // Restore suspended state.
        mSuspendSerialization = bOldSuspendState;

        // Mark resource as fully loaded.
        mResourceLoaded = true;

    } // End if serialize data.

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::unloadResource( )
{
    // Clear animation data (destroy controllers).
    TargetDataMap::iterator itTarget;
    for ( itTarget = mTargetData.begin(); itTarget != mTargetData.end(); ++itTarget )
    {
        TargetData & Data = itTarget->second;
        if ( Data.scaleController )
            Data.scaleController->scriptSafeDispose();
        if ( Data.rotationController )
            Data.rotationController->scriptSafeDispose();
        if ( Data.translationController )
            Data.translationController->scriptSafeDispose();

    } // Next Target
    mTargetData.clear();

    // Clear variables
    mName.clear();

    // Resource is no longer loaded, but if we are not internal
    // and the data WAS serialized, we can reload.
    if ( !isInternalReference() && mSetSerialized )
        mSourceRefId = mReferenceId;
    mResourceLoaded = false;
    
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : loadSet ()
/// <summary>
/// Load the specified animation set from the supplied database file.
/// Note : This function requires you to specify the resource manager into which
/// you would like all additional resources to be loaded if you are not loading 
/// this set directly via the cgResourceManager::LoadAnimationSet() methods.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::loadSet( cgUInt32 nSourceRefId, cgResourceManager * pManager /* = CG_NULL */ )
{
    // Dispose of any prior set data.
    dispose( false );

    // Set the resource manager if it has not already been set.
    if ( pManager && !mManager )
        mManager = pManager;

    // Handle exceptions
    try
    {
        // Reset frame bounds for later computation.
        mFirstFrame = INT_MAX;
        mLastFrame = INT_MIN;

        // If we are not simply wrapping the original database entry (i.e. the reference
        // identifier of this set resource doesn't match the source identifier) then we 
        // potentially need to re-serialize this data (i.e. clone it) to the database next 
        // time 'serializeSet()' is called (assuming we are not an internal resource). 
        // See 'loadResource()' for more information.
        bool bCloneData = ( !isInternalReference() && nSourceRefId != mReferenceId );
        
        // Load the primary mesh data entry.
        prepareQueries();
        mLoadSet.bindParameter( 1, nSourceRefId );
        if ( !mLoadSet.step() || !mLoadSet.nextRow() )
            throw cgExceptions::ResultException( _T("Failed to retrieve animation set data. World database has potentially become corrupt."), cgDebugSource() );

        // Retrieve the current 'soft' reference count for this component
        // if we are literally wrapping the database entry.
        if ( nSourceRefId == mReferenceId )
            mLoadSet.getColumn( _T("RefCount"), mSoftRefCount );

        // Retrieve the animation set details.
        mLoadSet.getColumn( _T("Name"), mName );
        mLoadSet.getColumn( _T("FrameRate"), mFramesPerSecond );

        // We're done with the data from the set query
        mLoadSet.reset();

        // Now load the target references.
        mLoadTargetData.reset();
        mLoadTargetData.bindParameter( 1, nSourceRefId );
        if ( !mLoadTargetData.step() )
            throw cgExceptions::ResultException( _T("Animation set data contained invalid or corrupt animation target information."), cgDebugSource() );

        // Iterate through each row returned and process the target data.
        cgString strTargetId, strControllerId;
        static const cgString SystemControllers[] = { _T("_sc"), _T("_r"), _T("_t") };
        for ( ; mLoadTargetData.nextRow(); )
        {
            // Get the target details.
            TargetData Data;
            cgUInt32 nDatabaseId;
            mLoadTargetData.getColumn( _T("TargetDataId"), nDatabaseId );
            mLoadTargetData.getColumn( _T("TargetIdentifier"), strTargetId );

            // Only associated with the original database row if we were not
            // instructed to clone the data. When the databaseId is zero,
            // this will force the automatic insertion of a new copy into
            // the database next time the set is serialized.
            if ( !bCloneData )
                Data.databaseId = nDatabaseId;
        
            // Find all assigned controllers for this target.
            mLoadTargetControllers.reset();
            mLoadTargetControllers.bindParameter( 1, nDatabaseId );
            if ( !mLoadTargetControllers.step() )
                throw cgExceptions::ResultException( _T("Animation set data contained invalid or corrupt animation target controller information."), cgDebugSource() );
            
            // Iterate through them.
            for ( ; mLoadTargetControllers.nextRow(); )
            {
                // Determine the type of the assigned controller.
                cgAnimationTargetControllerType::Base Type;
                mLoadTargetControllers.getColumn( _T("ControllerType"), (cgInt32&)Type );
                
                // Create a new controller instance and deserialize the data.
                cgAnimationTargetController * pController = cgAnimationTargetController::createInstance( Type );
                if ( !pController->deserialize( mLoadTargetControllers, bCloneData, mFirstFrame, mLastFrame ) )
                {
                    pController->scriptSafeDispose();
                    throw cgExceptions::ResultException( _T("Failed to load animation set controller or channel data."), cgDebugSource() );
                
                } // End if failed

                // Assign the controller
                mLoadTargetControllers.getColumn( _T("ControllerIdentifier"), strControllerId );
                if ( strControllerId == SystemControllers[0] )
                    Data.scaleController = pController;
                else if ( strControllerId == SystemControllers[1] )
                    Data.rotationController = pController;
                else if ( strControllerId == SystemControllers[2] )
                    Data.translationController = pController;
                else
                    pController->scriptSafeDispose();

            } // Next Controller

            // We're done with the controller data
            mLoadTargetControllers.reset();

            // Associate data with the target.
            mTargetData[ strTargetId ] = Data;

        } // Next Target Row

        // We're done with the target data.
        mLoadTargetData.reset();

        // Force serialization of all data next time 'serializeSet()' is called
        // if it was necessary for us to clone the animation set data.
        if ( bCloneData )
        {
            // We are not wrapping. Everything should be serialized.
            mSetSerialized        = false;
            mDBDirtyFlags         = AllDirty;

        } // End if not wrapping
        else
        {
            // We are wrapping. Everything is already serialized.
            mSetSerialized = true;
            mDBDirtyFlags  = 0;
        
        } // End if wrapping

        // The set is now prepared
        mResourceLoaded = true;

        // Success!
        return true;

    } // End Try Block

    catch ( const cgExceptions::ResultException & e )
    {
        // Release any pending read operations.
        mLoadSet.reset();
        mLoadTargetData.reset();
        mLoadTargetControllers.reset();
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;

    } // End Catch Block
    return true;
}

//-----------------------------------------------------------------------------
// Name : serializeSet() (Protected)
/// <summary>
/// Write or update the animation set data stored in the database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::serializeSet( )
{
    bool bTransaction = false;

    // Bail if we are not allowed to serialize any data.
    if ( !shouldSerialize() || mSuspendSerialization )
        return true;

    // Catch exceptions
    try
    {
        // Make sure database tables exist.
        if ( !mSetSerialized && !createTypeTables( RTID_AnimationSetResource ) )
            throw cgExceptions::ResultException( _T("cgWorldResourceComponent::createTypeTables"), cgDebugSource() );

        // If no animation data has yet been inserted, serialize everything.
        prepareQueries();
        if ( !mSetSerialized )
        {
            // Start a new transaction if we have not already.
            if ( !bTransaction )
                mWorld->beginTransaction( _T("serializeAnimationSet") );
            bTransaction = true;

            // Animation set entry does not exist at all at this stage so insert it.
            mInsertSet.bindParameter( 1, mReferenceId );
            mInsertSet.bindParameter( 2, (cgUInt32)0 ); // ToDo: 9999 - Flags
            mInsertSet.bindParameter( 3, mName );
            mInsertSet.bindParameter( 4, mFramesPerSecond );
            
            // Database ref count (just in case it has already been adjusted)
            mInsertSet.bindParameter( 5, mSoftRefCount );

            // Process!
            if ( !mInsertSet.step( true ) )
            {
                cgString strError;
                mInsertSet.getLastError( strError );
                throw cgExceptions::ResultException( cgString::format(_T("Failed to insert data for animation set resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

            } // End if failed

            // Target animation data should be serialized for the first time.
            mDBDirtyFlags = TargetDataDirty;
            
            // All other set properties have been serialized
            mSetSerialized  = true;
        
        } // End if not yet serialized
        else
        {
            // Update frame rate?
            if ( mDBDirtyFlags & FrameRateDirty )
            {
                // Start a new transaction if we have not already.
                if ( !bTransaction )
                    mWorld->beginTransaction( _T("serializeAnimationSet") );
                bTransaction = true;

                // Set has previously been serialized, but the frame rate data has been updated.
                mUpdateFrameRate.bindParameter( 1, mFramesPerSecond );
                mUpdateFrameRate.bindParameter( 2, mReferenceId );
                
                // Process!
                if ( !mUpdateFrameRate.step( true ) )
                {
                    cgString strError;
                    mUpdateFrameRate.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to update frame rate property for animation set resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

                // Frame rate has been serialized
                mDBDirtyFlags &= ~FrameRateDirty;

            } // End if frame rate dirty

            // Animation set name must be updated?
            if ( mDBDirtyFlags & NameDirty )
            {
                // Start a new transaction if we have not already.
                if ( !bTransaction )
                    mWorld->beginTransaction( _T("serializeAnimationSet") );
                bTransaction = true;
                
                // Set has previously been serialized, but the set name has been updated.
                mUpdateName.bindParameter( 1, mName );
                mUpdateName.bindParameter( 2, mReferenceId );
                
                // Process!
                if ( !mUpdateName.step( true ) )
                {
                    cgString strError;
                    mUpdateName.getLastError( strError );
                    throw cgExceptions::ResultException( cgString::format(_T("Failed to update name property for animation set resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                } // End if failed

                // Name has been serialized
                mDBDirtyFlags &= ~NameDirty;

            }// End if name dirty

        } // End if updating

        // Write animation set target data if it has been updated.
        if ( mDBDirtyFlags & TargetDataDirty )
        {
            static const cgString SystemChannels[] = { _T("_scale"), _T("_rotation"), _T("_translation") };

            // Start a new transaction if we have not already.
            if ( !bTransaction )
                mWorld->beginTransaction( _T("serializeAnimationSet") );
            bTransaction = true;

            // Process target data to see what (if anything) needs
            // to be updated for each individual target.
            TargetDataMap::iterator itData;
            for ( itData = mTargetData.begin(); itData != mTargetData.end(); ++itData )
            {
                TargetData & Data = itData->second;

                // If the target entry does not yet exist in the database
                // then create an entry to record its association with this set.
                if ( !Data.databaseId )
                {
                    mInsertTargetData.bindParameter( 1, getReferenceId() );
                    mInsertTargetData.bindParameter( 2, itData->first );
                    if ( !mInsertTargetData.step( true ) )
                    {
                        cgString strError;
                        mInsertTargetData.getLastError( strError );
                        throw cgExceptions::ResultException( cgString::format(_T("Failed to insert target data for animation set resource '0x%x'. Error: %s"), mReferenceId, strError.c_str()), cgDebugSource() );

                    } // End if failed
                    Data.databaseId = mInsertTargetData.getLastInsertId();

                } // End if no entry

                // Ask each of the target controllers assigned to the target entry to 
                // serialize itself and any of its channels.
                if ( Data.scaleController )
                    Data.scaleController->serialize( Data.databaseId, _T("_sc"), mWorld );
                if ( Data.rotationController )
                    Data.rotationController->serialize( Data.databaseId, _T("_r"), mWorld );
                if ( Data.translationController )
                    Data.translationController->serialize( Data.databaseId, _T("_t"), mWorld );

            } // Next target

            // Target data is no longer dirty
            mDBDirtyFlags &= ~TargetDataDirty;

        } // End if target data dirty

        // Commit any changes recorded.
        if ( bTransaction )
            mWorld->commitTransaction( _T("serializeAnimationSet") );

        // Success!
        return true;

    } // End Try

    catch ( const cgExceptions::ResultException & e )
    {
        // Roll back any transaction
        if ( bTransaction )
            mWorld->rollbackTransaction( _T("serializeAnimationSet") );
        
        // Log error and exit.
        cgAppLog::write( cgAppLog::Error, _T("%s\n"), e.toString().c_str() );
        return false;
    
    } // End catch

    return true;

}

//-----------------------------------------------------------------------------
//  Name : suspendSerialization ()
/// <summary>
/// Disable serialization until a subsequent call to 'resumeSerialization'. 
/// This is a useful form of optimization if you plan to make several updates
/// to the object. Optionally, the state of the object can be serialized
/// at this point (defaults to false).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::suspendSerialization( bool bFlush /* = false */ )
{
    if ( mSuspendSerialization ) 
        return true;

    // Flush on request.
    if ( bFlush )
    {
        if ( !serializeSet( ) )
            return false;
    
    } // End if flush

    // Update local member
    mSuspendSerialization = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : resumeSerialization ()
/// <summary>
/// Resume serialization from a previous call to 'suspendSerialization'. 
/// This is a useful form of optimization if you plan to make several updates
/// to the object. Optionally, the state of the object can be serialized
/// at this point (defaults to true).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::resumeSerialization( bool bFlush /* = true */ )
{
    if ( !mSuspendSerialization ) 
        return true;

    // Flush on request.
    if ( bFlush )
    {
        if ( !serializeSet( ) )
            return false;
    
    } // End if flush

    // Update local member
    mSuspendSerialization = false;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
// Name : isEmpty ( )
/// <summary>
/// Determine if the animation set contains any data.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::isEmpty( ) const
{
    return mTargetData.empty();
}

//-----------------------------------------------------------------------------
//  Name : setName ()
/// <summary>
/// Set the name of this animation set.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::setName( const cgString & strSetName )
{
    // Update the name.
    cgString strOldName = mName;
    mName = strSetName;

    // Name is now dirty.
    mDBDirtyFlags |= NameDirty;
    
    // Update database data as necessary.
    if ( !serializeSet( ) )
        mName = strOldName;
}

//-----------------------------------------------------------------------------
//  Name : getName ()
/// <summary>
/// Simply retrieve the name of this animation set.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgAnimationSet::getName() const
{
    return mName;
}

//-----------------------------------------------------------------------------
//  Name : getFrameRate ()
/// <summary>
/// Retrieve the rate at which this animation data is designed to play back.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat cgAnimationSet::getFrameRate( ) const
{
    return mFramesPerSecond;
}

//-----------------------------------------------------------------------------
//  Name : getTargetData ()
/// <summary>
/// Retrieve the animation target data contained in this set.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSet::TargetDataMap & cgAnimationSet::getTargetData() const
{
    return mTargetData;
}

//-----------------------------------------------------------------------------
//  Name : addScaleKey ()
/// <summary>
/// Add a new scale keyframe to the animation set for the specified 
/// animation target.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::addScaleKey( cgInt32 nFrame, const cgString & strTargetId, const cgVector3 & Scale )
{
    // If there is no scale controller, assign the default.
    TargetData & Data = mTargetData[ strTargetId ];
    if ( !Data.scaleController )
        Data.scaleController = new cgScaleXYZTargetController();

    // Initial insert will generate a 'linear' curve between last point
    // and the newly inserted point.
    switch ( Data.scaleController->getControllerType() )
    {
        case cgAnimationTargetControllerType::ScaleXYZ:
            ((cgScaleXYZTargetController*)Data.scaleController)->addLinearKey( nFrame, Scale );
            break;

        case cgAnimationTargetControllerType::UniformScale:
        {
            // Use the largest component.
            cgFloat fScale = max( Scale.x, Scale.y );
            fScale = max( fScale, Scale.z );
            ((cgUniformScaleTargetController*)Data.scaleController)->addLinearKey( nFrame, fScale );
            break;
        
        } // End switch UniformScale

    } // End switch type
    
    // Grow set bounds if necessary
    if ( nFrame < mFirstFrame )
        mFirstFrame = nFrame;
    if ( nFrame > mLastFrame )
        mLastFrame = nFrame;

    // Mark target data as dirty.
    mDBDirtyFlags |= TargetDataDirty;
    
    // Process
    serializeSet();
}

//-----------------------------------------------------------------------------
//  Name : addRotationKey ()
/// <summary>
/// Add a new rotation keyframe to the animation set for the specified 
/// animation target.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::addRotationKey( cgInt32 nFrame, const cgString & strTargetId, const cgQuaternion & Rotation )
{
    // If there is no rotation controller, assign the default.
    TargetData & Data = mTargetData[ strTargetId ];
    if ( !Data.rotationController )
        Data.rotationController = new cgEulerAnglesTargetController();

    // Initial insert will generate a 'linear' curve between last point
    // and the newly inserted point.
    switch ( Data.rotationController->getControllerType() )
    {
        case cgAnimationTargetControllerType::EulerAngles:
            ((cgEulerAnglesTargetController*)Data.rotationController)->addLinearKey( nFrame, Rotation );
            break;

        case cgAnimationTargetControllerType::Quaternion:
            ((cgQuaternionTargetController*)Data.rotationController)->addKey( nFrame, Rotation );
            break;

    } // End switch type
    
    // Grow set bounds if necessary
    if ( nFrame < mFirstFrame )
        mFirstFrame = nFrame;
    if ( nFrame > mLastFrame )
        mLastFrame = nFrame;

    // Mark target data as dirty.
    mDBDirtyFlags |= TargetDataDirty;
    
    // Process
    serializeSet();
}

//-----------------------------------------------------------------------------
//  Name : addTranslationKey ()
/// <summary>
/// Add a new translation keyframe to the animation set for the specified 
/// animation target.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::addTranslationKey( cgInt32 nFrame, const cgString & strTargetId, const cgVector3 & Translation )
{
    // If there is no translation controller, assign the default.
    TargetData & Data = mTargetData[ strTargetId ];
    if ( !Data.translationController )
        Data.translationController = new cgPositionXYZTargetController();

    // Initial insert will generate a 'linear' curve between last point
    // and the newly inserted point.
    switch ( Data.translationController->getControllerType() )
    {
        case cgAnimationTargetControllerType::PositionXYZ:
            ((cgPositionXYZTargetController*)Data.translationController)->addLinearKey( nFrame, Translation );
            break;

    } // End switch type
    
    // Grow set bounds if necessary
    if ( nFrame < mFirstFrame )
        mFirstFrame = nFrame;
    if ( nFrame > mLastFrame )
        mLastFrame = nFrame;

    // Mark target data as dirty.
    mDBDirtyFlags |= TargetDataDirty;
    
    // Process
    serializeSet();
}

//-----------------------------------------------------------------------------
//  Name : addSRTKey ()
/// <summary>
/// Add a new keyframe to the animation set for the specified target.
/// This is a utility method that allows each of the three scale,
/// rotation and translation animation components to be added in a single
/// call if available.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::addSRTKey( cgInt32 nFrame, const cgString & strTargetId, const cgVector3 & Scale, const cgQuaternion & Rotation, const cgVector3 & Translation )
{
    addScaleKey( nFrame, strTargetId, Scale );
    addRotationKey( nFrame, strTargetId, Rotation );
    addTranslationKey( nFrame, strTargetId, Translation );
}

//-----------------------------------------------------------------------------
//  Name : addMatrixKey ()
/// <summary>
/// Add a new keyframe to the animation set for the specified target.
/// This method accepts a single matrix which will be used to directly
/// apply the animation.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSet::addMatrixKey( cgInt32 nFrame, const cgString & strTargetId, const cgMatrix & mtxTransform )
{
    // Extract scale from matrix.
    cgVector3 Scale;
    Scale.x = cgVector3::length( (cgVector3&)mtxTransform._11 );
    Scale.y = cgVector3::length( (cgVector3&)mtxTransform._21 );
    Scale.z = cgVector3::length( (cgVector3&)mtxTransform._31 );

    // Same for translation.
    cgVector3 Translation;
    Translation.x = mtxTransform._41;
    Translation.y = mtxTransform._42;
    Translation.z = mtxTransform._43;

    // Generate rotation quaternion.
    cgQuaternion Rotation;
    cgMatrix m = mtxTransform;
    cgVector3::normalize( (cgVector3&)m._11, (cgVector3&)m._11 );
    cgVector3::normalize( (cgVector3&)m._21, (cgVector3&)m._21 );
    cgVector3::normalize( (cgVector3&)m._31, (cgVector3&)m._31 );
    ((cgVector3&)m._41) = cgVector3( 0,0,0);
    cgQuaternion::rotationMatrix( Rotation, m );
    cgQuaternion::normalize( Rotation, Rotation );

    // Add as key frames.
    addScaleKey( nFrame, strTargetId, Scale );
    addRotationKey( nFrame, strTargetId, Rotation );
    addTranslationKey( nFrame, strTargetId, Translation );
}

//-----------------------------------------------------------------------------
//  Name : getSRT ()
/// <summary>
/// Retrieve the scale, rotation and translation values for the specified
/// target at the specified position (in seconds).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::getSRT( cgDouble framePosition, const cgString & strTargetId, cgInt32 nMinFrame, cgInt32 nMaxFrame, cgVector3 & Scale, cgQuaternion & Rotation, cgVector3 & Translation )
{
    static const cgVector3 DefaultScale( 1, 1, 1 );
    static const cgQuaternion DefaultRotation( 0, 0, 0, 1 );
    static const cgVector3 DefaultTranslation( 0, 0, 0 );
    return getSRT( framePosition, strTargetId, nMinFrame, nMaxFrame, DefaultScale, DefaultRotation, DefaultTranslation, Scale, Rotation, Translation );
}

//-----------------------------------------------------------------------------
//  Name : getSRT ()
/// <summary>
/// Retrieve the scale, rotation and translation values for the specified
/// target at the specified position (in seconds).
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSet::getSRT( cgDouble fFramePosition, const cgString & strTargetId, cgInt32 nMinFrame, cgInt32 nMaxFrame, const cgVector3 & DefaultScale, const cgQuaternion & DefaultRotation, const cgVector3 & DefaultTranslation, cgVector3 & Scale, cgQuaternion & Rotation, cgVector3 & Translation )
{
    // Any target matching this identifier?
    TargetDataMap::iterator itTargetData = mTargetData.find( strTargetId );
    if ( itTargetData == mTargetData.end() )
    {
        Scale = DefaultScale;
        Rotation = DefaultRotation;
        Translation = DefaultTranslation;
        return false;
    
    } // End if no matching target

    // Cache reference for easy access
    TargetData & Data = itTargetData->second;

    // Compute the minimum and maximum period for this animation set.
    // It isn't necessarily the case that the set starts on frame 0.
    cgDouble fMinPeriod = (nMinFrame == 0x7FFFFFFF) ? (cgDouble)mFirstFrame : (cgDouble)nMinFrame;
    cgDouble fMaxPeriod = (nMaxFrame == 0x7FFFFFFF) ? (cgDouble)mLastFrame : (cgDouble)nMaxFrame;

    // Map the specified frame position into the "periodic" for this animation set.
    cgDouble fPeriodic = 0.0f;
    if ( fFramePosition > 0 )
        fPeriodic = fMinPeriod + fmod( fFramePosition, (fMaxPeriod - fMinPeriod) );
    else
        fPeriodic = fMinPeriod + ((fMaxPeriod - fMinPeriod) + fmod( fFramePosition, (fMaxPeriod - fMinPeriod) ));

    // Evaluate scale
    if ( Data.scaleController )
    {
        switch ( Data.scaleController->getControllerType() )
        {
            case cgAnimationTargetControllerType::ScaleXYZ:
                ((cgScaleXYZTargetController*)Data.scaleController)->evaluate( fPeriodic, Scale, DefaultScale );
                break;

            case cgAnimationTargetControllerType::UniformScale:
            {
                // Find the largest of the default scales for the.
                // evaluate method's default parameter.
                cgFloat fDefaultScale = max( DefaultScale.x, DefaultScale.y );
                fDefaultScale = max( fDefaultScale, DefaultScale.z );

                // Evaluate.
                ((cgUniformScaleTargetController*)Data.scaleController)->evaluate( fPeriodic, Scale.x, fDefaultScale );
                Scale.y = Scale.z = Scale.x;
                break;
            
            } // End case UniformScale

            default:
                Scale = DefaultScale;
                break;

        } // End switch type

    } // End if has scale
    else
        Scale = DefaultScale;

    // Evaluate rotation
    if ( Data.rotationController )
    {
        switch ( Data.rotationController->getControllerType() )
        {
            case cgAnimationTargetControllerType::EulerAngles:
            {
                cgEulerAngles Euler;
                ((cgEulerAnglesTargetController*)Data.rotationController)->evaluate( fPeriodic, Euler, DefaultRotation );
                Euler.toQuaternion( Rotation );
                break;
            
            } // End case EulerAngles

            case cgAnimationTargetControllerType::Quaternion:
                ((cgQuaternionTargetController*)Data.rotationController)->evaluate( fPeriodic, Rotation, DefaultRotation );
                break;
            
            default:
                Rotation = DefaultRotation;
                break;

        } // End switch type

    } // End if has scale
    else
        Rotation = DefaultRotation;

    // Evaluate translation
    if ( Data.translationController )
    {
        switch ( Data.translationController->getControllerType() )
        {
            case cgAnimationTargetControllerType::PositionXYZ:
                ((cgPositionXYZTargetController*)Data.translationController)->evaluate( fPeriodic, Translation, DefaultTranslation );
                break;
            
            default:
                Translation = DefaultTranslation;
                break;

        } // End switch type

    } // End if has scale
    else
        Translation = DefaultTranslation;

    // Success!
    return true;
}