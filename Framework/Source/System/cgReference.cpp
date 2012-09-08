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
// Name : cgReference.cpp                                                    //
//                                                                           //
// Desc : Base class which provides standardized reference counting          //
//        behaviors in addition to recording information about which         //
//        object instances are holding each reference.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgReference Module Includes
//-----------------------------------------------------------------------------
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
//  Name : cgReference() (Constructor)
/// <summary>
/// Object class constructor.
/// </summary>
//-----------------------------------------------------------------------------
// ToDo: Commented to aid with porting. Add again?
/*cgReference::cgReference( ) : cgEventDispatcher( )
{
    // Initialize variables to sensible defaults
    mReferenceId  = 0;
    mLiveRefCount = 0;
    mSoftRefCount = 0;
    mDisposed     = false;
    mDisposing    = false;
}*/

//-----------------------------------------------------------------------------
//  Name : cgReference() (Constructor)
/// <summary>
/// Object class constructor.
/// </summary>
//-----------------------------------------------------------------------------
cgReference::cgReference( cgUInt32 referenceId ) : cgEventDispatcher( )
{
    // Initialize variables to sensible defaults
    mReferenceId  = referenceId;
    mLiveRefCount = 0;
    mSoftRefCount = 0;
    mDisposed     = false;
    mDisposing    = false;

    // Register self with the reference manager.
    registerSelf( );
}

//-----------------------------------------------------------------------------
//  Name : ~cgReference() (Destructor)
/// <summary>
/// Clean up any resources being used.
/// </summary>
//-----------------------------------------------------------------------------
cgReference::~cgReference()
{
    // Clean up.
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgReference::dispose( bool disposeBase )
{
    // Iterate through reference map and detach us from any holder
    // in case we were prematurely destroyed.
    ReferenceMap::iterator itHolder = mReferencedBy.begin();
    for ( ; itHolder != mReferencedBy.end(); ++itHolder )
    {
        cgReference * reference = (cgReference*)itHolder->first;
        for ( cgInt32 i = 0; i < itHolder->second.refCount; ++i )
            reference->removeRefHold( this, true );

    } // Next Holder
    mReferencedBy.clear();

    // Unregister from the reference manager if necessary.
    if ( mReferenceId != 0 )
        unregisterSelf( );

    // Dispose of base class on request.
    if ( disposeBase )
        cgEventDispatcher::dispose( true );

    // We have now been disposed
    mDisposed  = true;
    mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : addReference ()
/// <summary>
/// Add a reference to this object. Required for automatic lifetime 
/// management. Specify true to the 'reconnecting' parameter if the 
/// reference holder is simply re-establishing an earlier reference. 
/// In this case, any soft reference count will not be incremented 
/// (default is false).
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::addReference( cgReference * holder )
{
    // Pass through to main method.
    addReference( holder, false );
}
void cgReference::addReference( cgReference * holder, bool reconnecting )
{
    ++mLiveRefCount;
    if ( !reconnecting )
        ++mSoftRefCount;
    
    // Record this reference if not anonymous.
    if ( holder )
    {
        ReferenceMap::iterator itRef = mReferencedBy.find( holder );
        if ( itRef == mReferencedBy.end() )
        {
            // New reference.
            ReferenceInfo refInfo;
            refInfo.reference = holder;
            refInfo.refCount  = 1;
            mReferencedBy[holder] = refInfo;

            // Notify that new reference was added for this target.
            onReferenceAdded( holder, 1, reconnecting );

        } // End if new reference
        else
        {
            // Update reference count
            itRef->second.refCount++;

            // Notify that new reference was added for this target.
            onReferenceAdded( holder, itRef->second.refCount, reconnecting );

        } // End if existing

        // Request that holder add its reference information.
        holder->addRefHold( this, reconnecting );

    } // End if not anonymous
    else
    {
        // Notify that new reference was added for this target.
        onReferenceAdded( CG_NULL, 0, reconnecting );
    
    } // End if anonymous
}

//-----------------------------------------------------------------------------
//  Name : removeReference ()
/// <summary>
/// Remove a reference to this object. Required for automatic lifetime
/// management. Specify true to the 'disconnecting' parameter if the
/// reference holder wants its reference removed from the list without
/// the overall reference count being decremented (Default is false).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgReference::removeReference( cgReference * holder )
{
    // Pass through to main method.
    return removeReference( holder, false );

}
cgInt32 cgReference::removeReference( cgReference * holder, bool disconnecting )
{
    // Decrement reference counts.
    --mLiveRefCount;
    if ( !disconnecting )
        --mSoftRefCount;

    // Remove this reference if not anonymous.
    if ( holder )
    {
        // Decrement reference info for this holder.
        ReferenceMap::iterator itRef = mReferencedBy.find( holder );
        if ( itRef != mReferencedBy.end() )
        {
            cgInt32 nHolderRefCount = --itRef->second.refCount;
            if ( nHolderRefCount == 0 )
                mReferencedBy.erase( holder );

            // Notify that reference was removed for this target.
            onReferenceRemoved( holder, nHolderRefCount, disconnecting );

        } // End if found
        else
        {
            // Notify that an unkown reference was removed for this target.
            // (This is a paranoia step, this should not happen!)
            onReferenceRemoved( CG_NULL, 0, disconnecting );
        
        } // End if not found

        // Request that holder release its reference information.
        holder->removeRefHold( this, disconnecting );

    } // End if not anonymous
    else
    {
        // Notify that reference was removed for this target.
        onReferenceRemoved( CG_NULL, 0, disconnecting );
    
    } // End if anonymous

    // All references removed?
    if ( !mLiveRefCount )
    {
        // Dispose of this item
        deleteReference();

        // Return final 0 reference count (cannot access members past this point)
        return 0;

    } // End if no longer referenced

    // Return number of remaining live references.
    return mLiveRefCount;
}

//-----------------------------------------------------------------------------
//  Name : deleteReference ()
/// <summary>
/// Force the deletion of this reference target.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::deleteReference( )
{
    // Delete only if the script engine
    // is no longer referencing us.
    scriptSafeDispose();
}

//-----------------------------------------------------------------------------
//  Name : addRefHold () (Protected)
/// <summary>
/// Add internal references of the specified object held by this
/// reference holder. This is an internal method that is called only by
/// the target of the reference in order to establish a bi-directional
/// relationship.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::addRefHold( cgReference * reference, bool reconnecting )
{
    // Record this reference if not anonymous.
    if ( reference )
    {
        ReferenceMap::iterator itRef = mReferencesTo.find( reference );
        if ( itRef == mReferencesTo.end() )
        {
            // New reference.
            ReferenceInfo refInfo;
            refInfo.reference = reference;
            refInfo.refCount  = 1;
            mReferencesTo[ reference ] = refInfo;

        } // End if new reference
        else
        {
            itRef->second.refCount++;

        } // End if existing

    } // End if not anonymous
}

//-----------------------------------------------------------------------------
//  Name : removeRefHold () (Protected)
/// <summary>
/// Remove internal references of the specified object held by this
/// reference holder. This is an internal method that is called only by
/// the target of the reference in order to finalize a bi-directional
/// relationship.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::removeRefHold( cgReference * reference, bool disconnecting )
{
    // Remove this reference if not anonymous.
    if ( reference != CG_NULL )
    {
        // Decrement reference info for this holder.
        ReferenceMap::iterator itRef = mReferencesTo.find( reference );
        if ( itRef != mReferencesTo.end() )
        {
            itRef->second.refCount--;
            if ( itRef->second.refCount == 0 )
                mReferencesTo.erase( reference );
        
        } // End if found

    } // End if not anonymous
}

//-----------------------------------------------------------------------------
//  Name : onReferenceAdded () (Virtual)
/// <summary>
/// Triggered whenever a new reference to this object is added by a
/// holder. Derived objects may override this virtual if they need to be
/// informed for any reason.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::onReferenceAdded( cgReference * holder, cgInt32 holderReferences, bool reconnecting )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : onReferenceRemoved () (Virtual)
/// <summary>
/// Triggered whenever an existing reference to this object is removed by
/// a holder. Derived objects may override this virtual if they need to
/// be informed for any reason.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::onReferenceRemoved( cgReference * holder, cgInt32 holderReferences, bool disconnecting )
{
    // Nothing in base implementation.
}

//-----------------------------------------------------------------------------
//  Name : processMessage ()
/// <summary>
/// Process any messages sent to us from other objects, or other parts
/// of the system via the reference messaging system (cgReference).
/// </summary>
//-----------------------------------------------------------------------------
bool cgReference::processMessage( cgMessage * message )
{
    // Nothing in default implementation.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : registerSelf () (Private)
/// <summary>
/// Register this reference with the reference manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::registerSelf( )
{
    cgReferenceManager::registerReference( this );
}

//-----------------------------------------------------------------------------
//  Name : unregisterSelf () (Private)
/// <summary>
/// Unregister this reference from the reference manager.
/// </summary>
//-----------------------------------------------------------------------------
void cgReference::unregisterSelf( )
{
    cgReferenceManager::unregisterReference( mReferenceId );
}