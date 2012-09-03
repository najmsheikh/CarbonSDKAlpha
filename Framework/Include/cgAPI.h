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
// File : cgAPI.h                                                            //
//                                                                           //
// Desc : Provides support types, macros and other items necessary to        //
//        interoperate correctly with DLL or library exports.                //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGAPI_H_ )
#define _CGE_CGAPI_H_

//-----------------------------------------------------------------------------
// cgAPI Header Includes
//-----------------------------------------------------------------------------
#include <vector>
#include <list>
#include <stack>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

//-----------------------------------------------------------------------------
// STL Export
//-----------------------------------------------------------------------------
// Disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
// Disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
// Disable 'needs to have dll-interface' warnings (template workaround).
#pragma warning (disable : 4251)

//-----------------------------------------------------------------------------
// Global Defines & Macros
//-----------------------------------------------------------------------------
// Macros to provide automatic integration with DLL interface where applicable.

// DLL compilation
#if defined( CGE_DLL_EXPORTS ) || defined( CGE_DLL_REEXPORTS )
#   define CGE_API __declspec(dllexport)
#elif( CGE_DLL_IMPORTS )
#   define CGE_API __declspec(dllimport)
#else
#   define CGE_API
#endif

// Utilities
#define __CGE_ANSI_STRING(_String) #_String
#define _CGE_ANSI_STRING(_String) __CGE_ANSI_STRING(_String)
#define __CGE_WIDE_STRING(_String) L ## _String
#define _CGE_WIDE_STRING(_String) __CGE_WIDE_STRING(_String)
#if defined(UNICODE) || defined(_UNICODE)
#   define _CGE_T_STRING(_String) __CGE_WIDE_STRING(_String)
#else
#   define _CGE_T_STRING(_String) __CGE_ANSI_STRING(_String)
#endif

// Asserts
#if defined(_DEBUG) || defined(CGE_ALWAYS_ASSERT)
    void cgAssertHandler( wchar_t * _Expr, wchar_t * _File, int _Line, wchar_t * _Msg ); // cgBase.cpp
    void cgToDoHandler( wchar_t * _Process, wchar_t * _File, int _Line, wchar_t * _Note ); // cgBase.cpp
#   define cgAssert(_Expr) (void)( (!!(_Expr)) || (cgAssertHandler(_CGE_WIDE_STRING(#_Expr), _CGE_WIDE_STRING(__FILE__), __LINE__, 0), 0) )
#   define cgAssertEx(_Expr,_Msg) (void)( (!!(_Expr)) || (cgAssertHandler(_CGE_WIDE_STRING(#_Expr), _CGE_WIDE_STRING(__FILE__), __LINE__, _CGE_WIDE_STRING(_Msg)), 0) )
#else
#   define cgAssert(_Expr)
#   define cgAssertEx(_Expr,_Msg)
#endif

// ToDo / Notes
#if 0 //defined(_DEBUG)
#   define _CGE_FILE_LINE __FILE__ "(" _CGE_ANSI_STRING(__LINE__) ") : "
#   define cgToDo(_Process,_Note) \
            __pragma(message( _CGE_FILE_LINE \
            "<" #_Process "> ToDo : " #_Note "\n" ))
#   define cgToDoAssert(_Process,_Note) \
            __pragma(message( _CGE_FILE_LINE \
            "<" #_Process "> ToDo : " #_Note "\n" )) \
            cgToDoHandler(_CGE_WIDE_STRING(#_Process), _CGE_WIDE_STRING(__FILE__), __LINE__, _CGE_WIDE_STRING(#_Note) );
#else
#   define cgToDo(_Process,_Note)
#   define cgToDoAssert(_Process,_Note)
#endif

// Utilities
#define cgMakeVersion( version, subversion, revision ) \
                       (((version & 0xFF) << 24) +  \
                       ((subversion & 0xFF) << 16) +  \
                       (revision & 0xFFFF))
#define cgDecomposeVersion( packedVersion, version, subversion, revision ) \
                        {version = ((packedVersion >> 24)&0xFF); \
                        subversion = ((packedVersion >> 16)&0xFF); \
                        revision = (packedVersion&0xFFFF);} \

//-----------------------------------------------------------------------------
//  Name : cgAutoPtr (Template Class)
/// <summary>
/// Custom auto pointer template class that allows the application to
/// manage variables as pointers (that do not require exporting).
/// </summary>
//-----------------------------------------------------------------------------
template <class _Vt, class _Et=size_t>
class CGE_API cgAutoPtr
{
    //-------------------------------------------------------------------------
    // Private Typedefs
    //-------------------------------------------------------------------------
    typedef cgAutoPtr<_Vt,_Et>  _self_type;

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : cgAutoPtr (Default Constructor)
    /// <summary>Default constructor for this class.</summary>
    //-------------------------------------------------------------------------
    cgAutoPtr( ) : _data( new Data() ) { }

    //-------------------------------------------------------------------------
    //  Name : cgAutoPtr (Copy Constructor)
    /// <summary>
    /// Copy constructor for this class. Duplicates the specified cgAutoPtr
    /// internal data and simply increments the reference count.
    /// </summary>
    //-------------------------------------------------------------------------
    cgAutoPtr( const _self_type& value ) : _data( value._data )
    {
        ++_data->refCount;
    }
    
    //-------------------------------------------------------------------------
    //  Name : ~cgAutoPtr (Destructor)
    /// <summary>
    /// Destructor for this class. Releases our reference to the shared
    /// data structure and deletes if necessary.
    /// </summary>
    //-------------------------------------------------------------------------
    ~cgAutoPtr( )
    {
        if ( --_data->refCount == 0 )
            delete _data;
    }
    
    //-------------------------------------------------------------------------
    //  Name : operator* ()
    /// <summary>
    /// Non-const dereference operator. If we are currently sharing the
    /// data with some other cgAutoPtr, performs the deferred copy and
    /// makes the data unique.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _Vt & operator*( )
    {
        _makeUnique();
        return _data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator* (const)
    /// <summary>
    /// Const dereference operator. Simply returns reference to the data
    /// shared or otherwise.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const _Vt & operator*( ) const
    {
        return _data->value;
    }
    
    //-------------------------------------------------------------------------
    //  Name : operator-> ()
    /// <summary>
    /// Non-const dereference operator. If we are currently sharing the
    /// data with some other cgAutoPtr, performs the deferred copy and
    /// makes the data unique.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _Vt * operator->()
    {
        _makeUnique();
        return &_data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator-> (const)
    /// <summary>
    /// Const dereference operator. Simply returns reference to the data
    /// shared or otherwise.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const _Vt * operator->() const
    {
        return &_data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator _Vt& ()
    /// <summary>
    /// Non-const conversion operator. If we are currently sharing the
    /// data with some other cgAutoPtr, performs the deferred copy and
    /// makes the data unique.
    /// </summary>
    //-------------------------------------------------------------------------
    inline operator _Vt&()
    {
        _makeUnique();
        return _data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator const _Vt& ()
    /// <summary>
    /// Const conversion operator. Simply returns reference to the data
    /// shared or otherwise.
    /// </summary>
    //-------------------------------------------------------------------------
    inline operator const _Vt&() const
    {
        return _data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator&()
    /// <summary>
    /// Non-const conversion operator. If we are currently sharing the
    /// data with some other cgAutoPtr, performs the deferred copy and
    /// makes the data unique.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _Vt * operator&()
    {
        _makeUnique();
        return &_data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator& (const)
    /// <summary>
    /// Const conversion operator. Simply returns reference to the data
    /// shared or otherwise.
    /// </summary>
    //-------------------------------------------------------------------------
    inline const _Vt* operator&() const
    {
        return &_data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator= ( const _Vt&)
    /// <summary>
    /// Assignment operator designed to duplicate the data in the specified 
    /// wrapped type into this instance' local copy. If we are not currently
    /// sharing our data with another cgAutoPtr, then we can simply pass it
    /// through to the wrapped object.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _Vt& operator=( const _Vt& value )
    {
        if ( _data->refCount == 1 )
        {
            _data->value = value;
        
        } // End if not shared
        else
        {
            Data * newData = new Data(value);
            if ( --_data->refCount == 0 )
                delete _data;
            _data = newData;
        
        } // End if shared
        return _data->value;
    }

    //-------------------------------------------------------------------------
    //  Name : operator= (const _self_type&)
    /// <summary>
    /// Assignment operator designed to assign data from another cgAutoPtr.
    /// In this case we simply reference the incoming cgAutoPtr::_data item
    /// and increment its reference count. This essentially provides
    /// support for a deferred copy of the data, where this instance of the
    /// cgAutoPtr will make itself "unique" only at the last minute if
    /// necessary.
    /// </summary>
    //-------------------------------------------------------------------------
    inline _self_type& operator=( const _self_type& value )
    {
        ++value._data->refCount;
        if ( --_data->refCount == 0 )
            delete _data;
        _data = value._data;          
        return *this;
    }
    
    //-------------------------------------------------------------------------
    //  Name : operator[] ()
    /// <summary>
    /// Indexing operator. Allows callers to retrieve elements of the 
    /// underlying data type should it expose this operator. Since this
    /// returns a non-const element, we need to ensure that the data we
    /// are referencing is unique at this stage.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _IndexType>
    inline _Et & operator[](_IndexType n)
    {
        _makeUnique();
        return _data->value[n];
    }

    //-------------------------------------------------------------------------
    //  Name : operator[] ( const)
    /// <summary>
    /// Indexing operator. Allows callers to retrieve elements of the 
    /// underlying data type should it expose this operator.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _IndexType>
    inline const _Et & operator[](_IndexType n) const
    {
        return _data->value[n];
    }

private:
    //-------------------------------------------------------------------------
    // Private Structures
    //-------------------------------------------------------------------------
    // Houses the actual data being managed by the cgAutoPtr object.
    struct Data
    {
        Data() : refCount(1) {}
        Data( const _Vt & v ) : value(v), refCount(1) {}
        _Vt     value;
        size_t  refCount;
    };

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //  Name : _makeUnique ()
    /// <summary>
    /// If the data stored in this object is currently shared with other
    /// cgAutoPtr objects then its data will be duplicated in order to
    /// ensure that this is the only one referencing it.
    /// </summary>
    //-------------------------------------------------------------------------
    inline void _makeUnique( )
    {
        // Are we sharing?
        if ( _data->refCount > 1 )
        {
            // Remove our reference and duplicate the data.
            --_data->refCount;
            _data = new Data(_data->value);
        
        } // End if shared?
    }
 
    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    Data * _data;

}; // End Class cgAutoPtr

/// <summary>Simple vector typedef. No export required.</summary>
#define CGE_VECTOR_DECLARE( _value, _typedefName ) \
    typedef std::vector<_value> _typedefName;
    //typedef cgAutoPtr<std::vector<_value>,_value> _typedefName##Ptr;

/// <summary>Simple list typedef. No export required.</summary>
#define CGE_LIST_DECLARE( _value, _typedefName ) \
    typedef std::list<_value> _typedefName;
    //typedef cgAutoPtr<std::list<_value>,_value> _typedefName##Ptr;
    
/// <summary>Simple stack typedef. No export required.</summary>
#define CGE_STACK_DECLARE( _value, _typedefName ) \
    typedef std::stack<_value> _typedefName;
    //typedef cgAutoPtr<std::stack<_value>,_value> _typedefName##Ptr;

/// <summary>Simple deque typedef. No export required.</summary>
#define CGE_DEQUE_DECLARE( _value, _typedefName ) \
    typedef std::deque<_value> _typedefName;
    //typedef cgAutoPtr<std::deque<_value>,_value> _typedefName##Ptr;

/// <summary>Simple map typedef. No export required.</summary>
#define CGE_MAP_DECLARE( _key, _value, _typedefName ) \
    typedef std::map<_key,_value> _typedefName;
    //typedef cgAutoPtr<std::map<_key,_value>,_value> _typedefName##Ptr;

/// <summary>Simple set typedef. No export required.</summary>
#define CGE_SET_DECLARE( _value, _typedefName ) \
    typedef std::set<_value> _typedefName;
    //typedef cgAutoPtr<std::set<_value>,_value> _typedefName##Ptr;

/// <summary>Simple unordered_map typedef. No export required.</summary>
#define CGE_UNORDEREDMAP_DECLARE( _key, _value, _typedefName ) \
    typedef std::tr1::unordered_map<_key,_value> _typedefName;
    //typedef cgAutoPtr<std::tr1::unordered_map<_key,_value>,_value> _typedefName##Ptr;

/// <summary>Simple unordered_set typedef. No export required.</summary>
#define CGE_UNORDEREDSET_DECLARE( _value, _typedefName ) \
    typedef std::tr1::unordered_set<_value> _typedefName;
    //typedef cgAutoPtr<std::tr1::unordered_set<_value>,_value> _typedefName##Ptr;


/// <summary>Simple vector typedef (with allocator). No export required.</summary>
#define CGE_VECTOR_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::vector<_value, _allocType> _typedefName;
    //typedef cgAutoPtr<std::vector<_value, _allocType>,_value> _typedefName##Ptr;

/// <summary>Simple list typedef (with allocator). No export required.</summary>
#define CGE_LIST_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::list<_value, _allocType> _typedefName;
    //typedef cgAutoPtr<std::list<_value, _allocType>,_value> _typedefName##Ptr;
    
/// <summary>Simple stack typedef (with allocator). No export required.</summary>
#define CGE_STACK_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::stack<_value, std::deque<_value,_allocType> > _typedefName;
    //typedef cgAutoPtr<std::stack<_value, std::deque<_value,_allocType> >,_value> _typedefName##Ptr;

/// <summary>Simple deque typedef (with allocator). No export required.</summary>
#define CGE_DEQUE_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::deque<_value, _allocType> _typedefName;
    //typedef cgAutoPtr<std::deque<_value, _allocType>,_value> _typedefName##Ptr;

/// <summary>Simple map typedef (with allocator). No export required.</summary>
#define CGE_MAP_ALLOC_DECLARE( _key, _value, _allocType, _typedefName ) \
    typedef std::map<_key,_value, std::less<_value>, _allocType> _typedefName;
    //typedef cgAutoPtr<std::map<_key,_value, std::less<_value>, _allocType>,_value> _typedefName##Ptr;

/// <summary>Simple set typedef (with allocator). No export required.</summary>
#define CGE_SET_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::set<_value, std::less<_value>, _allocType> _typedefName;
    //typedef cgAutoPtr<std::set<_value, std::less<_value>, _allocType>,_value> _typedefName##Ptr;

/// <summary>Simple unordered_map typedef (with allocator). No export required.</summary>
#define CGE_UNORDEREDMAP_ALLOC_DECLARE( _key, _value, _allocType, _typedefName ) \
    typedef std::tr1::unordered_map<_key,_value, std::hash<_key>, std::equal_to<_key>, _allocType> _typedefName;
    //typedef cgAutoPtr<std::tr1::unordered_map<_key,_value, std::hash<_key>, std::equal_to<_key>, _allocType>,_value> _typedefName##Ptr;

/// <summary>Simple unordered_set typedef (with allocator). No export required.</summary>
#define CGE_UNORDEREDSET_ALLOC_DECLARE( _value, _allocType, _typedefName ) \
    typedef std::tr1::unordered_set<_value, std::hash<_value>, std::equal_to<_value>, _allocType> _typedefName;
    //typedef cgAutoPtr<std::tr1::unordered_set<_value, std::hash<_value>, std::equal_to<_value>, _allocType>,_value> _typedefName##Ptr;

#endif // !_CGE_CGAPI_H_