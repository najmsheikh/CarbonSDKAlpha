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
// Name : cgPoolAllocator.h                                                  //
//                                                                           //
// Desc : Custom STL allocator, used as a means to provide a single          //
//        (potentially fixed and pre-allocated) pool of memory to an STL     //
//        container in order to accelerate allocations and deallocations.    //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGPOOLALLOCATOR_H_ )
#define _CGE_CGPOOLALLOCATOR_H_

//-----------------------------------------------------------------------------
// cgPoolAllocator Header Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgPoolAllocator (Class)
/// <summary>
/// Custom STL allocator, used as a means to provide a single (potentially 
/// fixed and pre-allocated) pool of memory to an STL container in order to 
/// accelerate allocations and deallocations.
/// </summary>
//-----------------------------------------------------------------------------
template <class T>
class cgPoolAllocator
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs
    //-------------------------------------------------------------------------
    typedef T           value_type;
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;

    typedef T*          pointer;
    typedef const T*    const_pointer;

    typedef T&          reference;
    typedef const T&    const_reference;

    template <class U>
    struct rebind
    {
        typedef cgPoolAllocator<U> other;
    };

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cgPoolAllocator() (Constructor)
    /// <summary>
    /// Constructor for this class.
    /// </summary>
    //-------------------------------------------------------------------------
    cgPoolAllocator( )
    {
    }

    cgPoolAllocator(int n)
    {
    }

    cgPoolAllocator( const cgPoolAllocator & init )
    {
    }

    template <class _Other>
    cgPoolAllocator(const cgPoolAllocator<_Other> &other)
    {
        /*Init();
        m_pool= other.m_pool;
        m_nPoolSize = other.m_nPoolSize;*/
    }

    private:
    void operator =(const cgPoolAllocator &);
public:

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : allocate()
    /// <summary>
    /// Allocate a specific number of the required value_type elements.
    /// </summary>
    //-------------------------------------------------------------------------
    pointer allocate( size_type nCount, const void* /*hint*/=0 )
    {
        return (pointer)malloc(nCount * sizeof(T));
    }
 
    //-------------------------------------------------------------------------
    // Name : deallocate()
    /// <summary>
    /// Deallocate the specified value_type elements.
    /// </summary>
    //-------------------------------------------------------------------------
    void deallocate( pointer p, size_type /*n*/ )
    {
        free( p );
    }

    //-------------------------------------------------------------------------
    // Name : construct()
    /// <summary>
    /// Construct a single item of the required value_type.
    /// </summary>
    //-------------------------------------------------------------------------
    void construct( pointer p, const T & val )
    {
        new (p) T(val);
    }
 
    //-------------------------------------------------------------------------
    // Name : destroy()
    /// <summary>
    /// Destroy a single item of the required value_type.
    /// </summary>
    //-------------------------------------------------------------------------
    void destroy( pointer p )
    {
        p->~T();
    }
 
    //-------------------------------------------------------------------------
    // Name : max_size()
    /// <summary>
    /// Describe the maximum number of elements that may be constructed.
    /// </summary>
    //-------------------------------------------------------------------------
    size_type max_size() const
    {
        return 0xFFFFFFFF / sizeof(T);
    }
};

#endif // !_CGE_CGPOOLALLOCATOR_H_