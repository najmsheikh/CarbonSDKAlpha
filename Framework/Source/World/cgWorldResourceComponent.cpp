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
// File : cgWorldResourceComponent.cpp                                       //
//                                                                           //
// Desc : Contains a specialized base class, similar to cgWorldComponent,    //
//        from which all database driven resources should derive. Such       //
//        components might include meshes and other data sources that wish   //
//        to participate in data instancing.                                 //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgWorldResourceComponent Module Includes
//-----------------------------------------------------------------------------
#include <World/cgWorldResourceComponent.h>
#include <World/cgWorldComponent.h>
#include <World/cgWorld.h>
#include <World/cgWorldQuery.h>
#include <System/cgMessageTypes.h>
#include <System/cgStringUtility.h>
#include <System/cgExceptions.h>

//-----------------------------------------------------------------------------
// Name : cgWorldResourceComponent() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgWorldResourceComponent::cgWorldResourceComponent( cgUInt32 nReferenceId, cgWorld * pWorld ) : cgResource( nReferenceId )
{
    // Initialize variables to sensible defaults
    mComponentTypeId   = 0;
    mWorld             = pWorld;
}

//-----------------------------------------------------------------------------
// Name : cgWorldResourceComponent() (Constructor)
/// <summary>Object class constructor.</summary>
//-----------------------------------------------------------------------------
cgWorldResourceComponent::cgWorldResourceComponent( cgUInt32 nReferenceId, cgWorld * pWorld, cgWorldResourceComponent * pInit ) : cgResource( nReferenceId )
{
    // Clear variables first in case exception is thrown.
    mComponentTypeId = 0;
    mWorld           = CG_NULL;

    // Ensure this is a valid operation
    if ( pInit == CG_NULL || pInit->queryReferenceType( this->getReferenceType() ) == false )
        throw cgExceptions::ResultException( _T("Unable to clone. Specified component is of incompatible type."), cgDebugSource() );

    // Initialize variables to sensible defaults
    mComponentTypeId   = pInit->mComponentTypeId;
    mWorld             = pWorld;
}

//-----------------------------------------------------------------------------
// Name : ~cgWorldResourceComponent() (Destructor)
/// <summary>Clean up any resources being used.</summary>
//-----------------------------------------------------------------------------
cgWorldResourceComponent::~cgWorldResourceComponent()
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
void cgWorldResourceComponent::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Dispose base.
    if ( bDisposeBase )
        cgResource::dispose( true );
    else
        mDisposing = false;
}


//-----------------------------------------------------------------------------
//  Name : createTypeTables() (Virtual)
/// <summary>
/// When this type of component is being inserted into the database for the
/// first time, this method will be called in order to allow it to create any 
/// necessary tables.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldResourceComponent::createTypeTables( const cgUID & TypeIdentifier )
{
    // Bail if tables have already been created for this type in the
    // specified world database.
    if ( mWorld->componentTablesExist( TypeIdentifier ) == true )
        return true;

    // Base class automatically attempts to create the layout based on
    // the contents of the file 'sys://Layout/{TypeIdentifier}' by default. This can
    // be overriden however if the derived class deems it necessary, for
    // instance to trigger the creation of a full chain of derived
    // component tables.

    // Retrieve the statements that define the required file layout.
    cgString strFile   = _T("sys://Layout/") + cgStringUtility::toString( TypeIdentifier, _T("B") );
    cgString strLayout = cgFileSystem::loadStringFromStream( strFile );
    if ( strLayout.empty() == true )
    {
        cgAppLog::write( cgAppLog::Debug | cgAppLog::Error, _T("Unable to determine file layout for requested world component type.\n") );
        return false;
    
    } // End if not found

    // Create tables / indexes / triggers as required.
    if ( mWorld->executeQuery( strLayout, false ) == false )
        return false;

    // Tables have been created.
    mWorld->componentTablesCreated( TypeIdentifier );

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
void cgWorldResourceComponent::onComponentModified( cgComponentModifiedEventArgs * e )
{
    cgMessage                   Msg;
    EventListenerList::iterator itListener;

    // Trigger 'onComponentModified' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        ((cgWorldComponentEventListener*)(*itListener))->onComponentModified( e );
    
    // Build the message for anyone listening via messaging system
    Msg.messageId      = cgSystemMessages::WorldComponent_Modified;
    Msg.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_WorldComponent, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onComponentCreated() (Virtual)
/// <summary>
/// When the component is first created, it needs to be inserted fully into the
/// world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldResourceComponent::onComponentCreated( cgComponentCreatedEventArgs * e )
{
    cgMessage                   Msg;
    EventListenerList::iterator itListener;

    // Record component type identifier.
    mComponentTypeId = e->componentTypeId;
    
    // Trigger 'onComponentCreated' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        ((cgWorldComponentEventListener*)(*itListener))->onComponentCreated( e );
    
    // Build the message for anyone listening via messaging system
    Msg.messageId      = cgSystemMessages::WorldComponent_Created;
    Msg.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_WorldComponent, &Msg );

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
bool cgWorldResourceComponent::onComponentLoading( cgComponentLoadingEventArgs * e )
{
    cgMessage                   Msg;
    EventListenerList::iterator itListener;
    bool                        bOwnsQuery = false;

    // Record component type identifier.
    mComponentTypeId = e->componentTypeId;

    // If our reference identifier doesn't match the source identifier, we were
    // cloned. Otherwise, If component data was not supplied (or we fail to retrieve
    // it), select it.
    if ( mReferenceId == e->sourceRefId )
    {
        if ( !e->componentData || !e->componentData->getColumn( _T("RefCount"), mSoftRefCount ) )
        {
            cgString strSQL = cgString::format( _T("SELECT RefCount FROM '%s' WHERE RefId=%i"), getDatabaseTable().c_str(), mReferenceId );
            e->componentData = new cgWorldQuery( mWorld, strSQL );
            bOwnsQuery = true;
            if ( !e->componentData->step() || !e->componentData->nextRow() )
            {
                e->componentData->reset();
                delete e->componentData;
                cgAppLog::write( cgAppLog::Error, _T("Failed to retrieve original database reference count during loading of world component.\n") );
                return false;

            } // End if failed

            // Retrieve the new reference count
            e->componentData->getColumn( _T("RefCount"), mSoftRefCount );
        
        } // End if no component data

    } // End if !cloned

    // We're done with this statement.
    if ( e->componentData )
        e->componentData->reset();
    if ( bOwnsQuery )
    {
        delete e->componentData;
        e->componentData = CG_NULL;
    
    } // End if owned
    
    // Trigger 'onComponentLoading' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        ((cgWorldComponentEventListener*)(*itListener))->onComponentLoading( e );
    
    // Build the message for anyone listening via messaging system
    Msg.messageId      = cgSystemMessages::WorldComponent_Loading;
    Msg.messageData    = e;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_WorldComponent, &Msg );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : onComponentDeleted() (Virtual)
/// <summary>
/// When the component is removed from the world, all of its rows needs to be
/// removed from the world database. This virtual method allows it to do so.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldResourceComponent::onComponentDeleted( )
{
    cgMessage                   Msg;
    EventListenerList::iterator itListener;

    // Remove this component from the database in its entirety.
    // If any additional tables other than the primary reported
    // by 'DatabaseTable' exist for this component and need to also
    // have records removed, the database author must perform this
    // manually through the use of automatic triggers on the main table.
    if ( shouldSerialize() == true )
    {
        cgString strSQL = cgString::format( _T("DELETE FROM '%s' WHERE RefId=%i"), this->getDatabaseTable().c_str(), mReferenceId );
        mWorld->executeQuery( strSQL, false );

    } // End if !internal

    // Trigger 'onComponentLoading' of all listeners (duplicate list in case
    // it is altered in response to event).
    EventListenerList Listeners = mEventListeners;
    for ( itListener = Listeners.begin(); itListener != Listeners.end(); ++itListener )
        ((cgWorldComponentEventListener*)(*itListener))->onComponentDeleted( );
    
    // Build the message for anyone listening via messaging system
    Msg.messageId      = cgSystemMessages::WorldComponent_Deleted;
    Msg.messageData    = CG_NULL;

    // Send to anyone interested
    cgReferenceManager::sendMessageToGroup( getReferenceId(), cgSystemMessageGroups::MGID_WorldComponent, &Msg );
}

//-----------------------------------------------------------------------------
//  Name : onReferenceAdded () (Virtual)
/// <summary>
/// Triggered whenever a new reference to this target is added by a holder.
/// Derived objects may override this virtual if they need to be informed
/// for any reason.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldResourceComponent::onReferenceAdded( cgReference * pHolder, cgInt32 nHolderReferences, bool bReconnecting )
{
    // If holder was truly fully adding a database reference
    // rather than simply reconnecting to the component then
    // we should increment our reference count.
    if ( bReconnecting == false && shouldSerialize() == true )
    {
        cgString strSQL = cgString::format( _T("UPDATE '%s' SET RefCount=%i WHERE RefId=%i"), this->getDatabaseTable().c_str(), mSoftRefCount, mReferenceId );
        mWorld->executeQuery( strSQL, false );
    
    } // End if adding DB reference.

    // Call base class implementation last.
    cgResource::onReferenceAdded( pHolder, nHolderReferences, bReconnecting );
}

//-----------------------------------------------------------------------------
// Name : onReferenceRemoved () (Virtual)
/// <summary>
/// Triggered whenever an existing reference to this target is removed by a
/// holder. Derived objects may override this virtual if they need to be
/// informed for any reason.
/// </summary>
//-----------------------------------------------------------------------------
void cgWorldResourceComponent::onReferenceRemoved( cgReference * pHolder, cgInt32 nHolderReferences, bool bDisconnecting )
{
    // If holder was truly fully removing a database reference
    // rather than simply disconnecting from the component then
    // we should decrement our reference count.
    if ( bDisconnecting == false && shouldSerialize() == true )
    {
        // If this is the last database reference, we should remove ourselves
        // entirely from the database. Otherwise just update the ref count.
        if ( mSoftRefCount == 0 )
        {
            onComponentDeleted( );
        
        } // End if delete
        else
        {
            cgString strSQL = cgString::format( _T("UPDATE '%s' SET RefCount=%i WHERE RefId=%i"), this->getDatabaseTable().c_str(), mSoftRefCount, mReferenceId );
            mWorld->executeQuery( strSQL, false );
        
        } // End if update
    
    } // End if removing DB reference.

    // Call base class implementation last.
    cgResource::onReferenceRemoved( pHolder, nHolderReferences, bDisconnecting );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldResourceComponent::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_WorldResourceComponent )
        return true;

    // Base class supports?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : getLocalTypeId()
/// <summary>
/// Retrieve this component's integer type identifier as it exists local to
/// the world database.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgWorldResourceComponent::getLocalTypeId( ) const
{
    return mComponentTypeId;
}

//-----------------------------------------------------------------------------
//  Name : getParentWorld ( )
/// <summary>
/// Retrieve the world to which this component belongs.
/// </summary>
//-----------------------------------------------------------------------------
cgWorld * cgWorldResourceComponent::getParentWorld( ) const
{
    return mWorld;
}

//-----------------------------------------------------------------------------
// Name : shouldSerialize ( )
/// <summary>
/// Determine if we should serialize our information to the currently open
/// world database.
/// </summary>
//-----------------------------------------------------------------------------
bool cgWorldResourceComponent::shouldSerialize( ) const
{
    return ((cgGetSandboxMode() == cgSandboxMode::Enabled) && !isInternalReference() && 
            !mDisposing && mReferenceId);
}