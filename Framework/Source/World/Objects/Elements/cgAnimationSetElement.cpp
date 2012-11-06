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
// Name : cgAnimationSetElement.cpp                                          //
//                                                                           //
// Desc : Class that provides an object assigned animation set exposed as    //
//        an object sub-element. This provides the integration between the   //
//        application (such as the editing environment) and the relevant     //
//        sub-component of the selected object.                              //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgAnimationSetElement Module Includes
//-----------------------------------------------------------------------------
#include <World/Objects/Elements/cgAnimationSetElement.h>
#include <Resources/cgAnimationSet.h>

//-----------------------------------------------------------------------------
// Static Member Definitions
//-----------------------------------------------------------------------------
cgWorldQuery cgAnimationSetElement::mInsertAnimationSetRef;
cgWorldQuery cgAnimationSetElement::mUpdateAnimationSetRef;
cgWorldQuery cgAnimationSetElement::mLoadAnimationSetRef;

///////////////////////////////////////////////////////////////////////////////
// cgAnimationSetElement Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgAnimationSetElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::cgAnimationSetElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject ) : cgObjectSubElement( nReferenceId, pParentObject )
{
    // The animation set handle should manage the resource as an 'owner'
    // handle. This will ensure that soft (database) reference counting
    // will be considered and that the object ownership is correctly recorded.
    mAnimationSet.setOwnerDetails( this );
    mAnimationSet.enableDatabaseUpdate( (isInternalReference() == false) );
}

//-----------------------------------------------------------------------------
//  Name : cgAnimationSetElement () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::cgAnimationSetElement( cgUInt32 nReferenceId, cgWorldObject * pParentObject, cgObjectSubElement * pInit ) : cgObjectSubElement( nReferenceId, pParentObject, pInit )
{
    // The animation set handle should manage the resource as an 'owner'
    // handle. This will ensure that soft (database) reference counting
    // will be considered and that the object ownership is correctly recorded.
    mAnimationSet.setOwnerDetails( this );
    mAnimationSet.enableDatabaseUpdate( (isInternalReference() == false) );

    // Initialize variables to sensible defaults
    cgAnimationSetElement * pElement = (cgAnimationSetElement*)pInit;
    mAnimationSet = pElement->mAnimationSet;

    // ToDo: May need to COPY the data at some point
    /*// Get source animation set and ensure it is loaded.
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
    resources->addAnimationSet( &newSetHandle, newSet, cgResourceFlags::ForceNew, animationSet->getResourceName(), cgDebugSource() );*/
}

//-----------------------------------------------------------------------------
//  Name : ~cgAnimationSetElement () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgAnimationSetElement::~cgAnimationSetElement()
{
    // Clean up object resources.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgAnimationSetElement::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // We should simply 'disconnect' from the underlying set, never
    // physically remove the reference from the database.
    mAnimationSet.enableDatabaseUpdate( false );

    // Release the resources we're managing (if any).
    mAnimationSet.close();

    // Re-enable set handle database updates in case
    // the object is resurrected.
    mAnimationSet.enableDatabaseUpdate( (isInternalReference() == false) );
    
    // Dispose base class if requested.
    if ( bDisposeBase == true )
        cgObjectSubElement::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : allocateNew() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgAnimationSetElement::allocateNew( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject )
{
    return new cgAnimationSetElement( nReferenceId, pObject );
}

//-----------------------------------------------------------------------------
//  Name : allocateClone() (Static)
/// <summary>
/// Allocate an object sub-element of this specific type, cloned from the
/// provided element.
/// </summary>
//-----------------------------------------------------------------------------
cgObjectSubElement * cgAnimationSetElement::allocateClone( const cgUID & type, cgUInt32 nReferenceId, cgWorldObject * pObject, cgObjectSubElement * pInit )
{
    return new cgAnimationSetElement( nReferenceId, pObject, pInit );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSetElement::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_AnimationSetElement )
        return true;

    // Supported by base?
    return cgObjectSubElement::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getElementCategory () (Virtual)
/// <summary>
/// Retrieve the unique identifier for the sub-element category to which this
/// type belongs.
/// </summary>
//-----------------------------------------------------------------------------
const cgUID & cgAnimationSetElement::getElementCategory( ) const
{
    return OSECID_AnimationSets;
}

//-----------------------------------------------------------------------------
//  Name : setAnimationSet ()
/// <summary>
/// Supply the handle to the animation set represented by this sub-element 
/// instance.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSetElement::setAnimationSet( cgAnimationSetHandle animationSet )
{
    // Update database as necessary.
    if ( shouldSerialize() )
    {
        prepareQueries();
        cgUInt32 animationSetRefId = animationSet.getReferenceId();
        if ( animationSetRefId >= cgReferenceManager::InternalRefThreshold )
            animationSetRefId = 0;
        mUpdateAnimationSetRef.bindParameter( 1, animationSetRefId );
        mUpdateAnimationSetRef.bindParameter( 2, mReferenceId );
        if ( !mUpdateAnimationSetRef.step( true ) )
        {
            cgString strError;
            mUpdateAnimationSetRef.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to update data source reference for animation set object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );
            return;
        
        } // End if failed

    } // End if serialize

    // Update local member
    mAnimationSet = animationSet;
}

//-----------------------------------------------------------------------------
//  Name : getAnimationSet ()
/// <summary>
/// Get the handle to the animation set represented by this sub-element 
/// instance.
/// </summary>
//-----------------------------------------------------------------------------
const cgAnimationSetHandle & cgAnimationSetElement::getAnimationSet( ) const
{
    return mAnimationSet;
}

//-----------------------------------------------------------------------------
// Name : getDisplayName ()
/// <summary>
/// Get the name that will be displayed in the sub element list for this
/// particular object sub element. By default this will simply return 
/// "<Unnamed>", but derived classes can override this behavior when required.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgAnimationSetElement::getDisplayName( ) const
{
    if ( !mAnimationSet.isValid() )
        return cgObjectSubElement::getDisplayName();
    const cgAnimationSet * animationSet = mAnimationSet.getResourceSilent();
    const cgString & displayName = animationSet->getName();
    if ( displayName.empty() )
        return cgString::format( _T("<Unnamed Set> (0x%x)"), animationSet->getReferenceId() );
    else
        return displayName;
}

//-----------------------------------------------------------------------------
//  Name : getDatabaseTable() (Virtual)
/// <summary>
/// Retrieve the name of the primary type database table for this type.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgAnimationSetElement::getDatabaseTable( ) const
{
    return _T("ObjectSubElements::AnimationSet");
}

//-----------------------------------------------------------------------------
// Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows the component to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSetElement::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    // Insert the new object sub-element.
    if ( !insertComponentData( ) )
        return false;

    // Call base class implementation last.
    return cgObjectSubElement::onComponentCreated( e );
}

//-----------------------------------------------------------------------------
// Name : insertComponentData()
/// <summary>
/// Insert new records into the world database to represent this object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgAnimationSetElement::insertComponentData( )
{
    if ( shouldSerialize() )
    {
        // Open a new transaction to allow us to roll-back on failure.
        mWorld->beginTransaction( _T("AnimationSetElement::insertComponentData") );

        // Update database.
        prepareQueries();
        mInsertAnimationSetRef.bindParameter( 1, mReferenceId );
        mInsertAnimationSetRef.bindParameter( 2, mAnimationSet.getReferenceId() );
        mInsertAnimationSetRef.bindParameter( 3, mSoftRefCount );
        
        // Execute
        if ( !mInsertAnimationSetRef.step( true ) )
        {
            cgString strError;
            mInsertAnimationSetRef.getLastError( strError );
            cgAppLog::write( cgAppLog::Error, _T("Failed to insert data for animation set object sub-element '0x%x' into database. Error: %s\n"), mReferenceId, strError.c_str() );
            mWorld->rollbackTransaction( _T("AnimationSetElement::insertComponentData") );
            return false;
        
        } // End if failed

        // Commit changes
        mWorld->commitTransaction( _T("AnimationSetElement::insertComponentData") );

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
bool cgAnimationSetElement::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    // Load the animation set reference.
    prepareQueries();
    mLoadAnimationSetRef.bindParameter( 1, e->sourceRefId );
    if ( !mLoadAnimationSetRef.step( ) || !mLoadAnimationSetRef.nextRow() )
    {
        // Log any error.
        cgString strError;
        if ( !mLoadAnimationSetRef.getLastError( strError ) )
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for animation set object sub-element '0x%x'. World database has potentially become corrupt.\n"), mReferenceId );
        else
            cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve data for animation set object sub-element '0x%x'. Error: %s\n"), mReferenceId, strError.c_str() );

        // Release any pending read operation.
        mLoadAnimationSetRef.reset();
        return false;
    
    } // End if failed
    
    // Allow component class to access the data we just retrieved.
    e->componentData = &mLoadAnimationSetRef;

    // Load parameters
    cgUInt32 animationSetRefId = 0;
    mLoadAnimationSetRef.getColumn( _T("DataSourceId"), animationSetRefId );

    // Load the referenced animation set if valid.
    if ( animationSetRefId )
    {
        // If we're cloning, we potentially need a new copy of the animation set data,
        // otherwise we can load it as a straight forward wrapped resource.
        cgUInt32 flags = 0;
        bool internalAnimationSet = false;
        if ( e->cloneMethod == cgCloneMethod::Copy )
        {
            internalAnimationSet = isInternalReference();
            flags = cgResourceFlags::ForceNew;
        
        } // End if copying

        // Load the set.
        cgAnimationSetHandle animationSet;
        cgResourceManager * resources = mWorld->getResourceManager();
        if ( !resources->loadAnimationSet( &animationSet, mWorld, animationSetRefId, internalAnimationSet, flags, cgDebugSource() ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Failed to instantiate or load animation set data source '0x%x' for object sub-element '0x%x'. Refer to any previous errors for more information.\n"), animationSetRefId, mReferenceId );
            
            // Release any pending read operations.
            mLoadAnimationSetRef.reset();
            return false;

        } // End if failed

        // Store the animation set in this element. Temporarily disable database updates,
        // we are simply 'reconnecting' to this resource.
        mAnimationSet.enableDatabaseUpdate( false, false );
        mAnimationSet = animationSet;
        mAnimationSet.enableDatabaseUpdate( isInternalReference() == false, false );

    } // End if load set

    // Call base class implementation to read remaining data.
    if ( !cgObjectSubElement::onComponentLoading( e ) )
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
void cgAnimationSetElement::onComponentDeleted( )
{
    // Remove our physical reference to any animation data. Full database 
    // update  and potentially removal should be allowed to occur (i.e. a 
    // full  de-reference rather than a simple disconnect) in this case.
    mAnimationSet.close();
    
    // Call base class implementation last.
    cgObjectSubElement::onComponentDeleted( );
}

//-----------------------------------------------------------------------------
// Name : prepareQueries ( ) (Protected)
/// <summary>
/// Prepare any cached world queries as necessary.
/// </summary>
//-----------------------------------------------------------------------------
void cgAnimationSetElement::prepareQueries()
{
    // Prepare the SQL statements as necessary.
    if ( cgGetSandboxMode() == cgSandboxMode::Enabled )
    {
        if ( !mInsertAnimationSetRef.isPrepared() )
            mInsertAnimationSetRef.prepare( mWorld, _T("INSERT INTO 'ObjectSubElements::AnimationSet' VALUES(?1,?2,?3)"), true );
        if ( !mUpdateAnimationSetRef.isPrepared() )
            mUpdateAnimationSetRef.prepare( mWorld, _T("UPDATE 'ObjectSubElements::AnimationSet' SET DataSourceId=?1 WHERE RefId=?2"), true );
        
    } // End if sandbox

    // Read queries
    if ( !mLoadAnimationSetRef.isPrepared() )
        mLoadAnimationSetRef.prepare( mWorld, _T("SELECT * FROM 'ObjectSubElements::AnimationSet' WHERE RefId=?1"), true );
}