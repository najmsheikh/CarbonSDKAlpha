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
// File: cgSphereTree.h                                                      //
//                                                                           //
// Desc: Highly optimized dynamic hierarchical sphere tree, ideal for        //
//       managing and querying scene dynamics.                               //
//       Portions Copyright (C) John W. Ratcliff, 2001                       //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSPHERETREE_H_ )
#define _CGE_CGSPHERETREE_H_

//-----------------------------------------------------------------------------
// chSphereTree Header Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptInterop.h>
#include <Math/cgBoundingSphere.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgSphereTreeSubNode;
class cgBSPTree;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
template <class T>
class cgPool
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgPool( size_t size )
    {
        mCapacity       = size;
        mElements       = new Element[size];
        mFreeElements   = mElements;
        mUsedElements   = CG_NULL;
        mIterator       = CG_NULL;
        mFreeCount      = size;
        mUsedCount      = 0;

        // Build the linked list.
        if ( size )
        {
            for ( size_t i = 0; i < size; ++i )
            {
                if ( (i+1) < size )
                    mElements[i].next = &mElements[i+1];
                else
                    mElements[i].next = CG_NULL;
                if ( i )
                    mElements[i].previous = &mElements[i-1];
                else
                    mElements[i].previous = CG_NULL;
            
            } // Next element

        } // End if !empty
    }

    ~cgPool()
    {
        clear();
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : releaseElement()
    /// <summary>
    /// Release the specified element back to the pool.
    /// </summary>
    //-------------------------------------------------------------------------
    void releaseElement( T * link )
    {
        // Update the current iterator if it matches.
        if ( (Element*)link == mIterator )
            mIterator = ((Element*)link)->next;

        // Previous element's "next" now points to this elements's "next".
        Element * previous = ((Element*)link)->previous;
        if ( previous )
        {
            // Remove from the middle of the 'used' linked list.
            Element * next = ((Element*)link)->next;
            previous->next = next;
            if ( next )
                next->previous = previous;
        
        } // End if has previous
        else
        {
            // Update the head of the 'used' linked list
            Element * next = ((Element*)link)->next;
            mUsedElements = next;
            if ( mUsedElements )
                mUsedElements->previous = CG_NULL;
        
        } // End if no previous

        // Re-attach at the head of the free list
        Element * firstFree = mFreeElements;
        mFreeElements = (Element*)link;
        mFreeElements->previous = CG_NULL;
        mFreeElements->next = firstFree;

        // Update statistics. Element has been freed.
        mUsedCount--;
        mFreeCount++;
    }

    //-------------------------------------------------------------------------
    // Name : getFreeElement ()
    /// <summary>
    /// Retrieve the next free allocated entry in the linked list.
    /// </summary>
    //-------------------------------------------------------------------------
    T * getFreeElement( bool link = true )
    {
        // Any free items remaining?
        if ( !mFreeElements )
            return CG_NULL;

        // Detach element from free list.
        Element * result = mFreeElements;
        mFreeElements = result->next;
        if ( mFreeElements )
            mFreeElements->previous = CG_NULL;
    
        // Attach to used list if requested.
        if ( link )
        {
            Element * firstUsed = mUsedElements;
            mUsedElements = result;
            if ( firstUsed ) 
                firstUsed->previous = result;
            mUsedElements->next = firstUsed;
            mUsedElements->previous = CG_NULL;
        
        } // End if link to list
        else
        {
            result->next = CG_NULL;
            result->previous = CG_NULL;

        } // End if don't link

        // Update statistics and then return the new element
        mUsedCount++;
        mFreeCount--;
        return &result->item;
    }

    //-------------------------------------------------------------------------
    // Name : begin ()
    /// <summary>
    /// Set the internal iterator at the start of the assigned element list and
    /// returns that first element.
    /// </summary>
    //-------------------------------------------------------------------------
    T * begin()
    {
        mIterator = mUsedElements;
        if ( mIterator )
            return &mIterator->item;
        else
            return CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : next ()
    /// <summary>
    /// Move the iterator forward and return the next element in the list.
    /// </summary>
    //-------------------------------------------------------------------------
    T * next( )
    {
        if ( mIterator )
            mIterator = mIterator->next;
        if ( mIterator )
            return &mIterator->item;
        else
            return CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : clear ()
    /// <summary>
    /// Deallocate all elements in the pool, and clear the container.
    /// </summary>
    //-------------------------------------------------------------------------
    void clear()
    {
        delete []mElements;
        mCapacity       = 0;
        mElements       = CG_NULL;
        mFreeElements   = CG_NULL;
        mUsedElements   = CG_NULL;
        mIterator       = CG_NULL;
        mFreeCount      = 0;
        mUsedCount      = 0;
    }

private:
    //-------------------------------------------------------------------------
    // Private Structures
    //-------------------------------------------------------------------------
    struct Element
    {
        T         item;
        Element * next;
        Element * previous;
    };

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    size_t      mCapacity;          // Total capacity of the pool.
    size_t      mUsedCount;         // Number of elements used.
    size_t      mFreeCount;         // Number of free elements remaining.
    Element   * mElements;          // Pool of pre-allocated elements.
    Element   * mFreeElements;      // Head of free element linked list.
    Element   * mUsedElements;      // Head of used element linked list.
    Element   * mIterator;
};

class cgSphereTreeFIFO
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgSphereTreeFIFO( size_t size )
    {
        mCount      = 0;
        mCurrent    = 0;
        mBottom     = 0;
        mSize       = size;
        mBuffer     = new cgSphereTreeSubNode*[size];
    }

    ~cgSphereTreeFIFO( )
    {
        clear();
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    cgSphereTreeSubNode ** push( cgSphereTreeSubNode * node )
    {
        ++mCount;
        cgSphereTreeSubNode ** result = &mBuffer[mCurrent];
        mBuffer[mCurrent] = node;
        ++mCurrent;
        if ( mCurrent == mSize )
            mCurrent = 0;
        return result;
    };

    cgSphereTreeSubNode * pop()
    {
        while ( mCurrent != mBottom )
        {
            --mCount;
            cgSphereTreeSubNode * result = mBuffer[mBottom];
            ++mBottom;
            if ( mBottom == mSize )
                mBottom = 0;
            if ( result )
                return result;
        }
        return CG_NULL;
    }

    bool flush( cgSphereTreeSubNode * node )
    {
        if ( mCurrent == mBottom )
            return false;

        for ( size_t i = mBottom; i != mCurrent; )
        {
            if ( mBuffer[i] == node )
            {
                mBuffer[i] = CG_NULL;
                return true;
            }
            if ( ++i == mSize )
                i = 0;
        
        } // Next
        return false;
    }

    //-------------------------------------------------------------------------
    // Name : clear ()
    /// <summary>
    /// Clear the buffer and deallocate internal resources.
    /// </summary>
    //-------------------------------------------------------------------------
    void clear( )
    {
        delete []mBuffer;
        mCount      = 0;
        mCurrent    = 0;
        mBottom     = 0;
        mSize       = 0;
        mBuffer     = CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : getEntryCount ()
    /// <summary>
    /// Determine the number of elements that are pushed onto the FIFO buffer.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_t getEntryCount( ) const
    {
        return mCount;
    }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    size_t                  mCount;
    size_t                  mCurrent;
    size_t                  mBottom;
    size_t                  mSize;
    cgSphereTreeSubNode  ** mBuffer;
};

//-----------------------------------------------------------------------------
// Name : cgSphereTree (Class)
/// <summary>
/// Highly optimized dynamic hierarchical sphere tree, ideal for managing and
/// querying scene dynamics.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgSphereTree : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgSphereTree, "SphereTree" )

public:
    //-------------------------------------------------------------------------
    // Protected Typedefs
    //-------------------------------------------------------------------------
    CGE_ARRAY_DECLARE( cgVisibilitySet*, VisibilitySetArray )

    //-------------------------------------------------------------------------
    // Public Enumerations
    //-------------------------------------------------------------------------
    enum NodeFlags
    {
        SuperSphere     = 0x1,      // This is a supersphere, allocated and deleted by us
        RootNode        = 0x8,      // This is the root node
        Recompute       = 0x10,     // The bounding sphere of this node needs to be recomputed.
        Integrate       = 0x20,     // This node needs to be reintegrated into the tree.
        Terminal        = 0x40,
        UpdateLeaves    = 0x80,

        // Frame coherence
        Hidden          = 0x200,    // Outside of view frustum
        Partial         = 0x400,    // Partially inside view frustum
        Inside          = 0x800     // Completely inside view frustum
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgSphereTree( cgUInt32 maxSpheres, cgFloat leafSize, cgFloat padding, cgBSPTree * staticVisTree );
    virtual ~cgSphereTree( );
    
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                    addVisibilitySet        ( cgVisibilitySet * set );
    void                    removeVisibilitySet     ( cgVisibilitySet * set );
    cgSphereTreeSubNode   * addSphere               ( const cgBoundingSphere & bounds, void * userData, NodeFlags flags = NodeFlags(Terminal) );
    void                    removeSphere            ( cgSphereTreeSubNode * node );
    void                    addIntegrate            ( cgSphereTreeSubNode * node );
    void                    addRecompute            ( cgSphereTreeSubNode * node );
    void                    process                 ( );
    void                    computeVisibility       ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 flags );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline size_t getVisibilitySetCount( ) const
    {
        return mSets.size();
    }
    inline const VisibilitySetArray & getVisibilitySets( ) const
    {
        return mSets;
    }
    inline cgBSPTree * getStaticVisTree( ) const
    {
        return mStaticVisTree;
    }
    
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void            dispose                 ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
    // Protected Methods
    //-------------------------------------------------------------------------
    void                    integrate           ( cgSphereTreeSubNode * node, cgSphereTreeSubNode * superSphere, cgFloat nodeSize );
    
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBSPTree                 * mStaticVisTree;
    cgFloat                     mMaximumLeafSize;
    cgFloat                     mPadding;
    cgSphereTreeSubNode       * mRoot;
    cgSphereTreeFIFO            mIntegrationFIFO;
    cgSphereTreeFIFO            mRecomputeFIFO;
    cgPool<cgSphereTreeSubNode> mNodePool;
    VisibilitySetArray          mSets;
    
}; // End Class cgSphereTree

//-----------------------------------------------------------------------------
// Name : cgSphereTreeSubNode (Class)
/// <summary>
/// Represents a single entry in the sphere tree hierarchy.
/// </summary>
//-----------------------------------------------------------------------------
class cgSphereTreeSubNode : public cgBoundingSphere
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgSphereTreeSubNode() :
        mParent(CG_NULL), mChildren(CG_NULL), mNextSibling(CG_NULL), mPreviousSibling(CG_NULL), mUserData(CG_NULL),
        mFIFO1(CG_NULL), mFIFO2(CG_NULL), mTree(CG_NULL), mFlags(0), mChildCount(0), mBindingDistance(0) {}

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool        recompute           ( cgFloat padding );
    void        invalidateVisibility( );
    void        computeVisibility   ( const cgFrustum & frustum, cgVisibilitySet * visibilityData, cgUInt32 setIndex, cgUInt32 searchFlags, cgVolumeQuery::Class state, cgBSPTree * staticVisTree, cgUInt32 sourceLeaf, const cgByte * sourceLeafVis );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : initialize ()
    /// <summary>
    /// Initialize this sphere tree node ready for population (or repopulation).
    /// </summary>
    //-------------------------------------------------------------------------
    inline void initialize( cgSphereTree * tree, const cgVector3 & center, cgFloat sphereRadius, void * userData )
    {
        position            = center;
        radius              = sphereRadius;
        mTree               = tree;
        mUserData           = userData;
        mParent             = CG_NULL;
        mNextSibling        = CG_NULL;
        mPreviousSibling    = CG_NULL;
        mChildren           = CG_NULL;
        mFIFO1              = CG_NULL;
        mFIFO2              = CG_NULL;
        mFlags              = 0;
        mBindingDistance    = 0;
        mChildCount         = 0;
        mLeafCount          = 0;

        // Size flag array appropriately.
        mVisFlags.resize( tree->getVisibilitySetCount(), 0 );
    }

    //-------------------------------------------------------------------------
    // Name : setFlag ()
    /// <summary>
    /// Set the specified description bit(s) in this node's flag member.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setFlag( cgSphereTree::NodeFlags flags )
    {
        mFlags |= flags;
    }

    //-------------------------------------------------------------------------
    // Name : clearFlag ()
    /// <summary>
    /// Unset the specified description bit(s) in this node's flag member.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void clearFlag( cgSphereTree::NodeFlags flags )
    {
        mFlags &= ~flags;
    }

    //-------------------------------------------------------------------------
    // Name : isFlagSet ()
    /// <summary>
    /// Check to see if the specified description bit or bits are set in this 
    /// node's flag member.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isFlagSet( cgSphereTree::NodeFlags flag ) const
    {
        return ((mFlags & flag) != 0);
    }

    //-------------------------------------------------------------------------
    // Name : setVisFlag ()
    /// <summary>
    /// Set the specified description bit(s) in the visibility flags field for
    /// the specified visibility set index.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setVisFlag( cgUInt32 setIndex, cgSphereTree::NodeFlags flags )
    {
        mVisFlags[setIndex] |= flags;
    }

    //-------------------------------------------------------------------------
    // Name : clearVisFlag ()
    /// <summary>
    /// Unset the specified description bit(s) in the visibility flags field for
    /// the specified visibility set index.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void clearVisFlag( cgUInt32 setIndex, cgSphereTree::NodeFlags flags )
    {
        mVisFlags[setIndex] &= ~flags;
    }

    //-------------------------------------------------------------------------
    // Name : isVisFlagSet ()
    /// <summary>
    /// Check to see if the specified description bit or bits are set in the 
    /// visibility flags field for the specified visibility set index.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool isVisFlagSet( cgUInt32 setIndex, cgSphereTree::NodeFlags flag ) const
    {
        return ((mVisFlags[setIndex] & flag) != 0);
    }

    //-------------------------------------------------------------------------
    // Name : getUserData ()
    /// <summary>
    /// Retrieve the user data associated with this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void * getUserData( ) const
    {
        return mUserData;
    }

    //-------------------------------------------------------------------------
    // Name : setUserData ()
    /// <summary>
    /// Update the user data pointer associated with this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setUserData( void * userData )
    {
        mUserData = userData;
    }

    //-------------------------------------------------------------------------
    // Name : getPreviousSibling ()
    /// <summary>
    /// Return the node that exists prior to this one in the linked list.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgSphereTreeSubNode * getPreviousSibling( )
    {
        return mPreviousSibling;
    }

    //-------------------------------------------------------------------------
    // Name : setPreviousSibling ()
    /// <summary>
    /// Update the node that exists prior to this one in the linked list.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setPreviousSibling( cgSphereTreeSubNode * node )
    {
        mPreviousSibling = node;
    }
    
    //-------------------------------------------------------------------------
    // Name : getNextSibling ()
    /// <summary>
    /// Return the node that exists after to this one in the linked list.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgSphereTreeSubNode * getNextSibling( )
    {
        return mNextSibling;
    }

    //-------------------------------------------------------------------------
    // Name : setNextSibling ()
    /// <summary>
    /// Update the node that exists after this one in the linked list.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setNextSibling( cgSphereTreeSubNode * node )
    {
        mNextSibling = node;
    }

    //-------------------------------------------------------------------------
    // Name : setFIFO1 ()
    /// <summary>
    /// Update the member that indicates this node's current slot in the
    /// recomputation FIFO buffer.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setFIFO1( cgSphereTreeSubNode ** p )
    {
        mFIFO1 = p;
    }

    //-------------------------------------------------------------------------
    // Name : setFIFO2 ()
    /// <summary>
    /// Update the member that indicates this node's current slot in the
    /// integration FIFO buffer.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setFIFO2( cgSphereTreeSubNode ** p )
    {
        mFIFO2 = p;
    }

    //-------------------------------------------------------------------------
    // Name : getChildCount ()
    /// <summary>
    /// Retrieve the total number of children currently attached to this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32 getChildCount( ) const
    {
        return mChildCount;
    }

    //-------------------------------------------------------------------------
    // Name : getChildren ()
    /// <summary>
    /// Retrieve the linked list containing references to children of this node.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgSphereTreeSubNode * getChildren( ) const
    {
        return mChildren;
    }

    //-------------------------------------------------------------------------
    // Name : getVisibilityFlags ()
    /// <summary>
    /// Retrieve the visibility flags that contain frame coherence information
    /// for each registered visibility set.
    /// </summary>
    //-------------------------------------------------------------------------
    inline cgUInt32Array & getVisibilityFlags( )
    {
        return mVisFlags;
    }

    //-------------------------------------------------------------------------
    // Name : setParent ()
    /// <summary>
    /// Update the member describing the parent node to which this node is
    /// attached.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void setParent( cgSphereTreeSubNode * parent )
    {
        mParent = parent;
    }

    //-------------------------------------------------------------------------
    // Name : addChild ()
    /// <summary>
    /// Add the specified node as a child of this one.
    /// </summary>
    //-------------------------------------------------------------------------
    void addChild( cgSphereTreeSubNode * node )
    {

        // Attach at head of children linked list
        cgSphereTreeSubNode * firstChild = mChildren;
        mChildren = node;

        node->setNextSibling( firstChild );
        node->setPreviousSibling( CG_NULL );
        node->setParent(this);

        if ( firstChild )
            firstChild->setPreviousSibling(node);

        ++mChildCount;

        #ifdef _DEBUG
            cgFloat distance = cgVector3::length( this->position - node->position );
            //cgAssert( (distance + node->radius) <= radius );
        #endif
    }

    //-------------------------------------------------------------------------
    // Name : removeChild ()
    /// <summary>
    /// Remove the specified child node from this parent.
    /// </summary>
    //-------------------------------------------------------------------------
    void removeChild( cgSphereTreeSubNode * node )
    {
        cgAssert( mChildCount );
        cgAssert( mChildren );
    
        // Debug mode validation.
        #ifdef _DEBUG
            cgSphereTreeSubNode * child = mChildren;
            bool found = false;
            while ( child )
            {
                if ( child == node )
                {
                    cgAssert( !found );
                    found = true;
                }
                child = child->getNextSibling();
            }
            cgAssert( found );
        #endif

        // Previous sibling's "next" now points to this node's "next".
        cgSphereTreeSubNode * previous = node->getPreviousSibling();
        cgSphereTreeSubNode * next = node->getNextSibling();
        if ( previous )
        {
            // Remove from middle of child linked list.
            previous->setNextSibling( next );
            if ( next )
                next->setPreviousSibling( previous );

        } // End if has previous
        else
        {
            // Update head of child linked list.
            mChildren = next;
            if ( mChildren )
                mChildren->setPreviousSibling(CG_NULL);
        
        } // End if no previous
        mChildCount--;

        // Remove from hierarchy if there are no longer any children.
        if ( !mChildCount && isFlagSet( cgSphereTree::SuperSphere ) )
            mTree->removeSphere( this );
    }

    //-------------------------------------------------------------------------
    // Name : unlink ()
    /// <summary>
    /// Unlink this node from the hierarchy.
    /// </summary>
    //-------------------------------------------------------------------------
    void unlink(void)
    {
        // Cannot unlink nodes with children!
        cgAssert( !mChildren );

        // Remove us from the recomputation FIFO
        if ( mFIFO1 ) // if we belong to fifo1, null us out
        {
            *mFIFO1 = CG_NULL;
            mFIFO1 = CG_NULL;
        
        } // End if pending recompute

        // Remove us from the integration FIFO
        if ( mFIFO2 )
        {
            *mFIFO2 = CG_NULL;
            mFIFO2 = CG_NULL;
        
        } // End if pending integrate

        // Unlink from parent.
        if ( mParent )
            mParent->removeChild(this);
        mParent = CG_NULL;
    }

    //-------------------------------------------------------------------------
    // Name : updateSphere ()
    /// <summary>
    /// Update the position of this node.
    /// </summary>
    //-------------------------------------------------------------------------
    void updateSphere( const cgVector3 & newPosition )
    {
        this->position = newPosition;
        setFlag( cgSphereTree::UpdateLeaves );

        // If we have a parent (meaning we are a valid leaf node) and we have not 
        // already been flagged for re-integration, then.....
        if ( mParent && !isFlagSet( cgSphereTree::Integrate ) )
        {
            cgFloat distanceSq = cgVector3::lengthSq( newPosition - mParent->position );

            // Does it exceed our binding distance? (Pierces the skin of our parent)
            if ( distanceSq >= mBindingDistance )
            {
                // If our parent is not already marked to be recomputed (rebalance the sphere), 
                // then add him to the recomputation FIFO.
                if ( !mParent->isFlagSet(cgSphereTree::Recompute) )
                    mTree->addRecompute(mParent);

                // Unlink ourselves from the parent sphere and place ourselves into the root node.
                unlink();
                mTree->addIntegrate(this);
            }
        }
    }

    //-------------------------------------------------------------------------
    // Name : updateSphere ()
    /// <summary>
    /// Update the position (and potentially radius) of this node.
    /// </summary>
    //-------------------------------------------------------------------------
    void updateSphere( const cgVector3 & newPosition, cgFloat newRadius )
    {
        this->position = newPosition;
        this->radius = newRadius;
        setFlag( cgSphereTree::UpdateLeaves );

        if ( mParent && !isFlagSet( cgSphereTree::Integrate ) )
        {
            // Recompute binding distance if necessary (radius changed).
            if ( newRadius != this->radius )
                computeBindingDistance(mParent);

            cgFloat distanceSq = cgVector3::lengthSq( newPosition - mParent->position );
            if ( distanceSq >= mBindingDistance )
            {
                if ( !mParent->isFlagSet(cgSphereTree::Recompute) )
                    mTree->addRecompute(mParent);
                unlink();
                mTree->addIntegrate(this);
            }
            else
            {
                if ( !mParent->isFlagSet(cgSphereTree::Recompute) )
                    mTree->addRecompute(mParent);
            }
        }
    }

    //-------------------------------------------------------------------------
    // Name : computeBindingDistance ()
    /// <summary>
    /// Compute (squared) distance from the edge of the parent node.
    /// </summary>
    //-------------------------------------------------------------------------
    void computeBindingDistance( cgSphereTreeSubNode * parent )
    {
        mBindingDistance = parent->radius - this->radius;
        if ( mBindingDistance <= 0 )
            mBindingDistance = 0;
        else
            mBindingDistance *= mBindingDistance;
    }

private:
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgSphereTreeSubNode   * mParent;
    cgSphereTreeSubNode   * mChildren;

    cgSphereTreeSubNode   * mNextSibling;
    cgSphereTreeSubNode   * mPreviousSibling;

    cgSphereTreeSubNode  ** mFIFO1;
    cgSphereTreeSubNode  ** mFIFO2;

    cgUInt32                mFlags;
    cgUInt32                mChildCount;
    cgFloat                 mBindingDistance;

    cgSphereTree          * mTree;
    void                  * mUserData;

    cgUInt32Array           mVisFlags;

    cgUInt32                mLeafCount;
    cgUInt32                mLeaves[1000];

}; // End Class cgSphereTreeSubNode

#endif // !_CGE_CGSPHERETREE_H_