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
// Name : cgReference.h                                                      //
//                                                                           //
// Desc : Base class which provides standardized reference counting          //
//        behaviors in addition to recording information about which         //
//        object instances are holding each reference.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGREFERENCE_H_ )
#define _CGE_CGREFERENCE_H_

//-----------------------------------------------------------------------------
// cgReference Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <System/cgReferenceManager.h>
#include <System/cgEventDispatcher.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Globally Unique Type Id(s)
//-----------------------------------------------------------------------------
// {50F0C010-5105-407A-A1CC-BFCF9872189D}
const cgUID RTID_Reference = {0x50F0C010, 0x5105, 0x407A, {0xA1, 0xCC, 0xBF, 0xCF, 0x98, 0x72, 0x18, 0x9D}};

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgMessage;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgReference (Base Class)
/// <summary>
/// Base class which provides standardized reference counting behaviors
/// in addition to recording information about which object instances are
/// holding each reference.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgReference : public cgEventDispatcher
{
    DECLARE_DERIVED_SCRIPTOBJECT( cgReference, cgEventDispatcher, "Reference" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             // ToDo: Commented to aid with porting. Add again?
             //cgReference( );
             cgReference( cgUInt32 referenceId );
    virtual ~cgReference( );

    //-------------------------------------------------------------------------
    // Public Structures
    //-------------------------------------------------------------------------
    // Information about any /active/ items holding a reference to this object
    // or any items that we hold a reference to.
    struct ReferenceInfo
    {
        cgReference * reference;   // The item maintaining a reference to this or vice-versa.
        cgInt32       refCount;    // The number of references that the above currently holds, or we hold to the above.
    };
    CGE_UNORDEREDMAP_DECLARE(void*, ReferenceInfo, ReferenceMap )

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    addReference        ( cgReference * holder );
    cgInt32                 removeReference     ( cgReference * holder );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual void            addReference        ( cgReference * holder, bool reconnecting );
    virtual cgInt32         removeReference     ( cgReference * holder, bool disconnecting );
    virtual void            deleteReference     ( );
    virtual bool            processMessage      ( cgMessage * message );
    virtual void            onReferenceAdded    ( cgReference * holder, cgInt32 holderReferences, bool reconnecting );
    virtual void            onReferenceRemoved  ( cgReference * holder, cgInt32 holderReferences, bool disconnecting );
    
    // Pure virtual methods
    virtual const cgUID   & getReferenceType    ( ) const = 0;
    virtual bool            queryReferenceType  ( const cgUID & type ) const = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose             ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : isInternalReference ()
    /// <summary>
    /// Determine if this reference is either an internal or serialized 
    /// reference.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isInternalReference( ) const
    {
        return ( mReferenceId >= cgReferenceManager::InternalRefThreshold || !mReferenceId );
    }

    //-------------------------------------------------------------------------
    // Name : isDisposed ()
    /// <summary>
    /// Determine if this object has been disposed.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isDisposed( )
    {
        return mDisposed;
    }

    //-------------------------------------------------------------------------
    //  Name : isDisposing ( )
    /// <summary>
    /// Determine if this object is in the process of being disposed.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isDisposing( ) const
    {
        return mDisposing;
    }

    //-------------------------------------------------------------------------
    //  Name : getReferenceId()
    /// <summary>
    /// Retrieve this object's globally unique identifier.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getReferenceId( ) const
    {
        return mReferenceId;
    }

    //-------------------------------------------------------------------------
    //  Name : getReferenceCount()
    /// <summary>
    /// Retrieve this object's current live or soft reference counts.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgInt32 getReferenceCount( bool live ) const
    {
        return live ? mLiveRefCount : mSoftRefCount;
    }

    //-------------------------------------------------------------------------
    //  Name : getReferenceHolders()
    /// <summary>
    /// Retrieve the list of reference objects which have reported their
    /// holding of a pointer to *this* reference.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const ReferenceMap & getReferenceHolders( ) const
    {
        return mReferencedBy;
    }
    
protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    addRefHold          ( cgReference * reference, bool reconnecting );
    void                    removeRefHold       ( cgReference * reference, bool disconnecting );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgUInt32            mReferenceId;   // Globally unique (application and database wide) integer identifier of this reference.
    cgInt32             mLiveRefCount;  // Number of live system items referencing this target (Defaults to 0. References must be explicitly added.)
    cgInt32             mSoftRefCount;  // Number of items referencing this target in total including any disconnected items.
    ReferenceMap        mReferencedBy;  // Map of references that hold this object.
    ReferenceMap        mReferencesTo;  // Map of references we are holding.
    bool                mDisposed;      // Object has been disposed.
    bool                mDisposing;     // Object is in the process of disposing.

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    void            registerSelf        ( );
    void            unregisterSelf      ( );
};

#endif // !_CGE_CGREFERENCE_H_