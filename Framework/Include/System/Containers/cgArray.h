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
// Name : cgArray.h                                                          //
//                                                                           //
// Desc : Templated array container akin to std::vector.                     //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGARRAY_H_ )
#define _CGE_CGARRAY_H_

//-------------------------------------------------------------------------
// Macros & Defines
//-------------------------------------------------------------------------
#define CGARRAY_CAPACITYGROWTHFACTOR 1.5f

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgArrayBase (Base Class)
/// <summary>
/// Base data class for cgArray type. Allows for querying of certain properties
/// of the array, even without knowing the template type information.
/// </summary>
//-----------------------------------------------------------------------------
class cgArrayBase
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;
    typedef char*               storage_type;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : cgArrayBase () (Constructor)
    /// <summary>
    /// Default constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    cgArrayBase( size_type elementSize ) : mSize( 0 ), mCapacity( 0 ), mElementSize( elementSize ), mStart( 0 ), mEnd( 0 )
    {
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : size ()
    /// <summary>
    /// Retrieve the total number of elements currently stored in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_type size( ) const
    {
        return mSize;
    }

    //-------------------------------------------------------------------------
    //  Name : empty ()
    /// <summary>
    /// Allows the caller to quickly determine if the array is empty.
    /// </summary>
    //-------------------------------------------------------------------------
    inline bool empty( ) const
    {
        return (mSize==0);
    }

    //-------------------------------------------------------------------------
    //  Name : capacity ()
    /// <summary>
    /// Retrieve the total number of elements currently capable of being stored 
    /// in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_type capacity( ) const
    {
        return mCapacity;
    }

    //-------------------------------------------------------------------------
    //  Name : element_size ()
    /// <summary>
    /// Retrieve the total size in bytes of each element in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_type element_size( ) const
    {
        return mElementSize;
    }

    //-------------------------------------------------------------------------
    //  Name : buffer ()
    /// <summary>
    /// Get the underlying memory buffer for the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void * buffer( )
    {
        return (void*)mStart;
    }

    //-------------------------------------------------------------------------
    //  Name : alloc_uninitialized ()
    /// <summary>
    /// Initialize the underlying array buffer without initializing the memory
    /// or triggering constructor behaviors.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void alloc_uninitialized( size_type elements, size_type elementSize )
    {
        mStart = (storage_type)(new char[(elements+1) * elementSize]);
        mEnd   = (storage_type)(((char*)mStart) + (elements * elementSize));
        mSize  = elements;
        mElementSize = elementSize;
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    size_type       mSize;          // Number of active elements in the array.
    storage_type    mStart;         // Underlying buffer.
    storage_type    mEnd;           // Reference to the first byte past the end of the array.
    size_type       mCapacity;      // Overall capacity of the array, in elements.
    size_type       mElementSize;   // Size in bytes of a single element in the array.
};

//-----------------------------------------------------------------------------
//  Name : cgArray (Template Class)
/// <summary>
/// Templated array container akin to std::vector.
/// </summary>
//-----------------------------------------------------------------------------
template <class T> class cgArray : public cgArrayBase
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    typedef T                   value_type;
    
    typedef value_type*         pointer;
    typedef value_type&         reference;
    typedef const value_type*   const_pointer;
    typedef const value_type&   const_reference;
    
    typedef value_type*         iterator;
    typedef const value_type*   const_iterator;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : cgArray () (Constructor)
    /// <summary>
    /// Default constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    cgArray( ) : cgArrayBase( sizeof(value_type) )
    {
    }

    //-------------------------------------------------------------------------
    //  Name : cgArray () (Constructor)
    /// <summary>
    /// Standard copy constructor. Duplicates the data contained in the
    /// specified array.
    /// </summary>
    //-------------------------------------------------------------------------
    cgArray( const cgArray<value_type> & init ) : cgArrayBase( sizeof(value_type) )
    {
        insert( end(), init.begin(), init.end() );
    }

    //-------------------------------------------------------------------------
    //  Name : cgArray () (Constructor)
    /// <summary>
    /// Constructor that allows the caller to set the initial size of the
    /// container.
    /// </summary>
    //-------------------------------------------------------------------------
    explicit cgArray( size_type initialSize ) : cgArrayBase( sizeof(value_type) )
    {
        _realloc( initialSize, initialSize, 0 );

        // Initialize any new elements with default
        for ( size_type i = 0; i < initialSize; ++i )
            new (((pointer)mStart)+i) value_type;
    }

    //-------------------------------------------------------------------------
    //  Name : cgArray () (Constructor)
    /// <summary>
    /// Constructor that allows the caller to set the initial size of the
    /// container, and populate each element with the specified default value.
    /// </summary>
    //-------------------------------------------------------------------------
    explicit cgArray( size_type initialSize, const_reference defaultInit ) : cgArrayBase( sizeof(value_type) )
    {
        _realloc( initialSize, initialSize, 0 );

        // Initialize any new elements with default
        for ( size_type i = 0; i < initialSize; ++i )
            new (((pointer)mStart)+i) value_type( defaultInit );
    }

    //-------------------------------------------------------------------------
    //  Name : cgArray () (Constructor)
    /// <summary>
    /// Constructor that allows the caller to duplicate a portion of another
    /// container with compatible iterators.
    /// </summary>
    //-------------------------------------------------------------------------
    explicit cgArray( const_iterator iterBegin, const_iterator iterEnd ) : cgArrayBase( sizeof(value_type) )
    {
        size_t initialSize = (iterEnd - iterBegin);
        _realloc( initialSize, initialSize, 0 );

        // Initialize any new elements based on the input iterators
        iterator newValue = begin();
        while ( iterBegin != iterEnd )
            new (newValue++) value_type( *iterBegin++ );
    }

    //-------------------------------------------------------------------------
    //  Name : ~cgArray () (Destructor)
    /// <summary>
    /// Destructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    ~cgArray( )
    {
        clear();
    }

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : front ()
    /// <summary>
    /// Retrieve the first element stored in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline reference front( )
    {
        return (*begin());
    }

    //-------------------------------------------------------------------------
    //  Name : front ()
    /// <summary>
    /// Retrieve the first element stored in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const_reference front( ) const
    {
        return (*begin());
    }

    //-------------------------------------------------------------------------
    //  Name : back ()
    /// <summary>
    /// Retrieve the final element stored in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline reference back( )
    {
        return (*(end()-1));
    }

    //-------------------------------------------------------------------------
    //  Name : back ()
    /// <summary>
    /// Retrieve the final element stored in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const_reference back( ) const
    {
        return (*(end()-1));
    }

    //-------------------------------------------------------------------------
    //  Name : erase ()
    /// <summary>
    /// Erase the specified element from the array.
    /// </summary>
    //-------------------------------------------------------------------------
    iterator erase( const_iterator offset )
    {
        size_type operateAt = (size_type)(offset - ((pointer)mStart));
        _realloc( mSize - 1, mCapacity, operateAt );
        return (((pointer)mStart) + operateAt);
    }

    //-------------------------------------------------------------------------
    //  Name : erase ()
    /// <summary>
    /// Erase the specified series of elements from the array.
    /// </summary>
    //-------------------------------------------------------------------------
    iterator erase( const_iterator first, const_iterator last )
    {
        size_type operateAt = (size_type)(first - ((pointer)mStart));
        _realloc( mSize - (last-first), mCapacity, operateAt );
        return (((pointer)mStart) + operateAt);
    }

    //-------------------------------------------------------------------------
    //  Name : swap ()
    /// <summary>
    /// Swap the contents of this array with the contents of the specified
    /// array.
    /// </summary>
    //-------------------------------------------------------------------------
    void swap( cgArray<value_type> & other )
    {
        // Swap buffer start
        pointer tempPointer = (pointer)mStart;
        mStart = other.mStart;
        other.mStart = (storage_type)tempPointer;

        // Swap buffer end.
        tempPointer = (pointer)mEnd;
        mEnd = other.mEnd;
        other.mEnd = (storage_type)tempPointer;

        // Swap size.
        size_type tempSize = mSize;
        mSize = other.mSize;
        other.mSize = tempSize;

        // Swap capacity.
        tempSize = mCapacity;
        mCapacity = other.mCapacity;
        other.mCapacity = tempSize;

        // Swap element size.
        tempSize = mElementSize;
        mElementSize = other.mElementSize;
        other.mElementSize = tempSize;
    }

    //-------------------------------------------------------------------------
    //  Name : resize ()
    /// <summary>
    /// Resize the array ready to store the specified number of elements.
    /// </summary>
    //-------------------------------------------------------------------------
    void resize( size_type newSize )
    {
        // No-op?
        if ( newSize == mSize )
            return;

        // Compute next capacity for the requested size so that we can
        // resize the buffer if needed. We never allow the capacity to
        // shrink in this version, which seems to correspond to MS's
        // vector implementation.
        size_type newCapacity = (newSize > mCapacity) ? _nextCapacity( mCapacity, newSize ) : mCapacity;

        // Increase capacity as required
        size_type oldSize = mSize;
        _realloc( newSize, newCapacity, mSize );

        // Initialize any new 'active' elements with default
        if ( oldSize < newSize )
        {
            for ( size_type i = oldSize; i < newSize; ++i )
                new (((pointer)mStart)+i) value_type;

        } // End if new elements added
    }

    //-------------------------------------------------------------------------
    //  Name : resize ()
    /// <summary>
    /// Resize the array ready to store the specified number of elements.
    /// Initialize any new elements with the specified default.
    /// </summary>
    //-------------------------------------------------------------------------
    void resize( size_type newSize, const_reference init )
    {
        // No-op?
        if ( newSize == mSize )
            return;

        // Compute next capacity for the requested size so that we can
        // resize the buffer if needed. We never allow the capacity to
        // shrink in this version, which seems to correspond to MS's
        // vector implementation.
        size_type newCapacity = (newSize > mCapacity) ? _nextCapacity( mCapacity, newSize ) : mCapacity;

        // Increase capacity as required
        size_type oldSize = mSize;
        _realloc( newSize, newCapacity, mSize );

        // Initialize any new 'active' elements with default
        if ( oldSize < newSize )
        {
            for ( size_type i = oldSize; i < newSize; ++i )
                new (((pointer)mStart)+i) value_type( init );

        } // End if new elements added

    }

    //-------------------------------------------------------------------------
    //  Name : reserve ()
    /// <summary>
    /// Reserve the specified amount of space in the array such that new
    /// inserts do not cause the internal buffer to be reallocated up to this
    /// new capacity.
    /// </summary>
    //-------------------------------------------------------------------------
    void reserve( size_type newCapacity )
    {
        // Reserve can never shrink the capacity.
        if ( newCapacity < mCapacity )
            return;

        // Create more (uninitialized) space in the array.
        _realloc( mSize, newCapacity, mSize );
    }

    //-------------------------------------------------------------------------
    //  Name : insert ()
    /// <summary>
    /// Insert a new item at the specified location in the array.
    /// </summary>
    //-------------------------------------------------------------------------
    iterator insert( iterator at, const_reference item )
    {
        // Compute next capacity for the required size so that we can
        // resize the buffer if needed. We never allow the capacity to
        // shrink in this version, which seems to correspond to MS's
        // vector implementation.
        size_type newSize     = mSize + 1;
        size_type newCapacity = (newSize > mCapacity) ? _nextCapacity( mCapacity, newSize ) : mCapacity;

        // Create room in the buffer.
        size_type insertAt = (at - ((pointer)mStart));
        _realloc( newSize, newCapacity, insertAt );
        
        // Use placement new to execute the relevant copy constructor
        // to duplicate the data for this item at the new location.
        new (((pointer)mStart)+insertAt) value_type( item );
        
        // Return first inserted item
        return (((pointer)mStart)+insertAt);
    }

    //-------------------------------------------------------------------------
    //  Name : insert ()
    /// <summary>
    /// Insert the specified number of new values at the specified location in 
    /// the array.
    /// </summary>
    //-------------------------------------------------------------------------
    iterator insert( iterator at, size_type count, const_reference item )
    {
        // No-op?
        if ( count == 0 )
            return at;

        // Compute next capacity for the required size so that we can
        // resize the buffer if needed. We never allow the capacity to
        // shrink in this version, which seems to correspond to MS's
        // vector implementation.
        size_type newSize     = mSize + count;
        size_type newCapacity = (newSize > mCapacity) ? _nextCapacity( mCapacity, newSize ) : mCapacity;

        // Create room in the buffer.
        size_type insertAt = (at - ((pointer)mStart));
        _realloc( newSize, newCapacity, insertAt );
        
        // Use placement new to execute the relevant copy constructor
        // or duplicate the data for this item at the new location(s).
        for ( size_type i = 0; i < count; ++i )
            new (((pointer)mStart)+insertAt+i) value_type( item );

        // Return first inserted item
        return (((pointer)mStart)+insertAt);
    }

    //-------------------------------------------------------------------------
    //  Name : insert ()
    /// <summary>
    /// Insert the specified series of values values at the indicated location in 
    /// the array. Returns a reference to the first inserted item.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _IteratorType>
    iterator insert( iterator at, _IteratorType insertBegin, _IteratorType insertEnd )
    {
        // No-op?
        if ( insertBegin == insertEnd )
            return end();

        // Count the number of elements we're inserting.
        size_type count = 0;
        for ( _IteratorType countIterator = insertBegin; countIterator != insertEnd; ++countIterator )
            ++count;

        // Compute next capacity for the required size so that we can
        // resize the buffer if needed.
        size_type newSize     = mSize + count;
        size_type newCapacity = (newSize > mCapacity) ? _nextCapacity( mCapacity, newSize ) : mCapacity;

        // Create room in the buffer.
        size_type insertAt = (at - ((pointer)mStart));
        _realloc( newSize, newCapacity, insertAt );
        
        // Use placement new to execute the relevant copy constructor
        // or duplicate the data for this item at the new location(s).
        for ( size_t i = 0; insertBegin != insertEnd; ++i, ++insertBegin )
            new (((pointer)mStart)+insertAt+i) value_type( (*insertBegin) );

        // Return first inserted item
        return (((pointer)mStart)+insertAt);
    }

    //-------------------------------------------------------------------------
    //  Name : push_back ()
    /// <summary>
    /// Insert the specified item at the end of the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void push_back( const_reference item )
    {
        // Insert the new item at the end of the array.
        insert( (pointer)mEnd, item );
    }

    //-------------------------------------------------------------------------
    //  Name : pop_back ()
    /// <summary>
    /// Remove the last item from the end of the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void pop_back( )
    {
        _realloc( mSize - 1, mCapacity, mSize - 1 );
    }

    //-------------------------------------------------------------------------
    //  Name : at ()
    /// <summary>
    /// Return a reference to the specified element within the container.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const_reference at( size_type index ) const
    {
        return *(((pointer)mStart)+index);
    }
    
    //-------------------------------------------------------------------------
    //  Name : at ()
    /// <summary>
    /// Return a reference to the specified element within the container.
    /// </summary>
    //-------------------------------------------------------------------------
    inline reference at( size_type index )
    {
        return *(((pointer)mStart)+index);
    }

    //-------------------------------------------------------------------------
    //  Name : clear ()
    /// <summary>
    /// Entirely clear the contents of the array.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void clear( )
    {
        _dealloc();
        mSize = 0;
        mCapacity = 0;
    }    

    //-------------------------------------------------------------------------
    //  Name : begin / end ()
    /// <summary>
    /// Retrieve the iterator items that allow for forward iteration through
    /// the array.
    /// </summary>
    //-------------------------------------------------------------------------
    iterator        begin() { return ((pointer)mStart); }
    const_iterator  begin() const { return ((pointer)mStart); }
    iterator        end() { return ((pointer)mEnd); }
    const_iterator  end() const { return ((pointer)mEnd); }

    //-------------------------------------------------------------------------
    //  Name : rbegin / rend ()
    /// <summary>
    /// Retrieve the iterator items that allow for reverse iteration through
    /// the array.
    /// </summary>
    //-------------------------------------------------------------------------
    // ToDo: 9999
    //iterator        rbegin() { return reverse_iterator(end()); }
    //const_iterator  rbegin() const { return const_reverse_iterator(end()); }
    //iterator        rend() { return reverse_iterator(begin()); }
    //const_iterator  rend() const { return const_reverse_iterator(begin()); }

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    inline const_reference operator[]( size_type index ) const
    {
        return at(index);
    }
    inline reference operator[]( size_type index )
    {
        return at(index);
    }
    inline cgArray<value_type> & operator= ( const cgArray<value_type> & other )
    {
        clear();
        insert( end(), other.begin(), other.end() );
        return *this;
    }
    inline bool operator< (const cgArray<value_type> & other ) const
    {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end() );
	}

private:
    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : _realloc ()
    /// <summary>
    /// Perform reallocation of the internal buffer based on the specified new
    /// size and capacity.
    /// </summary>
    //-------------------------------------------------------------------------
    void _realloc( size_type newSize, size_type newCapacity, size_type operateAt )
    {
        // How many entries are being inserted (0 if equal or shrinking)
        difference_type insertCount = (newSize > mSize) ? newSize - mSize : 0;
        
        // How many entries are being removed (0 if equal or growing)
        difference_type removeCount = (newSize < mSize) ? mSize - newSize : 0;

        // If size isn't changing, just assume we're operating at the end
        // to simplify later sections of this method.
        if ( newSize == mSize )
            operateAt = mSize;

        // Clamp the operation point to the NEW size (in case of shrinking).
        if ( operateAt > newSize )
            operateAt = newSize;

        // If existing capacity differs from the prior capacity
        // then we need to re-allocate.
        if ( newCapacity != mCapacity )
        {
            // Allocate enough room in a new buffer for new capacity (not
            // initialized at this stage). Take care to include the 'end'
            // element that always exists at the end of the array.
            char * newBuffer = new char[ (newCapacity + 1) * sizeof(value_type) ];

            // Any prior data to copy?
            if ( mStart )
            {
                // We now need to duplicate the data from the old buffer. This
                // is achieved as a two stage process for the data that falls
                // on either side of the element specified by the 'operateAt'
                // parameter. We use placement new to invoke the appropriate copy 
                // constructor in all cases since the new buffer's memory has
                // yet to be initialized.
                pointer dst  = ((pointer)newBuffer) + operateAt;
                pointer last = ((pointer)newBuffer);
                pointer src  = ((pointer)mStart) + operateAt;
                while ( dst != last )
                    new (--dst) value_type( *--src );

                // Now do the same for the other side of the point of operation.
                // We potentially need to move data after the insertion point 'up' by 
                // the required number of places indicated by the difference in size 
                // (unless we are shrinking, or inserting at the end of the array).
                dst  = ((pointer)newBuffer) + newSize;
                last = ((pointer)newBuffer) + operateAt + insertCount;
                src  = ((pointer)mEnd);
                while ( dst != last )
                    new (--dst) value_type( *--src );

            } // End if has old data

            // This is now our new buffer
            _dealloc();
            mStart = (storage_type)newBuffer;

        } // End if capacity altered
        else
        {
            // Growing or shrinking?
            if ( insertCount > 0 && operateAt < mSize )
            {
                // We potentially need to move data after the insertion point 'up' by 
                // the required number of places indicated by the difference in size.
                // For all entries that have yet to be constructed, we need to use 
                // placement new and the copy constructor.
                pointer dst   = (((pointer)mStart)+newSize);
                pointer last  = (dst-insertCount);
                pointer src   = last;
                while ( dst != last )
                    new ( --dst ) value_type( *--src );
                
                // We can just use the assignment operator for the rest.
                last = (((pointer)mStart)+operateAt+insertCount);
                while ( dst != last )
                    *--dst = *--src;
                
                // Finally; destruct the values that exist between the insert 
                // range that have since been moved by the above loop(s).
                last = (((pointer)mStart)+operateAt);
                while ( dst != last )
                    (--dst)->~value_type();

            } // End if inserting
            else if ( removeCount > 0 )
            {
                // We are potentially shrinking. First ensure that the destructor
                // is called for any elements in the buffer that are positioned
                // within the region to be removed.
                pointer dst   = ((pointer)mStart)+operateAt+removeCount;
                pointer last  = (dst-removeCount);
                while ( dst != last )
                    (--dst)->~value_type();

                // Shift values down as necessary using placement new
                // to reconstruct the values, ensuring we call the
                // destructor on each prior element as it moves.
                dst   = ((pointer)mStart)+operateAt;
                pointer src = dst + removeCount;
                last  = ((pointer)mStart)+mSize;
                while ( src != last )
                {
                    new ( dst++ ) value_type( *src );
                    (src++)->~value_type();
                }                    

            } // End if removing

        } // End if no capacity alteration

        // Resize complete.
        mSize     = newSize;
        mCapacity = newCapacity;
        mEnd      = (storage_type)(((pointer)mStart)+mSize);
    }

    //-------------------------------------------------------------------------
    //  Name : _dealloc ()
    /// <summary>
    /// Perform deallocation of the internal buffer.
    /// </summary>
    //-------------------------------------------------------------------------
    void _dealloc( )
    {
        // Call the destructor for all 'active' elements in the buffer
        // (we used placement new to initialize only those we needed).
        for ( size_type i = 0; i < mSize; ++i )
            (((pointer)mStart)+i)->~value_type();

        // Delete the internal buffer (no destructor).
        delete[] (char*)mStart;
        mStart = 0;
        mEnd   = 0;
    }

    //-------------------------------------------------------------------------
    //  Name : _nextCapacity ()
    /// <summary>
    /// Compute the next capacity that should be selected when inserting items.
    /// </summary>
    //-------------------------------------------------------------------------
    inline size_type _nextCapacity( size_type current, size_type newSize )
    {
        size_type next = current, newNext;
        while ( next < newSize )
        {
            newNext = (size_type)((float)next * CGARRAY_CAPACITYGROWTHFACTOR);
            next = (newNext == next) ? newNext + 1 : newNext;
        
        } // Next attempt
        return next;
    }
};

#endif // !_CGE_CGARRAY_H_
