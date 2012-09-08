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
// Name : cgReferenceManager.cpp                                             //
//                                                                           //
// Desc : Classes responsible for managing all active references (pretty     //
//        much anything derived from cgReference) within the system.         //
//        Also provides support for inter-reference messaging.               //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgReferenceManager Module Includes
//-----------------------------------------------------------------------------
#include <System/cgReferenceManager.h>
#include <System/cgReference.h>

//-----------------------------------------------------------------------------
// Static Member Variable Definitions
//-----------------------------------------------------------------------------
cgReferenceManager::ReferenceMap    cgReferenceManager::mInternalReferences;
cgReferenceManager::ReferenceMap    cgReferenceManager::mSerializedReferences;
cgReferenceManager::ReferencePtrSet cgReferenceManager::mValidReferences;
cgReferenceManager::MessageList     cgReferenceManager::mMessageQueue;
cgReferenceManager::MessageList     cgReferenceManager::mMessageGroupQueue;
cgReferenceManager::MessageList     cgReferenceManager::mMessageSubscriberQueue;
cgReferenceManager::MessageList     cgReferenceManager::mMessageAllQueue;
cgReferenceManager::GroupMap        cgReferenceManager::mMessagingGroups;
cgReferenceManager::SubscriberMap   cgReferenceManager::mSubscriberTable;
cgReferenceManager::SubscriberMap   cgReferenceManager::mSubscribedToTable;
cgUInt32                            cgReferenceManager::mNextInternalRefId        = cgReferenceManager::InternalRefThreshold;
bool                                cgReferenceManager::mInternalRefIdWrapped     = false;
cgTimer                             cgReferenceManager::mTimer;

///////////////////////////////////////////////////////////////////////////////
// cgReferenceManager Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : shutdown () (Static)
/// <summary>
/// Shutdown the reference management system, and optionally send any
/// outstanding messages.
/// Note : Outstanding messages will be destroyed regardless, however you can
/// request that the messages get sent first by specifying a value of
/// 'true' to the optional bFlushMessages parameter.
/// </summary>
//-----------------------------------------------------------------------------
void cgReferenceManager::shutdown( bool bFlushMessages /* = false */ )
{
    // Delete all active messages (and send if flushing)
    processMessages( 0xFFFFFFFF, true, bFlushMessages );

    // Clear target lists
    mSerializedReferences.clear();
    mInternalReferences.clear();
    mValidReferences.clear();
    mMessagingGroups.clear();
    mSubscriberTable.clear();
    mSubscribedToTable.clear();
}

//-----------------------------------------------------------------------------
//  Name : registerReference () (Static)
/// <summary>
/// Register the specified reference target with the system.
/// </summary>
//-----------------------------------------------------------------------------
void cgReferenceManager::registerReference( cgReference * pReference )
{
    // References with an identifier of 0 are excluded from management.
    if ( pReference == CG_NULL || pReference->getReferenceId() == 0 )
        return;

    // Is this an internal or serialized reference?
    cgUInt32 referenceId = pReference->getReferenceId();
    bool bInternal = (referenceId >= InternalRefThreshold);

    // Add this reference to the relevant list.
    if ( bInternal == true )
        mInternalReferences[ referenceId ] = pReference;
    else
        mSerializedReferences[ referenceId ] = pReference;

    // Also add to the valid reference table.
    mValidReferences.insert( pReference );
}

//-----------------------------------------------------------------------------
//  Name : unregisterReference () (Static)
/// <summary>
/// Unregister the specified reference target from the system.
/// Note : Outstanding messages originally sent from this reference will be 
/// destroyed regardless, however you can request that the messages get 
/// sent first by specifying a value of 'true' to the optional 
/// bFlushMessages parameter.
/// </summary>
//-----------------------------------------------------------------------------
void cgReferenceManager::unregisterReference( cgUInt32 nReferenceId, bool bFlushMessages /* = false */ )
{
    cgReference           * pReference = CG_NULL;
    GroupMap::iterator      itGroup;
    ReferenceMap::iterator  itReference;
    SubscriberMap::iterator itSubscribers;

    // Flush this reference target's messages before it finally goes out of scope (if requested)
    processMessages( nReferenceId, true, bFlushMessages );

    // Find the reference with this identifier and remove it from the lists.
    if ( nReferenceId >= InternalRefThreshold )
    {
        itReference = mInternalReferences.find( nReferenceId );
        if ( itReference != mInternalReferences.end() )
        {
            pReference = itReference->second;
            mInternalReferences.erase( itReference );
        
        } // End if found
    
    } // End if internal
    else
    {
        itReference = mSerializedReferences.find( nReferenceId );
        if ( itReference != mSerializedReferences.end() )
        {
            pReference = itReference->second;
            mSerializedReferences.erase( itReference );
        
        } // End if found

    } // End if serialized

    // We also need to remove the reference target from any groups 
    // to which it is subscribed.
    for ( itGroup = mMessagingGroups.begin(); itGroup != mMessagingGroups.end(); ++itGroup )
    {
        // Retrieve a reference to the set that lists all of the
        // references subscribed to the group.
        ReferenceSet & Set = itGroup->second;

        // Find the reference with this identifier and remove it if it is found.
        ReferenceSet::iterator itSet = Set.find( nReferenceId );
        if ( itSet != Set.end() )
            Set.erase( itSet );

    } // Next messaging group

    // Unsubscribe from all other references this was subscribed to.
    itSubscribers = mSubscribedToTable.find( nReferenceId );
    if ( itSubscribers != mSubscribedToTable.end() )
    {
        ReferenceSet::iterator itSet;
        ReferenceSet & Set = itSubscribers->second;
        for ( itSet = Set.begin(); itSet != Set.end(); )
        {
            cgUInt32 nToRefId = *itSet;
            ++itSet;
            unsubscribeFromReference( nReferenceId, nToRefId );
        
        } // Next Target

        // Remove from list
        mSubscribedToTable.erase( itSubscribers );
    
    } // End if found

    // Remove the subscription information
    itSubscribers = mSubscriberTable.find( nReferenceId );
    if ( itSubscribers != mSubscriberTable.end() )
        mSubscriberTable.erase( itSubscribers );

    // Finally, remove from the valid reference table.
    if ( pReference != CG_NULL )
        mValidReferences.erase( pReference );
}

//-----------------------------------------------------------------------------
//  Name : getReference () (Static)
/// <summary>
/// Retrieve the actual reference object associated with the specified 
/// identifier.
/// </summary>
//-----------------------------------------------------------------------------
cgReference * cgReferenceManager::getReference( cgUInt32 nReferenceId )
{
    ReferenceMap::iterator itReference;

    // Find the message target with this identifier and 
    // return the associated object pointer.
    if ( nReferenceId >= InternalRefThreshold )
    {
        itReference = mInternalReferences.find( nReferenceId );
        if ( itReference != mInternalReferences.end() )
            return itReference->second;
    
    } // End if internal
    else
    {
        itReference = mSerializedReferences.find( nReferenceId );
        if ( itReference != mSerializedReferences.end() )
            return itReference->second;
    
    } // End if serialized

    // Reference not found
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : isValidReference() (Static)
/// <summary>
/// Determine if the specified reference exists in the system or has been
/// destroyed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::isValidReference( cgReference * pReference )
{
    return (mValidReferences.find( pReference ) != mValidReferences.end());
}

//-----------------------------------------------------------------------------
//  Name : generateInternalRefId () (Static)
/// <summary>
/// Retrieve the next reference identifier in the series for internal 
/// references.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgReferenceManager::generateInternalRefId( )
{
    cgUInt32 referenceId = 0;

    // If wrapping of the identifier has occurred, we need to take
    // additional action to ensure the next identifier is available.
    if ( mInternalRefIdWrapped == true )
    {
        // Search for a free identifier.
        while ( mInternalReferences.find( mNextInternalRefId ) != mInternalReferences.end() )
        {
            // Try the next one.
            mNextInternalRefId++;

            // Wrap correctly (leave 0x0 and 0xFFFFFFFF free).
            if ( mNextInternalRefId == 0xFFFFFFFF )
                mNextInternalRefId = InternalRefThreshold;
        
        } // Next identifier

    } // End if wrapped

    // Return next internal identifier
    referenceId = mNextInternalRefId++;

    // Wrap correctly (leave 0x0 and 0xFFFFFFFF free).
    if ( mNextInternalRefId == 0xFFFFFFFF )
    {
        mInternalRefIdWrapped = true;
        mNextInternalRefId = InternalRefThreshold;

    } // End if wrapped

    // Done
    return referenceId;
}

//-----------------------------------------------------------------------------
//  Name : sendMessageTo () (Static)
/// <summary>
/// Send a message to a specific reference by identifier.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::sendMessageTo( cgUInt32 nFrom, cgUInt32 nTo, cgMessage * pMessage, cgFloat fSendDelay /* = 0.0f */ )
{
    // Validate requirements
    if ( !pMessage )
        return false;

    // Setup Consistant Options
    pMessage->sendTime              = mTimer.getTime( true );
    pMessage->fromId                = nFrom;
    pMessage->toId                  = nTo;
    pMessage->delayed               = false;
    pMessage->sourceUnregistered    = false;

    // Send immediately?
    if ( !fSendDelay )
    {
        // Just send message straight away
        issueMessage( pMessage, Individual );
    
    } // End if send immediately
    else
    {
        // Setup remaining message properties
        pMessage->delayed      = true;
        pMessage->deliveryTime = pMessage->sendTime + fSendDelay;

        // Queue up the message
        queueMessage( pMessage, mMessageQueue );

    } // End if delayed send
    
    // On a delayed message, a return code of true means that the message was
    // queued succesfully.
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sendMessageToSubscribers () (Static)
/// <summary>
/// Send a message to every reference currently listed as a subscriber 
/// to the reference indicated by 'nFrom' parameter.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::sendMessageToSubscribers( cgUInt32 nFrom, cgMessage * pMessage, cgFloat fSendDelay /* = 0.0f */ )
{
    // Validate requirements
    if ( !pMessage )
        return false;

    // Setup Consistant Options
    pMessage->sendTime              = mTimer.getTime( true );
    pMessage->fromId                = nFrom;
    pMessage->delayed               = false;
    pMessage->sourceUnregistered    = false;

    // Send immediately?
    if ( !fSendDelay )
    {
        // Just send message straight away
        issueMessage( pMessage, Subscribers );
        
    } // End if send immediately
    else
    {
        // Setup remaining message properties
        pMessage->delayed      = true;
        pMessage->deliveryTime = pMessage->sendTime + fSendDelay;

        // Queue up the message
        queueMessage( pMessage, mMessageSubscriberQueue );

    } // End if delayed send

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sendMessageToAll () (Static)
/// <summary>
/// Send a message to every reference currently registered with the
/// reference manager.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::sendMessageToAll( cgUInt32 nFrom, cgMessage * pMessage, cgFloat fSendDelay /* = 0.0f */ )
{
    // Validate requirements
    if ( !pMessage )
        return false;

    // Setup Consistant Options
    pMessage->sendTime              = mTimer.getTime( true );
    pMessage->fromId                = nFrom;
    pMessage->delayed               = false;
    pMessage->sourceUnregistered    = false;

    // Send immediately?
    if ( !fSendDelay )
    {
        // Just send message straight away
        issueMessage( pMessage, All );
        
    } // End if send immediately
    else
    {
        // Setup remaining message properties
        pMessage->delayed      = true;
        pMessage->deliveryTime = pMessage->sendTime + fSendDelay;

        // Queue up the message
        queueMessage( pMessage, mMessageAllQueue );

    } // End if delayed send

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : sendMessageToGroup () (Static)
/// <summary>
/// Send a message to every reference target currently subscribed to
/// the specified group.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::sendMessageToGroup( cgUInt32 nFrom, const cgUID & GroupUID, cgMessage * pMessage, cgFloat fSendDelay /* = 0.0f */ )
{
    // Validate requirements
    if ( !pMessage )
        return false;

    // Find the group with this identifier
    GroupMap::iterator itGroup = mMessagingGroups.find( GroupUID );

    // Did we find this messaging group?
    if ( itGroup != mMessagingGroups.end() )
    {
        // Setup Consistant Options
        pMessage->sendTime              = mTimer.getTime( true );
        pMessage->fromId                = nFrom;
        pMessage->groupToId             = GroupUID;
        pMessage->delayed               = false;
        pMessage->sourceUnregistered    = false;

        // Send immediately?
        if ( !fSendDelay )
        {
            // Just send message straight away
            issueMessage( pMessage, Group );

        } // End if send immediately
        else
        {
            // Setup remaining message properties
            pMessage->delayed      = true;
            pMessage->deliveryTime = pMessage->sendTime + fSendDelay;

            // Queue up the message
            queueMessage( pMessage, mMessageGroupQueue );

        } // End if delayed send

        // Group was found
        return true;

    } // End if message group exists
    
    // No group existed
    return false;
}

//-----------------------------------------------------------------------------
//  Name : subscribeToGroup () (Static)
/// <summary>
/// Add the specified reference to the specified messaging group.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::subscribeToGroup( cgUInt32 nSubscriber, const cgUID & GroupId )
{
    ReferenceMap::iterator itReference;

    // Find the reference target with this identifier
    if ( nSubscriber >= InternalRefThreshold )
    {
        itReference = mInternalReferences.find( nSubscriber );
        if ( itReference == mInternalReferences.end() )
            return false;
    
    } // End if internal
    else
    {
        itReference = mSerializedReferences.find( nSubscriber );
        if ( itReference == mSerializedReferences.end() )
            return false;
    
    } // End if serialized

    // Get the correct group
    ReferenceSet & Set = mMessagingGroups[GroupId];

    // Ensure this doesn't already exist.
    if ( Set.find( nSubscriber ) != Set.end() )
        return false;

    // Add to the group
    Set.insert( nSubscriber );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unsubscribeFromGroup () (Static)
/// <summary>
/// Remove the specified reference target from the specified group.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::unsubscribeFromGroup( cgUInt32 nSubscriber, const cgUID & GroupId )
{
    // Find the group with this identifier
    GroupMap::iterator itGroup = mMessagingGroups.find( GroupId );

    // Did we find this messaging group?
    if ( itGroup != mMessagingGroups.end() )
    {
        // Retrieve a reference to the set describing which targets
        // are subscribed to this group.
        ReferenceSet & Set = itGroup->second;

        // Find the message target with this identifier
        ReferenceSet::iterator itReference = Set.find( nSubscriber );
        if ( itReference == Set.end() )
            return false;
        
        // Remove from this group's set
        Set.erase( itReference );

        // Success!
        return true;

    } // End if found group

    // No group found
    return false;
}

//-----------------------------------------------------------------------------
//  Name : subscribeToReference () (Static)
/// <summary>
/// Allow a reference to 'subscribe' to another. Any messages sent from
/// the source via 'sendMessageToSubscribers()' will be sent to these
/// subscribed members.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::subscribeToReference( cgUInt32 nSubscriber, cgUInt32 nReferenceId )
{
    ReferenceMap::iterator itReference, itSubscriber;

    // Ensure that both reference identifiers are (currently) valid.
    if ( getReference( nSubscriber ) == CG_NULL || getReference( nReferenceId ) == CG_NULL )
        return false;

    // Both are valid, so just add the subscription.
    mSubscriberTable[ nReferenceId ].insert( nSubscriber );
    mSubscribedToTable[ nSubscriber ].insert( nReferenceId );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unsubscribeFromReference () (Static)
/// <summary>
/// Remove a reference subscription.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::unsubscribeFromReference( cgUInt32 nSubscriber, cgUInt32 nReferenceId )
{
    // Find the subscriber set for the reference we want to unsubscribe from.
    SubscriberMap::iterator itSubscribers = mSubscriberTable.find( nReferenceId );
    if ( itSubscribers != mSubscriberTable.end() )
    {
        // Remove the reference to the subscriber.
        itSubscribers->second.erase( nSubscriber );

    } // End if found

    // Also remove this information from the 'subscribed TO' set for the subscriber.
    itSubscribers = mSubscribedToTable.find( nSubscriber );
    if ( itSubscribers != mSubscribedToTable.end() )
    {
        // Remove the reference to the target of interest.
        itSubscribers->second.erase( nReferenceId );

    } // End if found

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : processMessages () (Static)
/// <summary>
/// Allow the reference system to process any outstanding / queued
/// reference target messages.
/// </summary>
//-----------------------------------------------------------------------------
void cgReferenceManager::processMessages( cgUInt32 nFrom /* = 0xFFFFFFFF */, bool bClearAll /* = false */, bool bSendOnClear /* = true */ )
{
    MessageList::iterator itMessage;
    cgDouble fCurrentTime = mTimer.getTime( true );

    // Loop through individual messages
    for ( itMessage = mMessageQueue.begin(); itMessage != mMessageQueue.end(); )
    {
        cgMessage * pMessage = *itMessage;
        if ( pMessage == CG_NULL )
        {
            ++itMessage;
            continue;
        
        } // End if invalid

        // Not time to send message yet?
        if ( bClearAll == false && pMessage->deliveryTime > fCurrentTime )
            break;

        // Limiting to a particular source target?
        if ( nFrom != 0xFFFFFFFF && nFrom != pMessage->fromId )
        {
            ++itMessage;
            continue;
        
        } // End if no match

        // Is the source being unregistered?
        if ( bClearAll == true )
            pMessage->sourceUnregistered = true;

        // Send the message!
        if ( bClearAll == false || bSendOnClear == true )
            issueMessage( pMessage, Individual );
        delete pMessage;

        // Erase this message from the list
        mMessageQueue.erase( itMessage++ );
    
    } // Next Individual Message

    // Loop through group messages
    for ( itMessage = mMessageGroupQueue.begin(); itMessage != mMessageGroupQueue.end(); )
    {
        cgMessage * pMessage = *itMessage;
        if ( pMessage == CG_NULL )
        {
            ++itMessage;
            continue;
        
        } // End if invalid

        // Not time to send message yet?
        if ( bClearAll == false && pMessage->deliveryTime > fCurrentTime )
            break;

        // Limiting to a particular source target?
        if ( nFrom != 0xFFFFFFFF && nFrom != pMessage->fromId )
        {
            ++itMessage;
            continue;
        
        } // End if no match

        // Is the source being unregistered?
        if ( bClearAll == true )
            pMessage->sourceUnregistered = true;

        // Send the message!
        if ( bClearAll == false || bSendOnClear == true )
            issueMessage( pMessage, Group );
        delete pMessage;

        // Erase this message from the list
        mMessageGroupQueue.erase( itMessage++ );

    } // Next Group Message

    // Loop through subscriber wide messages
    for ( itMessage = mMessageSubscriberQueue.begin(); itMessage != mMessageSubscriberQueue.end(); )
    {
        cgMessage * pMessage = *itMessage;
        if ( pMessage == CG_NULL )
        {
            ++itMessage;
            continue;
        
        } // End if invalid

        // Not time to send message yet?
        if ( bClearAll == false && pMessage->deliveryTime > fCurrentTime )
            break;

        // Limiting to a particular source target?
        if ( nFrom != 0xFFFFFFFF && nFrom != pMessage->fromId )
        {
            ++itMessage;
            continue;
        
        } // End if no match

        // Is the source being unregistered?
        if ( bClearAll == true )
            pMessage->sourceUnregistered = true;

        // Send the message!
        if ( bClearAll == false || bSendOnClear == true )
            issueMessage( pMessage, Subscribers );
        delete pMessage;

        // Erase this message from the list
        mMessageSubscriberQueue.erase( itMessage++ );

    } // Next Subscribers Message

    // Loop through system wide messages
    for ( itMessage = mMessageAllQueue.begin(); itMessage != mMessageAllQueue.end(); )
    {
        cgMessage * pMessage = *itMessage;
        if ( pMessage == CG_NULL )
        {
            ++itMessage;
            continue;
        
        } // End if invalid

        // Not time to send message yet?
        if ( bClearAll == false && pMessage->deliveryTime > fCurrentTime )
            break;

        // Limiting to a particular source target?
        if ( nFrom != 0xFFFFFFFF && nFrom != pMessage->fromId )
        {
            ++itMessage;
            continue;
        
        } // End if no match

        // Is the source being unregistered?
        if ( bClearAll == true )
            pMessage->sourceUnregistered = true;

        // Send the message!
        if ( bClearAll == false || bSendOnClear == true )
            issueMessage( pMessage, All );
        delete pMessage;

        // Erase this message from the list
        mMessageAllQueue.erase( itMessage++ );

    } // Next System Message
}

//-----------------------------------------------------------------------------
//  Name : queueMessage () (Static, Private)
/// <summary>
/// Queue up the specified message to the correct list and in the
/// correct order for optimal processing.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::queueMessage( cgMessage * pMessage, MessageList & MessageQueue )
{
    MessageList::iterator itMessage;

    // Must be queued!
    cgMessage * pNewMessage = new cgMessage();

    // Copy data structure (ensures that it doesn't go out of scope from caller)
    *pNewMessage = *pMessage;

    // Find the correct slot in the message queue for this item
    for ( itMessage = MessageQueue.begin(); itMessage != MessageQueue.end(); ++itMessage )
    {
        cgMessage * pItem = *itMessage;
        if (!pItem)
            continue;

        // Should we insert here?
        if ( pItem->deliveryTime > pMessage->deliveryTime )
            break;

    } // Next Message

    // Insert into the message queue
    MessageQueue.insert( itMessage, pNewMessage );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : issueMessage () (Static, Private)
/// <summary>
/// Perform the sending of the message to the necessary references.
/// </summary>
//-----------------------------------------------------------------------------
bool cgReferenceManager::issueMessage( cgMessage * pMessage, DestinationType Destination )
{
    cgDouble fCurrentTime = mTimer.getTime( true );

    // What type of object is this being sent to?
    switch ( Destination )
    {
        case Individual:
        {
            // Find the reference target with the specified 'To' identifier
            cgReference * pTo = getReference( pMessage->toId );
            
            // Did we find it?
            if ( pTo != CG_NULL )
            {
                // Set up remaining message properties
                pMessage->deliveryTime = fCurrentTime;

                // Send the message for processing
                return pTo->processMessage( pMessage );

            } // End if found
            break;
        
        } // End Case Individual
        case Subscribers:
        {
            // Find the list of subscribers.
            SubscriberMap::iterator itSubscribers = mSubscriberTable.find( pMessage->fromId );

            // Did we find subscribers for this source?
            if ( itSubscribers != mSubscriberTable.end() )
            {
                // Iterate through all subscribed targets.
                ReferenceSet::iterator itReference;
                ReferenceSet & Subscribers = itSubscribers->second;
                for ( itReference = Subscribers.begin(); itReference != Subscribers.end(); )
                {
                    cgReference * pTo = getReference( *itReference );
                    ++itReference;

                    // Did we find it?
                    if ( pTo != CG_NULL )
                    {
                        // Set up remaining message properties
                        pMessage->deliveryTime = fCurrentTime;

                        // Send the message for processing immediately
                        pTo->processMessage( pMessage );

                    } // End if found

                } // Next subscriber

            } // End if subscribers exist
            
            break;

        } // End Case Subscribers
        case Group:
        {
            // Find the group with this identifier
            GroupMap::iterator itGroup = mMessagingGroups.find( pMessage->groupToId );

            // Did we find this messaging group?
            if ( itGroup != mMessagingGroups.end() )
            {
                // Iterate through all reference targets subscribed to this group
                ReferenceSet::iterator itReference;
                ReferenceSet & Subscribers = itGroup->second;
                for ( itReference = Subscribers.begin(); itReference != Subscribers.end(); )
                {
                    cgReference * pTo = getReference( *itReference );
                    ++itReference;
                    
                    // Did we find it?
                    if ( pTo != CG_NULL )
                    {
                        // Set up remaining message properties
                        pMessage->deliveryTime = fCurrentTime;

                        // Send the message for processing immediately
                        pTo->processMessage( pMessage );
                    
                    } // End if found

                } // Next subscriber

            } // End if group exists

            break;
        
        } // End Case Group
        case All:
        {
            // Loop through all references. First the internal ones.
            ReferenceMap::iterator itReference;
            for ( itReference = mInternalReferences.begin(); itReference != mInternalReferences.end(); )
            {
                cgReference * pTo = itReference->second;
                ++itReference;
                if ( pTo != CG_NULL )
                {
                    // Setup remaining message properties
                    pMessage->deliveryTime = fCurrentTime;

                    // Send the message for processing immediately
                    pTo->processMessage( pMessage );

                } // End if valid

            } // Next reference

            // Then the so-called 'serialized' references.
            for ( itReference = mSerializedReferences.begin(); itReference != mSerializedReferences.end(); )
            {
                cgReference * pTo = itReference->second;
                ++itReference;
                if ( pTo != CG_NULL )
                {
                    // Setup remaining message properties
                    pMessage->deliveryTime = fCurrentTime;

                    // Send the message for processing immediately
                    pTo->processMessage( pMessage );

                } // End if valid

            } // Next reference
            break;

        } // End Case All
    
    } // End destination type switch
    
    // Target did not exist
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cgMessage Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgMessage () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMessage::cgMessage()
{
    // Initialize variables to sensible defaults
    messageId           = 0;
    messageData         = CG_NULL;
    guaranteedData      = false;
    dataSize            = 0;
    fromId              = 0;
    toId                = 0;
    sendTime            = 0.0f;
    deliveryTime        = 0.0f;
    delayed             = 0.0f;
    sourceUnregistered  = false;

    // Clear structures
    memset( &groupToId, 0, sizeof(cgUID) );
}

//-----------------------------------------------------------------------------
//  Name : ~cgMessage () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgMessage::~cgMessage()
{
    // Release allocated memory
    if ( messageData != CG_NULL && guaranteedData == true )
        delete [](cgByte*)messageData;

    // Clear variables
    messageData = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : SetGuaranteedData ()
/// <summary>
/// Make a copy of the specified data to store within the message itself.
/// This ensures that the data is valid until it reaches the target,
/// irrespective of whether the original message data is deleted.
/// </summary>
//-----------------------------------------------------------------------------
void cgMessage::SetGuaranteedData( void * pDataIn, cgUInt32 nSizeIn )
{
    // Release any previous
    if ( messageData != CG_NULL && guaranteedData == true )
        delete[] (cgByte*)messageData;
    messageData    = CG_NULL;
    guaranteedData = false;

    // Any data passed
    if ( pDataIn && nSizeIn )
    {
        // Allocated memory
        cgByte * pNewData = new cgByte[ nSizeIn ];
        memcpy( pNewData, pDataIn, nSizeIn );

        // Setup structure
        messageData    = (void*)pNewData;
        dataSize       = nSizeIn;
        guaranteedData = true;
    
    } // End if any data passed
}

//-----------------------------------------------------------------------------
//  Name : operator= ()
/// <summary>
/// Duplicate this message and any guaranteed / serialized data.
/// </summary>
//-----------------------------------------------------------------------------
cgMessage & cgMessage::operator = ( const cgMessage & MessageIn )
{
    // Is this a no-op?
    if ( &MessageIn == this )
        return (*this);

    // Release any previous
    if ( messageData != CG_NULL && guaranteedData == true )
        delete[] (cgByte*)messageData;
    messageData    = CG_NULL;
    guaranteedData = false;

    // Duplicate any data.
    if ( MessageIn.guaranteedData == true )
    {
        SetGuaranteedData( MessageIn.messageData, MessageIn.dataSize );
    
    } // End if guaranteed
    else
    {
        messageData = MessageIn.messageData;
        dataSize    = MessageIn.dataSize;
    
    } // End if not guaranteed

    // Copy remaining data
    messageId          = MessageIn.messageId;
    messageContext     = MessageIn.messageContext;
    fromId             = MessageIn.fromId;
    toId               = MessageIn.toId;
    sendTime           = MessageIn.sendTime;
    deliveryTime       = MessageIn.deliveryTime;
    delayed            = MessageIn.delayed;
    sourceUnregistered = MessageIn.sourceUnregistered;
    groupToId          = MessageIn.groupToId;
    return *this;

}