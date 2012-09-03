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
// Name : cgResource.cpp                                                     //
//                                                                           //
// Desc : Contains base resource classes and reference types from which all  //
//        other resource types should derive and interact with. In effect    //
//        this is the base for all types that will be directly managed by    //
//        the main application resource manager.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgResource Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgResource.h>
#include <Resources/cgResourceManager.h>

///////////////////////////////////////////////////////////////////////////////
// cgResource Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgResource () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResource::cgResource( cgUInt32 nReferenceId ) : cgReference( nReferenceId )
{
    // Initialize variables to sensible defaults
    mManager          = CG_NULL;
    mResourceName   = _T("");
    mResourceSource = _T("");
    mFlags            = 0;
    mDestroyDelay     = cgResourceManager::getDefaultDestroyDelay();
    mLastAccessed     = 0.0f;
    mLastReferenced   = 0.0f;
    mStreamType        = cgStreamType::None;
    mTimer            = cgTimer::getInstance();

    // Cached responses
    mResourceType      = cgResourceType::None;
    mResourceLoaded   = false;
    mResourceLost     = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgResource () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgResource::~cgResource( )
{
    // Clean up
    dispose( false );

    // Clear variables.
    mTimer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgResource::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear Variables
    mResourceName   = _T("");
    mResourceSource = _T("");
    mFlags            = 0;
    mDestroyDelay     = 0.0f;
    mLastAccessed     = 0.0f;
    mLastReferenced   = 0.0f;
    mStreamType        = cgStreamType::None;

    // Cached responses
    mResourceType      = cgResourceType::None;
    mResourceLoaded   = false;
    mResourceLost     = false;
    mCanEvict         = false;

    // Call base class implementation if requested.
    if ( bDisposeBase )
        cgReference::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : removeReference()
/// <summary>
/// Remove a reference to this resource (reference) optionally bypassing
/// any requested resource destruction delay. Required for automatic lifetime
/// management. Specify true to the 'bDisconnecting' parameter if the
/// reference holder wants its reference removed from the list without
/// the overall reference count being decremented (Default is false).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgResource::removeReference( cgReference * pHolder, bool bDisconnecting, bool bIgnoreDestructDelay )
{
    // If we are to ignore the destruct delay. Temporarily force destruction 
    // delay to 0. This means that the overriden 'removeReference()' logic 
    // will not trigger garbage collection.
    cgFloat fOldDelay = mDestroyDelay;
    if ( bIgnoreDestructDelay )
        mDestroyDelay = 0.0f;

    // Call standard base class implementation.
    cgInt32 nRefCount = removeReference( pHolder, bDisconnecting );    

    // Restore old destruction delay if we were not destroyed.
    if ( bIgnoreDestructDelay && nRefCount > 0 )
        mDestroyDelay = fOldDelay;

    // Return final ref count.
    return nRefCount;
}

//-----------------------------------------------------------------------------
//  Name : removeReference () (Virtual)
/// <summary>
/// Remove a reference to this target. Required for automatic lifetime
/// management. Specify true to the 'bDisconnecting' parameter if the
/// reference holder wants its reference removed from the list without
/// the overall reference count being decremented (Default is false).
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 cgResource::removeReference( cgReference * pHolder, bool bDisconnecting )
{
    // Call base class implementation first.
    cgInt32 nRefCount = cgReference::removeReference( pHolder, bDisconnecting );

    // The manager is the last thing referencing us?
    // ToDo: 9999 - Check to see if we truly do only have the resource 
    // manager as the remaining reference.
    if ( nRefCount == 1 && mManager && !(mFlags & cgResourceFlags::AlwaysResident)  )
    {
        if ( mDestroyDelay > 0 )
        {
            // Set up for delayed destruction
            mLastReferenced = mTimer->getTime();
            mManager->addGarbageResource( this );
        
        } // End if
        else
        {
            // Release the last reference to this resource
            removeReference( mManager, true );
            return 0;

        } // No destroy delay
    
    } // End if manager is the last reference

    // Return final ref count.
    return nRefCount;
}

//-----------------------------------------------------------------------------
//  Name : onReferenceAdded () (Virtual)
/// <summary>
/// Triggered whenever a new reference to this object is added by a holder.
/// Derived objects may override this virtual if they need to be informed
/// for any reason.
/// </summary>
//-----------------------------------------------------------------------------
void cgResource::onReferenceAdded( cgReference * pHolder , cgInt32 nHolderReferences, bool bReconnecting )
{
    // If we are not always resident, and we're being re-referenced
    // by something else in addition to the resource manager, we are
    // potentially in the garbage list and should be brought back to life.
    if ( mLiveRefCount == 2 && mManager && !(mFlags & cgResourceFlags::AlwaysResident) )
        mManager->removeGarbageResource( this );
}

//-----------------------------------------------------------------------------
// Name : onReferenceRemoved () (Virtual)
/// <summary>
/// Triggered whenever an existing reference to this object is removed by a
/// holder. Derived objects may override this virtual if they need to be
/// informed for any reason. NB: This occurs before the self destruction.
/// </summary>
//-----------------------------------------------------------------------------
void cgResource::onReferenceRemoved( cgReference * pHolder, cgInt32 nHolderReferences, bool bDisconnecting )
{
    // Remove from our parent manager just prior to destruction.
    if ( mLiveRefCount == 0 && mManager )
        mManager->removeResource( this );
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResource::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Resource )
        return true;

    // Unsupported.
    return false;
}

//-----------------------------------------------------------------------------
//  Name : getResourceName ()
/// <summary>
/// Retrieve the internal string name associated with this resource.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgResource::getResourceName( ) const
{
    return mResourceName;
}

//-----------------------------------------------------------------------------
//  Name : getResourceSource ()
/// <summary>
/// Retrieve the information string regarding which part of the
/// application allocated this resource.
/// </summary>
//-----------------------------------------------------------------------------
const cgString & cgResource::getResourceSource( ) const
{
    return mResourceSource;
}

//-----------------------------------------------------------------------------
// Name : setDestroyDelay ()
// Desc : Whenever a resource is released for the last time (i.e. nothing in
//        the application maintains a reference to it EXCEPT for the resource
//        manager) it is destroyed. This function allows you to specify a delay
//        in seconds for the period of time you would like it to remain resident
//        after the last reference is released. If another reference is made
//        to this resource at some point before this time elapses, the actual
//        destruction of the resource is cancelled and the resource reused.
// Note : Because multiple sources can reference this same resource object,
//        only the largest delay requested is stored unless you override by
//        specifying true to the bOverride parameter.
//-----------------------------------------------------------------------------
void cgResource::setDestroyDelay( cgFloat fDelay, bool bOverride /* = false */ )
{
    // Set the destroy delay if it is larger than the current delay
    // or the override is specified.
    if ( bOverride || fDelay > mDestroyDelay )
        mDestroyDelay = fDelay;
}

//-----------------------------------------------------------------------------
//  Name : setResourceName ()
/// <summary>
/// Store the name of this resource used for simple matching.
/// </summary>
//-----------------------------------------------------------------------------
void cgResource::setResourceName ( const cgString & strResourceName )
{
    // Duplicate new name
    mResourceName = strResourceName;
}

//-----------------------------------------------------------------------------
//  Name : setResourceSource ()
/// <summary>
/// Under debugging circumstances, it can be useful to store a source
/// (file and line) at which the resource was created. This data
/// can be stored along with the resource here.
/// Note : You can use the __FILE__ and __LINE__ defines to get access to this
/// information.
/// </summary>
//-----------------------------------------------------------------------------
void cgResource::setResourceSource( cgStreamType::Base StreamType, const cgString & strFile, cgUInt32 Line )
{
    // Clear the previous details
    mResourceSource.clear();

    // Store required values
    mStreamType = StreamType;

    // Validate Requirements
    if ( strFile.empty() == true )
        return;

    // Generate full source string (relative to current directory).
    mResourceSource = cgString::format( _T("%s(%i)"), cgFileSystem::getFileName( strFile ).c_str(), Line );
}

//-----------------------------------------------------------------------------
//  Name : setManagementData ()
/// <summary>
/// Informs the resource of the manager that is managing it (if any).
/// </summary>
//-----------------------------------------------------------------------------
void cgResource::setManagementData( cgResourceManager * pManager, cgUInt32 nFlags )
{
    // Store resource manager
    mManager = pManager;
    mFlags   = nFlags;
}

//-----------------------------------------------------------------------------
//  Name : exchangeResource () (Virtual)
/// <summary>
/// Swap internal resource data with that maintained by the specified 
/// resource. Use with extreme care.
/// </summary>
//-----------------------------------------------------------------------------
bool cgResource::exchangeResource( cgResource * pResource )
{
    // By default, resources cannot exchange.
    return false;
}