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
// Name : cgResource.h                                                       //
//                                                                           //
// Desc : Contains base resource classes and reference types from which all  //
//        other resource types should derive and interact with. In effect    //
//        this is the base for all types that will be directly managed by    //
//        the main application resource manager.                             //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGRESOURCE_H_ )
#define _CGE_CGRESOURCE_H_

//-----------------------------------------------------------------------------
// cgResource Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceTypes.h>
#include <System/cgReference.h>
#include <System/cgTimer.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgResourceManager;

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {EA6E9263-4247-4FC1-9ACE-92416E605AFF}
const cgUID RTID_Resource = {0xEA6E9263, 0x4247, 0x4FC1, {0x9A, 0xCE, 0x92, 0x41, 0x6E, 0x60, 0x5A, 0xFF}};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgResource (Class)
/// <summary>
/// Designed as a base class for accessing underlying resource data
/// such as textures, materials, meshes etc.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgResource : public cgReference
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgResource, cgReference, "Resource" )

    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend class cgResourceManager;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgResource( cgUInt32 referenceId );
    virtual ~cgResource( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgString& getResourceName             ( ) const;
    const cgString& getResourceSource           ( ) const;
    void            setResourceName             ( const cgString & resourceName );
    void            setDestroyDelay             ( cgFloat delay, bool overrideCurrent = false );
    cgInt32         removeReference             ( cgReference * holder, bool disconnecting, bool ignoreDestructDelay );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    // Inline functions for retrieving / setting cached resource related responses
    inline cgResourceType::Base getResourceType     ( ) const      { return mResourceType; }
    inline cgResourceManager  * getManager          ( ) const      { return mManager; }
    inline bool                 canEvict            ( ) const      { return mCanEvict; }
    inline bool                 isLoaded            ( ) const      { return mResourceLoaded; }
    inline bool                 isResourceLost      ( ) const      { return mResourceLost; }
    inline void                 setResourceLost     ( bool lost )  { mResourceLost = lost; }
    
    //-----------------------------------------------------------------------------
    // Name : resourceTouched ()
    // Desc : Should be called whenever the resource has been "used" by the system.
    //        This will cause the last access time to be updated to ensure the
    //        LMRU scheme is capable of detecting old/unused resources.
    //-----------------------------------------------------------------------------
    inline void resourceTouched( )
    {
        mLastAccessed = mTimer->getTime();

    } // End Function
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool            loadResource        ( ) = 0; //{ return true; }
    virtual bool            unloadResource      ( ) = 0; //{ return true; }
    virtual void            deviceLost          ( ) {};
    virtual void            deviceRestored      ( ) {};
    virtual bool            exchangeResource    ( cgResource * resource );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgReference)
    //-------------------------------------------------------------------------
    virtual const cgUID   & getReferenceType    ( ) const { return RTID_Resource; }
    virtual bool            queryReferenceType  ( const cgUID & type ) const;
    virtual cgInt32         removeReference     ( cgReference * holder, bool disconnecting );
    virtual void            onReferenceAdded    ( cgReference * holder, cgInt32 holderReferences, bool reconnecting );
    virtual void            onReferenceRemoved  ( cgReference * holder, cgInt32 holderReferences, bool disconnecting );

    // Promote remaining base class method overloads.
    using cgReference::addReference;
    using cgReference::removeReference;
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            setResourceSource   ( cgStreamType::Base streamType, const cgString & file, cgUInt32 line );
    virtual void            setManagementData   ( cgResourceManager * manager, cgUInt32 flags );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgString             mResourceName;     // Resource name used for simple named matching
    cgString             mResourceSource;   // Source file/line at which this resource was created
    cgResourceManager  * mManager;          // The resource manager currently responsible for managing this resource
    cgUInt32             mFlags;            // Stores the flags specified when the resource was loaded
    cgFloat              mDestroyDelay;     // The amount of time (in seconds) this resource should remain resident when it is no longer referenced
    cgDouble             mLastReferenced;   // The time at which the last reference was removed (for destruction delay processing)
    cgDouble             mLastAccessed;     // The time at which the resource was last accessed
    cgStreamType::Base   mStreamType;       // The type of stream which was used to create this resource (i.e. file, memory etc).
    cgTimer            * mTimer;            // Cached reference to the main application timer

    // Cached responses
    cgResourceType::Base mResourceType;     // Retrieve the resource type
    bool                 mResourceLoaded;   // Resource is loaded?
    bool                 mResourceLost;     // Was the resource lost (i.e. VB data was lost after a rebuild of the device?)
    bool                 mCanEvict;         // Can the resource by dynamically evicted and re-loaded at will by the manager?
};

#endif // !_CGE_CGRESOURCE_H_