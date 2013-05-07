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
// Name : cgReferenceManager.h                                               //
//                                                                           //
// Desc : Classes responsible for managing all active references (pretty     //
//        much anything derived from cgReference) within the system.         //
//        Also provides support for inter-reference messaging.               //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGREFERENCEMANAGER_H_ )
#define _CGE_CGREFERENCEMANAGER_H_

//-----------------------------------------------------------------------------
// cgReferenceManager Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <System/cgTimer.h>
#include <list>
#include <map>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgReference;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgMessage (Class)
/// <summary>
/// Core class containing generic message information for transport
/// to and from application objects via the reference messaging system.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgMessage
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
     cgMessage();
    ~cgMessage();

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void SetGuaranteedData( void * dataIn, cgUInt32 sizeIn );

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    cgMessage & operator = ( const cgMessage & messageIn );

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    // User provided details
    cgUInt32    messageId;              // Used to identify the type of message being sent.
    cgString    messageContext;         // Custom context string.
    void      * messageData;            // Data associated with the message.

    // System variables, should be considered read only!
    bool        guaranteedData;         // Is the data guaranteed, i.e. a duplicate of the message data has been made?
    cgUInt32    dataSize;               // Size of the guaranteed data area in bytes (messageData).
    cgUInt32    fromId;                 // The reference identifier (if any) of the message sender.
    cgUInt32    toId;                   // If the message is intended for an individual, this is its identifier.
    cgUID       groupToId;              // If the message is intended for a group, this is its identifier.
    cgDouble    sendTime;               // Time (in seconds) at which the message was sent.
    cgDouble    deliveryTime;           // Time (in seconds) at which the message was delivered.
    bool        delayed;                // The message was delayed prior to sending
    bool        sourceUnregistered;     // Notification that the message source has been / is being unregistered
};

//-----------------------------------------------------------------------------
//  Name : cgReferenceManager (Class)
/// <summary>
/// Class for handling and managing active references within the system
/// (those derived from cgReference) as well as providing support for reference 
/// messaging.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgReferenceManager
{
public:
    //-------------------------------------------------------------------------
    // Public Constants
    //-------------------------------------------------------------------------
    static const cgUInt32 InternalRefThreshold = 0x80000000;

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    // Management
    static void         shutdown                ( bool flushMessages = false );
    static void         registerReference       ( cgReference * reference );
    static void         unregisterReference     ( cgUInt32 referenceId, bool flushMessages = false );
    static cgReference* getReference            ( cgUInt32 referenceId );
    static cgUInt32     generateInternalRefId   ( );
    static bool         isValidReference        ( cgReference * reference );
    
    // Messaging
    static bool         sendMessageTo           ( cgUInt32 from, cgUInt32 to, cgMessage * message, cgFloat sendDelay = 0.0f );
    static bool         sendMessageToAll        ( cgUInt32 from, cgMessage * message, cgFloat sendDelay = 0.0f );
    static bool         sendMessageToSubscribers( cgUInt32 from, cgMessage * message, cgFloat sendDelay = 0.0f );
    static bool         sendMessageToGroup      ( cgUInt32 from, const cgUID & groupId, cgMessage * message, cgFloat sendDelay = 0.0f );
    static bool         subscribeToGroup        ( cgUInt32 nSubscriber, const cgUID & groupId );
    static bool         unsubscribeFromGroup    ( cgUInt32 nSubscriber, const cgUID & groupId );
    static bool         subscribeToReference    ( cgUInt32 nSubscriber, cgUInt32 referenceId );
    static bool         unsubscribeFromReference( cgUInt32 nSubscriber, cgUInt32 referenceId );
    static void         processMessages         ( cgUInt32 from = 0xFFFFFFFF, bool clearAll = false, bool sendOnClear = true );
   
private:
    //-------------------------------------------------------------------------
    // Private Enumerations
    //-------------------------------------------------------------------------
    enum DestinationType { Individual, Group, Subscribers, All };

    //-------------------------------------------------------------------------
    // Private Typedefs
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, cgReference*, ReferenceMap)
    CGE_UNORDEREDSET_DECLARE(cgUInt32, ReferenceSet)
    CGE_UNORDEREDSET_DECLARE(void*, ReferencePtrSet)
    CGE_UNORDEREDMAP_DECLARE(cgUID, ReferenceSet, GroupMap)
    CGE_UNORDEREDMAP_DECLARE(cgUInt32, ReferenceSet, SubscriberMap)
    CGE_LIST_DECLARE        (cgMessage*, MessageList)
    
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static bool                 queueMessage            ( cgMessage * message, MessageList & messageQueue );
    static bool                 issueMessage            ( cgMessage * message, DestinationType destination );
    
    //-------------------------------------------------------------------------
    // Private Static Variables
    //-------------------------------------------------------------------------
    static ReferenceMap         mSerializedReferences;      // List of currently registered 'serialized' reference targets
    static ReferenceMap         mInternalReferences;        // List of currently registered 'internal' reference targets
    static ReferencePtrSet      mValidReferences;           // A set containing pointers to /all/ valid references.
    static MessageList          mMessageQueue;              // List of messages queued for delayed sending
    static MessageList          mMessageGroupQueue;         // List of group messages queued for delayed sending
    static MessageList          mMessageAllQueue;           // List of system wide messages queued for delayed sending
    static MessageList          mMessageSubscriberQueue;    // List of messages, destined for a particular set of subscribers, queued for delayed sending
    static GroupMap             mMessagingGroups;           // List of targets categorised by group
    static SubscriberMap        mSubscriberTable;           // List of subscribers for specific targets.
    static SubscriberMap        mSubscribedToTable;         // Reverse of the above table.
    static cgUInt32             mNextInternalRefId;         // The next identifier to assign to internal references.
    static bool                 mInternalRefIdWrapped;      // Has the above 'next identifier' wrapped back to the beginning?
    static cgTimer              mTimer;                     // Internal timer used for timestamping and delivering delayed messages
};

#endif // !_CGE_CGREFERENCEMANAGER_H_